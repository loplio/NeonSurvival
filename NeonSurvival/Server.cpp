#include "stdafx.h"
#include "Server.h"
#include "Player.h"

void SERVER::init(HWND hWnd, char* IP)
{
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        printf("WSAStartup failed with error: %d\n", result);
        return;
    }

    clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSocket == INVALID_SOCKET) {
        printf("Socket creation failed with error: %ld\n", WSAGetLastError());
        WSACleanup();
        return;
    }

    struct sockaddr_in serverAddr;
    ZeroMemory(&serverAddr, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVERPORT);
    //serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    //serverAddr.sin_addr.s_addr = inet_addr("125.180.24.40");
    //serverAddr.sin_addr.s_addr = inet_addr("119.67.181.24");
    serverAddr.sin_addr.s_addr = inet_addr(IP);
    //serverAddr.sin_addr.s_addr = inet_addr("125.180.29.106");

    //네이클 알고리즘 OFF
    int DelayZeroOpt = 1;
    setsockopt(clientSocket, SOL_SOCKET, TCP_NODELAY, (const char*)&DelayZeroOpt, sizeof(DelayZeroOpt));

    result = WSAAsyncSelect(clientSocket, hWnd, WM_SOCKET, FD_CONNECT | FD_READ | FD_WRITE);
    if (result == SOCKET_ERROR) {
        printf("WSAAsyncSelect failed with error: %d\n", WSAGetLastError());
        closesocket(clientSocket);
        WSACleanup();
        return;
    }

    result = connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
    if (result == SOCKET_ERROR && WSAGetLastError() != WSAEWOULDBLOCK) {
        printf("Connect failed with error: %d\n", WSAGetLastError());
        closesocket(clientSocket);
        WSACleanup();
        return;
    }

    for (int i = 0; i < MAX_PLAYER; ++i)
    {
        PlayersPosition[i].id = -1;
        PlayersPosition2[i].id = -1;
    }
    
    for (int i = 0; i < MAX_MONSTER * 10; ++i)
    {
        MonsterData[i].HP = 1;
    }
}

void SERVER::ProcessSocketMessage(HWND hWnd, UINT unit, WPARAM wParam, LPARAM lParam)
{
    if (WSAGETSELECTERROR(lParam)) {
        printf("Socket error: %d\n", WSAGETSELECTERROR(lParam));
        return;
    }

    switch (WSAGETSELECTEVENT(lParam)) {
    case FD_CONNECT: {
        printf("Connected to server.\n");
        break;
    }
    case FD_READ: {
        //len = recv(wParam, (char*)&MessageType, sizeof(MessageType), 0);
        len = recv(wParam, (char*)&m_Packet, sizeof(m_Packet), 0);
        if (len == SOCKET_ERROR) {
            printf("read error : %d\n", WSAGetLastError());
            return;
        }

        switch (m_Packet.MessageType)
        {
        case MESSAGETYPE::LOGIN:
        {
            if (len == SOCKET_ERROR) {
                printf("login error : %d\n", WSAGetLastError());
                return;
            }
            printf("ClientNum : %d\n", atoi(m_Packet.buf));
            ClientNumId = atoi(m_Packet.buf);
            FirstConnect = true;
            break;
        }
        case MESSAGETYPE::INGAME:
        {
            memcpy(PlayersPosition2, &m_Packet.buf, sizeof(PlayersPosition2));
            if (len == SOCKET_ERROR) {
                printf("inGame error : %d\n", WSAGetLastError());
                return;
            }
            memcpy(MonsterData, &m_Packet.buf2, sizeof(MonsterData));
            if (len == SOCKET_ERROR) {
                printf("inGame error : %d\n", WSAGetLastError());
                return;
            }
            break;
        }
        case MESSAGETYPE::MONSTER_DATA:
        {
            std::cout << "Monster Data" << std::endl;
            memcpy(MonsterData, m_Packet.buf2, sizeof(MonsterData));

            if (len == SOCKET_ERROR) {
                printf("MONSTER_DATA error : %d\n", WSAGetLastError());
                return;
            }
            break;
        }
        case MESSAGETYPE::SHOT:
        {
            ShotClinetId = m_Packet.byte;
            break;
        }
        default:
            break;
        }
        break;
    }
    case FD_WRITE: {
        break;
    }
    }
}

int SERVER::SendMessageType(SOCKET& socket, MESSAGETYPE type)
{
    int byte = 0;
    byte = send(socket, (char*)&type, sizeof(int), 0);
    return byte;
}

