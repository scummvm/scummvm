/***************************************************************************
 xlib_driver.h Copyright (C) 2000 Christoph Reichenbach


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

    Christoph Reichenbach (CR) <creichen@gmail.com>

***************************************************************************/

#include <sci_memory.h>
#include <gfx_driver.h>
#ifndef X_DISPLAY_MISSING
#include <gfx_tools.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>

#ifdef HAVE_MITSHM
#include <sys/ipc.h>
#include <sys/shm.h>
#include <X11/extensions/XShm.h>
#if defined(HAVE_X11_EXTENSIONS_XRENDER_H)
#  define HAVE_RENDER
#  include <X11/extensions/Xrender.h>
#  if defined(HAVE_X11_XFT_XFT_H)
#    include <X11/Xft/Xft.h>
#  endif
#endif
#include <errno.h>
#endif

#ifdef HAVE_XM_MWMUTIL_H
#include <Xm/MwmUtil.h>
#endif

#define SCI_XLIB_PIXMAP_HANDLE_NORMAL 0
#define SCI_XLIB_PIXMAP_HANDLE_GRABBED 1

#define SCI_XLIB_SWAP_CTRL_CAPS (1 << 0)
#define SCI_XLIB_INSERT_MODE    (1 << 1)
#define SCI_XLIB_NLS		(1 << 2) /* Non-US keyboard support, sacrificing shortcut keys */

#define X_COLOR_EXT(c) ((c << 8) | c)

/* In C++ mode, we re-name ``class'' to ``class_''.  However, this screws up
** our interface to xlib, so we're not doing it in here.
*/
#ifdef class
#  undef class
#endif

static int flags;

struct _xlib_state {
	Display *display;
	Window window;
	GC gc;
	XGCValues gc_values;
	Colormap colormap;
	Pixmap visual[3];
	gfx_pixmap_t *priority[2];
#ifdef HAVE_MITSHM
	XShmSegmentInfo *shm[4];
#endif
	int use_render;
	int buckystate;
	XErrorHandler old_error_handler;
	Cursor mouse_cursor;
	byte *pointer_data[2];
	int used_bytespp; /* bytes actually used to display stuff, rather than bytes occupied in data space */
	XVisualInfo visinfo;
#ifdef HAVE_RENDER
	Picture picture;
#endif
};

#define VISUAL S->visinfo.visual

#define S ((struct _xlib_state *)(drv->state))

#define XASS(foo) { int val = foo; if (!val) xlderror(drv, __LINE__); }
#define XFACT drv->mode->xfact
#define YFACT drv->mode->yfact

#define DEBUGB if (drv->debug_flags & GFX_DEBUG_BASIC && ((debugline = __LINE__))) xldprintf
#define DEBUGU if (drv->debug_flags & GFX_DEBUG_UPDATES && ((debugline = __LINE__))) xldprintf
#define DEBUGPXM if (drv->debug_flags & GFX_DEBUG_PIXMAPS && ((debugline = __LINE__))) xldprintf
#define DEBUGPTR if (drv->debug_flags & GFX_DEBUG_POINTER && ((debugline = __LINE__))) xldprintf
#define ERROR if ((debugline = __LINE__)) xldprintf

#ifdef HAVE_MITSHM

XShmSegmentInfo shminfo;
int have_shmem = 0;
int x11_error = 0;

static int check_for_xshm(Display *display)
{
	int major, minor, ignore;
	Bool pixmaps;
	if (XQueryExtension(display, "MIT-SHM", &ignore, &ignore, &ignore)) {
		if (XShmQueryVersion( display, &major, &minor, &pixmaps) == True) {
			return (pixmaps == True) ? 2 : 1 ;
		} else {
			return 0;
		}
	}
	return 0;
}
#endif

#ifdef HAVE_RENDER
static int x_have_render(Display *display)
{
	int ignore, retval;

	printf("Checking for X11 RENDER extension:");

	retval = XQueryExtension(display, "RENDER", &ignore, &ignore, &ignore);

	if (retval)
		printf(" found.\n");
	else
		printf(" not found.\n");

	return retval;
}
#endif

static int debugline = 0;

static void
xldprintf(const char *fmt, ...)
{
	va_list argp;
	fprintf(stderr,"GFX-XLIB %d:", debugline);
	va_start(argp, fmt);
	vfprintf(stderr, fmt, argp);
	va_end(argp);
}

static void
xlderror(gfx_driver_t *drv, int line)
{
	xldprintf("Xlib Error in line %d\n", line);
}

static unsigned long
xlib_map_color(gfx_driver_t *drv, gfx_color_t color)
{
	gfx_mode_t *mode = drv->mode;
	unsigned long temp;
	unsigned long retval = 0;

	if (drv->mode->palette)
		return color.visual.global_index;

	temp = color.visual.r;
	temp |= temp << 8;
	temp |= temp << 16;
	retval |= (temp >> mode->red_shift) & (mode->red_mask);
	temp = color.visual.g;
	temp |= temp << 8;
	temp |= temp << 16;
	retval |= (temp >> mode->green_shift) & (mode->green_mask);
	temp = color.visual.b;
	temp |= temp << 8;
	temp |= temp << 16;
	retval |= (temp >> mode->blue_shift) & (mode->blue_mask);

	return retval;
}


static int
xlib_error_handler(Display *display, XErrorEvent *error)
{
	char errormsg[256];
#ifdef HAVE_MITSHM
	x11_error = 1;
#endif
	XGetErrorText(display, error->error_code, errormsg, 255);
	ERROR(" X11: %s\n", errormsg);
	return 0;
}

#define UPDATE_NLS_CAPABILITY 							\
		if (flags & SCI_XLIB_NLS)					\
			drv->capabilities |= GFX_CAPABILITY_KEYTRANSLATE;	\
		else								\
			drv->capabilities &= ~GFX_CAPABILITY_KEYTRANSLATE



