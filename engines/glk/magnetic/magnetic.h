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

/* Based on Magnetic interpreter version 2.3 */

#ifndef GLK_MAGNETIC_MAGNETIC
#define GLK_MAGNETIC_MAGNETIC

#include "common/scummsys.h"
#include "glk/glk_api.h"
#include "glk/magnetic/magnetic_types.h"
#include "glk/magnetic/magnetic_defs.h"
#include "glk/magnetic/detection.h"

namespace Glk {
namespace Magnetic {

class Magnetic;

typedef void (Magnetic::*CommandPtr)(const char *argument);

/* Glk subcommands and handler functions. */
struct gms_command_t {
	CommandPtr handler;                 ///< Subcommand handler
	const char *const command;          ///< Glk subcommand
	bool takes_argument;                ///< Argument flag
	bool undo_return;                   ///< "Undo" return value
} ;
typedef gms_command_t *gms_commandref_t;


/**
 * Magnetic game interpreter
 */
class Magnetic : public GlkAPI {
public:
	static const gms_command_t GMS_COMMAND_TABLE[14];
	static const gms_gamma_t GMS_GAMMA_TABLE[38];
private:
	GammaMode gms_gamma_mode;
	bool gms_animation_enabled, gms_prompt_enabled;
	bool gms_abbreviations_enabled, gms_commands_enabled;
	bool gms_graphics_enabled;

	// Glk Magnetic Scrolls port version number
	const glui32 GMS_PORT_VERSION;

	/**
	 * We use a maximum of five Glk windows, one for status, one for pictures,
	 * two for hints, and one for everything else.  The status and pictures
	 * windows may be NULL, depending on user selections and the capabilities
	 * of the Glk library.  The hints windows will normally be NULL, except
	 * when in the hints subsystem.
	 */
	winid_t gms_main_window, gms_status_window, gms_graphics_window;
	winid_t gms_hint_menu_window, gms_hint_text_window;

	/**
	 * Transcript stream and input log.  These are NULL if there is no current
	 * collection of these strings.
	 */
	strid_t gms_transcript_stream, gms_inputlog_stream;

	// Input read log stream, for reading back an input log
	strid_t gms_readlog_stream;

	/* Note about whether graphics is possible, or not. */
	bool gms_graphics_possible;

	/* Magnetic Scrolls standard input prompt string. */
	const char *const GMS_INPUT_PROMPT;

	/**
	 * The game's name, suitable for printing out on a status line, or other
	 * location where game information is relevant.  Set on game startup, by
	 * identifying the game from its text file header.
	 */
	const char *gms_gameid_game_name;

	/*
	 * The current picture bitmap being displayed, its width, height, palette,
	 * animation flag, and picture id.
	 */
	type8 *gms_graphics_bitmap;
	type16 gms_graphics_width, gms_graphics_height;
	type16 gms_graphics_palette[GMS_PALETTE_SIZE]; /* = { 0, ... }; */
	bool gms_graphics_animated;
	type32 gms_graphics_picture;

	/*
	 * Flags set on new picture, and on resize or arrange events, and a flag
	 * to indicate whether background repaint is stopped or active.
	 */
	bool gms_graphics_new_picture, gms_graphics_repaint;
	bool gms_graphics_active;

	/* Flag to try to monitor the state of interpreter graphics. */
	bool gms_graphics_interpreter;

	/*
	 * Pointer to the two graphics buffers, one the off-screen representation
	 * of pixels, and the other tracking on-screen data.  These are temporary
	 * graphics malloc'ed memory, and should be free'd on exit.
	 */
	type8 *gms_graphics_off_screen, *gms_graphics_on_screen;

	/*
	 * Pointer to the current active gamma table entry.  Because of the way
	 * it's queried, this may not be NULL, otherwise we risk a race, with
	 * admittedly a very low probability, with the updater.  So, it's init-
	 * ialized instead to the gamma table.  The real value in use is inserted
	 * on the first picture update timeout call for a new picture.
	 */
	gms_gammaref_t gms_graphics_current_gamma;

	/*
	 * The number of colors used in the palette by the current picture.  This
	 * value is also at risk of a race with the updater, so it too has a mild
	 * lie for a default value.
	 */
	int gms_graphics_color_count;

	/**
	 * The interpreter feeds us status line characters one at a time, with Tab
	 * indicating right justify, and CR indicating the line is complete.  To get
	 * this to fit with the Glk event and redraw model, here we'll buffer each
	 * completed status line, so we have a stable string to output when needed.
	 * It's also handy to have this buffer for Glk libraries that don't support
	 * separate windows.
	 */
	char gms_status_buffer[GMS_STATBUFFER_LENGTH];
	int gms_status_length;

