CPPC=g++
CPPFLAGS= -Wall -g -std=c++11
SRC=src
OBJ=obj
SRCS=$(wildcard $(SRC)/*.cpp)
OBJS=$(patsubst $(SRC)/%.cpp, $(OBJ)/%.o, $(SRCS))
OBJ_AUX=../net/obj
OBJ_EVENT=../events/obj
OBJ_AUTH=../authentication/obj
OBJS_AUX=$(wildcard $(OBJ_AUX)/*.o)
OBJS_EVENT=$(wildcard $(OBJ_EVENT)/*.o)
OBJS_AUTH=$(wildcard $(OBJ_AUTH)/*.o)

BINDIR=bin
BIN=$(BINDIR)/net_guard

all:$(BINDIR) $(BIN)

release:CPPFLAGS= -Wall -O2 -std=c++11
release: clean
release:$(BINDIR) $(BIN)

$(BIN): $(OBJS_AUX) $(OBJS_EVENT) $(OBJS_AUTH) $(OBJS)
	$(CPPC) $(CPPFLAGS) $^ -o $@

$(OBJ)/%.o: $(SRC)/%.cpp
	$(CPPC) $(CPPFLAGS) -c $< -o $@ 

$(BINDIR):
	mkdir -p $(BINDIR)

$(OBJ):
	mkdir -p $(OBJ)

clean:
	$(RM) $(BIN) $(OBJ)/*
	$(RM) -rf $(BIN).dSYM
