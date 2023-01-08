all: moac.exe

CC=gcc
CFLAGS=-O3 -ggdb -Wall -Wno-pointer-sign -Wno-char-subscripts
LDFLAGS=-O3 -ggdb
LIBS = -lwsock32 -lws2_32 -lz -lpng -lsdl2 -lsdl2main

OBJS	=	gui.o client.o skill.o dd.o font.o main.o sprite.o game.o\
		sound.o questlog.o resource.o sdl.o

moac.exe:       $(OBJS)
		$(CC) $(LDFLAGS) -o moac.exe $(OBJS)  $(LIBS)

dd.o:		dd.c main.h dd.h sdl.h
gfx.o:		gfx.c main.h dd.h neuquant.h
sprite.o:	sprite.c main.h sprite.h client.h
gui.o:		gui.c gui.h main.h dd.h client.h skill.h sprite.h resource.h sdl.h
main.o:		main.c main.h dd.h client.h resource.h
font.o:		font.c dd.h
client.o:	client.c main.h client.h
skill.o:      	skill.c main.h skill.h client.h
game.o:       	game.c main.h dd.h client.h sprite.h gui.h spell.h
sound.o:      	sound.c main.h
questlog.o:	questlog.c main.h
neuquant.o:   	neuquant.c neuquant.h
sdl.o:		sdl.c sdl.h

resource.o:	resource.rc
		windres -F pe-x86-64 resource.rc resource.o

clean:
		rm *.o
		rm moac.exe

