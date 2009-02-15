/***************************************************************************
 directfb_driver.c Copyright (C) 2002 Christoph Reichenbach


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
/* FreeSCI 0.3.1+ graphics driver module for libdirectfb */


#include <stdarg.h>
#include <gfx_system.h>
#include <gfx_driver.h>
#include <gfx_tools.h>
#include <assert.h>
#include <uinput.h>

#ifdef HAVE_DIRECTFB

#include <stdio.h>
#include <stdlib.h>
#include <directfb/directfb.h>
#include <unistd.h>

#define SCIDFB_DRIVER_VERSION "0.4"

typedef struct _event_queue_struct {
	sci_event_t evt;
	struct _event_queue_struct *next;
} event_queue_t;

/*
** For this driver to work to the fullest of its capabilities, the user running it
** must have rw permissions on the following:
** /dev/tty0 (including VT_ACTIVATE perms)
** /dev/tty7
** /dev/fb0
** /dev/psaux (if using a PS/2 mouse)
**
** VT_ACTIVATE permissions are, AFAIK, only available to root ATM :-(
*/

#define SCIDFB_CAP_ALPHA (1 << 0) /* Do we have an Alpha channel? */

#define SCIDFB_MAY_ALPHA (scidfb_internal_caps & SCIDFB_CAP_ALPHA)

/* Global data */

static IDirectFB *scidfb_framebuffer;		/* The global framebuffer */
static IDirectFBDisplayLayer *scidfb_layer;	/* The layer we use for drawing */
static IDirectFBSurface *scidfb_visual; /* Static visual buffer */
static IDirectFBSurface *scidfb_static_visual; /* Static visual buffer */
static gfx_pixmap_t *scidfb_static_priority = NULL;
static gfx_pixmap_t *scidfb_back_priority = NULL;
static int scidfb_internal_caps = 0;	/* Certain internal capabilities available to us */

/* Input structures */
static IDirectFBEventBuffer *scidfb_input_buffer = NULL;
static event_queue_t *scidfb_event_queue = NULL;
static unsigned int scidfb_buckybits = SCI_EVM_INSERT; 


/*#define LIST_ALL_DFB_MODES */ /* Uncomment for minimal debugging */

#define SCIDFB_CHECKED(f) \
	{								\
		DFBResult _r = (f);					\
		if (_r != DFB_OK) {					\
			scidfb_handle_error(_r, __FILE__, __LINE__);	\
			return GFX_ERROR;				\
		}							\
	}

#define SCIGFX_CHECKED(f) \
	{								\
		DFBResult _r = (f);					\
		if (_r != GFX_OK) {					\
			return _r;					\
		}							\
	}


static void
scidfb_handle_error(DFBResult errc, char *file, int line)
{
	const char *err = DirectFBErrorString(errc);

	GFXERROR("DFB-GFX, %s L%d: %s\n", file, line, err);
}


typedef struct {
	int x, y, bpp;
} scidfb_mode_t;


static DFBEnumerationResult
scidfb_mode_callback(unsigned int width, unsigned int height, unsigned int bpp, void *conf)
{
	scidfb_mode_t *aim = (scidfb_mode_t *) conf;
	scidfb_mode_t *best = aim + 1;

	if ((width >= aim->x && height >= aim->y && bpp >= aim->bpp)
	    /* Sufficient in all respects... */
	    && ((best->bpp == 0) /* First mode that matched */
		|| (width < best->x && height <= best->y) /* Improvement on x */
		|| (width <= best->x && height < best->y) /* Improvement on y */
		|| (width <= best->x && height <= best->y && bpp < best->bpp)
		/* Improvement on bpp */
		)) {
		best->x = width;
		best->y = height;
		best->bpp = bpp;
	}

#ifdef LIST_ALL_DFB_MODES
	GFXERROR("DFB-GFX: Supports %dx%d at %d bpp\n", width, height, bpp);
	return DFENUM_OK;
#endif

	if (aim->x == best->x
	    && aim->y == best->y
	    && aim->bpp == best->bpp)
		return DFENUM_CANCEL; /* We have what we were looking for */

	return DFENUM_OK; /* Continue searching */
}


