#pragma comment(lib, "ws2_32")
//#include <winsock.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <DirectXColors.h>
#include <DirectXCollision.h>

#include <chrono>  // 시간 측정을 위한 헤더 파일
#include <iostream>
#include <thread>
#include <vector>
#include <set>
#include <map>
#include <queue>
#include <cmath>
#include <algorithm>

#include "MyMath.h"
#include "GameObject.h"

using namespace DirectX;
using namespace std;

#define SERVERPORT	9000
#define BUFSIZE		2048
#define BUF2SIZE	5500
#define WM_SOCKET	(WM_USER+1)
#define MAX_PLAYER	3
#define MAX_MONSTER 5

// 소켓 정보 저장을 위한 구조체와 변수
typedef struct socketinfo
{
	SOCKET sock;
	char buf[BUFSIZE + 1];
	int recvbytes;
	int sendbytes;
	BOOL recvdelayed;
	BOOL firestConnect;
	int id;
	socketinfo* next;
} SOCKETINFO;

enum MESSAGETYPE {
	LOGIN = 100,
	INGAME = 101,
	MONSTER_DATA = 102,
	EXIT,
	SHOT,
	HIT
};

CRITICAL_SECTION cs;
SOCKETINFO* SocketInfoList;

// 윈도우 메세지 처리 함수
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void ProcessSocketMessage(HWND, UINT, WPARAM, LPARAM);

// 소켓 관리 함수
BOOL AddSocketInfo(SOCKET sock);
SOCKETINFO* GetSocketInfo(SOCKET sock);
void RemoveSocketInfo(SOCKET sock);

// 오류 출력 함수
void err_quit(const char* msg);
void err_display(const char* msg);
void err_display(int errcode);

//패킷
typedef struct{
	XMFLOAT3 position;
	int id;
}PACKET_INGAME;

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
	int			id;
	float		LayeredAngle;
	float		LayeredMaxAngle;
	float		LayeredRoate;
	float		NEXUSHP;
	bool		IsDead;
	bool        bEnable;
	int         nAnimationSet;
	bool		GameClear;
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
	int			id;
	XMFLOAT4X4	m_xmf4x4World;
	int			HP;
	int			MAXHP;
	int			State;
	float		AnimPosition;
	XMFLOAT3	Pos;
	int			SpawnPotal;
	int			TargetID;
	int			TargetType;
}PACKET_MONSTERDATA;

typedef struct {
	int MessageType;
	int id;
}PACKET_SHOT;

typedef struct {
	int MessageType;
	int byte;
	int hit;
	char buf[BUFSIZE];
	char buf2[BUF2SIZE];
}PACKET;

PACKET_INGAME P_InGame;
PACKET_INGAME2 P_InGame2;

PACKET m_Packet;
//GameWorld
PACKET_INGAME PlayersPostion[MAX_PLAYER];

typedef struct {
	PACKET_INGAME2 PlayersPostion2[MAX_PLAYER];
	PACKET_MONSTERDATA MonsterData[MAX_MONSTER * 10];
}PACKET_GAMEDATA;

TerrainInfo terrain(XMFLOAT3(512, 0, 512), XMFLOAT3(12.0f, 1.0f, 12.0f));

PACKET_GAMEDATA GameData;
int ConnectNum = 0;
int ArrConnect[3] = { -1,-1,-1 };
HANDLE hMonsterThread;
DWORD WINAPI MonsterThread(LPVOID arg);

float NexusHP = 2500.0f;
AStar astar;
CGameObject Monsters[MAX_MONSTER * 10];
XMFLOAT3 NexusPos = XMFLOAT3(3072, 255, 3072);
XMFLOAT3 PotalPos[3] = { XMFLOAT3(3575, 255, 3065) ,XMFLOAT3(3056 , 255, 3685) ,XMFLOAT3(2297 , 255, 3043)};

void UpdateConnectNum();
bool GameStart = false;
void UpdateMonsterData();
int	 DragonKill = 0;
bool GameClear = false;

