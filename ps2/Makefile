EE_BIN = SDLPAL.ELF

SOURCES  = . .. ../adplug
CFILES   = $(foreach dir, $(SOURCES), $(wildcard $(dir)/*.c))
CPPFILES = $(foreach dir, $(SOURCES), $(wildcard $(dir)/*.cpp))

EE_OBJS = $(CFILES:.c=$(INTER).o) $(CPPFILES:.cpp=$(INTER).o)

EE_INCS = -I$(PS2SDK)/ports/include/SDL2 -I$(PS2SDK)/ports/include

EE_LDFLAGS = -L$(PS2SDK)/ports/lib -L$(PS2DEV)/gsKit/lib 

EE_CFLAGS = -DUSE_RWOPS -DHAVE_CONFIG_H -DHAVE_MIXER -O2 -G0 -Wall -g -std=gnu99 -D_GNU_SOURCE -DHAVE_CONFIG_H

EE_CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti -std=c++11 -I. -I../
EE_ASFLAGS = $(CFLAGS)



LIBDIR =

EE_LIBS = -L. -L$(GSKIT)/lib -L../lib -lSDL2 -lps2_drivers -lpatches -lkernel -lgskit -laudsrv -ldmakit -lcdvd -lps2ip -ldebug -lconfig -lmc -lc -lhdd -lfileXio -lpoweroff -lnetman -lstdc++ -lgskit -ldmakit -lpoweroff -lfileXio -lpatches -lpng -lz -lmc -lfreetype -lvux -lcdvd -lnetman -lps2ips -laudsrv -lvorbisfile -lvorbis -logg -lpadx -lelf-loader-nocolour


all: $(EE_BIN)

iso: $(EE_BIN)
	cp SDLPAL.ELF ./iso/
	mkisofs  -l -o SDLPAL.ISO ./iso/*

clean:
	rm -f $(EE_OBJS) $(EE_BIN)

include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal

