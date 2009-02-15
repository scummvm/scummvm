/***************************************************************************
 ggi_driver.c Copyright (C) 2000 Christoph Reichenbach


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
/* FreeSCI 0.3.1+ graphics driver module for libggi */


#include <stdarg.h>
#include <gfx_system.h>
#include <gfx_driver.h>
#include <gfx_tools.h>
#include <assert.h>
#include <uinput.h>

#ifdef HAVE_LIBGGI

#include <ggi/ggi.h>
#include <ctype.h>


#define GGI_DRIVER_VERSION "0.4"

#define GFX_GGI_DEBUG

#ifdef GFX_GGI_DEBUG

#define POSMSG sciprintf("%s L%d:", __FILE__, __LINE__)
#define DEBUG_POINTER (!(drv->debug_flags & GFX_DEBUG_POINTER))? 0 : POSMSG && gfxprintf
#define DEBUG_UPDATES (!(drv->debug_flags & GFX_DEBUG_UPDATES))? 0 : POSMSG && gfxprintf
#define DEBUG_PIXMAPS (!(drv->debug_flags & GFX_DEBUG_PIXMAPS))? 0 : POSMSG && sciprintf
#define DEBUG_BASIC (!(drv->debug_flags & GFX_DEBUG_BASIC))? 0 : POSMSG && sciprintf
#else /* !GFX_GGI_DEBUG */
#define DEBUG_POINTER (1)? 0 :
#define DEBUG_UPDATES (1)? 0 :
#define DEBUG_PIXMAPS (1)? 0 :
#define DEBUG_BASIC (1)? 0 :
#endif /* !GFX_GGI_DEBUG */

static gfx_mode_t *
_aberr(char *file, int line, char *message);

static void
init_input_ggi();

#define MODE ((drv->mode)? drv->mode : _aberr(__FILE__, __LINE__, "drv->mode is NULL"))
#define STATE ((gfx_ggi_struct_t *)drv->state)
#define VISUAL ((gfx_ggi_struct_t *)drv->state)->vis
#define FRAMES ((gfx_ggi_struct_t *)drv->state)->frames

static gfx_mode_t *
_aberr(char *file, int line, char *message)
{
	fprintf(stderr,"GFXGGI: Fatal: %s L%d: %s\n", file, line, message);
	exit(1); /* Die */
	return NULL;
}

#define GGI_BUFFER_BACK 0
#define GGI_BUFFER_STATIC 1

#define SCI_GGI_SWAP_CTRL_CAPS (1 << 0)

typedef struct {
	ggi_visual_t vis;
	int frames;

	byte *alt_back_buffer; /* if frames < 2: Virtual back buffer */
	ggi_visual_t back_vis; /* Memory visual for the back buffer */

	byte *static_buffer; /* if frames < 3: Virtual static buffer */
	ggi_visual_t static_vis; /* Memory visual for the static buffer */

	gfx_pixmap_t *priority_maps[2];
	ggi_visual_t priority_visuals[2]; /* Visuals for the maps */

	int x_blank, y_blank;
	int x_blank2, y_blank2;

} gfx_ggi_struct_t;

static int flags;

static int
ggi_set_param(gfx_driver_t *drv, char *attribute, char *value)
{
	gfx_ggi_struct_t *meta = (gfx_ggi_struct_t *) drv->state;

	if (!strcmp(attribute, "swap_ctrl_caps") ||
	    !strcmp(attribute, "swap_caps_ctrl")) {
		if (string_truep(value))
			flags |= SCI_GGI_SWAP_CTRL_CAPS;
		else
			flags &= ~SCI_GGI_SWAP_CTRL_CAPS;

		return GFX_OK;
	}

	DEBUG_BASIC("ggi_set_param('%s' to '%s')\n", attribute, value);
	return GFX_ERROR;
}


static int
_open_meta_visuals(gfx_driver_t *drv)
{
	int i;

	for (i = 0; i < 2; i++) {
		if (!(STATE->priority_visuals[i] = ggiOpen("memory:pointer", STATE->priority_maps[i]->data))) {
			sciprintf("GFXGGI: Could not open priority map #%d\n", i);
			return 1;
		}
		if (ggiSetSimpleMode(STATE->priority_visuals[i], 320 * MODE->xfact, 200 * MODE->yfact, 1, GT_8BIT)) {
			sciprintf("GFXGGI: Could not set mode for priority visual %d\n", i);
			return 1;
		}
	}

	return 0;
}

