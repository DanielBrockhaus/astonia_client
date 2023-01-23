all: moac.exe anicopy.exe

CC=gcc
CFLAGS=-O3 -ggdb -Wall -Wno-pointer-sign -Wno-char-subscripts
LDFLAGS=-O3 -ggdb -Wl,-subsystem,windows
#-Wl,-subsystem,windows
LIBS = -lwsock32 -lws2_32 -lz -lpng -lsdl2 -lSDL2_mixer -lsdl2main -lzip

OBJS	=		gui/gui.o client/client.o client/skill.o game/dd.o game/font.o\
			game/main.o game/sprite.o game/game.o\
			sdl/sound.o game/resource.o sdl/sdl.o helper/helper.o\
			gui/dots.o gui/display.o gui/teleport.o gui/color.o gui/cmd.o gui/questlog.o

moac.exe:       	$(OBJS)
			$(CC) $(LDFLAGS) -o moac.exe $(OBJS)  $(LIBS)

anicopy.exe:		helper/anicopy.c
			$(CC) -O3 -ggdb -Wall -o anicopy.exe helper/anicopy.c

client/client.o:	client/client.c astonia.h client.h client/_client.h sdl.h

game/dd.o:		game/dd.c astonia.h game.h game/_game.h client.h sdl.h
game/font.o:		game/font.c game.h game/_game.h
game/game.o:    	game/game.c astonia.h game.h game/_game.h client.h gui.h
game/main.o:		game/main.c astonia.h game.h game/_game.h client.h gui.h sdl.h
game/skill.o:      	game/skill.c astonia.h game.h game/_game.h client.h
game/sprite.o:		game/sprite.c astonia.h game.h game/_game.h client.h gui.h

gui/color.o:		gui/color.c astonia.h gui.h gui/_gui.h client.h game.h
gui/cmd.o:		gui/cmd.c astonia.h gui.h gui/_gui.h client.h game.h sdl.h
gui/dots.o:		gui/dots.c astonia.h gui.h gui/_gui.h
gui/display.o:		gui/display.c astonia.h gui.h gui/_gui.h client.h game.h
gui/gui.o:		gui/gui.c astonia.h gui.h gui/_gui.h client.h game.h gui.h sdl.h
gui/teleport.o:		gui/teleport.c astonia.h gui.h gui/_gui.h client.h game.h
gui/questlog.o:		gui/questlog.c astonia.h gui.h gui/_gui.h client.h game.h

helper/helper.o:	helper/helper.c astonia.h

sdl/sdl.o:		sdl/sdl.c astonia.h sdl.h sdl/_sdl.h
sdl/sound.o:      	sdl/sound.c astonia.h sdl.h sdl/_sdl.h

game/resource.o:	game/resource.rc game/resource.h
			windres -F pe-x86-64 game/resource.rc game/resource.o

clean:
		rm client/*.o game/*.o gui/*.o helper/*.o sdl/*.o
		rm moac.exe

dlls:
	ldd moac.exe | grep mingw

