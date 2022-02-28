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
	bool IsSend = false; //이 플래그가 true일 때만 메시지를 클라이언트에게 보낸다.
	bool IsJoinRoom = false; //이 플래그가 false일 때만 메인쓰레드에서 세트에 소켓을 넣어준다.
	int SendingSize = 0;

public:
	inline static int MAX_BUFFER_SIZE = 1024;

}typedef ClientInfo;