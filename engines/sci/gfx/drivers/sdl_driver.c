/***************************************************************************
 sdl_driver.c Copyright (C) 2001 Solomon Peachy

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
		 Solomon Peachy <pizza@shaftnet.org>

***************************************************************************/

/* set optimisations for Win32: */
/* g on: enable global optimizations */
/* t on: use fast code */
/* y on: suppress creation of frame pointers on stack */
/* s off: disable minimize size code */
#ifdef _WIN32
#	include <memory.h>
#	ifndef SATISFY_PURIFY
#		pragma optimize( "s", off )
#		pragma optimize( "gty", on )
#		pragma intrinsic( memcpy, memset )
#	endif
#endif

#include <sci_memory.h>

#include <gfx_driver.h>
#ifdef HAVE_SDL
#include <gfx_tools.h>

#if !defined(_MSC_VER)
#  include <sys/time.h>
#endif

#include <SDL_config.h>
#undef HAVE_ICONV
#undef HAVE_ICONV_H
#undef HAVE_ALLOCA_H

#include <SDL.h>

#ifndef SDL_DISABLE
#	define SDL_DISABLE 0
#endif
#ifndef SDL_ALPHA_OPAQUE
#	define SDL_ALPHA_OPAQUE 255
#endif

#define SCI_SDL_HANDLE_NORMAL 0
#define SCI_SDL_HANDLE_GRABBED 1

#define SCI_SDL_SWAP_CTRL_CAPS (1 << 0)
#define SCI_SDL_FULLSCREEN (1 << 2)

static int
sdl_usec_sleep(struct _gfx_driver *drv, long usecs);

static int flags = 0;

struct _sdl_state {
	int used_bytespp;
	gfx_pixmap_t *priority[2];
	SDL_Color colors[256];
	SDL_Surface *visual[3];
	SDL_Surface *primary;
	int buckystate;
	byte *pointer_data[2];
	int alpha_mask;
	int SDL_alpha_shift;
	int SDL_alpha_loss;
};

#define S ((struct _sdl_state *)(drv->state))

#define XFACT drv->mode->xfact
#define YFACT drv->mode->yfact

#define DEBUGB if (drv->debug_flags & GFX_DEBUG_BASIC && ((debugline = __LINE__))) sdlprintf
#define DEBUGU if (drv->debug_flags & GFX_DEBUG_UPDATES && ((debugline = __LINE__))) sdlprintf
#define DEBUGPXM if (drv->debug_flags & GFX_DEBUG_PIXMAPS && ((debugline = __LINE__))) sdlprintf
#define DEBUGPTR if (drv->debug_flags & GFX_DEBUG_POINTER && ((debugline = __LINE__))) sdlprintf
#define SDLERROR if ((debugline = __LINE__)) sdlprintf
#define SDLPRINTF if ((debugline = __LINE__)) sdlprintf

#define ALPHASURFACE (S->used_bytespp == 4)

static int debugline = 0;

static void
sdlprintf(const char *fmt, ...)
{
	va_list argp;
	fprintf(stderr,"GFX-SDL %d:", debugline);
	va_start(argp, fmt);
	vfprintf(stderr, fmt, argp);
	va_end(argp);
}

static int
sdl_init_libsdl(struct _gfx_driver *drv)
{
	if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_NOPARACHUTE)) {
		DEBUGB("Failed to init SDL\n");
		return -1;
	}

	SDL_EnableUNICODE(SDL_ENABLE);

	return 0;
}

static int
sdl_alloc_primary(struct _gfx_driver *drv, int xfact, int yfact, int bytespp)
{
	int i = SDL_HWSURFACE | SDL_HWPALETTE;

	if (flags & SCI_SDL_FULLSCREEN) {
		i |= SDL_FULLSCREEN;
	}

	S->primary = SDL_SetVideoMode(xfact * 320, yfact * 200, bytespp << 3, i);

	if (!S->primary) {
		SDLERROR("Could not set up a primary SDL surface!\n");
		return -1;
	}

	if (S->primary->format->BytesPerPixel != bytespp) {
		SDLERROR("Could not set up a primary SDL surface of depth %d bpp!\n",bytespp);
		S->primary = NULL;
		return -1;
	}

	/* Set windowed flag */
	if (S->primary->flags & SDL_FULLSCREEN)
		drv->capabilities &= ~GFX_CAPABILITY_WINDOWED;
	else
		drv->capabilities |= GFX_CAPABILITY_WINDOWED;

	return 0;
}

static int
sdl_blit_surface(gfx_driver_t *drv,
		 SDL_Surface *src, SDL_Rect *srcrect, SDL_Surface *dst, SDL_Rect *dstrect)
{
	if (S->used_bytespp == 1) {
		SDL_SetColors(src, S->colors, 0, 256);
		SDL_SetColors(dst, S->colors, 0, 256);
	}
	return SDL_BlitSurface(src, srcrect, dst, dstrect);
}

