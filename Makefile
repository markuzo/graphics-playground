all: main

SOURCES  = main.cpp 
SOURCES += thirdparty/imgui/imgui.cpp
SOURCES += thirdparty/imgui/imgui_widgets.cpp
SOURCES += thirdparty/imgui/imgui_draw.cpp
SOURCES += thirdparty/imgui/examples/imgui_impl_glfw.cpp
SOURCES += thirdparty/imgui/examples/imgui_impl_opengl3.cpp

INCLUDES  = -Ithirdparty/imgui/
INCLUDES += -Ithirdparty/imgui/examples

LIBRARIES  = -lglfw
LIBRARIES += -lGLEW
LIBRARIES += -lGL

DEFINES = -DIMGUI_IMPL_OPENGL_LOADER_GLEW

main: main.cpp
	g++ -g --std=c++17 $(SOURCES) $(INCLUDES) $(DEFINES) -o main $(LIBRARIES)

clean: 
	rm main

run: all
	./main

# mispelled this way too much...
urn: run

gdb: all
	gdb main
