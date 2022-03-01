#define _CRT_SECURE_NO_WARNINGS
#include "Common.h"

std::pair<bool, unsigned int> CustomRecv(SOCKET S, char* Buf, int Len, int Flags, ClientInfo& ClntInfo)
{
	int rcvSize = 0;
	//SOCKET_ERROR가 -1이라 int로 했습니다.

	rcvSize = recv(S, Buf + ClntInfo.RcvSize, Len, Flags);

	if (rcvSize == 0 || rcvSize == SOCKET_ERROR)
	{
		return { false, SOCKET_ERROR };
	}

	ClntInfo.RcvSize += rcvSize;
	if (Buf[ClntInfo.RcvSize - 1] == '\n')
	{
		if (ClntInfo.RcvSize > ClientInfo::CRLR_SIZE)
		{
			Buf[ClntInfo.RcvSize - 1] = '\0';
			Buf[ClntInfo.RcvSize - 2] = '\0';
			ClntInfo.RcvSize -= ClientInfo::CRLR_SIZE;
		}
		//클라이언트에게 넘어온 \r\n을 서버단에서 무시하기 위해서
		return { true, ClntInfo.RcvSize };
	}

	return { false, ClntInfo.RcvSize };
}

std::pair<bool, unsigned int> CustomSend(SOCKET S, const char* Buf, int Len, int Flags, ClientInfo& ClntInfo)
{
	if (!ClntInfo.IsSend)
	{
		ClntInfo.SendingRightPos = Len;
		ClntInfo.SendingLeftPos = 0;
		ZeroMemory(ClntInfo.Buffer.data(), ClientInfo::MAX_BUFFER_SIZE);
		strcpy(ClntInfo.Buffer.data(), Buf);
	}
	//최초 전송시 원래 보내고자했던 사이즈를 기록해두고, 버퍼에 내용물을 복사해둔다.
	
	int sndSize = 0;
	
	sndSize = send(S, ClntInfo.Buffer.data() + ClntInfo.SendingLeftPos, Len, Flags);

	if (sndSize == SOCKET_ERROR)
	{
		return { false, SOCKET_ERROR };
	}

	if (sndSize == Len)//모든 데이터를 송신 버퍼에 복사 성공!
	{
		ClntInfo.IsSend = false;
		return { true, sndSize };
	}
	else
	{
		ClntInfo.IsSend = true;
		ClntInfo.SendingLeftPos += sndSize;
		return { false, sndSize };
	}
	//데이터를 전부 송신버퍼에 복사하지 못한 경우
	//IsSend를 true로 만들어 writeSet에 해당 소켓이 들어가
	//다음 기회에 남은 데이터를 다시 보낼 수 있도록 한다.
}