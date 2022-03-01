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
		CurParticipantInRoom[i] = 0;
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
				if (ClientInfosEachRoom[i][j].IsSend)//�������� �����Ͱ� ���Ҵٸ� ���� ������ ���ش�.
				{
					FD_SET(ClientInfosEachRoom[i][j].ClntSock, &WriteSet);
				}
				else //�������� �����Ͱ� ������ �� recv�� ��� �̷��.
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
		if (IsEmptyRoom[i]) //�� ���� �߰��ϸ� true�� ��ȯ�ϰ� ���ش�.
		{
			return true;
		}
	}
	return false;
}

void ChattingBuilding::EnteringRoom(const int RoomIndex, ClientInfo& Client)
{
	std::string roomEnterMsg = "";
	if (IsEmptyRoom[RoomIndex]) //�������� ���� �濡 �����Ϸ��� ��� Ŭ���̾�Ʈ���� �ȵȴٰ� �˷��ش�.
	{
		roomEnterMsg = "You can't entering the room (room is not exist)\r\n";
		CustomSend(Client.ClntSock, roomEnterMsg.c_str(), roomEnterMsg.size(), 0, Client);
		return;
	}

	if (CurParticipantInRoom[RoomIndex] + 1 <= MaximumParticipants[RoomIndex]) //���������� ����
	{
		Client.IsJoinRoom = true;
		lock.lock();
		ClientInfosEachRoom[RoomIndex].emplace_back(Client.ClntSock);
		lock.unlock();
		//���� ��ũ���͸� ������ ���Ӱ� ������ ����� �־��ش�.
		//TotalManager ���� �迭���� �̵��� ��Ű�� �Ѱ����� ������ ����� �� ���Ƽ�
		//�̷��� �����߽��ϴ�.
		roomEnterMsg = "Room name: " + RoomNames[RoomIndex] + "(" + std::to_string(++CurParticipantInRoom[RoomIndex]) +
			"/" + std::to_string(MaximumParticipants[RoomIndex]) + ")";
		CustomSend(Client.ClntSock, roomEnterMsg.c_str(), roomEnterMsg.size(), 0, Client);
	}
	else //�� �ִ� �ο��� �Ѿ ��� �������� �� �Ѵٰ� �˷��ش�.
	{
		roomEnterMsg = "You can't enter this room (room fulled already)\r\n";
		CustomSend(Client.ClntSock, roomEnterMsg.c_str(), roomEnterMsg.size(), 0, Client);
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
			SOCKET clntSock = ClientInfosEachRoom[i][j].ClntSock;
			std::array<char, 1024>& buffer = ClientInfosEachRoom[i][j].Buffer;

			if (FD_ISSET(clntSock, &WriteSet))  //�����͸� �������ϴ� ���
			{
				std::pair<bool, int> sendResult =
					CustomSend(clntSock, buffer.data(), ClientInfosEachRoom[i][j].SendingRightPos - ClientInfosEachRoom[i][j].SendingLeftPos,
						0, ClientInfosEachRoom[i][j]);

				if (sendResult.second == SOCKET_ERROR)
				{
					std::cout << "disconnect" << std::endl;
					RemoveClntSocket(i, j);
					continue;
				}
			}
			else if (FD_ISSET(clntSock, &ReadSet)) // �����͸� �о���ϴ� ���
			{
				std::pair<bool, int>rcvResult =
					CustomRecv(clntSock, buffer.data(), ClientInfo::MAX_BUFFER_SIZE, 0, ClientInfosEachRoom[i][j]);

				if (rcvResult.second == SOCKET_ERROR)
				{
					std::cout << "disconnect" << std::endl;
					RemoveClntSocket(i, j);
					continue;
				}
				//���� recv�� ������ ����ų� Ŭ���̾�Ʈ�� ������ �����
				//���� �迭���� �ش� Ŭ���̾�Ʈ�� ������ ���ش�.

				if (rcvResult.first)
				{
					buffer.data()[rcvResult.second] = '\0';

					std::string msgToSend = std::string("\r\nOther Client Name: ") + std::string(buffer.data(), buffer.data() + rcvResult.second) + "\r\n";
					//�޼����� ������ �ٸ� Ŭ���̾�Ʈ �̸�: �޽��� ���� �̴�.
					//e.g.) HJO: Hello world
					//(���� �α��� ��ɱ��� ������ �Ǹ� Other Client Name�� ���� ������ �̸��� �־��� �����Դϴ�.)
					for (int k = 0; k < ClientInfosEachRoom[i].size(); ++k)
					{
						CustomSend(ClientInfosEachRoom[i][k].ClntSock, msgToSend.c_str(), msgToSend.size(), 0, ClientInfosEachRoom[i][k]);
					}
					//���� �濡 ���� ��� Ŭ���̾�Ʈ(���� ����)���� ä�ó����� �����ش�.

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
	//���� �����忡�� ���� �����忡�� �����ϴ� ���� ������ ���� �����ϸ� ������ ���� �� ���� �� ����
	//��ŷ�� �س��� ���� �����ϴ� ���� ���� �����尡 �ϰԲ� �����߽��ϴ�.
	--CurParticipantInRoom[RoomNumber];
	//Ŭ���̾�Ʈ ������ �ش� ���� �����ٸ� ���� �ο� ���� ���ҽ����ݴϴ�.
	ClientInfosEachRoom[RoomNumber].erase(ClientInfosEachRoom[RoomNumber].begin() + Index);
}