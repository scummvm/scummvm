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

#include "common/stack.h"

#include "pink/cursor_mgr.h"
#include "pink/utils.h"

namespace Pink {

class PinkEngine;
class LeadActor;
class PDAPage;

struct Command;

class PDAMgr {
public:
	PDAMgr(PinkEngine *game);
	~PDAMgr();

	void loadState(Archive &archive) { _savedPage = archive.readString(); }
	void saveState(Archive &archive);

	void execute(const Command &command);
	void goToPage(const Common::String pageName);

	void update() { _cursorMgr.update(); }

	void close();

	void onLeftButtonClick(Common::Point point);
	void onLeftButtonUp();
	void onMouseMove(Common::Point point);

	PinkEngine *getGame() const { return _game; }
	const Common::String &getSavedPageName() { return _savedPage; }

	void setLead(LeadActor *lead) { _lead = lead; }

private:
	void loadGlobal();

	void initPerilButtons();

	void updateWheels(bool playSfx = 0);
	void updateLocator();

	void calculateIndexes();

	static bool isNavigate(const Common::String &name);
	static bool isDomain(const Common::String &name);

private:
	PinkEngine *_game;
	LeadActor *_lead;
	PDAPage *_page;
	PDAPage *_globalPage;
	CursorMgr _cursorMgr;
	Common::String _savedPage;
	Common::Stack<Common::String> _previousPages;
	byte _countryIndex;
	byte _domainIndex;
	byte _iteration;
	byte _handFrame;
	enum LeftHandAction {
		kLeft1,
		kLeft2,
		kLeft3,
		kLeft4
	} _leftHandAction;
};

} // End of namespace Pink

#endif