	/*
	 * Flag for if the user entered "help" as their last input, or if hints have
	 * been silenced as a result of already using a Glk command.
	 */
	int gms_help_requested, gms_help_hints_silenced;

	/*
	 * Output buffer.  We receive characters one at a time, and it's a bit
	 * more efficient for everyone if we buffer them, and output a complete
	 * string on a flush call.
	 */
	char *gms_output_buffer;
	int gms_output_allocation, gms_output_length;

	/*
	 * Flag to indicate if the last buffer flushed looked like it ended in a
	 * ">" prompt.
	 */
	int gms_output_prompt;

	/*
	 * Note of the interpreter's hints array.  Note that keeping its address
	 * like this assumes that it's either static or heap in the interpreter.
	 */
	ms_hint *gms_hints;

	/* Details of the current hint node on display from the hints array. */
	type16 gms_current_hint_node;

	/*
	 * Array of cursors for each hint.  The cursor indicates the current hint
	 * position in a folder, and the last hint shown in text hints.  Space
	 * is allocated as needed for a given set of hints, and needs to be freed
	 * on interpreter exit.
	 */
	int *gms_hint_cursor;

	/*
	 * Input buffer allocated for reading input lines.  The buffer is filled
	 * from either an input log, if one is currently being read, or from Glk
	 * line input.  We also need an "undo" notification flag.
	 */
	char gms_input_buffer[GMS_INPUTBUFFER_LENGTH];
	int gms_input_length, gms_input_cursor, gms_undo_notification;

	/*
	 * The following values need to be passed between the startup_code and main
	 * functions.
	 */
	const char *gms_game_message;  /* Error message. */

	/*
	 * Safety flags, to ensure we always get startup before main, and that
	 * we only get a call to main once.
	 */
	int gms_startup_called, gms_main_called;
private:
	type32 dreg[8], areg[8], i_count, string_size, rseed, pc, arg1i, mem_size;
	type16 properties, fl_sub, fl_tab, fl_size, fp_tab, fp_size;
	type8 zflag, nflag, cflag, vflag, byte1, byte2, regnr, admode, opsize;
	type8 *arg1, *arg2, is_reversible, running, tmparg[4];
	type8 lastchar, version, sd;
	type8 *decode_table, *restart, *code, *string, *string2;
	type8 *string3, *dict;
	type8 quick_flag, gfx_ver, *gfx_buf, *gfx_data;
	type8 *gfx2_hdr, *gfx2_buf;
	const char *gfx2_name;
	type16 gfx2_hsize;
	Common::File *gfx_fp;
	type8 *snd_buf, *snd_hdr;
	type16 snd_hsize;
	Common::File *snd_fp;

	type32 undo_regs[2][18], undo_pc, undo_size;
	type8 *undo[2], undo_stat[2];
	type16 gfxtable, table_dist;
	type16 v4_id, next_table;

#ifndef NO_ANIMATION
	type16 pos_table_size;
	type8 *command_table;
	type16s command_index;
	type16s pos_table_index;
	type16s pos_table_max;
	type8 anim_repeat;
	type16 pos_table_count[MAX_POSITIONS];
	picture anim_frame_table[MAX_ANIMS];
	ms_position pos_table[MAX_POSITIONS][MAX_ANIMS];
	lookup anim_table[MAX_POSITIONS];
	ms_position pos_array[MAX_FRAMES];
#endif

	/* Hint support */
	ms_hint *hints;
	type8 *hint_contents;

	/**
	 * Weighting values for calculating the luminance of a color.  There are
	 * two commonly used sets of values for these -- 299,587,114, taken from
	 * NTSC (Never The Same Color) 1953 standards, and 212,716,72, which is the
	 * set that modern CRTs tend to match.  The NTSC ones seem to give the best
	 * subjective results.
	 */
	const gms_rgb_t GMS_LUMINANCE_WEIGHTS;

	type8 *_saveData;
	size_t _saveSize;
private:
	type8 buffer[80], xpos, bufpos, log_on, ms_gfx_enabled, filename[256];
	Common::DumpFile *_log1, *_log2;
private:
	/* Method local statics in original code */
	glui32 crc_table[BYTE_MAX_VAL + 1];
	int luminance_weighting;
	gms_gammaref_t linear_gamma;
	uint32 pic_current_crc;			/* CRC of the current picture */
	uint32 hints_current_crc;		/* CRC of hints */
	bool hints_crc_initialized;
private:
	/**
	 * Performs initialization
	 */
	void initialize();

