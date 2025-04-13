#include "Server.h"
#include "ErrorLog.h"  // добавлено для логирования
#include <iostream>
#include <fstream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <filesystem>
#include <fstream>
#include <mutex>

namespace fs = std::filesystem;

Server::Server(unsigned short port, int maxClients, int maxFilesPerClient)
    : port(port), maxClients(maxClients), maxFilesPerClient(maxFilesPerClient) {
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0) {
        ErrorLog::logServerError(true, "Socket creation failed");
        throw std::runtime_error("Socket creation failed");
    }

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(server_sock, (sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        ErrorLog::logServerError(true, "Bind failed");
        throw std::runtime_error("Bind failed");
    }

    if (listen(server_sock, 5) < 0) {
        ErrorLog::logServerError(true, "Listen failed");
        throw std::runtime_error("Listen failed");
    }

    std::cout << "Server listening on port " << port << std::endl;
}

void Server::start() {
    acceptClients();
}

void Server::acceptClients() {
    while (true) {
        sockaddr_in client_addr{};
        socklen_t client_len = sizeof(client_addr);
        int client_sock = accept(server_sock, (sockaddr*)&client_addr, &client_len);

        if (client_sock < 0) {
            ErrorLog::logServerError(false, "Failed to accept client connection");
            continue;
        }

        std::lock_guard<std::mutex> lock(client_mutex);
        if ((int)active_clients.size() >= maxClients) {
            sendString(client_sock, "ERROR_TOO_MANY_CLIENTS");
            ErrorLog::logClientError(false, "Client rejected due to max client limit");
            close(client_sock);
            continue;
        }

        active_clients.insert(client_sock);
        std::thread(&Server::handleClient, this, client_sock).detach();
    }
}

void Server::handleClient(int client_sock) {
    std::string username, password;
    try {
        if (!authenticate(client_sock, username, password)) {
            sendString(client_sock, "AUTH_FAIL");
            ErrorLog::logClientError(false, "Authentication failed for client");
            safeClose(client_sock);
            return;
        }

        sendString(client_sock, "AUTH_OK");

        while (true) {
            std::string command = receiveString(client_sock);
            if (command == "UPLOAD") {
                if (!handleUpload(client_sock, username)) break;
            } else if (command == "MKDIR") {
                if (!handleMkdir(client_sock)) break;
            } else if (command.empty()) {
                break;
            }
        }
    } catch (const std::exception& e) {
        std::string errMsg = std::string("Exception while handling client: ") + e.what();
        ErrorLog::logClientError(true, errMsg);
        std::cerr << "[Client] Exception: " << e.what() << std::endl;
    }

    safeClose(client_sock);
}

bool Server::authenticate(int sock, std::string& username, std::string& password) {
    username = receiveString(sock);
    password = receiveString(sock);

    if (checkCredentials(username, password)) {
        return true;
    }
    return false;
}

bool Server::checkCredentials(const std::string& username, const std::string& password) {
    std::ifstream file("base.txt");
    if (!file.is_open()) {
        ErrorLog::logServerError(true, "Failed to open credentials file (base.txt)");
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        size_t pos = line.find(":");
        if (pos != std::string::npos) {
            std::string storedUsername = line.substr(0, pos);
            std::string storedPassword = line.substr(pos + 1);
            if (storedUsername == username && storedPassword == password) {
                return true;
            }
        }
    }

    return false;
}

bool Server::sendString(int sock, const std::string& message) {
    size_t length = message.size();
    if (send(sock, &length, sizeof(length), 0) != sizeof(length)) return false;
    return send(sock, message.c_str(), length, 0) == (ssize_t)length;
}

std::string Server::receiveString(int sock) {
    size_t length = 0;
    ssize_t r = recv(sock, &length, sizeof(length), MSG_WAITALL);
    if (r <= 0 || length > 100 * 1024 * 1024) return "";

    std::vector<char> buffer(length);
    r = recv(sock, buffer.data(), length, MSG_WAITALL);
    if (r <= 0) return "";
    return std::string(buffer.begin(), buffer.end());
}

bool Server::handleUpload(int sock, const std::string& username) {
    std::string destinationDir = receiveString(sock);
    std::string filename = receiveString(sock);
    std::string filesize_str = receiveString(sock);
    size_t filesize = std::stoull(filesize_str);

    {
        std::lock_guard<std::mutex> lock(client_mutex);
        if (file_upload_count[username] >= maxFilesPerClient) {
            sendString(sock, "ERROR_FILE_LIMIT_EXCEEDED");
            ErrorLog::logClientError(false, "File upload limit exceeded for user: " + username);
            return false;
        }
    }

    if (filesize > 0) {
        std::vector<char> buffer(filesize);
        if (recv(sock, buffer.data(), filesize, MSG_WAITALL) <= 0) {
            sendString(sock, "ERROR_FILE_RECEIVE");
            ErrorLog::logClientError(false, "Failed to receive file: " + filename);
            return false;
        }

        fs::create_directories(destinationDir);
        std::ofstream out(destinationDir + "/" + filename, std::ios::binary);
        if (!out.is_open()) {
            ErrorLog::logServerError(false, "Failed to save uploaded file: " + filename);
            sendString(sock, "ERROR_FILE_SAVE");
            return false;
        }
        out.write(buffer.data(), filesize);
        out.close();
    } else {
        fs::create_directories(destinationDir);
        std::ofstream out(destinationDir + "/" + filename, std::ios::binary);
        if (!out.is_open()) {
            ErrorLog::logServerError(false, "Failed to create empty file: " + filename);
            sendString(sock, "ERROR_FILE_CREATE");
            return false;
        }
        out.close();
    }

    {
        std::lock_guard<std::mutex> lock(client_mutex);
        file_upload_count[username]++;
    }

    sendString(sock, "FILE_OK");
    return true;
}

bool Server::handleMkdir(int sock) {
    std::string dirPath = receiveString(sock);
    try {
        fs::create_directories(dirPath);
        sendString(sock, "MKDIR_OK");
    } catch (const std::exception& e) {
        ErrorLog::logServerError(false, "Failed to create directory: " + dirPath + " | Reason: " + e.what());
        sendString(sock, "MKDIR_FAIL");
        return false;
    }
    return true;
}

void Server::safeClose(int sock) {
    std::lock_guard<std::mutex> lock(client_mutex);
    active_clients.erase(sock);
    close(sock);
}

Server::~Server() {
    if (server_sock != -1) {
        close(server_sock);
    }

    std::lock_guard<std::mutex> lock(client_mutex);
    for (int client_sock : active_clients) {
        close(client_sock);
    }
}

