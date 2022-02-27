#pragma once
#include "Common.h"
//TotalManager에게 외주를 받아 명령어를 처리해주는 클래스이다.

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
	//여기까지는 요청한 클라이언트에게 메시지 보내는 것
public:
	void SendingMail();
	void CreatingChattingroom();
	void EnteringChattingroom();
	void DisconnectingClient();

public:
	void ExecutingCommand(std::string& command);
};