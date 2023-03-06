CPPC=g++
CPPFLAGS= -Wall -g
SRC=src
OBJ=obj
SRCS=$(wildcard $(SRC)/*.cpp)
OBJS=$(patsubst $(SRC)/%.cpp, $(OBJ)/%.o, $(SRCS))
SUBMITSRCNAME=net_aux.zip

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
	$(RM) $(SUBMITSRCNAME)
submit_src: clean
submit_src:
	zip -r $(SUBMITSRCNAME) $(SRC) $(OBJ) $(BINDIR) Makefile