int main(int argc, char** argv)
{
	int retval;

	// 윈도우 클래스 등록
	WNDCLASS wndclass;
	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc = WndProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = NULL;
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = L"MyWndClass";
	if (!RegisterClass(&wndclass))
		return 1;

	// 윈도우 생성
	HWND hWnd = CreateWindow(L"MyWndClass", L"TCP 서버", WS_OVERLAPPEDWINDOW, 0, 0, 600, 200, NULL, NULL, NULL, NULL);
	if (hWnd == NULL)
		return 1;
	//ShowWindow(hWnd, SW_SHOWNORMAL);
	UpdateWindow(hWnd);

	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	// socket()
	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sock == INVALID_SOCKET)
		err_quit("socket()");

	// bind()
	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = bind(listen_sock, (SOCKADDR*)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR)
		err_quit("bind()");

	//네이글 알고리즘 OFF
	int DelayZeroOpt = 1;
	setsockopt(listen_sock, SOL_SOCKET, TCP_NODELAY, (const char*)&DelayZeroOpt, sizeof(DelayZeroOpt));

	// listen()
	retval = listen(listen_sock, SOMAXCONN);
	if (retval == SOCKET_ERROR)
		err_quit("listen()");

	// WSAAsyncSelect()
	retval = WSAAsyncSelect(listen_sock, hWnd, WM_SOCKET, FD_ACCEPT | FD_CLOSE);
	if (retval == SOCKET_ERROR)
		err_quit("WSAAsyncSelect()");

	for (int i = 0; i < MAX_PLAYER; ++i)
	{
		GameData.PlayersPostion2[i].id = -1;
		GameData.PlayersPostion2[i].GameClear = false;
	}
	
	XMFLOAT3 pos = XMFLOAT3(0.f, 5000.f, 0.f);
	for (int i = 0; i < MAX_MONSTER; ++i)
	{
		for (int j = 0; j < 10; ++j)
		{
			Monsters[i * 10 + j].m_Id = i * 10 + j;
			Monsters[i * 10 + j].m_State = 5;
			Monsters[i * 10 + j].m_AnimPosition = 0.0f;
			Monsters[i * 10 + j].m_HP = Monsters[j].MonsterHPs[j];
			Monsters[i * 10 + j].m_MAXHP = Monsters[j].MonsterHPs[j];
			Monsters[i * 10 + j].m_Speed = Monsters[j].MonsterSpeed[j];
			Monsters[i * 10 + j].m_PrevState = NULL;
			Monsters[i * 10 + j].m_TargetType = CGameObject::Nexus;
			Monsters[i * 10 + j].m_Type = j;
			Monsters[i * 10 + j].m_SpawnPotalNum = j % 3;
			Monsters[i * 10 + j].SetPosition(pos);
		}
	}
	// 0 10 20
	Monsters[0].m_SpawnPotalNum = 0;
	Monsters[10].m_SpawnPotalNum = 1;
	Monsters[20].m_SpawnPotalNum = 2;

	//9 19 29
	Monsters[9].m_SpawnPotalNum = 0;
	Monsters[19].m_SpawnPotalNum = 1;
	Monsters[29].m_SpawnPotalNum = 2;

	// 장애물 위치 설정.
	Obstacle* obstacle = new Obstacle[5];
	XMFLOAT3 BoundingCenter, BoundingExtent;

	/// LowerWall
	BoundingCenter = XMFLOAT3(-0.00411405414, 0.745403349, 0.00240635872);
	BoundingExtent = XMFLOAT3(0.178011268, 0.744693696, 3.52246284);
	obstacle[0].SetPosition(terrain.GetWidth(0.5f) - 81.0f, 0.0f, terrain.GetLength(0.5f) + 20.0f);
	obstacle[0].SetScale(12.0f, 12.0f, 12.0f);
	obstacle[0].Rotate(0.0f, 180.0f, 0.0f);
	obstacle[0].SetCorner(BoundingExtent, BoundingCenter, 12.0f);

	obstacle[1].SetPosition(terrain.GetWidth(0.5f) + 73.0f, 0.0f, terrain.GetLength(0.5f) + 30.0f);
	obstacle[1].SetScale(12.0f, 12.0f, 12.0f);
	obstacle[1].Rotate(0.0f, 180.0f, 0.0f);
	obstacle[1].SetCorner(BoundingExtent, BoundingCenter, 12.0f);

	/// UpperWall
	BoundingCenter = XMFLOAT3(-0.00730583817, 0.733893871, -0.191149592);
	BoundingExtent = XMFLOAT3(0.170963734, 0.749625683, 1.39590120);
	obstacle[2].SetPosition(terrain.GetWidth(0.5f) + 73.0f, 0.0f, terrain.GetLength(0.5f) - 29.0f);
	obstacle[2].SetScale(12.0f, 12.0f, 12.0f);
	obstacle[2].SetCorner(BoundingExtent, BoundingCenter, 12.0f);

	obstacle[3].SetPosition(terrain.GetWidth(0.5f) - 45.0f, 0.0f, terrain.GetLength(0.5f) + 104.0f);
	obstacle[3].SetScale(12.0f, 12.0f, 12.0f);
	obstacle[3].Rotate(0.0f, 90.0f, 0.0f);
	obstacle[3].SetCorner(BoundingExtent, BoundingCenter, 12.0f);

	obstacle[4].SetPosition(terrain.GetWidth(0.5f) + 30.0f, 0.0f, terrain.GetLength(0.5f) + 106.0f);
	obstacle[4].SetScale(12.0f, 12.0f, 12.0f);
	obstacle[4].Rotate(0.0f, -90.0f, 0.0f);
	obstacle[4].SetCorner(BoundingExtent, BoundingCenter, 12.0f);
	
	astar.SetObstacle(obstacle, 5);

	astar.path0 = astar.GetPath(PotalPos[0], NexusPos);
	astar.path1 = astar.GetPath(PotalPos[1], NexusPos);
	astar.path2 = astar.GetPath(PotalPos[2], NexusPos);

	UpdateMonsterData();

	hMonsterThread = CreateThread(NULL, 0, MonsterThread, NULL, 0, NULL);
	if (hMonsterThread != NULL) {
		CloseHandle(hMonsterThread);
	}
	// 메세지 루프
	MSG msg;
	// GetMessage()는 메세지 큐에 메세지가 존재한다면 가져와서 MSG 구조체에 그 값을 저장하고 TRUE를 반환한다. 그런데 만약 읽은 메세지가 WM_QUIT이면 FALSE를 리턴한다.
	while (GetMessage(&msg, nullptr, 0, 0) > 0)
	{
		// TranslateMessage()은 GetMessage()으로부터 전달받은 메시지가 WM_KEYDOWN 인지, 눌려진 키가 문자키인지 검사해보고 조건이 맞을 경우 WM_CHAR 메시지를 만들어 message queue에 덧붙이는 역할을 한다. 문자 입력이 아닐 경우는 아무 일도 하지 않는다.
		// 키보드 드라이버에 의해 ASCII 문자로 매핑된 키에 대해서만 WM_CHAR 메시지를 생성한다.
		TranslateMessage(&msg);
		// message queue에 덧붙여진 메시지는 DispatchMessage()에 의해 WndProc()으로 전달된다.
		DispatchMessage(&msg);
	}
	
	// 윈속 종료
	WSACleanup();
	return msg.wParam;
}