static int
ggi_init_specific(gfx_driver_t *drv, int xres, int yres, int bpp)
{
	gfx_ggi_struct_t *meta;
	ggi_graphtype graphtype;
	const ggi_pixelformat *pixelformat;
	int frames = 3;

	switch (bpp) {
	case 1: graphtype = GT_8BIT; break;
	case 2: graphtype = GT_16BIT; break;
	case 3: graphtype = GT_24BIT; break;
	case 4: graphtype = GT_32BIT; break;
	default: sciprintf("GFXGGI: Error: Invalid bytes per pixel value: %d\n", bpp);
		return GFX_ERROR;
	}

	drv->state = NULL;

	if (ggiInit() < 0)
		return GFX_FATAL;

	meta = (gfx_ggi_struct_t *) sci_calloc(sizeof(gfx_ggi_struct_t), 1);

	if (!(meta->vis = ggiOpen(NULL))) {
		DEBUG_BASIC("ggiOpen() failed!\n");
		free(meta);
		ggiExit();
		return GFX_FATAL;
	}

	while ((frames > 0) && (ggiSetSimpleMode(meta->vis, xres * 320, yres * 200, frames, graphtype)))
		--frames;

	if (!frames) {
		DEBUG_BASIC("Initializing %dx%d at %d bpp failed\n", xres*320, yres*200, bpp << 3);
		free(meta);
		ggiExit();
		return GFX_ERROR;
	}

	DEBUG_BASIC("Initialized with %d frames\n", frames);

	meta->frames = frames;

	pixelformat = ggiGetPixelFormat(meta->vis);
	drv->mode = gfx_new_mode(xres, yres, pixelformat->size >> 3,
				 pixelformat->red_mask, pixelformat->green_mask, pixelformat->blue_mask,
				 0, /* alpha mask */
				 pixelformat->red_shift, pixelformat->green_shift, pixelformat->blue_shift,
				 0, /* alpha shift */
				 (bpp == 1)? 256 : 0, 0);
	drv->state = meta;

	meta->priority_maps[GGI_BUFFER_BACK] =
		gfx_pixmap_alloc_index_data(gfx_new_pixmap(320 * xres, 200 * yres, GFX_RESID_NONE, 0, 0));
	meta->priority_maps[GGI_BUFFER_STATIC] =
		gfx_pixmap_alloc_index_data(gfx_new_pixmap(320 * xres, 200 * yres, GFX_RESID_NONE, 0, 0));

	meta->priority_maps[GGI_BUFFER_BACK]->flags |= GFX_PIXMAP_FLAG_SCALED_INDEX;
	meta->priority_maps[GGI_BUFFER_STATIC]->flags |= GFX_PIXMAP_FLAG_SCALED_INDEX;

	if (_open_meta_visuals(drv)) {
		free(meta);
		gfx_free_pixmap(drv, meta->priority_maps[GGI_BUFFER_BACK]);
		gfx_free_pixmap(drv, meta->priority_maps[GGI_BUFFER_STATIC]);
		ggiClose(meta->vis);
		ggiExit();
		return GFX_ERROR;
	}

	if (frames < 2) {
		meta->alt_back_buffer = (byte *) sci_malloc(bpp * 320 * 200 * xres * yres);
		meta->back_vis = ggiOpen("memory:pointer", meta->alt_back_buffer, NULL);
		if (ggiSetSimpleMode(meta->back_vis, xres * 320, yres * 200, 1, GT_8BIT)) {
			sciprintf("GFXGGI: Warning: Setting mode for memory visual failed\n");
		}
	} else meta->alt_back_buffer = NULL;

	if (frames < 3) {
		meta->static_buffer = (byte *) sci_malloc(bpp * 320 * 200 * xres * yres);
		meta->static_vis = ggiOpen("memory:pointer", meta->static_buffer, NULL);
		if (ggiSetSimpleMode(meta->static_vis, xres * 320, yres * 200, 1, GT_8BIT)) {
			sciprintf("GFXGGI: Warning: Setting mode for memory visual #2 failed\n");
		}
	} else meta->static_buffer = NULL;

	init_input_ggi();
	flags = 0;

	return GFX_OK;
}


