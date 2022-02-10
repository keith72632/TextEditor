CC = g++
FLAGS = -Wall -Wextra
SRCS = $(wildcard *.cpp)
OBJS = $(SRCS:.cpp=.o)
HEADERS = $(wildcard *.h)

all: run

%.o: %.cpp
	${CC} ${FLAGS} -c $^ -o $@

mary.o: mary.cpp
	${CC} ${FLAGS} -c $^ -o $@

mary: ${SRCS} 
	${CC} $^ -o $@

run: mary
	./mary 

test: mary
	./mary Makefile

clean:
	rm *.o mary
