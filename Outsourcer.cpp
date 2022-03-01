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
	std::cout << "Send command list to requesting client" << std::endl;
}

void Outsourcer::SendingUserList()
{
	std::cout << "���� ����Ʈ�� Ŭ���̾�Ʈ���� ���� ����" << std::endl;
}

void Outsourcer::SendingChattingroomList(ClientInfo& CommandRequestor)
{
	std::vector<ChattingBuilding*>& buildings = TotalManager::Instance().GetBuildings();
	
	for (int i = 0; i < buildings.size(); ++i)
	{
		const std::array<bool, ChattingBuilding::MAX_ROOM_NUM> isEmptyRoom = buildings[i]->GetIsEmptyRooms();
		const std::array<unsigned int, ChattingBuilding::MAX_ROOM_NUM> curParticipant = buildings[i]->GetCurParticipantInRooms();
		const std::array<unsigned int, ChattingBuilding::MAX_ROOM_NUM> maxParticipant = buildings[i]->GetMaximumParticipants();
		const std::array<std::string, ChattingBuilding::MAX_ROOM_NUM> roomName = buildings[i]->GetRoomNames();
		std::string sendMsg = "";
		for (int j = 0; j < ChattingBuilding::MAX_ROOM_NUM; ++j)
		{
			if(!isEmptyRoom[j])
			{
				std::string temp = "[" + std::to_string(i * ChattingBuilding::MAX_ROOM_NUM + j) + "]" +
					std::to_string(curParticipant[j]) + "/" + std::to_string(maxParticipant[j]) +" Room name: " + roomName[j] + "\r\n";
				sendMsg += temp;
			}
		}

		CustomSend(CommandRequestor.ClntSock, sendMsg.c_str(), sendMsg.size(), 0, CommandRequestor);
	}
	std::cout << "Send chattingroom list to client" << std::endl;
}

void Outsourcer::SendingChattingroomInfo(ClientInfo& CommandRequestor, unsigned int RoomIndex)
{
	std::vector<ChattingBuilding*> buildings = TotalManager::Instance().GetBuildings();
	unsigned int whichBuilding = RoomIndex / ChattingBuilding::MAX_ROOM_NUM;
	unsigned int whichRoom = RoomIndex % ChattingBuilding::MAX_ROOM_NUM;

	std::string sendMsg = "";
	if (buildings.size() * ChattingBuilding::MAX_ROOM_NUM - 1 < RoomIndex ||
		buildings[whichBuilding]->GetIsEmptyRooms()[whichRoom])
	{
		sendMsg = "Room that you asking is not exist\r\n";
		CustomSend(CommandRequestor.ClntSock, sendMsg.c_str(), sendMsg.size(), 0, CommandRequestor);
	}
	//���� �������� �ʴ� ���� ������ �����Ϸ��� ��� ����ó��
	else
	{
		const std::array<bool, ChattingBuilding::MAX_ROOM_NUM> isEmptyRoom = buildings[whichBuilding]->GetIsEmptyRooms();
		const std::array<unsigned int, ChattingBuilding::MAX_ROOM_NUM> curParticipant = buildings[whichBuilding]->GetCurParticipantInRooms();
		const std::array<unsigned int, ChattingBuilding::MAX_ROOM_NUM> maxParticipant = buildings[whichBuilding]->GetMaximumParticipants();
		const std::array<std::string, ChattingBuilding::MAX_ROOM_NUM> roomName = buildings[whichBuilding]->GetRoomNames();
		const std::array<std::string, ChattingBuilding::MAX_ROOM_NUM> creatingTime = buildings[whichBuilding]->GetRoomCreatingTimes();

		sendMsg = "[" + std::to_string(RoomIndex) + "]" + std::to_string(curParticipant[whichRoom])
			+ "/" + std::to_string(maxParticipant[whichRoom]) + " Room name: " + 
			roomName[whichRoom] + " Creating time: " + creatingTime[whichRoom] + "\r\n";
		
		const std::vector<std::string> userInfos = buildings[whichBuilding]->GetUsersNameAndEnteringTime(RoomIndex);
		for (int i = 0; i < userInfos.size(); ++i)
		{
			sendMsg += userInfos[i];
		}
		//���⿡ �߰������� �����ڿ� ���� �ð����� �����־�� �Ѵ�.
		CustomSend(CommandRequestor.ClntSock, sendMsg.c_str(), sendMsg.size(), 0, CommandRequestor);
	}
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
			if (buildings[i]->IsThereAnyEmptyRoom())//�ش� �ǹ��� �� ���� �ִٸ�
			{
				buildings[i]->OccupyingRoom(RoomName, clientInfos[ClntIndex], MaxParticipant);
				//���� �����ϰ� ���ְ�(ä�ù� ����), �� ������ ��û�� Ŭ���̾�Ʈ�� �濡 �־��ش�.
				//(�濡 �־��ִ� �ڵ�� OccupyingRoom �Լ��� ����)
				isRoomExist = true;
				break;
			}
		}
		if (!isRoomExist) //���� �� ���� ã�� �� �ߴٸ� ���ο� �ǹ��� �����ְ�, �ٽ� �� ���� ã���� �Ѵ�.
		{
			buildings.emplace_back(new ChattingBuilding());
			std::vector<std::thread>& subThreads = TotalManager::Instance().GetSubThreads();
			subThreads.emplace_back(std::thread(&ChattingBuilding::ProcessingLogic, buildings.back()));
			//���� ���Ӱ� ä�� ������ �ϳ� �������ٸ� �ش� ������ ���� Ŭ���̾�Ʈ�� �䱸(�޽��� �ְ� �ޱ�)��
			//ó������ ���ο� �����带 �ϳ� �����.
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
		//RoomIndex/maxRoom -> ä�ú��� �� �� ��° ä�ú����ΰ�?
		//RoomIndex % maxRoom -> �տ��� ���� ä�ú��� �� �� ��° ���ΰ�?

		buildings[buildingIndex]->EnteringRoom(roomIndex, CommandRequestor);
	}
}

