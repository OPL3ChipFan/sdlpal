#include "../main.h"
#include <math.h>
#include <pspkernel.h>
#include <pspctrl.h>
#include <SDL_thread.h>

#include <pspdisplay.h>

#include <pspsdk.h>
#include <psppower.h>
#include <pspthreadman.h>
#include <pspmp3.h>
#include <pspaudio.h>
#include <psputility.h>
#include <stdlib.h>
#include <stdio.h>

#include "native_mp3.h"

#define __FILENO_MAX 1024
typedef struct {
	u_int32_t descriptor;
	u_int32_t flags;
	u_int32_t ref_count;
	char *filename;
	u_int8_t type;
} __descriptormap_type;

extern __descriptormap_type *__descriptormap[__FILENO_MAX];

#define	_FOPEN		(-1)	/* from sys/file.h, kernel use only */
#define	_FREAD		0x0001	/* read enabled */
#define	_FWRITE		0x0002	/* write enabled */
#define	_FNDELAY	0x0800	/* non blocking I/O (4.2 style) */
#define	_FAPPEND	0x0400	/* append (writes guaranteed at the end) */
#define	_FMARK		0x0010	/* internal; mark during gc() */
#define	_FDEFER		0x0020	/* internal; defer for next gc pass */
#define	_FASYNC		0x2000	/* signal pgrp when data ready */
#define	_FCREAT		0x0040	/* open with file create */
#define	_FTRUNC		0x0200	/* open with truncation */
#define	_FEXCL		0x0080	/* error on open if file exists */
#define	_FNBIO	    _FNONBLOCK	/* non blocking I/O (sys5 style) */
#define	_FSYNC		0x1000	/* do all writes synchronously */
#define	_FNONBLOCK	0x0800	/* non blocking I/O (POSIX style) */
#define	_FNOCTTY	0x0100	/* don't assign a ctty on this open */

#define	O_ACCMODE	(O_RDONLY|O_WRONLY|O_RDWR)

/*
 * Flag values for open(2) and fcntl(2)
 * The kernel adds 1 to the open modes to turn it into some
 * combination of FREAD and FWRITE.
 */
#define	O_RDONLY	0		/* +1 == FREAD */
#define	O_WRONLY	1		/* +1 == FWRITE */
#define	O_RDWR		2		/* +1 == FREAD|FWRITE */
#define	O_APPEND	_FAPPEND
#define	O_CREAT		_FCREAT
#define	O_TRUNC		_FTRUNC
#define	O_EXCL		_FEXCL
 /*	O_SYNC		_FSYNC		not posix, defined below */
 /*	O_NDELAY	_FNDELAY 	set in include/fcntl.h */
 /*	O_NDELAY	_FNBIO 		set in 5include/fcntl.h */
#define	O_NONBLOCK	_FNONBLOCK
#define	O_NOCTTY	_FNOCTTY

PSP_MODULE_INFO("SDLPAL", PSP_MODULE_USER, VERS, REVS);
PSP_MAIN_THREAD_ATTR(PSP_THREAD_ATTR_USER);
PSP_HEAP_SIZE_MAX();

static SceCtrlData pad;
static SDL_sem *pad_sem = 0;
// static SDL_Thread *bthread = 0;
// static int running = 0;
static unsigned int old_button = 0;
static unsigned char old_x = 0, old_y = 0;

int dynamic_freq = 0;

SceOff fileseekpoints[32];
//volatile SceOff tempRNGpos;

volatile int gamePaused = 0;

BOOL
UTIL_GetScreenSize(
	DWORD *pdwScreenWidth,
	DWORD *pdwScreenHeight
)
{
	return FALSE;
}

BOOL
UTIL_IsAbsolutePath(
	LPCSTR  lpszFileName
)
{
	return FALSE;
}

