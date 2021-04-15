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

#include "snatcher/palette.h"
#include "snatcher/palevent_scd.h"

#include "common/algorithm.h"
#include "common/array.h"
#include "common/endian.h"
#include "common/func.h"
#include "common/textconsole.h"
#include "graphics/paletteman.h"

namespace Snatcher {

class SCDPalette : public Palette {
public:
	SCDPalette(PaletteManager *pm);
	~SCDPalette() override;

	bool enqueueEvent(const uint8 *data, uint32 curPos) override;
	void processEventQueue() override;

private:
	typedef Common::Functor1Mem<PalEventSCD*, void, SCDPalette> EventProc;
	Common::Array<EventProc*> _eventProcs;

	void event_palSet(PalEventSCD *evt);
	void event_palFadeFromBlack(PalEventSCD *evt);
	void event_palFadeToBlack(PalEventSCD *evt);
	void event_palFadeFromWhite(PalEventSCD *evt);
	void event_palFadeToWhite(PalEventSCD *evt);
	void event_palCycle(PalEventSCD *evt);
	void event_palFadeToColor(PalEventSCD *evt);
	void event_palFade2(PalEventSCD *evt);
	void event_palFadeFrom(PalEventSCD *evt);
	void event_palClear(PalEventSCD *evt);

	bool updateDelay(PalEventSCD *evt);
	void fadeStep(uint16 *modColor, uint16 toR, uint16 toG, uint16 toB);

	void updateSystemPalette();

	PalEventSCD *_eventQueue;
	PalEventSCD *_eventCurPos;

