#include "Server.h"

void SERVER::init(HWND hWnd)
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
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

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

    for (int i = 0; i < 2; ++i)
    {
        PlayersPosition[i].id = -1;
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
        //m_Packet = (PACKET)m_Packet;
        printf("read\n");
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
            //len = recv(wParam, (char*)&PlayersPosition, sizeof(PlayersPosition), 0);
            memcpy(PlayersPosition, m_Packet.buf, sizeof(PlayersPosition));
            if (len == SOCKET_ERROR) {
                printf("inGame error : %d\n", WSAGetLastError());
                return;
            }
            //printxmfloat4x4(PlayersPosition[0].position);
            //printxmfloat4x4(PlayersPosition[1].position);
            //printf("id : %d - x : %f y : %f z : %f\n", PlayersPosition[0].id, PlayersPosition[0].position.x, PlayersPosition[0].position.y, PlayersPosition[0].position.z);
            //printf("id : %d - x : %f y : %f z : %f\n", PlayersPosition[1].id, PlayersPosition[1].position.x, PlayersPosition[1].position.y, PlayersPosition[1].position.z);
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
    //printf("x : %f y : %f z : %f\n", P_InGame.position.x, P_InGame.position.y, P_InGame.position.z);
}

//void SERVER::UpdatePlayerPosition(const XMFLOAT4X4 &position)
//{
//    P_InGame.id = ClientNumId;
//    P_InGame.position = position;
//    //printf("x : %f y : %f z : %f\n", P_InGame.position.x, P_InGame.position.y, P_InGame.position.z);
//}

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

//void SERVER::SendPosition(const XMFLOAT4X4& position)
//{
//    if (IsCount() == false) return;
//    UpdatePlayerPosition(position);
//    printf("send position\n");
//    //len = SendMessageType(clientSocket, MESSAGETYPE::INGAME);
//
//    m_Packet.MessageType = MESSAGETYPE::INGAME;
//    m_Packet.byte = sizeof(PACKET_INGAME);
//    memcpy(m_Packet.buf, &P_InGame, sizeof(P_InGame));
//    len = send(clientSocket, (char*)&m_Packet, sizeof(m_Packet), 0);
//}

void SERVER::AddFPSCount()
{
    FPSCount++;
}

bool SERVER::IsCount()
{
    if (FPSCount >= 60)
    {
        FPSCount = 0;
        printf("IsCount\n");
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

void SERVER::printxmfloat4x4(const XMFLOAT4X4& p)
{
    std::cout << p._11 << p._12 << p._13 << p._14 << std::endl <<
        p._21 << p._22 << p._23 << p._24 << std::endl <<
        p._31 << p._32 << p._33 << p._34 << std::endl <<
        p._41 << p._42 << p._43 << p._44 << std::endl << std::endl;
}

//void SERVER::SetOtherPlayerPosition(std::vector<CGameObject*> &m_OtherPlayers)
//{
//    int myid = ClientNumId;
//    for (int i = 0; i < 2; ++i)
//    {
//        if (myid == PlayersPosition[i].id) continue;
//        int index = PlayersPosition[i].id;
//        m_OtherPlayers[index]->SetPosition(PlayersPosition[index].position);
//    }
//   
//}

//void SERVER::SetOtherPlayerPosition(std::vector<CGameObject**>& m_OtherPlayers)
//{
//    int myid = ClientNumId;
//    for (int i = 0; i < 2; ++i)
//    {
//        if (myid == PlayersPosition[i].id) continue;
//        int index = PlayersPosition[i].id;
//        m_OtherPlayers[index]->SetPosition(PlayersPosition[index].position);
//    }
//}
