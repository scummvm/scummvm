/*
 * $Id$
 *
 * $Log$
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

#if defined(NONSTANDARD_PORT)

#include <portdefs.h>

#elif defined(WIN32)

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

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

#include <SDL.h>
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

#else

#if defined(NEED_SDL_HEADERS)
#include <SDL.h>
#endif
#if !defined(__APPLE__CW)
#include <sys/types.h>
#include <sys/uio.h>
#endif
#if !defined (__BEOS__)
#include <unistd.h>
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
#ifdef ALLOW_GDI
/* Assume Win32 GDI is used, then... */
#define SCUMMVM_PLATFORM_VERSION  "Win32 GDI version"

#else
#ifdef ALLOW_X11
/* Assume pure X11 is used, then... */
#define SCUMMVM_PLATFORM_VERSION  "X11 version"

#else
#ifdef __APPLE__CW
#define SCUMMVM_PLATFORM_VERSION "Carbon Mac version"
#else

#ifdef SDL_COMPILEDVERSION
#define SCUMMVM_PLATFORM_VERSION  "SDL version"
//SDL_COMPILEDVERSION is a number... :(
//SDL_Linked_Version returns an SDL_Version structure...

#endif
#endif
#endif
#endif

