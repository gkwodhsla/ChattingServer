#include "Common.h"

std::pair<bool, unsigned int> CustomRecv(SOCKET S, char* Buf, int Len, int Flags, ClientInfo& ClntInfo)
{
	int rcvSize = 0;
	//SOCKET_ERROR가 -1이라 int로 했습니다.

	rcvSize = recv(S, Buf + ClntInfo.RcvSize, Len, Flags);

	if (rcvSize == 0 || rcvSize == SOCKET_ERROR)
	{
		return { false, 0 };
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