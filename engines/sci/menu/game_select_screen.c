/***************************************************************************
 game_select_screen.c Copyright (C) 2004 Hugues Valois


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
#include <sci_memory.h>
#include <gfx_operations.h>
#include "game_select.h"


#define MILLION 1000000


#define GS_WINDOW_LEFT		30
#define GS_WINDOW_TOP		26
#define GS_WINDOW_WIDTH		260
#define GS_WINDOW_HEIGHT	156

#define GS_SCAN_WINDOW_LEFT	30
#define GS_SCAN_WINDOW_TOP	73
#define GS_SCAN_WINDOW_WIDTH	260
#define GS_SCAN_WINDOW_HEIGHT	62

#define GS_LISTBOX_WIDTH	(GS_WINDOW_WIDTH - 78)
#define GS_LISTBOX_HEIGHT	(GS_WINDOW_HEIGHT - 43)
#define GS_LISTBOX_CONTENTS_HEIGHT	GS_LISTBOX_HEIGHT - 21

#define GS_BUTTON_PLAY		1
#define GS_BUTTON_QUIT		2

#define GS_BUTTON_FIRST		1
#define GS_BUTTON_LAST		2

static int
game_select_gfxop_usleep(gfx_driver_t *gfx_driver, long usecs)
{
	long time, utime;
	long wakeup_time, wakeup_utime;
	long add_seconds;
	int retval = GFX_OK;

	sci_gettime(&wakeup_time, &wakeup_utime);
	wakeup_utime += usecs;

	add_seconds = (wakeup_utime / MILLION);
	wakeup_time += add_seconds;
	wakeup_utime -= (MILLION * add_seconds);

	do {
		sci_gettime(&time, &utime);
		usecs = (wakeup_time - time) * MILLION + wakeup_utime - utime;
	} while ((usecs > 0) && !(retval = gfx_driver->usec_sleep(gfx_driver, usecs)));

	if (retval) {
		GFXWARN("Waiting failed\n");
	}

	return retval;
}

static sci_event_t
game_select_gfxop_get_event(gfx_driver_t *gfx_driver, unsigned int mask)
{
	sci_event_t event;

	event.type = 0;

	do
	{
		event = gfx_driver->get_event(gfx_driver);

	} while (event.type && !(event.type & mask));

	return event;
}

static gfx_pixmap_color_t 
create_pixmap_color_t(gfx_color_t color)
{
	gfx_pixmap_color_t pixmap_color;

	pixmap_color.global_index = GFX_COLOR_INDEX_UNMAPPED;
	pixmap_color.r = color.visual.r;
	pixmap_color.g = color.visual.g;
	pixmap_color.b = color.visual.b;

	return pixmap_color;
}

static gfx_color_t 
create_color_t(byte r, byte g, byte b)
{
	gfx_color_t color;

	color.visual.global_index = 0;
	color.mask = GFX_MASK_VISUAL;
	color.alpha = 1;
	color.priority = 0;
	color.control = 0;
	color.visual.r = r;
	color.visual.g = g;
	color.visual.b = b;

	return color;
}

static void
gfx_box_border(gfx_driver_t *gfx_driver, rect_t box, gfx_color_t color)
{
	box.x *= gfx_driver->mode->xfact;
	box.y *= gfx_driver->mode->yfact;
	box.xl *= gfx_driver->mode->xfact;
	box.yl *= gfx_driver->mode->yfact;

	{
		point_t ul = gfx_point (box.x, box.y);
		point_t ur = gfx_point (box.x + box.xl, box.y);
		point_t ll = gfx_point (box.x, box.y + box.yl);
		point_t lr = gfx_point (box.x + box.xl, box.y + box.yl);


		gfx_driver->draw_line(gfx_driver, ul, ur, color, GFX_LINE_MODE_CORRECT, GFX_LINE_STYLE_NORMAL);
		gfx_driver->draw_line(gfx_driver, ur, lr, color, GFX_LINE_MODE_CORRECT, GFX_LINE_STYLE_NORMAL);
		gfx_driver->draw_line(gfx_driver, lr, ll, color, GFX_LINE_MODE_CORRECT, GFX_LINE_STYLE_NORMAL);
		gfx_driver->draw_line(gfx_driver, ll, ul, color, GFX_LINE_MODE_CORRECT, GFX_LINE_STYLE_NORMAL);
	}
}

static void
gfx_box_fill(gfx_driver_t *gfx_driver, rect_t box, gfx_color_t color)
{
	box.x *= gfx_driver->mode->xfact;
	box.y *= gfx_driver->mode->yfact;
	box.xl *= gfx_driver->mode->xfact;
	box.yl *= gfx_driver->mode->yfact;

	gfx_driver->draw_filled_rect(gfx_driver, box, color, color, GFX_SHADE_FLAT);
}

static void
gfx_box_line(gfx_driver_t *gfx_driver, rect_t box, gfx_color_t color)
{
	box.x *= gfx_driver->mode->xfact;
	box.y *= gfx_driver->mode->yfact;
	box.xl *= gfx_driver->mode->xfact;
	box.yl *= gfx_driver->mode->yfact;


	{
		point_t p1 = gfx_point (box.x, box.y);
		point_t p2 = gfx_point (box.x + box.xl, box.y + box.yl);

		gfx_driver->draw_line(gfx_driver, p1, p2, color, GFX_LINE_MODE_CORRECT, GFX_LINE_STYLE_NORMAL);
	}
}

static void
gfx_box_text(gfx_driver_t *gfx_driver, rect_t box, gfx_bitmap_font_t* font, gfx_pixmap_color_t fgc, gfx_pixmap_color_t bgc, const char* text, int center)
{
	int width;
	int height;
	gfx_pixmap_t* pixmap;
	text_fragment_t *textsplits;
	int meas_width;
	int meas_height;
	int meas_line;
	int margin_x = 0;
	int margin_y = 0;
	int line_height;
	int last_offset;

	if (center != 0)
	{
		textsplits = gfxr_font_calculate_size(font, box.xl, text, &meas_width, &meas_height, &meas_line, &line_height, &last_offset, GFXR_FONT_FLAG_NO_NEWLINES | GFXR_FONT_FLAG_COUNT_WHITESPACE);
		if (textsplits != NULL)
		{
			free(textsplits);
		}

		margin_x = (box.xl - meas_width) / 2;
		margin_y = (box.yl - meas_height) / 2;
	}

	box.x *= gfx_driver->mode->xfact;
	box.y *= gfx_driver->mode->yfact;
	box.xl *= gfx_driver->mode->xfact;
	box.yl *= gfx_driver->mode->yfact;
	margin_x *= gfx_driver->mode->xfact;
	margin_y *= gfx_driver->mode->yfact;

	pixmap = gfxr_draw_font(font, text, strlen(text), &fgc, &fgc, NULL);
	if (pixmap != NULL)
	{
		pixmap->xoffset = 0;
		pixmap->yoffset = 0;

		gfx_xlate_pixmap(gfx_pixmap_alloc_data(pixmap, gfx_driver->mode), gfx_driver->mode, GFX_XLATE_FILTER_NONE);

		width = pixmap->index_xl * gfx_driver->mode->xfact;
		height = pixmap->index_yl * gfx_driver->mode->yfact;

		if (width > box.xl)
			width = box.xl;

		if (gfx_driver->draw_pixmap(gfx_driver, pixmap, GFX_NO_PRIORITY, gfx_rect(0, 0, width, height), gfx_rect(box.x + margin_x, box.y + margin_y, width, height), GFX_BUFFER_BACK) != GFX_OK)
		{
			GFXERROR("Error occured while drawing pixmap.\n");
		}

		gfx_free_pixmap(gfx_driver, pixmap);
	}
}

static void
game_select_display_button(gfx_driver_t *gfx_driver, gfx_bitmap_font_t* font, rect_t box, const char* button_text, int focus)
{
	gfx_color_t black;
	gfx_color_t white;

	black = create_color_t(0, 0, 0);
	white = create_color_t(255, 255, 255);

	gfx_box_border(gfx_driver, gfx_rect(box.x, box.y, box.xl, box.yl), black);
	gfx_box_fill(gfx_driver, gfx_rect(box.x + 1, box.y + 1, box.xl - 2, box.yl - 2), white);
	gfx_box_text(gfx_driver, gfx_rect(box.x, box.y + 1, box.xl, box.yl), font, create_pixmap_color_t(black), create_pixmap_color_t(white), button_text, 1);
	if (focus != 0)
		gfx_box_border(gfx_driver, gfx_rect(box.x + 1, box.y + 1, box.xl - 2, box.yl - 2), black);
}

static void
game_select_display_listbox(gfx_driver_t *gfx_driver, gfx_bitmap_font_t* font, rect_t box, game_t *game_list, int game_count, int first_game, int selected_game)
{
	int max_game;
	int pos = 0;
	int cur;
	int center_x;
	gfx_color_t black;
	gfx_color_t white;

	center_x = box.x + (box.xl / 2) - 1;

	black = create_color_t(0, 0, 0);
	white = create_color_t(255, 255, 255);

	/* list box */
	gfx_box_border(gfx_driver, box, black);
	gfx_box_fill(gfx_driver, gfx_rect(box.x + 1, box.y + 1, box.xl - 2, box.yl - 2), white);
	gfx_box_line(gfx_driver, gfx_rect(box.x, box.y + 10, box.xl, 0), black);
	gfx_box_line(gfx_driver, gfx_rect(box.x, box.y + box.yl - 10, box.xl, 0), black);

	/* list box scroll up */
	gfx_box_line(gfx_driver, gfx_rect(center_x, box.y + 2, 1, 0), black);
	gfx_box_line(gfx_driver, gfx_rect(center_x - 1, box.y + 3, 3, 0), black);
	gfx_box_line(gfx_driver, gfx_rect(center_x - 2, box.y + 4, 5, 0), black);
	gfx_box_line(gfx_driver, gfx_rect(center_x - 3, box.y + 5, 7, 0), black);
	gfx_box_fill(gfx_driver, gfx_rect(center_x - 1, box.y + 6, 4, 3), black);

	/* list box scroll up */
	gfx_box_line(gfx_driver, gfx_rect(center_x, box.y + box.yl - 2, 1, 0), black);
	gfx_box_line(gfx_driver, gfx_rect(center_x - 1, box.y + box.yl - 3, 3, 0), black);
	gfx_box_line(gfx_driver, gfx_rect(center_x - 2, box.y + box.yl - 4, 5, 0), black);
	gfx_box_line(gfx_driver, gfx_rect(center_x - 3, box.y + box.yl - 5, 7, 0), black);
	gfx_box_fill(gfx_driver, gfx_rect(center_x - 1, box.y + box.yl - 8, 4, 3), black);

	/* list box content */
	max_game = (int)(((double)(box.yl - 21)) / font->line_height);
	for (cur = first_game; cur < game_count && (cur - first_game < max_game); cur++, pos++)
	{
		if (selected_game == cur)
			gfx_box_fill(gfx_driver, gfx_rect(box.x + 1, box.y + 11 + (pos * font->line_height), box.xl - 1, font->line_height), black);

		gfx_box_text(gfx_driver, gfx_rect(box.x + 1, box.y + 11 + (pos * font->line_height), box.xl - 1, font->line_height), font, create_pixmap_color_t((selected_game == cur) ? white : black), create_pixmap_color_t((selected_game == cur) ? black : white), game_list[cur].name, 0);
	}
}

