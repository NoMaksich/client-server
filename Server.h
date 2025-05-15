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
    Server(unsigned short port, int maxClients = 5, int maxFilesPerClient = 10);

    ~Server();

    void start();

private:
    int server_sock;
    unsigned short port;
    int maxClients;
    int maxFilesPerClient;

    std::unordered_set<int> active_clients;

    std::mutex client_mutex;

    void acceptClients();

    void handleClient(int client_sock);

    bool authenticate(int sock, std::string& username, std::string& password);

    bool checkCredentials(const std::string& username, const std::string& password);

    bool sendString(int sock, const std::string& message);

    std::string receiveString(int sock);

    bool handleUpload(int sock, const std::string& username);

    bool handleMkdir(int sock);

    void safeClose(int sock);

    std::map<std::string, int> file_upload_count;
};