	/**
	 * Initializes settings from the ScummVM configuration
	 */
	void initializeSettings();

	/**
	 * Initializes the CRC table
	 */
	void initializeCRC();

	/**
	 * Initializes the linear gamma entry
	 */
	void initializeLinearGamma();

	/**
	 * Fatal error handler.  The function returns, expecting the caller to
	 * abort() or otherwise handle the error.
	 */
	void gms_fatal(const char *string);

	/**
	 * Non-failing malloc. Calls error if memory allocation fails
	 */
	void *gms_malloc(size_t size);

	/**
	 * Non-failing realloc. Calls error if memory allocation fails
	 */
	void *gms_realloc(void *ptr, size_t size);

	/**
	 * Local comparison routine that doesn't have an ANSI standard
	 */
	int gms_strncasecmp(const char *s1, const char *s2, size_t n);

	/**
	 * Local comparison routine that doesn't have an ANSI standard
	 */
	int gms_strcasecmp(const char *s1, const char *s2);

	/**
	 * Return the CRC of the bytes in buffer[0..length-1].
	 *
	 * This algorithm is taken from the PNG specification, version 1.0.
	 */
	glui32 gms_get_buffer_crc(const void *void_buffer, size_t length);

	/**
	 * Endian-safe unsigned 32 bit integer read from game text file.  Returns
	 * 0 on error, a known unused table value.
	 */
	type32 gms_gameid_read_uint32(int offset, Common::SeekableReadStream *stream);

	/**
	 * Identify a game from its text file header, and cache the game's name for
	 * later queries.  Sets the cache to NULL if not found.
	 */
	void gms_gameid_identify_game(const Common::String &text_file);

	/**
	 * Return the name of the game, or NULL if not identifiable.
	 */
	const char *gms_gameid_get_game_name() const {
		return gms_gameid_game_name;
	}

	/**
	 * If it's not open, open the graphics window.  Returns TRUE if graphics
	 * was successfully started, or already on.
	 */
	int gms_graphics_open();

	/**
	 * If open, close the graphics window and set back to NULL.
	 */
	void gms_graphics_close();

	/**
	 * If graphics enabled, start any background picture update processing.
	 */
	void gms_graphics_start();

	/**
	 * Stop any background picture update processing.
	 */
	void gms_graphics_stop();

	/**
	 * Return TRUE if graphics are currently being displayed, FALSE otherwise.
	 */
	int gms_graphics_are_displayed() const {
		return gms_graphics_window != nullptr;
	}

	/**
	 * Set up a complete repaint of the current picture in the graphics window.
	 * This function should be called on the appropriate Glk window resize and
	 * arrange events.
	 */
	void gms_graphics_paint();

	/**
	 * Restart graphics as if the current picture is a new picture.  This
	 * function should be called whenever graphics is re-enabled after being
	 * disabled, on change of gamma color correction policy, and on change
	 * of animation policy.
	 */
	void gms_graphics_restart();

	/**
	 * Analyze an image, and return the usage count of each palette color, and
	 * an overall count of how many colors out of the palette are used.  NULL
	 * arguments indicate no interest in the return value.
	 */
	void gms_graphics_count_colors(type8 bitmap[], type16 width, type16 height,
		int *color_count, long color_usage[]);

	/**
	 * General graphics color conversion
	 */
	void gms_graphics_game_to_rgb_color(type16 color, gms_gammaref_t gamma,
		gms_rgbref_t rgb_color);

	/**
	 * General graphics color conversion
	 */
	void gms_graphics_split_color(glui32 color, gms_rgbref_t rgb_color);

	/**
	 * General graphics color conversion
	 */
	glui32 gms_graphics_combine_color(gms_rgbref_t rgb_color);

	/**
	 * General graphics color conversion
	 */
	int gms_graphics_color_luminance(gms_rgbref_t rgb_color);

	/**
	 * Calculate the contrast variance of the given palette and color usage, at
	 * the given gamma correction level.  Helper functions for automatic gamma
	 * correction.
	 */
	static int gms_graphics_compare_luminance(const void *void_first, const void *void_second);

	/**
	 * Calculate the contrast variance of the given palette and color usage, at
	 * the given gamma correction level.  Helper functions for automatic gamma
	 * correction.
	 */
	long gms_graphics_contrast_variance(type16 palette[], long color_usage[],
		gms_gammaref_t gamma);

