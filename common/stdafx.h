/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2003 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
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
#pragma warning( disable : 4244 ) // turn off "conversion type" warning
#pragma warning( disable : 4390 ) // turn oof "empty statement" warning for BS2 code
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


#endif

#endif
