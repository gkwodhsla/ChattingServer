#include "Common.h"

std::pair<bool, int> CustomRecv(SOCKET S, char* Buf, int Len, int Flags, ClientInfo& ClntInfo)
{
	int rcvSize = 0;

	rcvSize = recv(S, Buf + ClntInfo.RcvSize, Len, Flags);

	if (rcvSize == 0 || rcvSize == SOCKET_ERROR)
	{
		return { false, 0 };
	}

	ClntInfo.RcvSize += rcvSize;
	if (Buf[ClntInfo.RcvSize - 1] == '\n')
	{
		Buf[ClntInfo.RcvSize - 1] = '\0';
		Buf[ClntInfo.RcvSize - 2] = '\0';
		ClntInfo.RcvSize -= 2;
		//클라이언트에게 넘어온 \r\n을 서버단에서 무시하기 위해서
		return { true, ClntInfo.RcvSize };
	}

	return { false, ClntInfo.RcvSize };
}