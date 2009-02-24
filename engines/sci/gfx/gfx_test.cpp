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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

/* gfx driver test and verification program */

#define DISABLE_SCI_MEMORY

#ifdef _MSC_VER
#  include <unistd.h>
#  include <sys/time.h>
#  include <windows.h>
#endif

#include <assert.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include <gfx_resource.h>
#include <gfx_tools.h>
#include <gfx_operations.h>

namespace Sci {

int
sci0_palette;

gfx_pixmap_color_t gfx_sci0_image_colors[1][16];

gfx_pixmap_color_t *
gfxr_interpreter_get_static_palette(gfx_resstate_t *state, int version, int *colors_nr, void *internal) {
	return NULL;
}

int
sciprintf(const char *fmt, ...) {
	va_list argp;
	va_start(argp, fmt);
	vprintf(fmt, argp);
	va_end(argp);
	return 0;
}


void *
memdup(void *mem, size_t size) {
	void *r = malloc(size);
	if (r)
		memcpy(r, mem, size);
	return r;
}

#ifdef FREESCI_PRIMARY_RESOURCE_H_
#  include "../scicore/sci_memory.c"
#endif

void
sci_gettime(long *seconds, long *useconds) {
	struct timeval tv;

#ifdef WIN32
	timeBeginPeriod(0);
#endif

	assert(!gettimeofday(&tv, NULL));
	*seconds = time(NULL);
	*useconds = tv.tv_usec;

#ifdef WIN32
	timeEndPeriod(0);
#endif
}

static int xres = 1;
static int yres = 1;
static int color_mode = 1;
static char *driver = NULL;
static int set_mode = 0;
static int nowait = 0;
static int skip_intro = 0;

#define ALL_TESTS "abcdefghijkl"
static char tests[256];

gfx_state_t graphics_state;
gfx_state_t *state = &graphics_state;

gfx_options_t graphics_options;
gfx_options_t *options = &graphics_options;


#define CAPABILITY_IDS_NR 11

static struct {
	int mask;
	char *description;
} capability_ids[CAPABILITY_IDS_NR] = {
	{GFX_CAPABILITY_SHADING, "shaded boxes"},
	{GFX_CAPABILITY_MOUSE_POINTER, "mouse pointers"},
	{GFX_CAPABILITY_COLOR_MOUSE_POINTER, "multi-color mouse pointers"},
	{GFX_CAPABILITY_PIXMAP_REGISTRY, "pixmaps must be registered"},
	{GFX_CAPABILITY_SCALEABLE_PIXMAPS, "pixmap scaling"},
	{GFX_CAPABILITY_STIPPLED_LINES, "stippled lines"},
	{GFX_CAPABILITY_MOUSE_SUPPORT, "pointing device input"},
	{GFX_CAPABILITY_POINTER_PIXMAP_REGISTRY, "pointer pixmaps must be registered"},
	{GFX_CAPABILITY_FINE_LINES, "fine lines"},
	{GFX_CAPABILITY_WINDOWED, "windowed mode active"},
	{GFX_CAPABILITY_KEYTRANSLATE, "built-in keyboard translation"}
};

int
init_driver(gfx_driver_t *drv) {
	int i;

	state->driver = drv;
	options->dirty_frames = GFXOP_DIRTY_FRAMES_CLUSTERS;

	if (set_mode) {
		if (gfxop_init(state, xres, yres, color_mode, options, NULL)) {
			printf("Custom initialization failed\n");
			return 1;
		}
	} else {
		if (gfxop_init_default(state, options, NULL)) {
			printf("Default initialization failed\n");
			return 1;
		}
	}

	printf("Using graphics driver %s, version %s\n", drv->name, drv->version);
	printf("Graphics driver capabilities:\n");
	for (i = 0; i < CAPABILITY_IDS_NR; i++)
		if (drv->capabilities & capability_ids[i].mask)
			printf("\t%s\n", capability_ids[i].description);

	printf("\n");
	return 0;
}


/* ---------------------------------- */
/* Functions for the resource manager */
/* ---------------------------------- */

int multicolored_pointers = 1; /* Whether to test multicolored pointer support */

#define TEST_PICS_NR 2
#define TEST_VIEWS_NR 1
#define TEST_FONTS_NR 1
#define TEST_CURSORS_NR 2

int test_pics[TEST_PICS_NR] = {0, 1};
int test_views[TEST_VIEWS_NR] = {0};
int test_fonts[TEST_FONTS_NR] = {0};
int test_cursors[TEST_CURSORS_NR] = {0, 1};

int
gfxr_interpreter_options_hash(gfx_resource_type_t type, int version, gfx_options_t *options, void *internal, int palette) {
	return 0;
}


int *
arrdup(int *src, int count) {
	int *retval = sci_malloc(sizeof(int) * count);
	memcpy(retval, src, sizeof(int) * count);
	return retval;
}

int *
gfxr_interpreter_get_resources(gfx_resstate_t *resstate, gfx_resource_type_t type,
	int version, int *entries_nr, void *internal) {
	switch (type) {

	case GFX_RESOURCE_TYPE_VIEW:
		*entries_nr = TEST_VIEWS_NR;
		return arrdup(test_views, TEST_VIEWS_NR);

	case GFX_RESOURCE_TYPE_PIC:
		*entries_nr = TEST_PICS_NR;
		return arrdup(test_pics, TEST_PICS_NR);

	case GFX_RESOURCE_TYPE_FONT:
		*entries_nr = TEST_FONTS_NR;
		return arrdup(test_fonts, TEST_FONTS_NR);

	case GFX_RESOURCE_TYPE_CURSOR:
		*entries_nr = TEST_CURSORS_NR;
		return arrdup(test_cursors, TEST_CURSORS_NR);

	default:
		fprintf(stderr, "Attept to get resource list for invalid resource type %d\n", type);
		return NULL;
	}
}

#define PIC_COLORS_NR 32

gfx_pixmap_color_t pic_colors[PIC_COLORS_NR] = {
	{GFX_COLOR_INDEX_UNMAPPED, 0x0f, 0x0f, 0x0f},
	{GFX_COLOR_INDEX_UNMAPPED, 0x1f, 0x1f, 0x1f},
	{GFX_COLOR_INDEX_UNMAPPED, 0x2f, 0x2f, 0x2f},
	{GFX_COLOR_INDEX_UNMAPPED, 0x3f, 0x3f, 0x3f},
	{GFX_COLOR_INDEX_UNMAPPED, 0x4f, 0x4f, 0x4f},
	{GFX_COLOR_INDEX_UNMAPPED, 0x5f, 0x5f, 0x5f},
	{GFX_COLOR_INDEX_UNMAPPED, 0x6f, 0x6f, 0x6f},
	{GFX_COLOR_INDEX_UNMAPPED, 0x7f, 0x7f, 0x7f},
	{GFX_COLOR_INDEX_UNMAPPED, 0x8f, 0x8f, 0x8f},
	{GFX_COLOR_INDEX_UNMAPPED, 0x9f, 0x9f, 0x9f},
	{GFX_COLOR_INDEX_UNMAPPED, 0xaf, 0xaf, 0xaf},
	{GFX_COLOR_INDEX_UNMAPPED, 0xbf, 0xbf, 0xbf},
	{GFX_COLOR_INDEX_UNMAPPED, 0xcf, 0xcf, 0xcf},
	{GFX_COLOR_INDEX_UNMAPPED, 0xdf, 0xdf, 0xdf},
	{GFX_COLOR_INDEX_UNMAPPED, 0xef, 0xef, 0xef},
	{GFX_COLOR_INDEX_UNMAPPED, 0xff, 0xff, 0xff},
	{GFX_COLOR_INDEX_UNMAPPED, 0x00, 0x00, 0x0f},
	{GFX_COLOR_INDEX_UNMAPPED, 0x00, 0x00, 0x1f},
	{GFX_COLOR_INDEX_UNMAPPED, 0x00, 0x00, 0x2f},
	{GFX_COLOR_INDEX_UNMAPPED, 0x00, 0x00, 0x3f},
	{GFX_COLOR_INDEX_UNMAPPED, 0x00, 0x00, 0x4f},
	{GFX_COLOR_INDEX_UNMAPPED, 0x00, 0x00, 0x5f},
	{GFX_COLOR_INDEX_UNMAPPED, 0x00, 0x00, 0x6f},
	{GFX_COLOR_INDEX_UNMAPPED, 0x00, 0x00, 0x7f},
	{GFX_COLOR_INDEX_UNMAPPED, 0x00, 0x00, 0x8f},
	{GFX_COLOR_INDEX_UNMAPPED, 0x00, 0x00, 0x9f},
	{GFX_COLOR_INDEX_UNMAPPED, 0x00, 0x00, 0xaf},
	{GFX_COLOR_INDEX_UNMAPPED, 0x00, 0x00, 0xbf},
	{GFX_COLOR_INDEX_UNMAPPED, 0x00, 0x00, 0xcf},
	{GFX_COLOR_INDEX_UNMAPPED, 0x00, 0x00, 0xdf},
	{GFX_COLOR_INDEX_UNMAPPED, 0x00, 0x00, 0xef},
	{GFX_COLOR_INDEX_UNMAPPED, 0x00, 0x00, 0xff}
};

gfxr_pic_t *
gfxr_interpreter_init_pic(int version, gfx_mode_t *mode, int ID, void *internal) {
	gfxr_pic_t *pic = sci_malloc(sizeof(gfxr_pic_t));

	pic->mode = mode;
	pic->undithered_buffer = NULL;
	pic->internal = NULL;

	pic->control_map = gfx_pixmap_alloc_index_data(gfx_new_pixmap(320, 200, ID, 2, 0));
	pic->priority_map = gfx_pixmap_alloc_index_data(gfx_new_pixmap(320 * mode->xfact,
	                    200 * mode->yfact, ID, 1, 0));
	pic->visual_map = gfx_pixmap_alloc_index_data(gfx_new_pixmap(320, 200, ID, 0, 0));

	pic->visual_map->colors = pic_colors;
	pic->visual_map->colors_nr = PIC_COLORS_NR;

	pic->visual_map->flags = GFX_PIXMAP_FLAG_EXTERNAL_PALETTE;
	pic->priority_map->flags = GFX_PIXMAP_FLAG_EXTERNAL_PALETTE;
	pic->control_map->flags = GFX_PIXMAP_FLAG_EXTERNAL_PALETTE;

	pic->priority_map->colors = pic_colors;
	pic->priority_map->colors_nr = PIC_COLORS_NR;
	pic->control_map->colors = pic_colors;
	pic->control_map->colors_nr = PIC_COLORS_NR;

	return pic;
}



void
gfxr_interpreter_clear_pic(int version, gfxr_pic_t *pic, void *internal) {
	memset(pic->visual_map->index_data, 0x00, 320 * 200);
	memset(pic->priority_map->index_data, 0, 320 * pic->mode->xfact * 200 * pic->mode->yfact);
	memset(pic->control_map->index_data, 0, GFXR_AUX_MAP_SIZE);
	memset(pic->aux_map, 0, GFXR_AUX_MAP_SIZE);
}


int
gfxr_interpreter_calculate_pic(gfx_resstate_t *state, gfxr_pic_t *scaled_pic, gfxr_pic_t *unscaled_pic,
	int flags, int default_palette, int nr, void *internal) {
	gfxr_pic_t *pic = scaled_pic;
	int i, x, y, pos;
	int xfact = pic->mode->xfact;
	int yfact = pic->mode->yfact;

	if (nr < 0 || nr > TEST_PICS_NR)
		return GFX_ERROR;

	switch (nr) {
	case 0:
		pos = 0;
		for (y = 0; y < 200; y++) {
			for (x = 0; x < 32; x++) {
				memset(pic->visual_map->index_data + pos, x, 5);
				memset(pic->visual_map->index_data + 315 + pos - x*10, x, 5);
				pos += 5;
			}
			pos += 160;
		}

		for (y = 0; y < 200*yfact; y++) {
			memset(pic->priority_map->index_data + y * 320*xfact + (100*xfact), 10, 120*xfact);
			memset(pic->priority_map->index_data + y * 320*xfact + (150*xfact), 20, 20*xfact);
		}
		break;

	case 1:
		memset(pic->visual_map->index_data + 80*320, 15, 120*320);
		memset(pic->priority_map->index_data + 80*xfact * 320*yfact, 20,
		       120*320*xfact*yfact);

		for (i = 40; i < 80; i++) {
			int j;

			memset(pic->visual_map->index_data + i*320 + 140, 15, 80);
			for (j = 0; j < pic->mode->yfact; j++)
				memset(pic->priority_map->index_data + (i*yfact + j)*320*xfact
				       + 140*xfact , 20, 80*xfact);
		}
		break;

	default:
		fprintf(stderr, "Attempt to reference invalid pic #%d\n", nr);
	}

	printf(">> resource manager retrieved pic #%d\n", nr);
	return GFX_OK;
}


#define VIEW_COLORS_NR 16

gfx_pixmap_color_t view_colors[VIEW_COLORS_NR] = {
	{GFX_COLOR_INDEX_UNMAPPED, 0x0f, 0x00, 0x00},
	{GFX_COLOR_INDEX_UNMAPPED, 0x2f, 0x20, 0x00},
	{GFX_COLOR_INDEX_UNMAPPED, 0x4f, 0x40, 0x00},
	{GFX_COLOR_INDEX_UNMAPPED, 0x6f, 0x60, 0x00},
	{GFX_COLOR_INDEX_UNMAPPED, 0x8f, 0x80, 0x00},
	{GFX_COLOR_INDEX_UNMAPPED, 0xaf, 0xa0, 0x00},
	{GFX_COLOR_INDEX_UNMAPPED, 0xcf, 0xc0, 0x00},
	{GFX_COLOR_INDEX_UNMAPPED, 0xef, 0xe0, 0x00},
	{GFX_COLOR_INDEX_UNMAPPED, 0x00, 0x1f, 0x00},
	{GFX_COLOR_INDEX_UNMAPPED, 0x00, 0x3f, 0x00},
	{GFX_COLOR_INDEX_UNMAPPED, 0x00, 0x5f, 0x00},
	{GFX_COLOR_INDEX_UNMAPPED, 0x00, 0x7f, 0x00},
	{GFX_COLOR_INDEX_UNMAPPED, 0x00, 0x9f, 0x00},
	{GFX_COLOR_INDEX_UNMAPPED, 0x00, 0xbf, 0x00},
	{GFX_COLOR_INDEX_UNMAPPED, 0x00, 0xdf, 0x00},
	{GFX_COLOR_INDEX_UNMAPPED, 0x00, 0xff, 0x00}
};

gfxr_view_t *
gfxr_interpreter_get_view(gfx_resstate_t *state, int nr, void *internal, int palette) {
	gfxr_view_t *view;
	gfxr_loop_t *loop;
	int i;

	if (nr < 0 || nr > TEST_VIEWS_NR)
		return NULL;

	view = sci_malloc(sizeof(gfxr_view_t));
	view->ID = nr | 2048;
	view->flags = GFX_PIXMAP_FLAG_EXTERNAL_PALETTE;

	view->colors_nr = VIEW_COLORS_NR;
	view->colors = view_colors;

	view->loops_nr = 1;
	view->loops = loop = sci_malloc(sizeof(gfxr_loop_t));

	loop->cels_nr = 3;
	loop->cels = sci_malloc(sizeof(gfx_pixmap_t *) * loop->cels_nr);

	for (i = 0; i < 3; i++) {
		gfx_pixmap_t *pxm = gfx_pixmap_alloc_index_data(gfx_new_pixmap(16, 16, 2048 | nr, 0, i));
		int offset = (i == 1) ? 8 : 0;
		int x, y;

		pxm->colors_nr = VIEW_COLORS_NR;
		pxm->flags = GFX_PIXMAP_FLAG_EXTERNAL_PALETTE;
		pxm->colors = view_colors;
		pxm->xoffset = 8;
		pxm->yoffset = 8;

		for (y = 0; y < 16; y++)
			for (x = 0; x < 16; x++) {
				int dx = (8 - x);
				int dy = (8 - y);
				int dist = dx * dx + dy * dy;
				int index = (dist * 8) / 64;
				int pos = x + y * 16;

				if (i == 2) {
					offset = (!dx || !dy) ? 8 : 0;
					if (offset == 8) index <<= 1;
				}

				index = 7 - index;
				if (index < 0)
					pxm->index_data[pos] = 0xff;
				else
					pxm->index_data[pos] = index + offset;
			}

		loop->cels[i] = pxm;
	}

	printf(">> resource manager retrieved view #%d\n", nr);

	return view;
}

#define BUILTIN_CHARS_NR 256
#define BUILTIN_CHARS_HEIGHT 8
#define BUILTIN_CHARS_WIDTH 8

extern byte builtin_font[];

gfx_bitmap_font_t *
gfxr_interpreter_get_font(gfx_resstate_t *state, int nr, void *internal) {
	gfx_bitmap_font_t *font;
	int i;
	if (nr < 0 || nr > TEST_FONTS_NR)
		return NULL;

	font = sci_malloc(sizeof(gfx_bitmap_font_t));
	font->ID = nr;
	font->chars_nr = BUILTIN_CHARS_NR;
	font->widths = sci_malloc(sizeof(int) * BUILTIN_CHARS_NR);
	for (i = 0; i < BUILTIN_CHARS_NR; i++)
		font->widths[i] = BUILTIN_CHARS_WIDTH;
	font->row_size = (BUILTIN_CHARS_WIDTH + 7) >> 3;
	font->height = font->line_height = BUILTIN_CHARS_HEIGHT;
	font->char_size = ((BUILTIN_CHARS_WIDTH + 7) >> 3) * BUILTIN_CHARS_HEIGHT;
	font->data = memdup(builtin_font, font->char_size * BUILTIN_CHARS_NR);

	printf(">> resource manager retrieved font #%d\n", nr);

	return font;
}

gfx_pixmap_color_t _cursor_colors[3] = {
	{GFX_COLOR_INDEX_UNMAPPED, 0, 0, 0},
	{GFX_COLOR_INDEX_UNMAPPED, 0xff, 0xff, 0xff},
	{GFX_COLOR_INDEX_UNMAPPED, 0x80, 0x80, 0x80}
};

gfx_pixmap_t *
gfxr_interpreter_get_cursor(gfx_resstate_t *state, int nr, void *internal) {
	gfx_pixmap_t *cursor;
	int xl, yl, x, y;

	if (nr < 0 || nr > TEST_CURSORS_NR)
		return NULL;

	if (!nr)
		xl = yl = 31;
	else {
		xl = 8;
		yl = 16;
	}


	cursor = gfx_pixmap_alloc_index_data(gfx_new_pixmap(xl, yl, 1024 + nr, 0, 0));
	cursor->colors = _cursor_colors;
	cursor->flags |= GFX_PIXMAP_FLAG_EXTERNAL_PALETTE;

	memset(cursor->index_data, 0xff, xl * yl);

	switch (nr) {
	case 0:
		cursor->colors_nr = multicolored_pointers ? 3 : 2;
		cursor->xoffset = 16;
		cursor->yoffset = 16;

		for (x = 0; x < 31; x++) if (x != 16) {
				cursor->index_data[31*x + 16] = 1;
				cursor->index_data[31*16 + x] = 1;

				if (multicolored_pointers && ((x < 8) || (x > 23))) {
					cursor->index_data[31*x + 14] = 0;
					cursor->index_data[31*x + 15] = 2;
					cursor->index_data[31*x + 17] = 2;
					cursor->index_data[31*x + 18] = 0;

					cursor->index_data[31*14 + x] = 0;
					cursor->index_data[31*15 + x] = 2;
					cursor->index_data[31*17 + x] = 2;
					cursor->index_data[31*18 + x] = 0;
				} else {
					cursor->index_data[31*x + 15] = 0;
					cursor->index_data[31*x + 17] = 0;

					cursor->index_data[31*15 + x] = 0;
					cursor->index_data[31*17 + x] = 0;
				}
			}
		break;

	case 1:
		cursor->colors_nr = 2;
		cursor->xoffset = 0;
		cursor->yoffset = 0;
		for (y = 0; y < yl; y++)
			for (x = 0; x <= (y >> 1); x++)
				cursor->index_data[x + y*xl] = 1;
		break;

	default:
		fprintf(stderr, "Attempt to load invalid pointer %d\n", nr);
		gfx_free_pixmap(state->driver, cursor);
		return NULL;
	}

	printf(">> resource manager retrieved cursor #%d\n", nr);

	return cursor;
}

gfx_pixmap_color_t *
gfxr_interpreter_get_palette(gfx_resstate_t *state, int version, int *colors_nr, void *internal, int nr) {
	return NULL;
}

int
gfxr_interpreter_needs_multicolored_pointers(int version, void *internal) {
	return multicolored_pointers;
}

gfx_color_t red, green, blue, dblue, white, white8, white16, white24, black, transparent;

void
init_colors() {
	gfxop_set_color(state, &red,         0xff, 0x00, 0x00, 0x00, -1, -1);
	gfxop_set_color(state, &green,       0x00, 0xff, 0x00, 0x00, -1, -1);
	gfxop_set_color(state, &blue,        0x00, 0x00, 0xff, 0x00, -1, -1);
	gfxop_set_color(state, &dblue,       0x00, 0x00, 0x40, 0x00, -1, -1);
	gfxop_set_color(state, &white,       0xff, 0xff, 0xff, 0x00, -1, -1);
	gfxop_set_color(state, &white8,      0xff, 0xff, 0xff, 0x00,  8, -1);
	gfxop_set_color(state, &white16,     0xff, 0xff, 0xff, 0x00, 16, -1);
	gfxop_set_color(state, &white24,     0xff, 0xff, 0xff, 0x00, 24, -1);
	gfxop_set_color(state, &black,       0x00, 0x00, 0x00, 0x00, -1, -1);
	gfxop_set_color(state, &transparent, -1  , -1  , -1  , 0x00, -1, -1);
}


#define MESSAGE(foo) if (message(foo)) { fprintf(stderr,"Message '%s' could not be print!\n", foo); return;}
#define MESSAGE1(foo,a) { char buf[1024]; sprintf(buf,foo,a); if (message(buf)) { fprintf(stderr,"Message '%s' could not be print!\n", buf); return;}}
#define MESSAGE2(foo,a,b) { char buf[1024]; sprintf(buf,foo,a,b); if (message(buf)) { fprintf(stderr,"Message '%s' could not be print!\n", buf); return;}}
#define MESSAGE3(foo,a,b,c) { char buf[1024]; sprintf(buf,foo,a,b,c); if (message(buf)) { fprintf(stderr,"Message '%s' could not be print!\n", buf); return;}}
#define MESSAGE4(foo,a,b,c,d) { char buf[1024]; sprintf(buf,foo,a,b,c,d); if (message(buf)) { fprintf(stderr,"Message '%s' could not be print!\n", buf); return;}}


int
waitkey(void) {
	int count = 100000;
	sci_event_t event;

	while (count--) {
		event = gfxop_get_event(state, SCI_EVT_ANY);

		if (event.type)
			return 0;

		gfxop_sleep(state, 1);
	}
	return 1;
}

int
wait_specific_key(int key) {
	int count = 20000;
	sci_event_t event;

	while (count--) {
		event = gfxop_get_event(state, SCI_EVT_ANY);

		if (event.type == SCI_EVT_KEYBOARD
		        && event.data == key)
			return 0;

		gfxop_sleep(state, 1);
	}
	return 1;
}


int
message(char *msg) {
	gfx_text_handle_t *handle;
	rect_t text_rect = gfx_rect(0, 150, 320, 50);


	handle = gfxop_new_text(state, 0, msg, 320, ALIGN_CENTER, ALIGN_TOP,
	                        white, white, black, 0);

	if (!handle) return 1;

	printf("-----------------------------------------\n%s\n-----------------------------------------\n", msg);

	gfxop_fill_box(state, text_rect, black);
	gfxop_draw_text(state, handle, text_rect);
	gfxop_free_text(state, handle);
	gfxop_update(state);
	return 0;
}

void
update(void) {
	/*	gfxop_update_box(state, gfx_rect(0, 0, 320, 150)); */
	gfxop_update(state);
}

void
explicit_clear_buffer(void) {
	gfxop_clear_box(state, gfx_rect(0, 0, 320, 150));
	gfxop_update(state);
}

void
clear_buffer(void) {
	gfxop_disable_dirty_frames(state);
	gfxop_clear_box(state, gfx_rect(0, 0, 320, 150));
	gfxop_enable_dirty_frames(state);
}

void
clear(void) {
	gfxop_fill_box(state, gfx_rect(0, 0, 320, 150), black);
}

void
identify_event(sci_event_t event) {
	switch (event.type) {

	case SCI_EVT_NONE:
		MESSAGE("No event");
		break;

	case SCI_EVT_MOUSE_PRESS:
		MESSAGE4("Mouse press at (%d,%d)\ndata/modifiers (%d/%d)",
		         state->pointer_pos.x, state->pointer_pos.y, event.data, event.buckybits);
		break;

	case SCI_EVT_MOUSE_RELEASE:
		MESSAGE4("Mouse release at (%d,%d)\ndata/modifiers (%d/%d)",
		         state->pointer_pos.x, state->pointer_pos.y, event.data, event.buckybits);
		break;

	case SCI_EVT_KEYBOARD:
		if (event.data > 127) {
			MESSAGE2("Key 0x%04x\nmodifiers %04x", event.data, event.buckybits);
		} else
			MESSAGE3("Key '%c' (0x%02x)\nmodifiers %04x", event.data, event.data, event.buckybits);
		break;

	case SCI_EVT_JOYSTICK:
		MESSAGE1("Joystick direction %d", event.data);
		break;

	case SCI_EVT_ERROR:
		MESSAGE("Error event");
		break;

	default:
		MESSAGE1("Unknown event type %d!\n", event.type);
	}
}


int
test_a(void) {
	if (message("-- Test A --\nText display and basic input\nPlease press 'space' within 20 seconds"))
		return 1;

	return (wait_specific_key(' '));
}


#define TEST_LINE(x, y, xl, yl) \
	gfxop_draw_line(state, gfx_point(x, y), gfx_point((x)+(xl), (y)+(yl)), blue, GFX_LINE_MODE_FAST, GFX_LINE_STYLE_NORMAL); \
	gfxop_set_clip_zone(state, gfx_rect(140, 60, 40, 40)); \
	gfxop_draw_line(state, gfx_point(x, y), gfx_point((x)+(xl), (y)+(yl)), red, GFX_LINE_MODE_FAST, GFX_LINE_STYLE_NORMAL); \
	gfxop_set_clip_zone(state, gfx_rect(0, 0, 320, 200));

#define LINES_NR 19

int test_b_lines[LINES_NR][4]  = {
	{10, 10, 300, 0},
	{160, 30, 0, 100},
	{162, 20, 0, 50},
	{162, 90, 0, 50},
	{110, 80, 100, 0},
	{100, 82, 50, 0},
	{170, 82, 50, 0},
	{135, 70, 20, -20},
	{135, 90, 20, 20},
	{185, 70, -20, -20},
	{185, 90, -20, 20},
	{150, 70, -20, -10},
	{150, 70, -10, -20},
	{170, 70, 20, -10},
	{170, 70, 10, -20},
	{150, 90, -20, 10},
	{150, 90, -10, 20},
	{170, 90, 10, 20},
	{170, 90, 20, 10}
};

void
test_b(void) {
	int i;

	MESSAGE("-- Test B --\nLines");
	waitkey();
	MESSAGE("Some tests will include 'fine' lines.\nNote that support for those is\noptional.");
	waitkey();

	gfxop_draw_line(state, gfx_point(30, 30), gfx_point(290, 30), red, GFX_LINE_MODE_FINE, GFX_LINE_STYLE_NORMAL);
	gfxop_draw_line(state, gfx_point(30, 40), gfx_point(290, 40), blue, GFX_LINE_MODE_FAST, GFX_LINE_STYLE_NORMAL);
	gfxop_draw_line(state, gfx_point(30, 50), gfx_point(290, 50), green, GFX_LINE_MODE_FINE, GFX_LINE_STYLE_STIPPLED);
	gfxop_draw_line(state, gfx_point(30, 60), gfx_point(290, 60), white, GFX_LINE_MODE_FAST, GFX_LINE_STYLE_STIPPLED);
	update();
	MESSAGE("B.0: horizontal lines:\nYou should now be seeing (top-down):\nred-fine, blue-normal,\ngreen-fine-stippled, white-stippled");
	waitkey();

	clear();
	gfxop_draw_line(state, gfx_point(30, 30), gfx_point(290, 130), blue, GFX_LINE_MODE_FAST, GFX_LINE_STYLE_NORMAL);
	gfxop_draw_line(state, gfx_point(30, 130), gfx_point(290, 30), blue, GFX_LINE_MODE_FAST, GFX_LINE_STYLE_NORMAL);
	gfxop_set_clip_zone(state, gfx_rect(140, 60, 40, 40));
	gfxop_draw_line(state, gfx_point(30, 30), gfx_point(290, 130), red, GFX_LINE_MODE_FAST, GFX_LINE_STYLE_NORMAL);
	gfxop_draw_line(state, gfx_point(30, 130), gfx_point(290, 30), red, GFX_LINE_MODE_FAST, GFX_LINE_STYLE_NORMAL);
	gfxop_set_clip_zone(state, gfx_rect(0, 0, 320, 200));
	update();
	MESSAGE("B.1: line clipping:\nTwo identical pairs of lines.\nblue lines are unclipped,\nred ones are clipped.");
	waitkey();

	clear();
	for (i = 0; i < LINES_NR; i++) {
		TEST_LINE(test_b_lines[i][0], test_b_lines[i][1], test_b_lines[i][2], test_b_lines[i][3]);
	}
	update();
	MESSAGE1("B.2: line clipping:\n%d lines.", LINES_NR);
	waitkey();

	clear();
	gfxop_draw_rectangle(state, gfx_rect(30, 30, 260, 100), red, GFX_LINE_MODE_FAST, GFX_LINE_STYLE_NORMAL);
	gfxop_draw_rectangle(state, gfx_rect(40, 40, 240, 80), red, GFX_LINE_MODE_FAST, GFX_LINE_STYLE_STIPPLED);
	update();
	MESSAGE("B.3: Rectangles:\nNormal rectangle (outside)\nStippled rectangle (inside)");
	waitkey();

	gfxop_draw_rectangle(state, gfx_rect(30, 30, 260, 100), green, GFX_LINE_MODE_FINE, GFX_LINE_STYLE_NORMAL);
	gfxop_draw_rectangle(state, gfx_rect(40, 40, 240, 80), green, GFX_LINE_MODE_FINE, GFX_LINE_STYLE_STIPPLED);
	update();
	MESSAGE("B.4: Fine rectangles (optional):\nTwo green rectangles should now have\noverwritten the _inner_ bounds\nof the red ones");
	waitkey();
}


void
test_c(void) {
	int i;
	clear();
	update();
	MESSAGE("-- Test C: Boxes and updates --");
	waitkey();

	gfxop_fill_box(state, gfx_rect(50, 50, 210, 80), red);
	update();
	MESSAGE("C.0: Simple box");
	waitkey();

	gfxop_disable_dirty_frames(state);
	clear();
	gfxop_fill_box(state, gfx_rect(30, 30, 260, 100), white);
	gfxop_fill_box(state, gfx_rect(50, 50, 210, 80), blue);
	gfxop_enable_dirty_frames(state);

	MESSAGE("C.1: Partial propagation\nA white box containing a blue box\nhas been written to the back buffer.\nPress a key to propagate it.");

	for (i = 0; i <= 40; i++) {
		gfxop_update_box(state, gfx_rect(i*4, 0 , 4, 150));
		gfxop_update_box(state, gfx_rect(317 - i*4, 0 , 4, 150));
		gfxop_sleep(state, 4);
	}

	gfxop_disable_dirty_frames(state);
	clear();
	gfxop_fill_box(state, gfx_rect(30, 30, 260, 100), red);
	gfxop_enable_dirty_frames(state);

	MESSAGE("C.2: Single line propagation\nPress a key to propagate a red box.\nNote that dirty frame\naccounting must be dis-\n"
	        "abled for manual updates\nto work like this.");
	waitkey();
	for (i = 159; i >= 0; i--) {
		gfxop_update_box(state, gfx_rect(i, 0 , 1, 150));
		gfxop_update_box(state, gfx_rect(319 - i, 0 , 1, 150));
		gfxop_sleep(state, 1);
	}

	clear();
	gfxop_draw_box(state, gfx_rect(150, 70, 20, 20), red, green, GFX_BOX_SHADE_FLAT);
	gfxop_draw_box(state, gfx_rect(120, 70, 20, 20), red, green, GFX_BOX_SHADE_LEFT);
	gfxop_draw_box(state, gfx_rect(180, 70, 20, 20), red, green, GFX_BOX_SHADE_RIGHT);
	gfxop_draw_box(state, gfx_rect(150, 40, 20, 20), red, green, GFX_BOX_SHADE_UP);
	gfxop_draw_box(state, gfx_rect(150, 100, 20, 20), red, green, GFX_BOX_SHADE_DOWN);
	update();
	MESSAGE("C.3: Gradient boxes\nIf your driver supports gradient\nboxes, you should now see\nthe four outer boxes being\nred on the inside,--more--");
	waitkey();
	MESSAGE("C.3: Gradient boxes\n(cont.)and green on the\n outside.\n\nIf unsupported, all should be red.");
	waitkey();
}


void
test_d(void) {
	rect_t line;
	int pressed = 0;
	sci_event_t event;

	event.type = 0;

	MESSAGE("-- Test D: Pointers and input --");
	clear();
	gfxop_fill_box(state, gfx_rect(30, 30, 260, 100), white);
	gfxop_fill_box(state, gfx_rect(50, 50, 210, 80), blue);
	waitkey();
	update();

	gfxop_set_pointer_cursor(state, 1);
	MESSAGE("D.0: Simple mouse pointer\nThis pointer's hot spot is at the\ntop left");
	waitkey();

	gfxop_set_pointer_cursor(state, 0);
	MESSAGE("D.1: Crosshair pointer\nHot spot is in the center.\nThis is a 'multicolor' pointer.");
	waitkey();

	MESSAGE("D.2: Mouse clicks\nPress and release buttons to\ndraw lines\nPress any key to continue");

	while (event.type != SCI_EVT_KEYBOARD) {
		event = gfxop_get_event(state, SCI_EVT_ANY);

		if (event.type == SCI_EVT_MOUSE_PRESS) {

			pressed = 1;
			line.x = state->pointer_pos.x;
			line.y = state->pointer_pos.y;

		} else if (event.type == SCI_EVT_MOUSE_RELEASE)

			if (pressed) {
				point_t line_pt = gfx_point(line.x, line.y);
				pressed = 0;
				line.xl = state->pointer_pos.x - line.x;
				line.yl = state->pointer_pos.y - line.y;
				gfxop_draw_line(state, state->pointer_pos, line_pt, red, GFX_LINE_MODE_FAST, GFX_LINE_STYLE_NORMAL);

				if (line.xl < 0) {
					line.x += line.xl;
					line.xl = - line.xl;
				}

				if (line.yl < 0) {
					line.y += line.yl;
					line.yl = - line.yl;
				}

				gfxop_update_box(state, line);
			}

		gfxop_sleep(state, 1);
	}

	event.type = 0;

	MESSAGE("D.3 Event test\nPress the space bar when finished");
	waitkey();

	while (event.type != SCI_EVT_KEYBOARD || event.data != ' ') {
		event = gfxop_get_event(state, SCI_EVT_ANY);

		if (event.type)
			identify_event(event);
	}
}


void
test_e(void) {
	int x;

	gfxop_set_pointer_cursor(state, 1);

	MESSAGE("-- Test E: Pics and Views --");
	waitkey();

	gfxop_new_pic(state, 0, 0, 0);

	explicit_clear_buffer();
	update();
	MESSAGE("E.0: Static buffer\nYou should now see a gray and\nblue background image now.\nIt is stored in the static\nbuffer, and --more--");
	waitkey();
	MESSAGE("E.0: Static buffer\n(cont.)then propagated to\nthe back buffer and then to\nthe front buffer.");
	waitkey();

	MESSAGE("E.1: Views and animation\nPress a key to see a cel move\nacross the screen\n(doing _full_ updates)");
	waitkey();

	for (x = -20; x < 340; x++) {
		clear_buffer();
		gfxop_draw_cel(state, 0, 0, 0, gfx_point(x, 40), white, 0);
		update();
		gfxop_sleep(state, 10);
	}

	MESSAGE("E.2: Pic views\nFour pic views will now be added to\nthe static buffer");
	waitkey();

	gfxop_draw_cel_static(state, 0, 0, 1, gfx_point(50, 100), white24, 0);
	gfxop_draw_cel_static(state, 0, 0, 1, gfx_point(50, 20), white24, 0);
	gfxop_draw_cel_static(state, 0, 0, 1, gfx_point(220, 20), white16, 0);
	gfxop_draw_cel_static(state, 0, 0, 1, gfx_point(220, 100), white16, 0);

	update();
	MESSAGE("E.2: Pic views\nThe pic views should NOT\nbe visible yet!\n");
	waitkey();

	explicit_clear_buffer();
	update();
	MESSAGE("E.2: Pic views\nNow they should be.\n");
	waitkey();

	MESSAGE("E.3: Priority buffer\nPress a key to see two cels move\nacross the screen and re-\nspecting the Z buffer");
	waitkey();

	for (x = -20; x < 340; x++) {
		clear_buffer();
		gfxop_draw_cel(state, 0, 0, 2, gfx_point(x, 20), white8, 0);
		gfxop_draw_cel(state, 0, 0, 2, gfx_point(x, 100), white16, 0);
		update();
		gfxop_sleep(state, 10);
	}

	gfxop_add_to_pic(state, 1, 0, 0);
	explicit_clear_buffer();
	update();
	MESSAGE("E.4: Adding to a pic\nSome new stuff should\nhave been added to the\nbackground pic now");
	waitkey();
	MESSAGE("E.5: Animation with partial updates\nIf you're running high-res,\nthis should be considerably\nfaster.");
	waitkey();
	for (x = -20; x < 340; x++) {
		gfxop_clear_box(state, gfx_rect(x - 9, 40 - 8, 17, 16));
		gfxop_clear_box(state, gfx_rect(x - 9, 70 - 8, 17, 16));
		gfxop_draw_cel(state, 0, 0, 2, gfx_point(x, 40), white16, 0);
		gfxop_draw_cel(state, 0, 0, 2, gfx_point(x, 70), white16, 0);
		gfxop_update(state);
		/*		gfxop_update_box(state, gfx_rect(x-1, 40, 17, 16)); */
		/*		gfxop_update_box(state, gfx_rect(x-1, 70, 17, 16)); */
		gfxop_sleep(state, 10);
	}
	waitkey();
}

void
test_wrap(int width, char *text) {
	rect_t rect = gfx_rect(0, 0, width, 120);
	gfx_text_handle_t *handle = gfxop_new_text(state, 0,
	                            text,
	                            width, ALIGN_LEFT, ALIGN_TOP, white, white, transparent, 0);

	gfxop_fill_box(state, rect, dblue);
	gfxop_draw_rectangle(state, rect, blue, GFX_LINE_MODE_FAST, GFX_LINE_STYLE_NORMAL);
	gfxop_draw_text(state, handle, gfx_rect(0, 0, 320, 150));
	gfxop_free_text(state, handle);
}

void
test_f(void) {
	int i;
	int x, y;
	gfx_text_handle_t *handle;
	MESSAGE("-- Test F: Full font test --");
	waitkey();

	handle = gfxop_new_text(state, 0, "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz1234567890",
	                        320, ALIGN_LEFT, ALIGN_TOP, white, white, transparent, 0);
	gfxop_draw_text(state, handle, gfx_rect(0, 0, 320, 150));
	gfxop_free_text(state, handle);
	update();

	MESSAGE("F.0: Font wrap\nYou should now see the alphabet\n(capitals, then small letters),\nand the numbers from 1 to 0"
	        "\nwrapped with a maxwidth of\n320 (screen width)");
	waitkey();

	MESSAGE("F.1: Font wrap:\nMisc. wrap widths\nAll text should be /within/ the\nblue box (it may touch\nthe border, though)");
	waitkey();
	for (i = 200; i > 50; i -= 7) {
		clear_buffer();
		test_wrap(i, "\"Far out in the uncharted regions of the western spiral arm of the galaxy...\"");
		update();
		MESSAGE1("\nwidth=%d", i);
		waitkey();
	}

	handle = gfxop_new_text(state, 0, "And now for something completely different.",
	                        320, ALIGN_LEFT, ALIGN_TOP, red, green, black, 0);
	gfxop_draw_text(state, handle, gfx_rect(0, 0, 320, 150));
	gfxop_free_text(state, handle);
	update();
	MESSAGE("F.2: Dithered text\nThis text should now be dithered\nred/green on black background");
	waitkey();

	clear_buffer();
	handle = gfxop_new_text(state, 0, "foo!",
	                        320, ALIGN_CENTER, ALIGN_CENTER, blue, blue, transparent, 0);
	x = 10;
	y = 10;

	for (i = 0; i < 1000; i++) {
		x = (x + (70 + (i / y) + y * y * x / (i + 1)));
		y = (y + (30 + (i / x) + x * x * y / (i + 1)));
		gfxop_draw_text(state, handle, gfx_rect(x % 320, y % 140, 0, 0));
	}

	gfxop_free_text(state, handle);
	update();
	MESSAGE("F.3: Multiple draws\nText handles may be used more\nthan once\n(1000 times here)");
	waitkey();
}

void
do_tests(char *conf) {
	init_colors();


	if (strchr(conf, 'a'))
		if (test_a())
			return;

	if (strchr(conf, 'b'))
		test_b();

	if (strchr(conf, 'c'))
		test_c();

	if (strchr(conf, 'd'))
		test_d();

	if (strchr(conf, 'e'))
		test_e();

	if (strchr(conf, 'f'))
		test_f();
}

int
c_quit(void *S) {
	exit(0);
	return 0; /* hahaha */
}

int
main(int argc, char **argv) {
	gfx_driver_t *drv = NULL;
	char c;

	strcpy(tests, ALL_TESTS);

	printf("gfx_test Copyright (C) 2000 Christoph Reichenbach\nThis program is provided WITHOUT WARRANTY of any kind. Please\n"
	       "refer to the file COPYING that should have come with this\ndistribution for licensing details.\n\n");

	while ((c = getopt(argc, argv, "nhslc:g:x:y:t:")) > -1)
		switch (c) {

		case 'h':
			printf("Usage: gfx_test [-l] [-h] [-g driver] [-x xfact] [-y yfact] [-c bpp]\n"
			       "-l: List all graphics targets\n"
			       "-x: Set x resolution scale factor\n"
			       "-y: Set y resolution scale factor\n"
			       "-s: Skip intro text and getchar()\n"
			       "-c: Set bytes per pixel\n"
			       "-h: Display this help message\n"
			       "-n: Immediately stop after displaying (for performance tests)\n"
			       "-g: Select any of the graphics drivers shown with -l\n"
			       "-t: Select the tests to run:\n"
			       "\ta: Text display and basic input\n"
			       "\tb: Lines\n"
			       "\tc: Boxes and updates\n"
			       "\td: Pointers and input\n"
			       "\te: Pics and Views\n"
			       "  -- for example, use \"-t abc\" to run tests A, B, and C\n"
			      );
			return 0;

		case 'n':
			nowait = 1;
			break;

		case 's':
			skip_intro = 1;
			break;

		case 'g':
			if (driver) sci_free(driver);
			driver = sci_strdup(optarg);
			break;

		case 'l': {
			int first = 1;
			int i = 0;

			printf("Available graphics drivers: ");
			while (gfx_get_driver_name(i)) {
				if (!first)
					printf(", ");
				first = 0;
				printf("%s", gfx_get_driver_name(i++));
			}
			printf("\n");
		}
		break;

		case 't':
			strcpy(tests, optarg);
			break;

		case 'x':
			set_mode = xres = atoi(optarg);
			if (xres < 1) {
				fprintf(stderr, "Invalid x scale factor!\n");
				return 1;
			}
			break;

		case 'y':
			set_mode = yres = atoi(optarg);
			if (yres < 1) {
				fprintf(stderr, "Invalid y scale factor!\n");
				return 1;
			}
			break;

		case 'c':
			set_mode = color_mode = atoi(optarg);
			if (color_mode < 1 || color_mode > 4) {
				fprintf(stderr, "Invalid number of bytes per pixel!\n");
				return 1;
			}
			break;

		default:
			fprintf(stderr, "Run 'gfx_test -h' for help\n");
			return 1;
		}

	drv = gfx_find_driver("/tmp", driver);

	if (drv) {
		printf("Using graphics driver '%s'\n", drv->name);

		if (!skip_intro) {
			printf("Testing will now start. The first test will check whether displaying\n"
			       "text and reading input works; it will display a message and wait for twenty\n"
			       "seconds for you to press the space bar. If it does not register a space bar\n"
			       "keypress, it will abort.\n"
			       "Note that it can happen that no text is displayed, but you still are able\n"
			       "to proceed to the next test by pressing the space bar. However, unless you\n"
			       "are running in windowed mode, you will not be able to read which kinds of\n"
			       "tests are being run, so it is recommended that you fix pixmap displaying\n"
			       "and back-to-front-buffer updating first, so that the text can be displayed\n"
			       "correctly.\n"
			       "-- Press any key to start the first test or Ctrl-C to abort --\n");
			getchar();
		}

		if (init_driver(drv)) {
			fprintf(stderr, "Initialization failed!\n");
			return 1;
		}

		do_tests(tests);

		if (gfxop_exit(state)) {
			fprintf(stderr, "Something weird happened while exitting...\n");
		}
	} else {
		fprintf(stderr, "No graphics driver found!\n");
		return 1;
	}

	return 0;
}



byte builtin_font[] = {
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

} // End of namespace Sci