static int
sdl_set_parameter(struct _gfx_driver *drv, char *attribute, char *value)
{
	if (!strncmp(attribute, "swap_ctrl_caps", 14) ||
			!strncmp(attribute, "swap_caps_ctrl", 14)) {
		if (string_truep(value))
			flags |= SCI_SDL_SWAP_CTRL_CAPS;
		else
			flags &= ~SCI_SDL_SWAP_CTRL_CAPS;
		return GFX_OK;
	}

	if (!strncmp(attribute, "fullscreen", 10)) {
		if (string_truep(value))
			flags |= SCI_SDL_FULLSCREEN;
		else
			flags &= ~SCI_SDL_FULLSCREEN;

		return GFX_OK;
	}


	SDLERROR("Attempt to set sdl parameter \"%s\" to \"%s\"\n", attribute, value);
	return GFX_ERROR;
}

static int
sdl_init_specific(struct _gfx_driver *drv, int xfact, int yfact, int bytespp)
{
	int red_shift, green_shift, blue_shift, alpha_shift;
	int xsize = xfact * 320;
	int ysize = yfact * 200;

	int i;

#ifdef _MSC_VER /* Win32 doesn't support mouse pointers greater than 64x64 */
	if (xfact > 2 || yfact > 2)
		drv->capabilities &= ~GFX_CAPABILITY_MOUSE_POINTER;
#endif
#if defined(__BEOS__) || defined(__amigaos4__) /* BeOS has been reported not to work well with the mouse pointer at all */
	drv->capabilities &= ~GFX_CAPABILITY_MOUSE_POINTER;
#endif

	if (sdl_init_libsdl(drv))
		return GFX_FATAL;

	if (!drv->state /* = S */)
		drv->state = sci_malloc(sizeof(struct _sdl_state));
	if (!drv->state)
		return GFX_FATAL;

	if (xfact < 1 || yfact < 1 || bytespp < 1 || bytespp > 4) {
		SDLERROR("Internal error: Attempt to open window w/ scale factors (%d,%d) and bpp=%d!\n",
		 xfact, yfact, bytespp);
	}

	if (sdl_alloc_primary(drv, xfact, yfact, bytespp))
		return GFX_FATAL;

	S->used_bytespp = bytespp;

	printf("Using primary SDL surface of %d,%d @%d bpp\n",
	       xsize, ysize, bytespp << 3);

	/*  if (S->primary->format->BytesPerPixel == 4) {
		S->alpha_mask = 0xff000000;
		S->SDL_alpha_shift = 24;
		S->SDL_alpha_loss = 0;
		alpha_shift = 0;
		} else { */
		S->alpha_mask = S->primary->format->Amask;
		S->SDL_alpha_shift = S->primary->format->Ashift;
		S->SDL_alpha_loss = S->primary->format->Aloss;
		alpha_shift = bytespp << 3;
		/*   }*/

	/* clear palette */
	for (i = 0; i < 256; i++) {
		S->colors[i].r = (i & 1)? 0 : 0;
		S->colors[i].g = (i & 2)? 0 : 0;
		S->colors[i].b = (i & 4)? 0 : 0;
	}
	if (bytespp == 1)
		SDL_SetColors(S->primary, S->colors, 0, 256);

	/* create an input event mask */
	SDL_EventState(SDL_ACTIVEEVENT, SDL_IGNORE);
	SDL_EventState(SDL_VIDEORESIZE, SDL_IGNORE);
	SDL_EventState(SDL_KEYUP, SDL_IGNORE);

	SDL_WM_SetCaption("FreeSCI", "freesci");

	SDL_ShowCursor(SDL_DISABLE);
	S->pointer_data[0] = NULL;
	S->pointer_data[1] = NULL;

	S->buckystate = 0;

	if (bytespp == 1) {
		red_shift = green_shift = blue_shift = alpha_shift = 0;
	} else {
		red_shift = 24 - S->primary->format->Rshift + S->primary->format->Rloss;
		green_shift = 24 - S->primary->format->Gshift + S->primary->format->Gloss;
		blue_shift = 24 - S->primary->format->Bshift + S->primary->format->Bloss;
	}

	printf("%08x %08x %08x %08x %d/%d=%d %d/%d=%d %d/%d=%d %d/%d=%d\n",
	 S->primary->format->Rmask,
	 S->primary->format->Gmask,
	 S->primary->format->Bmask,
	 S->alpha_mask,
	 /*	 S->primary->format->Amask,*/
	 S->primary->format->Rshift,
	 S->primary->format->Rloss,
	 red_shift,
	 S->primary->format->Gshift,
	 S->primary->format->Gloss,
	 green_shift,
	 S->primary->format->Bshift,
	 S->primary->format->Bloss,
	 blue_shift,
	 S->SDL_alpha_shift,
	 S->SDL_alpha_loss,
	 /*
	 S->primary->format->Ashift,
	 S->primary->format->Aloss, */
	 alpha_shift);

	for (i = 0; i < 2; i++) {
		S->priority[i] = gfx_pixmap_alloc_index_data(gfx_new_pixmap(xsize, ysize, GFX_RESID_NONE, -i, -777));
		if (!S->priority[i]) {
			SDLERROR("Out of memory: Could not allocate priority maps! (%dx%d)\n",
	    xsize, ysize);
			return GFX_FATAL;
		}
	}

	/* create the visual buffers */
	for (i = 0; i < 3; i++) {
		S->visual[i] = SDL_CreateRGBSurface(SDL_SRCALPHA,
					/* SDL_HWSURFACE | SDL_SWSURFACE, */
					xsize, ysize,
					bytespp << 3,
					S->primary->format->Rmask,
					S->primary->format->Gmask,
					S->primary->format->Bmask,
					S->alpha_mask);
		if (S->visual[i] == NULL) {
			SDLERROR("Could not set up visual buffers!\n");
			return GFX_FATAL;
		}

		if (ALPHASURFACE)
		 SDL_SetAlpha(S->visual[i],SDL_SRCALPHA,SDL_ALPHA_OPAQUE);

		if (SDL_FillRect(S->primary, NULL, SDL_MapRGB(S->primary->format, 0,0,0)))
			SDLERROR("Couldn't fill backbuffer!\n");
	}

	SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

	drv->mode = gfx_new_mode(xfact, yfact, bytespp,
			   S->primary->format->Rmask,
			   S->primary->format->Gmask,
			   S->primary->format->Bmask,
			   S->alpha_mask,
			   red_shift, green_shift, blue_shift, alpha_shift,
			   (bytespp == 1)? 256 : 0, 0); /*GFX_MODE_FLAG_REVERSE_ALPHA);*/

	return GFX_OK;
}