	/**
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
	gms_gammaref_t gms_graphics_equal_contrast_gamma(type16 palette[], long color_usage[]);

	/**
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
	gms_gammaref_t gms_graphics_select_gamma(type8 bitmap[], type16 width,
		type16 height, type16 palette[]);

	/**
	 * Clear the graphics window, and border and shade the area where the
	 * picture is going to be rendered.  This attempts a small raised effect
	 * for the picture, in keeping with modern trends.
	 */
	void gms_graphics_clear_and_border(winid_t glk_window,
		int x_offset, int y_offset, int pixel_size, type16 width, type16 height);

	/**
	 * Convert a Magnetic Scrolls color palette to a Glk one, using the given
	 * gamma corrections.
	 */
	void gms_graphics_convert_palette(type16 ms_palette[], gms_gammaref_t gamma,
		glui32 glk_palette[]);

	/**
	 * Given a picture width and height, return the x and y offsets to center
	 * this picture in the current graphics window.
	 */
	void gms_graphics_position_picture(winid_t glk_window,
		int pixel_size, type16 width, type16 height,
		int *x_offset, int *y_offset);

	/**
	 * Apply a single animation frame to the given off-screen image buffer, using
	 * the frame bitmap, width, height and mask, the off-screen buffer, and the
	 * width and height of the main picture.
	 *
	 * Note that 'mask' may be NULL, implying that no frame pixel is transparent.
	 */
	void gms_graphics_apply_animation_frame(type8 bitmap[],
		type16 frame_width, type16 frame_height, type8 mask[],
		int frame_x, int frame_y, type8 off_screen[], type16 width, type16 height);

	/**
	 * This function finds and applies the next set of animation frames to the
	 * given off-screen image buffer.  It's handed the width and height of the
	 * main picture, and the off-screen buffer.
	 *
	 * It returns FALSE if at the end of animations, TRUE if more animations
	 * remain.
	 */
	int gms_graphics_animate(type8 off_screen[], type16 width, type16 height);

#ifndef GARGLK
	/**
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
	int gms_graphics_is_vertex(type8 off_screen[], type16 width, type16 height,
		int x, int y);

	/**
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
	int gms_graphics_compare_layering_inverted(const void *void_first,
		const void *void_second);

	void gms_graphics_assign_layers(type8 off_screen[], type8 on_screen[],
		type16 width, type16 height, int layers[], long layer_usage[]);

	/**
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
	void gms_graphics_paint_region(winid_t glk_window, glui32 palette[], int layers[],
		type8 off_screen[], type8 on_screen[], int x, int y, int x_offset, int y_offset,
		int pixel_size, type16 width, type16 height);
	#endif

	void gms_graphics_paint_everything(winid_t glk_window,
		glui32 palette[], type8 off_screen[], int x_offset, int y_offset,
		type16 width, type16 height);

	/**
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
	void gms_graphics_timeout();

	/**
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
	void ms_showpic(type32 picture, type8 mode);

	/**
	 * Return TRUE if the graphics module data is loaded with a usable picture,
	 * FALSE if there is no picture available to display.
	 */
	int gms_graphics_picture_is_available() const {
		return gms_graphics_bitmap != nullptr;
	}

	/**
	 * Return the width, height, and animation flag of the currently loaded
	 * picture.  The function returns FALSE if no picture is loaded, otherwise
	 * TRUE, with picture details in the return arguments.
	 */
	int gms_graphics_get_picture_details(int *width, int *height, int *is_animated);

	/**
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
	int gms_graphics_get_rendering_details(const char **gamma, int *color_count,
		int *is_active);

	/**
	 * Return TRUE if it looks like interpreter graphics are turned on, FALSE
	 * otherwise.
	 */
	int gms_graphics_interpreter_enabled();

	/*
	 * gms_graphics_cleanup()
	 *
	 * Free memory resources allocated by graphics functions.  Called on game
	 * end.
	 */
	void gms_graphics_cleanup();

	/*---------------------------------------------------------------------*/
	/*  Glk port status line functions                                     */
	/*---------------------------------------------------------------------*/

	/**
	 * Receive one status character from the interpreter.  Characters are
	 * buffered internally, and on CR, the buffer is copied to the main static
	 * status buffer for use by the status line printing function.
	 */
	void ms_statuschar(type8 c);

	/*
	 * Update the information in the status window with the current contents of
	 * the completed status line buffer, or a default string if no completed
	 * status line.
	 */
	void gms_status_update();

