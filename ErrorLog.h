#ifndef ERRORLOG_H
#define ERRORLOG_H

#include <string>
#include <mutex>

class ErrorLog {
public:
    // Путь к лог-файлам для сервера и клиента
    static std::string serverLogFile;
    static std::string clientLogFile;

    // Логирование ошибок для сервера
    static void logServerError(bool critical, const std::string& errorMessage);

    // Логирование ошибок для клиента
    static void logClientError(bool critical, const std::string& errorMessage);

private:
    // Мьютекс для потокобезопасной записи в лог
    static std::mutex logMutex;

    // Общий метод для записи в лог-файл (для серверных и клиентских)
    static void logError(const std::string& logFile, bool critical, const std::string& errorMessage);
};

#endif // ERRORLOG_H

