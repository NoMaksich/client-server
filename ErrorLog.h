#include <string>
#include <mutex>

class ErrorLog {
public:
    static std::string serverLogFile;
    static std::string clientLogFile;

    static void logServerError(bool critical, const std::string& errorMessage);

    static void logClientError(bool critical, const std::string& errorMessage);

private:
    static std::mutex logMutex;

    static void logError(const std::string& logFile, bool critical, const std::string& errorMessage);
};

