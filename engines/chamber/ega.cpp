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
#include "chamber/renderer.h"
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

const byte cga_to_ega_color[] = { 0, 11, 13, 15 };

// ---------------------------------------------------------------------------
// Screen management
// ---------------------------------------------------------------------------

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

void ega_BlitFromBackBuffer(byte w, byte h, byte *screen, uint16 ofs) {
	ega_CopyScreenBlock(ega_backbuffer, w, h, screen, ofs);
}

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

byte ega_fond_clean[EGA_SCREEN_SIZE];

void ega_drawBackground(byte *target) {
	memcpy(target, ega_fond_clean, EGA_SCREEN_SIZE);
}

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
			    (((planarData[i]                    >> bit) & 1)     ) |
			    (((planarData[i + EGA_PLANAR_SIZE]   >> bit) & 1) << 1) |
			    (((planarData[i + EGA_PLANAR_SIZE*2] >> bit) & 1) << 2) |
			    (((planarData[i + EGA_PLANAR_SIZE*3] >> bit) & 1) << 3);
			*dst++ = px;
		}
	}

	memcpy(ega_fond_clean, ega_backbuffer, EGA_SCREEN_SIZE);

	Graphics::Surface *surface = new Graphics::Surface();
	surface->init(EGA_WIDTH, EGA_HEIGHT,
	              EGA_BYTES_PER_LINE,
	              ega_backbuffer,
	              Graphics::PixelFormat::createFormatCLUT8());
	return surface;
}

// ===========================================================================
// EGARenderer method implementations
// Implementations live directly here; no delegation to removed ega_* helpers.
// ===========================================================================

void EGARenderer::switchToGraphicsMode() {
	g_system->getPaletteManager()->setPalette(EGA_PALETTE, 0, 16);
}

void EGARenderer::colorSelect(byte /*csel*/) {
	g_system->getPaletteManager()->setPalette(EGA_PALETTE, 0, 16);
}

void EGARenderer::blitToScreen(int16 x, int16 y, int16 w, int16 h) {
	ega_blitToScreen(x, y, w, h);
}

void EGARenderer::blitToScreen(int16 ofs, int16 w, int16 h) {
	int16 dy = ofs / EGA_BYTES_PER_LINE;
	int16 dx = ofs % EGA_BYTES_PER_LINE;
	ega_blitToScreen(dx, dy, w, h);
}

uint16 EGARenderer::calcXY(uint16 x, uint16 y) {
	return y * EGA_BYTES_PER_LINE + x;
}

uint16 EGARenderer::calcXY_p(uint16 x, uint16 y) {
	return y * EGA_BYTES_PER_LINE + x * 4;
}

void EGARenderer::backBufferToRealFull() {
	memcpy(ega_screen, ega_backbuffer, EGA_SCREEN_SIZE);
	ega_blitToScreen(0, 0, EGA_WIDTH, EGA_HEIGHT);
}

void EGARenderer::realBufferToBackFull() {
	memcpy(ega_backbuffer, ega_screen, EGA_SCREEN_SIZE);
}

void EGARenderer::swapRealBackBuffer() {
	static byte tmp[EGA_SCREEN_SIZE];
	memcpy(tmp,            ega_screen,     EGA_SCREEN_SIZE);
	memcpy(ega_screen,     ega_backbuffer, EGA_SCREEN_SIZE);
	memcpy(ega_backbuffer, tmp,            EGA_SCREEN_SIZE);
	ega_blitToScreen(0, 0, EGA_WIDTH, EGA_HEIGHT);
}

void EGARenderer::copyScreenBlock(byte *source, uint16 w, uint16 h, byte *target, uint16 ofs) {
	ega_CopyScreenBlock(source, w * 4, h, target, ofs);
}

void EGARenderer::swapScreenRect(byte *pixels, uint16 w, uint16 h, byte *screen, uint16 ofs) {
	byte *old = scratch_mem2;
	uint16 oofs = ofs;
	uint16 pw = w * 4;
	for (uint16 y = 0; y < h; y++) {
		for (uint16 i = 0; i < pw; i++) {
			*old++ = screen[ofs + i];
			screen[ofs + i] = *pixels++;
		}
		ofs += EGA_BYTES_PER_LINE;
	}

	if (screen == ega_screen)
		ega_blitToScreen(oofs % EGA_BYTES_PER_LINE, oofs / EGA_BYTES_PER_LINE, pw, h);
}

byte *EGARenderer::backupImage(byte *screen, uint16 ofs, uint16 w, uint16 h, byte *buffer) {
	return ega_BackupImage(screen, ofs, w * 4, h, buffer);
}

