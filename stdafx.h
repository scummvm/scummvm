/*
 * $Id$
 *
 * $Log$
 * Revision 1.4  2001/11/03 06:40:51  cmatsuoka
 * Not including unistd.h in BeOS (breaks cross-compilation).
 *
 */

#if defined(WIN32)

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

#else

#if defined(NEED_SDL_HEADERS)
#include <SDL.h>
#endif
#include <sys/types.h>
#include <sys/uio.h>
#if !defined(__BEOS__)
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
