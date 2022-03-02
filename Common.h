#pragma once
#include <array>
#include <chrono>
#include <string>
#include <thread>
#include <vector>
#include <WinSock2.h>

const short DEFAULT_PORT = 8000;

struct ClientInfo
{
	ClientInfo();
	ClientInfo(SOCKET SocketDescriptor);
	ClientInfo(const ClientInfo& Other);
	virtual ~ClientInfo() = default;

	static const unsigned int MAX_BUFFER_SIZE = 1024;
	static const unsigned int CRLR_SIZE = 2;

	SOCKET ClientSock;
	std::array<char, ClientInfo::MAX_BUFFER_SIZE> Buffer;
	std::string Name;
	std::string ConnectionPoint;
	std::string EnteringTime;
	bool IsLogin;
	bool IsSend; //�� �÷��װ� true�� ���� �޽����� Ŭ���̾�Ʈ���� �����ϴ�.
	bool IsJoinRoom; //�� �÷��װ� false�� ���� ���ξ����忡�� ��Ʈ�� ������ �־��ݴϴ�.
	
	unsigned int RoomIndex;
	unsigned int SendingLeftPos; //������ ������ ���� ����Ʈ�Դϴ�. (300byte�� �������ϴµ� send�� 100byte�� ������ ��� �� ���� 100�� �˴ϴ�.)
	unsigned int SendingRightPos; //���� send �䱸 �� �Է� ���� ũ���Դϴ�.
	//���� �� ���� CustomSend ���� ���˴ϴ�.
	unsigned int RecvSize;

	static const int LOBBY_INDEX = 99999; // ����� ���� ���� �˻� �� �� ����ڰ� �κ� �ִ��� �Ǵ��ϱ� ���� ���˴ϴ�.
}typedef ClientInfo;
//Ŭ���̾�Ʈ�� ���� ��ũ���͸� ������ �پ��� ������ ��� �ִ� ����ü�Դϴ�.

std::pair<bool, unsigned int> CustomRecv(SOCKET S, char* Buf, int Len, int Flags, ClientInfo& CommandRequestor);
//������ �ڳ� Ŭ���̾�Ʈ�� ���� ���� Ư���� recv�Լ��Դϴ�.
//boolean ���� ���๮�ڸ� �������� �� ��������, uint�� Ŭ���̾�Ʈ���� ���� �޽����� size�� ���մϴ�.
//���๮�ڰ� ���� �� ���� ���ڿ��� �� �޾Ƽ� ��Ƴ��ٰ�
//���๮�ڸ� ������ true�� ���� ��Ƶξ��� ���ڿ��� ����� ��ȯ�մϴ�.

std::pair<bool, unsigned int> CustomSend(SOCKET S, const char* Buf, int Len, int Flags, ClientInfo& CommandRequestor);
//����ŷ ���� ȯ�濡�� sendȣ�� �� �����͸� ���� �� ������ ��찡 ����� ������
//�̸� ó�����ֱ� ���� �����߽��ϴ�.
//bool���� �����͸� ���� ���´���, uint�� ������ sendȣ�� �� ��ȯ�� ���Դϴ�.

std::string GetCurrentSystemTime();
//���� �ð��� string���� ��ȯ���ִ� �Լ��Դϴ�.

std::vector<std::string> Tokenizing(const std::string& Str);