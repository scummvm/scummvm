/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
 *
 * Copyright (C) 1999-2003 Sarien Team
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

#include "common/stdafx.h"

#include "agi/agi.h"
#include "agi/graphics.h"

namespace Agi {

#define DEV_X0(x) ((x) << 1)
#define DEV_X1(x) (((x) << 1) + 1)
#define DEV_Y(x) (x)

#ifndef MAX_INT
#  define MAX_INT (int)((unsigned)~0 >> 1)
#endif

static uint8 *agi_screen;
#ifdef USE_CONSOLE
static uint8 *console_screen;
#endif

static unsigned char *screen;

extern uint8 cur_font[];

/**
 * 16 color RGB palette (plus 16 transparent colors).
 * This array contains the 6-bit RGB values of the EGA palette exported
 * to the console drivers.
 */
uint8 ega_palette[16 * 3] = {
	0x00, 0x00, 0x00,
	0x00, 0x00, 0x2a,
	0x00, 0x2a, 0x00,
	0x00, 0x2a, 0x2a,
	0x2a, 0x00, 0x00,
	0x2a, 0x00, 0x2a,
	0x2a, 0x15, 0x00,
	0x2a, 0x2a, 0x2a,
	0x15, 0x15, 0x15,
	0x15, 0x15, 0x3f,
	0x15, 0x3f, 0x15,
	0x15, 0x3f, 0x3f,
	0x3f, 0x15, 0x15,
	0x3f, 0x15, 0x3f,
	0x3f, 0x3f, 0x15,
	0x3f, 0x3f, 0x3f
};

/**
 * 16 color amiga-ish palette.
 */
uint8 new_palette[16 * 3] = {
	0x00, 0x00, 0x00,
	0x00, 0x00, 0x3f,
	0x00, 0x2A, 0x00,
	0x00, 0x2A, 0x2A,
	0x33, 0x00, 0x00,
	0x2f, 0x1c, 0x37,
	0x23, 0x14, 0x00,
	0x2f, 0x2f, 0x2f,
	0x15, 0x15, 0x15,
	0x00, 0x2f, 0x3f,
	0x00, 0x33, 0x15,
	0x15, 0x3F, 0x3F,
	0x3f, 0x27, 0x23,
	0x3f, 0x15, 0x3f,
	0x3b, 0x3b, 0x00,
	0x3F, 0x3F, 0x3F
};

uint8 palette[32 * 3];

static uint16 cga_map[16] = {
	0x0000,			/*  0 - black */
	0x0d00,			/*  1 - blue */
	0x0b00,			/*  2 - green */
	0x0f00,			/*  3 - cyan */
	0x000b,			/*  4 - red */
	0x0b0d,			/*  5 - magenta */
	0x000d,			/*  6 - brown */
	0x0b0b,			/*  7 - gray */
	0x0d0d,			/*  8 - dark gray */
	0x0b0f,			/*  9 - light blue */
	0x0b0d,			/* 10 - light green */
	0x0f0d,			/* 11 - light cyan */
	0x0f0d,			/* 12 - light red */
	0x0f00,			/* 13 - light magenta */
	0x0f0b,			/* 14 - yellow */
	0x0f0f			/* 15 - white */
};

struct update_block {
	int x1, y1;
	int x2, y2;
};

static struct update_block update = {
	MAX_INT, MAX_INT, 0, 0
};

/*
 *  Layer 4:  640x480?  ==================  User display
 *                              ^
 *                              |  do_update(), put_block()
 *                              |
 *  Layer 3:  640x480?  ==================  Framebuffer
 *                              ^
 *                              |  flush_block(), put_pixels()
 *                              |
 *  Layer 2:  320x200   ==================  AGI engine screen (console), put_pixel()
 *                              |
 *  Layer 1:  160x168   ==================  AGI screen
 */

#ifdef USE_CONSOLE

/**
 * Draws a row of pixels in the output device framebuffer.
 * This function adds the console layer using transparent colors if
 * appropriate.
 */
static void put_pixels(const int x, const int y, const int w, uint8 *p) {
	int i;
	uint8 _b[GFX_WIDTH] = { 0 };
	uint8 *b, *c = NULL;

	if (console.y <= y) {
		memcpy(screen + x + y * 320, p, w);
		return;
	}

	b = &_b[0];
	c = &console_screen[x + (y + GFX_HEIGHT - 1 - console.y) * GFX_WIDTH];

	for (i = 0; i < w; i++, c++, p++) {
		*b++ = *c ? *c : *p + 16;
	}

	memcpy(screen + x + y * 320, p, w);
}

static void init_console() {
	int i;

	/* Console */
	console.line[0] = (char *)calloc(CONSOLE_LINES_BUFFER, CONSOLE_LINE_SIZE + 1);
	for (i = 1; i < CONSOLE_LINES_BUFFER; i++)
		console.line[i] = console.line[i - 1] + CONSOLE_LINE_SIZE + 1;
}

#else

static void put_pixels(const int x, const int y, const int w, uint8 *p) {
	gfx->put_pixels(x, y, w, p);
}

static void init_console()
{
}

#endif				/* USE_CONSOLE */

#define SHAKE_MAG 3
static uint8 *shake_h, *shake_v;

void shake_start() {
	int i;

	if ((shake_h = (uint8 *)malloc(GFX_WIDTH * SHAKE_MAG)) == NULL)
		return;

	if ((shake_v = (uint8 *)malloc(SHAKE_MAG * (GFX_HEIGHT - SHAKE_MAG))) == NULL) {
		free(shake_h);
		return;
	}

	for (i = 0; i < GFX_HEIGHT - SHAKE_MAG; i++) {
		memcpy(shake_v + i * SHAKE_MAG, agi_screen + i * GFX_WIDTH, SHAKE_MAG);
	}

	for (i = 0; i < SHAKE_MAG; i++) {
		memcpy(shake_h + i * GFX_WIDTH, agi_screen + i * GFX_WIDTH, GFX_WIDTH);
	}
}

void shake_screen(int n) {
	int i;

	if (n == 0) {
		for (i = 0; i < (GFX_HEIGHT - SHAKE_MAG); i++) {
			memmove(&agi_screen[GFX_WIDTH * i],
					&agi_screen[GFX_WIDTH * (i + SHAKE_MAG) + SHAKE_MAG],
					GFX_WIDTH - SHAKE_MAG);
		}
	} else {
		for (i = GFX_HEIGHT - SHAKE_MAG - 1; i >= 0; i--) {
			memmove(&agi_screen[GFX_WIDTH * (i + SHAKE_MAG) + SHAKE_MAG],
					&agi_screen[GFX_WIDTH * i], GFX_WIDTH - SHAKE_MAG);
		}
	}
}

void shake_end() {
	int i;

	for (i = 0; i < GFX_HEIGHT - SHAKE_MAG; i++) {
		memcpy(agi_screen + i * GFX_WIDTH, shake_v + i * SHAKE_MAG, SHAKE_MAG);
	}

	for (i = 0; i < SHAKE_MAG; i++) {
		memcpy(agi_screen + i * GFX_WIDTH, shake_h + i * GFX_WIDTH, GFX_WIDTH);
	}

	flush_block(0, 0, GFX_WIDTH - 1, GFX_HEIGHT - 1);

	free(shake_v);
	free(shake_h);
}

void put_text_character(int l, int x, int y, unsigned int c, int fg, int bg) {
	int x1, y1, xx, yy, cc;
	uint8 *p;

	p = cur_font + ((unsigned int)c * CHAR_LINES);
	for (y1 = 0; y1 < CHAR_LINES; y1++) {
		for (x1 = 0; x1 < CHAR_COLS; x1++) {
			xx = x + x1;
			yy = y + y1;
			cc = (*p & (1 << (7 - x1))) ? fg : bg;
#ifdef USE_CONSOLE
			if (l) {
				console_screen[xx + yy * GFX_WIDTH] = cc;
			} else
#endif
			{
				agi_screen[xx + yy * GFX_WIDTH] = cc;
			}
		}

		p++;
	}
	/* FIXME: we don't want this when we're writing on the
	 *        console!
	 */
	flush_block(x, y, x + CHAR_COLS - 1, y + CHAR_LINES - 1);
}

void draw_rectangle(int x1, int y1, int x2, int y2, int c) {
	int y, w, h;
	uint8 *p0;

	if (x1 >= GFX_WIDTH)
		x1 = GFX_WIDTH - 1;
	if (y1 >= GFX_HEIGHT)
		y1 = GFX_HEIGHT - 1;
	if (x2 >= GFX_WIDTH)
		x2 = GFX_WIDTH - 1;
	if (y2 >= GFX_HEIGHT)
		y2 = GFX_HEIGHT - 1;

	w = x2 - x1 + 1;
	h = y2 - y1 + 1;
	p0 = &agi_screen[x1 + y1 * GFX_WIDTH];
	for (y = 0; y < h; y++) {
		memset(p0, c, w);
		p0 += GFX_WIDTH;
	}
}

static void draw_frame(int x1, int y1, int x2, int y2, int c1, int c2) {
	int y, w;
	uint8 *p0;

	/* top line */
	w = x2 - x1 + 1;
	p0 = &agi_screen[x1 + y1 * GFX_WIDTH];
	memset(p0, c1, w);

	/* bottom line */
	p0 = &agi_screen[x1 + y2 * GFX_WIDTH];
	memset(p0, c2, w);

	/* side lines */
	for (y = y1; y <= y2; y++) {
		agi_screen[x1 + y * GFX_WIDTH] = c1;
		agi_screen[x2 + y * GFX_WIDTH] = c2;
	}
}

void draw_box(int x1, int y1, int x2, int y2, int colour1, int colour2, int m) {
	x1 += m;
	y1 += m;
	x2 -= m;
	y2 -= m;

	draw_rectangle(x1, y1, x2, y2, colour1);
	draw_frame(x1 + 2, y1 + 2, x2 - 2, y2 - 2, colour2, colour2);
	flush_block(x1, y1, x2, y2);
}

void print_character(int x, int y, char c, int fg, int bg) {
	x *= CHAR_COLS;
	y *= CHAR_LINES;

	put_text_character(0, x, y, c, fg, bg);
	// redundant! already inside put_text_character!
	// flush_block (x, y, x + CHAR_COLS - 1, y + CHAR_LINES - 1);
}

/**
 * Draw button
 * @param x  x coordinate of the button
 * @param y  y coordinate of the button
 * @param a  set if the button has focus
 * @param p  set if the button is pressed
 */
void draw_button(int x, int y, char *s, int a, int p) {
	int len = strlen(s);
	int x1, y1, x2, y2;

	x1 = x - 3;
	y1 = y - 3;
	x2 = x + CHAR_COLS * len + 2;
	y2 = y + CHAR_LINES + 2;

	while (*s) {
		put_text_character(0, x + (!!p), y + (!!p), *s++,
		    a ? 15 : 0, a ? 0 : 15);
		x += CHAR_COLS;
	}

	x1 -= 2;
	y1 -= 2;
	x2 += 2;
	y2 += 2;

	flush_block(x1, y1, x2, y2);
}

#ifdef USE_MOUSE
int test_button(int x, int y, char *s) {
	int len = strlen(s);
	int x1, y1, x2, y2;

	x1 = x - 3;
	y1 = y - 3;
	x2 = x + CHAR_COLS * len + 2;
	y2 = y + CHAR_LINES + 2;

	if ((int)mouse.x >= x1 && (int)mouse.y >= y1
	    && (int)mouse.x <= x2 && (int)mouse.y <= y2)
		return true;

	return false;
}
#endif

void put_block(int x1, int y1, int x2, int y2) {
	gfx_putblock(x1, y1, x2, y2);
}

void put_screen() {
	put_block(0, 0, GFX_WIDTH - 1, GFX_HEIGHT - 1);
}

void poll_timer() {
	agi_timer_low();
}

int get_key() {
	return agi_get_keypress_low();
}

int keypress() {
	return agi_is_keypress_low();
}

/*
 * Public functions
 */

/**
 * Initialize the color palette
 * This function initializes the color palette using the specified 16-color
 * RGB palette and creates 16 extra palette entries with translucent colors
 * for the interpreter console.
 * @param p  A pointer to the 16-color RGB palette.
 */
void init_palette(uint8 *p) {
	int i;

	for (i = 0; i < 48; i++) {
		palette[i] = p[i];
		palette[i + 48] = (p[i] + 0x30) >> 2;
	}
}

void gfx_set_palette() {
	int i;
	byte pal[32 * 4];

	for (i = 0; i < 32; i++) {
		pal[i * 4 + 0] = palette[i * 3 + 0] << 2;
		pal[i * 4 + 1] = palette[i * 3 + 1] << 2;
		pal[i * 4 + 2] = palette[i * 3 + 2] << 2;
		pal[i * 4 + 3] = 0;
	}
	g_system->setPalette(pal, 0, 32);
}

/* put a block onto the screen */
void gfx_putblock(int x1, int y1, int x2, int y2) {
	if (x1 >= GFX_WIDTH)
		x1 = GFX_WIDTH - 1;
	if (y1 >= GFX_HEIGHT)
		y1 = GFX_HEIGHT - 1;
	if (x2 >= GFX_WIDTH)
		x2 = GFX_WIDTH - 1;
	if (y2 >= GFX_HEIGHT)
		y2 = GFX_HEIGHT - 1;

	// force full update until fix wrong partial updates
	g_system->copyRectToScreen(screen, 320, 0, 0, 320, 200);
	//g_system->copyRectToScreen(screen, 320, x1, y1, x2 - x1 + 1, y2 - y1 + 1);
	//g_system->updateScreen();
}

static const byte mouseCursorArrow[] = {
	0x00, 0x00, 0x40, 0x00, 0x60, 0x00, 0x70, 0x00,
	0x78, 0x00, 0x7C, 0x00, 0x7E, 0x00, 0x7F, 0x00,
	0x7F, 0x80, 0x7C, 0x00, 0x6C, 0x00, 0x46, 0x00,
	0x06, 0x00, 0x03, 0x00, 0x03, 0x00, 0x00, 0x00,
	0xC0, 0x00, 0xE0, 0x00, 0xF0, 0x00, 0xF8, 0x00,
	0xFC, 0x00, 0xFE, 0x00, 0xFF, 0x00, 0xFF, 0x80,
	0xFF, 0xC0, 0xFF, 0xC0, 0xFE, 0x00, 0xFF, 0x00,
	0xCF, 0x00, 0x07, 0x80, 0x07, 0x80, 0x03, 0x80
};

/**
 * Initialize graphics device.
 *
 * @see deinit_video()
 */
int init_video() {
	if (opt.egapal)
		init_palette(ega_palette);
	else
		init_palette(new_palette);

	init_console();

	if ((agi_screen = (uint8 *)calloc(GFX_WIDTH, GFX_HEIGHT)) == NULL)
		return err_NotEnoughMemory;

#ifdef USE_CONSOLE
	if ((console_screen = (uint8 *)calloc(GFX_WIDTH, GFX_HEIGHT)) == NULL) {
		free(agi_screen);
		return err_NotEnoughMemory;
	}
#endif

	gfx_set_palette();

	byte mouseCursor[16 * 16];
	const byte *src = mouseCursorArrow;
	for (int i = 0; i < 32; ++i) {
		int offs = i * 8;
		for (byte mask = 0x80; mask != 0; mask >>= 1) {
			if (src[0] & mask) {
				mouseCursor[offs] = 2;
			} else if (src[32] & mask) {
				mouseCursor[offs] = 0;
			} else {
				mouseCursor[offs] = 0xFF;
			}
			++offs;
		}
		++src;
	}
	g_system->setMouseCursor(mouseCursor, 16, 16, 1, 1);

	return err_OK;
}

/**
 * Deinitialize graphics device.
 *
 * @see init_video()
 */
int deinit_video() {
	free(agi_screen);
#ifdef USE_CONSOLE
	free(console_screen);
#endif

	return err_OK;
}

int init_machine() {
	screen = (unsigned char *)malloc(320 * 200);
	clock_count = 0;
	clock_ticks = 0;

	return err_OK;
}

int deinit_machine() {
	free(screen);

	return err_OK;
}

/**
 * Write pixels on the output device.
 * This function writes a row of pixels on the output device. Only the
 * lower 4 bits of each pixel in the row will be used, making this
 * function suitable for use with rows from the AGI screen.
 * @param x x coordinate of the row start (AGI coord.)
 * @param y y coordinate of the row start (AGI coord.)
 * @param n number of pixels in the row
 * @param p pointer to the row start in the AGI screen
 */
void put_pixels_a(int x, int y, int n, uint8 *p) {
	if (opt.cgaemu) {
		for (x *= 2; n--; p++, x += 2) {
			register uint16 q = (cga_map[(*p & 0xf0) >> 4] << 4) | cga_map[*p & 0x0f];
#ifdef USE_CONSOLE
			if (debug_.priority)
				q >>= 4;
#endif
			*(uint16 *)&agi_screen[x + y * GFX_WIDTH] = q & 0x0f0f;
		}
	} else {
		for (x *= 2; n--; p++, x += 2) {
			register uint16 q = ((uint16) * p << 8) | *p;
#ifdef USE_CONSOLE
			if (debug_.priority)
				q >>= 4;
#endif
			*(uint16 *)&agi_screen[x + y * GFX_WIDTH] = q & 0x0f0f;
		}
	}
}

#ifdef USE_HIRES

void put_pixels_hires(int x, int y, int n, uint8 *p) {
	//y += CHAR_LINES;
	for (; n--; p++, x++) {
		uint8 q = *p;
#ifdef USE_CONSOLE
		if (debug_.priority)
			q >>= 4;
#endif
		agi_screen[x + y * GFX_WIDTH] = q & 0x0f;
	}
}

#endif

/**
 * Schedule blocks for blitting on the output device.
 * This function gets the coordinates of a block in the AGI screen and
 * schedule it to be updated in the output device.
 * @param x1 x coordinate of the upper left corner of the block (AGI coord.)
 * @param y1 y coordinate of the upper left corner of the block (AGI coord.)
 * @param x2 x coordinate of the lower right corner of the block (AGI coord.)
 * @param y2 y coordinate of the lower right corner of the block (AGI coord.)
 *
 * @see do_update()
 */
void schedule_update(int x1, int y1, int x2, int y2) {
	if (x1 < update.x1)
		update.x1 = x1;
	if (y1 < update.y1)
		update.y1 = y1;
	if (x2 > update.x2)
		update.x2 = x2;
	if (y2 > update.y2)
		update.y2 = y2;
}

/**
 * Update scheduled blocks on the output device.
 * This function exposes the blocks scheduled for updating to the output
 * device. Blocks can be scheduled at any point of the AGI cycle.
 *
 * @see schedule_update()
 */
void do_update() {
	if (update.x1 <= update.x2 && update.y1 <= update.y2) {
		gfx_putblock(update.x1, update.y1, update.x2, update.y2);
	}

	/* reset update block variables */
	update.x1 = MAX_INT;
	update.y1 = MAX_INT;
	update.x2 = 0;
	update.y2 = 0;
}

/**
 * Updates a block of the framebuffer with contents of the AGI engine screen.
 * This function updates a block in the output device with the contents of
 * the AGI engine screen, handling console transparency.
 * @param x1 x coordinate of the upper left corner of the block
 * @param y1 y coordinate of the upper left corner of the block
 * @param x2 x coordinate of the lower right corner of the block
 * @param y2 y coordinate of the lower right corner of the block
 *
 * @see flush_block_a()
 */
void flush_block(int x1, int y1, int x2, int y2) {
	int y, w;
	uint8 *p0;

	schedule_update(x1, y1, x2, y2);

	p0 = &agi_screen[x1 + y1 * GFX_WIDTH];
	w = x2 - x1 + 1;

	for (y = y1; y <= y2; y++) {
		put_pixels(x1, y, w, p0);
		p0 += GFX_WIDTH;
	}
}

/**
 * Updates a block of the framebuffer receiving AGI picture coordinates.
 * @param x1 x AGI picture coordinate of the upper left corner of the block
 * @param y1 y AGI picture coordinate of the upper left corner of the block
 * @param x2 x AGI picture coordinate of the lower right corner of the block
 * @param y2 y AGI picture coordinate of the lower right corner of the block
 *
 * @see flush_block()
 */
void flush_block_a(int x1, int y1, int x2, int y2) {
	//y1 += 8;
	//y2 += 8;
	flush_block(DEV_X0(x1), DEV_Y(y1), DEV_X1(x2), DEV_Y(y2));
}

/**
 * Updates the framebuffer with contents of the AGI engine screen (console-aware).
 * This function updates the output device with the contents of the AGI
 * screen, handling console transparency.
 */
void flush_screen() {
	flush_block(0, 0, GFX_WIDTH - 1, GFX_HEIGHT - 1);
}

/**
 * Clear the output device screen (console-aware).
 * This function clears the output device screen and updates the
 * output device. Contents of the AGI screen are left untouched. This
 * function can be used to simulate a switch to a text mode screen in
 * a graphic-only device.
 * @param c  color to clear the screen
 */
void clear_screen(int c) {
	memset(agi_screen, c, GFX_WIDTH * GFX_HEIGHT);
	flush_screen();
}

#ifdef USE_CONSOLE
/**
 * Clear the console screen.
 * This function clears the top n lines of the console screen.
 * @param n number of lines to clear (in pixels)
 */
void clear_console_screen(int n) {
	memset(console_screen + n * GFX_WIDTH, 0, (GFX_HEIGHT - n) * GFX_WIDTH);
}
#endif

/**
 * Save a block of the AGI engine screen
 */
void save_block(int x1, int y1, int x2, int y2, uint8 *b) {
	uint8 *p0;
	int w, h;

	p0 = &agi_screen[x1 + GFX_WIDTH * y1];
	w = x2 - x1 + 1;
	h = y2 - y1 + 1;
	while (h--) {
		memcpy(b, p0, w);
		b += w;
		p0 += GFX_WIDTH;
	}
}

/**
 * Restore a block of the AGI engine screen
 */
void restore_block(int x1, int y1, int x2, int y2, uint8 *b) {
	uint8 *p0;
	int w, h;

	p0 = &agi_screen[x1 + GFX_WIDTH * y1];
	w = x2 - x1 + 1;
	h = y2 - y1 + 1;
	while (h--) {
		memcpy(p0, b, w);
		b += w;
		p0 += GFX_WIDTH;
	}
	flush_block(x1, y1, x2, y2);
}

} // End of namespace Agi
