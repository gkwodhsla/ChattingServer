#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "ChattingBuilding.h"
#include "Outsourcer.h"
#include "TotalManager.h"
#include <iostream>

TotalManager::TotalManager():ListenSocket(0), CommandOutsourcer(nullptr)
{
	FD_ZERO(&WriteSet);
	FD_ZERO(&ReadSet);
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
		//채팅방에서 연결이 끊겨 소켓 정보가 관리 배열(ClientInfos)에서 지워져야 한다고 마킹이 되어있다면
		//마킹되어 있는 소켓 정보들을 전부 제거한다.

		FD_ZERO(&WriteSet);
		FD_ZERO(&ReadSet);

		FD_SET(ListenSocket, &ReadSet); //listen socket에서 계속 클라이언트를 받아들여야 하기 때문에...

		for (int i = 0; i < ClientInfos.size(); ++i)
		{
			if (!ClientInfos[i].IsJoinRoom) //채팅방에 참가하지 않았다면 TotalManager에서 소켓을 관리해준다.
			{
				if (ClientInfos[i].IsSend) //전송중인 데이터가 남았다면 마저 보내게 해준다.
				{
					FD_SET(ClientInfos[i].ClntSock, &WriteSet);
				}
				else //전송중인 데이터가 남았을 땐 recv를 잠시 미룬다.
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
			char* clntIpv4 = inet_ntoa(clntAddr.sin_addr);
			u_short clntPort = ntohs(clntAddr.sin_port);

			std::cout << "New client connected: " << clntIpv4 <<", " << clntPort << std::endl;
			ClientInfos.emplace_back(clntSocket, "");
			ClientInfos.back().ConnectionPoint += std::string(clntIpv4) + ":" + std::to_string(ntohs(clntPort));
			std::string welcomeMsg{ "Welcome to chatting server!\r\nPlease login using Login [Username] Command\r\n" };
			
			std::pair<bool, int> sndResult = CustomSend(clntSocket, welcomeMsg.c_str(), welcomeMsg.size(), 0, ClientInfos.back());
			if (sndResult.second == SOCKET_ERROR)
			{
				RemoveClntSocket(ClientInfos.size() - 1);
			}
			//로그인 기능은 추후 구현 예정.

		}
	}

	for (int i = 0; i < ClientInfos.size(); ++i)
	{
		SOCKET clntSocket = ClientInfos[i].ClntSock;
		if (FD_ISSET(clntSocket, &WriteSet))
		{
			unsigned int sendLen = ClientInfos[i].SendingRightPos - ClientInfos[i].SendingLeftPos;
			std::pair<bool, int> sendResult = CustomSend(clntSocket, ClientInfos[i].Buffer.data(), sendLen, 0, ClientInfos[i]);
			
			if (sendResult.second == SOCKET_ERROR)
			{
				RemoveClntSocket(i);
				continue;
			}
		}
		//만약 writeSet가 활성화 되어있다면 send 한다. (writeSet는 한 번에 전송을 실패했을 때 활성화된다!)
		else if (FD_ISSET(clntSocket, &ReadSet)) //만약 readSet가 활성화 되어있다면 recv 한다.
		{			
			std::pair<bool, int>rcvResult =
				CustomRecv(clntSocket, ClientInfos[i].Buffer.data(), ClientInfo::MAX_BUFFER_SIZE, 0, ClientInfos[i]);
			
			if (rcvResult.second == SOCKET_ERROR)
			{
				std::cout << "disconnect" << std::endl;
				RemoveClntSocket(i);
				continue;
			}
			//recv에 문제가 생기거나 상대방이 연결을 종료했다면 소켓 정보 관리배열에서
			//해당 소켓을 제거한다.
			if (rcvResult.first)
			{
				ClientInfos[i].Buffer[rcvResult.second] = '\0';
				std::cout << ClientInfos[i].Buffer.data() << std::endl;
				std::string command{ ClientInfos[i].Buffer.data() };
				CommandOutsourcer->ExecutingCommand(ClientInfos[i], i, command);
				//명령어를 성공적으로 받았다면 받은 명령어를 CommandOutsourcer에게 외주를 맡긴다.
				//CommandOutsourcer는 받은 명령어를 파싱해 적절하게 처리해준다.
				ZeroMemory(ClientInfos[i].Buffer.data(), ClientInfo::MAX_BUFFER_SIZE);
				ClientInfos[i].RcvSize = 0;
			}
		}
	}
}

void TotalManager::RemoveClntSocket(int index)
{
	closesocket(ClientInfos[index].ClntSock);
	ClientInfos.erase(ClientInfos.begin() + index);
}