static int
xlib_set_parameter(struct _gfx_driver *drv, char *attribute, char *value)
{
	if (!strncmp(attribute, "swap_ctrl_caps",17) ||
	    !strncmp(attribute, "swap_caps_ctrl",17)) {
		if (string_truep(value))
			flags |= SCI_XLIB_SWAP_CTRL_CAPS;
		else
			flags &= ~SCI_XLIB_SWAP_CTRL_CAPS;


		return GFX_OK;
	}

	if (!strncmp(attribute, "localised_keyboard", 18)
	    || !strncmp(attribute, "localized_keyboard", 18)) {
		if (string_truep(value))
			flags |= SCI_XLIB_NLS;
		else
			flags &= ~SCI_XLIB_NLS;

		UPDATE_NLS_CAPABILITY;

		return GFX_OK;

	}

	if (!strncmp(attribute, "disable_shmem", 14)) {
#ifdef HAVE_MITSHM
	  if (string_truep(value))
	    have_shmem = -1;
#endif
	  return GFX_OK;
	}

	ERROR("Attempt to set xlib parameter \"%s\" to \"%s\"\n", attribute, value);
	return GFX_ERROR;
}

Cursor
x_empty_cursor(Display *display, Drawable drawable) /* Generates an empty X cursor */
{
	byte cursor_data[] = {0};
	XColor black = {0,0,0};
	Pixmap cursor_map;

	Cursor retval;

	cursor_map = XCreateBitmapFromData(display, drawable, (char *) cursor_data, 1, 1);

	retval = XCreatePixmapCursor(display, cursor_map, cursor_map, &black, &black, 0, 0);

	XFreePixmap(display, cursor_map);

	return retval;
}

static int
xlib_draw_filled_rect(struct _gfx_driver *drv, rect_t rect,
                      gfx_color_t color1, gfx_color_t color2,
                      gfx_rectangle_fill_t shade_mode);
static int
xlib_draw_pixmap(struct _gfx_driver *drv, gfx_pixmap_t *pxm, int priority,
		 rect_t src, rect_t dest, gfx_buffer_t buffer);