	/**
	 * Print the current contents of the completed status line buffer out in the
	 * main window, if it has changed since the last call.  This is for non-
	 * windowing Glk libraries.
	 */
	void gms_status_print();

	/*
	 * gms_status_notify()
	 *
	 * Front end function for updating status.  Either updates the status window
	 * or prints the status line to the main window.
	 */
	void gms_status_notify();

	/*
	 * gms_status_redraw()
	 *
	 * Redraw the contents of any status window with the buffered status string.
	 * This function should be called on the appropriate Glk window resize and
	 * arrange events.
	 */
	void gms_status_redraw();

	/*---------------------------------------------------------------------*/
	/*  Glk port output functions                                          */
	/*---------------------------------------------------------------------*/

	/*
	 * gms_output_register_help_request()
	 * gms_output_silence_help_hints()
	 * gms_output_provide_help_hint()
	 *
	 * Register a request for help, and print a note of how to get Glk command
	 * help from the interpreter unless silenced.
	 */
	void gms_output_register_help_request();

	void gms_output_silence_help_hints();

	void gms_output_provide_help_hint();

	/*
	 * gms_game_prompted()
	 *
	 * Return TRUE if the last game output appears to have been a ">" prompt.
	 * Once called, the flag is reset to FALSE, and requires more game output
	 * to set it again.
	 */
	int gms_game_prompted();

	/*
	 * gms_detect_game_prompt()
	 *
	 * See if the last non-newline-terminated line in the output buffer seems
	 * to be a prompt, and set the game prompted flag if it does, otherwise
	 * clear it.
	 */
	void gms_detect_game_prompt();

	/*
	 * gms_output_delete()
	 *
	 * Delete all buffered output text.  Free all malloc'ed buffer memory, and
	 * return the buffer variables to their initial values.
	 */
	void gms_output_delete();

	/*
	 * gms_output_flush()
	 *
	 * Flush any buffered output text to the Glk main window, and clear the
	 * buffer.
	 */
	void gms_output_flush();

	/*
	 * ms_putchar()
	 *
	 * Buffer a character for eventual printing to the main window.
	 */
	void ms_putchar(type8 c);

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
	void gms_styled_string(glui32 style, const char *message);

	void gms_styled_char(glui32 style, char c);

	void gms_standout_string(const char *message);

	void gms_normal_string(const char *message);

	void gms_normal_char(char c);

	void gms_header_string(const char *message);

	void gms_banner_string(const char *message);

	/**
	 * Handle a core interpreter call to flush the output buffer.  Because Glk
	 * only flushes its buffers and displays text on g_vm->glk_select(), we can ignore
	 * these calls as long as we call gms_output_flush() when reading line input.
	 *
	 * Taking ms_flush() at face value can cause game text to appear before status
	 * line text where we are working with a non-windowing Glk, so it's best
	 * ignored where we can.
	 */
	void ms_flush();


	/*---------------------------------------------------------------------*/
	/*  Glk port hint functions                                            */
	/*---------------------------------------------------------------------*/

	/**
	 * Return the maximum hint node referred to by the tree under the given node.
	 * The result is the largest index found, or node, if greater.  Because the
	 * interpreter doesn't supply it, we need to uncover it the hard way.  The
	 * function is recursive, and since it is a tree search, assumes that hints
	 * is a tree, not a graph.
	 */
	type16 gms_get_hint_max_node(const ms_hint hints_[], type16 node);

	/**
	 * Return the content string for a given hint number within a given node.
	 * This counts over 'number' ASCII NULs in the node's content, returning
	 * the address of the string located this way.
	 */
	const char *gms_get_hint_content(const ms_hint hints_[], type16 node, int number);

	/**
	 * Return the topic string for a given hint node.  This is found by searching
	 * the parent node for a link to the node handed in.  For the root node, the
	 * string is defaulted, since the root node has no parent.
	 */
	const char *gms_get_hint_topic(const ms_hint hints_[], type16 node);

	/**
	 * If not already open, open the hints windows.  Returns TRUE if the windows
	 * opened, or were already open.
	 *
	 * The function creates two hints windows -- a text grid on top, for menus,
	 * and a text buffer below for hints.
	 */
	int gms_hint_open();

	/**
	 * If open, close the hints windows.
	 */
	void gms_hint_close();

	/**
	 * Return TRUE if hints windows are available.  If they're not, the hints
	 * system will need to use alternative output methods.
	 */
	int gms_hint_windows_available();

