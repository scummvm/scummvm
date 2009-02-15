/***************************************************************************
 gfx_console.c Copyright (C) 2002 Christoph Reichenbach


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
/* Graphical on-screen console */


#include <console.h>
#include <ctype.h>

#ifdef WANT_CONSOLE
#  define CON_MAX_CLUSTERS 16

#define CON_CLUSTER_SIZE 64

/* Number of console entries stored = CON_MAX_CLUSTERS * CON_CLUSTER_SIZE */

#define CON_ENTRY_USED(e) ((e).height > 0)

#define CON_INPUT_HISTORY_SIZE 64
#define CON_BUILTIN_CHARS_NR 256
#define CON_BUILTIN_CHARS_HEIGHT 8
#define CON_BUILTIN_CHARS_WIDTH 8

#define CON_OVERWRAP_SYMBOL 0x10

#define CON_GFX_PROMPT "$ "

extern byte con_builtin_font_data[];

typedef struct {
	int height; /* Number of pixels occupied by this entry, or 0 if unused */
	int pixmaps_nr;
	gfx_pixmap_t **pixmaps;
	char *text; /* Dynamically allocated */
} con_entry_t;


typedef struct _con_buffer {
	con_entry_t entries[CON_CLUSTER_SIZE];
	struct _con_buffer *next;
	struct _con_buffer *prev;
} con_buffer_t;

typedef struct {
	con_buffer_t *buf;
	int entry;
	int offset; /* pixel offset relative to the bottom */
} con_pos_t;

typedef struct {
	int locked_to_end;
	con_pos_t pos;
	gfx_pixmap_t *background;
	gfx_color_t color_bg, color_transparent, color_cursor, color_text, color_input;
	gfx_pixmap_t *input_prompt;
	gfx_pixmap_t *input_precursor;
	gfx_pixmap_t *input_oncursor;
	gfx_pixmap_t *input_postcursor;
	int cursor_position;
	int input_window; /* First character to display, may be up to -2 to include prompt */
	char *input_text;
	int input_bufsize;
	int input_prompt_pos; /* -strlen(input prompt) */
	int input_history_pos;
	int partial_write;
	char *input_history[CON_INPUT_HISTORY_SIZE];
} con_t;


/* Visual options for the con */
static int con_bg_red = 0;
static int con_bg_green = 0;
static int con_bg_blue = 64;
static int con_bg_alpha = 64;
static int con_displayed_lines = 180;
static int con_border_width = 3;

static int con_top_buffer_entry_nr = 0;
static int con_buffer_clusters = 0;
static con_buffer_t *con_buffer = NULL;
static con_buffer_t *con_buffer_tail = NULL;
static con_t con; /* The global con */
static gfx_bitmap_font_t con_font;
static int con_font_initialized = 0;
static gfx_state_t *con_last_gfx_state = NULL; 

/*-- Forwards --*/
static void
_con_free_entry_pixmaps(gfx_state_t *state, con_entry_t *entry);
/* Free all pixmaps from the specified entry
** Parameters: (gfx_state_t *) state: The state to free from
**             (con_entry_t *) entry: The entry to liberate from its pixmaps
*/

static gfx_pixmap_t *
_con_render_text(gfx_state_t *state, char *text, int len,
		 gfx_color_t *color, gfx_color_t *bgcolor);
/* Renders the specified text in the specified fg color
** Parameters: (gfx_state_t *) state: The state to render in
**             (char *) text: The text to render
**             (int) len: Length of the text to render, or -1 for strlen
**             (gfx_color_t *) color: The fg color to choose
**             (gfx_color_t *) bgcolor: The bg color to choose
** Returns   : (gfx_pixmap_t *) An appropriate pixmap
*/


void
con_jump_to_end(gfx_state_t *gfx);
/* Makes the console jump to the logical end of its buffer and redraws
** Parameters: (gfx_stat_t *) gfx: The graphics state to use fo rdrawing
*/

static void
_con_redraw(gfx_state_t *state, int update_display_field, int update_input_field);
/* Performs a (partial) redraw
** Parameters: (gfx_state_t *) state: The graphical state to draw with
**             (int) update_display_field: Whether the upper part of the
**		     console, used to display test, should be updated
**             (int) update_input_field: Whether the lower part of the
**                   console, used to display the user input, should be
**                   updated
*/

static void
free_con_buffer(con_buffer_t *buf);
/* Frees the specified con buffer and all of its predecessors
** Parameters: (con_buffer_t *) buf: The buffer to free
** Pixmaps are freed if neccessary, using the last used gfx state for con_gfx_show().
*/

void
con_gfx_hide(gfx_state_t *state);
/* Removes the console, restoring the background graphics
** Parameters: (gfx_state_t *state: The graphics state to draw with
*/

static gfx_pixmap_t **
_con_render_text_multiline(gfx_state_t *state, char *text, int maxchars, int *nr);
/*-- code --*/

static rect_t
con_box()
{
	return  gfx_rect(0, 0, 320,
			 con_displayed_lines
			 + con_border_width);
}

void
con_gfx_show(gfx_state_t *state)
{
	gfxop_set_clip_zone(state, gfx_rect(0, 0, 320, 200));
	con_last_gfx_state = state;

	con.locked_to_end = 1;
	con.background = gfxop_grab_pixmap(state, con_box());

	gfxop_set_color(state, &con.color_bg, con_bg_red, con_bg_green,
			con_bg_blue, con_bg_alpha, -1, -1);
	gfxop_set_color(state, &con.color_transparent, 0, 0, 0, 255, -1, -1);
	gfxop_set_color(state, &con.color_text,   255, 255, 255, 0, -1, -1);
	gfxop_set_color(state, &con.color_input,  255, 255, 0, 0, -1, -1);
	gfxop_set_color(state, &con.color_cursor, 255, 0, 0, 0, -1, -1);

	if (!con.input_prompt) {
		con.input_prompt = _con_render_text(state, CON_GFX_PROMPT, -1,
						    &con.color_input, NULL);
		con.input_text = (char*)sci_malloc(con.input_bufsize = 64);
		con.input_text[0] = 0;
		con.input_history_pos = 0;
		con.partial_write = 0;
		memset(con.input_history, 0, sizeof(char *) * CON_INPUT_HISTORY_SIZE);
	}

	con_jump_to_end(state);
	_con_redraw(state, 0, 1);
}

static void
_con_draw_bg_pic(gfx_state_t *state, rect_t zone)
{
	gfxop_draw_pixmap(state, con.background, zone, gfx_point(zone.x,zone.y));
}

void
con_jump_to_end(gfx_state_t *state)
{
	con.locked_to_end = 1;
	_con_redraw(state, 1, 0);
}


void
con_fold_text(gfx_state_t *state)
{
	con_buffer_t *seeker = con_buffer;

	/* Fold all text pixmaps */
	while (seeker) {
		int i;
		for (i = 0; i < CON_CLUSTER_SIZE; i++)
			if (CON_ENTRY_USED(seeker->entries[i])
			    && seeker->entries[i].text && seeker->entries[i].pixmaps_nr)
				_con_free_entry_pixmaps(state, &seeker->entries[i]);

		seeker = seeker->next;
	}
}

