#ifndef SERVER_H
#define SERVER_H

#include <string>
#include <vector>
#include <unordered_set>
#include <mutex>
#include <thread>
#include <fstream>
#include <map>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <filesystem>

namespace fs = std::filesystem;

class Server {
public:
    // Конструктор, принимающий параметры порта, максимальное количество клиентов и файлов на клиента
    Server(unsigned short port, int maxClients = 5, int maxFilesPerClient = 10);

    // Деструктор
    ~Server();

    // Основная функция для запуска сервера
    void start();

private:
    // Сокет для сервера
    int server_sock;
    unsigned short port;
    int maxClients;
    int maxFilesPerClient;

    // Множество для отслеживания активных соединений
    std::unordered_set<int> active_clients;

    // Защита от параллельного доступа
    std::mutex client_mutex;

    // Функция для принятия клиентов
    void acceptClients();

    // Функция для обработки клиента
    void handleClient(int client_sock);

    // Функция для аутентификации клиента
    bool authenticate(int sock, std::string& username, std::string& password);

    // Функция для проверки логина и пароля в базе данных
    bool checkCredentials(const std::string& username, const std::string& password);

    // Функция для отправки строки клиенту
    bool sendString(int sock, const std::string& message);

    // Функция для получения строки от клиента
    std::string receiveString(int sock);

    // Функция для обработки загрузки файла
    bool handleUpload(int sock, const std::string& username);

    // Функция для создания каталога
    bool handleMkdir(int sock);

    // Функция для безопасного закрытия сокета
    void safeClose(int sock);

    // Счётчик файлов для каждого клиента
    std::map<std::string, int> file_upload_count;
};

#endif // SERVER_H

