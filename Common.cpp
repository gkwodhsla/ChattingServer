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

	//���� �����ڿ��� �ʿ��� ������ Other�� ���� �����ϰ� �������� default ������
	//�ʱ�ȭ �մϴ�.
}


std::pair<bool, unsigned int> CustomRecv(SOCKET S, char* Buf, int Len, int Flags, ClientInfo& CommandRequestor)
{
	int recvSize = 0;
	//recv�� ������ �߻��� ��� SOCKET_ERROR�� �����ϴµ� �� ���� -1�̶�
	//uint�� �ƴ� int�� �߽��ϴ�.

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
		//Ŭ���̾�Ʈ���� �Ѿ�� \r\n�� �����ܿ��� �����ϱ� ���ؼ�
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
	//���� ���۽� ���� ���������ߴ� ����� ����صΰ�, ���ۿ� ���빰�� �����մϴ�.
	
	int sndSize = 0;
	//recv�� ���������� SOCKET_ERROR���� -1�̱� ������ uint�� �ƴ� int�� ����߽��ϴ�.
	
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
	//���� ��ȸ�� ���� �����͸� �ٽ� ���� �� �ֵ��� �����߽��ϴ�.
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