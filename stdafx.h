/*
 * $Id$
 *
 * $Log$
 * Revision 1.3.2.2  2001/12/20 00:18:03  yazoo
 * Resync with main branch, major FT fixes
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
#include <sys/types.h>
#include <sys/uio.h>
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
