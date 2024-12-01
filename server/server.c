#include <Winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>

#pragma comment(lib, "Ws2_32.lib") // Link Winsock library

void Error(const char* msg) {
    fprintf(stderr, "%s", msg);
    exit(1);
}

void InitHints(struct addrinfo* hints) {
    hints->ai_family = AF_INET;
    hints->ai_socktype = SOCK_STREAM;
    hints->ai_protocol = IPPROTO_TCP;
    hints->ai_flags = AI_PASSIVE;
}

#define DEFAULT_PORT "12345"
#define BUFFER_SIZE 512

int main() {

    // Step 1: Initialize Winsock
    WSADATA wsaData;
    struct addrinfo* result = NULL;
    struct addrinfo hints;
    SOCKET ClientSocket = INVALID_SOCKET;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);

    if (iResult != 0)
        Error("[-] WSAStartup failed\n");

    // Step 2: Initialize hints (comm method/protocol)
    ZeroMemory(&hints, sizeof(hints));
    InitHints(&hints);

    // Step 3: Resolve the local address and port
    iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
    if (iResult != 0)
        Error("[-] getaddrinfo failed\n");

    // Step 4: Create a socket
    SOCKET ListenSocket = INVALID_SOCKET;
    ListenSocket = socket(hints.ai_family, hints.ai_socktype, hints.ai_protocol);
    if (ListenSocket == INVALID_SOCKET) {
        freeaddrinfo(result);
        WSACleanup();
        Error("[-] List Socket failed\n");
    }

    // Step 5: Bind the socket
    iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult != 0) {
        freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();
        Error("[-] Bind failed\n");
    }
    freeaddrinfo(result);

    // Step 6: Listen for incoming connections
    iResult = listen(ListenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR) {
        closesocket(ListenSocket);
        WSACleanup();
        Error("[-] Listen failed\n");
    }
    printf("[+] Listening on port %s...\n", DEFAULT_PORT);

    // Step 7: Accept a client socket
    ClientSocket = accept(ListenSocket, NULL, NULL);
    if (ClientSocket == INVALID_SOCKET) {
        closesocket(ListenSocket);
        WSACleanup();
        Error("[-] Accept failed\n");
    }

    printf("[+] Client connected.\n");

    // Step 8: Send a message to the client
    const char* sendbuf = "Welcome to the server!";
    iResult = send(ClientSocket, sendbuf, (int)strlen(sendbuf), 0);
    if (iResult == SOCKET_ERROR) {
        printf("[-] Send failed: %d\n", WSAGetLastError());
        closesocket(ClientSocket);
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }
    printf("[+] Welcome message sent to the client.\n");

    // Step 9: Receive a message from the client
    char recvbuf[BUFFER_SIZE];
    int recvbuflen = BUFFER_SIZE;
    while (1) {
        iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
        if (iResult > 0) {
            recvbuf[iResult] = '\0'; // Null-terminate the received data
            printf("[+] Received from client: %s\n", recvbuf);
        }
        else if (iResult == 0) {
            printf("[+] Connection closed by client.\n");
            break;
        }
        else {
            printf("[-] recv failed: %d\n", WSAGetLastError());
            break;
        }
    }
    // Cleanup
    closesocket(ClientSocket);
    closesocket(ListenSocket);
    WSACleanup();
    printf("[+] Server shutdown.\n");

    return 0;
}
