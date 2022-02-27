#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib, "ws2_32")

#include <array>
#include <string>
#include <thread>
#include <vector>
#include <WinSock2.h>

struct ClientInfo
{
	ClientInfo() = delete;
	virtual ~ClientInfo() = default;
	ClientInfo(SOCKET SocketDiscriptor) :ClntSock(SocketDiscriptor){}
	SOCKET ClntSock;
	std::array<char, 1024> Buffer;
	std::string Name;
	bool IsSend = false; //이 플래그가 true일 때만 메시지를 클라이언트에게 보냅니다.

public:
	inline static int MAX_BUFFER_SIZE = 1024;

}typedef ClientInfo;

//class ChattingRoom;

class TotalManager final
{
public:
	TotalManager();
	TotalManager(const TotalManager&) = delete;
	TotalManager& operator=(const TotalManager&) = delete;
	virtual ~TotalManager();

public:
	void InitServer();
	void MainLogic();
	void CreateRoom();
	void DestroyRoom();

public:
	void PrintAllClientName();
	void PrintAllRoomInfo();

private:
	void ProcessingAfterSelect();
	void RemoveClntSocket(int index);
//public:
//	inline static const unsigned int MAX_ENTER = 64;
private:
	SOCKET ListenSocket;
	std::vector<ClientInfo> ClientInfos;
//	std::vector<ChattingRoom> Rooms;
	std::vector<std::thread> SubThreads;
	fd_set WriteSet;
	fd_set ReadSet;
};

