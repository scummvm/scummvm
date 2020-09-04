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

#include "glk/magnetic/magnetic_defs.h"
#include "glk/magnetic/magnetic.h"

namespace Glk {
namespace Magnetic {

const gms_command_t Magnetic::GMS_COMMAND_TABLE[14] = {
	{ &Magnetic::gms_command_summary,        "summary",        false, false },
	{ &Magnetic::gms_command_undo,           "undo",           false, true },
	{ &Magnetic::gms_command_script,         "script",         true,  false },
	{ &Magnetic::gms_command_inputlog,       "inputlog",       true,  false },
	{ &Magnetic::gms_command_readlog,        "readlog",        true,  false },
	{ &Magnetic::gms_command_abbreviations,  "abbreviations",  true,  false },
	{ &Magnetic::gms_command_graphics,       "graphics",       true,  false },
	{ &Magnetic::gms_command_gamma,          "gamma",          true,  false },
	{ &Magnetic::gms_command_animations,     "animations",     true,  false },
	{ &Magnetic::gms_command_prompts,        "prompts",        true,  false },
	{ &Magnetic::gms_command_version,        "version",        false, false },
	{ &Magnetic::gms_command_commands,       "commands",       true,  false },
	{ &Magnetic::gms_command_help,           "help",           true,  false },

	{ nullptr, nullptr, false, false}
};

const gms_gamma_t Magnetic::GMS_GAMMA_TABLE[38] = {
	{ "0.90", { 0,  29,  63,  99, 137, 175, 215, 255 }, true },
	{ "0.95", { 0,  33,  68, 105, 141, 179, 217, 255 }, true },
	{ "1.00", { 0,  36,  73, 109, 146, 182, 219, 255 }, false },
	{ "1.05", { 0,  40,  77, 114, 150, 185, 220, 255 }, true },
	{ "1.10", { 0,  43,  82, 118, 153, 188, 222, 255 }, true },
	{ "1.15", { 0,  47,  86, 122, 157, 190, 223, 255 }, true },
	{ "1.20", { 0,  50,  90, 126, 160, 193, 224, 255 }, true },
	{ "1.25", { 0,  54,  94, 129, 163, 195, 225, 255 }, true },
	{ "1.30", { 0,  57,  97, 133, 166, 197, 226, 255 }, true },
	{ "1.35", { 0,  60, 101, 136, 168, 199, 227, 255 }, true },
	{ "1.40", { 0,  64, 104, 139, 171, 201, 228, 255 }, true },
	{ "1.45", { 0,  67, 107, 142, 173, 202, 229, 255 }, true },
	{ "1.50", { 0,  70, 111, 145, 176, 204, 230, 255 }, true },
	{ "1.55", { 0,  73, 114, 148, 178, 205, 231, 255 }, true },
	{ "1.60", { 0,  76, 117, 150, 180, 207, 232, 255 }, true },
	{ "1.65", { 0,  78, 119, 153, 182, 208, 232, 255 }, true },
	{ "1.70", { 0,  81, 122, 155, 183, 209, 233, 255 }, true },
	{ "1.75", { 0,  84, 125, 157, 185, 210, 233, 255 }, true },
	{ "1.80", { 0,  87, 127, 159, 187, 212, 234, 255 }, true },
	{ "1.85", { 0,  89, 130, 161, 188, 213, 235, 255 }, true },
	{ "1.90", { 0,  92, 132, 163, 190, 214, 235, 255 }, true },
	{ "1.95", { 0,  94, 134, 165, 191, 215, 236, 255 }, true },
	{ "2.00", { 0,  96, 136, 167, 193, 216, 236, 255 }, true },
	{ "2.05", { 0,  99, 138, 169, 194, 216, 237, 255 }, true },
	{ "2.10", { 0, 101, 140, 170, 195, 217, 237, 255 }, true },
	{ "2.15", { 0, 103, 142, 172, 197, 218, 237, 255 }, true },
	{ "2.20", { 0, 105, 144, 173, 198, 219, 238, 255 }, true },
	{ "2.25", { 0, 107, 146, 175, 199, 220, 238, 255 }, true },
	{ "2.30", { 0, 109, 148, 176, 200, 220, 238, 255 }, true },
	{ "2.35", { 0, 111, 150, 178, 201, 221, 239, 255 }, true },
	{ "2.40", { 0, 113, 151, 179, 202, 222, 239, 255 }, true },
	{ "2.45", { 0, 115, 153, 180, 203, 222, 239, 255 }, true },
	{ "2.50", { 0, 117, 154, 182, 204, 223, 240, 255 }, true },
	{ "2.55", { 0, 119, 156, 183, 205, 223, 240, 255 }, true },
	{ "2.60", { 0, 121, 158, 184, 206, 224, 240, 255 }, true },
	{ "2.65", { 0, 122, 159, 185, 206, 225, 241, 255 }, true },
	{ "2.70", { 0, 124, 160, 186, 207, 225, 241, 255 }, true },
	{ NULL,   { 0,   0,   0,   0,   0,   0,   0,   0 }, false }
};

static gms_abbreviation_t GMS_ABBREVIATIONS[] = {
	{'c', "close"},    {'g', "again"},  {'i', "inventory"},
	{'k', "attack"},   {'l', "look"},   {'p', "open"},
	{'q', "quit"},     {'r', "drop"},   {'t', "take"},
	{'x', "examine"},  {'y', "yes"},    {'z', "wait"},
	{'\0', NULL}
};

/*---------------------------------------------------------------------*/
/*  Module constants                                                   */
/*---------------------------------------------------------------------*/

/* Glk Magnetic Scrolls port version number. */
static const glui32 GMS_PORT_VERSION = 0x00010601;

/* Magnetic Scrolls standard input prompt string. */
static const char *const GMS_INPUT_PROMPT = ">";

/*
 * Maximum number of regions to consider in a single repaint pass.  A
 * couple of hundred seems to strike the right balance between not too
 * sluggardly picture updates, and responsiveness to input during graphics
 * rendering, when combined with short timeouts.
 */
static const int GMS_REPAINT_LIMIT = 256;

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
static const glui32 GMS_GRAPHICS_TIMEOUT = 50;

/*
 * Count of timeouts to wait in between animation paints, and to wait on
 * repaint request.  Waiting for 2 timeouts of around 50ms, gets us to the
 * 100ms recommended animation frame rate.  Waiting after a repaint smooths
 * the display where the frame is being resized, by helping to avoid
 * graphics output while more resize events are received; around 1/2 second
 * seems okay.
 */
static const int GMS_GRAPHICS_ANIMATION_WAIT = 2,
GMS_GRAPHICS_REPAINT_WAIT = 10;

/* Pixel size multiplier for image size scaling. */
static const int GMS_GRAPHICS_PIXEL = 2;

/* Proportion of the display to use for graphics. */
static const glui32 GMS_GRAPHICS_PROPORTION = 60;

/*
 * Border and shading control.  For cases where we can't detect the back-
 * ground color of the main window, there's a default, white, background.
 * Bordering is black, with a 1 pixel border, 2 pixel shading, and 8 steps
 * of shading fade.
 */
static const glui32 GMS_GRAPHICS_DEFAULT_BACKGROUND = 0x00ffffff,
	GMS_GRAPHICS_BORDER_COLOR = 0x00000000;
static const int GMS_GRAPHICS_BORDER = 1,
	GMS_GRAPHICS_SHADING = 2,
	GMS_GRAPHICS_SHADE_STEPS = 8;

/*
 * Guaranteed unused pixel value.  This value is used to fill the on-screen
 * buffer on new pictures or repaints, resulting in a full paint of all
 * pixels since no off-screen, real picture, pixel will match it.
 */
static const int GMS_GRAPHICS_UNUSED_PIXEL = 0xff;

/* Default width used for non-windowing Glk status lines. */
static const int GMS_DEFAULT_STATUS_WIDTH = 74;

/* Success and fail return codes from hint functions. */
static const type8 GMS_HINT_SUCCESS = 1,
GMS_HINT_ERROR = 0;

/* Default window sizes for non-windowing Glk libraries. */
static const glui32 GMS_HINT_DEFAULT_WIDTH = 72,
GMS_HINT_DEFAULT_HEIGHT = 25;

/*
 * Special hint nodes indicating the root hint node, and a value to signal
 * quit from hints subsystem.
 */
static const type16 GMS_HINT_ROOT_NODE = 0,
GMS_HINTS_DONE = UINT16_MAX_VAL;

/* Generic hint topic for the root hints node. */
static const char *const GMS_GENERIC_TOPIC = "Hints Menu";

/*---------------------------------------------------------------------*/
/*  Glk port utility functions                                         */
/*---------------------------------------------------------------------*/

void Magnetic::gms_fatal(const char *str) {
	/*
	 * If the failure happens too early for us to have a window, print
	 * the message to stderr.
	 */
	if (!gms_main_window)
		error("\n\nINTERNAL ERROR: %s", str);

	/* Cancel all possible pending window input events. */
	glk_cancel_line_event(gms_main_window, NULL);
	glk_cancel_char_event(gms_main_window);
	if (gms_hint_menu_window) {
		glk_cancel_char_event(gms_hint_menu_window);
		glk_window_close(gms_hint_menu_window, NULL);
	}
	if (gms_hint_text_window) {
		glk_cancel_char_event(gms_hint_text_window);
		glk_window_close(gms_hint_text_window, NULL);
	}

	/* Print a message indicating the error. */
	glk_set_window(gms_main_window);
	glk_set_style(style_Normal);
	glk_put_string("\n\nINTERNAL ERROR: ");
	glk_put_string(str);

	glk_put_string("\n\nPlease record the details of this error, try to"
	                     " note down everything you did to cause it, and email"
	                     " this information to simon_baldwin@yahoo.com.\n\n");
}

void *Magnetic::gms_malloc(size_t size) {
	void *pointer;

	pointer = malloc(size);
	if (!pointer) {
		gms_fatal("GLK: Out of system memory");
		glk_exit();
	}

	return pointer;
}

void *Magnetic::gms_realloc(void *ptr, size_t size) {
	void *pointer;

	pointer = realloc(ptr, size);
	if (!pointer) {
		gms_fatal("GLK: Out of system memory");
		glk_exit();
	}

	return pointer;
}

int Magnetic::gms_strncasecmp(const char *s1, const char *s2, size_t n) {
	size_t index;

	for (index = 0; index < n; index++) {
		int diff;

		diff = glk_char_to_lower(s1[index]) - glk_char_to_lower(s2[index]);
		if (diff < 0 || diff > 0)
			return diff < 0 ? -1 : 1;
	}

	return 0;
}

int Magnetic::gms_strcasecmp(const char *s1, const char *s2) {
	size_t s1len, s2len;
	int result;

	s1len = strlen(s1);
	s2len = strlen(s2);

	result = gms_strncasecmp(s1, s2, s1len < s2len ? s1len : s2len);
	if (result < 0 || result > 0)
		return result;
	else
		return s1len < s2len ? -1 : s1len > s2len ? 1 : 0;
}

/*---------------------------------------------------------------------*/
/*  Glk port CRC functions                                             */
/*---------------------------------------------------------------------*/

glui32 Magnetic::gms_get_buffer_crc(const void *void_buffer, size_t length) {
	const char *buf = (const char *) void_buffer;
	uint32 crc;
	size_t index;

	/*
	 * Start with all ones in the crc, then update using table entries.  Xor
	 * with all ones again, finally, before returning.
	 */
	crc = 0xffffffff;
	for (index = 0; index < length; index++)
		crc = crc_table[(crc ^ buf[index]) & BYTE_MAX_VAL] ^ (crc >> BITS_PER_BYTE);
	return crc ^ 0xffffffff;
}

/*---------------------------------------------------------------------*/
/*  Glk port game identification data and identification functions     */
/*---------------------------------------------------------------------*/

type32 Magnetic::gms_gameid_read_uint32(int offset, Common::SeekableReadStream *stream) {
	if (!stream->seek(offset))
		return 0;
	return stream->readUint32BE();
}

void Magnetic::gms_gameid_identify_game(const Common::String &text_file) {
	Common::File stream;

	if (!stream.open(text_file))
		error("Error opening game file");

	type32 game_size, game_pc;
	gms_game_tableref_t game;

	/* Read the game's signature undo size and undo pc values. */
	game_size = gms_gameid_read_uint32(0x22, &stream);
	game_pc = gms_gameid_read_uint32(0x26, &stream);

	/* Search for these values in the table, and set game name if found. */
	game = gms_gameid_lookup_game(game_size, game_pc);
	gms_gameid_game_name = game ? game->name : NULL;
}

/*---------------------------------------------------------------------*/
/*  Glk port picture functions                                         */
/*---------------------------------------------------------------------*/

int Magnetic::gms_graphics_open() {
	if (!gms_graphics_window) {
		gms_graphics_window = glk_window_open(gms_main_window,
		                      winmethod_Above
		                      | winmethod_Proportional,
		                      GMS_GRAPHICS_PROPORTION,
		                      wintype_Graphics, 0);
	}

	return gms_graphics_window != NULL;
}

void Magnetic::gms_graphics_close() {
	if (gms_graphics_window) {
		glk_window_close(gms_graphics_window, NULL);
		gms_graphics_window = NULL;
	}
}

void Magnetic::gms_graphics_start() {
	if (gms_graphics_enabled) {
		/* If not running, start the updating "thread". */
		if (!gms_graphics_active) {
			glk_request_timer_events(GMS_GRAPHICS_TIMEOUT);
			gms_graphics_active = true;
		}
	}
}

void Magnetic::gms_graphics_stop() {
	/* If running, stop the updating "thread". */
	if (gms_graphics_active) {
		glk_request_timer_events(0);
		gms_graphics_active = false;
	}
}

void Magnetic::gms_graphics_paint() {
	if (gms_graphics_enabled && gms_graphics_are_displayed()) {
		/* Set the repaint flag, and start graphics. */
		gms_graphics_repaint = true;
		gms_graphics_start();
	}
}

void Magnetic::gms_graphics_restart() {
	if (gms_graphics_enabled && gms_graphics_are_displayed()) {
		/*
		 * If the picture is animated, we'll need to be able to re-get the
		 * first animation frame so that the picture can be treated as if
		 * it is a new one.  So here, we'll try to re-extract the current
		 * picture to do this.  Calling ms_extract() is safe because we
		 * don't get here unless graphics are displayed, and graphics aren't
		 * displayed until there's a valid picture loaded, and ms_showpic
		 * only loads a picture after it's called ms_extract and set the
		 * picture id into gms_graphics_picture.
		 *
		 * The bitmap and other picture stuff can be ignored because it's
		 * the precise same stuff as we already have in picture details
		 * variables.  If the ms_extract() fails, we'll carry on regardless,
		 * which may, or may not, result in the ideal picture display.
		 *
		 * One or two non-animated pictures return NULL from ms_extract()
		 * being re-called, so we'll restrict calls to animations only.
		 * And just to be safe, we'll also call only if we're already
		 * holding a bitmap (and we should be; how else could the graphics
		 * animation flag be set?...).
		 */
		if (gms_graphics_animated && gms_graphics_bitmap) {
			type8 animated;
			type16 width, height, palette[GMS_PALETTE_SIZE];

			/* Extract the bitmap into dummy variables. */
			(void)ms_extract(gms_graphics_picture, &width, &height, palette, &animated);
		}

		/* Set the new picture flag, and start graphics. */
		gms_graphics_new_picture = true;
		gms_graphics_start();
	}
}

void Magnetic::gms_graphics_count_colors(type8 bitmap[], type16 width, type16 height,
		int *color_count, long color_usage[]) {
	int x, y, count;
	long usage[GMS_PALETTE_SIZE], index_row;
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

	if (color_count)
		*color_count = count;

	if (color_usage)
		memcpy(color_usage, usage, sizeof(usage));
}

void Magnetic::gms_graphics_game_to_rgb_color(type16 color, gms_gammaref_t gamma,
		gms_rgbref_t rgb_color) {
	assert(gamma && rgb_color);

	/*
	 * Convert Magnetic Scrolls color, through gamma, into RGB.  This splits
	 * the color into components based on the 3-bits used in the game palette,
	 * and gamma-corrects and rescales each to the range 0-255, using the given
	 * correction.
	 */
	rgb_color->red   = gamma->table[(color & 0x700) >> 8];
	rgb_color->green = gamma->table[(color & 0x070) >> 4];
	rgb_color->blue  = gamma->table[(color & 0x007)];
}

void Magnetic::gms_graphics_split_color(glui32 color, gms_rgbref_t rgb_color) {
	assert(rgb_color);

	rgb_color->red   = (color >> 16) & 0xff;
	rgb_color->green = (color >> 8) & 0xff;
	rgb_color->blue  = color & 0xff;
}

glui32 Magnetic::gms_graphics_combine_color(gms_rgbref_t rgb_color) {
	assert(rgb_color && _screen->format.bytesPerPixel == 2);
	return  _screen->format.RGBToColor(rgb_color->red, rgb_color->green, rgb_color->blue);
}

int Magnetic::gms_graphics_color_luminance(gms_rgbref_t rgb_color) {
	/* Calculate the luminance and scale back by 1000 to 0-255 before return. */
	long luminance = ((long) rgb_color->red   * (long) GMS_LUMINANCE_WEIGHTS.red
	             + (long) rgb_color->green * (long) GMS_LUMINANCE_WEIGHTS.green
	             + (long) rgb_color->blue  * (long) GMS_LUMINANCE_WEIGHTS.blue);

	assert(luminance_weighting > 0);
	return (int)(luminance / luminance_weighting);
}

int Magnetic::gms_graphics_compare_luminance(const void *void_first,
        const void *void_second) {
	long first = *(const long *)void_first;
	long second = *(const long *)void_second;

	return first > second ? 1 : second > first ? -1 : 0;
}

long Magnetic::gms_graphics_contrast_variance(type16 palette[],
        long color_usage[], gms_gammaref_t gamma) {
	int index, count, has_black, mean;
	long sum;
	int contrast[GMS_PALETTE_SIZE];
	int luminance[GMS_PALETTE_SIZE + 1];  /* Luminance for each color,
                                           plus one extra for black */

	/* Calculate the luminance energy of each palette color at this gamma. */
	has_black = false;
	for (index = 0, count = 0; index < GMS_PALETTE_SIZE; index++) {
		if (color_usage[index] > 0) {
			gms_rgb_t rgb_color;

			/*
			 * Convert the 16-bit base picture color to RGB using the gamma
			 * currently under consideration.  Calculate luminance for this
			 * color and store in the next available luminance array entry.
			 */
			gms_graphics_game_to_rgb_color(palette[index], gamma, &rgb_color);
			luminance[count++] = gms_graphics_color_luminance(&rgb_color);

			/* Note if black is present in the palette. */
			has_black |= luminance[count - 1] == 0;
		}
	}

	/*
	 * For best results, we want to anchor contrast calculations to black, so
	 * if black is not represented in the palette, add it as an extra luminance.
	 */
	if (!has_black)
		luminance[count++] = 0;

	/* Sort luminance values so that the darkest color is at index 0. */
	qsort(luminance, count,
	      sizeof(*luminance), gms_graphics_compare_luminance);

	/*
	 * Calculate the difference in luminance between adjacent luminances in
	 * the sorted array, as contrast, and at the same time sum contrasts to
	 * calculate the mean.
	 */
	sum = 0;
	for (index = 0; index < count - 1; index++) {
		contrast[index] = luminance[index + 1] - luminance[index];
		sum += contrast[index];
	}
	mean = sum / (count - 1);

	/* Calculate and return the variance in contrasts. */
	sum = 0;
	for (index = 0; index < count - 1; index++)
		sum += (contrast[index] - mean) * (contrast[index] - mean);

	return sum / (count - 1);
}

gms_gammaref_t Magnetic::gms_graphics_equal_contrast_gamma(type16 palette[], long color_usage[]) {
	gms_gammaref_t gamma, result;
	long lowest_variance;
	assert(palette && color_usage);

	result = NULL;
	lowest_variance = INT32_MAX_VAL;

	/* Search the gamma table for the entry with the lowest contrast variance. */
	for (gamma = GMS_GAMMA_TABLE; gamma->level; gamma++) {
		long variance;

		/* Find the color contrast variance of the palette at this gamma. */
		variance = gms_graphics_contrast_variance(palette, color_usage, gamma);

		/*
		 * Compare the variance to the lowest so far, and if it is lower, note
		 * the gamma entry that produced it as being the current best found.
		 */
		if (variance < lowest_variance) {
			result = gamma;
			lowest_variance = variance;
		}
	}

	assert(result);
	return result;
}

gms_gammaref_t Magnetic::gms_graphics_select_gamma(type8 bitmap[],
        type16 width, type16 height, type16 palette[]) {
	long color_usage[GMS_PALETTE_SIZE];
	int color_count;
	gms_gammaref_t contrast_gamma;
	assert(linear_gamma);

	/*
	 * Check to see if automated correction is turned off; if it is, return
	 * the linear gamma.
	 */
	if (gms_gamma_mode == GAMMA_OFF)
		return linear_gamma;

	/*
	 * Get the color usage and count of total colors represented.  For a
	 * degenerate picture with one color or less, return the linear gamma.
	 */
	gms_graphics_count_colors(bitmap, width, height, &color_count, color_usage);
	if (color_count <= 1)
		return linear_gamma;

	/*
	 * Now calculate a gamma setting to give the most equal contrast across the
	 * picture colors.  We'll return either half this gamma, or all of it.
	 */
	contrast_gamma = gms_graphics_equal_contrast_gamma(palette, color_usage);

	/*
	 * For normal automated correction, return a gamma value half way between
	 * the linear gamma and the equal contrast gamma.
	 */
	if (gms_gamma_mode == GAMMA_NORMAL)
		return linear_gamma + (contrast_gamma - linear_gamma) / 2;

	/* Correction must be high; return the equal contrast gamma. */
	assert(gms_gamma_mode == GAMMA_HIGH);
	return contrast_gamma;
}

void Magnetic::gms_graphics_clear_and_border(winid_t glk_window,
        int x_offset, int y_offset, int pixel_size, type16 width, type16 height) {
	uint background;
	glui32 fade_color, shading_color;
	gms_rgb_t rgb_background, rgb_border, rgb_fade;
	int index;
	assert(glk_window);

	/*
	 * Try to detect the background color of the main window, by getting the
	 * background for Normal style (Glk offers no way to directly get a window's
	 * background color).  If we can get it, we'll match the graphics window
	 * background to it.  If we can't, we'll default the color to white.
	 */
	if (!glk_style_measure(gms_main_window,
	                             style_Normal, stylehint_BackColor, &background)) {
		/*
		 * Unable to get the main window background, so assume, and default
		 * graphics to white.
		 */
		background = GMS_GRAPHICS_DEFAULT_BACKGROUND;
	}

	/*
	 * Set the graphics window background to match the main window background,
	 * as best as we can tell, and clear the window.
	 */
	glk_window_set_background_color(glk_window, background);
	glk_window_clear(glk_window);

	/*
	 * For very small pictures, just border them, but don't try and do any
	 * shading.  Failing this check is probably highly unlikely.
	 */
	if (width < 2 * GMS_GRAPHICS_SHADE_STEPS
	        || height < 2 * GMS_GRAPHICS_SHADE_STEPS) {
		/* Paint a rectangle bigger than the picture by border pixels. */
		glk_window_fill_rect(glk_window,
		                           GMS_GRAPHICS_BORDER_COLOR,
		                           x_offset - GMS_GRAPHICS_BORDER,
		                           y_offset - GMS_GRAPHICS_BORDER,
		                           width * pixel_size + GMS_GRAPHICS_BORDER * 2,
		                           height * pixel_size + GMS_GRAPHICS_BORDER * 2);
		return;
	}

	/*
	 * Paint a rectangle bigger than the picture by border pixels all round,
	 * and with additional shading pixels right and below.  Some of these
	 * shading pixels are later overwritten by the fading loop below.  The
	 * picture will sit over this rectangle.
	 */
	glk_window_fill_rect(glk_window,
	                           GMS_GRAPHICS_BORDER_COLOR,
	                           x_offset - GMS_GRAPHICS_BORDER,
	                           y_offset - GMS_GRAPHICS_BORDER,
	                           width * pixel_size + GMS_GRAPHICS_BORDER * 2
	                           + GMS_GRAPHICS_SHADING,
	                           height * pixel_size + GMS_GRAPHICS_BORDER * 2
	                           + GMS_GRAPHICS_SHADING);

	/*
	 * Split the main window background color and the border color into
	 * components.
	 */
	gms_graphics_split_color(background, &rgb_background);
	gms_graphics_split_color(GMS_GRAPHICS_BORDER_COLOR, &rgb_border);

	/*
	 * Generate the incremental color to use in fade steps.  Here we're
	 * assuming that the border is always darker than the main window
	 * background (currently valid, as we're using black).
	 */
	rgb_fade.red = (rgb_background.red - rgb_border.red)
	               / GMS_GRAPHICS_SHADE_STEPS;
	rgb_fade.green = (rgb_background.green - rgb_border.green)
	                 / GMS_GRAPHICS_SHADE_STEPS;
	rgb_fade.blue = (rgb_background.blue - rgb_border.blue)
	                / GMS_GRAPHICS_SHADE_STEPS;

	/* Combine RGB fade into a single incremental Glk color. */
	fade_color = gms_graphics_combine_color(&rgb_fade);

	/* Fade in edge, from background to border, shading in stages. */
	shading_color = background;
	for (index = 0; index < GMS_GRAPHICS_SHADE_STEPS; index++) {
		/* Shade the two border areas with this color. */
		glk_window_fill_rect(glk_window, shading_color,
		                           x_offset + width * pixel_size
		                           + GMS_GRAPHICS_BORDER,
		                           y_offset + index - GMS_GRAPHICS_BORDER,
		                           GMS_GRAPHICS_SHADING, 1);
		glk_window_fill_rect(glk_window, shading_color,
		                           x_offset + index - GMS_GRAPHICS_BORDER,
		                           y_offset + height * pixel_size
		                           + GMS_GRAPHICS_BORDER,
		                           1, GMS_GRAPHICS_SHADING);

		/* Update the shading color for the fade next iteration. */
		shading_color -= fade_color;
	}
}

void Magnetic::gms_graphics_convert_palette(type16 ms_palette[], gms_gammaref_t gamma,
        glui32 glk_palette[]) {
	int index;
	assert(ms_palette && gamma && glk_palette);

	for (index = 0; index < GMS_PALETTE_SIZE; index++) {
		gms_rgb_t rgb_color;

		/*
		 * Convert the 16-bit base picture color through gamma to a 32-bit
		 * RGB color, and combine into a Glk color and store in the Glk palette.
		 */
		gms_graphics_game_to_rgb_color(ms_palette[index], gamma, &rgb_color);
		glk_palette[index] = gms_graphics_combine_color(&rgb_color);
	}
}

void Magnetic::gms_graphics_position_picture(winid_t glk_window,
        int pixel_size, type16 width, type16 height, int *x_offset, int *y_offset) {
	uint window_width, window_height;
	assert(glk_window && x_offset && y_offset);

	/* Measure the current graphics window dimensions. */
	glk_window_get_size(glk_window, &window_width, &window_height);

	/*
	 * Calculate and return an x and y offset to use on point plotting, so that
	 * the image centers inside the graphical window.
	 */
	*x_offset = ((int) window_width - width * pixel_size) / 2;
	*y_offset = ((int) window_height - height * pixel_size) / 2;
}

void Magnetic::gms_graphics_apply_animation_frame(type8 bitmap[],
        type16 frame_width, type16 frame_height, type8 mask[], int frame_x, int frame_y,
        type8 off_screen[], type16 width, type16 height) {
	int mask_width, x, y;
	type8 mask_hibit;
	long frame_row, buffer_row, mask_row;
	assert(bitmap && off_screen);

	/*
	 * It turns out that the mask isn't quite as described in defs.h, and thanks
	 * to Torbjorn Andersson and his Gtk port of Magnetic for illuminating this.
	 * The mask is made up of lines of 16-bit words, so the mask width is always
	 * even.  Here we'll calculate the real width of a mask, and also set a high
	 * bit for later on.
	 */
	mask_width = (((frame_width - 1) / BITS_PER_BYTE) + 2) & (~1);
	mask_hibit = 1 << (BITS_PER_BYTE - 1);

	/*
	 * Initialize row index components; these are optimizations to avoid the
	 * need for multiplications in the frame iteration loop.
	 */
	frame_row = 0;
	buffer_row = frame_y * width;
	mask_row = 0;

	/*
	 * Iterate over each frame row, clipping where y lies outside the main
	 * picture area.
	 */
	for (y = 0; y < frame_height; y++) {
		/* Clip if y is outside the main picture area. */
		if (y + frame_y < 0 || y + frame_y >= height) {
			/* Update optimization variables as if not clipped. */
			frame_row += frame_width;
			buffer_row += width;
			mask_row += mask_width;
			continue;
		}

		/* Iterate over each frame column, clipping again. */
		for (x = 0; x < frame_width; x++) {
			long frame_index, buffer_index;

			/* Clip if x is outside the main picture area. */
			if (x + frame_x < 0 || x + frame_x >= width)
				continue;

			/*
			 * If there's a mask, check the bit associated with this x,y, and
			 * ignore any transparent pixels.
			 */
			if (mask) {
				type8 mask_byte;

				/* Isolate the mask byte, and test the transparency bit. */
				mask_byte = mask[mask_row + (x / BITS_PER_BYTE)];
				if ((mask_byte & (mask_hibit >> (x % BITS_PER_BYTE))) != 0)
					continue;
			}

			/*
			 * Calculate indexes for this pixel into the frame, and into the
			 * main off-screen buffer, and transfer the frame pixel into the
			 * off-screen buffer.
			 */
			frame_index = frame_row + x;
			buffer_index = buffer_row + x + frame_x;
			off_screen[buffer_index] = bitmap[frame_index];
		}

		/* Update row index components on change of y. */
		frame_row += frame_width;
		buffer_row += width;
		mask_row += mask_width;
	}
}

int Magnetic::gms_graphics_animate(type8 off_screen[], type16 width, type16 height) {
	struct ms_position *positions;
	type16 count;
	type8 status;
	int frame;
	assert(off_screen);

	/* Search for more animation frames, and return zero if none. */
	status = ms_animate(&positions, &count);
	if (status == 0)
		return false;

	/* Apply each animation frame to the off-screen buffer. */
	for (frame = 0; frame < count; frame++) {
		type8 *bitmap, *mask;
		type16 frame_width, frame_height;

		/*
		 * Get the bitmap and other details for this frame.  If we can't get
		 * this animation frame, skip it and see if any others are available.
		 */
		bitmap = ms_get_anim_frame(positions[frame].number,
		                           &frame_width, &frame_height, &mask);
		if (bitmap) {
			gms_graphics_apply_animation_frame(bitmap,
			                                   frame_width, frame_height, mask,
			                                   positions[frame].x,
			                                   positions[frame].y,
			                                   off_screen, width, height);
		}
	}

	/* Return true since more animation frames remain. */
	return true;
}

#ifndef GARGLK
int Magnetic::gms_graphics_is_vertex(type8 off_screen[], type16 width, type16 height,
		int x, int y) {
	type8 pixel;
	int above, below, left, right;
	long index_row;
	assert(off_screen);

	/* Use an index row to cut down on multiplications. */
	index_row = y * width;

	/* Find the color of the reference pixel. */
	pixel = off_screen[index_row + x];
	assert(pixel < GMS_PALETTE_SIZE);

	/*
	 * Detect differences between the reference pixel and its upper, lower, left
	 * and right neighbors.  Mark as different if the neighbor doesn't exist,
	 * that is, at the edge of the picture.
	 */
	above = (y == 0 || off_screen[index_row - width + x] != pixel);
	below = (y == height - 1 || off_screen[index_row + width + x] != pixel);
	left  = (x == 0 || off_screen[index_row + x - 1] != pixel);
	right = (x == width - 1 || off_screen[index_row + x + 1] != pixel);

	/*
	 * Return true if this pixel lies at the vertex of a rectangular, fillable,
	 * area.  That is, if two adjacent neighbors aren't the same color (or if
	 * absent -- at the edge of the picture).
	 */
	return ((above || below) && (left || right));
}

int Magnetic::gms_graphics_compare_layering_inverted(const void *void_first,
        const void *void_second) {
	gms_layering_t *first = (gms_layering_t *) void_first;
	gms_layering_t *second = (gms_layering_t *) void_second;

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

void Magnetic::gms_graphics_assign_layers(type8 off_screen[], type8 on_screen[],
                                       type16 width, type16 height,
                                       int layers[], long layer_usage[]) {
	int index, x, y;
	long index_row;
	gms_layering_t layering[GMS_PALETTE_SIZE];
	assert(off_screen && on_screen && layers && layer_usage);

	/* Clear initial complexity and usage counts, and set initial colors. */
	for (index = 0; index < GMS_PALETTE_SIZE; index++) {
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
			 * Get the index for this pixel, and update complexity and usage
			 * if off-screen and on-screen pixels differ.
			 */
			idx = index_row + x;
			if (on_screen[idx] != off_screen[idx]) {
				if (gms_graphics_is_vertex(off_screen, width, height, x, y))
					layering[off_screen[idx]].complexity++;

				layering[off_screen[idx]].usage++;
			}
		}
	}

	/*
	 * Sort counts to form color indexes.  The primary sort is on the shape
	 * complexity, and within this, on color usage.
	 */
	qsort(layering, GMS_PALETTE_SIZE,
	      sizeof(*layering), gms_graphics_compare_layering_inverted);

	/*
	 * Assign a layer to each palette color, and also return the layer usage
	 * for each layer.
	 */
	for (index = 0; index < GMS_PALETTE_SIZE; index++) {
		layers[layering[index].color] = index;
		layer_usage[index] = layering[index].usage;
	}
}

void Magnetic::gms_graphics_paint_region(winid_t glk_window, glui32 palette[], int layers[],
                                      type8 off_screen[], type8 on_screen[],
                                      int x, int y, int x_offset, int y_offset,
                                      int pixel_size, type16 width, type16 height) {
	type8 pixel;
	int layer, x_min, x_max, y_min, y_max, x_index, y_index;
	long index_row;
	assert(glk_window && palette && layers && off_screen && on_screen);

	/* Find the color and layer for the initial pixel. */
	pixel = off_screen[y * width + x];
	layer = layers[pixel];
	assert(pixel < GMS_PALETTE_SIZE);

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
	glk_window_fill_rect(glk_window, palette[pixel],
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

void Magnetic::gms_graphics_paint_everything(winid_t glk_window,
        glui32 palette[], type8 off_screen[], int x_offset, int y_offset,
        type16 width, type16 height) {
	type16 x, y;
	glui32 pixel;

	Graphics::ManagedSurface s(width, height, _screen->format);

	for (y = 0; y < height; y++) {
		uint16 *lineP = (uint16 *)s.getBasePtr(0, y);

		for (x = 0; x < width; ++x, ++lineP) {
			pixel = palette[off_screen[y * width + x]];
			*lineP = pixel;
		}
	}

	glk_image_draw_scaled(glk_window, s, (uint)-1, x_offset, y_offset,
		width * GMS_GRAPHICS_PIXEL, height * GMS_GRAPHICS_PIXEL);
}

void Magnetic::gms_graphics_timeout() {
	static glui32 palette[GMS_PALETTE_SIZE];   /* Precomputed Glk palette */
#ifndef GARGLK
	static int layers[GMS_PALETTE_SIZE];       /* Assigned image layers */
	static long layer_usage[GMS_PALETTE_SIZE]; /* Image layer occupancies */
#endif

	static int deferred_repaint = false;       /* Local delayed repaint flag */
	static int ignore_counter;                 /* Count of calls ignored */

	static int x_offset, y_offset;             /* Point plot offsets */
	static int yield_counter;                  /* Yields in rendering */
#ifndef GARGLK
	static int saved_layer;                    /* Saved current layer */
	static int saved_x, saved_y;               /* Saved x,y coord */
	static int total_regions;                  /* Debug statistic */
#endif

	type8 *on_screen;                          /* On-screen image buffer */
	type8 *off_screen;                         /* Off-screen image buffer */
	long picture_size;                         /* Picture size in pixels */
//  int layer;                                 /* Image layer iterator */
//  int x, y;                                  /* Image iterators */
//  int regions;                               /* Count of regions painted */

	/* Ignore the call if the current graphics state is inactive. */
	if (!gms_graphics_active)
		return;
	assert(gms_graphics_window);

	/*
	 * On detecting a repaint request, note the flag in a local static variable,
	 * then set up a graphics delay to wait until, hopefully, the resize, if
	 * that's what caused it, is complete, and return.  This makes resizing the
	 * window a lot smoother, since it prevents unnecessary region paints where
	 * we are receiving consecutive Glk arrange or redraw events.
	 */
	if (gms_graphics_repaint) {
		deferred_repaint = true;
		gms_graphics_repaint = false;
		ignore_counter = GMS_GRAPHICS_REPAINT_WAIT - 1;
		return;
	}

	/*
	 * If asked to ignore a given number of calls, decrement the ignore counter
	 * and return having done nothing more.  This lets us delay graphics
	 * operations by a number of timeouts, providing animation timing and
	 * partial protection from resize event "storms".
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
	picture_size = gms_graphics_width * gms_graphics_height;
	off_screen = gms_graphics_off_screen;
	on_screen = gms_graphics_on_screen;

	/*
	 * If we received a new picture, set up the local static variables for that
	 * picture -- decide on gamma correction, convert the color palette, and
	 * initialize the off_screen buffer to be the base picture.
	 */
	if (gms_graphics_new_picture) {
		/* Initialize the off_screen buffer to be a copy of the base picture. */
		free(off_screen);
		off_screen = (type8 *)gms_malloc(picture_size * sizeof(*off_screen));
		memcpy(off_screen, gms_graphics_bitmap,
		       picture_size * sizeof(*off_screen));

		/* Note the buffer for freeing on cleanup. */
		gms_graphics_off_screen = off_screen;

		/*
		 * If the picture is animated, apply the first animation frames now.
		 * This is important, since they form an intrinsic part of the first
		 * displayed image (in type2 animation cases, perhaps _all_ of the
		 * first displayed image).
		 */
		if (gms_graphics_animated) {
			gms_graphics_animate(off_screen,
			                     gms_graphics_width, gms_graphics_height);
		}

		/*
		 * Select a suitable gamma for the picture, taking care to use the
		 * off-screen buffer.
		 */
		gms_graphics_current_gamma =
		    gms_graphics_select_gamma(off_screen,
		                              gms_graphics_width,
		                              gms_graphics_height,
		                              gms_graphics_palette);

		/*
		 * Pre-convert all the picture palette colors into their corresponding
		 * Glk colors.
		 */
		gms_graphics_convert_palette(gms_graphics_palette,
		                             gms_graphics_current_gamma, palette);

		/* Save the color count for possible queries later. */
		gms_graphics_count_colors(off_screen,
		                          gms_graphics_width, gms_graphics_height,
		                          &gms_graphics_color_count, NULL);
	}

	/*
	 * For a new picture, or a repaint of a prior one, calculate new values for
	 * the x and y offsets used to draw image points, and set the on-screen
	 * buffer to an unused pixel value, in effect invalidating all on-screen
	 * data.  Also, reset the saved image scan coordinates so that we scan for
	 * unpainted pixels from top left starting at layer zero, and clear the
	 * graphics window.
	 */
	if (gms_graphics_new_picture || deferred_repaint) {
		/*
		 * Calculate the x and y offset to center the picture in the graphics
		 * window.
		 */
		gms_graphics_position_picture(gms_graphics_window,
		                              GMS_GRAPHICS_PIXEL,
		                              gms_graphics_width, gms_graphics_height,
		                              &x_offset, &y_offset);

		/*
		 * Reset all on-screen pixels to an unused value, guaranteed not to
		 * match any in a real picture.  This forces all pixels to be repainted
		 * on a buffer/on-screen comparison.
		 */
		free(on_screen);
		on_screen = (type8 *)gms_malloc(picture_size * sizeof(*on_screen));
		memset(on_screen, GMS_GRAPHICS_UNUSED_PIXEL,
		       picture_size * sizeof(*on_screen));

		/* Note the buffer for freeing on cleanup. */
		gms_graphics_on_screen = on_screen;

		/*
		 * Assign new layers to the current image.  This sorts colors by usage
		 * and puts the most used colors in the lower layers.  It also hands us
		 * a count of pixels in each layer, useful for knowing when to stop
		 * scanning for layers in the rendering loop.
		 */
#ifndef GARGLK
		gms_graphics_assign_layers(off_screen, on_screen,
		                           gms_graphics_width, gms_graphics_height,
		                           layers, layer_usage);

		saved_layer = 0;
		saved_x = 0;
		saved_y = 0;
		total_regions = 0;
#endif

		/* Clear the graphics window. */
		gms_graphics_clear_and_border(gms_graphics_window,
		                              x_offset, y_offset,
		                              GMS_GRAPHICS_PIXEL,
		                              gms_graphics_width, gms_graphics_height);

		/* Start a fresh picture rendering pass. */
		yield_counter = 0;

		/* Clear the new picture and deferred repaint flags. */
		gms_graphics_new_picture = false;
		deferred_repaint = false;
	}

#ifndef GARGLK
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
	        layer < GMS_PALETTE_SIZE && layer_usage[layer] > 0; layer++) {
		long index_row;

		/*
		 * As an optimization to avoid multiplications in the loop, maintain a
		 * separate index row.
		 */
		index_row = saved_y * gms_graphics_width;
		for (y = saved_y; y < gms_graphics_height; y++) {
			for (x = saved_x; x < gms_graphics_width; x++) {
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
					gms_graphics_paint_region(gms_graphics_window,
					                          palette, layers,
					                          off_screen, on_screen,
					                          x, y, x_offset, y_offset,
					                          GMS_GRAPHICS_PIXEL,
					                          gms_graphics_width,
					                          gms_graphics_height);

					/*
					 * Increment count of regions handled, and yield, by
					 * returning, if the limit on paint regions is reached.
					 * Before returning, save the current layer and scan
					 * coordinates, so we can pick up here on the next call.
					 */
					regions++;
					if (regions >= GMS_REPAINT_LIMIT) {
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
			index_row += gms_graphics_width;
		}

		/* Reset the saved y coordinate on layer change. */
		saved_y = 0;
	}

	/*
	 * If we reach this point, then we didn't get to the limit on regions
	 * painted on this pass.  In that case, we've finished rendering the
	 * image.
	 */
	assert(regions < GMS_REPAINT_LIMIT);
	total_regions += regions;

#else
	gms_graphics_paint_everything
	(gms_graphics_window,
	 palette, off_screen,
	 x_offset, y_offset,
	 gms_graphics_width,
	 gms_graphics_height);
#endif

	/*
	 * If animated, and if animations are enabled, handle further animation
	 * frames, if any.
	 */
	if (gms_animation_enabled && gms_graphics_animated) {
		int more_animation;

		/*
		 * Reset the off-screen buffer to a copy of the base picture.  This is
		 * the correct state for applying animation frames.
		 */
		memcpy(off_screen, gms_graphics_bitmap,
		       picture_size * sizeof(*off_screen));

		/*
		 * Apply any further animations.  If none, then stop the graphics
		 * "thread" and return.  There's no more to be done until something
		 * restarts us.
		 */
		more_animation = gms_graphics_animate(off_screen,
		                                      gms_graphics_width,
		                                      gms_graphics_height);
		if (!more_animation) {
			/*
			 * There's one extra wrinkle here.  The base picture we've just put
			 * into the off-screen buffer isn't really complete (and for type2
			 * animations, might be pure garbage), so if we happen to get a
			 * repaint after an animation has ended, the off-screen data we'll
			 * be painting could well look wrong.
			 *
			 * So... here we want to set the off-screen buffer to contain the
			 * final animation frame.  Fortunately, we still have it in the
			 * on-screen buffer.
			 */
			memcpy(off_screen, on_screen, picture_size * sizeof(*off_screen));
			gms_graphics_stop();
			return;
		}

		/*
		 * Re-assign layers based on animation changes to the off-screen
		 * buffer.
		 */
#ifndef GARGLK
		gms_graphics_assign_layers(off_screen, on_screen,
		                           gms_graphics_width, gms_graphics_height,
		                           layers, layer_usage);
#endif

		/*
		 * Set up an animation wait, adjusted here by the number of times we
		 * had to yield while rendering, as we're now that late with animations,
		 * and capped at zero, as we can't do anything to compensate for being
		 * too late.  In practice, we're running too close to the edge to have
		 * much of an effect here, but nevertheless...
		 */
		ignore_counter = GMS_GRAPHICS_ANIMATION_WAIT - 1;
		if (yield_counter > ignore_counter)
			ignore_counter = 0;
		else
			ignore_counter -= yield_counter;

		/* Start a fresh picture rendering pass. */
		yield_counter = 0;
#ifndef GARGLK
		saved_layer = 0;
		saved_x = 0;
		saved_y = 0;
		total_regions = 0;
#endif
	} else {
		/*
		 * Not an animated picture, so just stop graphics, as again, there's
		 * no more to be done until something restarts us.
		 */
		gms_graphics_stop();
	}
}

void Magnetic::ms_showpic(type32 picture, type8 mode) {
	type8 *bitmap, animated;
	type16 width, height, palette[GMS_PALETTE_SIZE];
	long picture_bytes;
	glui32 crc;

	/* See if the mode indicates no graphics. */
	if (mode == 0) {
		/* Note that the interpreter turned graphics off. */
		gms_graphics_interpreter = false;

		/*
		 * If we are currently displaying the graphics window, stop any update
		 * "thread" and turn off graphics.
		 */
		if (gms_graphics_enabled && gms_graphics_are_displayed()) {
			gms_graphics_stop();
			gms_graphics_close();
		}

		/* Nothing more to do now graphics are off. */
		return;
	}

	/* Note that the interpreter turned graphics on. */
	gms_graphics_interpreter = true;

	/*
	 * Obtain the image details for the requested picture.  The call returns
	 * NULL if there's a problem with the picture.
	 */
	bitmap = ms_extract(picture, &width, &height, palette, &animated);
	if (!bitmap)
		return;

	/* Note the last thing passed to ms_extract, in case of graphics restarts. */
	gms_graphics_picture = picture;

	/* Calculate the picture size, and the CRC for the bitmap data. */
	picture_bytes = width * height * sizeof(*bitmap);
	crc = gms_get_buffer_crc(bitmap, picture_bytes);

	/*
	 * If there is no change of picture, we might be able to largely ignore the
	 * call.  Check for a change, and if we don't see one, and if graphics are
	 * enabled and being displayed, we can safely ignore the call.
	 */
	if (width == gms_graphics_width
	        && height == gms_graphics_height
	        && crc == pic_current_crc
	        && gms_graphics_enabled && gms_graphics_are_displayed())
		return;

	/*
	 * We know now that this is either a genuine change of picture, or graphics
	 * were off and have been turned on.  So, record picture details, ensure
	 * graphics is on, set the flags, and start the background graphics update.
	 */

	/*
	 * Save the picture details for the update code.  Here we take a complete
	 * local copy of the bitmap, since the interpreter core may reuse part of
	 * its memory for animations.
	 */
	free(gms_graphics_bitmap);
	gms_graphics_bitmap = (type8 *)gms_malloc(picture_bytes);
	memcpy(gms_graphics_bitmap, bitmap, picture_bytes);
	gms_graphics_width = width;
	gms_graphics_height = height;
	memcpy(gms_graphics_palette, palette, sizeof(palette));
	gms_graphics_animated = animated;

	/* Retain the new picture CRC. */
	pic_current_crc = crc;

	/*
	 * If graphics are enabled, ensure the window is displayed, set the
	 * appropriate flags, and start graphics update.  If they're not enabled,
	 * the picture details will simply stick around in module variables until
	 * they are required.
	 */
	if (gms_graphics_enabled) {
		/*
		 * Ensure graphics on, then set the new picture flag and start the
		 * updating "thread".
		 */
		if (gms_graphics_open()) {
			gms_graphics_new_picture = true;
			gms_graphics_start();
		}
	}
}

int Magnetic::gms_graphics_get_picture_details(int *width, int *height, int *is_animated) {
	if (gms_graphics_picture_is_available()) {
		if (width)
			*width = gms_graphics_width;
		if (height)
			*height = gms_graphics_height;
		if (is_animated)
			*is_animated = gms_graphics_animated;

		return true;
	}

	return false;
}

int Magnetic::gms_graphics_get_rendering_details(const char **gamma,
		int *color_count, int *is_active) {
	if (gms_graphics_enabled && gms_graphics_are_displayed()) {
		/*
		 * Return the string representing the gamma correction.  If racing
		 * with timeouts, we might return the gamma for the last picture.
		 */
		if (gamma) {
			assert(gms_graphics_current_gamma);
			*gamma = gms_graphics_current_gamma->level;
		}

		/*
		 * Return the color count noted by timeouts on the first timeout
		 * following a new picture.  Again, we might return the one for
		 * the prior picture.
		 */
		if (color_count)
			*color_count = gms_graphics_color_count;

		/* Return graphics active flag. */
		if (is_active)
			*is_active = gms_graphics_active;

		return true;
	}

	return false;
}

int Magnetic::gms_graphics_interpreter_enabled() {
	return gms_graphics_interpreter;
}

void Magnetic::gms_graphics_cleanup() {
	free(gms_graphics_bitmap);
	gms_graphics_bitmap = NULL;
	free(gms_graphics_off_screen);
	gms_graphics_off_screen = NULL;
	free(gms_graphics_on_screen);
	gms_graphics_on_screen = NULL;

	gms_graphics_animated = false;
	gms_graphics_picture = 0;
}

/*---------------------------------------------------------------------*/
/*  Glk port status line functions                                     */
/*---------------------------------------------------------------------*/

void Magnetic::ms_statuschar(type8 c) {
	static char buffer_[GMS_STATBUFFER_LENGTH];
	static int length = 0;

	/*
	 * If the status character is newline, transfer locally buffered data to
	 * the common buffer, empty the local buffer; otherwise, if space permits,
	 * buffer the character.
	 */
	if (c == '\n') {
		memcpy(gms_status_buffer, buffer_, length);
		gms_status_length = length;

		length = 0;
	} else {
		if (length < (int)sizeof(buffer_))
			buffer_[length++] = c;
	}
}

void Magnetic::gms_status_update() {
	uint width, height;
	int index;
	assert(gms_status_window);

	glk_window_get_size(gms_status_window, &width, &height);
	if (height > 0) {
		glk_window_clear(gms_status_window);
		glk_window_move_cursor(gms_status_window, 0, 0);
		glk_set_window(gms_status_window);

		glk_set_style(style_User1);
		for (index = 0; index < (int)width; index++)
			glk_put_char(' ');
		glk_window_move_cursor(gms_status_window, 1, 0);

		if (gms_status_length > 0) {
			/*
			 * Output each character from the status line buffer.  If the
			 * character is Tab, position the cursor to eleven characters shy
			 * of the status window right.
			 */
			for (index = 0; index < gms_status_length; index++) {
				if (gms_status_buffer[index] == '\t')
					glk_window_move_cursor(gms_status_window, width - 11, 0);
				else
					glk_put_char(gms_status_buffer[index]);
			}
		} else {
			const char *game_name;

			/*
			 * We have no status line to display, so print the game's name, or
			 * a standard message if unable to identify the game.  Having no
			 * status line is common with Magnetic Windows games, which don't,
			 * in general, seem to use one.
			 */
			game_name = gms_gameid_get_game_name();
			glk_put_string(game_name ? game_name : "ScummVM Magnetic version 2.3");
		}

		glk_set_window(gms_main_window);
	}
}

void Magnetic::gms_status_print() {
	static char buffer_[GMS_STATBUFFER_LENGTH];
	static int length = 0;

	int index, column;

	/*
	 * Do nothing if there is no status line to print, or if the status
	 * line hasn't changed since last printed.
	 */
	if (gms_status_length == 0
	        || (gms_status_length == length
	            && strncmp(buffer_, gms_status_buffer, length)) == 0)
		return;

	/* Set fixed width font to try to preserve status line formatting. */
	glk_set_style(style_Preformatted);

	/* Bracket, and output the status line buffer_. */
	glk_put_string("[ ");
	column = 1;
	for (index = 0; index < gms_status_length; index++) {
		/*
		 * If the character is Tab, position the cursor to eleven characters
		 * shy of the right edge.  In the absence of the real window dimensions,
		 * we'll select 74 characters, which gives us a 78 character status
		 * line; pretty standard.
		 */
		if (gms_status_buffer[index] == '\t') {
			while (column <= GMS_DEFAULT_STATUS_WIDTH - 11) {
				glk_put_char(' ');
				column++;
			}
		} else {
			glk_put_char(gms_status_buffer[index]);
			column++;
		}
	}

	while (column <= GMS_DEFAULT_STATUS_WIDTH) {
		glk_put_char(' ');
		column++;
	}
	glk_put_string(" ]\n");

	/* Save the details of the printed status buffer_. */
	memcpy(buffer_, gms_status_buffer, gms_status_length);
	length = gms_status_length;
}

void Magnetic::gms_status_notify() {
	if (gms_status_window)
		gms_status_update();
	else
		gms_status_print();
}

void Magnetic::gms_status_redraw() {
	if (gms_status_window) {
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
		parent = glk_window_get_parent(gms_status_window);
		glk_window_set_arrangement(parent,
		                                 winmethod_Above | winmethod_Fixed, 1, NULL);

		gms_status_update();
	}
}

/*---------------------------------------------------------------------*/
/*  Glk port output functions                                          */
/*---------------------------------------------------------------------*/

void Magnetic::gms_output_register_help_request() {
	gms_help_requested = true;
}

void Magnetic::gms_output_silence_help_hints() {
	gms_help_hints_silenced = true;
}

void Magnetic::gms_output_provide_help_hint() {
	if (gms_help_requested && !gms_help_hints_silenced) {
		glk_set_style(style_Emphasized);
		glk_put_string("[Try 'glk help' for help on special interpreter"
		                     " commands]\n");

		gms_help_requested = false;
		glk_set_style(style_Normal);
	}
}

int Magnetic::gms_game_prompted() {
	int result;

	result = gms_output_prompt;
	gms_output_prompt = false;

	return result;
}

void Magnetic::gms_detect_game_prompt() {
	int index;

	gms_output_prompt = false;

	/*
	 * Search for a prompt across any last unterminated buffered line; a prompt
	 * is any non-space character on that line.
	 */
	for (index = gms_output_length - 1;
	        index >= 0 && gms_output_buffer[index] != '\n'; index--) {
		if (gms_output_buffer[index] != ' ') {
			gms_output_prompt = true;
			break;
		}
	}
}

void Magnetic::gms_output_delete() {
	free(gms_output_buffer);
	gms_output_buffer = NULL;
	gms_output_allocation = gms_output_length = 0;
}

void Magnetic::gms_output_flush() {
	assert(glk_stream_get_current());

	if (gms_output_length > 0) {
		/*
		 * See if the game issued a standard prompt, then print the buffer to
		 * the main window.  If providing a help hint, position that before
		 * the game's prompt (if any).
		 */
		gms_detect_game_prompt();
		glk_set_style(style_Normal);

		if (gms_output_prompt) {
			int index;

			for (index = gms_output_length - 1;
			        index >= 0 && gms_output_buffer[index] != '\n';)
				index--;

			glk_put_buffer(gms_output_buffer, index + 1);
			gms_output_provide_help_hint();
			glk_put_buffer(gms_output_buffer + index + 1,
			                     gms_output_length - index - 1);
		} else {
			glk_put_buffer(gms_output_buffer, gms_output_length);
			gms_output_provide_help_hint();
		}

		gms_output_delete();
	}
}

void Magnetic::ms_putchar(type8 c) {
	int bytes;
	assert(gms_output_length <= gms_output_allocation);

	/*
	 * See if the character is a backspace.  Magnetic Scrolls games can send
	 * backspace characters to the display.  We'll need to handle such
	 * characters specially, by taking the last character out of the buffer.
	 */
	if (c == '\b') {
		if (gms_output_length > 0)
			gms_output_length--;

		return;
	}

	/* Grow the output buffer if necessary, then add the character. */
	for (bytes = gms_output_allocation; bytes < gms_output_length + 1;)
		bytes = bytes == 0 ? 1 : bytes << 1;

	if (bytes > gms_output_allocation) {
		gms_output_buffer = (char *)gms_realloc(gms_output_buffer, bytes);
		gms_output_allocation = bytes;
	}

	gms_output_buffer[gms_output_length++] = c;
}

void Magnetic::gms_styled_string(glui32 style, const char *message) {
	assert(message);

	glk_set_style(style);
	glk_put_string(message);
	glk_set_style(style_Normal);
}

void Magnetic::gms_styled_char(glui32 style, char c) {
	char str[2];

	str[0] = c;
	str[1] = '\0';
	gms_styled_string(style, str);
}

void Magnetic::gms_standout_string(const char *message) {
	gms_styled_string(style_Emphasized, message);
}

void Magnetic::gms_normal_string(const char *message) {
	gms_styled_string(style_Normal, message);
}

void Magnetic::gms_normal_char(char c) {
	gms_styled_char(style_Normal, c);
}

void Magnetic::gms_header_string(const char *message) {
	gms_styled_string(style_Header, message);
}

void Magnetic::gms_banner_string(const char *message) {
	gms_styled_string(style_Subheader, message);
}

void Magnetic::ms_flush() {
}

/*---------------------------------------------------------------------*/
/*  Glk port hint functions                                            */
/*---------------------------------------------------------------------*/

type16 Magnetic::gms_get_hint_max_node(const struct ms_hint hints_[], type16 node) {
	const struct ms_hint *hint;
	int index;
	type16 max_node;
	assert(hints_);

	hint = hints_ + node;
	max_node = node;

	switch (hint->nodetype) {
	case GMS_HINT_TYPE_TEXT:
		break;

	case GMS_HINT_TYPE_FOLDER:
		/*
		 * Recursively find the maximum node reference for each link, and keep
		 * the largest value found.
		 */
		for (index = 0; index < hint->elcount; index++) {
			type16 link_max;

			link_max = gms_get_hint_max_node(hints_, hint->links[index]);
			if (link_max > max_node)
				max_node = link_max;
		}
		break;

	default:
		gms_fatal("GLK: Invalid hints_ node type encountered");
		glk_exit();
	}

	/*
	 * Return the largest node reference found, capped to avoid overlapping the
	 * special end-hints_ value.
	 */
	return max_node < GMS_HINTS_DONE ? max_node : GMS_HINTS_DONE - 1;
}

const char *Magnetic::gms_get_hint_content(const struct ms_hint hints_[], type16 node, int number) {
	const struct ms_hint *hint;
	int offset, index;
	assert(hints_);

	hint = hints_ + node;

	/* Run through content until 'number' strings found. */
	offset = 0;
	for (index = 0; index < number; index++)
		offset += strlen(hint->content + offset) + 1;

	/* Return the start of the number'th string encountered. */
	return hint->content + offset;
}

const char *Magnetic::gms_get_hint_topic(const ms_hint hints_[], type16 node) {
	assert(hints_);

	if (node == GMS_HINT_ROOT_NODE) {
		/* If the node is the root node, return a generic string. */
		return GMS_GENERIC_TOPIC;
	} else {
		type16 parent;
		int index;
		const char *topic;

		/*
		 * Search the parent for a link to node, and use that as the hint topic;
		 * NULL if none found.
		 */
		parent = hints_[node].parent;

		topic = NULL;
		for (index = 0; index < hints_[parent].elcount; index++) {
			if (hints_[parent].links[index] == node) {
				topic = gms_get_hint_content(hints_, parent, index);
				break;
			}
		}

		return topic ? topic : GMS_GENERIC_TOPIC;
	}
}

int Magnetic::gms_hint_open() {
	if (!gms_hint_menu_window) {
		assert(!gms_hint_text_window);

		/*
		 * Open the hint menu window.  The initial size is two lines, but we'll
		 * change this later to suit the hint.
		 */
		gms_hint_menu_window = glk_window_open(gms_main_window,
		                       winmethod_Above | winmethod_Fixed,
		                       2, wintype_TextGrid, 0);
		if (!gms_hint_menu_window)
			return false;

		/*
		 * Now open the hints text window.  This is set to be 100% of the size
		 * of the main window, so should cover what remains of it completely.
		 */
		gms_hint_text_window = glk_window_open(gms_main_window,
		                       winmethod_Above
		                       | winmethod_Proportional,
		                       100, wintype_TextBuffer, 0);
		if (!gms_hint_text_window) {
			glk_window_close(gms_hint_menu_window, NULL);
			gms_hint_menu_window = NULL;
			return false;
		}
	}

	return true;
}

void Magnetic::Magnetic::gms_hint_close() {
	if (gms_hint_menu_window) {
		assert(gms_hint_text_window);

		glk_window_close(gms_hint_menu_window, NULL);
		gms_hint_menu_window = NULL;
		glk_window_close(gms_hint_text_window, NULL);
		gms_hint_text_window = NULL;
	}
}

int Magnetic::gms_hint_windows_available() {
	return (gms_hint_menu_window && gms_hint_text_window);
}

void Magnetic::gms_hint_menu_print(int line, int column, const char *string_,
		glui32 width, glui32 height) {
	assert(string_);

	/* Ignore the call if the text position is outside the window. */
	if (!(line > (int)height || column > (int)width)) {
		if (gms_hint_windows_available()) {
			int posn, index;

			glk_window_move_cursor(gms_hint_menu_window, column, line);
			glk_set_window(gms_hint_menu_window);

			/* Write until the end of the string_, or the end of the window. */
			for (posn = column, index = 0;
			        posn < (int)width && index < (int)strlen(string_); posn++, index++) {
				glk_put_char(string_[index]);
			}

			glk_set_window(gms_main_window);
		} else {
			static int current_line = 0;    /* Retained line number */
			static int current_column = 0;  /* Retained col number */

			int index;

			/*
			 * Check the line number against the last one output.  If it is less,
			 * assume the start of a new block.  In this case, perform a hokey
			 * type of screen clear.
			 */
			if (line < current_line) {
				for (index = 0; index < (int)height; index++)
					gms_normal_char('\n');

				current_line = 0;
				current_column = 0;
			}

			/* Print blank lines until the target line is reached. */
			for (; current_line < line; current_line++) {
				gms_normal_char('\n');
				current_column = 0;
			}

			/* Now print spaces until the target column is reached. */
			for (; current_column < column; current_column++)
				gms_normal_char(' ');

			/*
			 * Write characters until the end of the string_, or the end of the
			 * (self-imposed not-really-there) window.
			 */
			for (index = 0;
			        current_column < (int)width && index < (int)strlen(string_);
			        current_column++, index++) {
				gms_normal_char(string_[index]);
			}
		}
	}
}

void Magnetic::gms_hint_menu_header(int line, const char *string_,
		glui32 width, glui32 height) {
	int posn, length;
	assert(string_);

	/* Output the text in the approximate line center. */
	length = strlen(string_);
	posn = length < (int)width ? (width - length) / 2 : 0;
	gms_hint_menu_print(line, posn, string_, width, height);
}

void Magnetic::gms_hint_menu_justify(int line, const char *left_string,
		const char *right_string, glui32 width, glui32 height) {
	int posn, length;
	assert(left_string && right_string);

	/* Write left text normally to window left. */
	gms_hint_menu_print(line, 0, left_string, width, height);

	/* Output the right text flush with the right of the window. */
	length = strlen(right_string);
	posn = length < (int)width ? width - length : 0;
	gms_hint_menu_print(line, posn, right_string, width, height);
}

void Magnetic::gms_hint_text_print(const char *string_) {
	assert(string_);

	if (gms_hint_windows_available()) {
		glk_set_window(gms_hint_text_window);
		glk_put_string(string_);
		glk_set_window(gms_main_window);
	} else
		gms_normal_string(string_);
}

void Magnetic::gms_hint_menutext_start() {
	/*
	 * Twiddle for non-windowing libraries; 'clear' the main window by writing
	 * a null string at line 1, then a null string at line 0.  This works
	 * because we know the current output line in gms_hint_menu_print() is zero,
	 * since we set it that way with gms_hint_menutext_done(), or if this is
	 * the first call, then that's its initial value.
	 */
	if (!gms_hint_windows_available()) {
		gms_hint_menu_print(1, 0, "",
		                    GMS_HINT_DEFAULT_WIDTH, GMS_HINT_DEFAULT_HEIGHT);
		gms_hint_menu_print(0, 0, "",
		                    GMS_HINT_DEFAULT_WIDTH, GMS_HINT_DEFAULT_HEIGHT);
	}
}

void Magnetic::gms_hint_menutext_done() {
	/*
	 * Twiddle for non-windowing libraries; 'clear' the main window by writing
	 * an empty string to line zero.  For windowing Glk libraries, this function
	 * does nothing.
	 */
	if (!gms_hint_windows_available()) {
		gms_hint_menu_print(0, 0, "",
		                    GMS_HINT_DEFAULT_WIDTH, GMS_HINT_DEFAULT_HEIGHT);
	}
}

void Magnetic::gms_hint_menutext_char_event(event_t *event) {
	assert(event);

	if (gms_hint_windows_available()) {
		glk_request_char_event(gms_hint_menu_window);
		glk_request_char_event(gms_hint_text_window);

		gms_event_wait(evtype_CharInput, event);
		assert(event->window == gms_hint_menu_window
		       || event->window == gms_hint_text_window);

		glk_cancel_char_event(gms_hint_menu_window);
		glk_cancel_char_event(gms_hint_text_window);
	} else {
		glk_request_char_event(gms_main_window);
		gms_event_wait(evtype_CharInput, event);
	}
}

void Magnetic::gms_hint_arrange_windows(int requested_lines, glui32 *width, glui32 *height) {
	if (gms_hint_windows_available()) {
		winid_t parent;

		/* Resize the hint menu window to fit the current hint. */
		parent = glk_window_get_parent(gms_hint_menu_window);
		glk_window_set_arrangement(parent,
		                                 winmethod_Above | winmethod_Fixed,
		                                 requested_lines, NULL);

		uint width_temp, height_temp;

		/* Measure, and return the size of the hint menu window. */
		glk_window_get_size(gms_hint_menu_window, &width_temp, &height_temp);

		*width = width_temp;
		*height = height_temp;

		/* Clear both the hint menu and the hint text window. */
		glk_window_clear(gms_hint_menu_window);
		glk_window_clear(gms_hint_text_window);
	} else {
		/*
		 * No hints windows, so default width and height.  The hints output
		 * functions will cope with this.
		 */
		if (width)
			*width = GMS_HINT_DEFAULT_WIDTH;
		if (height)
			*height = GMS_HINT_DEFAULT_HEIGHT;
	}
}

void Magnetic::gms_hint_display_folder(const ms_hint hints_[],
		const int cursor[], type16 node) {
	glui32 width, height;
	int line, index;
	assert(hints_ && cursor);

	/*
	 * Arrange windows to suit the hint folder.  For a folder menu window we
	 * use one line for each element, three for the controls, and two spacers,
	 * making a total of five additional lines.  Width and height receive the
	 * actual menu window dimensions.
	 */
	gms_hint_arrange_windows(hints_[node].elcount + 5, &width, &height);

	/* Paint in the menu header. */
	line = 0;
	gms_hint_menu_header(line++,
	                     gms_get_hint_topic(hints_, node),
	                     width, height);
	gms_hint_menu_justify(line++,
	                      " N = next subject  ", "  P = previous ",
	                      width, height);
	gms_hint_menu_justify(line++,
	                      " RETURN = read subject  ",
	                      node == GMS_HINT_ROOT_NODE
	                      ? "  Q = resume game " : "  Q = previous menu ",
	                      width, height);

	/*
	 * Output a blank line, then the menu for the node's folder hint.  The folder
	 * text for the selected hint is preceded by a '>' pointer.
	 */
	line++;
	for (index = 0; index < hints_[node].elcount; index++) {
		gms_hint_menu_print(line, 3,
		                    index == cursor[node] ? ">" : " ",
		                    width, height);
		gms_hint_menu_print(line++, 5,
		                    gms_get_hint_content(hints_, node, index),
		                    width, height);
	}

	/*
	 * Terminate with a blank line; using a single space here improves cursor
	 * positioning for optimized output libraries (for example, without it,
	 * curses output will leave the cursor at the end of the previous line).
	 */
	gms_hint_menu_print(line, 0, " ", width, height);
}

void Magnetic::gms_hint_display_text(const ms_hint hints_[],
		const int cursor[], type16 node) {
	glui32 width, height;
	int line, index;
	assert(hints_ && cursor);

	/*
	 * Arrange windows to suit the hint text.  For a hint menu, we use a simple
	 * two-line set of controls; everything else is in the hints_ text window.
	 * Width and height receive the actual menu window dimensions.
	 */
	gms_hint_arrange_windows(2, &width, &height);

	/* Paint in a short menu header. */
	line = 0;
	gms_hint_menu_header(line++,
	                     gms_get_hint_topic(hints_, node),
	                     width, height);
	gms_hint_menu_justify(line++,
	                      " RETURN = read hint  ", "  Q = previous menu ",
	                      width, height);

	/*
	 * Output hints_ to the hints_ text window.  hints_ not yet exposed are
	 * indicated by the cursor for the hint, and are displayed as a dash.
	 */
	gms_hint_text_print("\n");
	for (index = 0; index < hints_[node].elcount; index++) {
		char buf[16];

		sprintf(buf, "%3d.  ", index + 1);
		gms_hint_text_print(buf);

		gms_hint_text_print(index < cursor[node]
		                    ? gms_get_hint_content(hints_, node, index) : "-");
		gms_hint_text_print("\n");
	}
}

void Magnetic::gms_hint_display(const ms_hint hints_[], const int cursor[], type16 node) {
	assert(hints_ && cursor);

	switch (hints_[node].nodetype) {
	case GMS_HINT_TYPE_TEXT:
		gms_hint_display_text(hints_, cursor, node);
		break;

	case GMS_HINT_TYPE_FOLDER:
		gms_hint_display_folder(hints_, cursor, node);
		break;

	default:
		gms_fatal("GLK: Invalid hints_ node type encountered");
		glk_exit();
	}
}

type16 Magnetic::gms_hint_handle_folder(const ms_hint hints_[],
		int cursor[], type16 node, glui32 keycode) {
	unsigned char response;
	type16 next_node;
	assert(hints_ && cursor);

	/* Convert key code into a single response character. */
	switch (keycode) {
	case keycode_Down:
		response = 'N';
		break;
	case keycode_Up:
		response = 'P';
		break;
	case keycode_Right:
	case keycode_Return:
		response = '\n';
		break;
	case keycode_Left:
	case keycode_Escape:
		response = 'Q';
		break;
	default:
		response = keycode <= BYTE_MAX_VAL ? glk_char_to_upper(keycode) : 0;
		break;
	}

	/*
	 * Now handle the response character.  We'll default the next node to be
	 * this node, but a response case can change it.
	 */
	next_node = node;
	switch (response) {
	case 'N':
		/* Advance the hint cursor, wrapping at the folder end. */
		if (cursor[node] < hints_[node].elcount - 1)
			cursor[node]++;
		else
			cursor[node] = 0;
		break;

	case 'P':
		/* Regress the hint cursor, wrapping at the folder start. */
		if (cursor[node] > 0)
			cursor[node]--;
		else
			cursor[node] = hints_[node].elcount - 1;
		break;

	case '\n':
		/* The next node is the hint node at the cursor position. */
		next_node = hints_[node].links[cursor[node]];
		break;

	case 'Q':
		/* If root, we're done; if not, next node is node's parent. */
		next_node = node == GMS_HINT_ROOT_NODE
		            ? GMS_HINTS_DONE : hints_[node].parent;
		break;

	default:
		break;
	}

	return next_node;
}

type16 Magnetic::gms_hint_handle_text(const ms_hint hints_[],
		int cursor[], type16 node, glui32 keycode) {
	unsigned char response;
	type16 next_node;
	assert(hints_ && cursor);

	/* Convert key code into a single response character. */
	switch (keycode) {
	case keycode_Right:
	case keycode_Return:
		response = '\n';
		break;
	case keycode_Left:
	case keycode_Escape:
		response = 'Q';
		break;
	default:
		response = keycode <= BYTE_MAX_VAL ? glk_char_to_upper(keycode) : 0;
		break;
	}

	/*
	 * Now handle the response character.  We'll default the next node to be
	 * this node, but a response case can change it.
	 */
	next_node = node;
	switch (response) {
	case '\n':
		/* If not at end of the hint, advance the hint cursor. */
		if (cursor[node] < hints_[node].elcount)
			cursor[node]++;
		break;

	case 'Q':
		/* Done with this hint node, so next node is its parent. */
		next_node = hints_[node].parent;
		break;

	default:
		break;
	}

	return next_node;
}

type16 Magnetic::gms_hint_handle(const ms_hint hints_[],
		int cursor[], type16 node, glui32 keycode) {
	type16 next_node;
	assert(hints_ && cursor);

	next_node = GMS_HINT_ROOT_NODE;
	switch (hints_[node].nodetype) {
	case GMS_HINT_TYPE_TEXT:
		next_node = gms_hint_handle_text(hints_, cursor, node, keycode);
		break;

	case GMS_HINT_TYPE_FOLDER:
		next_node = gms_hint_handle_folder(hints_, cursor, node, keycode);
		break;

	default:
		gms_fatal("GLK: Invalid hints_ node type encountered");
		glk_exit();
	}

	return next_node;
}

type8 Magnetic::ms_showhints(ms_hint *hints_) {
	type16 hint_count;
	glui32 crc;
	assert(hints_);

	/*
	 * Find the number of hints_ in the array.  To do this, we'll visit every
	 * node in a tree search, starting at the root, to locate the maximum node
	 * number found, then add one to that.  It's a pity that the interpreter
	 * doesn't hand us this information directly.
	 */
	hint_count = gms_get_hint_max_node(hints_, GMS_HINT_ROOT_NODE) + 1;

	/*
	 * Calculate a CRC for the hints_ array data.  If the CRC has changed, or
	 * this is the first call, assign a new cursor array.
	 */
	crc = gms_get_buffer_crc(hints_, hint_count * sizeof(*hints_));
	if (crc != hints_current_crc || !hints_crc_initialized) {
		int bytes;

		/* Allocate new cursors, and set all to zero initial state. */
		free(gms_hint_cursor);
		bytes = hint_count * sizeof(*gms_hint_cursor);
		gms_hint_cursor = (int *)gms_malloc(bytes);
		memset(gms_hint_cursor, 0, bytes);

		/*
		 * Retain the hints_ CRC, for later comparisons, and set is_initialized
		 * flag.
		 */
		hints_current_crc = crc;
		hints_crc_initialized = true;
	}

	/*
	 * Save the hints_ array passed in.  This is done here since even if the data
	 * remains the same (found by the CRC check above), the pointer to it might
	 * have changed.
	 */
	gms_hints = hints_;

	/*
	 * Try to create the hints_ windows.  If they can't be created, perhaps
	 * because the Glk library doesn't support it, the output functions will
	 * work around this.
	 */
	gms_hint_open();
	gms_hint_menutext_start();

	/*
	 * Begin hints_ display at the root node, and navigate until the user exits
	 * hints_.
	 */
	gms_current_hint_node = GMS_HINT_ROOT_NODE;
	while (gms_current_hint_node != GMS_HINTS_DONE) {
		event_t event;

		assert(gms_current_hint_node < hint_count);
		gms_hint_display(gms_hints, gms_hint_cursor, gms_current_hint_node);

		/* Get and handle a character key event for hint navigation. */
		gms_hint_menutext_char_event(&event);
		assert(event.type == evtype_CharInput);
		gms_current_hint_node = gms_hint_handle(gms_hints,
		                                        gms_hint_cursor,
		                                        gms_current_hint_node,
		                                        event.val1);
	}

	/* Done with hint windows. */
	gms_hint_menutext_done();
	gms_hint_close();

	return GMS_HINT_SUCCESS;
}

void Magnetic::gms_hint_redraw() {
	if (gms_hint_windows_available()) {
		assert(gms_hints && gms_hint_cursor);
		gms_hint_display(gms_hints, gms_hint_cursor, gms_current_hint_node);
	}
}

void Magnetic::gms_hints_cleanup() {
	free(gms_hint_cursor);
	gms_hint_cursor = NULL;

	gms_hints = NULL;
	gms_current_hint_node = 0;
}

void Magnetic::ms_playmusic(type8 *midi_data, type32 length, type16 tempo) {
}

/*---------------------------------------------------------------------*/
/*  Glk command escape functions                                       */
/*---------------------------------------------------------------------*/

void Magnetic::gms_command_undo(const char *argument) {
	assert(argument);
}

void Magnetic::gms_command_script(const char *argument) {
	assert(argument);

	if (gms_strcasecmp(argument, "on") == 0) {
		frefid_t fileref;

		if (gms_transcript_stream) {
			gms_normal_string("Glk transcript is already on.\n");
			return;
		}

		fileref = glk_fileref_create_by_prompt(fileusage_Transcript
		          | fileusage_TextMode,
		          filemode_WriteAppend, 0);
		if (!fileref) {
			gms_standout_string("Glk transcript failed.\n");
			return;
		}

		gms_transcript_stream = glk_stream_open_file(fileref,
		                        filemode_WriteAppend, 0);
		glk_fileref_destroy(fileref);
		if (!gms_transcript_stream) {
			gms_standout_string("Glk transcript failed.\n");
			return;
		}

		glk_window_set_echo_stream(gms_main_window, gms_transcript_stream);

		gms_normal_string("Glk transcript is now on.\n");
	}

	else if (gms_strcasecmp(argument, "off") == 0) {
		if (!gms_transcript_stream) {
			gms_normal_string("Glk transcript is already off.\n");
			return;
		}

		glk_stream_close(gms_transcript_stream, NULL);
		gms_transcript_stream = NULL;

		glk_window_set_echo_stream(gms_main_window, NULL);

		gms_normal_string("Glk transcript is now off.\n");
	}

	else if (strlen(argument) == 0) {
		gms_normal_string("Glk transcript is ");
		gms_normal_string(gms_transcript_stream ? "on" : "off");
		gms_normal_string(".\n");
	}

	else {
		gms_normal_string("Glk transcript can be ");
		gms_standout_string("on");
		gms_normal_string(", or ");
		gms_standout_string("off");
		gms_normal_string(".\n");
	}
}

void Magnetic::gms_command_inputlog(const char *argument) {
	assert(argument);

	if (gms_strcasecmp(argument, "on") == 0) {
		frefid_t fileref;

		if (gms_inputlog_stream) {
			gms_normal_string("Glk input logging is already on.\n");
			return;
		}

		fileref = glk_fileref_create_by_prompt(fileusage_InputRecord
		          | fileusage_BinaryMode,
		          filemode_WriteAppend, 0);
		if (!fileref) {
			gms_standout_string("Glk input logging failed.\n");
			return;
		}

		gms_inputlog_stream = glk_stream_open_file(fileref,
		                      filemode_WriteAppend, 0);
		glk_fileref_destroy(fileref);
		if (!gms_inputlog_stream) {
			gms_standout_string("Glk input logging failed.\n");
			return;
		}

		gms_normal_string("Glk input logging is now on.\n");
	}

	else if (gms_strcasecmp(argument, "off") == 0) {
		if (!gms_inputlog_stream) {
			gms_normal_string("Glk input logging is already off.\n");
			return;
		}

		glk_stream_close(gms_inputlog_stream, NULL);
		gms_inputlog_stream = NULL;

		gms_normal_string("Glk input log is now off.\n");
	}

	else if (strlen(argument) == 0) {
		gms_normal_string("Glk input logging is ");
		gms_normal_string(gms_inputlog_stream ? "on" : "off");
		gms_normal_string(".\n");
	}

	else {
		gms_normal_string("Glk input logging can be ");
		gms_standout_string("on");
		gms_normal_string(", or ");
		gms_standout_string("off");
		gms_normal_string(".\n");
	}
}

void Magnetic::gms_command_readlog(const char *argument) {
	assert(argument);

	if (gms_strcasecmp(argument, "on") == 0) {
		frefid_t fileref;

		if (gms_readlog_stream) {
			gms_normal_string("Glk read log is already on.\n");
			return;
		}

		fileref = glk_fileref_create_by_prompt(fileusage_InputRecord
		          | fileusage_BinaryMode,
		          filemode_Read, 0);
		if (!fileref) {
			gms_standout_string("Glk read log failed.\n");
			return;
		}

		if (!glk_fileref_does_file_exist(fileref)) {
			glk_fileref_destroy(fileref);
			gms_standout_string("Glk read log failed.\n");
			return;
		}

		gms_readlog_stream = glk_stream_open_file(fileref, filemode_Read, 0);
		glk_fileref_destroy(fileref);
		if (!gms_readlog_stream) {
			gms_standout_string("Glk read log failed.\n");
			return;
		}

		gms_normal_string("Glk read log is now on.\n");
	}

	else if (gms_strcasecmp(argument, "off") == 0) {
		if (!gms_readlog_stream) {
			gms_normal_string("Glk read log is already off.\n");
			return;
		}

		glk_stream_close(gms_readlog_stream, NULL);
		gms_readlog_stream = NULL;

		gms_normal_string("Glk read log is now off.\n");
	}

	else if (strlen(argument) == 0) {
		gms_normal_string("Glk read log is ");
		gms_normal_string(gms_readlog_stream ? "on" : "off");
		gms_normal_string(".\n");
	}

	else {
		gms_normal_string("Glk read log can be ");
		gms_standout_string("on");
		gms_normal_string(", or ");
		gms_standout_string("off");
		gms_normal_string(".\n");
	}
}

void Magnetic::gms_command_abbreviations(const char *argument) {
	assert(argument);

	if (gms_strcasecmp(argument, "on") == 0) {
		if (gms_abbreviations_enabled) {
			gms_normal_string("Glk abbreviation expansions are already on.\n");
			return;
		}

		gms_abbreviations_enabled = true;
		gms_normal_string("Glk abbreviation expansions are now on.\n");
	}

	else if (gms_strcasecmp(argument, "off") == 0) {
		if (!gms_abbreviations_enabled) {
			gms_normal_string("Glk abbreviation expansions are already off.\n");
			return;
		}

		gms_abbreviations_enabled = false;
		gms_normal_string("Glk abbreviation expansions are now off.\n");
	}

	else if (strlen(argument) == 0) {
		gms_normal_string("Glk abbreviation expansions are ");
		gms_normal_string(gms_abbreviations_enabled ? "on" : "off");
		gms_normal_string(".\n");
	}

	else {
		gms_normal_string("Glk abbreviation expansions can be ");
		gms_standout_string("on");
		gms_normal_string(", or ");
		gms_standout_string("off");
		gms_normal_string(".\n");
	}
}

void Magnetic::gms_command_graphics(const char *argument) {
	assert(argument);

	if (!gms_graphics_possible) {
		gms_normal_string("Glk graphics are not available.\n");
		return;
	}

	if (gms_strcasecmp(argument, "on") == 0) {
		if (gms_graphics_enabled) {
			gms_normal_string("Glk graphics are already on.\n");
			return;
		}

		gms_graphics_enabled = true;

		/* If a picture is loaded, call the restart function to repaint it. */
		if (gms_graphics_picture_is_available()) {
			if (!gms_graphics_open()) {
				gms_normal_string("Glk graphics error.\n");
				return;
			}
			gms_graphics_restart();
		}

		gms_normal_string("Glk graphics are now on.\n");
	}

	else if (gms_strcasecmp(argument, "off") == 0) {
		if (!gms_graphics_enabled) {
			gms_normal_string("Glk graphics are already off.\n");
			return;
		}

		/*
		 * Set graphics to disabled, and stop any graphics processing.  Close
		 * the graphics window.
		 */
		gms_graphics_enabled = false;
		gms_graphics_stop();
		gms_graphics_close();

		gms_normal_string("Glk graphics are now off.\n");
	}

	else if (strlen(argument) == 0) {
		gms_normal_string("Glk graphics are available,");
		gms_normal_string(gms_graphics_enabled
		                  ? " and enabled.\n" : " but disabled.\n");

		if (gms_graphics_picture_is_available()) {
			int width, height, is_animated;

			if (gms_graphics_get_picture_details(&width, &height, &is_animated)) {
				char buf[16];

				gms_normal_string("There is ");
				gms_normal_string(is_animated ? "an animated" : "a");
				gms_normal_string(" picture loaded, ");

				sprintf(buf, "%d", width);
				gms_normal_string(buf);
				gms_normal_string(" by ");

				sprintf(buf, "%d", height);
				gms_normal_string(buf);

				gms_normal_string(" pixels.\n");
			}
		}

		if (!gms_graphics_interpreter_enabled())
			gms_normal_string("Interpreter graphics are disabled.\n");

		if (gms_graphics_enabled && gms_graphics_are_displayed()) {
			int color_count, is_active;
			const char *gamma;

			if (gms_graphics_get_rendering_details(&gamma, &color_count,
			                                       &is_active)) {
				char buf[16];

				gms_normal_string("Graphics are ");
				gms_normal_string(is_active ? "active, " : "displayed, ");

				sprintf(buf, "%d", color_count);
				gms_normal_string(buf);
				gms_normal_string(" colours");

				if (gms_gamma_mode == GAMMA_OFF)
					gms_normal_string(", without gamma correction");
				else {
					gms_normal_string(", with gamma ");
					gms_normal_string(gamma);
					gms_normal_string(" correction");
				}
				gms_normal_string(".\n");
			} else
				gms_normal_string("Graphics are being displayed.\n");
		}

		if (gms_graphics_enabled && !gms_graphics_are_displayed())
			gms_normal_string("Graphics are not being displayed.\n");
	}

	else {
		gms_normal_string("Glk graphics can be ");
		gms_standout_string("on");
		gms_normal_string(", or ");
		gms_standout_string("off");
		gms_normal_string(".\n");
	}
}

void Magnetic::gms_command_gamma(const char *argument) {
	assert(argument);

	if (!gms_graphics_possible) {
		gms_normal_string("Glk automatic gamma correction is not available.\n");
		return;
	}

	if (gms_strcasecmp(argument, "high") == 0) {
		if (gms_gamma_mode == GAMMA_HIGH) {
			gms_normal_string("Glk automatic gamma correction mode is"
			                  " already 'high'.\n");
			return;
		}

		gms_gamma_mode = GAMMA_HIGH;
		gms_graphics_restart();

		gms_normal_string("Glk automatic gamma correction mode is"
		                  " now 'high'.\n");
	}

	else if (gms_strcasecmp(argument, "normal") == 0
	         || gms_strcasecmp(argument, "on") == 0) {
		if (gms_gamma_mode == GAMMA_NORMAL) {
			gms_normal_string("Glk automatic gamma correction mode is"
			                  " already 'normal'.\n");
			return;
		}

		gms_gamma_mode = GAMMA_NORMAL;
		gms_graphics_restart();

		gms_normal_string("Glk automatic gamma correction mode is"
		                  " now 'normal'.\n");
	}

	else if (gms_strcasecmp(argument, "none") == 0
	         || gms_strcasecmp(argument, "off") == 0) {
		if (gms_gamma_mode == GAMMA_OFF) {
			gms_normal_string("Glk automatic gamma correction mode is"
			                  " already 'off'.\n");
			return;
		}

		gms_gamma_mode = GAMMA_OFF;
		gms_graphics_restart();

		gms_normal_string("Glk automatic gamma correction mode is"
		                  " now 'off'.\n");
	}

	else if (strlen(argument) == 0) {
		gms_normal_string("Glk automatic gamma correction mode is '");
		switch (gms_gamma_mode) {
		case GAMMA_OFF:
		default:
			gms_normal_string("off");
			break;
		case GAMMA_NORMAL:
			gms_normal_string("normal");
			break;
		case GAMMA_HIGH:
			gms_normal_string("high");
			break;
		}
		gms_normal_string("'.\n");
	}

	else {
		gms_normal_string("Glk automatic gamma correction mode can be ");
		gms_standout_string("high");
		gms_normal_string(", ");
		gms_standout_string("normal");
		gms_normal_string(", or ");
		gms_standout_string("off");
		gms_normal_string(".\n");
	}
}

void Magnetic::gms_command_animations(const char *argument) {
	assert(argument);

	if (!gms_graphics_possible) {
		gms_normal_string("Glk graphics animations are not available.\n");
		return;
	}

	if (gms_strcasecmp(argument, "on") == 0) {
		int is_animated;

		if (gms_animation_enabled) {
			gms_normal_string("Glk graphics animations are already on.\n");
			return;
		}

		/*
		 * Set animation to on, and restart graphics if the current picture
		 * is animated; if it isn't, we can leave it displayed as is, since
		 * changing animation mode doesn't affect this picture.
		 */
		gms_animation_enabled = true;
		if (gms_graphics_get_picture_details(NULL, NULL, &is_animated)) {
			if (is_animated)
				gms_graphics_restart();
		}

		gms_normal_string("Glk graphics animations are now on.\n");
	}

	else if (gms_strcasecmp(argument, "off") == 0) {
		int is_animated;

		if (!gms_animation_enabled) {
			gms_normal_string("Glk graphics animations are already off.\n");
			return;
		}

		gms_animation_enabled = false;
		if (gms_graphics_get_picture_details(NULL, NULL, &is_animated)) {
			if (is_animated)
				gms_graphics_restart();
		}

		gms_normal_string("Glk graphics animations are now off.\n");
	}

	else if (strlen(argument) == 0) {
		gms_normal_string("Glk graphics animations are ");
		gms_normal_string(gms_animation_enabled ? "on" : "off");
		gms_normal_string(".\n");
	}

	else {
		gms_normal_string("Glk graphics animations can be ");
		gms_standout_string("on");
		gms_normal_string(", or ");
		gms_standout_string("off");
		gms_normal_string(".\n");
	}
}

void Magnetic::gms_command_prompts(const char *argument) {
	assert(argument);

	if (gms_strcasecmp(argument, "on") == 0) {
		if (gms_prompt_enabled) {
			gms_normal_string("Glk extra prompts are already on.\n");
			return;
		}

		gms_prompt_enabled = true;
		gms_normal_string("Glk extra prompts are now on.\n");

		/* Check for a game prompt to clear the flag. */
		gms_game_prompted();
	}

	else if (gms_strcasecmp(argument, "off") == 0) {
		if (!gms_prompt_enabled) {
			gms_normal_string("Glk extra prompts are already off.\n");
			return;
		}

		gms_prompt_enabled = false;
		gms_normal_string("Glk extra prompts are now off.\n");
	}

	else if (strlen(argument) == 0) {
		gms_normal_string("Glk extra prompts are ");
		gms_normal_string(gms_prompt_enabled ? "on" : "off");
		gms_normal_string(".\n");
	}

	else {
		gms_normal_string("Glk extra prompts can be ");
		gms_standout_string("on");
		gms_normal_string(", or ");
		gms_standout_string("off");
		gms_normal_string(".\n");
	}
}

void Magnetic::gms_command_print_version_number(glui32 version_) {
	Common::String str = Common::String::format("%lu.%lu.%lu",
	        (unsigned long)version_ >> 16,
	        (unsigned long)(version_ >> 8) & 0xff,
	        (unsigned long)version_ & 0xff);
	gms_normal_string(str.c_str());
}

void Magnetic::gms_command_version(const char *argument) {
	glui32 version_;
	assert(argument);

	gms_normal_string("This is version_ ");
	gms_command_print_version_number(GMS_PORT_VERSION);
	gms_normal_string(" of the Glk Magnetic port.\n");

	version_ = glk_gestalt(gestalt_Version, 0);
	gms_normal_string("The Glk library version_ is ");
	gms_command_print_version_number(version_);
	gms_normal_string(".\n");
}

void Magnetic::gms_command_commands(const char *argument) {
	assert(argument);

	if (gms_strcasecmp(argument, "on") == 0) {
		gms_normal_string("Glk commands are already on.\n");
	}

	else if (gms_strcasecmp(argument, "off") == 0) {
		gms_commands_enabled = false;
		gms_normal_string("Glk commands are now off.\n");
	}

	else if (strlen(argument) == 0) {
		gms_normal_string("Glk commands are ");
		gms_normal_string(gms_commands_enabled ? "on" : "off");
		gms_normal_string(".\n");
	}

	else {
		gms_normal_string("Glk commands can be ");
		gms_standout_string("on");
		gms_normal_string(", or ");
		gms_standout_string("off");
		gms_normal_string(".\n");
	}
}

void Magnetic::gms_command_summary(const char *argument) {
	const gms_command_t *entry;
	assert(argument);

	/*
	 * Call handlers that have status to report with an empty argument,
	 * prompting each to print its current setting.
	 */
	for (entry = GMS_COMMAND_TABLE; entry->command; entry++) {
		if (entry->handler == &Magnetic::gms_command_summary
		        || entry->handler == &Magnetic::gms_command_undo
		        || entry->handler == &Magnetic::gms_command_help)
			continue;

		(this->*entry->handler)("");
	}
}

void Magnetic::gms_command_help(const char *command) {
	const gms_command_t *entry, *matched;
	assert(command);

	if (strlen(command) == 0) {
		gms_normal_string("Glk commands are");
		for (entry = GMS_COMMAND_TABLE; entry->command; entry++) {
			const gms_command_t *next;

			next = entry + 1;
			gms_normal_string(next->command ? " " : " and ");
			gms_standout_string(entry->command);
			gms_normal_string(next->command ? "," : ".\n\n");
		}

		gms_normal_string("Glk commands may be abbreviated, as long as"
		                  " the abbreviation is unambiguous.  Use ");
		gms_standout_string("glk help");
		gms_normal_string(" followed by a Glk command name for help on that"
		                  " command.\n");
		return;
	}

	matched = NULL;
	for (entry = GMS_COMMAND_TABLE; entry->command; entry++) {
		if (gms_strncasecmp(command, entry->command, strlen(command)) == 0) {
			if (matched) {
				gms_normal_string("The Glk command ");
				gms_standout_string(command);
				gms_normal_string(" is ambiguous.  Try ");
				gms_standout_string("glk help");
				gms_normal_string(" for more information.\n");
				return;
			}
			matched = entry;
		}
	}
	if (!matched) {
		gms_normal_string("The Glk command ");
		gms_standout_string(command);
		gms_normal_string(" is not valid.  Try ");
		gms_standout_string("glk help");
		gms_normal_string(" for more information.\n");
		return;
	}

	if (matched->handler == &Magnetic::gms_command_summary) {
		gms_normal_string("Prints a summary of all the current Glk Magnetic"
		                  " settings.\n");
	}

	else if (matched->handler == &Magnetic::gms_command_undo) {
		gms_normal_string("Undoes a single game turn.\n\nEquivalent to the"
		                  " standalone game 'undo' command.\n");
	}

	else if (matched->handler == &Magnetic::gms_command_script) {
		gms_normal_string("Logs the game's output to a file.\n\nUse ");
		gms_standout_string("glk script on");
		gms_normal_string(" to begin logging game output, and ");
		gms_standout_string("glk script off");
		gms_normal_string(" to end it.  Glk Magnetic will ask you for a file"
		                  " when you turn scripts on.\n");
	}

	else if (matched->handler == &Magnetic::gms_command_inputlog) {
		gms_normal_string("Records the commands you type into a game.\n\nUse ");
		gms_standout_string("glk inputlog on");
		gms_normal_string(", to begin recording your commands, and ");
		gms_standout_string("glk inputlog off");
		gms_normal_string(" to turn off input logs.  You can play back"
		                  " recorded commands into a game with the ");
		gms_standout_string("glk readlog");
		gms_normal_string(" command.\n");
	}

	else if (matched->handler == &Magnetic::gms_command_readlog) {
		gms_normal_string("Plays back commands recorded with ");
		gms_standout_string("glk inputlog on");
		gms_normal_string(".\n\nUse ");
		gms_standout_string("glk readlog on");
		gms_normal_string(".  Command play back stops at the end of the"
		                  " file.  You can also play back commands from a"
		                  " text file created using any standard editor.\n");
	}

	else if (matched->handler == &Magnetic::gms_command_abbreviations) {
		gms_normal_string("Controls abbreviation expansion.\n\nGlk Magnetic"
		                  " automatically expands several standard single"
		                  " letter abbreviations for you; for example, \"x\""
		                  " becomes \"examine\".  Use ");
		gms_standout_string("glk abbreviations on");
		gms_normal_string(" to turn this feature on, and ");
		gms_standout_string("glk abbreviations off");
		gms_normal_string(" to turn it off.  While the feature is on, you"
		                  " can bypass abbreviation expansion for an"
		                  " individual game command by prefixing it with a"
		                  " single quote.\n");
	}

	else if (matched->handler == &Magnetic::gms_command_graphics) {
		gms_normal_string("Turns interpreter graphics on and off.\n\nUse ");
		gms_standout_string("glk graphics on");
		gms_normal_string(" to enable interpreter graphics, and ");
		gms_standout_string("glk graphics off");
		gms_normal_string(" to turn graphics off and close the graphics window."
		                  "  This control works slightly differently to the"
		                  " 'graphics' command in Magnetic Windows and Magnetic"
		                  " Scrolls games themselves; the game's 'graphics'"
		                  " command may disable new images, but leave old ones"
		                  " displayed.  For graphics to be displayed, they"
		                  " must be turned on in both the game and the"
		                  " interpreter.\n");
	}

	else if (matched->handler == &Magnetic::gms_command_gamma) {
		gms_normal_string("Sets the level of automatic gamma correction applied"
		                  " to game graphics.\n\nUse ");
		gms_standout_string("glk gamma normal");
		gms_normal_string(" to set moderate automatic colour contrast"
		                  " correction, ");
		gms_standout_string("glk gamma high");
		gms_normal_string(" to set high automatic colour contrast correction,"
		                  " or ");
		gms_standout_string("glk gamma off");
		gms_normal_string(" to turn off all automatic gamma correction.\n");
	}

	else if (matched->handler == &Magnetic::gms_command_animations) {
		gms_normal_string("Turns graphic animations on and off.\n\nUse ");
		gms_standout_string("glk animation on");
		gms_normal_string(" to enable animations, or ");
		gms_standout_string("glk animation off");
		gms_normal_string(" to turn animations off.  Not all game graphics are"
		                  " animated, so this control works only on graphics"
		                  " that are animated.  When animation is off, Glk"
		                  " Magnetic displays only the static portions of a"
		                  " game's pictures.\n");
	}

	else if (matched->handler == &Magnetic::gms_command_prompts) {
		gms_normal_string("Controls extra input prompting.\n\n"
		                  "Glk Magnetic can issue a replacement '>' input"
		                  " prompt if it detects that the game hasn't prompted"
		                  " after, say, an empty input line.  Use ");
		gms_standout_string("glk prompts on");
		gms_normal_string(" to turn this feature on, and ");
		gms_standout_string("glk prompts off");
		gms_normal_string(" to turn it off.\n");
	}

	else if (matched->handler == &Magnetic::gms_command_version) {
		gms_normal_string("Prints the version numbers of the Glk library"
		                  " and the Glk Magnetic port.\n");
	}

	else if (matched->handler == &Magnetic::gms_command_commands) {
		gms_normal_string("Turn off Glk commands.\n\nUse ");
		gms_standout_string("glk commands off");
		gms_normal_string(" to disable all Glk commands, including this one."
		                  "  Once turned off, there is no way to turn Glk"
		                  " commands back on while inside the game.\n");
	}

	else if (matched->handler == &Magnetic::gms_command_help)
		gms_command_help("");

	else
		gms_normal_string("There is no help available on that Glk command."
		                  "  Sorry.\n");
}

int Magnetic::gms_command_escape(const char *string_, int *undo_command) {
	int posn;
	char *string_copy, *command, *argument;
	assert(string_ && undo_command);

	/*
	 * Return false if the string doesn't begin with the Glk command escape
	 * introducer.
	 */
	posn = strspn(string_, "\t ");
	if (gms_strncasecmp(string_ + posn, "glk", strlen("glk")) != 0)
		return false;

	/* Take a copy of the string_, without any leading space or introducer. */
	string_copy = (char *)gms_malloc(strlen(string_ + posn) + 1 - strlen("glk"));
	strcpy(string_copy, string_ + posn + strlen("glk"));

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
		const gms_command_t *entry, *matched;
		int matches;

		/*
		 * Search for the first unambiguous table command string_ matching
		 * the command passed in.
		 */
		matches = 0;
		matched = NULL;
		for (entry = GMS_COMMAND_TABLE; entry->command; entry++) {
			if (gms_strncasecmp(command, entry->command, strlen(command)) == 0) {
				matches++;
				matched = entry;
			}
		}

		/* If the match was unambiguous, call the command handler. */
		if (matches == 1) {
			if (!matched->undo_return)
				gms_normal_char('\n');
			(this->*(matched->handler))(argument);

			if (!matched->takes_argument && strlen(argument) > 0) {
				gms_normal_string("[The ");
				gms_standout_string(matched->command);
				gms_normal_string(" command ignores arguments.]\n");
			}

			*undo_command = matched->undo_return;
		}

		/* No match, or the command was ambiguous. */
		else {
			gms_normal_string("\nThe Glk command ");
			gms_standout_string(command);
			gms_normal_string(" is ");
			gms_normal_string(matches == 0 ? "not valid" : "ambiguous");
			gms_normal_string(".  Try ");
			gms_standout_string("glk help");
			gms_normal_string(" for more information.\n");
		}
	} else {
		gms_normal_char('\n');
		gms_command_help("");
	}

	/* The string_ contained a Glk command; return true. */
	free(string_copy);
	return true;
}

int Magnetic::gms_command_undo_special(const char *string_) {
	int posn, end;
	assert(string_);

	/* Find the start and end of the first string_ word. */
	posn = strspn(string_, "\t ");
	end = posn + strcspn(string_ + posn, "\t ");

	/* See if string_ contains an "undo" request, with nothing following. */
	if (end - posn == (int)strlen("undo")
	        && gms_strncasecmp(string_ + posn, "undo", end - posn) == 0) {
		posn = end + strspn(string_ + end, "\t ");
		if (string_[posn] == '\0')
			return true;
	}

	return false;
}

/*---------------------------------------------------------------------*/
/*  Glk port input functions                                           */
/*---------------------------------------------------------------------*/

void Magnetic::gms_expand_abbreviations(char *buffer_, int size) {
	char *command, abbreviation;
	const char *expansion;
	gms_abbreviationref_t entry;
	assert(buffer_);

	/* Ignore anything that isn't a single letter command. */
	command = buffer_ + strspn(buffer_, "\t ");
	if (!(strlen(command) == 1
	        || (strlen(command) > 1 && Common::isSpace(command[1]))))
		return;

	/* Scan the abbreviations table for a match. */
	abbreviation = glk_char_to_lower((unsigned char) command[0]);
	expansion = NULL;
	for (entry = GMS_ABBREVIATIONS; entry->expansion; entry++) {
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
		if ((int)(strlen(buffer_) + strlen(expansion)) - 1 >= size)
			return;

		memmove(command + strlen(expansion) - 1, command, strlen(command) + 1);
		memcpy(command, expansion, strlen(expansion));

#if 0
		gms_standout_string("[");
		gms_standout_char(abbreviation);
		gms_standout_string(" -> ");
		gms_standout_string(expansion);
		gms_standout_string("]\n");
#endif
	}
}

void Magnetic::gms_buffer_input() {
	event_t event;

	/*
	 * Update the current status line display, and flush any pending buffered
	 * output.
	 */
	gms_status_notify();
	gms_output_flush();

	/*
	 * Magnetic Windows games tend not to issue a prompt after reading an empty
	 * line of input.  This can make for a very blank looking screen.
	 *
	 * To slightly improve things, if it looks like we didn't get a prompt from
	 * the game, do our own.
	 */
	if (gms_prompt_enabled && !gms_game_prompted()) {
		gms_normal_char('\n');
		gms_normal_string(GMS_INPUT_PROMPT);
	}

	/*
	 * If we have an input log to read from, use that until it is exhausted.  On
	 * end of file, close the stream and resume input from line requests.
	 */
	if (gms_readlog_stream) {
		glui32 chars;

		/* Get the next line from the log stream. */
		chars = glk_get_line_stream(gms_readlog_stream,
		                                  gms_input_buffer, sizeof(gms_input_buffer));
		if (chars > 0) {
			/* Echo the line just read in input style. */
			glk_set_style(style_Input);
			glk_put_buffer(gms_input_buffer, chars);
			glk_set_style(style_Normal);

			/* Note how many characters buffered, and return. */
			gms_input_length = chars;
			return;
		}

		/*
		 * We're at the end of the log stream.  Close it, and then continue
		 * on to request a line from Glk.
		 */
		glk_stream_close(gms_readlog_stream, NULL);
		gms_readlog_stream = NULL;
	}

	/*
	 * No input log being read, or we just hit the end of file on one.  Revert
	 * to normal line input; start by getting a new line from Glk.
	 */
	glk_request_line_event(gms_main_window,
	                             gms_input_buffer, sizeof(gms_input_buffer) - 1, 0);
	gms_event_wait(evtype_LineInput, &event);
	if (shouldQuit()) {
		glk_cancel_line_event(gms_main_window, &event);
		return;
	}

	/* Terminate the input line with a NUL. */
	assert(event.val1 <= sizeof(gms_input_buffer) - 1);
	gms_input_buffer[event.val1] = '\0';

	/* Special handling for "undo" commands. */
	if (gms_command_undo_special(gms_input_buffer)) {
		/* Write the "undo" to any input log. */
		if (gms_inputlog_stream) {
			glk_put_string_stream(gms_inputlog_stream, gms_input_buffer);
			glk_put_char_stream(gms_inputlog_stream, '\n');
		}

		/* Overwrite buffer with an empty line if we saw "undo". */
		gms_input_buffer[0] = '\n';
		gms_input_length = 1;

		gms_undo_notification = true;
		return;
	}

	/*
	 * If neither abbreviations nor local commands are enabled, use the data
	 * read above without further massaging.
	 */
	if (gms_abbreviations_enabled || gms_commands_enabled) {
		char *command;

		/*
		 * If the first non-space input character is a quote, bypass all
		 * abbreviation expansion and local command recognition, and use the
		 * unadulterated input, less introductory quote.
		 */
		command = gms_input_buffer + strspn(gms_input_buffer, "\t ");
		if (command[0] == '\'') {
			/* Delete the quote with memmove(). */
			memmove(command, command + 1, strlen(command));
		} else {
			/* Check for, and expand, any abbreviated commands. */
			if (gms_abbreviations_enabled) {
				gms_expand_abbreviations(gms_input_buffer,
				                         sizeof(gms_input_buffer));
			}

			/*
			 * Check for standalone "help", then for Glk port special commands;
			 * suppress the interpreter's use of this input for Glk commands
			 * by overwriting the line with a single newline character.
			 */
			if (gms_commands_enabled) {
				int posn;

				posn = strspn(gms_input_buffer, "\t ");
				if (gms_strncasecmp(gms_input_buffer + posn,
				                    "help", strlen("help")) == 0) {
					if (strspn(gms_input_buffer + posn + strlen("help"), "\t ")
					        == strlen(gms_input_buffer + posn + strlen("help"))) {
						gms_output_register_help_request();
					}
				}

				if (gms_command_escape(gms_input_buffer,
				                       &gms_undo_notification)) {
					gms_output_silence_help_hints();
					gms_input_buffer[0] = '\n';
					gms_input_length = 1;
					return;
				}
			}
		}
	}

	/*
	 * If there is an input log active, log this input string to it.  Note that
	 * by logging here we get any abbreviation expansions but we won't log glk
	 * special commands, nor any input read from a current open input log.
	 */
	if (gms_inputlog_stream) {
		glk_put_string_stream(gms_inputlog_stream, gms_input_buffer);
		glk_put_char_stream(gms_inputlog_stream, '\n');
	}

	/*
	 * Now append a newline to the buffer, since Glk line input doesn't provide
	 * one, and in any case, abbreviation expansion may have edited the buffer
	 * contents (and in particular, changed the length).
	 */
	gms_input_buffer[strlen(gms_input_buffer) + 1] = '\0';
	gms_input_buffer[strlen(gms_input_buffer)] = '\n';

	/* Note how many characters are buffered after all of the above. */
	gms_input_length = strlen(gms_input_buffer);
}

type8 Magnetic::ms_getchar(type8 trans) {
	/* See if we are at the end of the input buffer. */
	if (gms_input_cursor == gms_input_length) {
		/*
		 * Try to read in more data, and rewind buffer cursor.  As well as
		 * reading input, this may set an undo notification.
		 */
		gms_buffer_input();
		gms_input_cursor = 0;

		if (shouldQuit())
			return '\0';
		
		if (gms_undo_notification) {
			/*
			 * Clear the undo notification, and discard buffered input (usually
			 * just the '\n' placed there when the undo command was recognized).
			 */
			gms_undo_notification = false;
			gms_input_length = 0;

			/*
			 * Return the special 0, or a blank line if no undo is allowed at
			 * this point.
			 */
			return trans ? 0 : '\n';
		}
	}

	/* Return the next character from the input buffer. */
	assert(gms_input_cursor < gms_input_length);
	return gms_input_buffer[gms_input_cursor++];
}

/*---------------------------------------------------------------------*/
/*  Glk port event functions                                           */
/*---------------------------------------------------------------------*/

void Magnetic::gms_event_wait(glui32 wait_type, event_t *event) {
	assert(event);

	do {
		glk_select(event);

		switch (event->type) {
		case evtype_Arrange:
		case evtype_Redraw:
			/* Refresh any sensitive windows on size events. */
			gms_status_redraw();
			gms_hint_redraw();
			gms_graphics_paint();
			break;

		case evtype_Timer:
			/* Do background graphics updates on timeout. */
			gms_graphics_timeout();
			break;

		case evtype_Quit:
			return;

		default:
			break;
		}
	} while (event->type != (EvType)wait_type);
}

/*---------------------------------------------------------------------*/
/*  Functions intercepted by link-time wrappers                        */
/*---------------------------------------------------------------------*/

int Magnetic::__wrap_toupper(int ch) {
	unsigned char uch;

	uch = glk_char_to_upper((unsigned char) ch);
	return (int) uch;
}

int Magnetic::__wrap_tolower(int ch) {
	unsigned char lch;

	lch = glk_char_to_lower((unsigned char) ch);
	return (int) lch;
}

/*---------------------------------------------------------------------*/
/*  main() and options parsing                                         */
/*---------------------------------------------------------------------*/

void Magnetic::gms_establish_filenames(const char *name, char **text, char **graphics, char **hints_) {
	char *base, *text_file, *graphics_file, *hints_file;
	Common::File stream;
	assert(name && text && graphics && hints_);

	/* Take a destroyable copy of the input filename. */
	base = (char *)gms_malloc(strlen(name) + 1);
	strcpy(base, name);

	/* If base has an extension .MAG, .GFX, or .HNT, remove it. */
	if (strlen(base) > strlen(".XXX")) {
		if (gms_strcasecmp(base + strlen(base) - strlen(".MAG"), ".MAG") == 0
		        || gms_strcasecmp(base + strlen(base) - strlen(".GFX"), ".GFX") == 0
		        || gms_strcasecmp(base + strlen(base) - strlen(".HNT"), ".HNT") == 0)
			base[strlen(base) - strlen(".XXX")] = '\0';
	}

	/* Allocate space for the return text file. */
	text_file = (char *)gms_malloc(strlen(base) + strlen(".MAG") + 1);

	/* Form a candidate text file, by adding a .MAG extension. */
	strcpy(text_file, base);
	strcat(text_file, ".MAG");

	if (!stream.open(text_file)) {
		/* Retry, using a .mag extension instead. */
		strcpy(text_file, base);
		strcat(text_file, ".mag");

		if (!stream.open(text_file)) {
			/*
			 * No access to a usable game text file.  Return immediately,
			 * without looking for any associated graphics or hints_ files.
			 */
			*text = NULL;
			*graphics = NULL;
			*hints_ = NULL;

			free(text_file);
			free(base);
			return;
		}
	}
	stream.close();

	/* Now allocate space for the return graphics file. */
	graphics_file = (char *)gms_malloc(strlen(base) + strlen(".GFX") + 1);

	/* As above, form a candidate graphics file, using a .GFX extension. */
	strcpy(graphics_file, base);
	strcat(graphics_file, ".GFX");

	if (!stream.open(graphics_file)) {
		/* Retry, using a .gfx extension instead. */
		strcpy(graphics_file, base);
		strcat(graphics_file, ".gfx");

		if (!stream.open(graphics_file)) {
			/*
			 * No access to any graphics file.  In this case, free memory and
			 * reset graphics file to NULL.
			 */
			free(graphics_file);
			graphics_file = NULL;
		}
	}
	stream.close();

	/* Now allocate space for the return hints_ file. */
	hints_file = (char *)gms_malloc(strlen(base) + strlen(".HNT") + 1);

	/* As above, form a candidate graphics file, using a .HNT extension. */
	strcpy(hints_file, base);
	strcat(hints_file, ".HNT");

	if (!stream.open(hints_file)) {
		/* Retry, using a .hnt extension instead. */
		strcpy(hints_file, base);
		strcat(hints_file, ".hnt");

		if (!stream.open(hints_file)) {
			/*
			 * No access to any hints_ file.  In this case, free memory and
			 * reset hints_ file to NULL.
			 */
			free(hints_file);
			hints_file = NULL;
		}
	}
	stream.close();

	/* Return the text file, and graphics and hints_, which may be NULL. */
	*text = text_file;
	*graphics = graphics_file;
	*hints_ = hints_file;

	free(base);
}

void Magnetic::gms_main() {
	char *text_file = NULL, *graphics_file = NULL, *hints_file = NULL;
	int ms_init_status, is_running;

	/* Create the main Glk window, and set its stream as current. */
	gms_main_window = glk_window_open(0, 0, 0, wintype_TextBuffer, 0);
	if (!gms_main_window) {
		gms_fatal("GLK: Can't open main window");
		glk_exit();
		return;
	}
	glk_window_clear(gms_main_window);
	glk_set_window(gms_main_window);
	glk_set_style(style_Normal);

	/*
	 * Given the basic game name, try to come up with usable text, graphics,
	 * and hints filenames.  The graphics and hints files may be null, but the
	 * text file may not.
	 */
	Common::String gameFile = getFilename();
	gms_establish_filenames(gameFile.c_str(), &text_file, &graphics_file, &hints_file);

	/* Set the possibility of pictures depending on graphics file. */
	if (graphics_file) {
		/*
		 * Check Glk library capabilities, and note pictures are impossible if
		 * the library can't offer both graphics and timers.  We need timers to
		 * create the background "thread" for picture updates.
		 */
		gms_graphics_possible = glk_gestalt(gestalt_Graphics, 0)
		                        && glk_gestalt(gestalt_Timer, 0);
	} else
		gms_graphics_possible = false;


	/*
	 * If pictures are impossible, clear pictures enabled flag.  That is, act
	 * as if -np was given on the command line, even though it may not have
	 * been.  If pictures are impossible, they can never be enabled.
	 */
	if (!gms_graphics_possible)
		gms_graphics_enabled = false;

	/* Try to create a one-line status window.  We can live without it. */
	glk_stylehint_set(wintype_TextGrid, style_User1, stylehint_ReverseColor, 1);
	gms_status_window = glk_window_open(gms_main_window,
	                    winmethod_Above | winmethod_Fixed,
	                    1, wintype_TextGrid, 0);

	/*
	 * Load the game.  If no graphics are possible, then passing the NULL to
	 * ms_init() runs a game without graphics.
	 */
	if (gms_graphics_possible) {
		assert(graphics_file);
		ms_init_status = ms_init(text_file, graphics_file, hints_file, NULL);
	} else
		ms_init_status = ms_init(text_file, NULL, hints_file, NULL);

	/* Look for a complete failure to load the game. */
	if (ms_init_status == 0) {
		if (gms_status_window)
			glk_window_close(gms_status_window, NULL);
		gms_header_string("Glk Magnetic Error\n\n");
		gms_normal_string("Can't load game '");
		gms_normal_string(gameFile.c_str());
		gms_normal_char('\'');

		gms_normal_char('\n');

		/*
		 * Free the text file path, any graphics/hints file path, and
		 * interpreter allocated memory.
		 */
		free(text_file);
		free(graphics_file);
		free(hints_file);
		ms_freemem();
		glk_exit();
	}

	/* Try to identify the game from its text file header. */
	gms_gameid_identify_game(text_file);

	/* Look for failure to load just game graphics. */
	if (gms_graphics_possible && ms_init_status == 1) {
		/*
		 * Output a warning if graphics failed, but the main game text
		 * initialized okay.
		 */
		gms_standout_string("Error: Unable to open graphics file\n"
		                    "Continuing without pictures...\n\n");

		gms_graphics_possible = false;
	}

	/* Run the game opcodes -- ms_rungame() returns false on game end. */
	do {
		is_running = ms_rungame() && !shouldQuit();
		glk_tick();
	} while (is_running);

	/* Handle any updated status and pending buffered output. */
	gms_status_notify();
	gms_output_flush();

	/* Turn off any background graphics "thread". */
	gms_graphics_stop();

	/* Free interpreter allocated memory. */
	ms_freemem();

	/*
	 * Free any temporary memory that may have been used by graphics and hints.
	 */
	gms_graphics_cleanup();
	gms_hints_cleanup();

	/* Close any open transcript, input log, and/or read log. */
	if (gms_transcript_stream) {
		glk_stream_close(gms_transcript_stream, NULL);
		gms_transcript_stream = NULL;
	}
	if (gms_inputlog_stream) {
		glk_stream_close(gms_inputlog_stream, NULL);
		gms_inputlog_stream = NULL;
	}
	if (gms_readlog_stream) {
		glk_stream_close(gms_readlog_stream, NULL);
		gms_readlog_stream = NULL;
	}

	/* Free the text file path, and any graphics/hints file path. */
	free(text_file);
	free(graphics_file);
	free(hints_file);
}

/*---------------------------------------------------------------------*/
/*  Linkage between Glk entry/exit calls and the Magnetic interpreter  */
/*---------------------------------------------------------------------*/

void Magnetic::glk_main() {
	assert(gms_startup_called && !gms_main_called);
	gms_main_called = true;

	/* Call the interpreter main function. */
	gms_main();
}

void Magnetic::write(const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	Common::String s = Common::String::format(fmt, ap);
	va_end(ap);
	glk_put_buffer(s.c_str(), s.size());
}

void Magnetic::writeChar(char c) {
	glk_put_char(c);
}

void Magnetic::script_write(type8 c) {
	if (log_on == 2) {
		if (_log1) {
			_log1->writeByte(c);
		}
	}
}

void Magnetic::transcript_write(type8 c) {
	if (_log2) {
		_log2->writeByte(c);
	}
}

} // End of namespace Magnetic
} // End of namespace Glk