static int
sdl_init(struct _gfx_driver *drv)
{
	int depth = 0;
	int i;

	if (sdl_init_libsdl(drv))
		return GFX_FATAL;

	i = SDL_HWSURFACE | SDL_HWPALETTE;
	if (flags & SCI_SDL_FULLSCREEN) {
		i |= SDL_FULLSCREEN;
	}

	depth = SDL_VideoModeOK(640,400, 32, i);
	if (depth && (! sdl_init_specific(drv, 2, 2, depth >> 3 )))
		return GFX_OK;

	DEBUGB("Failed to find visual!\n");
	return GFX_FATAL;
}

static void
sdl_exit(struct _gfx_driver *drv)
{
	int i;
	if (S) {
		for (i = 0; i < 2; i++) {
			gfx_free_pixmap(drv, S->priority[i]);
			S->priority[i] = NULL;
		}

		for (i = 0; i < 3; i++) {
			SDL_FreeSurface(S->visual[i]);
			S->visual[i] = NULL;
		}

		SDL_FreeCursor(SDL_GetCursor());

		for (i = 0; i < 2; i++)
			if (S->pointer_data[i]) {
				free(S->pointer_data[i]);
				S->pointer_data[i] = NULL;
			}

	}

	SDL_QuitSubSystem(SDL_INIT_VIDEO);

	if (!SDL_WasInit(SDL_INIT_EVERYTHING)) {
		SDLPRINTF("No active SDL subsystems found.. shutting down SDL\n");
		SDL_Quit();
	}
}

static void
toggle_fullscreen(struct _gfx_driver *drv)
{
	rect_t src;
	point_t dest;

	flags ^= SCI_SDL_FULLSCREEN;
	if (sdl_alloc_primary(drv, XFACT, YFACT, drv->mode->bytespp)) {
		SDLERROR("failed to switch to full-screen mode\n");
		/* Failed to set mode, revert to previous */
		flags ^= SCI_SDL_FULLSCREEN;

		if (sdl_alloc_primary(drv, XFACT, YFACT, drv->mode->bytespp)) {
			/* This shouldn't happen... */
			SDLERROR("failed to revert to previous display mode\n");
			exit(-1);
		}
	}

	src.x = 0;
	src.y = 0;
	src.xl = XFACT * 320;
	src.yl = YFACT * 200;
	dest.x = 0;
	dest.y = 0;

	drv->update(drv, src, dest, GFX_BUFFER_FRONT);
}

/*** Drawing operations ***/

static Uint32
sdl_map_color(gfx_driver_t *drv, gfx_color_t color)
{
	int opacity = 255 - color.alpha;

	if (drv->mode->palette && opacity < 255) {
		if (opacity < 127)
			opacity = 0;
		else
			opacity = 255;
			
	}

	if (drv->mode->palette)
		return color.visual.global_index;

	return SDL_MapRGBA(S->visual[0]->format,
			   color.visual.r,
			   color.visual.g,
			   color.visual.b,
			   opacity);
}