// 윈도우 메세지 처리
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_SOCKET:	// 소켓 관련 윈도우 메세지
		ProcessSocketMessage(hWnd, uMsg, wParam, lParam);
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

// 소켓 관련 윈도우 메세지 처리
void ProcessSocketMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// 데이터 통신에 사용할 변수
	SOCKETINFO* ptr;
	SOCKET client_sock;
	SOCKADDR_IN clientaddr;
	int addrlen, retval;

	// 오류 발생 여부 확인
	if (WSAGETSELECTERROR(lParam))
	{
		err_display(WSAGETSELECTERROR(lParam));
		RemoveSocketInfo(wParam);
		return;
	}

	// 메세지 처리
	switch (WSAGETSELECTEVENT(lParam))
	{
	case FD_ACCEPT:
		addrlen = sizeof(clientaddr);
		client_sock = accept(wParam, (SOCKADDR*)&clientaddr, &addrlen);
		if (client_sock == INVALID_SOCKET)
		{
			err_display("accept()");
			return;
		}
		printf("\n[TCP 서버] 클라이언트 접속: IP 주소=%s, 포트 번호=%d\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
		AddSocketInfo(client_sock);
		retval = WSAAsyncSelect(client_sock, hWnd, WM_SOCKET, FD_READ | FD_WRITE | FD_CLOSE);
		if (retval == SOCKET_ERROR)
		{
			err_display("WSAAsyncSelect()");
			RemoveSocketInfo(client_sock);
		}
		break;
	case FD_READ:
	{
		ptr = GetSocketInfo(wParam);
		//if (ptr->recvbytes > 0)
		//{
		//	ptr->recvbytes = true;
		//	return;
		//}

		retval = recv(ptr->sock, (char*)&m_Packet, sizeof(m_Packet), 0);
		if (retval == SOCKET_ERROR) {
			printf("read error : %d\n", WSAGetLastError());
			return;
		}
		//ptr->recvbytes = retval;

		switch (m_Packet.MessageType)
		{
		case MESSAGETYPE::LOGIN:
			printf("send LOGIN : %d\n", (int)m_Packet.buf);
			break;
		case MESSAGETYPE::INGAME:
		{
			////////////////////////////////////////////////	PACKET 2
			PACKET_INGAME2 pInGame;
			memcpy(&pInGame, m_Packet.buf, sizeof(PACKET_INGAME2));
			GameData.PlayersPostion2[pInGame.id].id = pInGame.id;
			GameData.PlayersPostion2[pInGame.id].position = pInGame.position;
			GameData.PlayersPostion2[pInGame.id].velocity = pInGame.velocity;
			GameData.PlayersPostion2[pInGame.id].pitch = pInGame.pitch;
			GameData.PlayersPostion2[pInGame.id].yaw = pInGame.yaw;
			GameData.PlayersPostion2[pInGame.id].roll = pInGame.roll;
			GameData.PlayersPostion2[pInGame.id].xmf4x4World = pInGame.xmf4x4World;
			GameData.PlayersPostion2[pInGame.id].IsDash = pInGame.IsDash;
			GameData.PlayersPostion2[pInGame.id].GunType = pInGame.GunType;
			GameData.PlayersPostion2[pInGame.id].AniKeyFrame = pInGame.AniKeyFrame;
			GameData.PlayersPostion2[pInGame.id].fLength = pInGame.fLength;
			GameData.PlayersPostion2[pInGame.id].InnResultAnimBundle = pInGame.InnResultAnimBundle;
			GameData.PlayersPostion2[pInGame.id].UpVector = pInGame.UpVector;
			GameData.PlayersPostion2[pInGame.id].RightVector = pInGame.RightVector;
			GameData.PlayersPostion2[pInGame.id].LookVector = pInGame.LookVector;
			GameData.PlayersPostion2[pInGame.id].AnicurrentTrack = pInGame.AnicurrentTrack;
			GameData.PlayersPostion2[pInGame.id].xmf4x4Transform = pInGame.xmf4x4Transform;
			GameData.PlayersPostion2[pInGame.id].Fire = pInGame.Fire;
			GameData.PlayersPostion2[pInGame.id].RayDirection = pInGame.RayDirection;
			GameData.PlayersPostion2[pInGame.id].LayeredAngle = pInGame.LayeredAngle;
			GameData.PlayersPostion2[pInGame.id].LayeredMaxAngle = pInGame.LayeredMaxAngle;
			GameData.PlayersPostion2[pInGame.id].LayeredRoate = pInGame.LayeredRoate;
			GameData.PlayersPostion2[pInGame.id].NEXUSHP = NexusHP;
			GameData.PlayersPostion2[pInGame.id].IsDead = pInGame.IsDead;
			GameData.PlayersPostion2[pInGame.id].bEnable = pInGame.bEnable;
			GameData.PlayersPostion2[pInGame.id].nAnimationSet = pInGame.nAnimationSet;
			GameData.PlayersPostion2[pInGame.id].GameClear = GameClear;

			//retval = send(ptr->sock, (char*)&ptr->messageType, sizeof(ptr->messageType), 0);
			//모든 플레이어 PACKET_INGAME정보를 모든 플레이어에게 전송
			m_Packet.MessageType = MESSAGETYPE::INGAME;
			m_Packet.byte = sizeof(GameData);
			memcpy(m_Packet.buf, &GameData.PlayersPostion2 , sizeof(GameData.PlayersPostion2)); //플레이어 데이터
			memcpy(m_Packet.buf2, &GameData.MonsterData , sizeof(GameData.MonsterData)); //몬스터 데이터
			//printf("%d\n%d\n", sizeof(GameData.PlayersPostion2), sizeof(GameData.MonsterData));
			////////////////////////////////////////////////	PACKET 2

			//if (ptr->recvbytes <= ptr->sendbytes)
			//	return;

			retval = send(ptr->sock, (char*)&m_Packet, sizeof(m_Packet), 0);
			if (retval == SOCKET_ERROR) {
				printf("send error : %d\n", WSAGetLastError());
				//RemoveSocketInfo(wParam);
				return;
			}
			//ptr->sendbytes += retval;
			//if (ptr->recvbytes == ptr->sendbytes)
			//{
			//	ptr->recvbytes = ptr->sendbytes = 0;
			//	if (ptr->recvdelayed)
			//	{
			//		ptr->recvdelayed = false;
			//		PostMessage(hWnd, WM_SOCKET, wParam, FD_READ);
			//	}
			//}
			break;
		}
		case MESSAGETYPE::EXIT:
		{
			printf("exit\n");
			int id = m_Packet.byte;
			if (ArrConnect[id] == 1)
			{
				ArrConnect[id] = -1;
			}
			printf("EXIT : %d %d %d\n", ArrConnect[0], ArrConnect[1], ArrConnect[2]);
			UpdateConnectNum();
			RemoveSocketInfo(wParam);
			break;
		}
		case MESSAGETYPE::SHOT:
		{
			SOCKETINFO* ptrlist = SocketInfoList;

			while (ptrlist)
			{
				if (ptrlist->sock != ptr->sock)
				{
					//m_Packet.MessageType = MESSAGETYPE::TEST;
					PACKET_SHOT pShot;
					pShot.MessageType = MESSAGETYPE::SHOT;
					pShot.id = ptr->id;
					retval = send(ptrlist->sock, (char*)&pShot, sizeof(PACKET_SHOT), 0);
				}
				ptrlist = ptrlist->next;
			}
			break;
		}
		case MESSAGETYPE::HIT:
		{
			//printf("Pid = %d,Mid = %d,dmg = %d\n",ptr->id, m_Packet.byte, m_Packet.hit);
			int monsterid = m_Packet.byte;
			int dmg = m_Packet.hit;
			Monsters[monsterid].m_HP -= dmg;
			
			if (Monsters[monsterid].m_HP <= 0 && Monsters[monsterid].m_State != CGameObject::DIE)
			{
				Monsters[monsterid].m_State = CGameObject::DIE;
				Monsters[monsterid].m_AnimPosition = 0.0f;

				if (Monsters[monsterid].m_Type == CGameObject::Dragon)
				{
					DragonKill++;
					if (DragonKill >= 3 && GameClear == false) // 드래곤 3마리 다 잡으면 게임 종료
					{
						GameClear = true;
					}
				}
			}
			break;
		}
		default:
			break;
		}
		if (retval == SOCKET_ERROR)
		{
			return;
		}
		break;
	}
	case FD_WRITE:
	{
		//처음 접속시 호출
		printf("FD_WRITE\n");
		ptr = GetSocketInfo(wParam);
		m_Packet.MessageType = MESSAGETYPE::LOGIN;
		m_Packet.byte = sizeof(int);
		// 접속중인 id 검색
		for (int i = 0; i < 3; ++i)
		{
			if (ArrConnect[i] == -1) //i번째 미접속
			{
				ArrConnect[i] = 1;
				itoa(i, m_Packet.buf, 10);
				GameData.PlayersPostion2[i].id = i;
				ptr->id = i;
				retval = send(ptr->sock, (char*)&m_Packet, sizeof(m_Packet), 0);
				UpdateConnectNum();
				break;
			}
		}
		//printf("Connect : %d %d %d\n", ArrConnect[0], ArrConnect[1], ArrConnect[2]);
		//GameData.PlayersPostion2[ConnectNum].id = ConnectNum;
		//retval = send(ptr->sock, (char*)&m_Packet, sizeof(m_Packet), 0);
		//ConnectNum++;

		if (retval == SOCKET_ERROR)
		{
			err_display("send()");
			RemoveSocketInfo(wParam);
			return;
		}
		break;
	}
	case FD_CLOSE:
	{
		RemoveSocketInfo(wParam);
		break;
	}
	}
}

