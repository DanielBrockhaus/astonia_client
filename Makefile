all: moac.exe

CC=gcc
CFLAGS=-O3 -ggdb -Wall -Wno-pointer-sign -Wno-char-subscripts
LDFLAGS=-O3 -ggdb -Wl,-subsystem,windows
LIBS = -lwsock32 -lws2_32 -lz -lpng -lsdl2 -lSDL2_mixer -lsdl2main -lzip

OBJS	=	gui.o client.o skill.o dd.o font.o main.o sprite.o game.o\
		sound.o questlog.o resource.o sdl.o

moac.exe:       $(OBJS)
		$(CC) $(LDFLAGS) -o moac.exe $(OBJS)  $(LIBS)

client.o:	client.c main.h client.h sound.h
dd.o:		dd.c main.h dd.h client.h sdl.h
font.o:		font.c dd.h
game.o:       	game.c main.h dd.h client.h sprite.h gui.h sound.h
gui.o:		gui.c gui.h main.h dd.h client.h skill.h sprite.h sdl.h sound.h
main.o:		main.c main.h dd.h client.h sound.h gui.h sdl.h sprite.h
questlog.o:	questlog.c dd.h client.h sprite.h gui.h main.h sound.h
sdl.o:		sdl.c sdl.h main.h sound.h
skill.o:      	skill.c main.h skill.h client.h
sound.o:      	sound.c main.h sound.h dd.h
sprite.o:	sprite.c main.h sprite.h client.h

resource.o:	resource.rc
		windres -F pe-x86-64 resource.rc resource.o

clean:
		rm *.o
		rm moac.exe

