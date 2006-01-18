/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2006 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $Header$
 *
 */

#ifndef _STDAFX_H
#define _STDAFX_H

#if defined(_WIN32_WCE) && _WIN32_WCE < 300

#define NONSTANDARD_PORT

#endif

#if defined(NONSTANDARD_PORT)

#include <portdefs.h>

#elif defined(WIN32)

#ifdef _MSC_VER
#pragma once
#pragma warning( disable : 4068 ) // turn off "unknown pragma" warning
#pragma warning( disable : 4100 ) // turn off "unreferenced formal parameter" warning
#pragma warning( disable : 4127 ) // turn off "conditional expression is constant" warning
#pragma warning( disable : 4201 ) // turn off "nonstandard extension used : nameless struct/union" warning
#pragma warning( disable : 4244 ) // turn off "conversion type" warning
#pragma warning( disable : 4310 ) // turn off "cast truncates constant value" warning
#pragma warning( disable : 4355 ) // turn off "base member init" warning
#pragma warning( disable : 4390 ) // turn off "empty statement" warning for BS2 code
#pragma warning( disable : 4121 ) // turn off "alignment of a member was sensitive to packing" warning
#pragma warning( disable : 4511 ) // turn off "copy constructor could not be generated" warning
#pragma warning( disable : 4512 ) // turn off "assignment operator could not be generated" warning

// FIXME: 4702 & 4706 - may be enabled
#pragma warning( disable : 4702 ) // turn off "unreachable code" warning
#pragma warning( disable : 4706 ) // turn off "assignment within conditional expression" warning

#define vsnprintf _vsnprintf
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
#include <time.h>
#include <math.h>

#else

#if defined(__MORPHOS__)
#include <devices/timer.h>
#undef CMD_INVALID
#endif
#if !defined(macintosh) && !defined(PALMOS_ARM)
#include <sys/types.h>
#if !defined(__PLAYSTATION2__) && !defined(__PSP__) && !defined(__amigaos4__) && !defined(__SYMBIAN32__)
#include <sys/uio.h>
#endif
#if !defined(__amigaos4__)
#include <sys/param.h>
#endif
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
#include <time.h>
#include <math.h>


#endif

#ifndef MAXPATHLEN
#define MAXPATHLEN 256
#endif

#endif
