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

#ifndef PINK_PDA_MGR_H
#define PINK_PDA_MGR_H

#include "pink/cursor_mgr.h"
#include "pink/utils.h"

namespace Pink {

class PinkEngine;
class Actor;
class LeadActor;
class PDAPage;

struct Command;

class PDAMgr {
public:
	PDAMgr(PinkEngine *game);

	void loadState(Archive &archive) { _savedPage = archive.readString(); }
	void saveState(Archive &archive);

	void execute(const Command &command);
	void goToPage(const Common::String &pageName);

	void update() { _cursorMgr.update(); }

	void onLeftButtonClick(Common::Point point);
	void onMouseMove(Common::Point point);

	PinkEngine *getGame() const { return _game; }
	const Common::String &getSavedPageName() { return _savedPage; }

	void setLead(LeadActor *lead) { _lead = lead; }

private:
	void close();
	void loadGlobal();

	PinkEngine *_game;
	LeadActor *_lead;
	PDAPage *_page;
	CursorMgr _cursorMgr;
	Array<Actor *> _globalActors;
	Common::String _savedPage;
};

} // End of namespace Pink

#endif
