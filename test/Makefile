CPPC=g++
CPPFLAGS=-Wall -g -std=c++14 $(INC_TEST)
INC_TEST=-I/opt/homebrew/Cellar/googletest/1.13.0/include/
LIB_TEST=-L/opt/homebrew/Cellar/googletest/1.13.0/lib/ -lgtest -lpthread
OBJ_SRCS=../srcs/obj/
OBJS_SRCS=$(wildcard $(OBJ_SRCS)/*.o)
BINDIR=bin
BIN=$(BINDIR)/test

all:$(BINDIR) $(BIN) 

$(BIN): $(OBJS_SRCS)
	$(CPPC) $(CPPFLAGS) $(LIB_TEST) $^ src/test.cpp -o $@ 

$(BINDIR):
	mkdir -p $@

clean:
	$(RM) -r $(BINDIR)/*
