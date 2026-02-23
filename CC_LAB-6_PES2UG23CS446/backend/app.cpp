#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main() {
    char hostname[256];
    gethostname(hostname, sizeof(hostname));
    hostname[255] = '\0';

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        std::cerr << "Socket creation failed\n";
        return 1;
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);

    if (bind(server_fd, (sockaddr*)&address, sizeof(address)) < 0) {
        std::cerr << "Bind failed\n";
        return 1;
    }

    if (listen(server_fd, 10) < 0) {
        std::cerr << "Listen failed\n";
        return 1;
    }

    std::cout << "Server listening on port 8080 (hostname: "
              << hostname << ")" << std::endl;

    while (true) {
        int client_fd = accept(server_fd, nullptr, nullptr);
        if (client_fd < 0) continue;

        // Read request (important for clean TCP close)
        char buffer[1024];
        recv(client_fd, buffer, sizeof(buffer), 0);

        std::string body = "Served by backend: ";
        body += hostname;
        body += "\n";

        std::string response =
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/plain\r\n"
            "Content-Length: " + std::to_string(body.length()) + "\r\n"
            "Connection: close\r\n"
            "\r\n" +
            body;

        send(client_fd, response.c_str(), response.size(), 0);

        shutdown(client_fd, SHUT_RDWR);   // clean shutdown
        close(client_fd);
    }

    return 0;
}
