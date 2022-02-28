#pragma once
#include "Common.h"
//TotalManager���� ���ָ� �޾� ��ɾ ó�����ִ� Ŭ�����̴�.

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
	//��������� ��û�� Ŭ���̾�Ʈ���� �޽��� ������ ��
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