	uint16 *_colors;
	uint8 *_internalPalette;
	uint8 _updateFlags;
};

SCDPalette::SCDPalette(PaletteManager *pm) : Palette(pm), _eventProcs(), _eventQueue(0), _eventCurPos(0), _colors(0), _internalPalette(0), _updateFlags(0) {
#define P_OP(a)	_eventProcs.push_back(new EventProc(this, &SCDPalette::event_##a));
	_eventProcs.push_back(0);
	P_OP(palSet);
	P_OP(palFadeFromBlack);
	P_OP(palFadeToBlack);
	P_OP(palFadeFromWhite);
	P_OP(palFadeToWhite);
	P_OP(palCycle);
	P_OP(palFadeToColor);
	P_OP(palFade2);
	P_OP(palFadeFrom);
	P_OP(palClear);
#undef P_OP

	_eventQueue = _eventCurPos = new PalEventSCD[12];
	memset(_eventQueue, 0, 12 * sizeof(PalEventSCD));
	_colors = new uint16[128];
	memset(_colors, 0, 128 * sizeof(uint16));
	_internalPalette = new uint8[256];
	memset(_internalPalette, 0, 256 * sizeof(uint8));
}

SCDPalette::~SCDPalette() {
	delete[] _eventQueue;
	delete[] _colors;
	delete[] _internalPalette;

	for (Common::Array<EventProc*>::iterator i = _eventProcs.begin(); i != _eventProcs.end(); ++i)
		delete *i;
}

bool SCDPalette::enqueueEvent(const uint8 *data, uint32 curPos) {
	const uint8 *dataStart = data;
	data = &data[curPos];

	for (int i = 0; i < 12; ++i) {
		if (++_eventCurPos > &_eventQueue[11])
			_eventCurPos = _eventQueue;
		if (_eventCurPos->cmd)
			continue;

		_eventCurPos->cmd = *data++;
		_eventCurPos->delay = *data++;
		_eventCurPos->countDown = 0;
		_eventCurPos->destOffset = *data++;
		_eventCurPos->len = (*data++) + 1;
		_eventCurPos->srcOffsets = _eventCurPos->srcOffsetCur = (uint32*)data;
		_eventCurPos->srcOrig = dataStart;
		_eventCurPos->progress = 0;

		if (_eventCurPos->cmd & 0x80) {
			_eventCurPos->cmd &= ~0x80;
			for (int ii = 0; ii < 12; ++ii) {
				if (_eventQueue[ii].cmd == 6)
					_eventQueue[ii].cmd = 0;
			}
		}
		return true;
	}

	return false;
}

void SCDPalette::processEventQueue() {
	PalEventSCD *p = _eventCurPos;
	for (int i = 0; i < 12; ++i) {
		if (++p > &_eventQueue[11])
			p = _eventQueue;

		if (!p->cmd)
			continue;

		if (p->cmd > _eventProcs.size())
			error("SCDPalette::processEventQueue(): Invalid opcode 0x%02x", p->cmd);

		if (_eventProcs[p->cmd]->isValid())
			(*_eventProcs[p->cmd])(p);
	}

	//if (_updateFlags & 1)
		updateSystemPalette();

	//if (_updateFlags & 2)
		updateSystemPalette();
	_updateFlags = 0;
}

void SCDPalette::event_palSet(PalEventSCD *evt) {
	if (updateDelay(evt))
		return;

	uint16 *dst = &_colors[evt->destOffset >> 1];
	uint32 srcOffs = READ_BE_UINT32(evt->srcOffsetCur);
	assert(srcOffs >= 0x28000);
	const uint16 *src = (const uint16*)&evt->srcOrig[srcOffs - 0x28000];

	for (int i = 0; i < evt->len; ++i)
		*dst++ = READ_BE_UINT16(src++);

	if (READ_BE_UINT16(++evt->srcOffsetCur) == 0xFFFF)
		evt->cmd = 0;	
}

void SCDPalette::event_palFadeFromBlack(PalEventSCD *evt) {
	uint16 *dst = &_colors[evt->destOffset >> 1];
	Common::fill<uint16*, uint16>(dst, &dst[evt->len], 0);
	evt->cmd = 9;
	event_palFadeFrom(evt);
}

void SCDPalette::event_palFadeToBlack(PalEventSCD *evt) {
	if (updateDelay(evt))
		return;

	uint16 *dst = &_colors[evt->destOffset >> 1];
	for (int i = 0; i < evt->len; ++i) {
		if (*dst & 0xF)
			*dst -= 0x2;
		if (*dst & 0xF0)
			*dst -= 0x20;
		if (*dst & 0xF00)
			*dst -= 0x200;
	}
	if (++evt->progress == 8)
		evt->cmd = 0;
}

void SCDPalette::event_palFadeFromWhite(PalEventSCD *evt) {
	uint16 *dst = &_colors[evt->destOffset >> 1];
	Common::fill<uint16*, uint16>(dst, &dst[evt->len], 0xEEE);
	evt->cmd = 9;
	event_palFadeFrom(evt);
}

void SCDPalette::event_palFadeToWhite(PalEventSCD *evt) {
	if (updateDelay(evt))
		return;

	uint16 *dst = &_colors[evt->destOffset >> 1];
	for (int i = 0; i < evt->len; ++i) {
		if ((*dst & 0xF) < 0xE)
			*dst += 0x2;
		if ((*dst & 0xF0) < 0xE0)
			*dst += 0x20;
		if ((*dst & 0xF00) < 0xE00)
			*dst += 0x200;
	}
	if (++evt->progress == 8)
		evt->cmd = 0;
}

void SCDPalette::event_palCycle(PalEventSCD *evt) {
	if (updateDelay(evt))
		return;

	uint16 *dst = &_colors[evt->destOffset >> 1];
	uint32 srcOffs = READ_BE_UINT32(evt->srcOffsetCur);
	assert(srcOffs >= 0x28000);
	const uint16 *src = (const uint16*)&evt->srcOrig[srcOffs - 0x28000];

	for (int i = 0; i < evt->len; ++i)
		*dst++ = READ_BE_UINT16(src++);

	if (READ_BE_UINT16(++evt->srcOffsetCur) == 0xFFFF)
		evt->srcOffsetCur = evt->srcOffsets;
}

void SCDPalette::event_palFadeToColor(PalEventSCD *evt) {
	if (updateDelay(evt))
		return;

	uint16 *dst = &_colors[evt->destOffset >> 1];
	uint32 srcOffs = READ_BE_UINT32(evt->srcOffsetCur);
	assert(srcOffs >= 0x28000);
	uint16 col = READ_BE_UINT16(&evt->srcOrig[srcOffs - 0x28000]);

	for (int i = 0; i < evt->len; ++i)
		fadeStep(dst++, col & 0xF, col & 0xF0, col & 0xF00);

	if (++evt->progress == 8)
		evt->cmd = 0;
}

void SCDPalette::event_palFade2(PalEventSCD *evt) {
	if (updateDelay(evt))
		return;

	uint16 *dst = &_colors[evt->destOffset >> 1];
	uint32 srcOffs = READ_BE_UINT32(evt->srcOffsetCur);
	assert(srcOffs >= 0x28000);
	const uint16 *src = (const uint16*)&evt->srcOrig[srcOffs - 0x28000];

	for (int i = 0; i < evt->len; ++i) {
		uint16 col = READ_BE_UINT16(src++);
		uint8 r = col & 0xF;
		uint8 g = (col & 0xF0) >> 4;
		uint8 b = (col & 0xF00) >> 8;
		col = ((r + g + b) / 3) & 0xFFFE;		
		fadeStep(dst++, col & 0xF, (col << 4) & 0xF0, (col << 8) & 0xF00);
	}

	if (++evt->progress == 8)
		evt->cmd = 0;
}

void SCDPalette::event_palFadeFrom(PalEventSCD *evt) {
	if (updateDelay(evt))
		return;

	uint16 *dst = &_colors[evt->destOffset >> 1];
	uint32 srcOffs = READ_BE_UINT32(evt->srcOffsetCur);
	assert(srcOffs >= 0x28000);
	const uint16 *src = (const uint16*)&evt->srcOrig[srcOffs - 0x28000];

	for (int i = 0; i < evt->len; ++i) {
		uint16 col = READ_BE_UINT16(src++);
		fadeStep(dst++, col & 0xF, col & 0xF0, col & 0xF00);
	}

	if (++evt->progress == 8)
		evt->cmd = 0;
}


void SCDPalette::event_palClear(PalEventSCD *evt) {
	if (updateDelay(evt))
		return;

	uint16 *dst = &_colors[evt->destOffset >> 1];
	Common::fill<uint16*, uint16>(dst, &dst[evt->len], 0);
	evt->cmd = 0;
}

bool SCDPalette::updateDelay(PalEventSCD *evt) {
	if (evt->countDown) {
		evt->countDown--;
		return true;
	}

	evt->countDown = evt->delay;
	_updateFlags |= ((evt->destOffset & 0x80) ? 2 : 1);
	return false;
}

void SCDPalette::fadeStep(uint16 *modColor, uint16 toR, uint16 toG, uint16 toB) {
	uint16 r = *modColor & 0xF;
	uint16 g = *modColor & 0xF0;
	uint16 b = *modColor & 0xF00;

	if (r < toR)
		r += 0x2;
	else if (r > toR)
		r -= 0x2;

	if (g < toG)
		g += 0x20;
	else if (g > toG)
		g -= 0x20;

	if (b < toB)
		b += 0x200;
	else if (b > toB)
		b -= 0x200;

	*modColor = (r | g | b);
}

void SCDPalette::updateSystemPalette() {
	const uint16 *src = &_colors[0 << 6];
	uint8 *dst = _internalPalette;


	/*uint8 rgbColors[48];
	uint8 *dst = rgbColors;

	if (srcPalID >= 31 && srcPalID <= 38) {
		src = &_segaCustomPalettes[(srcPalID - 31) << 4];
	} else if (srcPalID >= 0) {
		int temp = 0;
		const uint16 *palettes = _vm->staticres()->loadRawDataBe16(kEoB1PalettesSega, temp);
		if (!palettes)
			return;
		src = &palettes[srcPalID << 4];
	}
	*/
	// R: bits 1, 2, 3   G: bits 5, 6, 7   B: bits 9, 10, 11
	for (int i = 0; i < 64; ++i) {
		uint16 in = *src++;
		//_segaCurPalette[dstPalID << 4 | i] = in;
#if 0
		static const uint8 col[8] = { 0, 52, 87, 116, 144, 172, 206, 255 };
		*dst++ = col[(in & 0x00F) >> 1];
		*dst++ = col[(in & 0x0F0) >> 5];
		*dst++ = col[(in & 0xF00) >> 9];
#else
		*dst++ = ((in & 0x00F) >> 1) * 255 / 7;
		*dst++ = ((in & 0x0F0) >> 5) * 255 / 7;
		*dst++ = ((in & 0xF00) >> 9) * 255 / 7;
#endif
	}

	/*getPalette(0).copy(rgbColors, 0, 16, dstPalID << 4);

	if (_specialColorReplace) {
		const uint8 swapColors[6] = { 0x08, 0x09, 0x0C, 0x0D, 0x0E, 0x0F };
		for (int i = 0; i < 6; ++i)
			getPalette(0).copy(getPalette(0), 0x10 | swapColors[i], 1, swapColors[i]);
	}

	if (set)
		setScreenPalette(getPalette(0));
		*/
	_palMan->setPalette(_internalPalette, 0, 256);
}

Palette *Palette::createSegaPalette(PaletteManager *pm) {
	return new SCDPalette(pm);
}

} // End of namespace Snatcher
