CPPC = g++
CPPFLAGS = -g -Wall
SRC=src
OBJ=obj
SRCS=$(wildcard $(SRC)/*.cpp)
OBJS=$(patsubst $(SRC)/%.cpp, $(OBJ)/%.o, $(SRCS))
SUBMITSRCNAME=events.zip

all:$(OBJ)
all: $(OBJS)

release: CFLAGS = -O2
release:clean
release:$(OBJ)
release: $(OBJS)

$(OBJ):
	mkdir -p $(OBJ)

$(OBJ)/%.o: $(SRC)/%.cpp
	$(CPPC) $(CPPFLAGS) -c $< -o $@ 
clean:
	$(RM) -r $(OBJ)/*
submit_src: clean
submit_src:
	zip -r $(SUBMITSRCNAME) $(SRC) $(OBJ) Makefile
