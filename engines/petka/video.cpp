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

const uint kShakeTime = 30;
const int kShakeOffset = 3;

VideoSystem::VideoSystem() :
	_shake(false), _shift(false), _shakeTime(0), _time(0) {
	makeAllDirty();
	_time = g_system->getMillis();
}

void VideoSystem::update() {
	Interface *interface = g_vm->getQSystem()->_currInterface;
	uint32 time = g_system->getMillis();
	if (interface) {
		for (uint i = 0; i < interface->_objs.size(); ++i) {
			interface->_objs[i]->update(time - _time);
		}

		for (uint i = 0; i < interface->_objs.size(); ++i) {
			interface->_objs[i]->updateZ();
		}

		sort();

		for (uint i = 0; i < interface->_objs.size(); ++i) {
			interface->_objs[i]->draw();
		}
	}

	_time = time;
	_rects.clear();

	if (_shake) {
		g_system->setShakePos(_shift ? kShakeOffset : 0, 0);
		time = g_system->getMillis();
		if (time - _shakeTime > kShakeTime) {
			_shift = !_shift;
			_shakeTime = time;
		}
	}
	_screen.update();
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

void VideoSystem::setShake(bool shake) {
	_shake = shake;
	g_system->setShakePos(0, 0);
}

void VideoSystem::sort() {
	Common::Array<QVisibleObject *> &objs = g_vm->getQSystem()->_currInterface->_objs;
	for (uint i = 0; i < objs.size() - 1; ++i) {
		int minIndex = i;
		for (uint j = i + 1; j < objs.size(); ++j) {
			if (objs[j]->_z < objs[minIndex]->_z) {
				minIndex = j;
			}
		}

		if (i != minIndex) {
			QVisibleObject *tmp = objs[i];
			objs[i] = objs[minIndex];
			objs[minIndex] = tmp;
		}
	}
}

}