//현재 접속중인 클라 개수 갱신
void UpdateConnectNum()
{
	int temp = 0;
	for (int i = 0; i < 3; ++i)
	{
		if (ArrConnect[i] == 1)
		{
			temp++;
		}
	}

	ConnectNum = temp;
	if (ConnectNum == 3)
	{
		GameStart = true;
	}
	printf("현재 접속중인 클라 수 - %d\n", ConnectNum);
}

// 소켓 정보 추가
BOOL AddSocketInfo(SOCKET sock)
{
	SOCKETINFO* ptr = new SOCKETINFO;
	if (ptr == NULL)
	{
		printf("[오류] 메모리가 부족합니다!\n");
		return FALSE;
	}

	ptr->sock = sock;
	ptr->recvbytes = 0;
	ptr->sendbytes = 0;
	ptr->recvdelayed = FALSE;
	ptr->next = SocketInfoList;
	ptr->firestConnect = FALSE;
	ptr->id = -1;
	SocketInfoList = ptr;

	return TRUE;
}

// 소켓 정보 얻기
SOCKETINFO* GetSocketInfo(SOCKET sock)
{
	SOCKETINFO* ptr = SocketInfoList;

	while (ptr)
	{
		if (ptr->sock == sock)
			return ptr;
		ptr = ptr->next;
	}

	return NULL;
}

