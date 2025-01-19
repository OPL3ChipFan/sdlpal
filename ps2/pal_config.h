#ifndef PAL_CONFIG_H
# define PAL_CONFIG_H

#ifndef PS2
# define PS2
#endif


#ifdef PS2

# define PAL_HAS_JOYSTICKS     1
# define PAL_HAS_MOUSE         0
# define PAL_HAS_MP3           0
# define PAL_HAS_OGG           0
# define PAL_HAS_OPUS          0

# define PAL_PREFIX            ""
# define PAL_SAVE_PREFIX       ""

# define PAL_DEFAULT_WINDOW_WIDTH   640
# define PAL_DEFAULT_WINDOW_HEIGHT  400

# if SDL_VERSION_ATLEAST(2,0,0)
#  define PAL_VIDEO_INIT_FLAGS  (SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE)
# else
#  define PAL_VIDEO_INIT_FLAGS  (SDL_HWSURFACE | (gConfig.fFullScreen ? SDL_FULLSCREEN : 0))
# endif

# define PAL_SDL_INIT_FLAGS	(SDL_INIT_VIDEO | SDL_INIT_AUDIO)
//# define PAL_SDL_INIT_FLAGS   ( SDL_INIT_AUDIO | SDL_INIT_JOYSTICK)

# define PAL_SCALE_SCREEN     TRUE

# define PAL_PLATFORM         "PS2"
# define PAL_CREDIT           "SDLPal Team"
# define PAL_PORTYEAR         "2010-2025"

# define PAL_FILESYSTEM_IGNORE_CASE         1
# define PAL_HAS_PLATFORM_SPECIFIC_UTILS    1
# define PAL_HAS_PLATFORM_STARTUP           1

# define PAL_FORCE_UPDATE_ON_PALETTE_SET    1

# include <strings.h>
# include <ctype.h>
# include <sys/time.h>
# include <alloca.h>
# define SDL_strncasecmp strncasecmp
# define VERS 2
# define REVS 0

#ifndef PATH_MAX
# define PATH_MAX 320
#endif

# define PAL_FATAL_OUTPUT(s) printf(s)
#endif

#endif