static int
ggi_init(gfx_driver_t *drv)
{
	gfx_ggi_struct_t *meta;
	ggi_mode mode;
	int x_blank, y_blank;
	const ggi_pixelformat *pixelformat;
	mode.frames = 3;
	mode.visible.x = mode.visible.y = mode.virt.x = mode.virt.y
		= mode.size.x = mode.size.y = mode.dpp.x = mode.dpp.y = GGI_AUTO;

	mode.graphtype = GT_AUTO;

	drv->state = NULL;

	if (ggiInit() < 0)
		return GFX_FATAL;

	meta = (gfx_ggi_struct_t *) sci_calloc(sizeof(gfx_ggi_struct_t), 1);

	if (!(meta->vis = ggiOpen(NULL))) {
		DEBUG_BASIC("ggiOpen() failed!\n");
		free(meta);
		ggiExit();
		return GFX_FATAL;
	}

	while (mode.frames && ggiCheckMode(meta->vis, &mode))
		--(mode.frames);

	if (!mode.frames) {
		sciprintf("GFXGGI: Could not find any graphics mode!\n");
		free(meta);
		ggiExit();
		return GFX_FATAL;
	}

	x_blank = mode.visible.x % 320;
	y_blank = mode.visible.y % 200;
	mode.visible.x -= x_blank;
	mode.visible.y -= y_blank;

	if (mode.visible.x == 0)
		mode.visible.x = 320;

	if (mode.visible.y == 0)
		mode.visible.y = 200;

	if (GT_DEPTH(mode.graphtype) < 8)
		mode.graphtype = GT_8BIT; /* We don't support less than 8 bpp */

	DEBUG_BASIC("Attempting to create mode with %dx%d, graphtype=%08x, %d frames\n",
		    mode.visible.x, mode.visible.y, mode.graphtype, mode.frames);

	mode.virt.x = mode.visible.x;
	mode.virt.y = mode.visible.y;
	mode.size.x = GGI_AUTO;
	mode.size.y = GGI_AUTO;

	if (ggiSetMode(meta->vis, &mode)) {
		sciprintf("GFXGGI: Could not set proposed graphics mode!\n");

		mode.virt.x = mode.visible.x += x_blank;
		mode.virt.y = mode.visible.y += y_blank;
		mode.size.x = GGI_AUTO;
		mode.size.y = GGI_AUTO;

		DEBUG_BASIC("Attempting to create augmented mode with %dx%d, graphtype=%08x, %d frames\n",
			    mode.visible.x, mode.visible.y, mode.graphtype, mode.frames);

		if (ggiSetMode(meta->vis, &mode)) {
			sciprintf("GFXGGI: Could not set proposed graphics mode!\n");
			free(meta);
			ggiExit();
			return GFX_FATAL;
		}

		ggiSetOrigin(meta->vis, (x_blank >> 1), (y_blank >> 1));

		mode.virt.x = mode.size.x = mode.visible.x -= x_blank;
		mode.virt.y = mode.size.y = mode.visible.y -= y_blank;
	} else
		x_blank = y_blank = 0;

	meta->frames = mode.frames;

	pixelformat = ggiGetPixelFormat(meta->vis);

	drv->mode = gfx_new_mode(mode.visible.x / 320, mode.visible.y / 200, pixelformat->size >> 3,
				 pixelformat->red_mask, pixelformat->green_mask, pixelformat->blue_mask,
				 0, /* alpha mask */
				 pixelformat->red_shift, pixelformat->green_shift, pixelformat->blue_shift,
				 0, /* alpha shift */
				 (GT_SCHEME(mode.graphtype) == GT_PALETTE)? (1 << GT_DEPTH(mode.graphtype)) : 0, 0);

	drv->state = meta;

	meta->priority_maps[GGI_BUFFER_BACK] =
		gfx_pixmap_alloc_index_data(gfx_new_pixmap(mode.visible.x, mode.visible.y, GFX_RESID_NONE, 0, 0));
	meta->priority_maps[GGI_BUFFER_STATIC] =
		gfx_pixmap_alloc_index_data(gfx_new_pixmap(mode.visible.x, mode.visible.y, GFX_RESID_NONE, 0, 0));

	meta->priority_maps[GGI_BUFFER_BACK]->flags |= GFX_PIXMAP_FLAG_SCALED_INDEX;
	meta->priority_maps[GGI_BUFFER_STATIC]->flags |= GFX_PIXMAP_FLAG_SCALED_INDEX;

	if (_open_meta_visuals(drv)) {
		free(meta);
		gfx_free_pixmap(drv, meta->priority_maps[GGI_BUFFER_BACK]);
		gfx_free_pixmap(drv, meta->priority_maps[GGI_BUFFER_STATIC]);
		ggiClose(meta->vis);
		ggiExit();
		return GFX_FATAL;
	}

	if (meta->frames < 2) {
		meta->alt_back_buffer = (byte *) sci_malloc((pixelformat->size >> 3) * mode.visible.x * mode.visible.y);
		meta->back_vis = ggiOpen("memory:pointer", meta->alt_back_buffer, NULL);
		if (ggiSetSimpleMode(meta->back_vis, mode.visible.x, mode.visible.y, 1, GT_8BIT)) {
			sciprintf("GFXGGI: Warning: Setting mode for memory visual failed\n");
		}
	} else meta->alt_back_buffer = NULL;

	if (meta->frames < 3) {
		meta->static_buffer = (byte *) sci_malloc((pixelformat->size >> 3) * mode.visible.x * mode.visible.y);
		meta->static_vis = ggiOpen("memory:pointer", meta->static_buffer, NULL);
		if (ggiSetSimpleMode(meta->static_vis, mode.visible.x, mode.visible.y, 1, GT_8BIT)) {
			sciprintf("GFXGGI: Warning: Setting mode for memory visual #2 failed\n");
		}
	} else meta->static_buffer = NULL;

	init_input_ggi();
	flags = 0;

	STATE->x_blank = x_blank;
	STATE->y_blank = y_blank;
	STATE->x_blank2 = x_blank >> 1;
	STATE->y_blank2 = y_blank >> 1;

	return GFX_OK;
}


