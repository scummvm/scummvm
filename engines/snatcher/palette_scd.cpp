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

#include "snatcher/graphics.h"
#include "snatcher/palette.h"
#include "snatcher/palevent_scd.h"

#include "common/algorithm.h"
#include "common/array.h"
#include "common/debug.h"
#include "common/endian.h"
#include "common/func.h"
#include "common/textconsole.h"

#include "graphics/paletteman.h"
#include "graphics/pixelformat.h"
#include "graphics/segagfx.h"

namespace Snatcher {

class GraphicsEngine;
class SCDPalette : public Palette {
public:
	SCDPalette(const Graphics::PixelFormat *pxf, PaletteManager *pm, GraphicsEngine::GfxState &state);
	~SCDPalette() override;

	bool enqueueEvent(ResourcePointer &res) override;
	void processEventQueue() override;
	void clearEvents() override;
	void setDefaults(int mode) override;
	void update() override;
	void hINTCallback(void *segaRenderer) override;
	const uint8 *getSystemPalette() const override { return _sysPalette; }

	void selectPalettes(uint32) override;
	void getPresetColor(int pal, int index, uint8 &r, uint8 &g, uint8 &b) const override;
	void setColor(int index, uint8 r, uint8 g, uint8 b) override;
	void getColor(int index, uint8 &r, uint8 &g, uint8 &b) const override;
	void adjustColor(int index, int8 r, int8 g, int8 b) override;

private:
	void updateSystemPalette();

	typedef Common::Functor1Mem<PalEventSCD*, void, SCDPalette> EventProc;
	Common::Array<EventProc*> _eventProcs;

	void event_palSet(PalEventSCD *evt);
	void event_palFadeFromBlack(PalEventSCD *evt);
	void event_palFadeToBlack(PalEventSCD *evt);
	void event_palFadeFromWhite(PalEventSCD *evt);
	void event_palFadeToWhite(PalEventSCD *evt);
	void event_palCycle(PalEventSCD *evt);
	void event_palFadeToColor(PalEventSCD *evt);
	void event_palFadeToGrey(PalEventSCD *evt);
	void event_palFadeToPal(PalEventSCD *evt);
	void event_palClear(PalEventSCD *evt);

	bool updateDelay(PalEventSCD *evt);
	void fadeStep(uint16 *modColor, uint16 toR, uint16 toG, uint16 toB);

	PalEventSCD *_eventQueue;
	PalEventSCD *_eventCurPos;

	uint16 *_colors;
	uint16 *_colors2;
	uint8 *_sysPalette;

private:
	typedef Common::Functor1Mem<Graphics::SegaRenderer*, void, SCDPalette> HINTFunc;
	Common::Array<HINTFunc*> _hINTProcs;
	const HINTFunc *_hINTHandler;

	void setHINTHandler(uint8 num);

