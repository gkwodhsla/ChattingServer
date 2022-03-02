#include "ChattingBuilding.h"
#include "Outsourcer.h"
#include "TotalManager.h"
#include <iostream>
#include <chrono>

ChattingBuilding::ChattingBuilding():ShouldLogicStop(false)
{
	for (int i = 0; i < MAX_ROOM_NUM; ++i)
	{
		RoomNames[i] = "";
		MaximumParticipants[i] = 0;
		IsEmptyRooms[i] = true;
		CurParticipantInRooms[i] = 0;
		RoomCreatingTimes[i] = "";
	}
	FD_ZERO(&ReadSet);
	FD_ZERO(&WriteSet);
}

void ChattingBuilding::ProcessingLogic()
{
	struct timeval timeout;

	timeout.tv_sec = 0;
	timeout.tv_usec = ChattingBuilding::SOCKET_TIME_WAIT_US;

	while (!ShouldLogicStop)
	{
		FD_ZERO(&WriteSet);
		FD_ZERO(&ReadSet);

		lock.lock();
		for (int i = 0; i < MAX_ROOM_NUM; ++i)
		{
			for (int j = 0; j < ClientInfosEachRoom[i].size(); ++j)
			{
				if (ClientInfosEachRoom[i][j].IsSend)//전송중인 데이터가 남았다면 마저 보내게 해줍니다.
				{
					FD_SET(ClientInfosEachRoom[i][j].ClientSock, &WriteSet);
				}
				else //전송중인 데이터가 남았을 땐 recv를 잠시 미룹니다.
				{
					FD_SET(ClientInfosEachRoom[i][j].ClientSock, &ReadSet);
				}
			}
		}
		lock.unlock();
		select(0, &ReadSet, &WriteSet, nullptr, &timeout);
		lock.lock();
		ProcessingAfterSelect();
		lock.unlock();
	}
}

unsigned int ChattingBuilding::OccupyingRoom(const std::string& RoomName, ClientInfo& Client, const int MaximumParticipant)
{
	unsigned int roomIndex = 0;
	for (int i = 0; i < MAX_ROOM_NUM; ++i)
	{
		if (IsEmptyRooms[i])
		{
			IsEmptyRooms[i] = false;
			RoomNames[i] = RoomName;
			MaximumParticipants[i] = MaximumParticipant;
			RoomCreatingTimes[i] = GetCurrentSystemTime();
			EnteringRoom(i, Client);
			roomIndex = i;
			break;
		}
	}
	return roomIndex;

}

bool ChattingBuilding::IsThereAnyEmptyRoom()
{
	for (int i = 0; i < MAX_ROOM_NUM; ++i)
	{
		if (IsEmptyRooms[i]) //빈 방을 발견하면 true를 반환해줍니다.
		{
			return true;
		}
	}
	return false;
}