static DFBEnumerationResult
scidfb_layer_callback(unsigned int layer_id, DFBDisplayLayerCapabilities caps, void *resultp)
{
	unsigned int *results = (unsigned int *) resultp;

	if (!(caps & DLCAPS_SURFACE))
		return DFENUM_OK; /* We need a surface */

	if (!results[1])
		results[0] = layer_id;

	results[1] = 1;

	if (caps & DLCAPS_ALPHACHANNEL) {
		/* Optimal! */
		scidfb_internal_caps |= SCIDFB_MAY_ALPHA;
		return DFENUM_CANCEL; /* We're done */
	}

	/* Found something, but it might get better */
	return DFENUM_OK;
}

#define LOOKING_FOR_POINTING_DEVICE 1
#define LOOKING_FOR_KEYBOARD 2
#define FOUND_IT 255

static DFBEnumerationResult
scidfb_input_callback(unsigned int device_id, DFBInputDeviceDescription descr, void *p)
{
	int *state = (int *) p;
	int *result = state + 1;

	/* Check for mouse */
	if (*state == LOOKING_FOR_POINTING_DEVICE
	    && (descr.type & DIDTF_MOUSE)) {
		*result = device_id;
		*state = FOUND_IT;
		return DFENUM_CANCEL;
	}

	/* Check for keyboard */
	if (*state == LOOKING_FOR_KEYBOARD
	    && (descr.type & DIDTF_KEYBOARD)) {
		*result = device_id;
		*state = FOUND_IT;
		return DFENUM_CANCEL;
	}

	return DFENUM_OK;
}

static int
scidfb_find_layer()
{
	unsigned int results[2];

	results[1] = 0;
	SCIDFB_CHECKED(scidfb_framebuffer->EnumDisplayLayers(scidfb_framebuffer,
						      scidfb_layer_callback, &results));

	if (!results[1])
		return GFX_FATAL; /* No decent layer! */
	else {
		SCIDFB_CHECKED(scidfb_framebuffer->GetDisplayLayer(scidfb_framebuffer,
							    results[0],
							    &scidfb_layer));
		
		return GFX_OK;
	}
}

static int
_scidfb_init_input(int *found_keyboard, int *found_mouse)
{
	int inputs[2];

	inputs[0] = LOOKING_FOR_POINTING_DEVICE;
	SCIDFB_CHECKED(scidfb_framebuffer->EnumInputDevices(scidfb_framebuffer,
							    scidfb_input_callback, &inputs));

	if ((*found_mouse = (inputs[0] == FOUND_IT))) {
		IDirectFBInputDevice *mouse;
		SCIDFB_CHECKED(scidfb_framebuffer->GetInputDevice(scidfb_framebuffer,
								  inputs[1], &mouse));

		SCIDFB_CHECKED(mouse->CreateEventBuffer(mouse, &scidfb_input_buffer));
	}

	inputs[0] = LOOKING_FOR_KEYBOARD;
	SCIDFB_CHECKED(scidfb_framebuffer->EnumInputDevices(scidfb_framebuffer,
							    scidfb_input_callback, &inputs));
	if ((*found_keyboard =  (inputs[0] == FOUND_IT))) {
		IDirectFBInputDevice *keyboard;
		SCIDFB_CHECKED(scidfb_framebuffer->GetInputDevice(scidfb_framebuffer,
								  inputs[1], &keyboard));

		if (scidfb_input_buffer)
			SCIDFB_CHECKED(keyboard->AttachEventBuffer(keyboard, scidfb_input_buffer))
		else 
			SCIDFB_CHECKED(keyboard->CreateEventBuffer(keyboard,
								   &scidfb_input_buffer));
	}

	
	SCIDFB_CHECKED(scidfb_framebuffer->CreateEventBuffer(scidfb_framebuffer,
							     DICAPS_KEYS
							     | DICAPS_AXES
							     | DICAPS_BUTTONS,
							     &scidfb_input_buffer));
}

static int
_scidfb_init_gfx_mode(int width, int height, int bpp)
{
	scidfb_mode_t modes[2];
	modes[0].x = width;
	modes[0].y = height;
	modes[0].bpp = bpp;

	modes[1].bpp = 0; /* nothing found yet */

	SCIDFB_CHECKED(DirectFBCreate(&scidfb_framebuffer));
	SCIDFB_CHECKED(scidfb_framebuffer->EnumVideoModes(scidfb_framebuffer,
							  scidfb_mode_callback, &modes));

	if (modes[1].bpp) {
		DFBDisplayLayerConfig conf;

		if (scidfb_find_layer()) {
			GFXERROR("DFB-GFX: No useable display layer found");
			return GFX_FATAL;
		}

		SCIDFB_CHECKED(scidfb_framebuffer->SetVideoMode(scidfb_framebuffer,
							 modes[1].x, modes[1].y, modes[1].bpp));

		SCIDFB_CHECKED(scidfb_layer->SetCooperativeLevel(scidfb_layer, DLSCL_EXCLUSIVE));
		SCIDFB_CHECKED(scidfb_layer->GetSurface(scidfb_layer, &scidfb_visual));

		return GFX_OK;
	} else
		GFXERROR("DFB-GFX: No matching visual could be found!\n");

	return GFX_FATAL;
}


