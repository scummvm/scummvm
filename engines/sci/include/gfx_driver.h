/***************************************************************************
 gfx_driver.h Copyright (C) 2000 Christoph Reichenbach

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

#ifndef _SCI_GFX_DRIVER_H_
#define _SCI_GFX_DRIVER_H_

#include "sci/include/gfx_system.h"
#include "sci/include/uinput.h"


typedef enum {
	GFX_BUFFER_FRONT = 0,
	GFX_BUFFER_BACK = 1,
	GFX_BUFFER_STATIC = 2
} gfx_buffer_t;


/* graphics driver hints */
#define GFX_CAPABILITY_SHADING (1<<0)
#define GFX_CAPABILITY_MOUSE_POINTER (1<<1)
#define GFX_CAPABILITY_COLOR_MOUSE_POINTER (1<<2)
#define GFX_CAPABILITY_PIXMAP_REGISTRY (1<<3)
#define GFX_CAPABILITY_SCALEABLE_PIXMAPS (1<<4)
#define GFX_CAPABILITY_STIPPLED_LINES (1<<6)
#define GFX_CAPABILITY_MOUSE_SUPPORT (1<<7)
#define GFX_CAPABILITY_POINTER_PIXMAP_REGISTRY (1<<8)
#define GFX_CAPABILITY_FINE_LINES (1<<9)
#define GFX_CAPABILITY_WINDOWED (1<<10)
#define GFX_CAPABILITY_KEYTRANSLATE (1<<11)

#define GFX_DEBUG_POINTER (1<<0)
#define GFX_DEBUG_UPDATES (1<<1)
#define GFX_DEBUG_PIXMAPS (1<<2)
#define GFX_DEBUG_BASIC (1<<3) /* Basic geometric ops (lines, boxes, etc) */

/* Principial graphics driver architecture
** ---------------------------------------
**
** All graphics drivers must provide
** - One visual front buffer (the actually visible thing)
** - Two dynamic back buffers:
**   + visual
**   + priority
** - Two static buffers  (containing the background image and picviews):
**   + visual
**   + priority
**
** The control buffer is handled outside the graphics driver architecture.
** Graphics are drawn by first setting the static buffers, then updating
** the back buffers (from the static buffers), adding all picviews and other
** widgets, and finally updating the front buffer.
**
** All coordinates refer to the scaled coordinate system.
** Invalid parameters should produce an error message.
** Support for some valid parameter values is optional (like different line
** modes). If an unsupported but valid parameter is specified, the function
** must use a reasonable default value.
*/

#define SCI_GFX_DRIVER_VERSION 0
#define SCI_GFX_DRIVER_MAGIC 0xf001337

