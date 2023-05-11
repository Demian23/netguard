CPPC=g++
CPPFLAGS=-Wall -g -std=c++11
SRC=src
OBJ=obj
SRCS=$(wildcard $(SRC)/*.cpp)
OBJS=$(patsubst $(SRC)/%.cpp, $(OBJ)/%.o, $(SRCS))

all:$(OBJS)

release:CPPFLAGS= -Wall -O1 -std=c++11
release: clean
release:$(OBJS) 

$(OBJ)/%.o: $(SRC)/%.cpp
	$(CPPC) $(CPPFLAGS) -c $< -o $@ 

$(OBJ):
	mkdir -p $@

clean:
	$(RM) $(OBJ)/*
