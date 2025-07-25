#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <cstring>
#include <thread>

void handle_client(int client_socket) {
    char buffer[1024];
    
    while (true) {
        memset(buffer, 0, sizeof(buffer));
        int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
        
        if (bytes_received <= 0) {
            if (bytes_received == 0) {
                std::cout << "Клиент отключился" << std::endl;
            } else {
                std::cerr << "Ошибка при чтении данных" << std::endl;
            }
            break;
        }
        
        std::cout << "Получено сообщение: " << buffer << std::endl;
        
        // Эхо-ответ
        // std::string response = "Сервер получил: ";
        // response += buffer;
        std::string response;
        std::cout << ">";
        std::getline(std::cin, response);
        send(client_socket, response.c_str(), response.size(), 0);
    }
    
    close(client_socket);
}

int server(int argc, char* argv[]) {

    int port = 8080;

    std::cout << argv[2] << std::endl;

    if (argc >= 2){
        port = std::stoi(argv[2]);
    }

    // Создание сокета
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        std::cerr << "Ошибка при создании сокета" << std::endl;
        return 1;
    }

    // Настройка адреса сервера
    sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port); // Порт 8080
    server_address.sin_addr.s_addr = INADDR_ANY; // Принимать соединения на все IP

    // Привязка сокета к адресу
    if (bind(server_socket, (struct sockaddr*)&server_address, sizeof(server_address)) == -1) {
        std::cerr << "Ошибка при привязке сокета" << std::endl;
        close(server_socket);
        return 1;
    }
    // Переход в режим прослушивания
 
    if (listen(server_socket, 1) == -1) { // Очередь длиной 5
        std::cerr << "Ошибка при переходе в режим прослушивания" << std::endl;
        close(server_socket);
        return 1;
    }
    std::cout << "Сервер запущен и ожидает подключений на порту " << port << std::endl;   

    

     while (true) {
        sockaddr_in client_addr{};
        socklen_t client_addr_len = sizeof(client_addr);
        int client_socket = accept(server_socket, (sockaddr*)&client_addr, &client_addr_len);
        char client_ip[INET_ADDRSTRLEN];
        // inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
        // std::cout << "Новое подключение от " << client_ip << ":" 
        //           << ntohs(client_addr.sin_port) << std::endl;
        
        if (client_socket < 0) {
            std::cerr << "Ошибка accept" << std::endl;
            continue;
        }
        
        std::thread(handle_client, client_socket).detach();
    }  


    close(server_socket); // Закрытие сокета
    return 0;
}

int client(int argc, char* argv[]) {

    int port = 8080;
    std::string ip = "127.0.0.1";

    if (argc >= 3){
        ip = argv[2];
        port = std::stoi(argv[3]);
    }
    std::cout << ip << ":" << port << std::endl;

    // 1. Создание сокета
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (client_socket == -1) {
        std::cerr << "Ошибка при создании сокета" << std::endl;
        return 1;
    }

    // 2. Настройка адреса сервера (куда подключаемся)
    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;           // IPv4
    server_addr.sin_port = htons(port);         // Порт сервера
    server_addr.sin_addr.s_addr = inet_addr(ip.c_str());  // IP сервера (localhost)

    // 3. Подключение к серверу
    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        std::cerr << "Ошибка подключения к серверу" << std::endl;
        close(client_socket);
        return 1;
    }

    std::cout << "Успешное подключение к серверу!" << std::endl;
    std::string message;
    // 4. Отправка данных
    while (true) {
        std::string message;
        std::cout << "> ";
        std::getline(std::cin, message);
        
        if (message == "exit") break;
        
        send(client_socket, message.c_str(), message.size(), 0);
        
        char buffer[1024] = {0};
        int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
        if (bytes_received > 0) {
            std::cout << "Ответ сервера: " << buffer << std::endl;
        }
    }
    // 6. Закрытие сокета
    close(client_socket);
    return 0;
}

int main(int argc, char* argv[]){
    std::string mode = argv[1];
    if (mode == "server") server(argc, argv);
    if (mode == "client") client(argc, argv);
}