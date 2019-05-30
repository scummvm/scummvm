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

#include "petka/q_object.h"
#include "petka/q_object_bg.h"

namespace Petka {

class PetkaEngine;

class QSystem {
public:
	explicit QSystem(PetkaEngine &vm);
	~QSystem();

	bool init();

	void addMessage(const QMessage &msg);
	void addMessage(uint16 objId, uint16 opcode, int16 arg1 = 0, int16 arg2 = 0, int16 arg3 = 0, int16 unk1 = 0, int16 unk2 = 0);
	void addMessageForAllObjects(uint16 opcode, int16 arg1 = 0, int16 arg2 = 0, int16 arg3 = 0, int16 unk1 = 0, int16 unk2 = 0);

private:
	PetkaEngine &_vm;
	Common::Array<QObject> _objs;
	Common::Array<QObjectBG> _bgs;
	Common::Array<QMessageObject *> _allObjects;
	Common::Array<QMessage> _messages;
};

} // End of namespace Petka

#endif
