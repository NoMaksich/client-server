#include "Server.h"
#include <iostream>

int main() {
    unsigned short port = 33333;               // Указываем порт для сервера
    int maxClients = 3;                        // Максимальное количество клиентов
    int maxFilesPerClient = 5;                 // Максимальное количество файлов на клиента

    try {
        // Создание объекта сервера с указанными параметрами
        Server server(port, maxClients, maxFilesPerClient);
        
        // Запуск сервера
        std::cout << "Server is starting on port " << port << "...\n";
        server.start();
    } catch (const std::exception& e) {
        // Обработка ошибок и вывод сообщения
        std::cerr << "Server error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}