void
con_gfx_hide(gfx_state_t *state)
{
	/* Restore background */
	_con_draw_bg_pic(state, con_box());

	if (con.background)
		gfxop_free_pixmap(state, con.background);
	con.background = NULL;

	con_fold_text(state);
	gfxop_update(state);
}

static inline con_buffer_t *
_create_con_buffer(con_buffer_t *prev)
{
	con_buffer_t *buf = (con_buffer_t *)sci_malloc(sizeof (con_buffer_t));
	int i;

	for (i = 0; i < CON_CLUSTER_SIZE; i++)
		buf->entries[i].height = 0;

	buf->prev = prev;
	buf->next = NULL;
	if (prev)
		prev->next = buf;

	con_buffer_clusters++;

	return buf;
}


static inline void
_add_into_con_buffer(gfx_pixmap_t *pixmap, char *text)
{
	con_entry_t *target;

	if (!con_buffer) {
		con_buffer = con_buffer_tail = _create_con_buffer(NULL);
		con_top_buffer_entry_nr = 0;
	}

	if (con_top_buffer_entry_nr == CON_CLUSTER_SIZE) {
		/* Out of entries in this cluster */
		con_buffer = _create_con_buffer(con_buffer);
		con_top_buffer_entry_nr = 0;
	}

	target = con_buffer->entries + con_top_buffer_entry_nr;

	if (con.partial_write && text) {
		int real_entry = con_top_buffer_entry_nr - 1;
		int needlen = strlen(text);
		char *oldtext;

		if (real_entry < 0)
			target = con_buffer->prev->entries + CON_CLUSTER_SIZE - 1;
		else
			target = con_buffer->entries + real_entry;

		if (target->pixmaps)
			_con_free_entry_pixmaps(con_last_gfx_state, target);

		needlen += strlen(target->text);
		oldtext = target->text;
		target->text = (char *)sci_malloc(needlen+1);
		strcpy(target->text, oldtext);
		strcat(target->text, text);
		free(oldtext);
		free(text);

		con.partial_write = (target->text && *(target->text) &&
				     target->text[strlen(target->text) - 1] != '\n');

		return;
	}
	else ++con_top_buffer_entry_nr;


	con.partial_write = (text && *(text) &&
			     text[strlen(text) - 1] != '\n');

	if (pixmap)
		target->height = pixmap->index_yl;
	else
		target->height = 1; /* Will be calculated on demand */

	if (!pixmap) {
		target->pixmaps = NULL;
		target->pixmaps_nr = 0;
	} else {
		target->pixmaps = (gfx_pixmap_t **)sci_malloc(sizeof(gfx_pixmap_t *));
		target->pixmaps[0] = pixmap;
		target->pixmaps_nr = 1;
	}
	target->text = text;

	while (con_buffer_clusters > CON_MAX_CLUSTERS
	       && con_buffer_tail) {
		if (con_buffer_tail->next) {
			con_buffer_tail = con_buffer_tail->next;
			free_con_buffer(con_buffer_tail->prev);
		} else {
			fprintf(stderr,"WARNING: During cleanup, con_buffer_tail ran out!\n");
			free_con_buffer(con_buffer_tail->prev);
			con_buffer_tail = con_buffer = NULL;
		}
	}
}


void
con_gfx_insert_string(char *string)
{
	if (string)
		_add_into_con_buffer(NULL, string);
}

void
con_gfx_insert_pixmap(gfx_pixmap_t *pixmap)
{
	if (pixmap)
		_add_into_con_buffer(pixmap, NULL);
}

static int
_unfold_graphics(gfx_state_t *state, con_entry_t *drawme, int nr_chars)
     /* Returns whether unfolding was neccessary */
{
	if (drawme->text && !drawme->pixmaps_nr) {
		int i;
		drawme->pixmaps = _con_render_text_multiline(state, drawme->text,
							     nr_chars,
							     &drawme->pixmaps_nr);

		drawme->height = 0;
		for (i = 0; i < drawme->pixmaps_nr; i++)
			drawme->height +=
				(drawme->pixmaps[i]->yl + state->driver->mode->yfact - 1)
				/ state->driver->mode->yfact;
		/* Divide by scaler, round up */
		return 1;
	}

	return 0;
}

void
con_scroll(gfx_state_t *state, int offset, int maxchars)
{
	con_entry_t *next_entry;
	/* Scrolls within the display by the specified amount */

	if (con.locked_to_end) {
		con.pos.buf = con_buffer;
		con.pos.offset = 0;
		con.pos.entry = con_top_buffer_entry_nr - 1;
	}

	if (!con.pos.buf)
		return;

	con.locked_to_end = 0;

	con.pos.offset += offset; /* offset exceeds size -> Use PREVIOUS entry */

	while (con.pos.offset < 0 || con.pos.offset > con.pos.buf->entries[con.pos.entry].height) {

		if (con.pos.offset < 0) {
			if (++con.pos.entry == CON_CLUSTER_SIZE
			    || ((con.pos.buf == con_buffer)
			    && (con.pos.entry >= con_top_buffer_entry_nr))) {
				if (con.pos.buf->next) {
					con.pos.entry = 0;
					con.pos.buf = con.pos.buf->next;
				} else {
					con_jump_to_end(state);
					return;
				}
			}

			next_entry = con.pos.buf->entries + con.pos.entry;

			_unfold_graphics(state, next_entry, maxchars);
			con.pos.offset += next_entry->height;
		} else { /* offset too great ? */

			if (con.pos.entry == 0) {
				if (con.pos.buf->prev) {
					con.pos.entry = CON_CLUSTER_SIZE;
					con.pos.buf = con.pos.buf->prev;
				} else {
					con.pos.offset = con.pos.buf->entries[0].height - 1;
					return;
				}
			}
			--con.pos.entry;

			next_entry = con.pos.buf->entries + con.pos.entry;

			_unfold_graphics(state, next_entry, maxchars);
			con.pos.offset -= next_entry->height;

			if (con.pos.offset < 0)
				con.pos.offset = -con.pos.offset;
		}
	}
}

void
con_gfx_init()
{
	con_set_string_callback(con_gfx_insert_string);
	con_set_pixmap_callback(con_gfx_insert_pixmap);
	con.input_prompt = NULL;
	con.input_text = NULL;
	con.input_window = con.input_prompt_pos = -(int)strlen(CON_GFX_PROMPT);
	con.cursor_position = 0;
	con.input_precursor = NULL;
	con.input_postcursor = NULL;
	con.input_oncursor = NULL;
}


static void
_init_con_font()
{
	int i;

	con_font.ID = 0;
	con_font.chars_nr = CON_BUILTIN_CHARS_NR;
	con_font.widths = (int *)sci_malloc(sizeof(int) * CON_BUILTIN_CHARS_NR);
	for (i = 0; i < CON_BUILTIN_CHARS_NR; i++)
		con_font.widths[i] = CON_BUILTIN_CHARS_WIDTH;
	con_font.row_size = (CON_BUILTIN_CHARS_WIDTH + 7) >> 3;
	con_font.height = con_font.line_height = CON_BUILTIN_CHARS_HEIGHT;
	con_font.char_size = ((CON_BUILTIN_CHARS_WIDTH + 7) >> 3) * CON_BUILTIN_CHARS_HEIGHT;
	con_font.data = con_builtin_font_data;

	con_font_initialized = 1;
}