// 소켓 정보 제거
void RemoveSocketInfo(SOCKET sock)
{
	SOCKADDR_IN clientaddr;
	int addrlen = sizeof(clientaddr);
	getpeername(sock, (SOCKADDR*)&clientaddr, &addrlen);
	printf("[TCP 서버] 클라이언트 종료: IP 주소=%s, 포트 번호=%d\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));

	SOCKETINFO* curr = SocketInfoList;
	SOCKETINFO* prev = NULL;

	while (curr)
	{
		if (curr->sock == sock)
		{
			if (prev)
				prev->next = curr->next;
			else
				SocketInfoList = curr->next;
			closesocket(curr->sock);
			delete curr;
			return;
		}
		prev = curr;
		curr = curr->next;
	}
}

// 소켓 함수 오류 출력 후 종료
void err_quit(const char* msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL
	);
	MessageBox(NULL, (LPCTSTR)&lpMsgBuf, (LPCWSTR)msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
	exit(1);
}

// 소켓 함수 오류 출력
void err_display(const char* msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL
	);
	printf("[%s] %s\n", msg, (char*)lpMsgBuf);
	LocalFree(lpMsgBuf);
}

// 소켓 함수 오류 출력
void err_display(int errcode)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, errcode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL
	);
	printf("[오류] %s\n", (char*)lpMsgBuf);
	LocalFree(lpMsgBuf);
}

void SpawnMonster();
void WaveSpawnMonster();
void MonstersUpdate(double Elapsedtime);
void WaveSpawnMonster_TYPE_N(int Type,int n);
int dist(XMFLOAT3& v1, XMFLOAT3& v2);
int SpawnCount = 0;
int WaveLevel = 1; //일정 시간에 따른 난이도 상승

//플레이어 위치 값 보기
void PrintPlayerPosition()
{
	for (int i = 0; i < MAX_PLAYER; ++i)
	{
		if (GameData.PlayersPostion2[i].id != -1)
		{
			printf("P%d X : %f, Y : %f, Z : %f\n", i, GameData.PlayersPostion2[i].position.x,
				GameData.PlayersPostion2[i].position.y,
				GameData.PlayersPostion2[i].position.z);
		}
	}
}

