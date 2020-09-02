CC := gcc
SERVER := server
SERVER_SOURCES := main.c input_handling.c client_thread.c init_shut.c network_control.c
SERVER_LIBS := -lpthread -lrt

.PHONY: all
all: server_exec clean

server_debug: $(SERVER_SOURCES)
		@$(CC) $(SERVER_SOURCES) -o $(SERVER) $(SERVER_LIBS) -g
		@gdb $(SERVER)
		@rm -rf $(SERVER)

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
	@echo ' make server_debug - Compile and execute for debugging via gdb.'
	@echo '                     Binary will be removed after exiting debugging;'
	@echo ' make clean        - Remove generated files in current directory.'
	@echo '                     Only the source .c and .h files will remain;'
	@echo ' make help         - Print this message.'
	@echo ''