	/**
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
	void gms_hint_menu_print(int line, int column, const char *string_,
		glui32 width, glui32 height);

	void gms_hint_menu_header(int line, const char *string_,
		glui32 width, glui32 height);

	void gms_hint_menu_justify(int line, const char *left_string,
		const char *right_string, glui32 width, glui32 height);

	void gms_hint_text_print(const char *string_);

	void gms_hint_menutext_start();

	void gms_hint_menutext_done();

	/**
	 * Request and return a character event from the hints windows.  In practice,
	 * this means either of the hints windows if available, or the main window
	 * if not.
	 */
	void gms_hint_menutext_char_event(event_t *event);

	/**
	 * Arrange the hints windows so that the hint menu window has the requested
	 * number of lines.  Returns the actual hint menu window width and height,
	 * or defaults if no hints windows are available.
	 */
	void gms_hint_arrange_windows(int requested_lines, glui32 *width, glui32 *height);

	/**
	 * Update the hints windows for the given folder hint node.
	 */
	void gms_hint_display_folder(const struct ms_hint hints_[],
		const int cursor[], type16 node);

	/**
	 * Update the hints windows for the given text hint node.
	 */
	void gms_hint_display_text(const struct ms_hint hints_[],
		const int cursor[], type16 node);

	/**
	 * Display the given hint using the appropriate display function.
	 */
	void gms_hint_display(const struct ms_hint hints_[],
		const int cursor[], type16 node);

	/**
	 * Handle a Glk keycode for the given folder hint.  Return the next node to
	 * handle, or the special end-hints on Quit at the root node.
	 */
	type16 gms_hint_handle_folder(const ms_hint hints_[],
		int cursor[], type16 node, glui32 keycode);

	/**
	 * Handle a Glk keycode for the given text hint.  Return the next node to
	 * handle.
	 */
	type16 gms_hint_handle_text(const ms_hint hints[],
		int cursor[], type16 node, glui32 keycode);

	/**
	 * Handle a Glk keycode for the given hint using the appropriate handler
	 * function.  Return the next node to handle.
	 */
	type16 gms_hint_handle(const ms_hint hints_[], int cursor[],
		type16 node, glui32 keycode);

	/**
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
	type8 ms_showhints(ms_hint *hints_);

	/**
	 * Update the hints windows for the current hint.  This function should be
	 * called from the event handler on resize events, to repaint the hints
	 * display.  It does nothing if no hints windows have been opened, since
	 * in this case, there's no resize action required -- either we're not in
	 * the hints subsystem, or hints are being displayed in the main game
	 * window, for whatever reason.
	 */
	void gms_hint_redraw();

	/**
	 * Free memory resources allocated by hints functions.  Called on game
	 * end.
	 */
	void gms_hints_cleanup();

	void ms_playmusic(type8 *midi_data, type32 length, type16 tempo);

	/*---------------------------------------------------------------------*/
	/*  Glk command escape functions                                       */
	/*---------------------------------------------------------------------*/

	/**
	 * Stub function for the undo command.  The real work is to return the
	 * undo code to the input functions.
	 */
	void gms_command_undo(const char *argument);

	/**
	 * Turn game output scripting (logging) on and off.
	 */
	void gms_command_script(const char *argument);

	/**
	 * Turn game input logging on and off.
	 */
	void gms_command_inputlog(const char *argument);

	/**
	 * Set the game input log, to read input from a file.
	 */
	void gms_command_readlog(const char *argument);

	/**
	 * Turn abbreviation expansions on and off.
	 */
	void gms_command_abbreviations(const char *argument);

	/**
	 * Enable or disable graphics more permanently than is done by the main
	 * interpreter.  Also, print out a few brief details about the graphics
	 * state of the program.
	 */
	void gms_command_graphics(const char *argument);

	/**
	 * Enable or disable picture gamma corrections.
	 */
	void gms_command_gamma(const char *argument);

	/**
	 * Enable or disable picture animations.
	 */
	void gms_command_animations(const char *argument);

	/**
	 * Turn the extra "> " prompt output on and off.
	 */
	void gms_command_prompts(const char *argument);

	/**
	 * gms_command_print_version_number()
	 * gms_command_version()
	 *
	 * Print out the Glk library version number.
	 */
	void gms_command_print_version_number(glui32 version);

	void gms_command_version(const char *argument);

	/**
	 * Turn command escapes off.  Once off, there's no way to turn them back on.
	 * Commands must be on already to enter this function.
	 */
	void gms_command_commands(const char *argument);

