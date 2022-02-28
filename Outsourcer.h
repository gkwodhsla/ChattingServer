#pragma once
#include "Common.h"
//TotalManager에게 외주를 받아 명령어를 처리해주는 클래스이다.

class Outsourcer final
{
public:
	Outsourcer() = default;
	virtual ~Outsourcer() = default;

public:
	void SendingCommandList(ClientInfo& CommandRequestor);
	void SendingUserList();
	void SendingChattingroomList(ClientInfo& CommandRequestor);
	void SendingChattingroomInfo();
	void SendingUserInfo();
	//여기까지는 요청한 클라이언트에게 메시지 보내는 것
public:
	void SendingMail();
	void CreatingChattingroom(const std::string& RoomName, const int ClntIndex, const int MaxParticipant);
	void EnteringChattingroom(const int RoomIndex, ClientInfo& CommandRequestor);
	void DisconnectingClient();

//위에 9개 함수를 하나 씩 채워나갈 예정

public:
	void ExecutingCommand(ClientInfo& CommandRequestor, const int ClntIndex, std::string& Command);
	//TotalManager는 이 함수를 통해 Outsourcer에게 명령어를 대신 처리하게 맡긴다.

private:
	const std::string CommandList = { "\r\nH\t\t\t\t\tShow all command list\r\nUS\t\t\t\t\tShow all user\r\nLT\t\t\t\t\tShow all chattingroom\r\nST [room number]\t\t\tShow chattingroom info\r\nPF[other user Id]\t\t\tShow user info\r\nTO [other user Id] [message]\t\tSend mail\r\nO [Maximum participants] [Room name]\tCreate chattingroom\r\nJ [Room number]\t\t\t\tJoin the chattingroom\r\nX\t\t\t\t\tdisconnecting\r\n" };
	//클라이언트가 h키를 통해 명령어를 요청했을 때 보낼 메시지이다.
};

//이 클래스는 TotalManager에게 명령어를 받아 파싱하고, 명령어에 따라
//적절하게 처리해주는 클래스이다.