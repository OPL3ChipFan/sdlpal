TARGET = sdlpal

SOURCES  = . .. ../adplug
CFILES   = $(foreach dir, $(SOURCES), $(wildcard $(dir)/*.c))
CPPFILES = $(foreach dir, $(SOURCES), $(wildcard $(dir)/*.cpp))

OBJS = $(CFILES:.c=$(INTER).o) $(CPPFILES:.cpp=$(INTER).o)

PSPSDK = $(shell psp-config --pspsdk-path)

INCDIR = 
CFLAGS = -O3 -G0 -Wall -g -I/usr/local/pspdev/psp/include/SDL -DPSP=1 -std=gnu99 -D_GNU_SOURCE
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti -std=c++11 -I../
ASFLAGS = $(CFLAGS)

LIBDIR =
LIBS = -L/usr/local/pspdev/psp/lib -lSDL -lm -lGL -lpspvfpu -L/usr/local/pspdev/psp/sdk/lib \
-lpspdebug -lpspgu -lpspctrl -lpspge -lpspdisplay -lpsphprm -lpspsdk -lpsprtc -lpspaudio -lc \
-lpspuser -lpsputility -lpspkernel -lpspnet_inet -lpspirkeyb -lpsppower \
 -lstdc++

EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = sdlpal
#PSP_EBOOT_ICON = icon0.png
#PSP_EBOOT_PIC1 = PIC1.png

BUILD_PRX = 1
PSP_FW_VERSION = 371
#PSP_LARGE_MEMORY=1 

include $(PSPSDK)/lib/build.mak