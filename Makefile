CC = gcc
BIN = bin/
SRC = src/
EXEC = k_means
THREADS = 8

CFLAGS = -O2 -Wall

.DEFAULT_GOAL = k_means

k_means: $(SRC)k_means.c
	$(CC) $(CFLAGS) $(SRC)k_means.c -o $(BIN)$(EXEC)

clean:
	rm -r bin/*

runseq:
./$(BIN)$(EXEC) 10000000 $(CP_CLUSTERS)

runpar:
./$(BIN)$(EXEC) 10000000 $(CP_CLUSTERS) $THREADS
