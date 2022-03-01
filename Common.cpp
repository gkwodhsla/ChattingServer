#define _CRT_SECURE_NO_WARNINGS
#include "Common.h"

std::pair<bool, unsigned int> CustomRecv(SOCKET S, char* Buf, int Len, int Flags, ClientInfo& CommandRequestor)
{
	int rcvSize = 0;
	//SOCKET_ERROR가 -1이라 int로 했습니다.

	rcvSize = recv(S, Buf + CommandRequestor.RcvSize, Len, Flags);

	if (rcvSize == 0 || rcvSize == SOCKET_ERROR)
	{
		return { false, SOCKET_ERROR };
	}

	CommandRequestor.RcvSize += rcvSize;
	if (Buf[CommandRequestor.RcvSize - 1] == '\n')
	{
		if (CommandRequestor.RcvSize > ClientInfo::CRLR_SIZE)
		{
			Buf[CommandRequestor.RcvSize - 1] = '\0';
			Buf[CommandRequestor.RcvSize - 2] = '\0';
			CommandRequestor.RcvSize -= ClientInfo::CRLR_SIZE;
		}
		//클라이언트에게 넘어온 \r\n을 서버단에서 무시하기 위해서
		return { true, CommandRequestor.RcvSize };
	}

	return { false, CommandRequestor.RcvSize };
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
	//최초 전송시 원래 보내고자했던 사이즈를 기록해두고, 버퍼에 내용물을 복사해둔다.
	
	int sndSize = 0;
	
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
	//다음 기회에 남은 데이터를 다시 보낼 수 있도록 한다.
}

std::string GetCurrentSystemTime()
{
	__time64_t curTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	tm tm;
	localtime_s(&tm, &curTime);
	
	return std::to_string(tm.tm_hour) + ":" + std::to_string(tm.tm_min);
}