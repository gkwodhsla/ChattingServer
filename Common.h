#pragma once
#include <array>
#include <string>
#include <thread>
#include <vector>
#include <WinSock2.h>
struct ClientInfo
{
	ClientInfo() = delete;
	virtual ~ClientInfo() = default;
	ClientInfo(SOCKET SocketDiscriptor) :ClntSock(SocketDiscriptor) {}
	SOCKET ClntSock;
	std::array<char, 1024> Buffer;
	std::string Name;
	bool IsSend = false; //�� �÷��װ� true�� ���� �޽����� Ŭ���̾�Ʈ���� ������.
	bool IsJoinRoom = false; //�� �÷��װ� false�� ���� ���ξ����忡�� ��Ʈ�� ������ �־��ش�.
	bool WillBeRemoved = false; //ä�ù濡 ������ ���¿��� Ŭ���̾�Ʈ ������ ����� �ش� �÷��װ� true���ǰ�,
								//���� �����忡�� �ش� �÷��װ� true���� �˻��� ������ �����Ѵ�.
	int SendingSize = 0;
	int RcvSize = 0;

public:
	inline static int MAX_BUFFER_SIZE = 1024;

}typedef ClientInfo;

std::pair<bool, int> CustomRecv(SOCKET S, char* Buf, int Len, int Flags, ClientInfo& ClntInfo);
//�ڳ� Ŭ���̾�Ʈ�� ���� ���� Ư���� recv�Լ��̴�.
//���๮�ڰ� ���� �� ���� ���ڿ��� �� �޾Ƽ� ��Ƴ��ٰ�
//���๮�ڸ� ������ true�� ���� ��Ƶξ��� ����� ��ȯ�Ѵ�.