#include "ChattingBuilding.h"
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

	while (1)
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
		//���⼭���� ���� ó���� �����Ѵ�.
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
		if (IsEmptyRoom[i]) //�� ���� �߰��ϸ� true�� ��ȯ�ϰ� ���ش�.
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

			if (FD_ISSET(clntSock, &WriteSet))  //�����͸� �������ϴ� ���
			{

			}
			else if(FD_ISSET(clntSock, &ReadSet)) // �����͸� �о�� ��� 
			{
				int recvSize = 0;
				recvSize = recv(clntSock, ClientInfosEachRoom[i][j].Buffer.data(), ClientInfo::MAX_BUFFER_SIZE, 0);
				ClientInfosEachRoom[i][j].Buffer.data()[recvSize] = '\0';
				std::cout << "Thread ID: " << std::this_thread::get_id() << ", Room Name: " << RoomNames[i] << ", Content: " << ClientInfosEachRoom[i][j].Buffer.data() << std::endl;
				for (int k = 0; k < ClientInfosEachRoom[i].size(); ++k)
				{
					send(ClientInfosEachRoom[i][k].ClntSock, ClientInfosEachRoom[i][j].Buffer.data(), recvSize + 1, 0);
				}
			}
		}
	}
}