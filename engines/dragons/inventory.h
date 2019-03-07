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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef DRAGONS_INVENTORY_H
#define DRAGONS_INVENTORY_H

#include "common/system.h"

namespace Dragons {

class Actor;
class ActorManager;

class Inventory {
private:
	DragonsEngine *_vm;
	int32 _sequenceId;
	int16 _screenPositionIndex;
	Actor *_actor;
	int16 _type;

public:
	Inventory(DragonsEngine *vm);

	void init(ActorManager *actorManager);
	void loadScene(uint32 sceneId);

	int32 getSequenceId() {
		return _sequenceId;
	}

	int16 getType() { return _type; }
	int16 getPositionIndex() { return _screenPositionIndex; }
	Common::Point getPosition();

	bool isVisible() {
		return _type != 0;
	}

	void hide() { _type = 0; }
	void show(int16 type) { _type = type; }

	void updateVisibility();


};

} // End of namespace Dragons

#endif //DRAGONS_INVENTORY_H
