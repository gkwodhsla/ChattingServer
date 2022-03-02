#pragma once
#include "Common.h"
//TotalManager에게 외주를 받아 명령어를 처리해주는 클래스입니다.

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
	//TotalManager는 이 함수를 통해 Outsourcer에게 명령어를 대신 처리하게 맡깁니다.

private:
	Outsourcer() = default;
	virtual ~Outsourcer() = default;
	Outsourcer(const Outsourcer&) = delete;
	Outsourcer& operator=(const Outsourcer&) = delete;

private:
	void ExecutingChattingRoomCommand(ClientInfo& CommandRequestor, std::string& Command);
	void SendingCommandList(ClientInfo& CommandRequestor);
	//클라이언트가 사용할 수 있는 명령어 리스트를 보내줍니다.
	void SendingUserList(ClientInfo& CommandRequestor);
	//유저 목록을 클라이언트에게 보냅니다.
	void SendingChattingroomList(ClientInfo& CommandRequestor);
	//채팅방 목록을 클라이언트에게 보냅니다.
	void SendingChattingroomInfo(ClientInfo& CommandRequestor, const std::string& RoomIndex);
	//특정 채팅방 정보를 클라이언트에게 보냅니다.
	void SendingUserInfo(ClientInfo& CommandRequestor, const std::string& Name);
	//특정 유저의 정보를 클라이언트에게 보냅니다.
	//여기까지는 요청한 클라이언트에게 메시지 보내는 것
private:
	void Login(ClientInfo& CommandRequestor, const std::string& Name);
	//로그인을 처리해줍니다.
	void SendingMail(ClientInfo& CommandRequestor, const std::string& Name, const std::string& Msg);
	//특정 유저에게 귓속말을 보냅니다.
	void CreatingChattingroom(const std::string& RoomName, const int ClientIndex, const int MaxParticipant, ClientInfo& CommandRequestor);
	//클라이언트가 요청한 제목과 최대 참가인원을 참고해 채팅방을 만들어줍니다.
	void EnteringChattingroom(const int RoomIndex, ClientInfo& CommandRequestor);
	//채팅방에 진입합니다.
	void DisconnectingClient(ClientInfo& CommandRequestor);
	//이 요청을 요구한 클라이언트와 연결을 종료합니다.
	void QuitRoom(ClientInfo& CommandRequestor);
//위에 9개 함수를 하나 씩 채워나갈 예정입니다.

private:
	bool CheckingAlphabetInStr(const std::string&);

private:
	static inline const std::string CommandList = { "\r\nH\t\t\t\t\tShow all command list\r\nUS\t\t\t\t\tShow all user\r\nLT\t\t\t\t\tShow all chattingroom\r\nST [room number]\t\t\tShow chattingroom info\r\nPF [other user Id]\t\t\tShow user info\r\nTO [other user Id] [message]\t\tSend mail\r\nO [Maximum participants] [Room name]\tCreate chattingroom\r\nJ [Room number]\t\t\t\tJoin the chattingroom\r\nX\t\t\t\t\tdisconnecting\r\n" };
	static inline const std::string CommandListForRoom = { "\r\n/H\t\t\t\t\tShow all command list\r\n/US\t\t\t\t\tShow all user\r\n/LT\t\t\t\t\tShow all chattingroom\r\n/ST [room number]\t\t\tShow chattingroom info\r\n/PF [other user Id]\t\t\tShow user info\r\n/TO [other user Id] [message]\t\tSend mail\r\n/IN [User Name]\t\t\t\tInviting user\r\n/Q\t\t\t\t\tQuit\r\n\r\n" };
	//클라이언트가 h키를 통해 명령어를 요청했을 때 보낼 메시지입니다.
	static const unsigned int CORRECT_LOGIN_TOKEN_NUM = 2;
};

//이 클래스는 TotalManager에게 명령어를 받아 파싱하고, 명령어에 따라
//적절하게 처리해주는 클래스입니다.
//현재 명령어 리스트 출력, 방 생성, 방 진입까지 구현했습니다.
//3월 1일에 방 생성, 진입 조금 더 다듬고, 목록 및 특정 채팅방, 유저 출력 구현해보고자 합니다.