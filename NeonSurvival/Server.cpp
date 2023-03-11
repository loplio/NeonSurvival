#include "Server.h"

void SERVER::init(HWND hWnd)
{
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        printf("WSAStartup failed with error: %d\n", result);
        return;
    }

    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
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
        int len = recv(wParam, (char*)&MessageType, sizeof(MessageType), 0);
        printf("read\n");
        switch (MessageType)
        {
        case MESSAGETYPE::LOGIN:
        {
            int len = recv(wParam, (char*)&ClientNumId, sizeof(int), 0);
            printf("ClientNum : %d\n", ClientNumId);
            break;
        }
        case MESSAGETYPE::INGAME:
        {
            int len = recv(wParam, (char*)&PlayersPostion, sizeof(PlayersPostion), 0);
            printf("id : %d - x : %f y : %f z : %f", PlayersPostion[0].id, PlayersPostion[0].position.x, PlayersPostion[0].position.y, PlayersPostion[0].position.z);
            printf("id : %d - x : %f y : %f z : %f", PlayersPostion[1].id, PlayersPostion[1].position.x, PlayersPostion[1].position.y, PlayersPostion[1].position.z);
            break;
        }
        default:
            break;
        }
        break;
    }
    case FD_WRITE: {
        printf("write\n");
        if (FirstConnect == false)
        {
            FirstConnect = true;
            int type = MESSAGETYPE::LOGIN;
            int len = send(wParam, (char*)&type, sizeof(type), 0);
            printf("%d\n", (char*)type);
        }
        else
        {
            int len = send(wParam, buf, BUFSIZE, 0);

        }
        break;
    }

    }
}

