#pragma once
#include "Common.h"
//TotalManager���� ���ָ� �޾� ��ɾ ó�����ִ� Ŭ�����̴�.

class Outsourcer
{
public:
	Outsourcer() = default;
	virtual ~Outsourcer() = default;

public:
	void SendingCommandList();
	void SendingUserList();
	void SendingChattingroomList();
	void SendingChattingroomInfo();
	void SendingUserInfo();
	//��������� ��û�� Ŭ���̾�Ʈ���� �޽��� ������ ��
public:
	void SendingMail();
	void CreatingChattingroom();
	void EnteringChattingroom();
	void DisconnectingClient();

public:
	void ExecutingCommand(std::string& command);
};