all: moac.exe

BCCPATH=c:\borland\bcc55

CC=gcc
CFLAGS=-O -g -Wall -Wno-pointer-sign -Wno-char-subscripts -m32
LDFLAGS=-O -g -m32
LIBS = -lgdi32 -lwsock32 -lws2_32 -lz -lpng -lddraw -ldsound -lcomctl32

moac.exe:       Makefile gui.o client.o skill.o dd.o font.o gfx.o main.o sprite.o game.o neuquant.o resource.res edit.o edit_tool.o sound.o questlog.o resource.o
		$(CC) $(LDFLAGS) -o moac.exe gui.o client.o skill.o dd.o neuquant.o font.o gfx.o main.o sprite.o game.o edit.o edit_tool.o sound.o questlog.o resource.o $(LIBS)

dd.o:		dd.c main.h dd.h
gfx.o:		gfx.c main.h dd.h neuquant.h
sprite.o:	sprite.c main.h sprite.h client.h
gui.o:		gui.c gui.h main.h dd.h client.h skill.h sprite.h resource.h
main.o:		main.c main.h dd.h client.h resource.h
font.o:		font.c dd.h
client.o:	client.c main.h client.h
skill.o:      	skill.c main.h skill.h client.h
game.o:       	game.c main.h dd.h client.h sprite.h gui.h spell.h edit.h
edit.o:       	edit.c main.h dd.h client.h gui.h edit.h edit.h sprite.h resource.h
edit_tool.o:  	edit_tool.c main.h edit.h
sound.o:      	sound.c main.h
questlog.o:	questlog.c main.h
neuquant.o:   	neuquant.c neuquant.h

resource.o:	resource.res
		windres -F pe-i386 resource.res resource.o

resource.res:   resource.rc resource.h
		$(BCCPATH)\bin\brcc32 -fo resource.res resource.rc

clean:
		rm *.o
		rm moac.exe

