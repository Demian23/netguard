CPPC=g++
CPPFLAGS=-Wall -g -std=c++11
OBJ_GUI=../gui/obj/
OBJS_GUI=$(wildcard $(OBJ_GUI)/*.o)
OBJ_SRCS=../srcs/obj/
OBJS_SRCS=$(wildcard $(OBJ_SRCS)/*.o)
LIB_GUI=-L/opt/homebrew/Cellar/fltk/1.3.8_1/lib/ -lfltk
BINDIR=bin
INC_GUI=-I/opt/homebrew/Cellar/fltk/1.3.8_1/include/
BIN=$(BINDIR)/netguard

all:$(BIN_DIR) $(BIN)

release:CPPFLAGS= -Wall -O1 -std=c++11
release:$(BIN_DIR) $(BIN)

$(BIN): $(OBJS_GUI) $(OBJS_SRCS) $(OBJS)
	$(CPPC) $(CPPFLAGS) $(INC_GUI) $(LIB_GUI) $^ src/main.cpp -o $@ 


$(BIN_DIR):
	mkdir -p $@

clean:
	$(RM) -rf $(BINDIR)/* 
