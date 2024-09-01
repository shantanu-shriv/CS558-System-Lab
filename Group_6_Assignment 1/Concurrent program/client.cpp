#include <iostream>
#include <cstring>

#ifdef _WIN32
#include <winsock2.h>
#else
#include <unistd.h>
#include <arpa/inet.h>
#endif

using namespace std;


const int BUFFER_SIZE = 1024;

int main() {
#ifdef _WIN32
    // Initialize Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "Failed to initialize Winsock" << endl;
        return 1;
    }
#endif

    // TCP client
    int tcpClientSocket = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in tcpServerAddr{};
    tcpServerAddr.sin_family = AF_INET;

    // Prompt user for server IP and port
    cout << "Enter server IP address: ";
    char serverIP[15];
    cin >> serverIP;
    cout << "Enter server port: ";
    int tcpServerPort;
    cin >> tcpServerPort;

    tcpServerAddr.sin_port = htons(tcpServerPort);

#ifdef _WIN32
    tcpServerAddr.sin_addr.s_addr = inet_addr(serverIP);
#else
    inet_pton(AF_INET, serverIP, &tcpServerAddr.sin_addr);
#endif

    connect(tcpClientSocket, reinterpret_cast<sockaddr*>(&tcpServerAddr), sizeof(tcpServerAddr));

    // Send Request Message (Type 1) over TCP
    cout << "Enter Request Message (Type 1): ";
    char requestMessage[BUFFER_SIZE];
    cin.ignore(); // Clear input buffer
    cin.getline(requestMessage, BUFFER_SIZE);

    send(tcpClientSocket, requestMessage, strlen(requestMessage), 0);
    cout << "Sent Request Message (Type 1) to server: " << requestMessage << endl;

    // Receive Response Message (Type 2) with UDP port over TCP
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, sizeof(buffer));
    recv(tcpClientSocket, buffer, sizeof(buffer), 0);
    cout << "Received Response Message (Type 2) from server with UDP port: " << buffer << endl;

    // Close TCP connection
    closesocket(tcpClientSocket);

    // UDP client for data transfer
    int udpClientSocket = socket(AF_INET, SOCK_DGRAM, 0);
    sockaddr_in udpServerAddr{};
    udpServerAddr.sin_family = AF_INET;
    udpServerAddr.sin_port = htons(atoi(buffer));  // Convert received UDP port to integer

#ifdef _WIN32
    udpServerAddr.sin_addr.s_addr = inet_addr(serverIP);
#else
    inet_pton(AF_INET, serverIP, &udpServerAddr.sin_addr);
#endif

    // Send Data Message (Type 3) over UDP
    cout << "Enter Data Message (Type 3): ";
    char dataMessage[BUFFER_SIZE];
    cin.ignore(); // Clear input buffer
    cin.getline(dataMessage, BUFFER_SIZE);

    sendto(udpClientSocket, dataMessage, strlen(dataMessage), 0, reinterpret_cast<sockaddr*>(&udpServerAddr), sizeof(udpServerAddr));
    cout << "Sent Data Message (Type 3) to server: " << dataMessage << endl;

    // Receive Data Response (Type 4) over UDP
    memset(buffer, 0, sizeof(buffer));
    recvfrom(udpClientSocket, buffer, sizeof(buffer), 0, nullptr, nullptr);
    cout << "Received Data Response (Type 4) from server: " << buffer << endl;

    // Close UDP socket
    closesocket(udpClientSocket);

#ifdef _WIN32
    // Cleanup Winsock
    WSACleanup();
#endif
    return 0;
}
