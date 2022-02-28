#pragma once
#include <array>
#include <string>
#include <thread>
#include <vector>
#include <WinSock2.h>
struct ClientInfo
{
	static const unsigned int MAX_BUFFER_SIZE = 1024;
	static const unsigned int CRLR_SIZE = 2;
	ClientInfo():Name(""), IsSend(false), IsJoinRoom(false), 
		WillBeRemoved(false), SendingSize(0), RcvSize(0) {}
	
	virtual ~ClientInfo() = default;
	ClientInfo(SOCKET SocketDiscriptor) :ClntSock(SocketDiscriptor), Name(""), IsSend(false), IsJoinRoom(false),
		WillBeRemoved(false), SendingSize(0), RcvSize(0) {}
	SOCKET ClntSock;
	std::array<char, ClientInfo::MAX_BUFFER_SIZE> Buffer;
	std::string Name;
	bool IsSend; //이 플래그가 true일 때만 메시지를 클라이언트에게 보낸다.
	bool IsJoinRoom; //이 플래그가 false일 때만 메인쓰레드에서 세트에 소켓을 넣어준다.
	bool WillBeRemoved; //채팅방에 접속한 상태에서 클라이언트 연결이 끊기면 해당 플래그가 true가되고,
								//메인 쓰레드에서 해당 플래그가 true인지 검사해 실제로 제거한다.
	unsigned int SendingSize;
	unsigned int RcvSize;
}typedef ClientInfo;
//클라이언트의 소켓 디스크립터를 포함한 다양한 정보를 담고 있는 구조체입니다.

std::pair<bool, unsigned int> CustomRecv(SOCKET S, char* Buf, int Len, int Flags, ClientInfo& ClntInfo);
//텔넷 클라이언트를 위해 만든 특별한 recv함수이다.
// boolean 값은 개행문자를 만났는지 안 만났는지, uint는 클라이언트에게 받은 메시지의 size.
//개행문자가 들어올 때 까지 문자열을 쭉 받아서 모아놓다가
//개행문자를 만나면 true와 여태 모아두었던 사이즈를 반환한다.
