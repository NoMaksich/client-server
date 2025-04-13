#include "ErrorLog.h"
#include <iostream>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <mutex>

std::string ErrorLog::serverLogFile = "server_log.txt";  // Лог-файл для сервера
std::string ErrorLog::clientLogFile = "client_log.txt";  // Лог-файл для клиента
std::mutex ErrorLog::logMutex;  // Мьютекс для потокобезопасности

void ErrorLog::logServerError(bool critical, const std::string& errorMessage) {
    logError(serverLogFile, critical, errorMessage);
}

void ErrorLog::logClientError(bool critical, const std::string& errorMessage) {
    logError(clientLogFile, critical, errorMessage);
}

void ErrorLog::logError(const std::string& logFile, bool critical, const std::string& errorMessage) {
    // Блокировка мьютекса для потокобезопасности
    std::lock_guard<std::mutex> lock(logMutex);

    std::ofstream logFileStream(logFile, std::ios_base::app); // Открытие файла для добавления данных
    if (logFileStream.is_open()) {
        // Получаем текущее время для записи в лог
        auto now = std::chrono::system_clock::now();
        time_t now_c = std::chrono::system_clock::to_time_t(now);
        struct tm* timeinfo = localtime(&now_c);
        char buffer[80];
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);

        // Записываем дату, время, критичность и сообщение об ошибке в файл
        logFileStream << "Date and time: " << buffer
                      << " | Critical: " << (critical ? "Yes" : "No")
                      << " | Error: " << errorMessage << std::endl;

        logFileStream.close();  // Закрытие файла
    } else {
        // Если не удалось открыть файл для записи, выводим сообщение об ошибке
        std::cerr << "Unable to open log file: " << logFile << std::endl;
    }
}

