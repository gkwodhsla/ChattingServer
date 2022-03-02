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
				if (ClientInfosEachRoom[i][j].IsSend)//�������� �����Ͱ� ���Ҵٸ� ���� ������ ���ݴϴ�.
				{
					FD_SET(ClientInfosEachRoom[i][j].ClientSock, &WriteSet);
				}
				else //�������� �����Ͱ� ������ �� recv�� ��� �̷�ϴ�.
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
		if (IsEmptyRooms[i]) //�� ���� �߰��ϸ� true�� ��ȯ���ݴϴ�.
		{
			return true;
		}
	}
	return false;
}

void ChattingBuilding::EnteringRoom(const int RoomIndex, ClientInfo& Client)
{
	std::string roomEnterMsg = "";
	if (IsEmptyRooms[RoomIndex]) //�������� ���� �濡 �����Ϸ��� ��� Ŭ���̾�Ʈ���� �ȵȴٰ� �˷��ݴϴ�.
	{
		roomEnterMsg = "You can't entering the room (room is not exist)\r\n";
		CustomSend(Client.ClientSock, roomEnterMsg.c_str(), roomEnterMsg.size(), 0, Client);
		return;
	}

	if (CurParticipantInRooms[RoomIndex] + 1 <= MaximumParticipants[RoomIndex]) //���������� ����
	{
		Client.IsJoinRoom = true;
		lock.lock();
		ClientInfosEachRoom[RoomIndex].emplace_back(Client);
		ClientInfosEachRoom[RoomIndex].back().EnteringTime = GetCurrentSystemTime();
		lock.unlock();
		//�� ������ ��û�� Ŭ���̾�Ʈ�� ������ ������ ��û�� �濡 �־��ݴϴ�.
		//TotalManager ���� �迭���� Ŭ���̾�Ʈ�� ���� �̵� ��Ű�� �Ѱ����� ������ ����� �� ���Ƽ�
		//�̷��� �����߽��ϴ�.
		roomEnterMsg = "Room name: " + RoomNames[RoomIndex] + "(" + std::to_string(++CurParticipantInRooms[RoomIndex]) +
			"/" + std::to_string(MaximumParticipants[RoomIndex]) + ")\r\n";
		CustomSend(Client.ClientSock, roomEnterMsg.c_str(), roomEnterMsg.size(), 0, Client);
	}
	else //�� �ִ� �ο��� �Ѿ ��� �������� �� �Ѵٰ� �˷��ݴϴ�.
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

			if (FD_ISSET(ClientSock, &WriteSet))  //�����͸� �������ϴ� ���
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
			else if (FD_ISSET(ClientSock, &ReadSet)) // �����͸� �о���ϴ� ���
			{
				std::pair<bool, int>recvResult =
					CustomRecv(ClientSock, buffer.data(), ClientInfo::MAX_BUFFER_SIZE, 0, ClientInfosEachRoom[i][j]);

				if (recvResult.second == SOCKET_ERROR)
				{
					std::cout << "disconnect" << std::endl;
					RemoveClientSocket(i, j);
					continue;
				}
				//���� recv�� ������ ����ų� Ŭ���̾�Ʈ�� ������ �����
				//���� �迭���� �ش� Ŭ���̾�Ʈ�� ������ ���ݴϴ�.

				if (recvResult.first)//Ŭ���̾�Ʈ�� \r\n�� �Է��ߴٸ�
				{
					if (buffer.data()[0] == '/')
					{
						std::string command = std::string(buffer.data(), buffer.data() + recvResult.second);
						Outsourcer::Instance().ExecutingCommand(ClientInfosEachRoom[i][j], -1, command);
						//2�� ° ���ڴ� ä�ù� ��� ���� �� ���õ˴ϴ�.
						//2�� ° ���ڴ� TotalManager���� �� ����� ����� �������� ���� ���˴ϴ�.
						//����� �����ϴ� �Լ��� ������ ����� ����� �� �򰥸����Ͽ� �̷��� �����߽��ϴ�.
					}
					else
					{
						buffer.data()[recvResult.second] = '\0';

						std::string msgToSend = "\r\n" + ClientInfosEachRoom[i][j].Name + ": "
							+ std::string(buffer.data(), buffer.data() + recvResult.second) + "\r\n";
						//�޼����� ������ �ٸ� Ŭ���̾�Ʈ �̸�: �޽��� ���� �Դϴ�.
						//e.g.) HJO: Hello world
						for (int k = 0; k < ClientInfosEachRoom[i].size(); ++k)
						{
							CustomSend(ClientInfosEachRoom[i][k].ClientSock, msgToSend.c_str(), msgToSend.size(), 0, ClientInfosEachRoom[i][k]);
						}
						//���� �濡 ���� ��� Ŭ���̾�Ʈ(���� ����)���� ä�ó����� �����ݴϴ�.

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
	//Ŭ���̾�Ʈ ������ �ش� ���� �����ٸ� ���� �ο� ���� ���ҽ����ݴϴ�.
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
