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
#include "graphics/surface.h"

#include "chamber/chamber.h"
#include "chamber/common.h"
#include "chamber/cga.h"
#include "chamber/ega.h"
#include "chamber/input.h"
#include "chamber/resdata.h"
#include "chamber/room.h"

namespace Chamber {

// ---------------------------------------------------------------------------
// EGA 16-color palette (standard EGA RGB values)
// Source: kult/kult.cpp — this is the correct version (NOT BGR-swapped)
// ---------------------------------------------------------------------------
const byte EGA_PALETTE[16 * 3] = {
	0x00, 0x00, 0x00,  // 0  black
	0x00, 0x00, 0xaa,  // 1  dark blue
	0x00, 0xaa, 0x00,  // 2  dark green
	0x00, 0xaa, 0xaa,  // 3  dark cyan
	0xaa, 0x00, 0x00,  // 4  dark red
	0xaa, 0x00, 0xaa,  // 5  dark magenta
	0xaa, 0x55, 0x00,  // 6  brown
	0xaa, 0xaa, 0xaa,  // 7  light grey
	0x00, 0x00, 0x00,  // 8  dark grey (same as black in standard EGA)
	0x55, 0x55, 0xff,  // 9  bright blue
	0x55, 0xff, 0x55,  // 10 bright green
	0x55, 0xff, 0xff,  // 11 bright cyan
	0xff, 0x55, 0x55,  // 12 bright red
	0xff, 0x55, 0xff,  // 13 bright magenta
	0xff, 0xff, 0x55,  // 14 yellow
	0xff, 0xff, 0xff,  // 15 white
};

// ---------------------------------------------------------------------------
// EGA framebuffers — aliases to the shared CGA/EGA buffers (both 64000 bytes)
// ega_screen    -> CGA_SCREENBUFFER (frontbuffer in room.cpp)
// ega_backbuffer -> backbuffer       (backbuffer in room.cpp)
// ---------------------------------------------------------------------------
byte *ega_screen    = CGA_SCREENBUFFER;
byte *ega_backbuffer = backbuffer;

// ---------------------------------------------------------------------------
// Screen management
// ---------------------------------------------------------------------------

void ega_switchToGraphicsMode() {
	g_system->getPaletteManager()->setPalette(EGA_PALETTE, 0, 16);
}

void ega_blitToScreen(int16 x, int16 y, int16 w, int16 h) {
	if (x < 0) { w += x; x = 0; }
	if (y < 0) { h += y; y = 0; }
	if (x + w > EGA_WIDTH)  w = EGA_WIDTH  - x;
	if (y + h > EGA_HEIGHT) h = EGA_HEIGHT - y;
	if (w <= 0 || h <= 0)
		return;

	g_system->copyRectToScreen(
		ega_screen + y * EGA_BYTES_PER_LINE + x,
		EGA_BYTES_PER_LINE,
		x, y, w, h);
	g_system->updateScreen();
}

void ega_BackBufferToRealFull() {
	memcpy(ega_screen, ega_backbuffer, EGA_SCREEN_SIZE);
	ega_blitToScreen(0, 0, EGA_WIDTH, EGA_HEIGHT);
}

void ega_RealBufferToBackFull() {
	memcpy(ega_backbuffer, ega_screen, EGA_SCREEN_SIZE);
}

void ega_SwapRealBackBuffer() {
	static byte tmp[EGA_SCREEN_SIZE];
	memcpy(tmp,            ega_screen,    EGA_SCREEN_SIZE);
	memcpy(ega_screen,     ega_backbuffer, EGA_SCREEN_SIZE);
	memcpy(ega_backbuffer, tmp,            EGA_SCREEN_SIZE);
	ega_blitToScreen(0, 0, EGA_WIDTH, EGA_HEIGHT);
}

// ---------------------------------------------------------------------------
// Offset helpers
// EGA: linear layout, no interlacing — offset = y * 320 + x
// ---------------------------------------------------------------------------

uint16 ega_CalcXY(uint16 x, uint16 y) {
	return y * EGA_BYTES_PER_LINE + x;
}

// "packed" variant: x is in 4-pixel units (matching CGA convention)
uint16 ega_CalcXY_p(uint16 x, uint16 y) {
	return y * EGA_BYTES_PER_LINE + x * 4;
}

// ---------------------------------------------------------------------------
// Memory / block blit helpers
// w is in *bytes* == *pixels* in EGA mode (1 byte per pixel)
// ---------------------------------------------------------------------------

void ega_CopyScreenBlock(byte *source, uint16 w, uint16 h, byte *target, uint16 ofs) {
	uint16 oofs = ofs;
	for (uint16 y = 0; y < h; y++) {
		memcpy(target + ofs, source + ofs, w);
		ofs += EGA_BYTES_PER_LINE;
	}

	if (target == ega_screen)
		ega_blitToScreen(oofs % EGA_BYTES_PER_LINE, oofs / EGA_BYTES_PER_LINE, w, h);
}

void ega_SwapScreenRect(byte *pixels, uint16 w, uint16 h, byte *screen, uint16 ofs) {
	byte *old = scratch_mem2;
	uint16 oofs = ofs;
	for (uint16 y = 0; y < h; y++) {
		for (uint16 i = 0; i < w; i++) {
			*old++ = screen[ofs + i];
			screen[ofs + i] = *pixels++;
		}
		ofs += EGA_BYTES_PER_LINE;
	}

	if (screen == ega_screen)
		ega_blitToScreen(oofs % EGA_BYTES_PER_LINE, oofs / EGA_BYTES_PER_LINE, w, h);
}

// EGA backup image header (6 bytes):
//   [0]   h   (byte)
//   [1]   w_cga = w_pixels / 4 (byte, CGA byte-units; avoids overflow for w <= 1020 px)
//   [2:3] ofs (uint16, linear EGA offset)
// Pixel data follows at buffer+4, w_pixels bytes per row × h rows.
// w is passed as pixel width (w_cga * 4).  Callers from cga_BackupImage
// already multiply by 4; callers from backupAndShowSprite pass surf->w directly.
byte *ega_BackupImage(byte *source, uint16 ofs, uint16 w, uint16 h, byte *buffer) {
	*(byte *)(buffer + 0) = (byte)h;
	*(byte *)(buffer + 1) = (byte)(w >> 2); // store CGA byte-units to keep in one byte
	*(uint16 *)(buffer + 2) = ofs;
	buffer += 4;
	for (uint16 y = 0; y < h; y++) {
		memcpy(buffer, source + ofs, w);
		buffer += w;
		ofs += EGA_BYTES_PER_LINE;
	}
	return buffer;
}

void ega_RestoreImage(byte *buffer, byte *target) {
	if (!buffer)
		return;
	uint16 h   = *(byte *)(buffer + 0);
	uint16 w   = (uint16)*(byte *)(buffer + 1) * 4; // restore pixel width
	uint16 ofs = *(uint16 *)(buffer + 2);
	buffer += 4;
	ega_Blit(buffer, w, w, h, target, ofs);
}

void ega_RefreshImageData(byte *buffer) {
	if (!buffer)
		return;
	uint16 h   = *(byte *)(buffer + 0);
	uint16 w   = (uint16)*(byte *)(buffer + 1) * 4; // restore pixel width
	uint16 ofs = *(uint16 *)(buffer + 2);
	ega_CopyScreenBlock(ega_screen, w, h, ega_backbuffer, ofs);
}

void ega_RestoreBackupImage(byte *target) {
	ega_RestoreImage(scratch_mem2, target);
}

void ega_Blit(byte *pixels, uint16 pw, uint16 w, uint16 h, byte *screen, uint16 ofs) {
	uint16 oofs = ofs;
	for (uint16 y = 0; y < h; y++) {
		memcpy(screen + ofs, pixels, w);
		pixels += pw;
		ofs += EGA_BYTES_PER_LINE;
	}

	if (screen == ega_screen)
		ega_blitToScreen(oofs % EGA_BYTES_PER_LINE, oofs / EGA_BYTES_PER_LINE, w, h);
}

void ega_BlitAndWait(byte *pixels, uint16 pw, uint16 w, uint16 h, byte *screen, uint16 ofs) {
	ega_Blit(pixels, pw, w, h, screen, ofs);
	// waitVBlank is defined in cga.cpp and calls pollInput + delayMillis
	pollInput();
	g_system->delayMillis(10);
}

void ega_Fill(byte pixel, uint16 w, uint16 h, byte *screen, uint16 ofs) {
	uint16 oofs = ofs;
	debug(1, "ega_Fill: color=%d w=%d h=%d ofs=%d (x=%d y=%d)", pixel, w, h, ofs, ofs % EGA_BYTES_PER_LINE, ofs / EGA_BYTES_PER_LINE);
	for (uint16 y = 0; y < h; y++) {
		memset(screen + ofs, pixel, w);
		ofs += EGA_BYTES_PER_LINE;
	}

	if (screen == ega_screen)
		ega_blitToScreen(oofs % EGA_BYTES_PER_LINE, oofs / EGA_BYTES_PER_LINE, w, h);
}

void ega_FillAndWait(byte pixel, uint16 w, uint16 h, byte *screen, uint16 ofs) {
	ega_Fill(pixel, w, h, screen, ofs);
	pollInput();
	g_system->delayMillis(10);
}

void ega_BlitFromBackBuffer(byte w, byte h, byte *screen, uint16 ofs) {
	ega_CopyScreenBlock(ega_backbuffer, w, h, screen, ofs);
}

// ---------------------------------------------------------------------------
// Sprite blitters
// EGA sprites are stored as decoded CLUT8: 1 byte per pixel, index 0 = transparent
// ---------------------------------------------------------------------------

void ega_BlitSprite(byte *pixels, int16 pw, uint16 w, uint16 h, byte *screen, uint16 ofs) {
	uint16 oofs = ofs;
	for (uint16 y = 0; y < h; y++) {
		for (uint16 x = 0; x < w; x++) {
			if (pixels[x] != 0)
				screen[ofs + x] = pixels[x];
		}
		pixels += pw;
		ofs += EGA_BYTES_PER_LINE;
	}

	if (screen == ega_screen)
		ega_blitToScreen(oofs % EGA_BYTES_PER_LINE, oofs / EGA_BYTES_PER_LINE, w, h);
}

void ega_BlitSpriteFlip(byte *pixels, int16 pw, uint16 w, uint16 h, byte *screen, uint16 ofs) {
	uint16 oofs = ofs;
	for (uint16 y = 0; y < h; y++) {
		for (uint16 x = 0; x < w; x++) {
			if (pixels[w - 1 - x] != 0)
				screen[ofs + x] = pixels[w - 1 - x];
		}
		pixels += pw;
		ofs += EGA_BYTES_PER_LINE;
	}

	if (screen == ega_screen)
		ega_blitToScreen(oofs % EGA_BYTES_PER_LINE, oofs / EGA_BYTES_PER_LINE, w, h);
}

void ega_BlitSpriteBak(byte *pixels, int16 pw, uint16 w, uint16 h, byte *screen, uint16 ofs, byte *backup, byte mask) {
	uint16 oofs = ofs;
	for (uint16 y = 0; y < h; y++) {
		for (uint16 x = 0; x < w; x++) {
			*backup++ = screen[ofs + x];
			if (pixels[x] != 0)
				screen[ofs + x] = pixels[x] & mask;
		}
		pixels += pw;
		ofs += EGA_BYTES_PER_LINE;
	}

	if (screen == ega_screen)
		ega_blitToScreen(oofs % EGA_BYTES_PER_LINE, oofs / EGA_BYTES_PER_LINE, w, h);
}

void ega_BlitScratchBackSprite(uint16 sprofs, uint16 w, uint16 h, byte *screen, uint16 ofs) {
	byte *pixels = scratch_mem2 + 2 + sprofs;
	uint16 oofs = ofs;
	for (uint16 y = 0; y < h; y++) {
		for (uint16 x = 0; x < w; x++) {
			// In CGA the scratch sprite has (mask, pixel) pairs per pixel-byte.
			// In EGA mode the scratch sprite is plain CLUT8; use ega_backbuffer for transparency.
			byte p = pixels[x];
			screen[ofs + x] = (p != 0) ? p : ega_backbuffer[ofs + x];
		}
		pixels += w;
		ofs += EGA_BYTES_PER_LINE;
	}

	if (screen == ega_screen)
		ega_blitToScreen(oofs % EGA_BYTES_PER_LINE, oofs / EGA_BYTES_PER_LINE, w, h);
}

// ---------------------------------------------------------------------------
// Draw primitives
// ---------------------------------------------------------------------------

void ega_DrawVLine(uint16 x, uint16 y, uint16 l, byte color, byte *target) {
	uint16 ofs = ega_CalcXY(x, y);
	for (uint16 i = 0; i < l; i++) {
		target[ofs] = color;
		ofs += EGA_BYTES_PER_LINE;
	}

	if (target == ega_screen)
		ega_blitToScreen(x, y, 1, l);
}

void ega_DrawHLine(uint16 x, uint16 y, uint16 l, byte color, byte *target) {
	uint16 ofs = ega_CalcXY(x, y);
	memset(target + ofs, color, l);

	if (target == ega_screen)
		ega_blitToScreen(x, y, l, 1);
}

uint16 ega_DrawHLineWithEnds(uint16 bmask, uint16 bpix, byte color, uint16 l, byte *target, uint16 ofs) {
	// bmask / bpix encode end-cap bytes in CGA 2bpp packed format (high byte = left, low byte = right).
	// color is a CGA 2bpp packed fill byte. l is inner segment length in CGA bytes.
	// Expand everything to CLUT8 EGA pixels: each CGA byte → 4 pixels.
	uint16 oofs = ofs;

	// Left end-cap: expand bpix high byte (4 pixels), applying bmask
	byte leftPix = (bpix >> 8) & 0xFF;
	byte leftMask = (bmask >> 8) & 0xFF;
	for (int p = 3; p >= 0; p--) {
		byte m = (leftMask >> (p * 2)) & 0x03;
		byte px = (leftPix >> (p * 2)) & 0x03;
		if (m == 0)
			target[ofs] = cga_to_ega_color[px];
		// else: leave existing pixel (masked out)
		ofs++;
	}

	// Inner segment: l CGA bytes → l*4 pixels of solid color
	byte egaColor = cga_to_ega_color[(color >> 6) & 0x03];
	memset(target + ofs, egaColor, l * 4);
	ofs += l * 4;

	// Right end-cap: expand bpix low byte (4 pixels), applying bmask
	byte rightPix = bpix & 0xFF;
	byte rightMask = bmask & 0xFF;
	for (int p = 3; p >= 0; p--) {
		byte m = (rightMask >> (p * 2)) & 0x03;
		byte px = (rightPix >> (p * 2)) & 0x03;
		if (m == 0)
			target[ofs] = cga_to_ega_color[px];
		ofs++;
	}

	uint16 totalW = (l + 2) * 4;
	if (target == ega_screen)
		ega_blitToScreen(oofs % EGA_BYTES_PER_LINE, oofs / EGA_BYTES_PER_LINE, totalW, 1);

	return oofs + EGA_BYTES_PER_LINE;
}

// CGA palette 1 high-intensity → EGA color index mapping
// CGA 0 (background) → EGA 0 (black)
// CGA 1 (bright cyan) → EGA 11 (bright cyan)
// CGA 2 (bright magenta) → EGA 13 (bright magenta)
// CGA 3 (white) → EGA 15 (white)
const byte cga_to_ega_color[] = { 0, 11, 13, 15 };

void ega_PrintChar(byte c, byte *target) {
	// carpc_data: 64 characters × fontHeight bytes, each byte encodes a 4-pixel row.
	// char_xlat_table maps each font byte to a CGA 2bpp packed byte.
	// We extract the 4 CGA 2-bit color values and map to EGA indices.
	byte *font = carpc_data + c * g_vm->_fontHeight;
	uint16 ofs  = ega_CalcXY_p(char_draw_coords_x, char_draw_coords_y);

	for (uint16 row = 0; row < (uint16)g_vm->_fontHeight; row++) {
		byte bits = char_xlat_table[*font++];
		for (int p = 3; p >= 0; p--) {
			byte col = (bits >> (p * 2)) & 0x03;
			target[ofs + (3 - p)] = cga_to_ega_color[col];
		}
		ofs += EGA_BYTES_PER_LINE;
	}

	char_draw_coords_x++;

	if (target == ega_screen)
		ega_blitToScreen((char_draw_coords_x - 1) * g_vm->_fontWidth,
		                  char_draw_coords_y,
		                  g_vm->_fontWidth, g_vm->_fontHeight);
}

// ---------------------------------------------------------------------------
// Background loader (Phase 1 milestone)
// Reads a 32000-byte 4-plane EGA file and unpacks it into ega_backbuffer.
// Format: 4 planes × 8000 bytes.
//   plane[0] = bits 0 of each pixel (8 pixels per byte, MSB first)
//   plane[1] = bits 1
//   plane[2] = bits 2
//   plane[3] = bits 3
// Resulting pixel index = bit3<<3 | bit2<<2 | bit1<<1 | bit0
// Reference: kult/kult.cpp:268-285
// ---------------------------------------------------------------------------
Graphics::Surface *ega_loadFond(const char *filename) {
	Common::File fd;
	if (!fd.open(filename)) {
		warning("ega_loadFond: cannot open %s", filename);
		return nullptr;
	}

	static byte planarData[EGA_PLANAR_SIZE * 4];
	fd.read(planarData, sizeof(planarData));

	byte *dst = ega_backbuffer;
	for (uint i = 0; i < EGA_PLANAR_SIZE; i++) {
		for (int bit = 7; bit >= 0; bit--) {
			byte px =
			    (((planarData[i]                   >> bit) & 1)     ) |
			    (((planarData[i + EGA_PLANAR_SIZE]  >> bit) & 1) << 1) |
			    (((planarData[i + EGA_PLANAR_SIZE*2]>> bit) & 1) << 2) |
			    (((planarData[i + EGA_PLANAR_SIZE*3]>> bit) & 1) << 3);
			*dst++ = px;
		}
	}

	// Save a clean copy for background restoration
	memcpy(ega_fond_clean, ega_backbuffer, EGA_SCREEN_SIZE);

	// Return a Graphics::Surface that wraps ega_backbuffer so the existing
	// loadFond() caller chain (which expects a Surface*) stays happy.
	Graphics::Surface *surface = new Graphics::Surface();
	surface->init(EGA_WIDTH, EGA_HEIGHT,
	              EGA_BYTES_PER_LINE,
	              ega_backbuffer,
	              Graphics::PixelFormat::createFormatCLUT8());
	return surface;
}

// Clean copy of decoded FOND.EGA for room background restoration
byte ega_fond_clean[EGA_SCREEN_SIZE];

void ega_drawBackground(byte *target) {
	// In EGA mode, the FOND.EGA contains the pre-rendered background.
	// Restore the entire frame from the saved clean copy.
	memcpy(target, ega_fond_clean, EGA_SCREEN_SIZE);
}

} // End of namespace Chamber
