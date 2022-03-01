#pragma once
#include "Common.h"
#include <atomic>
#include <mutex>

class ChattingBuilding final
{
public:
	ChattingBuilding();
	ChattingBuilding(const ChattingBuilding&) = delete;
	ChattingBuilding& operator=(const ChattingBuilding&) = delete;
	virtual ~ChattingBuilding();

public:
	void ProcessingLogic(); //�̰����� select, recv�� �����Ѵ�.
	bool IsThereAnyEmptyRoom(); //���� �� ���� �ִٸ� true�� �ƴ϶�� false�� ��ȯ�Ѵ�.
	unsigned int OccupyingRoom(const std::string& RoomName, ClientInfo& Requestor, const int MaximumParticipant); //���� �����ϰ�, �̸��� �ο��Ѵ�.
	void EnteringRoom(const int RoomIndex, ClientInfo& Client); //�ش� ������ ���� ��Ų��.
	void StopChattingroomLogic() { ShouldLogicStop = true; }

private:
	void ProcessingAfterSelect();
	void RemoveClntSocket(int RoomNumber, int Index);

public:
	static const unsigned int MAX_ROOM_NUM = 4;
	static const unsigned int MAX_PARTICIPANT_EACH_ROOM = 16;
	static const int SOCKET_TIME_WAIT_US = 100;

public:
	const std::array<std::string, MAX_ROOM_NUM>& GetRoomNames() const { return RoomNames; }
	const std::array<std::string, MAX_ROOM_NUM>& GetRoomCreatingTimes() const { return RoomCreatingTimes; }
	const std::array<unsigned int, MAX_ROOM_NUM>& GetMaximumParticipants() const { return MaximumParticipants; }
	const std::array<unsigned int, MAX_ROOM_NUM>& GetCurParticipantInRooms() const { return CurParticipantInRooms; }
	const std::array<bool, MAX_ROOM_NUM>& GetIsEmptyRooms() const { return IsEmptyRooms; }
	const std::vector<std::string> GetUsersNameAndEnteringTime(unsigned int RoomIndex)const;

private:
	std::vector<ClientInfo> ClientInfosEachRoom[MAX_ROOM_NUM];
	//ClientInfosEachRoom[i][j] -> i��° �濡�� ä������ j��° Ŭ���̾�Ʈ
	std::array<std::string, MAX_ROOM_NUM> RoomNames;
	std::array<std::string, MAX_ROOM_NUM> RoomCreatingTimes;
	std::array<unsigned int, MAX_ROOM_NUM> MaximumParticipants;
	//����ڰ� ���� �ִ� �����ο� ��
	std::array<unsigned int, MAX_ROOM_NUM> CurParticipantInRooms;
	std::array<bool, MAX_ROOM_NUM> IsEmptyRooms;
	fd_set ReadSet, WriteSet;
	std::atomic<bool> ShouldLogicStop;
	std::mutex lock;
	//MainThread���� �ش� �ǹ��� Thread�� �ߴܽ�Ű�� ���� �� �� �÷��׸� true�� ������ش�.
};

//�� Ŭ������ �� 4���� ä�ù�� ������ ä�ù濡 ������ Ŭ���̾�Ʈ�� �����մϴ�.
//�� 4���� ���̸� �ϳ��� �ǹ��� �ȴٰ� ������ �̸��� ChattingBuilding�̶�� �������ϴ�.
//select ���� �ִ� 64���� ������ ó���� �� �ִٰ� �ؼ� �ִ� �� 4��, �� �渶�� �ִ�
//���� �ο��� 16������ �������׽��ϴ�.
