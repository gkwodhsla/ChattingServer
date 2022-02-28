#pragma once
#pragma comment(lib, "ws2_32")

#include "Common.h"

class Outsourcer;
class ChattingBuilding;

class TotalManager final
{
public:
	static TotalManager& Instance()
	{
		static TotalManager instance;
		return instance;
	}
	TotalManager(const TotalManager&) = delete;
	TotalManager& operator=(const TotalManager&) = delete;

public:
	void InitServer();
	void MainLogic();
	void MarkingForRemoveClntSocket(SOCKET SocketDiscriptor);

public:
	std::vector<ClientInfo>& GetClientInfos() { return ClientInfos; }
	const std::vector<ClientInfo>& GetClientInfos() const { return ClientInfos; }
	std::vector<ChattingBuilding*>& GetBuildings() { return Buildings; }
	const std::vector<ChattingBuilding*>& GetBuildings() const { return Buildings; }
	std::vector<std::thread>& GetSubThreads() { return SubThreads; }
	const std::vector<std::thread>& GetSubThreads() const { return SubThreads; }

private:
	TotalManager();
	virtual ~TotalManager();

private:
	void ProcessingAfterSelect();
	void RemoveClntSocket(int index);

private:
	SOCKET ListenSocket;
	std::vector<ClientInfo> ClientInfos;
	std::vector<ChattingBuilding*> Buildings;
	std::vector<std::thread> SubThreads;
	Outsourcer* CommandOutsourcer = nullptr;
	fd_set WriteSet;
	fd_set ReadSet;
};

//이 클래스는 이름 그대로 모든 자원을 관리하는 클래스이다.
//MainLogic에서 접속하는 클라이언트들을 받아들이고, 아직 채팅방에
//참가하지 않은 클라이언트들의 명령어를 받아 CommandOutsourcer에게
//외주를 맡겨 명령어를 적절히 처리한다.