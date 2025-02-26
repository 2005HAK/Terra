#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <thread>

#define PORT 8080

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    bool data = true;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
        std::cerr << "Erro ao criar socket" << std::endl;
        return 1;
    }

    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        std::cerr << "Erro ao fazer bind" << std::endl;
        return 1;
    }

    if (listen(server_fd, 3) < 0) {
        std::cerr << "Erro ao escutar" << std::endl;
        return 1;
    }

    std::cout << "Servidor aguardando conexão..." << std::endl;
    new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
    if (new_socket < 0) {
        std::cerr << "Erro ao aceitar conexão" << std::endl;
        return 1;
    }

    std::cout << "Conexão estabelecida. Pressione Enter para alternar o valor." << std::endl;

    std::thread inputThread([&]() {
        while (true) {
            std::cin.get();
            data = !data;
            std::cout << "Valor alterado para: " << (data ? "true" : "false") << std::endl;
        }
    });

    while (true) {
        send(new_socket, &data, sizeof(data), 0);
        usleep(500000);
    }

    inputThread.join();
    close(new_socket);
    close(server_fd);
    return 0;
}
