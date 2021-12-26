/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef PINK_HANDLER_MGR_H
#define PINK_HANDLER_MGR_H

#include "pink/objects/object.h"

namespace Pink {

class InventoryItem;
class InventoryMgr;

class Handler;
class HandlerLeftClick;
class HandlerUseClick;
class HandlerTimer;

class Actor;

class HandlerMgr : public Object {
public:
	~HandlerMgr() override;
	void deserialize(Archive &archive) override;

	void toConsole() const override;

	HandlerUseClick *findSuitableHandlerUseClick(const Actor *actor, const Common::String &itemName) const;
	HandlerLeftClick *findSuitableHandlerLeftClick(const Actor *actor) const;

	void onTimerMessage(Actor *actor);
	void onLeftClickMessage(Actor *actor);
	void onUseClickMessage(Actor *actor, InventoryItem *item, InventoryMgr *mgr);

private:
	Handler *findSuitableHandlerTimer(const Actor *actor);

	Array<HandlerLeftClick *> _leftClickHandlers;
	Array<HandlerUseClick *> _useClickHandlers;
	Array<Handler *> _timerHandlers;
};

}

#endif
