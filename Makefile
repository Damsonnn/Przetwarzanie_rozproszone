SOURCES=$(wildcard *.cpp)
HEADERS=$(SOURCES:.cpp=.h)
FLAGS=-DDEBUG -g

all: main

main: $(SOURCES) $(HEADERS)
	mpicxx $(SOURCES) $(FLAGS) -o main 

clear: clean

clean:
	rm main

run: main
	mpirun -np 10 -oversubscribe ./main