DWORD WINAPI MonsterThread(LPVOID arg)
{
	const std::chrono::duration<double> frame_rate = std::chrono::duration<double>(1.0 / 60.0);
	
	// 이전 프레임의 시간
	auto prev_time = std::chrono::high_resolution_clock::now();
	
	double SpwanCoolTime = 0.0f;
	double SpawnTime = 1.5f;
	
	double WaveCoolTime = 0.0f;
	double WaveTime = 15.0f;

	while (true) {
		auto start_time = std::chrono::high_resolution_clock::now();  // 현재 시간 기록
		
		// 프레임 간의 시간 간격 계산
		auto elapsed_time = std::chrono::duration_cast<std::chrono::duration<double>>(start_time - prev_time);
		prev_time = start_time;
		std::this_thread::sleep_for(frame_rate - elapsed_time);

		if (GameStart)
		{
			SpwanCoolTime += elapsed_time.count();
			
			if (SpwanCoolTime >= SpawnTime)
			{
				SpwanCoolTime = 0.0f;
				SpawnMonster();
			}

			WaveCoolTime += elapsed_time.count();
			//25초마다 추가 웨이브
			if (WaveCoolTime >= WaveTime)
			{
				WaveCoolTime = 0.0f;
				WaveLevel++;
				WaveSpawnMonster();
				printf("현재 웨이브 레벨 : %d\n", WaveLevel);
			}
			//dt
			MonstersUpdate(elapsed_time.count());

			//PrintPlayerPosition();
		}
	}
	return 0;
}
bool CheckSpawnMonsterType(int waveLevel, int type)
{
	if (WaveLevel == 1)
	{
		// 기본 등장 몹 - Bee, Spider, Bat, KingCobra, Golem, 
		if (type == CGameObject::Giant_Bee || type == CGameObject::Spider || type == CGameObject::Bat
			|| type == CGameObject::KingCobra || type == CGameObject::Golem)
		{
			return true;
		}
	}
	else if (WaveLevel == 3) // + TreasureChest
	{
		if (type == CGameObject::Giant_Bee || type == CGameObject::Spider || type == CGameObject::Bat
			|| type == CGameObject::KingCobra || type == CGameObject::Golem || type == CGameObject::TreasureChest)
		{
			return true;
		}
	}
	else if (WaveLevel == 5) // + Magama
	{
		if (type == CGameObject::Giant_Bee || type == CGameObject::Spider || type == CGameObject::Bat
			|| type == CGameObject::KingCobra || type == CGameObject::Golem || type == CGameObject::TreasureChest
			|| type == CGameObject::Magama)
		{
			return true;
		}
	}
	else if (WaveLevel >= 7) // + Treant
	{
		if (type == CGameObject::Giant_Bee || type == CGameObject::Spider || type == CGameObject::Bat
			|| type == CGameObject::KingCobra || type == CGameObject::Golem || type == CGameObject::TreasureChest
			|| type == CGameObject::Magama || type == CGameObject::Treant)
		{
			return true;
		}
	}
	return false;
}

void SpawnMonster()
{
	if (SpawnCount > MAX_MONSTER * 10) return;
	
	// printf("spwn : %d\n", SpawnCount++);
	// enum { Dragon, Giant_Bee, Golem, KingCobra, TreasureChest, Spider, Bat, Magama, Treant, Wolf };
	// 30초마다 wavelevel +1
	// 기본 등장 몹 - Bee, Spider, Bat, KingCobra, Golem, 
	// 30초마다 추가로 등장하는 몹 - TreasureChest, Magama , Treant
	// 특수 몹 - Wolf
	// 아마도 보스 - Dragon

	for (int i = 0; i < MAX_MONSTER * 10; ++i)
	{
		if (CheckSpawnMonsterType(WaveLevel, Monsters[i].m_Type))
		{
			if (Monsters[i].m_State == CGameObject::NONE)
			{
				Monsters[i].m_PrevState = GameData.MonsterData[i].State;
				Monsters[i].m_State = CGameObject::IDLE;
				Monsters[i].SetPosition(PotalPos[Monsters[i].m_SpawnPotalNum]);
				break;
			}
		}
	}
}

void WaveSpawnMonster_TYPE_N(int Type,int n)
{
	int Count = 1;
	for (int i = 0; i < MAX_MONSTER * 10; ++i)
	{
		if (Count <= 3)
		{
			if (Monsters[i].m_Type == Type)
			{
				if (Monsters[i].m_State != CGameObject::NONE)
				{
					Count++;
				}

				if (Monsters[i].m_State == CGameObject::NONE)
				{
					Monsters[i].m_PrevState = GameData.MonsterData[i].State;
					Monsters[i].m_State = CGameObject::IDLE;
					Count++;
				}
			}
		}
	}
}

void WaveSpawnMonster()
{
	//30초마다 몬스터 추가 출현
	if (WaveLevel == 3 || WaveLevel == 7)
	{
		WaveSpawnMonster_TYPE_N(CGameObject::Wolf,3);
	}
	else if (WaveLevel == 10)
	{
		//웨이브 레벨 10때 드래곤 출현
		WaveSpawnMonster_TYPE_N(CGameObject::Dragon, 3);
	}
}