void SERVER::UpdatePlayerPosition(const XMFLOAT3 &position)
{
    P_InGame.id = ClientNumId;
    P_InGame.position = position;
}

void SERVER::SendPosition(const XMFLOAT3& position)
{
    //if (IsCount() == false) return;
    UpdatePlayerPosition(position);
    printf("send position\n");
    //len = SendMessageType(clientSocket, MESSAGETYPE::INGAME);

    m_Packet.MessageType = MESSAGETYPE::INGAME;
    m_Packet.byte = sizeof(PACKET_INGAME);
    memcpy(m_Packet.buf, &P_InGame, sizeof(P_InGame));
    len = send(clientSocket, (char*)&m_Packet, sizeof(m_Packet), 0);
}

void SERVER::AddFPSCount(float dt)
{
    //FPSCount++;
    FPS += dt;
    //printf("%d\n", FPSCount);
}

bool SERVER::IsCount()
{
    /*if (FPSCount >= 6)
    {
        FPSCount = 0;
        printf("IsCount\n");
        return true;
    }
    */
    if (FPS >= 0.1f)
    {
        FPS = 0.0f;

        return true;
    }
    
    return false;
}

void SERVER::err_quit(const char* msg)
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
void SERVER::err_display(const char* msg)
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
void SERVER::err_display(int errcode)
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

PACKET_INGAME* SERVER::GetPlayersPosition()
{
    return PlayersPosition;
}

void SERVER::SendPlayerData(CPlayer& player,int GunType, float flength, int anibundle)
{
    // ###
    P_InGame2.id = ClientNumId;
    P_InGame2.position = player.GetPosition();
    P_InGame2.velocity = player.GetVelocity();
    P_InGame2.pitch = player.GetPitch();
    P_InGame2.yaw = player.GetYaw();
    P_InGame2.roll = player.GetRoll();
    P_InGame2.xmf4x4World = player.m_xmf4x4World;
    P_InGame2.GunType = GunType;
    P_InGame2.IsDash = player.IsDash;
    P_InGame2.fLength = flength;
    P_InGame2.InnResultAnimBundle = anibundle;
    P_InGame2.UpVector = player.GetUpVector();
    P_InGame2.RightVector = player.GetRightVector();
    P_InGame2.LookVector = player.GetLookVector();
    P_InGame2.AnicurrentTrack = player.m_pSkinnedAnimationController->m_nCurrentTrack;
    P_InGame2.AniKeyFrame = player.m_pSkinnedAnimationController->m_pAnimationTracks[P_InGame2.AnicurrentTrack].m_fPosition;
    P_InGame2.xmf4x4Transform = player.m_xmf4x4Transform;
    P_InGame2.Fire = player.GetFire();
    P_InGame2.RayDirection = player.GetRayDirection();
    P_InGame2.LayeredAngle = player.m_pSkinnedAnimationController->m_LayeredAngle;
    P_InGame2.LayeredMaxAngle = player.m_pSkinnedAnimationController->m_LayeredMaxAngle;
    P_InGame2.LayeredRoate = player.m_pSkinnedAnimationController->m_LayeredRotate;
    P_InGame2.IsDead = player.GetDead();
    //P_InGame2.player = player;

    m_Packet.MessageType = MESSAGETYPE::INGAME;
    m_Packet.byte = sizeof(PACKET_INGAME2);
    memcpy(m_Packet.buf, &P_InGame2, sizeof(P_InGame2));
    len = send(clientSocket, (char*)&m_Packet, sizeof(m_Packet), 0);
}

void SERVER::printxmfloat4x4(const XMFLOAT4X4& p)
{
    //std::cout << p._11 << p._12 << p._13 << p._14 << std::endl <<
    //    p._21 << p._22 << p._23 << p._24 << std::endl <<
    //    p._31 << p._32 << p._33 << p._34 << std::endl <<
    //    p._41 << p._42 << p._43 << p._44 << std::endl << std::endl;
}

void SERVER::SendShot()
{
    int msg = MESSAGETYPE::SHOT;
    len = send(clientSocket, (char*)&msg, sizeof(msg), 0);
}

void SERVER::SendExit()
{
    PACKET_EXIT pExit = { MESSAGETYPE::EXIT ,ClientNumId };
    len = send(clientSocket, (char*)&pExit, sizeof(PACKET_EXIT), 0);
}

void SERVER::SendHit(int monsterId,int dmg)
{
    PACKET_HIT pHit;
    pHit.MessageType = MESSAGETYPE::HIT;
    pHit.id = monsterId;
    pHit.dmg = dmg;
    len = send(clientSocket, (char*)&pHit, sizeof(PACKET_HIT), 0);
}