void EGARenderer::restoreImage(byte *buffer, byte *target) {
	if (!buffer)
		return;
	uint16 h   = *(byte *)(buffer + 0);
	uint16 w   = (uint16)*(byte *)(buffer + 1) * 4;
	uint16 ofs = *(uint16 *)(buffer + 2);
	buffer += 4;
	blit(buffer, w / 4, w / 4, h, target, ofs);
}

void EGARenderer::refreshImageData(byte *buffer) {
	if (!buffer)
		return;
	uint16 h   = *(byte *)(buffer + 0);
	uint16 w   = (uint16)*(byte *)(buffer + 1) * 4;
	uint16 ofs = *(uint16 *)(buffer + 2);
	ega_CopyScreenBlock(ega_screen, w, h, ega_backbuffer, ofs);
}

void EGARenderer::restoreBackupImage(byte *target) {
	restoreImage(scratch_mem2, target);
}

void EGARenderer::blit(byte *pixels, uint16 pw, uint16 w, uint16 h, byte *screen, uint16 ofs) {
	uint16 oofs = ofs;
	uint16 pw_px = pw * 4;
	uint16 w_px  = w  * 4;
	for (uint16 y = 0; y < h; y++) {
		memcpy(screen + ofs, pixels, w_px);
		pixels += pw_px;
		ofs += EGA_BYTES_PER_LINE;
	}

	if (screen == ega_screen)
		ega_blitToScreen(oofs % EGA_BYTES_PER_LINE, oofs / EGA_BYTES_PER_LINE, w_px, h);
}

void EGARenderer::blitAndWait(byte *pixels, uint16 pw, uint16 w, uint16 h, byte *screen, uint16 ofs) {
	blit(pixels, pw, w, h, screen, ofs);
	pollInput();
	g_system->delayMillis(10);
}

void EGARenderer::fill(byte pixel, uint16 w, uint16 h, byte *screen, uint16 ofs) {
	byte egaColor = cga_to_ega_color[(pixel >> 6) & 0x03];
	uint16 oofs = ofs;
	uint16 w_px = w * 4;
	for (uint16 y = 0; y < h; y++) {
		memset(screen + ofs, egaColor, w_px);
		ofs += EGA_BYTES_PER_LINE;
	}

	if (screen == ega_screen)
		ega_blitToScreen(oofs % EGA_BYTES_PER_LINE, oofs / EGA_BYTES_PER_LINE, w_px, h);
}

void EGARenderer::fillAndWait(byte pixel, uint16 w, uint16 h, byte *screen, uint16 ofs) {
	fill(pixel, w, h, screen, ofs);
	pollInput();
	g_system->delayMillis(10);
}

void EGARenderer::blitSprite(byte *pixels, int16 pw, uint16 w, uint16 h, byte *screen, uint16 ofs) {
	uint16 oofs = ofs;
	if ((pw < 0 ? -pw : pw) == (int16)(w * 4)) {
		uint16 ega_w = w * 4;
		for (uint16 y = 0; y < h; y++) {
			for (uint16 x = 0; x < ega_w; x++) {
				if (pixels[x] != 0)
					screen[ofs + x] = pixels[x];
			}
			pixels += pw;
			ofs += EGA_BYTES_PER_LINE;
		}
		if (screen == ega_screen)
			ega_blitToScreen(oofs % EGA_BYTES_PER_LINE, oofs / EGA_BYTES_PER_LINE, ega_w, h);
		return;
	}
	for (uint16 row = 0; row < h; row++) {
		for (uint16 bx = 0; bx < w; bx++) {
			byte mb = pixels[bx * 2];
			byte cb = pixels[bx * 2 + 1];
			for (int p = 3; p >= 0; p--) {
				byte m = (mb >> (p * 2)) & 0x03;
				byte c = (cb >> (p * 2)) & 0x03;
				if (m == 0)
					screen[ofs + bx * 4 + (3 - p)] = cga_to_ega_color[c];
			}
		}
		pixels += pw;
		ofs += EGA_BYTES_PER_LINE;
	}
	if (screen == ega_screen)
		ega_blitToScreen(oofs % EGA_BYTES_PER_LINE, oofs / EGA_BYTES_PER_LINE, w * 4, h);
}

