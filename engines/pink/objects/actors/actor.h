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

#ifndef PINK_ACTOR_H
#define PINK_ACTOR_H

#include "common/rect.h"

#include "pink/utils.h"

namespace Pink {

class Page;
class Action;
class Sequencer;
class Director;
class CursorMgr;
class InventoryItem;
class InventoryMgr;

class Actor : public NamedObject {
public:
	Actor();
	~Actor() override;

	void deserialize(Archive &archive) override;

	void loadState(Archive &archive);
	void saveState(Archive &archive);

	virtual void init(bool paused);
	bool initPalette(Director *director);

	void toConsole() const override;

	bool isPlaying() const { return !_isActionEnded; }
	virtual void pause(bool paused);

	void endAction() { _isActionEnded = true; }

	virtual bool isSupporting() const { return false; }
	virtual bool isCursor() const { return false; }

	virtual bool isLeftClickHandlers() const { return false; }
	virtual bool isUseClickHandlers(InventoryItem *item) const { return false; }

	virtual void onMouseOver(Common::Point point, CursorMgr *mgr);
	virtual void onMouseOverWithItem(Common::Point point, const Common::String &itemName, CursorMgr *cursorMgr);

	virtual void onTimerMessage() {}
	virtual void onLeftClickMessage() {}
	virtual void onUseClickMessage(InventoryItem *item, InventoryMgr *mgr) {}

	Action *findAction(const Common::String &name);

	Action *getAction() { return _action; }
	const Action *getAction() const { return _action; }
	Page *getPage() { return _page; }
	const Page *getPage() const { return _page; }

	InventoryMgr *getInventoryMgr() const;

	virtual Common::String getPDALink() const;

	virtual Common::String getLocation() const;

	void setAction(const Common::String &name) { setAction(findAction(name)); }
	void setAction(Action *newAction);
	void setAction(Action *newAction, bool loadingSave);

protected:
	Page *_page;

	Action *_action;
	Array<Action *> _actions;

	bool _isActionEnded;
};

} // End of namespace Pink

#endif
