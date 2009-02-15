#include "list.h"

#ifndef _SCI_GAME_SELECT_H
#define _SCI_GAME_SELECT_H

typedef struct game
{
	char *name;
	char dir[PATH_MAX];
	int conf_nr;
} game_t;

typedef struct games_list
{
	LIST_ENTRY(games_list) entries;

	game_t game;
} games_list_t;

typedef LIST_HEAD(games_list_head, games_list) games_list_head_t;

int game_select_gfxop_init_default(gfx_state_t *state, gfx_options_t *options, void *misc_info);

int game_select_gfxop_init(gfx_state_t *state, int xfact, int yfact, gfx_color_mode_t bpp, gfx_options_t *options, void *misc_info);

int
game_select_display(gfx_driver_t *gfx_driver, game_t* game_list, int game_count, gfx_bitmap_font_t* font_default, gfx_bitmap_font_t* font_small);

void game_select_scan_info(gfx_driver_t *gfx_driver, gfx_bitmap_font_t* font_default, gfx_bitmap_font_t* font_small, char *name, int total);


#if 0
/* this can be used to generate code that creates a particular font at runtime */
/* this is meant to be used as a development tool */
void save_font(int id, gfx_bitmap_font_t* font)
#endif 

#endif /* _SCI_GAME_SELECT_H */