/* This code shamelessly lifted from the SDL_gfxPrimitives package */
static void lineColor2(SDL_Surface *dst, Sint16 x1, Sint16 y1, Sint16 x2, Sint16 y2, Uint32 color)
{
	int pixx, pixy;
	int x,y;
	int dx,dy;
	int sx,sy;
	int swaptmp;
	Uint8 *pixel;

	dx = x2 - x1;
	dy = y2 - y1;
	sx = (dx >= 0) ? 1 : -1;
	sy = (dy >= 0) ? 1 : -1;

	dx = sx * dx + 1;
	dy = sy * dy + 1;
	pixx = dst->format->BytesPerPixel;
	pixy = dst->pitch;
	pixel = ((Uint8*)dst->pixels) + pixx * (int)x1 + pixy * (int)y1;
	pixx *= sx;
	pixy *= sy;
	if (dx < dy) {
	 swaptmp = dx; dx = dy; dy = swaptmp;
	 swaptmp = pixx; pixx = pixy; pixy = swaptmp;
	}

/* Draw */
	x=0;
	y=0;
	switch(dst->format->BytesPerPixel) {
	 case 1:
		for(; x < dx; x++, pixel += pixx) {
			*pixel = color;
			y += dy;
			if (y >= dx) {
				y -= dx; pixel += pixy;
			}
		}
		break;
	 case 2:
		for (; x < dx; x++, pixel += pixx) {
			*(Uint16*)pixel = color;
			y += dy;
			if (y >= dx) {
				y -= dx;
				pixel += pixy;
			}
		}
		break;
	 case 3:
		for(; x < dx; x++, pixel += pixx) {
			if(SDL_BYTEORDER == SDL_BIG_ENDIAN) {
				pixel[0] = (color >> 16) & 0xff;
				pixel[1] = (color >> 8) & 0xff;
				pixel[2] = color & 0xff;
			} else {
				pixel[0] = color & 0xff;
				pixel[1] = (color >> 8) & 0xff;
				pixel[2] = (color >> 16) & 0xff;
			}
			y += dy;
			if (y >= dx) {
				y -= dx;
				pixel += pixy;
			}
		}
		break;
	 case 4:
		for(; x < dx; x++, pixel += pixx) {
			*(Uint32*)pixel = color;
			y += dy;
			if (y >= dx) {
				y -= dx;
				pixel += pixy;
			}
		}
		break;
	 default:
		fprintf(stderr, "invalid depth\n");
	}

}

static int
sdl_draw_line(struct _gfx_driver *drv, point_t start, point_t end, gfx_color_t color,
	      gfx_line_mode_t line_mode, gfx_line_style_t line_style)
{
	Uint32 scolor;
	int xfact = (line_mode == GFX_LINE_MODE_FINE)? 1: XFACT;
	int yfact = (line_mode == GFX_LINE_MODE_FINE)? 1: YFACT;
	int xsize = S->visual[1]->w;
	int ysize = S->visual[1]->h;

	if (color.mask & GFX_MASK_VISUAL) {
		int xc, yc;
		point_t nstart, nend;

		scolor = sdl_map_color(drv, color);

		for (xc = 0; xc < xfact; xc++)
			for (yc = 0; yc < yfact; yc++) {
				nstart.x = start.x + xc;
				nstart.y = start.y + yc;
				nend.x = end.x + xc;
				nend.y = end.y + yc;

				if (nstart.x < 0)
					nstart.x = 0;
				if (nend.x < 0)
					nstart.x = 0;
				if (nstart.y < 0)
					nstart.y = 0;
				if (nend.y < 0)
					nend.y = 0;
				if (nstart.x > xsize)
					nstart.x = xsize;
				if (nend.x >= xsize)
					nend.x = xsize -1;
				if (nstart.y > ysize)
					nstart.y = ysize;
				if (nend.y >= ysize)
					nend.y = ysize -1;

#if 0
				fprintf(stderr, "draw %d %d to %d %d %08x %d %d\n", nstart.x,
					nstart.y, nend.x, nend.yl, scolor, xsize, ysize);
#endif

				SDL_LockSurface(S->visual[1]);
				lineColor2(S->visual[1], (Sint16)nstart.x, (Sint16)nstart.y,
					  (Sint16)nend.x, (Sint16)nend.y, scolor);
				SDL_UnlockSurface(S->visual[1]);

				if (color.mask & GFX_MASK_PRIORITY) {
					gfx_draw_line_pixmap_i(S->priority[0], nstart, nend,
							       color.priority);
				}
			}
	}

	return GFX_OK;
}

static int
sdl_draw_filled_rect(struct _gfx_driver *drv, rect_t rect,
		     gfx_color_t color1, gfx_color_t color2,
		     gfx_rectangle_fill_t shade_mode)
{
	Uint32 color;
	SDL_Rect srect;

	if (color1.mask & GFX_MASK_VISUAL) {
		color = sdl_map_color(drv, color1);

		srect.x = rect.x;
		srect.y = rect.y;
		srect.w = rect.xl;
		srect.h = rect.yl;

		if (SDL_FillRect(S->visual[1], &srect, color))
			SDLERROR("Can't fill rect");
	}

	if (color1.mask & GFX_MASK_PRIORITY)
		gfx_draw_box_pixmap_i(S->priority[0], rect, color1.priority);

	return GFX_OK;
}

/*** Pixmap operations ***/

static int
sdl_register_pixmap(struct _gfx_driver *drv, gfx_pixmap_t *pxm)
{
	SDL_Surface *reg_surface;

	if (pxm->internal.info) {
		SDLERROR("Attempt to register pixmap twice!\n");
		return GFX_ERROR;
	}

	reg_surface = 
		SDL_CreateRGBSurfaceFrom(pxm->data, pxm->xl, pxm->yl,
					 S->used_bytespp << 3,
					 S->used_bytespp * pxm->xl,
					 S->primary->format->Rmask,
					 S->primary->format->Gmask,
					 S->primary->format->Bmask,
					 S->alpha_mask);

	if (ALPHASURFACE)
			SDL_SetAlpha(reg_surface, SDL_SRCALPHA,SDL_ALPHA_OPAQUE);

	pxm->internal.handle = SCI_SDL_HANDLE_NORMAL;

	DEBUGPXM("Registered surface %d/%d/%d at %p (%dx%d)\n", pxm->ID, pxm->loop, pxm->cel,
		pxm->internal.info, pxm->xl, pxm->yl);

	pxm->internal.info = reg_surface;

	return GFX_OK;
}

