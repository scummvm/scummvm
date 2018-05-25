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

#ifndef PINK_PINK_H
#define PINK_PINK_H

#include "common/random.h"

#include "engines/engine.h"

#include "graphics/wincursor.h"

#include "gui/EventRecorder.h"
#include "gui/debugger.h"

#include "pink/constants.h"
#include "pink/director.h"
#include "pink/file.h"
#include "pink/utils.h"

/*
 *  This is the namespace of the Pink engine.
 *
 *  Status of this engine: In Development
 *
 *  Internal name of original engine: OxCart Runtime
 *
 *  Games using this engine:
 *  - The Pink Panther: Passport to Peril
 *  - The Pink Panther: Hokus Pokus Pink
 *
 *  Peril game status:
 *      Fully playable*
 *
 *  Pokus game status:
 *      Fully Playable*
 *      sequences by timer are broken (HandlerTimerSequences)
 *
 *  Known bugs:
 *      High CPU usage(no dirty rects)
 *      Walking sprites don't recalculate position
 *      Walking can't be skipped
 *      PDA doesn't work
 *
 */

namespace Pink {

class Console;
class Archive;
class NamedObject;
class Module;
class GamePage;
class LeadActor;

enum {
	kPinkDebugGeneral = 1 << 0,
	kPinkDebugLoadingResources = 1 << 1,
	kPinkDebugLoadingObjects = 1 << 2,
	kPinkDebugGraphics = 1 << 3,
	kPinkDebugSound = 1 << 4
};

class PinkEngine : public Engine {
public:
	PinkEngine(OSystem *system, const ADGameDescription *desc);
	~PinkEngine();

	virtual Common::Error run();

	virtual bool hasFeature(EngineFeature f) const;

	virtual Common::Error loadGameState(int slot);
	virtual bool canLoadGameStateCurrently();

	virtual Common::Error saveGameState(int slot, const Common::String &desc);
	virtual bool canSaveGameStateCurrently();

	void load(Archive &archive);
	void initModule(const Common::String &moduleName, const Common::String &pageName, Archive *saveFile);
	void changeScene(GamePage *page);

	OrbFile *getOrb()  { return &_orb; }
	BroFile *getBro()  { return _bro; }
	Common::RandomSource &getRnd() { return _rnd; };
	Director *getDirector() { return &_director; }

	void setNextExecutors(const Common::String &nextModule, const Common::String &nextPage);
	void setLeadActor(LeadActor *actor) { _actor = actor; };
	void setCursor(uint cursorIndex);

	void setVariable(Common::String &variable, Common::String &value);
	bool checkValueOfVariable(Common::String &variable, Common::String &value);

protected:
	virtual void pauseEngineIntern(bool pause);

private:
	Common::Error init();
	bool loadCursors();

	void loadModule(int index);

	Console *_console;
	Common::RandomSource _rnd;
	Common::Array<Graphics::WinCursorGroup *> _cursors;

	Common::String _nextModule;
	Common::String _nextPage;

	OrbFile  _orb;
	BroFile *_bro;

	Director _director;
	LeadActor *_actor;

	Module *_module;
	Array<NamedObject *> _modules;

	StringMap _variables;

	const ADGameDescription _desc;
};

Common::String generateSaveName(int slot, const char *gameId);

} // End of namespace Pink

#endif
