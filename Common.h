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
	bool WillBeRemoved = false; //채팅방에 접속한 상태에서 클라이언트 연결이 끊기면 해당 플래그가 true가되고,
								//메인 쓰레드에서 해당 플래그가 true인지 검사해 실제로 제거한다.
	int SendingSize = 0;
	int RcvSize = 0;

public:
	inline static int MAX_BUFFER_SIZE = 1024;

}typedef ClientInfo;

std::pair<bool, int> CustomRecv(SOCKET S, char* Buf, int Len, int Flags, ClientInfo& ClntInfo);
//텔넷 클라이언트를 위해 만든 특별한 recv함수이다.
//개행문자가 들어올 때 까지 문자열을 쭉 받아서 모아놓다가
//개행문자를 만나면 true와 여태 모아두었던 사이즈를 반환한다.