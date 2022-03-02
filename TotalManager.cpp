#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "ChattingBuilding.h"
#include "Outsourcer.h"
#include "TotalManager.h"
#include <iostream>

TotalManager::TotalManager():ListenSocket(0), CommandOutsourcer(nullptr)
{
	Buildings.emplace_back(new ChattingBuilding());
	SubThreads.emplace_back(&ChattingBuilding::ProcessingLogic, Buildings.back());
	CommandOutsourcer = new Outsourcer();
	FD_ZERO(&WriteSet);
	FD_ZERO(&ReadSet);
	InitServer();
}

TotalManager::~TotalManager()
{
	delete CommandOutsourcer;
	CommandOutsourcer = nullptr;

	int buildingSize = Buildings.size();
	for (int i = 0; i < buildingSize; ++i)
	{
		Buildings[i]->StopChattingroomLogic();
		SubThreads[i].join();
		//채팅방들의 처리를 관할하던 서브 쓰레드들을 전부 중지 시킵니다.
		//안전하게 종료하는 것을 기다리기 위해 join으로 끝날 때 까지 기다려줍니다.
		if (Buildings[i] != nullptr)
		{
			delete Buildings[i];
			Buildings[i] = nullptr;
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

void TotalManager::MarkingForRemoveClientSocket(SOCKET SocketDescriptor)
{
	for (int i = 0; i < ClientInfos.size(); ++i)
	{
		if (SocketDescriptor == ClientInfos[i].ClientSock)
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
				RemoveClientSocket(i);
				i = -1;
			}
		}
		//채팅방에서 연결이 끊겨 소켓 정보가 관리 배열(ClientInfos)에서 지워져야 한다고 마킹이 되어있다면
		//<마킹되어 있는 소켓 정보>들을 <전부 제거>합니다.

		FD_ZERO(&WriteSet);
		FD_ZERO(&ReadSet);

		FD_SET(ListenSocket, &ReadSet); //listen socket에서 계속 클라이언트를 받아들여야 하기 때문에...

		for (int i = 0; i < ClientInfos.size(); ++i)
		{
			if (!ClientInfos[i].IsJoinRoom) //채팅방에 참가하지 않았다면 TotalManager에서 소켓을 관리해줍니다.
			{
				if (ClientInfos[i].IsSend) //전송중인 데이터가 남았다면 마저 보낼 수 있게 writeset에 소켓을 넣어줍니다.
				{
					FD_SET(ClientInfos[i].ClientSock, &WriteSet);
				}
				//★ writeset는 데이터 전송을 전부 못 했을 때만 활성화 됩니다.
				//ChattingBuilding에서 서브쓰레드(ProcessingLogic)를 통해 유저간 대화를 전송할 때도
				//동일하게 적용됩니다.
				else
				{
					FD_SET(ClientInfos[i].ClientSock, &ReadSet);
				}
				//전송중인 데이터가 남았을 땐 recv를 잠시 미룹니다.
				//(보내야 할 데이터가 아직 Buffer에 남아있는데 recv를 하면 Buffer가 오염되기 때문에)
			}
		}
		select(0, &ReadSet, &WriteSet, nullptr, nullptr); // 하나라도 준비가 된 소켓이 있을 때 까지 대기합니다.

		ProcessingAfterSelect();
	}
}

void TotalManager::ProcessingAfterSelect()
{
	if (FD_ISSET(ListenSocket, &ReadSet)) //listen socket에서 받을 준비가 됐다면 accept로 새로운 클라이언트를 받습니다.
	{
		AcceptingNewClient();
	}

	for (int i = 0; i < ClientInfos.size(); ++i)
	{
		SOCKET clientSocket = ClientInfos[i].ClientSock;
		if (FD_ISSET(clientSocket, &WriteSet))
		{
			unsigned int sendLen = ClientInfos[i].SendingRightPos - ClientInfos[i].SendingLeftPos;
			//ㅁㅁㅁ■LPㅁㅁㅁ■RP RightPos - LeftPos가 보내야할 남은 데이터의 크기가 됩니다.
			std::pair<bool, int> sendResult = CustomSend(clientSocket, ClientInfos[i].Buffer.data(), sendLen, 0, ClientInfos[i]);
			
			if (sendResult.second == SOCKET_ERROR)
			{
				RemoveClientSocket(i);
				continue;
			}
		}
		else if (FD_ISSET(clientSocket, &ReadSet)) //만약 readSet가 활성화 되어있다면 recv 합니다.
		{			
			std::pair<bool, int>recvResult =
				CustomRecv(clientSocket, ClientInfos[i].Buffer.data(), ClientInfo::MAX_BUFFER_SIZE, 0, ClientInfos[i]);
			
			if (recvResult.second == SOCKET_ERROR)
			{
				std::cout << "disconnect" << std::endl;
				RemoveClientSocket(i);
				continue;
			}
			//recv에 문제가 생기거나 상대방이 연결을 종료했다면 소켓 정보 관리배열에서
			//해당 소켓을 제거합니다.
			if (recvResult.first)
			{
				ClientInfos[i].Buffer[recvResult.second] = '\0';
				std::cout << ClientInfos[i].Buffer.data() << std::endl;
				std::string command{ ClientInfos[i].Buffer.data() };
				CommandOutsourcer->ExecutingCommand(ClientInfos[i], i, command);
				//명령어를 성공적으로 받았다면 받은 명령어를 CommandOutsourcer에게 외주를 맡깁니다.
				//CommandOutsourcer는 받은 명령어를 파싱해 적절하게 처리해줍니다.
				ZeroMemory(ClientInfos[i].Buffer.data(), ClientInfo::MAX_BUFFER_SIZE);
				ClientInfos[i].RecvSize = 0;
				//명령어 처리가 끝나면 명령어를 담았던 버퍼를 비워줍니다.
			}
		}
	}
}

void TotalManager::RemoveClientSocket(int index)
{
	closesocket(ClientInfos[index].ClientSock);
	ClientInfos.erase(ClientInfos.begin() + index);
}

void TotalManager::AcceptingNewClient()
{
	SOCKET clientSocket;
	SOCKADDR_IN clientAddr;
	ZeroMemory(&clientAddr, sizeof(clientAddr));
	int clientAddrSize = sizeof(clientAddr);
	clientSocket = accept(ListenSocket, (SOCKADDR*)&clientAddr, &clientAddrSize);
	if (clientSocket == INVALID_SOCKET)
	{
		std::cout << "accept failed..." << std::endl;
	}
	else //만약 클라이언트가 성공적으로 accept 되었다면 클라이언트 정보를 관리하는 벡터에 넣어줍니다.
	{
		char* clientIpv4 = inet_ntoa(clientAddr.sin_addr);
		u_short clientPort = ntohs(clientAddr.sin_port);

		std::cout << "New client connected: " << clientIpv4 << ", " << clientPort << std::endl;
		ClientInfos.emplace_back(clientSocket);
		ClientInfos.back().ConnectionPoint += std::string(clientIpv4) + ":" + std::to_string(ntohs(clientPort));
		//접속지 정보는 나중에 유저 세부 검색 시 사용됩니다.
		std::string welcomeMsg{ "Welcome to chatting server!\r\nPlease login using Login [Username] Command\r\n" };

		std::pair<bool, int> sndResult = CustomSend(clientSocket, welcomeMsg.c_str(), welcomeMsg.size(), 0, ClientInfos.back());
		if (sndResult.second == SOCKET_ERROR)
		{
			RemoveClientSocket(ClientInfos.size() - 1);
		}
	}
}
