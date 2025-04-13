CC = g++
CFLAGS = -g -O0 -Wall
LIBS = -lcryptopp
LDFLAGS =-lcryptopp

# Исходные файлы для сервера и клиента
SRCS_SERVER = main.cpp Server.cpp ErrorLog.cpp
SRCS_CLIENT = Client.cpp main_client.cpp ErrorLog.cpp
OBJS_SERVER = $(SRCS_SERVER:.cpp=.o)
OBJS_CLIENT = $(SRCS_CLIENT:.cpp=.o)

# Исполняемые файлы
EXEC_SERVER = server
EXEC_CLIENT = client

# Правило по умолчанию (собрать сервер и клиент)
all: $(EXEC_SERVER) $(EXEC_CLIENT)

# Правила сборки сервера
$(EXEC_SERVER): $(OBJS_SERVER)
	$(CC) $(CFLAGS) $(OBJS_SERVER) $(LIBS) -o $(EXEC_SERVER)

# Правила сборки клиента
$(EXEC_CLIENT): $(OBJS_CLIENT)
	$(CC) $(CFLAGS) $(OBJS_CLIENT) $(LIBS) -o $(EXEC_CLIENT)

# Компиляция исходников в объектные файлы
%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

# Очистка объектных и исполняемых файлов
clean:
	rm -f $(OBJS_SERVER) $(OBJS_CLIENT) $(EXEC_SERVER) $(EXEC_CLIENT)

# Запуск сервера
run_server: $(EXEC_SERVER)
	./$(EXEC_SERVER)

# Запуск клиента
run_client: $(EXEC_CLIENT)
	./$(EXEC_CLIENT)