	void hIntHandler_dummy(Graphics::SegaRenderer*) {}
	void hIntHandler_characterChatPortraitPalette(Graphics::SegaRenderer *sr);
	int _transitionStep;
};

SCDPalette::SCDPalette(const Graphics::PixelFormat *pxf, PaletteManager *pm, GraphicsEngine::GfxState &state) : Palette(pxf, pm, state), _eventProcs(),
	_eventQueue(nullptr), _eventCurPos(nullptr), _colors(nullptr), _colors2(nullptr), _sysPalette(nullptr), _hINTHandler(nullptr), _transitionStep(0) {

#define P_OP(a)	_eventProcs.push_back(new EventProc(this, &SCDPalette::event_##a));
	_eventProcs.push_back(nullptr);
	P_OP(palSet);
	P_OP(palFadeFromBlack);
	P_OP(palFadeToBlack);
	P_OP(palFadeFromWhite);
	P_OP(palFadeToWhite);
	P_OP(palCycle);
	P_OP(palFadeToColor);
	P_OP(palFadeToGrey);
	P_OP(palFadeToPal);
	P_OP(palClear);
#undef P_OP

	_hINTProcs.push_back(new HINTFunc(this, &SCDPalette::hIntHandler_characterChatPortraitPalette));
	_hINTProcs.push_back(new HINTFunc(this, &SCDPalette::hIntHandler_dummy));

	_eventQueue = _eventCurPos = new PalEventSCD[12];
	_colors = new uint16[128]();
	_colors2 = new uint16[128]();
	_sysPalette = new uint8[256]();
}

SCDPalette::~SCDPalette() {
	delete[] _eventQueue;
	delete[] _colors;
	delete[] _colors2;
	delete[] _sysPalette;

	for (Common::Array<EventProc*>::const_iterator i = _eventProcs.begin(); i != _eventProcs.end(); ++i)
		delete *i;
	for (Common::Array<HINTFunc*>::const_iterator i = _hINTProcs.begin(); i != _hINTProcs.end(); ++i)
		delete *i;
}

bool SCDPalette::enqueueEvent(ResourcePointer &res) {
	for (int i = 0; i < 12; ++i) {
		if (++_eventCurPos > &_eventQueue[11])
			_eventCurPos = _eventQueue;
		if (_eventCurPos->cmd)
			continue;

		_eventCurPos->res = res;
		_eventCurPos->cmd = *res++;
		_eventCurPos->delay = *res++;
		_eventCurPos->countDown = 0;
		_eventCurPos->destFlag = (*res() & 0x80);
		_eventCurPos->destOffset = (*res++) >> 1;
		_eventCurPos->len = (*res++) + 1;
		_eventCurPos->srcOffsets = _eventCurPos->srcOffsetCur = reinterpret_cast<const uint32*>(res());
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
	if (_gfxState.getVar(8))
		return;
	_gfxState.setVar(2, 0);

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
}

void SCDPalette::clearEvents() {
	for (int i = 0; i < 12; ++i)
		_eventQueue[i].clear();
}

void SCDPalette::setDefaults(int mode) {
	if (mode == 0) {
		int cnt = (_gfxState.testFlag(1, 1) || _gfxState.getVar(5)) ? 64 : 48;
		_gfxState.clearFlag(1, 1);
		Common::fill<uint16*, uint16>(&_colors[64 - cnt], &_colors[cnt], 0);
		if (_gfxState.testFlag(1, 2)) {
			_gfxState.clearFlag(1, 2);
			Common::fill<uint16*, uint16>(&_colors[80], &_colors[128], 0);
			_gfxState.setFlag(7, 1);
		}
	} else if (mode == 1) {
		static const uint16 colors[] =  { 0x000, 0xEEE, 0x0EE, 0x86E, 0xEE0, 0xE42, 0x4E0, 0x000 };
		Common::copy<const uint16*, uint16*>(colors, &colors[ARRAYSIZE(colors)], _colors);
	}
	_gfxState.setFlag(7, 0);
}

static const uint16 _presetColors[7][16] = {
	{ 0x0eee, 0x0000, 0x0ccc, 0x0aaa, 0x0888, 0x0222, 0x0caa, 0x0aaa, 0x0a88, 0x0888, 0x0866, 0x0866, 0x0644, 0x0644, 0x0444, 0x0422 },
	{ 0x0eee, 0x0000, 0x008e, 0x008c, 0x006c, 0x004a, 0x00ee, 0x00ce, 0x00ce, 0x00ae, 0x00ac, 0x008c, 0x008c, 0x006c, 0x006c, 0x004a },
	{ 0x0eee, 0x0000, 0x006e, 0x004c, 0x004c, 0x0028, 0x008e, 0x006e, 0x004e, 0x000c, 0x000c, 0x000a, 0x000a, 0x0008, 0x0008, 0x0006 },
	{ 0x0000, 0x0000, 0x0000, 0x0020, 0x0040, 0x0060, 0x0080, 0x00a0, 0x00c0, 0x0aea, 0x06c6, 0x08e8, 0x0eee, 0x0000, 0x0000, 0x0000 },
	{ 0x0000, 0x0000, 0x0600, 0x0800, 0x0a00, 0x0e00, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 },
	{ 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0060, 0x0080, 0x00a0, 0x00c0, 0x0aea, 0x06c6, 0x08e8, 0x0eee, 0x0000, 0x0000, 0x0000 },
	{ 0x0000, 0x0eee, 0x00ee, 0x000e, 0x0ee0, 0x0e00, 0x00e0, 0x0600, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000 }
};

void SCDPalette::selectPalettes(uint32 sel) {
	for (int i = 0; i < 4; ++i) {
		uint8 p = (sel >> (24 - 8 * i)) & 0xFF;
		if (p == 0xFF)
			continue;
		assert(p < ARRAYSIZE(_presetColors));
		Common::copy<const uint16 *, uint16 *>(_presetColors[p], _presetColors[p + 1], &_colors[i << 4]);
	}
	_gfxState.setFlag(7, 0);
}

void SCDPalette::getPresetColor(int pal, int index, uint8 &r, uint8 &g, uint8 &b) const {
	assert(pal < ARRAYSIZE(_presetColors));
	assert(index < ARRAYSIZE(_presetColors[0]));
	r = (_presetColors[pal][index] & 0xF00) >> 8;
	g = (_presetColors[pal][index] & 0xF0) >> 4;
	b = _presetColors[pal][index] & 0xF;
}

void SCDPalette::setColor(int index, uint8 r, uint8 g, uint8 b) {
	assert(index >= 0 && index < 64);
	_colors[index] = (r & 0x0E) << 8 | (g & 0x0E) << 4 | (b & 0x0E);
	_gfxState.setFlag(7, 0);
}

void SCDPalette::getColor(int index, uint8 &r, uint8 &g, uint8 &b) const {
	r = (_colors[index] & 0xF00) >> 8;
	g = (_colors[index] & 0xF0) >> 4;
	b = _colors[index] & 0xF;
}

void SCDPalette::adjustColor(int index, int8 r, int8 g, int8 b) {
	assert(index >= 0 && index < 64);
	_colors[index] = (((((_colors[index] & 0xE00) >> 8) + (r & 0x0E)) & 0x0E) << 8) | (((((_colors[index] & 0xE0) >> 4) + (g & 0x0E)) & 0x0E) << 4) | (((_colors[index] + (b & 0x0E)) & 0x0E));
	_gfxState.setFlag(7, 0);
}

void SCDPalette::update() {
	if (!_gfxState.getVar(7) && !_gfxState.getVar(11))
		return;

	if (_gfxState.getVar(11) == 0xFF) {
		_gfxState.setFlag(7, 0);
		_gfxState.setVar(11, 0);
	} else if (_gfxState.getVar(11)) {
		setHINTHandler(0);
	}

	bool palChanged = false;
	if (_gfxState.testFlag(7, 0) || _gfxState.getVar(11)) {
		Common::copy<const uint16*, uint16*>(_colors, &_colors[64], _colors2);
		palChanged = true;
	}

	if (_gfxState.testFlag(7, 1))
		Common::copy<const uint16*, uint16*>(&_colors[64], &_colors[128], &_colors2[64]);

	_gfxState.clearFlag(7, 0);
	_gfxState.clearFlag(7, 1);

	if (_gfxState.testFlag(7, 2)) {
		Common::fill<uint16*, uint16>(_colors2, &_colors2[64], 0xEEE);
		_gfxState.setFlag(7, 0);
		_gfxState.clearFlag(7, 2);
		palChanged = true;
	}

	if (palChanged)
		updateSystemPalette();
}

void SCDPalette::hINTCallback(void *segaRenderer) {
	Graphics::SegaRenderer *sr = static_cast<Graphics::SegaRenderer*>(segaRenderer);
	if (_gfxState.getVar(11) && _hINTHandler && _hINTHandler->isValid()) {
		(*_hINTHandler)(sr);
		updateSystemPalette();
		sr->setRenderColorTable(_sysPalette, 0, 64);
	}
}

void SCDPalette::updateSystemPalette() {
	const uint16 *src = _colors2;
	uint8 *dst = _sysPalette;

	// R: bits 1, 2, 3   G: bits 5, 6, 7   B: bits 9, 10, 11
	for (int i = 0; i < 64; ++i) {
		uint16 in = *src++;
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

	if (_pixelFormat.bytesPerPixel == 1)
		_palMan->setPalette(_sysPalette, 0, 256);
}

void SCDPalette::event_palSet(PalEventSCD *evt) {
	if (updateDelay(evt))
		return;

	uint16 *dst = &_colors[evt->destOffset];
	uint32 srcOffs = READ_BE_UINT32(evt->srcOffsetCur);
	const uint16 *src = reinterpret_cast<const uint16*>(evt->res.makePtr(srcOffs)());
	for (int i = 0; i < evt->len; ++i)
		*dst++ = READ_BE_UINT16(src++);

	if (READ_BE_UINT16(++evt->srcOffsetCur) == 0xFFFF)
		evt->cmd = 0;
}

void SCDPalette::event_palFadeFromBlack(PalEventSCD *evt) {
	uint16 *dst = &_colors[evt->destOffset];
	Common::fill<uint16*, uint16>(dst, &dst[evt->len], 0);
	evt->cmd = 9;
	event_palFadeToPal(evt);
}

void SCDPalette::event_palFadeToBlack(PalEventSCD *evt) {
	if (updateDelay(evt))
		return;

	uint16 *dst = &_colors[evt->destOffset];
	for (int i = 0; i < evt->len; ++i) {
		if (*dst & 0xF)
			*dst -= 0x2;
		if (*dst & 0xF0)
			*dst -= 0x20;
		if (*dst & 0xF00)
			*dst -= 0x200;
		++dst;
	}
	if (++evt->progress == 8)
		evt->cmd = 0;
}

void SCDPalette::event_palFadeFromWhite(PalEventSCD *evt) {
	uint16 *dst = &_colors[evt->destOffset];
	Common::fill<uint16*, uint16>(dst, &dst[evt->len], 0xEEE);
	evt->cmd = 9;
	event_palFadeToPal(evt);
}

void SCDPalette::event_palFadeToWhite(PalEventSCD *evt) {
	if (updateDelay(evt))
		return;

	uint16 *dst = &_colors[evt->destOffset];
	for (int i = 0; i < evt->len; ++i) {
		if ((*dst & 0xF) < 0xE)
			*dst += 0x2;
		if ((*dst & 0xF0) < 0xE0)
			*dst += 0x20;
		if ((*dst & 0xF00) < 0xE00)
			*dst += 0x200;
		++dst;
	}
	if (++evt->progress == 8)
		evt->cmd = 0;
}

void SCDPalette::event_palCycle(PalEventSCD *evt) {
	if (updateDelay(evt))
		return;

	uint16 *dst = &_colors[evt->destOffset];
	uint32 srcOffs = READ_BE_UINT32(evt->srcOffsetCur);
	const uint16 *src = reinterpret_cast<const uint16*>(evt->res.makePtr(srcOffs)());
	for (int i = 0; i < evt->len; ++i)
		*dst++ = READ_BE_UINT16(src++);

	if (READ_BE_UINT16(++evt->srcOffsetCur) == 0xFFFF)
		evt->srcOffsetCur = evt->srcOffsets;
}

void SCDPalette::event_palFadeToColor(PalEventSCD *evt) {
	if (updateDelay(evt))
		return;

	uint16 *dst = &_colors[evt->destOffset];
	uint32 srcOffs = READ_BE_UINT32(evt->srcOffsetCur);
	uint16 col = READ_BE_UINT16(evt->res.makePtr(srcOffs)());

	for (int i = 0; i < evt->len; ++i)
		fadeStep(dst++, col & 0xF, col & 0xF0, col & 0xF00);

	if (++evt->progress == 8)
		evt->cmd = 0;
}

void SCDPalette::event_palFadeToGrey(PalEventSCD *evt) {
	if (updateDelay(evt))
		return;

	uint16 *dst = &_colors[evt->destOffset];
	uint32 srcOffs = READ_BE_UINT32(evt->srcOffsetCur);
	const uint16 *src = reinterpret_cast<const uint16*>(evt->res.makePtr(srcOffs)());

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

void SCDPalette::event_palFadeToPal(PalEventSCD *evt) {
	if (updateDelay(evt))
		return;

	uint16 *dst = &_colors[evt->destOffset];
	uint32 srcOffs = READ_BE_UINT32(evt->srcOffsetCur);
	const uint16 *src = reinterpret_cast<const uint16*>(evt->res.makePtr(srcOffs)());

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
	uint16 *dst = &_colors[evt->destOffset];
	Common::fill<uint16*, uint16>(dst, &dst[evt->len], 0);
	evt->cmd = 0;
}

bool SCDPalette::updateDelay(PalEventSCD *evt) {
	if (evt->countDown) {
		evt->countDown--;
		return true;
	}

	evt->countDown = evt->delay;
	_gfxState.setFlag(7, evt->destFlag ? 1 : 0);
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

void SCDPalette::setHINTHandler(uint8 num) {
	if (num < _hINTProcs.size())
		_hINTHandler = _hINTProcs[num];
	 else
		error("%s(): Invalid HINT handler %d", __FUNCTION__, num);
	_transitionStep = 0;
}

void SCDPalette::hIntHandler_characterChatPortraitPalette(Graphics::SegaRenderer *sr) {
	if (_transitionStep++ != 136)
		return;
	Common::copy<const uint16*, uint16*>(&_colors2[80], &_colors2[128], &_colors2[16]);
	sr->hINT_setCounter(255);
	sr->hINT_enable(false);
}

Palette *Palette::createSegaPalette(const Graphics::PixelFormat *pxf, PaletteManager *pm, GraphicsEngine::GfxState &state) {
	return new SCDPalette(pxf, pm, state);
}

} // End of namespace Snatcher