typedef struct _gfx_driver { /* Graphics driver */

	const char *name; /* Graphics driver name. Unique identifier, should consist of
	                   ** lower-case (where applicable) alphanumerics
	                   */

	const char *version; /* Free-form version description (for informative purposes
		           ** only)
		           */

	int sci_driver_magic; /* SCI_GFX_DRIVER_MAGIC */
	int sci_driver_version; /* SCI_GFX_DRIVER_VERSION */

	gfx_mode_t *mode; /* Currently active mode, NULL if no mode is active */

	int pointer_x, pointer_y; /* Mouse pointer position */

	int capabilities; /* The driver's capabilities: A list of flags that may
	                  ** be pre-defined or set after a successful initialization.
	                  */
	/* Capability flags:
	**
	** The words MUST, SHOULD and MAY are to be interpreted as described in
	** the IETF RFC 1123.
	**
	** GFX_CAPABILITY_SHADING: draw_filled_rect() supports drawing shaded
	**    rectangles.
	** GFX_CAPABILITY_MOUSE_POINTER: The driver has built-in support for mouse
	**    pointers (monochrome or colored).
	** GFX_CAPABILITY_COLOR_MOUSE_POINTER: The driver has built-in support for
	**    colored mouse pointers.
	** GFX_CAPABILITY_PIXMAP_REGISTRY: System provides a pixmap registry. The
	**    invoking functions will assume that all pixmaps MUST be registered;
	**    if this flag is not set, it assumes that pixmaps MUST NOT be
	**    registered. Note that this excludes pointer pixmaps (see below)
	** GFX_CAPABILITY_POINTER_PIXMAP_REGISTRY: The system provides a pixmap
	**    registry which mouse pointers have to be registered in explicitly.
	**    This MUST be used only if the registry is identical to the 'normal' pixmap
	**    registry. Otherwise, it MUST be handled manually by the driver,
	**    unless pointer support is disabled completely.
	** GFX_CAPABILITY_SCALEABLE_PIXMAPS: Pixmap scaling is fully supported.
	**    If this capability is flag is set, all pixmaps passed to the driver
	**    will be unscaled.
	** GFX_CAPABILITY_STIPPLED_LINES: The driver is able to draw stippled lines
	**    horizontally and vertically (xl = 0 or yl = 0).
	** GFX_CAPABILITY_MOUSE_SUPPORT: There is some support for mouse (or similar)
	**    input. Note that this flag may be disabled by external code after
	**    initialization time, if no support for mouse pointer /drawing/ is
	**    available.
	** GFX_CAPABILITY_FINE_LINES: Should be set IFF the driver supports drawing
	**    fine (width 1) lines
	** GFX_CAPABILITY_WINDOWED: Driver runs in a window and supports a debug
	**    console running on stdin/stdout
	** GFX_CAPABILITY_KEYTRANSLATE: The driver's input layer automatically
	**    handles 'shifted' keys (i.e. turning shift-'a' to 'A' etc.).
	**    Drivers only need to handle this if they desire to support
	**    non-US keyboard layouts, usually by localisation methods
	**    provided by the underlying windowing or operating system.
	*/

	unsigned int debug_flags; /* Driver debug flags */


	/*** Initialization ***/

	int (*set_parameter)(struct _gfx_driver *drv, char *attribute, char *value);
	/* Sets a driver-specific parameter
	** Parameters: (gfx_driver_t *) drv: Pointer to the affected driver
	**             (char *) attribute: Name of the attribute/parameter to set
	**             (char *) value: The value to set, or NULL to query the value
	** Returns   : (int) GFX_OK or GFX_FATAL, which signals a fatal error
	**                   condition.
	** This function should make extensive use of sciprintf() to signal invalid
	** values or unapplicable attributes.
	** Note that it may be called either before initialization (to interpret
	** config file or command line parameters) or afterwars (from the command
	** console).
	*/

	int (*init_specific)(struct _gfx_driver *drv, int xres, int yres,
	                     int bytespp);
	/* Attempts to initialize a specific graphics mode
	** Parameters: (gfx_driver_t *) drv: The affected driver
	**             (int x int) xres, yres: Horizontal and vertical scaling
	**                         factors
	**             (int) bytespp: Any of GFX_COLOR_MODE_*. GFX_COLOR_MODE_INDEX
	**                   implies color index mode.
	** Returns   : (int) GFX_OK on success, GFX_ERROR if the mode could not be
	**                   set, or GFX_FATAL if the graphics target is unuseable.
	** The scaling factors apply to the standard SCI resolution of 320x200 pixels
	** and is used for internal representation of graphical data. The physical
	** resolution set by the graphics driver may be different for practical
	** reasons.
	** Must also set drv->mode, preferably with the gfx_new_mode() function
	** specified in gfx_tools.h.
	*/

	int (*init)(struct _gfx_driver *drv);
	/* Initialize any graphics mode
	** Parameters: (gfx_driver_t *) drv: The affected driver
	** Returns   : (int) GFX_OK on success, GFX_FATAL otherwise.
	** This function attempts to set /any/ graphics mode, starting with the one
	** most 'natural' to the graphics target. Target implementors have relatively
	** free reign in choosing the heuristics used to determine the resulting
	** mode.
	** Must also set drv->mode, preferably with the gfx_new_mode() function
	** specified in gfx_tools.h.
	*/

	void (*exit)(struct _gfx_driver *drv);
	/* Uninitializes the current graphics mode
	** Paramters: (gfx_driver_t *) drv: The driver to uninitialize
	** Return   : (void)
	** This function frees all memory allocated by the graphics driver,
	** including mode and palette information, uninstalls all console commands
	** introduced by preceeding init() or init_specific() commands, and does any
	** clean-up work (like closing visuals or returning to text mode) required by
	** the graphics infrastructure used.
	*/


	/*** Drawing operations ***/

	int (*draw_line)(struct _gfx_driver *drv,
	                 point_t start, point_t end,
	                 gfx_color_t color,
	                 gfx_line_mode_t line_mode, gfx_line_style_t line_style);
	/* Draws a single line to the back buffer.
	** Parameters: (gfx_driver_t *) drv: The driver affected
	**             (point_t) start: Starting point of the line to draw
	**             (point_t) end: End point of the line to draw
	**             (gfx_color_t *) color: The color to draw with
	**             (int) line_mode: Any of the line modes
	**             (int) line_style: Any of the line styles
	** Returns   : (int) GFX_OK or GFX_FATAL
	** Note that color.priority is relevant and must be drawn if
	** (color.mask & GFX_MASK_PRIORITY).
	** Support for line modes other than GFX_LINE_MODE_FAST is optional.
	** For non-fine lines, the coordinates provided describe the upper left
	** corner of the pixels of the line to draw.
	** line_style support is optional, if GFX_CAPABILITY_STIPPLED_LINES is not
	** set.
	*/

	int (*draw_filled_rect)(struct _gfx_driver *drv, rect_t rect,
	                        gfx_color_t color1, gfx_color_t color2,
	                        gfx_rectangle_fill_t shade_mode);
	/* Draws a single filled and possibly shaded rectangle to the back buffer.
	** Parameters: (gfx_driver_t *) drv: The driver affected
	**             (rect_t *) rect: The rectangle to draw
	**             (gfx_color_t *) color1, color2: The colors to draw with
	**             (int) shade_mode: Any of GFX_SHADE_*.
	** Returns   : (int) GFX_OK or GFX_FATAL
	** Note that color.priority is relevant and must be drawn if
	** (color.mask & GFX_MASK_PRIORITY).
	** color2 is relevant only if shade_mode is not GFX_SHADE_FLAT.
	** Support for shade modes other than GFX_SHADE_FLAT is optional.
	*/

	/*** Pixmap operations ***/

	int (*register_pixmap)(struct _gfx_driver *drv, gfx_pixmap_t *pxm);
	/* Registers a pixmap with the driver.
	** Parameters: (gfx_driver_t *) drv: The driver
	**             (gfx_pixmap_t *) pxm: The pixmap to register
	** Returns   : GFX_OK or GFX_FATAL
	** This function may be NULL if GFX_CAPABILITY_PIXMAP_REGISTRY is not
	** set.
	** pxm->internal may be used to store any handle or meta information.
	*/

	int (*unregister_pixmap)(struct _gfx_driver *drv, gfx_pixmap_t *pxm);
	/* Unregisters a pixmap previously registered with register_pixmap()
	** Parameters: (gfx_driver_t *) drv: The driver
	**             (gfx_pixmap_t *) pxm: The pixmap to register
	** Returns   : (int) GFX_OK or GFX_FATAL, or GFX_ERROR if pxm was
	**             not registered
	** Just like register_pixmap(), this function may be NULL unless
	** GFX_CAPABILITY_PIXMAP_REGISTRY is set.
	*/

	int (*draw_pixmap)(struct _gfx_driver *drv, gfx_pixmap_t *pxm, int priority,
	                   rect_t src, rect_t dest, gfx_buffer_t buffer);
	/* Draws part of a pixmap to the static or back buffer
	** Parameters: (gfx_driver_t *) drv: The affected driver
	**             (gfx_pixmap_t *) pxm: The pixmap to draw
	**             (int) priority: The priority to draw with, or GFX_NO_PRIORITY
	**                   to draw on top of everything without setting the
	**                   priority back buffer
	**             (rect_t) src: The pixmap-relative source rectangle
	**             (rect_t) dest: The destination rectangle
	**             (int) buffer: One of GFX_BUFFER_STATIC and GFX_BUFFER_BACK
	** Returns   : (int) GFX_OK or GFX_FATAL, or GFX_ERROR if pxm was not
	**                   (but should have been) registered.
	** dest.xl and dest.yl must be evaluated and used for scaling if
	** GFX_CAPABILITY_SCALEABLE_PIXMAPS is supported.
	*/

	int (*grab_pixmap)(struct _gfx_driver *drv, rect_t src, gfx_pixmap_t *pxm,
	                   gfx_map_mask_t map);
	/* Grabs an image from the visual or priority back buffer
	** Parameters: (gfx_driver_t *) drv: The affected driver
	**             (rect_t) src: The rectangle to grab
	**             (gfx_pixmap_t *) pxm: The pixmap structure the data is to
	**                              be written to
	**             (int) map: GFX_MASK_VISUAL or GFX_MASK_PRIORITY
	** Returns   : (int) GFX_OK, GFX_FATAL, or GFX_ERROR for invalid map values
	** pxm may be assumed to be empty and pre-allocated with an appropriate
	** memory size.
	** This function is now mandatory.
	*/


	/*** Buffer operations ***/

	int (*update)(struct _gfx_driver *drv, rect_t src, point_t dest,
	              gfx_buffer_t buffer);
	/* Updates the front buffer or the back buffers
	** Parameters: (gfx_driver_t *) drv: The affected driver
	**             (rect_t) src: Source rectangle
	**             (point_t) dest: Destination point
	**             (int) buffer: One of GFX_BUFFER_FRONT or GFX_BUFFER_BACK
	** Returns   : (int) GFX_OK, GFX_ERROR or GFX_FATAL
	** This function updates either the visual front buffer, or the two back
	** buffers, by copying the specified source region to the destination
	** region.
	** For heuristical reasons, it may be assumed that the x and y fields of
	** src and dest will be identical in /most/ cases.
	** If they aren't, the priority map will not be required to be copied.
	*/

	int (*set_static_buffer)(struct _gfx_driver *drv, gfx_pixmap_t *pic,
	                         gfx_pixmap_t *priority);
	/* Sets the contents of the static visual and priority buffers
	** Parameters: (gfx_driver_t *) drv: The affected driver
	**             (gfx_pixmap_t *) pic: The image defining the new content
	**                              of the visual back buffer
	**             (gfx_pixmap_t *) priority: The priority map containing
	**                              the new content of the priority back buffer
	**                              in the index buffer
	** Returns   : (int) GFX_OK or GFX_FATAL
	** pic and priority may be modified or written to freely. They may also be
	** used as the actual static buffers, since they are not freed and re-
	** allocated between calls to set_static_buffer() and update(), unless
	** exit() was called in between.
	** Note that later version of the driver interface may disallow modifying
	** pic and priority.
	** pic and priority are always scaled to the appropriate resolution, even
	** if GFX_CAPABILITY_SCALEABLE_PIXMAPS is set.
	*/


	/*** Mouse pointer operations ***/

	int (*set_pointer)(struct _gfx_driver *drv, gfx_pixmap_t *pointer);
	/* Sets a new mouse pointer.
	** Parameters: (gfx_driver_t *) drv: The driver to modify
	**             (gfx_pixmap_t *) pointer: The pointer to set, or NULL to set
	**                              no pointer
	** Returns   : (int) GFX_OK or GFX_FATAL
	** This function may be NULL if GFX_CAPABILITY_MOUSE_POINTER is not set.
	** If pointer is not NULL, it will have been scaled to the appropriate
	** size and registered as a pixmap (if neccessary) beforehand.
	** If this function is called for a target that supports only two-color
	** pointers, the image is a color index image, where only color index values
	** 0, 1, and GFX_COLOR_INDEX_TRANSPARENT are used.
	*/


	/*** Palette operations ***/

	int (*set_palette)(struct _gfx_driver *drv, int index, byte red, byte green,
	                   byte blue);
	/* Manipulates a palette index in the hardware palette
	** Parameters: (gfx_driver_t *) drv: The driver affected
	**             (int) index: The index of the palette entry to modify
	**             (int x int x int) red, green, blue: The RGB intensities to
	**                               set for the specified index. The minimum
	**                               intensity is 0, maximum is 0xff.
	** Returns   : (int) GFX_OK, GFX_ERROR or GFX_FATAL
	** This function does not need to update mode->palette, as this is done
	** by the calling code.
	** set_palette() is only required for targets supporting color index mode.
	*/


	/*** Event management ***/

	sci_event_t (*get_event)(struct _gfx_driver *drv);
	/* Returns the next event in the event queue for this driver
	** Parameters: (gfx_driver_t *) drv: The driver to query
	** Returns   : (sci_event_t) The oldest event still in the driver's event
	**                           queue, or the null event if there is none.
	*/

	int (*usec_sleep)(struct _gfx_driver *drv, long usecs);
	/* Sleeps the specified amount of microseconds, or until the mouse moves
	** Parameters: (gfx_driver_t *) drv: The relevant driver
	**             (long) usecs: Amount of microseconds to sleep
	** Returns   : (int) GFX_OK or GFX_FATAL
	** This function returns when the specified amount of microseconds has
	** elapsed, or when the mouse pointer has been moved and needs to be redrawn.
	** Only targets that can handle colored mouse pointers may choose to handle
	** all mouse management internally.
	*/

	void *state; /* Reserved for internal use */

} gfx_driver_t;



gfx_driver_t *
gfx_find_driver(char *, char *name);
/* Attempts to match a graphics driver to a name
** Parameters: (char *) path: The path to search in
**             (char *) name: The name of the graphics driver to look for
**                            or NULL for the default driver
** Returns   : (gfx_driver_t *) The resulting driver, or NULL if none
**                              was found
*/

const char *
gfx_get_driver_name(int nr);
/* Retreives the name of the driver with the specified number
** Parameters: (int) nr: Number of the driver
**             (char *) The driver's name
** Note that this function only makes sense within a loop or if nr=0, since
** the result value is valid iff nr >= 0 AND there does not exist an nr'
** with 0 <= nr' < nr so that gfx_get_driver_name(nr') == NULL.
*/

/*** Utility functions for set_parameter implementations */

int
string_truep(char *value);
/* Tests whether a string expresses truth
** Parameters: (char *) value: The value to test
** Returns   : non-zero iff 'value' contans a string expressing something
** along the lines of "yes"
*/

int
string_falsep(char *value);
/* Tests whether a string expresses falsehood
** Parameters: (char *) value: The value to test
** Returns   : non-zero iff 'value' contans a string expressing something
** along the lines of "no"
*/



#endif /* !_SCI_GFX_DRIVER_H_ */
