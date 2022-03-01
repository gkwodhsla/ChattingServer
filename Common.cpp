#define _CRT_SECURE_NO_WARNINGS
#include "Common.h"

std::pair<bool, unsigned int> CustomRecv(SOCKET S, char* Buf, int Len, int Flags, ClientInfo& CommandRequestor)
{
	int rcvSize = 0;
	//SOCKET_ERROR�� -1�̶� int�� �߽��ϴ�.

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
		//Ŭ���̾�Ʈ���� �Ѿ�� \r\n�� �����ܿ��� �����ϱ� ���ؼ�
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
	//���� ���۽� ���� ���������ߴ� ����� ����صΰ�, ���ۿ� ���빰�� �����صд�.
	
	int sndSize = 0;
	
	sndSize = send(S, CommandRequestor.Buffer.data() + CommandRequestor.SendingLeftPos, Len, Flags);

	if (sndSize == SOCKET_ERROR)
	{
		return { false, SOCKET_ERROR };
	}

	if (sndSize == Len)//��� �����͸� �۽� ���ۿ� ���� ����!
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
	//�����͸� ���� �۽Ź��ۿ� �������� ���� ���
	//IsSend�� true�� ����� writeSet�� �ش� ������ ��
	//���� ��ȸ�� ���� �����͸� �ٽ� ���� �� �ֵ��� �Ѵ�.
}

std::string GetCurrentSystemTime()
{
	__time64_t curTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	tm tm;
	localtime_s(&tm, &curTime);
	
	return std::to_string(tm.tm_hour) + ":" + std::to_string(tm.tm_min);
}