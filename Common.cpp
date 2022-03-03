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

ClientInfo::ClientInfo(const ClientInfo& Other):ClientSock(Other.ClientSock), Name(Other.Name), ConnectionPoint(Other.ConnectionPoint),
EnteringTime(Other.EnteringTime), IsLogin(Other.IsLogin), IsSend(Other.IsSend), IsJoinRoom(Other.IsJoinRoom), RoomIndex(Other.RoomIndex),
SendingLeftPos(0), SendingRightPos(0), RecvSize(0)
{
	strcpy(Buffer.data(), Other.Buffer.data());
	//���� �����ڿ��� �ʿ��� ������ Other�� ���� �����ϰ� 
	//�������� default ������ �ʱ�ȭ �մϴ�.
}

ClientInfo& ClientInfo::operator=(const ClientInfo& Rhs)
{
	if (this == &Rhs)
	{
		return *this;
	}
	ClientSock = Rhs.ClientSock;
	strcpy(Buffer.data(), Rhs.Buffer.data());
	Name = Rhs.Name;
	ConnectionPoint = Rhs.ConnectionPoint;
	EnteringTime = Rhs.EnteringTime;
	IsLogin = Rhs.IsLogin;
	IsSend = Rhs.IsSend;
	IsJoinRoom = Rhs.IsJoinRoom;
	RoomIndex = Rhs.RoomIndex;
	SendingLeftPos = Rhs.SendingLeftPos;
	SendingRightPos = Rhs.SendingRightPos;
	RecvSize = Rhs.RecvSize;

	return *this;
}

std::pair<bool, unsigned int> CustomRecv(SOCKET S, char* Buf, int Len, int Flags, ClientInfo& CommandRequestor)
{
	int recvSize = 0;
	//recv�� ������ �߻��� ��� SOCKET_ERROR�� �����ϴµ� �� ���� -1�̶�
	//uint�� �ƴ� int�� �߽��ϴ�.
	Len = min(Len, ClientInfo::MAX_BUFFER_SIZE);
	//�ִ� ���� ũ�⸦ �ѱ� ��� ������ Len�� ���Դϴ�.
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
		return { true, min(ClientInfo::MAX_BUFFER_SIZE - 1, CommandRequestor.RecvSize) };
		//�ִ� ���� ũ�⸦ �Ѿ ������� �����մϴ�.
	}

	return { false, min(ClientInfo::MAX_BUFFER_SIZE - 1, CommandRequestor.RecvSize) };
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
	
	int sendSize = 0;
	//recv�� ���������� SOCKET_ERROR���� -1�̱� ������ uint�� �ƴ� int�� ����߽��ϴ�.
	Len = min(Len, ClientInfo::MAX_BUFFER_SIZE);
	
	sendSize = send(S, CommandRequestor.Buffer.data() + CommandRequestor.SendingLeftPos, Len, Flags);

	if (sendSize == SOCKET_ERROR)
	{
		return { false, SOCKET_ERROR };
	}

	if (sendSize == Len)//��� �����͸� �۽� ���ۿ� ���� ����!
	{
		CommandRequestor.IsSend = false;
		return { true, sendSize };
	}
	else
	{
		CommandRequestor.IsSend = true;
		CommandRequestor.SendingLeftPos += sendSize;
		return { false, sendSize };
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
	std::istringstream stream{ Str };
	std::vector<std::string> tokens;
	while (std::getline(stream, temp, ' '))
	{
		tokens.emplace_back(temp);
	}
	return tokens;
}