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

#include "graphics/surface.h"

#include "petka/flc.h"
#include "petka/walk.h"
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

	_showMap = true;

	_fxId = 0;
	_musicId = 0;
}

void QObjectBG::processMessage(const QMessage &msg) {
	QMessageObject::processMessage(msg);
	switch (msg.opcode) {
	case kSet: {
		_resourceId = msg.arg1;

		QSystem *sys = g_vm->getQSystem();
		if (g_vm->isPetka2() && !sys->_totalInit && sys->_mainInterface->_roomId == _id) {
			auto petka = sys->getPetka();
			auto chapay = sys->getChapay();

			auto bkgName = g_vm->resMgr()->findResourceName(_resourceId);

			petka->_walk->setBackground(bkgName);
			chapay->_walk->setBackground(bkgName);

			petka->setPos(Common::Point(petka->_x_, petka->_y_), false);
			chapay->setPos(Common::Point(chapay->_x_, chapay->_y_), false);
			break;
		}
	}
	case kMusic:
		_musicId = msg.arg1;
		break;
	case kBGsFX:
		_fxId = msg.arg1;
		break;
	case kMap:
		_showMap = (msg.arg1 != 0);
		break;
	case kNoMap:
		_showMap = false;
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
	Graphics::Surface *s = g_vm->resMgr()->getSurface(_resourceId);
	if (!s)
		return;
	int xOffset = g_vm->getQSystem()->_xOffset;
	for (auto rect : g_vm->videoSystem()->rects()) {
		rect.translate(xOffset, 0);
		g_vm->videoSystem()->blitFrom(*s, rect, Common::Point(rect.left - xOffset, rect.top));
	}
}

void QObjectBG::goTo() {
	QSystem *sys = g_vm->getQSystem();

	sys->getPetka()->stopWalk();
	sys->getChapay()->stopWalk();

	int oldRoomId = sys->_mainInterface->_roomId;
	sys->_mainInterface->loadRoom(_id, false);

	QMessageObject *oldRoom = sys->findObject(oldRoomId);

	Common::ScopedPtr<Common::SeekableReadStream> bgsStream(g_vm->openFile("BGs.ini", true));
	Common::INIFile bgsIni;
	bgsIni.allowNonEnglishCharacters();
	bgsIni.loadFromStream(*bgsStream);

	Common::String entranceName;
	if (bgsIni.getKey(oldRoom->_name, _name, entranceName)) {
		setEntrance(entranceName);
		return;
	}

	for (auto o : sys->_allObjects) {
		QObjectBG *bg = dynamic_cast<QObjectBG *>(o);
		if (bg && bgsIni.getKey(bg->_name, _name, entranceName)) {
			setEntrance(entranceName);
		}
	}
}

void QObjectBG::setEntrance(const Common::String &name) {
	QSystem *sys = g_vm->getQSystem();
	QMessageObject *entrance = sys->findObject(name);
	if (entrance) {
		sys->getPetka()->_z = 0;
		sys->getChapay()->_z = 0;

		sys->getPetka()->setPos(Common::Point(entrance->_walkX, entrance->_walkY), false);
		sys->getChapay()->setPos(Common::Point(entrance->_walkX, entrance->_walkY - 2), false);

		sys->_xOffset = CLIP<int32>(entrance->_walkX - 320, 0, sys->_sceneWidth - 640);
		sys->_reqOffset = sys->_xOffset;
	}
	g_vm->videoSystem()->makeAllDirty();
}

void QObjectBG::readInisData(Common::INIFile &names, Common::INIFile &cast, Common::INIFile *bgs) {
	if (bgs) {
		Common::String perspective;
		bgs->getKey(_name, "Settings", perspective);
		if (!perspective.empty()) {
			sscanf(perspective.c_str(), "%lf %lf %d %d %lf", &_persp.f0, &_persp.k, &_persp.y0, &_persp.y1, &_persp.f1);
		} else {
			_persp.f0 = 1.0;
			_persp.f1 = 1.0;
			_persp.k = 0.0;
			_persp.y0 = 0;
			_persp.y1 = 480;
		}
	}
	QMessageObject::readInisData(names, cast, bgs);
}

} // End of namespace Petka
