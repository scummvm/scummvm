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
	~Actor();

	void deserialize(Archive &archive) override;

	void loadState(Archive &archive);
	void saveState(Archive &archive);

	virtual void init(bool unk);
	bool initPallete(Director *director);

	void toConsole() override ;

	bool isPlaying();
	virtual void pause(bool paused);

	void hide();
	void endAction();

	virtual bool isLeftClickHandlers();
	virtual bool isUseClickHandlers(InventoryItem *item);

	virtual void onMouseOver(const Common::Point point, CursorMgr *mgr);
	virtual void onHover(const Common::Point point, const Common::String &itemName, CursorMgr *cursorMgr);

	virtual void onClick();

	virtual void onTimerMessage();
	virtual bool onLeftClickMessage();
	virtual bool onUseClickMessage(InventoryItem *item, InventoryMgr *mgr);

	Action *findAction(const Common::String &name);

	Action *getAction() const;
	Page *getPage() const;
	Sequencer *getSequencer() const;

	virtual const Common::String &getLocation() const;

	void setAction(const Common::String &name);
	void setAction(Action *newAction);
	void setAction(Action *newAction, bool unk);

	void setPage(Page *page);

protected:
	Page *_page;

	Action *_action;
	Array<Action *> _actions;

	bool _isActionEnded;
};

} // End of namespace Pink

#endif
