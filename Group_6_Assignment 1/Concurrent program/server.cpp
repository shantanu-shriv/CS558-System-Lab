#include <iostream>
#include <cstring>
#include <thread>
#include <vector>

#ifdef _WIN32
#include <winsock2.h>
typedef int socklen_t;
#else
#include <unistd.h>
#include <arpa/inet.h>
#endif

using namespace std;
const int BUFFER_SIZE = 1024;

void handleClient(int clientSocket) {
    // Receive Request Message (Type 1) over TCP
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, sizeof(buffer));
    recv(clientSocket, buffer, sizeof(buffer), 0);
    cout << "Received Request Message (Type 1) from client: " << buffer << endl;

    // Prompt user for UDP port
    cout << "Enter UDP port for data transfer: ";
    int udpPort;
    cin >> udpPort;

    // Send Response Message (Type 2) with UDP port over TCP
    snprintf(buffer, sizeof(buffer), "%d", udpPort);
    send(clientSocket, buffer, strlen(buffer), 0);
    cout << "Sent Response Message (Type 2) with UDP port: " << buffer << endl;

    // Close TCP connection
    closesocket(clientSocket);

    // UDP server for data transfer
    int udpServerSocket = socket(AF_INET, SOCK_DGRAM, 0);
    sockaddr_in udpServerAddr{};
    udpServerAddr.sin_family = AF_INET;
    udpServerAddr.sin_port = htons(udpPort);
    udpServerAddr.sin_addr.s_addr = INADDR_ANY;

    bind(udpServerSocket, reinterpret_cast<sockaddr*>(&udpServerAddr), sizeof(udpServerAddr));
    cout << "UDP Server waiting for data transfer on port " << udpPort << endl;

    // Receive Data Message (Type 3) over UDP
    memset(buffer, 0, sizeof(buffer));
    recvfrom(udpServerSocket, buffer, sizeof(buffer), 0, nullptr, nullptr);
    cout << "Received Data Message (Type 3) from client: " << buffer << endl;

    // Send Data Response (Type 4) over UDP
    const char* dataResponse = "Data received successfully!";
    sendto(udpServerSocket, dataResponse, strlen(dataResponse), 0, nullptr, 0);
    cout << "Sent Data Response (Type 4) to client: " << dataResponse << endl;

    // Close UDP socket
    closesocket(udpServerSocket);
}

int main() {
#ifdef _WIN32
    // Initialize Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "Failed to initialize Winsock" << endl;
        return 1;
    }
#endif

    // TCP server
    int tcpServerSocket = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in tcpServerAddr{};
    tcpServerAddr.sin_family = AF_INET;

    // Prompt user for server port
    cout << "Enter TCP server port: ";
    int tcpServerPort;
    cin >> tcpServerPort;
    tcpServerAddr.sin_port = htons(tcpServerPort);
    tcpServerAddr.sin_addr.s_addr = INADDR_ANY;

    bind(tcpServerSocket, reinterpret_cast<sockaddr*>(&tcpServerAddr), sizeof(tcpServerAddr));
    listen(tcpServerSocket, 5);
    cout << "TCP Server waiting for connections on port " << ntohs(tcpServerAddr.sin_port) << endl;

    vector<thread> threads;

    while (true) {
        sockaddr_in clientAddr{};
        socklen_t clientAddrLen = sizeof(clientAddr);
        int clientSocket = accept(tcpServerSocket, reinterpret_cast<sockaddr*>(&clientAddr), &clientAddrLen);
        cout << "TCP Connection established with client: " << inet_ntoa(clientAddr.sin_addr) << endl;

        // Create a new thread for each client
        threads.emplace_back(handleClient, clientSocket);
    }

    // Join all threads
    for (auto& thread : threads) {
        thread.join();
    }

    // Close TCP server socket
    closesocket(tcpServerSocket);

#ifdef _WIN32
    // Cleanup Winsock
    WSACleanup();
#endif

    return 0;
}
