CPPC=g++
CPPFLAGS= -Wall -g --std=c++14
SRC=src
OBJ=obj
SRCS=$(wildcard $(SRC)/*.cpp)
OBJS=$(patsubst $(SRC)/%.cpp, $(OBJ)/%.o, $(SRCS))

all:$(OBJ)
all:$(OBJS)

release:CPPFLAGS= -Wall -O2
release: clean
release:$(OBJ)
release:$(OBJS)

$(OBJ)/%.o: $(SRC)/%.cpp
	$(CPPC) $(CPPFLAGS) -c $< -o $@ 
$(OBJ):
	mkdir -p $(OBJ)
clean:
	$(RM) -r $(OBJ)/*
