#pragma once
#include "Common.h"
//TotalManager���� ���ָ� �޾� ��ɾ ó�����ִ� Ŭ�����Դϴ�.

class Outsourcer final
{
public:
	static Outsourcer& Instance()
	{
		static Outsourcer instance;
		return instance;
	}

public:
	void ExecutingCommand(ClientInfo& CommandRequestor, const int ClientIndex, std::string& Command);
	//TotalManager�� �� �Լ��� ���� Outsourcer���� ��ɾ ��� ó���ϰ� �ñ�ϴ�.

private:
	Outsourcer() = default;
	virtual ~Outsourcer() = default;
	Outsourcer(const Outsourcer&) = delete;
	Outsourcer& operator=(const Outsourcer&) = delete;

private:
	void ExecutingChattingRoomCommand(ClientInfo& CommandRequestor, std::string& Command);
	void SendingCommandList(ClientInfo& CommandRequestor);
	//Ŭ���̾�Ʈ�� ����� �� �ִ� ��ɾ� ����Ʈ�� �����ݴϴ�.
	void SendingUserList(ClientInfo& CommandRequestor);
	//���� ����� Ŭ���̾�Ʈ���� �����ϴ�.
	void SendingChattingroomList(ClientInfo& CommandRequestor);
	//ä�ù� ����� Ŭ���̾�Ʈ���� �����ϴ�.
	void SendingChattingroomInfo(ClientInfo& CommandRequestor, const std::string& RoomIndex);
	//Ư�� ä�ù� ������ Ŭ���̾�Ʈ���� �����ϴ�.
	void SendingUserInfo(ClientInfo& CommandRequestor, const std::string& Name);
	//Ư�� ������ ������ Ŭ���̾�Ʈ���� �����ϴ�.
	//��������� ��û�� Ŭ���̾�Ʈ���� �޽��� ������ ��
private:
	void Login(ClientInfo& CommandRequestor, const std::string& Name);
	//�α����� ó�����ݴϴ�.
	void SendingMail(ClientInfo& CommandRequestor, const std::string& Name, const std::string& Msg);
	//Ư�� �������� �ӼӸ��� �����ϴ�.
	void CreatingChattingroom(const std::string& RoomName, const int ClientIndex, const int MaxParticipant, ClientInfo& CommandRequestor);
	//Ŭ���̾�Ʈ�� ��û�� ����� �ִ� �����ο��� ������ ä�ù��� ������ݴϴ�.
	void EnteringChattingroom(const int RoomIndex, ClientInfo& CommandRequestor);
	//ä�ù濡 �����մϴ�.
	void DisconnectingClient(ClientInfo& CommandRequestor);
	//�� ��û�� �䱸�� Ŭ���̾�Ʈ�� ������ �����մϴ�.
	void QuitRoom(ClientInfo& CommandRequestor);
//���� 9�� �Լ��� �ϳ� �� ä������ �����Դϴ�.

private:
	bool CheckingAlphabetInStr(const std::string&);

private:
	static inline const std::string CommandList = { "\r\nH\t\t\t\t\tShow all command list\r\nUS\t\t\t\t\tShow all user\r\nLT\t\t\t\t\tShow all chattingroom\r\nST [room number]\t\t\tShow chattingroom info\r\nPF [other user Id]\t\t\tShow user info\r\nTO [other user Id] [message]\t\tSend mail\r\nO [Maximum participants] [Room name]\tCreate chattingroom\r\nJ [Room number]\t\t\t\tJoin the chattingroom\r\nX\t\t\t\t\tdisconnecting\r\n" };
	static inline const std::string CommandListForRoom = { "\r\n/H\t\t\t\t\tShow all command list\r\n/US\t\t\t\t\tShow all user\r\n/LT\t\t\t\t\tShow all chattingroom\r\n/ST [room number]\t\t\tShow chattingroom info\r\n/PF [other user Id]\t\t\tShow user info\r\n/TO [other user Id] [message]\t\tSend mail\r\n/IN [User Name]\t\t\t\tInviting user\r\n/Q\t\t\t\t\tQuit\r\n\r\n" };
	//Ŭ���̾�Ʈ�� hŰ�� ���� ��ɾ ��û���� �� ���� �޽����Դϴ�.
	static const unsigned int CORRECT_LOGIN_TOKEN_NUM = 2;
};

//�� Ŭ������ TotalManager���� ��ɾ �޾� �Ľ��ϰ�, ��ɾ ����
//�����ϰ� ó�����ִ� Ŭ�����Դϴ�.
//���� ��ɾ� ����Ʈ ���, �� ����, �� ���Ա��� �����߽��ϴ�.
//3�� 1�Ͽ� �� ����, ���� ���� �� �ٵ��, ��� �� Ư�� ä�ù�, ���� ��� �����غ����� �մϴ�.