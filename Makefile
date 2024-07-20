PKGS=glfw3 freetype2
CC=clang
CXX=clang++
CXXFLAGS=-Wall -Wextra -stdlib=libc++ --std=c++17
EXTRACXXFLAGS=-I dependencies/glad/include -I dependencies/tree-sitter/lib/include `pkg-config --cflags $(PKGS)`
LIBS=`pkg-config --static --libs $(PKGS)`
SRCS=src/Editor.cpp src/EditorStorage.cpp src/FileManager.cpp src/Font.cpp src/Main.cpp src/Renderer.cpp src/Window.cpp bin/int/glad.o bin/int/tree-sitter.o

release: bin bin/int bin/dce

debug: CXXFLAGS += -DDEBUG -g
debug: bin bin/int bin/dce

bin:
	mkdir -p bin

bin/int:
	mkdir -p bin/int

clean:
	rm -r bin

bin/dce: $(SRCS)
	$(CXX) $(CXXFLAGS) $(EXTRACXXFLAGS) -o bin/dce $(SRCS) $(LIBS)

bin/int/glad.o:
	$(CC) -I dependencies/glad/include -o bin/int/glad.o -c dependencies/glad/src/glad.c

bin/int/tree-sitter.o:
	$(CC) -I dependencies/tree-sitter/lib/include -I dependencies/tree-sitter/lib/src \
					-o bin/int/tree-sitter.o -c dependencies/tree-sitter/lib/src/lib.c
