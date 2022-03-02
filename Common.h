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
	bool IsSend; //이 플래그가 true일 때만 메시지를 클라이언트에게 보냅니다.
	bool IsJoinRoom; //이 플래그가 false일 때만 메인쓰레드에서 세트에 소켓을 넣어줍니다.
	
	unsigned int RoomIndex;
	unsigned int SendingLeftPos; //전송을 시작할 시작 포인트입니다. (300byte를 보내야하는데 send가 100byte만 성공한 경우 이 값은 100이 됩니다.)
	unsigned int SendingRightPos; //최초 send 요구 시 입력 받은 크기입니다.
	//위에 두 값은 CustomSend 에서 사용됩니다.
	unsigned int RecvSize;

	static const int LOBBY_INDEX = 99999; // 사용자 세부 정보 검색 시 이 사용자가 로비에 있는지 판단하기 위해 사용됩니다.
}typedef ClientInfo;
//클라이언트의 소켓 디스크립터를 포함해 다양한 정보를 담고 있는 구조체입니다.

std::pair<bool, unsigned int> CustomRecv(SOCKET S, char* Buf, int Len, int Flags, ClientInfo& CommandRequestor);
//윈도우 텔넷 클라이언트를 위해 만든 특별한 recv함수입니다.
//boolean 값은 개행문자를 만났는지 안 만났는지, uint는 클라이언트에게 받은 메시지의 size를 뜻합니다.
//개행문자가 들어올 때 까지 문자열을 쭉 받아서 모아놓다가
//개행문자를 만나면 true와 여태 모아두었던 문자열의 사이즈를 반환합니다.

std::pair<bool, unsigned int> CustomSend(SOCKET S, const char* Buf, int Len, int Flags, ClientInfo& CommandRequestor);
//논블로킹 소켓 환경에서 send호출 시 데이터를 전부 못 보내는 경우가 생기기 때문에
//이를 처리해주기 위해 제작했습니다.
//bool값은 데이터를 전부 보냈는지, uint는 실제로 send호출 시 반환된 값입니다.

std::string GetCurrentSystemTime();
//현재 시각을 string으로 반환해주는 함수입니다.

std::vector<std::string> Tokenizing(const std::string& Str);