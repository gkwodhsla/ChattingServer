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
	void ProcessingLogic(); //이곳에서 select, recv를 수행한다.
	bool IsThereAnyEmptyRoom(); //만약 빈 방이 있다면 true를 아니라면 false를 반환한다.
	unsigned int OccupyingRoom(const std::string& RoomName, ClientInfo& Requestor, const int MaximumParticipant); //방을 차지하고, 이름을 부여한다.
	void EnteringRoom(const int RoomIndex, ClientInfo& Client); //해당 방으로 참석 시킨다.
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
	//ClientInfosEachRoom[i][j] -> i번째 방에서 채팅중인 j번째 클라이언트
	std::array<std::string, MAX_ROOM_NUM> RoomNames;
	std::array<std::string, MAX_ROOM_NUM> RoomCreatingTimes;
	std::array<unsigned int, MAX_ROOM_NUM> MaximumParticipants;
	//사용자가 정한 최대 접속인원 수
	std::array<unsigned int, MAX_ROOM_NUM> CurParticipantInRooms;
	std::array<bool, MAX_ROOM_NUM> IsEmptyRooms;
	fd_set ReadSet, WriteSet;
	std::atomic<bool> ShouldLogicStop;
	std::mutex lock;
	//MainThread에서 해당 건물의 Thread를 중단시키고 싶을 때 이 플래그를 true로 만들어준다.
};

//이 클래스는 총 4개의 채팅방과 각각의 채팅방에 접속한 클라이언트를 관리합니다.
//방 4개가 모이면 하나의 건물이 된다고 생각해 이름을 ChattingBuilding이라고 지었습니다.
//select 모델이 최대 64개의 소켓을 처리할 수 있다고 해서 최대 방 4개, 각 방마다 최대
//접속 인원을 16명으로 고정시켰습니다.