static int
scidfb_build_virtual_surface(IDirectFBSurface **result, int width, int height,
			     IDirectFBSurface *prototype)
{
	/* This function creates a _virtual_ surface, i.e. a memory buffer,
	** without flipping support  */
	DFBSurfaceDescription srf_desc;
	IDirectFBSurface *surface;

	SCIDFB_CHECKED(prototype->GetCapabilities(prototype, &(srf_desc.caps)));
	srf_desc.caps &= ~DSCAPS_FLIPPING;
	srf_desc.width = width;
	srf_desc.height = height;
	SCIDFB_CHECKED(prototype->GetPixelFormat(prototype, &(srf_desc.pixelformat)));
	srf_desc.flags = (DSDESC_CAPS | DSDESC_WIDTH | DSDESC_HEIGHT | DSDESC_PIXELFORMAT);
	SCIDFB_CHECKED(scidfb_framebuffer->CreateSurface(scidfb_framebuffer, &srf_desc, &surface));
	SCIDFB_CHECKED(surface->Clear(surface, 0, 0, 0, 255));

	*result = surface;

	return GFX_OK;
}

static int
scidfb_deactivate_pointer()
{
		SCIDFB_CHECKED(scidfb_layer->EnableCursor(scidfb_layer, 0));
		return 0;
}


  /*----------------------------------------------------------------*/
/*-----------          Event queue implementation        --------------*/
  /*----------------------------------------------------------------*/

#define MILLION 1000000

static int
scidfb_xlate_key(DFBInputDeviceKeyIdentifier key, int keysym)
{
	if ((keysym >= 'A') && (keysym <= 'Z'))
		return keysym;
	if ((keysym >= 'a') && (keysym <= 'z'))
		return keysym;
	if ((keysym >= '0') && (keysym <= '9'))
		return keysym;

	switch (key) {

	case DIKI_0: return '0';
	case DIKI_1: return '1';
	case DIKI_2: return '2';
	case DIKI_3: return '3';
	case DIKI_4: return '4';
	case DIKI_5: return '5';
	case DIKI_6: return '6';
	case DIKI_7: return '7';
	case DIKI_8: return '8';
	case DIKI_9: return '9';

	case DIKI_F1: return SCI_K_F1;
	case DIKI_F2: return SCI_K_F2;
	case DIKI_F3: return SCI_K_F3;
	case DIKI_F4: return SCI_K_F4;
	case DIKI_F5: return SCI_K_F5;
	case DIKI_F6: return SCI_K_F6;
	case DIKI_F7: return SCI_K_F7;
	case DIKI_F8: return SCI_K_F8;
	case DIKI_F9: return SCI_K_F9;
	case DIKI_F10: return SCI_K_F10;

	case DIKI_KP_0:
	case DIKI_INSERT: scidfb_buckybits ^= SCI_EVM_INSERT;
		return SCI_K_INSERT;

	case DIKI_ESCAPE: return SCI_K_ESC;
	case DIKI_LEFT: return SCI_K_LEFT;
	case DIKI_RIGHT: return SCI_K_RIGHT;
	case DIKI_UP: return SCI_K_UP;
	case DIKI_DOWN: return SCI_K_DOWN;
	case DIKI_TAB: return SCI_K_TAB;
	case DIKI_ENTER: return SCI_K_ENTER;
	case DIKI_SPACE: return ' ';
	case DIKI_BACKSPACE: return SCI_K_BACKSPACE;
	case DIKI_DELETE: return SCI_K_DELETE;
	case DIKI_HOME: return SCI_K_HOME;
	case DIKI_END: return SCI_K_END;
	case DIKI_PAGE_UP: return SCI_K_PGDOWN;
	case DIKI_PAGE_DOWN: return SCI_K_PGUP;

	case DIKI_QUOTE_LEFT: return '`';
	case DIKI_MINUS_SIGN: return '-';
	case DIKI_EQUALS_SIGN: return '=';
	case DIKI_BRACKET_LEFT: return '[';
	case DIKI_BRACKET_RIGHT: return ']';
	case DIKI_BACKSLASH: return '\\';
	case DIKI_SEMICOLON: return ';';
	case DIKI_QUOTE_RIGHT: return '\'';
	case DIKI_COMMA: return ',';
	case DIKI_SLASH: return '/';
	case DIKI_PERIOD: return '.';

	case DIKI_KP_DIV: return '/';
	case DIKI_KP_MULT: return '*';
	case DIKI_KP_MINUS: return '-';
	case DIKI_KP_PLUS: return '+';
	case DIKI_KP_ENTER: return SCI_K_ENTER;
	case DIKI_KP_SPACE: return ' ';
	case DIKI_KP_TAB: return SCI_K_TAB;
	case DIKI_KP_F1: return SCI_K_F1;
	case DIKI_KP_F2: return SCI_K_F2;
	case DIKI_KP_F3: return SCI_K_F3;
	case DIKI_KP_F4: return SCI_K_F4;
	case DIKI_KP_EQUAL: return '=';
          
	case DIKI_KP_DECIMAL: return SCI_K_DELETE;
	case DIKI_KP_1: return SCI_K_END;
	case DIKI_KP_2: return SCI_K_DOWN;
	case DIKI_KP_3: return SCI_K_PGDOWN;
	case DIKI_KP_4: return SCI_K_LEFT;
	case DIKI_KP_5: return SCI_K_CENTER;
	case DIKI_KP_6: return SCI_K_RIGHT;
	case DIKI_KP_7: return SCI_K_HOME;
	case DIKI_KP_8: return SCI_K_UP;
	case DIKI_KP_9: return SCI_K_PGUP;

	default:

	}
	return 0; /* Could not map key */
}