	/**
	 * Report all current Glk settings.
	 */
	void gms_command_summary(const char *argument);

	/**
	 * Document the available Glk commands.
	 */
	void gms_command_help(const char *command);

	/**
	 * This function is handed each input line.  If the line contains a specific
	 * Glk port command, handle it and return TRUE, otherwise return FALSE.
	 *
	 * On unambiguous returns, it will also set the value for undo_command to the
	 * table undo return value.
	 */
	int gms_command_escape(const char *string_, int *undo_command);

	/**
	 * This function makes a special case of the input line containing the single
	 * word "undo", treating it as if it is "glk undo".  This makes life a bit
	 * more convenient for the player, since it's the same behavior that most
	 * other IF systems have.  It returns TRUE if "undo" found, FALSE otherwise.
	 */
	int gms_command_undo_special(const char *string_);

	/*---------------------------------------------------------------------*/
	/*  Glk port input functions                                           */
	/*---------------------------------------------------------------------*/

	/**
	 * Expand a few common one-character abbreviations commonly found in other
	 * game systems, but not always normal in Magnetic Scrolls games.
	 */
	void gms_expand_abbreviations(char *buffer_, int size);

	/**
	 * Read and buffer a line of input.  If there is an input log active, then
	 * data is taken by reading this first.  Otherwise, the function gets a
	 * line from Glk.
	 *
	 * It also makes special cases of some lines read from the user, either
	 * handling commands inside them directly, or expanding abbreviations as
	 * appropriate.  This is not reflected in the buffer, which is adjusted as
	 * required before returning.
	 */
	void gms_buffer_input();

	/**
	 * Return the single next character to the interpreter.  This function
	 * extracts characters from the input buffer until empty, when it then
	 * tries to buffer more data.
	 */
	type8 ms_getchar(type8 trans);

	/*---------------------------------------------------------------------*/
	/*  Glk port event functions                                           */
	/*---------------------------------------------------------------------*/

	/**
	 * Process Glk events until one of the expected type arrives.  Return
	 * the event of that type.
	 */
	void gms_event_wait(glui32 wait_type, event_t *event);

	/*---------------------------------------------------------------------*/
	/*  Functions intercepted by link-time wrappers                        */
	/*---------------------------------------------------------------------*/

	/**
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
	int __wrap_toupper(int ch);

	int __wrap_tolower(int ch);

	/*---------------------------------------------------------------------*/
	/*  main and options parsing                                           */
	/*---------------------------------------------------------------------*/

	/**
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
	void gms_establish_filenames(const char *name, char **text, char **graphics, char **hints_);

	void gms_main();

	/*---------------------------------------------------------------------*/
	/*  Linkage between Glk entry/exit calls and the Magnetic interpreter  */
	/*---------------------------------------------------------------------*/

	/*
	 * glk_main()
	 *
	 * Main entry point for Glk.  Here, all startup is done, and we call our
	 * function to run the game.
	 */
	void glk_main();

	void write(const char *fmt, ...);

	void writeChar(char c);
private:
	/* Convert virtual pointer to effective pointer */
	type8 *effective(type32 ptr);

	static type32 read_l(type8 *ptr) {
		return (type32)((type32)ptr[0] << 24 | (type32)ptr[1] << 16 | (type32)ptr[2] << 8 | (type32)ptr[3]);
	}

	static type16 read_w(type8 *ptr) {
		return (type16)(ptr[0] << 8 | ptr[1]);
	}

	static type32 read_l2(type8 *ptr) {
		return ((type32)ptr[1] << 24 | (type32)ptr[0] << 16 | (type32)ptr[3] << 8 | (type32)ptr[2]);
	}

	static type16 read_w2(type8 *ptr) {
		return (type16)(ptr[1] << 8 | ptr[0]);
	}

	static void write_l(type8 *ptr, type32 val);

	static void write_w(type8 *ptr, type16 val);

	/* Standard rand - for equal cross-platform behaviour */
	void ms_seed(type32 seed) {
		rseed = seed;
	}

	type32 rand_emu();

	void ms_freemem();

	type8 ms_is_running() const {
		return running;
	}

	type8 ms_is_magwin() const {
		return (version == 4) ? 1 : 0;
	}

	void ms_stop() {
		running = 0;
	}

	type8 init_gfx1(type8 *header);

	type8 init_gfx2(type8 *header);

	type8 init_snd(type8 *header);

	/* zero all registers and flags and load the game */
	type8 ms_init(const char *name, const char *gfxname, const char *hntname, const char *sndname);

