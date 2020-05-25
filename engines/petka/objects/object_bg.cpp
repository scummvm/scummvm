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
#include "petka/interfaces/sequence.h"
#include "petka/objects/object_bg.h"
#include "petka/objects/heroes.h"

namespace Petka {

QObjectBG::QObjectBG() {
	_x = 0;
	_y = 0;
	_z = 0;

	_showMap = 1;

	_fxId = 0;
	_musicId = 0;
}

void QObjectBG::processMessage(const QMessage &msg) {
	QMessageObject::processMessage(msg);
	switch (msg.opcode) {
	case kSet:
		_resourceId = msg.arg1;
		break;
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
		goTo();
		break;
	case kSetSeq:
		g_vm->getQSystem()->_sequenceInterface->start(_id);
		break;
	case kEndSeq:
		g_vm->getQSystem()->_sequenceInterface->stop();
		break;
	default:
		break;
	}

}

void QObjectBG::draw() {
	Graphics::Surface *s = g_vm->resMgr()->loadBitmap(_resourceId);
	if (s) {
		const Common::List<Common::Rect> &dirty = g_vm->videoSystem()->rects();
		for (Common::List<Common::Rect>::const_iterator it = dirty.begin(); it != dirty.end(); ++it) {
			g_vm->videoSystem()->blitFrom(*s, *it, Common::Point(it->left, it->top));
		}
	}
}

void QObjectBG::goTo() {
	QSystem *sys = g_vm->getQSystem();

	sys->_petka->stopWalk();
	sys->_chapayev->stopWalk();

	int oldRoomId = sys->_mainInterface->_roomId;
	sys->_mainInterface->loadRoom(_id, false);

	QMessageObject *oldRoom = sys->findObject(oldRoomId);

	Common::ScopedPtr<Common::SeekableReadStream> bgsStream(g_vm->openFile("BGs.ini", false));
	Common::INIFile bgsIni;
	bgsIni.allowNonEnglishCharacters();
	bgsIni.loadFromStream(*bgsStream);

	Common::String entranceName;
	if (bgsIni.getKey(oldRoom->_name, _name, entranceName)) {
		setEntrance(entranceName);
		return;
	}

	Common::Array<QObjectBG> &bgs = sys->_bgs;
	for (uint i = 0; i < bgs.size(); ++i) {
		if (bgsIni.getKey(bgs[i]._name, _name, entranceName)) {
			setEntrance(entranceName);
			break;
		}
	}
}

void QObjectBG::setEntrance(const Common::String &name) {
	QSystem *sys = g_vm->getQSystem();
	QMessageObject *entrance = sys->findObject(name);
	if (entrance) {
		sys->_petka->_z = 0;
		sys->_chapayev->_z = 0;

		sys->_petka->setPos(entrance->_walkX, entrance->_walkY);
		sys->_chapayev->setPos(entrance->_walkX, entrance->_walkY - 2);

		sys->_xOffset = CLIP<int32>(entrance->_walkX - 320, 0, sys->_sceneWidth - 640);
		sys->_field6C = sys->_xOffset;
	}
	g_vm->videoSystem()->makeAllDirty();
}

}
