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

#ifndef TESORO_CORE_SCRIPTRUNNER_H
#define TESORO_CORE_SCRIPTRUNNER_H

#include "common/array.h"
#include "common/scummsys.h"

#include "tesoro/core/speechtable.h"

namespace Tesoro {

class Scripts;

class Level;
class Display;
class Object;
class ScriptLibrary;
class ScriptBlock;
struct ScriptRun;

/**
 * The list of dialogue choices of a script menu, one frame of it: the choices are laid out from the top, pushed down so that the
 * block ends near the bottom of the screen, the one under the mouse is drawn in yellow, and a click on one returns it.
 */
class DialogueChoiceMenu {
public:
	/** Returns the number of the chosen line, or -1 while none is clicked. */
	static int run(Scripts &scripts, Display &display, const SpeechTable::Line *const *lines, uint32 count);
};

/** What one script block keeps while it runs. */
struct ScriptRun {
	uint32 blockIndex = 0;
	Level *level = nullptr;
	byte *variables = nullptr;			// the game's variable bytes, shared by every script
	uint32 pc = 0;						// the command being run or waited for
	int32 menuStart = -1;				// where the menu being shown begins, -1 for none
	Object *actor = nullptr;			// the object whose speech or animation the script waits for
	Common::Array<Object *> characters;	// the block's characters, found when it started
	uint32 sleepStart = 0;
};

/** Runs the compiled script blocks. */
class ScriptRunner {
public:
	ScriptRunner(Scripts &scripts, const ScriptLibrary &library);

	ScriptRun &run(uint32 blockIndex) { return _runs[blockIndex]; }

	/** Starts a block at a label. Returns 3. */
	int start(uint32 blockIndex, uint64 label, byte *variables, Level *level);
	/** One frame of a running script. Returns 2 while it waits, 3 when it did something. */
	int tick(uint32 blockIndex);
	/** The walk a script waited for has ended: goes on with the next command. */
	void resume(uint32 blockIndex);

private:
	int execute(ScriptRun &run);
	int finish(ScriptRun &run);
	int collectMenu(ScriptRun &run);
	int tickMenu(ScriptRun &run);
	Object *character(ScriptRun &run, int32 index);
	const ScriptBlock &block(const ScriptRun &run) const;

	Scripts &_scripts;
	const ScriptLibrary &_library;
	Common::Array<ScriptRun> _runs;

	// Shared by every script, as in the original.
	uint32 _speechEnd = 0;
	bool _menuShown = false;
	int32 _menuChoice = 0;
	static const uint32 kMaxChoices = 10;
	uint32 _menuCount = 0;
	uint32 _menuPhrases[kMaxChoices] = {};
	uint32 _menuResume[kMaxChoices] = {};
};

} // End of namespace Tesoro

#endif