	type8 is_blank(type16 line, type16 width);

	type8 *ms_extract1(type8 pic, type16 *w, type16 *h, type16 *pal);

	type16s find_name_in_header(const char *name, type8 upper);

	void extract_frame(struct picture *pic);

	type8 *ms_extract2(const char *name, type16 *w, type16 *h, type16 *pal, type8 *is_anim);

	type8 *ms_extract(type32 pic, type16 *w, type16 *h, type16 *pal, type8 *is_anim);

	type8 ms_animate(struct ms_position **positions, type16 *count);

	type8 *ms_get_anim_frame(type16s number, type16 *width, type16 *height, type8 **mask);

	type8 ms_anim_is_repeating() const;

	type16s find_name_in_sndheader(const char *name);

	type8 *sound_extract(const char *name, type32 *length, type16 *tempo);

	void save_undo();

	type8 ms_undo();

#ifdef LOGEMU
	void log_status();
#endif

	void ms_status();

	type32 ms_count() const {
		return i_count;
	}

	/* align register pointer for word/byte accesses */
	type8 *reg_align(type8 *ptr, type8 size);

	type32 read_reg(int i, int s);

	void write_reg(int i, int s, type32 val);

	/* [35c4] */
	void char_out(type8 c);

	/* extract addressing mode information [1c6f] */
	void set_info(type8 b);

	/* read a word and increase pc */
	void read_word();

	/* get addressing mode and set arg1 [1c84] */
	void set_arg1();

	/* get addressing mode and set arg2 [1bc5] */
	void set_arg2_nosize(int use_dx, type8 b);

	void set_arg2(int use_dx, type8 b);

	/* [1b9e] */
	void swap_args();

	/* [1cdc] */
	void push(type32 c);

	/* [1cd1] */
	type32 pop();

	/* check addressing mode and get argument [2e85] */
	void get_arg();

	void set_flags();

	/* [263a] */
	int condition(type8 b);

	/* [26dc] */
	void branch(type8 b);

	/* [2869] */
	void do_add(type8 adda);

	/* [2923] */
	void do_sub(type8 suba);

	/* [283b] */
	void do_eor();

	/* [280d] */
	void do_and();

	/* [27df] */
	void do_or();

	/* [289f] */
	void do_cmp();

	/* [2973] */
	void do_move();

	type8 do_btst(type8 a);

	/* bit operation entry point [307c] */
	void do_bop(type8 b, type8 a);

	void check_btst();

	void check_lea();

	/* [33cc] */
	void check_movem();

	/* [3357] */
	void check_movem2();

	/* [30e4] in Jinxter, ~540 lines of 6510 spaghetti-code */
	/* The mother of all bugs, but hey - no gotos used :-) */
	void dict_lookup();

	/* A0=findproperties(D0) [2b86], properties_ptr=[2b78] A0FE */
	void do_findprop();

	void write_string();

	void output_number(type16 number);

	type16 output_text(const char *text);

	type16s hint_input();

	type16 show_hints_text(ms_hint *hintsData, type16 index);

	void do_line_a();

	/* emulate an instruction [1b7e] */
	type8 ms_rungame();
private:
	type8 ms_load_file(const char *name, type8 *ptr, type16 size);

	type8 ms_save_file(const char *name, type8 *ptr, type16 size);

	void script_write(type8 c);

	void transcript_write(type8 c);
public:
	/**
	 * Constructor
	 */
	Magnetic(OSystem *syst, const GlkGameDescription &gameDesc);

	/**
	 * Run the game
	 */
	void runGame() override;

	/**
	 * Returns the running interpreter type
	 */
	InterpreterType getInterpreterType() const override {
		return INTERPRETER_MAGNETIC;
	}

	/**
	 * The Magnetic engine currently doesn't support loading savegames from the GMM
	 */
	bool canLoadGameStateCurrently() override {
		return false;
	}

	/**
	 * The Magnetic engine currently doesn't support saving games from the GMM
	 */
	bool canSaveGameStateCurrently() override {
		return false;
	}

	/**
	 * Load a savegame from the passed Quetzal file chunk stream
	 */
	Common::Error readSaveData(Common::SeekableReadStream *rs) override;

	/**
	 * Save the game. The passed write stream represents access to the UMem chunk
	 * in the Quetzal save file that will be created
	 */
	Common::Error writeGameData(Common::WriteStream *ws) override;
};

extern Magnetic *g_vm;

} // End of namespace Magnetic
} // End of namespace Glk

#endif
