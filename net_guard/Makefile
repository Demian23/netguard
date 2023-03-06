CPPC=g++
CPPFLAGS= -Wall -g
SRC=src/
OBJ_AUX=../aux/obj
OBJ_EVENT=../events/obj
OBJ=obj/
OBJS=$(patsubst $(SRC)/%.c, $(OBJ)/%.o, $(SRCS))
SRCS=$(wildcard $(SRC)/*.cpp)
OBJS_AUX=$(wildcard $(OBJ_AUX)/*.o)
OBJS_EVENT=$(wildcard $(OBJ_EVENT)/*.o)
BINDIR=bin
BIN=$(BINDIR)/net_guard

all:$(BINDIR)
all:$(OBJ)
all:$(BIN)

release:CPPFLAGS= -Wall -O2
release: clean
release:$(OBJ)
release:$(BINDIR)
release:$(BIN)

$(BIN): $(OBJS_AUX) $(OBJS_EVENT) $(OBJS)
	$(CPPC) $(CPPFLAGS) $^ -o $@

$(BINDIR):
	mkdir -p $(BINDIR)
$(OBJ):
	mkdir -p $(OBJ)
$(OBJ)/%.o: $(SRC)/%.cpp
	$(CPPC) $(CPPFLAGS) -c $< -o $@ 
clean:
	$(RM) $(OBJ)/* $(BIN)
	$(RM) -rf $(BIN).dSYM
