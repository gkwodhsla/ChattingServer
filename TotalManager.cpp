#include "TotalManager.h"
#include <iostream>

TotalManager::TotalManager()
{
	InitServer();
	ClientInfos.reserve(100);
}

TotalManager::~TotalManager()
{
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
		FD_ZERO(&WriteSet);
		FD_ZERO(&ReadSet);

		FD_SET(ListenSocket, &ReadSet); //listen socket���� ��� Ŭ���̾�Ʈ�� �޾Ƶ鿩�� �ϱ� ������...

		for (int i = 0; i < ClientInfos.size(); ++i)
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
		select(0, &ReadSet, &WriteSet, nullptr, nullptr); // �ϳ��� �غ� �� ������ ���� �� ���� ����Ѵ�.

		ProcessingAfterSelect();
	}
}

void TotalManager::CreateRoom()
{
}

void TotalManager::DestroyRoom()
{
}

void TotalManager::PrintAllClientName()
{
}

void TotalManager::PrintAllRoomInfo()
{
}

void TotalManager::ProcessingAfterSelect()
{
	if (FD_ISSET(ListenSocket, &ReadSet)) //���� ���Ͽ��� ���� �غ� �ƴٸ� accept�� ���ο� Ŭ���̾�Ʈ�� �޴´�.
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
			sendSize = send(clntSocket, ClientInfos[i].Buffer.data(), ClientInfo::MAX_BUFFER_SIZE, 0);
			if (sendSize == SOCKET_ERROR)
			{
				RemoveClntSocket(i);
				continue;
			}
		}
		else if (FD_ISSET(clntSocket, &ReadSet)) //���� readSet�� Ȱ��ȭ �Ǿ��ִٸ� recv �Ѵ�.
		{
			unsigned int rcvSize = 0;
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
			std::cout << ClientInfos[i].Buffer.data() << std::endl;
			//���Ͽ� ������ ����ų� ������ ����� ���� �迭���� �ش� ������ ������ ������.
		}
	}
}

void TotalManager::RemoveClntSocket(int index)
{
	closesocket(ClientInfos[index].ClntSock);
	ClientInfos.erase(ClientInfos.begin() + index);
}