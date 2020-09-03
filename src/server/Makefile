CC := gcc
SERVER := server
SERVER_SOURCES := main.c\
									input_handling.c\
									init_shut.c\
									thread_actions.c\
									network/network_control.c\
									network/network_accept.c\
									network/network_cl_handling.c\
									network/network_dist.c\
									network/network_sync.c
SERVER_LIBS := -lpthread -lrt

.PHONY: all
all: server_exec clean

server_debug: $(SERVER_SOURCES)
		@$(CC) $(SERVER_SOURCES) -o $(SERVER) $(SERVER_LIBS) -g

bins: server

server: $(SERVER_SOURCES)
		@$(CC) $(SERVER_SOURCES) -o $(SERVER) $(SERVER_LIBS) -ansi -pedantic -Wall

server_exec: $(SERVER_SOURCES)
		@$(CC) $(SERVER_SOURCES) -o $(SERVER) $(SERVER_LIBS) -ansi -pedantic -Wall
		@./$(SERVER)
		@rm -rf $(SERVER)

.PHONY: clean
clean:
		@rm -rf $(SERVER)

.PHONY: help
help:
	@echo ''
	@echo ' Net-PacMan server program.'
	@echo ''
	@echo ' make              - Compile and execute server binadry.'
	@echo '                     Binary will be removed after exiting the program;'
	@echo ' make server       - Compile server binadry. No execution and removal;'
	@echo ' make server_debug - Compile binary with debuggin info.'
	@echo '                     No execution and removal;'
	@echo ' make clean        - Remove generated files in current directory.'
	@echo '                     Only the source .c and .h files will remain;'
	@echo ' make help         - Print this message.'
	@echo ''
