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

#ifndef PINK_GAME_PAGE_H
#define PINK_GAME_PAGE_H

#include "common/memstream.h"

#include "pink/objects/pages/page.h"

namespace Pink {

class CursorMgr;
class HandlerSequences;

class GamePage : public Page {
public:
	GamePage();
	~GamePage() override;
	void toConsole() const override;
	void deserialize(Archive &archive) override;

	void loadState(Archive &archive);
	void saveState(Archive &archive);

	void load(Archive &archive) override;
	void unload();
	void loadManagers();
	void init(bool isLoadingSave);

	Sequencer *getSequencer() override { return _sequencer; }
	WalkMgr *getWalkMgr() override { return _walkMgr; }
	Module *getModule() override { return _module; }
	const Module *getModule() const override { return _module; }

	bool checkValueOfVariable(const Common::String &variable, const Common::String &value) const override;
	void setVariable(Common::String &variable, Common::String &value) override;

	void clear() override;

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
	Array<HandlerSequences *> _handlers;
	StringMap _variables;
};

}

#endif