static gfx_pixmap_t **
_con_render_text_multiline(gfx_state_t *state, char *text, int maxchars, int *nr)
{
	int pixmaps_allocd = 1;
	gfx_pixmap_t **retval = (gfx_pixmap_t **)sci_malloc(sizeof(gfx_pixmap_t *) * pixmaps_allocd);
	char *printbuf = (char *)sci_malloc(maxchars + 8);
	int index = 0;
	int overwrap = 0;

	while (*text) {
		int len = 0;

		if (overwrap) {
			len = 2;
			printbuf[0] = ' ';
			printbuf[1] = CON_OVERWRAP_SYMBOL;
			overwrap = 0;
		}

		while (*text &&
		       *text != '\n' && len < maxchars) {
			if (*text != '\t')
				printbuf[len++] = *text;
			else {
				int tabwidth = 8 - (len & 7);
				memset(printbuf + len, ' ', tabwidth);
				len += tabwidth;
			}
			text++;
		}

		if (*text && (*text != '\n'))
			overwrap = 1;

		if (index == pixmaps_allocd)
			retval = (gfx_pixmap_t **)sci_realloc(retval, sizeof(gfx_pixmap_t *) * (pixmaps_allocd+= 4));

		retval[index++] = _con_render_text(state, printbuf, len,
						   &con.color_text, NULL);
		if (*text) text += (1 - overwrap);
	}

	*nr = index;
	sci_free(printbuf);
	return retval;
}

static gfx_pixmap_t *
_con_render_text(gfx_state_t *state, char *text, int len,
		 gfx_color_t *color, gfx_color_t *bgcolor)
{
	gfx_pixmap_t *pxm;

	if (len < 0)
		len = strlen(text);

	if (!con_font_initialized)
		_init_con_font();

	pxm = gfxr_draw_font(&con_font, text, len,
			     &color->visual,
			     &color->visual,
			     (bgcolor) ? &bgcolor->visual : NULL);

	pxm->flags |= GFX_PIXMAP_FLAG_SCALED_INDEX;

	gfx_xlate_pixmap(gfx_pixmap_alloc_data(pxm, state->driver->mode),
			 state->driver->mode, GFX_XLATE_FILTER_NONE);
	return pxm;
}

static inline int
_str_move_blank(char *data, int data_length, int initial_offset, int direction)
     /* Finds the next beginning or end of a word */
{
	int offset = initial_offset;
	int abort = (direction < 0)? 0 : data_length;
	int lookahead = (direction < 0)? -1 : 0;

	if (offset != abort)
		offset += direction;

	while (offset != abort && isspace(data[offset]))
		offset += direction;

	while (offset != abort && !isspace(data[offset + lookahead]))
		offset += direction;

	return offset;
}

char *
con_history_get_prev(int *handle) /* Should be -1 if not initialized yet */
{
	int nexthandle;

	if (*handle == con.input_history_pos)
		return NULL;

	if (*handle == -1)
		*handle = con.input_history_pos;

	nexthandle = (*handle) - 1;
	if (nexthandle == -1)
		nexthandle = CON_INPUT_HISTORY_SIZE - 1;

	if (con.input_history[nexthandle])
		*handle = nexthandle;

	return con.input_history[nexthandle];
}

char *
con_history_get_next(int *handle)
{
	int last = (con.input_history_pos + CON_INPUT_HISTORY_SIZE - 1) % CON_INPUT_HISTORY_SIZE;
	int nexthandle;

	if (*handle < 0 || *handle > CON_INPUT_HISTORY_SIZE)
		return NULL;

	if (*handle == last) {
		*handle = -1;
		return NULL; /* End of history */
	}

	nexthandle = *handle + 1;
	if (nexthandle == CON_INPUT_HISTORY_SIZE)
		nexthandle = 0;

	if (con.input_history[nexthandle])
		*handle = nexthandle;

	return con.input_history[nexthandle];
}

void
con_history_archive(char *msg)
{
	char **writepos = &(con.input_history[con.input_history_pos]);

	if (*writepos)
		sci_free(*writepos);

	*writepos = msg;

	if (++con.input_history_pos >= CON_INPUT_HISTORY_SIZE)
		con.input_history_pos = 0;
}

