#include "ChattingBuilding.h"
#include <iostream>

ChattingBuilding::ChattingBuilding()
{
}

void ChattingBuilding::ProcessingLogic()
{
	while (1)
	{
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
		select(0, &ReadSet, &WriteSet, nullptr, nullptr);
		//���⼭���� ���� ó���� �����Ѵ�.
		ProcessingAfterSelect();
	}
}

void ChattingBuilding::PrintAllRoomName()
{
}

void ChattingBuilding::OccupyingRoom(const std::string& RoomName)
{
	for (int i = 0; i < MAX_ROOM_NUM; ++i)
	{
		if (IsEmptyRoom[i])
		{
			IsEmptyRoom[i] = false;
			RoomNames[i] = RoomName;
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

void ChattingBuilding::EnteringRoom(const int RoomIndex, const ClientInfo& Client)
{
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
				std::cout << "Thread ID: " << std::this_thread::get_id() << ", Room Name: " << RoomNames[i] << ", Content: " << ClientInfosEachRoom[i][j].Buffer.data() << std::endl;
			}
		}
	}
}