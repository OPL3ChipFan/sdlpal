/* -*- mode: c; tab-width: 4; c-basic-offset: 4; c-file-style: "linux" -*- */
//
// Copyright (c) 2009-2011, Wei Mingzhi <whistler_wmz@users.sf.net>.
// Copyright (c) 2011-2019, SDLPAL development team.
// All rights reserved.
//
// This file is part of SDLPAL.
//
// SDLPAL is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

#include <pspkernel.h>
#include <pspdebug.h>
#include <pspctrl.h>
#include <pspdisplay.h>
#include <stdio.h>
#include <pspaudio.h>
#include <pspmp3.h>
#include <psputility.h>

#include "../common.h"

extern SceUID thid;
extern int playingNativeMP3;
extern int ioread_err;

extern char MP3filename[PAL_MAX_PATH];
extern char monitor_MP3filename[PAL_MAX_PATH];
extern int current_floop;
extern int current_iMusicVolume;
extern char cwd_buff[PAL_MAX_PATH];

extern void clearFileNameCache(void);
extern int playNativeMP3(const char* filename, int fLoop, int iMusicVolume);
extern int stopNativeMP3(void);
extern int initNativeMP3(void);
extern int shutdownNativeMP3(void);