char *
con_gfx_read(gfx_state_t *state)
{
	int chwidth = CON_BUILTIN_CHARS_WIDTH / state->driver->mode->xfact;
	int maxchars = 320 / chwidth;
	sci_event_t evt;
	char *retval;
	int done = 0;
	int slen = strlen(con.input_text);
	int history_handle = -1;

	do {
		int old_pos = con.cursor_position;
		int must_resize = 0; /* Have to re-calculate the strlen */
		int must_redraw = 0; /* Redraw input field */
		int must_rewin = 0; /* Re-calculate window */
		int must_redraw_text = 0; /* Redraw display field */
		if (slen+1 >= con.input_bufsize)
			con.input_text = (char *)sci_realloc(con.input_text, con.input_bufsize += 64); 

		evt.type = 0;
		while (!evt.type)
			evt = gfxop_get_event(state, SCI_EVT_ANY);

		if (evt.type == SCI_EVT_KEYBOARD) {

			if (evt.buckybits & SCI_EVM_CTRL) {
				switch(evt.data) {
				case 'p':
				case 'P': {
					char *hist = con_history_get_prev(&history_handle);

					if (hist) {
						sci_free(con.input_text);
						con.input_text = sci_strdup(hist);
					}
					must_resize = must_redraw = must_rewin = 1;
				}
					break;

				case 'n':
				case 'N': {
					char *hist = con_history_get_next(&history_handle);

					if (hist) {
						sci_free(con.input_text);
						con.input_text = sci_strdup(hist);
					}
					must_resize = must_redraw = must_rewin = 1;
				}
					break;

				case 'a':
				case 'A': /* C-a */
					con.cursor_position = 0;
					break;

				case 'b':
				case 'B': /* C-b */
					if (con.cursor_position)
						--con.cursor_position;
					break;

				case 'e':
				case 'E': /* C-e */
					con.cursor_position = slen;
					break;

				case 'f':
				case 'F': /* C-f */
					if (con.cursor_position < slen)
						++con.cursor_position;
					break;

				case 'd':
				case 'D':
					memmove(con.input_text + con.cursor_position,
						con.input_text + con.cursor_position + 1,
						slen - con.cursor_position);
					must_resize = must_redraw = 1;
					break;

				case 'h':
				case 'H':
					if (!con.cursor_position)
						break;

					memmove(con.input_text + con.cursor_position - 1,
						con.input_text + con.cursor_position,
						slen - con.cursor_position + 1);
					must_resize = must_redraw = 1;
					--con.cursor_position;
					break;

				case 'k':
				case 'K':
					con.input_text[con.cursor_position] = 0;
					must_resize = must_redraw = 1;
					break;

				case '`': return "go";

				default:
					break;
				}
			} else if (evt.buckybits & SCI_EVM_ALT) {
				switch(evt.data) {
				case 'b':
				case 'B':
					con.cursor_position = _str_move_blank(con.input_text,
									      slen,
									      con.cursor_position,
									      -1);
					break;

				case 'f':
				case 'F':
					con.cursor_position = _str_move_blank(con.input_text,
									      slen,
									      con.cursor_position,
									      1);
					break;

				case 'd':
				case 'D': {
					int delpos = _str_move_blank(con.input_text, slen,
								     con.cursor_position, 1);

					must_resize = must_redraw = 1;
					memmove(con.input_text + con.cursor_position,
						con.input_text + delpos,
						slen - delpos + 1);
				}

				default:
					break;
				}
			} else switch (evt.data) {

			case SCI_K_UP: {
				char *hist = con_history_get_prev(&history_handle);

				if (hist) {
					sci_free(con.input_text);
					con.input_text = sci_strdup(hist);
				}
				must_resize = must_redraw = must_rewin = 1;
			}
				break;

			case SCI_K_DOWN: {
				char *hist = con_history_get_next(&history_handle);

				if (hist) {
					sci_free(con.input_text);
					con.input_text = sci_strdup(hist);
				}
				must_resize = must_redraw = must_rewin = 1;
			}
				break;


			case SCI_K_LEFT:
				if (con.cursor_position)
					--con.cursor_position;
				break;

			case SCI_K_RIGHT:
				if (con.cursor_position < slen)
					++con.cursor_position;
				break;


			case SCI_K_PGDOWN:
				must_redraw_text = 1;
				con_scroll(state, -75, maxchars);
				break;

			case SCI_K_PGUP:
				must_redraw_text = 1;
				con_scroll(state, 75, maxchars);
				break;

			case SCI_K_END:
				con_jump_to_end(state);
				must_redraw_text = 1;
				break;

			case SCI_K_DELETE:
				memmove(con.input_text + con.cursor_position,
					con.input_text + con.cursor_position + 1,
					slen - con.cursor_position);
				must_resize = must_redraw = 1;
				break;

			case SCI_K_BACKSPACE:
				if (!con.cursor_position)
					break;

				memmove(con.input_text + con.cursor_position - 1,
					con.input_text + con.cursor_position,
					slen - con.cursor_position + 1);
				must_resize = must_redraw = 1;
				--con.cursor_position;
				break;


			case SCI_K_ENTER:
				done = 1;
				break;

			default:
				if ((evt.character >= 32) && (evt.character <= 255)) {
					memmove(con.input_text + con.cursor_position + 1,
						con.input_text + con.cursor_position,
						slen - con.cursor_position + 1);

					con.input_text[con.cursor_position] = evt.character;
					++con.cursor_position;
					++slen;
					must_redraw = 1;
				}
			}
		}

		if (must_resize)
			slen = strlen(con.input_text);

		if (old_pos != con.cursor_position)
			must_redraw = 1;

		if (must_rewin) {
			int chwidth = CON_BUILTIN_CHARS_WIDTH / state->driver->mode->xfact;
			int nr_chars = 320 / chwidth;

			con.cursor_position = slen;
			con.input_window = slen - nr_chars + (nr_chars >> 3);
		}

		if (must_redraw || must_redraw_text) {
			_con_redraw(state, must_redraw_text, must_redraw);
			gfxop_update(state);
		}
	} while (!done);

	retval = con.input_text;
	con.input_text = (char *)sci_malloc(64);
	con.input_text[0] = 0;
	con.input_window = con.input_prompt_pos;
	con.cursor_position = 0;

	if (!*retval) {
		int hist = -1;
		sci_free(retval);
		return con_history_get_prev(&hist);
	}
	/* else */
	con_history_archive(retval);
	return retval;
}