static int
sdl_unregister_pixmap(struct _gfx_driver *drv, gfx_pixmap_t *pxm)
{
	DEBUGPXM("Freeing surface %d/%d/%d at %p\n", pxm->ID, pxm->loop, pxm->cel,
		pxm->internal.info);

	if (!pxm->internal.info) {
		SDLERROR("Attempt to unregister pixmap twice!\n");
		return GFX_ERROR;
	}

	SDL_FreeSurface((SDL_Surface *) pxm->internal.info);
	pxm->internal.info = NULL;
	if (pxm->internal.handle != SCI_SDL_HANDLE_GRABBED)
		free(pxm->data);
	pxm->data = NULL;
	return GFX_OK;
}

static int
sdl_draw_pixmap(struct _gfx_driver *drv, gfx_pixmap_t *pxm, int priority,
		rect_t src, rect_t dest, gfx_buffer_t buffer)
{
	int bufnr = (buffer == GFX_BUFFER_STATIC)? 2:1;
	int pribufnr = bufnr -1;

	SDL_Surface *temp;
	SDL_Rect srect;
	SDL_Rect drect;

	if (dest.xl != src.xl || dest.yl != src.yl) {
		SDLERROR("Attempt to scale pixmap (%dx%d)->(%dx%d): Not supported\n",
	  src.xl, src.yl, dest.xl, dest.yl);
		return GFX_ERROR;
	}

	srect.x = src.x;
	srect.y = src.y;
	srect.w = src.xl;
	srect.h = src.yl;
	drect.x = dest.x;
	drect.y = dest.y;
	drect.w = dest.xl;
	drect.h = dest.yl;

	DEBUGU("Drawing %d (%d,%d)(%dx%d) onto (%d,%d)\n", pxm, srect.x, srect.y,
	       srect.w, srect.h, drect.x, drect.y);

	if (pxm->internal.handle == SCI_SDL_HANDLE_GRABBED) {
		if (sdl_blit_surface(drv, (SDL_Surface *)pxm->internal.info, &srect ,
			S->visual[bufnr], &drect )) {
			SDLERROR("blt failed");
			return GFX_ERROR;
		}
		return GFX_OK;
	}

	temp = SDL_CreateRGBSurface(SDL_SWSURFACE, drect.w, drect.h,
			      S->used_bytespp << 3,
			      S->primary->format->Rmask,
			      S->primary->format->Gmask,
			      S->primary->format->Bmask,
			      S->alpha_mask);

	if (ALPHASURFACE)
		SDL_SetAlpha(temp, SDL_SRCALPHA,SDL_ALPHA_OPAQUE);

	if (!temp) {
		SDLERROR("Failed to allocate SDL surface");
		return GFX_ERROR;
	}

	srect.x = dest.x;
	srect.y = dest.y;
	drect.x = 0;
	drect.y = 0;

	if(sdl_blit_surface(drv, S->visual[bufnr], &srect, temp, &drect))
		SDLERROR("blt failed");

	SDL_LockSurface(temp);
	gfx_crossblit_pixmap(drv->mode, pxm, priority, src, dest,
		       (byte *) temp->pixels, temp->pitch,
		       S->priority[pribufnr]->index_data,
		       S->priority[pribufnr]->index_xl, 1,
		       GFX_CROSSBLIT_FLAG_DATA_IS_HOMED);
	SDL_UnlockSurface(temp);

	srect.x = 0;
	srect.y = 0;
	drect.x = dest.x;
	drect.y = dest.y;

	if(sdl_blit_surface(drv, temp, &srect, S->visual[bufnr], &drect))
		SDLERROR("blt failed");

	SDL_FreeSurface(temp);
	return GFX_OK;
}

