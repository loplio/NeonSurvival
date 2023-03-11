#pragma once
#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>

#include <DirectXMath.h>
using namespace DirectX;

#define SERVERPORT	9000
#define BUFSIZE		512
#define WM_SOCKET	(WM_USER+1)

enum MESSAGETYPE{
	LOGIN = 100,
	INGAME,
};

typedef struct{
	XMFLOAT3 position;
	int id;
} PACKET_INGAME;

class SERVER {
private:
	SERVER() {}
	SERVER(const SERVER& ref) {}
	SERVER& operator=(const SERVER& ref) {}
	~SERVER() {}

	char buf[BUFSIZE];
	int ClientNumId = 0;
	bool FirstConnect = false;
	int MessageType = 0;
	PACKET_INGAME P_InGame;
	PACKET_INGAME PlayersPosition[2];
public:
	static SERVER& getIncetance() {
		static SERVER s;
		return s;
	}

	void init(HWND);
	void ProcessSocketMessage(HWND, UINT, WPARAM, LPARAM);
	void SendMessageType(SOCKET socket,int type);
	void UpdatePlayerPosition(const XMFLOAT3 &position);
};