static void
_con_redraw(gfx_state_t *state, int update_display_field, int update_input_field)
{
	int chwidth = CON_BUILTIN_CHARS_WIDTH / state->driver->mode->xfact;
	int nr_chars = 320 / chwidth;
	int offset = con_displayed_lines;
	int pixmap_index = -42;
	int must_recompute_pixmap_index = 1;
	int max_offset;
	con_pos_t pos = con.pos;
	rect_t fullbox = con_box();
	int yscale = state->driver->mode->yfact;
	int input_field_height = con.input_prompt ?
		(con.input_prompt->index_yl + yscale - 1) / yscale   : 0;
	/* This delta is in "virtual" SCI pixels */
	int input_field_size = con_border_width + input_field_height;
	/* Let's consider the bottom padding to be part of the input field */


	if (!update_input_field && !update_display_field)
		return;
	if (!update_input_field)
		fullbox.yl -= input_field_size;

	if (!update_display_field) {
		fullbox.y = fullbox.yl - input_field_size;
		fullbox.yl = input_field_size;
	}

	if (con.color_bg.alpha)
		_con_draw_bg_pic(state, fullbox);

	/* Draw overlay box */
	gfxop_draw_box(state, fullbox, con.color_bg, con.color_bg,
		       GFX_BOX_SHADE_FLAT);

	if (update_input_field) {

		if (con_border_width >= 2) {
			int y = con_displayed_lines + con_border_width - 2;

			gfxop_draw_line(state, gfx_point(0, y), gfx_point(319, y),
					con.color_input, GFX_LINE_MODE_FINE,
					GFX_LINE_STYLE_NORMAL);
		}

		if (con.input_prompt) {
			int promptlen = strlen(CON_GFX_PROMPT);

			if (con.cursor_position - con.input_window < (nr_chars  >> 3))
				con.input_window -= (nr_chars >> 1);
			else if (con.cursor_position - con.input_window > (nr_chars - (nr_chars >> 3)))
				con.input_window += (nr_chars >> 1);

			if (con.input_window < con.input_prompt_pos)
				con.input_window = con.input_prompt_pos;

			offset -= input_field_height;

			if (con.input_oncursor) {
				gfxop_free_pixmap(state, con.input_oncursor);
				con.input_oncursor = NULL;
			}

			if (con.input_text) {
				char oncursorbuf[2];
				char *postcursor_text = con.input_text + con.cursor_position + 1;
				char temp_sep;

				oncursorbuf[1] = 0;
				oncursorbuf[0] = temp_sep = con.input_text[con.cursor_position];

				if (!temp_sep)
					oncursorbuf[0] = ' '; /* Draw at least a blank cursor */

				if (con.input_precursor) {
					gfxop_free_pixmap(state, con.input_precursor);
					con.input_precursor = NULL;
				}
				if (con.input_postcursor) {
					gfxop_free_pixmap(state, con.input_postcursor);
					con.input_postcursor = NULL;
				}
				
				con.input_oncursor = _con_render_text(state, oncursorbuf, -1,
								      &con.color_input,
								      &con.color_cursor);
				if (con.input_text[0])
					con.input_precursor = _con_render_text(state,
									       con.input_text, -1,
									       &con.color_input,
									       NULL);
				if (postcursor_text[-1])
					con.input_postcursor = _con_render_text(state,
										postcursor_text,
										-1,
										&con.color_input,
										NULL);

				con.input_text[con.cursor_position] = temp_sep; 
			} else {
				con.input_oncursor = _con_render_text(state, " ", -1,
								      &con.color_input,
								      &con.color_cursor);
			}


			if (con.input_window < 0) {
				con.input_prompt->xoffset = 0;
				con.input_prompt->yoffset = 0;
				gfxop_draw_pixmap(state, con.input_prompt,
						  gfx_rect(0, 0,
							   con.input_prompt->index_xl,
							   con.input_prompt->index_yl),
						  gfx_point(chwidth * (strlen(CON_GFX_PROMPT)
								       + con.input_window),
							    offset));
			}

			if (con.input_precursor && con.input_window < con.cursor_position)
				gfxop_draw_pixmap(state, con.input_precursor,
						  gfx_rect(0, 0,
							   con.input_precursor->index_xl,
							   con.input_precursor->index_yl),
						  gfx_point(chwidth * -con.input_window,
							    offset));

			if (con.input_postcursor && con.input_window + nr_chars - 1 >
			    con.cursor_position) {
				gfxop_draw_pixmap(state, con.input_postcursor,
						  gfx_rect(0, 0,
							   con.input_postcursor->index_xl,
							   con.input_postcursor->index_yl),
						  gfx_point(chwidth * (con.cursor_position + 1
								       - con.input_window),
							    offset));
			}

			if (con.input_oncursor)
				gfxop_draw_pixmap(state, con.input_oncursor,
						  gfx_rect(0, 0,
							   con.input_oncursor->index_xl,
							   con.input_oncursor->index_yl),
						  gfx_point(chwidth * (con.cursor_position
								       - con.input_window),
							    offset));


		}
	} else
		offset -= input_field_height;

	if (!update_display_field) {
		gfxop_update_box(state, fullbox);
		return;
	}

	max_offset = offset;

	if (con.locked_to_end) {
		pos.buf = con_buffer;
		pos.offset = 0;
		pos.entry = con_top_buffer_entry_nr - 1;
	}

	while (pos.buf && offset >= 0) {
		con_entry_t *drawme;
		int depth = pos.offset;
		int line_yl;

		pos.offset = 0;

		if (pos.entry < 0) {
			pos.entry = CON_CLUSTER_SIZE - 1;
			if (pos.buf)
				pos.buf = pos.buf->prev;

			if (!pos.buf)
				break;
		}

		drawme = &(pos.buf->entries[pos.entry]);

		if (_unfold_graphics(state, drawme, nr_chars))
			must_recompute_pixmap_index = 1;

		if (must_recompute_pixmap_index) {
			pixmap_index = drawme->pixmaps_nr - 1;
			must_recompute_pixmap_index = 0;
		}

		if (pixmap_index < 0) {
			pos.entry--;
			continue;
		}

		while (pixmap_index >= 0
		       && depth > (drawme->pixmaps[pixmap_index]->yl + yscale - 1) / yscale)
			depth -= (drawme->pixmaps[pixmap_index--]->yl + yscale -1) / yscale;

		if (pixmap_index == -1) {
			fprintf(stderr, "ERROR: Offset too great! It was %d in a block of height %d\n",
				con.pos.offset, drawme->height);
			exit(1);
			continue;
		}

		line_yl = (drawme->pixmaps[pixmap_index]->yl + yscale - 1) / yscale;

		offset -= line_yl - depth;
		depth = line_yl;

		if (offset + depth > max_offset)
			depth = max_offset - offset;

		gfxop_draw_pixmap(state, drawme->pixmaps[pixmap_index],
				  gfx_rect(0, 0,
					   drawme->pixmaps[pixmap_index]->index_xl, depth),
				  gfx_point(0, offset));
		/*
		** TODO: Insert stuff into overwrapped lines **
		if (pixmap_index)
			gfxop_draw_line(state, gfx_rect(chwidth - 2, offset,
							chwidth - 2, offset + depth),
					con.color_text, GFX_LINE_MODE_FINE, GFX_LINE_STYLE_NORMAL);
		*/

		if (!pixmap_index) {
			pos.entry--;
			must_recompute_pixmap_index = 1;
		} else
			--pixmap_index;
	}

	gfxop_update_box(state, fullbox);
}

static void
_con_free_entry_pixmaps(gfx_state_t *state, con_entry_t *entry)
{
	int j;

	if (entry->pixmaps) {
		for (j = 0; j < entry->pixmaps_nr; j++)
			gfxop_free_pixmap(state, entry->pixmaps[j]);
		sci_free(entry->pixmaps);
		entry->pixmaps_nr = 0;
		entry->pixmaps = NULL;
	}
}


static void
_free_con_buffer(con_buffer_t *buf);

static void
free_con_buffer(con_buffer_t *buf)
/* Frees a con buffer and all of its predecessors */
{
	/* First, make sure we're not destroying the current display */
	if (!con.locked_to_end) {
		con_buffer_t *seeker = con.pos.buf;
		while (seeker && seeker != buf)
			seeker = seeker->prev;

		if (seeker) {
			if (seeker->prev)
				con.pos.buf = seeker->next;
			else
				con.locked_to_end = 1;
		}
	}
	_free_con_buffer(buf);
}

static void
_free_con_buffer(con_buffer_t *buf)
{
	int i;

	if (buf) {
		con_buffer_t *prev = buf->prev;

		if (buf->next)
			buf->next->prev = NULL;
		for (i = 0; i < CON_CLUSTER_SIZE; i++)
			if (CON_ENTRY_USED(buf->entries[i])) {
				if (buf->entries[i].text)
					sci_free(buf->entries[i].text);
				_con_free_entry_pixmaps(con_last_gfx_state, &buf->entries[i]);

				buf->entries[i].height = -1;
			}
		sci_free(buf);
		--con_buffer_clusters;

		if (prev)
			_free_con_buffer(prev);
	}
}


