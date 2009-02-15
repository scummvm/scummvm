/***************************************************************************
 gfx_drivers.c Copyright (C) 2001 Christoph Reichenbach


 This program may be modified and copied freely according to the terms of
 the GNU general public license (GPL), as long as the above copyright
 notice and the licensing information contained herein are preserved.

 Please refer to www.gnu.org for licensing details.

 This work is provided AS IS, without warranty of any kind, expressed or
 implied, including but not limited to the warranties of merchantibility,
 noninfringement, and fitness for a specific purpose. The author will not
 be held liable for any damage caused by this work or derivatives of it.

 By using this source code, you agree to the licensing terms as stated
 above.


 Please contact the maintainer for bug reports or inquiries.

 Current Maintainer:

    Christoph Reichenbach (CR) <jameson@linuxgames.com>

***************************************************************************/

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif
#include <gfx_driver.h>
#include <modules.h>


static char *oldname = NULL;
static void *oldhandle;



#ifndef HAVE_DLOPEN
#  ifdef HAVE_LIBGGI
extern gfx_driver_t gfx_driver_ggi;
#  endif


#  ifndef X_DISPLAY_MISSING
extern gfx_driver_t gfx_driver_xlib;
#  endif

#  ifdef HAVE_DIRECTX
extern gfx_driver_t gfx_driver_dx;
#  endif

#  ifdef HAVE_DDRAW
extern gfx_driver_t gfx_driver_dd;
#  endif

#  ifdef HAVE_SDL
extern gfx_driver_t gfx_driver_sdl;
#  endif

#  ifdef HAVE_DIRECTFB
extern gfx_driver_t gfx_driver_dfb;
#  endif

#  ifdef _DREAMCAST
extern gfx_driver_t gfx_driver_dc;
#  endif
#endif

extern gfx_driver_t gfx_driver_null;

static gfx_driver_t *gfx_drivers[] = {
#ifndef HAVE_DLOPEN
#  ifndef X_DISPLAY_MISSING
	&gfx_driver_xlib,
#  endif
#  ifdef HAVE_SDL
	&gfx_driver_sdl,
#  endif
#  ifdef HAVE_DIRECTX
	&gfx_driver_dx,
#  endif
#  ifdef HAVE_DDRAW
	&gfx_driver_dd,
#  endif
#  ifdef HAVE_DIRECTFB
	&gfx_driver_dfb,
#  endif
#  ifdef HAVE_LIBGGI
	&gfx_driver_ggi,
#  endif
#  ifdef _DREAMCAST
	&gfx_driver_dc,
#  endif
#endif
	&gfx_driver_null,
	NULL
};

#define DRIVER_TYPE "gfx"
#define DRIVER_PREFIX "gfx_driver_"
#define DRIVER_FILE_SUFFIX "_driver"

#ifdef HAVE_DLOPEN
struct _gfx_driver *
gfx_find_driver(char *path, char *name)
{
	int retval = 0;

	if (oldhandle)
		sci_close_module(oldhandle, DRIVER_TYPE, oldname);

	if (!name) { /* Find default driver */
#ifdef _WIN32
		name = "sdl";
#else /* !_WIN32 */
#  ifndef X_DISPLAY_MISSING
		if (getenv("DISPLAY"))
			name = "xlib";
		else
#  endif
			name = "ggi";
#endif /* !_WIN32 */
	}

	oldname = name;
	return (struct _gfx_driver *)
		sci_find_module(path, name, DRIVER_TYPE,
				DRIVER_PREFIX,
				DRIVER_FILE_SUFFIX,
				SCI_GFX_DRIVER_MAGIC,
				SCI_GFX_DRIVER_VERSION,
				&oldhandle);
}
#else /* No dlopen */
struct _gfx_driver *
gfx_find_driver(char *path, char *name)
{
	int retval = 0;

	if (!name) { /* Find default driver */
#ifndef X_DISPLAY_MISSING
		if (getenv("DISPLAY"))
			return &gfx_driver_xlib;
#endif
#if defined (MACOSX) && defined(HAVE_SDL)
		return &gfx_driver_sdl;
#endif
		return gfx_drivers[0];
	}

	while (gfx_drivers[retval] && strcasecmp(name, gfx_drivers[retval]->name))
		retval++;

	return gfx_drivers[retval];
}
#endif


const char *
gfx_get_driver_name(int nr)
{
	return (gfx_drivers[nr])? gfx_drivers[nr]->name : NULL;
}


int
string_truep(char *value)
{
	return !(strcasecmp(value, "ok") &&
		strcasecmp(value, "enable") &&
		strcasecmp(value, "1") &&
		strcasecmp(value, "true") &&
		strcasecmp(value, "yes") &&
		strcasecmp(value, "on"));
}


int
string_falsep(char *value)
{
	return !(strcasecmp(value, "disable") &&
		strcasecmp(value, "0") &&
		strcasecmp(value, "false") &&
		strcasecmp(value, "no") &&
		strcasecmp(value, "off"));
}