static int
xlib_init_specific(struct _gfx_driver *drv, int xfact, int yfact, int bytespp)
{
	XVisualInfo xvisinfo;
	XSetWindowAttributes win_attr;
	int default_screen;
	int vistype = (bytespp == 1)? 3 /* PseudoColor */ : 4 /* TrueColor */;
	int red_shift, green_shift, blue_shift, alpha_shift;
	int bytespp_physical;
	int depth_mod; /* Number of bits to subtract from the depth while checking */
	unsigned int alpha_mask;
	int xsize, ysize;
	XSizeHints *size_hints;
	XClassHint *class_hint;
	XImage *foo_image = NULL;
	int reverse_endian = 0;
#ifdef HAVE_XM_MWMUTIL_H
	PropMotifWmHints motif_hints;
	Atom prop, proptype;
#endif

	int i;


	UPDATE_NLS_CAPABILITY;

	if (!drv->state /* = S */)
		drv->state = sci_malloc(sizeof(struct _xlib_state));

	flags |= SCI_XLIB_INSERT_MODE;

	S->display = XOpenDisplay(NULL);

	if (!S->display) {
		ERROR("Could not open X connection!\n");
		return GFX_FATAL;
	}

	default_screen = DefaultScreen(S->display);

	if (xfact == -1 && yfact == -1) { /* Detect (used INTERNALLY!) */
		xfact = 2;
		if (DisplayWidth(S->display, default_screen) < 640
		    || DisplayHeight(S->display, default_screen) < 400)
			xfact = 1;

		yfact = xfact;
	}

	xsize = xfact * 320;
	ysize = yfact * 200;
	if (xfact < 1 || yfact < 1 || bytespp < 1 || bytespp > 4) {
		ERROR("Internal error: Attempt to open window w/ scale factors (%d,%d) and bpp=%d!\n",
		      xfact, yfact, bytespp);
		BREAKPOINT();
	}

#ifdef HAVE_MITSHM
	if (!have_shmem) {
	  have_shmem = check_for_xshm(S->display);
	  if (have_shmem) {
	    printf("Using the MIT-SHM extension (%d/%d)\n", have_shmem,
		   XShmPixmapFormat(S->display));
	  }
	  memset(&shminfo, 0, sizeof(XShmSegmentInfo));
	}
	for (i = 0; i < 4; i++)
	  S->shm[i] = NULL;
#endif

	depth_mod = 0;

	/* Try all bit size twiddling */
	for (depth_mod = 0; depth_mod < 8; depth_mod++)
		/* Try all viable depths */
		for (; bytespp > 0; bytespp--)
			/* Try all interesting visuals */
			for (vistype = 4; vistype >= 3; vistype--)
				if (XMatchVisualInfo(S->display, default_screen,
						     (bytespp << 3) - depth_mod, vistype, &xvisinfo))
					goto found_visual;
 found_visual:

	S->visinfo = xvisinfo;

	if (vistype < 3 || ((vistype == 3) && (bytespp != 1))) {
		if (bytespp == 1) {
			ERROR("Could not get an 8 bit Pseudocolor visual!\n");
		} else {
			ERROR("Could not get a %d bit TrueColor visual!\n", bytespp << 3);
		}
		return GFX_FATAL;
	}

	S->colormap = win_attr.colormap =
		XCreateColormap(S->display, RootWindow(S->display, default_screen),
				VISUAL, (bytespp == 1)? AllocAll : AllocNone);

	win_attr.event_mask = PointerMotionMask | StructureNotifyMask | ButtonPressMask
		| ButtonReleaseMask | KeyPressMask | KeyReleaseMask | ExposureMask;
	win_attr.background_pixel = win_attr.border_pixel = 0;

	S->window = XCreateWindow(S->display, RootWindow(S->display, default_screen),
				  0, 0, xsize, ysize, 0, xvisinfo.depth, InputOutput,
				  VISUAL, (CWBackPixel | CWBorderPixel | CWColormap | CWEventMask),
				  &win_attr);

	if (!S->window) {
		ERROR("Could not create window of size %dx%d!\n", 320*xfact, 200*yfact);
		return GFX_FATAL;
	}

	XSync(S->display, False);
#ifdef HAVE_XM_MWMUTIL_H
	motif_hints.flags = MWM_HINTS_DECORATIONS|MWM_HINTS_FUNCTIONS;
	motif_hints.decorations = MWM_DECOR_BORDER|MWM_DECOR_TITLE|MWM_DECOR_MENU|MWM_DECOR_MINIMIZE;
	motif_hints.functions=0
#ifdef MWM_FUNC_MOVE
		| MWM_FUNC_MOVE
#endif
#ifdef MWM_FUNC_MINIMIZE
		| MWM_FUNC_MINIMIZE
#endif
#ifdef MWM_FUNC_CLOSE
		| MWM_FUNC_CLOSE
#endif
#ifdef MWM_FUNC_QUIT_APP
		| MWM_FUNC_QUIT_APP
#endif
		;

	prop = XInternAtom(S->display, "_MOTIF_WM_HINTS", True );
	if (prop) {
		proptype = prop;
		XChangeProperty(S->display, S->window, prop, proptype, 32, PropModeReplace, (unsigned char *) &motif_hints, PROP_MOTIF_WM_HINTS_ELEMENTS);
	}
#endif

	XStoreName(S->display, S->window, "FreeSCI");
	XDefineCursor(S->display, S->window, (S->mouse_cursor = x_empty_cursor(S->display, S->window)));

	XMapWindow(S->display, S->window);
	S->buckystate = 0;

	if (bytespp == 1)
		red_shift = green_shift = blue_shift = 0;
	else {
		red_shift = 32 - ffs((~xvisinfo.red_mask >> 1) & (xvisinfo.red_mask));
		green_shift = 32 - ffs((~xvisinfo.green_mask >> 1) & (xvisinfo.green_mask));
		blue_shift = 32 - ffs((~xvisinfo.blue_mask >> 1) & (xvisinfo.blue_mask));
	}

	class_hint = XAllocClassHint();
	class_hint->res_name = "FreeSCI";
	class_hint->res_class = "FreeSCI";
	XSetIconName(S->display, S->window, "FreeSCI");
	XSetClassHint(S->display, S->window, class_hint);
	XFree(class_hint);
	size_hints = XAllocSizeHints();
	size_hints->base_width = size_hints->min_width = size_hints->max_width = xsize;
	size_hints->base_height = size_hints->min_height = size_hints->max_height = ysize;
	size_hints->flags |= PMinSize | PMaxSize | PBaseSize;
	XSetWMNormalHints(S->display, S->window, size_hints);
	XFree(size_hints);

	S->gc_values.foreground = BlackPixel(S->display, DefaultScreen(S->display));
	S->gc = XCreateGC(S->display, S->window, GCForeground, &(S->gc_values));

	for (i = 0; i < 2; i++) {
		S->priority[i] = gfx_pixmap_alloc_index_data(gfx_new_pixmap(xsize, ysize, GFX_RESID_NONE, -i, -777));
		if (!S->priority[i]) {
			ERROR("Out of memory: Could not allocate priority maps! (%dx%d)\n",
			      xsize, ysize);
			return GFX_FATAL;
		}
	}

	foo_image = XCreateImage(S->display, VISUAL,
				 bytespp << 3, ZPixmap, 0, (char*)sci_malloc(23), 2, 2, 8, 0);

	bytespp_physical = foo_image->bits_per_pixel >> 3;
#ifdef WORDS_BIGENDIAN
	reverse_endian = foo_image->byte_order == LSBFirst;
#else
	reverse_endian = foo_image->byte_order == MSBFirst;
#endif
	XDestroyImage(foo_image);

#ifdef HAVE_MITSHM
	/* set up and test the XSHM extension to make sure it's sane */
	if (have_shmem) {
	  XErrorHandler old_handler;

	  x11_error = 0;
	  old_handler = XSetErrorHandler(xlib_error_handler);

	  foo_image = XShmCreateImage(S->display, VISUAL,
				      bytespp_physical << 3, ZPixmap, 0, &shminfo, 2, 2);
	  if (foo_image)
	    shminfo.shmid = shmget(IPC_PRIVATE,
				   foo_image->bytes_per_line *
				   foo_image->height,
				   IPC_CREAT | 0777);
	  if (-1 == shminfo.shmid) {
	    have_shmem = 0;
	    ERROR("System does not support SysV IPC, disabling XSHM\n");
	    perror("reason");
	    foo_image = NULL;
	  } else {

		  shminfo.shmaddr = (char *) shmat(shminfo.shmid, 0, 0);
		  if ((void *) -1 == shminfo.shmaddr) {
			  ERROR("Could not attach shared memory segment\n");
			  perror("reason");
			  if (foo_image)
				  XDestroyImage(foo_image);
			  return GFX_FATAL;
		  }

		  foo_image->data = shminfo.shmaddr;
		  shminfo.readOnly = False;

		  XShmAttach(S->display, &shminfo);
		  XSync(S->display, False);
		  shmctl(shminfo.shmid, IPC_RMID, 0);

		  if (x11_error) {
			  have_shmem = 0;
			  ERROR("System does not support Shared XImages, disabling\n");
			  shmdt(shminfo.shmaddr);
			  XDestroyImage(foo_image);
			  foo_image = NULL;
			  x11_error = 0;
		  }
		  XSetErrorHandler(old_handler);
	  }
	}

#endif


	alpha_mask = xvisinfo.red_mask | xvisinfo.green_mask | xvisinfo.blue_mask;
	if (!reverse_endian && bytespp_physical == 4 && (!(alpha_mask & 0xff000000) || !(alpha_mask & 0xff))) {
		if (alpha_mask & 0xff) {
			alpha_mask = 0xff000000;
			alpha_shift = 0;
		} else { /* Lowest byte */
			alpha_mask = 0x000000ff;
			alpha_shift = 24;
		}
	} else {
		alpha_mask = 0;
		alpha_shift = 0;
	}
	/* create the visual buffers */
	for (i = 0; i < 3; i++) {
#ifdef HAVE_MITSHM
	  XErrorHandler old_handler;

	  if (have_shmem && have_shmem != 2) {
	    ERROR("Shared memory pixmaps not supported.  Reverting\n");
	    perror("reason");
	    have_shmem = 0;
	  }

	  if (have_shmem) {
	    old_handler = XSetErrorHandler(xlib_error_handler);

	    if ((S->shm[i] = (XShmSegmentInfo*)sci_malloc(sizeof(XShmSegmentInfo))) == 0) {
	      ERROR("AIEEEE!  Malloc failed!\n");
	      return GFX_FATAL;
	    }
	    memset(S->shm[i], 0, sizeof(XShmSegmentInfo));

	    S->shm[i]->shmid = shmget(IPC_PRIVATE, xsize * ysize *
				      bytespp_physical,
				      IPC_CREAT | IPC_EXCL | 0666);
	    S->shm[i]->readOnly = False;

	    if (S->shm[i]->shmid == -1) {
	      have_shmem = 0;
	      ERROR("System does not support SysV IPC, disabling XSHM\n");
	      perror("reason");
	    }
	  }
	  if (have_shmem) {
	    S->shm[i]->shmaddr = (char *) shmat(S->shm[i]->shmid, 0, 0);
	    if (S->shm[i]->shmaddr == (void *) -1) {
	      ERROR("Could not attach shared memory segment\n");
	      perror("reason");
	      have_shmem = 0;
	    }
	  }
	  if (have_shmem) {
	    if (!XShmAttach(S->display, S->shm[i]) || x11_error) {
	      ERROR("ARGH!  Can't attach shared memory segment\n");
	      have_shmem = 0;
	    }
	    XSync(S->display, False);
	    shmctl(S->shm[i]->shmid, IPC_RMID, 0);
	  }

	  if (have_shmem && !x11_error) {
	    S->visual[i] = XShmCreatePixmap(S->display, S->window,
					    S->shm[i]->shmaddr,
					    S->shm[i], xsize, ysize,
					    bytespp << 3);
	    XSync(S->display, False);

	    if (x11_error || !S->visual[i]) {
	      ERROR("Shared Memory Pixmaps not supported on this system. Disabling!\n");
		have_shmem = 0;
		XFreePixmap(S->display, S->visual[i]);
		XShmDetach(S->display ,S->shm[i]);
		XSync(S->display, False);
		S->visual[i] = 0;
		x11_error = 0;
		shmdt(S->shm[i]->shmaddr);
		sci_free(S->shm[i]);

	    }

	  }
	  XSetErrorHandler(old_handler);

	  if (!have_shmem)
#endif
	    S->visual[i] = XCreatePixmap(S->display, S->window, xsize, ysize, bytespp << 3);

	  XFillRectangle(S->display, S->visual[i], S->gc, 0, 0, xsize, ysize);
	}

	/** X RENDER handling **/
#ifdef HAVE_RENDER
	S->use_render = x_have_render(S->display);

	if (S->use_render) {
		XRenderPictFormat * format
			= XRenderFindVisualFormat (S->display,
						   VISUAL);
		S->picture =  XRenderCreatePicture (S->display,
						    (Drawable) S->visual[1],
						    format,
						    0, 0);
	} else /* No Xrender */
		drv->draw_filled_rect = xlib_draw_filled_rect;
#else
	S->use_render = 0;
#endif
	/** End of X RENDER handling **/


	drv->mode = gfx_new_mode(xfact, yfact, bytespp_physical,
				 xvisinfo.red_mask, xvisinfo.green_mask,
				 xvisinfo.blue_mask, alpha_mask,
				 red_shift, green_shift, blue_shift, alpha_shift,
				 (bytespp == 1)? xvisinfo.colormap_size : 0,
				 (reverse_endian)? GFX_MODE_FLAG_REVERSE_ENDIAN : 0);

#ifdef HAVE_MITSHM
	if (have_shmem) {
	  XShmDetach(S->display, &shminfo);
	  XDestroyImage(foo_image);
	  shmdt(shminfo.shmaddr);
	}
#endif

	S->used_bytespp = bytespp;
	S->old_error_handler = (XErrorHandler) XSetErrorHandler(xlib_error_handler);
	S->pointer_data[0] = NULL;
	S->pointer_data[1] = NULL;


	return GFX_OK;
}


