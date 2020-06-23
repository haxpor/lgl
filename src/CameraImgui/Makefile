# adapted from https://github.com/ocornut/imgui/blob/master/examples/example_glfw_opengl3/Makefile
# as it supports glfw + opengl3

EXE = cameraimgui

SOURCES = main.cpp
SOURCES += imgui.cpp imgui_demo.cpp imgui_draw.cpp imgui_widgets.cpp
SOURCES += imgui_impl_glfw.cpp imgui_impl_opengl3.cpp
SOURCES += ../../externals/glad/src/glad.c
SOURCES += ../../src/lgl/Error.cpp ../../src/lgl/Shader.cpp ../../src/lgl/Util.cpp

OBJS = $(addsuffix .o, $(basename $(notdir $(SOURCES))))

CXX = g++
CXXFLAGS = -g -Wall -Wextra -pedantic -fno-exceptions -Wno-stringop-overflow -Wno-unused-parameter -std=c++11
CXXFLAGS += -I../../externals/glad/include -I../../externals/stb_image -I../../externals -I../../includes -I./
CXXLDFLAGS = -lglfw -lGL -lX11 -lpthread -lm -ldl

%.o:%.cpp
	$(CXX) $(CXXFLAGS) -D IMGUI_IMPL_OPENGL_LOADER_GLAD -c -o $@ $<

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
