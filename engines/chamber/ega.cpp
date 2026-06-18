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
#include "graphics/palette.h"
#include "graphics/paletteman.h"
#include "graphics/surface.h"

#include "chamber/chamber.h"
#include "chamber/common.h"
#include "chamber/cga.h"
#include "chamber/ega.h"
#include "chamber/input.h"
#include "chamber/resdata.h"
#include "chamber/renderer.h"
#include "chamber/ega_resource.h"
#include "chamber/room.h"

namespace Chamber {

byte *ega_backbuffer = backbuffer;

const byte cga_to_ega_color[] = { 0, 11, 13, 15 };

// ---------------------------------------------------------------------------
// Memory / block blit helpers
// w is in *bytes* == *pixels* in EGA mode (1 byte per pixel)
// ---------------------------------------------------------------------------

static void ega_CopyScreenBlock(byte *source, uint16 w, uint16 h, byte *target, uint16 ofs) {
	uint16 oofs = ofs;
	for (uint16 y = 0; y < h; y++) {
		memcpy(target + ofs, source + ofs, w);
		ofs += EGA_BYTES_PER_LINE;
	}

	if (target == SCREENBUFFER)
		g_vm->_renderer->blitToScreen((int16)(oofs % EGA_BYTES_PER_LINE), (int16)(oofs / EGA_BYTES_PER_LINE), (int16)w, (int16)h);
}

byte ega_fond_clean[EGA_SCREEN_SIZE];

void ega_drawBackground(byte *target) {
	memcpy(target, ega_fond_clean, EGA_SCREEN_SIZE);
	if (target == SCREENBUFFER)
		g_vm->_renderer->blitToScreen(0, 0, EGA_WIDTH, EGA_HEIGHT);
}

#define EGA_WIPE_BLOCK 20

static int s_wipeBlocks;

/*Reveal one clean-background block onto the screen; flush+throttle every few blocks*/
static void egaRevealBlock(int bx, int by) {
	int w = (bx + EGA_WIPE_BLOCK > EGA_WIDTH) ? (EGA_WIDTH - bx) : EGA_WIPE_BLOCK;
	int h = (by + EGA_WIPE_BLOCK > EGA_HEIGHT) ? (EGA_HEIGHT - by) : EGA_WIPE_BLOCK;
	if (w <= 0 || h <= 0)
		return;
	for (int i = 0; i < h; i++)
		memcpy(SCREENBUFFER + (by + i) * EGA_BYTES_PER_LINE + bx,
		       ega_fond_clean + (by + i) * EGA_BYTES_PER_LINE + bx, w);
	if (++s_wipeBlocks % 3 == 0) {
		g_vm->_renderer->blitToScreen(0, 0, EGA_WIDTH, EGA_HEIGHT);
		waitVBlank();
	}
}

void ega_zoneRevealWipe(void) {
	// Original zone-change transition for The Ring / passages: first the empty
	// background pattern spirals in over the old room (no room underneath), then
	// the new room is shown. Here we spiral in the clean background, then blit the
	// new room (already composited in the backbuffer) on top.
	const int B = EGA_WIPE_BLOCK;
	int left = 0, top = 0, right = EGA_WIDTH, bottom = EGA_HEIGHT;
	s_wipeBlocks = 0;

	while (left < right && top < bottom) {
		for (int x = left; x < right; x += B)        /*top row, L->R*/
			egaRevealBlock(x, top);
		top += B;
		for (int y = top; y < bottom; y += B)        /*right col, T->B*/
			egaRevealBlock(right - B, y);
		right -= B;
		if (top < bottom) {
			for (int x = right - B; x >= left; x -= B)   /*bottom row, R->L*/
				egaRevealBlock(x, bottom - B);
			bottom -= B;
		}
		if (left < right) {
			for (int y = bottom - B; y >= top; y -= B)   /*left col, B->T*/
				egaRevealBlock(left, y);
			left += B;
		}
	}

	// make sure the whole play area is the clean background (no room), then hold
	// briefly so the empty background is visible before the room appears
	memcpy(SCREENBUFFER, ega_fond_clean, EGA_SCREEN_SIZE);
	g_vm->_renderer->blitToScreen(0, 0, EGA_WIDTH, EGA_HEIGHT);
	for (int i = 0; i < 8; i++)
		waitVBlank();

	// now show the new room (already composited in the backbuffer)
	memcpy(SCREENBUFFER, ega_backbuffer, EGA_SCREEN_SIZE);
	g_vm->_renderer->blitToScreen(0, 0, EGA_WIDTH, EGA_HEIGHT);
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

static void setGameEGAPalette() {
	Graphics::Palette pal = Graphics::Palette::createEGAPalette();
	byte palData[16 * 3];
	memcpy(palData, pal.data(), sizeof(palData));
	/* This game remaps EGA color 8 to black (not standard dark gray) */
	palData[8 * 3] = palData[8 * 3 + 1] = palData[8 * 3 + 2] = 0;
	g_system->getPaletteManager()->setPalette(palData, 0, 16);
}

void EGARenderer::switchToGraphicsMode() {
	setGameEGAPalette();
}

void EGARenderer::colorSelect(byte /*csel*/) {
	setGameEGAPalette();
}

void EGARenderer::blitToScreen(int16 x, int16 y, int16 w, int16 h) {
	if (x < 0) { w += x; x = 0; }
	if (y < 0) { h += y; y = 0; }
	if (x + w > EGA_WIDTH)  w = EGA_WIDTH  - x;
	if (y + h > EGA_HEIGHT) h = EGA_HEIGHT - y;
	if (w <= 0 || h <= 0)
		return;
	g_system->copyRectToScreen(SCREENBUFFER + y * EGA_BYTES_PER_LINE + x, EGA_BYTES_PER_LINE, x, y, w, h);
	g_system->updateScreen();
}

void EGARenderer::blitToScreen(int16 ofs, int16 w, int16 h) {
	blitToScreen((int16)(ofs % EGA_BYTES_PER_LINE), (int16)(ofs / EGA_BYTES_PER_LINE), w, h);
}

uint16 EGARenderer::calcXY(uint16 x, uint16 y) {
	return y * EGA_BYTES_PER_LINE + x;
}

uint16 EGARenderer::calcXY_p(uint16 x, uint16 y) {
	return y * EGA_BYTES_PER_LINE + x * 4;
}

void EGARenderer::backBufferToRealFull() {
	memcpy(SCREENBUFFER, ega_backbuffer, EGA_SCREEN_SIZE);
	blitToScreen(0, 0, EGA_WIDTH, EGA_HEIGHT);
}

void EGARenderer::realBufferToBackFull() {
	memcpy(ega_backbuffer, SCREENBUFFER, EGA_SCREEN_SIZE);
}

void EGARenderer::swapRealBackBuffer() {
	static byte tmp[EGA_SCREEN_SIZE];
	memcpy(tmp,            SCREENBUFFER,     EGA_SCREEN_SIZE);
	memcpy(SCREENBUFFER,     ega_backbuffer, EGA_SCREEN_SIZE);
	memcpy(ega_backbuffer, tmp,            EGA_SCREEN_SIZE);
	blitToScreen(0, 0, EGA_WIDTH, EGA_HEIGHT);
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

	if (screen == SCREENBUFFER)
		blitToScreen(oofs % EGA_BYTES_PER_LINE, oofs / EGA_BYTES_PER_LINE, pw, h);
}

byte *EGARenderer::backupImage(byte *screen, uint16 ofs, uint16 w, uint16 h, byte *buffer) {
	w = w * 4;
	*(byte *)(buffer + 0) = (byte)h;
	*(byte *)(buffer + 1) = (byte)(w >> 2);
	*(uint16 *)(buffer + 2) = ofs;
	buffer += 4;
	for (uint16 y = 0; y < h; y++) {
		memcpy(buffer, screen + ofs, w);
		buffer += w;
		ofs += EGA_BYTES_PER_LINE;
	}
	return buffer;
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
	ega_CopyScreenBlock(SCREENBUFFER, w, h, ega_backbuffer, ofs);
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

	if (screen == SCREENBUFFER)
		blitToScreen(oofs % EGA_BYTES_PER_LINE, oofs / EGA_BYTES_PER_LINE, w_px, h);
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

	if (screen == SCREENBUFFER)
		blitToScreen(oofs % EGA_BYTES_PER_LINE, oofs / EGA_BYTES_PER_LINE, w_px, h);
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
		if (screen == SCREENBUFFER)
			blitToScreen(oofs % EGA_BYTES_PER_LINE, oofs / EGA_BYTES_PER_LINE, ega_w, h);
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
	if (screen == SCREENBUFFER)
		blitToScreen(oofs % EGA_BYTES_PER_LINE, oofs / EGA_BYTES_PER_LINE, w * 4, h);
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
		if (screen == SCREENBUFFER)
			blitToScreen(oofs % EGA_BYTES_PER_LINE, oofs / EGA_BYTES_PER_LINE, ega_w, h);
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
	if (screen == SCREENBUFFER)
		blitToScreen(startX, oofs / EGA_BYTES_PER_LINE, w * 4, h);
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

	if (screen == SCREENBUFFER)
		blitToScreen(oofs % EGA_BYTES_PER_LINE, oofs / EGA_BYTES_PER_LINE, w_px, h);
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

	if (screen == SCREENBUFFER)
		blitToScreen(oofs % EGA_BYTES_PER_LINE, oofs / EGA_BYTES_PER_LINE, w_px, h);
}

void EGARenderer::blitFromBackBuffer(byte w, byte h, byte *screen, uint16 ofs) {
	ega_CopyScreenBlock(ega_backbuffer, w * 4, h, screen, ofs);
}

void EGARenderer::drawVLine(uint16 x, uint16 y, uint16 l, byte color, byte *target) {
	byte egaColor = cga_to_ega_color[color & 0x03];
	uint16 ofs = calcXY(x, y);
	for (uint16 i = 0; i < l; i++) {
		target[ofs] = egaColor;
		ofs += EGA_BYTES_PER_LINE;
	}

	if (target == SCREENBUFFER)
		blitToScreen(x, y, 1, l);
}

void EGARenderer::drawHLine(uint16 x, uint16 y, uint16 l, byte color, byte *target) {
	byte egaColor = cga_to_ega_color[color & 0x03];
	uint16 ofs = calcXY(x, y);
	memset(target + ofs, egaColor, l);

	if (target == SCREENBUFFER)
		blitToScreen(x, y, l, 1);
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
	if (target == SCREENBUFFER)
		blitToScreen(oofs % EGA_BYTES_PER_LINE, oofs / EGA_BYTES_PER_LINE, totalW, 1);

	return oofs + EGA_BYTES_PER_LINE;
}

void EGARenderer::printChar(byte c, byte *target) {
	byte *font = carpc_data + c * g_vm->_fontHeight;
	uint16 ofs  = calcXY_p(char_draw_coords_x, char_draw_coords_y);

	for (uint16 row = 0; row < (uint16)g_vm->_fontHeight; row++) {
		byte bits = char_xlat_table[*font++];
		for (int p = 3; p >= 0; p--) {
			byte col = (bits >> (p * 2)) & 0x03;
			target[ofs + (3 - p)] = cga_to_ega_color[col];
		}
		ofs += EGA_BYTES_PER_LINE;
	}

	char_draw_coords_x++;

	if (target == SCREENBUFFER)
		blitToScreen((char_draw_coords_x - 1) * g_vm->_fontWidth,
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

/* EGA port of the CGA lift transitions: shift the block one line/column per
   step, fill the freed edge from source (the new image), then blit+vblank so
   the move is animated. ofs is a linear EGA offset; w is in CGA bytes, so a
   block column is 4 EGA pixels wide. */

/*Shift block n lines down, filling the freed top line from source*/
void EGARenderer::hideScreenBlockLiftToDown(uint16 n, byte *screen, byte *source, uint16 w, uint16 h, byte *target, uint16 ofs) {
	uint16 ew = w * 4;
	while (n--) {
		uint16 sofs = ofs;
		uint16 tofs = ofs + EGA_BYTES_PER_LINE;
		for (uint16 i = 0; i < h; i++) {
			memcpy(target + tofs, screen + sofs, ew);
			tofs = sofs;
			sofs -= EGA_BYTES_PER_LINE;
		}
		memcpy(target + tofs, source + tofs, ew);
		if (screen == SCREENBUFFER)
			blitToScreen(0, 0, EGA_WIDTH, EGA_HEIGHT);
		waitVBlank();
		ofs += EGA_BYTES_PER_LINE;
	}
}

/*Shift block n lines up, filling the freed bottom line from source*/
void EGARenderer::hideScreenBlockLiftToUp(uint16 n, byte *screen, byte *source, uint16 w, uint16 h, byte *target, uint16 ofs) {
	uint16 ew = w * 4;
	while (n--) {
		uint16 sofs = ofs;
		uint16 tofs = ofs - EGA_BYTES_PER_LINE;
		for (uint16 i = 0; i < h; i++) {
			memcpy(target + tofs, screen + sofs, ew);
			tofs = sofs;
			sofs += EGA_BYTES_PER_LINE;
		}
		memcpy(target + tofs, source + tofs, ew);
		if (screen == SCREENBUFFER)
			blitToScreen(0, 0, EGA_WIDTH, EGA_HEIGHT);
		waitVBlank();
		ofs -= EGA_BYTES_PER_LINE;
	}
}

/*Shift block n columns left, filling the freed right column from source*/
void EGARenderer::hideScreenBlockLiftToLeft(uint16 n, byte *screen, byte *source, uint16 w, uint16 h, byte *target, uint16 ofs) {
	uint16 ew = w * 4;
	while (n--) {
		uint16 sofs = ofs;
		for (uint16 i = 0; i < h; i++) {
			uint16 tofs = sofs - 4;
			memmove(target + tofs, screen + sofs, ew);
			memcpy(target + tofs + ew, source + tofs + ew, 4);
			sofs += EGA_BYTES_PER_LINE;
		}
		if (screen == SCREENBUFFER)
			blitToScreen(0, 0, EGA_WIDTH, EGA_HEIGHT);
		waitVBlank();
		ofs -= 4;
	}
}

/*Shift block n columns right, filling the freed left column from source*/
void EGARenderer::hideScreenBlockLiftToRight(uint16 n, byte *screen, byte *source, uint16 w, uint16 h, byte *target, uint16 ofs) {
	uint16 ew = w * 4;
	while (n--) {
		uint16 sofs = ofs;
		for (uint16 i = 0; i < h; i++) {
			uint16 tofs = sofs + 4;
			memmove(target + tofs - ew, screen + sofs - ew, ew);
			memcpy(target + tofs - ew, source + tofs - ew, 4);
			sofs += EGA_BYTES_PER_LINE;
		}
		if (screen == SCREENBUFFER)
			blitToScreen(0, 0, EGA_WIDTH, EGA_HEIGHT);
		waitVBlank();
		ofs += 4;
	}
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
	/*Caller (e.g. jaggedZoom) issues one waitVBlank/blitToScreen per cycle;
	  flushing the whole screen here turned each line into an updateScreen.*/
}

// Nearest-neighbour scale an 8-bpp source (srcW x srcH pixels) into a
// dstW x dstH pixel rect at (dstX, dstY). This is the 8-bpp equivalent of the
// CGA cga_ZoomInplace math; the CGA partial-pixel/xbase packing logic collapses
// to a plain sample-and-copy here since every EGA pixel is one byte. Every write
// is clipped to the 320x200 screen so the saucer animation can never bleed past
// the framebuffer bounds.
static void ega_scaleImage(const byte *pixels, uint16 srcW, uint16 srcH,
                           int16 dstX, int16 dstY, uint16 dstW, uint16 dstH, byte *target) {
	if (dstW == 0 || dstH == 0 || srcW == 0 || srcH == 0)
		return;

	// 8.8 fixed-point source advance per target pixel. Use srcW/dstW (not
	// (srcW-1)/dstW): at a 1:1 ratio that yields the identity map so the final
	// source column/row is preserved -- the -1 form dropped it, which clipped the
	// last pixel column (visible as the missing stroke of the "D" in THE END).
	uint32 xstep = ((uint32)srcW << 8) / dstW;
	uint32 ystep = ((uint32)srcH << 8) / dstH;

	uint32 yval = 0;
	for (uint16 ty = 0; ty < dstH; ty++, yval += ystep) {
		int16 py = dstY + ty;
		if (py < 0 || py >= EGA_HEIGHT)
			continue;
		uint16 sy = yval >> 8;
		if (sy >= srcH)
			sy = srcH - 1;
		const byte *srcRow = pixels + sy * srcW;
		byte *dstRow = target + py * EGA_BYTES_PER_LINE;
		uint32 xval = 0;
		for (uint16 tx = 0; tx < dstW; tx++, xval += xstep) {
			int16 px = dstX + tx;
			if (px < 0 || px >= EGA_WIDTH)
				continue;
			uint16 sx = xval >> 8;
			if (sx >= srcW)
				sx = srcW - 1;
			dstRow[px] = srcRow[sx];
		}
	}

	if (target == SCREENBUFFER) {
		// flush the clipped destination rect
		int16 x0 = dstX < 0 ? 0 : dstX;
		int16 y0 = dstY < 0 ? 0 : dstY;
		int16 x1 = dstX + (int16)dstW;
		int16 y1 = dstY + (int16)dstH;
		if (x1 > EGA_WIDTH)  x1 = EGA_WIDTH;
		if (y1 > EGA_HEIGHT) y1 = EGA_HEIGHT;
		if (x1 > x0 && y1 > y0)
			g_vm->_renderer->blitToScreen(x0, y0, x1 - x0, y1 - y0);
	}
}

void EGARenderer::zoomImage(byte *pixels, byte w, byte h, byte nw, byte nh, byte *target, uint16 ofs) {
	// nw is the target width in 4-pixel bytes, nh the height in rows (see CGA zoomImage).
	ega_scaleImage(pixels, w * 4, h, ofs % EGA_BYTES_PER_LINE, ofs / EGA_BYTES_PER_LINE, nw * 4, nh, target);
}

void EGARenderer::animZoomIn(byte *pixels, byte w, byte h, byte *target, uint16 ofs) {
	/* Progressive zoom-in reveal (CGA animZoomIn equivalent for EGA): draw the
	   image scaled from a small size up to full, growing out of its centre.
	   Each larger frame is centred and covers the previous one, so no per-step
	   restore is needed. zoomImage() draws and flushes the scaled rect. */
	uint16 fw = (uint16)w * 4;          /* final width in pixels */
	uint16 fh = h;                      /* final height in rows */
	uint16 ox = ofs % EGA_BYTES_PER_LINE;
	uint16 oy = ofs / EGA_BYTES_PER_LINE;
	uint16 cx = ox + fw / 2;            /* zoom centre */
	uint16 cy = oy + fh / 2;

	uint16 maxside = (fw > fh) ? fw : fh;
	uint16 nsteps = maxside / 2;
	if (nsteps < 8)
		nsteps = 8;
	if (nsteps > 64)
		nsteps = 64;

	for (uint16 s = 1; s < nsteps; s++) {
		byte nw = (byte)((uint16)w * s / nsteps);
		byte nh = (byte)((uint16)h * s / nsteps);
		if (nw < 1)
			nw = 1;
		if (nh < 1)
			nh = 1;
		uint16 tx = cx - (uint16)nw * 4 / 2;
		uint16 ty = cy - (uint16)nh / 2;
		zoomImage(pixels, w, h, nw, nh, target, ty * EGA_BYTES_PER_LINE + tx);
		waitVBlank();
	}

	/* final full-size image */
	blitAndWait(pixels, w, w, h, target, ofs);
}

void EGARenderer::zoomInplaceXY(byte *pixels, byte w, byte h, byte nw, byte nh, uint16 x, uint16 y, byte *target) {
	// AnimSaucer's only caller passes the saucer path data from SOUCO.BIN, whose
	// x/y/nw/nh are raw pixel coordinates (0..319), not 4-pixel-byte columns. So x
	// is already a pixel column here -- pass it straight through (no *4), otherwise
	// every frame after the first lands far off-screen to the right.
	ega_scaleImage(pixels, w * 4, h, x, y, nw, nh, target);
}

void EGARenderer::drawSprite(byte *sprite, byte *screen, uint16 ofs) {
	Graphics::Surface *surf = reinterpret_cast<Graphics::Surface *>(sprite);
	blitSprite((byte *)surf->getPixels(), surf->pitch, surf->w / 4, surf->h, screen, ofs);
}

void EGARenderer::drawSpriteFlip(byte *sprite, byte *screen, uint16 ofs) {
	Graphics::Surface *surf = reinterpret_cast<Graphics::Surface *>(sprite);
	blitSpriteFlip((byte *)surf->getPixels(), surf->pitch, surf->w / 4, surf->h, screen, ofs);
}

byte *EGARenderer::loadSprit(byte index) {
	return reinterpret_cast<byte *>(ega_sprit_res->getSprite(index));
}

byte *EGARenderer::loadPersSprit(byte index) {
	return reinterpret_cast<byte *>(ega_perso_res->getSprite(index));
}

void EGARenderer::backupAndShowSprite(byte index, byte x, byte y) {
	byte *sprite = loadSprit(index);
	uint16 ofs = calcXY_p(x, y);
	Graphics::Surface *surf = reinterpret_cast<Graphics::Surface *>(sprite);
	backupImage(SCREENBUFFER, ofs, surf->w / 4, surf->h, scratch_mem2);
	drawSprite(sprite, SCREENBUFFER, ofs);
}

void EGARenderer::animLiftToDown(byte *pixels, uint16 pw, uint16 w, uint16 h, byte *screen, uint16 ofs) {
	uint16 i;
	uint16 epw = pw * 4;
	pixels += epw * (h - 1);
	for (i = 1; i <= h; i++) {
		blitAndWait(pixels, pw, w, i, screen, ofs);
		pixels -= epw;
	}
}

void EGARenderer::animLiftToUp(byte *pixels, uint16 pw, uint16 w, uint16 h, byte *screen, uint16 x, uint16 y) {
	uint16 i;
	for (i = 1; i <= h; i++) {
		blitAndWait(pixels, pw, w, i, screen, calcXY_p(x, y));
		y -= 1;
	}
}

} // End of namespace Chamber
