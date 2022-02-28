#define _CRT_SECURE_NO_WARNINGS
#include "ChattingBuilding.h"
#include "Outsourcer.h"
#include "TotalManager.h"
#include <algorithm>
#include <sstream>
#include <iostream>

void Outsourcer::SendingCommandList(ClientInfo& CommandRequestor)
{
	int sendLen = 0;
	CommandRequestor.IsSend = true;
	strcpy(CommandRequestor.Buffer.data(), CommandList.c_str());
	CommandRequestor.SendingSize = CommandList.size();

	//sendLen = send(CommandRequestor.ClntSock, CommandList.c_str(), CommandList.size(), 0);
	std::cout << "Send command list to requesting client" << std::endl;
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

void Outsourcer::CreatingChattingroom(const std::string& RoomName, const int ClntIndex, const int MaxParticipant)
{
	std::vector<ChattingBuilding*>& buildings = TotalManager::Instance().GetBuildings();
	std::vector<ClientInfo>& clientInfos = TotalManager::Instance().GetClientInfos();
	
	while (1)
	{
		bool isRoomExist = false;
		for (int i = 0; i < buildings.size(); ++i)
		{
			if (buildings[i]->IsThereAnyEmptyRoom())//해당 건물에 빈 방이 있다면
			{
				buildings[i]->OccupyingRoom(RoomName, clientInfos[ClntIndex], MaxParticipant); //방을 차지하게 해주고, 요청한 클라이언트를 방에 넣어준다.
				isRoomExist = true;
				break;
			}
		}
		if (!isRoomExist) //만약 빈 방을 찾지 못 했다면 새로운 건물을 지어준다.
		{
			buildings.emplace_back(new ChattingBuilding());
			std::vector<std::thread>& subThreads = TotalManager::Instance().GetSubThreads();
			subThreads.emplace_back(std::thread(&ChattingBuilding::ProcessingLogic, buildings.back()));
		}
		else
		{
			break;
		}
	}
}

void Outsourcer::EnteringChattingroom(const int RoomIndex, ClientInfo& CommandRequestor)
{
	std::vector<ChattingBuilding*>& buildings = TotalManager::Instance().GetBuildings();
	int maxRoom = ChattingBuilding::MAX_ROOM_NUM;
	buildings[RoomIndex / maxRoom]->EnteringRoom(RoomIndex % maxRoom, CommandRequestor);
}

void Outsourcer::DisconnectingClient()
{
}

void Outsourcer::ExecutingCommand(ClientInfo& CommandRequestor, const int ClntIndex, std::string& Command)
{
	std::transform(Command.begin(), Command.end(), Command.begin(),
		[](unsigned char c) { return tolower(c); });
	//우선 명령어를 모두 소문자로 변환한다.

	std::string temp;
	std::istringstream is{ Command };
	std::vector<std::string> tokens;
	while (std::getline(is, temp, ' '))
	{
		tokens.emplace_back(temp);
	}
	
	if (tokens[0] == "h")
	{
		SendingCommandList(CommandRequestor);
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
		if (std::stoi(tokens[1]) > 16)
		{
			std::string msg{ "\r\nMaximum participant must under 16" };
			send(CommandRequestor.ClntSock, msg.c_str(), msg.size(), 0);
		}
		else
		{
			CreatingChattingroom(tokens[2], ClntIndex, std::stoi(tokens[1]));
		}
	}
	else if (tokens[0] == "j")
	{
		EnteringChattingroom(std::stoi(tokens[1]), CommandRequestor);
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