static void
ggi_exit(gfx_driver_t *drv)
{
	if (drv->state) {
		gfx_free_pixmap(drv, STATE->priority_maps[0]);
		gfx_free_pixmap(drv, STATE->priority_maps[1]);

		if (STATE->frames < 2)
			ggiClose(STATE->back_vis);

		if (STATE->frames < 3)
			ggiClose(STATE->static_vis);

		ggiClose(STATE->priority_visuals[0]);
		ggiClose(STATE->priority_visuals[1]);

		ggiClose(VISUAL);
	}

	gfx_free_mode(drv->mode);

	ggiExit();

	if (FRAMES < 2)
		free(STATE->alt_back_buffer);

	free(drv->state);
}


static inline ggi_visual_t
get_writeable_back_visual(gfx_driver_t *drv)
{
	if (STATE->frames > 1) {
		ggiSetWriteFrame(VISUAL, 1);
		return VISUAL;
	} else
		return STATE->back_vis;
}

static inline ggi_visual_t
get_writeable_static_visual(gfx_driver_t *drv)
{
	if (STATE->frames > 2) {
		ggiSetWriteFrame(VISUAL, 2);
		return VISUAL;
	} else
		return STATE->static_vis;
}

static inline ggi_pixel
ggi_map_color(gfx_driver_t *drv, ggi_visual_t vis, gfx_color_t color)
{
	if (MODE->palette)
		return (ggi_pixel) color.visual.global_index;
	else {
		ggi_color gcolor;
		gcolor.r = ((int) color.visual.r << 8) | color.visual.r;
		gcolor.g = ((int) color.visual.g << 8) | color.visual.g;
		gcolor.b = ((int) color.visual.b << 8) | color.visual.b;
		gcolor.a = ((int) color.alpha << 8) | color.alpha;

		return ggiMapColor(vis, &gcolor);
	}
}

static int
ggi_draw_filled_rect(gfx_driver_t *drv, rect_t box, gfx_color_t color1, gfx_color_t color2,
		     gfx_rectangle_fill_t shade_mode);

static int
ggi_draw_line(gfx_driver_t *drv, point_t start, point_t end, gfx_color_t color,
	      gfx_line_mode_t line_mode, gfx_line_style_t line_style)
{
	ggi_pixel pixel;

	int xw = MODE->xfact, yw = MODE->yfact;
	int rx, ry, endx, endy;
	int xc, yc;

	if (line_mode == GFX_LINE_MODE_FINE)
		xw = yw = 1;
	else {
		xw = MODE->xfact, yw = MODE->yfact;
	}

	rx = start.x;
	ry = start.y;
	endx = end.x;
	endy = end.y;

	if ((rx == endx) && (ry == endy))
		return ggi_draw_filled_rect(drv, gfx_rect(rx, ry, xw, yw), color, color, GFX_SHADE_FLAT);

	if (color.mask & GFX_MASK_PRIORITY) {
		ggi_visual_t privis = STATE->priority_visuals[GFX_BUFFER_BACK];

		ggiSetGCForeground(privis, color.priority);

		for (xc = 0; xc < xw; xc++)
			ggiDrawLine(privis, rx + xc, ry, endx + xc, endy);
		if (yw > 0)
			for (xc = 0; xc < xw; xc++)
				ggiDrawLine(privis, rx + xc, ry + yw - 1, endx + xc, endy + yw - 1);

		if (yw > 1) {
			for (yc = 1; yc < yw-1; yc++)
				ggiDrawLine(privis, rx, ry + yc, endx, endy + yc);

			if (xw > 0)
				for (yc = 1; yc < yw-1; yc++)
					ggiDrawLine(privis, rx + xw - 1, ry + yc, endx + xw - 1, endy + yc);
		}
	}

	if (color.mask & GFX_MASK_VISUAL) {
		ggi_visual_t vis;

		pixel = ggi_map_color(drv, VISUAL, color);

		vis = get_writeable_back_visual(drv);

		ggiSetGCForeground(vis, pixel);

		for (xc = 0; xc < xw; xc++)
			ggiDrawLine(vis, rx + xc, ry, endx + xc, endy);
		if (yw > 0)
			for (xc = 0; xc < xw; xc++)
				ggiDrawLine(vis, rx + xc, ry + yw - 1, endx + xc, endy + yw - 1);

		if (yw > 1) {
			for (yc = 1; yc < yw-1; yc++)
				ggiDrawLine(vis, rx, ry + yc, endx, endy + yc);

			if (xw > 0)
				for (yc = 1; yc < yw-1; yc++)
					ggiDrawLine(vis, rx + xw - 1, ry + yc, endx + xw - 1, endy + yc);
		}
	}

	return GFX_OK;
}

