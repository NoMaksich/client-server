#ifndef CLIENT_H
#define CLIENT_H

#include <string>
#include <iostream>
#include <stdexcept>
#include <sys/socket.h>
#include <arpa/inet.h>

class Client {
public:
    Client(const std::string& server_ip, unsigned short server_port, const std::string& username, const std::string& password);
    ~Client();

    void connectToServer();
    bool authenticate();
    void uploadFile(const std::string& filepath, const std::string& destinationDir);
    void createDirectory(const std::string& dirPath);
    void closeConnection();

private:
    int sock;
    std::string server_ip;
    unsigned short server_port;
    std::string username;
    std::string password;

    bool sendString(const std::string& message);
    std::string receiveString();
};

#endif // CLIENT_H

