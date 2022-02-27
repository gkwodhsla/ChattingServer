#include "Outsourcer.h"
#include <algorithm>
#include <sstream>
#include <iostream>

void Outsourcer::SendingCommandList()
{
	std::cout << "��ɾ� ����� Ŭ���̾�Ʈ���� ���� ����" << std::endl;
}

void Outsourcer::SendingUserList()
{
	std::cout << "���� ����Ʈ�� Ŭ���̾�Ʈ���� ���� ����" << std::endl;
}

void Outsourcer::SendingChattingroomList()
{
	std::cout << "ä�÷� ����Ʈ�� Ŭ���̾�Ʈ���� ���� ����" << std::endl;
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
	//�켱 ��ɾ ��� �ҹ��ڷ� ��ȯ�Ѵ�.

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
		//��ɾ ����� �Է��϶�� Ŭ�󿡰� �����ֱ�
	}
}