static void
game_select_display_window(gfx_driver_t *gfx_driver, gfx_bitmap_font_t* font, rect_t box, const char* title)
{
	gfx_color_t black;
	gfx_color_t white;
	gfx_color_t grey;

	black = create_color_t(0, 0, 0);
	white = create_color_t(255, 255, 255);
	grey = create_color_t(85, 85, 85);

	/* window border */
	gfx_box_border(gfx_driver, box, black);

	/* window fill */
	gfx_box_fill(gfx_driver, gfx_rect(box.x + 1, box.y + 1, box.xl - 1, box.yl - 1), white);

	/* window title */
	gfx_box_fill(gfx_driver, gfx_rect(box.x + 1, box.y + 1, box.xl - 1, font->height), grey);
	gfx_box_line(gfx_driver, gfx_rect(box.x, box.y + font->height + 1, box.xl, 0), black);
	gfx_box_text(gfx_driver, gfx_rect(box.x + 1, box.y + 2, box.xl - 1, font->height - 1), font, create_pixmap_color_t(white), create_pixmap_color_t(grey), title, 1);

	/* window shade */
	gfx_box_line(gfx_driver, gfx_rect(box.x + box.xl + 1, box.y + 3, 0, box.yl - 2), black);
	gfx_box_line(gfx_driver, gfx_rect(box.x + 3, box.y + box.yl + 1, box.xl - 2, 0), black);
}

