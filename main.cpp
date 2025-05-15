#include "Server.h"
#include <iostream>

int main() {
    unsigned short port = 33333;
    int maxClients = 3;                        
    int maxFilesPerClient = 5;                

    try {
        Server server(port, maxClients, maxFilesPerClient);
        
        std::cout << "Server is starting on port " << port << "...\n";
        server.start();
    } catch (const std::exception& e) {
        std::cerr << "Server error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}

