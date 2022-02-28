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
	void OccupyingRoom(const std::string& RoomName, ClientInfo& Requestor, const int MaximumParticipant); //���� �����ϰ�, �̸��� �ο��Ѵ�.
	void EnteringRoom(const int RoomIndex, ClientInfo& Client); //�ش� ������ ���� ��Ų��.
	void StopChattingroomLogic() { ShouldLogicStop = true; }

private:
	void ProcessingAfterSelect();
	void RemoveClntSocket(int RoomNumber, int Index);

public:
	static const unsigned int MAX_ROOM_NUM = 4;
	static const unsigned int MAX_PARTICIPANT_EACH_ROOM = 16;
	static const int SOCKET_TIME_WAIT_MS = 100;

private:
	std::vector<ClientInfo> ClientInfosEachRoom[MAX_ROOM_NUM];
	//ClientInfosEachRoom[i][j] -> i��° �濡�� ä������ j��° Ŭ���̾�Ʈ
	std::string RoomNames[MAX_ROOM_NUM];
	int MaximumParticipants[MAX_ROOM_NUM];
	//����ڰ� ���� �ִ� �����ο� ��
	bool IsEmptyRoom[MAX_ROOM_NUM];
	fd_set ReadSet, WriteSet;
	std::atomic<bool> ShouldLogicStop;
	std::mutex lock;
	//MainThread���� �ش� �ǹ��� Thread�� �ߴܽ�Ű�� ���� �� �� �÷��׸� true�� ������ش�.
};

//�� Ŭ������ �� 4���� ä�ù�� ������ ä�ù濡 ������ Ŭ���̾�Ʈ�� �����մϴ�.
//�� 4���� ���̸� �ϳ��� �ǹ��� �ȴٰ� ������ �̸��� ChattingBuilding�̶�� �������ϴ�.
//select ���� �ִ� 64���� ������ ó���� �� �ִٰ� �ؼ� �ִ� �� 4��, �� �渶�� �ִ�
//���� �ο��� 16������ �������׽��ϴ�.