byte con_builtin_font_data[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x7e, 0x81, 0xa5, 0x81, 0xbd, 0x99, 0x81, 0x7e,
  0x7e, 0xff, 0xdb, 0xff, 0xc3, 0xe7, 0xff, 0x7e,
  0x6c, 0xfe, 0xfe, 0xfe, 0x7c, 0x38, 0x10, 0x00,
  0x10, 0x38, 0x7c, 0xfe, 0x7c, 0x38, 0x10, 0x00,
  0x38, 0x7c, 0x38, 0xfe, 0xfe, 0xd6, 0x10, 0x38,
  0x10, 0x38, 0x7c, 0xfe, 0xfe, 0x7c, 0x10, 0x38,
  0x00, 0x00, 0x18, 0x3c, 0x3c, 0x18, 0x00, 0x00,
  0xff, 0xff, 0xe7, 0xc3, 0xc3, 0xe7, 0xff, 0xff,
  0x00, 0x3c, 0x66, 0x42, 0x42, 0x66, 0x3c, 0x00,
  0xff, 0xc3, 0x99, 0xbd, 0xbd, 0x99, 0xc3, 0xff,
  0x0f, 0x07, 0x0f, 0x7d, 0xcc, 0xcc, 0xcc, 0x78,
  0x3c, 0x66, 0x66, 0x66, 0x3c, 0x18, 0x7e, 0x18,
  0x3f, 0x33, 0x3f, 0x30, 0x30, 0x70, 0xf0, 0xe0,
  0x7f, 0x63, 0x7f, 0x63, 0x63, 0x67, 0xe6, 0xc0,
  0x18, 0xdb, 0x3c, 0xe7, 0xe7, 0x3c, 0xdb, 0x18,
  0x80, 0xe0, 0xf8, 0xfe, 0xf8, 0xe0, 0x80, 0x00,
  0x02, 0x0e, 0x3e, 0xfe, 0x3e, 0x0e, 0x02, 0x00,
  0x18, 0x3c, 0x7e, 0x18, 0x18, 0x7e, 0x3c, 0x18,
  0x66, 0x66, 0x66, 0x66, 0x66, 0x00, 0x66, 0x00,
  0x7f, 0xdb, 0xdb, 0x7b, 0x1b, 0x1b, 0x1b, 0x00,
  0x3e, 0x61, 0x3c, 0x66, 0x66, 0x3c, 0x86, 0x7c,
  0x00, 0x00, 0x00, 0x00, 0x7e, 0x7e, 0x7e, 0x00,
  0x18, 0x3c, 0x7e, 0x18, 0x7e, 0x3c, 0x18, 0xff,
  0x18, 0x3c, 0x7e, 0x18, 0x18, 0x18, 0x18, 0x00,
  0x18, 0x18, 0x18, 0x18, 0x7e, 0x3c, 0x18, 0x00,
  0x00, 0x18, 0x0c, 0xfe, 0x0c, 0x18, 0x00, 0x00,
  0x00, 0x30, 0x60, 0xfe, 0x60, 0x30, 0x00, 0x00,
  0x00, 0x00, 0xc0, 0xc0, 0xc0, 0xfe, 0x00, 0x00,
  0x00, 0x24, 0x66, 0xff, 0x66, 0x24, 0x00, 0x00,
  0x00, 0x18, 0x3c, 0x7e, 0xff, 0xff, 0x00, 0x00,
  0x00, 0xff, 0xff, 0x7e, 0x3c, 0x18, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x18, 0x3c, 0x3c, 0x18, 0x18, 0x00, 0x18, 0x00,
  0x66, 0x66, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x6c, 0x6c, 0xfe, 0x6c, 0xfe, 0x6c, 0x6c, 0x00,
  0x18, 0x3e, 0x60, 0x3c, 0x06, 0x7c, 0x18, 0x00,
  0x00, 0xc6, 0xcc, 0x18, 0x30, 0x66, 0xc6, 0x00,
  0x38, 0x6c, 0x38, 0x76, 0xdc, 0xcc, 0x76, 0x00,
  0x18, 0x18, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x0c, 0x18, 0x30, 0x30, 0x30, 0x18, 0x0c, 0x00,
  0x30, 0x18, 0x0c, 0x0c, 0x0c, 0x18, 0x30, 0x00,
  0x00, 0x66, 0x3c, 0xff, 0x3c, 0x66, 0x00, 0x00,
  0x00, 0x18, 0x18, 0x7e, 0x18, 0x18, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x30,
  0x00, 0x00, 0x00, 0x7e, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x00,
  0x06, 0x0c, 0x18, 0x30, 0x60, 0xc0, 0x80, 0x00,
  0x38, 0x6c, 0xc6, 0xc6, 0xc6, 0x6c, 0x38, 0x00,
  0x18, 0x38, 0x18, 0x18, 0x18, 0x18, 0x7e, 0x00,
  0x7c, 0xc6, 0x06, 0x1c, 0x30, 0x66, 0xfe, 0x00,
  0x7c, 0xc6, 0x06, 0x3c, 0x06, 0xc6, 0x7c, 0x00,
  0x1c, 0x3c, 0x6c, 0xcc, 0xfe, 0x0c, 0x1e, 0x00,
  0xfe, 0xc0, 0xc0, 0xfc, 0x06, 0xc6, 0x7c, 0x00,
  0x38, 0x60, 0xc0, 0xfc, 0xc6, 0xc6, 0x7c, 0x00,
  0xfe, 0xc6, 0x0c, 0x18, 0x30, 0x30, 0x30, 0x00,
  0x7c, 0xc6, 0xc6, 0x7c, 0xc6, 0xc6, 0x7c, 0x00,
  0x7c, 0xc6, 0xc6, 0x7e, 0x06, 0x0c, 0x78, 0x00,
  0x00, 0x18, 0x18, 0x00, 0x00, 0x18, 0x18, 0x00,
  0x00, 0x18, 0x18, 0x00, 0x00, 0x18, 0x18, 0x30,
  0x06, 0x0c, 0x18, 0x30, 0x18, 0x0c, 0x06, 0x00,
  0x00, 0x00, 0x7e, 0x00, 0x00, 0x7e, 0x00, 0x00,
  0x60, 0x30, 0x18, 0x0c, 0x18, 0x30, 0x60, 0x00,
  0x7c, 0xc6, 0x0c, 0x18, 0x18, 0x00, 0x18, 0x00,
  0x7c, 0xc6, 0xde, 0xde, 0xde, 0xc0, 0x78, 0x00,
  0x38, 0x6c, 0xc6, 0xfe, 0xc6, 0xc6, 0xc6, 0x00,
  0xfc, 0x66, 0x66, 0x7c, 0x66, 0x66, 0xfc, 0x00,
  0x3c, 0x66, 0xc0, 0xc0, 0xc0, 0x66, 0x3c, 0x00,
  0xf8, 0x6c, 0x66, 0x66, 0x66, 0x6c, 0xf8, 0x00,
  0xfe, 0x62, 0x68, 0x78, 0x68, 0x62, 0xfe, 0x00,
  0xfe, 0x62, 0x68, 0x78, 0x68, 0x60, 0xf0, 0x00,
  0x3c, 0x66, 0xc0, 0xc0, 0xce, 0x66, 0x3a, 0x00,
  0xc6, 0xc6, 0xc6, 0xfe, 0xc6, 0xc6, 0xc6, 0x00,
  0x3c, 0x18, 0x18, 0x18, 0x18, 0x18, 0x3c, 0x00,
  0x1e, 0x0c, 0x0c, 0x0c, 0xcc, 0xcc, 0x78, 0x00,
  0xe6, 0x66, 0x6c, 0x78, 0x6c, 0x66, 0xe6, 0x00,
  0xf0, 0x60, 0x60, 0x60, 0x62, 0x66, 0xfe, 0x00,
  0xc6, 0xee, 0xfe, 0xfe, 0xd6, 0xc6, 0xc6, 0x00,
  0xc6, 0xe6, 0xf6, 0xde, 0xce, 0xc6, 0xc6, 0x00,
  0x7c, 0xc6, 0xc6, 0xc6, 0xc6, 0xc6, 0x7c, 0x00,
  0xfc, 0x66, 0x66, 0x7c, 0x60, 0x60, 0xf0, 0x00,
  0x7c, 0xc6, 0xc6, 0xc6, 0xc6, 0xce, 0x7c, 0x0e,
  0xfc, 0x66, 0x66, 0x7c, 0x6c, 0x66, 0xe6, 0x00,
  0x3c, 0x66, 0x30, 0x18, 0x0c, 0x66, 0x3c, 0x00,
  0x7e, 0x7e, 0x5a, 0x18, 0x18, 0x18, 0x3c, 0x00,
  0xc6, 0xc6, 0xc6, 0xc6, 0xc6, 0xc6, 0x7c, 0x00,
  0xc6, 0xc6, 0xc6, 0xc6, 0xc6, 0x6c, 0x38, 0x00,
  0xc6, 0xc6, 0xc6, 0xd6, 0xd6, 0xfe, 0x6c, 0x00,
  0xc6, 0xc6, 0x6c, 0x38, 0x6c, 0xc6, 0xc6, 0x00,
  0x66, 0x66, 0x66, 0x3c, 0x18, 0x18, 0x3c, 0x00,
  0xfe, 0xc6, 0x8c, 0x18, 0x32, 0x66, 0xfe, 0x00,
  0x3c, 0x30, 0x30, 0x30, 0x30, 0x30, 0x3c, 0x00,
  0xc0, 0x60, 0x30, 0x18, 0x0c, 0x06, 0x02, 0x00,
  0x3c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x3c, 0x00,
  0x10, 0x38, 0x6c, 0xc6, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff,
  0x30, 0x18, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x78, 0x0c, 0x7c, 0xcc, 0x76, 0x00,
  0xe0, 0x60, 0x7c, 0x66, 0x66, 0x66, 0xdc, 0x00,
  0x00, 0x00, 0x7c, 0xc6, 0xc0, 0xc6, 0x7c, 0x00,
  0x1c, 0x0c, 0x7c, 0xcc, 0xcc, 0xcc, 0x76, 0x00,
  0x00, 0x00, 0x7c, 0xc6, 0xfe, 0xc0, 0x7c, 0x00,
  0x3c, 0x66, 0x60, 0xf8, 0x60, 0x60, 0xf0, 0x00,
  0x00, 0x00, 0x76, 0xcc, 0xcc, 0x7c, 0x0c, 0xf8,
  0xe0, 0x60, 0x6c, 0x76, 0x66, 0x66, 0xe6, 0x00,
  0x18, 0x00, 0x38, 0x18, 0x18, 0x18, 0x3c, 0x00,
  0x06, 0x00, 0x06, 0x06, 0x06, 0x66, 0x66, 0x3c,
  0xe0, 0x60, 0x66, 0x6c, 0x78, 0x6c, 0xe6, 0x00,
  0x38, 0x18, 0x18, 0x18, 0x18, 0x18, 0x3c, 0x00,
  0x00, 0x00, 0xec, 0xfe, 0xd6, 0xd6, 0xd6, 0x00,
  0x00, 0x00, 0xdc, 0x66, 0x66, 0x66, 0x66, 0x00,
  0x00, 0x00, 0x7c, 0xc6, 0xc6, 0xc6, 0x7c, 0x00,
  0x00, 0x00, 0xdc, 0x66, 0x66, 0x7c, 0x60, 0xf0,
  0x00, 0x00, 0x76, 0xcc, 0xcc, 0x7c, 0x0c, 0x1e,
  0x00, 0x00, 0xdc, 0x76, 0x60, 0x60, 0xf0, 0x00,
  0x00, 0x00, 0x7e, 0xc0, 0x7c, 0x06, 0xfc, 0x00,
  0x30, 0x30, 0xfc, 0x30, 0x30, 0x36, 0x1c, 0x00,
  0x00, 0x00, 0xcc, 0xcc, 0xcc, 0xcc, 0x76, 0x00,
  0x00, 0x00, 0xc6, 0xc6, 0xc6, 0x6c, 0x38, 0x00,
  0x00, 0x00, 0xc6, 0xd6, 0xd6, 0xfe, 0x6c, 0x00,
  0x00, 0x00, 0xc6, 0x6c, 0x38, 0x6c, 0xc6, 0x00,
  0x00, 0x00, 0xc6, 0xc6, 0xc6, 0x7e, 0x06, 0xfc,
  0x00, 0x00, 0x7e, 0x4c, 0x18, 0x32, 0x7e, 0x00,
  0x0e, 0x18, 0x18, 0x70, 0x18, 0x18, 0x0e, 0x00,
  0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00,
  0x70, 0x18, 0x18, 0x0e, 0x18, 0x18, 0x70, 0x00,
  0x76, 0xdc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x10, 0x38, 0x6c, 0xc6, 0xc6, 0xfe, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x18, 0x00, 0x18, 0x18, 0x3c, 0x3c, 0x18, 0x00,
  0x18, 0x18, 0x7e, 0xc0, 0xc0, 0x7e, 0x18, 0x18,
  0x38, 0x6c, 0x64, 0xf0, 0x60, 0x66, 0xfc, 0x00,
  0x00, 0xc6, 0x7c, 0xc6, 0xc6, 0x7c, 0xc6, 0x00,
  0x66, 0x66, 0x3c, 0x7e, 0x18, 0x7e, 0x18, 0x18,
  0x18, 0x18, 0x18, 0x00, 0x00, 0x18, 0x18, 0x18,
  0x3e, 0x61, 0x3c, 0x66, 0x66, 0x3c, 0x86, 0x7c,
  0x00, 0xc6, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x7e, 0x81, 0x9d, 0xa1, 0xa1, 0x9d, 0x81, 0x7e,
  0x3c, 0x6c, 0x6c, 0x3e, 0x00, 0x7e, 0x00, 0x00,
  0x00, 0x33, 0x66, 0xcc, 0x66, 0x33, 0x00, 0x00,
  0x00, 0x00, 0x00, 0xfe, 0x06, 0x06, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x7e, 0x00, 0x00, 0x00, 0x00,
  0x7e, 0x81, 0xb9, 0xa5, 0xb9, 0xa5, 0x81, 0x7e,
  0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x38, 0x6c, 0x6c, 0x38, 0x00, 0x00, 0x00, 0x00,
  0x18, 0x18, 0x7e, 0x18, 0x18, 0x00, 0x7e, 0x00,
  0x78, 0x0c, 0x18, 0x30, 0x7c, 0x00, 0x00, 0x00,
  0x78, 0x0c, 0x38, 0x0c, 0x78, 0x00, 0x00, 0x00,
  0x0c, 0x18, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x66, 0x66, 0x66, 0x66, 0x7c, 0xc0,
  0x7f, 0xdb, 0xdb, 0x7b, 0x1b, 0x1b, 0x1b, 0x00,
  0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x0c, 0x38,
  0x18, 0x38, 0x18, 0x18, 0x3c, 0x00, 0x00, 0x00,
  0x38, 0x6c, 0x6c, 0x38, 0x00, 0x7c, 0x00, 0x00,
  0x00, 0xcc, 0x66, 0x33, 0x66, 0xcc, 0x00, 0x00,
  0x63, 0xe6, 0x6c, 0x7a, 0x36, 0x6a, 0xdf, 0x06,
  0x63, 0xe6, 0x6c, 0x7e, 0x33, 0x66, 0xcc, 0x0f,
  0xe1, 0x32, 0xe4, 0x3a, 0xf6, 0x2a, 0x5f, 0x86,
  0x18, 0x00, 0x18, 0x18, 0x30, 0x63, 0x3e, 0x00,
  0x18, 0x0c, 0x38, 0x6c, 0xc6, 0xfe, 0xc6, 0x00,
  0x30, 0x60, 0x38, 0x6c, 0xc6, 0xfe, 0xc6, 0x00,
  0x7c, 0x82, 0x38, 0x6c, 0xc6, 0xfe, 0xc6, 0x00,
  0x76, 0xdc, 0x38, 0x6c, 0xc6, 0xfe, 0xc6, 0x00,
  0xc6, 0x38, 0x6c, 0xc6, 0xfe, 0xc6, 0xc6, 0x00,
  0x38, 0x6c, 0x7c, 0xc6, 0xfe, 0xc6, 0xc6, 0x00,
  0x3e, 0x6c, 0xcc, 0xfe, 0xcc, 0xcc, 0xce, 0x00,
  0x7c, 0xc6, 0xc0, 0xc0, 0xc6, 0x7c, 0x0c, 0x78,
  0x30, 0x18, 0xfe, 0xc0, 0xfc, 0xc0, 0xfe, 0x00,
  0x18, 0x30, 0xfe, 0xc0, 0xf8, 0xc0, 0xfe, 0x00,
  0x7c, 0x82, 0xfe, 0xc0, 0xfc, 0xc0, 0xfe, 0x00,
  0xc6, 0x00, 0xfe, 0xc0, 0xfc, 0xc0, 0xfe, 0x00,
  0x30, 0x18, 0x3c, 0x18, 0x18, 0x18, 0x3c, 0x00,
  0x0c, 0x18, 0x3c, 0x18, 0x18, 0x18, 0x3c, 0x00,
  0x3c, 0x42, 0x3c, 0x18, 0x18, 0x18, 0x3c, 0x00,
  0x66, 0x00, 0x3c, 0x18, 0x18, 0x18, 0x3c, 0x00,
  0xf8, 0x6c, 0x66, 0xf6, 0x66, 0x6c, 0xf8, 0x00,
  0x76, 0xdc, 0x00, 0xe6, 0xf6, 0xde, 0xce, 0x00,
  0x0c, 0x06, 0x38, 0x6c, 0xc6, 0x6c, 0x38, 0x00,
  0x30, 0x60, 0x38, 0x6c, 0xc6, 0x6c, 0x38, 0x00,
  0x7c, 0x82, 0x38, 0x6c, 0xc6, 0x6c, 0x38, 0x00,
  0x76, 0xdc, 0x38, 0x6c, 0xc6, 0x6c, 0x38, 0x00,
  0xc6, 0x38, 0x6c, 0xc6, 0xc6, 0x6c, 0x38, 0x00,
  0x00, 0xc6, 0x6c, 0x38, 0x6c, 0xc6, 0x00, 0x00,
  0x3a, 0x6c, 0xce, 0xd6, 0xe6, 0x6c, 0xb8, 0x00,
  0x60, 0x30, 0xc6, 0xc6, 0xc6, 0xc6, 0x7c, 0x00,
  0x18, 0x30, 0xc6, 0xc6, 0xc6, 0xc6, 0x7c, 0x00,
  0x7c, 0x82, 0x00, 0xc6, 0xc6, 0xc6, 0x7c, 0x00,
  0xc6, 0x00, 0xc6, 0xc6, 0xc6, 0xc6, 0x7c, 0x00,
  0x0c, 0x18, 0x66, 0x66, 0x3c, 0x18, 0x3c, 0x00,
  0xf0, 0x60, 0x7c, 0x66, 0x7c, 0x60, 0xf0, 0x00,
  0x78, 0xcc, 0xcc, 0xd8, 0xcc, 0xc6, 0xcc, 0x00,
  0x30, 0x18, 0x78, 0x0c, 0x7c, 0xcc, 0x76, 0x00,
  0x18, 0x30, 0x78, 0x0c, 0x7c, 0xcc, 0x76, 0x00,
  0x7c, 0x82, 0x78, 0x0c, 0x7c, 0xcc, 0x76, 0x00,
  0x76, 0xdc, 0x7c, 0x06, 0x7e, 0xc6, 0x7e, 0x00,
  0xc6, 0x00, 0x78, 0x0c, 0x7c, 0xcc, 0x76, 0x00,
  0x30, 0x30, 0x78, 0x0c, 0x7c, 0xcc, 0x76, 0x00,
  0x00, 0x00, 0x7e, 0x12, 0xfe, 0x90, 0xfe, 0x00,
  0x00, 0x00, 0x7e, 0xc0, 0xc0, 0x7e, 0x0c, 0x38,
  0x30, 0x18, 0x7c, 0xc6, 0xfe, 0xc0, 0x7c, 0x00,
  0x0c, 0x18, 0x7c, 0xc6, 0xfe, 0xc0, 0x7c, 0x00,
  0x7c, 0x82, 0x7c, 0xc6, 0xfe, 0xc0, 0x7c, 0x00,
  0xc6, 0x00, 0x7c, 0xc6, 0xfe, 0xc0, 0x7c, 0x00,
  0x30, 0x18, 0x00, 0x38, 0x18, 0x18, 0x3c, 0x00,
  0x0c, 0x18, 0x00, 0x38, 0x18, 0x18, 0x3c, 0x00,
  0x7c, 0x82, 0x38, 0x18, 0x18, 0x18, 0x3c, 0x00,
  0x66, 0x00, 0x38, 0x18, 0x18, 0x18, 0x3c, 0x00,
  0x30, 0x7e, 0x0c, 0x7c, 0xcc, 0xcc, 0x78, 0x00,
  0x76, 0xdc, 0x00, 0xdc, 0x66, 0x66, 0x66, 0x00,
  0x30, 0x18, 0x7c, 0xc6, 0xc6, 0xc6, 0x7c, 0x00,
  0x0c, 0x18, 0x7c, 0xc6, 0xc6, 0xc6, 0x7c, 0x00,
  0x7c, 0x82, 0x7c, 0xc6, 0xc6, 0xc6, 0x7c, 0x00,
  0x76, 0xdc, 0x7c, 0xc6, 0xc6, 0xc6, 0x7c, 0x00,
  0xc6, 0x00, 0x7c, 0xc6, 0xc6, 0xc6, 0x7c, 0x00,
  0x00, 0x18, 0x00, 0x7e, 0x00, 0x18, 0x00, 0x00,
  0x00, 0x02, 0x7c, 0xce, 0xd6, 0xe6, 0x7c, 0x80,
  0x60, 0x30, 0xcc, 0xcc, 0xcc, 0xcc, 0x76, 0x00,
  0x18, 0x30, 0xcc, 0xcc, 0xcc, 0xcc, 0x76, 0x00,
  0x78, 0x84, 0x00, 0xcc, 0xcc, 0xcc, 0x76, 0x00,
  0xcc, 0x00, 0xcc, 0xcc, 0xcc, 0xcc, 0x76, 0x00,
  0x18, 0x30, 0xc6, 0xc6, 0xc6, 0x7e, 0x06, 0xfc,
  0xe0, 0x60, 0x7c, 0x66, 0x66, 0x7c, 0x60, 0xf0,
  0xc6, 0x00, 0xc6, 0xc6, 0xc6, 0x7e, 0x06, 0xfc
};

#endif /* !WANT_CONSOLE */
