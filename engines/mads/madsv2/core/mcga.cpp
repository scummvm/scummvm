/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/system.h"
#include "graphics/paletteman.h"
#include "mads/madsv2/core/general.h"
#include "mads/madsv2/core/mem.h"
#include "mads/madsv2/core/pal.h"
#include "mads/madsv2/engine.h"

namespace MADS {
namespace MADSV2 {

#define mcga_retrace_magic      14      /* Dave McKibbin's magic heuristic # */

/* --------------------------------------------------------------------------
 * Default VGA DAC palette for mode 13h (256 colours, 6-bit 0-63 range).
 *
 * In DOS, the BIOS loaded this table into the DAC when the video mode was
 * set.  mcga_compute_retrace_parameters() read it back and stored a copy in
 * master_palette.  ScummVM has no hardware DAC to read, so we supply the
 * table explicitly.
 *
 * Source: captured via DOSBox debugger immediately after kernel_game_startup
 * sets the video mode, before any game palette calls run.
 *   Entries  0–15 : standard EGA/VGA 16-colour compatibility palette
 *   Entries 16–31 : near-black utility + skin/cloth tones (room 922) + grey ramp
 *   Entries 32–55 : full-saturation hue wheel (blue→magenta→red→yellow→green→cyan)
 *   Entries 56–255: not yet captured; zeroed until confirmed
 *
 * NOTE: entries 0–3 were recorded after pal_white() ran (grayscale ramp).
 * The true BIOS defaults for those four slots are the first four EGA colours
 * ((0,0,0) (0,0,42) (0,42,0) (0,42,42)), but pal_interface() overwrites them
 * immediately after this function returns, so the distinction is moot.
 * -------------------------------------------------------------------------- */
static const RGBcolor vga_default_pal[256] = {
    /* 0-15: EGA compatibility (entries 0-3 show post-pal_white grayscale) */
    {0x00,0x00,0x00}, {0x15,0x15,0x15}, {0x2A,0x2A,0x2A}, {0x3F,0x3F,0x3F}, /* 0-3  */
    {0x2A,0x00,0x00}, {0x2A,0x00,0x2A}, {0x2A,0x15,0x00}, {0x2A,0x2A,0x2A}, /* 4-7  */
    {0x15,0x15,0x15}, {0x15,0x15,0x3F}, {0x15,0x3F,0x15}, {0x15,0x3F,0x3F}, /* 8-11 */
    {0x3F,0x15,0x15}, {0x3F,0x15,0x3F}, {0x3F,0x3F,0x15}, {0x3F,0x3F,0x3F}, /* 12-15*/

    /* 16-31: near-black + game skin/cloth tones + grey ramp                 */
    {0x00,0x00,0x00}, {0x05,0x05,0x05},                                       /* 16-17*/
    {0x31,0x27,0x1A}, {0x25,0x18,0x12}, {0x11,0x10,0x21}, {0x05,0x0F,0x18},  /* 18-21*/
    {0x18,0x1C,0x1E}, {0x0F,0x14,0x16}, {0x16,0x14,0x14}, {0x00,0x2A,0x2A},  /* 22-25*/
    {0x24,0x24,0x24}, {0x28,0x28,0x28}, {0x2D,0x2D,0x2D}, {0x32,0x32,0x32},  /* 26-29*/
    {0x38,0x38,0x38}, {0x3F,0x3F,0x3F},                                       /* 30-31*/

    /* 32-55: full-saturation hue wheel (matches standard VGA BIOS defaults) */
    /* blue → magenta */
    {0x00,0x00,0x3F}, {0x10,0x00,0x3F}, {0x1F,0x00,0x3F}, {0x2F,0x00,0x3F}, /* 32-35*/
    /* magenta → red */
    {0x3F,0x00,0x3F}, {0x3F,0x00,0x2F}, {0x3F,0x00,0x1F}, {0x3F,0x00,0x10}, /* 36-39*/
    /* red → yellow */
    {0x3F,0x00,0x00}, {0x3F,0x10,0x00}, {0x3F,0x1F,0x00}, {0x3F,0x2F,0x00}, /* 40-43*/
    /* yellow → green */
    {0x3F,0x3F,0x00}, {0x2F,0x3F,0x00}, {0x1F,0x3F,0x00}, {0x10,0x3F,0x00}, /* 44-47*/
    /* green → cyan */
    {0x00,0x3F,0x00}, {0x00,0x3F,0x10}, {0x00,0x3F,0x1F}, {0x00,0x3F,0x2F}, /* 48-51*/
    /* cyan → blue */
    {0x00,0x3F,0x3F}, {0x00,0x2F,0x3F}, {0x00,0x1F,0x3F}, {0x00,0x10,0x3F}, /* 52-55*/

    /* 56-255: not yet captured; zeroed */
};

word mcga_shakes = false;
int  mcga_retrace_computed = false;
word mcga_retrace_ticks = 0;
int  mcga_retrace_max_colors = 256;
int  mcga_retrace_max_bytes = 768;

word mcga_palette_update = false;
int  mcga_palette_fast = true;


void mcga_put_pixel(word x, word y, byte c) {
	byte *dest = (byte *)g_engine->getScreen()->getBasePtr(x, y);
	*dest = c;
}

byte mcga_get_pixel(word x, word y) {
	return *(byte *)g_engine->getScreen()->getBasePtr(x, y);
}

byte *mcga_open_window(word x, word y, word xsize, word ysize) {
	byte *out;
	byte *live;
	byte *work;
	word *run;
	word yy, size;

	size = xsize * ysize + 8;
	out = (byte *)mem_get((long)size);
	if (out == NULL)
		return(NULL);

	run = (word *)out;
	work = out + 8;

	*run++ = x;
	*run++ = y;
	*run++ = xsize;
	*run++ = ysize;

	live = (byte *)g_engine->getScreen()->getBasePtr(x, y);

	for (yy = 1; yy <= ysize; yy++) {
		memcpy(work, live, xsize);
		work += xsize;
		live += 320;
	}

	return out;
}

void mcga_close_window(byte *inp) {
	word *run;
	byte *live;
	byte *work;
	word x, y, xsize, ysize, yy;

	run = (word *)inp;
	work = inp + 8;
	x = *run++;
	y = *run++;
	xsize = *run++;
	ysize = *run++;

	live = (byte *)g_engine->getScreen()->getBasePtr(x, y);

	for (yy = 1; yy <= ysize; yy++) {
		memcpy(live, work, xsize);
		work += xsize;
		live += 320;
	}

	mem_free(inp);
}

void mcga_getpal(Palette *pal) {
	byte tmp[Graphics::PALETTE_COUNT * 3];
	g_system->getPaletteManager()->grabPalette(tmp, 0, Graphics::PALETTE_COUNT);
	for (int i = 0; i < Graphics::PALETTE_COUNT; i++) {
		(*pal)[i].r = tmp[i * 3 + 0] * 63 / 255;
		(*pal)[i].g = tmp[i * 3 + 1] * 63 / 255;
		(*pal)[i].b = tmp[i * 3 + 2] * 63 / 255;
	}
}

void mcga_setpal_range(Palette *pal, int first_color, int num_colors) {
	byte tmp[Graphics::PALETTE_COUNT * 3];
	for (int i = 0; i < num_colors; i++) {
		tmp[i * 3 + 0] = (*pal)[first_color + i].r * 255 / 63;
		tmp[i * 3 + 1] = (*pal)[first_color + i].g * 255 / 63;
		tmp[i * 3 + 2] = (*pal)[first_color + i].b * 255 / 63;
	}
	g_system->getPaletteManager()->setPalette(tmp, first_color, num_colors);
}

void mcga_setpal(Palette *pal) {
	mcga_setpal_range(pal, 0, Graphics::PALETTE_COUNT);
}

void mcga_cls(byte inp) {
	g_engine->getScreen()->clear(inp);
}

void mcga_retrace() {
	g_engine->getScreen()->update();
}

static word mcga_time_palette_swap(Palette *pal, int first_color, int num_colors) {
	mcga_setpal(pal);
	return 0;
}

void mcga_compute_retrace_parameters(void) {
	// In DOS, the BIOS initialised the VGA DAC with a default 256-colour
	// palette when the video mode was set; this function read that palette
	// back and stored it in master_palette so the engine had a populated
	// baseline before any room-specific colours were allocated.
	// In ScummVM there is no hardware DAC to read, so we copy the known
	// default table directly instead of calling mcga_getpal().
	memcpy(&master_palette, &vga_default_pal, sizeof(Palette));
	mcga_setpal(&master_palette);

	// On original hardware this function also measured how many palette
	// entries could be written inside one vertical retrace interval.
	// Under a modern graphics API there is no such constraint.
	mcga_palette_fast = true;
	mcga_retrace_max_colors = Graphics::PALETTE_COUNT;
	mcga_retrace_max_bytes = Graphics::PALETTE_SIZE;
	mcga_retrace_computed = true;
}

void mcga_shake() {
	warning("TODO: mcga_shake");
}

void mcga_reset() {
	Palette temp_palette;
	pal_interface(temp_palette);

	memcpy(&temp_palette[20], &temp_palette[6], sizeof(RGBcolor));
	memcpy(&temp_palette[56], &temp_palette[8], sizeof(RGBcolor) << 3);

	mcga_setpal(&temp_palette);
}

} // namespace MADSV2
} // namespace MADS