static void
xlib_xdpy_info()
{
	int i;
	XVisualInfo foo;
	XVisualInfo *visuals;
	int visuals_nr;
	Display *display = XOpenDisplay(NULL);
	const char *vis_classes[6] = {"StaticGray", "GrayScale", "StaticColor",
			    "PseudoColor", "TrueColor", "DirectColor"};

	printf("Visuals provided by X11 server:\n");
	visuals = XGetVisualInfo(display, VisualNoMask, &foo, &visuals_nr);

	if (!visuals_nr) {
		printf("  None!\n");
	}

	for (i = 0; i < visuals_nr; i++) {
		XVisualInfo *visual = visuals + i;

		/* This works around an incompatibility between C++ and xlib: access visual->class */
		int visual_class = *((int *) (((byte *)(&(visual->depth))) + sizeof(unsigned int)));

		printf("%d:\t%d bpp %s(%d)\n"
		       "\tR:%08lx G:%08lx B:%08lx\n"
		       "\tbits_per_rgb=%d\n"
		       "\tcolormap_size=%d\n\n",
		       i,
		       visual->depth,
		       (visual_class < 0 || visual_class >5)?
		       "INVALID" :
		       vis_classes[visual_class],
		       visual_class,
		       visual->red_mask, visual->green_mask, visual->blue_mask,
		       visual->bits_per_rgb, visual->colormap_size);

	}

	if (visuals)
		XFree(visuals);
}

static int
xlib_init(struct _gfx_driver *drv)
{
	int i;

	/* Try 32-bit mode last due to compiz issue with bit depth 32. */
	for (i = 3; i > 0; i--)
		if (!xlib_init_specific(drv, -1, -1, i))
			return GFX_OK;

	if (!xlib_init_specific(drv, -1, -1, 4))
		return GFX_OK;

	fprintf(stderr, "Could not find supported mode!\n");
	xlib_xdpy_info();

	return GFX_FATAL;
}

static void
xlib_exit(struct _gfx_driver *drv)
{
	int i;
	if (S) {
		for (i = 0; i < 2; i++) {
			gfx_free_pixmap(drv, S->priority[i]);
			S->priority[i] = NULL;
		}

		for (i = 0; i < 3; i++) {
#ifdef HAVE_MITSHM
		  if (have_shmem && S->shm[i]) {
		    XFreePixmap(S->display, S->visual[i]);
		    XShmDetach(S->display, S->shm[i]);

		    if (S->shm[i]->shmid >=0)
		      shmctl(S->shm[i]->shmid, IPC_RMID, 0);
		    if (S->shm[i]->shmaddr)
		      shmdt(S->shm[i]->shmaddr);

		    sci_free(S->shm[i]);
		    S->shm[i] = NULL;
		  } else
#endif
		    XFreePixmap(S->display, S->visual[i]);
		}

#ifdef HAVE_RENDER
		XRenderFreePicture(S->display, S->picture);
#endif
		XFreeGC(S->display, S->gc);
		XDestroyWindow(S->display, S->window);
		XCloseDisplay(S->display);
		XSetErrorHandler((XErrorHandler) (S->old_error_handler));
		sci_free(S);
		drv->state /* = S */ = NULL;
		gfx_free_mode(drv->mode);
	}
}


  /*** Drawing operations ***/

