CPPC=g++
CPPFLAGS= -Wall -g
SRC=main.cpp
OBJ_AUX=../aux/obj
OBJ_EVENT=../events/obj
SRCS=$(wildcard $(SRC)/*.cpp)
OBJS_AUX=$(wildcard $(OBJ_AUX)/*.o)
OBJS_EVENT=$(wildcard $(OBJ_EVENT)/*.o)
BINDIR=bin
BIN=$(BINDIR)/net_guard

all:$(BINDIR)
all:$(BIN)

release:CPPFLAGS= -Wall -O2
release: clean
release:$(BINDIR)
release:$(BIN)

$(BIN): $(OBJS_AUX) $(OBJS_EVENT)
	$(CPPC) $(CPPFLAGS) $^ $(SRC) -o $@

$(BINDIR):
	mkdir -p $(BINDIR)
clean:
	$(RM) $(BIN)
	$(RM) -rf $(BIN).dSYM
