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

#include "common/stream.h"

#include "petka/obj.h"
#include "petka/q_interface.h"
#include "petka/q_system.h"
#include "petka/q_manager.h"
#include "petka/sound.h"
#include "petka/petka.h"

namespace Petka {

QInterface::QInterface()
	: _objUnderCursor(nullptr) {}

QInterfaceMain::QInterfaceMain() {
	Common::ScopedPtr<Common::SeekableReadStream> stream(g_vm->openFile("backgrnd.bg", true));
	if (!stream)
		return;
	_bgs.resize(stream->readUint32LE());
	for (uint i = 0; i < _bgs.size(); ++i) {
		_bgs[i].objId = stream->readUint16LE();
		_bgs[i].attachedObjIds.resize(stream->readUint32LE());
		for (uint j = 0; j < _bgs[i].attachedObjIds.size(); ++j) {
			_bgs[i].attachedObjIds[i] = stream->readUint16LE();
			QMessageObject *obj = g_vm->getQSystem()->findObject(_bgs[i].attachedObjIds[i]);
			obj->_x = stream->readSint32LE();
			obj->_y = stream->readSint32LE();
			obj->_z = stream->readSint32LE();
			obj->_field14 = stream->readSint32LE();
			obj->_field18 = stream->readSint32LE();
		}
	}
}

const Common::Array<BGInfo> QInterfaceMain::bgInfos() {
	return _bgs;
}

void QInterfaceStartup::start() {
	g_vm->getQSystem()->update();

	QObjectBG *bg = (QObjectBG *)g_vm->getQSystem()->findObject("STARTUP");
	_objs.push_back(bg);

	Sound *s = g_vm->soundMgr()->addSound(g_vm->resMgr()->findSoundName(bg->_musicId), Audio::Mixer::SoundType::kMusicSoundType);
	s->play(true);

	const Common::Array<BGInfo> infos = g_vm->getQSystem()->_mainInterface->_bgs;

	for (uint i = 0; i < infos.size(); ++i) {
		if (infos[i].objId != bg->_id) {
			continue;
		}
		QMessageObject *obj = g_vm->getQSystem()->findObject(infos[i].objId);
		obj->_z = 1;
		obj->_x = 0;
		obj->_y = 0;
		obj->_field24 = 1;
		obj->_field20 = 1;
		obj->_field28 = 1;
		obj->_animate = 1;
		obj->_isShown = 0;
		_objs.push_back(obj);
	}
}

} // End of namespace Petka