static int
xlib_draw_line(struct _gfx_driver *drv, point_t start, point_t end, gfx_color_t color,
	       gfx_line_mode_t line_mode, gfx_line_style_t line_style)
{
	int linewidth = (line_mode == GFX_LINE_MODE_FINE)? 1:
		(drv->mode->xfact + drv->mode->yfact) >> 1;

	if (color.mask & GFX_MASK_VISUAL) {
		int xmod = drv->mode->xfact >> 1;
		int ymod = drv->mode->yfact >> 1;

		if (line_mode == GFX_LINE_MODE_FINE)
			xmod = ymod = 0;

		S->gc_values.foreground = xlib_map_color(drv, color);
		S->gc_values.line_width = linewidth;
		S->gc_values.line_style = (line_style == GFX_LINE_STYLE_NORMAL)?
			LineSolid : LineOnOffDash;
		S->gc_values.cap_style = CapProjecting;

		XChangeGC(S->display, S->gc, GCLineWidth | GCLineStyle | GCForeground | GCCapStyle, &(S->gc_values));

		XASS(XDrawLine(S->display, S->visual[1], S->gc,
			       start.x + xmod, start.y + ymod,
			       end.x + xmod, end.y + ymod));
	}

	if (color.mask & GFX_MASK_PRIORITY) {
		int xc, yc;
		point_t nstart, nend;

		linewidth--;
		for (xc = 0; xc <= linewidth; xc++)
			for (yc = -linewidth; yc <= linewidth; yc++) {
				nstart.x = start.x + xc;
				nstart.y = start.y + yc;

				nend.x = end.x + xc;
				nend.y = end.y + yc;

				gfx_draw_line_pixmap_i(S->priority[0],
						       nstart, nend, color.priority);
			}
	}

	return GFX_OK;
}

static int
xlib_draw_filled_rect(struct _gfx_driver *drv, rect_t rect,
		      gfx_color_t color1, gfx_color_t color2,
		      gfx_rectangle_fill_t shade_mode)
{
	if (color1.mask & GFX_MASK_VISUAL) {
		S->gc_values.foreground = xlib_map_color(drv, color1);
		XChangeGC(S->display, S->gc, GCForeground, &(S->gc_values));
		XASS(XFillRectangle(S->display, S->visual[1], S->gc, rect.x, rect.y,
				    rect.xl, rect.yl));
	}

	if (color1.mask & GFX_MASK_PRIORITY)
		gfx_draw_box_pixmap_i(S->priority[0], rect, color1.priority);

	return GFX_OK;
}

#ifdef HAVE_RENDER
static int
xlib_draw_filled_rect_RENDER(struct _gfx_driver *drv, rect_t rect,
			     gfx_color_t color1, gfx_color_t color2,
			     gfx_rectangle_fill_t shade_mode)
{
	if (S->used_bytespp == 1) /* No room for alpha! */
		return xlib_draw_filled_rect(drv, rect, color1, color2, shade_mode);

	if (color1.mask & GFX_MASK_VISUAL) {
		XRenderColor fg;

		fg.red = X_COLOR_EXT(color1.visual.r);
		fg.green = X_COLOR_EXT(color1.visual.g);
		fg.blue = X_COLOR_EXT(color1.visual.b);
		fg.alpha = 0xffff - X_COLOR_EXT(color1.alpha);


		XRenderFillRectangle(S->display,
				     PictOpOver,
				     S->picture,
				     &fg,
				     rect.x, rect.y,
				     rect.xl, rect.yl);
	}

	if (color1.mask & GFX_MASK_PRIORITY)
		gfx_draw_box_pixmap_i(S->priority[0], rect, color1.priority);

	return GFX_OK;
}
#endif

  /*** Pixmap operations ***/

static int
xlib_register_pixmap(struct _gfx_driver *drv, gfx_pixmap_t *pxm)
{
	if (pxm->internal.info) {
		ERROR("Attempt to register pixmap twice!\n");
		return GFX_ERROR;
	}
	pxm->internal.info = XCreateImage(S->display, VISUAL,
					    S->used_bytespp << 3, ZPixmap, 0, (char *) pxm->data, pxm->xl,
					    pxm->yl, 8, 0);

	DEBUGPXM("Registered pixmap %d/%d/%d at %p (%dx%d)\n", pxm->ID, pxm->loop, pxm->cel,
		 pxm->internal.info, pxm->xl, pxm->yl);
	return GFX_OK;
}

static int
xlib_unregister_pixmap(struct _gfx_driver *drv, gfx_pixmap_t *pxm)
{
	DEBUGPXM("Freeing pixmap %d/%d/%d at %p\n", pxm->ID, pxm->loop, pxm->cel,
		 pxm->internal.info);

	if (!pxm->internal.info) {
		ERROR("Attempt to unregister pixmap twice!\n");
		return GFX_ERROR;
	}

	XDestroyImage((XImage *) pxm->internal.info);
	pxm->internal.info = NULL;
	pxm->data = NULL; /* Freed by XDestroyImage */
	return GFX_OK;
}