static int
ggi_draw_filled_rect(gfx_driver_t *drv, rect_t box, gfx_color_t color1, gfx_color_t color2,
		     gfx_rectangle_fill_t shade_mode)
{
	if (color1.mask & GFX_MASK_VISUAL) {
		ggi_pixel pixel = ggi_map_color(drv, VISUAL, color1);
		ggi_visual_t vis = get_writeable_back_visual(drv);

		ggiSetGCForeground(vis, pixel);
		ggiDrawBox(vis, box.x, box.y, box.xl, box.yl);
	}

	if (color1.mask & GFX_MASK_PRIORITY) {
		ggi_visual_t vis;

		ggiSetGCForeground(vis = STATE->priority_visuals[GFX_BUFFER_BACK], color1.priority);
		ggiDrawBox(vis, box.x, box.y, box.xl, box.yl);
	}

	return GFX_OK;
}

/**************/
/* Pixmap ops */
/**************/

int
ggi_draw_pixmap(gfx_driver_t *drv, gfx_pixmap_t *pxm, int priority,
		rect_t src, rect_t dest, gfx_buffer_t buffer)
{
	ggi_visual_t vis = VISUAL;
	const ggi_directbuffer *dbuf;
	byte *pri_map = NULL;

	if (dest.xl != src.xl || dest.yl != src.yl) {
		GFXERROR("Attempt to scale pixmap (%dx%d)->(%dx%d): Not supported\n",
		      src.xl, src.yl, dest.xl, dest.yl);
		return GFX_ERROR;
	}

	switch (buffer) {

	case GFX_BUFFER_FRONT:
		GFXERROR("Attempt to draw pixmap to front buffer\n");
		return GFX_ERROR;

	case GFX_BUFFER_BACK:
		if (STATE->frames > 1)
			ggiSetWriteFrame(VISUAL, 1);
		else vis = STATE->back_vis;
		pri_map = STATE->priority_maps[GGI_BUFFER_BACK]->index_data;
		break;


	case GFX_BUFFER_STATIC:
		if (STATE->frames > 2)
			ggiSetWriteFrame(VISUAL, 2);
		else vis = STATE->static_vis;
		pri_map = STATE->priority_maps[GGI_BUFFER_STATIC]->index_data;
		break;


	default:
		GFXERROR("Unexpected buffer ID %d\n", buffer);
		return GFX_ERROR;

	}

	assert(pri_map);
	assert(vis);

	dbuf = ggiDBGetBuffer(vis, (vis == VISUAL)? buffer : 0);
	if (!dbuf) {
		GFXERROR("Could not acquire direct buffer!\n");
		return GFX_FATAL;
	}

	if (dbuf->resource) {
		if (ggiResourceAcquire(dbuf->resource, GGI_ACTYPE_WRITE)) {
			GFXERROR("Failed to allocate resource for direct buffer!\n");
			return GFX_FATAL;
		}
	}

	if (dbuf->layout != blPixelLinearBuffer) {
		char *type;

		switch (dbuf->layout) {
		case blPixelPlanarBuffer: type = "planar";
			break;

		case blExtended: type = "extended";
			break;

		default: type = "invalid";

		}

		GFXERROR("Error: Pixel buffer is %s! Non-linear buffers are not supported.\n", type);

		if (dbuf->resource)
			ggiResourceRelease(dbuf->resource);
		return GFX_FATAL;
	}

	gfx_crossblit_pixmap(MODE, pxm, priority, src, dest, (byte *) dbuf->write,
			     dbuf->buffer.plb.stride,
			     pri_map, MODE->xfact * 320, 1, 0);

	/*	ggiPutBox(vis, dest.x * MODE->xfact, dest.y * MODE->yfact, pxm->xl, pxm->yl, pxm->data); */

	if (dbuf->resource)
		ggiResourceRelease(dbuf->resource);

	return GFX_OK;
}

int
ggi_grab_pixmap(gfx_driver_t *drv, rect_t src, gfx_pixmap_t *pxm,
		gfx_map_mask_t map)
{
	ggi_visual_t vis = VISUAL;
	int error;

	pxm->xl = src.xl;
	pxm->yl = src.yl;

	if (STATE->frames > 1)
		ggiSetReadFrame(VISUAL, 1);
	else vis = STATE->back_vis;

	if (src.x < 0 || src.y < 0) {
		GFXERROR("Attempt to grab pixmap from invalid coordinates (%d,%d)\n", src.x, src.y);
		return GFX_ERROR;
	}

	if (!pxm->data) {
		GFXERROR("Attempt to grab pixmap to unallocated memory\n");
		return GFX_ERROR;
	}

	if ((error = ggiGetBox(vis, src.x, src.y, src.xl, src.yl, pxm->data))) {
		GFXERROR("ggiGetBox(%d, %d, %d, %d) returned error code %d\n", src.x, src.y, src.xl, src.yl, error);
		return GFX_ERROR;
	}

	return GFX_OK;
}