void UpdateMonsterPath(int i, int targetType, int state, float radiusOfAction, XMFLOAT3& target)
{
	XMFLOAT3&& pos = Monsters[i].GetPosition();
	Monsters[i].m_TargetType = targetType;

	if (!Monsters[i].m_path.empty())
	{
		Monsters[i].m_TargetPos = Monsters[i].m_path.front();
		Monsters[i].m_TargetPos.y = 255.0f;

		if (Monsters[i].m_path.size() == 1 && dist(target, pos) <= radiusOfAction)
		{
			Monsters[i].m_path.erase(Monsters[i].m_path.begin());
			Monsters[i].m_AnimPosition = 0.0f;
			Monsters[i].m_PrevState = GameData.MonsterData[i].State;
			Monsters[i].m_State = state;
		}
		else if (dist(Monsters[i].m_TargetPos, pos) < 0.01f)
		{
			Monsters[i].m_path.erase(Monsters[i].m_path.begin());
		}
	}
}

void MonstersUpdate(double Elapsedtime)
{	
	for (int i = 0; i < MAX_MONSTER * 10; ++i)
	{
		Monsters[i].m_AnimPosition += Elapsedtime;
		if (Monsters[i].m_AnimPosition > 1.0f) Monsters[i].m_AnimPosition = 0.0f;

		switch (Monsters[i].m_State)
		{
		case CGameObject::DIE:
		{
			Monsters[i].m_SpawnCoolTime += Elapsedtime;
			if (Monsters[i].m_SpawnCoolTime >= 0.6)
			{
				Monsters[i].m_SpawnCoolTime = 0.0;
				Monsters[i].m_State = CGameObject::NONE;
				Monsters[i].m_HP = Monsters[i].m_MAXHP;
				Monsters[i].m_AnimPosition = 0.0f;
				Monsters[i].m_Speed = Monsters[i].MonsterSpeed[i];
				Monsters[i].m_TargetType = CGameObject::Nexus;
				Monsters[i].SetPosition(0.0f,5000.0f,0.0f);
			}
			break;
		}
		case CGameObject::IDLE: //스폰
		{
			Monsters[i].m_SpawnToMoveDelay += Elapsedtime;
			if (Monsters[i].m_SpawnToMoveDelay >= 1.0f)
			{
				Monsters[i].m_SpawnToMoveDelay = 0.0f;
				Monsters[i].m_PrevState = GameData.MonsterData[i].State;
				Monsters[i].SetPosition(PotalPos[Monsters[i].m_SpawnPotalNum]);
				float temp = 1.0f + WaveLevel * 0.005f;
				Monsters[i].m_MAXHP = Monsters[i].m_MAXHP * temp;
				Monsters[i].m_HP = Monsters[i].m_MAXHP;

				Monsters[i].m_path = astar.GetStartPath(Monsters[i].m_SpawnPotalNum);

				Monsters[i].m_State = CGameObject::MOVE;
			}
			break;
		}
		case CGameObject::MOVE:
		{
			if (Monsters[i].m_Type == CGameObject::Wolf)
			{
				XMFLOAT3 pos = Monsters[i].GetPosition();
				XMFLOAT3 pPos = GameData.PlayersPostion2[Monsters[i].m_TargetId].position;

				// 경로 재설정.
				Monsters[i].m_ResetPathTime += Elapsedtime;
				if (Monsters[i].m_ResetPathTime < 1.0f)
				{
					float MaxDistance = FLT_MAX;
					int playerID = -1;
					// 모든 플레이어의 위치를 확인하여 가장 가까운 플레이어 탐색
					for (int j = 0; j < MAX_PLAYER; ++j)
					{
						if (GameData.PlayersPostion2[j].id != -1 && GameData.PlayersPostion2[j].IsDead == false)
						{
							XMFLOAT3 pPos = GameData.PlayersPostion2[j].position;
							float distance = dist(pPos, pos);
							if (distance < MaxDistance)
							{
								MaxDistance = distance;
								playerID = j;
							}
						}
					}

					Monsters[i].m_TargetId = playerID;
					Monsters[i].m_ResetPathTime = 0.0f;
					Monsters[i].m_path = astar.GetPath(Monsters[i].GetPosition(), pPos);
				}

				UpdateMonsterPath(i, CGameObject::Player, CGameObject::ATTACK, 22.0f, pPos);

				Monsters[i].MoveForward(METER_PER_PIXEL(Monsters[i].m_Speed) * Elapsedtime);
				Monsters[i].SetLookAt(Monsters[i].m_TargetPos);
				break;
			}
			else
			{
				XMFLOAT3 pos = Monsters[i].GetPosition();
				// 적의 어그로를 초기화 및 어그로 최소 거리 설정
				float closestDistance = 40.0f;
				int closestPlayerId = -1;

				// 모든 플레이어의 위치를 확인하여 가장 가까운 플레이어 탐색
				for (int j = 0; j < MAX_PLAYER; ++j)
				{
					if (GameData.PlayersPostion2[j].id != -1 && GameData.PlayersPostion2[j].IsDead == false)
					{
						XMFLOAT3 pPos = GameData.PlayersPostion2[j].position;
						float distance = dist(pPos, pos);
						if (distance < closestDistance)
						{
							closestDistance = distance;
							closestPlayerId = j;
						}
					}
				}

				// 가장 가까운 플레이어를 어그로 대상으로 설정
				if (closestPlayerId != -1)
				{
					XMFLOAT3 pPos = GameData.PlayersPostion2[closestPlayerId].position;
					//Monsters[i].m_TargetType = CGameObject::Player;
					//Monsters[i].m_TargetPos = pPos;

					Monsters[i].m_ResetPathTime += Elapsedtime;
					if (Monsters[i].m_ResetPathTime < 1.0f)
					{
						Monsters[i].m_ResetPathTime = 0.0f;
						Monsters[i].m_path = astar.GetPath(Monsters[i].GetPosition(), pPos);
					}

					Monsters[i].m_TargetId = closestPlayerId;
					UpdateMonsterPath(i, CGameObject::Player, CGameObject::ATTACK, 22.0f, pPos);
				}
				else
				{
					// 경로 재설정.
					if (Monsters[i].m_TargetType != CGameObject::Nexus)
					{
						Monsters[i].m_path = astar.GetPath(Monsters[i].GetPosition(), NexusPos);
					}

					UpdateMonsterPath(i, CGameObject::Nexus, CGameObject::ATTACK, 50.0f, NexusPos);
				}

				Monsters[i].MoveForward(METER_PER_PIXEL(Monsters[i].m_Speed) * Elapsedtime);
				Monsters[i].SetLookAt(Monsters[i].m_TargetPos);
				break;
			}
		}
		case CGameObject::ATTACK:
		{
			XMFLOAT3 pos = Monsters[i].GetPosition();

			if (Monsters[i].m_TargetType == CGameObject::Player)
			{
				XMFLOAT3 pPos = GameData.PlayersPostion2[Monsters[i].m_TargetId].position;
				if (dist(pPos, pos) > 30.0f)
				{
					Monsters[i].m_AnimPosition = 0.0f;
					Monsters[i].m_PrevState = GameData.MonsterData[i].State;
					Monsters[i].m_State = CGameObject::MOVE;
					Monsters[i].m_TargetType = CGameObject::Nexus;
				}
				if (GameData.PlayersPostion2[Monsters[i].m_TargetId].IsDead)
				{
					Monsters[i].m_AnimPosition = 0.0f;
					Monsters[i].m_PrevState = GameData.MonsterData[i].State;
					Monsters[i].m_State = CGameObject::MOVE;
				}
			}
			else if (Monsters[i].m_TargetType == CGameObject::Nexus)
			{
				Monsters[i].m_AttackCoolTime += Elapsedtime;
				if (Monsters[i].m_AttackCoolTime >= 0.9f) // 공격 주기
				{
					//넥서스 체력 감소
					Monsters[i].m_AttackCoolTime = 0.0f;
					NexusHP -= 7.0f;
					//printf("%f\n", NexusHP);
				}
			}
			break;
		}
		default:
			break;
		}
	}
	UpdateMonsterData();
}

