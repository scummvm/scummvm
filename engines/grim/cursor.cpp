/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#include "engines/grim/cursor.h"
#include "engines/grim/gfx_base.h"
#include "common/config-manager.h"
#include "common/file.h"
#include "engines/grim/bitmap.h"
#include "graphics/surface.h"
#include "graphics/cursorman.h"
#include "common/system.h"


const static int numCursors = 9;
static const char * const cursorName[] =
#ifdef ANDROID
	{"", "redcircle", "bluecircle", "cursor3", "cursor4", "cursor5",
	 "cursor6", "hourglass", "wave" };
#else
	{"cursor0", "cursor1", "cursor2", "cursor3", "cursor4", "cursor5",
	"cursor6", "cursor7", "" };
#endif

namespace Grim {

CursorData::CursorData(const Common::String& name) :
	_frameTick(0), _rotTick(0), _frame(0), _repeat(true), _rotDelta(0), _rot(0) {
	_anim = nullptr;
	_frames = 0;
	if (name == "wave") _repeat = false;
	if (name == "redcircle") _rotDelta = 2;
	if (name == "bluecircle") _rotDelta = -2;

	if (SearchMan.hasFile(name + "_0.tga")) {
		// animation
		while (SearchMan.hasFile(Common::String::format((name+"_%d.tga").c_str(),_frames)))
			_frames++;

		_anim = new Bitmap*[_frames];
		for (int i=0; i<_frames; i++)
			_anim[i] = load(Common::String::format((name+"_%d.tga").c_str(),i));
	} else {
		// single image
		if (SearchMan.hasFile(name+".tga")) {
			_frames = 1;
			_anim = new Bitmap*[1];
			_anim[0] = load(name + ".tga");
		}
	}
}

CursorData::~CursorData() {
	if (_anim != nullptr)
		delete[] _anim;
	_anim = nullptr;
}

Bitmap* CursorData::load(const Common::String& name) {
	Bitmap * bmp = Bitmap::create(name.c_str());
	bmp->_data->_smoothInterpolation = true;
	bmp->_data->_canRotate = _rotDelta != 0;
	bmp->_data->load();
	bmp->_data->_hasTransparency = true;
	return bmp;
}

void CursorData::reset() {
	_frame = 0;
	_frameTick = _rotTick = g_system->getMillis();
}

void CursorData::draw(const Common::Point& pos) {
	if (_anim == nullptr) return;

	unsigned delta = 50;
	unsigned curTick = g_system->getMillis();
	if (curTick - _frameTick > 5 * delta) {
		_frameTick = curTick;
		_frame = 0;
	}
	while (curTick - _frameTick > delta) {
		_frameTick += delta;
		_frame++;
		if (_repeat)
			_frame = _frame % _frames;
	}
	if (_frame >= _frames)
		return;

	if (_rotDelta != 0) {
		_rot += _rotDelta * 0.001 * (curTick-_rotTick);
		if (fabs(curTick-_rotTick) > 3000) _rot = 0;
		_rotTick = curTick;
		_rot = fmod(_rot + 2*M_PI, 2*M_PI);
	}

	Bitmap* bmp = _anim[_frame];
	Common::Point hs(bmp->getWidth() / 2, bmp->getHeight() / 2);
	g_driver->drawBitmap(bmp,pos.x - hs.x, pos.y - hs.y,0,_rot);
}

Cursor::Cursor(GrimEngine *vm) :
	_position(320, 210),
	_curCursor(0)
{
	for (int i=0; i<2; i++)
		_persistentCursor[i] = -1;
	_data = new CursorData*[numCursors];
	for(int i=0; i<numCursors; i++)
		_data[i] = nullptr;
	loadAvailableCursors();
	_scaleX = 1.0f/g_driver->getScaleW();
	_scaleY = 1.0f/g_driver->getScaleH();
}

Cursor::~Cursor() {
	for(int i=0; i<numCursors; i++) {
		if (_data[i]) delete _data[i];
	}
	delete[] _data;
}

void Cursor::updatePosition(Common::Point& mouse) {
	_position.x = mouse.x * _scaleX;
	_position.y = mouse.y * _scaleY;
}

void Cursor::loadAvailableCursors() {
	for(int i=0; i<numCursors; i++) {
		if (_data[i]) delete _data[i];
		_data[i] = new CursorData(cursorName[i]);
	}
	CursorMan.showMouse(false);
}

void Cursor::reload() {
	loadAvailableCursors();
}

void Cursor::setCursor(int id) {
	if (_curCursor != id && id >= 0) {
		_data[id]->reset();
	}
	_curCursor = id;
}

void Cursor::setPersistent(int pc, int id, int x, int y) {
	if (_persistentCursor[pc] != id && id >= 0) {
		_data[id]->reset();
	}
	_persistentCursor[pc] = id;
	_persistentPosition[pc].x = x;
	_persistentPosition[pc].y = y;
}

void Cursor::draw() {
	if (_curCursor >= 0)
		_data[_curCursor]->draw(_position);
	for (int i=0; i<2; i++)
		if (_persistentCursor[i] >= 0)
			_data[_persistentCursor[i]]->draw(_persistentPosition[i]);
}

} /* namespace Grim */
