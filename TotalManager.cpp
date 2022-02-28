#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "ChattingBuilding.h"
#include "Outsourcer.h"
#include "TotalManager.h"
#include <iostream>

TotalManager::TotalManager()
{
	Buildings.emplace_back(new ChattingBuilding());
	SubThreads.emplace_back(&ChattingBuilding::ProcessingLogic, Buildings.back());
	CommandOutsourcer = new Outsourcer();
	InitServer();
}

TotalManager::~TotalManager()
{
	delete CommandOutsourcer;
	CommandOutsourcer = nullptr;

	int buildingSize = Buildings.size();
	for (int i = 0; i < buildingSize; ++i)
	{
		Buildings[0]->StopChattingroomLogic();
		SubThreads[0].join();
		//ä�ù���� ó���� �����ϴ� ��������� ���� ���� ��Ų��.
		//�����ϰ� �����ϴ� ���� ��ٸ��� ���� join���� ��ٷ��ش�.
		if (Buildings[0] != nullptr)
		{
			delete Buildings[0];
			Buildings[0] = nullptr;
		}
	}

	closesocket(ListenSocket);
	WSACleanup();
}

void TotalManager::InitServer()
{
	WSAData wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);
	ListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (ListenSocket == INVALID_SOCKET)
	{
		std::cout << "Create listen socket has been failed..." << std::endl;
	}
	SOCKADDR_IN serverAddrInfo;
	ZeroMemory(&serverAddrInfo, sizeof(serverAddrInfo));
	serverAddrInfo.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddrInfo.sin_family = AF_INET;
	serverAddrInfo.sin_port = htons(8000);
	if (bind(ListenSocket, (SOCKADDR*)&serverAddrInfo, sizeof(serverAddrInfo)) == SOCKET_ERROR)
	{
		std::cout << "bind failed..." << std::endl;
	}
	if (listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR)
	{
		std::cout << "listen failed..." << std::endl;
	}
	unsigned long on = 1;
	ioctlsocket(ListenSocket, FIONBIO, &on);
}

void TotalManager::MainLogic()
{
	while (1)
	{
		for (int i = 0; i < ClientInfos.size(); ++i)
		{
			if (ClientInfos[i].WillBeRemoved)
			{
				RemoveClntSocket(i);
				i = -1;
			}
		}
		//������ ���� ���� ������ ���� �迭���� �������� �Ѵٰ� ��ŷ�� �Ǿ��ִ� �������� ��� �����.
		FD_ZERO(&WriteSet);
		FD_ZERO(&ReadSet);

		FD_SET(ListenSocket, &ReadSet); //listen socket���� ��� Ŭ���̾�Ʈ�� �޾Ƶ鿩�� �ϱ� ������...

		for (int i = 0; i < ClientInfos.size(); ++i)
		{
			if (!ClientInfos[i].IsJoinRoom) //�濡 �������� �ʰ�, �κ� �ӹ��� �ִٸ� TotalManager���� ������ �������ش�.
			{
				if (ClientInfos[i].IsSend)
				{
					FD_SET(ClientInfos[i].ClntSock, &WriteSet);
				}
				else
				{
					FD_SET(ClientInfos[i].ClntSock, &ReadSet);
				}
			}
		}
		select(0, &ReadSet, &WriteSet, nullptr, nullptr); // �ϳ��� �غ� �� ������ ���� �� ���� ����Ѵ�.

		ProcessingAfterSelect();
	}
}

void TotalManager::ProcessingAfterSelect()
{
	if (FD_ISSET(ListenSocket, &ReadSet)) //listen socket���� ���� �غ� �ƴٸ� accept�� ���ο� Ŭ���̾�Ʈ�� �޴´�.
	{
		SOCKET clntSocket;
		SOCKADDR_IN clntAddr;
		ZeroMemory(&clntAddr, sizeof(clntAddr));
		int clntAddrSize = sizeof(clntAddr);
		clntSocket = accept(ListenSocket, (SOCKADDR*)&clntAddr, &clntAddrSize);
		if (clntSocket == INVALID_SOCKET)
		{
			std::cout << "accept failed..." << std::endl;
		}
		else //���� Ŭ���̾�Ʈ�� ���������� accept �Ǿ��ٸ� Ŭ���̾�Ʈ ������ �����ϴ� ���Ϳ� �־��ش�.
		{
			std::string welcomeMsg{ "Welcome to chatting server!\r\nYou can login using Login [Username] Command\r\n" };
			send(clntSocket, welcomeMsg.c_str(), welcomeMsg.size(), 0);

			std::cout << "New client connected: " << inet_ntoa(clntAddr.sin_addr) << std::endl;
			ClientInfos.emplace_back(clntSocket);
		}
	}

	for (int i = 0; i < ClientInfos.size(); ++i)
	{
		SOCKET clntSocket = ClientInfos[i].ClntSock;
		if (FD_ISSET(clntSocket, &WriteSet)) //���� writeSet�� Ȱ��ȭ �Ǿ��ִٸ� send �Ѵ�.
		{
			unsigned int sendSize = 0;
			sendSize = send(clntSocket, ClientInfos[i].Buffer.data(), ClientInfos[i].SendingSize, 0);
			ClientInfos[i].IsSend = false;
			if (sendSize == SOCKET_ERROR)
			{
				RemoveClntSocket(i);
				continue;
			}
		}
		else if (FD_ISSET(clntSocket, &ReadSet)) //���� readSet�� Ȱ��ȭ �Ǿ��ִٸ� recv �Ѵ�.
		{
			unsigned int rcvSize = 0;
			ZeroMemory(ClientInfos[i].Buffer.data(), ClientInfo::MAX_BUFFER_SIZE);
			//Ŭ���̾�Ʈ���� ��ɾ �ޱ� ���� ���۸� ����ش�.
			rcvSize = recv(clntSocket, ClientInfos[i].Buffer.data(), ClientInfo::MAX_BUFFER_SIZE, 0);
			if (rcvSize == SOCKET_ERROR)
			{
				std::cout << "recv error" << std::endl;
				RemoveClntSocket(i);
				continue;
			}
			else if (rcvSize == 0)
			{
				std::cout << "disconnect" << std::endl;
				RemoveClntSocket(i);
				continue;
			}
			ClientInfos[i].Buffer[rcvSize] = '\0';
			std::cout << ClientInfos[i].Buffer.data() << std::endl;
			std::string temp{ ClientInfos[i].Buffer.data() };
			CommandOutsourcer->ExecutingCommand(ClientInfos[i], i, temp);
		}
	}
}

void TotalManager::RemoveClntSocket(int index)
{
	closesocket(ClientInfos[index].ClntSock);
	ClientInfos.erase(ClientInfos.begin() + index);
}

void TotalManager::MarkingForRemoveClntSocket(SOCKET SocketDiscriptor)
{
	for (int i = 0; i < ClientInfos.size(); ++i)
	{
		if (SocketDiscriptor == ClientInfos[i].ClntSock)
		{
			ClientInfos[i].WillBeRemoved = true;
			break;
		}
	}
}