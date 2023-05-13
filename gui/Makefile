CPPC=g++
CPPFLAGS=-Wall -g -std=c++11 $(INC_GUI) 
SRC=src
OBJ=obj
SRCS=$(wildcard $(SRC)/*.cpp)
OBJS=$(patsubst $(SRC)/%.cpp, $(OBJ)/%.o, $(SRCS))
INC_GUI=-I/opt/homebrew/Cellar/fltk/1.3.8_1/include/
LIB_GUI=-L/opt/homebrew/Cellar/fltk/1.3.8_1/lib/ -lfltk

all:$(OBJ) $(OBJS) 

release: CPPFLAGS=-Wall -O2 -std=c++11 $(INC_GUI)
release:$(OBJ) $(OBJS)

$(BIN): $(OBJS)
	$(CPPC) $(CPPFLAGS) -o $@ 

$(OBJ)/%.o: $(SRC)/%.cpp
	$(CPPC) $(CPPFLAGS) -c $< -o $@ 

$(OBJ):
	mkdir -p $@

gui: 
	fluid -c $(SRC)/gui.fl
	mv gui.cpp src/old_code/gui_cpp.o
	mv gui.h src/old_code/gui_h.o

clean: 
	$(RM) -rf $(OBJ)/*