scidfb_handle_bucky(int add, DFBInputDeviceKeyIdentifier key)
{
	int modifier = 0;

	switch(key) {
	case DIKI_SHIFT_L: modifier = SCI_EVM_LSHIFT;
		break;
	case DIKI_SHIFT_R: modifier = SCI_EVM_RSHIFT;
		break;
	case DIKI_CONTROL_L:
	case DIKI_CONTROL_R: modifier = SCI_EVM_CTRL;
		break;
	case DIKI_ALT_L:
	case DIKI_ALT_R:
	case DIKI_META_L:
	case DIKI_META_R: modifier = SCI_EVM_ALT;
		break;
	case DIKI_CAPS_LOCK: modifier = SCI_EVM_CAPSLOCK;
		break;
	case DIKI_NUM_LOCK: modifier = SCI_EVM_NUMLOCK;
		break;
	case DIKI_SCROLL_LOCK: modifier = SCI_EVM_SCRLOCK;
		break;
	}

	if (add)
		scidfb_buckybits |= modifier;
	else
		scidfb_buckybits &= ~modifier;
}

static sci_event_t
scidfb_xlate_event(gfx_driver_t *drv, DFBEvent dfb_pre_evt)
{
	sci_event_t retval;

	retval.type = SCI_EVT_NONE;
	retval.buckybits = scidfb_buckybits;

	if (dfb_pre_evt.clazz == DFEC_INPUT) { /* What kind of idiot named these field members? */
		DFBInputEvent dfbevt = dfb_pre_evt.input;
		switch (dfbevt.type) {

		case DIET_KEYPRESS:
			retval.type = SCI_EVT_KEYBOARD;
			scidfb_handle_bucky(1, dfbevt.key_id);
			retval.data = scidfb_xlate_key(dfbevt.key_id, dfbevt.key_symbol);
			if (retval.data == 0)
				retval.type = SCI_EVT_NONE;
			break;

		case DIET_KEYRELEASE:
			scidfb_handle_bucky(0, dfbevt.key_id);
			break;

		case DIET_BUTTONPRESS:
			retval.type = SCI_EVT_MOUSE_PRESS;
			retval.data = dfbevt.button - DIBI_FIRST;
			break;

		case DIET_AXISMOTION:{
			int *victim = NULL;

			if (dfbevt.axis == DIAI_X)
				victim = &(drv->pointer_x);
			else if (dfbevt.axis == DIAI_Y)
				victim = &(drv->pointer_y);
			else break;

			if (dfbevt.flags & DIEF_AXISABS)
				*victim = dfbevt.axisabs;
			else if (dfbevt.flags & DIEF_AXISREL)
				*victim += dfbevt.axisrel;
		}
			break;

		case DIET_BUTTONRELEASE:
			retval.type = SCI_EVT_MOUSE_RELEASE;
			retval.data = dfbevt.button - DIBI_FIRST;
			break;
		}
	}

	return retval;
}

