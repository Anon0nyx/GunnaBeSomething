#include <iostream>
#include <process.h>

#include "server.h"

SOCKET server_fd;
int port = 6969;

unsigned __stdcall client_thread(void* args) {
    SOCKET client_socket = (SOCKET)((void**)args)[0];
    delete[] static_cast<void**>(args);

    handle_client(client_socket);
    return 0;
}

int main(int argc, char* argv[]) {
    // See if we got a port from our user, set it if not
    if (argc > 1) {
        try {
            port = std::stoi(argv[1]);
            if (port < 1 || port > 65535) {
                throw std::out_of_range("Port number out of range");
            }
        } catch (const std::exception& e) {
            std::cerr << "Invalid port number. Using default port 6969" << std::endl;
            port = 6969;
        }
    }

    try {
        // Initial configuration steps 
        initialize_mime_types();
        initialize_routes();

        // Now to actually build the server itself
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            std::cerr << "WSAStartup failed with error: " << WSAGetLastError() << std::endl;
            exit(EXIT_FAILURE);
        }

        // Setup our socket
        server_fd = socket(AF_INET, SOCK_STREAM, 0); // Setup our socket
        if (server_fd == INVALID_SOCKET) {
            std::cerr << "socket failed with error: " << WSAGetLastError() << std::endl;
            WSACleanup();
            exit(EXIT_FAILURE);
        }

        // Define some variables for the server
		sockaddr_in address;
		address.sin_family = AF_INET;
		address.sin_addr.s_addr = INADDR_ANY;
		address.sin_port = htons(port);

		// Bing our socket to our port
		if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) == SOCKET_ERROR) {
			std::cerr << "bind failed with error: " << WSAGetLastError() << std::endl;
			std::cerr << "Make sure the port " << port << " is not already in use." << std::endl;
			closesocket(server_fd);
			WSACleanup();
			exit(EXIT_FAILURE);
		}

		// Ensure that the server is able to listen without error
		if (listen(server_fd, 10) == SOCKET_ERROR) {
			std::cerr << "listen failed with error: " << WSAGetLastError() << std::endl;
			closesocket(server_fd);
			WSACleanup();
			exit(EXIT_FAILURE);
		}

		std::cout << "Server listening on port " << port << std::endl;
		   
		// This is where the server actually starts listening, everything before this is just
		// A check to make sure that the initialization worked properly
		while (true) {
			SOCKET client_socket = accept(server_fd, nullptr, nullptr);
            if (client_socket == INVALID_SOCKET) {
				std::cerr << "accept failed with error: " << WSAGetLastError() << std::endl;
				closesocket(server_fd);
				WSACleanup();
				exit(EXIT_FAILURE);
		    }
            void** args = new void*[1];
            args[0] = (void*)client_socket;
            uintptr_t thread = _beginthreadex(NULL, 0, client_thread, args, 0, NULL);
            if (thread == 0) {
                std::cerr << "Failed to create thread with error: " << GetLastError() << std::endl;
                delete[] args;
                closesocket(client_socket);
            } else {
                CloseHandle((HANDLE)thread);
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}
