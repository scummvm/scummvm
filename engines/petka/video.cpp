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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/system.h"

#include "petka/sound.h"
#include "petka/flc.h"
#include "petka/petka.h"
#include "petka/q_system.h"
#include "petka/interfaces/main.h"
#include "petka/interfaces/dialog_interface.h"
#include "petka/objects/object.h"
#include "petka/video.h"

namespace Petka {

VideoSystem::VideoSystem() :
	_shake(false), _shift(false), _shakeTime(0), _time(0) {
	makeAllDirty();
	_time = g_system->getMillis();
}

static bool objCmp(QVisibleObject *&l, QVisibleObject *&r) {
	return l->_z < r->_z;
}

void VideoSystem::update() {
	Interface *interface = g_vm->getQSystem()->_currInterface;
	int time = g_system->getMillis();
	if (interface) {
		for (uint i = 0; i < interface->_objs.size(); ++i) {
			interface->_objs[i]->update(time - _time);
		}

		for (uint i = 0; i < interface->_objs.size(); ++i) {
			interface->_objs[i]->updateZ();
		}

		Common::sort(interface->_objs.begin(), interface->_objs.end(), objCmp);

		for (uint i = 0; i < interface->_objs.size(); ++i) {
			interface->_objs[i]->draw();
		}
	}
	_time = time;
	_rects.clear();

	if (_shake) {
		int width = _screen.w;
		int x =  0;

		if (_shift) {
			Graphics::Surface s;
			s.create(3, 480, g_system->getScreenFormat());
			g_system->copyRectToScreen(s.getPixels(), s.pitch, 0, 0, s.w, s.h);
			width -= 3;
			x = 3;
		}

		g_system->copyRectToScreen(_screen.getPixels(), _screen.pitch, x, 0, width, _screen.h);
		g_system->updateScreen();

		uint32 time = g_system->getMillis();
		if (time - _shakeTime > 30) {
			_shift = !_shift;
			_shakeTime = time;
		}
	} else {
		_screen.update();
	}
}

void VideoSystem::addDirtyRect(const Common::Rect &rect) {
	_rects.push_back(rect);
}

const Common::List<Common::Rect> VideoSystem::rects() const {
	return _rects;
}

Graphics::Screen &VideoSystem::screen() {
	return _screen;
}

void VideoSystem::addDirtyRect(Common::Point pos, FlicDecoder &flc) {
	Common::Rect rect = flc.getBounds();
	rect.translate(pos.x, pos.y);
	addDirtyRect(rect);
}

void VideoSystem::addDirtyRectFromMsk(Common::Point pos, FlicDecoder &flc) {
	const Common::Array<Common::Rect> &rects = flc.getMskRects();
	for (uint i = 0; i < rects.size(); ++i) {
		Common::Rect r = rects[i];
		r.translate(pos.x, pos.y);
		_rects.push_back(r);
	}
}

void VideoSystem::makeAllDirty() {
	addDirtyRect(Common::Rect(640, 480));
}

void VideoSystem::updateTime() {
	_time = g_system->getMillis();
}

}