static void
game_select_draw_bg(gfx_driver_t *gfx_driver, gfx_bitmap_font_t* font)
{
	char title[255];
	gfx_color_t black;
	gfx_color_t white;
	gfx_color_t blue;

	black = create_color_t(0, 0, 0);
	white = create_color_t(255, 255, 255);
	blue = create_color_t(0, 0, 170);

	strcpy(title, "FreeSCI");
	strcat(title, " ");
	strcat(title, VERSION);

	/* menu bar */
	gfx_box_fill(gfx_driver, gfx_rect(0, 0, 320, font->height), white);
	gfx_box_line(gfx_driver, gfx_rect(0, font->height, 320, 0), black);
	gfx_box_text(gfx_driver, gfx_rect(0, 1, 320, font->height - 1), font, create_pixmap_color_t(black), create_pixmap_color_t(white), title, 1);

	/* background */
	gfx_box_fill(gfx_driver, gfx_rect(0, 10, 320, 190), blue);
}

static void
game_select_display_game_list(gfx_driver_t *gfx_driver, gfx_bitmap_font_t* font_default, gfx_bitmap_font_t* font_small, game_t *game_list, int game_count, int first_game, int selected_game, int focus_button)
{
	gfx_color_t black;
	gfx_color_t white;

	black = create_color_t(0, 0, 0);
	white = create_color_t(255, 255, 255);

	game_select_draw_bg(gfx_driver, font_default);

	/* window */
	game_select_display_window(gfx_driver, font_default, gfx_rect(GS_WINDOW_LEFT, GS_WINDOW_TOP, GS_WINDOW_WIDTH, GS_WINDOW_HEIGHT), "Play a Game");

	/* window text */
	gfx_box_text(gfx_driver, gfx_rect(GS_WINDOW_LEFT + 6, GS_WINDOW_TOP + 16, GS_WINDOW_WIDTH - 11, 8), font_default, create_pixmap_color_t(black), create_pixmap_color_t(white), "Select the game that you would like", 0);
	gfx_box_text(gfx_driver, gfx_rect(GS_WINDOW_LEFT + 6, GS_WINDOW_TOP + 24, GS_WINDOW_WIDTH - 11, 8), font_default, create_pixmap_color_t(black), create_pixmap_color_t(white), "to play.", 0);

	/* window list box */
	game_select_display_listbox(gfx_driver, font_small, gfx_rect(GS_WINDOW_LEFT + 5, GS_WINDOW_TOP + 39, GS_LISTBOX_WIDTH, GS_LISTBOX_HEIGHT), game_list, game_count, first_game, selected_game);

	/* window play button */
	game_select_display_button(gfx_driver, font_default, gfx_rect(GS_WINDOW_LEFT + GS_WINDOW_WIDTH - 69, GS_WINDOW_TOP + 39, 64, font_default->height + 1), "Play", focus_button == GS_BUTTON_PLAY);

	/* window quit button */
	game_select_display_button(gfx_driver, font_default, gfx_rect(GS_WINDOW_LEFT + GS_WINDOW_WIDTH - 69, GS_WINDOW_TOP + 39 + font_default->height + 5, 64, font_default->height + 1), "Quit", focus_button == GS_BUTTON_QUIT);
}

