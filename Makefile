# Pumpkin's

# (re)define default variables.
CC  = clang
CXX = clang
#CC = g++
#CC = gcc
#CXX = g++
CFLAGS   = -DDEBUG -Wall -pedantic -g
CXXFLAGS = -DDEBUG -Wall -pedantic -g -std=c++11
LDLIBS = -lstdc++ -lSDL2 -lSDL2_ttf -llua
$(RM) = rm -f

SRCS = main.cpp sdl.cpp sprite.cpp textrenderer.cpp grid.cpp tileset.cpp window.cpp socket.cpp luaconfig.cpp
OBJS = $(SRCS:.cpp=.o)
EXE = hkc

all : $(EXE)

$(EXE) : $(OBJS)
	$(CXX) $(LDLIBS) -o $(EXE) $(OBJS)

%.o : %.cpp %.h

.PHONY : clean clear

clean :
	$(RM) *.o

clear : clean
	$(RM) $(EXE)

