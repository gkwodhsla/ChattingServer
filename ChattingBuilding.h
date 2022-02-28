#pragma once
#include "Common.h"
#include <atomic>

class ChattingBuilding
{
public:
	ChattingBuilding();
	ChattingBuilding(const ChattingBuilding&) = delete;
	ChattingBuilding& operator=(const ChattingBuilding&) = delete;
	virtual ~ChattingBuilding();

public:
	void ProcessingLogic(); //TotalManager처럼 select, send, recv 를 수행한다.
	void PrintAllRoomName();
	bool IsThereAnyEmptyRoom(); //만약 빈 방이 있다면 true를 아니라면 false를 반환한다.
	void OccupyingRoom(const std::string& RoomName, ClientInfo& Client, const int MaximumParticipant); //방을 차지하고, 이름을 부여한다.
	void EnteringRoom(const int RoomIndex, ClientInfo& Client); //해당 방으로 참석 시킨다.
	void StopChattingroomLogic() { ShouldLogicStop = true; }

private:
	void ProcessingAfterSelect();
	void RemoveClntSocket(int RoomNumber, int Index);

public:
	static const unsigned int MAX_ROOM_NUM = 4;
	static const unsigned int MAX_PARTICIPANT_EACH_ROOM = 16;

private:
	std::vector<ClientInfo> ClientInfosEachRoom[MAX_ROOM_NUM];
	std::string RoomNames[MAX_ROOM_NUM];
	int MaximumParticipants[MAX_ROOM_NUM] = {0, 0, 0, 0};
	bool IsEmptyRoom[MAX_ROOM_NUM] = {true, true, true, true};
	fd_set ReadSet, WriteSet;
	std::atomic<bool> ShouldLogicStop = false;
};