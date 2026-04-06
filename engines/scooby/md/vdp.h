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
 */

// VDP code taken from the genemu project
// https://github.com/rasky/genemu

#ifndef SCOOBY_VDP_H
#define SCOOBY_VDP_H

#include "graphics/surface.h"
#include "scooby/md/mem.h"
#include <stdint.h>

namespace Scooby {

#if 0
#define VDP_MASTER_FREQ       53693175   // NTSC
#define VDP_MASTER_FREQ       53203424   // PAL
#endif

#define VDP_CYCLES_PER_LINE   3420
#define YM2612_FREQ           53267

#define M68K_FREQ_DIVISOR     7
#define Z80_FREQ_DIVISOR      14

class VDP {
//    friend bool loadstate(const char *fn);
//    friend void savestate(const char *fn);
public:
	enum PlaneType {
		PlaneA,
		PlaneB,
		Window
	};

private:
	uint8_t VRAM[0x10000];
	uint16_t CRAM[0x40];
	uint16_t VSRAM[0x40];  // only 40 words are really used
	uint8_t SAT_CACHE[0x400]; // internal copy of SAT
	uint8_t regs[0x20];
	uint16_t fifo[4];
	uint16_t address_reg;
	uint8_t code_reg;
	uint16_t status_reg;
	bool hint_pending;
	int _vcounter;
	int line_counter_interrupt;
	bool command_word_pending;
	bool dma_fill_pending;
	bool hvcounter_latched;
	uint16_t hvcounter_latch;
	int sprite_overflow;
	bool sprite_collision;
	int mode_h40;
	int mode_pal;

	void VRAM_W(uint16_t address, uint8_t value);

public:
	VDP();
	void scanline(uint8_t *screen);

	void reset();

	unsigned int num_scanlines();

	int screen_offset();
	int screen_width();

private:
	void register_w(int reg, uint8_t value);

	int vcounter();

	int hcounter();

	bool vblank();

	void dma_trigger();

	void dma_fill(uint16_t value);

	void dma_copy();

	void dma_m68k();

	void push_fifo(uint16_t value);

	int get_nametable_A();
	int get_nametable_B();
	int get_nametable_W();

public:
	uint16_t status_register_r();

	void control_port_w(uint16_t value);

	void data_port_w16(uint16_t value);

	uint16_t data_port_r16(void);

	uint16_t hvcounter_r16(void);

	int irq_acked(int level);

	void zeroVRAM(uint16 destAddress, uint16 lengthInWords);

	void writeVRAM(uint16 destAddress, byte *srcBuf, uint16 length);
	void writeCRAM(uint16 destAddress, byte *srcBuf, uint16 length);
	void writeVSRAM(uint16 destAddress, byte *srcBuf, uint16 length);

	void writeVRAMWord(uint16 destAddress, uint16 word);

	void writePalette(uint16 paletteIndex, uint16 colour);
	uint16 readPaletteRecord(uint16 paletteIndex);

	int getNameTableAddress(PlaneType planeType);
	int getPlaneWidth(PlaneType planeType);
private:
	//Gfx methods

	template <bool fliph>
	bool draw_pattern(uint8_t *screen, uint8_t *pattern, uint8_t attrs);
	template <bool check_overdraw>
	bool draw_pattern(uint8_t *screen, uint16_t name, int paty);
	void draw_plane_ab(uint8_t *screen, int line, int ntaddr, uint16_t hs, uint16_t *vsram);
	void draw_plane_a(uint8_t *screen, int y);
	void draw_plane_b(uint8_t *screen, int y);
	void draw_plane_w(uint8_t *screen, int y);
	void draw_sprites(uint8_t *screen, int line);

	uint8_t* get_hscroll_vram(int line);
	inline bool in_window(int x, int y);
	uint8_t mix(uint8_t back, uint8_t b, uint8_t a, uint8_t s);

	void render_scanline(uint8_t *screen, int line);
};

//extern class VDP VDP;

void vdp_init(void);

void vdp_scanline(uint8_t *screen);

void vdp_mem_w8(unsigned int address, unsigned int value);

void vdp_mem_w16(unsigned int address, unsigned int value);

unsigned int vdp_mem_r8(unsigned int address);

unsigned int vdp_mem_r16(unsigned int address);

} // End of namespace Scooby

#endif SCOOBY_VDP_H
