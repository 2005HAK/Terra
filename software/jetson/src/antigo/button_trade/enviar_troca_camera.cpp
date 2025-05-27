#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
/*
Só Troca o Cout e o cin ali, se mandar 0 ele troca pra camera 0 se mandar 1 troca pra camera 1 :D
*/
void send_int(const char* host, int port, int number) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        std::cerr << "Erro ao criar socket\n";
        return;
    }
    
    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, host, &server_addr.sin_addr) <= 0) {
        std::cerr << "Endereço inválido\n";
        close(sock);
        return;
    }
    
    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Erro ao conectar\n";
        close(sock);
        return;
    }
    
    int32_t net_number = htonl(number); 
    send(sock, &net_number, sizeof(net_number), 0);
    std::cout << "Número " << number << " enviado com sucesso!\n";
    
    close(sock);
}

int main() {
    const char* server_ip = "127.0.0.1";
    int port = 12345;
    int number;
    
    std::cout << "Digite um número inteiro: ";
    std::cin >> number;
    
    send_int(server_ip, port, number);
    return 0;
}
