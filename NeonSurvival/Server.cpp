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
      /*  if (RecvByte > 0)
        {
            RecvDelayed = true;
            return;
        }*/
        int len = 0;
        len = recv(wParam, (char*)&MessageType, sizeof(MessageType), 0);
        printf("read\n");
        switch (MessageType)
        {
        case MESSAGETYPE::LOGIN:
        {
            len = recv(wParam, (char*)&ClientNumId, sizeof(int), 0);
            printf("ClientNum : %d\n", ClientNumId);
            FirstConnect = true;
            break;
        }
        case MESSAGETYPE::INGAME:
        {
            len = recv(wParam, (char*)&PlayersPosition, sizeof(PlayersPosition), 0);
            printf("id : %d - x : %f y : %f z : %f\n", PlayersPosition[0].id, PlayersPosition[0].position.x, PlayersPosition[0].position.y, PlayersPosition[0].position.z);
            printf("id : %d - x : %f y : %f z : %f\n", PlayersPosition[1].id, PlayersPosition[1].position.x, PlayersPosition[1].position.y, PlayersPosition[1].position.z);
            RecvByte = 16;
            break;
        }
        default:
            break;
        }
        printf("recv byte : %d\n", RecvByte);
        //RecvByte += len;
        break;
    }
    case FD_WRITE: {
        //if (RecvByte <= SendByte) return;
        printf("write\n");
        int len = 0;
        if (FirstConnect == false)
        {
           /* int type = MESSAGETYPE::LOGIN;
            int len = send(wParam, (char*)&type, sizeof(type), 0);*/
            len = SendMessageType(wParam, MESSAGETYPE::LOGIN);
        }
        else
        {
            //int type = MESSAGETYPE::LOGIN;
            //int len = send(wParam, (char*)&type, sizeof(P_InGame), 0);
            printf("position\n");
            len = SendMessageType(wParam, MESSAGETYPE::INGAME);
            len = send(wParam, (char*)&P_InGame, sizeof(PACKET_INGAME), 0);

        }
        //PostMessage(hWnd, WM_SOCKET, wParam, FD_READ);
        /*SendByte += len;
        printf("sedn byte : %d\n", SendByte);
        if (RecvByte == SendByte)
        {
            RecvByte = SendByte = 0;
            printf("sedn byte : 0000\n");
            if (RecvDelayed)
            {
                RecvDelayed = false;
                PostMessage(hWnd, WM_SOCKET, wParam, FD_READ);
            }
        }*/
        break;
    }

    }
}

int SERVER::SendMessageType(SOCKET socket,int type)
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

void SERVER::SendPosition(const XMFLOAT3& position)
{
    int len = 0;
    UpdatePlayerPosition(position);
    printf("send position\n");
    len = SendMessageType(clientSocket, MESSAGETYPE::INGAME);
    len = send(clientSocket, (char*)&P_InGame, sizeof(PACKET_INGAME), 0);
}

