all: bin/moac.exe bin/anicopy.exe bin/amod.dll

CC=gcc
CFLAGS=-O3 -ggdb -Wall -Wno-pointer-sign -Wno-char-subscripts
LDFLAGS=-O3 -ggdb -Wl,-subsystem,windows
#-Wl,-subsystem,windows
LIBS = -lwsock32 -lws2_32 -lz -lpng -lsdl2 -lSDL2_mixer -lsdl2main -lzip

OBJS	=		src/gui/gui.o src/client/client.o src/client/skill.o src/game/dd.o src/game/font.o\
			src/game/main.o src/game/sprite.o src/game/game.o src/modder/modder.o\
			src/sdl/sound.o src/game/resource.o src/sdl/sdl.o src/helper/helper.o\
			src/gui/dots.o src/gui/display.o src/gui/teleport.o src/gui/color.o src/gui/cmd.o src/gui/questlog.o

bin/moac.exe lib/moac.a &:	$(OBJS)
			$(CC) $(LDFLAGS) -Wl,--out-implib,lib/moac.a -o bin/moac.exe $(OBJS) $(LIBS)

bin/amod.dll:		src/amod/amod.o lib/moac.a
			$(CC) $(LDFLAGS) -shared -o bin/amod.dll src/amod/amod.o lib/moac.a

src/amod/amod.o:	src/amod/amod.c src/amod/amod.h

bin/anicopy.exe:	src/helper/anicopy.c
			$(CC) -O3 -ggdb -Wall -o bin/anicopy.exe src/helper/anicopy.c

src/client/client.o:	src/client/client.c src/astonia.h src/client.h src/client/_client.h src/sdl.h

src/game/dd.o:		src/game/dd.c src/astonia.h src/game.h src/game/_game.h src/client.h src/sdl.h
src/game/font.o:	src/game/font.c src/game.h src/game/_game.h
src/game/game.o:    	src/game/game.c src/astonia.h src/game.h src/game/_game.h src/client.h src/gui.h
src/game/main.o:	src/game/main.c src/astonia.h src/game.h src/game/_game.h src/client.h src/gui.h src/sdl.h src/modder.h
src/game/skill.o:      	src/game/skill.c src/astonia.h src/game.h src/game/_game.h src/client.h
src/game/sprite.o:	src/game/sprite.c src/astonia.h src/game.h src/game/_game.h src/client.h src/gui.h

src/gui/color.o:	src/gui/color.c src/astonia.h src/gui.h src/gui/_gui.h src/client.h src/game.h
src/gui/cmd.o:		src/gui/cmd.c src/astonia.h src/gui.h src/gui/_gui.h src/client.h src/game.h src/sdl.h
src/gui/dots.o:		src/gui/dots.c src/astonia.h src/gui.h src/gui/_gui.h
src/gui/display.o:	src/gui/display.c src/astonia.h src/gui.h src/gui/_gui.h src/client.h src/game.h
src/gui/gui.o:		src/gui/gui.c src/astonia.h src/gui.h src/gui/_gui.h src/client.h src/game.h src/gui.h src/sdl.h src/modder.h
src/gui/teleport.o:	src/gui/teleport.c src/astonia.h src/gui.h src/gui/_gui.h src/client.h src/game.h
src/gui/questlog.o:	src/gui/questlog.c src/astonia.h src/gui.h src/gui/_gui.h src/client.h src/game.h

src/helper/helper.o:	src/helper/helper.c src/astonia.h

src/modder/modder.o:	src/modder/modder.c src/astonia.h src/modder.h src/modder/_modder.h

src/sdl/sdl.o:		src/sdl/sdl.c src/astonia.h src/sdl.h src/sdl/_sdl.h
src/sdl/sound.o:      	src/sdl/sound.c src/astonia.h src/sdl.h src/sdl/_sdl.h

src/game/resource.o:	src/game/resource.rc src/game/resource.h
			windres -F pe-x86-64 src/game/resource.rc src/game/resource.o

clean:
		rm src/client/*.o src/game/*.o src/gui/*.o helper/*.o src/sdl/*.o
		rm moac.exe

distrib:
	ldd bin/moac.exe | grep mingw | awk 'NF == 4 { system("cp " $3 " bin") }'

