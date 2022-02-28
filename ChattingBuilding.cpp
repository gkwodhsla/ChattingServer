#include "ChattingBuilding.h"
#include "TotalManager.h"
#include <iostream>
#include <mutex>

std::mutex gLock;

ChattingBuilding::ChattingBuilding()
{
}

void ChattingBuilding::ProcessingLogic()
{
	struct timeval timeout;

	timeout.tv_sec = 0;
	timeout.tv_usec = 500;

	while (!ShouldLogicStop)
	{
		gLock.lock();
		FD_ZERO(&WriteSet);
		FD_ZERO(&ReadSet);

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
		//여기서부터 로직 처리를 진행한다.
		ProcessingAfterSelect();
		gLock.unlock();
	}
}

void ChattingBuilding::PrintAllRoomName()
{
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
	ClientInfosEachRoom[RoomIndex].emplace_back(Client.ClntSock);
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
			else if (FD_ISSET(clntSock, &ReadSet)) // 데이터를 읽어온 경우 
			{
				int rcvSize = 0;
				std::array<char, 1024>& buffer = ClientInfosEachRoom[i][j].Buffer;

				ZeroMemory(buffer.data(), ClientInfo::MAX_BUFFER_SIZE);
				//클라이언트에게 메시지를 받기 전에 버퍼를 비워준다.
				//보낼 데이터가 없는 경우에만 read_set에 소켓을 넣어주기 때문에 보낼 데이터가 유실되지 않는다.
				rcvSize = recv(clntSock, buffer.data(), ClientInfo::MAX_BUFFER_SIZE, 0);

				if (rcvSize == SOCKET_ERROR)
				{
					std::cout << "recv error" << std::endl;
					RemoveClntSocket(i, j);
					continue;
				}
				else if (rcvSize == 0)
				{
					std::cout << "disconnect" << std::endl;
					RemoveClntSocket(i, j);
					continue;
				}


				buffer.data()[rcvSize] = '\0';

				std::string msgToSend = std::string("\r\nOther Client Name: ") + std::string(buffer.data(), buffer.data() + rcvSize) + "\r\n";
				//메세지의 형식은 다른 클라이언트 이름: 메시지 내용 이다.
				//e.g.) HJO: Hello world
				//추후 로그인 기능까지 구현이 되면 Other Client Name에 실제 유저의 이름을 넣어줄 예정.
				for (int k = 0; k < ClientInfosEachRoom[i].size(); ++k)
				{
					send(ClientInfosEachRoom[i][k].ClntSock, msgToSend.c_str(), msgToSend.size(), 0);
				}

				std::cout << "Thread ID: " << std::this_thread::get_id() << ", Room Name: " << RoomNames[i] << ", Content: " << ClientInfosEachRoom[i][j].Buffer.data() << std::endl;
			}
		}
	}
}

void ChattingBuilding::RemoveClntSocket(int RoomNumber, int Index)
{
	TotalManager::Instance().MarkingForRemoveClntSocket(ClientInfosEachRoom[RoomNumber][Index].ClntSock);
	ClientInfosEachRoom[RoomNumber].erase(ClientInfosEachRoom[RoomNumber].begin() + Index);
}