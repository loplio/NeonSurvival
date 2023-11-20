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
#define BUFSIZE		2048
#define BUF2SIZE	4000
#define WM_SOCKET	(WM_USER+1)
#define MAX_PLAYER	3
#define MAX_MONSTER 3

enum MESSAGETYPE{
	LOGIN = 100,
	INGAME = 101,
	MONSTER_DATA,
	EXIT,
	SHOT,
};

enum MONSTER_STATE {
	NONE = 199,
	SPAWN = 200,
	MOVE = 201,
	ATTACK = 202,
	DIE = 203,
	TargetPlayer = 204,
	TargetNexus,

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
	bool		Fire;
	XMFLOAT3	RayDirection;
	int id;
} PACKET_INGAME2;

typedef struct {
	int			TargetType;
	XMFLOAT3	TargetPos;
	int			HP;
	int			MAXHP;
	int			Id;
	int			State;
	int			SpawnPotalNum;
	float		Speed;
	double		SpawnToMoveDelay;
	int			PrevState;
}PACKET_MONSTER_DATA;

typedef struct {
	XMFLOAT4X4	m_xmf4x4World;
	int			HP;
	int			MAXHP;
	int			State;
	XMFLOAT3	Pos;
	int			SpawnPotal;
}PACKET_MONSTERDATA;

typedef struct {
	int MessageType;
	int byte;
	char buf[BUFSIZE];
	char buf2[BUF2SIZE];
}PACKET;

typedef struct {
	PACKET_INGAME2 PlayersPostion2[MAX_PLAYER];
	PACKET_MONSTERDATA MonsterData[MAX_MONSTER];
}PACKET_GAMEDATA;

class SERVER {
private:
	SERVER() {}
	SERVER(const SERVER& ref) {}
	SERVER& operator=(const SERVER& ref) {}
	~SERVER() {}

	PACKET m_Packet;
	int ClientNumId = -1;
	int MessageType = 0;
	int SendByte = 0;
	int RecvByte = 0;
	int FPSCount = 0;
	int len = 0;
	float FPS = 0.0f;
	bool FirstConnect = false;
	bool RecvDelayed = false;
	SOCKET clientSocket;
	PACKET_INGAME P_InGame;
	PACKET_INGAME PlayersPosition[MAX_PLAYER];

	PACKET_INGAME2 P_InGame2;
	PACKET_INGAME2 PlayersPosition2[MAX_PLAYER];
	
	PACKET_MONSTERDATA MonsterData[30];
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
	void SendShot();
	//void SendPosition(const XMFLOAT4X4& woldpos);
	void AddFPSCount(float dt);
	bool IsCount();

	int GetClientNumId() { return ClientNumId; }
	PACKET_INGAME* GetPlayersPosition();
	PACKET_INGAME2* GetPlayersPosition2() { return PlayersPosition2; }
	PACKET_MONSTERDATA* GetMonsterData() { return MonsterData; }
	//void SetOtherPlayerPosition(std::vector<CGameObject*> &m_OtherPlayers);
	//void SetOtherPlayerPosition(std::vector<CGameObject**>& m_OtherPlayers);

	void err_quit(const char* msg);
	void err_display(const char* msg);
	void err_display(int errcode);
	void printxmfloat4x4(const XMFLOAT4X4& p);
};
