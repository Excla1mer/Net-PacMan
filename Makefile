CC := gcc
FLAGS:= -Wall

SERVER := bin/server
SERVER_SOURCES := src/server/main.c\
									src/server/input_handling.c\
									src/server/init_shut.c\
									src/server/thread_actions.c\
									src/server/network/network_control.c\
									src/server/network/network_accept.c\
									src/server/network/network_cl_handling.c\
									src/server/network/network_dist.c\
									src/server/network/network_sync.c
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

client:
	@$(CC) $(CLIENT_SOURCES) -o $(CLIENT) $(CLIENT_LIBS) $(FLAGS)

client_exec:
	@$(CC) $(CLIENT_SOURCES) -o $(CLIENT) $(CLIENT_LIBS) $(FLAGS)
	@./$(CLIENT)
	@rm -rf $(CLIENT)


clean:
	@rm -rf *.o
	@rm -rf $(SERVER) $(CLIENT)

.PHONY: help
help:
	@echo '----------------------------------------------------------------------'
	@echo ' Net-PacMan program.'
	@echo '----------------------------------------------------------------------'
	@echo ' Common targets:'
	@echo '----------------------------------------------------------------------'
	@echo ' make              - Compile server and client binadry;'
	@echo ' make clean        - Remove generated files in current directory.'
	@echo '                     Only the source .c and .h files will remain;'
	@echo ' make help         - Print this message.'
	@echo '----------------------------------------------------------------------'
	@echo ' Server targets:'
	@echo '----------------------------------------------------------------------'
	@echo ' make server_exec  - Compile and execute server binadry.'
	@echo '                     Binary will be removed after exiting the program;'
	@echo ' make server       - Compile server binadry. No execution and removal;'
	@echo ' make server_debug - Compile binary with debuggin info.'
	@echo '                     No execution and removal;'
	@echo '----------------------------------------------------------------------'
	@echo ' Client targets:'
	@echo '----------------------------------------------------------------------'
	@echo ' make client_exec  - Compile and execute client binadry.'
	@echo '                     Binary will be removed after exiting the program;'
	@echo ' make client       - Compile client binadry. No execution and removal;'
	@echo '----------------------------------------------------------------------'