static int
sdl_grab_pixmap(struct _gfx_driver *drv, rect_t src, gfx_pixmap_t *pxm,
		gfx_map_mask_t map)
{


	if (src.x < 0 || src.y < 0) {
		SDLERROR("Attempt to grab pixmap from invalid coordinates (%d,%d)\n", src.x, src.y);
		return GFX_ERROR;
	}

	if (!pxm->data) {
		SDLERROR("Attempt to grab pixmap to unallocated memory\n");
		return GFX_ERROR;
	}
	switch (map) {

	case GFX_MASK_VISUAL: {
		SDL_Rect srect, drect;
		SDL_Surface *temp;

		pxm->xl = src.xl;
		pxm->yl = src.yl;
		temp = SDL_CreateRGBSurface(SDL_SWSURFACE, src.xl, src.yl,
				S->used_bytespp << 3,
				S->primary->format->Rmask,
				S->primary->format->Gmask,
				S->primary->format->Bmask,
				S->alpha_mask);

		if (!temp) {
			SDLERROR("Failed to allocate SDL surface");
			return GFX_ERROR;
		}

		if (SDL_MUSTLOCK(temp))
			sciprintf("Warning: SDL surface for pixmap grabbing requires locking\n");

		if (ALPHASURFACE)
			SDL_SetAlpha(temp, SDL_SRCALPHA,SDL_ALPHA_OPAQUE);

		srect.x = src.x;
		srect.y = src.y;
		srect.w = src.xl;
		srect.h = src.yl;
		drect.x = 0;
		drect.y = 0;
		drect.w = src.xl;
		drect.h = src.yl;

		if (sdl_blit_surface(drv, S->visual[1], &srect, temp, &drect))
			SDLERROR("grab_pixmap:  grab blit failed!\n");

		pxm->internal.info = temp;
		pxm->internal.handle = SCI_SDL_HANDLE_GRABBED;
		pxm->flags |= GFX_PIXMAP_FLAG_INSTALLED | GFX_PIXMAP_FLAG_EXTERNAL_PALETTE | GFX_PIXMAP_FLAG_PALETTE_SET;
		free(pxm->data);
		pxm->data = (byte *) temp->pixels;

		DEBUGPXM("Grabbed surface %p (%dx%d)(%dx%d)\n",
	     pxm->internal.info, srect.x, srect.y, pxm->xl, pxm->yl);

		break;
	}

	case GFX_MASK_PRIORITY:
		SDLERROR("FIXME: priority map grab not implemented yet!\n");
		break;

	default:
		SDLERROR("Attempt to grab pixmap from invalid map 0x%02x\n", map);
		return GFX_ERROR;
	}

	return GFX_OK;
}


	/*** Buffer operations ***/

static int
sdl_update(struct _gfx_driver *drv, rect_t src, point_t dest, gfx_buffer_t buffer)
{
	int data_source = (buffer == GFX_BUFFER_BACK)? 2 : 1;
	int data_dest = data_source - 1;
	SDL_Rect srect, drect;

	if (src.x != dest.x || src.y != dest.y) {
		DEBUGU("Updating %d (%d,%d)(%dx%d) to (%d,%d) on %d\n", buffer, src.x, src.y,
	   src.xl, src.yl, dest.x, dest.y, data_dest);
	} else {
		DEBUGU("Updating %d (%d,%d)(%dx%d) to %d\n", buffer, src.x, src.y, src.xl, src.yl, data_dest);
	}

	srect.x = src.x;
	srect.y = src.y;
	srect.w = src.xl;
	srect.h = src.yl;
	drect.x = dest.x;
	drect.y = dest.y;
	drect.w = src.xl;
	drect.h = src.yl;

	switch (buffer) {
	case GFX_BUFFER_BACK:
		if (sdl_blit_surface(drv, S->visual[data_source], &srect,
			S->visual[data_dest], &drect))
			SDLERROR("surface update failed!\n");

		if ((src.x == dest.x) && (src.y == dest.y))
			gfx_copy_pixmap_box_i(S->priority[0], S->priority[1], src);
		break;
	case GFX_BUFFER_FRONT:
		if (sdl_blit_surface(drv, S->visual[data_source], &srect, S->primary, &drect))
			SDLERROR("primary surface update failed!\n");
		SDL_UpdateRect(S->primary, drect.x, drect.y, drect.w, drect.h);
		break;
	default:
		GFXERROR("Invalid buffer %d in update!\n", buffer);
		return GFX_ERROR;
	}

	return GFX_OK;
}

static int
sdl_set_static_buffer(struct _gfx_driver *drv, gfx_pixmap_t *pic, gfx_pixmap_t *priority)
{

	if (!pic->internal.info) {
		SDLERROR("Attempt to set static buffer with unregisterd pixmap!\n");
		return GFX_ERROR;
	}
	sdl_blit_surface(drv, (SDL_Surface *)pic->internal.info, NULL,
		  S->visual[2], NULL);

	gfx_copy_pixmap_box_i(S->priority[1], priority, gfx_rect(0, 0, 320*XFACT, 200*YFACT));

	return GFX_OK;
}

	/*** Palette operations ***/

static int
sdl_set_palette(struct _gfx_driver *drv, int index, byte red, byte green, byte blue)
{
	if (index < 0 || index > 255)
	{
		SDLERROR("Attempt to set invalid palette entry %d\n", index);
		return GFX_ERROR;
	}

	S->colors[index].r = red;
	S->colors[index].g = green;
	S->colors[index].b = blue;

	SDL_SetColors(S->primary, S->colors + index, index, 1);
	return GFX_OK;
}


	/*** Mouse pointer operations ***/

byte *
sdl_create_cursor_rawdata(gfx_driver_t *drv, gfx_pixmap_t *pointer, int mode)
{
	int linewidth = (pointer->xl + 7) >> 3;
	int lines = pointer->yl;
	int xc, yc;
	byte *data = (byte*)sci_calloc(linewidth, lines);
	byte *linebase = data, *pos;
	byte *src = pointer->index_data;

	for (yc = 0; yc < pointer->index_yl; yc++) {
		int scalectr;
		int bitc = 7;
		pos = linebase;

		for (xc = 0; xc < pointer->index_xl; xc++) {
			int draw = mode ? (*src == 0) : (*src < 255);
			for (scalectr = 0; scalectr < XFACT; scalectr++) {
				if (draw)
				  *pos |= (1 << bitc);
				bitc--;
				if (bitc < 0) {
					bitc = 7;
					pos++;
				}
			}
			src++;
		}
		for (scalectr = 1; scalectr < YFACT; scalectr++)
			memcpy(linebase + linewidth * scalectr, linebase, linewidth);
		linebase += linewidth * YFACT;
	}
	return data;
}


