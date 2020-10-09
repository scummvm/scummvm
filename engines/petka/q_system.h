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
#include "common/events.h"
#include "common/stream.h"
#include "common/list.h"
#include "common/hashmap.h"
#include "common/hash-str.h"

#include "petka/objects/object_bg.h"

namespace Petka {

class PetkaEngine;
class QObjectCase;
class QObjectCursor;
class QObjectStar;
class QObjectPetka;
class QObjectChapayev;
class InterfaceSaveLoad;
class InterfaceSequence;
class InterfaceMain;
class InterfaceStartup;
class InterfacePanel;
class InterfaceMap;
class Interface;

class QSystem {
public:
	QSystem(PetkaEngine &vm);
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

	QObjectPetka *getPetka() const;
	QObjectChapayev *getChapay() const;
	QObjectCursor *getCursor() const;
	QObjectCase *getCase() const;
	QObjectStar *getStar() const;

	void startSaveLoad(int id);

	void togglePanelInterface();
	void toggleMapInterface();
	void toggleCase();

	void goPrevInterface();

	void setCursorAction(int action);

	void onEvent(const Common::Event &event);

public:
	PetkaEngine &_vm;

	Common::Array<QMessageObject *> _allObjects;
	Common::List<QMessage> _messages;
	Common::ScopedPtr<InterfaceMain> _mainInterface;
	Common::ScopedPtr<InterfaceSaveLoad> _saveLoadInterface;
	Common::ScopedPtr<InterfaceSequence> _sequenceInterface;
	Common::ScopedPtr<InterfaceStartup> _startupInterface;
	Common::ScopedPtr<InterfacePanel> _panelInterface;
	Common::ScopedPtr<InterfaceMap> _mapInterface;
	Interface *_currInterface;
	Interface *_prevInterface;

	bool _totalInit;
	int _fxId;
	int _musicId;

	int _sceneWidth;
	int _xOffset;
	int _reqOffset;

	QObjectBG *_room;
};

} // End of namespace Petka

#endif
