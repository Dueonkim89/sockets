# Makefile

# Compiler and executables to be made
CC = g++
EXECS = serverM serverA serverB serverC client monitor
FLAGS = -Wall -std=c++17

all: $(EXECS)

# Build each executable
serverM: serverM.cpp
	$(CC) $(FLAGS) -o serverM serverM.cpp
	
serverA: serverA.cpp
	$(CC) $(FLAGS) -o serverA serverA.cpp

serverB: serverB.cpp
	$(CC) $(FLAGS) -o serverB serverB.cpp

serverC: serverC.cpp
	$(CC) $(FLAGS) -o serverC serverC.cpp
	
client: client.cpp
	$(CC) $(FLAGS) -o client client.cpp
	
monitor: monitor.cpp
	$(CC) $(FLAGS) -o monitor monitor.cpp

# rm -f serverM serverA serverB serverC client monitor
clean:
	rm -f $(EXECS)