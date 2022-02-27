#include "Outsourcer.h"
#include <algorithm>
#include <sstream>
#include <iostream>

void Outsourcer::SendingCommandList()
{
	std::cout << "명령어 목록을 클라이언트에게 보낼 예정" << std::endl;
}

void Outsourcer::SendingUserList()
{
	std::cout << "유저 리스트를 클라이언트에게 보낼 예정" << std::endl;
}

void Outsourcer::SendingChattingroomList()
{
	std::cout << "채팅룸 리스트를 클라이언트에게 보낼 예정" << std::endl;
}

void Outsourcer::SendingChattingroomInfo()
{
}

void Outsourcer::SendingUserInfo()
{
}

void Outsourcer::SendingMail()
{
}

void Outsourcer::CreatingChattingroom()
{
}

void Outsourcer::EnteringChattingroom()
{
}

void Outsourcer::DisconnectingClient()
{
}

void Outsourcer::ExecutingCommand(std::string& command)
{
	std::transform(command.begin(), command.end(), command.begin(),
		[](unsigned char c) { return tolower(c); });
	//우선 명령어를 모두 소문자로 변환한다.

	std::string temp;
	std::istringstream is{ command };
	std::vector<std::string> tokens;
	while (std::getline(is, temp, ' '))
	{
		tokens.emplace_back(temp);
	}
	
	if (tokens[0] == "h")
	{
		SendingCommandList();
	}
	else if (tokens[0] == "us")
	{
		SendingUserList();
	}
	else if (tokens[0] == "lt")
	{
		SendingChattingroomList();
	}
	else if (tokens[0] == "st")
	{
		SendingChattingroomInfo();
	}
	else if (tokens[0] == "pf")
	{
		SendingUserInfo();
	}
	else if (tokens[0] == "to")
	{
		SendingMail();
	}
	else if (tokens[0] == "o")
	{
		CreatingChattingroom();
	}
	else if (tokens[0] == "j")
	{
		EnteringChattingroom();
	}
	else if (tokens[0] == "x")
	{
		DisconnectingClient();
	}
	else
	{
		//명령어를 제대로 입력하라고 클라에게 보내주기
	}
}
