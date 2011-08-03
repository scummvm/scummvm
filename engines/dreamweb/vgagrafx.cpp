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

#include "dreamweb/dreamweb.h"
#include "engines/util.h"
#include "graphics/surface.h"

namespace DreamGen {

uint8 *DreamGenContext::workspace() {
	push(es);
	es = data.word(kWorkspace);
	uint8 *result = es.ptr(0, 0);
	es = pop();
	return result;
}

void DreamGenContext::allocatework() {
	data.word(kWorkspace) = allocatemem(0x1000);
}

void DreamGenContext::multiget() {
	multiget(di, bx, cl, ch);
}

void DreamGenContext::multiget(uint16 x, uint16 y, uint8 w, uint8 h) {
	unsigned src = x + y * kScreenwidth;
	unsigned dst = (uint16)si;
	es = ds;
	ds = data.word(kWorkspace);
	if (y + h > 200)
		h = 200 - y;
	if (x + w > 320)
		w = 320 - x;
	//debug(1, "multiget %u,%u %ux%u -> segment: %04x->%04x", x, y, w, h, (uint16)ds, (uint16)es);
	for(unsigned l = 0; l < h; ++l) {
		uint8 *src_p = ds.ptr(src + kScreenwidth * l, w);
		uint8 *dst_p = es.ptr(dst + w * l, w);
		memcpy(dst_p, src_p, w);
	}
	si += w * h;
	di = src + kScreenwidth * h;
	cx = 0;
}

void DreamGenContext::multiput() {
	multiput(di, bx, cl, ch);
}

void DreamGenContext::multiput(uint16 x, uint16 y, uint8 w, uint8 h) {
	unsigned src = (uint16)si;
	unsigned dst = x + y * kScreenwidth;
	es = data.word(kWorkspace);
	if (y + h > 200)
		h = 200 - y;
	if (x + w > 320)
		w = 320 - x;
	//debug(1, "multiput %ux%u -> segment: %04x->%04x", w, h, (uint16)ds, (uint16)es);
	for(unsigned l = 0; l < h; ++l) {
		uint8 *src_p = ds.ptr(src + w * l, w);
		uint8 *dst_p = es.ptr(dst + kScreenwidth * l, w);
		memcpy(dst_p, src_p, w);
	}
	si += w * h;
	di = dst + kScreenwidth * h;
	cx = 0;
}

void DreamGenContext::multidump(uint16 x, uint16 y, uint8 width, uint8 height) {
	ds = data.word(kWorkspace);
	unsigned offset = x + y * kScreenwidth;
	//debug(1, "multidump %ux%u(segment: %04x) -> %d,%d(address: %d)", w, h, (uint16)ds, x, y, offset);
	engine->blit(ds.ptr(offset, width * height), kScreenwidth, x, y, width, height);
}

void DreamGenContext::multidump() {
	multidump(di, bx, cl, ch);
	unsigned offset = di + bx * kScreenwidth;
	si = di = offset + ch * kScreenwidth;
	cx = 0;
}

void DreamGenContext::worktoscreen() {
	uint size = 320 * 200;
	engine->blit(workspace(), 320, 0, 0, 320, 200);
	di = si = size;
	cx = 0;
}

void DreamGenContext::printundermon() {
	engine->printUnderMonitor();
}

void DreamGenContext::cls() {
	engine->cls();
}

void DreamGenContext::frameoutnm(uint8 *dst, const uint8 *src, uint16 pitch, uint16 width, uint16 height, uint16 x, uint16 y) {
	dst += pitch * y + x;

	for (uint16 j = 0; j < height; ++j) {
		memcpy(dst, src, width);
		dst += pitch;
		src += width;
	}
}

void DreamGenContext::frameoutbh(uint8 *dst, const uint8 *src, uint16 pitch, uint16 width, uint16 height, uint16 x, uint16 y) {
	uint16 stride = pitch - width;
	dst += y * pitch + x;

	for (uint16 i = 0; i < height; ++i) {
		for (uint16 j = 0; j < width; ++j) {
			if (*dst == 0xff) {
				*dst = *src;
			}
			++src;
			++dst;
		}
		dst += stride;
	}
}

void DreamGenContext::frameoutfx(uint8 *dst, const uint8 *src, uint16 pitch, uint16 width, uint16 height, uint16 x, uint16 y) {
	uint16 stride = pitch - width;
	dst += y * pitch + x;
	dst -= width;

	for (uint16 j = 0; j < height; ++j) {
		for (uint16 i = 0; i < width; ++i) {
			uint8 pixel = src[width - i - 1];
			if (pixel)
				*dst = pixel;
			++dst;
		}
		src += width;
		dst += stride;
	}
}

void DreamGenContext::doshake() {
	uint8 &counter = data.byte(kShakecounter);
	_cmp(counter, 48);
	if (flags.z())
		return;

	_add(counter, 1);
	static const int shakeTable[] = {
		0, -2,  3, -2,  0,  2,  4, -1,
		1, -3,  3,  2,  0, -2,  3, -2,
		0,  2,  4, -1,  1, -3,  3,  2,
		0, -2,  3, -2,  0,  2,  4, -1,

		1, -3,  3,  2,  0, -2,  3, -2,
		0,  2,  4, -1,  1, -3,  3,  2,
		0, -2,  3, -2,  0,  2,  4, -1,
		1, -3,  3,  2,  0, -2,  3, -2,

		0,  2,  4, -1,  1, -3,  3,  2,
		0, -2,  3, -2,  0,  2,  4, -1,
		1, -3,  3,  2,  0, -2,  3, -2,
		0,  2,  4, -1,  1, -3,  3,  2,

		0, -2,  3, -2,  0,  2,  4, -1,
		1, -3,  3,  0,
	};
	int offset = shakeTable[counter];
	engine->setShakePos(offset >= 0 ? offset : -offset);
}

void DreamGenContext::vsync() {
	push(ax);
	push(bx);
	push(cx);
	push(dx);
	push(si);
	push(di);
	push(es);
	push(ds);
	engine->waitForVSync();
	ds = pop();
	es = pop();
	di = pop();
	si = pop();
	dx = pop();
	cx = pop();
	bx = pop();
	ax = pop();
}

void DreamGenContext::setmode() {
	vsync();
	initGraphics(320, 200, false);
}

static Common::String getFilename(Context &context) {
	uint16 name_ptr = context.dx;
	Common::String name;
	uint8 c;
	while((c = context.cs.byte(name_ptr++)) != 0)
		name += (char)c;
	return name;
}

void DreamGenContext::showpcx() {
	Common::String name = getFilename(*this);
	Common::File pcxFile;

	if (!pcxFile.open(name)) {
		warning("showpcx: Could not open '%s'", name.c_str());
		return;
	}

	uint8 *maingamepal;
	int i, j;

	// Read the 16-color palette into the 'maingamepal' buffer. Note that
	// the color components have to be adjusted from 8 to 6 bits.

	pcxFile.seek(16, SEEK_SET);
	es = data.word(kBuffers);
	maingamepal = es.ptr(kMaingamepal, 768);
	pcxFile.read(maingamepal, 48);

	memset(maingamepal + 48, 0xff, 720);
	for (i = 0; i < 48; i++) {
		maingamepal[i] >>= 2;
	}

	// Decode the image data.

	Graphics::Surface *s = g_system->lockScreen();
	Common::Rect rect(640, 480);

	s->fillRect(rect, 0);
	pcxFile.seek(128, SEEK_SET);

	for (int y = 0; y < 480; y++) {
		byte *dst = (byte *)s->getBasePtr(0, y);
		int decoded = 0;

		while (decoded < 320) {
			byte col = pcxFile.readByte();
			byte len;

			if ((col & 0xc0) == 0xc0) {
				len = col & 0x3f;
				col = pcxFile.readByte();
			} else {
				len = 1;
			}

			// The image uses 16 colors and is stored as four bit
			// planes, one for each bit of the color, least
			// significant bit plane first.

			for (i = 0; i < len; i++) {
				int plane = decoded / 80;
				int pos = decoded % 80;

				for (j = 0; j < 8; j++) {
					byte bit = (col >> (7 - j)) & 1;
					dst[8 * pos + j] |= (bit << plane);
				}

				decoded++;
			}
		}
	}

	g_system->unlockScreen();
	pcxFile.close();
}

void DreamGenContext::frameoutv(uint8 *dst, const uint8 *src, uint16 pitch, uint16 width, uint16 height, uint16 x, uint16 y) {
	// NB : These resilience checks were not in the original engine, but did they result in undefined behaviour
	// or was something broken during porting to C++?
	assert(pitch == 320);

	if(x >= 320)
		return;
	if(y >= 200)
		return;
	if(x + width > 320) {
		width = 320 - x;
	}
	if(y + height > 200) {
		height = 200 - y;
	}

	uint16 stride = pitch - width;
	dst += pitch * y + x;

	for (uint16 j = 0; j < height; ++j) {
		for (uint16 i = 0; i < width; ++i) {
			uint8 pixel = *src++;
			if (pixel)
				*dst = pixel;
			++dst;
		}
		dst += stride;
	}
}

void DreamGenContext::showframe(const void *frameData, uint16 x, uint16 y, uint16 frameNumber, uint8 effectsFlag, uint8 *width, uint8 *height) {
	const Frame *frame = ((const Frame *)frameData) + frameNumber;
	if ((frame->width == 0) && (frame->height == 0)) {
		*width = 0;
		*height = 0;
		return;
	}

//notblankshow:
	if ((effectsFlag & 128) == 0) {
		x += frame->x;
		y += frame->y;
	}
//skipoffsets:

	*width = frame->width;
	*height = frame->height;
	const uint8 *pSrc = ((const uint8 *)frameData) + frame->ptr() + 2080;

	if (effectsFlag) {
		if (effectsFlag & 128) { //centred
			x -= *width / 2;
			y -= *height / 2;
		}
		if (effectsFlag & 64) { //diffdest
			frameoutfx(es.ptr(0, dx * *height), pSrc, dx, *width, *height, x, y);
			return;
		}
		if (effectsFlag & 8) { //printlist
			/*
			push(ax);
			al = x - data.word(kMapadx);
			ah = y - data.word(kMapady);
			//addtoprintlist(); // NB: Commented in the original asm
			ax = pop();
			*/
		}
		if (effectsFlag & 4) { //flippedx
			frameoutfx(workspace(), pSrc, 320, *width, *height, x, y);
			return;
		}
		if (effectsFlag & 2) { //nomask
			frameoutnm(workspace(), pSrc, 320, *width, *height, x, y);
			return;
		}
		if (effectsFlag & 32) {
			frameoutbh(workspace(), pSrc, 320, *width, *height, x, y);
			return;
		}
	}
//noeffects:
	frameoutv(workspace(), pSrc, 320, *width, *height, x, y);
	return;
}

void DreamGenContext::showframe() {
	uint8 width, height;
	showframe(ds.ptr(0, 0), di, bx, ax & 0x1ff, ah & 0xfe, &width, &height);
	cl = width;
	ch = height;
}

void DreamGenContext::clearwork() {
	memset(workspace(), 0, 320*200);
}

} /*namespace dreamgen */

