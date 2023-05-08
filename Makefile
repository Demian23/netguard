CPPC=g++
CPPFLAGS=-Wall -g -std=c++11
SRC=src
OBJ=build/obj
BIN_DIR=build/bin
BIN=$(BIN_DIR)/netguard
TEST_DIR=test
TEST=$(TEST_DIR)/test
SRCS=$(wildcard $(SRC)/*.cpp)
OBJS=$(patsubst $(SRC)/%.cpp, $(OBJ)/%.o, $(SRCS))
INC_TEST=-I/opt/homebrew/Cellar/googletest/1.13.0/include/
LIB_TEST=-L/opt/homebrew/Cellar/googletest/1.13.0/lib/ -lgtest -lpthread

all:$(OBJ) $(BIN_DIR) $(BIN)

release:CPPFLAGS= -Wall -O1 -std=c++11
release: clean
release:$(OBJ) $(BIN_DIR) $(BIN)

test: clean
test: CPPFLAGS= -Wall -g -std=c++14 $(INC_TEST) 
test: $(TEST)

$(TEST): $(OBJS)
	$(CPPC) $(CPPFLAGS) $(LIB_TEST) $^ $(TEST_DIR)/src/test.cpp -o $@
$(BIN): $(OBJS)
	$(CPPC) $(CPPFLAGS) $^ build/src/main.cpp -o $@ 

$(OBJ)/%.o: $(SRC)/%.cpp
	$(CPPC) $(CPPFLAGS) -c $< -o $@ 

$(OBJ):
	mkdir -p $@

$(BIN_DIR):
	mkdir -p $@

clean:
	$(RM) $(OBJ)/*
	$(RM) -rf $(BIN_DIR)/* 
	$(RM) $(TEST)