static int
xlib_draw_pixmap(struct _gfx_driver *drv, gfx_pixmap_t *pxm, int priority,
		 rect_t src, rect_t dest, gfx_buffer_t buffer)
{
	int bufnr = (buffer == GFX_BUFFER_STATIC)? 2:1;
	int pribufnr = bufnr -1;
	XImage *tempimg;

	if (dest.xl != src.xl || dest.yl != src.yl) {
		ERROR("Attempt to scale pixmap (%dx%d)->(%dx%d): Not supported\n",
		      src.xl, src.yl, dest.xl, dest.yl);
		return GFX_ERROR;
	}

	if (pxm->internal.handle == SCI_XLIB_PIXMAP_HANDLE_GRABBED) {
		XPutImage(S->display, S->visual[bufnr], S->gc, (XImage *) pxm->internal.info,
			  src.x, src.y, dest.x, dest.y, dest.xl, dest.yl);
		return GFX_OK;
	}

	tempimg = XGetImage(S->display, S->visual[bufnr], dest.x, dest.y,
			    dest.xl, dest.yl, 0xffffffff, ZPixmap);

	if (!tempimg) {
		ERROR("Failed to grab X image!\n");
		return GFX_ERROR;
	}

	gfx_crossblit_pixmap(drv->mode, pxm, priority, src, dest,
			     (byte *) tempimg->data, tempimg->bytes_per_line,
			     S->priority[pribufnr]->index_data,
			     S->priority[pribufnr]->index_xl, 1,
			     GFX_CROSSBLIT_FLAG_DATA_IS_HOMED);

	XPutImage(S->display, S->visual[bufnr], S->gc, tempimg,
		  0, 0, dest.x, dest.y, dest.xl, dest.yl);

	XDestroyImage(tempimg);
	return GFX_OK;
}


static int
xlib_grab_pixmap(struct _gfx_driver *drv, rect_t src, gfx_pixmap_t *pxm,
		 gfx_map_mask_t map)
{

	if (src.x < 0 || src.y < 0) {
		ERROR("Attempt to grab pixmap from invalid coordinates (%d,%d)\n", src.x, src.y);
		return GFX_ERROR;
	}

	if (!pxm->data) {
		ERROR("Attempt to grab pixmap to unallocated memory\n");
		return GFX_ERROR;
	}

	switch (map) {

	case GFX_MASK_VISUAL:
		pxm->xl = src.xl;
		pxm->yl = src.yl;

		pxm->internal.info = XGetImage(S->display, S->visual[1], src.x, src.y,
					       src.xl, src.yl, 0xffffffff, ZPixmap);
		pxm->internal.handle = SCI_XLIB_PIXMAP_HANDLE_GRABBED;
		pxm->flags |= GFX_PIXMAP_FLAG_INSTALLED | GFX_PIXMAP_FLAG_EXTERNAL_PALETTE | GFX_PIXMAP_FLAG_PALETTE_SET;
		sci_free(pxm->data);
		pxm->data = (byte *) ((XImage *)(pxm->internal.info))->data;
		break;

	case GFX_MASK_PRIORITY:
		ERROR("FIXME: priority map grab not implemented yet!\n");
		break;

	default:
		ERROR("Attempt to grab pixmap from invalid map 0x%02x\n", map);
		return GFX_ERROR;
	}

	return GFX_OK;
}


  /*** Buffer operations ***/

static int
xlib_update(struct _gfx_driver *drv, rect_t src, point_t dest, gfx_buffer_t buffer)
{
	int data_source = (buffer == GFX_BUFFER_BACK)? 2 : 1;
	int data_dest = data_source - 1;


	if (src.x != dest.x || src.y != dest.y) {
		DEBUGU("Updating %d (%d,%d)(%dx%d) to (%d,%d)\n", buffer, src.x, src.y,
		       src.xl, src.yl, dest.x, dest.y);
	} else {
		DEBUGU("Updating %d (%d,%d)(%dx%d)\n", buffer, src.x, src.y, src.xl, src.yl);
	}

	XCopyArea(S->display, S->visual[data_source], S->visual[data_dest], S->gc,
		  src.x, src.y, src.xl, src.yl, dest.x, dest.y);

	if (buffer == GFX_BUFFER_BACK && (src.x == dest.x) && (src.y == dest.y))
		gfx_copy_pixmap_box_i(S->priority[0], S->priority[1], src);
	else {
		gfx_color_t col;
		col.mask = GFX_MASK_VISUAL;
		col.visual.r = 0xff;
		col.visual.g = 0;
		col.visual.b = 0;

		XCopyArea(S->display, S->visual[0], S->window, S->gc,
			  dest.x, dest.y, src.xl, src.yl, dest.x, dest.y);
	}

	return GFX_OK;
}

static int
xlib_set_static_buffer(struct _gfx_driver *drv, gfx_pixmap_t *pic, gfx_pixmap_t *priority)
{

	if (!pic->internal.info) {
		ERROR("Attempt to set static buffer with unregisterd pixmap!\n");
		return GFX_ERROR;
	}
	XPutImage(S->display, S->visual[2], S->gc, (XImage *) pic->internal.info,
		  0, 0, 0, 0, 320 * XFACT, 200 * YFACT);
	gfx_copy_pixmap_box_i(S->priority[1], priority, gfx_rect(0, 0, 320*XFACT, 200*YFACT));

	return GFX_OK;
}


  /*** Mouse pointer operations ***/

byte *
xlib_create_cursor_data(gfx_driver_t *drv, gfx_pixmap_t *pointer, int mode)
{
	int linewidth = (pointer->xl + 7) >> 3;
	int lines = pointer->yl;
	int xc, yc;
	int xfact = drv->mode->xfact;
	byte *data = (byte*)sci_calloc(linewidth, lines);
	byte *linebase = data, *pos;
	byte *src = pointer->index_data;


	for (yc = 0; yc < pointer->index_yl; yc++) {
		int scalectr;
		int bitc = 0;
		pos = linebase;


		for (xc = 0; xc < pointer->index_xl; xc++) {
			int draw = mode?
				(*src == 0) : (*src < 255);

			for (scalectr = 0; scalectr < xfact; scalectr++) {
				if (draw)
					*pos |= (1 << bitc);

				bitc++;
				if (bitc == 8) {
					bitc = 0;
					pos++;
				}
			}

			src++;
		}
		for (scalectr = 1; scalectr < drv->mode->yfact; scalectr++)
			memcpy(linebase + linewidth * scalectr, linebase, linewidth);

		linebase += linewidth * drv->mode->yfact;
	}

	return data;
}

