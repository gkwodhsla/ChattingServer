#pragma once
#include "Common.h"

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
	void OccupyingRoom(const std::string& RoomName); //���� �����ϰ�, �̸��� �ο��Ѵ�.
	bool IsThereAnyEmptyRoom(); //���� �� ���� �ִٸ� true�� �ƴ϶�� false�� ��ȯ�Ѵ�.
	void EnteringRoom(const int RoomIndex, const ClientInfo& Client); //�ش� ������ ���� ��Ų��.

private:
	void ProcessingAfterSelect();

private:
	static const unsigned int MAX_ROOM_NUM = 4;
	static const unsigned int MAX_PARTICIPANT_EACH_ROOM = 16;

private:
	std::vector<ClientInfo> ClientInfosEachRoom[MAX_ROOM_NUM];
	std::string RoomNames[MAX_ROOM_NUM];
	bool IsEmptyRoom[MAX_ROOM_NUM] = {true, };
	fd_set ReadSet, WriteSet;
};