static void
scidfb_queue_event(sci_event_t evt)
{
	event_queue_t *node = malloc(sizeof(event_queue_t));
	event_queue_t **seekerp = &scidfb_event_queue;

	node->evt = evt;
	node->next = NULL;

	while (*seekerp)
		seekerp = &((*seekerp)->next);

	*seekerp = node;
}


static int
scidfb_queue_next_event(gfx_driver_t *drv, long micros_to_wait)
{
	DFBEvent dfb_event;
	long secs = micros_to_wait / MILLION;
	long usecs = (micros_to_wait - (secs * MILLION)) / 1000;
	int has_event;
	DFBResult timeout;

	timeout = scidfb_input_buffer->WaitForEventWithTimeout(scidfb_input_buffer,
									 secs, usecs);
	if (timeout != DFB_TIMEOUT)
		SCIDFB_CHECKED(timeout);

	has_event = scidfb_input_buffer->HasEvent(scidfb_input_buffer);
	if (has_event == DFB_OK) {
		sci_event_t evt;
		/* We have a new event */
		SCIDFB_CHECKED(scidfb_input_buffer->GetEvent(scidfb_input_buffer, &dfb_event));
		evt = scidfb_xlate_event(drv, dfb_event);

		if (evt.type != SCI_EVT_NONE)
			scidfb_queue_event(evt);

		return 0;
	} else
		if (has_event != DFB_BUFFEREMPTY)
			return has_event; /* An error occured */
}


static int
_scidfb_read_event(sci_event_t *evtp)
{
	if (scidfb_event_queue) {
		event_queue_t *q = scidfb_event_queue;
		*evtp = q->evt;
		scidfb_event_queue = q->next;
		free(q);
		
		return 1;
	} else {
		evtp->type = SCI_EVT_NONE;
		return 0;
	}
}




  /*----------------------------------------------------------------*/
/*----------- Implementations required by gfx_driver_t  --------------*/
  /*----------------------------------------------------------------*/


static int
scidfb_set_parameter(gfx_driver_t *drv, char *name, char *value)
{
	fprintf(stderr, "FIXME: These must be caught and moved in between DirectFBInit() and ..Create()\n");
	exit(1);
	SCIDFB_CHECKED(DirectFBSetOption(name, value));
	return GFX_OK;
}

static int
_scidfb_decode_pixel_format(DFBSurfacePixelFormat pixel_format,
			    int *rm, int *gm, int *bm, int *am,
			    int *rs, int *gs, int *bs, int *as, int *bytespp)
{
	/* Initially, the masks are set to the number of bits they occupy, and shifts
	** are left shifts */
	int _rs, _gs, _bs, _as;

	_as = 0;
	*am = 0;
	*as = 0;

	switch (pixel_format) {

	case DSPF_RGB15:
		*bytespp = 2;
		*rm = 5; _rs = 10;
		*gm = 5; _gs = 5;
		*bm = 5; _bs = 0;
		break;

	case DSPF_RGB16:
		*bytespp = 2;
		*rm = 5; _rs = 11;
		*gm = 6; _gs = 5;
		*bm = 5; _bs = 0;
		break;

	case DSPF_RGB24:
		*bytespp = 3;
		*rm = 8; _rs = 16;
		*gm = 8; _gs = 8;
		*bm = 8; _bs = 0;
		break;

	case DSPF_RGB32:
		*bytespp = 4;
		*rm = 8; _rs = 16;
		*gm = 8; _gs = 8;
		*bm = 8; _bs = 0;
		break;

	case DSPF_ARGB:
		*bytespp = 4;
		*am = 8; _as = 24;
		*rm = 8; _rs = 16;
		*gm = 8; _gs = 8;
		*bm = 8; _bs = 0;
		break;

	case DSPF_RGB332:
		*bytespp = 1;
		*rm = 3; _rs = 5;
		*gm = 3; _gs = 2;
		*bm = 2; _bs = 0;
		break;

	default:
		GFXERROR("DFB-GFX: Unknown/unsupported pixel format %08x\n", pixel_format);
		return GFX_FATAL;
	}

	/* Calculate correct shifts */
	*rs = 32 - *rm - _rs;
	*gs = 32 - *gm - _gs;
	*bs = 32 - *bm - _bs;
	*as = 32 - *am - _as;

	/* Calculate correct masks */
	*rm = ((1 << (*rm)) - 1) << _rs;
	*gm = ((1 << (*gm)) - 1) << _gs;
	*bm = ((1 << (*bm)) - 1) << _bs;
	*am = ((1 << (*am)) - 1) << _as;

	return GFX_OK;
}

