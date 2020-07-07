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

#ifndef PINK_SUPPORTING_ACTOR_H
#define PINK_SUPPORTING_ACTOR_H

#include "pink/objects/actors/actor.h"
#include "pink/objects/handlers/handler_mgr.h"

namespace Pink {

class InventoryItem;
class InventoryMgr;

class SupportingActor : public Actor {
public:
	void deserialize(Archive &archive) override;

	void toConsole() const override;

	bool isSupporting() const override { return true; }

	bool isLeftClickHandlers() const override;
	bool isUseClickHandlers(InventoryItem *item) const override;

	void onMouseOver(Common::Point point, CursorMgr *mgr) override;
	void onMouseOverWithItem(Common::Point point, const Common::String &itemName, CursorMgr *cursorMgr) override;

	void onTimerMessage() override;
	void onLeftClickMessage() override;
	void onUseClickMessage(InventoryItem *item, InventoryMgr *mgr) override;

	Common::String getPDALink() const override;
	Common::String getLocation() const override;

private:
	HandlerMgr _handlerMgr;

	Common::String _location;
	Common::String _pdaLink;
	Common::String _cursor;
};

} // End of namespace Pink

#endif
