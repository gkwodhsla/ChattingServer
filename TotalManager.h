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
	void CreateRoom();
	void DestroyRoom();

public:
	const std::vector<ClientInfo>& GetClientInfos() const { return ClientInfos; }
	const std::vector<ChattingBuilding*>& GetRooms() const { return Buildings; }

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