int PSP_resume_callback(int unknown, int pwrflags, void* common)
{
        
        //volatile static SceOff tempRNGpos;
        
	scePowerLock(0);
	UTIL_LogOutput(LOGLEVEL_INFO, "PSP_resume_callback pwrflags =  0x%x \n", pwrflags);
	if (pwrflags & PSP_POWER_CB_RESUME_COMPLETE)
	{
gamePaused = 0;
		UTIL_LogOutput(LOGLEVEL_INFO, "PSP_POWER_CB_RESUME_COMPLETE \n");
		// On certian PSP-1000 system, file handles will be invalid after PSP_POWER_CB_RESUME_COMPLETE 

		for (int i = 3; i < 13; i++) {
			if (__descriptormap[i]) {

				int scefd;
				int sce_flags;

				static volatile SceOff seekpos;
				//UTIL_LogOutput(LOGLEVEL_INFO, "%s seekpoint = 0x%x \n", __descriptormap[i]->filename, fileseekpoints[i]);
				
				/* O_RDONLY starts at 0, where PSP_O_RDONLY starts at 1, so remap the read/write
				   flags by adding 1. */
				sce_flags = (__descriptormap[i]->flags & O_ACCMODE) + 1;

				/* Translate standard open flags into the flags understood by the PSP kernel. */
				if (__descriptormap[i]->flags & O_APPEND) {
					sce_flags |= PSP_O_APPEND;
				}
				if (__descriptormap[i]->flags & O_CREAT) {
					sce_flags |= PSP_O_CREAT;
				}
				if (__descriptormap[i]->flags & O_TRUNC) {
					sce_flags |= PSP_O_TRUNC;
				}
				if (__descriptormap[i]->flags & O_EXCL) {
					sce_flags |= PSP_O_EXCL;
				}
				if (__descriptormap[i]->flags & O_NONBLOCK) {
					sce_flags |= PSP_O_NBLOCK;
				}

				scefd = sceIoOpen(__descriptormap[i]->filename, sce_flags, 0777);
				__descriptormap[i]->descriptor = scefd;
				
				//UTIL_LogOutput(LOGLEVEL_INFO, "RESUME fileseekpoint = 0x%lld \n", fileseekpoints[i]);
				UTIL_LogOutput(LOGLEVEL_INFO, "RESUME fileseekpoints[%d] = 0x%llX \n", i, fileseekpoints[i]);
				//UTIL_LogOutput(LOGLEVEL_INFO, "RESUME 1 tempRNGpos = 0x%lld \n", tempRNGpos);
				
				seekpos = sceIoLseek(__descriptormap[i]->descriptor, fileseekpoints[i], SEEK_SET);
				UTIL_LogOutput(LOGLEVEL_INFO, "seekpos = 0x%llX \n", sceIoLseek(__descriptormap[i]->descriptor, seekpos, SEEK_SET));
				
				//UTIL_LogOutput(LOGLEVEL_INFO, "2 tempRNGpos = 0x%lld \n", tempRNGpos);

				//seekpos = sceIoLseek(__descriptormap[i]->descriptor, 0, SEEK_CUR);
				//UTIL_LogOutput(LOGLEVEL_INFO, "sceIoLseek = 0x%lld \n", sceIoLseek(__descriptormap[i]->descriptor, 0, SEEK_CUR));
				
				//UTIL_LogOutput(LOGLEVEL_INFO, "3 tempRNGpos = 0x%lld \n", tempRNGpos);
			}
		}
		UTIL_LogOutput(LOGLEVEL_INFO, "RESUME MP3filename = %s \n", MP3filename);
		// Clear the filename cache. 
		memset(monitor_MP3filename, 0, PAL_MAX_PATH);
		// Cache the filename. 
		strcpy(monitor_MP3filename, MP3filename);

		getcwd(cwd_buff, PAL_MAX_PATH);
		UTIL_LogOutput(LOGLEVEL_INFO, "getcwd returned =  %s \n", cwd_buff);
		sceIoChdir(cwd_buff);

		playNativeMP3(monitor_MP3filename, current_floop, current_iMusicVolume);
		/*
		fclose(gpGlobals->f.fpFBP);
		gpGlobals->f.fpFBP = UTIL_OpenRequiredFile("fbp.mkf");
		fclose(gpGlobals->f.fpMGO);
		gpGlobals->f.fpMGO = UTIL_OpenRequiredFile("mgo.mkf");
		fclose(gpGlobals->f.fpBALL);
		gpGlobals->f.fpBALL = UTIL_OpenRequiredFile("ball.mkf");
		fclose(gpGlobals->f.fpDATA);
		gpGlobals->f.fpDATA = UTIL_OpenRequiredFile("data.mkf");
		fclose(gpGlobals->f.fpF);
		gpGlobals->f.fpF = UTIL_OpenRequiredFile("f.mkf");
		//fclose(gpGlobals->f.fpFIRE);
		//gpGlobals->f.fpFIRE = UTIL_OpenRequiredFile("fire.mkf");
		//fclose(gpGlobals->f.fpRGM);
		//gpGlobals->f.fpRGM = UTIL_OpenRequiredFile("rgm.mkf");
		fclose(gpGlobals->f.fpSSS);
		gpGlobals->f.fpSSS = UTIL_OpenRequiredFile("sss.mkf");
		*/
	}else if (pwrflags & PSP_POWER_CB_POWER_SWITCH) {
// || pwrflags & PSP_POWER_CB_SUSPENDING
		stopNativeMP3();
gamePaused=1;
		sceKernelDelayThread(1000000);
		for (int i = 3; i < 13; i++) {
			if (__descriptormap[i]) {
				UTIL_LogOutput(LOGLEVEL_INFO, "__descriptormap[%d]->filename =  %s \n", i, __descriptormap[i]->filename);
			}
		}
		for (int i = 3; i < 13; i++) {
			UTIL_LogOutput(LOGLEVEL_INFO, "PSP_POWER_CB_SUSPENDING i = %d \n", i);
			if (__descriptormap[i]) {
				// UTIL_LogOutput(LOGLEVEL_INFO, "ftell(%d) = %d \n", i, ftell(i));
				fileseekpoints[i] = sceIoLseek(__descriptormap[i]->descriptor, 0, SEEK_CUR);
				//tempRNGpos = fileseekpoints[i];
				UTIL_LogOutput(LOGLEVEL_INFO, "__descriptormap[%d]->filename =  %s \n", i, __descriptormap[i]->filename);
				UTIL_LogOutput(LOGLEVEL_INFO, "fileseekpoints[%d] = 0x%llx \n", i, fileseekpoints[i]);
				//UTIL_LogOutput(LOGLEVEL_INFO, "tempRNGpos SUSPENDING = 0x%lld \n", tempRNGpos);
				sceIoClose(__descriptormap[i]->descriptor);
			}
		}
	}

	int cbid;
	cbid = sceKernelCreateCallback("suspend Callback", PSP_resume_callback, NULL);
	scePowerRegisterCallback(0, cbid);
	scePowerUnlock(0);
	return 0;
}

