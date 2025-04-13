#include "Client.h"
#include <iostream>

int main() {
    std::string server_ip = "127.0.0.1"; // Укажите IP сервера
    unsigned short server_port = 33333;  // Укажите порт сервера
    std::string username;
    std::string password;

    // Вводим логин и пароль
    std::cout << "Enter username: ";
    std::cin >> username;

    std::cout << "Enter password: ";
    std::cin >> password;

    try {
        // Создаем объект клиента
        Client client(server_ip, server_port, username, password);

        // Подключаемся к серверу
        client.connectToServer();

        // Пытаемся пройти аутентификацию
        if (client.authenticate()) {
            std::string command;
            while (true) {
                std::cout << "Enter command (UPLOAD, MKDIR, EXIT): ";
                std::cin >> command;

                if (command == "UPLOAD") {
                    std::string filepath;
                    std::string destinationDir;
                    std::cout << "Enter file path: ";
                    std::cin >> filepath;
                    std::cout << "Enter destination directory: ";
                    std::cin >> destinationDir;
                    client.uploadFile(filepath, destinationDir);
                } else if (command == "MKDIR") {
                    std::string dirPath;
                    std::cout << "Enter directory path: ";
                    std::cin >> dirPath;
                    client.createDirectory(dirPath);
                } else if (command == "EXIT") {
                    std::cout << "Exiting program." << std::endl;
                    break;
                } else {
                    std::cout << "Unknown command!" << std::endl;
                }
            }
        } else {
            std::cerr << "Authentication failed. Closing connection." << std::endl;
        }
    } catch (const std::runtime_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}