void EGARenderer::blitSpriteFlip(byte *pixels, int16 pw, uint16 w, uint16 h, byte *screen, uint16 ofs) {
	uint16 oofs = ofs;
	if ((pw < 0 ? -pw : pw) == (int16)(w * 4)) {
		uint16 ega_w = w * 4;
		for (uint16 y = 0; y < h; y++) {
			for (uint16 x = 0; x < ega_w; x++) {
				if (pixels[ega_w - 1 - x] != 0)
					screen[ofs + x] = pixels[ega_w - 1 - x];
			}
			pixels += pw;
			ofs += EGA_BYTES_PER_LINE;
		}
		if (screen == ega_screen)
			ega_blitToScreen(oofs % EGA_BYTES_PER_LINE, oofs / EGA_BYTES_PER_LINE, ega_w, h);
		return;
	}
	for (uint16 row = 0; row < h; row++) {
		for (uint16 bx = 0; bx < w; bx++) {
			byte mb = pixels[bx * 2];
			byte cb = pixels[bx * 2 + 1];
			for (int p = 3; p >= 0; p--) {
				byte m = (mb >> (p * 2)) & 0x03;
				byte c = (cb >> (p * 2)) & 0x03;
				if (m == 0)
					screen[ofs - (bx * 4 + (3 - p))] = cga_to_ega_color[c];
			}
		}
		pixels += pw;
		ofs += EGA_BYTES_PER_LINE;
	}
	uint16 startX = (oofs % EGA_BYTES_PER_LINE) - (w * 4 - 1);
	if (screen == ega_screen)
		ega_blitToScreen(startX, oofs / EGA_BYTES_PER_LINE, w * 4, h);
}

void EGARenderer::blitSpriteBak(byte *pixels, int16 pw, uint16 w, uint16 h, byte *screen, uint16 ofs, byte *backup, byte mask) {
	uint16 oofs = ofs;
	uint16 w_px = w * 4;
	for (uint16 y = 0; y < h; y++) {
		for (uint16 x = 0; x < w_px; x++) {
			*backup++ = screen[ofs + x];
			if (pixels[x] != 0)
				screen[ofs + x] = pixels[x] & mask;
		}
		pixels += pw * 4;
		ofs += EGA_BYTES_PER_LINE;
	}

	if (screen == ega_screen)
		ega_blitToScreen(oofs % EGA_BYTES_PER_LINE, oofs / EGA_BYTES_PER_LINE, w_px, h);
}

void EGARenderer::blitScratchBackSprite(uint16 sprofs, uint16 w, uint16 h, byte *screen, uint16 ofs) {
	byte *pixels = scratch_mem2 + 2 + sprofs * 2;
	uint16 w_px  = w * 4;
	uint16 oofs  = ofs;
	for (uint16 y = 0; y < h; y++) {
		for (uint16 x = 0; x < w_px; x++) {
			byte p = pixels[x];
			screen[ofs + x] = (p != 0) ? p : ega_backbuffer[ofs + x];
		}
		pixels += w_px;
		ofs += EGA_BYTES_PER_LINE;
	}

	if (screen == ega_screen)
		ega_blitToScreen(oofs % EGA_BYTES_PER_LINE, oofs / EGA_BYTES_PER_LINE, w_px, h);
}

void EGARenderer::blitFromBackBuffer(byte w, byte h, byte *screen, uint16 ofs) {
	ega_BlitFromBackBuffer(w * 4, h, screen, ofs);
}

void EGARenderer::drawVLine(uint16 x, uint16 y, uint16 l, byte color, byte *target) {
	byte egaColor = cga_to_ega_color[color & 0x03];
	uint16 ofs = ega_CalcXY(x, y);
	for (uint16 i = 0; i < l; i++) {
		target[ofs] = egaColor;
		ofs += EGA_BYTES_PER_LINE;
	}

	if (target == ega_screen)
		ega_blitToScreen(x, y, 1, l);
}

void EGARenderer::drawHLine(uint16 x, uint16 y, uint16 l, byte color, byte *target) {
	byte egaColor = cga_to_ega_color[color & 0x03];
	uint16 ofs = ega_CalcXY(x, y);
	memset(target + ofs, egaColor, l);

	if (target == ega_screen)
		ega_blitToScreen(x, y, l, 1);
}

