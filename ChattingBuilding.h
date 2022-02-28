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
	void ProcessingLogic(); //TotalManageró�� select, send, recv �� �����Ѵ�.
	void PrintAllRoomName();
	bool IsThereAnyEmptyRoom(); //���� �� ���� �ִٸ� true�� �ƴ϶�� false�� ��ȯ�Ѵ�.
	void OccupyingRoom(const std::string& RoomName, ClientInfo& Client, const int MaximumParticipant); //���� �����ϰ�, �̸��� �ο��Ѵ�.
	void EnteringRoom(const int RoomIndex, ClientInfo& Client); //�ش� ������ ���� ��Ų��.
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