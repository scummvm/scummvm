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

#ifndef PETKA_Q_SYSTEM_H
#define PETKA_Q_SYSTEM_H

#include "common/ptr.h"
#include "common/stream.h"
#include "common/list.h"

#include "petka/objects/object_bg.h"

namespace Petka {

class PetkaEngine;
class QObjectCase;
class QObjectCursor;
class QObjectStar;
class QObjectPetka;
class QObjectChapayev;
class InterfaceSaveLoad;
class InterfaceMain;
class InterfaceStartup;
class InterfacePanel;
class InterfaceMap;
class Interface;

class QSystem {
public:
	explicit QSystem();
	~QSystem();

	bool init();
	void load(Common::ReadStream *s);
	void save(Common::WriteStream *s);

	void update();

	void addMessage(const QMessage &msg);
	void addMessage(uint16 objId, uint16 opcode, int16 arg1 = 0, int16 arg2 = 0, int16 arg3 = 0, int32 unk = 0, QMessageObject *sender = nullptr);
	void addMessageForAllObjects(uint16 opcode, int16 arg1 = 0, int16 arg2 = 0, int16 arg3 = 0, int32 unk = 0, QMessageObject *sender = nullptr);

	QMessageObject *findObject(int16 id);
	QMessageObject *findObject(const Common::String &name);

	void togglePanelInterface();
	void toggleMapInterface();

	void setChapayev();

public:
	Common::Array<QObject> _objs;
	Common::Array<QObjectBG> _bgs;
	Common::Array<QMessageObject *> _allObjects;
	Common::List<QMessage> _messages;
	Common::ScopedPtr<QObjectPetka> _petka;
	Common::ScopedPtr<QObjectChapayev> _chapayev;
	Common::ScopedPtr<QObjectCursor> _cursor;
	Common::ScopedPtr<QObjectCase> _case;
	Common::ScopedPtr<QObjectStar> _star;
	Common::ScopedPtr<InterfaceMain> _mainInterface;
	Common::ScopedPtr<InterfaceSaveLoad> _saveLoadInterface;
	Common::ScopedPtr<InterfaceStartup> _startupInterface;
	Common::ScopedPtr<InterfacePanel> _panelInterface;
	Common::ScopedPtr<InterfaceMap> _mapInterface;
	Interface *_currInterface;
	Interface *_prevInterface;

	int _isIniting;
	int _fxId;
	int _musicId;

	QObjectBG *_room;
};

} // End of namespace Petka

#endif