void Outsourcer::DisconnectingClient()
{
}

bool Outsourcer::CheckingAlphabetInStr(const std::string& Str)
{
	bool isThereAlphabet = false;
	for (int i = 0; i < Str.size(); ++i)
	{
		if (isalpha(Str[i]))
		{
			isThereAlphabet = true;
			break;
		}
	}
	
	return isThereAlphabet;
}

void Outsourcer::ExecutingCommand(ClientInfo& CommandRequestor, const int ClntIndex, std::string& Command)
{
	std::transform(Command.begin(), Command.end(), Command.begin(),
		[](unsigned char c) { return tolower(c); });
	//�켱 �Է¹��� ��ɾ ��� �ҹ��ڷ� �ٲ۴�.

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
		bool isThereAlphabet = CheckingAlphabetInStr(tokens[1]);
		if (isThereAlphabet)
		{
			std::string failMsg{ "\r\nRoom Index only accept numeric number(0~)\r\n" };
			CustomSend(CommandRequestor.ClntSock, failMsg.c_str(), failMsg.size(), 0, CommandRequestor);
		}
		else
		{
			SendingChattingroomInfo(CommandRequestor, std::stoul(tokens[1], nullptr, 0));
		}
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
		bool isThereAlphabet = CheckingAlphabetInStr(tokens[1]);
		if (isThereAlphabet)
		{
			std::string failMsg{ "\r\nParticipant count only accept numeric number\r\n" };
			CustomSend(CommandRequestor.ClntSock, failMsg.c_str(), failMsg.size(), 0, CommandRequestor);
		}
		//���ڰ� ���;��ϴµ� ���ĺ��� �ϳ��� ���´ٸ� �ٽ� ��ɾ �Է��ش޶�� ��û�Ѵ�.
		else if (std::stoi(tokens[1]) > 16 || std::stoi(tokens[1]) < 2)
		{
			std::string failMsg{ "\r\nparticipant range must (2 ~ 16)\r\n" };
			CustomSend(CommandRequestor.ClntSock, failMsg.c_str(), failMsg.size(), 0, CommandRequestor);
		}
		//���� �ִ� �����ο� ���� �ѱ� ���ڰ� �Էµƴٸ� �ٽ� ��ɾ �Է��ش޶�� ��û�Ѵ�.
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
		//��ɾ ����� �Է��϶�� Ŭ���̾�Ʈ���� �޽��� �����ֱ�.
	}
}
