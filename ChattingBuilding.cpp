#include "ChattingBuilding.h"
#include "TotalManager.h"
#include <iostream>

ChattingBuilding::ChattingBuilding():ShouldLogicStop(false)
{
	for (int i = 0; i < MAX_ROOM_NUM; ++i)
	{
		RoomNames[i] = "";
		MaximumParticipants[i] = 0;
		IsEmptyRoom[i] = true;
	}
	FD_ZERO(&ReadSet);
	FD_ZERO(&WriteSet);
}

void ChattingBuilding::ProcessingLogic()
{
	struct timeval timeout;

	timeout.tv_sec = 0;
	timeout.tv_usec = ChattingBuilding::SOCKET_TIME_WAIT_MS;

	while (!ShouldLogicStop)
	{
		FD_ZERO(&WriteSet);
		FD_ZERO(&ReadSet);

		lock.lock();
		for (int i = 0; i < MAX_ROOM_NUM; ++i)
		{
			for (int j = 0; j < ClientInfosEachRoom[i].size(); ++j)
			{
				if (ClientInfosEachRoom[i][j].IsSend)
				{
					FD_SET(ClientInfosEachRoom[i][j].ClntSock, &WriteSet);
				}
				else
				{
					FD_SET(ClientInfosEachRoom[i][j].ClntSock, &ReadSet);
				}
			}
		}
		select(0, &ReadSet, &WriteSet, nullptr, &timeout);
		ProcessingAfterSelect();
		lock.unlock();
	}
}

void ChattingBuilding::OccupyingRoom(const std::string& RoomName, ClientInfo& Client, const int MaximumParticipant)
{
	for (int i = 0; i < MAX_ROOM_NUM; ++i)
	{
		if (IsEmptyRoom[i])
		{
			IsEmptyRoom[i] = false;
			RoomNames[i] = RoomName;
			MaximumParticipants[i] = MaximumParticipant;
			EnteringRoom(i, Client);
			break;
		}
	}
}

bool ChattingBuilding::IsThereAnyEmptyRoom()
{
	for (int i = 0; i < MAX_ROOM_NUM; ++i)
	{
		if (IsEmptyRoom[i]) //빈 방을 발견하면 true를 반환하게 해준다.
		{
			return true;
		}
	}
	return false;
}

void ChattingBuilding::EnteringRoom(const int RoomIndex, ClientInfo& Client)
{
	Client.IsJoinRoom = true;
	lock.lock();
	ClientInfosEachRoom[RoomIndex].emplace_back(Client.ClntSock);
	lock.unlock();
	//소켓 디스크립터만 복사해 새롭게 정보를 만들어 넣어준다.
	//TotalManager 관리 배열에서 이동을 시키면 한곳에서 관리가 어려울 것 같아서
	//이렇게 구현했습니다.
}

ChattingBuilding::~ChattingBuilding()
{
}

void ChattingBuilding::ProcessingAfterSelect()
{
	for (int i = 0; i < MAX_ROOM_NUM; ++i)
	{
		for (int j = 0; j < ClientInfosEachRoom[i].size(); ++j)
		{
			SOCKET clntSock = ClientInfosEachRoom[i][j].ClntSock;

			if (FD_ISSET(clntSock, &WriteSet))  //데이터를 보내야하는 경우
			{

			}
			else if (FD_ISSET(clntSock, &ReadSet)) // 데이터를 읽어야하는 경우
			{
				std::array<char, 1024>& buffer = ClientInfosEachRoom[i][j].Buffer;

				std::pair<bool, int>rcvResult =
					CustomRecv(clntSock, buffer.data(), ClientInfo::MAX_BUFFER_SIZE, 0, ClientInfosEachRoom[i][j]);

				if (rcvResult.second == 0)
				{
					std::cout << "disconnect" << std::endl;
					RemoveClntSocket(i, j);
					continue;
				}
				//만약 recv에 문제가 생기거나 클라이언트와 연결이 끊기면
				//관리 배열에서 해당 클라이언트의 정보를 빼준다.

				if (rcvResult.first)
				{
					buffer.data()[rcvResult.second] = '\0';

					std::string msgToSend = std::string("\r\nOther Client Name: ") + std::string(buffer.data(), buffer.data() + rcvResult.second) + "\r\n";
					//메세지의 형식은 다른 클라이언트 이름: 메시지 내용 이다.
					//e.g.) HJO: Hello world
					//(추후 로그인 기능까지 구현이 되면 Other Client Name에 실제 유저의 이름을 넣어줄 예정입니다.)
					for (int k = 0; k < ClientInfosEachRoom[i].size(); ++k)
					{
						send(ClientInfosEachRoom[i][k].ClntSock, msgToSend.c_str(), msgToSend.size(), 0);
					}
					//같은 방에 속한 모든 클라이언트(본인 포함)에게 채팅내용을 보내준다.

					std::cout << "Thread ID: " << std::this_thread::get_id() << ", Room Name: " << RoomNames[i] << ", Content: " << ClientInfosEachRoom[i][j].Buffer.data() << std::endl;
					ZeroMemory(buffer.data(), ClientInfo::MAX_BUFFER_SIZE);
					ClientInfosEachRoom[i][j].RcvSize = 0;
				}
			}
		}
	}
}

void ChattingBuilding::RemoveClntSocket(int RoomNumber, int Index)
{
	TotalManager::Instance().MarkingForRemoveClntSocket(ClientInfosEachRoom[RoomNumber][Index].ClntSock);
	//서브 쓰레드에서 메인 쓰레드에서 관리하는 소켓 정보를 직접 수정하면 문제가 생길 수 있을 것 같아
	//마킹만 해놓고 직접 제거하는 것은 메인 쓰레드가 하게끔 구현했습니다.
	ClientInfosEachRoom[RoomNumber].erase(ClientInfosEachRoom[RoomNumber].begin() + Index);
}