void ChattingBuilding::EnteringRoom(const int RoomIndex, ClientInfo& Client)
{
	std::string roomEnterMsg = "";
	if (IsEmptyRooms[RoomIndex]) //개설되지 않은 방에 접속하려는 경우 클라이언트에게 안된다고 알려줍니다.
	{
		roomEnterMsg = "You can't entering the room (room is not exist)\r\n";
		CustomSend(Client.ClientSock, roomEnterMsg.c_str(), roomEnterMsg.size(), 0, Client);
		return;
	}

	if (CurParticipantInRooms[RoomIndex] + 1 <= MaximumParticipants[RoomIndex]) //정상적으로 접속
	{
		Client.IsJoinRoom = true;
		lock.lock();
		ClientInfosEachRoom[RoomIndex].emplace_back(Client);
		ClientInfosEachRoom[RoomIndex].back().EnteringTime = GetCurrentSystemTime();
		lock.unlock();
		//방 진입을 요청한 클라이언트의 정보를 복사해 요청한 방에 넣어줍니다.
		//TotalManager 관리 배열에서 클라이언트를 직접 이동 시키면 한곳에서 관리가 어려울 것 같아서
		//이렇게 구현했습니다.
		roomEnterMsg = "Room name: " + RoomNames[RoomIndex] + "(" + std::to_string(++CurParticipantInRooms[RoomIndex]) +
			"/" + std::to_string(MaximumParticipants[RoomIndex]) + ")\r\n";
		CustomSend(Client.ClientSock, roomEnterMsg.c_str(), roomEnterMsg.size(), 0, Client);
	}
	else //방 최대 인원을 넘어선 경우 접속하지 못 한다고 알려줍니다.
	{
		roomEnterMsg = "You can't enter this room (room fulled already)\r\n";
		CustomSend(Client.ClientSock, roomEnterMsg.c_str(), roomEnterMsg.size(), 0, Client);
	}
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
			SOCKET ClientSock = ClientInfosEachRoom[i][j].ClientSock;
			std::array<char, 1024>& buffer = ClientInfosEachRoom[i][j].Buffer;

			if (FD_ISSET(ClientSock, &WriteSet))  //데이터를 보내야하는 경우
			{
				std::pair<bool, int> sendResult =
					CustomSend(ClientSock, buffer.data(), ClientInfosEachRoom[i][j].SendingRightPos - ClientInfosEachRoom[i][j].SendingLeftPos,
						0, ClientInfosEachRoom[i][j]);

				if (sendResult.second == SOCKET_ERROR)
				{
					std::cout << "disconnect" << std::endl;
					RemoveClientSocket(i, j);
					continue;
				}
			}
			else if (FD_ISSET(ClientSock, &ReadSet)) // 데이터를 읽어야하는 경우
			{
				std::pair<bool, int>recvResult =
					CustomRecv(ClientSock, buffer.data(), ClientInfo::MAX_BUFFER_SIZE, 0, ClientInfosEachRoom[i][j]);

				if (recvResult.second == SOCKET_ERROR)
				{
					std::cout << "disconnect" << std::endl;
					RemoveClientSocket(i, j);
					continue;
				}
				//만약 recv에 문제가 생기거나 클라이언트와 연결이 끊기면
				//관리 배열에서 해당 클라이언트의 정보를 빼줍니다.

				if (recvResult.first)//클라이언트가 \r\n을 입력했다면
				{
					if (buffer.data()[0] == '/')
					{
						std::string command = std::string(buffer.data(), buffer.data() + recvResult.second);
						Outsourcer::Instance().ExecutingCommand(ClientInfosEachRoom[i][j], -1, command);
						//2번 째 인자는 채팅방 명령 수행 시 무시됩니다.
						//2번 째 인자는 TotalManager에서 방 만들기 명령을 수행했을 때만 사용됩니다.
						//명령을 수행하는 함수를 여러개 만들면 사용할 때 헷갈릴듯하여 이렇게 구현했습니다.
					}
					else
					{
						buffer.data()[recvResult.second] = '\0';

						std::string msgToSend = "\r\n" + ClientInfosEachRoom[i][j].Name + ": "
							+ std::string(buffer.data(), buffer.data() + recvResult.second) + "\r\n";
						//메세지의 형식은 다른 클라이언트 이름: 메시지 내용 입니다.
						//e.g.) HJO: Hello world
						for (int k = 0; k < ClientInfosEachRoom[i].size(); ++k)
						{
							CustomSend(ClientInfosEachRoom[i][k].ClientSock, msgToSend.c_str(), msgToSend.size(), 0, ClientInfosEachRoom[i][k]);
						}
						//같은 방에 속한 모든 클라이언트(본인 포함)에게 채팅내용을 보내줍니다.

						std::cout << "Thread ID: " << std::this_thread::get_id() << ", Room Name: " << RoomNames[i] << ", Content: " << ClientInfosEachRoom[i][j].Buffer.data() << std::endl;
					}
					ZeroMemory(buffer.data(), ClientInfo::MAX_BUFFER_SIZE);
					ClientInfosEachRoom[i][j].RecvSize = 0;
				}
			}
		}
	}
}

void ChattingBuilding::RemoveClientSocket(int RoomNumber, int Index)
{
	TotalManager::ClientInfoLock.lock();
	TotalManager::Instance().RemoveClientSocket(ClientInfosEachRoom[RoomNumber][Index]);
	TotalManager::ClientInfoLock.unlock();
	--CurParticipantInRooms[RoomNumber];
	//클라이언트 소켓이 해당 방을 떠난다면 참가 인원 수를 감소시켜줍니다.
	ClientInfosEachRoom[RoomNumber].erase(ClientInfosEachRoom[RoomNumber].begin() + Index);
}

const std::vector<std::string> ChattingBuilding::GetUsersNameAndEnteringTime(unsigned int RoomIndex) const
{
	std::vector<std::string> retVal;

	for (int i = 0; i < ClientInfosEachRoom[RoomIndex].size(); ++i)
	{
		retVal.emplace_back(ClientInfosEachRoom[RoomIndex][i].Name + "\t" + ClientInfosEachRoom[RoomIndex][i].EnteringTime + "\r\n");
	}
	return retVal;
}
