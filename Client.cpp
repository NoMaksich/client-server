#include "Client.h"
#include "ErrorLog.h"

#include <iostream>
#include <fstream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <filesystem>
#include <vector>

namespace fs = std::filesystem;

Client::Client(const std::string& server_ip, unsigned short server_port, const std::string& username, const std::string& password)
    : server_ip(server_ip), server_port(server_port), username(username), password(password), sock(-1) {}

Client::~Client() {
    closeConnection();
}

void Client::connectToServer() {
    sockaddr_in server_addr;
    sock = socket(AF_INET, SOCK_STREAM, 0);

    if (sock < 0) {
        ErrorLog::logClientError(true, "Socket creation failed");
        throw std::runtime_error("Socket creation failed");
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    if (inet_pton(AF_INET, server_ip.c_str(), &server_addr.sin_addr) <= 0) {
        ErrorLog::logClientError(true, "Invalid address or address not supported");
        throw std::runtime_error("Invalid address or Address not supported");
    }

    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        ErrorLog::logClientError(true, "Connection to server failed");
        throw std::runtime_error("Connection failed");
    }

    std::cout << "Connected to the server at " << server_ip << ":" << server_port << std::endl;
}

bool Client::authenticate() {
    std::string firstResponse = receiveString();

    if (firstResponse == "ERROR_TOO_MANY_CLIENTS") {
        std::cerr << "Server is full. Too many clients connected." << std::endl;
        close(sock);
        return false;
    } else if (firstResponse == "AUTH_REQUEST") {
        sendString(username);
        sendString(password);

        std::string authResponse = receiveString();
        if (authResponse == "AUTH_OK") {
            std::cout << "Authenticated successfully!" << std::endl;
            return true;
        } else {
            std::cerr << "Authentication failed!" << std::endl;
            close(sock);
            return false;
        }
    } else {
        std::cerr << "Unexpected response from server: " << firstResponse << std::endl;
        close(sock);
        return false;
    }
}


void Client::uploadFile(const std::string& filepath, const std::string& destinationDir) {
    std::ifstream file(filepath, std::ios::binary | std::ios::ate);
    if (!file) {
        std::cerr << "Error: File not found!" << std::endl;
        ErrorLog::logClientError(false, "File not found: " + filepath);
        return;
    }

    std::streamsize filesize = file.tellg();
    file.seekg(0, std::ios::beg);

    sendString("UPLOAD");
    sendString(destinationDir);
    std::string filename = fs::path(filepath).filename().string();
    sendString(filename);
    sendString(std::to_string(filesize));

    if (filesize > 0) {
        std::vector<char> buffer(filesize);
        file.read(buffer.data(), filesize);
        if (send(sock, buffer.data(), filesize, 0) < 0) {
            std::cerr << "Error sending file data!" << std::endl;
            ErrorLog::logClientError(true, "Failed to send file data: " + filepath);
            return;
        }
    }

    std::string response = receiveString();
    if (response == "FILE_OK") {
        std::cout << "File uploaded successfully!" << std::endl;
    } else if (response == "ERROR_FILE_LIMIT_EXCEEDED") {
        std::cout << "Error: File limit exceeded. You cannot upload more files." << std::endl;
        ErrorLog::logClientError(false, "Upload failed: file limit exceeded for user " + username);
    } else {
        std::cout << "Error uploading file." << std::endl;
        ErrorLog::logClientError(true, "Unknown error uploading file: " + filepath);
    }
}

void Client::createDirectory(const std::string& dirPath) {
    sendString("MKDIR");
    sendString(dirPath);

    std::string response = receiveString();
    if (response == "MKDIR_OK") {
        std::cout << "Directory created successfully!" << std::endl;
    } else {
        std::cout << "Error creating directory." << std::endl;
        ErrorLog::logClientError(false, "Failed to create directory: " + dirPath);
    }
}

void Client::closeConnection() {
    if (sock != -1) {
        close(sock);
        sock = -1;
    }
}

bool Client::sendString(const std::string& message) {
    size_t length = message.size();
    if (send(sock, &length, sizeof(length), 0) != sizeof(length)) {
        ErrorLog::logClientError(true, "Failed to send string length");
        return false;
    }
    if (send(sock, message.c_str(), length, 0) != (ssize_t)length) {
        ErrorLog::logClientError(true, "Failed to send string content");
        return false;
    }
    return true;
}

std::string Client::receiveString() {
    size_t length = 0;
    ssize_t r = recv(sock, &length, sizeof(length), MSG_WAITALL);
    if (r <= 0 || length > 100 * 1024 * 1024) {
        ErrorLog::logClientError(true, "Failed to receive string length or length too large");
        return "";
    }

    std::vector<char> buffer(length);
    r = recv(sock, buffer.data(), length, MSG_WAITALL);
    if (r <= 0) {
        ErrorLog::logClientError(true, "Failed to receive string data");
        return "";
    }
    return std::string(buffer.begin(), buffer.end());
}