/************/
/* Misc ops */
/************/


static int
ggi_update(gfx_driver_t *drv, rect_t src, point_t dest, gfx_buffer_t buffer)
{
	int sx = src.x, sy = src.y;
	int dx = dest.x, dy = dest.y;
	int xl = src.xl, yl = src.yl;

	switch (buffer) {
	case GFX_BUFFER_FRONT:
		ggiSetWriteFrame(VISUAL, 0);

		if (STATE->frames < 2)
			ggiCrossBlit(STATE->back_vis, sx, sy, xl, yl, VISUAL, dx + STATE->x_blank2, dy + STATE->y_blank2);
		else {
			ggiSetReadFrame(VISUAL, 1);
			ggiCopyBox(VISUAL, sx, sy, xl, yl, dx + STATE->x_blank2, dy + STATE->y_blank2);
		}

		break;

	case GFX_BUFFER_BACK:
		if (src.x == dest.x && src.y == dest.y)
			gfx_copy_pixmap_box_i(STATE->priority_maps[GGI_BUFFER_BACK], STATE->priority_maps[GGI_BUFFER_STATIC], src);

		if (STATE->frames > 1)
			ggiSetWriteFrame(VISUAL, 1);

		if (STATE->frames > 2) {
			ggiSetReadFrame(VISUAL, 2);
			ggiCopyBox(VISUAL, sx, sy, xl, yl, dx, dy);
			return GFX_OK;
		}

		ggiCrossBlit(STATE->static_vis, sx, sy, xl, yl, STATE->back_vis, dx, dy);

		break;

	default:
		GFXERROR("Invalid buffer %d in update!\n", buffer);
		return GFX_ERROR;
	}
	return GFX_OK;
}


static int
ggi_set_palette(gfx_driver_t *drv, int index, byte red, byte green, byte blue)
{
	ggi_color color;
	color.r = (red << 8) | red;
	color.g = (green << 8) | green;
	color.b = (blue << 8) | blue;

	/*	DEBUG_POINTER(stderr,"Setting index %d to %04x %04x %04x\n", index, color.r, color.g, color.b); */

	if (ggiSetPalette(VISUAL, index, 1, &color) < 0)
		return GFX_ERROR;
	else
		return GFX_OK;
}


static int
ggi_set_static_buffer(gfx_driver_t *drv, gfx_pixmap_t *pic, gfx_pixmap_t *priority)
{
	ggi_visual_t vis = get_writeable_static_visual(drv);

	/* First, check if the priority map is sane */
	if (priority->index_xl != STATE->priority_maps[GGI_BUFFER_STATIC]->index_xl
	    || priority->index_yl != STATE->priority_maps[GGI_BUFFER_STATIC]->index_yl) {
		GFXERROR("Invalid priority map: (%dx%d) vs expected (%dx%d)\n",
			 priority->index_xl, priority->index_yl,
			 STATE->priority_maps[GGI_BUFFER_STATIC]->index_xl,
			 STATE->priority_maps[GGI_BUFFER_STATIC]->index_yl);
		return GFX_ERROR;
	}

	ggiPutBox(vis, 0, 0, pic->xl, pic->yl, pic->data);

	memcpy(STATE->priority_maps[GGI_BUFFER_STATIC]->index_data, priority->index_data,
	       priority->index_xl * priority->index_yl);

	return GFX_OK;
}


/********************/
/* Input management */
/********************/


struct timeval _sci_ggi_redraw_loopt, _sci_ggi_loopt;
/* timer variables */

int _sci_ggi_double_visual;

static int buckybits;

#define SCI_TIMEVAL_ADD(timev, addusec)  \
  { timev.tv_usec += addusec;            \
  while (timev.tv_usec >= 1000000L) {    \
    timev.tv_usec -= 1000000L;           \
    timev.tv_sec++;                      \
  }}

#define SCI_TIMEVAL_LATER(later, earlier) \
  ((later.tv_sec == earlier.tv_sec)? (later.tv_usec >= earlier.tv_usec) \
    : (later.tv_sec >= earlier.tv_sec))


