#include <winsock2.h>
#include <windows.h>
#include <iostream>
#include <sstream>
#include <openssl/sha.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "libssl.lib")
#pragma comment(lib, "libcrypto.lib")

std::string base64_encode(const unsigned char* input, int length) {
    BIO* b64 = BIO_new(BIO_f_base64());
    BIO* bio = BIO_new(BIO_s_mem());
    bio = BIO_push(b64, bio);
    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
    BIO_write(bio, input, length);
    BIO_flush(bio);
    BUF_MEM* buffer;
    BIO_get_mem_ptr(bio, &buffer);
    std::string result(buffer->data, buffer->length);
    BIO_free_all(bio);
    return result;
}

std::string generate_websocket_accept(const std::string& key) {
    std::string magic_string = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
    std::string combined = key + magic_string;
    unsigned char hash[SHA_DIGEST_LENGTH];
    SHA1(reinterpret_cast<const unsigned char*>(combined.c_str()), combined.size(), hash);
    return base64_encode(hash, SHA_DIGEST_LENGTH);
}

bool do_handshake(SOCKET clientSocket) {
    const int bufferSize = 2048;
    char buffer[bufferSize];
    int bytesReceived = recv(clientSocket, buffer, bufferSize, 0);
    if (bytesReceived <= 0) return false;

    std::string request(buffer, bytesReceived);
    std::istringstream stream(request);
    std::string line;
    std::string secWebSocketKey;
    while (std::getline(stream, line) && !line.empty() && line != "\r") {
        if (line.find("Sec-WebSocket-Key") != std::string::npos) {
            secWebSocketKey = line.substr(line.find(":") + 2);
            secWebSocketKey.pop_back(); // Remove \r
        }
    }

    if (secWebSocketKey.empty()) return false;
    std::string acceptKey = generate_websocket_accept(secWebSocketKey);

    std::ostringstream response;
    response << "HTTP/1.1 101 Switching Protocols\r\n";
    response << "Upgrade: websocket\r\n";
    response << "Connection: Upgrade\r\n";
    response << "Sec-WebSocket-Accept: " << acceptKey << "\r\n";
    response << "\r\n";

    std::string responseStr = response.str();
    send(clientSocket, responseStr.c_str(), responseStr.length(), 0);

    return true;
}

int main() {
    WSADATA wsaData;
    SOCKET ServerSocket, ClientSocket;
    sockaddr_in serverAddr, clientAddr;

    WSAStartup(MAKEWORD(2, 2), &wsaData);
    ServerSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(9001);

    bind(ServerSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
    listen(ServerSocket, 1);

    int clientAddrSize = sizeof(clientAddr);
    ClientSocket = accept(ServerSocket, (SOCKADDR*)&clientAddr, &clientAddrSize);
    if (ClientSocket == INVALID_SOCKET) {
        closesocket(ServerSocket);
        WSACleanup();
        return 1;
    }

    if (!do_handshake(ClientSocket)) {
        closesocket(ClientSocket);
        closesocket(ServerSocket);
        WSACleanup();
        return 1;
    }

    const int bufferSize = 2048;
    char buffer[bufferSize];
    int bytesReceived;
    while ((bytesReceived = recv(ClientSocket, buffer, bufferSize, 0)) > 0) {
        send(ClientSocket, buffer, bytesReceived, 0);
    }

    closesocket(ClientSocket);
    closesocket(ServerSocket);
    WSACleanup();
    return 0;
}

