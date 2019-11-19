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

#include "glk/magnetic/defs.h"
#include "glk/magnetic/magnetic.h"

namespace Glk {
namespace Magnetic {

/*
 * True and false definitions -- usually defined in glkstart.h, but we need
 * them early, so we'll define them here too.  We also need NULL, but that's
 * normally from stdio.h or one of it's cousins.
 */
#ifndef FALSE
# define FALSE false
#endif
#ifndef TRUE
# define TRUE false
#endif

#define BYTE_MAX 255
#define CHAR_BIT 8
#define UINT16_MAX 0xffff
#define INT32_MAX 0x7fffffff

/*---------------------------------------------------------------------*/
/*  Module variables, miscellaneous other stuff                        */
/*---------------------------------------------------------------------*/

/* Glk Magnetic Scrolls port version number. */
static const glui32 GMS_PORT_VERSION = 0x00010601;

/*
 * We use a maximum of five Glk windows, one for status, one for pictures,
 * two for hints, and one for everything else.  The status and pictures
 * windows may be NULL, depending on user selections and the capabilities
 * of the Glk library.  The hints windows will normally be NULL, except
 * when in the hints subsystem.
 */
static winid_t gms_main_window = NULL,
               gms_status_window = NULL,
               gms_graphics_window = NULL,
               gms_hint_menu_window = NULL,
               gms_hint_text_window = NULL;

/*
 * Transcript stream and input log.  These are NULL if there is no current
 * collection of these strings.
 */
static strid_t gms_transcript_stream = NULL,
               gms_inputlog_stream = NULL;

/* Input read log stream, for reading back an input log. */
static strid_t gms_readlog_stream = NULL;

/* Note about whether graphics is possible, or not. */
static int gms_graphics_possible = TRUE;

/* Options that may be turned off or set by command line flags. */
static int gms_graphics_enabled = TRUE;
enum GammaMode {
	GAMMA_OFF, GAMMA_NORMAL, GAMMA_HIGH
};
static GammaMode gms_gamma_mode = GAMMA_NORMAL;
static int gms_animation_enabled = TRUE,
           gms_prompt_enabled = TRUE,
           gms_abbreviations_enabled = TRUE,
           gms_commands_enabled = TRUE;

/* Magnetic Scrolls standard input prompt string. */
static const char *const GMS_INPUT_PROMPT = ">";

/* Forward declaration of event wait function. */
static void gms_event_wait(glui32 wait_type, event_t *event);


/*---------------------------------------------------------------------*/
/*  Glk port utility functions                                         */
/*---------------------------------------------------------------------*/

/*
 * gms_fatal()
 *
 * Fatal error handler.  The function returns, expecting the caller to
 * abort() or otherwise handle the error.
 */
static void gms_fatal(const char *string) {
	/*
	 * If the failure happens too early for us to have a window, print
	 * the message to stderr.
	 */
	if (!gms_main_window)
		error("\n\nINTERNAL ERROR: %s", string);

	/* Cancel all possible pending window input events. */
	g_vm->glk_cancel_line_event(gms_main_window, NULL);
	g_vm->glk_cancel_char_event(gms_main_window);
	if (gms_hint_menu_window) {
		g_vm->glk_cancel_char_event(gms_hint_menu_window);
		g_vm->glk_window_close(gms_hint_menu_window, NULL);
	}
	if (gms_hint_text_window) {
		g_vm->glk_cancel_char_event(gms_hint_text_window);
		g_vm->glk_window_close(gms_hint_text_window, NULL);
	}

	/* Print a message indicating the error. */
	g_vm->glk_set_window(gms_main_window);
	g_vm->glk_set_style(style_Normal);
	g_vm->glk_put_string("\n\nINTERNAL ERROR: ");
	g_vm->glk_put_string(string);

	g_vm->glk_put_string("\n\nPlease record the details of this error, try to"
	                     " note down everything you did to cause it, and email"
	                     " this information to simon_baldwin@yahoo.com.\n\n");
}


/*
 * gms_malloc()
 * gms_realloc()
 *
 * Non-failing malloc and realloc; call gms_fatal and exit if memory
 * allocation fails.
 */
static void *gms_malloc(size_t size) {
	void *pointer;

	pointer = malloc(size);
	if (!pointer) {
		gms_fatal("GLK: Out of system memory");
		g_vm->glk_exit();
	}

	return pointer;
}

static void *gms_realloc(void *ptr, size_t size) {
	void *pointer;

	pointer = realloc(ptr, size);
	if (!pointer) {
		gms_fatal("GLK: Out of system memory");
		g_vm->glk_exit();
	}

	return pointer;
}


/*
 * gms_strncasecmp()
 * gms_strcasecmp()
 *
 * Strncasecmp and strcasecmp are not ANSI functions, so here are local
 * definitions to do the same jobs.
 */
static int gms_strncasecmp(const char *s1, const char *s2, size_t n) {
	size_t index;

	for (index = 0; index < n; index++) {
		int diff;

		diff = g_vm->glk_char_to_lower(s1[index]) - g_vm->glk_char_to_lower(s2[index]);
		if (diff < 0 || diff > 0)
			return diff < 0 ? -1 : 1;
	}

	return 0;
}

static int gms_strcasecmp(const char *s1, const char *s2) {
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

/* CRC table initialization polynomial. */
static const glui32 GMS_CRC_POLYNOMIAL = 0xedb88320;


/*
 * gms_get_buffer_crc()
 *
 * Return the CRC of the bytes in buffer[0..length-1].
 *
 * This algorithm is taken from the PNG specification, version 1.0.
 */
static glui32 gms_get_buffer_crc(const void *void_buffer, size_t length) {
	static int is_initialized = FALSE;
	static glui32 crc_table[BYTE_MAX + 1];

	const char *buffer = (const char *) void_buffer;
	glui32 crc;
	size_t index;

	/* Build the static CRC lookup table on first call. */
	if (!is_initialized) {
		for (index = 0; index < BYTE_MAX + 1; index++) {
			int bit;

			crc = (glui32) index;
			for (bit = 0; bit < CHAR_BIT; bit++)
				crc = crc & 1 ? GMS_CRC_POLYNOMIAL ^ (crc >> 1) : crc >> 1;

			crc_table[index] = crc;
		}

		is_initialized = TRUE;

		/* CRC lookup table self-test, after is_initialized set -- recursion. */
		assert(gms_get_buffer_crc("123456789", 9) == 0xcbf43926);
	}

	/*
	 * Start with all ones in the crc, then update using table entries.  Xor
	 * with all ones again, finally, before returning.
	 */
	crc = 0xffffffff;
	for (index = 0; index < length; index++)
		crc = crc_table[(crc ^ buffer[index]) & BYTE_MAX] ^ (crc >> CHAR_BIT);
	return crc ^ 0xffffffff;
}


/*---------------------------------------------------------------------*/
/*  Glk port game identification data and identification functions     */
/*---------------------------------------------------------------------*/

/*
 * The game's name, suitable for printing out on a status line, or other
 * location where game information is relevant.  Set on game startup, by
 * identifying the game from its text file header.
 */
static const char *gms_gameid_game_name = NULL;


/*
 * The following game database is built from Generic/games.txt, and is used
 * to identify the game being run.  Magnetic Scrolls games don't generally
 * supply a status line, so this data can be used instead.
 */
struct gms_game_table_t {
	const type32 undo_size;   /* Header word at offset 0x22 */
	const type32 undo_pc;     /* Header word at offset 0x26 */
	const char *const name;   /* Game title and platform */
};
typedef const gms_game_table_t *gms_game_tableref_t;

static const gms_game_table_t GMS_GAME_TABLE[] = {
	{0x2100, 0x427e, "Corruption v1.11 (Amstrad CPC)"},
	{0x2100, 0x43a0, "Corruption v1.11 (Archimedes)"},
	{0x2100, 0x43a0, "Corruption v1.11 (DOS)"},
	{0x2100, 0x4336, "Corruption v1.11 (Commodore 64)"},
	{0x2100, 0x4222, "Corruption v1.11 (Spectrum +3)"},
	{0x2100, 0x4350, "Corruption v1.12 (Archimedes)"},
	{0x2500, 0x6624, "Corruption v1.12 (DOS, Magnetic Windows)"},

	{0x2300, 0x3fa0, "Fish v1.02 (DOS)"},
	{0x2400, 0x4364, "Fish v1.03 (Spectrum +3)"},
	{0x2300, 0x3f72, "Fish v1.07 (Commodore 64)"},
	{0x2200, 0x3f9c, "Fish v1.08 (Archimedes)"},
	{0x2a00, 0x583a, "Fish v1.10 (DOS, Magnetic Windows)"},

	{0x5000, 0x6c30, "Guild v1.0 (Amstrad CPC)"},
	{0x5000, 0x6cac, "Guild v1.0 (Commodore 64)"},
	{0x5000, 0x6d5c, "Guild v1.1 (DOS)"},
	{0x3300, 0x698a, "Guild v1.3 (Archimedes)"},
	{0x3200, 0x6772, "Guild v1.3 (Spectrum +3)"},
	{0x3400, 0x6528, "Guild v1.3 (DOS, Magnetic Windows)"},

	{0x2b00, 0x488c, "Jinxter v1.05 (Commodore 64)"},
	{0x2c00, 0x4a08, "Jinxter v1.05 (DOS)"},
	{0x2c00, 0x487a, "Jinxter v1.05 (Spectrum +3)"},
	{0x2c00, 0x4a56, "Jinxter v1.10 (DOS)"},
	{0x2b00, 0x4924, "Jinxter v1.22 (Amstrad CPC)"},
	{0x2c00, 0x4960, "Jinxter v1.30 (Archimedes)"},

	{0x1600, 0x3940, "Myth v1.0 (Commodore 64)"},
	{0x1500, 0x3a0a, "Myth v1.0 (DOS)"},

	{0x3600, 0x42cc, "Pawn v2.3 (Amstrad CPC)"},
	{0x3600, 0x4420, "Pawn v2.3 (Archimedes)"},
	{0x3600, 0x3fb0, "Pawn v2.3 (Commodore 64)"},
	{0x3600, 0x4420, "Pawn v2.3 (DOS)"},
	{0x3900, 0x42e4, "Pawn v2.3 (Spectrum 128)"},
	{0x3900, 0x42f4, "Pawn v2.4 (Spectrum +3)"},

	{0x3900, 0x75f2, "Wonderland v1.21 (DOS, Magnetic Windows)"},
	{0x3900, 0x75f8, "Wonderland v1.27 (Archimedes)"},
	{0, 0, NULL}
};


/*
 * gms_gameid_lookup_game()
 *
 * Look up and return the game table entry given a game's undo size and
 * undo pc values.  Returns the entry, or NULL if not found.
 */
static gms_game_tableref_t gms_gameid_lookup_game(type32 undo_size, type32 undo_pc) {
	gms_game_tableref_t game;

	for (game = GMS_GAME_TABLE; game->name; game++) {
		if (game->undo_size == undo_size && game->undo_pc == undo_pc)
			break;
	}

	return game->name ? game : NULL;
}


/*
 * gms_gameid_read_uint32()
 *
 * Endian-safe unsigned 32 bit integer read from game text file.  Returns
 * 0 on error, a known unused table value.
 */
static type32 gms_gameid_read_uint32(int offset, Common::SeekableReadStream *stream) {
	if (!stream->seek(offset))
		return 0;
	return stream->readUint32LE();
}


/*
 * gms_gameid_identify_game()
 *
 * Identify a game from its text file header, and cache the game's name for
 * later queries.  Sets the cache to NULL if not found.
 */
static void gms_gameid_identify_game(const char *text_file) {
	Common::File stream;

	if (stream.open(text_file)) {
		type32 undo_size, undo_pc;
		gms_game_tableref_t game;

		/* Read the game's signature undo size and undo pc values. */
		undo_size = gms_gameid_read_uint32(0x22, &stream);
		undo_pc = gms_gameid_read_uint32(0x26, &stream);
		stream.close();

		/* Search for these values in the table, and set game name if found. */
		game = gms_gameid_lookup_game(undo_size, undo_pc);
		gms_gameid_game_name = game ? game->name : NULL;
	} else {
		gms_gameid_game_name = NULL;
	}
}


/*
 * gms_gameid_get_game_name()
 *
 * Return the name of the game, or NULL if not identifiable.
 */
static const char *gms_gameid_get_game_name(void) {
	return gms_gameid_game_name;
}


/*---------------------------------------------------------------------*/
/*  Glk port picture functions                                         */
/*---------------------------------------------------------------------*/

/*
 * Color conversions lookup tables, and a word about gamma corrections.
 *
 * When uncorrected, some game pictures can look dark (Corruption, Won-
 * derland), whereas others look just fine (Guild Of Thieves, Jinxter).
 *
 * The standard general-purpose gamma correction is around 2.1, with
 * specific values, normally, of 2.5-2.7 for IBM PC systems, and 1.8 for
 * Macintosh.  However, applying even the low end of this range can make
 * some pictures look washed out, yet improve others nicely.
 *
 * To try to solve this, here we'll set up a precalculated table with
 * discrete gamma values.  On displaying a picture, we'll try to find a
 * gamma correction that seems to offer a reasonable level of contrast
 * for the picture.
 *
 * Here's an AWK script to create the gamma table:
 *
 * BEGIN { max=255.0; step=max/7.0
 *         for (gamma=0.9; gamma<=2.7; gamma+=0.05) {
 *             printf "  {\"%2.2f\", {0, ", gamma
 *             for (i=1; i<8; i++) {
 *                 printf "%3.0f", (((step*i / max) ^ (1.0/gamma)) * max)
 *                 printf "%s", (i<7) ? ", " : ""
 *             }
 *             printf "}, "
 *             printf "%s },\n", (gamma>0.99 && gamma<1.01) ? "FALSE" : "TRUE "
 *         } }
 *
 */
typedef const struct {
	const char *const level;       /* Gamma correction level. */
	const unsigned char table[8];  /* Color lookup table. */
	const int is_corrected;        /* Flag if non-linear. */
} gms_gamma_t;
typedef gms_gamma_t *gms_gammaref_t;

static gms_gamma_t GMS_GAMMA_TABLE[] = {
	{"0.90", {0,  29,  63,  99, 137, 175, 215, 255}, TRUE},
	{"0.95", {0,  33,  68, 105, 141, 179, 217, 255}, TRUE},
	{"1.00", {0,  36,  73, 109, 146, 182, 219, 255}, FALSE},
	{"1.05", {0,  40,  77, 114, 150, 185, 220, 255}, TRUE},
	{"1.10", {0,  43,  82, 118, 153, 188, 222, 255}, TRUE},
	{"1.15", {0,  47,  86, 122, 157, 190, 223, 255}, TRUE},
	{"1.20", {0,  50,  90, 126, 160, 193, 224, 255}, TRUE},
	{"1.25", {0,  54,  94, 129, 163, 195, 225, 255}, TRUE},
	{"1.30", {0,  57,  97, 133, 166, 197, 226, 255}, TRUE},
	{"1.35", {0,  60, 101, 136, 168, 199, 227, 255}, TRUE},
	{"1.40", {0,  64, 104, 139, 171, 201, 228, 255}, TRUE},
	{"1.45", {0,  67, 107, 142, 173, 202, 229, 255}, TRUE},
	{"1.50", {0,  70, 111, 145, 176, 204, 230, 255}, TRUE},
	{"1.55", {0,  73, 114, 148, 178, 205, 231, 255}, TRUE},
	{"1.60", {0,  76, 117, 150, 180, 207, 232, 255}, TRUE},
	{"1.65", {0,  78, 119, 153, 182, 208, 232, 255}, TRUE},
	{"1.70", {0,  81, 122, 155, 183, 209, 233, 255}, TRUE},
	{"1.75", {0,  84, 125, 157, 185, 210, 233, 255}, TRUE},
	{"1.80", {0,  87, 127, 159, 187, 212, 234, 255}, TRUE},
	{"1.85", {0,  89, 130, 161, 188, 213, 235, 255}, TRUE},
	{"1.90", {0,  92, 132, 163, 190, 214, 235, 255}, TRUE},
	{"1.95", {0,  94, 134, 165, 191, 215, 236, 255}, TRUE},
	{"2.00", {0,  96, 136, 167, 193, 216, 236, 255}, TRUE},
	{"2.05", {0,  99, 138, 169, 194, 216, 237, 255}, TRUE},
	{"2.10", {0, 101, 140, 170, 195, 217, 237, 255}, TRUE},
	{"2.15", {0, 103, 142, 172, 197, 218, 237, 255}, TRUE},
	{"2.20", {0, 105, 144, 173, 198, 219, 238, 255}, TRUE},
	{"2.25", {0, 107, 146, 175, 199, 220, 238, 255}, TRUE},
	{"2.30", {0, 109, 148, 176, 200, 220, 238, 255}, TRUE},
	{"2.35", {0, 111, 150, 178, 201, 221, 239, 255}, TRUE},
	{"2.40", {0, 113, 151, 179, 202, 222, 239, 255}, TRUE},
	{"2.45", {0, 115, 153, 180, 203, 222, 239, 255}, TRUE},
	{"2.50", {0, 117, 154, 182, 204, 223, 240, 255}, TRUE},
	{"2.55", {0, 119, 156, 183, 205, 223, 240, 255}, TRUE},
	{"2.60", {0, 121, 158, 184, 206, 224, 240, 255}, TRUE},
	{"2.65", {0, 122, 159, 185, 206, 225, 241, 255}, TRUE},
	{"2.70", {0, 124, 160, 186, 207, 225, 241, 255}, TRUE},
	{NULL,   {0,   0,   0,   0,   0,   0,   0,   0}, FALSE}
};

/* R,G,B color triple definition. */
typedef struct {
	int red, green, blue;
} gms_rgb_t;
typedef gms_rgb_t *gms_rgbref_t;

/*
 * Weighting values for calculating the luminance of a color.  There are
 * two commonly used sets of values for these -- 299,587,114, taken from
 * NTSC (Never The Same Color) 1953 standards, and 212,716,72, which is the
 * set that modern CRTs tend to match.  The NTSC ones seem to give the best
 * subjective results.
 */
static const gms_rgb_t GMS_LUMINANCE_WEIGHTS = { 299, 587, 114 };

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

/*
 * The current picture bitmap being displayed, its width, height, palette,
 * animation flag, and picture id.
 */
enum { GMS_PALETTE_SIZE = 16 };
static type8 *gms_graphics_bitmap = NULL;
static type16 gms_graphics_width = 0,
              gms_graphics_height = 0,
              gms_graphics_palette[GMS_PALETTE_SIZE]; /* = { 0, ... }; */
static bool gms_graphics_animated = FALSE;
static type32 gms_graphics_picture = 0;

/*
 * Flags set on new picture, and on resize or arrange events, and a flag
 * to indicate whether background repaint is stopped or active.
 */
static bool gms_graphics_new_picture = FALSE,
            gms_graphics_repaint = FALSE,
            gms_graphics_active = FALSE;

/* Flag to try to monitor the state of interpreter graphics. */
static bool gms_graphics_interpreter = FALSE;

/*
 * Pointer to the two graphics buffers, one the off-screen representation
 * of pixels, and the other tracking on-screen data.  These are temporary
 * graphics malloc'ed memory, and should be free'd on exit.
 */
static type8 *gms_graphics_off_screen = NULL,
              *gms_graphics_on_screen = NULL;

/*
 * Pointer to the current active gamma table entry.  Because of the way
 * it's queried, this may not be NULL, otherwise we risk a race, with
 * admittedly a very low probability, with the updater.  So, it's init-
 * ialized instead to the gamma table.  The real value in use is inserted
 * on the first picture update timeout call for a new picture.
 */
static gms_gammaref_t gms_graphics_current_gamma = GMS_GAMMA_TABLE;

/*
 * The number of colors used in the palette by the current picture.  This
 * value is also at risk of a race with the updater, so it too has a mild
 * lie for a default value.
 */
static int gms_graphics_color_count = GMS_PALETTE_SIZE;


/*
 * gms_graphics_open()
 *
 * If it's not open, open the graphics window.  Returns TRUE if graphics
 * was successfully started, or already on.
 */
static int gms_graphics_open(void) {
	if (!gms_graphics_window) {
		gms_graphics_window = g_vm->glk_window_open(gms_main_window,
		                      winmethod_Above
		                      | winmethod_Proportional,
		                      GMS_GRAPHICS_PROPORTION,
		                      wintype_Graphics, 0);
	}

	return gms_graphics_window != NULL;
}


/*
 * gms_graphics_close()
 *
 * If open, close the graphics window and set back to NULL.
 */
static void gms_graphics_close(void) {
	if (gms_graphics_window) {
		g_vm->glk_window_close(gms_graphics_window, NULL);
		gms_graphics_window = NULL;
	}
}


/*
 * gms_graphics_start()
 *
 * If graphics enabled, start any background picture update processing.
 */
static void gms_graphics_start(void) {
	if (gms_graphics_enabled) {
		/* If not running, start the updating "thread". */
		if (!gms_graphics_active) {
			g_vm->glk_request_timer_events(GMS_GRAPHICS_TIMEOUT);
			gms_graphics_active = TRUE;
		}
	}
}


/*
 * gms_graphics_stop()
 *
 * Stop any background picture update processing.
 */
static void gms_graphics_stop(void) {
	/* If running, stop the updating "thread". */
	if (gms_graphics_active) {
		g_vm->glk_request_timer_events(0);
		gms_graphics_active = FALSE;
	}
}


/*
 * gms_graphics_are_displayed()
 *
 * Return TRUE if graphics are currently being displayed, FALSE otherwise.
 */
static int gms_graphics_are_displayed(void) {
	return gms_graphics_window != NULL;
}


/*
 * gms_graphics_paint()
 *
 * Set up a complete repaint of the current picture in the graphics window.
 * This function should be called on the appropriate Glk window resize and
 * arrange events.
 */
static void gms_graphics_paint(void) {
	if (gms_graphics_enabled && gms_graphics_are_displayed()) {
		/* Set the repaint flag, and start graphics. */
		gms_graphics_repaint = TRUE;
		gms_graphics_start();
	}
}


/*
 * gms_graphics_restart()
 *
 * Restart graphics as if the current picture is a new picture.  This
 * function should be called whenever graphics is re-enabled after being
 * disabled, on change of gamma color correction policy, and on change
 * of animation policy.
 */
static void gms_graphics_restart(void) {
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
		gms_graphics_new_picture = TRUE;
		gms_graphics_start();
	}
}


/*
 * gms_graphics_count_colors()
 *
 * Analyze an image, and return the usage count of each palette color, and
 * an overall count of how many colors out of the palette are used.  NULL
 * arguments indicate no interest in the return value.
 */
static void gms_graphics_count_colors(type8 bitmap[], type16 width, type16 height,
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


/*
 * gms_graphics_game_to_rgb_color()
 * gms_graphics_split_color()
 * gms_graphics_combine_color()
 * gms_graphics_color_luminance()
 *
 * General graphics helper functions, to convert between Magnetic Scrolls
 * and RGB color representations, and between RGB and Glk glui32 color
 * representations, and to calculate color luminance.
 */
static void
gms_graphics_game_to_rgb_color(type16 color, gms_gammaref_t gamma,
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

static void gms_graphics_split_color(glui32 color, gms_rgbref_t rgb_color) {
	assert(rgb_color);

	rgb_color->red   = (color >> 16) & 0xff;
	rgb_color->green = (color >> 8) & 0xff;
	rgb_color->blue  = color & 0xff;
}

static glui32 gms_graphics_combine_color(gms_rgbref_t rgb_color) {
	glui32 color;
	assert(rgb_color);

	color = (rgb_color->red << 16) | (rgb_color->green << 8) | rgb_color->blue;
	return color;
}

static int gms_graphics_color_luminance(gms_rgbref_t rgb_color) {
	static int is_initialized = FALSE;
	static int weighting = 0;

	long luminance;

	/* On the first call, calculate the overall weighting. */
	if (!is_initialized) {
		weighting = GMS_LUMINANCE_WEIGHTS.red + GMS_LUMINANCE_WEIGHTS.green
		            + GMS_LUMINANCE_WEIGHTS.blue;

		is_initialized = TRUE;
	}

	/* Calculate the luminance and scale back by 1000 to 0-255 before return. */
	luminance = ((long) rgb_color->red   * (long) GMS_LUMINANCE_WEIGHTS.red
	             + (long) rgb_color->green * (long) GMS_LUMINANCE_WEIGHTS.green
	             + (long) rgb_color->blue  * (long) GMS_LUMINANCE_WEIGHTS.blue);

	assert(weighting > 0);
	return (int)(luminance / weighting);
}


/*
 * gms_graphics_compare_luminance()
 * gms_graphics_constrast_variance()
 *
 * Calculate the contrast variance of the given palette and color usage, at
 * the given gamma correction level.  Helper functions for automatic gamma
 * correction.
 */
static int gms_graphics_compare_luminance(const void *void_first,
        const void *void_second) {
	long first = *(const long *)void_first;
	long second = *(const long *)void_second;

	return first > second ? 1 : second > first ? -1 : 0;
}

static long gms_graphics_contrast_variance(type16 palette[],
        long color_usage[], gms_gammaref_t gamma) {
	int index, count, has_black, mean;
	long sum;
	int contrast[GMS_PALETTE_SIZE];
	int luminance[GMS_PALETTE_SIZE + 1];  /* Luminance for each color,
                                           plus one extra for black */

	/* Calculate the luminance energy of each palette color at this gamma. */
	has_black = FALSE;
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


/*
 * gms_graphics_equal_contrast_gamma()
 *
 * Try to find a gamma correction for the given palette and color usage that
 * gives relatively equal contrast among the displayed colors.
 *
 * To do this, we search the gamma tables, computing color luminance for each
 * color in the palette given this gamma.  From luminances, we then compute
 * the contrasts between the colors, and settle on the gamma correction that
 * gives the most even and well-distributed picture contrast.  We ignore
 * colors not used in the palette.
 *
 * Note that we don't consider how often a palette color is used, only whether
 * it's represented, or not.  Some weighting might improve things, but the
 * simple method seems to work adequately.  In practice, as there are only 16
 * colors in a palette, most pictures use most colors in a relatively well
 * distributed manner.  This algorithm probably wouldn't work well on real
 * photographs, though.
 */
static gms_gammaref_t gms_graphics_equal_contrast_gamma(type16 palette[], long color_usage[]) {
	gms_gammaref_t gamma, result;
	long lowest_variance;
	assert(palette && color_usage);

	result = NULL;
	lowest_variance = INT32_MAX;

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


/*
 * gms_graphics_select_gamma()
 *
 * Select a suitable gamma for the picture, based on the current gamma mode.
 *
 * The function returns either the linear gamma, a gamma value half way
 * between linear and the gamma that gives the most even contrast, or just
 * the gamma that gives the most even contrast.
 *
 * In the normal case, a value half way to the extreme case of making color
 * contrast equal for all colors is, subjectively, a reasonable value to use.
 * The problem cases are the darkest pictures, and selecting this value
 * brightens them while at the same time not making them look overbright or
 * too "sunny".
 */
static gms_gammaref_t gms_graphics_select_gamma(type8 bitmap[],
        type16 width, type16 height, type16 palette[]) {
	static int is_initialized = FALSE;
	static gms_gammaref_t linear_gamma = NULL;

	long color_usage[GMS_PALETTE_SIZE];
	int color_count;
	gms_gammaref_t contrast_gamma;

	/* On first call, find and cache the uncorrected gamma table entry. */
	if (!is_initialized) {
		gms_gammaref_t gamma;

		for (gamma = GMS_GAMMA_TABLE; gamma->level; gamma++) {
			if (!gamma->is_corrected) {
				linear_gamma = gamma;
				break;
			}
		}

		is_initialized = TRUE;
	}
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


/*
 * gms_graphics_clear_and_border()
 *
 * Clear the graphics window, and border and shade the area where the
 * picture is going to be rendered.  This attempts a small raised effect
 * for the picture, in keeping with modern trends.
 */
static void gms_graphics_clear_and_border(winid_t glk_window,
        int x_offset, int y_offset, int pixel_size, type16 width, type16 height) {
	glui32 background, fade_color, shading_color;
	gms_rgb_t rgb_background, rgb_border, rgb_fade;
	int index;
	assert(glk_window);

	/*
	 * Try to detect the background color of the main window, by getting the
	 * background for Normal style (Glk offers no way to directly get a window's
	 * background color).  If we can get it, we'll match the graphics window
	 * background to it.  If we can't, we'll default the color to white.
	 */
	if (!g_vm->glk_style_measure(gms_main_window,
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
	g_vm->glk_window_set_background_color(glk_window, background);
	g_vm->glk_window_clear(glk_window);

	/*
	 * For very small pictures, just border them, but don't try and do any
	 * shading.  Failing this check is probably highly unlikely.
	 */
	if (width < 2 * GMS_GRAPHICS_SHADE_STEPS
	        || height < 2 * GMS_GRAPHICS_SHADE_STEPS) {
		/* Paint a rectangle bigger than the picture by border pixels. */
		g_vm->glk_window_fill_rect(glk_window,
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
	g_vm->glk_window_fill_rect(glk_window,
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
		g_vm->glk_window_fill_rect(glk_window, shading_color,
		                           x_offset + width * pixel_size
		                           + GMS_GRAPHICS_BORDER,
		                           y_offset + index - GMS_GRAPHICS_BORDER,
		                           GMS_GRAPHICS_SHADING, 1);
		g_vm->glk_window_fill_rect(glk_window, shading_color,
		                           x_offset + index - GMS_GRAPHICS_BORDER,
		                           y_offset + height * pixel_size
		                           + GMS_GRAPHICS_BORDER,
		                           1, GMS_GRAPHICS_SHADING);

		/* Update the shading color for the fade next iteration. */
		shading_color -= fade_color;
	}
}


/*
 * gms_graphics_convert_palette()
 *
 * Convert a Magnetic Scrolls color palette to a Glk one, using the given
 * gamma corrections.
 */
static void gms_graphics_convert_palette(type16 ms_palette[], gms_gammaref_t gamma,
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


/*
 * gms_graphics_position_picture()
 *
 * Given a picture width and height, return the x and y offsets to center
 * this picture in the current graphics window.
 */
static void gms_graphics_position_picture(winid_t glk_window,
        int pixel_size, type16 width, type16 height,
        int *x_offset, int *y_offset) {
	glui32 window_width, window_height;
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


/*
 * gms_graphics_apply_animation_frame()
 *
 * Apply a single animation frame to the given off-screen image buffer, using
 * the frame bitmap, width, height and mask, the off-screen buffer, and the
 * width and height of the main picture.
 *
 * Note that 'mask' may be NULL, implying that no frame pixel is transparent.
 */
static void gms_graphics_apply_animation_frame(type8 bitmap[],
        type16 frame_width, type16 frame_height,
        type8 mask[], int frame_x, int frame_y,
        type8 off_screen[], type16 width,
        type16 height) {
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
	mask_width = (((frame_width - 1) / CHAR_BIT) + 2) & (~1);
	mask_hibit = 1 << (CHAR_BIT - 1);

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
				mask_byte = mask[mask_row + (x / CHAR_BIT)];
				if ((mask_byte & (mask_hibit >> (x % CHAR_BIT))) != 0)
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


/*
 * gms_graphics_animate()
 *
 * This function finds and applies the next set of animation frames to the
 * given off-screen image buffer.  It's handed the width and height of the
 * main picture, and the off-screen buffer.
 *
 * It returns FALSE if at the end of animations, TRUE if more animations
 * remain.
 */
static int gms_graphics_animate(type8 off_screen[], type16 width, type16 height) {
	struct ms_position *positions;
	type16 count;
	type8 status;
	int frame;
	assert(off_screen);

	/* Search for more animation frames, and return zero if none. */
	status = ms_animate(&positions, &count);
	if (status == 0)
		return FALSE;

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

	/* Return TRUE since more animation frames remain. */
	return TRUE;
}

#ifndef GARGLK
/*
 * gms_graphics_is_vertex()
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
static int gms_graphics_is_vertex(type8 off_screen[], type16 width, type16 height,
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
	 * Return TRUE if this pixel lies at the vertex of a rectangular, fillable,
	 * area.  That is, if two adjacent neighbors aren't the same color (or if
	 * absent -- at the edge of the picture).
	 */
	return ((above || below) && (left || right));
}

/*
 * gms_graphics_compare_layering_inverted()
 * gms_graphics_assign_layers()
 *
 * Given two sets of image bitmaps, and a palette, this function will
 * assign layers palette colors.
 *
 * Layers are assigned by first counting the number of vertices in the
 * color plane, to get a measure of the complexity of shapes displayed in
 * this color, and also the raw number of times each palette color is
 * used.  This is then sorted, so that layers are assigned to colors, with
 * the lowest layer being the color with the most complex shapes, and
 * within this (or where the count of vertices is zero, as it could be
 * in some animation frames) the most used color.
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
typedef struct {
	long complexity;  /* Count of vertices for this color. */
	long usage;       /* Color usage count. */
	int color;        /* Color index into palette. */
} gms_layering_t;

static int gms_graphics_compare_layering_inverted(const void *void_first,
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

static void gms_graphics_assign_layers(type8 off_screen[], type8 on_screen[],
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

/*
 * gms_graphics_paint_region()
 *
 * This is a partially optimized point plot.  Given a point in the graphics
 * bitmap, it tries to extend the point to a color region, and fill a number
 * of pixels in a single Glk rectangle fill.  The goal here is to reduce the
 * number of Glk rectangle fills, which tend to be extremely inefficient
 * operations for generalized point plotting.
 *
 * The extension works in image layers; each palette color is assigned a
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
static void gms_graphics_paint_region(winid_t glk_window, glui32 palette[], int layers[],
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

static void gms_graphics_paint_everything(winid_t glk_window,
        glui32 palette[],
        type8 off_screen[],
        int x_offset, int y_offset,
        type16 width, type16 height) {
	type8       pixel;          /* Reference pixel color */
	int     x, y;

	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x ++) {
			pixel = off_screen[ y * width + x ];
			g_vm->glk_window_fill_rect(glk_window,
			                           palette[ pixel ],
			                           x * GMS_GRAPHICS_PIXEL + x_offset,
			                           y * GMS_GRAPHICS_PIXEL + y_offset,
			                           GMS_GRAPHICS_PIXEL, GMS_GRAPHICS_PIXEL);
		}
	}
}

/*
 * gms_graphics_timeout()
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
static void gms_graphics_timeout() {
	static glui32 palette[GMS_PALETTE_SIZE];   /* Precomputed Glk palette */
#ifndef GARGLK
	static int layers[GMS_PALETTE_SIZE];       /* Assigned image layers */
	static long layer_usage[GMS_PALETTE_SIZE]; /* Image layer occupancies */
#endif

	static int deferred_repaint = FALSE;       /* Local delayed repaint flag */
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
		deferred_repaint = TRUE;
		gms_graphics_repaint = FALSE;
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
		gms_graphics_new_picture = FALSE;
		deferred_repaint = FALSE;
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


/*
 * ms_showpic()
 *
 * Called by the main interpreter when it wants us to display a picture.
 * The function gets the picture bitmap, palette, and dimensions, and
 * saves them, and the picture id, in module variables for the background
 * rendering function.
 *
 * The graphics window is opened if required, or closed if mode is zero.
 *
 * The function checks for changes of actual picture by calculating the
 * CRC for picture data; this helps to prevent unnecessary repaints in
 * cases where the interpreter passes us the same picture as we're already
 * displaying.  There is a less than 1 in 4,294,967,296 chance that a new
 * picture will be missed.  We'll live with that.
 *
 * Why use CRCs, rather than simply storing the values of picture passed in
 * a static variable?  Because some games, typically Magnetic Windows, use
 * the picture argument as a form of string pointer, and can pass in the
 * same value for several, perhaps all, game pictures.  If we just checked
 * for a change in the picture argument, we'd never see one.  So we must
 * instead look for changes in the real picture data.
 */
void ms_showpic(type32 picture, type8 mode) {
	static glui32 current_crc = 0;  /* CRC of the current picture */

	type8 *bitmap, animated;
	type16 width, height, palette[GMS_PALETTE_SIZE];
	long picture_bytes;
	glui32 crc;

	/* See if the mode indicates no graphics. */
	if (mode == 0) {
		/* Note that the interpreter turned graphics off. */
		gms_graphics_interpreter = FALSE;

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
	gms_graphics_interpreter = TRUE;

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
	        && crc == current_crc
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
	current_crc = crc;

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
			gms_graphics_new_picture = TRUE;
			gms_graphics_start();
		}
	}
}


/*
 * gms_graphics_picture_is_available()
 *
 * Return TRUE if the graphics module data is loaded with a usable picture,
 * FALSE if there is no picture available to display.
 */
static int
gms_graphics_picture_is_available(void) {
	return gms_graphics_bitmap != NULL;
}


/*
 * gms_graphics_get_picture_details()
 *
 * Return the width, height, and animation flag of the currently loaded
 * picture.  The function returns FALSE if no picture is loaded, otherwise
 * TRUE, with picture details in the return arguments.
 */
static int gms_graphics_get_picture_details(int *width, int *height, int *is_animated) {
	if (gms_graphics_picture_is_available()) {
		if (width)
			*width = gms_graphics_width;
		if (height)
			*height = gms_graphics_height;
		if (is_animated)
			*is_animated = gms_graphics_animated;

		return TRUE;
	}

	return FALSE;
}


/*
 * gms_graphics_get_rendering_details()
 *
 * Returns the current level of applied gamma correction, as a string, the
 * count of colors in the picture, and a flag indicating if graphics is
 * active (busy).  The function return FALSE if graphics is not enabled or
 * if not being displayed, otherwise TRUE with the gamma, color count, and
 * active flag in the return arguments.
 *
 * This function races with the graphics timeout, as it returns information
 * set up by the first timeout following a new picture.  There's a very,
 * very small chance that it might win the race, in which case out-of-date
 * gamma and color count values are returned.
 */
static int gms_graphics_get_rendering_details(const char **gamma, int *color_count,
        int *is_active) {
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

		return TRUE;
	}

	return FALSE;
}


/*
 * gms_graphics_interpreter_enabled()
 *
 * Return TRUE if it looks like interpreter graphics are turned on, FALSE
 * otherwise.
 */
static int gms_graphics_interpreter_enabled(void) {
	return gms_graphics_interpreter;
}


/*
 * gms_graphics_cleanup()
 *
 * Free memory resources allocated by graphics functions.  Called on game
 * end.
 */
static void gms_graphics_cleanup(void) {
	free(gms_graphics_bitmap);
	gms_graphics_bitmap = NULL;
	free(gms_graphics_off_screen);
	gms_graphics_off_screen = NULL;
	free(gms_graphics_on_screen);
	gms_graphics_on_screen = NULL;

	gms_graphics_animated = FALSE;
	gms_graphics_picture = 0;
}


/*---------------------------------------------------------------------*/
/*  Glk port status line functions                                     */
/*---------------------------------------------------------------------*/

/*
 * The interpreter feeds us status line characters one at a time, with Tab
 * indicating right justify, and CR indicating the line is complete.  To get
 * this to fit with the Glk event and redraw model, here we'll buffer each
 * completed status line, so we have a stable string to output when needed.
 * It's also handy to have this buffer for Glk libraries that don't support
 * separate windows.
 */
enum { GMS_STATBUFFER_LENGTH = 1024 };
static char gms_status_buffer[GMS_STATBUFFER_LENGTH];
static int gms_status_length = 0;

/* Default width used for non-windowing Glk status lines. */
static const int GMS_DEFAULT_STATUS_WIDTH = 74;


/*
 * ms_statuschar()
 *
 * Receive one status character from the interpreter.  Characters are
 * buffered internally, and on CR, the buffer is copied to the main static
 * status buffer for use by the status line printing function.
 */
void ms_statuschar(type8 c) {
	static char buffer[GMS_STATBUFFER_LENGTH];
	static int length = 0;

	/*
	 * If the status character is newline, transfer locally buffered data to
	 * the common buffer, empty the local buffer; otherwise, if space permits,
	 * buffer the character.
	 */
	if (c == '\n') {
		memcpy(gms_status_buffer, buffer, length);
		gms_status_length = length;

		length = 0;
	} else {
		if (length < (int)sizeof(buffer))
			buffer[length++] = c;
	}
}


/*
 * gms_status_update()
 *
 * Update the information in the status window with the current contents of
 * the completed status line buffer, or a default string if no completed
 * status line.
 */
static void gms_status_update(void) {
	glui32 width, height;
	int index;
	assert(gms_status_window);

	g_vm->glk_window_get_size(gms_status_window, &width, &height);
	if (height > 0) {
		g_vm->glk_window_clear(gms_status_window);
		g_vm->glk_window_move_cursor(gms_status_window, 0, 0);
		g_vm->glk_set_window(gms_status_window);

		g_vm->glk_set_style(style_User1);
		for (index = 0; index < (int)width; index++)
			g_vm->glk_put_char(' ');
		g_vm->glk_window_move_cursor(gms_status_window, 1, 0);

		if (gms_status_length > 0) {
			/*
			 * Output each character from the status line buffer.  If the
			 * character is Tab, position the cursor to eleven characters shy
			 * of the status window right.
			 */
			for (index = 0; index < gms_status_length; index++) {
				if (gms_status_buffer[index] == '\t')
					g_vm->glk_window_move_cursor(gms_status_window, width - 11, 0);
				else
					g_vm->glk_put_char(gms_status_buffer[index]);
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
			g_vm->glk_put_string(game_name ? game_name : "ScummVM Magnetic version 2.3");
		}

		g_vm->glk_set_window(gms_main_window);
	}
}


/*
 * gms_status_print()
 *
 * Print the current contents of the completed status line buffer out in the
 * main window, if it has changed since the last call.  This is for non-
 * windowing Glk libraries.
 */
static void gms_status_print(void) {
	static char buffer[GMS_STATBUFFER_LENGTH];
	static int length = 0;

	int index, column;

	/*
	 * Do nothing if there is no status line to print, or if the status
	 * line hasn't changed since last printed.
	 */
	if (gms_status_length == 0
	        || (gms_status_length == length
	            && strncmp(buffer, gms_status_buffer, length)) == 0)
		return;

	/* Set fixed width font to try to preserve status line formatting. */
	g_vm->glk_set_style(style_Preformatted);

	/* Bracket, and output the status line buffer. */
	g_vm->glk_put_string("[ ");
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
				g_vm->glk_put_char(' ');
				column++;
			}
		} else {
			g_vm->glk_put_char(gms_status_buffer[index]);
			column++;
		}
	}

	while (column <= GMS_DEFAULT_STATUS_WIDTH) {
		g_vm->glk_put_char(' ');
		column++;
	}
	g_vm->glk_put_string(" ]\n");

	/* Save the details of the printed status buffer. */
	memcpy(buffer, gms_status_buffer, gms_status_length);
	length = gms_status_length;
}


/*
 * gms_status_notify()
 *
 * Front end function for updating status.  Either updates the status window
 * or prints the status line to the main window.
 */
static void gms_status_notify(void) {
	if (gms_status_window)
		gms_status_update();
	else
		gms_status_print();
}


/*
 * gms_status_redraw()
 *
 * Redraw the contents of any status window with the buffered status string.
 * This function should be called on the appropriate Glk window resize and
 * arrange events.
 */
static void gms_status_redraw(void) {
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
		parent = g_vm->glk_window_get_parent(gms_status_window);
		g_vm->glk_window_set_arrangement(parent,
		                                 winmethod_Above | winmethod_Fixed, 1, NULL);

		gms_status_update();
	}
}


/*---------------------------------------------------------------------*/
/*  Glk port output functions                                          */
/*---------------------------------------------------------------------*/

/*
 * Flag for if the user entered "help" as their last input, or if hints have
 * been silenced as a result of already using a Glk command.
 */
static int gms_help_requested = FALSE,
           gms_help_hints_silenced = FALSE;

/*
 * Output buffer.  We receive characters one at a time, and it's a bit
 * more efficient for everyone if we buffer them, and output a complete
 * string on a flush call.
 */
static char *gms_output_buffer = NULL;
static int gms_output_allocation = 0,
           gms_output_length = 0;

/*
 * Flag to indicate if the last buffer flushed looked like it ended in a
 * ">" prompt.
 */
static int gms_output_prompt = FALSE;


/*
 * gms_output_register_help_request()
 * gms_output_silence_help_hints()
 * gms_output_provide_help_hint()
 *
 * Register a request for help, and print a note of how to get Glk command
 * help from the interpreter unless silenced.
 */
static void gms_output_register_help_request(void) {
	gms_help_requested = TRUE;
}

static void gms_output_silence_help_hints(void) {
	gms_help_hints_silenced = TRUE;
}

static void gms_output_provide_help_hint(void) {
	if (gms_help_requested && !gms_help_hints_silenced) {
		g_vm->glk_set_style(style_Emphasized);
		g_vm->glk_put_string("[Try 'glk help' for help on special interpreter"
		                     " commands]\n");

		gms_help_requested = FALSE;
		g_vm->glk_set_style(style_Normal);
	}
}


/*
 * gms_game_prompted()
 *
 * Return TRUE if the last game output appears to have been a ">" prompt.
 * Once called, the flag is reset to FALSE, and requires more game output
 * to set it again.
 */
static int gms_game_prompted(void) {
	int result;

	result = gms_output_prompt;
	gms_output_prompt = FALSE;

	return result;
}


/*
 * gms_detect_game_prompt()
 *
 * See if the last non-newline-terminated line in the output buffer seems
 * to be a prompt, and set the game prompted flag if it does, otherwise
 * clear it.
 */
static void gms_detect_game_prompt(void) {
	int index;

	gms_output_prompt = FALSE;

	/*
	 * Search for a prompt across any last unterminated buffered line; a prompt
	 * is any non-space character on that line.
	 */
	for (index = gms_output_length - 1;
	        index >= 0 && gms_output_buffer[index] != '\n'; index--) {
		if (gms_output_buffer[index] != ' ') {
			gms_output_prompt = TRUE;
			break;
		}
	}
}


/*
 * gms_output_delete()
 *
 * Delete all buffered output text.  Free all malloc'ed buffer memory, and
 * return the buffer variables to their initial values.
 */
static void gms_output_delete(void) {
	free(gms_output_buffer);
	gms_output_buffer = NULL;
	gms_output_allocation = gms_output_length = 0;
}


/*
 * gms_output_flush()
 *
 * Flush any buffered output text to the Glk main window, and clear the
 * buffer.
 */
static void gms_output_flush(void) {
	assert(g_vm->glk_stream_get_current());

	if (gms_output_length > 0) {
		/*
		 * See if the game issued a standard prompt, then print the buffer to
		 * the main window.  If providing a help hint, position that before
		 * the game's prompt (if any).
		 */
		gms_detect_game_prompt();
		g_vm->glk_set_style(style_Normal);

		if (gms_output_prompt) {
			int index;

			for (index = gms_output_length - 1;
			        index >= 0 && gms_output_buffer[index] != '\n';)
				index--;

			g_vm->glk_put_buffer(gms_output_buffer, index + 1);
			gms_output_provide_help_hint();
			g_vm->glk_put_buffer(gms_output_buffer + index + 1,
			                     gms_output_length - index - 1);
		} else {
			g_vm->glk_put_buffer(gms_output_buffer, gms_output_length);
			gms_output_provide_help_hint();
		}

		gms_output_delete();
	}
}


/*
 * ms_putchar()
 *
 * Buffer a character for eventual printing to the main window.
 */
void ms_putchar(type8 c) {
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


/*
 * gms_styled_string()
 * gms_styled_char()
 * gms_standout_string()
 * gms_standout_char()
 * gms_normal_string()
 * gms_normal_char()
 * gms_header_string()
 * gms_banner_string()
 *
 * Convenience functions to print strings in assorted styles.  A standout
 * string is one that hints that it's from the interpreter, not the game.
 */
static void gms_styled_string(glui32 style, const char *message) {
	assert(message);

	g_vm->glk_set_style(style);
	g_vm->glk_put_string(message);
	g_vm->glk_set_style(style_Normal);
}

static void gms_styled_char(glui32 style, char c) {
	char buffer[2];

	buffer[0] = c;
	buffer[1] = '\0';
	gms_styled_string(style, buffer);
}

static void gms_standout_string(const char *message) {
	gms_styled_string(style_Emphasized, message);
}

#if 0
static void gms_standout_char(char c) {
	gms_styled_char(style_Emphasized, c);
}
#endif

static void gms_normal_string(const char *message) {
	gms_styled_string(style_Normal, message);
}

static void gms_normal_char(char c) {
	gms_styled_char(style_Normal, c);
}

static void gms_header_string(const char *message) {
	gms_styled_string(style_Header, message);
}

static void gms_banner_string(const char *message) {
	gms_styled_string(style_Subheader, message);
}

/*
 * ms_flush()
 *
 * Handle a core interpreter call to flush the output buffer.  Because Glk
 * only flushes its buffers and displays text on g_vm->glk_select(), we can ignore
 * these calls as long as we call gms_output_flush() when reading line input.
 *
 * Taking ms_flush() at face value can cause game text to appear before status
 * line text where we are working with a non-windowing Glk, so it's best
 * ignored where we can.
 */
void ms_flush(void) {
}


/*---------------------------------------------------------------------*/
/*  Glk port hint functions                                            */
/*---------------------------------------------------------------------*/

/* Hint type definitions. */
enum {
	GMS_HINT_TYPE_FOLDER = 1,
	GMS_HINT_TYPE_TEXT = 2
};

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
                    GMS_HINTS_DONE = UINT16_MAX;

/* Generic hint topic for the root hints node. */
static const char *const GMS_GENERIC_TOPIC = "Hints Menu";

/*
 * Note of the interpreter's hints array.  Note that keeping its address
 * like this assumes that it's either static or heap in the interpreter.
 */
static struct ms_hint *gms_hints = NULL;

/* Details of the current hint node on display from the hints array. */
static type16 gms_current_hint_node = 0;

/*
 * Array of cursors for each hint.  The cursor indicates the current hint
 * position in a folder, and the last hint shown in text hints.  Space
 * is allocated as needed for a given set of hints, and needs to be freed
 * on interpreter exit.
 */
static int *gms_hint_cursor = NULL;


/*
 * gms_get_hint_max_node()
 *
 * Return the maximum hint node referred to by the tree under the given node.
 * The result is the largest index found, or node, if greater.  Because the
 * interpreter doesn't supply it, we need to uncover it the hard way.  The
 * function is recursive, and since it is a tree search, assumes that hints
 * is a tree, not a graph.
 */
static type16 gms_get_hint_max_node(const struct ms_hint hints[], type16 node) {
	const struct ms_hint *hint;
	int index;
	type16 max_node;
	assert(hints);

	hint = hints + node;
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

			link_max = gms_get_hint_max_node(hints, hint->links[index]);
			if (link_max > max_node)
				max_node = link_max;
		}
		break;

	default:
		gms_fatal("GLK: Invalid hints node type encountered");
		g_vm->glk_exit();
	}

	/*
	 * Return the largest node reference found, capped to avoid overlapping the
	 * special end-hints value.
	 */
	return max_node < GMS_HINTS_DONE ? max_node : GMS_HINTS_DONE - 1;
}


/*
 * gms_get_hint_content()
 *
 * Return the content string for a given hint number within a given node.
 * This counts over 'number' ASCII NULs in the node's content, returning
 * the address of the string located this way.
 */
static const char *gms_get_hint_content(const struct ms_hint hints[], type16 node, int number) {
	const struct ms_hint *hint;
	int offset, index;
	assert(hints);

	hint = hints + node;

	/* Run through content until 'number' strings found. */
	offset = 0;
	for (index = 0; index < number; index++)
		offset += strlen(hint->content + offset) + 1;

	/* Return the start of the number'th string encountered. */
	return hint->content + offset;
}


/*
 * gms_get_hint_topic()
 *
 * Return the topic string for a given hint node.  This is found by searching
 * the parent node for a link to the node handed in.  For the root node, the
 * string is defaulted, since the root node has no parent.
 */
static const char *gms_get_hint_topic(const struct ms_hint hints[], type16 node) {
	assert(hints);

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
		parent = hints[node].parent;

		topic = NULL;
		for (index = 0; index < hints[parent].elcount; index++) {
			if (hints[parent].links[index] == node) {
				topic = gms_get_hint_content(hints, parent, index);
				break;
			}
		}

		return topic ? topic : GMS_GENERIC_TOPIC;
	}
}


/*
 * gms_hint_open()
 *
 * If not already open, open the hints windows.  Returns TRUE if the windows
 * opened, or were already open.
 *
 * The function creates two hints windows -- a text grid on top, for menus,
 * and a text buffer below for hints.
 */
static int gms_hint_open(void) {
	if (!gms_hint_menu_window) {
		assert(!gms_hint_text_window);

		/*
		 * Open the hint menu window.  The initial size is two lines, but we'll
		 * change this later to suit the hint.
		 */
		gms_hint_menu_window = g_vm->glk_window_open(gms_main_window,
		                       winmethod_Above | winmethod_Fixed,
		                       2, wintype_TextGrid, 0);
		if (!gms_hint_menu_window)
			return FALSE;

		/*
		 * Now open the hints text window.  This is set to be 100% of the size
		 * of the main window, so should cover what remains of it completely.
		 */
		gms_hint_text_window = g_vm->glk_window_open(gms_main_window,
		                       winmethod_Above
		                       | winmethod_Proportional,
		                       100, wintype_TextBuffer, 0);
		if (!gms_hint_text_window) {
			g_vm->glk_window_close(gms_hint_menu_window, NULL);
			gms_hint_menu_window = NULL;
			return FALSE;
		}
	}

	return TRUE;
}


/*
 * gms_hint_close()
 *
 * If open, close the hints windows.
 */
static void gms_hint_close(void) {
	if (gms_hint_menu_window) {
		assert(gms_hint_text_window);

		g_vm->glk_window_close(gms_hint_menu_window, NULL);
		gms_hint_menu_window = NULL;
		g_vm->glk_window_close(gms_hint_text_window, NULL);
		gms_hint_text_window = NULL;
	}
}


/*
 * gms_hint_windows_available()
 *
 * Return TRUE if hints windows are available.  If they're not, the hints
 * system will need to use alternative output methods.
 */
static int gms_hint_windows_available(void) {
	return (gms_hint_menu_window && gms_hint_text_window);
}


/*
 * gms_hint_menu_print()
 * gms_hint_menu_header()
 * gms_hint_menu_justify()
 * gms_hint_text_print()
 * gms_hint_menutext_done()
 * gms_hint_menutext_start()
 *
 * Output functions for writing hints.  These functions will write to hints
 * windows where available, and to the main window where not.  When writing
 * to hints windows, they also take care not to line wrap in the menu window.
 * Limited formatting is available.
 */
static void gms_hint_menu_print(int line, int column, const char *string,
                                glui32 width, glui32 height) {
	assert(string);

	/* Ignore the call if the text position is outside the window. */
	if (!(line > (int)height || column > (int)width)) {
		if (gms_hint_windows_available()) {
			int posn, index;

			g_vm->glk_window_move_cursor(gms_hint_menu_window, column, line);
			g_vm->glk_set_window(gms_hint_menu_window);

			/* Write until the end of the string, or the end of the window. */
			for (posn = column, index = 0;
			        posn < (int)width && index < (int)strlen(string); posn++, index++) {
				g_vm->glk_put_char(string[index]);
			}

			g_vm->glk_set_window(gms_main_window);
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
			 * Write characters until the end of the string, or the end of the
			 * (self-imposed not-really-there) window.
			 */
			for (index = 0;
			        current_column < (int)width && index < (int)strlen(string);
			        current_column++, index++) {
				gms_normal_char(string[index]);
			}
		}
	}
}

static void gms_hint_menu_header(int line, const char *string,
                                 glui32 width, glui32 height) {
	int posn, length;
	assert(string);

	/* Output the text in the approximate line center. */
	length = strlen(string);
	posn = length < (int)width ? (width - length) / 2 : 0;
	gms_hint_menu_print(line, posn, string, width, height);
}

static void gms_hint_menu_justify(int line,
                                  const char *left_string, const char *right_string,
                                  glui32 width, glui32 height) {
	int posn, length;
	assert(left_string && right_string);

	/* Write left text normally to window left. */
	gms_hint_menu_print(line, 0, left_string, width, height);

	/* Output the right text flush with the right of the window. */
	length = strlen(right_string);
	posn = length < (int)width ? width - length : 0;
	gms_hint_menu_print(line, posn, right_string, width, height);
}

static void gms_hint_text_print(const char *string) {
	assert(string);

	if (gms_hint_windows_available()) {
		g_vm->glk_set_window(gms_hint_text_window);
		g_vm->glk_put_string(string);
		g_vm->glk_set_window(gms_main_window);
	} else
		gms_normal_string(string);
}

static void gms_hint_menutext_start(void) {
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

static void gms_hint_menutext_done(void) {
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


/*
 * gms_hint_menutext_char_event()
 *
 * Request and return a character event from the hints windows.  In practice,
 * this means either of the hints windows if available, or the main window
 * if not.
 */
static void gms_hint_menutext_char_event(event_t *event) {
	assert(event);

	if (gms_hint_windows_available()) {
		g_vm->glk_request_char_event(gms_hint_menu_window);
		g_vm->glk_request_char_event(gms_hint_text_window);

		gms_event_wait(evtype_CharInput, event);
		assert(event->window == gms_hint_menu_window
		       || event->window == gms_hint_text_window);

		g_vm->glk_cancel_char_event(gms_hint_menu_window);
		g_vm->glk_cancel_char_event(gms_hint_text_window);
	} else {
		g_vm->glk_request_char_event(gms_main_window);
		gms_event_wait(evtype_CharInput, event);
	}
}


/*
 * gms_hint_arrange_windows()
 *
 * Arrange the hints windows so that the hint menu window has the requested
 * number of lines.  Returns the actual hint menu window width and height,
 * or defaults if no hints windows are available.
 */
static void gms_hint_arrange_windows(int requested_lines, glui32 *width, glui32 *height) {
	if (gms_hint_windows_available()) {
		winid_t parent;

		/* Resize the hint menu window to fit the current hint. */
		parent = g_vm->glk_window_get_parent(gms_hint_menu_window);
		g_vm->glk_window_set_arrangement(parent,
		                                 winmethod_Above | winmethod_Fixed,
		                                 requested_lines, NULL);

		/* Measure, and return the size of the hint menu window. */
		g_vm->glk_window_get_size(gms_hint_menu_window, width, height);

		/* Clear both the hint menu and the hint text window. */
		g_vm->glk_window_clear(gms_hint_menu_window);
		g_vm->glk_window_clear(gms_hint_text_window);
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


/*
 * gms_hint_display_folder()
 *
 * Update the hints windows for the given folder hint node.
 */
static void gms_hint_display_folder(const struct ms_hint hints[],
                                    const int cursor[], type16 node) {
	glui32 width, height;
	int line, index;
	assert(hints && cursor);

	/*
	 * Arrange windows to suit the hint folder.  For a folder menu window we
	 * use one line for each element, three for the controls, and two spacers,
	 * making a total of five additional lines.  Width and height receive the
	 * actual menu window dimensions.
	 */
	gms_hint_arrange_windows(hints[node].elcount + 5, &width, &height);

	/* Paint in the menu header. */
	line = 0;
	gms_hint_menu_header(line++,
	                     gms_get_hint_topic(hints, node),
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
	for (index = 0; index < hints[node].elcount; index++) {
		gms_hint_menu_print(line, 3,
		                    index == cursor[node] ? ">" : " ",
		                    width, height);
		gms_hint_menu_print(line++, 5,
		                    gms_get_hint_content(hints, node, index),
		                    width, height);
	}

	/*
	 * Terminate with a blank line; using a single space here improves cursor
	 * positioning for optimized output libraries (for example, without it,
	 * curses output will leave the cursor at the end of the previous line).
	 */
	gms_hint_menu_print(line, 0, " ", width, height);
}


/*
 * gms_hint_display_text()
 *
 * Update the hints windows for the given text hint node.
 */
static void gms_hint_display_text(const struct ms_hint hints[],
                                  const int cursor[], type16 node) {
	glui32 width, height;
	int line, index;
	assert(hints && cursor);

	/*
	 * Arrange windows to suit the hint text.  For a hint menu, we use a simple
	 * two-line set of controls; everything else is in the hints text window.
	 * Width and height receive the actual menu window dimensions.
	 */
	gms_hint_arrange_windows(2, &width, &height);

	/* Paint in a short menu header. */
	line = 0;
	gms_hint_menu_header(line++,
	                     gms_get_hint_topic(hints, node),
	                     width, height);
	gms_hint_menu_justify(line++,
	                      " RETURN = read hint  ", "  Q = previous menu ",
	                      width, height);

	/*
	 * Output hints to the hints text window.  Hints not yet exposed are
	 * indicated by the cursor for the hint, and are displayed as a dash.
	 */
	gms_hint_text_print("\n");
	for (index = 0; index < hints[node].elcount; index++) {
		char buffer[16];

		sprintf(buffer, "%3d.  ", index + 1);
		gms_hint_text_print(buffer);

		gms_hint_text_print(index < cursor[node]
		                    ? gms_get_hint_content(hints, node, index) : "-");
		gms_hint_text_print("\n");
	}
}


/*
 * gms_hint_display()
 *
 * Display the given hint using the appropriate display function.
 */
static void gms_hint_display(const struct ms_hint hints[],
                             const int cursor[], type16 node) {
	assert(hints && cursor);

	switch (hints[node].nodetype) {
	case GMS_HINT_TYPE_TEXT:
		gms_hint_display_text(hints, cursor, node);
		break;

	case GMS_HINT_TYPE_FOLDER:
		gms_hint_display_folder(hints, cursor, node);
		break;

	default:
		gms_fatal("GLK: Invalid hints node type encountered");
		g_vm->glk_exit();
	}
}


/*
 * gms_hint_handle_folder()
 *
 * Handle a Glk keycode for the given folder hint.  Return the next node to
 * handle, or the special end-hints on Quit at the root node.
 */
static type16 gms_hint_handle_folder(const struct ms_hint hints[],
                                     int cursor[], type16 node, glui32 keycode) {
	unsigned char response;
	type16 next_node;
	assert(hints && cursor);

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
		response = keycode <= BYTE_MAX ? g_vm->glk_char_to_upper(keycode) : 0;
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
		if (cursor[node] < hints[node].elcount - 1)
			cursor[node]++;
		else
			cursor[node] = 0;
		break;

	case 'P':
		/* Regress the hint cursor, wrapping at the folder start. */
		if (cursor[node] > 0)
			cursor[node]--;
		else
			cursor[node] = hints[node].elcount - 1;
		break;

	case '\n':
		/* The next node is the hint node at the cursor position. */
		next_node = hints[node].links[cursor[node]];
		break;

	case 'Q':
		/* If root, we're done; if not, next node is node's parent. */
		next_node = node == GMS_HINT_ROOT_NODE
		            ? GMS_HINTS_DONE : hints[node].parent;
		break;

	default:
		break;
	}

	return next_node;
}


/*
 * gms_hint_handle_text()
 *
 * Handle a Glk keycode for the given text hint.  Return the next node to
 * handle.
 */
static type16 gms_hint_handle_text(const struct ms_hint hints[],
                                   int cursor[], type16 node, glui32 keycode) {
	unsigned char response;
	type16 next_node;
	assert(hints && cursor);

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
		response = keycode <= BYTE_MAX ? g_vm->glk_char_to_upper(keycode) : 0;
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
		if (cursor[node] < hints[node].elcount)
			cursor[node]++;
		break;

	case 'Q':
		/* Done with this hint node, so next node is its parent. */
		next_node = hints[node].parent;
		break;

	default:
		break;
	}

	return next_node;
}


/*
 * gms_hint_handle()
 *
 * Handle a Glk keycode for the given hint using the appropriate handler
 * function.  Return the next node to handle.
 */
static type16 gms_hint_handle(const struct ms_hint hints[],
                              int cursor[], type16 node, glui32 keycode) {
	type16 next_node;
	assert(hints && cursor);

	next_node = GMS_HINT_ROOT_NODE;
	switch (hints[node].nodetype) {
	case GMS_HINT_TYPE_TEXT:
		next_node = gms_hint_handle_text(hints, cursor, node, keycode);
		break;

	case GMS_HINT_TYPE_FOLDER:
		next_node = gms_hint_handle_folder(hints, cursor, node, keycode);
		break;

	default:
		gms_fatal("GLK: Invalid hints node type encountered");
		g_vm->glk_exit();
	}

	return next_node;
}


/*
 * ms_showhints()
 *
 * Start game hints.  These are modal, though there's no overriding Glk
 * reason why.  It's just that this matches the way they're implemented by
 * most Inform games.  This may not be the best way of doing help, but at
 * least it's likely to be familiar, and anything more ambitious may be
 * beyond the current Glk capabilities.
 *
 * This function uses CRCs to detect any change of hints data.  Normally,
 * we'd expect none, at least within a given game run, but we can probably
 * handle it okay if it happens.
 */
type8 ms_showhints(struct ms_hint *hints) {
	static int is_initialized = FALSE;
	static glui32 current_crc = 0;

	type16 hint_count;
	glui32 crc;
	assert(hints);

	/*
	 * Find the number of hints in the array.  To do this, we'll visit every
	 * node in a tree search, starting at the root, to locate the maximum node
	 * number found, then add one to that.  It's a pity that the interpreter
	 * doesn't hand us this information directly.
	 */
	hint_count = gms_get_hint_max_node(hints, GMS_HINT_ROOT_NODE) + 1;

	/*
	 * Calculate a CRC for the hints array data.  If the CRC has changed, or
	 * this is the first call, assign a new cursor array.
	 */
	crc = gms_get_buffer_crc(hints, hint_count * sizeof(*hints));
	if (crc != current_crc || !is_initialized) {
		int bytes;

		/* Allocate new cursors, and set all to zero initial state. */
		free(gms_hint_cursor);
		bytes = hint_count * sizeof(*gms_hint_cursor);
		gms_hint_cursor = (int *)gms_malloc(bytes);
		memset(gms_hint_cursor, 0, bytes);

		/*
		 * Retain the hints CRC, for later comparisons, and set is_initialized
		 * flag.
		 */
		current_crc = crc;
		is_initialized = TRUE;
	}

	/*
	 * Save the hints array passed in.  This is done here since even if the data
	 * remains the same (found by the CRC check above), the pointer to it might
	 * have changed.
	 */
	gms_hints = hints;

	/*
	 * Try to create the hints windows.  If they can't be created, perhaps
	 * because the Glk library doesn't support it, the output functions will
	 * work around this.
	 */
	gms_hint_open();
	gms_hint_menutext_start();

	/*
	 * Begin hints display at the root node, and navigate until the user exits
	 * hints.
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


/*
 * gms_hint_redraw()
 *
 * Update the hints windows for the current hint.  This function should be
 * called from the event handler on resize events, to repaint the hints
 * display.  It does nothing if no hints windows have been opened, since
 * in this case, there's no resize action required -- either we're not in
 * the hints subsystem, or hints are being displayed in the main game
 * window, for whatever reason.
 */
static void gms_hint_redraw(void) {
	if (gms_hint_windows_available()) {
		assert(gms_hints && gms_hint_cursor);
		gms_hint_display(gms_hints, gms_hint_cursor, gms_current_hint_node);
	}
}


/*
 * gms_hints_cleanup()
 *
 * Free memory resources allocated by hints functions.  Called on game
 * end.
 */
static void gms_hints_cleanup(void) {
	free(gms_hint_cursor);
	gms_hint_cursor = NULL;

	gms_hints = NULL;
	gms_current_hint_node = 0;
}


void ms_playmusic(type8 *midi_data, type32 length, type16 tempo) {
}


/*---------------------------------------------------------------------*/
/*  Glk command escape functions                                       */
/*---------------------------------------------------------------------*/

/*
 * gms_command_undo()
 *
 * Stub function for the undo command.  The real work is to return the
 * undo code to the input functions.
 */
static void gms_command_undo(const char *argument) {
	assert(argument);
}


/*
 * gms_command_script()
 *
 * Turn game output scripting (logging) on and off.
 */
static void gms_command_script(const char *argument) {
	assert(argument);

	if (gms_strcasecmp(argument, "on") == 0) {
		frefid_t fileref;

		if (gms_transcript_stream) {
			gms_normal_string("Glk transcript is already on.\n");
			return;
		}

		fileref = g_vm->glk_fileref_create_by_prompt(fileusage_Transcript
		          | fileusage_TextMode,
		          filemode_WriteAppend, 0);
		if (!fileref) {
			gms_standout_string("Glk transcript failed.\n");
			return;
		}

		gms_transcript_stream = g_vm->glk_stream_open_file(fileref,
		                        filemode_WriteAppend, 0);
		g_vm->glk_fileref_destroy(fileref);
		if (!gms_transcript_stream) {
			gms_standout_string("Glk transcript failed.\n");
			return;
		}

		g_vm->glk_window_set_echo_stream(gms_main_window, gms_transcript_stream);

		gms_normal_string("Glk transcript is now on.\n");
	}

	else if (gms_strcasecmp(argument, "off") == 0) {
		if (!gms_transcript_stream) {
			gms_normal_string("Glk transcript is already off.\n");
			return;
		}

		g_vm->glk_stream_close(gms_transcript_stream, NULL);
		gms_transcript_stream = NULL;

		g_vm->glk_window_set_echo_stream(gms_main_window, NULL);

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


/*
 * gms_command_inputlog()
 *
 * Turn game input logging on and off.
 */
static void gms_command_inputlog(const char *argument) {
	assert(argument);

	if (gms_strcasecmp(argument, "on") == 0) {
		frefid_t fileref;

		if (gms_inputlog_stream) {
			gms_normal_string("Glk input logging is already on.\n");
			return;
		}

		fileref = g_vm->glk_fileref_create_by_prompt(fileusage_InputRecord
		          | fileusage_BinaryMode,
		          filemode_WriteAppend, 0);
		if (!fileref) {
			gms_standout_string("Glk input logging failed.\n");
			return;
		}

		gms_inputlog_stream = g_vm->glk_stream_open_file(fileref,
		                      filemode_WriteAppend, 0);
		g_vm->glk_fileref_destroy(fileref);
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

		g_vm->glk_stream_close(gms_inputlog_stream, NULL);
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


/*
 * gms_command_readlog()
 *
 * Set the game input log, to read input from a file.
 */
static void gms_command_readlog(const char *argument) {
	assert(argument);

	if (gms_strcasecmp(argument, "on") == 0) {
		frefid_t fileref;

		if (gms_readlog_stream) {
			gms_normal_string("Glk read log is already on.\n");
			return;
		}

		fileref = g_vm->glk_fileref_create_by_prompt(fileusage_InputRecord
		          | fileusage_BinaryMode,
		          filemode_Read, 0);
		if (!fileref) {
			gms_standout_string("Glk read log failed.\n");
			return;
		}

		if (!g_vm->glk_fileref_does_file_exist(fileref)) {
			g_vm->glk_fileref_destroy(fileref);
			gms_standout_string("Glk read log failed.\n");
			return;
		}

		gms_readlog_stream = g_vm->glk_stream_open_file(fileref, filemode_Read, 0);
		g_vm->glk_fileref_destroy(fileref);
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

		g_vm->glk_stream_close(gms_readlog_stream, NULL);
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


/*
 * gms_command_abbreviations()
 *
 * Turn abbreviation expansions on and off.
 */
static void gms_command_abbreviations(const char *argument) {
	assert(argument);

	if (gms_strcasecmp(argument, "on") == 0) {
		if (gms_abbreviations_enabled) {
			gms_normal_string("Glk abbreviation expansions are already on.\n");
			return;
		}

		gms_abbreviations_enabled = TRUE;
		gms_normal_string("Glk abbreviation expansions are now on.\n");
	}

	else if (gms_strcasecmp(argument, "off") == 0) {
		if (!gms_abbreviations_enabled) {
			gms_normal_string("Glk abbreviation expansions are already off.\n");
			return;
		}

		gms_abbreviations_enabled = FALSE;
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


/*
 * gms_command_graphics()
 *
 * Enable or disable graphics more permanently than is done by the main
 * interpreter.  Also, print out a few brief details about the graphics
 * state of the program.
 */
static void gms_command_graphics(const char *argument) {
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

		gms_graphics_enabled = TRUE;

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
		gms_graphics_enabled = FALSE;
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
				char buffer[16];

				gms_normal_string("There is ");
				gms_normal_string(is_animated ? "an animated" : "a");
				gms_normal_string(" picture loaded, ");

				sprintf(buffer, "%d", width);
				gms_normal_string(buffer);
				gms_normal_string(" by ");

				sprintf(buffer, "%d", height);
				gms_normal_string(buffer);

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
				char buffer[16];

				gms_normal_string("Graphics are ");
				gms_normal_string(is_active ? "active, " : "displayed, ");

				sprintf(buffer, "%d", color_count);
				gms_normal_string(buffer);
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


/*
 * gms_command_gamma()
 *
 * Enable or disable picture gamma corrections.
 */
static void gms_command_gamma(const char *argument) {
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


/*
 * gms_command_animations()
 *
 * Enable or disable picture animations.
 */
static void gms_command_animations(const char *argument) {
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
		gms_animation_enabled = TRUE;
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

		gms_animation_enabled = FALSE;
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


/*
 * gms_command_prompts()
 *
 * Turn the extra "> " prompt output on and off.
 */
static void gms_command_prompts(const char *argument) {
	assert(argument);

	if (gms_strcasecmp(argument, "on") == 0) {
		if (gms_prompt_enabled) {
			gms_normal_string("Glk extra prompts are already on.\n");
			return;
		}

		gms_prompt_enabled = TRUE;
		gms_normal_string("Glk extra prompts are now on.\n");

		/* Check for a game prompt to clear the flag. */
		gms_game_prompted();
	}

	else if (gms_strcasecmp(argument, "off") == 0) {
		if (!gms_prompt_enabled) {
			gms_normal_string("Glk extra prompts are already off.\n");
			return;
		}

		gms_prompt_enabled = FALSE;
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


/*
 * gms_command_print_version_number()
 * gms_command_version()
 *
 * Print out the Glk library version number.
 */
static void gms_command_print_version_number(glui32 version) {
	char buffer[64];

	sprintf(buffer, "%lu.%lu.%lu",
	        (unsigned long) version >> 16,
	        (unsigned long)(version >> 8) & 0xff,
	        (unsigned long) version & 0xff);
	gms_normal_string(buffer);
}

static void
gms_command_version(const char *argument) {
	glui32 version;
	assert(argument);

	gms_normal_string("This is version ");
	gms_command_print_version_number(GMS_PORT_VERSION);
	gms_normal_string(" of the Glk Magnetic port.\n");

	version = g_vm->glk_gestalt(gestalt_Version, 0);
	gms_normal_string("The Glk library version is ");
	gms_command_print_version_number(version);
	gms_normal_string(".\n");
}


/*
 * gms_command_commands()
 *
 * Turn command escapes off.  Once off, there's no way to turn them back on.
 * Commands must be on already to enter this function.
 */
static void gms_command_commands(const char *argument) {
	assert(argument);

	if (gms_strcasecmp(argument, "on") == 0) {
		gms_normal_string("Glk commands are already on.\n");
	}

	else if (gms_strcasecmp(argument, "off") == 0) {
		gms_commands_enabled = FALSE;
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

/* Glk subcommands and handler functions. */
typedef const struct {
	const char *const command;                      /* Glk subcommand. */
	void (* const handler)(const char *argument);   /* Subcommand handler. */
	const int takes_argument;                       /* Argument flag. */
	const int undo_return;                          /* "Undo" return value. */
} gms_command_t;
typedef gms_command_t *gms_commandref_t;

static void gms_command_summary(const char *argument);
static void gms_command_help(const char *argument);

static gms_command_t GMS_COMMAND_TABLE[] = {
	{"summary",        gms_command_summary,        FALSE, FALSE},
	{"undo",           gms_command_undo,           FALSE, TRUE},
	{"script",         gms_command_script,         TRUE,  FALSE},
	{"inputlog",       gms_command_inputlog,       TRUE,  FALSE},
	{"readlog",        gms_command_readlog,        TRUE,  FALSE},
	{"abbreviations",  gms_command_abbreviations,  TRUE,  FALSE},
	{"graphics",       gms_command_graphics,       TRUE,  FALSE},
	{"gamma",          gms_command_gamma,          TRUE,  FALSE},
	{"animations",     gms_command_animations,     TRUE,  FALSE},
	{"prompts",        gms_command_prompts,        TRUE,  FALSE},
	{"version",        gms_command_version,        FALSE, FALSE},
	{"commands",       gms_command_commands,       TRUE,  FALSE},
	{"help",           gms_command_help,           TRUE,  FALSE},
	{NULL, NULL, FALSE, FALSE}
};


/*
 * gms_command_summary()
 *
 * Report all current Glk settings.
 */
static void gms_command_summary(const char *argument) {
	gms_commandref_t entry;
	assert(argument);

	/*
	 * Call handlers that have status to report with an empty argument,
	 * prompting each to print its current setting.
	 */
	for (entry = GMS_COMMAND_TABLE; entry->command; entry++) {
		if (entry->handler == gms_command_summary
		        || entry->handler == gms_command_undo
		        || entry->handler == gms_command_help)
			continue;

		entry->handler("");
	}
}


/*
 * gms_command_help()
 *
 * Document the available Glk commands.
 */
static void gms_command_help(const char *command) {
	gms_commandref_t entry, matched;
	assert(command);

	if (strlen(command) == 0) {
		gms_normal_string("Glk commands are");
		for (entry = GMS_COMMAND_TABLE; entry->command; entry++) {
			gms_commandref_t next;

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

	if (matched->handler == gms_command_summary) {
		gms_normal_string("Prints a summary of all the current Glk Magnetic"
		                  " settings.\n");
	}

	else if (matched->handler == gms_command_undo) {
		gms_normal_string("Undoes a single game turn.\n\nEquivalent to the"
		                  " standalone game 'undo' command.\n");
	}

	else if (matched->handler == gms_command_script) {
		gms_normal_string("Logs the game's output to a file.\n\nUse ");
		gms_standout_string("glk script on");
		gms_normal_string(" to begin logging game output, and ");
		gms_standout_string("glk script off");
		gms_normal_string(" to end it.  Glk Magnetic will ask you for a file"
		                  " when you turn scripts on.\n");
	}

	else if (matched->handler == gms_command_inputlog) {
		gms_normal_string("Records the commands you type into a game.\n\nUse ");
		gms_standout_string("glk inputlog on");
		gms_normal_string(", to begin recording your commands, and ");
		gms_standout_string("glk inputlog off");
		gms_normal_string(" to turn off input logs.  You can play back"
		                  " recorded commands into a game with the ");
		gms_standout_string("glk readlog");
		gms_normal_string(" command.\n");
	}

	else if (matched->handler == gms_command_readlog) {
		gms_normal_string("Plays back commands recorded with ");
		gms_standout_string("glk inputlog on");
		gms_normal_string(".\n\nUse ");
		gms_standout_string("glk readlog on");
		gms_normal_string(".  Command play back stops at the end of the"
		                  " file.  You can also play back commands from a"
		                  " text file created using any standard editor.\n");
	}

	else if (matched->handler == gms_command_abbreviations) {
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

	else if (matched->handler == gms_command_graphics) {
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

	else if (matched->handler == gms_command_gamma) {
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

	else if (matched->handler == gms_command_animations) {
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

	else if (matched->handler == gms_command_prompts) {
		gms_normal_string("Controls extra input prompting.\n\n"
		                  "Glk Magnetic can issue a replacement '>' input"
		                  " prompt if it detects that the game hasn't prompted"
		                  " after, say, an empty input line.  Use ");
		gms_standout_string("glk prompts on");
		gms_normal_string(" to turn this feature on, and ");
		gms_standout_string("glk prompts off");
		gms_normal_string(" to turn it off.\n");
	}

	else if (matched->handler == gms_command_version) {
		gms_normal_string("Prints the version numbers of the Glk library"
		                  " and the Glk Magnetic port.\n");
	}

	else if (matched->handler == gms_command_commands) {
		gms_normal_string("Turn off Glk commands.\n\nUse ");
		gms_standout_string("glk commands off");
		gms_normal_string(" to disable all Glk commands, including this one."
		                  "  Once turned off, there is no way to turn Glk"
		                  " commands back on while inside the game.\n");
	}

	else if (matched->handler == gms_command_help)
		gms_command_help("");

	else
		gms_normal_string("There is no help available on that Glk command."
		                  "  Sorry.\n");
}


/*
 * gms_command_escape()
 *
 * This function is handed each input line.  If the line contains a specific
 * Glk port command, handle it and return TRUE, otherwise return FALSE.
 *
 * On unambiguous returns, it will also set the value for undo_command to the
 * table undo return value.
 */
static int gms_command_escape(const char *string, int *undo_command) {
	int posn;
	char *string_copy, *command, *argument;
	assert(string && undo_command);

	/*
	 * Return FALSE if the string doesn't begin with the Glk command escape
	 * introducer.
	 */
	posn = strspn(string, "\t ");
	if (gms_strncasecmp(string + posn, "glk", strlen("glk")) != 0)
		return FALSE;

	/* Take a copy of the string, without any leading space or introducer. */
	string_copy = (char *)gms_malloc(strlen(string + posn) + 1 - strlen("glk"));
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
		gms_commandref_t entry, matched;
		int matches;

		/*
		 * Search for the first unambiguous table command string matching
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
			matched->handler(argument);

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

	/* The string contained a Glk command; return TRUE. */
	free(string_copy);
	return TRUE;
}


/*
 * gms_command_undo_special()
 *
 * This function makes a special case of the input line containing the single
 * word "undo", treating it as if it is "glk undo".  This makes life a bit
 * more convenient for the player, since it's the same behavior that most
 * other IF systems have.  It returns TRUE if "undo" found, FALSE otherwise.
 */
static int gms_command_undo_special(const char *string) {
	int posn, end;
	assert(string);

	/* Find the start and end of the first string word. */
	posn = strspn(string, "\t ");
	end = posn + strcspn(string + posn, "\t ");

	/* See if string contains an "undo" request, with nothing following. */
	if (end - posn == (int)strlen("undo")
	        && gms_strncasecmp(string + posn, "undo", end - posn) == 0) {
		posn = end + strspn(string + end, "\t ");
		if (string[posn] == '\0')
			return TRUE;
	}

	return FALSE;
}


/*---------------------------------------------------------------------*/
/*  Glk port input functions                                           */
/*---------------------------------------------------------------------*/

/*
 * Input buffer allocated for reading input lines.  The buffer is filled
 * from either an input log, if one is currently being read, or from Glk
 * line input.  We also need an "undo" notification flag.
 */
enum { GMS_INPUTBUFFER_LENGTH = 256 };
static char gms_input_buffer[GMS_INPUTBUFFER_LENGTH];
static int gms_input_length = 0,
           gms_input_cursor = 0,
           gms_undo_notification = FALSE;

/* Table of single-character command abbreviations. */
typedef const struct {
	const char abbreviation;       /* Abbreviation character. */
	const char *const expansion;   /* Expansion string. */
} gms_abbreviation_t;
typedef gms_abbreviation_t *gms_abbreviationref_t;

static gms_abbreviation_t GMS_ABBREVIATIONS[] = {
	{'c', "close"},    {'g', "again"},  {'i', "inventory"},
	{'k', "attack"},   {'l', "look"},   {'p', "open"},
	{'q', "quit"},     {'r', "drop"},   {'t', "take"},
	{'x', "examine"},  {'y', "yes"},    {'z', "wait"},
	{'\0', NULL}
};


/*
 * gms_expand_abbreviations()
 *
 * Expand a few common one-character abbreviations commonly found in other
 * game systems, but not always normal in Magnetic Scrolls games.
 */
static void gms_expand_abbreviations(char *buffer, int size) {
	char *command, abbreviation;
	const char *expansion;
	gms_abbreviationref_t entry;
	assert(buffer);

	/* Ignore anything that isn't a single letter command. */
	command = buffer + strspn(buffer, "\t ");
	if (!(strlen(command) == 1
	        || (strlen(command) > 1 && Common::isSpace(command[1]))))
		return;

	/* Scan the abbreviations table for a match. */
	abbreviation = g_vm->glk_char_to_lower((unsigned char) command[0]);
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
		if ((int)(strlen(buffer) + strlen(expansion)) - 1 >= size)
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


/*
 * gms_buffer_input
 *
 * Read and buffer a line of input.  If there is an input log active, then
 * data is taken by reading this first.  Otherwise, the function gets a
 * line from Glk.
 *
 * It also makes special cases of some lines read from the user, either
 * handling commands inside them directly, or expanding abbreviations as
 * appropriate.  This is not reflected in the buffer, which is adjusted as
 * required before returning.
 */
static void gms_buffer_input(void) {
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
		chars = g_vm->glk_get_line_stream(gms_readlog_stream,
		                                  gms_input_buffer, sizeof(gms_input_buffer));
		if (chars > 0) {
			/* Echo the line just read in input style. */
			g_vm->glk_set_style(style_Input);
			g_vm->glk_put_buffer(gms_input_buffer, chars);
			g_vm->glk_set_style(style_Normal);

			/* Note how many characters buffered, and return. */
			gms_input_length = chars;
			return;
		}

		/*
		 * We're at the end of the log stream.  Close it, and then continue
		 * on to request a line from Glk.
		 */
		g_vm->glk_stream_close(gms_readlog_stream, NULL);
		gms_readlog_stream = NULL;
	}

	/*
	 * No input log being read, or we just hit the end of file on one.  Revert
	 * to normal line input; start by getting a new line from Glk.
	 */
	g_vm->glk_request_line_event(gms_main_window,
	                             gms_input_buffer, sizeof(gms_input_buffer) - 1, 0);
	gms_event_wait(evtype_LineInput, &event);

	/* Terminate the input line with a NUL. */
	assert(event.val1 <= sizeof(gms_input_buffer) - 1);
	gms_input_buffer[event.val1] = '\0';

	/* Special handling for "undo" commands. */
	if (gms_command_undo_special(gms_input_buffer)) {
		/* Write the "undo" to any input log. */
		if (gms_inputlog_stream) {
			g_vm->glk_put_string_stream(gms_inputlog_stream, gms_input_buffer);
			g_vm->glk_put_char_stream(gms_inputlog_stream, '\n');
		}

		/* Overwrite buffer with an empty line if we saw "undo". */
		gms_input_buffer[0] = '\n';
		gms_input_length = 1;

		gms_undo_notification = TRUE;
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
		g_vm->glk_put_string_stream(gms_inputlog_stream, gms_input_buffer);
		g_vm->glk_put_char_stream(gms_inputlog_stream, '\n');
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


/*
 * ms_getchar()
 *
 * Return the single next character to the interpreter.  This function
 * extracts characters from the input buffer until empty, when it then
 * tries to buffer more data.
 */
type8 ms_getchar(type8 trans) {
	/* See if we are at the end of the input buffer. */
	if (gms_input_cursor == gms_input_length) {
		/*
		 * Try to read in more data, and rewind buffer cursor.  As well as
		 * reading input, this may set an undo notification.
		 */
		gms_buffer_input();
		gms_input_cursor = 0;

		if (gms_undo_notification) {
			/*
			 * Clear the undo notification, and discard buffered input (usually
			 * just the '\n' placed there when the undo command was recognized).
			 */
			gms_undo_notification = FALSE;
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

#if 0
/*
 * gms_confirm()
 *
 * Print a confirmation prompt, and read a single input character, taking
 * only [YyNn] input.  If the character is 'Y' or 'y', return TRUE.
 */
static int gms_confirm(const char *prompt) {
	event_t event;
	unsigned char response;
	assert(prompt);

	/*
	 * Print the confirmation prompt, in a style that hints that it's from the
	 * interpreter, not the game.
	 */
	gms_standout_string(prompt);

	/* Wait for a single 'Y' or 'N' character response. */
	response = ' ';
	do {
		g_vm->glk_request_char_event(gms_main_window);
		gms_event_wait(evtype_CharInput, &event);

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
#endif

/*---------------------------------------------------------------------*/
/*  Glk port event functions                                           */
/*---------------------------------------------------------------------*/

/*
 * gms_event_wait()
 *
 * Process Glk events until one of the expected type arrives.  Return
 * the event of that type.
 */
static void gms_event_wait(glui32 wait_type, event_t *event) {
	assert(event);

	do {
		g_vm->glk_select(event);

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

		default:
			break;
		}
	} while (event->type != (EvType)wait_type);
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
 * The following values need to be passed between the startup_code and main
 * functions.
 */
static const char *gms_gamefile = NULL,      /* Name of game file. */
                   *gms_game_message = NULL;  /* Error message. */


/*
 * gms_establish_filenames()
 *
 * Given a game name, try to establish three filenames from it - the main game
 * text file, the (optional) graphics data file, and the (optional) hints
 * file.  Given an input "file" X, the function looks for X.MAG or X.mag for
 * game data, X.GFX or X.gfx for graphics, and X.HNT or X.hnt for hints.
 * If the input file already ends with .MAG, .GFX, or .HNT, the extension
 * is stripped first.
 *
 * The function returns NULL for filenames not available.  It's not fatal if
 * the graphics filename or hints filename is NULL, but it is if the main game
 * filename is NULL.  Filenames are malloc'ed, and need to be freed by the
 * caller.
 */
static void gms_establish_filenames(const char *name, char **text, char **graphics, char **hints) {
	char *base, *text_file, *graphics_file, *hints_file;
	Common::File stream;
	assert(name && text && graphics && hints);

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
			 * without looking for any associated graphics or hints files.
			 */
			*text = NULL;
			*graphics = NULL;
			*hints = NULL;

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

	/* Now allocate space for the return hints file. */
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
			 * No access to any hints file.  In this case, free memory and
			 * reset hints file to NULL.
			 */
			free(hints_file);
			hints_file = NULL;
		}
	}
	stream.close();

	/* Return the text file, and graphics and hints, which may be NULL. */
	*text = text_file;
	*graphics = graphics_file;
	*hints = hints_file;

	free(base);
}


/*
 * gms_startup_code()
 * gms_main()
 *
 * Together, these functions take the place of the original main().  The
 * first one is called from glkunix_startup_code(), to parse and generally
 * handle options.  The second is called from g_vm->glk_main(), and does the real
 * work of running the game.
 */
int gms_startup_code(int argc, char *argv[]) {
	int argv_index;

	/* Handle command line arguments. */
	for (argv_index = 1;
	        argv_index < argc && argv[argv_index][0] == '-'; argv_index++) {
		if (strcmp(argv[argv_index], "-nc") == 0) {
			gms_commands_enabled = FALSE;
			continue;
		}
		if (strcmp(argv[argv_index], "-na") == 0) {
			gms_abbreviations_enabled = FALSE;
			continue;
		}
		if (strcmp(argv[argv_index], "-np") == 0) {
			gms_graphics_enabled = FALSE;
			continue;
		}
		if (strcmp(argv[argv_index], "-ng") == 0) {
			gms_gamma_mode = GAMMA_OFF;
			continue;
		}
		if (strcmp(argv[argv_index], "-nx") == 0) {
			gms_animation_enabled = FALSE;
			continue;
		}
		if (strcmp(argv[argv_index], "-ne") == 0) {
			gms_prompt_enabled = FALSE;
			continue;
		}
		return FALSE;
	}

	/*
	 * Get the name of the game file.  Since we need this in our call from
	 * g_vm->glk_main, we need to keep it in a module static variable.  If the game
	 * file name is omitted, then here we'll set the pointer to NULL, and
	 * complain about it later in main.  Passing the message string around
	 * like this is a nuisance...
	 */
	if (argv_index == argc - 1) {
		gms_gamefile = argv[argv_index];
		gms_game_message = NULL;
#ifdef GARGLK
		{
			const char *s;
			s = strrchr(gms_gamefile, '\\');
			if (s)
				g_vm->garglk_set_story_name(s + 1);
			s = strrchr(gms_gamefile, '/');
			if (s)
				g_vm->garglk_set_story_name(s + 1);
		}
#endif
	} else {
		gms_gamefile = NULL;
		if (argv_index < argc - 1)
			gms_game_message = "More than one game file was given"
			                   " on the command line.";
		else
			gms_game_message = "No game file was given on the command line.";
	}

	/* All startup options were handled successfully. */
	return TRUE;
}

void gms_main() {
	char *text_file = NULL, *graphics_file = NULL, *hints_file = NULL;
	int ms_init_status, is_running;

	/* Ensure Magnetic Scrolls internal types have the right sizes. */
	if (!(sizeof(type8) == 1 && sizeof(type8s) == 1
	        && sizeof(type16) == 2 && sizeof(type16s) == 2
	        && sizeof(type32) == 4 && sizeof(type32s) == 4)) {
		gms_fatal("GLK: Types sized incorrectly, recompilation is needed");
		g_vm->glk_exit();
	}

	/* Create the main Glk window, and set its stream as current. */
	gms_main_window = g_vm->glk_window_open(0, 0, 0, wintype_TextBuffer, 0);
	if (!gms_main_window) {
		gms_fatal("GLK: Can't open main window");
		g_vm->glk_exit();
	}
	g_vm->glk_window_clear(gms_main_window);
	g_vm->glk_set_window(gms_main_window);
	g_vm->glk_set_style(style_Normal);

	/* If there's a problem with the game file, complain now. */
	if (!gms_gamefile) {
		assert(gms_game_message);
		gms_header_string("Glk Magnetic Error\n\n");
		gms_normal_string(gms_game_message);
		gms_normal_char('\n');
		g_vm->glk_exit();
		return;
	}

	/*
	 * Given the basic game name, try to come up with usable text, graphics,
	 * and hints filenames.  The graphics and hints files may be null, but the
	 * text file may not.
	 */
	gms_establish_filenames(gms_gamefile,
	                        &text_file, &graphics_file, &hints_file);
	if (!text_file) {
		assert(!graphics_file && !hints_file);
		gms_header_string("Glk Magnetic Error\n\n");
		gms_normal_string("Can't find or open game '");
		gms_normal_string(gms_gamefile);
		gms_normal_string("[.mag|.MAG]'");

		gms_normal_char('\n');
		g_vm->glk_exit();
	}

	/* Set the possibility of pictures depending on graphics file. */
	if (graphics_file) {
		/*
		 * Check Glk library capabilities, and note pictures are impossible if
		 * the library can't offer both graphics and timers.  We need timers to
		 * create the background "thread" for picture updates.
		 */
		gms_graphics_possible = g_vm->glk_gestalt(gestalt_Graphics, 0)
		                        && g_vm->glk_gestalt(gestalt_Timer, 0);
	} else
		gms_graphics_possible = FALSE;


	/*
	 * If pictures are impossible, clear pictures enabled flag.  That is, act
	 * as if -np was given on the command line, even though it may not have
	 * been.  If pictures are impossible, they can never be enabled.
	 */
	if (!gms_graphics_possible)
		gms_graphics_enabled = FALSE;

	/* Try to create a one-line status window.  We can live without it. */
	g_vm->glk_stylehint_set(wintype_TextGrid, style_User1, stylehint_ReverseColor, 1);
	gms_status_window = g_vm->glk_window_open(gms_main_window,
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
			g_vm->glk_window_close(gms_status_window, NULL);
		gms_header_string("Glk Magnetic Error\n\n");
		gms_normal_string("Can't load game '");
		gms_normal_string(gms_gamefile);
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
		g_vm->glk_exit();
	}

	/* Try to identify the game from its text file header. */
	gms_gameid_identify_game(text_file);

	/* Print out a short banner. */
	gms_header_string("\nMagnetic Scrolls Interpreter, version 2.3\n");
	gms_banner_string("Written by Niclas Karlsson\n"
	                  "Glk interface by Simon Baldwin\n\n");

	/* Look for failure to load just game graphics. */
	if (gms_graphics_possible && ms_init_status == 1) {
		/*
		 * Output a warning if graphics failed, but the main game text
		 * initialized okay.
		 */
		gms_standout_string("Error: Unable to open graphics file\n"
		                    "Continuing without pictures...\n\n");

		gms_graphics_possible = FALSE;
	}

	/* Run the game opcodes -- ms_rungame() returns FALSE on game end. */
	do {
		is_running = ms_rungame();
		g_vm->glk_tick();
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
		g_vm->glk_stream_close(gms_transcript_stream, NULL);
		gms_transcript_stream = NULL;
	}
	if (gms_inputlog_stream) {
		g_vm->glk_stream_close(gms_inputlog_stream, NULL);
		gms_inputlog_stream = NULL;
	}
	if (gms_readlog_stream) {
		g_vm->glk_stream_close(gms_readlog_stream, NULL);
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

/*
 * Safety flags, to ensure we always get startup before main, and that
 * we only get a call to main once.
 */
static int gms_startup_called = FALSE,
           gms_main_called = FALSE;

/*
 * g_vm->glk_main()
 *
 * Main entry point for Glk.  Here, all startup is done, and we call our
 * function to run the game.
 */
void glk_main(void) {
	assert(gms_startup_called && !gms_main_called);
	gms_main_called = TRUE;

	/* Call the interpreter main function. */
	gms_main();
}


/*
 * Glk arguments for UNIX versions of the Glk interpreter.
 */
#if 0
glkunix_argumentlist_t glkunix_arguments[] = {
	{
		(char *) "-nc", glkunix_arg_NoValue,
		(char *) "-nc        No local handling for Glk special commands"
	},
	{
		(char *) "-na", glkunix_arg_NoValue,
		(char *) "-na        Turn off abbreviation expansions"
	},
	{
		(char *) "-np", glkunix_arg_NoValue,
		(char *) "-np        Turn off pictures"
	},
	{
		(char *) "-ng", glkunix_arg_NoValue,
		(char *) "-ng        Turn off automatic gamma correction on pictures"
	},
	{
		(char *) "-nx", glkunix_arg_NoValue,
		(char *) "-nx        Turn off picture animations"
	},
	{
		(char *) "-ne", glkunix_arg_NoValue,
		(char *) "-ne        Turn off additional interpreter prompt"
	},
	{
		(char *) "", glkunix_arg_ValueCanFollow,
		(char *) "filename   game to run"
	},
	{NULL, glkunix_arg_End, NULL}
};
#endif

void write(const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	Common::String s = Common::String::format(fmt, ap);
	va_end(ap);
	g_vm->glk_put_buffer(s.c_str(), s.size());
}

void writeChar(char c) {
	g_vm->glk_put_char(c);
}

} // End of namespace Magnetic
} // End of namespace Glk