static sci_event_t
ggi_get_event(gfx_driver_t *drv)
{
	struct timeval temptime = {0,0};
	gfx_ggi_struct_t *meta = (gfx_ggi_struct_t *) drv->state;
	int modifiers;

	while (1) {

		if (ggiEventPoll(VISUAL, emAll, &temptime)) {
			ggi_event event;
			sci_event_t retval;

			ggiEventRead(VISUAL, &event, emAll);

			if (flags & SCI_GGI_SWAP_CTRL_CAPS
			    && ((event.any.type == evKeyPress)
				|| (event.any.type == evKeyRepeat)
				|| (event.any.type == evKeyRelease))) {

				switch (event.key.label) {
				case GIIK_CtrlL: event.key.label = GIIK_CapsLock; break;
				case GIIK_CapsLock: event.key.label = GIIK_CtrlL; break;
				}
			}

			switch (event.any.type) {
			case evKeyPress:
			case evKeyRepeat:
				retval.type = SCI_EVT_KEYBOARD;
				retval.data=-1;
				retval.buckybits = 0;
				switch(event.key.label)
				{
				case GIIK_P4:
				case GIIK_Left: retval.data=SCI_K_LEFT; retval.buckybits = SCI_EVM_NUMLOCK; break;
				case GIIK_P6:
				case GIIK_Right: retval.data=SCI_K_RIGHT; retval.buckybits = SCI_EVM_NUMLOCK;break;
				case GIIK_P8:
				case GIIK_Up: retval.data=SCI_K_UP; retval.buckybits = SCI_EVM_NUMLOCK;break;
				case GIIK_P2:
				case GIIK_Down: retval.data=SCI_K_DOWN; retval.buckybits = SCI_EVM_NUMLOCK;break;
				case GIIK_P7:
				case GIIK_Home: retval.data=SCI_K_HOME; retval.buckybits = SCI_EVM_NUMLOCK;break;
				case GIIK_P1:
				case GIIK_End: retval.data=SCI_K_END; retval.buckybits = SCI_EVM_NUMLOCK;break;
				case GIIK_P9:
				case GIIK_PageUp: retval.data=SCI_K_PGUP; retval.buckybits = SCI_EVM_NUMLOCK;break;
				case GIIK_P3:
				case GIIK_PageDown: retval.data=SCI_K_PGDOWN; retval.buckybits = SCI_EVM_NUMLOCK;break;
				case GIIK_P5: retval.data=SCI_K_CENTER; retval.buckybits = SCI_EVM_NUMLOCK;break;

				case GIIUC_Minus:
				case GIIK_PMinus: retval.data = '-'; break;
				case GIIUC_Plus:
				case GIIK_PPlus: retval.data = '+'; break;

				case GIIUC_Grave: retval.data = '`'; break;
#if 0
				case GIIK_ShiftL: buckybits |= SCI_EVM_LSHIFT; break;
				case GIIK_ShiftR: buckybits |= SCI_EVM_RSHIFT; break;
				case GIIK_CtrlR:
				case GIIK_CtrlL: buckybits |= SCI_EVM_CTRL; break;
				case GIIK_AltL:
				case GIIK_AltR:
				case GIIK_MetaL:
				case GIIK_MetaR: buckybits |= SCI_EVM_ALT; break;
				case GIIK_CapsLock: buckybits ^= SCI_EVM_CAPSLOCK; break;
				case GIIK_NumLock: buckybits ^= SCI_EVM_NUMLOCK; break;
				case GIIK_ScrollLock: buckybits ^= SCI_EVM_SCRLOCK; break;
#endif
				case GIIK_Insert: buckybits ^= SCI_EVM_INSERT; break;
				case GIIK_PEnter:
				case GIIK_Enter: retval.data='\r'; break;
				case GIIUC_Tab: retval.data='\t'; break;
				case GIIUC_Space: retval.data=' '; break;
				case GIIUC_BackSpace: retval.data=SCI_K_BACKSPACE; break;
				case GIIK_F1: retval.data = SCI_K_F1; break;
				case GIIK_F2: retval.data = SCI_K_F2; break;
				case GIIK_F3: retval.data = SCI_K_F3; break;
				case GIIK_F4: retval.data = SCI_K_F4; break;
				case GIIK_F5: retval.data = SCI_K_F5; break;
				case GIIK_F6: retval.data = SCI_K_F6; break;
				case GIIK_F7: retval.data = SCI_K_F7; break;
				case GIIK_F8: retval.data = SCI_K_F8; break;
				case GIIK_F9: retval.data = SCI_K_F9; break;
				case GIIK_F10: retval.data = SCI_K_F10; break;
				case GIIUC_Escape: retval.data = SCI_K_ESC; break;

					/*FIXME: Add all special keys in a sane way*/
				default:
				{
					if(event.key.label>='a' && event.key.label<='z')
						retval.data=event.key.label-'a'+97;
					if(event.key.label>='A' && event.key.label<='Z')
						retval.data=event.key.label-'A'+97;
					if(event.key.label>='0' && event.key.label<='9')
						retval.data=event.key.label-'0'+48;
				}
				}

				modifiers = event.key.modifiers;

				buckybits = (buckybits & SCI_EVM_INSERT) |
					(((modifiers & GII_MOD_CAPS)? SCI_EVM_LSHIFT | SCI_EVM_RSHIFT : 0)
					 | ((modifiers & GII_MOD_CTRL)? SCI_EVM_CTRL : 0)
					 | ((modifiers & (GII_MOD_ALT | GII_MOD_META))? SCI_EVM_ALT : 0)
					 | ((modifiers & GII_MOD_NUM)? SCI_EVM_NUMLOCK : 0)
					 | ((modifiers & GII_MOD_SCROLL)? SCI_EVM_SCRLOCK : 0))
					^ ((modifiers & GII_MOD_SHIFT)? SCI_EVM_LSHIFT | SCI_EVM_RSHIFT : 0);

				if(retval.data==-1) continue;
				retval.buckybits |= buckybits;

				return retval;

			case evKeyRelease:
#if 0
				switch(event.key.label)
				{
				case GIIK_ShiftL: buckybits &= ~SCI_EVM_LSHIFT; break;
				case GIIK_ShiftR: buckybits &= ~SCI_EVM_RSHIFT; break;
				case GIIK_CtrlR:
				case GIIK_CtrlL: buckybits &= ~SCI_EVM_CTRL; break;
				case GIIK_AltL:
				case GIIK_AltR:
				case GIIK_MetaL:
				case GIIK_MetaR: buckybits &= ~SCI_EVM_ALT; break;
				}
#endif
				continue;

			case evPtrButtonPress:
				retval.type = SCI_EVT_MOUSE_PRESS;
				retval.data = event.pbutton.button;
				retval.buckybits=buckybits;

				if (event.pbutton.button == GII_PBUTTON_LEFT)
					retval.buckybits |= SCI_EVM_CTRL;

				if (event.pbutton.button == GII_PBUTTON_RIGHT)
					retval.buckybits |= SCI_EVM_LSHIFT | SCI_EVM_RSHIFT;
				return retval;

			case evPtrButtonRelease:
				retval.type = SCI_EVT_MOUSE_RELEASE;
				retval.data = event.pbutton.button;
				retval.buckybits=buckybits;

				if (event.pbutton.button == GII_PBUTTON_LEFT)
					retval.buckybits |= SCI_EVM_CTRL;

				if (event.pbutton.button == GII_PBUTTON_RIGHT)
					retval.buckybits |= SCI_EVM_LSHIFT | SCI_EVM_RSHIFT;

				return retval;

			case evPtrAbsolute:
				drv->pointer_x = event.pmove.x - STATE->x_blank2;
				drv->pointer_y = event.pmove.y - STATE->y_blank2;
				continue;

			case evPtrRelative:
				drv->pointer_x += event.pmove.x;
				drv->pointer_y += event.pmove.y;
				/* FIXME: This may make the pointer too fast on high res! */
				continue;
			}
		} else {
			sci_event_t retval;

			retval.type = SCI_EVT_NONE; /* Nothing happened */
			return retval;
		}

	}
}

