#define _CRT_SECURE_NO_WARNINGS
#include "Common.h"

std::pair<bool, unsigned int> CustomRecv(SOCKET S, char* Buf, int Len, int Flags, ClientInfo& ClntInfo)
{
	int rcvSize = 0;
	//SOCKET_ERROR�� -1�̶� int�� �߽��ϴ�.

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
		//Ŭ���̾�Ʈ���� �Ѿ�� \r\n�� �����ܿ��� �����ϱ� ���ؼ�
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
	//���� ���۽� ���� ���������ߴ� ����� ����صΰ�, ���ۿ� ���빰�� �����صд�.
	
	int sndSize = 0;
	
	sndSize = send(S, ClntInfo.Buffer.data() + ClntInfo.SendingLeftPos, Len, Flags);

	if (sndSize == SOCKET_ERROR)
	{
		return { false, SOCKET_ERROR };
	}

	if (sndSize == Len)//��� �����͸� �۽� ���ۿ� ���� ����!
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
	//�����͸� ���� �۽Ź��ۿ� �������� ���� ���
	//IsSend�� true�� ����� writeSet�� �ش� ������ ��
	//���� ��ȸ�� ���� �����͸� �ٽ� ���� �� �ֵ��� �Ѵ�.
}