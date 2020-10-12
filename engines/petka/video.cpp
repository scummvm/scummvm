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

#include "petka/flc.h"
#include "petka/petka.h"
#include "petka/q_system.h"
#include "petka/interfaces/interface.h"
#include "petka/video.h"

namespace Petka {

const uint kShakeTime = 30;
const int kShakeOffset = 3;

VideoSystem::VideoSystem(PetkaEngine &vm)
	: _vm(vm) {
	_shakeTime = 0;
	_time = g_system->getMillis();
	_shake = false;
	_shift = false;
	_allowAddingRects = false;
}

void VideoSystem::update() {
	QSystem *sys = _vm.getQSystem();
	Interface *interface = sys->_currInterface;
	uint32 time = g_system->getMillis();

	assert(sys);
	assert(interface);

	interface->update(time - _time);

	mergeDirtyRects();

	_allowAddingRects = false;
	interface->draw();
	_allowAddingRects = true;

	for (Common::Rect &r : _dirtyRects) {
		const byte *srcP = (const byte *)getBasePtr(r.left, r.top);
		g_system->copyRectToScreen(srcP, pitch, r.left, r.top, r.width(), r.height());
	}

	_dirtyRects.clear();

	_time = time;

	if (_shake) {
		g_system->setShakePos(_shift ? kShakeOffset : 0, 0);
		if (time - _shakeTime > kShakeTime) {
			_shift = !_shift;
			_shakeTime = time;
		}
	}

	g_system->updateScreen();
}

void VideoSystem::addDirtyRect(const Common::Rect &rect) {
	if (_allowAddingRects) {
		Graphics::Screen::addDirtyRect(rect);
	}
}

void VideoSystem::addDirtyRect(Common::Point pos, Common::Rect rect) {
	rect.translate(pos.x, pos.y);
	addDirtyRect(rect);
}

void VideoSystem::addDirtyRect(Common::Point pos, FlicDecoder &flc) {
	pos.x = pos.x - _vm.getQSystem()->_xOffset;
	addDirtyRect(pos, flc.getBounds());
}

void VideoSystem::addDirtyMskRects(Common::Point pos, FlicDecoder &flc) {
	for (auto rect : flc.getMskRects()) {
		addDirtyRect(pos, rect);
	}
}

void VideoSystem::addDirtyMskRects(FlicDecoder &flc) {
	addDirtyMskRects(Common::Point(0, 0), flc);
}

const Common::List<Common::Rect> &VideoSystem::rects() const {
	return _dirtyRects;
}

void VideoSystem::updateTime() {
	_time = g_system->getMillis();
}

void VideoSystem::setShake(bool shake) {
	_shake = shake;
	g_system->setShakePos(0, 0);
}

} // End of namespace Petka