bool IsNan(const XMFLOAT4X4& xmf4x4world)
{
	if (std::isnan(xmf4x4world._11) || std::isnan(xmf4x4world._12) || std::isnan(xmf4x4world._13)
		|| std::isnan(xmf4x4world._21) || std::isnan(xmf4x4world._22) || std::isnan(xmf4x4world._23)
		|| std::isnan(xmf4x4world._31) || std::isnan(xmf4x4world._32) || std::isnan(xmf4x4world._33)
		|| std::isnan(xmf4x4world._41) || std::isnan(xmf4x4world._42) || std::isnan(xmf4x4world._43))
		return true;

	return false;
}

void UpdateMonsterData()
{
	for (int i = 0; i < MAX_MONSTER * 10; ++i)
	{
		GameData.MonsterData[i].id = Monsters[i].m_Id;
		GameData.MonsterData[i].HP = Monsters[i].m_HP;
		GameData.MonsterData[i].MAXHP = Monsters[i].m_MAXHP;
		GameData.MonsterData[i].State = Monsters[i].m_State;
		GameData.MonsterData[i].AnimPosition = Monsters[i].m_AnimPosition;
		GameData.MonsterData[i].Pos = Monsters[i].GetPosition();
		GameData.MonsterData[i].SpawnPotal = Monsters[i].m_SpawnPotalNum;
		GameData.MonsterData[i].TargetID = Monsters[i].m_TargetId;
		GameData.MonsterData[i].TargetType = Monsters[i].m_TargetType;
		if (IsNan(Monsters[i].m_xmf4x4World) != true) 
			GameData.MonsterData[i].m_xmf4x4World = Monsters[i].m_xmf4x4World;
		else
			GameData.MonsterData[i].m_xmf4x4World = Matrix4x4::Identity();
	}
}

int dist(XMFLOAT3& v1, XMFLOAT3& v2)
{
	int len = 0;
	XMFLOAT3 sub = Vector3::Subtract(v1, v2);
	len = sqrt(pow(sub.x, 2) + pow(sub.y, 2) + pow(sub.z, 2));
	return len;
}