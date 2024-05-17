PKGS=glfw3 freetype2
CC=gcc
CFLAGS=-Wall -Wextra -std=c11 -Idependencies/glad/include `pkg-config --cflags $(PKGS)`
LIBS=`pkg-config --static --libs $(PKGS)` -lm
SRCS=src/app.c src/window.c src/renderer.c src/font.c src/file-manager.c dependencies/glad/src/glad.c

release: dce

debug: 
	CFLAGS += -DDEBUG
debug: dce

dce: $(SRCS)
	$(CC) $(CFLAGS) -o bin/dce $(SRCS) $(LIBS)
