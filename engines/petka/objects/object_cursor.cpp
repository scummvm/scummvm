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

#include "common/ini-file.h"
#include "common/stream.h"
#include "common/system.h"
#include "common/events.h"

#include "graphics/colormasks.h"
#include "graphics/surface.h"

#include "petka/flc.h"
#include "petka/petka.h"
#include "petka/video.h"
#include "petka/q_system.h"
#include "petka/q_manager.h"
#include "petka/objects/object_cursor.h"

namespace Petka {

QObjectCursor::QObjectCursor() {
	_id = 4097;
	_z = 1000;
	_resourceId = 5002;
	Common::Point pos = g_vm->getEventManager()->getMousePos();
	_x = pos.x;
	_y = pos.y;
	g_vm->resMgr()->loadFlic(5002);
	_actionType = kLook;
}

void QObjectCursor::draw() {
	if (!_isShown) {
		return;
	}
	FlicDecoder *flc = g_vm->resMgr()->loadFlic(_resourceId);
	const Graphics::Surface *frame = flc->getCurrentFrame();
	if (frame) {
		Graphics::Surface *s = frame->convertTo(g_system->getScreenFormat(), flc->getPalette());
		Common::Rect srcRect(flc->getBounds());
		srcRect.translate(_x, _y);
		srcRect.clip(640, 480);
		Common::Rect destRect(srcRect);
		srcRect.translate(-_x, -_y);
		g_vm->videoSystem()->screen().transBlitFrom(*s, srcRect, destRect, flc->getTransColor(s->format));
		s->free();
		delete s;
	}
}

void QObjectCursor::update(int time) {
	if (!_isShown || !_animate)
		return;
	FlicDecoder *flc = g_vm->resMgr()->loadFlic(_resourceId);
	_time += time;
	while (flc && _time >= flc->getDelay()) {
		flc->setFrame(-1);
		Common::Rect dirty(flc->getBounds());
		dirty.translate(_x, _y);
		g_vm->videoSystem()->addDirtyRect(dirty);
		_time -= flc->getDelay();
	}
}

void QObjectCursor::setCursorPos(int x, int y, bool center) {
	FlicDecoder *flc = g_vm->resMgr()->loadFlic(_resourceId);
	if (!_animate) {
		flc->setFrame(1);
	}

	Common::Rect dirty(flc->getBounds());
	dirty.translate(_x, _y);
	g_vm->videoSystem()->addDirtyRect(dirty);

	if (center) {
		x = x - flc->getBounds().left - dirty.width() / 2;
		y = y - flc->getBounds().top - dirty.height() / 2;
	}

	_x = x;
	_y = y;

	dirty = flc->getBounds();
	dirty.translate(_x, _y);
	g_vm->videoSystem()->addDirtyRect(dirty);
}

void QObjectCursor::show(bool v) {
	FlicDecoder *flc = g_vm->resMgr()->loadFlic(_resourceId);
	Common::Rect rect = flc->getBounds();
	rect.translate(_x, _y);
	g_vm->videoSystem()->addDirtyRect(rect);
	QMessageObject::show(v);
}

}
