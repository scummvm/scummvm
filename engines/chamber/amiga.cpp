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

#include "common/file.h"
#include "common/system.h"
#include "graphics/paletteman.h"

#include "chamber/chamber.h"
#include "chamber/common.h"
#include "chamber/amiga.h"
#include "chamber/cga.h"
#include "chamber/ega.h"
#include "chamber/resdata.h"

namespace Chamber {

const byte *amiga_palette_table = nullptr;
const byte *amiga_room_palette_table = nullptr;

struct AmigaResEnt {
	const char *name;
	byte **buffer;
	uint32 offset;    // EU "Kult (M3)"
	uint32 size;
	uint32 offsetUS;  // US "Chamber" (Draconian)
	uint32 sizeUS;
};

#define AMIGA_PAL_OFFSET 40896
#define AMIGA_PAL_OFFSET_US 41010

// Each room takes a shared base and drops its own 5-word delta over slots 1..5
#define AMIGA_ROOM_PAL_OFFSET 40268
#define AMIGA_ROOM_PAL_OFFSET_US 40362
#define AMIGA_ROOM_DELTA_OFFSET 40068
#define AMIGA_ROOM_DELTA_OFFSET_US 40162
#define AMIGA_NUM_ROOM_PALETTES 20

// The US build ships no VERBE/MOTSE/DESCE/DIALE text files; the English banks
// are embedded in the exe (in this order, right after ANICO). EU keeps them as
// separate files because it is multilingual.
#define AMIGA_DIALE_OFFSET_US  82656
#define AMIGA_DESCE_OFFSET_US  92296
#define AMIGA_VERBE_OFFSET_US 102746
#define AMIGA_MOTSE_OFFSET_US 104066
#define AMIGA_MOTSE_END_US    105166

static const byte *amiga_room_delta_table = nullptr;

// Accent colours every room shares. Slots 1..5 get overwritten by the delta
static const uint16 amiga_room_base_pal[16] = {
	0x000, // 0  black (background)
	0x000, // 1  stone  } slots 1..5 overwritten by the per-room delta
	0x000, // 2  stone
	0x000, // 3  stone
	0x000, // 4  stone
	0x000, // 5  stone
	0x101, // 6  crevice
	0xE14, // 7  bright red UI accent (room-name-bar / timer-box border)
	0x886, // 8  stone/sprite highlight
	0x64A, // 9  blue-purple accent
	0x700, // 10 dark red
	0xEA8, // 11 peach (dialog boxes)
	0xD66, // 12 red (medallion accent)
	0x446, // 13 slate blue (portrait background)
	0xA8B, // 14 light mauve
	0xFFF  // 15 white (text box / highlights)
};

static AmigaResEnt amiga_res[] = {
	//   name          buffer         EU off  EU sz   US off  US sz
	{ "SOUCO.BIN", &souco_data,  37724,   424,  37818,   424 },
	{ "ZONES.BIN", &zones_data,  43250,  9014,  43428,  9014 },
	{ "TEMPL.BIN", &templ_data,  52318, 27336,  52496, 27370 },
	{ "MURSM.BIN", &mursm_data,  79654,    76,  79866,    76 },
	{ "ANIMA.BIN", &anima_data,  79730,  2046,  79942,  2046 },
	{ "ANICO.BIN", &anico_data,  81776,   667,  81988,   667 },
	{ "ARPLA.BIN", &arpla_data, 105612,  8024, 105166,  8024 },
	{ "CARAC.BIN", &carpc_data, 113636,   384, 113190,   384 },
	{ "GAUSS.BIN", &gauss_data, 114148,  2880, 113702,  2880 },
	{ "ALEAT.BIN", &aleat_data, 117028,   256, 116582,   256 },
	{ "ICONE.BIN", &icone_data, 117284,  2752, 116838,  2752 },
	{ "LUTIN.BIN", &lutin_data, 120036,  2800, 119594,  2800 },
};
static const int kAmigaNumRes = sizeof(amiga_res) / sizeof(amiga_res[0]);

// Copy an exe-embedded text bank into its fixed engine buffer, clamped to the
// buffer size and to the space up to the next bank so we never overrun either.
static void copyAmigaText(byte *dst, uint32 bufMax, const byte *raw, uint32 sz, uint32 off, uint32 next) {
	uint32 n = next - off;
	if (n > bufMax)
		n = bufMax;
	if (off + n > sz)
		return;
	memcpy(dst, raw + off, n);
}

int16 loadAmigaStaticData() {
	// Safe to call more than once: it runs both before the title splash (to make
	// the palette available) and again from loadStaticData(); the second call
	// returns immediately.
	if (amiga_palette_table != nullptr)
		return 1;

	Common::File kult;
	if (!kult.open("KULT")) {
		warning("loadAmigaStaticData(): cannot open KULT");
		return 0;
	}

	uint32 sz = kult.size();
	byte *raw = new byte[sz];
	if (kult.read(raw, sz) != sz) {
		warning("loadAmigaStaticData(): short read on KULT");
		delete[] raw;
		return 0;
	}

	// Reuse _pxiData (freed by the destructor) to own the KULT image
	delete[] g_vm->_pxiData;
	g_vm->_pxiData = raw;

	// The US "Chamber" build uses a different exe with its own offset column
	const bool isUS = g_vm->getLanguage() == Common::EN_USA;

	for (int i = 0; i < kAmigaNumRes; i++) {
		uint32 off = isUS ? amiga_res[i].offsetUS : amiga_res[i].offset;
		uint32 rsz = isUS ? amiga_res[i].sizeUS : amiga_res[i].size;
		if (off + rsz > sz) {
			warning("loadAmigaStaticData(): %s past EOF", amiga_res[i].name);
			return 0;
		}
		*amiga_res[i].buffer = raw + off;
	}

	amiga_palette_table = raw + (isUS ? AMIGA_PAL_OFFSET_US : AMIGA_PAL_OFFSET);
	amiga_room_palette_table = raw + (isUS ? AMIGA_ROOM_PAL_OFFSET_US : AMIGA_ROOM_PAL_OFFSET);
	amiga_room_delta_table = raw + (isUS ? AMIGA_ROOM_DELTA_OFFSET_US : AMIGA_ROOM_DELTA_OFFSET);

	// The US build has no VERBE/MOTSE/DESCE/DIALE files; slice the embedded
	// English banks out of the exe into the buffers loadFile() would have filled.
	if (isUS) {
		copyAmigaText(diali_data, RES_DIALI_MAX, raw, sz, AMIGA_DIALE_OFFSET_US, AMIGA_DESCE_OFFSET_US);
		copyAmigaText(desci_data, RES_DESCI_MAX, raw, sz, AMIGA_DESCE_OFFSET_US, AMIGA_VERBE_OFFSET_US);
		copyAmigaText(vepci_data, RES_VEPCI_MAX, raw, sz, AMIGA_VERBE_OFFSET_US, AMIGA_MOTSE_OFFSET_US);
		copyAmigaText(motsi_data, RES_MOTSI_MAX, raw, sz, AMIGA_MOTSE_OFFSET_US, AMIGA_MOTSE_END_US);
	}

	// Amiga glyphs sit one bit too high, shift them back into the low nibble
	for (uint32 i = 0; i < 384; i++)
		carpc_data[i] = (carpc_data[i] >> 1) & 0x0F;

	// SOURI (cursor) is a separate file, not embedded in KULT
	static byte souri_buf[RES_SOURI_MAX];
	if (!loadFile("SOURI.BIN", souri_buf))
		warning("loadAmigaStaticData(): SOURI.BIN not found");
	souri_data = souri_buf;

	return 1;
}

void amigaApplyPalette(byte index) {
	if (!amiga_palette_table)
		return;

	// Out-of-range indexes (e.g. CGA colorSelect bytes) are a no-op
	if (index >= AMIGA_NUM_PALETTES)
		return;

	const byte *src = amiga_palette_table;
	byte palData[16 * 3];
	for (int c = 0; c < 16; c++) {
		uint16 w = (src[c * 2] << 8) | src[c * 2 + 1];   // big-endian 0x0RGB
		// Expand each 4-bit channel to 8 bits
		palData[c * 3 + 0] = ((w >> 8) & 0xF) * 17;
		palData[c * 3 + 1] = ((w >> 4) & 0xF) * 17;
		palData[c * 3 + 2] = ( w       & 0xF) * 17;
	}
	g_system->getPaletteManager()->setPalette(palData, 0, 16);
}

void amigaApplyRoomPalette(byte index) {
	if (!amiga_room_delta_table)
		return;
	if (index >= AMIGA_NUM_ROOM_PALETTES)
		index = 0;

	// Base palette plus this room's 5-word delta overlaid onto slots 1..5
	uint16 pal[16];
	for (int c = 0; c < 16; c++)
		pal[c] = amiga_room_base_pal[c];
	const byte *delta = amiga_room_delta_table + index * (5 * 2);
	for (int c = 0; c < 5; c++)
		pal[1 + c] = (delta[c * 2] << 8) | delta[c * 2 + 1];

	byte palData[16 * 3];
	for (int c = 0; c < 16; c++) {
		uint16 w = pal[c];
		palData[c * 3 + 0] = ((w >> 8) & 0xF) * 17;
		palData[c * 3 + 1] = ((w >> 4) & 0xF) * 17;
		palData[c * 3 + 2] = ( w       & 0xF) * 17;
	}
	g_system->getPaletteManager()->setPalette(palData, 0, 16);
}

void AmigaRenderer::switchToGraphicsMode() {
	// No-op until loadAmigaStaticData() has run
	amigaApplyPalette(0);
}

void AmigaRenderer::colorSelect(byte csel) {
	// On Amiga this carries the real palette index (see room.cpp)
	amigaApplyPalette(csel);
}

static const byte amiga_cga_to_slot[4] = { 0, 11, 7, 15 };

void AmigaRenderer::drawVLine(uint16 x, uint16 y, uint16 l, byte color, byte *target) {
	byte pixel = amiga_cga_to_slot[color & 0x03];
	uint16 ofs = calcXY(x, y);
	for (uint16 i = 0; i < l; i++) {
		target[ofs] = pixel;
		ofs += EGA_BYTES_PER_LINE;
	}

	if (target == SCREENBUFFER)
		blitToScreen(x, y, 1, l);
}

void AmigaRenderer::drawHLine(uint16 x, uint16 y, uint16 l, byte color, byte *target) {
	byte pixel = amiga_cga_to_slot[color & 0x03];
	memset(target + calcXY(x, y), pixel, l);

	if (target == SCREENBUFFER)
		blitToScreen(x, y, l, 1);
}

} // End of namespace Chamber
