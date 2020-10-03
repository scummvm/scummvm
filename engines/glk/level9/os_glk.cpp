/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "glk/level9/os_glk.h"
#include "glk/level9/level9_main.h"
#include "glk/level9/level9.h"
#include "common/config-manager.h"
#include "common/textconsole.h"

namespace Glk {
namespace Level9 {

/*---------------------------------------------------------------------*/
/*  Module variables, miscellaneous other stuff                        */
/*---------------------------------------------------------------------*/

/* Glk Level 9 port version number. */
static const glui32 GLN_PORT_VERSION = 0x00020201;

/*
 * We use a maximum of three Glk windows, one for status, one for pictures,
 * and one for everything else.  The status and pictures windows may be
 * nullptr, depending on user selections and the capabilities of the Glk
 * library.
 */
static winid_t gln_main_window, gln_status_window, gln_graphics_window;

/*
 * Transcript stream and input log.  These are nullptr if there is no current
 * collection of these strings.
 */
static strid_t gln_transcript_stream, gln_inputlog_stream;

/* Input read log stream, for reading back an input log. */
static strid_t gln_readlog_stream;

/* Note about whether graphics is possible, or not. */
bool gln_graphics_possible;

/* Options that may be turned off by command line flags. */
bool gln_graphics_enabled, gln_intercept_enabled, gln_prompt_enabled;
bool gln_loopcheck_enabled, gln_abbreviations_enabled, gln_commands_enabled;

/* Reason for stopping the game, used to detect restarts and ^C exits. */
enum StopReason {
	STOP_NONE, STOP_FORCE, STOP_RESTART, STOP_EXIT
};
static StopReason gln_stop_reason;

/* Level 9 standard input prompt string. */
static const char *const GLN_INPUT_PROMPT = "> ";

/*
 * Typedef equivalents for interpreter types (uncapitalized to avoid appearing
 * as macros), and some internal interpreter symbols symbols used for our own
 * deviant purposes.
 */
typedef L9BOOL gln_bool;
typedef L9BYTE gln_byte;
typedef L9UINT16 gln_uint16;
typedef L9UINT32 gln_uint32;

extern void save();
extern void restore();
extern gln_bool Cheating;
extern gln_uint32 FileSize;

/* Forward declarations of event wait and other miscellaneous functions. */
static void gln_event_wait(glui32 wait_type, event_t *event);
static void gln_event_wait_2(glui32 wait_type_1,
                             glui32 wait_type_2, event_t *event);

static void gln_watchdog_tick();
static void gln_standout_string(const char *message);

static int gln_confirm(const char *prompt);

/* Picture variables */
/* Graphics file directory, and type of graphics found in it. */
static char *gln_graphics_bitmap_directory = nullptr;
static BitmapType gln_graphics_bitmap_type = NO_BITMAPS;

/* The current picture id being displayed. */
enum { GLN_PALETTE_SIZE = 32 };
static gln_byte *gln_graphics_bitmap = nullptr;
static gln_uint16 gln_graphics_width = 0,
gln_graphics_height = 0;
static Colour gln_graphics_palette[GLN_PALETTE_SIZE]; /* = { 0, ... }; */
static int gln_graphics_picture = -1;

/*
 * Flags set on new picture, and on resize or arrange events, and a flag
 * to indicate whether background repaint is stopped or active.
 */
static int gln_graphics_new_picture = FALSE,
gln_graphics_repaint = FALSE,
gln_graphics_active = FALSE;

/*
 * State to monitor the state of interpreter graphics.  The values of the
 * enumerations match the modes supplied by os_graphics().
 */
enum GraphicsState {
	GLN_GRAPHICS_OFF = 0,
	GLN_GRAPHICS_LINE_MODE = 1,
	GLN_GRAPHICS_BITMAP_MODE = 2
};
static GraphicsState gln_graphics_interpreter_state = GLN_GRAPHICS_OFF;


/*
 * Pointer to the two graphics buffers, one the off-screen representation
 * of pixels, and the other tracking on-screen data.  These are temporary
 * graphics malloc'ed memory, and should be free'd on exit.
 */
static gln_byte *gln_graphics_off_screen = nullptr,
*gln_graphics_on_screen = nullptr;

/*
 * The number of colors used in the palette by the current picture.  Because
 * of the way it's queried, we risk a race, with admittedly a very low
 * probability, with the updater.  So, it's initialized instead to the
 * largest possible value.  The real value in use is inserted on the first
 * picture update timeout call for a new picture.
 */
static int gln_graphics_color_count = GLN_PALETTE_SIZE;


/*---------------------------------------------------------------------*/
/*  Glk port utility functions                                         */
/*---------------------------------------------------------------------*/

void gln_initialize() {
	gln_main_window = nullptr;
	gln_status_window = nullptr;
	gln_graphics_window = nullptr;
	gln_transcript_stream = nullptr;
	gln_inputlog_stream = nullptr;
	gln_readlog_stream = nullptr;
	gln_graphics_possible = TRUE;
	gln_graphics_enabled = TRUE;
	gln_intercept_enabled = TRUE;
	gln_prompt_enabled = TRUE;
	gln_loopcheck_enabled = TRUE;
	gln_abbreviations_enabled = TRUE;
	gln_commands_enabled = TRUE;
	gln_stop_reason = STOP_NONE;

	gln_graphics_bitmap_directory = nullptr;
	gln_graphics_bitmap_type = NO_BITMAPS;
	gln_graphics_bitmap = nullptr;
	gln_graphics_width = 0;
	gln_graphics_height = 0;
	gln_graphics_picture = -1;
	gln_graphics_new_picture = FALSE;
	gln_graphics_repaint = FALSE;
	gln_graphics_active = FALSE;
	gln_graphics_interpreter_state = GLN_GRAPHICS_OFF;
	gln_graphics_off_screen = nullptr;
	gln_graphics_on_screen = nullptr;
	gln_graphics_color_count = GLN_PALETTE_SIZE;
}

/*
 * gln_fatal()
 *
 * Fatal error handler.  The function returns, expecting the caller to
 * abort() or otherwise handle the error.
 */
static void gln_fatal(const char *string) {
	/*
	 * If the failure happens too early for us to have a window, print
	 * the message to stderr.
	 */
	if (!gln_main_window) {
		warning("INTERNAL ERROR: %s", string);
		return;
	}

	/* Cancel all possible pending window input events. */
	g_vm->glk_cancel_line_event(gln_main_window, nullptr);
	g_vm->glk_cancel_char_event(gln_main_window);

	/* Print a message indicating the error. */
	g_vm->glk_set_window(gln_main_window);
	g_vm->glk_set_style(style_Normal);
	g_vm->glk_put_string("\n\nINTERNAL ERROR: ");
	g_vm->glk_put_string(string);

	g_vm->glk_put_string("\n\nPlease record the details of this error, try to"
	                     " note down everything you did to cause it, and email"
	                     " this information to simon_baldwin@yahoo.com.\n\n");
}


/*
 * gln_malloc()
 * gln_realloc()
 *
 * Non-failing malloc and realloc; call gln_fatal and exit if memory
 * allocation fails.
 */
static void *gln_malloc(size_t size) {
	void *pointer;

	pointer = malloc(size);
	if (!pointer) {
		gln_fatal("GLK: Out of system memory");
		g_vm->glk_exit();
	}

	return pointer;
}

static void *gln_realloc(void *ptr, size_t size) {
	void *pointer;

	pointer = realloc(ptr, size);
	if (!pointer) {
		gln_fatal("GLK: Out of system memory");
		g_vm->glk_exit();
	}

	return pointer;
}


/*
 * gln_strncasecmp()
 * gln_strcasecmp()
 *
 * Strncasecmp and strcasecmp are not ANSI functions, so here are local
 * definitions to do the same jobs.
 *
 * They're global here so that the core interpreter can use them; otherwise
 * it tries to use the non-ANSI str[n]icmp() functions.
 */
int gln_strncasecmp(const char *s1, const char *s2, size_t n) {
	size_t index;

	for (index = 0; index < n; index++) {
		int diff;

		diff = g_vm->glk_char_to_lower(s1[index]) - g_vm->glk_char_to_lower(s2[index]);
		if (diff < 0 || diff > 0)
			return diff < 0 ? -1 : 1;
	}

	return 0;
}

int gln_strcasecmp(const char *s1, const char *s2) {
	size_t s1len, s2len;
	int result;

	s1len = strlen(s1);
	s2len = strlen(s2);

	result = gln_strncasecmp(s1, s2, s1len < s2len ? s1len : s2len);
	if (result < 0 || result > 0)
		return result;
	else
		return s1len < s2len ? -1 : s1len > s2len ? 1 : 0;
}

/*---------------------------------------------------------------------*/
/*  Glk port bitmap picture functions                                  */
/*---------------------------------------------------------------------*/

/* R,G,B color triple definition. */
struct gln_rgb_t {
	int red, green, blue;
};
typedef gln_rgb_t *gln_rgbref_t;

/*
 * Maximum number of regions to consider in a single repaint pass.  A
 * couple of hundred seems to strike the right balance between not too
 * sluggardly picture updates, and responsiveness to input during graphics
 * rendering, when combined with short timeouts.
 */
static const int GLN_REPAINT_LIMIT = 256;

/*
 * Graphics timeout; we like an update call after this period (ms).  In
 * practice, this timeout may actually be shorter than the time taken
 * to reach the limit on repaint regions, but because Glk guarantees that
 * user interactions (in this case, line events) take precedence over
 * timeouts, this should be okay; we'll still see a game that responds to
 * input each time the background repaint function yields.
 *
 * Setting this value is tricky.  We'd like it to be the shortest possible
 * consistent with getting other stuff done, say 10ms.  However, Xglk has
 * a granularity of 50ms on checking for timeouts, as it uses a 1/20s
 * timeout on X select.  This means that the shortest timeout we'll ever
 * get from Xglk will be 50ms, so there's no point in setting this shorter
 * than that.  With luck, other Glk libraries will be more efficient than
 * this, and can give us higher timer resolution; we'll set 50ms here, and
 * hope that no other Glk library is worse.
 */
static const glui32 GLN_GRAPHICS_TIMEOUT = 50;

/*
 * Count of timeouts to wait on.  Waiting after a repaint smooths the
 * display where the frame is being resized, by helping to avoid graphics
 * output while more resize events are received; around 1/2 second seems
 * okay.
 */
static const int GLN_GRAPHICS_REPAINT_WAIT = 10;

#ifdef GFX_SCALE_BY_FACTOR
/* Pixel size multiplier for image size scaling. */
static const int GLN_GRAPHICS_PIXEL = 1;
#endif

/* Proportion of the display to use for graphics. */
static const glui32 GLN_GRAPHICS_PROPORTION = 50;

/*
 * Special title picture number, requiring its own handling, and count of
 * timeouts to wait on after fully rendering the title picture (~2 seconds).
 */
static const int GLN_GRAPHICS_TITLE_PICTURE = 0,
                 GLN_GRAPHICS_TITLE_WAIT = 40;

/*
 * Border and shading control.  For cases where we can't detect the back-
 * ground color of the main window, there's a default, white, background.
 * Bordering is black, with a 1 pixel border, 2 pixel shading, and 8 steps
 * of shading fade.
 */
static const glui32 GLN_GRAPHICS_DEFAULT_BACKGROUND = 0x00ffffff,
                    GLN_GRAPHICS_BORDER_COLOR = 0x00000000;
static const int GLN_GRAPHICS_BORDER = 1,
                 GLN_GRAPHICS_SHADING = 2,
                 GLN_GRAPHICS_SHADE_STEPS = 8;

/*
 * Guaranteed unused pixel value.  This value is used to fill the on-screen
 * buffer on new pictures or repaints, resulting in a full paint of all
 * pixels since no off-screen, real picture, pixel will match it.
 */
static const int GLN_GRAPHICS_UNUSED_PIXEL = 0xff;


/*
 * gln_graphics_open()
 *
 * If it's not open, open the graphics window.  Returns TRUE if graphics
 * was successfully started, or already on.
 */
static int gln_graphics_open() {
	if (!gln_graphics_window) {
		gln_graphics_window = g_vm->glk_window_open(gln_main_window,
		                      winmethod_Above
		                      | winmethod_Proportional,
		                      GLN_GRAPHICS_PROPORTION,
		                      wintype_Graphics, 0);
	}

	return gln_graphics_window != nullptr;
}


/*
 * gln_graphics_close()
 *
 * If open, close the graphics window and set back to nullptr.
 */
static void gln_graphics_close() {
	if (gln_graphics_window) {
		g_vm->glk_window_close(gln_graphics_window, nullptr);
		gln_graphics_window = nullptr;
	}
}


/*
 * gln_graphics_start()
 *
 * If graphics enabled, start any background picture update processing.
 */
static void gln_graphics_start() {
	if (gln_graphics_enabled) {
		/* If not running, start the updating "thread". */
		if (!gln_graphics_active) {
			g_vm->glk_request_timer_events(GLN_GRAPHICS_TIMEOUT);
			gln_graphics_active = TRUE;
		}
	}
}


/*
 * gln_graphics_stop()
 *
 * Stop any background picture update processing.
 */
static void gln_graphics_stop() {
	/* If running, stop the updating "thread". */
	if (gln_graphics_active) {
		g_vm->glk_request_timer_events(0);
		gln_graphics_active = FALSE;
	}
}


/*
 * gln_graphics_are_displayed()
 *
 * Return TRUE if graphics are currently being displayed, FALSE otherwise.
 */
static int gln_graphics_are_displayed() {
	return gln_graphics_window != nullptr;
}


/*
 * gln_graphics_paint()
 *
 * Set up a complete repaint of the current picture in the graphics window.
 * This function should be called on the appropriate Glk window resize and
 * arrange events.
 */
static void gln_graphics_paint() {
	if (gln_graphics_enabled && gln_graphics_are_displayed()) {
		/* Set the repaint flag, and start graphics. */
		gln_graphics_repaint = TRUE;
		gln_graphics_start();
	}
}


/*
 * gln_graphics_restart()
 *
 * Restart graphics as if the current picture is a new picture.  This
 * function should be called whenever graphics is re-enabled after being
 * disabled.
 */
static void gln_graphics_restart() {
	if (gln_graphics_enabled && gln_graphics_are_displayed()) {
		/* Set the new picture flag, and start graphics. */
		gln_graphics_new_picture = TRUE;
		gln_graphics_start();
	}
}


/*
 * gln_graphics_count_colors()
 *
 * Analyze an image, and return an overall count of how many colors out of
 * the palette are used.
 */
static int gln_graphics_count_colors(gln_byte bitmap[], gln_uint16 width, gln_uint16 height) {
	int x, y, count;
	long usage[GLN_PALETTE_SIZE], index_row;
	assert(bitmap);

	/*
	 * Traverse the image, counting each pixel usage.  For the y iterator,
	 * maintain an index row as an optimization to avoid multiplications in
	 * the loop.
	 */
	count = 0;
	memset(usage, 0, sizeof(usage));
	for (y = 0, index_row = 0; y < height; y++, index_row += width) {
		for (x = 0; x < width; x++) {
			long index;

			/* Get the pixel index, and update the count for this color. */
			index = index_row + x;
			usage[bitmap[index]]++;

			/* If color usage is now 1, note new color encountered. */
			if (usage[bitmap[index]] == 1)
				count++;
		}
	}

	return count;
}


/*
 * gln_graphics_split_color()
 * gln_graphics_combine_color()
 *
 * General graphics helper functions, to convert between RGB and Glk glui32
 * color representations.
 */
static void gln_graphics_split_color(glui32 color, gln_rgbref_t rgb_color) {
	assert(rgb_color);

	rgb_color->red   = (color >> 16) & 0xff;
	rgb_color->green = (color >> 8) & 0xff;
	rgb_color->blue  = color & 0xff;
}

static glui32 gln_graphics_combine_color(gln_rgbref_t rgb_color) {
	glui32 color;
	assert(rgb_color);

	color = (rgb_color->red << 16) | (rgb_color->green << 8) | rgb_color->blue;
	return color;
}


/*
 * gln_graphics_clear_and_border()
 *
 * Clear the graphics window, and border and shade the area where the
 * picture is going to be rendered.  This attempts a small raised effect
 * for the picture, in keeping with modern trends.
 */
static void gln_graphics_clear_and_border(winid_t glk_window, int x_offset, int y_offset,
		int pixel_size, gln_uint16 width, gln_uint16 height) {
	uint background;
	glui32 fade_color, shading_color;
	gln_rgb_t rgb_background, rgb_border, rgb_fade;
	int index;
	assert(glk_window);

	/*
	 * Try to detect the background color of the main window, by getting the
	 * background for Normal style (Glk offers no way to directly get a window's
	 * background color).  If we can get it, we'll match the graphics window
	 * background to it.  If we can't, we'll default the color to white.
	 */
	if (!g_vm->glk_style_measure(gln_main_window,
	                             style_Normal, stylehint_BackColor, &background)) {
		/*
		 * Unable to get the main window background, so assume, and default
		 * graphics to white.
		 */
		background = GLN_GRAPHICS_DEFAULT_BACKGROUND;
	}

	/*
	 * Set the graphics window background to match the main window background,
	 * as best as we can tell, and clear the window.
	 */
	g_vm->glk_window_set_background_color(glk_window, background);
	g_vm->glk_window_clear(glk_window);
#ifndef GARGLK
	/*
	 * For very small pictures, just border them, but don't try and
	 * do any shading.  Failing this check is probably highly unlikely.
	 */
	if (width < 2 * GLN_GRAPHICS_SHADE_STEPS
			|| height < 2 * GLN_GRAPHICS_SHADE_STEPS) {
		/* Paint a rectangle bigger than the picture by border pixels. */
		g_vm->glk_window_fill_rect(glk_window,
		                           GLN_GRAPHICS_BORDER_COLOR,
		                           x_offset - GLN_GRAPHICS_BORDER,
		                           y_offset - GLN_GRAPHICS_BORDER,
		                           width * pixel_size + GLN_GRAPHICS_BORDER * 2,
		                           height * pixel_size + GLN_GRAPHICS_BORDER * 2);
		return;
	}
#endif
	/*
	 * Paint a rectangle bigger than the picture by border pixels all round,
	 * and with additional shading pixels right and below.  Some of these
	 * shading pixels are later overwritten by the fading loop below.  The
	 * picture will sit over this rectangle.
	 */
	g_vm->glk_window_fill_rect(glk_window,
	                           GLN_GRAPHICS_BORDER_COLOR,
	                           x_offset - GLN_GRAPHICS_BORDER,
	                           y_offset - GLN_GRAPHICS_BORDER,
	                           width * pixel_size + GLN_GRAPHICS_BORDER * 2
	                           + GLN_GRAPHICS_SHADING,
	                           height * pixel_size + GLN_GRAPHICS_BORDER * 2
	                           + GLN_GRAPHICS_SHADING);

	/*
	 * Split the main window background color and the border color into
	 * components.
	 */
	gln_graphics_split_color(background, &rgb_background);
	gln_graphics_split_color(GLN_GRAPHICS_BORDER_COLOR, &rgb_border);

	/*
	 * Generate the incremental color to use in fade steps.  Here we're
	 * assuming that the border is always darker than the main window
	 * background (currently valid, as we're using black).
	 */
	rgb_fade.red = (rgb_background.red - rgb_border.red)
	               / GLN_GRAPHICS_SHADE_STEPS;
	rgb_fade.green = (rgb_background.green - rgb_border.green)
	                 / GLN_GRAPHICS_SHADE_STEPS;
	rgb_fade.blue = (rgb_background.blue - rgb_border.blue)
	                / GLN_GRAPHICS_SHADE_STEPS;

	/* Combine RGB fade into a single incremental Glk color. */
	fade_color = gln_graphics_combine_color(&rgb_fade);

	/* Fade in edge, from background to border, shading in stages. */
	shading_color = background;
	for (index = 0; index < GLN_GRAPHICS_SHADE_STEPS; index++) {
		/* Shade the two border areas with this color. */
		g_vm->glk_window_fill_rect(glk_window, shading_color,
		                           x_offset + width * pixel_size
		                           + GLN_GRAPHICS_BORDER,
		                           y_offset + index - GLN_GRAPHICS_BORDER,
		                           GLN_GRAPHICS_SHADING, 1);
		g_vm->glk_window_fill_rect(glk_window, shading_color,
		                           x_offset + index - GLN_GRAPHICS_BORDER,
		                           y_offset + height * pixel_size
		                           + GLN_GRAPHICS_BORDER,
		                           1, GLN_GRAPHICS_SHADING);

		/* Update the shading color for the fade next iteration. */
		shading_color -= fade_color;
	}
}


/*
 * gln_graphics_convert_palette()
 *
 * Convert a Level 9 bitmap color palette to a Glk one.
 */
static void gln_graphics_convert_palette(Colour ln_palette[], glui32 glk_palette[]) {
	int index;
	assert(ln_palette && glk_palette);

	for (index = 0; index < GLN_PALETTE_SIZE; index++) {
		Colour colour;
		gln_rgb_t gln_color;

		/* Convert color from Level 9 to internal RGB, then to Glk color. */
		colour = ln_palette[index];
		gln_color.red   = colour.red;
		gln_color.green = colour.green;
		gln_color.blue  = colour.blue;
		glk_palette[index] = gln_graphics_combine_color(&gln_color);
	}
}

#ifdef GFX_SCALE_BY_FACTOR
/*
 * gln_graphics_position_picture()
 *
 * Given a picture width and height, return the x and y offsets to center
 * this picture in the current graphics window.
 */
static void gln_graphics_position_picture(winid_t glk_window, int pixel_size,
        gln_uint16 width, gln_uint16 height, int *x_offset, int *y_offset) {
	uint window_width, window_height;
	assert(glk_window && x_offset && y_offset);

	/* Measure the current graphics window dimensions. */
	g_vm->glk_window_get_size(glk_window, &window_width, &window_height);

	/*
	 * Calculate and return an x and y offset to use on point plotting, so that
	 * the image centers inside the graphical window.
	 */
	*x_offset = ((int) window_width - width * pixel_size) / 2;
	*y_offset = ((int) window_height - height * pixel_size) / 2;
}
#endif

/*
 * gms_graphics_compare_layering_inverted()
 * gln_graphics_assign_layers()
 *
 * Given two sets of image bitmaps, and a palette, this function will
 * assign layers palette colors.
 *
 * Layers are assigned by first counting the number of vertices in the
 * color plane, to get a measure of the complexity of shapes displayed in
 * this color, and also the raw number of times each palette color is
 * used.  This is then sorted, so that layers are assigned to colors, with
 * the lowest layer being the color with the most complex shapes, and
 * within this (or where the count of vertices is zero) the most used color.
 *
 * The function compares pixels in the two image bitmaps given, these
 * being the off-screen and on-screen buffers, and generates counts only
 * where these bitmaps differ.  This ensures that only pixels not yet
 * painted are included in layering.
 *
 * As well as assigning layers, this function returns a set of layer usage
 * flags, to help the rendering loop to terminate as early as possible.
 *
 * By painting lower layers first, the paint can take in larger areas if
 * it's permitted to include not-yet-validated higher levels.  This helps
 * minimize the amount of Glk areas fills needed to render a picture.
 */
struct gln_layering_t {
	long complexity;  /* Count of vertices for this color. */
	long usage;       /* Color usage count. */
	int color;        /* Color index into palette. */
};

#ifndef GARGLK

/*
 * gln_graphics_is_vertex()
 *
 * Given a point, return TRUE if that point is the vertex of a fillable
 * region.  This is a helper function for layering pictures.  When assign-
 * ing layers, we want to weight the colors that have the most complex
 * shapes, or the largest count of isolated areas, heavier than simpler
 * areas.
 *
 * By painting the colors with the largest number of isolated areas or
 * the most complex shapes first, we help to minimize the number of fill
 * regions needed to render the complete picture.
 */
static int gln_graphics_is_vertex(gln_byte off_screen[], gln_uint16 width, gln_uint16 height,
	int x, int y) {
	gln_byte pixel;
	int above, below, left, right;
	long index_row;
	assert(off_screen);

	/* Use an index row to cut down on multiplications. */
	index_row = y * width;

	/* Find the color of the reference pixel. */
	pixel = off_screen[index_row + x];
	assert(pixel < GLN_PALETTE_SIZE);

	/*
	 * Detect differences between the reference pixel and its upper, lower, left
	 * and right neighbors.  Mark as different if the neighbor doesn't exist,
	 * that is, at the edge of the picture.
	 */
	above = (y == 0 || off_screen[index_row - width + x] != pixel);
	below = (y == height - 1 || off_screen[index_row + width + x] != pixel);
	left = (x == 0 || off_screen[index_row + x - 1] != pixel);
	right = (x == width - 1 || off_screen[index_row + x + 1] != pixel);

	/*
	 * Return TRUE if this pixel lies at the vertex of a rectangular, fillable,
	 * area.  That is, if two adjacent neighbors aren't the same color (or if
	 * absent -- at the edge of the picture).
	 */
	return ((above || below) && (left || right));
}

static int gln_graphics_compare_layering_inverted(const void *void_first,
        const void *void_second) {
	const gln_layering_t *first = (const gln_layering_t *)void_first;
	const gln_layering_t *second = (const gln_layering_t *)void_second;

	/*
	 * Order by complexity first, then by usage, putting largest first.  Some
	 * colors may have no vertices at all when doing animation frames, but
	 * rendering optimization relies on the first layer that contains no areas
	 * to fill halting the rendering loop.  So it's important here that we order
	 * indexes so that colors that render complex shapes come first, non-empty,
	 * but simpler shaped colors next, and finally all genuinely empty layers.
	 */
	return second->complexity > first->complexity ? 1 :
	       first->complexity > second->complexity ? -1 :
	       second->usage > first->usage ? 1 :
	       first->usage > second->usage ? -1 : 0;
}

static void gln_graphics_assign_layers(gln_byte off_screen[], gln_byte on_screen[],
		gln_uint16 width, gln_uint16 height, int layers[], long layer_usage[]) {
	int index, x, y;
	long index_row;
	gln_layering_t layering[GLN_PALETTE_SIZE];
	assert(off_screen && on_screen && layers && layer_usage);

	/* Clear initial complexity and usage counts, and set initial colors. */
	for (index = 0; index < GLN_PALETTE_SIZE; index++) {
		layering[index].complexity = 0;
		layering[index].usage = 0;
		layering[index].color = index;
	}

	/*
	 * Traverse the image, counting vertices and pixel usage where the pixels
	 * differ between the off-screen and on-screen buffers.  Optimize by
	 * maintaining an index row to avoid multiplications.
	 */
	for (y = 0, index_row = 0; y < height; y++, index_row += width) {
		for (x = 0; x < width; x++) {
			long idx;

			/*
			 * Get the idx for this pixel, and update complexity and usage
			 * if off-screen and on-screen pixels differ.
			 */
			idx = index_row + x;
			if (on_screen[idx] != off_screen[idx]) {
				if (gln_graphics_is_vertex(off_screen, width, height, x, y))
					layering[off_screen[idx]].complexity++;

				layering[off_screen[idx]].usage++;
			}
		}
	}

	/*
	 * Sort counts to form color indexes.  The primary sort is on the shape
	 * complexity, and within this, on color usage.
	 */
	qsort(layering, GLN_PALETTE_SIZE,
	      sizeof(*layering), gln_graphics_compare_layering_inverted);

	/*
	 * Assign a layer to each palette color, and also return the layer usage
	 * for each layer.
	 */
	for (index = 0; index < GLN_PALETTE_SIZE; index++) {
		layers[layering[index].color] = index;
		layer_usage[index] = layering[index].usage;
	}
}

/*
 * gln_graphics_paint_region()
 *
 * This is a partially optimized point plot.  Given a point in the graphics
 * bitmap, it tries to extend the point to a color region, and fill a number
 * of pixels in a single Glk rectangle fill.  The goal here is to reduce the
 * number of Glk rectangle fills, which tend to be extremely inefficient
 * operations for generalized point plotting.
 *
 * The extension works in image layers; each palette color is assigned* a
 * layer, and we paint each layer individually, starting at the lowest.  So,
 * the region is free to fill any invalidated pixel in a higher layer, and
 * all pixels, invalidated or already validated, in the same layer.  In
 * practice, it is good enough to look for either invalidated pixels or pixels
 * in the same layer, and construct a region as large as possible from these,
 * then on marking points as validated, mark only those in the same layer as
 * the initial point.
 *
 * The optimization here is not the best possible, but is reasonable.  What
 * we do is to try and stretch the region horizontally first, then vertically.
 * In practice, we might find larger areas by stretching vertically and then
 * horizontally, or by stretching both dimensions at the same time.  In
 * mitigation, the number of colors in a picture is small (16), and the
 * aspect ratio of pictures makes them generally wider than they are tall.
 *
 * Once we've found the region, we render it with a single Glk rectangle fill,
 * and mark all the pixels in this region that match the layer of the initial
 * given point as validated.
 */
static void gln_graphics_paint_region(winid_t glk_window, glui32 palette[], int layers[],
		gln_byte off_screen[], gln_byte on_screen[], int x, int y, int x_offset, int y_offset,
		int pixel_size, gln_uint16 width, gln_uint16 height) {
	gln_byte pixel;
	int layer, x_min, x_max, y_min, y_max, x_index, y_index;
	long index_row;
	assert(glk_window && palette && layers && off_screen && on_screen);

	/* Find the color and layer for the initial pixel. */
	pixel = off_screen[y * width + x];
	layer = layers[pixel];
	assert(pixel < GLN_PALETTE_SIZE);

	/*
	 * Start by finding the extent to which we can pull the x coordinate and
	 * still find either invalidated pixels, or pixels in this layer.
	 *
	 * Use an index row to remove multiplications from the loops.
	 */
	index_row = y * width;
	for (x_min = x; x_min - 1 >= 0; x_min--) {
		long index = index_row + x_min - 1;

		if (on_screen[index] == off_screen[index]
		        && layers[off_screen[index]] != layer)
			break;
	}
	for (x_max = x; x_max + 1 < width; x_max++) {
		long index = index_row + x_max + 1;

		if (on_screen[index] == off_screen[index]
		        && layers[off_screen[index]] != layer)
			break;
	}

	/*
	 * Now try to stretch the height of the region, by extending the y
	 * coordinate as much as possible too.  Again, we're looking for pixels
	 * that are invalidated or ones in the same layer.  We need to check
	 * across the full width of the current region.
	 *
	 * As above, an index row removes multiplications from the loops.
	 */
	for (y_min = y, index_row = (y - 1) * width;
	        y_min - 1 >= 0; y_min--, index_row -= width) {
		for (x_index = x_min; x_index <= x_max; x_index++) {
			long index = index_row + x_index;

			if (on_screen[index] == off_screen[index]
			        && layers[off_screen[index]] != layer)
				goto break_y_min;
		}
	}
break_y_min:

	for (y_max = y, index_row = (y + 1) * width;
	        y_max + 1 < height; y_max++, index_row += width) {
		for (x_index = x_min; x_index <= x_max; x_index++) {
			long index = index_row + x_index;

			if (on_screen[index] == off_screen[index]
			        && layers[off_screen[index]] != layer)
				goto break_y_max;
		}
	}
break_y_max:

	/* Fill the region using Glk's rectangle fill. */
	g_vm->glk_window_fill_rect(glk_window, palette[pixel],
	                           x_min * pixel_size + x_offset,
	                           y_min * pixel_size + y_offset,
	                           (x_max - x_min + 1) * pixel_size,
	                           (y_max - y_min + 1) * pixel_size);

	/*
	 * Validate each pixel in the reference layer that was rendered by the
	 * rectangle fill.  We don't validate pixels that are not in this layer
	 * (and are by definition in higher layers, as we've validated all lower
	 * layers), since although we colored them, we did it for optimization
	 * reasons, and they're not yet colored correctly.
	 *
	 * Maintain an index row as an optimization to avoid multiplication.
	 */
	index_row = y_min * width;
	for (y_index = y_min; y_index <= y_max; y_index++) {
		for (x_index = x_min; x_index <= x_max; x_index++) {
			long index;

			/*
			 * Get the index for x_index,y_index.  If the layers match, update
			 * the on-screen buffer.
			 */
			index = index_row + x_index;
			if (layers[off_screen[index]] == layer) {
				assert(off_screen[index] == pixel);
				on_screen[index] = off_screen[index];
			}
		}

		/* Update row index component on change of y. */
		index_row += width;
	}
}
#endif

static void gln_graphics_paint_everything(winid_t glk_window, Colour palette[],
		gln_byte off_screen[], int x_offset, int y_offset, gln_uint16 width, gln_uint16 height) {
	Graphics::PixelFormat format(4, 8, 8, 8, 8, 24, 16, 8, 0);
	Graphics::ManagedSurface s(width, height, format);

	for (int y = 0; y < height; ++y) {
		uint32 *lineP = (uint32 *)s.getBasePtr(0, y);
		for (int x = 0; x < width; ++x, ++lineP) {
			byte pixel = off_screen[y * width + x];
			assert(pixel < GLN_PALETTE_SIZE);
			const Colour &col = palette[pixel];

			*lineP = format.RGBToColor(col.red, col.green, col.blue);
		}
	}

	#ifdef GFX_SCALE_BY_FACTOR
	g_vm->glk_image_draw_scaled(glk_window, s, (uint)-1, x_offset, y_offset,
		width * GLN_GRAPHICS_PIXEL, height * GLN_GRAPHICS_PIXEL);
	#else
	uint winWidth, winHeight;
	g_vm->glk_window_get_size(glk_window, &winWidth, &winHeight);
	g_vm->glk_image_draw_scaled(glk_window, s, (uint)-1, 0, 0, winWidth, winHeight);
	#endif
}

/*
 * gln_graphics_timeout()
 *
 * This is a background function, called on Glk timeouts.  Its job is to
 * repaint some of the current graphics image.  On successive calls, it
 * does a part of the repaint, then yields to other processing.  This is
 * useful since the Glk primitive to plot points in graphical windows is
 * extremely slow; this way, the repaint doesn't block game play.
 *
 * The function should be called on Glk timeout events.  When the repaint
 * is complete, the function will turn off Glk timers.
 *
 * The function uses double-buffering to track how much of the graphics
 * buffer has been rendered.  This helps to minimize the amount of point
 * plots required, as only the differences between the two buffers need
 * to be rendered.
 */
static void gln_graphics_timeout() {
	static glui32 palette[GLN_PALETTE_SIZE];   /* Precomputed Glk palette */

	static int deferred_repaint = FALSE;       /* Local delayed repaint flag */
	static int ignore_counter;                 /* Count of calls ignored */
	static int x_offset, y_offset;             /* Point plot offsets */

#ifndef GARGLK
	static int yield_counter;                  /* Yields in rendering */
	static int saved_layer;                    /* Saved current layer */
	static int saved_x, saved_y;               /* Saved x,y coord */

	static int total_regions;                  /* Debug statistic */
#endif
	gln_byte *on_screen;                       /* On-screen image buffer */
	gln_byte *off_screen;                      /* Off-screen image buffer */
	long picture_size;                         /* Picture size in pixels */

	/* Ignore the call if the current graphics state is inactive. */
	if (!gln_graphics_active)
		return;
	assert(gln_graphics_window);

	/*
	 * On detecting a repaint request, note the flag in a local static variable,
	 * then set up a graphics delay to wait until, hopefully, the resize, if
	 * that's what caused it, is complete, and return.  This makes resizing the
	 * window a lot smoother, since it prevents unnecessary region paints where
	 * we are receiving consecutive Glk arrange or redraw events.
	 */
	if (gln_graphics_repaint) {
		deferred_repaint = TRUE;
		gln_graphics_repaint = FALSE;
		ignore_counter = GLN_GRAPHICS_REPAINT_WAIT - 1;
		return;
	}

	/*
	 * If asked to ignore a given number of calls, decrement the ignore counter
	 * and return having done nothing more.  This lets us delay graphics
	 * operations by a number of timeouts, providing partial protection from
	 * resize event "storms".
	 *
	 * Note -- to wait for N timeouts, set the count of timeouts to be ignored
	 * to N-1.
	 */
	assert(ignore_counter >= 0);
	if (ignore_counter > 0) {
		ignore_counter--;
		return;
	}

	/* Calculate the picture size, and synchronize screen buffer pointers. */
	picture_size = gln_graphics_width * gln_graphics_height;
	off_screen = gln_graphics_off_screen;
	on_screen = gln_graphics_on_screen;

	/*
	 * If we received a new picture, set up the local static variables for that
	 * picture -- convert the color palette, and initialize the off_screen
	 * buffer to be the base picture.
	 */
	if (gln_graphics_new_picture) {
		/* Initialize the off_screen buffer to be a copy of the base picture. */
		free(off_screen);
		off_screen = (gln_byte *)gln_malloc(picture_size * sizeof(*off_screen));
		memcpy(off_screen, gln_graphics_bitmap,
		       picture_size * sizeof(*off_screen));

		/* Note the buffer for freeing on cleanup. */
		gln_graphics_off_screen = off_screen;

		/*
		 * Pre-convert all the picture palette colors into their corresponding
		 * Glk colors.
		 */
		gln_graphics_convert_palette(gln_graphics_palette, palette);

		/* Save the color count for possible queries later. */
		gln_graphics_color_count =
		    gln_graphics_count_colors(off_screen,
		                              gln_graphics_width, gln_graphics_height);
	}

	/*
	 * For a new picture, or a repaint of a prior one, calculate new values for
	 * the x and y offsets used to draw image points, and set the on-screen
	 * buffer to an unused pixel value, in effect invalidating all on-screen
	 * data.  Also, reset the saved image scan coordinates so that we scan for
	 * unpainted pixels from top left starting at layer zero, and clear the
	 * graphics window.
	 */
	if (gln_graphics_new_picture || deferred_repaint) {
		#ifdef GFX_SCALE_BY_FACTOR
		/*
		 * Calculate the x and y offset to center the picture in the graphics
		 * window.
		 */
		gln_graphics_position_picture(gln_graphics_window,
		                              GLN_GRAPHICS_PIXEL,
		                              gln_graphics_width, gln_graphics_height,
		                              &x_offset, &y_offset);
		#else
		x_offset = y_offset = 0;
		#endif

		/*
		 * Reset all on-screen pixels to an unused value, guaranteed not to
		 * match any in a real picture.  This forces all pixels to be repainted
		 * on a buffer/on-screen comparison.
		 */
		free(on_screen);
		on_screen = (gln_byte *)gln_malloc(picture_size * sizeof(*on_screen));
		memset(on_screen, GLN_GRAPHICS_UNUSED_PIXEL,
		       picture_size * sizeof(*on_screen));

		/* Note the buffer for freeing on cleanup. */
		gln_graphics_on_screen = on_screen;

		/*
		 * Assign new layers to the current image.  This sorts colors by usage
		 * and puts the most used colors in the lower layers.  It also hands us
		 * a count of pixels in each layer, useful for knowing when to stop
		 * scanning for layers in the rendering loop.
		 */
#ifndef GARGLK
		gln_graphics_assign_layers(off_screen, on_screen,
		                           gln_graphics_width, gln_graphics_height,
		                           layers, layer_usage);
#endif

		/* Clear the graphics window. */
		gln_graphics_clear_and_border(gln_graphics_window,
		                              x_offset, y_offset,
		#ifdef GFX_SCALE_BY_FACTOR
		                              GLN_GRAPHICS_PIXEL,
		#else
										1,
		#endif
		                              gln_graphics_width, gln_graphics_height);
#ifndef GARGLK
		/* Start a fresh picture rendering pass. */
		yield_counter = 0;
		saved_layer = 0;
		saved_x = 0;
		saved_y = 0;
		total_regions = 0;
#endif

		/* Clear the new picture and deferred repaint flags. */
		gln_graphics_new_picture = FALSE;
		deferred_repaint = FALSE;
	}

#ifndef GARGLK
	int layer;                                 /* Image layer iterator */
	int x, y;                                  /* Image iterators */
	int regions;                               /* Count of regions painted */
	static int layers[GLN_PALETTE_SIZE];       /* Assigned image layers */
	static long layer_usage[GLN_PALETTE_SIZE]; /* Image layer occupancies */

	/*
	 * Make a portion of an image pass, from lower to higher image layers,
	 * scanning for invalidated pixels that are in the current image layer we
	 * are painting.  Each invalidated pixel gives rise to a region paint,
	 * which equates to one Glk rectangle fill.
	 *
	 * When the limit on regions is reached, save the current image pass layer
	 * and coordinates, and yield control to the main game playing code by
	 * returning.  On the next call, pick up where we left off.
	 *
	 * As an optimization, we can leave the loop on the first empty layer we
	 * encounter.  Since layers are ordered by complexity and color usage, all
	 * layers higher than the first unused one will also be empty, so we don't
	 * need to scan them.
	 */
	regions = 0;
	for (layer = saved_layer;
	        layer < GLN_PALETTE_SIZE && layer_usage[layer] > 0; layer++) {
		long index_row;

		/*
		 * As an optimization to avoid multiplications in the loop, maintain a
		 * separate index row.
		 */
		index_row = saved_y * gln_graphics_width;
		for (y = saved_y; y < gln_graphics_height; y++) {
			for (x = saved_x; x < gln_graphics_width; x++) {
				long index;

				/* Get the index for this pixel. */
				index = index_row + x;
				assert(index < picture_size * sizeof(*off_screen));

				/*
				 * Ignore pixels not in the current layer, and pixels not
				 * currently invalid (that is, ones whose on-screen represen-
				 * tation matches the off-screen buffer).
				 */
				if (layers[off_screen[index]] == layer
				        && on_screen[index] != off_screen[index]) {
					/*
					 * Rather than painting just one pixel, here we try to
					 * paint the maximal region we can for the layer of the
					 * given pixel.
					 */
					gln_graphics_paint_region(gln_graphics_window,
					                          palette, layers,
					                          off_screen, on_screen,
					                          x, y, x_offset, y_offset,
					                          GLN_GRAPHICS_PIXEL,
					                          gln_graphics_width,
					                          gln_graphics_height);

					/*
					 * Increment count of regions handled, and yield, by
					 * returning, if the limit on paint regions is reached.
					 * Before returning, save the current layer and scan
					 * coordinates, so we can pick up here on the next call.
					 */
					regions++;
					if (regions >= GLN_REPAINT_LIMIT) {
						yield_counter++;
						saved_layer = layer;
						saved_x = x;
						saved_y = y;
						total_regions += regions;
						return;
					}
				}
			}

			/* Reset the saved x coordinate on y increment. */
			saved_x = 0;

			/* Update the index row on change of y. */
			index_row += gln_graphics_width;
		}

		/* Reset the saved y coordinate on layer change. */
		saved_y = 0;
	}

	/*
	 * If we reach this point, then we didn't get to the limit on regions
	 * painted on this pass.  In that case, we've finished rendering the
	 * image.
	 */
	assert(regions < GLN_REPAINT_LIMIT);
	total_regions += regions;

#else
	gln_graphics_paint_everything(gln_graphics_window, gln_graphics_palette, off_screen,
		x_offset, y_offset, gln_graphics_width, gln_graphics_height);
#endif

	/* Stop graphics; there's no more to be done until something restarts us. */
	gln_graphics_stop();
}

/*
 * gln_graphics_locate_bitmaps()
 *
 * Given the name of the game file being run, try to set up the graphics
 * directory and bitmap type for that game.  If none available, set the
 * directory to NULL, and bitmap type to NO_BITMAPS.
 */
static void gln_graphics_locate_bitmaps(const char *gamefile) {
	const char *basename;
	char *dirname;
	BitmapType bitmap_type;

	/* Find the start of the last element of the filename passed in. */
	basename = gamefile;

	/* Take a copy of the directory part of the filename. */
	dirname = (char *)gln_malloc(basename - gamefile + 1);
	strncpy(dirname, gamefile, basename - gamefile);
	dirname[basename - gamefile] = '\0';

	/*
	 * Use the core interpreter to search for suitable bitmaps.  If none found,
	 * free allocated memory and return noting none available.
	 */
	bitmap_type = DetectBitmaps(dirname);
	if (bitmap_type == NO_BITMAPS) {
		free(dirname);
		gln_graphics_bitmap_directory = NULL;
		gln_graphics_bitmap_type = NO_BITMAPS;
		return;
	}

	/* Record the bitmap details for later use. */
	gln_graphics_bitmap_directory = dirname;
	gln_graphics_bitmap_type = bitmap_type;
}


/*
 * gln_graphics_handle_title_picture()
 *
 * Picture 0 is special, normally the title picture.  Unless we handle it
 * specially, the next picture comes along and instantly overwrites it.
 * Here, then, we try to delay until the picture has rendered, allowing the
 * delay to be broken with a keypress.
 */
static void gln_graphics_handle_title_picture() {
	event_t event;
	int count;

	gln_standout_string("\n[ Press any key to skip the title picture... ]\n\n");

	/* Wait until a keypress or graphics rendering is complete. */
	g_vm->glk_request_char_event(gln_main_window);
	do {
		gln_event_wait_2(evtype_CharInput, evtype_Timer, &event);

		/*
		 * If a character was pressed, return.  This will let the game
		 * progress, probably into showing the next bitmap.
		 */
		if (event.type == evtype_CharInput) {
			gln_watchdog_tick();
			return;
		}
	} while (gln_graphics_active);

	/*
	 * Now wait another couple of seconds, or until a keypress.  We'll do this
	 * in graphics timeout chunks, so that if graphics restarts while we're
	 * delaying, and it requests timer events and overwrites ours, we wind up
	 * with the identical timer event period to the one we're expecting anyway.
	 */
	g_vm->glk_request_timer_events(GLN_GRAPHICS_TIMEOUT);
	for (count = 0; count < GLN_GRAPHICS_TITLE_WAIT; count++) {
		gln_event_wait_2(evtype_CharInput, evtype_Timer, &event);

		if (event.type == evtype_CharInput)
			break;
	}

	/*
	 * While we waited, a Glk arrange or redraw event could have triggered
	 * graphics into repainting, and using timers.  To handle this, stop timers
	 * only if graphics is inactive.  If active, graphics will stop timers
	 * itself when it finishes rendering.  We can't stop timers here while
	 * graphics is active; that will hang the graphics "thread".
	 */
	if (!gln_graphics_active)
		g_vm->glk_request_timer_events(0);

	/* Cancel possible pending character event, and continue on. */
	g_vm->glk_cancel_char_event(gln_main_window);
	gln_watchdog_tick();
}


/*
 * os_show_bitmap()
 *
 * Called by the main interpreter when it wants us to display a picture.
 *
 * The function gets the picture bitmap, palette, and dimensions, and saves
 * them, and the picture id, in module variables for the background rendering
 * function.
 */
void os_show_bitmap(int picture, int x, int y) {
	Bitmap *bitmap;
	long picture_bytes;

	/*
	 * If interpreter graphics are disabled, the only way we can get into here
	 * is using #picture.  It seems that the interpreter won't always deliver
	 * correct bitmaps with #picture when in text mode, so it's simplest here
	 * if we just ignore those calls.
	 */
	if (gln_graphics_interpreter_state != GLN_GRAPHICS_BITMAP_MODE)
		return;

	/* Ignore repeat calls for the currently displayed picture. */
	if (picture == gln_graphics_picture)
		return;

	/*
	 * Get the core interpreter's bitmap for the requested picture.  If this
	 * returns NULL, the picture doesn't exist, so ignore the call silently.
	 */
	bitmap = DecodeBitmap(gln_graphics_bitmap_directory,
	                      gln_graphics_bitmap_type, picture, x, y);
	if (!bitmap)
		return;

	/*
	 * Note the last thing passed to os_show_bitmap, to avoid possible repaints
	 * of the current picture.
	 */
	gln_graphics_picture = picture;

	/* Calculate the picture size in bytes. */
	picture_bytes = bitmap->width * bitmap->height * sizeof(*bitmap->bitmap);

	/*
	 * Save the picture details for the update code.  Here we take a complete
	 * local copy of the bitmap, dimensions, and palette.  The core interpreter
	 * may return a palette with fewer colors than our maximum, so unused local
	 * palette entries are set to zero.
	 */
	free(gln_graphics_bitmap);
	gln_graphics_bitmap = (gln_byte *)gln_malloc(picture_bytes);
	memcpy(gln_graphics_bitmap, bitmap->bitmap, picture_bytes);
	gln_graphics_width = bitmap->width;
	gln_graphics_height = bitmap->height;
	memset(gln_graphics_palette, 0, sizeof(gln_graphics_palette));
	memcpy(gln_graphics_palette, bitmap->palette,
	       bitmap->npalette * sizeof(bitmap->palette[0]));

	/*
	 * If graphics are enabled, both at the Glk level and in the core
	 * interpreter, ensure the window is displayed, set the appropriate flags,
	 * and start graphics update.  If they're not enabled, the picture details
	 * will simply stick around in module variables until they are required.
	 */
	if (gln_graphics_enabled
	        && gln_graphics_interpreter_state == GLN_GRAPHICS_BITMAP_MODE) {
		/*
		 * Ensure graphics on, then set the new picture flag and start the
		 * updating "thread".  If this is the title picture, start special
		 * handling.
		 */
		if (gln_graphics_open()) {
			gln_graphics_new_picture = TRUE;
			gln_graphics_start();

			if (picture == GLN_GRAPHICS_TITLE_PICTURE)
				gln_graphics_handle_title_picture();
		}
	}
}


/*
 * gln_graphics_picture_is_available()
 *
 * Return TRUE if the graphics module data is loaded with a usable picture,
 * FALSE if there is no picture available to display.
 */
static int gln_graphics_picture_is_available() {
	return gln_graphics_bitmap != nullptr;
}


/*
 * gln_graphics_get_picture_details()
 *
 * Return the width and height of the currently loaded picture.  The function
 * returns FALSE if no picture is loaded, otherwise TRUE, with picture details
 * in the return arguments.
 */
static int gln_graphics_get_picture_details(int *width, int *height) {
	if (gln_graphics_picture_is_available()) {
		if (width)
			*width = gln_graphics_width;
		if (height)
			*height = gln_graphics_height;

		return TRUE;
	}

	return FALSE;
}


/*
 * gln_graphics_get_rendering_details()
 *
 * Returns the type of bitmap in use (if any), as a string, the count of
 * colors in the picture, and a flag indicating if graphics is active (busy).
 * The function return FALSE if graphics is not enabled or if not being
 * displayed, otherwise TRUE with the bitmap type, color count, and active
 * flag in the return arguments.
 *
 * This function races with the graphics timeout, as it returns information
 * set up by the first timeout following a new picture.  There's a very
 * very small chance that it might win the race, in which case out-of-date
 * values are returned.
 */
static int gln_graphics_get_rendering_details(const char **bitmap_type,
		int *color_count, int *is_active) {
	if (gln_graphics_enabled && gln_graphics_are_displayed()) {
		/*
		 * Convert the detected bitmap type into a string and return it.
		 * A nullptr bitmap string implies no bitmaps.
		 */
		if (bitmap_type) {
			const char *return_type;

			switch (gln_graphics_bitmap_type) {
			case AMIGA_BITMAPS:
				return_type = "Amiga";
				break;
			case PC1_BITMAPS:
				return_type = "IBM PC(1)";
				break;
			case PC2_BITMAPS:
				return_type = "IBM PC(2)";
				break;
			case C64_BITMAPS:
				return_type = "Commodore 64";
				break;
			case BBC_BITMAPS:
				return_type = "BBC B";
				break;
			case CPC_BITMAPS:
				return_type = "Amstrad CPC/Spectrum";
				break;
			case MAC_BITMAPS:
				return_type = "Macintosh";
				break;
			case ST1_BITMAPS:
				return_type = "Atari ST(1)";
				break;
			case ST2_BITMAPS:
				return_type = "Atari ST(2)";
				break;
			case NO_BITMAPS:
			default:
				return_type = nullptr;
				break;
			}

			*bitmap_type = return_type;
		}

		/*
		 * Return the color count noted by timeouts on the first timeout
		 * following a new picture.  We might return the one for the prior
		 * picture.
		 */
		if (color_count)
			*color_count = gln_graphics_color_count;

		/* Return graphics active flag. */
		if (is_active)
			*is_active = gln_graphics_active;

		return TRUE;
	}

	return FALSE;
}


/*
 * gln_graphics_interpreter_enabled()
 *
 * Return TRUE if it looks like interpreter graphics are turned on, FALSE
 * otherwise.
 */
static int gln_graphics_interpreter_enabled() {
	return gln_graphics_interpreter_state != GLN_GRAPHICS_OFF;
}


/*
 * gln_graphics_cleanup()
 *
 * Free memory resources allocated by graphics functions.  Called on game
 * end.
 */
static void gln_graphics_cleanup() {
	free(gln_graphics_bitmap);
	gln_graphics_bitmap = nullptr;
	free(gln_graphics_off_screen);
	gln_graphics_off_screen = nullptr;
	free(gln_graphics_on_screen);
	gln_graphics_on_screen = nullptr;
	free(gln_graphics_bitmap_directory);
	gln_graphics_bitmap_directory = nullptr;

	gln_graphics_bitmap_type = NO_BITMAPS;
	gln_graphics_picture = -1;
}


/*---------------------------------------------------------------------*/
/*  Glk port line drawing picture adapter functions                    */
/*---------------------------------------------------------------------*/

/*
 * Graphics color table.  These eight colors are selected into the four-
 * color palette by os_setcolour().  The standard Amiga palette is rather
 * over-vibrant, so to soften it a bit this table uses non-primary colors.
 */
static const gln_rgb_t GLN_LINEGRAPHICS_COLOR_TABLE[] = {
	{ 47,  79,  79},  /* DarkSlateGray  [Black] */
	{238,  44,  44},  /* Firebrick2     [Red] */
	{ 67, 205, 128},  /* SeaGreen3      [Green] */
	{238, 201,   0},  /* Gold2          [Yellow] */
	{ 92, 172, 238},  /* SteelBlue2     [Blue] */
	{139,  87,  66},  /* LightSalmon4   [Brown] */
	{175, 238, 238},  /* PaleTurquoise  [Cyan] */
	{245, 245, 245},  /* WhiteSmoke     [White] */
};

/*
 * Structure of a Seed Fill segment entry, and a growable stack-based array
 * of segments pending fill.  When length exceeds size, size is increased
 * and the array grown.
 */
struct gln_linegraphics_segment_t {
	int y;   /* Segment y coordinate */
	int xl;  /* Segment x left hand side coordinate */
	int xr;  /* Segment x right hand side coordinate */
	int dy;  /* Segment y delta */
};

static gln_linegraphics_segment_t *gln_linegraphics_fill_segments = nullptr;
static int gln_linegraphics_fill_segments_allocation = 0,
           gln_linegraphics_fill_segments_length = 0;


/*
 * gln_linegraphics_create_context()
 *
 * Initialize a new constructed bitmap graphics context for line drawn
 * graphics.
 */
static void gln_linegraphics_create_context() {
	int width, height;
	long picture_bytes;

	/* Get the picture size, and calculate the bytes in the bitmap. */
	GetPictureSize(&width, &height);
	picture_bytes = width * height * sizeof(*gln_graphics_bitmap);

	/*
	 * Destroy any current bitmap, and begin a fresh one.  Here we set the
	 * bitmap and the palette to all zeroes; this equates to all black.
	 */
	free(gln_graphics_bitmap);
	gln_graphics_bitmap = (gln_byte *)gln_malloc(picture_bytes);
	memset(gln_graphics_bitmap, 0, picture_bytes);
	gln_graphics_width = width;
	gln_graphics_height = height;
	memset(gln_graphics_palette, 0, sizeof(gln_graphics_palette));

	/* Set graphics picture number to -1; this is not a real game bitmap. */
	gln_graphics_picture = -1;
}


/*
 * gln_linegraphics_clear_context()
 *
 * Clear the complete graphical drawing area, setting all pixels to zero,
 * and resetting the palette to all black as well.
 */
static void gln_linegraphics_clear_context() {
	long picture_bytes;

	/* Get the picture size, and zero all bytes in the bitmap. */
	picture_bytes = gln_graphics_width
	                * gln_graphics_height * sizeof(*gln_graphics_bitmap);
	memset(gln_graphics_bitmap, 0, picture_bytes);

	/* Clear palette colors to all black. */
	memset(gln_graphics_palette, 0, sizeof(gln_graphics_palette));
}


/*
 * gln_linegraphics_set_palette_color()
 *
 * Copy the indicated main color table entry into the palette.
 */
static void gln_linegraphics_set_palette_color(int colour, int index) {
	const gln_rgb_t *entry;
	assert(colour < GLN_PALETTE_SIZE);
	assert(index < (int)sizeof(GLN_LINEGRAPHICS_COLOR_TABLE)
	       / (int)sizeof(GLN_LINEGRAPHICS_COLOR_TABLE[0]));

	/* Copy the color table entry to the constructed game palette. */
	entry = GLN_LINEGRAPHICS_COLOR_TABLE + index;
	gln_graphics_palette[colour].red   = entry->red;
	gln_graphics_palette[colour].green = entry->green;
	gln_graphics_palette[colour].blue  = entry->blue;
}


/*
 * gln_linegraphics_get_pixel()
 * gln_linegraphics_set_pixel()
 *
 * Return and set the bitmap pixel at x,y.
 */
static gln_byte gln_linegraphics_get_pixel(int x, int y) {
	assert(x >= 0 && x < gln_graphics_width
	       && y >= 0 && y < gln_graphics_height);

	return gln_graphics_bitmap[y * gln_graphics_width + x];
}

static void gln_linegraphics_set_pixel(int x, int y, gln_byte color) {
	assert(x >= 0 && x < gln_graphics_width
	       && y >= 0 && y < gln_graphics_height);

	gln_graphics_bitmap[y * gln_graphics_width + x] = color;
}


/*
 * gln_linegraphics_plot_clip()
 * gln_linegraphics_draw_line_if()
 *
 * Draw a line from x1,y1 to x2,y2 in colour1, where the existing pixel
 * colour is colour2.  The function uses Bresenham's algorithm.  The second
 * function, gln_graphics_plot_clip, is a line drawing helper; it handles
 * clipping, and the requirement to plot a point only if it matches colour2.
 */
static void gln_linegraphics_plot_clip(int x, int y, int colour1, int colour2) {
	/*
	 * Clip the plot if the value is outside the context.  Otherwise, plot the
	 * pixel as colour1 if it is currently colour2.
	 */
	if (x >= 0 && x < gln_graphics_width && y >= 0 && y < gln_graphics_height) {
		if (gln_linegraphics_get_pixel(x, y) == colour2)
			gln_linegraphics_set_pixel(x, y, colour1);
	}
}

static void gln_linegraphics_draw_line_if(int x1, int y1, int x2, int y2,
		int colour1, int colour2) {
	int x, y, dx, dy, incx, incy, balance;

	/* Ignore any odd request where there will be no colour changes. */
	if (colour1 == colour2)
		return;

	/* Normalize the line into deltas and increments. */
	if (x2 >= x1) {
		dx = x2 - x1;
		incx = 1;
	} else {
		dx = x1 - x2;
		incx = -1;
	}

	if (y2 >= y1) {
		dy = y2 - y1;
		incy = 1;
	} else {
		dy = y1 - y2;
		incy = -1;
	}

	/* Start at x1,y1. */
	x = x1;
	y = y1;

	/* Decide on a direction to progress in. */
	if (dx >= dy) {
		dy <<= 1;
		balance = dy - dx;
		dx <<= 1;

		/* Loop until we reach the end point of the line. */
		while (x != x2) {
			gln_linegraphics_plot_clip(x, y, colour1, colour2);
			if (balance >= 0) {
				y += incy;
				balance -= dx;
			}
			balance += dy;
			x += incx;
		}
		gln_linegraphics_plot_clip(x, y, colour1, colour2);
	} else {
		dx <<= 1;
		balance = dx - dy;
		dy <<= 1;

		/* Loop until we reach the end point of the line. */
		while (y != y2) {
			gln_linegraphics_plot_clip(x, y, colour1, colour2);
			if (balance >= 0) {
				x += incx;
				balance -= dy;
			}
			balance += dx;
			y += incy;
		}
		gln_linegraphics_plot_clip(x, y, colour1, colour2);
	}
}


/*
 * gln_linegraphics_push_fill_segment()
 * gln_linegraphics_pop_fill_segment()
 * gln_linegraphics_fill_4way_if()
 *
 * Area fill algorithm, set a region to colour1 if it is currently set to
 * colour2.  This function is a derivation of Paul Heckbert's Seed Fill,
 * from "Graphics Gems", Academic Press, 1990, which fills 4-connected
 * neighbors.
 *
 * The main modification is to make segment stacks growable, through the
 * helper push and pop functions.  There is also a small adaptation to
 * check explicitly for color2, to meet the Level 9 API.
 */
static void gln_linegraphics_push_fill_segment(int y, int xl, int xr, int dy) {
	/* Clip points outside the graphics context. */
	if (!(y + dy < 0 || y + dy >= gln_graphics_height)) {
		int length, allocation;

		length = ++gln_linegraphics_fill_segments_length;
		allocation = gln_linegraphics_fill_segments_allocation;

		/* Grow the segments stack if required, successively doubling. */
		if (length > allocation) {
			size_t bytes;

			allocation = allocation == 0 ? 1 : allocation << 1;

			bytes = allocation * sizeof(*gln_linegraphics_fill_segments);
			gln_linegraphics_fill_segments =
			    (gln_linegraphics_segment_t *)gln_realloc(gln_linegraphics_fill_segments, bytes);
		}

		/* Push top of segments stack. */
		gln_linegraphics_fill_segments[length - 1].y  = y;
		gln_linegraphics_fill_segments[length - 1].xl = xl;
		gln_linegraphics_fill_segments[length - 1].xr = xr;
		gln_linegraphics_fill_segments[length - 1].dy = dy;

		/* Write back local dimensions copies. */
		gln_linegraphics_fill_segments_length = length;
		gln_linegraphics_fill_segments_allocation = allocation;
	}
}

static void gln_linegraphics_pop_fill_segment(int *y, int *xl, int *xr, int *dy) {
	int length;
	assert(gln_linegraphics_fill_segments_length > 0);

	length = --gln_linegraphics_fill_segments_length;

	/* Pop top of segments stack. */
	*y  = gln_linegraphics_fill_segments[length].y;
	*xl = gln_linegraphics_fill_segments[length].xl;
	*xr = gln_linegraphics_fill_segments[length].xr;
	*dy = gln_linegraphics_fill_segments[length].dy;
}

static void gln_linegraphics_fill_4way_if(int x, int y, int colour1, int colour2) {
	/* Ignore any odd request where there will be no colour changes. */
	if (colour1 == colour2)
		return;

	/* Clip fill requests to visible graphics region. */
	if (x >= 0 && x < gln_graphics_width && y >= 0 && y < gln_graphics_height) {
		int left, x1, x2, dy, x_lo, x_hi;

		/*
		 * Level 9 API; explicit check for a match against colour2.  This also
		 * covers the standard Seed Fill check that old pixel value should not
		 * equal colour1, because of the color1 == colour2 comparison above.
		 */
		if (gln_linegraphics_get_pixel(x, y) != colour2)
			return;

		/*
		 * Set up inclusive window dimension to ease algorithm translation.
		 * The original worked with inclusive rectangle limits.
		 */
		x_lo = 0;
		x_hi = gln_graphics_width - 1;

		/*
		 * The first of these is "needed in some cases", the second is the seed
		 * segment, popped first.
		 */
		gln_linegraphics_push_fill_segment(y, x, x, 1);
		gln_linegraphics_push_fill_segment(y + 1, x, x, -1);

		while (gln_linegraphics_fill_segments_length > 0) {
			/* Pop segment off stack and add delta to y coord. */
			gln_linegraphics_pop_fill_segment(&y, &x1, &x2, &dy);
			y += dy;

			/*
			 * Segment of scan line y-dy for x1<=x<=x2 was previously filled,
			 * now explore adjacent pixels in scan line y.
			 */
			for (x = x1;
			        x >= x_lo && gln_linegraphics_get_pixel(x, y) == colour2;
			        x--) {
				gln_linegraphics_set_pixel(x, y, colour1);
			}

			if (x >= x1)
				goto skip;

			left = x + 1;
			if (left < x1) {
				/* Leak on left? */
				gln_linegraphics_push_fill_segment(y, left, x1 - 1, -dy);
			}

			x = x1 + 1;
			do {
				for (;
				        x <= x_hi && gln_linegraphics_get_pixel(x, y) == colour2;
				        x++) {
					gln_linegraphics_set_pixel(x, y, colour1);
				}

				gln_linegraphics_push_fill_segment(y, left, x - 1, dy);

				if (x > x2 + 1) {
					/* Leak on right? */
					gln_linegraphics_push_fill_segment(y, x2 + 1, x - 1, -dy);
				}

skip:
				for (x++;
				        x <= x2 && gln_linegraphics_get_pixel(x, y) != colour2;
				        x++)
					;

				left = x;
			} while (x <= x2);
		}
	}
}


/*
 * os_cleargraphics()
 * os_setcolour()
 * os_drawline()
 * os_fill()
 *
 * Interpreter entry points for line drawing graphics.  All calls to these
 * are ignored if line drawing mode is not set.
 */
void os_cleargraphics() {
	if (gln_graphics_interpreter_state == GLN_GRAPHICS_LINE_MODE)
		gln_linegraphics_clear_context();
}

void os_setcolour(int colour, int index) {
	if (gln_graphics_interpreter_state == GLN_GRAPHICS_LINE_MODE)
		gln_linegraphics_set_palette_color(colour, index);
}

void os_drawline(int x1, int y1, int x2, int y2, int colour1, int colour2) {
	if (gln_graphics_interpreter_state == GLN_GRAPHICS_LINE_MODE)
		gln_linegraphics_draw_line_if(x1, y1, x2, y2, colour1, colour2);
}

void os_fill(int x, int y, int colour1, int colour2) {
	if (gln_graphics_interpreter_state == GLN_GRAPHICS_LINE_MODE)
		gln_linegraphics_fill_4way_if(x, y, colour1, colour2);
}


/*
 * gln_linegraphics_process()
 *
 * Process as many graphics opcodes as are available, constructing the
 * resulting image as a bitmap.  When complete, treat as normal bitmaps.
 */
static void gln_linegraphics_process() {
	/*
	 * If interpreter graphics are not set to line mode, ignore any call that
	 * arrives here.
	 */
	if (gln_graphics_interpreter_state == GLN_GRAPHICS_LINE_MODE) {
		int opcodes_count;

		/* Run all the available graphics opcodes. */
		for (opcodes_count = 0; RunGraphics();) {
			opcodes_count++;
			g_vm->glk_tick();
		}

		/*
		 * If graphics is enabled and we created an image with graphics
		 * opcodes above, open a graphics window and start bitmap display.
		 */
		if (gln_graphics_enabled && opcodes_count > 0) {
			if (gln_graphics_open()) {
				/* Set the new picture flag, and start the updating "thread". */
				gln_graphics_new_picture = TRUE;
				gln_graphics_start();
			}
		}
	}
}


/*
 * gln_linegraphics_cleanup()
 *
 * Free memory resources allocated by line graphics functions.  Called on
 * game end.
 */
static void gln_linegraphics_cleanup() {
	free(gln_linegraphics_fill_segments);
	gln_linegraphics_fill_segments = nullptr;

	gln_linegraphics_fill_segments_allocation = 0;
	gln_linegraphics_fill_segments_length = 0;
}


/*---------------------------------------------------------------------*/
/*  Glk picture dispatch (bitmap or line), and timer arbitration       */
/*---------------------------------------------------------------------*/

/*
 * Note of the current set graphics mode, to detect changes in mode from
 * the core interpreter.
 */
static int gln_graphics_current_mode = -1;

/* Note indicating if the graphics "thread" is temporarily suspended. */
static int gln_graphics_suspended = FALSE;


/*
 * os_graphics()
 *
 * Called by the main interpreter to turn graphics on and off.  Mode 0
 * turns graphics off, mode 1 is line drawing graphics, and mode 2 is
 * bitmap graphics.
 *
 * This function tracks the current state of interpreter graphics setting
 * using gln_graphics_interpreter_state.
 */
void os_graphics(int mode) {
	/* Ignore the call unless it changes the graphics mode. */
	if (mode != gln_graphics_current_mode) {
		/* Set tracked interpreter state given the input mode. */
		switch (mode) {
		case 0:
			gln_graphics_interpreter_state = GLN_GRAPHICS_OFF;
			break;

		case 1:
			gln_graphics_interpreter_state = GLN_GRAPHICS_LINE_MODE;
			break;

		case 2:
			/* If no graphics bitmaps were detected, ignore this call. */
			if (!gln_graphics_bitmap_directory
			        || gln_graphics_bitmap_type == NO_BITMAPS)
				return;

			gln_graphics_interpreter_state = GLN_GRAPHICS_BITMAP_MODE;
			break;
		}

		/* Given the interpreter state, update graphics activities. */
		switch (gln_graphics_interpreter_state) {
		case GLN_GRAPHICS_OFF:

			/* If currently displaying graphics, stop and close window. */
			if (gln_graphics_enabled && gln_graphics_are_displayed()) {
				gln_graphics_stop();
				gln_graphics_close();
			}
			break;

		case GLN_GRAPHICS_LINE_MODE:
		case GLN_GRAPHICS_BITMAP_MODE:

			/* Create a new graphics context on switch to line mode. */
			if (gln_graphics_interpreter_state == GLN_GRAPHICS_LINE_MODE)
				gln_linegraphics_create_context();

			/*
			 * If we have a picture loaded already, restart graphics. If not,
			 * we'll delay this until one is supplied by a call to
			 * os_show_bitmap().
			 */
			if (gln_graphics_enabled && gln_graphics_bitmap) {
				if (gln_graphics_open())
					gln_graphics_restart();
			}
			break;
		}

		/* Note the current mode so changes can be detected. */
		gln_graphics_current_mode = mode;
	}
}


/*
 * gln_arbitrate_request_timer_events()
 *
 * Shim function for g_vm->glk_request_timer_events(), this function should be
 * called by other functional areas in place of the main timer event setting
 * function.  It suspends graphics if busy when setting timer events, and
 * resumes graphics if necessary when clearing timer events.
 *
 * On resuming, it calls the graphics timeout function to simulate the
 * timeout that has (probably) been missed.  This also ensures that tight
 * loops that enable then disable timers using this function don't lock out
 * the graphics completely.
 *
 * Use only in paired calls, the first non-zero, the second zero, and use
 * no graphics functions between calls.
 */
static void gln_arbitrate_request_timer_events(glui32 millisecs) {
	if (millisecs > 0) {
		/* Setting timer events; suspend graphics if currently active. */
		if (gln_graphics_active) {
			gln_graphics_suspended = TRUE;
			gln_graphics_stop();
		}

		/* Set timer events as requested. */
		g_vm->glk_request_timer_events(millisecs);
	} else {
		/*
		 * Resume graphics if currently suspended, otherwise cancel timer
		 * events as requested by the caller.
		 */
		if (gln_graphics_suspended) {
			gln_graphics_suspended = FALSE;
			gln_graphics_start();

			/* Simulate the "missed" graphics timeout. */
			gln_graphics_timeout();
		} else
			g_vm->glk_request_timer_events(0);
	}
}


/*---------------------------------------------------------------------*/
/*  Glk port infinite loop detection functions                         */
/*---------------------------------------------------------------------*/

/* Short timeout to wait purely in order to get the display updated. */
static const glui32 GLN_WATCHDOG_FIXUP = 50;

/*
 * Timestamp of the last watchdog tick call, and timeout.  This is used to
 * monitor the elapsed time since the interpreter made an I/O call.  If it
 * remains silent for long enough, set by the timeout, we'll offer the
 * option to end the game.  A timeout of zero disables the watchdog.
 */
static uint32 gln_watchdog_monitor = 0;
static double gln_watchdog_timeout_secs = 0.0;

/*
 * To save thrashing in time(), we want to check for timeouts less frequently
 * than we're polled.  Here's the control for that.
 */
static int gln_watchdog_check_period = 0,
           gln_watchdog_check_counter = 0;


/*
 * gln_watchdog_start()
 * gln_watchdog_stop()
 *
 * Start and stop watchdog monitoring.
 */
static void gln_watchdog_start(int timeout, int period) {
	assert(timeout > 0 && period > 0);

	gln_watchdog_timeout_secs = (double) timeout;
	gln_watchdog_check_period = period;
	gln_watchdog_check_counter = period;
	gln_watchdog_monitor = g_system->getMillis();
}

static void gln_watchdog_stop() {
	gln_watchdog_timeout_secs = 0;
}


/*
 * gln_watchdog_tick()
 *
 * Set the watchdog timestamp to the current system time.
 *
 * This function should be called just before almost every os_* function
 * returns to the interpreter, as a means of timing how long the interpreter
 * dwells in running game code.
 */
static void gln_watchdog_tick() {
	gln_watchdog_monitor = g_system->getMillis();
}


/*
 * gln_watchdog_has_timed_out()
 *
 * Check to see if too much time has elapsed since the last tick.  If it has,
 * offer the option to stop the game, and if accepted, return TRUE.  Otherwise,
 * if no timeout, or if the watchdog is disabled, return FALSE.
 *
 * This function only checks every N calls; it's called extremely frequently
 * from opcode handling, and will thrash in time() if it checks on each call.
 */
static int gln_watchdog_has_timed_out() {
	/* If loop detection is off or the timeout is set to zero, do nothing. */
	if (gln_loopcheck_enabled && gln_watchdog_timeout_secs > 0) {
		uint32 now;
		double delta_time;

		/*
		 * Wait until we've seen enough calls to make a timeout check.  If we
		 * haven't, return FALSE, otherwise reset the counter and continue.
		 */
		if (--gln_watchdog_check_counter > 0)
			return FALSE;
		else
			gln_watchdog_check_counter = gln_watchdog_check_period;

		/*
		 * Determine how much time has passed, and offer to end the game if it
		 * exceeds the allowable timeout.
		 */
		now = g_system->getMillis();
		delta_time = (now - gln_watchdog_monitor) / 1000;

		if (delta_time >= gln_watchdog_timeout_secs) {
			if (gln_confirm("\nThe game may be in an infinite loop.  Do you"
			                " want to stop it? [Y or N] ")) {
				gln_watchdog_tick();
				return TRUE;
			}

			/*
			 * If we have timers, set a really short timeout and let it expire.
			 * This is to force a display update with the response of the
			 * confirm -- without this, we may not get a screen update for a
			 * while since at this point the game isn't, by definition, doing
			 * any input or output.  If we don't have timers, no biggie.
			 */
			if (g_vm->glk_gestalt(gestalt_Timer, 0)) {
				event_t event;

				gln_arbitrate_request_timer_events(GLN_WATCHDOG_FIXUP);
				gln_event_wait(evtype_Timer, &event);
				gln_arbitrate_request_timer_events(0);
			}

			/* Reset the monitor and drop into FALSE return -- stop rejected. */
			gln_watchdog_tick();
		}
	}

	/* No timeout indicated, or offer rejected by the user. */
	return FALSE;
}


/*---------------------------------------------------------------------*/
/*  Glk port status line functions                                     */
/*---------------------------------------------------------------------*/

/* Default width used for non-windowing Glk status lines. */
static const int GLN_DEFAULT_STATUS_WIDTH = 74;


/*
 * gln_status_update()
 *
 * Update the information in the status window with the current contents of
 * the current game identity string, or a default string if no game identity
 * could be established.
 */
static void gln_status_update() {
	uint width, height;
	assert(gln_status_window);

	g_vm->glk_window_get_size(gln_status_window, &width, &height);
	if (height > 0) {
		const char *game_name;

		g_vm->glk_window_clear(gln_status_window);
		g_vm->glk_window_move_cursor(gln_status_window, 0, 0);
		g_vm->glk_set_window(gln_status_window);

		/*
		 * Try to establish a game identity to display; if none, use a standard
		 * message instead.
		 */
		game_name = g_vm->_detection._gameName;
		g_vm->glk_put_string(game_name ? game_name : "ScummVM GLK Level 9 Game");

		g_vm->glk_set_window(gln_main_window);
	}
}


/*
 * gln_status_print()
 *
 * Print the current contents of the game identity out in the main window,
 * if it has changed since the last call.  This is for non-windowing Glk
 * libraries.
 *
 * To save memory management hassles, this function uses the CRC functions
 * to detect changes of game identity string, and gambles a little on the
 * belief that two games' strings won't have the same CRC.
 */
static void gln_status_print() {
	static int is_initialized = FALSE;
	static gln_uint16 crc = 0;

	const char *game_name;

	/* Get the current game name, and do nothing if none available. */
	game_name = g_vm->_detection._gameName;
	if (game_name) {
		gln_uint16 new_crc;

		/*
		 * If not the first call and the game identity string has not changed,
		 * again, do nothing.
		 */
		new_crc = g_vm->_detection.gln_get_buffer_crc(game_name, strlen(game_name));
		if (!is_initialized || new_crc != crc) {
			int index;

#ifndef GARGLK
			/* Set fixed width font to try to preserve status line formatting. */
			g_vm->glk_set_style(style_Preformatted);
#endif

			/* Bracket, and output the extracted game name. */
			g_vm->glk_put_string("[ ");
			g_vm->glk_put_string(game_name);

			for (index = strlen(game_name);
			        index <= GLN_DEFAULT_STATUS_WIDTH; index++)
				g_vm->glk_put_char(' ');
			g_vm->glk_put_string(" ]\n");

			crc = new_crc;
			is_initialized = TRUE;
		}
	}
}


/*
 * gln_status_notify()
 *
 * Front end function for updating status.  Either updates the status window
 * or prints the status line to the main window.
 */
static void gln_status_notify() {
	if (gln_status_window)
		gln_status_update();
	else
		gln_status_print();
}


/*
 * gln_status_redraw()
 *
 * Redraw the contents of any status window with the buffered status string.
 * This function should be called on the appropriate Glk window resize and
 * arrange events.
 */
static void gln_status_redraw() {
	if (gln_status_window) {
		winid_t parent;

		/*
		 * Rearrange the status window, without changing its actual arrangement
		 * in any way.  This is a hack to work round incorrect window repainting
		 * in Xglk; it forces a complete repaint of affected windows on Glk
		 * window resize and arrange events, and works in part because Xglk
		 * doesn't check for actual arrangement changes in any way before
		 * invalidating its windows.  The hack should be harmless to Glk
		 * libraries other than Xglk, moreover, we're careful to activate it
		 * only on resize and arrange events.
		 */
		parent = g_vm->glk_window_get_parent(gln_status_window);
		g_vm->glk_window_set_arrangement(parent,
		                                 winmethod_Above | winmethod_Fixed, 1, nullptr);

		gln_status_update();
	}
}


/*---------------------------------------------------------------------*/
/*  Glk port output functions                                          */
/*---------------------------------------------------------------------*/

/*
 * Flag for if the user entered "help" as their last input, or if hints have
 * been silenced as a result of already using a Glk command.
 */
static int gln_help_requested = FALSE,
           gln_help_hints_silenced = FALSE;

/*
 * Output buffer.  We receive characters one at a time, and it's a bit
 * more efficient for everyone if we buffer them, and output a complete
 * string on a flush call.
 */
static char *gln_output_buffer = nullptr;
static int gln_output_allocation = 0,
           gln_output_length = 0;

/*
 * Output activity flag.  Set when os_printchar() is called, and queried
 * periodically by os_readchar().  Helps os_readchar() judge whether it must
 * request input, or when it's being used as a crude scroll control.
 */
static int gln_output_activity = FALSE;

/*
 * Flag to indicate if the last buffer flushed looked like it ended in a
 * "> " prompt.  Some later games switch to this mode after a while, and
 * it's nice not to duplicate this prompt with our own.
 */
static int gln_output_prompt = FALSE;


/*
 * gln_output_notify()
 *
 * Register recent text output from the interpreter.  This function is
 * called by os_printchar().
 */
static void gln_output_notify() {
	gln_output_activity = TRUE;
}


/*
 * gln_recent_output()
 *
 * Return TRUE if the interpreter has recently output text, FALSE otherwise.
 * Clears the flag, so that more output text is required before the next
 * call returns TRUE.
 */
static int gln_recent_output() {
	int result;

	result = gln_output_activity;
	gln_output_activity = FALSE;

	return result;
}


/*
 * gln_output_register_help_request()
 * gln_output_silence_help_hints()
 * gln_output_provide_help_hint()
 *
 * Register a request for help, and print a note of how to get Glk command
 * help from the interpreter unless silenced.
 */
static void gln_output_register_help_request() {
	gln_help_requested = TRUE;
}

static void gln_output_silence_help_hints() {
	gln_help_hints_silenced = TRUE;
}

static void gln_output_provide_help_hint() {
	if (gln_help_requested && !gln_help_hints_silenced) {
		g_vm->glk_set_style(style_Emphasized);
		g_vm->glk_put_string("[Try 'glk help' for help on special interpreter"
		                     " commands]\n");

		gln_help_requested = FALSE;
		g_vm->glk_set_style(style_Normal);
	}
}


/*
 * gln_game_prompted()
 *
 * Return TRUE if the last game output appears to have been a "> " prompt.
 * Once called, the flag is reset to FALSE, and requires more game output
 * to set it again.
 */
static int gln_game_prompted() {
	int result;

	result = gln_output_prompt;
	gln_output_prompt = FALSE;

	return result;
}


/*
 * gln_detect_game_prompt()
 *
 * See if the last non-newline-terminated line in the output buffer seems
 * to be a prompt, and set the game prompted flag if it does, otherwise
 * clear it.
 */
static void gln_detect_game_prompt() {
	int index;

	gln_output_prompt = FALSE;

	/*
	 * Search for a prompt across any last unterminated buffered line; a prompt
	 * is any non-space character on that line.
	 */
	for (index = gln_output_length - 1;
	        index >= 0 && gln_output_buffer[index] != '\n'; index--) {
		if (gln_output_buffer[index] != ' ') {
			gln_output_prompt = TRUE;
			break;
		}
	}
}


/*
 * gln_output_delete()
 *
 * Delete all buffered output text.  Free all malloc'ed buffer memory, and
 * return the buffer variables to their initial values.
 */
static void gln_output_delete() {
	free(gln_output_buffer);
	gln_output_buffer = nullptr;
	gln_output_allocation = gln_output_length = 0;
}


/*
 * gln_output_flush()
 *
 * Flush any buffered output text to the Glk main window, and clear the
 * buffer.  Check in passing for game prompts that duplicate our's.
 */
static void gln_output_flush() {
	assert(g_vm->glk_stream_get_current());

	if (gln_output_length > 0) {
		/*
		 * See if the game issued a standard prompt, then print the buffer to
		 * the main window.  If providing a help hint, position that before
		 * the game's prompt (if any).
		 */
		gln_detect_game_prompt();

		if (gln_output_prompt) {
			int index;

			for (index = gln_output_length - 1;
			        index >= 0 && gln_output_buffer[index] != '\n';)
				index--;

			g_vm->glk_put_buffer(gln_output_buffer, index + 1);
			gln_output_provide_help_hint();
			g_vm->glk_put_buffer(gln_output_buffer + index + 1,
			                     gln_output_length - index - 1);
		} else {
			g_vm->glk_put_buffer(gln_output_buffer, gln_output_length);
			gln_output_provide_help_hint();
		}

		gln_output_delete();
	}
}


/*
 * os_printchar()
 *
 * Buffer a character for eventual printing to the main window.
 */
void os_printchar(char c) {
	int bytes;
	assert(gln_output_length <= gln_output_allocation);

	/* Grow the output buffer if necessary. */
	for (bytes = gln_output_allocation; bytes < gln_output_length + 1;)
		bytes = bytes == 0 ? 1 : bytes << 1;

	if (bytes > gln_output_allocation) {
		gln_output_buffer = (char *)gln_realloc(gln_output_buffer, bytes);
		gln_output_allocation = bytes;
	}

	/*
	 * Add the character to the buffer, handling return as a newline, and
	 * note that the game created some output.
	 */
	gln_output_buffer[gln_output_length++] = (c == '\r' ? '\n' : c);
	gln_output_notify();
}


/*
 * gln_styled_string()
 * gln_styled_char()
 * gln_standout_string()
 * gln_standout_char()
 * gln_normal_string()
 * gln_normal_char()
 * gln_header_string()
 * gln_banner_string()
 *
 * Convenience functions to print strings in assorted styles.  A standout
 * string is one that hints that it's from the interpreter, not the game.
 */
static void gln_styled_string(glui32 style, const char *message) {
	assert(message);

	g_vm->glk_set_style(style);
	g_vm->glk_put_string(message);
	g_vm->glk_set_style(style_Normal);
}

static void gln_styled_char(glui32 style, char c) {
	char buffer[2];

	buffer[0] = c;
	buffer[1] = '\0';
	gln_styled_string(style, buffer);
}

static void gln_standout_string(const char *message) {
	gln_styled_string(style_Emphasized, message);
}

#ifndef GARGLK

static void gln_standout_char(char c) {
	gln_styled_char(style_Emphasized, c);
}

#endif

static void gln_normal_string(const char *message) {
	gln_styled_string(style_Normal, message);
}

static void gln_normal_char(char c) {
	gln_styled_char(style_Normal, c);
}

static void gln_header_string(const char *message) {
	gln_styled_string(style_Header, message);
}

#ifndef GARGLK

static void gln_banner_string(const char *message) {
	gln_styled_string(style_Subheader, message);
}

#endif


/*
 * os_flush()
 *
 * Handle a core interpreter call to flush the output buffer.  Because Glk
 * only flushes its buffers and displays text on g_vm->glk_select(), we can ignore
 * these calls as long as we call g_vm->glk_output_flush() when reading line or
 * character input.
 *
 * Taking os_flush() at face value can cause game text to appear before status
 * line text where we are working with a non-windowing Glk, so it's best
 * ignored where we can.
 */
void os_flush() {
}


/*---------------------------------------------------------------------*/
/*  Glk command escape functions                                       */
/*---------------------------------------------------------------------*/

/*
 * gln_command_script()
 *
 * Turn game output scripting (logging) on and off.
 */
static void gln_command_script(const char *argument) {
	assert(argument);

	if (gln_strcasecmp(argument, "on") == 0) {
		frefid_t fileref;

		if (gln_transcript_stream) {
			gln_normal_string("Glk transcript is already on.\n");
			return;
		}

		fileref = g_vm->glk_fileref_create_by_prompt(fileusage_Transcript
		          | fileusage_TextMode,
		          filemode_WriteAppend, 0);
		if (!fileref) {
			gln_standout_string("Glk transcript failed.\n");
			return;
		}

		gln_transcript_stream = g_vm->glk_stream_open_file(fileref,
		                        filemode_WriteAppend, 0);
		g_vm->glk_fileref_destroy(fileref);
		if (!gln_transcript_stream) {
			gln_standout_string("Glk transcript failed.\n");
			return;
		}

		g_vm->glk_window_set_echo_stream(gln_main_window, gln_transcript_stream);

		gln_normal_string("Glk transcript is now on.\n");
	}

	else if (gln_strcasecmp(argument, "off") == 0) {
		if (!gln_transcript_stream) {
			gln_normal_string("Glk transcript is already off.\n");
			return;
		}

		g_vm->glk_stream_close(gln_transcript_stream, nullptr);
		gln_transcript_stream = nullptr;

		g_vm->glk_window_set_echo_stream(gln_main_window, nullptr);

		gln_normal_string("Glk transcript is now off.\n");
	}

	else if (strlen(argument) == 0) {
		gln_normal_string("Glk transcript is ");
		gln_normal_string(gln_transcript_stream ? "on" : "off");
		gln_normal_string(".\n");
	}

	else {
		gln_normal_string("Glk transcript can be ");
		gln_standout_string("on");
		gln_normal_string(", or ");
		gln_standout_string("off");
		gln_normal_string(".\n");
	}
}


/*
 * gln_command_inputlog()
 *
 * Turn game input logging on and off.
 */
static void gln_command_inputlog(const char *argument) {
	assert(argument);

	if (gln_strcasecmp(argument, "on") == 0) {
		frefid_t fileref;

		if (gln_inputlog_stream) {
			gln_normal_string("Glk input logging is already on.\n");
			return;
		}

		fileref = g_vm->glk_fileref_create_by_prompt(fileusage_InputRecord
		          | fileusage_BinaryMode,
		          filemode_WriteAppend, 0);
		if (!fileref) {
			gln_standout_string("Glk input logging failed.\n");
			return;
		}

		gln_inputlog_stream = g_vm->glk_stream_open_file(fileref,
		                      filemode_WriteAppend, 0);
		g_vm->glk_fileref_destroy(fileref);
		if (!gln_inputlog_stream) {
			gln_standout_string("Glk input logging failed.\n");
			return;
		}

		gln_normal_string("Glk input logging is now on.\n");
	}

	else if (gln_strcasecmp(argument, "off") == 0) {
		if (!gln_inputlog_stream) {
			gln_normal_string("Glk input logging is already off.\n");
			return;
		}

		g_vm->glk_stream_close(gln_inputlog_stream, nullptr);
		gln_inputlog_stream = nullptr;

		gln_normal_string("Glk input log is now off.\n");
	}

	else if (strlen(argument) == 0) {
		gln_normal_string("Glk input logging is ");
		gln_normal_string(gln_inputlog_stream ? "on" : "off");
		gln_normal_string(".\n");
	}

	else {
		gln_normal_string("Glk input logging can be ");
		gln_standout_string("on");
		gln_normal_string(", or ");
		gln_standout_string("off");
		gln_normal_string(".\n");
	}
}


/*
 * gln_command_readlog()
 *
 * Set the game input log, to read input from a file.
 */
static void gln_command_readlog(const char *argument) {
	assert(argument);

	if (gln_strcasecmp(argument, "on") == 0) {
		frefid_t fileref;

		if (gln_readlog_stream) {
			gln_normal_string("Glk read log is already on.\n");
			return;
		}

		fileref = g_vm->glk_fileref_create_by_prompt(fileusage_InputRecord
		          | fileusage_BinaryMode,
		          filemode_Read, 0);
		if (!fileref) {
			gln_standout_string("Glk read log failed.\n");
			return;
		}

		if (!g_vm->glk_fileref_does_file_exist(fileref)) {
			g_vm->glk_fileref_destroy(fileref);
			gln_standout_string("Glk read log failed.\n");
			return;
		}

		gln_readlog_stream = g_vm->glk_stream_open_file(fileref, filemode_Read, 0);
		g_vm->glk_fileref_destroy(fileref);
		if (!gln_readlog_stream) {
			gln_standout_string("Glk read log failed.\n");
			return;
		}

		gln_normal_string("Glk read log is now on.\n");
	}

	else if (gln_strcasecmp(argument, "off") == 0) {
		if (!gln_readlog_stream) {
			gln_normal_string("Glk read log is already off.\n");
			return;
		}

		g_vm->glk_stream_close(gln_readlog_stream, nullptr);
		gln_readlog_stream = nullptr;

		gln_normal_string("Glk read log is now off.\n");
	}

	else if (strlen(argument) == 0) {
		gln_normal_string("Glk read log is ");
		gln_normal_string(gln_readlog_stream ? "on" : "off");
		gln_normal_string(".\n");
	}

	else {
		gln_normal_string("Glk read log can be ");
		gln_standout_string("on");
		gln_normal_string(", or ");
		gln_standout_string("off");
		gln_normal_string(".\n");
	}
}


/*
 * gln_command_abbreviations()
 *
 * Turn abbreviation expansions on and off.
 */
static void gln_command_abbreviations(const char *argument) {
	assert(argument);

	if (gln_strcasecmp(argument, "on") == 0) {
		if (gln_abbreviations_enabled) {
			gln_normal_string("Glk abbreviation expansions are already on.\n");
			return;
		}

		gln_abbreviations_enabled = TRUE;
		gln_normal_string("Glk abbreviation expansions are now on.\n");
	}

	else if (gln_strcasecmp(argument, "off") == 0) {
		if (!gln_abbreviations_enabled) {
			gln_normal_string("Glk abbreviation expansions are already off.\n");
			return;
		}

		gln_abbreviations_enabled = FALSE;
		gln_normal_string("Glk abbreviation expansions are now off.\n");
	}

	else if (strlen(argument) == 0) {
		gln_normal_string("Glk abbreviation expansions are ");
		gln_normal_string(gln_abbreviations_enabled ? "on" : "off");
		gln_normal_string(".\n");
	}

	else {
		gln_normal_string("Glk abbreviation expansions can be ");
		gln_standout_string("on");
		gln_normal_string(", or ");
		gln_standout_string("off");
		gln_normal_string(".\n");
	}
}


/*
 * gln_command_graphics()
 *
 * Enable or disable graphics more permanently than is done by the main
 * interpreter.  Also, print out a few brief details about the graphics
 * state of the program.
 */
static void gln_command_graphics(const char *argument) {
	assert(argument);

	if (!gln_graphics_possible) {
		gln_normal_string("Glk graphics are not available.\n");
		return;
	}

	if (gln_strcasecmp(argument, "on") == 0) {
		if (gln_graphics_enabled) {
			gln_normal_string("Glk graphics are already on.\n");
			return;
		}

		gln_graphics_enabled = TRUE;

		/* If a picture is loaded, call the restart function to repaint it. */
		if (gln_graphics_picture_is_available()) {
			if (!gln_graphics_open()) {
				gln_normal_string("Glk graphics error.\n");
				return;
			}
			gln_graphics_restart();
		}

		gln_normal_string("Glk graphics are now on.\n");
	}

	else if (gln_strcasecmp(argument, "off") == 0) {
		if (!gln_graphics_enabled) {
			gln_normal_string("Glk graphics are already off.\n");
			return;
		}

		/*
		 * Set graphics to disabled, and stop any graphics processing.  Close
		 * the graphics window.
		 */
		gln_graphics_enabled = FALSE;
		gln_graphics_stop();
		gln_graphics_close();

		gln_normal_string("Glk graphics are now off.\n");
	}

	else if (strlen(argument) == 0) {
		gln_normal_string("Glk graphics are available,");
		gln_normal_string(gln_graphics_enabled
		                  ? " and enabled.\n" : " but disabled.\n");

		if (gln_graphics_picture_is_available()) {
			int width, height;

			if (gln_graphics_get_picture_details(&width, &height)) {
				char buffer[16];

				gln_normal_string("There is a picture loaded, ");

				sprintf(buffer, "%d", width);
				gln_normal_string(buffer);
				gln_normal_string(" by ");

				sprintf(buffer, "%d", height);
				gln_normal_string(buffer);

				gln_normal_string(" pixels.\n");
			}
		}

		if (!gln_graphics_interpreter_enabled())
			gln_normal_string("Interpreter graphics are disabled.\n");

		if (gln_graphics_enabled && gln_graphics_are_displayed()) {
			const char *bitmap_type;
			int color_count, is_active;

			if (gln_graphics_get_rendering_details(&bitmap_type,
			                                       &color_count, &is_active)) {
				char buffer[16];

				gln_normal_string("Graphics are ");
				gln_normal_string(is_active ? "active, " : "displayed, ");

				sprintf(buffer, "%d", color_count);
				gln_normal_string(buffer);
				gln_normal_string(" colours");

				if (bitmap_type) {
					gln_normal_string(", ");
					gln_normal_string(bitmap_type);
					gln_normal_string(" bitmaps");
				}
				gln_normal_string(".\n");
			} else
				gln_normal_string("Graphics are being displayed.\n");
		}

		if (gln_graphics_enabled && !gln_graphics_are_displayed())
			gln_normal_string("Graphics are not being displayed.\n");
	}

	else {
		gln_normal_string("Glk graphics can be ");
		gln_standout_string("on");
		gln_normal_string(", or ");
		gln_standout_string("off");
		gln_normal_string(".\n");
	}
}


/*
 * gln_command_loopchecks()
 *
 * Turn loop checking (for game infinite loops) on and off.
 */
static void gln_command_loopchecks(const char *argument) {
	assert(argument);

	if (gln_strcasecmp(argument, "on") == 0) {
		if (gln_loopcheck_enabled) {
			gln_normal_string("Glk loop detection is already on.\n");
			return;
		}

		gln_loopcheck_enabled = TRUE;
		gln_normal_string("Glk loop detection is now on.\n");
	}

	else if (gln_strcasecmp(argument, "off") == 0) {
		if (!gln_loopcheck_enabled) {
			gln_normal_string("Glk loop detection is already off.\n");
			return;
		}

		gln_loopcheck_enabled = FALSE;
		gln_normal_string("Glk loop detection is now off.\n");
	}

	else if (strlen(argument) == 0) {
		gln_normal_string("Glk loop detection is ");
		gln_normal_string(gln_loopcheck_enabled ? "on" : "off");
		gln_normal_string(".\n");
	}

	else {
		gln_normal_string("Glk loop detection can be ");
		gln_standout_string("on");
		gln_normal_string(", or ");
		gln_standout_string("off");
		gln_normal_string(".\n");
	}
}


/*
 * gln_command_locals()
 *
 * Turn local interpretation of "quit" etc. on and off.
 */
static void gln_command_locals(const char *argument) {
	assert(argument);

	if (gln_strcasecmp(argument, "on") == 0) {
		if (gln_intercept_enabled) {
			gln_normal_string("Glk local commands are already on.\n");
			return;
		}

		gln_intercept_enabled = TRUE;
		gln_normal_string("Glk local commands are now on.\n");
	}

	else if (gln_strcasecmp(argument, "off") == 0) {
		if (!gln_intercept_enabled) {
			gln_normal_string("Glk local commands are already off.\n");
			return;
		}

		gln_intercept_enabled = FALSE;
		gln_normal_string("Glk local commands are now off.\n");
	}

	else if (strlen(argument) == 0) {
		gln_normal_string("Glk local commands are ");
		gln_normal_string(gln_intercept_enabled ? "on" : "off");
		gln_normal_string(".\n");
	}

	else {
		gln_normal_string("Glk local commands can be ");
		gln_standout_string("on");
		gln_normal_string(", or ");
		gln_standout_string("off");
		gln_normal_string(".\n");
	}
}


/*
 * gln_command_prompts()
 *
 * Turn the extra "> " prompt output on and off.
 */
static void gln_command_prompts(const char *argument) {
	assert(argument);

	if (gln_strcasecmp(argument, "on") == 0) {
		if (gln_prompt_enabled) {
			gln_normal_string("Glk extra prompts are already on.\n");
			return;
		}

		gln_prompt_enabled = TRUE;
		gln_normal_string("Glk extra prompts are now on.\n");

		/* Check for a game prompt to clear the flag. */
		gln_game_prompted();
	}

	else if (gln_strcasecmp(argument, "off") == 0) {
		if (!gln_prompt_enabled) {
			gln_normal_string("Glk extra prompts are already off.\n");
			return;
		}

		gln_prompt_enabled = FALSE;
		gln_normal_string("Glk extra prompts are now off.\n");
	}

	else if (strlen(argument) == 0) {
		gln_normal_string("Glk extra prompts are ");
		gln_normal_string(gln_prompt_enabled ? "on" : "off");
		gln_normal_string(".\n");
	}

	else {
		gln_normal_string("Glk extra prompts can be ");
		gln_standout_string("on");
		gln_normal_string(", or ");
		gln_standout_string("off");
		gln_normal_string(".\n");
	}
}


/*
 * gln_command_print_version_number()
 * gln_command_version()
 *
 * Print out the Glk library version number.
 */
static void gln_command_print_version_number(glui32 version) {
	char buffer[64];

	sprintf(buffer, "%lu.%lu.%lu",
	        (unsigned long) version >> 16,
	        (unsigned long)(version >> 8) & 0xff,
	        (unsigned long) version & 0xff);
	gln_normal_string(buffer);
}

static void gln_command_version(const char *argument) {
	glui32 version;
	assert(argument);

	gln_normal_string("This is version ");
	gln_command_print_version_number(GLN_PORT_VERSION);
	gln_normal_string(" of the Glk Level 9 port.\n");

	version = g_vm->glk_gestalt(gestalt_Version, 0);
	gln_normal_string("The Glk library version is ");
	gln_command_print_version_number(version);
	gln_normal_string(".\n");
}


/*
 * gln_command_commands()
 *
 * Turn command escapes off.  Once off, there's no way to turn them back on.
 * Commands must be on already to enter this function.
 */
static void gln_command_commands(const char *argument) {
	assert(argument);

	if (gln_strcasecmp(argument, "on") == 0) {
		gln_normal_string("Glk commands are already on.\n");
	}

	else if (gln_strcasecmp(argument, "off") == 0) {
		gln_commands_enabled = FALSE;
		gln_normal_string("Glk commands are now off.\n");
	}

	else if (strlen(argument) == 0) {
		gln_normal_string("Glk commands are ");
		gln_normal_string(gln_commands_enabled ? "on" : "off");
		gln_normal_string(".\n");
	}

	else {
		gln_normal_string("Glk commands can be ");
		gln_standout_string("on");
		gln_normal_string(", or ");
		gln_standout_string("off");
		gln_normal_string(".\n");
	}
}


/* Glk subcommands and handler functions. */
struct gln_command_t {
	const char *const command;                      /* Glk subcommand. */
	void (* const handler)(const char *argument);   /* Subcommand handler. */
	const int takes_argument;                       /* Argument flag. */
};
typedef const gln_command_t *gln_commandref_t;

static void gln_command_summary(const char *argument);
static void gln_command_help(const char *argument);

static const gln_command_t GLN_COMMAND_TABLE[] = {
	{"summary",        gln_command_summary,        FALSE},
	{"script",         gln_command_script,         TRUE},
	{"inputlog",       gln_command_inputlog,       TRUE},
	{"readlog",        gln_command_readlog,        TRUE},
	{"abbreviations",  gln_command_abbreviations,  TRUE},
	{"graphics",       gln_command_graphics,       TRUE},
	{"loopchecks",     gln_command_loopchecks,     TRUE},
	{"locals",         gln_command_locals,         TRUE},
	{"prompts",        gln_command_prompts,        TRUE},
	{"version",        gln_command_version,        FALSE},
	{"commands",       gln_command_commands,       TRUE},
	{"help",           gln_command_help,           TRUE},
	{nullptr, nullptr, FALSE}
};


/*
 * gln_command_summary()
 *
 * Report all current Glk settings.
 */
static void gln_command_summary(const char *argument) {
	gln_commandref_t entry;
	assert(argument);

	/*
	 * Call handlers that have status to report with an empty argument,
	 * prompting each to print its current setting.
	 */
	for (entry = GLN_COMMAND_TABLE; entry->command; entry++) {
		if (entry->handler == gln_command_summary
		        || entry->handler == gln_command_help)
			continue;

		entry->handler("");
	}
}


/*
 * gln_command_help()
 *
 * Document the available Glk commands.
 */
static void gln_command_help(const char *command) {
	gln_commandref_t entry, matched;
	assert(command);

	if (strlen(command) == 0) {
		gln_normal_string("Glk commands are");
		for (entry = GLN_COMMAND_TABLE; entry->command; entry++) {
			gln_commandref_t next;

			next = entry + 1;
			gln_normal_string(next->command ? " " : " and ");
			gln_standout_string(entry->command);
			gln_normal_string(next->command ? "," : ".\n\n");
		}

		gln_normal_string("Glk commands may be abbreviated, as long as"
		                  " the abbreviation is unambiguous.  Use ");
		gln_standout_string("glk help");
		gln_normal_string(" followed by a Glk command name for help on that"
		                  " command.\n");
		return;
	}

	matched = nullptr;
	for (entry = GLN_COMMAND_TABLE; entry->command; entry++) {
		if (gln_strncasecmp(command, entry->command, strlen(command)) == 0) {
			if (matched) {
				gln_normal_string("The Glk command ");
				gln_standout_string(command);
				gln_normal_string(" is ambiguous.  Try ");
				gln_standout_string("glk help");
				gln_normal_string(" for more information.\n");
				return;
			}
			matched = entry;
		}
	}
	if (!matched) {
		gln_normal_string("The Glk command ");
		gln_standout_string(command);
		gln_normal_string(" is not valid.  Try ");
		gln_standout_string("glk help");
		gln_normal_string(" for more information.\n");
		return;
	}

	if (matched->handler == gln_command_summary) {
		gln_normal_string("Prints a summary of all the current Glk Level 9"
		                  " settings.\n");
	}

	else if (matched->handler == gln_command_script) {
		gln_normal_string("Logs the game's output to a file.\n\nUse ");
		gln_standout_string("glk script on");
		gln_normal_string(" to begin logging game output, and ");
		gln_standout_string("glk script off");
		gln_normal_string(" to end it.  Glk Level 9 will ask you for a file"
		                  " when you turn scripts on.\n");
	}

	else if (matched->handler == gln_command_inputlog) {
		gln_normal_string("Records the commands you type into a game.\n\nUse ");
		gln_standout_string("glk inputlog on");
		gln_normal_string(", to begin recording your commands, and ");
		gln_standout_string("glk inputlog off");
		gln_normal_string(" to turn off input logs.  You can play back"
		                  " recorded commands into a game with the ");
		gln_standout_string("glk readlog");
		gln_normal_string(" command.\n");
	}

	else if (matched->handler == gln_command_readlog) {
		gln_normal_string("Plays back commands recorded with ");
		gln_standout_string("glk inputlog on");
		gln_normal_string(".\n\nUse ");
		gln_standout_string("glk readlog on");
		gln_normal_string(".  Command play back stops at the end of the"
		                  " file.  You can also play back commands from a"
		                  " text file created using any standard editor.\n");
	}

	else if (matched->handler == gln_command_abbreviations) {
		gln_normal_string("Controls abbreviation expansion.\n\nGlk Level 9"
		                  " automatically expands several standard single"
		                  " letter abbreviations for you; for example, \"x\""
		                  " becomes \"examine\".  Use ");
		gln_standout_string("glk abbreviations on");
		gln_normal_string(" to turn this feature on, and ");
		gln_standout_string("glk abbreviations off");
		gln_normal_string(" to turn it off.  While the feature is on, you"
		                  " can bypass abbreviation expansion for an"
		                  " individual game command by prefixing it with a"
		                  " single quote.\n");
	}

	else if (matched->handler == gln_command_graphics) {
		gln_normal_string("Turns interpreter graphics on and off.\n\nUse ");
		gln_standout_string("glk graphics on");
		gln_normal_string(" to enable interpreter graphics, and ");
		gln_standout_string("glk graphics off");
		gln_normal_string(" to turn graphics off and close the graphics window."
		                  "  This control works slightly differently to the"
		                  " 'graphics' command in Level 9 games themselves; the"
		                  " game's 'graphics' command may disable new images,"
		                  " but leave old ones displayed.  For graphics to be"
		                  " displayed, they must be turned on in both the game"
		                  " and the interpreter.\n");
	}

	else if (matched->handler == gln_command_loopchecks) {
		gln_normal_string("Controls game infinite loop monitoring.\n\n"
		                  "Some Level 9 games can enter an infinite loop if they"
		                  " have nothing better to do.  A game might do this"
		                  " after it has ended, should you decline its offer"
		                  " to rerun.  To avoid the need to kill the interpreter"
		                  " completely if a game does this, Glk Level 9 monitors"
		                  " a game's input and output, and offers the option to"
		                  " end the program gracefully if a game is silent for"
		                  " a few seconds.  Use ");
		gln_standout_string("glk loopchecks on");
		gln_normal_string(" to turn this feature on, and ");
		gln_standout_string("glk loopchecks off");
		gln_normal_string(" to turn it off.\n");
	}

	else if (matched->handler == gln_command_locals) {
		gln_normal_string("Controls interception of selected game commands.\n\n"
		                  "Some Level 9 games were written for cassette tape"
		                  " based microprocessor systems, and the way in which"
		                  " they save, restore, and restart games can reflect"
		                  " this.  There is also often no straightforward way"
		                  " to quit from a game.\n\nTo make playing a Level 9"
		                  " game appear similar to other systems, Glk Level 9"
		                  " will trap the commands 'quit', 'restart', 'save',"
		                  " 'restore', and 'load' (a synonym for 'restore') and"
		                  " handle them locally within the interpreter.  Use ");
		gln_standout_string("glk locals on");
		gln_normal_string(" to turn this feature on, and ");
		gln_standout_string("glk locals off");
		gln_normal_string(" to turn it off.\n");
	}

	else if (matched->handler == gln_command_prompts) {
		gln_normal_string("Controls extra input prompting.\n\n"
		                  "Glk Level 9 can issue a replacement '>' input"
		                  " prompt if it detects that the game hasn't prompted"
		                  " after, say, an empty input line.  Use ");
		gln_standout_string("glk prompts on");
		gln_normal_string(" to turn this feature on, and ");
		gln_standout_string("glk prompts off");
		gln_normal_string(" to turn it off.\n");
	}

	else if (matched->handler == gln_command_version) {
		gln_normal_string("Prints the version numbers of the Glk library"
		                  " and the Glk Level 9 port.\n");
	}

	else if (matched->handler == gln_command_commands) {
		gln_normal_string("Turn off Glk commands.\n\nUse ");
		gln_standout_string("glk commands off");
		gln_normal_string(" to disable all Glk commands, including this one."
		                  "  Once turned off, there is no way to turn Glk"
		                  " commands back on while inside the game.\n");
	}

	else if (matched->handler == gln_command_help)
		gln_command_help("");

	else
		gln_normal_string("There is no help available on that Glk command."
		                  "  Sorry.\n");
}


/*
 * gln_command_escape()
 *
 * This function is handed each input line.  If the line contains a specific
 * Glk port command, handle it and return TRUE, otherwise return FALSE.
 */
static int gln_command_escape(const char *string) {
	int posn;
	char *string_copy, *command, *argument;
	assert(string);

	/*
	 * Return FALSE if the string doesn't begin with the Glk command escape
	 * introducer.
	 */
	posn = strspn(string, "\t ");
	if (gln_strncasecmp(string + posn, "glk", strlen("glk")) != 0)
		return FALSE;

	/* Take a copy of the string, without any leading space or introducer. */
	string_copy = (char *)gln_malloc(strlen(string + posn) + 1 - strlen("glk"));
	strcpy(string_copy, string + posn + strlen("glk"));

	/*
	 * Find the subcommand; the first word in the string copy.  Find its end,
	 * and ensure it terminates with a NUL.
	 */
	posn = strspn(string_copy, "\t ");
	command = string_copy + posn;
	posn += strcspn(string_copy + posn, "\t ");
	if (string_copy[posn] != '\0')
		string_copy[posn++] = '\0';

	/*
	 * Now find any argument data for the command, ensuring it too terminates
	 * with a NUL.
	 */
	posn += strspn(string_copy + posn, "\t ");
	argument = string_copy + posn;
	posn += strcspn(string_copy + posn, "\t ");
	string_copy[posn] = '\0';

	/*
	 * Try to handle the command and argument as a Glk subcommand.  If it
	 * doesn't run unambiguously, print command usage.  Treat an empty command
	 * as "help".
	 */
	if (strlen(command) > 0) {
		gln_commandref_t entry, matched;
		int matches;

		/*
		 * Search for the first unambiguous table command string matching
		 * the command passed in.
		 */
		matches = 0;
		matched = nullptr;
		for (entry = GLN_COMMAND_TABLE; entry->command; entry++) {
			if (gln_strncasecmp(command, entry->command, strlen(command)) == 0) {
				matches++;
				matched = entry;
			}
		}

		/* If the match was unambiguous, call the command handler. */
		if (matches == 1) {
			gln_normal_char('\n');
			matched->handler(argument);

			if (!matched->takes_argument && strlen(argument) > 0) {
				gln_normal_string("[The ");
				gln_standout_string(matched->command);
				gln_normal_string(" command ignores arguments.]\n");
			}
		}

		/* No match, or the command was ambiguous. */
		else {
			gln_normal_string("\nThe Glk command ");
			gln_standout_string(command);
			gln_normal_string(" is ");
			gln_normal_string(matches == 0 ? "not valid" : "ambiguous");
			gln_normal_string(".  Try ");
			gln_standout_string("glk help");
			gln_normal_string(" for more information.\n");
		}
	} else {
		gln_normal_char('\n');
		gln_command_help("");
	}

	/* The string contained a Glk command; return TRUE. */
	free(string_copy);
	return TRUE;
}


/*
 * gln_command_intercept()
 *
 * The Level 9 games handle the commands "quit" and "restart" oddly, and
 * somewhat similarly.  Both prompt "Press SPACE to play again", and then
 * ignore all characters except space.  This makes it especially hard to exit
 * from a game without killing the interpreter process.  They also handle
 * "restore" via an odd security mechanism which has no real place here (the
 * base Level 9 interpreter sidesteps this with its "#restore" command, and
 * has some bugs in "save").
 *
 * To try to improve these, here we'll catch and special case the input lines
 * "quit", "save", "restore", and "restart".  "Load" is a synonym for
 * "restore".
 *
 * On "quit" or "restart", the function sets the interpreter stop reason
 * code, stops the current game run.  On "save" or "restore" it calls the
 * appropriate internal interpreter function.
 *
 * The return value is TRUE if an intercepted command was found, otherwise
 * FALSE.
 */
static int gln_command_intercept(char *string) {
	int posn, result;
	char *string_copy, *trailing;
	assert(string);

	result = FALSE;

	/* Take a copy of the string, excluding any leading whitespace. */
	posn = strspn(string, "\t ");
	string_copy = (char *)gln_malloc(strlen(string + posn) + 1);
	strcpy(string_copy, string + posn);

	/*
	 * Find the space or NUL after the first word, and check that anything
	 * after it the first word is whitespace only.
	 */
	posn = strcspn(string_copy, "\t ");
	trailing = string_copy + posn;
	if (trailing[strspn(trailing, "\t ")] == '\0') {
		/* Terminate the string copy for easy comparisons. */
		string_copy[posn] = '\0';

		/* If this command was "quit", confirm, then call StopGame(). */
		if (gln_strcasecmp(string_copy, "quit") == 0) {
			if (gln_confirm("\nDo you really want to stop? [Y or N] ")) {
				gln_stop_reason = STOP_EXIT;
				StopGame();
			}
			result = TRUE;
		}

		/* If this command was "restart", confirm, then call StopGame(). */
		else if (gln_strcasecmp(string_copy, "restart") == 0) {
			if (gln_confirm("\nDo you really want to restart? [Y or N] ")) {
				gln_stop_reason = STOP_RESTART;
				StopGame();
			}
			result = TRUE;
		}

		/* If this command was "save", simply call save(). */
		else if (gln_strcasecmp(string_copy, "save") == 0) {
			gln_standout_string("\nSaving using interpreter\n\n");
			save();
			result = TRUE;
		}

		/* If this command was "restore" or "load", call restore(). */
		else if (gln_strcasecmp(string_copy, "restore") == 0
		         || gln_strcasecmp(string_copy, "load") == 0) {
			gln_standout_string("\nRestoring using interpreter\n\n");
			restore();
			result = TRUE;
		}
	}

	free(string_copy);
	return result;
}


/*---------------------------------------------------------------------*/
/*  Glk port input functions                                           */
/*---------------------------------------------------------------------*/

/* Ctrl-C and Ctrl-U character constants. */
static const char GLN_CONTROL_C = '\003',
                  GLN_CONTROL_U = '\025';

/*
 * os_readchar() call count limit, after which we really read a character.
 * Also, call count limit on os_stoplist calls, after which we poll for a
 * character press to stop the listing, and a stoplist poll timeout.
 */
static const int GLN_READCHAR_LIMIT = 1024,
                 GLN_STOPLIST_LIMIT = 10;
static const glui32 GLN_STOPLIST_TIMEOUT = 50;

/* Quote used to suppress abbreviation expansion and local commands. */
static const char GLN_QUOTED_INPUT = '\'';


/*
 * Note of when the interpreter is in list output.  The last element of any
 * list generally lacks a terminating newline, and unless we do something
 * special with it, it'll look like a valid prompt to us.
 */
static int gln_inside_list = FALSE;


/* Table of single-character command abbreviations. */
struct gln_abbreviation_t {
	const char abbreviation;       /* Abbreviation character. */
	const char *const expansion;   /* Expansion string. */
};
typedef const gln_abbreviation_t *gln_abbreviationref_t;

static const gln_abbreviation_t GLN_ABBREVIATIONS[] = {
	{'c', "close"},    {'g', "again"},  {'i', "inventory"},
	{'k', "attack"},   {'l', "look"},   {'p', "open"},
	{'q', "quit"},     {'r', "drop"},   {'t', "take"},
	{'x', "examine"},  {'y', "yes"},    {'z', "wait"},
	{'\0', nullptr}
};


/*
 * gln_expand_abbreviations()
 *
 * Expand a few common one-character abbreviations commonly found in other
 * game systems, but not always normal in Level 9 games.
 */
static void gln_expand_abbreviations(char *buffer, int size) {
	char *command, abbreviation;
	const char *expansion;
	gln_abbreviationref_t entry;
	assert(buffer);

	/* Ignore anything that isn't a single letter command. */
	command = buffer + strspn(buffer, "\t ");
	if (!(strlen(command) == 1
	        || (strlen(command) > 1 && Common::isSpace(command[1]))))
		return;

	/* Scan the abbreviations table for a match. */
	abbreviation = g_vm->glk_char_to_lower((unsigned char) command[0]);
	expansion = nullptr;
	for (entry = GLN_ABBREVIATIONS; entry->expansion; entry++) {
		if (entry->abbreviation == abbreviation) {
			expansion = entry->expansion;
			break;
		}
	}

	/*
	 * If a match found, check for a fit, then replace the character with the
	 * expansion string.
	 */
	if (expansion) {
		if (strlen(buffer) + strlen(expansion) - 1 >= (uint)size)
			return;

		memmove(command + strlen(expansion) - 1, command, strlen(command) + 1);
		memcpy(command, expansion, strlen(expansion));

#ifndef GARGLK
		gln_standout_string("[");
		gln_standout_char(abbreviation);
		gln_standout_string(" -> ");
		gln_standout_string(expansion);
		gln_standout_string("]\n");
#endif
	}
}


/*
 * gln_output_endlist()
 *
 * The core interpreter doesn't terminate lists with a newline, so we take
 * care of that here; a fixup for input functions.
 */
static void gln_output_endlist() {
	if (gln_inside_list) {
		/*
		 * Supply the missing newline, using os_printchar() so that list output
		 * doesn't look like a prompt when we come to flush it.
		 */
		os_printchar('\n');

		gln_inside_list = FALSE;
	}
}


/*
 * os_input()
 *
 * Read a line from the keyboard.  This function makes a special case of
 * some command strings, and will also perform abbreviation expansion.
 */
gln_bool os_input(char *buffer, int size) {
	event_t event;
	assert(buffer);

	/* If doing linemode graphics, run all graphic opcodes available. */
	gln_linegraphics_process();

	/*
	 * Update the current status line display, flush any pending buffered
	 * output, and terminate any open list.
	 */
	gln_status_notify();
	gln_output_endlist();
	gln_output_flush();

	/*
	 * Level 9 games tend not to issue a prompt after reading an empty
	 * line of input, and the Adrian Mole games don't issue a prompt at
	 * all when outside the 1/2/3 menuing system.  This can make for a
	 * very blank looking screen.
	 *
	 * To slightly improve things, if it looks like we didn't get a
	 * prompt from the game, do our own.
	 */
	if (gln_prompt_enabled && !gln_game_prompted()) {
		gln_normal_char('\n');
		gln_normal_string(GLN_INPUT_PROMPT);
	}

	/*
	 * If we have an input log to read from, use that until it is exhausted.  On
	 * end of file, close the stream and resume input from line requests.
	 */
	if (gln_readlog_stream) {
		glui32 chars;

		/* Get the next line from the log stream. */
		chars = g_vm->glk_get_line_stream(gln_readlog_stream, buffer, size);
		if (chars > 0) {
			/* Echo the line just read in input style. */
			g_vm->glk_set_style(style_Input);
			g_vm->glk_put_buffer(buffer, chars);
			g_vm->glk_set_style(style_Normal);

			/* Tick the watchdog, and return. */
			gln_watchdog_tick();
			return TRUE;
		}

		/*
		 * We're at the end of the log stream.  Close it, and then continue
		 * on to request a line from Glk.
		 */
		g_vm->glk_stream_close(gln_readlog_stream, nullptr);
		gln_readlog_stream = nullptr;
	}

	/*
	 * No input log being read, or we just hit the end of file on one.  Revert
	 * to normal line input; start by getting a new line from Glk.
	 */
	g_vm->glk_request_line_event(gln_main_window, buffer, size - 1, 0);
	gln_event_wait(evtype_LineInput, &event);
	if (g_vm->shouldQuit()) {
		g_vm->glk_cancel_line_event(gln_main_window, &event);
		gln_stop_reason = STOP_EXIT;
		return FALSE;
	}

	/* Terminate the input line with a NUL. */
	assert((int)event.val1 <= size - 1);
	buffer[event.val1] = '\0';

	/*
	 * If neither abbreviations nor local commands are enabled, nor game
	 * command interceptions, use the data read above without further massaging.
	 */
	if (gln_abbreviations_enabled
	        || gln_commands_enabled || gln_intercept_enabled) {
		char *command;

		/*
		 * If the first non-space input character is a quote, bypass all
		 * abbreviation expansion and local command recognition, and use the
		 * unadulterated input, less introductory quote.
		 */
		command = buffer + strspn(buffer, "\t ");
		if (command[0] == GLN_QUOTED_INPUT) {
			/* Delete the quote with memmove(). */
			memmove(command, command + 1, strlen(command));
		} else {
			/* Check for, and expand, and abbreviated commands. */
			if (gln_abbreviations_enabled)
				gln_expand_abbreviations(buffer, size);

			/*
			 * Check for standalone "help", then for Glk port special commands;
			 * suppress the interpreter's use of this input for Glk commands by
			 * returning FALSE.
			 */
			if (gln_commands_enabled) {
				int posn;

				posn = strspn(buffer, "\t ");
				if (gln_strncasecmp(buffer + posn, "help", strlen("help")) == 0) {
					if (strspn(buffer + posn + strlen("help"), "\t ")
					        == strlen(buffer + posn + strlen("help"))) {
						gln_output_register_help_request();
					}
				}

				if (gln_command_escape(buffer)) {
					gln_output_silence_help_hints();
					gln_watchdog_tick();
					return FALSE;
				}
			}

			/*
			 * Check for locally intercepted commands, again returning FALSE if
			 * one is handled.
			 */
			if (gln_intercept_enabled) {
				if (gln_command_intercept(buffer)) {
					gln_watchdog_tick();
					return FALSE;
				}
			}
		}
	}

	/*
	 * If there is an input log active, log this input string to it. Note that
	 * by logging here we get any abbreviation expansions but we won't log glk
	 * special commands, nor any input read from a current open input log.
	 */
	if (gln_inputlog_stream) {
		g_vm->glk_put_string_stream(gln_inputlog_stream, buffer);
		g_vm->glk_put_char_stream(gln_inputlog_stream, '\n');
	}

	gln_watchdog_tick();
	return TRUE;
}


/*
 * os_readchar()
 *
 * Poll the keyboard for characters, and return the character code of any key
 * pressed, or 0 if none pressed.
 *
 * Simple though this sounds, it's tough to do right in a timesharing OS, and
 * requires something close to an abuse of Glk.
 *
 * The initial, tempting, implementation is to wait inside this function for
 * a key press, then return the code.  Unfortunately, this causes problems in
 * the Level 9 interpreter.  Here's why: the interpreter is a VM emulating a
 * single-user microprocessor system.  On such a system, it's quite okay for
 * code to spin in a loop waiting for a keypress; there's nothing else
 * happening on the system, so it can burn CPU.  To wait for a keypress, game
 * code might first wait for no-keypress (0 from this function), then a
 * keypress (non-0), then no-keypress again (and it does indeed seem to do
 * just this).  If, in os_readchar(), we simply wait for and return key codes,
 * we'll never return a 0, so the above wait for a keypress in the game will
 * hang forever.
 *
 * To make matters more complex, some Level 9 games poll for keypresses as a
 * way for a user to halt scrolling.  For these polls, we really want to
 * return 0, otherwise the output grinds to a halt.  Moreover, some games even
 * use key polling as a crude form of timeout - poll and increment a counter,
 * and exit when either os_readchar() returns non-0, or after some 300 or so
 * polls.
 *
 * So, this function MUST return 0 sometimes, and real key codes other times.
 * The solution adopted is best described as expedient.  Depending on what Glk
 * provides in the way of timers, we'll do one of two things:
 *
 *   o If we have timers, we'll set up a timeout, and poll for a key press
 *     within that timeout.  As a way to smooth output for games that use key
 *     press polling for scroll control, we'll ignore calls until we get two
 *     in a row without intervening character output.
 *
 *   o If we don't have timers, then we'll return 0 most of the time, and then
 *     really wait for a key one time out of some number.  A game polling for
 *     keypresses to halt scrolling will probably be to the point where it
 *     cannot continue without user input at this juncture, and once we've
 *     rejected a few hundred calls we can now really wait for Glk key press
 *     event, and avoid a spinning loop.  A game using key polling as crude
 *     timing may, or may not, time out in the calls for which we return 0.
 *
 * Empirically, this all seems to work.  The only odd behaviour is with the
 * DEMO mode of Adrian Mole where Glk has no timers, and this is primarily
 * because the DEMO mode relies on the delay of keyboard polling for part of
 * its effect; on a modern system, the time to call through here is nowhere
 * near the time consumed by the original platform.  The other point of note
 * is that this all means that we can't return characters from any readlog
 * with this function; its timing stuff and its general polling nature make
 * it impossible to connect to readlog, so it just won't work at all with the
 * Adrian Mole games, Glk timers or otherwise.
 */
char os_readchar(int millis) {
	static int call_count = 0;

	event_t event;
	char character;

	/* If doing linemode graphics, run all graphic opcodes available. */
	gln_linegraphics_process();

	/*
	 * Here's the way we try to emulate keyboard polling for the case of no Glk
	 * timers.  We'll say nothing is pressed for some number of consecutive
	 * calls, then continue after that number of calls.
	 */
	if (!g_vm->glk_gestalt(gestalt_Timer, 0)) {
		if (++call_count < GLN_READCHAR_LIMIT) {
			/* Call tick as we may be outside an opcode loop. */
			g_vm->glk_tick();
			gln_watchdog_tick();
			return 0;
		} else
			call_count = 0;
	}

	/*
	 * If we have Glk timers, we can smooth game output with games that contin-
	 * uously use this input function by pretending that there is no keypress
	 * if the game printed output since the last call.  This helps with the
	 * Adrian Mole games, which check for a keypress at the end of a line as a
	 * way to temporarily halt scrolling.
	 */
	if (g_vm->glk_gestalt(gestalt_Timer, 0)) {
		if (gln_recent_output()) {
			/* Call tick as we may be outside an opcode loop. */
			g_vm->glk_tick();
			gln_watchdog_tick();
			return 0;
		}
	}

	/*
	 * Now flush any pending buffered output.  We do it here rather than earlier
	 * as it only needs to be done when we're going to request Glk input, and
	 * we may have avoided this with the checks above.
	 */
	gln_status_notify();
	gln_output_endlist();
	gln_output_flush();

	/*
	 * Set up a character event request, and a timeout if the Glk library can
	 * do them, and wait until one or the other occurs.  Loop until we read an
	 * acceptable ASCII character (if we don't time out).
	 */
	do {
		g_vm->glk_request_char_event(gln_main_window);
		if (g_vm->glk_gestalt(gestalt_Timer, 0)) {
			gln_arbitrate_request_timer_events(millis);
			gln_event_wait_2(evtype_CharInput, evtype_Timer, &event);
			gln_arbitrate_request_timer_events(0);

			/*
			 * If the event was a timeout, cancel the unfilled character
			 * request, and return no-keypress value.
			 */
			if (event.type == evtype_Timer) {
				g_vm->glk_cancel_char_event(gln_main_window);
				gln_watchdog_tick();
				return 0;
			}
		} else
			gln_event_wait(evtype_CharInput, &event);
	} while (event.val1 > BYTE_MAX && event.val1 != keycode_Return);

	/* Extract the character from the event, converting Return, no echo. */
	character = event.val1 == keycode_Return ? '\n' : event.val1;

	/*
	 * Special case ^U as a way to press a key on a wait, yet return a code to
	 * the interpreter as if no key was pressed.  Useful if scrolling stops
	 * where there are no Glk timers, to get scrolling started again.  ^U is
	 * always active.
	 */
	if (character == GLN_CONTROL_U) {
		gln_watchdog_tick();
		return 0;
	}

	/*
	 * Special case ^C to quit the program.  Without this, there's no easy way
	 * to exit from a game that never uses os_input(), but instead continually
	 * uses just os_readchar().  ^C handling can be disabled with command line
	 * options.
	 */
	if (gln_intercept_enabled && character == GLN_CONTROL_C) {
		if (gln_confirm("\n\nDo you really want to stop? [Y or N] ")) {
			gln_stop_reason = STOP_EXIT;
			StopGame();

			gln_watchdog_tick();
			return 0;
		}
	}

	/*
	 * If there is a transcript stream, send the input to it as a single line
	 * string, otherwise it won't be visible in the transcript.
	 */
	if (gln_transcript_stream) {
		g_vm->glk_put_char_stream(gln_transcript_stream, character);
		g_vm->glk_put_char_stream(gln_transcript_stream, '\n');
	}

	/* Finally, return the single character read. */
	gln_watchdog_tick();
	return character;
}


/*
 * os_stoplist()
 *
 * This is called from #dictionary listings to poll for a request to stop
 * the listing.  A check for keypress is usual at this point.  However, Glk
 * cannot check for keypresses without a delay, which slows listing consid-
 * erably, since it also adjusts and renders the display.  As a compromise,
 * then, we'll check for keypresses on a small percentage of calls, say one
 * in ten, which means that listings happen with only a short delay, but
 * there's still an opportunity to stop them.
 *
 * This applies only where the Glk library has timers.  Where it doesn't, we
 * can't check for keypresses without blocking, so we do no checks at all,
 * and let lists always run to completion.
 */
gln_bool os_stoplist() {
	static int call_count = 0;

	event_t event;
	int is_stop_confirmed;

	/* Note that the interpreter is producing a list. */
	gln_inside_list = TRUE;

	/*
	 * If there are no Glk timers, then polling for a keypress but continuing
	 * on if there isn't one is not an option.  So flush output, return FALSE,
	 * and just keep listing on to the end.
	 */
	if (!g_vm->glk_gestalt(gestalt_Timer, 0)) {
		gln_output_flush();
		gln_watchdog_tick();
		return FALSE;
	}

	/* Increment the call count, and return FALSE if under the limit. */
	if (++call_count < GLN_STOPLIST_LIMIT) {
		/* Call tick as we may be outside an opcode loop. */
		g_vm->glk_tick();
		gln_watchdog_tick();
		return FALSE;
	} else
		call_count = 0;

	/* Flush any pending buffered output, delayed to here in case avoidable. */
	gln_output_flush();

	/*
	 * Look for a keypress, with a very short timeout in place, in a similar
	 * way as done for os_readchar() above.
	 */
	g_vm->glk_request_char_event(gln_main_window);
	gln_arbitrate_request_timer_events(GLN_STOPLIST_TIMEOUT);
	gln_event_wait_2(evtype_CharInput, evtype_Timer, &event);
	gln_arbitrate_request_timer_events(0);

	/*
	 * If the event was a timeout, cancel the unfilled character request, and
	 * return FALSE to continue listing.
	 */
	if (event.type == evtype_Timer) {
		g_vm->glk_cancel_char_event(gln_main_window);
		gln_watchdog_tick();
		return FALSE;
	}

	/* Keypress detected, so offer to stop listing. */
	assert(event.type == evtype_CharInput);
	is_stop_confirmed = gln_confirm("\n\nStop listing? [Y or N] ");

	/*
	 * As we've output a newline, we no longer consider that we're inside a
	 * list.  Clear the flag, and also clear prompt detection by polling it.
	 */
	gln_inside_list = FALSE;
	gln_game_prompted();

	/* Return TRUE if stop was confirmed, FALSE to keep listing. */
	gln_watchdog_tick();
	return is_stop_confirmed;
}


/*
 * gln_confirm()
 *
 * Print a confirmation prompt, and read a single input character, taking
 * only [YyNn] input.  If the character is 'Y' or 'y', return TRUE.
 */
static int gln_confirm(const char *prompt) {
	event_t event;
	unsigned char response;
	assert(prompt);

	/*
	 * Print the confirmation prompt, in a style that hints that it's from the
	 * interpreter, not the game.
	 */
	gln_standout_string(prompt);

	/* Wait for a single 'Y' or 'N' character response. */
	response = ' ';
	do {
		g_vm->glk_request_char_event(gln_main_window);
		gln_event_wait(evtype_CharInput, &event);

		if (event.val1 <= BYTE_MAX)
			response = g_vm->glk_char_to_upper(event.val1);
	} while (!(response == 'Y' || response == 'N'));

	/* Echo the confirmation response, and a blank line. */
	g_vm->glk_set_style(style_Input);
	g_vm->glk_put_string(response == 'Y' ? "Yes" : "No");
	g_vm->glk_set_style(style_Normal);
	g_vm->glk_put_string("\n\n");

	return response == 'Y';
}


/*---------------------------------------------------------------------*/
/*  Glk port event functions                                           */
/*---------------------------------------------------------------------*/

/*
 * gln_event_wait_2()
 * gln_event_wait()
 *
 * Process Glk events until one of the expected type, or types, arrives.
 * Return the event of that type.
 */
static void gln_event_wait_2(glui32 wait_type_1, glui32 wait_type_2, event_t *event) {
	assert(event);

	do {
		g_vm->glk_select(event);
		if (g_vm->shouldQuit())
			return;

		switch (event->type) {
		case evtype_Arrange:
		case evtype_Redraw:
			/* Refresh any sensitive windows on size events. */
			gln_status_redraw();
			gln_graphics_paint();
			break;

		case evtype_Timer:
			/* Do background graphics updates on timeout. */
			gln_graphics_timeout();
			break;

		default:
			break;
		}
	} while (event->type != (EvType)wait_type_1 && event->type != (EvType)wait_type_2);
}

static void gln_event_wait(glui32 wait_type, event_t *event) {
	assert(event);
	gln_event_wait_2(wait_type, evtype_None, event);
}

/*---------------------------------------------------------------------*/
/*  Glk port multi-file game functions                                 */
/*---------------------------------------------------------------------*/

/*
 * os_get_game_file ()
 *
 * This function is a bit of a cheat.  It's called when the emulator has
 * detected a request from the game to restart the tape, on a tape-based
 * game.  Ordinarily, we should prompt the player for the name of the
 * system file containing the next game part.  Unfortunately, Glk doesn't
 * make this at all easy.  The requirement is to return a filename, but Glk
 * hides these inside fileref_t's, and won't let them out.
 *
 * Theoretically, according to the porting guide, this function should
 * prompt the user for a new game file name, that being the next part of the
 * game just (presumably) completed.
 *
 * However, the newname passed in is always the current game file name, as
 * level9.c ensures this for us.  If we search for, and find, and then inc-
 * rement, the last digit in the filename passed in, we wind up with, in
 * all likelihood, the right file path.  This is icky.
 *
 * This function is likely to be a source of portability problems on
 * platforms that don't implement a file path/name mechanism that matches
 * the expectations of the Level 9 base interpreter fairly closely.
 */
gln_bool os_get_game_file(char *newname, int size) {
	char *basename;
	int index, digit, file_number;
	Common::File f;
	assert(newname);

	basename = newname;

	/* Search for the last numeric character in the basename. */
	digit = -1;
	for (index = strlen(basename) - 1; index >= 0; index--) {
		if (Common::isDigit(basename[index])) {
			digit = index;
			break;
		}
	}
	if (digit == -1) {
		gln_watchdog_tick();
		return FALSE;
	}

	/*
	 * Convert the digit to a file number and increment it.  Fail if the new
	 * file number is outside 1..9.
	 */
	file_number = basename[digit] - '0' + 1;
	if (file_number < 1 || file_number > 9) {
		gln_watchdog_tick();
		return FALSE;
	}

	/* Write the new number back into the file. */
	basename[digit] = file_number + '0';

	/* Flush pending output, then display the filename generated. */
	gln_output_flush();
	gln_game_prompted();
	gln_standout_string("\nNext load file: ");
	gln_standout_string(basename);
	gln_standout_string("\n\n");

	/*
	 * Try to confirm access to the file.  Otherwise, if we return TRUE but the
	 * interpreter can't open the file, it stops the game, and we then lose any
	 * chance to save it before quitting.
	 */
	if (!Common::File::exists(newname)) {
		/* Restore newname to how it was, and return fail. */
		basename[digit] = file_number - 1 + '0';
		gln_watchdog_tick();
		return FALSE;
	}

	/* Encourage game name re-lookup, and return success. */
	g_vm->_detection.gln_gameid_game_name_reset();
	gln_watchdog_tick();
	return TRUE;
}


/*
 * os_set_filenumber()
 *
 * This function returns the next file in a game series for a disk-based
 * game (typically, gamedat1.dat, gamedat2.dat...).  It finds a single digit
 * in a filename, and resets it to the new value passed in.  The implemen-
 * tation here is based on the generic interface version, and with the same
 * limitations, specifically being limited to file numbers in the range 0
 * to 9, since it works on only the last digit character in the filename
 * buffer passed in.
 *
 * This function may also be a source of portability problems on platforms
 * that don't use "traditional" file path schemes.
 */
void os_set_filenumber(char *newname, int size, int file_number) {
	char *basename;
	int index, digit;
	assert(newname);

	/* Do nothing if the file number is beyond what we can handle. */
	if (file_number < 0 || file_number > 9) {
		gln_watchdog_tick();
		return;
	}

	basename = newname;

	/* Search for the last numeric character in the basename. */
	digit = -1;
	for (index = strlen(basename) - 1; index >= 0; index--) {
		if (Common::isDigit(basename[index])) {
			digit = index;
			break;
		}
	}
	if (digit == -1) {
		gln_watchdog_tick();
		return;
	}

	/* Reset the digit in the file name. */
	basename[digit] = file_number + '0';

	/* Flush pending output, then display the filename generated. */
	gln_output_flush();
	gln_game_prompted();
	gln_standout_string("\nNext disk file: ");
	gln_standout_string(basename);
	gln_standout_string("\n\n");

	/* Encourage game name re-lookup, and return. */
	g_vm->_detection.gln_gameid_game_name_reset();
	gln_watchdog_tick();
}


/*
 * os_open_script_file()
 *
 * Handles player calls to the "#play" meta-command.  Because we have our
 * own way of handling scripts, this function is a stub.
 */
Common::SeekableReadStream *os_open_script_file() {
	return nullptr;
}


/*---------------------------------------------------------------------*/
/*  Functions intercepted by link-time wrappers                        */
/*---------------------------------------------------------------------*/

/*
 * __wrap_toupper()
 * __wrap_tolower()
 *
 * Wrapper functions around toupper() and tolower().  The Linux linker's
 * --wrap option will convert calls to mumble() to __wrap_mumble() if we
 * give it the right options.  We'll use this feature to translate all
 * toupper() and tolower() calls in the interpreter code into calls to
 * Glk's versions of these functions.
 *
 * It's not critical that we do this.  If a linker, say a non-Linux one,
 * won't do --wrap, then just do without it.  It's unlikely that there
 * will be much noticeable difference.
 */
int __wrap_toupper(int ch) {
	unsigned char uch;

	uch = g_vm->glk_char_to_upper((unsigned char) ch);
	return (int) uch;
}

int __wrap_tolower(int ch) {
	unsigned char lch;

	lch = g_vm->glk_char_to_lower((unsigned char) ch);
	return (int) lch;
}


/*---------------------------------------------------------------------*/
/*  main() and options parsing                                         */
/*---------------------------------------------------------------------*/

/*
 * Watchdog timeout -- we'll wait for five seconds of silence from the core
 * interpreter before offering to stop the game forcibly, and we'll check
 * it every 10,240 opcodes.
 */
static const int GLN_WATCHDOG_TIMEOUT = 5,
                 GLN_WATCHDOG_PERIOD = 10240;

/*
 * gln_establish_picture_filename()
 *
 * Given a game name, try to create an (optional) graphics data file. For
 * an input "file" X, the function looks for X.PIC or X.pic, then for
 * PICTURE.DAT or picture.dat in the same directory as X.  If the input file
 * already ends with a three-letter extension, it's stripped first.
 *
 * The function returns nullptr if a graphics file is not available.  It's not
 * fatal for this to be the case.  Filenames are malloc'ed, and need to be
 * freed by the caller.
 *
 * The function uses fopen() rather than access() since fopen() is an ANSI
 * standard function, and access() isn't.
 */
static void gln_establish_picture_filename(const char *name, char **graphics) {
	char *base, *graphics_file;
	Common::File f;
	assert(name && graphics);

	/* Take a destroyable copy of the input filename. */
	base = (char *)gln_malloc(strlen(name) + 1);
	strcpy(base, name);

	/* If base has an extension .LEV, .SNA, or similar, remove it. */
	if (strrchr(base, '.')) {
		base[strlen(base) - strlen(strrchr(base, '.'))] = '\0';
	}

	/* Allocate space for the return graphics file. */
	graphics_file = (char *)gln_malloc(strlen(base) + strlen(".___") + 1);

	/* Form a candidate graphics file, using a .PIC extension. */
	if (!f.isOpen()) {
		strcpy(graphics_file, base);
		strcat(graphics_file, ".PIC");
		f.open(graphics_file);
	}

	if (!f.isOpen()) {
		strcpy(graphics_file, base);
		strcat(graphics_file, ".pic");
		f.open(graphics_file);
	}

	/* Form a candidate graphics file, using a .CGA extension. */
	if (!f.isOpen()) {
		strcpy(graphics_file, base);
		strcat(graphics_file, ".CGA");
		f.open(graphics_file);
	}

	if (!f.isOpen()) {
		strcpy(graphics_file, base);
		strcat(graphics_file, ".cga");
		f.open(graphics_file);
	}

	/* Form a candidate graphics file, using a .HRC extension. */
	if (!f.isOpen()) {
		strcpy(graphics_file, base);
		strcat(graphics_file, ".HRC");
		f.open(graphics_file);
	}

	if (!f.isOpen()) {
		strcpy(graphics_file, base);
		strcat(graphics_file, ".hrc");
		f.open(graphics_file);
	}

	/* No access to graphics file. */
	if (!f.isOpen()) {
		free(graphics_file);
		graphics_file = nullptr;
	}

	f.close();

	/* If we found a graphics file, return its name immediately. */
	if (graphics_file) {
		*graphics = graphics_file;
		free(base);
		return;
	}

	/* Again, allocate space for the return graphics file. */
	graphics_file = (char *)gln_malloc(strlen(base) + strlen("PICTURE.DAT") + 1);

	/* As above, form a candidate graphics file. */
	strcpy(graphics_file, base);
	strcat(graphics_file, "PICTURE.DAT");

	if (!f.open(graphics_file)) {
		/* Retry, using picture.dat extension instead. */
		strcpy(graphics_file, base);
		strcat(graphics_file, "picture.dat");
		if (!f.open(graphics_file)) {
			/*
			 * No access to this graphics file.  In this case, free memory
			 * and reset graphics file to nullptr.
			 */
			free(graphics_file);
			graphics_file = nullptr;
		}
	}

	f.close();

	/*
	 * Return whatever we found for the graphics file (nullptr if none found),
	 * and free base.
	 */
	*graphics = graphics_file;
	free(base);
}

/*
 * gln_startup_code()
 * gln_main()
 *
 * Together, these functions take the place of the original main().  The
 * first one is called from glkunix_startup_code(), to parse and generally
 * handle options.  The second is called from g_vm->glk_main(), and does the real
 * work of running the game.
 */
int gln_startup_code(int argc, char *argv[]) {
	int argv_index;

	/* Handle command line arguments. */
	for (argv_index = 1;
	        argv_index < argc && argv[argv_index][0] == '-'; argv_index++) {
		if (strcmp(argv[argv_index], "-ni") == 0) {
			gln_intercept_enabled = FALSE;
			continue;
		}
		if (strcmp(argv[argv_index], "-nc") == 0) {
			gln_commands_enabled = FALSE;
			continue;
		}
		if (strcmp(argv[argv_index], "-na") == 0) {
			gln_abbreviations_enabled = FALSE;
			continue;
		}
		if (strcmp(argv[argv_index], "-np") == 0) {
			gln_graphics_enabled = FALSE;
			continue;
		}
		if (strcmp(argv[argv_index], "-ne") == 0) {
			gln_prompt_enabled = FALSE;
			continue;
		}
		if (strcmp(argv[argv_index], "-nl") == 0) {
			gln_loopcheck_enabled = FALSE;
			continue;
		}
		return FALSE;
	}

	/* All startup options were handled successfully. */
	return TRUE;
}

void gln_main(const char *filename) {
	char *graphics_file = nullptr;
	int is_running;
	int saveSlot = ConfMan.hasKey("save_slot") ? ConfMan.getInt("save_slot") : -1;

	/* Create the main Glk window, and set its stream as current. */
	gln_main_window = g_vm->glk_window_open(0, 0, 0, wintype_TextBuffer, 0);
	if (!gln_main_window) {
		gln_fatal("GLK: Can't open main window");
		g_vm->glk_exit();
	}
	g_vm->glk_window_clear(gln_main_window);
	g_vm->glk_set_window(gln_main_window);
	g_vm->glk_set_style(style_Normal);

	/*
	 * Given the basic game name, try to come up with a usable graphics
	 * filenames.  The graphics file may be null.
	 */
	gln_establish_picture_filename(filename, &graphics_file);

	/*
	 * Check Glk library capabilities, and note pictures are impossible if the
	 * library can't offer both graphics and timers.  We need timers to create
	 * the background "thread" for picture updates.
	 */
	gln_graphics_possible = g_vm->glk_gestalt(gestalt_Graphics, 0)
	                        && g_vm->glk_gestalt(gestalt_Timer, 0);

	/*
	 * If pictures are impossible, clear pictures enabled flag.  That is, act
	 * as if -np was given on the command line, even though it may not have
	 * been.  If pictures are impossible, they can never be enabled.
	 */
	if (!gln_graphics_possible)
		gln_graphics_enabled = FALSE;

	/* If pictures are possible, search for bitmap graphics. */
	if (gln_graphics_possible)
		gln_graphics_locate_bitmaps(filename);

	/* Try to create a one-line status window.  We can live without it. */
	/*
	  gln_status_window = g_vm->glk_window_open (gln_main_window,
	                                       winmethod_Above | winmethod_Fixed,
	                                       1, wintype_TextGrid, 0);
	*/

	/* Repeat this game until no more restarts requested. */
	do {
		g_vm->glk_window_clear(gln_main_window);

		/*
		 * In a multi-file game, restarting may mean reverting back to part one
		 * of the game.  So we have to encourage a re-lookup of the game name
		 * at this point.
		 */
		g_vm->_detection.gln_gameid_game_name_reset();

		/* Load the game, sending in any established graphics file. */
		int errNum = 0;
		if (!LoadGame(filename, graphics_file)) {
			if (gln_status_window)
				g_vm->glk_window_close(gln_status_window, nullptr);
			gln_header_string("Glk Level 9 Error\n\n");
			gln_normal_string("Can't find, open, or load game file '");
			gln_normal_string(filename);
			gln_normal_char('\'');
			if (errNum != 0) {
				gln_normal_string(": ERROR");
			}
			gln_normal_char('\n');

			/*
			 * Nothing more to be done, so we'll free interpreter allocated
			 * memory, then break rather than exit, to run memory cleanup and
			 * close any open streams.
			 */
			FreeMemory();
			break;
		}

		/* Print out a short banner. */
		gln_header_string("\nLevel 9 Interpreter, ScummVM version\n");

		/*
		 * Set the stop reason indicator to none.  A game will then exit with a
		 * reason if we call StopGame(), or none if it exits of its own accord
		 * (or with the "#quit" command, say).
		 */
		gln_stop_reason = STOP_NONE;

		/* Start, or restart, watchdog checking. */
		gln_watchdog_start(GLN_WATCHDOG_TIMEOUT, GLN_WATCHDOG_PERIOD);

		/* Load any savegame selected directly from the ScummVM launcher */
		if (saveSlot != -1) {
			if (g_vm->loadGameState(saveSlot).getCode() == Common::kNoError)
				printstring("\rGame restored.\r");
			else
				printstring("\rUnable to restore game.\r");

			saveSlot = -1;
		}

		/* Run the game until StopGame called, or RunGame() returns FALSE. */
		do {
			is_running = RunGame();
			g_vm->glk_tick();

			/* Poll for watchdog timeout. */
			if (is_running && gln_watchdog_has_timed_out()) {
				gln_stop_reason = STOP_FORCE;
				StopGame();
				break;
			}
		} while (is_running);

		/* Stop watchdog functions, and flush any pending buffered output. */
		gln_watchdog_stop();
		gln_status_notify();
		gln_output_flush();

		/* Free interpreter allocated memory. */
		FreeMemory();

		/*
		 * Unset any "stuck" game 'cheating' flag.  This can get stuck on if
		 * exit is forced from the #cheat mode in the Adrian Mole games, which
		 * otherwise loop infinitely.  Unsetting the flag here permits restarts;
		 * without this, the core interpreter remains permanently in silent
		 * #cheat mode.
		 */
		Cheating = FALSE;

		/*
		 * If the stop reason is none, something in the game stopped itself, or
		 * the user entered "#quit".  If the stop reason is force, the user
		 * terminated because of an apparent infinite loop.  For both of these,
		 * offer the choice to restart, or not (equivalent to exit).
		 */
		if (gln_stop_reason == STOP_NONE || gln_stop_reason == STOP_FORCE) {
			gln_standout_string(gln_stop_reason == STOP_NONE
			                    ? "\nThe game has exited.\n"
			                    : "\nGame exit was forced.  The current game"
			                    " state is unrecoverable.  Sorry.\n");

			if (gln_confirm("\nDo you want to restart? [Y or N] "))
				gln_stop_reason = STOP_RESTART;
			else
				gln_stop_reason = STOP_EXIT;
		}
	} while (gln_stop_reason == STOP_RESTART);

	/* Free any temporary memory that may have been used by graphics. */
	gln_graphics_cleanup();
	gln_linegraphics_cleanup();

	/* Close any open transcript, input log, and/or read log. */
	if (gln_transcript_stream) {
		g_vm->glk_stream_close(gln_transcript_stream, nullptr);
		gln_transcript_stream = nullptr;
	}
	if (gln_inputlog_stream) {
		g_vm->glk_stream_close(gln_inputlog_stream, nullptr);
		gln_inputlog_stream = nullptr;
	}
	if (gln_readlog_stream) {
		g_vm->glk_stream_close(gln_readlog_stream, nullptr);
		gln_readlog_stream = nullptr;
	}

	/* Free any graphics file path. */
	free(graphics_file);
}

} // End of namespace Level9
} // End of namespace Glk
