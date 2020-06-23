EXE = a.out

SOURCES = main.cpp

OBJS = $(addsuffix .o, $(basename $(notdir $(SOURCES))))

CXX = g++
CXXFLAGS = -g -Wall -Wextra -pedantic -fno-exceptions -Wno-stringop-overflow -Wno-unused-parameter -std=c++11
CXXFLAGS += -I../../includes -I./ -I../../externals
CXXLDFLAGS = -lGL -lX11 -lpthread -lm -ldl

%.o:%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

all: $(EXE)
	@echo Build complete

$(EXE): $(OBJS)
	$(CXX) -o $@ $^ $(CXXLDFLAGS)

clean:
	rm -f $(EXE) $(OBJS)
