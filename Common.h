#pragma once
#include <array>
#include <chrono>
#include <string>
#include <thread>
#include <vector>
#include <WinSock2.h>
struct ClientInfo
{
	static const unsigned int MAX_BUFFER_SIZE = 1024;
	static const unsigned int CRLR_SIZE = 2;
	ClientInfo():Name("TempName"), EnteringTime(""), IsSend(false), IsJoinRoom(false),
		WillBeRemoved(false), SendingLeftPos(0), RcvSize(0), SendingRightPos(0) {}
	
	virtual ~ClientInfo() = default;
	ClientInfo(SOCKET SocketDiscriptor) :ClntSock(SocketDiscriptor), Name("TempName"), EnteringTime(""), IsSend(false), IsJoinRoom(false),
		WillBeRemoved(false), SendingLeftPos(0), RcvSize(0), SendingRightPos(0) {}
	SOCKET ClntSock;
	std::array<char, ClientInfo::MAX_BUFFER_SIZE> Buffer;
	std::string Name;
	std::string EnteringTime;
	bool IsSend; //�� �÷��װ� true�� ���� �޽����� Ŭ���̾�Ʈ���� ������.
	bool IsJoinRoom; //�� �÷��װ� false�� ���� ���ξ����忡�� ��Ʈ�� ������ �־��ش�.
	bool WillBeRemoved; //ä�ù濡 ������ ���¿��� Ŭ���̾�Ʈ ������ ����� �ش� �÷��װ� true���ǰ�,
								//���� �����忡�� �ش� �÷��װ� true���� �˻��� ������ �����Ѵ�.
	unsigned int SendingLeftPos; //���� �����ϰ� ���� ũ�� (300byte�����ؾ��ϴµ� send�Լ��� 200byte�� �����ϸ� �� ���� 100byte�� �ȴ�.)
	unsigned int SendingRightPos;
	unsigned int RcvSize;
}typedef ClientInfo;
//Ŭ���̾�Ʈ�� ���� ��ũ���͸� ������ �پ��� ������ ��� �ִ� ����ü�Դϴ�.

std::pair<bool, unsigned int> CustomRecv(SOCKET S, char* Buf, int Len, int Flags, ClientInfo& ClntInfo);
//�ڳ� Ŭ���̾�Ʈ�� ���� ���� Ư���� recv�Լ��̴�.
// boolean ���� ���๮�ڸ� �������� �� ��������, uint�� Ŭ���̾�Ʈ���� ���� �޽����� size.
//���๮�ڰ� ���� �� ���� ���ڿ��� �� �޾Ƽ� ��Ƴ��ٰ�
//���๮�ڸ� ������ true�� ���� ��Ƶξ��� ����� ��ȯ�Ѵ�.

std::pair<bool, unsigned int> CustomSend(SOCKET S, const char* Buf, int Len, int Flags, ClientInfo& ClntInfo);
//����ŷ ���� ȯ�濡�� sendȣ�� �� �����͸� ���� �� ������ ��찡 ����� ������
//�̸� ó�����ֱ� ���� �����߽��ϴ�.
//bool���� �����͸� ���� ���´���, �ڴ� ������ sendȣ�� �� ��ȯ�� ���Դϴ�.

std::string GetCurrentSystemTime();