static void
init_input_ggi()
{
	_sci_ggi_loopt = _sci_ggi_redraw_loopt;
	buckybits = SCI_EVM_INSERT; /* Start up in "insert" mode */
	/* reset timers, leave them at current time to send redraw events ASAP */

}


int
ggi_usleep(gfx_driver_t* drv, long usec)
{
	struct timeval tv = {0, usec};

	while(tv.tv_usec>0)
	{
		if(ggiEventPoll(VISUAL, emPtrMove, &tv))
		{
			ggi_event e;
			ggiEventRead(VISUAL, &e, emPtrMove);
			switch(e.any.type)
			{
			case evPtrRelative:
			{
				drv->pointer_x+=e.pmove.x;
				drv->pointer_y+=e.pmove.y;
			} return GFX_OK;
			case evPtrAbsolute:
			{
				drv->pointer_x=e.pmove.x - STATE->x_blank2;
				drv->pointer_y=e.pmove.y - STATE->y_blank2;
			} return GFX_OK;
			}
		}
	}
	return GFX_OK;
}




gfx_driver_t gfx_driver_ggi = {
	"ggi",
	GGI_DRIVER_VERSION,
	SCI_GFX_DRIVER_MAGIC,
	SCI_GFX_DRIVER_VERSION,
	NULL,
	0,0,
	GFX_CAPABILITY_FINE_LINES, GFX_DEBUG_POINTER
	| GFX_DEBUG_UPDATES | GFX_DEBUG_PIXMAPS | GFX_DEBUG_BASIC,
	ggi_set_param,
	ggi_init_specific,
	ggi_init,
	ggi_exit,
	ggi_draw_line,
	ggi_draw_filled_rect,
	NULL,
	NULL,
	ggi_draw_pixmap,
	ggi_grab_pixmap,
	ggi_update,
	ggi_set_static_buffer,
	NULL,
	ggi_set_palette,
	ggi_get_event,
	ggi_usleep
};


#endif /* HAVE_LIBGGI */
