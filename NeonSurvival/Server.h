#pragma once
#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <iostream>

#include <DirectXMath.h>
#include <vector>

#include "Player.h"

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

typedef struct {
	XMFLOAT3	position;
	float		pitch;
	float		yaw;
	float		roll;
	XMFLOAT3	velocity;
	XMFLOAT4X4	xmf4x4World;
	XMFLOAT4X4	xmf4x4Transform;
	bool		IsDash;
	int			GunType;
	float		AniKeyFrame;
	float		fLength;
	int			InnResultAnimBundle;
	XMFLOAT3	UpVector;
	XMFLOAT3	RightVector;
	XMFLOAT3	LookVector;
	int			AnicurrentTrack;
	int id;
} PACKET_INGAME2;

typedef struct {
	int MessageType;
	int byte;
	char buf[BUFSIZE];
}PACKET;

class SERVER {
private:
	SERVER() {}
	SERVER(const SERVER& ref) {}
	SERVER& operator=(const SERVER& ref) {}
	~SERVER() {}

	PACKET m_Packet;
	int ClientNumId = 0;
	int MessageType = 0;
	int SendByte = 0;
	int RecvByte = 0;
	int FPSCount = 0;
	int len = 0;
	bool FirstConnect = false;
	bool RecvDelayed = false;
	SOCKET clientSocket;
	PACKET_INGAME P_InGame;
	PACKET_INGAME PlayersPosition[2];

	PACKET_INGAME2 P_InGame2;
	PACKET_INGAME2 PlayersPosition2[2];
	
public:
	static SERVER& getInstance() {
		static SERVER s;
		return s;
	}
	void init(HWND);
	void ProcessSocketMessage(HWND, UINT, WPARAM, LPARAM);
	int SendMessageType(SOCKET& socket, MESSAGETYPE type);
	void UpdatePlayerPosition(const XMFLOAT3 &position);
	//void UpdatePlayerPosition(const XMFLOAT4X4 &position);
	void SendPosition(const XMFLOAT3& position);
	void SendPlayerData(CPlayer& player,int GunType,float flength,int anibundle);

	//void SendPosition(const XMFLOAT4X4& woldpos);
	void AddFPSCount();
	bool IsCount();

	int GetClientNumId() { return ClientNumId; }
	PACKET_INGAME* GetPlayersPosition();
	PACKET_INGAME2* GetPlayersPosition2() { return PlayersPosition2; }
	//void SetOtherPlayerPosition(std::vector<CGameObject*> &m_OtherPlayers);
	//void SetOtherPlayerPosition(std::vector<CGameObject**>& m_OtherPlayers);

	void err_quit(const char* msg);
	void err_display(const char* msg);
	void err_display(int errcode);
	void printxmfloat4x4(const XMFLOAT4X4& p);
};