int PSP_exit_callback(int arg1, int arg2, void* common)
{
	exit(0);
	return 0;
}

int PSP_callback_thread(SceSize args, void* argp)
{
	int cbid;
	cbid = sceKernelCreateCallback("Exit Callback", PSP_exit_callback, NULL);
	sceKernelRegisterExitCallback(cbid);
	cbid = sceKernelCreateCallback("suspend Callback", PSP_resume_callback, NULL);
	scePowerRegisterCallback(0, cbid);
	sceKernelSleepThreadCB();
	return 0;
}


int PSP_setup_callbacks(void)
{
	int thid = 0;
	thid = sceKernelCreateThread("update_thread", PSP_callback_thread, 0x11, 0xFA0, 0, 0);
	if (thid >= 0)
		sceKernelStartThread(thid, 0, 0);
	return thid;
}

int PSP_switch_frequency(int freq)
{
	if (dynamic_freq) {
		if (freq == 1) {
			scePowerSetClockFrequency(333, 333, 166);
		}
		else
		{
			scePowerSetClockFrequency(300, 300, 150);
		}
	}
	return 0;
}

void PAL_calc_Axes(
	unsigned char x,
	unsigned char y
)
{
	if (x < y && x + y < 51)
	{
		g_InputState.dwKeyPress = kKeyLeft;
		g_InputState.prevdir = g_InputState.dir;
		g_InputState.dir = kDirWest;
		return;
	}
	if (x < y && x + y>51)
	{
		g_InputState.dwKeyPress = kKeyDown;
		g_InputState.prevdir = g_InputState.dir;
		g_InputState.dir = kDirSouth;
		return;
	}
	if (x > y && x + y < 51)
	{
		g_InputState.dwKeyPress = kKeyUp;
		g_InputState.prevdir = g_InputState.dir;
		g_InputState.dir = kDirNorth;
		return;
	}
	if (x > y && x + y > 51)
	{
		g_InputState.dwKeyPress = kKeyRight;
		g_InputState.prevdir = g_InputState.dir;
		g_InputState.dir = kDirEast;
		return;
	}
	g_InputState.prevdir = (gpGlobals->fInBattle ? kDirUnknown : g_InputState.dir);
	g_InputState.dir = kDirUnknown;
}