static SDL_Cursor
*sdl_create_cursor_data(gfx_driver_t *drv, gfx_pixmap_t *pointer)
{
	byte *visual_data, *mask_data;

	S->pointer_data[0] = visual_data = sdl_create_cursor_rawdata(drv, pointer, 1);
	S->pointer_data[1] = mask_data = sdl_create_cursor_rawdata(drv, pointer, 0);

	return SDL_CreateCursor(visual_data, mask_data,
			  pointer->xl, pointer->yl,
			  pointer->xoffset, pointer->yoffset);

}

static int sdl_set_pointer (struct _gfx_driver *drv, gfx_pixmap_t *pointer)
{
	int i;

	if (pointer == NULL)
		SDL_ShowCursor(SDL_DISABLE);
	else {
		SDL_Cursor *cursor;
		for (i = 0; i < 2; i++)
			if (S->pointer_data[i]) {
				free(S->pointer_data[i]);
				S->pointer_data[i] = NULL;
			}

		cursor = SDL_GetCursor();
		SDL_SetCursor(sdl_create_cursor_data(drv, pointer));
		SDL_FreeCursor(cursor);
		SDL_ShowCursor(SDL_ENABLE);
	}

	return 0;
}

/*** Event management ***/

int
sdl_map_key(gfx_driver_t *drv, SDL_keysym keysym)
{
	SDLKey skey = keysym.sym;
	int rkey = keysym.unicode & 0x7f;

	if ((skey >= SDLK_a) && (skey <= SDLK_z))
		return ('a' + (skey - SDLK_a));

	if ((skey >= SDLK_0) && (skey <= SDLK_9))
		return ('0' + (skey - SDLK_0));

	if (flags & SCI_SDL_SWAP_CTRL_CAPS) {
		switch (skey) {
		case SDLK_LCTRL: skey = SDLK_CAPSLOCK; break;
		case SDLK_CAPSLOCK: skey = SDLK_LCTRL; break;
		default: break;
		}
	}

	switch (skey) {
		/* XXXX catch KMOD_NUM for KP0-9 */
	case SDLK_BACKSPACE: return SCI_K_BACKSPACE;
	case SDLK_TAB: return 9;
	case SDLK_ESCAPE: return SCI_K_ESC;
	case SDLK_RETURN:
	case SDLK_KP_ENTER:
		if (SDL_GetModState() & KMOD_ALT) {
			toggle_fullscreen(drv);
			return 0;
		}
		return SCI_K_ENTER;
	case SDLK_KP_PERIOD: return SCI_K_DELETE;
	case SDLK_KP0:
	case SDLK_INSERT: return SCI_K_INSERT;
	case SDLK_KP1:
	case SDLK_END: return SCI_K_END;
	case SDLK_KP2:
	case SDLK_DOWN: return SCI_K_DOWN;
	case SDLK_KP3:
	case SDLK_PAGEDOWN: return SCI_K_PGDOWN;
	case SDLK_KP4:
	case SDLK_LEFT: return SCI_K_LEFT;
	case SDLK_KP5: return SCI_K_CENTER;
	case SDLK_KP6:
	case SDLK_RIGHT: return SCI_K_RIGHT;
	case SDLK_KP7:
	case SDLK_HOME: return SCI_K_HOME;
	case SDLK_KP8:
	case SDLK_UP: return SCI_K_UP;
	case SDLK_KP9:
	case SDLK_PAGEUP: return SCI_K_PGUP;

	case SDLK_F1: return SCI_K_F1;
	case SDLK_F2: return SCI_K_F2;
	case SDLK_F3: return SCI_K_F3;
	case SDLK_F4: return SCI_K_F4;
	case SDLK_F5: return SCI_K_F5;
	case SDLK_F6: return SCI_K_F6;
	case SDLK_F7: return SCI_K_F7;
	case SDLK_F8: return SCI_K_F8;
	case SDLK_F9: return SCI_K_F9;
	case SDLK_F10: return SCI_K_F10;

	case SDLK_LCTRL:
	case SDLK_RCTRL:
	case SDLK_LALT:
	case SDLK_RALT:
	case SDLK_LMETA:
	case SDLK_RMETA:
	case SDLK_CAPSLOCK:
	case SDLK_SCROLLOCK:
	case SDLK_NUMLOCK:
	case SDLK_LSHIFT:
	case SDLK_RSHIFT:  return 0;

	case SDLK_PLUS:
	case SDLK_KP_PLUS: return '+';
	case SDLK_SLASH:
	case SDLK_KP_DIVIDE: return '/';
	case SDLK_MINUS:
	case SDLK_KP_MINUS: return '-';
	case SDLK_ASTERISK:
	case SDLK_KP_MULTIPLY: return '*';
	case SDLK_EQUALS:
	case SDLK_KP_EQUALS: return '=';

	case SDLK_COMMA:
	case SDLK_PERIOD:
	case SDLK_BACKSLASH:
	case SDLK_SEMICOLON:
	case SDLK_QUOTE:
	case SDLK_LEFTBRACKET:
	case SDLK_RIGHTBRACKET:
	case SDLK_LESS:
	case SDLK_DOLLAR:
	case SDLK_GREATER: return rkey;
	case SDLK_SPACE: return ' ';

#ifdef MACOSX
	case SDLK_WORLD_0:
#endif
	case SDLK_BACKQUOTE:
		if (keysym.mod & KMOD_CTRL)
			return '`';
		else
			return rkey;

	default:
		break;
	}

	sciprintf("Unknown SDL keysym: %04x (%d) \n", skey, rkey);
	return 0;
}


