#define _CRT_SECURE_NO_WARNINGS
#include "Common.h"
#include <sstream>

ClientInfo::ClientInfo():ClientSock(0), Name("TempName"), EnteringTime(""), ConnectionPoint(""), IsSend(false), IsJoinRoom(false), IsLogin(false),
RoomIndex(LOBBY_INDEX), SendingLeftPos(0), RecvSize(0), SendingRightPos(0)
{
	ZeroMemory(&Buffer, ClientInfo::MAX_BUFFER_SIZE);
}

ClientInfo::ClientInfo(SOCKET SocketDescriptor):ClientSock(SocketDescriptor), Name(""), EnteringTime(""), ConnectionPoint(""), IsSend(false), IsJoinRoom(false), IsLogin(false), RoomIndex(LOBBY_INDEX), SendingLeftPos(0), RecvSize(0), SendingRightPos(0)
{
	ZeroMemory(&Buffer, ClientInfo::MAX_BUFFER_SIZE);
}

ClientInfo::ClientInfo(const ClientInfo& Other)
{
	ClientSock = Other.ClientSock;
	Name = Other.Name;
	ConnectionPoint = Other.ConnectionPoint;
	EnteringTime = Other.EnteringTime;
	IsLogin = Other.IsLogin;
	IsSend = Other.IsSend;
	IsJoinRoom= Other.IsJoinRoom;
	RoomIndex = Other.RoomIndex;
	
	ZeroMemory(&Buffer, ClientInfo::MAX_BUFFER_SIZE);
	SendingLeftPos = 0;
	SendingRightPos = 0;
	RecvSize = 0;

	//복사 생성자에서 필요한 정보만 Other로 부터 복사하고 나머지는 default 값으로
	//초기화 합니다.
}


std::pair<bool, unsigned int> CustomRecv(SOCKET S, char* Buf, int Len, int Flags, ClientInfo& CommandRequestor)
{
	int recvSize = 0;
	//recv시 문제가 발생한 경우 SOCKET_ERROR를 리턴하는데 이 값이 -1이라
	//uint가 아닌 int로 했습니다.

	recvSize = recv(S, Buf + CommandRequestor.RecvSize, Len, Flags);

	if (recvSize == 0 || recvSize == SOCKET_ERROR)
	{
		return { false, SOCKET_ERROR };
	}

	CommandRequestor.RecvSize += recvSize;
	if (Buf[CommandRequestor.RecvSize - 1] == '\n')
	{
		if (CommandRequestor.RecvSize > ClientInfo::CRLR_SIZE)
		{
			Buf[CommandRequestor.RecvSize - 1] = '\0';
			Buf[CommandRequestor.RecvSize - 2] = '\0';
			CommandRequestor.RecvSize -= ClientInfo::CRLR_SIZE;
		}
		//클라이언트에게 넘어온 \r\n을 서버단에서 무시하기 위해서
		return { true, CommandRequestor.RecvSize };
	}

	return { false, CommandRequestor.RecvSize };
}

std::pair<bool, unsigned int> CustomSend(SOCKET S, const char* Buf, int Len, int Flags, ClientInfo& CommandRequestor)
{
	if (!CommandRequestor.IsSend)
	{
		CommandRequestor.SendingRightPos = Len;
		CommandRequestor.SendingLeftPos = 0;
		ZeroMemory(CommandRequestor.Buffer.data(), ClientInfo::MAX_BUFFER_SIZE);
		strcpy(CommandRequestor.Buffer.data(), Buf);
	}
	//최초 전송시 원래 보내고자했던 사이즈를 기록해두고, 버퍼에 내용물을 복사합니다.
	
	int sndSize = 0;
	//recv와 마찬가지로 SOCKET_ERROR값이 -1이기 때문에 uint가 아닌 int를 사용했습니다.
	
	sndSize = send(S, CommandRequestor.Buffer.data() + CommandRequestor.SendingLeftPos, Len, Flags);

	if (sndSize == SOCKET_ERROR)
	{
		return { false, SOCKET_ERROR };
	}

	if (sndSize == Len)//모든 데이터를 송신 버퍼에 복사 성공!
	{
		CommandRequestor.IsSend = false;
		return { true, sndSize };
	}
	else
	{
		CommandRequestor.IsSend = true;
		CommandRequestor.SendingLeftPos += sndSize;
		return { false, sndSize };
	}
	//데이터를 전부 송신버퍼에 복사하지 못한 경우
	//IsSend를 true로 만들어 writeSet에 해당 소켓이 들어가
	//다음 기회에 남은 데이터를 다시 보낼 수 있도록 구현했습니다.
}

std::string GetCurrentSystemTime()
{
	__time64_t curTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	tm tm;
	localtime_s(&tm, &curTime);
	
	return std::to_string(tm.tm_hour) + ":" + std::to_string(tm.tm_min);
}

std::vector<std::string> Tokenizing(const std::string& Str)
{
	std::string temp;
	std::istringstream is{ Str };
	std::vector<std::string> tokens;
	while (std::getline(is, temp, ' '))
	{
		tokens.emplace_back(temp);
	}
	return tokens;
}