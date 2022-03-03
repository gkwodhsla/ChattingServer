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
	void ProcessingLogic();
	//�̰����� select,send, recv�� �����մϴ�. TotalManager�� MainLogic�� �ϴ� ���� �����մϴ�.
	//���� �����尡 �� �Լ��� �����մϴ�.
public:
	bool IsThereAnyEmptyRoom(); //���� �� �ǹ��� �� ���� �ִٸ� true�� �ƴ϶�� false�� ��ȯ�մϴ�.
	unsigned int OccupyingRoom(const std::string& RoomName, ClientInfo& Requestor, const int MaximumParticipant);
	//���� �����ϰ�, �̸��� �ο��մϴ�.
	void EnteringRoom(const int RoomIndex, ClientInfo& Client); //�ش� ������ ���� ��ŵ�ϴ�.
	void StopChattingroomLogic() { ShouldLogicStop = true; }
	//�� �Լ����� ���ξ����忡�� �����ϴ� �Լ��Դϴ�. ���꾲����� ������� �ʽ��ϴ�.	

public:
	static const unsigned int MAX_ROOM_NUM = 4;
	static const unsigned int MAX_PARTICIPANT_EACH_ROOM = 16;

private:
	void ProcessingAfterSelect();
	void RemoveClientSocket(int RoomNumber, int Index);
	void ResetRoomInfo(unsigned int roomIndex);
	//�� ���� �� �ش� ���� ������ ��� �ʱ� ���·� ���� �����ݴϴ�.

public:
	const std::array<std::string, MAX_ROOM_NUM>& GetRoomNames() const { return RoomNames; }
	const std::array<std::string, MAX_ROOM_NUM>& GetRoomCreatingTimes() const { return RoomCreatingTimes; }
	const std::array<unsigned int, MAX_ROOM_NUM>& GetMaximumParticipants() const { return MaximumParticipants; }
	const std::array<unsigned int, MAX_ROOM_NUM>& GetCurParticipantInRooms() const { return CurParticipantInRooms; }
	const std::array<bool, MAX_ROOM_NUM>& GetIsEmptyRooms() const { return IsEmptyRooms; }
	const std::vector<std::string> GetUsersNameAndEnteringTime(unsigned int RoomIndex)const;
	//const ���� �ܿ��� �ʿ��� �� ���� �ʾ� const ������ ��������ϴ�.

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
	// �� �÷��װ� false�� �Ǹ� ���꾲���� ������ �ߴܽ�ŵ�ϴ�.
	std::mutex lock;
	//���ξ����忡�� ���꾲������ �����͸� �ǵ帱 �� ���Ǵ� lock�Դϴ�.
	//�� �����ϱ� ���� ��� ���ξ����忡�� ���꾲������ ClientInfosEachRoom�� �ǵ帮��
	//������ �� lock�� ���˴ϴ�.
};

//�� Ŭ������ �� 4���� ä�ù�� ������ ä�ù濡 ������ Ŭ���̾�Ʈ�� �����մϴ�.
//�� 4���� ���̸� �ϳ��� �ǹ��� �ȴٰ� ������ �̸��� ChattingBuilding�̶�� �������ϴ�.
//select ���� �ִ� 64���� ������ ó���� �� �ִٰ� �ؼ� �ִ� �� 4��, �� �渶�� �ִ�
//���� �ο��� 16������ �������׽��ϴ�.
