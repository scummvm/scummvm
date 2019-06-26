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
#include "petka/interfaces/main.h"
#include "petka/objects/object_bg.h"

namespace Petka {

void QObjectBG::processMessage(const QMessage &msg) {
	QMessageObject::processMessage(msg);
	switch (msg.opcode) {
	case kSet:
		_resourceId = msg.arg1;
	case kMusic:
		_musicId = msg.arg1;
		break;
	case kBGsFX:
		_fxId = msg.arg1;
		break;
	case kMap:
		_showMap = msg.arg1 != 0;
		break;
	case kNoMap:
		_showMap = 0;
		break;
	case kGoTo:
		g_vm->getQSystem()->_mainInterface->loadRoom(_id, false);
		g_vm->videoSystem()->addDirtyRect(Common::Rect(640, 480));
		break;
	case kSetSeq:
		break;
	case kEndSeq:
		break;
	}

}

void QObjectBG::draw() {
	Graphics::Surface *s = g_vm->resMgr()->loadBitmap(_resourceId);
	if (s) {
		const Common::List<Common::Rect> &dirty = g_vm->videoSystem()->rects();
		for (Common::List<Common::Rect>::const_iterator it = dirty.begin(); it != dirty.end(); ++it) {
			g_vm->videoSystem()->screen().blitFrom(*s, *it, Common::Point(it->left, it->top));
		}
	}
}

}
