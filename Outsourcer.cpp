#define _CRT_SECURE_NO_WARNINGS
#include "ChattingBuilding.h"
#include "Outsourcer.h"
#include "TotalManager.h"
#include <algorithm>
#include <cctype>
#include <iostream>


void Outsourcer::ExecutingCommand(ClientInfo& CommandRequestor, const int ClientIndex, std::string& Command)
{
	std::transform(Command.begin(), Command.end(), Command.begin(),
		[](unsigned char c) { return tolower(c); });
	//우선 입력받은 명령어를 모두 소문자로 바꾼다.

	std::vector<std::string> tokens = Tokenizing(Command);
	
	
	if (tokens[0] == "login")
	{
		std::string failMsg;
		if (CommandRequestor.IsLogin) //이미 로그인 됐다면 실패!
		{
			failMsg = "You already login!\r\n";
			CustomSend(CommandRequestor.ClientSock, failMsg.c_str(), failMsg.size(), 0, CommandRequestor);
		}
		else if (tokens.size() < CORRECT_LOGIN_TOKEN_NUM) //만약 login명령어만 입력하고 이름을 입력하지 않은 경우라면 실패!
		{
			failMsg = "Please enter the name\r\n";
			CustomSend(CommandRequestor.ClientSock, failMsg.c_str(), failMsg.size(), 0, CommandRequestor);
		}
		else
		{
			Login(CommandRequestor, tokens[1]);
		}
	}

	if (CommandRequestor.IsLogin)
	{
		if (tokens[0] == "h")
		{
			SendingCommandList(CommandRequestor);
		}
		else if (tokens[0] == "us")
		{
			SendingUserList(CommandRequestor);
		}
		else if (tokens[0] == "lt")
		{
			SendingChattingroomList(CommandRequestor);
		}
		else if (tokens[0] == "st")
		{
			bool isThereAlphabet = CheckingAlphabetInStr(tokens[1]);
			if (isThereAlphabet) //방 사이즈에 정수가 아닌 다른 값이 들어온다면 방 생성 불가!
			{
				std::string failMsg{ "\r\nRoom Index only accept numeric number(0~)\r\n" };
				CustomSend(CommandRequestor.ClientSock, failMsg.c_str(), failMsg.size(), 0, CommandRequestor);
			}
			else
			{
				SendingChattingroomInfo(CommandRequestor, std::stoul(tokens[1], nullptr, 0));
			}
		}
		else if (tokens[0] == "pf")
		{
			SendingUserInfo(CommandRequestor, tokens[1]);
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
				CustomSend(CommandRequestor.ClientSock, failMsg.c_str(), failMsg.size(), 0, CommandRequestor);
			}
			//정수가 나와야하는데 알파벳이 하나라도 나온다면 다시 명령어를 입력해달라고 요청!
			else if (std::stoi(tokens[1]) > 16 || std::stoi(tokens[1]) < 2)
			{
				std::string failMsg{ "\r\nparticipant range must (2 ~ 16)\r\n" };
				CustomSend(CommandRequestor.ClientSock, failMsg.c_str(), failMsg.size(), 0, CommandRequestor);
			}
			//만약 최대 참석인원 수를 넘긴 숫자가 입력됐다면 다시 명령어를 입력해달라고 요청!
			else
			{
				CreatingChattingroom(tokens[2], ClientIndex, std::stoi(tokens[1]), CommandRequestor);
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
	else//로그인 하지 않은 상태에서 명령어를 입력하면 로그인을 먼저 하라고 알려줍니다.
	{
		std::string failMsg;
		failMsg = "Please login first\r\n";
		CustomSend(CommandRequestor.ClientSock, failMsg.c_str(), failMsg.size(), 0, CommandRequestor);
	}
}




void Outsourcer::SendingCommandList(ClientInfo& CommandRequestor)
{
	std::pair<bool, int> sendResult = CustomSend(CommandRequestor.ClientSock, CommandList.c_str(), CommandList.size(), 0, CommandRequestor);
	std::cout << "Send command list to requesting client" << std::endl;
}

void Outsourcer::SendingUserList(ClientInfo& CommandRequestor)
{
	const std::vector<ClientInfo>& userInfos = TotalManager::Instance().GetClientInfos();

	std::string sendMsg = "\r\n";
	for (int i = 0; i < userInfos.size(); ++i)//유저 정보를 순회하며 메시지를 완성 시킵니다.
	{
		sendMsg += "Name: " + userInfos[i].Name + "\tConnected from:" + userInfos[i].ConnectionPoint + "\r\n";
	}
	CustomSend(CommandRequestor.ClientSock, sendMsg.c_str(), sendMsg.size(), 0, CommandRequestor);
	std::cout << "Send user list to requesting client" << std::endl;
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
			if (!isEmptyRoom[j])
			{
				std::string temp = "[" + std::to_string(i * ChattingBuilding::MAX_ROOM_NUM + j) + "]" +
					std::to_string(curParticipant[j]) + "/" + std::to_string(maxParticipant[j]) + " Room name: " + roomName[j] + "\r\n";
				sendMsg += temp;
			}
		}

		CustomSend(CommandRequestor.ClientSock, sendMsg.c_str(), sendMsg.size(), 0, CommandRequestor);
	}
	std::cout << "Send chattingroom list to client" << std::endl;
}