void
sdl_fetch_event(gfx_driver_t *drv, sci_event_t *sci_event)
{
	SDL_Event event;

	while (SDL_PollEvent(&event)) {

		switch (event.type) {
		case SDL_KEYDOWN: {
			int modifiers = event.key.keysym.mod;
			sci_event->type = SCI_EVT_KEYBOARD;

			S->buckystate = (((modifiers & KMOD_CAPS)? SCI_EVM_LSHIFT | SCI_EVM_RSHIFT : 0)
					 | ((modifiers & KMOD_CTRL)? SCI_EVM_CTRL : 0)
					 | ((modifiers & KMOD_ALT)? SCI_EVM_ALT : 0)
					 | ((modifiers & KMOD_NUM) ? SCI_EVM_NUMLOCK : 0)
					 | ((modifiers & KMOD_RSHIFT)? SCI_EVM_RSHIFT : 0)
					 | ((modifiers & KMOD_LSHIFT)? SCI_EVM_LSHIFT : 0));

			sci_event->buckybits = S->buckystate;
			sci_event->data = sdl_map_key(drv, event.key.keysym);
			if (sci_event->data)
				return;
			break;
		}
		case SDL_MOUSEBUTTONDOWN:
			sci_event->type = SCI_EVT_MOUSE_PRESS;
			sci_event->buckybits = S->buckystate;
			sci_event->data = event.button.button - 1;
			drv->pointer_x = event.button.x;
			drv->pointer_y = event.button.y;
			return;
		case SDL_MOUSEBUTTONUP:
			sci_event->type = SCI_EVT_MOUSE_RELEASE;
			sci_event->buckybits = S->buckystate;
			sci_event->data = event.button.button - 1;
			drv->pointer_x = event.button.x;
			drv->pointer_y = event.button.y;
			return;
		case SDL_MOUSEMOTION:
			drv->pointer_x = event.motion.x;
			drv->pointer_y = event.motion.y;
			break;
		case SDL_QUIT:
			sci_event->type = SCI_EVT_QUIT;
			return;
			break;
		case SDL_VIDEOEXPOSE:
			break;
		default:
			SDLERROR("Received unhandled SDL event %04x\n", event.type);
		}
	}

	sci_event->type = SCI_EVT_NONE; /* No event. */
}

static sci_event_t
sdl_get_event(struct _gfx_driver *drv)
{
	sci_event_t input;

	sdl_fetch_event(drv, &input);
	return input;
}

static int
sdl_usec_sleep(struct _gfx_driver *drv, long usecs)
{
	int msecs;
	SDL_Event event;

	/* Wait at most 10ms to keep mouse cursor responsive. */
	msecs = usecs / 1000;
	if (msecs > 10)
		msecs = 10;

	SDL_PumpEvents();
	while (SDL_PeepEvents(&event, 1, SDL_GETEVENT,
	    SDL_EVENTMASK(SDL_MOUSEMOTION)) == 1) {
		drv->pointer_x = event.motion.x;
		drv->pointer_y = event.motion.y;
	}

	SDL_Delay(msecs);

	return GFX_OK;
}

gfx_driver_t
gfx_driver_sdl = {
	"sdl",
	"0.3a",
	SCI_GFX_DRIVER_MAGIC,
	SCI_GFX_DRIVER_VERSION,
	NULL,
	0, 0,
	GFX_CAPABILITY_MOUSE_SUPPORT | GFX_CAPABILITY_MOUSE_POINTER
	| GFX_CAPABILITY_PIXMAP_REGISTRY | GFX_CAPABILITY_FINE_LINES,
	0, /*GFX_DEBUG_POINTER | GFX_DEBUG_UPDATES | GFX_DEBUG_PIXMAPS | GFX_DEBUG_BASIC, */
	sdl_set_parameter,
	sdl_init_specific,
	sdl_init,
	sdl_exit,
	sdl_draw_line,
	sdl_draw_filled_rect,
	sdl_register_pixmap,
	sdl_unregister_pixmap,
	sdl_draw_pixmap,
	sdl_grab_pixmap,
	sdl_update,
	sdl_set_static_buffer,
	sdl_set_pointer,
	sdl_set_palette,
	sdl_get_event,
	sdl_usec_sleep,
	NULL
};

#endif /* HAVE_SDL */


/* reset to original optimisations for Win32: */
/* (does not reset intrinsics) */
#ifdef _WIN32
//#pragma optimize( "", on )
#endif