static int
xlib_set_pointer(struct _gfx_driver *drv, gfx_pixmap_t *pointer)
{
	int i;
	XFreeCursor(S->display, S->mouse_cursor);

	for (i = 0; i < 2; i++)
		if (S->pointer_data[i]) {
			sci_free(S->pointer_data[i]);
			S->pointer_data[i] = NULL;
		}

	if (pointer == NULL)
		S->mouse_cursor = x_empty_cursor(S->display, S->window);
	else {
		XColor cols[2];
		Pixmap visual, mask;
		byte *mask_data, *visual_data;
		int real_xl = ((pointer->xl + 7) >> 3) << 3;
		int i;

		for (i = 0; i < 2; i++) {
			cols[i].red = pointer->colors[i].r;
			cols[i].red |= (cols[i].red << 8);
			cols[i].green = pointer->colors[i].g;
			cols[i].green |= (cols[i].green << 8);
			cols[i].blue = pointer->colors[i].b;
			cols[i].blue |= (cols[i].blue << 8);
		}

		S->pointer_data[0] = visual_data = xlib_create_cursor_data(drv, pointer, 1);
		S->pointer_data[1] = mask_data = xlib_create_cursor_data(drv, pointer, 0);
		S->pointer_data[0] = NULL;
		S->pointer_data[1] = NULL;
		visual = XCreateBitmapFromData(S->display, S->window, (char *) visual_data, real_xl, pointer->yl);
		mask = XCreateBitmapFromData(S->display, S->window, (char *) mask_data, real_xl, pointer->yl);


		S->mouse_cursor =
			XCreatePixmapCursor(S->display, visual, mask,
					    &(cols[0]), &(cols[1]),
					    pointer->xoffset, pointer->yoffset);

		XFreePixmap(S->display, visual);
		XFreePixmap(S->display, mask);
		sci_free(mask_data);
		sci_free(visual_data);
	}

	XDefineCursor(S->display, S->window, S->mouse_cursor);

	return 0;
}


  /*** Palette operations ***/

static int
xlib_set_palette(struct _gfx_driver *drv, int index, byte red, byte green, byte blue)
{
	char stringbuf[8];
	sprintf(stringbuf, "#%02x%02x%02x", red, green, blue);  /* Argh. */

	XStoreNamedColor(S->display, S->colormap, stringbuf, index, DoRed | DoGreen | DoBlue);
	/* Isn't there some way to do this without strings? */

	return GFX_OK;
}


  /*** Event management ***/
/*
int
x_unmap_key(gfx_driver_t *drv, int keycode)
{
	KeySym xkey = XKeycodeToKeysym(S->display, keycode, 0);

	return 0;
}
*/
int
x_map_key(gfx_driver_t *drv, XEvent *key_event, char *character)
{
        KeySym xkey = XKeycodeToKeysym(S->display, key_event->xkey.keycode, 0);

	*character = 0;

	if (flags & SCI_XLIB_SWAP_CTRL_CAPS) {
		switch (xkey) {
		case XK_Control_L: xkey = XK_Caps_Lock; break;
		case XK_Caps_Lock: xkey = XK_Control_L; break;
		}
	}

	switch(xkey) {

	case XK_BackSpace: return SCI_K_BACKSPACE;
	case XK_Tab: return 9;
	case XK_Escape: return SCI_K_ESC;
	case XK_Return:
	case XK_KP_Enter: return SCI_K_ENTER;

	case XK_KP_Decimal:
	case XK_KP_Delete: return SCI_K_DELETE;
	case XK_KP_0:
	case XK_KP_Insert: return SCI_K_INSERT;
	case XK_End:
	case XK_KP_End:
	case XK_KP_1: return SCI_K_END;
	case XK_Down:
	case XK_KP_Down:
	case XK_KP_2: return SCI_K_DOWN;
	case XK_Page_Down:
	case XK_KP_Page_Down:
	case XK_KP_3: return SCI_K_PGDOWN;
	case XK_Left:
	case XK_KP_Left:
	case XK_KP_4: return SCI_K_LEFT;
	case XK_KP_Begin:
	case XK_KP_5: return SCI_K_CENTER;
	case XK_Right:
	case XK_KP_Right:
	case XK_KP_6: return SCI_K_RIGHT;
	case XK_Home:
	case XK_KP_Home:
	case XK_KP_7: return SCI_K_HOME;
	case XK_Up:
	case XK_KP_Up:
	case XK_KP_8: return SCI_K_UP;
	case XK_Page_Up:
	case XK_KP_Page_Up:
	case XK_KP_9: return SCI_K_PGUP;

	case XK_F1: return SCI_K_F1;
	case XK_F2: return SCI_K_F2;
	case XK_F3: return SCI_K_F3;
	case XK_F4: return SCI_K_F4;
	case XK_F5: return SCI_K_F5;
	case XK_F6: return SCI_K_F6;
	case XK_F7: return SCI_K_F7;
	case XK_F8: return SCI_K_F8;
	case XK_F9: return SCI_K_F9;
	case XK_F10: return SCI_K_F10;


	case XK_Control_L:
	case XK_Control_R:/* S->buckystate |= SCI_EVM_CTRL; return 0; */
	case XK_Alt_L:
	case XK_Alt_R:/* S->buckystate |= SCI_EVM_ALT; return 0; */
	case XK_Caps_Lock:
	case XK_Shift_Lock:/* S->buckystate ^= SCI_EVM_CAPSLOCK; return 0; */
	case XK_Scroll_Lock:/* S->buckystate ^= SCI_EVM_SCRLOCK; return 0; */
	case XK_Num_Lock:/* S->buckystate ^= SCI_EVM_NUMLOCK; return 0; */
	case XK_Shift_L:/* S->buckystate |= SCI_EVM_LSHIFT; return 0; */
	case XK_Shift_R:/* S->buckystate |= SCI_EVM_RSHIFT; return 0; */
		return 0;
	default:
		break;
	}


	if (flags & SCI_XLIB_NLS) {
		/* Localised key lookup */
		XLookupString(&(key_event->xkey), character, 1, &xkey, NULL);
	}

	if ((xkey >= ' ') && (xkey <= '~'))
		return xkey; /* All printable ASCII characters */

	switch (xkey) {
	case XK_KP_Add: return '+';
	case XK_KP_Divide: return '/';
	case XK_KP_Subtract: return '-';
	case XK_KP_Multiply: return '*';
	}

	if (*character)
		return xkey; /* Should suffice for all practical purposes */

	sciprintf("Unknown X keysym: %04x\n", xkey);
	return 0;
}