static int input_event_filter(const SDL_Event* lpEvent, volatile PALINPUTSTATE* state)
{

	unsigned int button;
	unsigned char x, y;

	sceCtrlPeekBufferPositive(&pad, 1);

	SDL_SemWait(pad_sem);
	button = pad.Buttons;
	x = pad.Lx;
	y = pad.Ly;
	SDL_SemPost(pad_sem);

	//
	//Axes
	//
	x /= 5;
	y /= 5;
	BOOL onCenter = (x > 16 && x < 32) && (y > 16 && y < 32);
	if (!onCenter)
	{
		if (old_x != x || old_y != y)
		{
			PAL_calc_Axes(x, y);
			old_y = y;
			old_x = x;
		}
	}
	else if (!button)
	{
		g_InputState.prevdir = (gpGlobals->fInBattle ? kDirUnknown : g_InputState.dir);
		g_InputState.dir = kDirUnknown;
	}

	//
	//Buttons
	//
	int changed = (button != old_button);
	old_button = button;
	if (changed)
	{
		if (button & PSP_CTRL_UP)
		{
			g_InputState.prevdir = (gpGlobals->fInBattle ? kDirUnknown : g_InputState.dir);
			g_InputState.dir = kDirNorth;
			g_InputState.dwKeyPress = kKeyUp;
			return 1;
		}
		if (button & PSP_CTRL_DOWN)
		{
			g_InputState.prevdir = (gpGlobals->fInBattle ? kDirUnknown : g_InputState.dir);
			g_InputState.dir = kDirSouth;
			g_InputState.dwKeyPress = kKeyDown;
			return 1;
		}
		if (button & PSP_CTRL_LEFT)
		{
			g_InputState.prevdir = (gpGlobals->fInBattle ? kDirUnknown : g_InputState.dir);
			g_InputState.dir = kDirWest;
			g_InputState.dwKeyPress = kKeyLeft;
			return 1;
		}
		if (button & PSP_CTRL_RIGHT)
		{
			g_InputState.prevdir = (gpGlobals->fInBattle ? kDirUnknown : g_InputState.dir);
			g_InputState.dir = kDirEast;
			g_InputState.dwKeyPress = kKeyRight;
			return 1;
		}
		if (button & PSP_CTRL_SQUARE)
		{
			g_InputState.dwKeyPress = kKeyForce;
			return 1;
		}
		if (button & PSP_CTRL_TRIANGLE)
		{
			g_InputState.dwKeyPress = kKeyRepeat;
			return 1;
		}
		if (button & PSP_CTRL_CIRCLE)
		{
			g_InputState.dwKeyPress = kKeySearch;
			return 1;
		}
		if (button & PSP_CTRL_CROSS)
		{
			g_InputState.dwKeyPress = kKeyMenu;
			return 1;
		}
		if ((button & PSP_CTRL_LTRIGGER) && (button & PSP_CTRL_SELECT))
		{
			dynamic_freq = 0;
			scePowerSetClockFrequency(333, 333, 166);
			return 1;
		}
		if ((button & PSP_CTRL_RTRIGGER) && (button & PSP_CTRL_SELECT))
		{
			dynamic_freq = 1;
			scePowerSetClockFrequency(300, 300, 150);
			return 1;
		}
		if (button & PSP_CTRL_START)
		{
			// MP3 ioread error handling tester. 
            // ioread_err = 1;
			return 1;
		}
		if (button & PSP_CTRL_SELECT)
		{
			g_InputState.dwKeyPress = kKeyStatus;
			return 1;
		}
		if (button & PSP_CTRL_LTRIGGER)
		{
			g_InputState.dwKeyPress = kKeyPgUp;
			return 1;
		}
		if (button & PSP_CTRL_RTRIGGER)
		{
			g_InputState.dwKeyPress = kKeyPgDn;
			return 1;
		}
		g_InputState.prevdir = (gpGlobals->fInBattle ? kDirUnknown : g_InputState.dir);
		g_InputState.dir = kDirUnknown;
		return 1;
	}
	return 1;
}

void screenPrintf(char* s) {
	printf("MESSAGE: \n");
	for (int i = 3; i < 13; i++) {
		if (__descriptormap[i]) {
			printf("__descriptormap[%d]->filename =  %s \n", i, __descriptormap[i]->filename);
		}
	}
	printf(s);
	sceKernelDelayThread(10000000);
}

void
UTIL_LogToScreen(
	LOGLEVEL       _,
	const char* string,
	const char* __
)
{
	printf(string);
}

INT
UTIL_Platform_Init(
	int argc,
	char* argv[]
)
{
	pspDebugScreenInit();
	UTIL_LogAddOutputCallback(UTIL_LogToScreen, gConfig.iLogLevel);

	sceCtrlSetSamplingCycle(0);
	sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);
	pad.Buttons = 0;

	PAL_RegisterInputFilter(NULL, input_event_filter, NULL);

	PSP_setup_callbacks();
	//
	// Register callbacks 
	//
	atexit(sceKernelExitGame);
	//
	// set PSP CPU clock
	//
	scePowerSetClockFrequency(333, 333, 166);

	// Load modules
	sceUtilityLoadModule(PSP_MODULE_AV_AVCODEC);
	/*
	if (status < 0)
	{
		ERRORMSG("ERROR: sceUtilityLoadModule(PSP_MODULE_AV_AVCODEC) returned 0x%08X\n", status);
	}
	*/
	sceUtilityLoadModule(PSP_MODULE_AV_MP3);
	
	/*
	if (status < 0)
	{
		ERRORMSG("ERROR: sceUtilityLoadModule(PSP_MODULE_AV_MP3) returned 0x%08X\n", status);
	}
	*/
	return 0;
}

INT
UTIL_Platform_Startup(
	int argc,
	char* argv[]
)
{
	return 0;
}

VOID
UTIL_Platform_Quit(
	VOID
)
{
	return;
}
