#ifndef SERVER_H
#define SERVER_H

#include <string>
#include <unordered_map>
#include <functional>
#include <windows.h>

class HTTPServer {
public:
    HTTPServer(int port);
    void start();
    void handle_client(SOCKET client_socket);

private:
    SOCKET server_fd;
    int port;
    std::unordered_map<std::string, std::string> mime_types;
    std::unordered_map<std::string, std::function<std::string(const std::string&)>> get_routes;
    std::unordered_map<std::string, std::function<std::string(const std::string&)>> post_routes;
    
    void initialize_mime_types();
    void initialize_routes();
    std::string get_mime_type(const std::string& path);
    std::string get_file_content(const std::string& path);
    std::string handle_get_request(const std::string& path);
    std::string handle_post_request(const std::string& path, const std::string& body);
    std::string get_current_time();
};

#endif // SERVER_H