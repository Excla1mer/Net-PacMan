CC := gcc
FLAGS:= -Wall

SERVER := bin/server
SERVER_SOURCES := src/server/main.c src/server/input_handling.c\
	src/server/client_thread.c src/server/init_shut.c\
	src/server/network_control.c
SERVER_LIBS := -lpthread -lrt

CLIENT := bin/client
CLIENT_SOURCES := src/client/client.c
CLIENT_LIBS := -lpthread -lcsfml-graphics -lcsfml-window -lcsfml-system

.PHONY: all

all: client server 

bins: server client

server: $(SERVER_SOURCES)
	@$(CC) $(SERVER_SOURCES) -o $(SERVER) $(SERVER_LIBS) $(FLAGS)

server_exec: $(SERVER_SOURCES)
	@$(CC) $(SERVER_SOURCES) -o $(SERVER) $(SERVER_LIBS) $(FLAGS)
	@./$(SERVER)
	@rm -rf $(SERVER)

server_debug: $(SERVER_SOURCES)
	@$(CC) $(SERVER_SOURCES) -o $(SERVER) $(SERVER_LIBS) -g
	@gdb $(SERVER)
	@rm -rf $(SERVER)

client:
	@$(CC) $(CLIENT_SOURCES) -o $(CLIENT) $(CLIENT_LIBS) $(FLAGS)

client_exec:
	@$(CC) $(CLIENT_SOURCES) -o $(CLIENT) $(CLIENT_LIBS) $(FLAGS)
	@./$(CLIENT)
	@rm -rf $(CLIENT)


clean:
	@rm -rf *.o
	@rm -rf $(SERVER) $(CLIENT)

help_server:
	@echo ''
	@echo ' Net-PacMan server program.'
	@echo ''
	@echo ' make              - Compile and execute server binadry.'
	@echo '                     Binary will be removed after exiting the program;'
	@echo ' make server       - Compile server binadry. No execution and removal;'
	@echo ' make server_debug - Compile and execute for debugging via gdb.'
	@echo '                     Binary will be removed after exiting debugging;'
	@echo ' make clean        - Remove generated files in current directory.'
	@echo '                     Only the source .c and .h files will remain;'
	@echo ' make help         - Print this message.'
	@echo ''
