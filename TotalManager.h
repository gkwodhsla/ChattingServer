#pragma once

#include "Common.h"
#include <mutex>

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
	void RemoveClientSocket(int index);
	void RemoveClientSocket(const ClientInfo& Info);

public:
	std::vector<ClientInfo>& GetClientInfos() { return ClientInfos; }
	const std::vector<ClientInfo>& GetClientInfos() const { return ClientInfos; }
	std::vector<ChattingBuilding*>& GetBuildings() { return Buildings; }
	const std::vector<ChattingBuilding*>& GetBuildings() const { return Buildings; }
	std::vector<std::thread>& GetSubThreads() { return SubThreads; }
	const std::vector<std::thread>& GetSubThreads() const { return SubThreads; }

public:
	static inline std::mutex ClientInfoLock;

private:
	TotalManager();
	virtual ~TotalManager();

private:
	void ProcessingAfterSelect();
	void AcceptingNewClient();

private:
	SOCKET ListenSocket;
	std::vector<ClientInfo> ClientInfos;
	std::vector<ChattingBuilding*> Buildings;
	std::vector<std::thread> SubThreads;
	fd_set WriteSet;
	fd_set ReadSet;
};

//이 클래스는 이름 그대로 모든 자원을 관리하는 클래스입니다.
//MainLogic에서 접속하는 클라이언트들을 받아들이고, 아직 채팅방에
//참가하지 않은 클라이언트들의 명령어를 받아 CommandOutsourcer에게
//외주를 맡겨 클라이언트의 요구를 적절히 처리합니다.