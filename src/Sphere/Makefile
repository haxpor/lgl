EXE = sphere.out

SOURCES = main.cpp
SOURCES += Sphere.cpp
SOURCES += ../../externals/glad/src/glad.c
SOURCES += ../../src/lgl/Error.cpp ../../src/lgl/Shader.cpp ../../src/lgl/Util.cpp

OBJS = $(addsuffix .o, $(basename $(notdir $(SOURCES))))

CXX = g++
CXXFLAGS = -g -Wall -Wextra -pedantic -fno-exceptions -Wno-stringop-overflow -Wno-unused-parameter -std=c++11
CXXFLAGS += -I../../externals/glad/include -I../../externals/stb_image -I../../includes -I./
CXXLDFLAGS = -lglfw -lGL -lX11 -lpthread -lm -ldl

%.o:%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o:../../src/lgl/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o:../../externals/glad/src/%.c
	$(CXX) $(CXXFLAGS) -c -o $@ $<

all: $(EXE)
	@echo Build complete

$(EXE): $(OBJS)
	$(CXX) -o $@ $^ $(CXXLDFLAGS)

clean:
	rm -f $(EXE) $(OBJS)
