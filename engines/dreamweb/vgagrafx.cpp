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
	uint8 *result = segRef(data.word(kWorkspace)).ptr(0, 0);
	return result;
}

void DreamGenContext::allocatework() {
	data.word(kWorkspace) = allocatemem(0x1000);
}

void DreamGenContext::multiget() {
	multiget(ds.ptr(si, 0), di, bx, cl, ch);
	si += cl * ch;
	di += bx * kScreenwidth + kScreenwidth * ch;
	cx = 0;
}

void DreamGenContext::multiget(uint8 *dst, uint16 x, uint16 y, uint8 w, uint8 h) {
	assert(x < 320);
	assert(y < 200);
	const uint8 *src = workspace() + x + y * kScreenwidth;
	if (y + h > 200)
		h = 200 - y;
	if (x + w > 320)
		w = 320 - x;
	//debug(1, "multiget %u,%u %ux%u -> segment: %04x->%04x", x, y, w, h, (uint16)ds, (uint16)es);
	for(unsigned l = 0; l < h; ++l) {
		const uint8 *src_p = src + kScreenwidth * l;
		uint8 *dst_p = dst + w * l;
		memcpy(dst_p, src_p, w);
	}
}

void DreamGenContext::multiput() {
	multiput(ds.ptr(si, 0), di, bx, cl, ch);
	si += cl * ch;
	di += bx * kScreenwidth + kScreenwidth * ch;
	cx = 0;
}

void DreamGenContext::multiput(const uint8 *src, uint16 x, uint16 y, uint8 w, uint8 h) {
	assert(x < 320);
	assert(y < 200);
	uint8 *dst = workspace() + x + y * kScreenwidth;
	if (y + h > 200)
		h = 200 - y;
	if (x + w > 320)
		w = 320 - x;
	//debug(1, "multiput %ux%u -> segment: %04x->%04x", w, h, (uint16)ds, (uint16)es);
	for(unsigned l = 0; l < h; ++l) {
		const uint8 *src_p = src + w * l;
		uint8 *dst_p = dst + kScreenwidth * l;
		memcpy(dst_p, src_p, w);
	}
}

void DreamGenContext::multidump(uint16 x, uint16 y, uint8 width, uint8 height) {
	unsigned offset = x + y * kScreenwidth;
	//debug(1, "multidump %ux%u(segment: %04x) -> %d,%d(address: %d)", w, h, (uint16)ds, x, y, offset);
	engine->blit(workspace() + offset, kScreenwidth, x, y, width, height);
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
	if (counter == 48)
		return;

	++counter;
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

void DreamGenContext::frameoutv(uint8 *dst, const uint8 *src, uint16 pitch, uint16 width, uint16 height, int16 x, int16 y) {
	// NB : These resilience checks were not in the original engine, but did they result in undefined behaviour
	// or was something broken during porting to C++?
	assert(pitch == 320);

	if(x < 0) {
		assert(width >= -x);
		width -= -x;
		src += -x;
		x = 0;
	}
	if(y < 0) {
		assert(height >= -y);
		height -= -y;
		src += (-y) * width;
		y = 0;
	}
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

void DreamGenContext::showframe(const Frame *frameData, uint16 x, uint16 y, uint16 frameNumber, uint8 effectsFlag) {
	uint8 width, height;
	showframe(frameData, x, y, frameNumber, effectsFlag, &width, &height);
}

void DreamGenContext::showframe(const Frame *frameData, uint16 x, uint16 y, uint16 frameNumber, uint8 effectsFlag, uint8 *width, uint8 *height) {
	const Frame *frame = frameData + frameNumber;
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
	showframe((Frame *)ds.ptr(0, 0), di, bx, ax & 0x1ff, ah & 0xfe, &width, &height);
	cl = width;
	ch = height;
}

void DreamGenContext::clearwork() {
	memset(workspace(), 0, 320*200);
}

void DreamGenContext::zoom() {
	if (data.word(kWatchingtime) != 0)
		return;
	if (data.byte(kZoomon) != 1)
		return;
	if (data.byte(kCommandtype) >= 199) {
		putunderzoom();
		return;
	}
	uint16 srcOffset = (data.word(kOldpointery) - 9) * 320 + (data.word(kOldpointerx) - 11);
	uint16 dstOffset = (kZoomy + 4) * 320 + (kZoomx + 5);
	const uint8 *src = workspace() + srcOffset;
	uint8 *dst = workspace() + dstOffset;
	for(size_t i=0; i<20; ++i) {
		for(size_t j=0; j<23; ++j) {
			uint8 v = src[j];
			dst[2*j+0] = v;
			dst[2*j+1] = v; 
			dst[2*j+320] = v;
			dst[2*j+321] = v; 
		}
		src += 320;
		dst += 320*2;
	}
	crosshair();
	data.byte(kDidzoom) = 1;
}

void DreamGenContext::paneltomap() {
	multiget(segRef(data.word(kMapstore)).ptr(0, 0), data.word(kMapxstart) + data.word(kMapadx), data.word(kMapystart) + data.word(kMapady), data.byte(kMapxsize), data.byte(kMapysize));
}

void DreamGenContext::maptopanel() {
	multiput(segRef(data.word(kMapstore)).ptr(0, 0), data.word(kMapxstart) + data.word(kMapadx), data.word(kMapystart) + data.word(kMapady), data.byte(kMapxsize), data.byte(kMapysize));
}

void DreamGenContext::dumpmap() {
	multidump(data.word(kMapxstart) + data.word(kMapadx), data.word(kMapystart) + data.word(kMapady), data.byte(kMapxsize), data.byte(kMapysize));
}

void DreamGenContext::transferinv() {
	const Frame *freeFrames = (const Frame *)segRef(data.word(kFreeframes)).ptr(kFrframedata, 0);
	const Frame *freeFrame = freeFrames + (3 * data.byte(kItemtotran) + 1);
	Frame *exFrames = (Frame *)segRef(data.word(kExtras)).ptr(kExframedata, 0);
	Frame *exFrame = exFrames + (3 * data.byte(kExpos) + 1);
	exFrame->width = freeFrame->width;
	exFrame->height = freeFrame->height;
	exFrame->x = freeFrame->x;
	exFrame->y = freeFrame->y;
	uint16 byteCount = freeFrame->width * freeFrame->height;
	const uint8 *src = segRef(data.word(kFreeframes)).ptr(kFrframes + freeFrame->ptr(), byteCount);
	uint8 *dst = segRef(data.word(kExtras)).ptr(kExframes + data.word(kExframepos), byteCount);
	memcpy(dst, src, byteCount);
	exFrame->setPtr(data.word(kExframepos));
	data.word(kExframepos) += byteCount;
}

bool DreamGenContext::pixelcheckset(const ObjPos *pos, uint8 x, uint8 y) {
	x -= pos->xMin;
	y -= pos->yMin;
	SetObject *setObject = getsetad(pos->index);
	Frame *frame = (Frame *)segRef(data.word(kSetframes)).ptr(kFramedata, 0) + setObject->index;
	const uint8 *ptr = segRef(data.word(kSetframes)).ptr(kFrames, 0) + frame->ptr() + y * frame->width + x;
	return *ptr != 0;
}

} /*namespace dreamgen */

