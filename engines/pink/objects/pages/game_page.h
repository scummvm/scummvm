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

#ifndef PINK_GAME_PAGE_H
#define PINK_GAME_PAGE_H

#include "common/memstream.h"

#include "pink/objects/pages/page.h"

namespace Pink {

class CursorMgr;
class HandlerStartPage;

class GamePage : public Page {
public:
	GamePage();
	~GamePage();
	virtual void toConsole();
	virtual void deserialize(Archive &archive);

	void loadState(Archive &archive);
	void saveState(Archive &archive);

	virtual void load(Archive &archive);
	void unload();
	void loadManagers();
	void init(bool isLoadingSave);

	Sequencer *getSequencer() { return _sequencer; }
	WalkMgr *getWalkMgr() { return _walkMgr; }
	Module *getModule() { return _module; }

	bool checkValueOfVariable(const Common::String &variable, const Common::String &value);
	void setVariable(Common::String &variable, Common::String &value);

	virtual void clear();

	friend class Console;

private:
	bool initHandler();

	void loadStateFromMem();
	void saveStateToMem();

	bool _isLoaded;
	Common::MemoryReadWriteStream *_memFile;
	Module *_module;
	CursorMgr *_cursorMgr;
	WalkMgr *_walkMgr;
	Sequencer *_sequencer;
	Array<HandlerStartPage *> _handlers;
	StringMap _variables;
};

}

#endif
