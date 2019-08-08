#include "../main.h"
#include <math.h>
#include <pspkernel.h>
#include <pspctrl.h>
#include <SDL_thread.h>

#include <pspdisplay.h>

#include <pspsdk.h>
#include <psppower.h>
#include <pspthreadman.h>
#include <stdlib.h>
#include <stdio.h>

PSP_MODULE_INFO("SDLPAL", PSP_MODULE_USER, VERS, REVS);
PSP_MAIN_THREAD_ATTR(PSP_THREAD_ATTR_USER);
PSP_HEAP_SIZE_MAX();

static SceCtrlData pad;
static SDL_sem *pad_sem = 0;
static SDL_Thread *bthread = 0;
static int running = 0;
static unsigned int old_button = 0;
static unsigned char old_x = 0, old_y = 0;

int dynamic_freq = 0;

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
	if (pwrflags & PSP_POWER_CB_RESUME_COMPLETE)
	{
		// On certian PSP-1000 system, file handles will be invalid after PSP_POWER_CB_RESUME_COMPLETE 
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
		fclose(gpGlobals->f.fpFIRE);
		gpGlobals->f.fpFIRE = UTIL_OpenRequiredFile("fire.mkf");
		fclose(gpGlobals->f.fpRGM);
		gpGlobals->f.fpRGM = UTIL_OpenRequiredFile("rgm.mkf");
		fclose(gpGlobals->f.fpSSS);
		gpGlobals->f.fpSSS = UTIL_OpenRequiredFile("sss.mkf");
	}
	int cbid;
	cbid = sceKernelCreateCallback("suspend Callback", PSP_resume_callback, NULL);
	scePowerRegisterCallback(0, cbid);
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
}


INT
UTIL_Platform_Init(
	int argc,
	char* argv[]
)
{
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
}

INT
UTIL_Platform_Startup(
	int argc,
	char* argv[]
)
{
}

VOID
UTIL_Platform_Quit(
	VOID
)
{
}