static int
scidfb_init_specific(gfx_driver_t *drv, int xres, int yres, int bytespp)
{
	DFBSurfacePixelFormat pixel_format;
	char **p;
	char *foo = "";
	int n = 1;

	int width, height;
	int red_mask, green_mask, blue_mask, alpha_mask;
	int red_shift, green_shift, blue_shift, alpha_shift;
	int bytespp_real;
	int mouse, keyboard;

	p = &foo;
	DirectFBInit(&n, &p);

	SCIDFB_CHECKED(DirectFBSetOption("no-sighandler", "1"));

	SCIGFX_CHECKED(_scidfb_init_gfx_mode(xres, yres, (1 << bytespp) - 1));
	SCIGFX_CHECKED(_scidfb_init_input(&keyboard, &mouse));

	if (!keyboard) {
		GFXERROR("DFB-GFX: No keyboard found-- aborting...\n");
		return GFX_FATAL;
	}
	if (!mouse) {
		GFXWARN("DFB-GFX: No pointing device found, disabling mouse support...\n");
		drv->capabilities &= ~GFX_CAPABILITY_MOUSE_SUPPORT;
	}

	SCIDFB_CHECKED(scidfb_visual->GetSize(scidfb_visual, &width, &height));

	scidfb_deactivate_pointer();
	SCIDFB_CHECKED(scidfb_visual->Clear(scidfb_visual, 0, 0, 0, 255));
	SCIDFB_CHECKED(scidfb_visual->Flip(scidfb_visual, NULL, DSFLIP_BLIT));
	

	/* If neccessary, create a sub-surface */
	if (width > xres * 320 || height > yres * 200) {
		IDirectFBSurface *subsurface;
		DFBRectangle region;
		region.w = xres * 320;
		region.h = yres * 200;
		region.x = (width - region.w) >> 1;
		region.y = (height - region.h) >> 1;

		SCIDFB_CHECKED(scidfb_visual->GetSubSurface(scidfb_visual, &region, &subsurface));

		scidfb_visual = subsurface;
	}

	scidfb_visual->SetDrawingFlags(scidfb_visual, DSDRAW_BLEND); /* Unchecked: It's just a
								     ** feature  */
	SCIDFB_CHECKED(scidfb_visual->GetPixelFormat(scidfb_visual, &pixel_format));

	SCIGFX_CHECKED(_scidfb_decode_pixel_format(pixel_format,
						   &red_mask, &green_mask, &blue_mask, &alpha_mask,
						   &red_shift, &green_shift, &blue_shift, &alpha_shift,
						   &bytespp_real));

	fprintf(stderr, "Mode %08x -> (%x>>%d, %x>>%d, %x>>%d, %x>>%d) at %d bytespp\n",
		pixel_format,
		red_mask, red_shift,
		green_mask, green_shift,
		blue_mask, blue_shift,
		alpha_mask, alpha_shift,
		bytespp_real);

	drv->mode = gfx_new_mode(xres, yres,
				 bytespp_real,
				 red_mask, green_mask, blue_mask, alpha_mask,
				 red_shift, green_shift, blue_shift, alpha_shift,
				 0 /* not palette mode*/,
				 0);

	SCIGFX_CHECKED(scidfb_build_virtual_surface(&scidfb_static_visual,
						    drv->mode->xfact * 320,
						    drv->mode->yfact * 200,
						    scidfb_visual));

	scidfb_back_priority =
		gfx_pixmap_alloc_index_data(gfx_new_pixmap(drv->mode->xfact * 320,
							   drv->mode->yfact * 200,
							   GFX_RESID_NONE, 0, 0));

	return GFX_OK;
}

