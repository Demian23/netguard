CPPC=g++
CPPFLAGS= -Wall -g
SRC=src/modules
OBJ_AUX=../aux/obj
OBJ_EVENT=../events/obj
OBJ_AUTH=../authentication/obj
OBJ_BOT=../net_bot/obj
OBJ=obj/
OBJS=$(patsubst $(SRC)/%.cpp, $(OBJ)/%.o, $(SRCS))
SRCS=$(wildcard $(SRC)/*.cpp)
OBJS_AUX=$(wildcard $(OBJ_AUX)/*.o)
OBJS_EVENT=$(wildcard $(OBJ_EVENT)/*.o)
OBJS_BOT=$(wildcard $(OBJ_BOT)/*.o)
OBJS_AUTH=$(wildcard $(OBJ_AUTH)/*.o)

BINDIR=bin
BIN=$(BINDIR)/net_guard
BIN_BOT=$(BINDIR)/net_bot

CPPFLAGS_BOT=--std=c++14 -Wall -lTgBot -lboost_system -lssl -lcrypto -lpthread
LIB_BOOST=/opt/homebrew/Cellar/boost/1.81.0_1/lib/
LIB_SSL=/opt/homebrew/Cellar/openssl@3/3.0.8/lib/
AUX_INCLUDE=/usr/local/include/

all:$(BINDIR) $(OBJ) $(BIN)

release:CPPFLAGS= -Wall -O2
release: clean
release:$(OBJ) $(BINDIR) $(BIN)

net_bot:$(BINDIR)
net_bot:$(OBJ)
net_bot:$(BIN_BOT)

$(BIN_BOT): $(OBJS_BOT) $(OBJS_AUX) $(OBJS_EVENT) $(OBJS)
	$(CPPC) $(CPPFLAGS_BOT) $^ -o $@ -I$(AUX_INCLUDE) -L$(LIB_BOOST) -L$(LIB_SSL) 


$(BIN): $(OBJS_AUX) $(OBJS_EVENT) $(OBJS_AUTH) $(OBJS)
	$(CPPC) $(CPPFLAGS) $^ src/main.cpp -o $@

$(BINDIR):
	mkdir -p $(BINDIR)

$(OBJ):
	mkdir -p $(OBJ)

$(OBJ)/%.o: $(SRC)/%.cpp
	$(CPPC) $(CPPFLAGS) -c $< -o $@ 

clean:
	$(RM) $(OBJ)/* $(BIN) $(BIN_BOT)
	$(RM) -rf $(BIN).dSYM $(BIN_BOT).dSYM
