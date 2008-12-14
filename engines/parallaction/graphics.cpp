
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
 * $URL$
 * $Id$
 *
 */

#include "common/system.h"
#include "common/file.h"
#include "graphics/primitives.h"

#include "parallaction/input.h"
#include "parallaction/parallaction.h"


namespace Parallaction {

// this is the size of the receiving buffer for unpacked frames,
// since BRA uses some insanely big animations (the largest is
// part0/ani/dino.ani).
#define MAXIMUM_UNPACKED_BITMAP_SIZE	641*401


void Gfx::registerVar(const Common::String &name, int32 initialValue) {
	if (_vars.contains(name)) {
		warning("Variable '%s' already registered, ignoring initial value.\n", name.c_str());
	} else {
		_vars.setVal(name, initialValue);
	}
}

void Gfx::setVar(const Common::String &name, int32 value) {
	if (!_vars.contains(name)) {
		warning("Variable '%s' doesn't exist, skipping assignment.\n", name.c_str());
	} else {
		_vars.setVal(name, value);
	}
}

int32 Gfx::getVar(const Common::String &name) {
	int32 v = 0;

	if (!_vars.contains(name)) {
		warning("Variable '%s' doesn't exist, returning default value.\n", name.c_str());
	} else {
		v = _vars.getVal(name);
	}

	return v;
}



#define	LABEL_TRANSPARENT_COLOR 0xFF

void halfbritePixel(int x, int y, int color, void *data) {
	Graphics::Surface *surf = (Graphics::Surface *)data;
	byte *pixel = (byte*)surf->getBasePtr(x, y);
	*pixel &= ~0x20;
}

void drawCircleLine(int xCenter, int yCenter, int x, int y, int color, void (*plotProc)(int, int, int, void *), void *data){
	Graphics::drawLine(xCenter + x, yCenter + y, xCenter - x, yCenter + y, color, plotProc, data);
	Graphics::drawLine(xCenter + x, yCenter - y, xCenter - x, yCenter - y, color, plotProc, data);
	Graphics::drawLine(xCenter + y, yCenter + x, xCenter - y, yCenter + x, color, plotProc, data);
	Graphics::drawLine(xCenter + y, yCenter - x, xCenter - y, yCenter - x, color, plotProc, data);
}

void drawCircle(int xCenter, int yCenter, int radius, int color, void (*plotProc)(int, int, int, void *), void *data) {
	int x = 0;
	int y = radius;
	int p = 1 - radius;

	/* Plot first set of points */
	drawCircleLine(xCenter, yCenter, x, y, color, plotProc, data);

	while (x < y) {
		x++;
		if (p < 0)
			p += 2*x + 1;
		else {
			y--;
			p += 2 * (x-y) + 1;
		}
		drawCircleLine(xCenter, yCenter, x, y, color, plotProc, data);
	}
}




Palette::Palette() {

	int gameType = _vm->getGameType();

	if (gameType == GType_Nippon) {
		_colors = 32;
		_hb = (_vm->getPlatform() == Common::kPlatformAmiga);
	} else
	if (gameType == GType_BRA) {
		_colors = 256;
		_hb = false;
	} else
		error("can't create palette for id = '%i'", gameType);

	_size = _colors * 3;

	makeBlack();
}

Palette::Palette(const Palette &pal) {
	clone(pal);
}

void Palette::clone(const Palette &pal) {
	_colors = pal._colors;
	_hb = pal._hb;
	_size = pal._size;
	memcpy(_data, pal._data, _size);
}


void Palette::makeBlack() {
	memset(_data, 0, _size);
}

void Palette::setEntry(uint index, int red, int green, int blue) {
	assert(index < _colors);

	if (red >= 0)
		_data[index*3] = red & 0xFF;

	if (green >= 0)
		_data[index*3+1] = green & 0xFF;

	if (blue >= 0)
		_data[index*3+2] = blue & 0xFF;
}

void Palette::getEntry(uint index, int &red, int &green, int &blue) {
	assert(index < _colors);
	red   = _data[index*3];
	green = _data[index*3+1];
	blue  = _data[index*3+2];
}

void Palette::makeGrayscale() {
	byte v;
	for (uint16 i = 0; i < _colors; i++) {
		v = MAX(_data[i*3+1], _data[i*3+2]);
		v = MAX(v, _data[i*3]);
		setEntry(i, v, v, v);
	}
}

void Palette::fadeTo(const Palette& target, uint step) {

	if (step == 0)
		return;

	for (uint16 i = 0; i < _size; i++) {
		if (_data[i] == target._data[i]) continue;

		if (_data[i] < target._data[i])
			_data[i] = CLIP(_data[i] + step, (uint)0, (uint)target._data[i]);
		else
			_data[i] = CLIP(_data[i] - step, (uint)target._data[i], (uint)255);
	}

	return;
}

uint Palette::fillRGBA(byte *rgba) {

	byte r, g, b;
	byte *hbPal = rgba + _colors * 4;

	for (uint32 i = 0; i < _colors; i++) {
		r = (_data[i*3]   << 2) | (_data[i*3]   >> 4);
		g = (_data[i*3+1] << 2) | (_data[i*3+1] >> 4);
		b = (_data[i*3+2] << 2) | (_data[i*3+2] >> 4);

		rgba[i*4]   = r;
		rgba[i*4+1] = g;
		rgba[i*4+2] = b;
		rgba[i*4+3] = 0;

		if (_hb) {
			hbPal[i*4]   = r >> 1;
			hbPal[i*4+1] = g >> 1;
			hbPal[i*4+2] = b >> 1;
			hbPal[i*4+3] = 0;
		}

	}

	return ((_hb) ? 2 : 1) * _colors;
}

void Palette::rotate(uint first, uint last, bool forward) {

	byte tmp[3];

	if (forward) {					// forward

		tmp[0] = _data[first * 3];
		tmp[1] = _data[first * 3 + 1];
		tmp[2] = _data[first * 3 + 2];

		memmove(_data+first*3, _data+(first+1)*3, (last - first)*3);

		_data[last * 3]	 = tmp[0];
		_data[last * 3 + 1] = tmp[1];
		_data[last * 3 + 2] = tmp[2];

	} else {											// backward

		tmp[0] = _data[last * 3];
		tmp[1] = _data[last * 3 + 1];
		tmp[2] = _data[last * 3 + 2];

		memmove(_data+(first+1)*3, _data+first*3, (last - first)*3);

		_data[first * 3]	  = tmp[0];
		_data[first * 3 + 1] = tmp[1];
		_data[first * 3 + 2] = tmp[2];

	}

}



void Gfx::setPalette(Palette pal) {
	byte sysPal[256*4];

	uint n = pal.fillRGBA(sysPal);
	_vm->_system->setPalette(sysPal, 0, n);
}

void Gfx::setBlackPalette() {
	Palette pal;
	setPalette(pal);
}




void Gfx::animatePalette() {

	// avoid forcing setPalette when not needed
	bool done = false;

	PaletteFxRange *range;
	for (uint16 i = 0; i < 4; i++) {
		range = &_backgroundInfo->ranges[i];

		if ((range->_flags & 1) == 0) continue;		// animated palette
		range->_timer += range->_step * 2;	// update timer

		if (range->_timer < 0x4000) continue;		// check timeout

		range->_timer = 0;							// reset timer

		_palette.rotate(range->_first, range->_last, (range->_flags & 2) != 0);

		done = true;
	}

	if (done) {
		setPalette(_palette);
	}

	return;
}





void Gfx::setHalfbriteMode(bool enable) {
	if (_vm->getPlatform() != Common::kPlatformAmiga) return;
	if (enable == _halfbrite) return;

	_halfbrite = !_halfbrite;
	_hbCircleRadius = 0;
}

#define HALFBRITE_CIRCLE_RADIUS		48
void Gfx::setProjectorPos(int x, int y) {
	_hbCircleRadius = HALFBRITE_CIRCLE_RADIUS;
	_hbCirclePos.x = x + _hbCircleRadius;
	_hbCirclePos.y = y + _hbCircleRadius;
}

void Gfx::setProjectorProgram(int16 *data) {
	_nextProjectorPos = data;
}

void Gfx::drawInventory() {
/*
	if ((_engineFlags & kEngineInventory) == 0) {
		return;
	}
*/
	if (_vm->_input->_inputMode != Input::kInputModeInventory) {
		return;
	}

	Common::Rect r;
	_vm->_inventoryRenderer->getRect(r);
	byte *data = _vm->_inventoryRenderer->getData();

	copyRectToScreen(data, r.width(), r.left, r.top, r.width(), r.height());
}

void Gfx::drawList(Graphics::Surface &surface, GfxObjArray &list) {
	if (list.size() == 0) {
		return;
	}

	for (uint i = 0; i < list.size(); i++) {
		drawGfxObject(list[i], surface);
	}
}

void Gfx::beginFrame() {
	_skipBackground = (_backgroundInfo->bg.pixels == 0);	// don't render frame if background is missing

	if (!_skipBackground) {
		int32 oldBackgroundMode = _varBackgroundMode;
		_varBackgroundMode = getVar("background_mode");
		if (oldBackgroundMode != _varBackgroundMode) {
			switch (_varBackgroundMode) {
			case 1:
				_bitmapMask.free();
				break;
			case 2:
				_bitmapMask.create(_backgroundInfo->width, _backgroundInfo->height, 1);
				byte *data = (byte*)_bitmapMask.pixels;
				for (uint y = 0; y < _bitmapMask.h; y++) {
					for (uint x = 0; x < _bitmapMask.w; x++) {
						*data++ = _backgroundInfo->mask.getValue(x, y);
					}
				}
#if 0
				Common::DumpFile dump;
				dump.open("maskdump.bin");
				dump.write(_bitmapMask.pixels, _bitmapMask.w * _bitmapMask.h);
				dump.close();
#endif
				break;
			}
		}
	}

	_varDrawPathZones = getVar("draw_path_zones");
	if (_varDrawPathZones == 1 && _gameType != GType_BRA) {
		setVar("draw_path_zones", 0);
		_varDrawPathZones = 0;
		warning("Path zones are supported only in Big Red Adventure");
	}
}

int32 Gfx::getRenderMode(const char *type) {

	int32 mode = getVar(type);
	if (mode < 0 || mode > 2) {
		warning("new value for '%s' is wrong: resetting default", type);
		setVar(type, 1);
		mode = 1;
	}
	return mode;

}

void Gfx::copyRectToScreen(const byte *buf, int pitch, int x, int y, int w, int h) {
	if (_doubleBuffering) {
		if (_overlayMode)
			x += _scrollPos;

		byte *dst = (byte*)_backBuffer.getBasePtr(x, y);
		for (int i = 0; i < h; i++) {
			memcpy(dst, buf, w);
			buf += pitch;
			dst += _backBuffer.pitch;
		}
	} else {
		_vm->_system->copyRectToScreen(buf, pitch, x, y, w, h);
	}
}

void Gfx::clearScreen() {
	if (_doubleBuffering) {
		if (_backBuffer.pixels) {
			Common::Rect r(_backBuffer.w, _backBuffer.h);
			_backBuffer.fillRect(r, 0);
		}
	} else {
		_vm->_system->clearScreen();
	}
}

Graphics::Surface *Gfx::lockScreen() {
	if (_doubleBuffering) {
		return &_backBuffer;
	}
	return _vm->_system->lockScreen();
}

void Gfx::unlockScreen() {
	if (_doubleBuffering) {
		return;
	}
	_vm->_system->unlockScreen();
}

void Gfx::updateScreenIntern() {
	if (_doubleBuffering) {
		byte *data = (byte*)_backBuffer.getBasePtr(_scrollPos, 0);
		_vm->_system->copyRectToScreen(data, _backBuffer.pitch, 0, 0, _vm->_screenWidth, _vm->_screenHeight);
	}

	_vm->_system->updateScreen();
}

int Gfx::getScrollPos() {
	return _scrollPos;
}

void Gfx::setScrollPos(int scrollX) {
	_scrollPos = CLIP(scrollX, _minScroll, _maxScroll);
}

void Gfx::updateScreen() {

	// the scene is calculated in game coordinates, so no translation
	// is needed
	_overlayMode = false;

	if (!_skipBackground) {
		// background may not cover the whole screen, so adjust bulk update size
		uint w = _backgroundInfo->width;
		uint h = _backgroundInfo->height;

		byte *backgroundData = 0;
		uint16 backgroundPitch = 0;
		switch (_varBackgroundMode) {
		case 1:
			backgroundData = (byte*)_backgroundInfo->bg.getBasePtr(0, 0);
			backgroundPitch = _backgroundInfo->bg.pitch;
			break;
		case 2:
			backgroundData = (byte*)_bitmapMask.getBasePtr(0, 0);
			backgroundPitch = _bitmapMask.pitch;
			break;
		}
		copyRectToScreen(backgroundData, backgroundPitch, _backgroundInfo->x, _backgroundInfo->y, w, h);
	}

	if (_varDrawPathZones == 1) {
		Graphics::Surface *surf = lockScreen();
		ZoneList::iterator b = _vm->_location._zones.begin();
		ZoneList::iterator e = _vm->_location._zones.end();
		for (; b != e; b++) {
			ZonePtr z = *b;
			if (z->_type & kZonePath) {
				surf->frameRect(Common::Rect(z->getX(), z->getY(), z->getX() + z->width(), z->getY() + z->height()), 2);
			}
		}
		unlockScreen();
	}

	sortScene();
	Graphics::Surface *surf = lockScreen();
		// draws animations frames and other game items
		drawList(*surf, _sceneObjects);

		// special effects
		applyHalfbriteEffect_NS(*surf);

		// draws inventory, labels and dialogue items
		drawOverlay(*surf);
	unlockScreen();

	updateScreenIntern();
}

void Gfx::applyHalfbriteEffect_NS(Graphics::Surface &surf) {
	if (!_halfbrite) {
		return;
	}

	byte *buf = (byte*)surf.pixels;
	for (int i = 0; i < surf.w*surf.h; i++) {
		*buf++ |= 0x20;
	}

	if (_nextProjectorPos) {
		int16 x = *_nextProjectorPos++;
		int16 y = *_nextProjectorPos++;
		if (x == -1 && y == -1) {
			_nextProjectorPos = 0;
		} else {
			setProjectorPos(x, y);
		}
	}
	if (_hbCircleRadius > 0) {
		drawCircle(_hbCirclePos.x, _hbCirclePos.y, _hbCircleRadius, 0, &halfbritePixel, &surf);
	}
}

void Gfx::drawOverlay(Graphics::Surface &surf) {
	// the following items are handled in screen coordinates, so they need translation before
	// being drawn
	_overlayMode = true;

	drawInventory();

	updateFloatingLabel();

	drawList(surf, _items);
	drawList(surf, _balloons);
	drawList(surf, _labels);
}

//
//	graphic primitives
//


void Gfx::patchBackground(Graphics::Surface &surf, int16 x, int16 y, bool mask) {

	Common::Rect r(surf.w, surf.h);
	r.moveTo(x, y);

	uint16 z = (mask) ? _backgroundInfo->getLayer(y) : LAYER_FOREGROUND;
	blt(r, (byte*)surf.pixels, &_backgroundInfo->bg, z, 100, 0);
}

void Gfx::fillBackground(const Common::Rect& r, byte color) {
	_backgroundInfo->bg.fillRect(r, color);
}

void Gfx::invertBackground(const Common::Rect& r) {

	byte *d = (byte*)_backgroundInfo->bg.getBasePtr(r.left, r.top);

	for (int i = 0; i < r.height(); i++) {
		for (int j = 0; j < r.width(); j++) {
			*d ^= 0x1F;
			d++;
		}

		d += (_backgroundInfo->bg.pitch - r.width());
	}

}





void setupLabelSurface(Graphics::Surface &surf, uint w, uint h) {
	surf.create(w, h, 1);
	surf.fillRect(Common::Rect(w,h), LABEL_TRANSPARENT_COLOR);
}

uint Gfx::renderFloatingLabel(Font *font, char *text) {

	Graphics::Surface *cnv = new Graphics::Surface;

	uint w, h;

	if (_vm->getPlatform() == Common::kPlatformAmiga) {
		w = font->getStringWidth(text) + 16;
		h = 10;

		setupLabelSurface(*cnv, w, h);

		font->setColor(7);
		font->drawString((byte*)cnv->pixels + 1, cnv->w, text);
		font->drawString((byte*)cnv->pixels + 1 + cnv->w * 2, cnv->w, text);
		font->drawString((byte*)cnv->pixels + cnv->w, cnv->w, text);
		font->drawString((byte*)cnv->pixels + 2 + cnv->w, cnv->w, text);
		font->setColor(1);
		font->drawString((byte*)cnv->pixels + 1 + cnv->w, cnv->w, text);
	} else {
		w = font->getStringWidth(text);
		h = font->height();

		setupLabelSurface(*cnv, w, h);

		drawText(font, cnv, 0, 0, text, 0);
	}

	GfxObj *obj = new GfxObj(kGfxObjTypeLabel, new SurfaceToFrames(cnv), "floatingLabel");
	obj->transparentKey = LABEL_TRANSPARENT_COLOR;
	obj->layer = LAYER_FOREGROUND;

	uint id = _labels.size();
	_labels.insert_at(id, obj);

	return id;
}

void Gfx::showFloatingLabel(uint label) {
	assert(label < _labels.size());

	hideFloatingLabel();

	_labels[label]->x = -1000;
	_labels[label]->y = -1000;
	_labels[label]->setFlags(kGfxObjVisible);

	_floatingLabel = label;
}

void Gfx::hideFloatingLabel() {
	if (_floatingLabel != NO_FLOATING_LABEL) {
		_labels[_floatingLabel]->clearFlags(kGfxObjVisible);
	}
	_floatingLabel = NO_FLOATING_LABEL;
}


void Gfx::updateFloatingLabel() {
	if (_floatingLabel == NO_FLOATING_LABEL) {
		return;
	}

	struct FloatingLabelTraits {
		Common::Point _offsetWithItem;
		Common::Point _offsetWithoutItem;
		int	_minX;
		int _minY;
		int	_maxX;
		int _maxY;
	} *traits;

	Common::Rect r;
	_labels[_floatingLabel]->getRect(0, r);

	if (_gameType == GType_Nippon) {
		FloatingLabelTraits traits_NS = {
			Common::Point(16 - r.width()/2, 34),
			Common::Point(8 - r.width()/2, 21),
			0, 0, _vm->_screenWidth - r.width(), 190
		};
		traits = &traits_NS;
	} else {
		// FIXME: _maxY for BRA is not constant (390), but depends on _vm->_subtitleY
		FloatingLabelTraits traits_BR = {
			Common::Point(34 - r.width()/2, 70),
			Common::Point(16 - r.width()/2, 37),
			0, 0, _vm->_screenWidth - r.width(), 390
		};
		traits = &traits_BR;
	}

	Common::Point	cursor;
	_vm->_input->getCursorPos(cursor);
	Common::Point offset = (_vm->_input->_activeItem._id) ? traits->_offsetWithItem : traits->_offsetWithoutItem;

	_labels[_floatingLabel]->x = CLIP(cursor.x + offset.x, traits->_minX, traits->_maxX);
	_labels[_floatingLabel]->y = CLIP(cursor.y + offset.y, traits->_minY, traits->_maxY);
}




uint Gfx::createLabel(Font *font, const char *text, byte color) {
	assert(_labels.size() < MAX_NUM_LABELS);

	Graphics::Surface *cnv = new Graphics::Surface;

	uint w, h;

	if (_vm->getPlatform() == Common::kPlatformAmiga) {
		w = font->getStringWidth(text) + 2;
		h = font->height() + 2;

		setupLabelSurface(*cnv, w, h);

		drawText(font, cnv, 0, 2, text, 0);
		drawText(font, cnv, 2, 0, text, color);
	} else {
		w = font->getStringWidth(text);
		h = font->height();

		setupLabelSurface(*cnv, w, h);

		drawText(font, cnv, 0, 0, text, color);
	}

	GfxObj *obj = new GfxObj(kGfxObjTypeLabel, new SurfaceToFrames(cnv), "label");
	obj->transparentKey = LABEL_TRANSPARENT_COLOR;
	obj->layer = LAYER_FOREGROUND;

	int id = _labels.size();

	_labels.insert_at(id, obj);

	return id;
}

void Gfx::showLabel(uint id, int16 x, int16 y) {
	assert(id < _labels.size());
	_labels[id]->setFlags(kGfxObjVisible);

	Common::Rect r;
	_labels[id]->getRect(0, r);

	if (x == CENTER_LABEL_HORIZONTAL) {
		x = CLIP<int16>((_backgroundInfo->width - r.width()) / 2, 0, _backgroundInfo->width/2);
	}

	if (y == CENTER_LABEL_VERTICAL) {
		y = CLIP<int16>((_vm->_screenHeight - r.height()) / 2, 0, _vm->_screenHeight/2);
	}

	_labels[id]->x = x;
	_labels[id]->y = y;
}

void Gfx::hideLabel(uint id) {
	assert(id < _labels.size());
	_labels[id]->clearFlags(kGfxObjVisible);
}

void Gfx::freeLabels() {
	for (uint i = 0; i < _labels.size(); i++) {
		delete _labels[i];
	}
	_labels.clear();
	_floatingLabel = NO_FLOATING_LABEL;
}



void Gfx::copyRect(const Common::Rect &r, Graphics::Surface &src, Graphics::Surface &dst) {

	byte *s = (byte*)src.getBasePtr(r.left, r.top);
	byte *d = (byte*)dst.getBasePtr(0, 0);

	for (uint16 i = 0; i < r.height(); i++) {
		memcpy(d, s, r.width());

		s += src.pitch;
		d += dst.pitch;
	}

	return;
}

void Gfx::grabBackground(const Common::Rect& r, Graphics::Surface &dst) {
	copyRect(r, _backgroundInfo->bg, dst);
}


Gfx::Gfx(Parallaction* vm) :
	_vm(vm), _disk(vm->_disk), _scrollPos(0), _minScroll(0), _maxScroll(0) {

	_gameType = _vm->getGameType();
	_doubleBuffering = _gameType != GType_Nippon;

	initGraphics(_vm->_screenWidth, _vm->_screenHeight, _gameType == GType_BRA);

	setPalette(_palette);

	_floatingLabel = NO_FLOATING_LABEL;

	_backgroundInfo = 0;

	_halfbrite = false;
	_nextProjectorPos = 0;
	_hbCircleRadius = 0;

	_unpackedBitmap = new byte[MAXIMUM_UNPACKED_BITMAP_SIZE];
	assert(_unpackedBitmap);

	registerVar("background_mode", 1);
	_varBackgroundMode = 1;

	registerVar("draw_path_zones", 0);

	if ((_gameType == GType_BRA) && (_vm->getPlatform() == Common::kPlatformPC)) {
	// this loads the backup palette needed by the PC version of BRA (see setBackground()).
		BackgroundInfo	paletteInfo;
		_disk->loadSlide(paletteInfo, "pointer");
		_backupPal.clone(paletteInfo.palette);
	}

	return;
}

Gfx::~Gfx() {

	_backBuffer.free();

	delete _backgroundInfo;

	freeLabels();

	delete []_unpackedBitmap;

	return;
}



int Gfx::setItem(GfxObj* frames, uint16 x, uint16 y, byte transparentColor) {
	int id = _items.size();

	frames->x = x;
	frames->y = y;
	frames->transparentKey = transparentColor;
	frames->layer = LAYER_FOREGROUND;
	frames->setFlags(kGfxObjVisible);

	_items.insert_at(id, frames);

	setItemFrame(id, 0);

	return id;
}

void Gfx::setItemFrame(uint item, uint16 f) {
	_items[item]->frame = f;
}


GfxObj* Gfx::registerBalloon(Frames *frames, const char *text) {

	GfxObj *obj = new GfxObj(kGfxObjTypeBalloon, frames, text);

	obj->layer = LAYER_FOREGROUND;
	obj->frame = 0;
	obj->setFlags(kGfxObjVisible);

	_balloons.push_back(obj);

	return obj;
}

void Gfx::destroyBalloons() {
	for (uint i = 0; i < _balloons.size(); i++) {
		delete _balloons[i];
	}
	_balloons.clear();
}

void Gfx::freeItems() {
	_items.clear();
}

void Gfx::setBackground(uint type, BackgroundInfo *info) {
	delete _backgroundInfo;
	_backgroundInfo = info;

	if (type == kBackgroundLocation) {
		// The PC version of BRA needs the entries 20-31 of the palette to be constant, but
		// the background resource files are screwed up. The right colors come from an unused
		// bitmap (pointer.bmp). Nothing is known about the Amiga version so far.
		if ((_vm->getGameType() == GType_BRA) && (_vm->getPlatform() == Common::kPlatformPC)) {
			int r, g, b;
			for (uint i = 16; i < 32; i++) {
				_backupPal.getEntry(i, r, g, b);
				_backgroundInfo->palette.setEntry(i, r, g, b);
			}
		}

		setPalette(_backgroundInfo->palette);
		_palette.clone(_backgroundInfo->palette);
	} else {
		for (uint i = 0; i < 6; i++)
			_backgroundInfo->ranges[i]._flags = 0;	// disable palette cycling for slides
		setPalette(_backgroundInfo->palette);
	}

	if (_gameType == GType_BRA) {
		int width = CLIP(info->width, (int)_vm->_screenWidth, info->width);
		int height = CLIP(info->height, (int)_vm->_screenHeight, info->height);

		if (width != _backBuffer.w || height != _backBuffer.h) {
			_backBuffer.create(width, height, 1);
		}
	}

	_minScroll = 0;
	_maxScroll = _backgroundInfo->width - _vm->_screenWidth;
}

} // namespace Parallaction