void
x_get_event(gfx_driver_t *drv, int eventmask, long wait_usec, sci_event_t *sci_event)
{
	int x_button_xlate[] = {0, 1, 3, 2, 4, 5};
	XEvent event;
	Window window = S->window;
	Display *display = S->display;
	struct timeval ctime, timeout_time, sleep_time;
	long usecs_to_sleep;

	eventmask |= ExposureMask; /* Always check for this */

	gettimeofday(&timeout_time, NULL);
	timeout_time.tv_usec += wait_usec;

	/* Calculate wait time */
	timeout_time.tv_sec += (timeout_time.tv_usec / 1000000);
	timeout_time.tv_usec %= 1000000;

	do {
		int hasnext_event = 1;

		while (hasnext_event) {
			if (sci_event) { /* Capable of handling any event? */
				hasnext_event = XPending(display);
				if (hasnext_event)
					XNextEvent(display, &event);
			} else 
				hasnext_event = XCheckWindowEvent(display, window, eventmask, &event);

			
			if (hasnext_event) 
				switch (event.type) {

				    case ReparentNotify:
				    case ConfigureNotify:
				    case MapNotify:
				    case UnmapNotify:
					    break;

				    case KeyPress: {
					    int modifiers = event.xkey.state;
					    char ch = 0;
					    sci_event->type = SCI_EVT_KEYBOARD;

					    S->buckystate = ((flags & SCI_XLIB_INSERT_MODE)? SCI_EVM_INSERT : 0)
						    | (((modifiers & LockMask)? SCI_EVM_LSHIFT | SCI_EVM_RSHIFT : 0)
						       | ((modifiers & ControlMask)? SCI_EVM_CTRL : 0)
						       | ((modifiers & (Mod1Mask | Mod4Mask))? SCI_EVM_ALT : 0)
						       | ((modifiers & Mod2Mask)? SCI_EVM_NUMLOCK : 0)
						       | ((modifiers & Mod5Mask)? SCI_EVM_SCRLOCK : 0))
						    ^ ((modifiers & ShiftMask)? SCI_EVM_LSHIFT | SCI_EVM_RSHIFT : 0);

					    sci_event->buckybits = S->buckystate;
					    sci_event->data =
						    x_map_key(drv, &event, &ch);

					    if (ch)
						    sci_event->character = ch;
					    else
						    sci_event->character = sci_event->data;

					    if (sci_event->data == SCI_K_INSERT)
						    flags ^= SCI_XLIB_INSERT_MODE;

					    if (sci_event->data)
						    return;

					    break;
				    }

				    case KeyRelease:
					    /*x_unmap_key(drv, event.xkey.keycode);*/
					    break;

				    case ButtonPress: {
					    sci_event->type = SCI_EVT_MOUSE_PRESS;
					    sci_event->buckybits = S->buckystate;
					    sci_event->data = x_button_xlate[event.xbutton.button];
					    return;
				    }

				    case ButtonRelease: {
					    sci_event->type = SCI_EVT_MOUSE_RELEASE;
					    sci_event->buckybits = S->buckystate;
					    sci_event->data = x_button_xlate[event.xbutton.button];
					    return;
				    }

				    case MotionNotify: {

					    drv->pointer_x = event.xmotion.x;
					    drv->pointer_y = event.xmotion.y;
					    if (!sci_event)
					    	    /* Wake up from sleep */
					    	    return;
				    }
					    break;

				    case GraphicsExpose:
				    case Expose: {
					    XCopyArea(S->display, S->visual[0], S->window, S->gc,
						      event.xexpose.x, event.xexpose.y, event.xexpose.width, event.xexpose.height,
						      event.xexpose.x, event.xexpose.y);
				    }
					    break;

				    case NoExpose:
					    break;

				    default:
					    ERROR("Received unhandled X event %04x\n", event.type);
				}
		}

		gettimeofday(&ctime, NULL);

		usecs_to_sleep = (timeout_time.tv_sec > ctime.tv_sec)? 1000000 : 0;
		usecs_to_sleep += timeout_time.tv_usec - ctime.tv_usec;
		if (ctime.tv_sec > timeout_time.tv_sec) usecs_to_sleep = -1;


		if (usecs_to_sleep > 0) {

			if (usecs_to_sleep > 10000)
				usecs_to_sleep = 10000; /* Sleep for a maximum of 10 ms */

			sleep_time.tv_usec = usecs_to_sleep;
			sleep_time.tv_sec = 0;

			select(0, NULL, NULL, NULL, &sleep_time); /* Sleep. */
		}

	} while (usecs_to_sleep >= 0);

	if (sci_event)
		sci_event->type = SCI_EVT_NONE; /* No event. */
}


static sci_event_t
xlib_get_event(struct _gfx_driver *drv)
{
	sci_event_t input;

	x_get_event(drv, PointerMotionMask | StructureNotifyMask | ButtonPressMask
		    | ButtonReleaseMask | KeyPressMask | KeyReleaseMask,
		    0, &input);

	return input;
}


static int
xlib_usec_sleep(struct _gfx_driver *drv, long usecs)
{
	x_get_event(drv, PointerMotionMask | StructureNotifyMask, usecs, NULL);
	return GFX_OK;
}

gfx_driver_t
gfx_driver_xlib = {
	"xlib",
	"0.6a",
	SCI_GFX_DRIVER_MAGIC,
	SCI_GFX_DRIVER_VERSION,
	NULL,
	0, 0,
	GFX_CAPABILITY_STIPPLED_LINES | GFX_CAPABILITY_MOUSE_SUPPORT
	| GFX_CAPABILITY_MOUSE_POINTER | GFX_CAPABILITY_PIXMAP_REGISTRY
	| GFX_CAPABILITY_FINE_LINES | GFX_CAPABILITY_WINDOWED
	| GFX_CAPABILITY_KEYTRANSLATE,
	0/*GFX_DEBUG_POINTER | GFX_DEBUG_UPDATES | GFX_DEBUG_PIXMAPS | GFX_DEBUG_BASIC*/,
	xlib_set_parameter,
	xlib_init_specific,
	xlib_init,
	xlib_exit,
	xlib_draw_line,
#ifdef HAVE_RENDER
	xlib_draw_filled_rect_RENDER,
#else
	xlib_draw_filled_rect,
#endif
	xlib_register_pixmap,
	xlib_unregister_pixmap,
	xlib_draw_pixmap,
	xlib_grab_pixmap,
	xlib_update,
	xlib_set_static_buffer,
	xlib_set_pointer,
	xlib_set_palette,
	xlib_get_event,
	xlib_usec_sleep,
	NULL
};

#endif /* X_DISPLAY_MISSING */
