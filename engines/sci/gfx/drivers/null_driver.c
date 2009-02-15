/***************************************************************************
 null_driver.c Copyright (C) 2000 Christoph Reichenbach


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

#include <gfx_driver.h>
#include <gfx_tools.h>

static int debug_sleep = 0;
static int debug_draw = 0;

static int
null_set_parameter(struct _gfx_driver *drv, char *attribute, char *value)
{
	printf("[GFX-NULL] Setting '%s' <- '%s'\n", attribute, value);

	return GFX_OK;
}



static int
null_init_specific(struct _gfx_driver *drv, int xfact, int yfact, int bytespp)
{
	printf("[GFX-NULL] Initializing specific: %dx%d, %d bytespp\n",
		  xfact, yfact, bytespp);

	drv->mode = gfx_new_mode(xfact, yfact, bytespp,
				 0, 0, 0, 0,
				 0, 0, 0, 0, 0, 0);

	return GFX_OK;
}


static int
null_init(struct _gfx_driver *drv)
{
	printf("[GFX-NULL] Initializing default\n");
	return null_init_specific(drv, 1, 1, 1);
}

static void
null_exit(struct _gfx_driver *drv)
{
	printf("[GFX-NULL] Exitting\n");
}


  /*** Drawing operations ***/

static int
null_draw_line(struct _gfx_driver *drv, point_t start, point_t end,
	       gfx_color_t color,
               gfx_line_mode_t line_mode, gfx_line_style_t line_style)
{
	if (debug_draw)
		printf("[GFX-NULL] Line (%d,%d) -- (%d,%d)\n",
		       GFX_PRINT_POINT(start),
		       GFX_PRINT_POINT(end));
	return GFX_OK;
}

static int
null_draw_filled_rect(struct _gfx_driver *drv, rect_t rect,
                      gfx_color_t color1, gfx_color_t color2,
                      gfx_rectangle_fill_t shade_mode)
{
	if (debug_draw)
	    printf("[GFX-NULL] Box (%d,%d)d(%d,%d)\n",
		   GFX_PRINT_RECT(rect));
	return GFX_OK;
}


  /*** Pixmap operations ***/

static int
null_register_pixmap(struct _gfx_driver *drv, gfx_pixmap_t *pxm)
{
	return GFX_OK;
}

static int
null_unregister_pixmap(struct _gfx_driver *drv, gfx_pixmap_t *pxm)
{
	return GFX_OK;
}

static int
null_draw_pixmap(struct _gfx_driver *drv, gfx_pixmap_t *pxm, int priority,
                 rect_t src, rect_t dest, gfx_buffer_t buffer)
{
	return GFX_OK;
}

static int
null_grab_pixmap(struct _gfx_driver *drv, rect_t src, gfx_pixmap_t *pxm,
                 gfx_map_mask_t map)
{
	return GFX_OK;
	pxm->xl = src.xl;
	pxm->yl = src.yl;
}


  /*** Buffer operations ***/

static int
null_update(struct _gfx_driver *drv, rect_t src, point_t dest, gfx_buffer_t buffer)
{
	return GFX_OK;
}

static int
null_set_static_buffer(struct _gfx_driver *drv, gfx_pixmap_t *pic, gfx_pixmap_t *priority)
{
	return GFX_OK;
}


  /*** Mouse pointer operations ***/


static int
null_set_pointer(struct _gfx_driver *drv, gfx_pixmap_t *pointer)
{
	return GFX_OK;
}


  /*** Palette operations ***/

static int
null_set_palette(struct _gfx_driver *drv, int index, byte red, byte green, byte blue)
{
	return GFX_OK;
}


  /*** Event management ***/

static sci_event_t
null_get_event(struct _gfx_driver *drv)
{
	sci_event_t input;

	input.type = SCI_EVT_NONE;

	return input;
}


static int
null_usec_sleep(struct _gfx_driver *drv, long usecs)
{
	if (debug_sleep)
		sciprintf("[GFX-NULL] Sleeping %ld usecs...\n", usecs);
	return GFX_OK;
}

gfx_driver_t
gfx_driver_null = {
	"null",
	"0.1",
	SCI_GFX_DRIVER_MAGIC,
	SCI_GFX_DRIVER_VERSION,
	NULL,
	0, 0,
	GFX_CAPABILITY_WINDOWED,
	GFX_DEBUG_POINTER | GFX_DEBUG_UPDATES | GFX_DEBUG_PIXMAPS | GFX_DEBUG_BASIC,
	null_set_parameter,
	null_init_specific,
	null_init,
	null_exit,
	null_draw_line,
	null_draw_filled_rect,
	null_register_pixmap,
	null_unregister_pixmap,
	null_draw_pixmap,
	null_grab_pixmap,
	null_update,
	null_set_static_buffer,
	null_set_pointer,
	null_set_palette,
	null_get_event,
	null_usec_sleep,
	NULL
};
