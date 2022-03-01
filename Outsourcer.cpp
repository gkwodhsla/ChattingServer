#define _CRT_SECURE_NO_WARNINGS
#include "ChattingBuilding.h"
#include "Outsourcer.h"
#include "TotalManager.h"
#include <algorithm>
#include <cctype>
#include <sstream>
#include <iostream>

void Outsourcer::SendingCommandList(ClientInfo& CommandRequestor)
{
	std::pair<bool, int> sendResult = CustomSend(CommandRequestor.ClntSock, CommandList.c_str(), CommandList.size(), 0, CommandRequestor);
	
	//if (sendResult.second == SOCKET_ERROR)
	//{
	//	return false;
	//}
	std::cout << " Send command list to requesting client" << std::endl;
}

void Outsourcer::SendingUserList()
{
	std::cout << "유저 리스트를 클라이언트에게 보낼 예정" << std::endl;
}

void Outsourcer::SendingChattingroomList(ClientInfo& CommandRequestor)
{
	std::vector<ChattingBuilding*>& buildings = TotalManager::Instance().GetBuildings();
	
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
				buildings[i]->OccupyingRoom(RoomName, clientInfos[ClntIndex], MaxParticipant);
				//방을 차지하게 해주고(채팅방 개설), 방 개설을 요청한 클라이언트를 방에 넣어준다.
				//(방에 넣어주는 코드는 OccupyingRoom 함수에 존재)
				isRoomExist = true;
				break;
			}
		}
		if (!isRoomExist) //만약 빈 방을 찾지 못 했다면 새로운 건물을 지어주고, 다시 빈 방을 찾도록 한다.
		{
			buildings.emplace_back(new ChattingBuilding());
			std::vector<std::thread>& subThreads = TotalManager::Instance().GetSubThreads();
			subThreads.emplace_back(std::thread(&ChattingBuilding::ProcessingLogic, buildings.back()));
			//만약 새롭게 채팅 빌딩이 하나 세워졌다면 해당 빌딩에 속한 클라이언트의 요구(메시지 주고 받기)를
			//처리해줄 새로운 쓰레드를 하나 만든다.
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
	if (buildings.size() * ChattingBuilding::MAX_ROOM_NUM  - 1 < RoomIndex)
	{
		std::string failMsg = "You can't entering the room (room is not exist)\r\n";
		CustomSend(CommandRequestor.ClntSock, failMsg.c_str(), failMsg.size(), 0, CommandRequestor);
	}
	else
	{
		int maxRoom = ChattingBuilding::MAX_ROOM_NUM;
		int buildingIndex = RoomIndex / maxRoom;
		int roomIndex = RoomIndex % maxRoom;
		//RoomIndex/maxRoom -> 채팅빌딩 중 몇 번째 채팅빌딩인가?
		//RoomIndex % maxRoom -> 앞에서 정한 채팅빌딩 중 몇 번째 방인가?

		buildings[buildingIndex]->EnteringRoom(roomIndex, CommandRequestor);
	}
}

void Outsourcer::DisconnectingClient()
{
}

void Outsourcer::ExecutingCommand(ClientInfo& CommandRequestor, const int ClntIndex, std::string& Command)
{
	std::transform(Command.begin(), Command.end(), Command.begin(),
		[](unsigned char c) { return tolower(c); });
	//우선 입력받은 명령어를 모두 소문자로 바꾼다.

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
		SendingChattingroomList(CommandRequestor);
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
		bool isThereAlphabet = false;
		for (int i = 0; i < tokens[1].size(); ++i)
		{
			if (isalpha(tokens[1][i]))
			{
				isThereAlphabet = true;
				break;
			}
		}
		if (isThereAlphabet)
		{
			std::string failMsg{ "\r\nParticipant count only accept numeric number\r\n" };
			CustomSend(CommandRequestor.ClntSock, failMsg.c_str(), failMsg.size(), 0, CommandRequestor);
			//if (sendResult.second == SOCKET_ERROR)
			//{
			//	return false;
			//}
		}
		//숫자가 나와야하는데 알파벳이 하나라도 나온다면 다시 명령어를 입력해달라고 요청한다.
		else if (std::stoi(tokens[1]) > 16 || std::stoi(tokens[1]) < 2)
		{
			std::string failMsg{ "\r\nparticipant range must (2 ~ 16)\r\n" };
			CustomSend(CommandRequestor.ClntSock, failMsg.c_str(), failMsg.size(), 0, CommandRequestor);
			//if (sendResult.second == SOCKET_ERROR)
			//{
			//	return false;
			//}
		}
		//만약 최대 참석인원 수를 넘긴 숫자가 입력됐다면 다시 명령어를 입력해달라고 요청한다.
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
		//명령어를 제대로 입력하라고 클라이언트에게 메시지 보내주기.
	}
}