uint16 EGARenderer::drawHLineWithEnds(uint16 bmask, uint16 bpix, byte color, uint16 l, byte *target, uint16 ofs) {
	uint16 oofs = ofs;

	byte leftPix  = (bpix  >> 8) & 0xFF;
	byte leftMask = (bmask >> 8) & 0xFF;
	for (int p = 3; p >= 0; p--) {
		byte m  = (leftMask >> (p * 2)) & 0x03;
		byte px = (leftPix  >> (p * 2)) & 0x03;
		if (m == 0)
			target[ofs] = cga_to_ega_color[px];
		ofs++;
	}

	byte egaColor = cga_to_ega_color[(color >> 6) & 0x03];
	memset(target + ofs, egaColor, l * 4);
	ofs += l * 4;

	byte rightPix  = bpix  & 0xFF;
	byte rightMask = bmask & 0xFF;
	for (int p = 3; p >= 0; p--) {
		byte m  = (rightMask >> (p * 2)) & 0x03;
		byte px = (rightPix  >> (p * 2)) & 0x03;
		if (m == 0)
			target[ofs] = cga_to_ega_color[px];
		ofs++;
	}

	uint16 totalW = (l + 2) * 4;
	if (target == ega_screen)
		ega_blitToScreen(oofs % EGA_BYTES_PER_LINE, oofs / EGA_BYTES_PER_LINE, totalW, 1);

	return oofs + EGA_BYTES_PER_LINE;
}

void EGARenderer::printChar(byte c, byte *target) {
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

void EGARenderer::animLiftToLeft(uint16 n, byte *pixels, uint16 pw, uint16 w, uint16 h, byte *screen, uint16 ofs) {
	for (uint16 i = 0; i < n; i++) {
		blitAndWait(pixels, pw, w + i, h, screen, ofs);
		ofs -= 4;
	}
}

void EGARenderer::animLiftToRight(uint16 n, byte *pixels, uint16 pw, uint16 w, uint16 h, byte *screen, uint16 ofs) {
	for (uint16 i = 0; i < n; i++) {
		blitAndWait(pixels, pw, w + i, h, screen, ofs);
		pixels -= 4;
	}
}

void EGARenderer::hideScreenBlockLiftToDown(uint16 n, byte *screen, byte *source, uint16 w, uint16 h, byte *target, uint16 ofs) {
	ega_CopyScreenBlock(source, w * 4, h + n, target, ofs);
}

void EGARenderer::hideScreenBlockLiftToUp(uint16 n, byte *screen, byte *source, uint16 w, uint16 h, byte *target, uint16 ofs) {
	ega_CopyScreenBlock(source, w * 4, h + n, target, ofs);
}

void EGARenderer::hideScreenBlockLiftToLeft(uint16 n, byte *screen, byte *source, uint16 w, uint16 h, byte *target, uint16 ofs) {
	ega_CopyScreenBlock(source, (w + n) * 4, h, target, ofs - n * 4);
}

void EGARenderer::hideScreenBlockLiftToRight(uint16 n, byte *screen, byte *source, uint16 w, uint16 h, byte *target, uint16 ofs) {
	ega_CopyScreenBlock(source, (w + n) * 4, h, target, ofs - w * 4);
}

void EGARenderer::hideShatterFall(byte *screen, byte *source, uint16 w, uint16 h, byte *target, uint16 ofs) {
	ega_CopyScreenBlock(source, w * 4, h, target, ofs);
}

void EGARenderer::traceLine(uint16 sx, uint16 ex, uint16 sy, uint16 ey, byte *source, byte *target) {
	int16 x0 = sx, y0 = sy, x1 = ex, y1 = ey;
	int16 ddx = (x1 >= x0) ? 1 : -1;
	int16 ddy = (y1 >= y0) ? 1 : -1;
	int16 abw = (x1 >= x0) ? (x1 - x0) : (x0 - x1);
	int16 abh = (y1 >= y0) ? (y1 - y0) : (y0 - y1);
	int16 err = abw - abh;
	for (;;) {
		uint16 o = y0 * EGA_BYTES_PER_LINE + x0;
		if (o < EGA_SCREEN_SIZE)
			target[o] = source[o];
		if (x0 == x1 && y0 == y1) break;
		int16 e2 = err * 2;
		if (e2 > -abh) { err -= abh; x0 += ddx; }
		if (e2 <  abw) { err += abw; y0 += ddy; }
	}
	if (target == ega_screen)
		ega_blitToScreen(0, 0, EGA_WIDTH, EGA_HEIGHT);
}

void EGARenderer::zoomImage(byte *pixels, byte w, byte h, byte /*nw*/, byte /*nh*/, byte *target, uint16 ofs) {
	blitAndWait(pixels, w, w, h, target, ofs);
}

void EGARenderer::animZoomIn(byte *pixels, byte w, byte h, byte *target, uint16 ofs) {
	blitAndWait(pixels, w, w, h, target, ofs);
}

void EGARenderer::zoomInplaceXY(byte *pixels, byte w, byte h, byte /*nw*/, byte /*nh*/, uint16 x, uint16 y, byte *target) {
	blit(pixels, w, w, h, target, ega_CalcXY_p(x, y));
}

} // End of namespace Chamber