void
game_select_scan_info(gfx_driver_t *gfx_driver, gfx_bitmap_font_t* font_default, gfx_bitmap_font_t* font_small, char *name, int total)
{
	gfx_color_t black;
	gfx_color_t white;
	gfx_color_t grey;
	gfx_color_t blue;
	rect_t box;
	int error;
	char text[256];

	black = create_color_t(0, 0, 0);
	white = create_color_t(255, 255, 255);
	grey = create_color_t(85, 85, 85);
	blue = create_color_t(0, 0, 170);

	game_select_draw_bg(gfx_driver, font_default);

	/* window */
	game_select_display_window(gfx_driver, font_default, gfx_rect(GS_SCAN_WINDOW_LEFT, GS_SCAN_WINDOW_TOP, GS_SCAN_WINDOW_WIDTH, GS_SCAN_WINDOW_HEIGHT), "Scanning for Games");

	/* window text */
	gfx_box_text(gfx_driver, gfx_rect(GS_SCAN_WINDOW_LEFT + 6, GS_SCAN_WINDOW_TOP + 16, GS_SCAN_WINDOW_WIDTH - 11, 8), font_default, create_pixmap_color_t(black), create_pixmap_color_t(white), "Please wait, scanning for games...", 0);

	if (name) {
		snprintf(text, 256, "Adding: %s", name);
	
		gfx_box_text(gfx_driver, gfx_rect(GS_SCAN_WINDOW_LEFT + 6, GS_SCAN_WINDOW_TOP + 32, GS_SCAN_WINDOW_WIDTH - 11, 8), font_default, create_pixmap_color_t(black), create_pixmap_color_t(white), text, 0);
	}
	
	snprintf(text, 256, "Games found: %i", total);

	gfx_box_text(gfx_driver, gfx_rect(GS_SCAN_WINDOW_LEFT + 6, GS_SCAN_WINDOW_TOP + 48, GS_SCAN_WINDOW_WIDTH - 11, 8), font_default, create_pixmap_color_t(black), create_pixmap_color_t(white), text, 0);

	box = gfx_rect(0, 0, 320 * gfx_driver->mode->xfact, 200 * gfx_driver->mode->yfact);

	if ((error = gfx_driver->update(gfx_driver, box, gfx_point(box.x, box.y), GFX_BUFFER_FRONT)))
	{
		GFXERROR("Error occured while updating region (%d,%d,%d,%d) in buffer %d\n", box.x, box.y, box.xl, box.yl, GFX_BUFFER_BACK);
	}
}

