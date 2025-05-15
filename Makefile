CC = g++
CFLAGS = -g -O0 -Wall
LIBS = -lcryptopp
LDFLAGS =-lcryptopp

SRCS_SERVER = main.cpp Server.cpp ErrorLog.cpp
SRCS_CLIENT = Client.cpp main_client.cpp ErrorLog.cpp
OBJS_SERVER = $(SRCS_SERVER:.cpp=.o)
OBJS_CLIENT = $(SRCS_CLIENT:.cpp=.o)

EXEC_SERVER = server
EXEC_CLIENT = client

all: $(EXEC_SERVER) $(EXEC_CLIENT)

$(EXEC_SERVER): $(OBJS_SERVER)
	$(CC) $(CFLAGS) $(OBJS_SERVER) $(LIBS) -o $(EXEC_SERVER)

$(EXEC_CLIENT): $(OBJS_CLIENT)
	$(CC) $(CFLAGS) $(OBJS_CLIENT) $(LIBS) -o $(EXEC_CLIENT)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS_SERVER) $(OBJS_CLIENT) $(EXEC_SERVER) $(EXEC_CLIENT)

run_server: $(EXEC_SERVER)
	./$(EXEC_SERVER)

run_client: $(EXEC_CLIENT)
	./$(EXEC_CLIENT)

