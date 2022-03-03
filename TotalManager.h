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

//�� Ŭ������ �̸� �״�� ��� �ڿ��� �����ϴ� Ŭ�����Դϴ�.
//MainLogic���� �����ϴ� Ŭ���̾�Ʈ���� �޾Ƶ��̰�, ���� ä�ù濡
//�������� ���� Ŭ���̾�Ʈ���� ��ɾ �޾� CommandOutsourcer����
//���ָ� �ð� Ŭ���̾�Ʈ�� �䱸�� ������ ó���մϴ�.