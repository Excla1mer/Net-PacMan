CC := gcc
SERVER := server
SERVER_SOURCES := main.c\
									input_handling.c\
									init_shut.c\
									init_actions.c\
									network/network_control.c\
									network/network_accept.c\
									network/network_cl_handling.c\
									network/network_dist.c\
									network/network_sync.c
SERVER_LIBS := -lpthread -lrt

.PHONY: all
all: exec clean

debug: $(SERVER_SOURCES)
		@$(CC) $(SERVER_SOURCES) -o $(SERVER) $(SERVER_LIBS) -g

val_check:
		@$(CC) $(SERVER_SOURCES) -o $(SERVER) $(SERVER_LIBS) -g
		@valgrind -s --leak-check=full --track-origins=yes --show-leak-kinds=all ./server
		@rm -rf $(SERVER)

bin: server

bin: $(SERVER_SOURCES)
		@$(CC) $(SERVER_SOURCES) -o $(SERVER) $(SERVER_LIBS) -ansi -pedantic -Wall

exec: $(SERVER_SOURCES)
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
	@echo ' make						- Compile and execute server binadry.'
	@echo '                   Binary will be removed after exiting the program;'
	@echo ' make bin				- Compile server binadry. No execution and removal;'
	@echo ' make debug			- Compile and execute binary via valgrind'
	@echo '                   Binary will be removed after exiting the program;'
	@echo ' make val_check	- Compile binary with debuggin info.'
	@echo '                   No execution and removal;'
	@echo ' make clean 			- Remove generated files in current directory.'
	@echo '                   Only the source .c and .h files will remain;'
	@echo ' make help       - Print this message.'
	@echo ''
