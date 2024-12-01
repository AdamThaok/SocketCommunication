#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib") // Link with Winsock library

#define DEFAULT_PORT "12345"
#define SERVER_ADDR "127.0.0.1" // Replace with your server's IP address

void Error(const char* msg) {
    fprintf(stderr, "%s\n", msg);
    exit(1);
}

int main() {
    WSADATA wsaData;
    SOCKET ConnectSocket = INVALID_SOCKET;
    struct addrinfo* result = NULL, hints;
    char sendbuf[512];
    char recvbuf[512];
    int iResult;

    // Step 1: Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        Error("[-] WSAStartup failed");
    }

    // Step 2: Setup address info hints
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;         // IPv4
    hints.ai_socktype = SOCK_STREAM;   // TCP
    hints.ai_protocol = IPPROTO_TCP;   // TCP

    // Step 3: Resolve the server address and port
    iResult = getaddrinfo(SERVER_ADDR, DEFAULT_PORT, &hints, &result);
    if (iResult != 0) {
        WSACleanup();
        Error("[-] getaddrinfo failed");
    }

    // Step 4: Create a socket to connect to the server
    ConnectSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ConnectSocket == INVALID_SOCKET) {
        freeaddrinfo(result);
        WSACleanup();
        Error("[-] Socket creation failed");
    }

    // Step 5: Connect to the server
    iResult = connect(ConnectSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        closesocket(ConnectSocket);
        freeaddrinfo(result);
        WSACleanup();
        Error("[-] Unable to connect to server");
    }

    freeaddrinfo(result); // Free address info as it is no longer needed

    printf("[+] Connected to the server. Type 'exit' to quit.\n");

    // Step 6: Send and receive data in a loop
    while (1) {
        printf("> ");
        fgets(sendbuf, sizeof(sendbuf), stdin);
        sendbuf[strcspn(sendbuf, "\n")] = '\0'; // Remove trailing newline

        // Exit condition
        if (strcmp(sendbuf, "exit") == 0) {
            printf("[+] Closing connection.\n");
            break;
        }

        // Send the message
        iResult = send(ConnectSocket, sendbuf, (int)strlen(sendbuf), 0);
        if (iResult == SOCKET_ERROR) {
            printf("[-] Send failed: %d\n", WSAGetLastError());
            break;
        }

        // Receive the server's response
        if (iResult > 0) {
            recvbuf[iResult] = '\0'; // Null-terminate the received data
            printf("Server: %s\n", recvbuf);
        }
        else if (iResult == 0) {
            printf("[+] Connection closed by server.\n");
            break;
        }
        else {
            printf("[-] recv failed: %d\n", WSAGetLastError());
            break;
        }
    }

















    // Step 7: Cleanup
    closesocket(ConnectSocket);
    WSACleanup();
    printf("[+] Client shutdown.\n");
    return 0;
}