static int
_scidfb_set_color(gfx_color_t *c)
{
	SCIDFB_CHECKED(scidfb_visual->SetColor(scidfb_visual,
					       c->visual.r, c->visual.g, c->visual.b,
					       (c->mask & GFX_MASK_VISUAL)? (255 - c->alpha) : 0));
	return GFX_OK;
}

static int
scidfb_init(gfx_driver_t *drv)
{
	return scidfb_init_specific(drv, 2, 2, 1);
}


static void
scidfb_exit(gfx_driver_t *drv)
{
	gfx_free_pixmap(drv, scidfb_back_priority);
}

static int
scidfb_draw_line(gfx_driver_t *drv, point_t start, point_t end, gfx_color_t color,
		 gfx_line_mode_t line_mode, gfx_line_style_t line_style)
{
	int xc, yc;

	SCIGFX_CHECKED(_scidfb_set_color(&color));

	if (line_mode == GFX_LINE_MODE_FINE) {
		SCIDFB_CHECKED(scidfb_visual->DrawLine(scidfb_visual,
						       start.x, start.y,
						       end.x, end.y));
	} else /* "Correct" lines */
		for (xc = 0; xc < drv->mode->xfact; xc++)
			for (yc = 0; yc < drv->mode->yfact; yc++)
				SCIDFB_CHECKED(scidfb_visual->DrawLine(scidfb_visual,
								       start.x + xc,
								       start.y + yc,
								       end.x + xc,
								       end.y + yc));
	
	return GFX_OK;
}

static int
scidfb_draw_filled_rect(gfx_driver_t *drv, rect_t rect,
			gfx_color_t color1, gfx_color_t color2,
			gfx_rectangle_fill_t shade_mode)
{
	SCIGFX_CHECKED(_scidfb_set_color(&color1));
	SCIDFB_CHECKED(scidfb_visual->FillRectangle(scidfb_visual,
						    rect.x, rect.y,
						    rect.xl, rect.yl));

	return GFX_OK;
}

static int
scidfb_register_pixmap(gfx_driver_t *drv, gfx_pixmap_t *pxm)
{
	return GFX_OK;
}


static int
scidfb_unregister_pixmap(gfx_driver_t *drv, gfx_pixmap_t *pxm)
{
	return GFX_OK;
}


static int
scidfb_draw_pixmap(gfx_driver_t *drv, gfx_pixmap_t *pxm, int priority,
		   rect_t src, rect_t dest, gfx_buffer_t buffer)
{
	void *_dest_data;
	byte *dest_data;
	int line_width;
        IDirectFBSurface *visual = (buffer == GFX_BUFFER_STATIC)?
		scidfb_static_visual : scidfb_visual;
	gfx_pixmap_t *priority_map = (buffer == GFX_BUFFER_STATIC)?
		scidfb_static_priority : scidfb_back_priority;

	SCIDFB_CHECKED(visual->Lock(visual, DSLF_WRITE | DSLF_READ,
				    &_dest_data, &line_width));
	dest_data = (byte *) _dest_data;

	gfx_crossblit_pixmap(drv->mode, pxm, priority, src, dest, dest_data,
			     line_width,
			     priority_map->index_data, priority_map->index_xl,
			     1, 0);

	SCIDFB_CHECKED(visual->Unlock(visual));
}

static int
scidfb_grab_pixmap(gfx_driver_t *drv, rect_t src, gfx_pixmap_t *pxm,
		   gfx_map_mask_t map)
{
	void *_src_data;
	byte *src_data;
	byte *dest = pxm->data;
	int line_width;
	int write_width = drv->mode->bytespp * (src.xl);

	pxm->xl = src.xl;
	pxm->yl = src.yl;

	if (map != GFX_MASK_VISUAL) {
		fprintf(stderr, "Not trying to read from visual mask-- not supported!");
		return GFX_FATAL;
	}

	SCIDFB_CHECKED(scidfb_visual->Lock(scidfb_visual, DSLF_READ | DSLF_WRITE,
					   /* Must use DSLF_WRITE to choose the back buffer,
					   ** otherwise the front buffer would be chosen */
					   &_src_data, &line_width));
	src_data = (byte *) _src_data;

	src_data += (drv->mode->bytespp * src.x)
		+ (line_width * src.y); /* left upper corner of the source */

	if (src.y < 0){
		fprintf(stderr, "src.y=%d\n", src.y);
		exit(1);
	}

	while (src.yl--) {
		memcpy(dest, src_data, write_width);
		src_data += line_width;
		dest += write_width;
	}

	
	SCIDFB_CHECKED(scidfb_visual->Unlock(scidfb_visual));
}

