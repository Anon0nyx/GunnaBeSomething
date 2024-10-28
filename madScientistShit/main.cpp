#include <winsock2.h>
#include <windows.h>
#include <iostream>
#include <string>
#include <vector>
#include <thread>

#pragma comment(lib, "ws2_32.lib")

struct Player {
    float x, y;
    bool isJumping;
    float dy;
};

std::vector<std::vector<int>> grid(120, std::vector<int>(160, 0));
Player player = { 5, 20, false, 0.0f };

void setupGrid() {
    int groundLevel = grid.size() / 2;
    for (int row = 0; row < grid.size(); ++row) {
        for (int col = 0; col < grid[0].size(); ++col) {
            if (row >= groundLevel) {
                grid[row][col] = 2; // Brown blocks
            } else if (row == groundLevel - 1) {
                grid[row][col] = 1; // Green blocks
            }
        }
    }
    std::cout << "Grid setup complete." << std::endl;
}

bool isColliding(float x, float y) {
    int bottomY = y + 3 - 1;
    for (int i = 0; i < 3; ++i) {
        if (grid[static_cast<int>(y + i)][static_cast<int>(x)] != 0) {
            return true;
        }
    }
    return false;
}

void applyGravity() {
    player.dy += 0.02;
    if (!isColliding(player.x, player.y + player.dy)) {
        player.y += player.dy;
    } else {
        player.dy = 0;
        player.isJumping = false;
    }
}

void handleMovement(char key) {
    std::cout << "Key received: " << key << std::endl;
    if (key == 's' && !isColliding(player.x, player.y + 0.05)) player.y += 0.05;
    if (key == 'a' && !isColliding(player.x - 0.05, player.y)) player.x -= 0.05;
    if (key == 'd' && !isColliding(player.x + 0.05, player.y)) player.x += 0.05;
    if (key == ' ' && !player.isJumping) {
        player.dy = -0.5;
        player.isJumping = true;
    }
}

std::string serializeState() {
    std::string state = std::to_string(player.x) + "," + std::to_string(player.y) + "," + std::to_string(player.isJumping);
    std::cout << "Serialized state: " << state << std::endl;
    return state;
}

void gameLoop() {
    applyGravity();
    // Add other game logic here
}

void handleClient(SOCKET ClientSocket) {
    char recvBuffer[512];
    bool running = true;

    while (running) {
        int bytesReceived = recv(ClientSocket, recvBuffer, 512, 0);
        if (bytesReceived > 0) {
            handleMovement(recvBuffer[0]);

            gameLoop();

            std::string state = serializeState();
            int bytesSent = send(ClientSocket, state.c_str(), state.size(), 0);
            if (bytesSent == SOCKET_ERROR) {
                std::cerr << "Send failed." << std::endl;
                running = false;
            } else {
                std::cout << "Bytes sent: " << bytesSent << std::endl;
            }
        } else if (bytesReceived == 0) {
            std::cout << "Connection closed by client." << std::endl;
            running = false;
        } else {
            std::cerr << "Recv failed." << std::endl;
            running = false;
        }
    }

    closesocket(ClientSocket);
}

int main() {
    WSADATA wsaData;
    SOCKET ServerSocket, ClientSocket;
    sockaddr_in serverAddr, clientAddr;

    WSAStartup(MAKEWORD(2, 2), &wsaData);
    std::cout << "WSAStartup complete." << std::endl;

    ServerSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (ServerSocket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed." << std::endl;
        return 1;
    }
    std::cout << "Socket creation successful." << std::endl;

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(9001);

    if (bind(ServerSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Bind failed." << std::endl;
        closesocket(ServerSocket);
        WSACleanup();
        return 1;
    }
    std::cout << "Bind successful." << std::endl;

    if (listen(ServerSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Listen failed." << std::endl;
        closesocket(ServerSocket);
        WSACleanup();
        return 1;
    }
    std::cout << "Listening on port 9001." << std::endl;

    setupGrid();

    while (true) {
        int clientAddrSize = sizeof(clientAddr);
        ClientSocket = accept(ServerSocket, (SOCKADDR*)&clientAddr, &clientAddrSize);
        if (ClientSocket == INVALID_SOCKET) {
            std::cerr << "Accept failed." << std::endl;
        } else {
            std::cout << "Client connected." << std::endl;
            std::thread(handleClient, ClientSocket).detach(); // Handle client in a new thread
        }
    }

    closesocket(ServerSocket);
    WSACleanup();

    return 0;
}

