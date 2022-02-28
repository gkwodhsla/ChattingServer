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

public:
	void ExecutingCommand(ClientInfo& CommandRequestor, const int ClntIndex, std::string& Command);

private:
	const std::string CommandList = { "\r\nH\t\t\t\t\tShow all command list\r\nUS\t\t\t\t\tShow all user\r\nLT\t\t\t\t\tShow all chattingroom\r\nST [room number]\t\t\tShow chattingroom info\r\nPF[other user Id]\t\t\tShow user info\r\nTO [other user Id] [message]\t\tSend mail\r\nO [Maximum participants] [Room name]\tCreate chattingroom\r\nJ [Room number]\t\t\t\tJoin the chattingroom\r\nX\t\t\t\t\tdisconnecting\r\n" };
};