static int
scidfb_update(gfx_driver_t *drv, rect_t src, point_t dest, gfx_buffer_t buffer)
{
	if (src.x != dest.x
	    || src.y != dest.y) {
		GFXERROR("DFB-GFX: Attempt to update from (%d,%d,%d,%d) to (%d,%d)\n",
			 GFX_PRINT_RECT(src), dest.x, dest.y);
	}

	if (buffer == GFX_BUFFER_FRONT) {
		DFBRegion region;
		region.x1 = src.x;
		region.y1 = src.y;
		region.x2 = src.x + src.xl;
		region.y2 = src.y + src.yl;

		SCIDFB_CHECKED(scidfb_visual->Flip(scidfb_visual, &region, DSFLIP_BLIT));
	} else { /* Back buffer update */
		DFBRectangle dest_rect;
		dest_rect.x = src.x;
		dest_rect.y = src.y;
		dest_rect.w = src.xl;
		dest_rect.h = src.yl;

		SCIDFB_CHECKED(scidfb_visual->Blit(scidfb_visual, 
						   scidfb_static_visual,
						   &dest_rect,
						   dest.x, dest.y));

		/* Now update the priority map: */
		if (scidfb_static_priority) /* only if the buffers have been initialized */
			gfx_copy_pixmap_box_i(scidfb_back_priority, scidfb_static_priority, src);
	}

	return GFX_OK;
}

static int
scidfb_set_static_buffer(gfx_driver_t *drv, gfx_pixmap_t *pic, gfx_pixmap_t *priority)
{
	void *_dest;
	byte *dest;
	byte *src = pic->data;
	int line_width;
	int draw_width = pic->xl * drv->mode->bytespp;
	int i;

	scidfb_static_priority = priority;
	SCIDFB_CHECKED(scidfb_static_visual->Lock(scidfb_static_visual, DSLF_WRITE,
						&_dest, &line_width));
	dest = (byte *) _dest;
	
	for (i = 0; i < pic->yl; i++) {
		memcpy(dest, src, draw_width);
		dest += line_width;
		src += draw_width;
	}

	SCIDFB_CHECKED(scidfb_static_visual->Unlock(scidfb_static_visual));

	return GFX_OK;
}

static void
_normalize_pointer(gfx_driver_t *drv)
{
	int maxw = drv->mode->xfact * 320;
	int maxh = drv->mode->yfact * 200;

	if (drv->pointer_x < 0)
		drv->pointer_x = 0;
	if (drv->pointer_x >= maxw)
		drv->pointer_x = maxw - 1;
	if (drv->pointer_y < 0)
		drv->pointer_y = 0;
	if (drv->pointer_y >= maxh)
		drv->pointer_y = maxh - 1;
}


static sci_event_t
scidfb_get_event(gfx_driver_t *drv)
{
	sci_event_t retval;

	scidfb_queue_next_event(drv, 0);
	_scidfb_read_event(&retval);

	_normalize_pointer(drv);


	return retval;
}

static int
scidfb_usec_sleep(gfx_driver_t *drv, long usecs)
{
	scidfb_queue_next_event(drv, usecs);
	_normalize_pointer(drv);
	return GFX_OK;
}

gfx_driver_t gfx_driver_dfb = {
	"dfb",
	SCIDFB_DRIVER_VERSION,
	SCI_GFX_DRIVER_MAGIC,
	SCI_GFX_DRIVER_VERSION,
	NULL,
	0, 0,
	GFX_CAPABILITY_MOUSE_SUPPORT
	| GFX_CAPABILITY_FINE_LINES,
	0,
	scidfb_set_parameter,
	scidfb_init_specific,
	scidfb_init,
	scidfb_exit,
	scidfb_draw_line,
	scidfb_draw_filled_rect,
	scidfb_register_pixmap,
	scidfb_unregister_pixmap,
	scidfb_draw_pixmap,
	scidfb_grab_pixmap,
	scidfb_update,
	scidfb_set_static_buffer,
	NULL,
	NULL,
	scidfb_get_event,
	scidfb_usec_sleep,
	NULL
};

#endif