int
game_select_display(gfx_driver_t *gfx_driver, game_t *game_list, int game_count, gfx_bitmap_font_t* font_default, gfx_bitmap_font_t* font_small)
{
	int error;
	int cont;
	sci_event_t event;
	int focus_button = GS_BUTTON_PLAY;
	int selected_game = 0;
	int first_game = 0;
	int max_game;
	rect_t box;

	box = gfx_rect(0, 0, 320 * gfx_driver->mode->xfact, 200 * gfx_driver->mode->yfact);

	max_game = (int)(((double)GS_LISTBOX_CONTENTS_HEIGHT) / font_small->line_height);

	game_select_display_game_list(gfx_driver, font_default, font_small, game_list, game_count, first_game, selected_game, focus_button);
	if ((error = gfx_driver->update(gfx_driver, box, gfx_point(box.x, box.y), GFX_BUFFER_FRONT)))
	{
		GFXERROR("Error occured while updating region (%d,%d,%d,%d) in buffer %d\n", box.x, box.y, box.xl, box.yl, GFX_BUFFER_BACK);
	}

	cont = 2;

	while (cont) 
	{
		event = game_select_gfxop_get_event(gfx_driver, SCI_EVT_KEYBOARD);

		if (event.type == SCI_EVT_KEYBOARD)
		{
			if (event.data == SCI_K_ENTER)
			{
				switch (focus_button)
				{
					case GS_BUTTON_PLAY:
						cont = 0;
						break;
					case GS_BUTTON_QUIT:
						cont = 0;
						selected_game = -1;
						break;
				}
			}
			else if ((event.buckybits & (SCI_K_TAB | SCI_EVM_RSHIFT)) || (event.buckybits & (SCI_K_TAB | SCI_EVM_LSHIFT)))
			{
				focus_button--;

				if (focus_button < GS_BUTTON_FIRST)
					focus_button = GS_BUTTON_LAST;

				game_select_display_game_list(gfx_driver, font_default, font_small, game_list, game_count, first_game, selected_game, focus_button);
				if ((error = gfx_driver->update(gfx_driver, box, gfx_point(box.x, box.y), GFX_BUFFER_FRONT)))
				{
					GFXERROR("Error occured while updating region (%d,%d,%d,%d) in buffer %d\n", box.x, box.y, box.xl, box.yl, GFX_BUFFER_BACK);
				}
			}
			else if (event.data == SCI_K_TAB)
			{
				focus_button++;

				if (focus_button > GS_BUTTON_LAST)
					focus_button = GS_BUTTON_FIRST;

				game_select_display_game_list(gfx_driver, font_default, font_small, game_list, game_count, first_game, selected_game, focus_button);
				if ((error = gfx_driver->update(gfx_driver, box, gfx_point(box.x, box.y), GFX_BUFFER_FRONT)))
				{
					GFXERROR("Error occured while updating region (%d,%d,%d,%d) in buffer %d\n", box.x, box.y, box.xl, box.yl, GFX_BUFFER_BACK);
				}
			}
			else if (event.data == SCI_K_ESC)
			{
				cont = 0;
				selected_game = -1;
			}
			else if (event.data == SCI_K_UP)
			{
				if (selected_game > 0)
				{
					selected_game--;

					focus_button = GS_BUTTON_PLAY;

					if (selected_game < first_game)
					{
						first_game = selected_game;
					}

					game_select_display_game_list(gfx_driver, font_default, font_small, game_list, game_count, first_game, selected_game, focus_button);
					if ((error = gfx_driver->update(gfx_driver, box, gfx_point(box.x, box.y), GFX_BUFFER_FRONT)))
					{
						GFXERROR("Error occured while updating region (%d,%d,%d,%d) in buffer %d\n", box.x, box.y, box.xl, box.yl, GFX_BUFFER_BACK);
					}
				}
			}
			else if (event.data == SCI_K_DOWN)
			{
				if (selected_game < game_count - 1)
				{
					selected_game++;

					focus_button = GS_BUTTON_PLAY;

					if (selected_game - first_game + 1 > max_game)
					{
						first_game = selected_game - max_game + 1;
					}

					game_select_display_game_list(gfx_driver, font_default, font_small, game_list, game_count, first_game, selected_game, focus_button);
					if ((error = gfx_driver->update(gfx_driver, box, gfx_point(box.x, box.y), GFX_BUFFER_FRONT)))
					{
						GFXERROR("Error occured while updating region (%d,%d,%d,%d) in buffer %d\n", box.x, box.y, box.xl, box.yl, GFX_BUFFER_BACK);
					}
				}
			}
			else if (event.data == SCI_K_PGUP)
			{
				selected_game -= (max_game - 1);
				if (selected_game < 0)
				{
					selected_game = 0;
				}

				focus_button = GS_BUTTON_PLAY;

				if (selected_game < first_game)
				{
					first_game = selected_game;
				}

				game_select_display_game_list(gfx_driver, font_default, font_small, game_list, game_count, first_game, selected_game, focus_button);
				if ((error = gfx_driver->update(gfx_driver, box, gfx_point(box.x, box.y), GFX_BUFFER_FRONT)))
				{
					GFXERROR("Error occured while updating region (%d,%d,%d,%d) in buffer %d\n", box.x, box.y, box.xl, box.yl, GFX_BUFFER_BACK);
				}
			}
			else if (event.data == SCI_K_PGDOWN)
			{
				selected_game += max_game - 1;
				if (selected_game >= game_count)
				{
					selected_game = game_count - 1;
				}

				focus_button = GS_BUTTON_PLAY;

				if (selected_game - first_game + 1 > max_game)
				{
					first_game = selected_game - max_game + 1;
				}

				game_select_display_game_list(gfx_driver, font_default, font_small, game_list, game_count, first_game, selected_game, focus_button);
				if ((error = gfx_driver->update(gfx_driver, box, gfx_point(box.x, box.y), GFX_BUFFER_FRONT)))
				{
					GFXERROR("Error occured while updating region (%d,%d,%d,%d) in buffer %d\n", box.x, box.y, box.xl, box.yl, GFX_BUFFER_BACK);
				}
			}
		}

		game_select_gfxop_usleep(gfx_driver, 25000);
	}

	return selected_game;
}
