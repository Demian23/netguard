CPPC=g++
CPPFLAGS=-Wall -g -std=c++11
SRC=src
OBJ=build/obj
BIN_DIR=build/bin
BIN=$(BIN_DIR)/netguard
SRCS=$(wildcard $(SRC)/*.cpp)
OBJS=$(patsubst $(SRC)/%.cpp, $(OBJ)/%.o, $(SRCS))

all:$(OBJ) $(BIN_DIR) $(BIN)

release:CPPFLAGS= -Wall -O1 -std=c++11
release: clean
release:$(OBJ) $(BIN_DIR) $(BIN)

$(BIN): $(OBJS)
	$(CPPC) $(CPPFLAGS) $^ -o $@ 

$(OBJ)/%.o: $(SRC)/%.cpp
	$(CPPC) $(CPPFLAGS) -c $< -o $@ 

$(OBJ):
	mkdir -p $@

$(BIN_DIR):
	mkdir -p $@

clean:
	$(RM) $(OBJ)/*
	$(RM) -rf $(BIN_DIR)/* 
