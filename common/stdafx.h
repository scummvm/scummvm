/*
 * $Id$
 *
 * $Log$
 * Revision 1.3  2002/11/19 08:05:46  arisme
 * Small Windows CE changes : detect old WinCE as NONSTANDARD_PORT, change __cdecl for these systems, add 2 sampling rates for WinCE due to problems with Smush mixer in 11 kHz
 *
 * Revision 1.2  2002/09/22 11:37:53  kirben
 *
 * Remove mention of Windows GDI build
 *
 * Revision 1.1.1.1  2002/08/21 16:07:23  fingolfin
 * new ScummVM CVS module
 *
 * Revision 1.18  2002/07/08 13:33:10  fingolfin
 * two more small QNX fixes
 *
 * Revision 1.17  2002/06/02 20:28:09  bbrox
 * Small warning fix + double inclusion protection (can always be useful
 * :) ).
 *
 * Revision 1.16  2002/05/05 20:04:25  fingolfin
 * cleaning up the mess drigo left... <sigh>
 *
 * Revision 1.15  2002/05/05 19:06:51  drigo
 * Fixed some things for Macintosh ports
 *
 * Revision 1.14  2002/04/18 21:40:23  tomjoad
 * Reenable MorphOS Midi driver, small updates to CD open code (only when CD audio is requested) and start options
 *
 * Revision 1.13  2002/04/12 21:26:34  strigeus
 * new video engine (expect broken non-sdl builds),
 * simon the sorcerer 1 & 2 support (non SCUMM games)
 *
 * Revision 1.12  2002/03/14 22:45:22  arisme
 * Minor changes to compile WinCE port
 *
 * Revision 1.11  2002/03/09 13:48:53  drigo
 * Support for MacOS classic port
 *
 * Revision 1.10  2002/03/08 17:05:09  mutle
 * Some changes, need to be done to get the Mac Port running. For example Point is now called ScummPoint, as the name Point is already in use by Apple.
 *
 * Revision 1.9  2002/03/06 12:24:56  ender
 * Applied cleanup and scaling patch by Rob.
 *
 * Revision 1.8  2001/11/20 07:13:01  vasyl
 * Added ability for ports to override all includes in stdafx.h. To use this feature
 * port must define symbol NONSTANDARD_PORT. Port must also provide
 * port-specific portdefs.h with all includes, symbol defines and everything else
 * port needs.
 *
 * Revision 1.7  2001/11/11 16:54:45  strigeus
 * implemented some sam&max specific features,
 * fixed some bugs
 *
 * Revision 1.6  2001/11/06 22:59:59  cmatsuoka
 * Re-added changes to allow cygwin and beos cross-compilation.
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

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


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
#include <malloc.h>
#include <assert.h>
#include <mmsystem.h>
#include <ctype.h>
#include <Winuser.h>
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


/* Semi-Platform-specific version info */
#ifdef ALLOW_X11
/* Assume pure X11 is used, then... */
#define SCUMMVM_PLATFORM_VERSION  "X11 version"

#else

#ifdef MACOS_CARBON
#define SCUMMVM_PLATFORM_VERSION "Macintosh version"
#else

#ifdef SDL_COMPILEDVERSION
#define SCUMMVM_PLATFORM_VERSION  "SDL version"
//SDL_COMPILEDVERSION is a number... :(
//SDL_Linked_Version returns an SDL_Version structure...

#endif
#endif
#endif

#endif
