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
		//채팅방들의 처리를 관할하던 쓰레드들을 전부 중지 시킨다.
		//안전하게 종료하는 것을 기다리기 위해 join으로 기다려준다.
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
		//연결이 끊겨 소켓 정보가 관리 배열에서 지워져야 한다고 마킹이 되어있는 정보들을 모두 지운다.
		FD_ZERO(&WriteSet);
		FD_ZERO(&ReadSet);

		FD_SET(ListenSocket, &ReadSet); //listen socket에서 계속 클라이언트를 받아들여야 하기 때문에...

		for (int i = 0; i < ClientInfos.size(); ++i)
		{
			if (!ClientInfos[i].IsJoinRoom) //방에 참가하지 않고, 로비에 머물고 있다면 TotalManager에서 소켓을 관리해준다.
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
		select(0, &ReadSet, &WriteSet, nullptr, nullptr); // 하나라도 준비가 된 소켓이 있을 때 까지 대기한다.

		ProcessingAfterSelect();
	}
}

void TotalManager::ProcessingAfterSelect()
{
	if (FD_ISSET(ListenSocket, &ReadSet)) //listen socket에서 받을 준비가 됐다면 accept로 새로운 클라이언트를 받는다.
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
		else //만약 클라이언트가 성공적으로 accept 되었다면 클라이언트 정보를 관리하는 벡터에 넣어준다.
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
		if (FD_ISSET(clntSocket, &WriteSet)) //만약 writeSet가 활성화 되어있다면 send 한다.
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
		else if (FD_ISSET(clntSocket, &ReadSet)) //만약 readSet가 활성화 되어있다면 recv 한다.
		{
			unsigned int rcvSize = 0;
			ZeroMemory(ClientInfos[i].Buffer.data(), ClientInfo::MAX_BUFFER_SIZE);
			//클라이언트에게 명령어를 받기 전에 버퍼를 비워준다.
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