#include <iostream>
#include <sstream>
#include <fstream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <process.h>
#include <ctime>
#include <iomanip>

#include <string>
#include <unordered_map>
#include <functional>
#include <Windows.h>

#pragma comment(lib, "Ws2_32.lib")

std::unordered_map<std::string, std::string> mime_types;
std::unordered_map<std::string, std::function<std::string(const std::string&)>> get_routes;
std::unordered_map<std::string, std::function<std::string(const std::string&)>> post_routes;

void initialize_mime_types();
void initialize_routes();
void handle_client(SOCKET client_sock);
std::string get_mime_type(const std::string& path);
std::string get_file_content(const std::string& path);
std::string handle_get_request(const std::string& path);
std::string handle_post_request(const std::string& path, const std::string& body);
std::string get_current_time();

SOCKET server_fd;
int port;

void handle_client(SOCKET client_socket) {
    char buffer[4096]; // Set our buffer for receiving messages
    int read_bytes = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
    if (read_bytes < 0) {
        std::cerr << "recv failed" << std::endl;
        closesocket(client_socket);
        return;
    }
    buffer[read_bytes] = '\0';

    std::istringstream request_stream(buffer);
    std::string method, path, version;
    request_stream >> method >> path >> version; // put request parts into individual variables

    sockaddr_in client_addr;
    int addrlen = sizeof(client_addr);
    getpeername(client_socket, (struct sockaddr*)&client_addr, &addrlen);
    char client_ip[INET_ADDRSTRLEN];
    strcpy(client_ip, inet_ntoa(client_addr.sin_addr));

    std::cout << "[" << get_current_time() << "] " << method << " request from " << client_ip << " for " << path << std::endl;

    std::string response;
    if (method == "GET") {
        response = handle_get_request(path);
    } else if (method == "POST") {
        std::string body;
        std::string line;
        while (std::getline(request_stream, line) && line != "\r") {
            // Skip headers
        }
        while (std::getline(request_stream, line)) {
            body += line + "\n";
        }
        response = handle_post_request(path, body);
    } else {
        response = "HTTP/1.1 405 Method Not Allowed\r\n\r\n";
    }

    send(client_socket, response.c_str(), response.size(), 0);
    closesocket(client_socket);
}

void initialize_mime_types() {
    mime_types[".html"] = "text/html";
    mime_types[".css"] = "text/css";
    mime_types[".js"] = "application/javascript";
    mime_types[".png"] = "image/png";
    mime_types[".jpg"] = "image/jpeg";
    mime_types[".gif"] = "image/gif";
    mime_types[".txt"] = "text/plain";
}

// As we can see, we are storing the available routes and their pages in a string object
void initialize_routes() {
    // Pretty simple map structure that stores each of the existing routes for the web pages
    get_routes["/"] = [](const std::string&) { return handle_get_request("/index.html"); };
    get_routes["/gol"] = [](const std::string&) { return handle_get_request("/game_of_life.html"); };
    get_routes["/mandelbrot"] = [](const std::string&) { return handle_get_request("/mandelbrot.html"); };
    get_routes["/cpp_api"] = [](const std::string&) { return handle_get_request("/test.html"); };
   
    // Define the two sets of POST routes that we have so far
    post_routes["/api/echo"] = [](const std::string& body) {
        return "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n{\"echo\": \"" + body + "\"}";
    };
    
    post_routes["/api/reverse"] = [](const std::string& body) {
        std::string reversed(body.rbegin(), body.rend());
        return "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n{\"reversed\": \"" + reversed + "\"}";
    };
} 

// Determining the MIME type of an outgoing file
std::string get_mime_type(const std::string& path) {
    size_t dot_pos = path.find_last_of(".");
    if (dot_pos != std::string::npos) {
        std::string ext = path.substr(dot_pos);
        if (mime_types.find(ext) != mime_types.end()) {
            return mime_types[ext];
        }
    }
    return "application/octet-stream";
}

std::string get_file_content(const std::string& path) {
    std::cout << path << std::endl;
    std::string full_path = "../../../templates/" + path;
    std::ifstream file(full_path, std::ios::binary);
    if (!file) {
        std::cout << "File not found: " << full_path << std::endl;
        return "";
    }

    return std::string((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
}

std::string handle_get_request(const std::string& path) {
    std::cout << "Handling GET request for path: " << path << std::endl;

    std::string file_path = path;
    if (path == "/") {
        file_path = "index.html";
    }

    auto route = get_routes.find(file_path);
    if (route != get_routes.end()) {
        std::cout << "Found route handler for: " << file_path << std::endl;
        return route->second(file_path);
    }

    std::string file_content = get_file_content(file_path);
    if (file_content.empty()) {
        std::cout << "File content is empty for: " << file_path << std::endl;
        return "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\nContent-Length: 9\r\n\r\nNot Found";
    }

    std::string mime_type = get_mime_type(file_path);
    std::ostringstream response;
    response << "HTTP/1.1 200 OK\r\n"
             << "Content-Type: " << mime_type << "\r\n"
             << "Content-Length: " << file_content.size() << "\r\n"
             << "\r\n"
             << file_content;

    std::cout << "Serving file: " << file_path << " with MIME type: " << mime_type << std::endl;
    return response.str();
}

std::string handle_post_request(const std::string& path, const std::string& body) {
    auto route = post_routes.find(path);
    if (route != post_routes.end()) {
        return route->second(body);
    }
    return "HTTP/1.1 404 Not Found\r\n\r\nEndpoint not found";
}

std::string get_current_time() {
    time_t now = time(0);
    struct tm *tstruct = localtime(&now);
    char buf[80];
    strftime(buf, sizeof(buf), "%Y-%m-%d %X", tstruct);
    return buf;
}

int main(int argc, char* argv[]) {
    std::cout << argv[0] << std::endl;
    int port = 6969;
    
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
            std::cout << "Server has made it here" << std::endl;
			SOCKET client_socket = accept(server_fd, nullptr, nullptr);
			if (client_socket == INVALID_SOCKET) {
				std::cerr << "accept failed with error: " << WSAGetLastError() << std::endl;
				closesocket(server_fd);
				WSACleanup();
				exit(EXIT_FAILURE);
			}
            std::cout << "Client socket received" << std::endl;
		    handle_client(client_socket);
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}