void Outsourcer::SendingChattingroomInfo(ClientInfo& CommandRequestor, unsigned int RoomIndex)
{
	std::vector<ChattingBuilding*> buildings = TotalManager::Instance().GetBuildings();
	unsigned int whichBuilding = RoomIndex / ChattingBuilding::MAX_ROOM_NUM;
	unsigned int whichRoom = RoomIndex % ChattingBuilding::MAX_ROOM_NUM;
	//RoomIndex/maxRoom -> 채팅빌딩 중 몇 번째 채팅빌딩인가?
	//RoomIndex % maxRoom -> 앞에서 정한 채팅빌딩 중 몇 번째 방인가?
	
	std::string sendMsg = "";
	if (buildings.size() * ChattingBuilding::MAX_ROOM_NUM - 1 < RoomIndex ||
		buildings[whichBuilding]->GetIsEmptyRooms()[whichRoom])
	{
		sendMsg = "Room that you asking is not exist\r\n";
		CustomSend(CommandRequestor.ClientSock, sendMsg.c_str(), sendMsg.size(), 0, CommandRequestor);
	}
	////생성된 채팅방 보다 큰 수의 RoomIndex가 들어오면 에러 메시지를 보내줍니다.
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
		//방 정보와 더불어 방에 참가하고 있는 참여자와 참여 시간까지 붙여서 보내줍니다.
		CustomSend(CommandRequestor.ClientSock, sendMsg.c_str(), sendMsg.size(), 0, CommandRequestor);
	}
}

void Outsourcer::SendingUserInfo(ClientInfo& CommandRequestor, const std::string& Name)
{
	const std::vector<ClientInfo>& userInfos = TotalManager::Instance().GetClientInfos();

	std::string sendMsg = "";
	for (int i = 0; i < userInfos.size(); ++i)
	{
		if (userInfos[i].Name == Name)
		{
			if (userInfos[i].RoomIndex == ClientInfo::LOBBY_INDEX)
			{
				sendMsg += Name + " is located in lobby\r\n";
			}
			//유저가 로비에 위치한 경우입니다.
			else
			{
				sendMsg += Name + " is located in " + std::to_string(userInfos[i].RoomIndex) + "room\r\n";
			}
			sendMsg += "Connected from: " + userInfos[i].ConnectionPoint + "\r\n";
			CustomSend(CommandRequestor.ClientSock, sendMsg.c_str(), sendMsg.size(), 0, CommandRequestor);
			return;
		}
	}

	//찾고자 하는 유저가 채팅서버에 존재하지 않는 경우 입니다.
	sendMsg += Name + " is not exist in server\r\n";
	CustomSend(CommandRequestor.ClientSock, sendMsg.c_str(), sendMsg.size(), 0, CommandRequestor);
}

void Outsourcer::Login(ClientInfo& CommandRequestor, const std::string& Name)
{
	CommandRequestor.IsLogin = true;
	CommandRequestor.Name = Name;
	std::string sendMsg = "\r\nNow you will be called [" + Name + "]\r\n" + "Check the command list using the H or h key.\r\n";
	CustomSend(CommandRequestor.ClientSock, sendMsg.c_str(), sendMsg.size(), 0, CommandRequestor);
}

void Outsourcer::SendingMail()
{
}

void Outsourcer::CreatingChattingroom(const std::string& RoomName, const int ClientIndex, const int MaxParticipant, ClientInfo& CommandRequestor)
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
				unsigned int roomIndex = buildings[i]->OccupyingRoom(RoomName, clientInfos[ClientIndex], MaxParticipant);
				//방을 차지하게 해주고(채팅방 개설), 방 개설을 요청한 클라이언트를 방에 넣어줍니다.
				//(방에 넣어주는 코드는 OccupyingRoom 함수에 존재)
				CommandRequestor.RoomIndex = i * ChattingBuilding::MAX_ROOM_NUM + roomIndex;
				isRoomExist = true;
				break;
			}
		}
		if (!isRoomExist) //만약 빈 방을 찾지 못 했다면 새로운 건물을 지어주고, 다시 빈 방을 찾도록 합니다.
		{
			buildings.emplace_back(new ChattingBuilding());
			std::vector<std::thread>& subThreads = TotalManager::Instance().GetSubThreads();
			subThreads.emplace_back(std::thread(&ChattingBuilding::ProcessingLogic, buildings.back()));
			//만약 새롭게 채팅 빌딩이 하나 세워졌다면 해당 빌딩에 속한 클라이언트의 요구(메시지 주고 받기)를
			//처리해줄 새로운 서브 쓰레드를 하나 만들어줍니다.
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
	if (buildings.size() * ChattingBuilding::MAX_ROOM_NUM - 1 < RoomIndex)
	{
		std::string failMsg = "You can't entering the room (room is not exist)\r\n";
		CustomSend(CommandRequestor.ClientSock, failMsg.c_str(), failMsg.size(), 0, CommandRequestor);
	}
	else
	{
		int maxRoom = ChattingBuilding::MAX_ROOM_NUM;
		int buildingIndex = RoomIndex / maxRoom;
		int roomIndex = RoomIndex % maxRoom;
		//RoomIndex/maxRoom -> 채팅빌딩 중 몇 번째 채팅빌딩인가?
		//RoomIndex % maxRoom -> 앞에서 정한 채팅빌딩 중 몇 번째 방인가?

		buildings[buildingIndex]->EnteringRoom(roomIndex, CommandRequestor);
		CommandRequestor.RoomIndex = RoomIndex;
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