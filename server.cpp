#include <iostream>
#include <sstream>
#include <fstream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <process.h>
#include <ctime>
#include <iomanip>

#include "server.h"

std::unordered_map<std::string, std::string> mime_types;
std::unordered_map<std::string, std::function<std::string(const std::string&)>> get_routes;
std::unordered_map<std::string, std::function<std::string(const std::string&)>> post_routes;

#pragma comment(lib, "Ws2_32.lib")

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
    get_routes["/game"] = [](const std::string&) { return handle_get_request("/sideViewGameTest.html"); };
 
    // Define the two sets of POST routes that we have so far
    post_routes["/api/echo"] = [](const std::string& body) {
        std::cout << body << std::endl;
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