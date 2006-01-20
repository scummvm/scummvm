// Residual - Virtual machine to run LucasArts' 3D adventure games
// Copyright (C) 2003-2005 The ScummVM-Residual Team (www.scummvm.org)
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2.1 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA

#ifndef _STDAFX_H
#define _STDAFX_H


typedef struct Mutex *MutexRef;

class StackLock {
	MutexRef _mutex;
public:
	StackLock(MutexRef mutex);
	~StackLock();
};

MutexRef createMutex();
void lockMutex(MutexRef mutex);
void unlockMutex(MutexRef mutex);
void deleteMutex(MutexRef mutex);

#ifndef _MSC_VER
#include <unistd.h>
#endif

#if defined(WIN32)

#ifdef _MSC_VER
#pragma once
#pragma warning( disable : 4068 ) // turn off "unknown pragma" warning
#pragma warning( disable : 4244 ) // turn off "conversion type" warning
#endif

#if !defined(_WIN32_WCE)


#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#define NOGDICAPMASKS
#define OEMRESOURCE
#define NONLS
#define NOICONS
#define NOMCX
#define NOPROFILER
#define NOKANJI
#define NOSERVICE
#define NOMETAFILE
#define NOCOMM
#define NOCRYPT
#define NOIME
#define NOATOM
#define NOCTLMGR
#define NOCLIPBOARD
#define NOMEMMGR
#define NOSYSMETRICS
#define NOMENUS
#define NOOPENFILE
#define NOWH
#define NOSOUND
#define NODRAWTEXT


#endif


#include <windows.h>
#if defined(ARRAYSIZE)
// VS2005beta2 introduces new stuff in winnt.h
#undef ARRAYSIZE
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <stdarg.h>
#include <fcntl.h>
#include <conio.h>
#include <assert.h>
#include <mmsystem.h>
#include <ctype.h>
#include <winuser.h>
#include <direct.h>
#define strcasecmp stricmp
#define M_PI 3.14159265358979323846

#ifndef _MSC_VER
#include <stdint.h>
#include <dirent.h>
#endif

#else

#if defined(__MORPHOS__)
#include <devices/timer.h>
#undef CMD_INVALID
#endif
#if !defined(macintosh)
#include <sys/types.h>
#include <sys/uio.h>
#endif
#if !defined (__BEOS__)
#include <unistd.h>
#endif
#if defined(__QNXNTO__)
#include <strings.h>	/* For strcasecmp */
#endif
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>
#include <ctype.h>
#include <dirent.h>


#endif

#endif
