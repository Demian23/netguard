CPPC=g++
CPPFLAGS= -Wall -g
OBJ_AUX=../net/obj
OBJ_EVENT=../events/obj
OBJ_AUTH=../authentication/obj
OBJS_AUX=$(wildcard $(OBJ_AUX)/*.o)
OBJS_EVENT=$(wildcard $(OBJ_EVENT)/*.o)
OBJS_AUTH=$(wildcard $(OBJ_AUTH)/*.o)

BINDIR=bin
BIN=$(BINDIR)/test

all:$(BINDIR) $(BIN)

release:CPPFLAGS= -Wall -O2
release: clean
release:$(BINDIR) $(BIN)

$(BIN): $(OBJS_AUX) $(OBJS_EVENT) $(OBJS_AUTH)
	$(CPPC) $(CPPFLAGS) $^ src/test.cpp -o $@

$(BINDIR):
	mkdir -p $(BINDIR)

clean:
	$(RM) $(BIN)
	$(RM) -rf $(BIN).dSYM
