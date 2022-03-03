#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "ChattingBuilding.h"
#include "Outsourcer.h"
#include "TotalManager.h"
#include <iostream>

TotalManager::TotalManager():
ListenSocket(0)
{
	Buildings.emplace_back(new ChattingBuilding());
	SubThreads.emplace_back(&ChattingBuilding::ProcessingLogic, Buildings.back());
	FD_ZERO(&WriteSet);
	FD_ZERO(&ReadSet);
	InitServer();
}

TotalManager::~TotalManager()
{
	size_t buildingSize = Buildings.size();
	for (int i = 0; i < buildingSize; ++i)
	{
		Buildings[i]->StopChattingroomLogic();
		SubThreads[i].join();
		//ä�ù���� ó���� �����ϴ� ���� ��������� ���� ���� ��ŵ�ϴ�.
		//�����ϰ� �����ϴ� ���� ��ٸ��� ���� join���� ���� �� ���� ��ٷ��ݴϴ�.
		if (!Buildings[i])
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
	serverAddrInfo.sin_port = htons(DEFAULT_PORT);
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
	struct timeval timeout;

	timeout.tv_sec = 0;
	timeout.tv_usec = SOCKET_TIME_WAIT_US;
	while (1)
	{
		FD_ZERO(&WriteSet);
		FD_ZERO(&ReadSet);

		FD_SET(ListenSocket, &ReadSet); //listen socket���� ��� Ŭ���̾�Ʈ�� �޾Ƶ鿩�� �ϱ� ������...

		for (int i = 0; i < ClientInfos.size(); ++i)
		{
			if (!ClientInfos[i].IsJoinRoom) //ä�ù濡 �������� �ʾҴٸ� TotalManager���� ������ �������ݴϴ�.
			{
				if (ClientInfos[i].IsSend) //�������� �����Ͱ� ���Ҵٸ� ���� ���� �� �ְ� writeset�� ������ �־��ݴϴ�.
				{
					FD_SET(ClientInfos[i].ClientSock, &WriteSet);
				}
				//�� writeset�� ������ ������ ���� �� ���� ���� Ȱ��ȭ �˴ϴ�.
				//ChattingBuilding���� ���꾲����(ProcessingLogic)�� ���� ������ ��ȭ�� ������ ����
				//�����ϰ� ����˴ϴ�.
				else
				{
					FD_SET(ClientInfos[i].ClientSock, &ReadSet);
				}
				//�������� �����Ͱ� ������ �� recv�� ��� �̷�ϴ�.
				//(������ �� �����Ͱ� ���� Buffer�� �����ִµ� recv�� �ϸ� Buffer�� �����Ǳ� ������)
			}
		}
		select(0, &ReadSet, &WriteSet, nullptr, &timeout);

		ProcessingAfterSelect();
	}
}

void TotalManager::ProcessingAfterSelect()
{
	if (FD_ISSET(ListenSocket, &ReadSet)) //listen socket���� ���� �غ� �ƴٸ� accept�� ���ο� Ŭ���̾�Ʈ�� �޽��ϴ�.
	{
		AcceptingNewClient();
	}

	std::lock_guard<std::mutex> lockGuard(TotalManager::ClientInfoLock);
	//���꾲���忡�� ClientInfos �迭�� �����ϴ� ��찡 �ֱ⿡ lock�� Ȱ���߽��ϴ�.
	for (int i = 0; i < ClientInfos.size(); ++i)
	{
		SOCKET clientSocket = ClientInfos[i].ClientSock;
		if (FD_ISSET(clientSocket, &WriteSet))
		{
			unsigned int sendLen = ClientInfos[i].SendingRightPos - ClientInfos[i].SendingLeftPos;
			//��������LP��������RP RightPos - LeftPos�� �������� ���� �������� ũ�Ⱑ �˴ϴ�.
			std::pair<bool, int> sendResult = CustomSend(clientSocket, ClientInfos[i].Buffer.data(), sendLen, 0, ClientInfos[i]);
			
			if (sendResult.second == SOCKET_ERROR)
			{
				RemoveClientSocket(i);
				continue;
			}
		}
		else if (FD_ISSET(clientSocket, &ReadSet)) //���� readSet�� Ȱ��ȭ �Ǿ��ִٸ� recv �մϴ�.
		{			
			std::pair<bool, int>recvResult =
				CustomRecv(clientSocket, ClientInfos[i].Buffer.data(), ClientInfo::MAX_BUFFER_SIZE, 0, ClientInfos[i]);
			
			if (recvResult.second == SOCKET_ERROR)
			{
				std::cout << "disconnect" << std::endl;
				RemoveClientSocket(i);
			}
			//recv�� ������ ����ų� ������ ������ �����ߴٸ� ���� ���� �����迭����
			//�ش� ������ �����մϴ�.
			if (recvResult.first)
			{
				ClientInfos[i].Buffer[recvResult.second] = '\0';
				std::cout << ClientInfos[i].Buffer.data() << std::endl;
				std::string command{ ClientInfos[i].Buffer.data() };
				Outsourcer::Instance().ExecutingCommand(ClientInfos[i], i, command);
				//��ɾ ���������� �޾Ҵٸ� ���� ��ɾ CommandOutsourcer���� ���ָ� �ñ�ϴ�.
				//CommandOutsourcer�� ���� ��ɾ �Ľ��� �����ϰ� ó�����ݴϴ�.
				ZeroMemory(ClientInfos[i].Buffer.data(), ClientInfo::MAX_BUFFER_SIZE);
				ClientInfos[i].RecvSize = 0;
				//��ɾ� ó���� ������ ��ɾ ��Ҵ� ���۸� ����ݴϴ�.
			}
		}
	}
}

void TotalManager::RemoveClientSocket(int index)
{
	std::lock_guard<std::mutex> lockGuard(TotalManager::ClientInfoLock);
	closesocket(ClientInfos[index].ClientSock);
	ClientInfos.erase(ClientInfos.begin() + index);
}

void TotalManager::RemoveClientSocket(const ClientInfo& Info)
{
	std::lock_guard<std::mutex> lockGuard(TotalManager::ClientInfoLock);
	int index = -1;
	for (int i = 0; i < ClientInfos.size(); ++i)
	{
		if (Info.ClientSock == ClientInfos[i].ClientSock)
		{
			index = i;
			break;
		}
	}
	if (index != -1)
	{
		closesocket(ClientInfos[index].ClientSock);
		ClientInfos.erase(ClientInfos.begin() + index);
	}
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
	else //���� Ŭ���̾�Ʈ�� ���������� accept �Ǿ��ٸ� Ŭ���̾�Ʈ ������ �����ϴ� ���Ϳ� �־��ݴϴ�.
	{
		char* clientIpv4 = inet_ntoa(clientAddr.sin_addr);
		u_short clientPort = ntohs(clientAddr.sin_port);

		std::cout << "New client connected: " << clientIpv4 << ", " << clientPort << std::endl;
		ClientInfos.emplace_back(clientSocket);
		ClientInfos.back().ConnectionPoint += std::string(clientIpv4) + ":" + std::to_string(ntohs(clientPort));
		//������ ������ ���߿� ���� ���� �˻� �� ���˴ϴ�.
		std::string welcomeMsg{ "Welcome to chatting server!\r\nPlease login using Login [Username] Command\r\n" };

		std::pair<bool, int> sndResult = CustomSend(clientSocket, welcomeMsg.c_str(), welcomeMsg.size(), 0, ClientInfos.back());
		if (sndResult.second == SOCKET_ERROR)
		{
			RemoveClientSocket(ClientInfos.size() - 1);
		}
	}
}
