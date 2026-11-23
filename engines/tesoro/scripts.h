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

#ifndef TESORO_SCRIPTS_H
#define TESORO_SCRIPTS_H

#include "common/array.h"

#include "tesoro/core/object.h"
#include "tesoro/core/scriptblock.h"
#include "tesoro/core/scriptrunner.h"
#include "tesoro/core/walkarea.h"

namespace Tesoro {

class Level;
class World;

/** What a running script block is in the main list: its turn runs the block. */
class ScriptTask : public Object, public WalkTarget {
public:
	ScriptTask(ScriptRunner &runner, uint32 block) : _runner(runner), _block(block) { initPlain(); }
	int update() override { return _runner.tick(_block); }
	int arrived() override {
		_runner.resume(_block);
		return 3;
	}

private:
	ScriptRunner &_runner;
	uint32 _block;
};


/** The scripts: the compiled blocks, the runner, and everything a running block asks of the game. */
class Scripts {
public:
	enum { kMaxChoices = 20 };

	explicit Scripts(World &world) : _w(world) {}
	~Scripts();

	bool load();

	int start(Level *level, const char *block, const char *label, byte *variables);

	uint32 now();
	Object *jim();
	Level *levelFromRef(uint32 ref);
	Object *findObject(Level *level, uint64 key);
	// A new script goes in front of equal priorities, so a script it starts waits before the one that started it.
	void insertScript(uint32 block, int priority);
	void removeScript(uint32 block);
	void removeObject(Object *object);
	void setCursorActive(bool enabled);
	void say(Object *actor, uint32 phrase, uint32 extra);

	void sayJim(uint32 phrase);
	bool isSpeaking(Object *actor);

	void animate(Object *object, byte first, int16 last, int32 third, bool loop);

	void activate(Object *object, bool on);
	void show(Object *object, bool on);

	bool animationBusy(Object *object);

	void walk(Object *object, int16 x, int16 y, uint32 notifyBlock);

	void look(Object *object, int8 direction);

	// An object held in a speaking pose goes back to what it was doing.
	void releaseIfBusy(Object *character);

	void beginMenu();

	int showMenu(const uint32 *phrases, uint32 count);

	void endMenu();

	void runDo(ScriptRun &run, uint32 ordinal);

	void boardGame(ScriptRun &run, uint32 type, uint32 ordinal);

	Level *moveToLevel(ScriptRun &run, int index, int32 x, int32 y);
	/** Places the level's camera on Jim at once. */
	void cameraToJim(Level *where);
	/** Looks the block's characters up again after a level change: the cached ones belong to the old level. */
	void rebindCharacters(ScriptRun &run);

	void playVideo(uint32 number);

	void resumeJim();

	void playSound(const char *name);

	void playTrack(byte track);
	void throneScene(ScriptRun &run, int variant);
	// Choice menu
	void clearLabel();
	int32 mouseX();
	int32 mouseY();
	bool leftButton();
	void showCursorAt(int32 x, int32 y);
	void removeJimFromMainList();

	const ScriptLibrary &library() const { return _library; }

private:
	friend struct SceneAdapter;
	World &_w;
	ScriptLibrary _library;
	ScriptRunner *_runner = nullptr;
	Common::Array<ScriptTask *> _tasks;
	Common::Array<byte> _text[4];
	byte _scratch[256];
	uint16 _savedStep = 0;
};


/** The scene routines' view of the game, for the block that is running. */
struct SceneAdapter {
	SceneAdapter(Scripts &scripts, ScriptRun &run);
	void enterLevel(int level, int32 x, int32 y);
	void enterLevelAtJim(int level);
	void enterLevelAtFloor(int level);
	void enterLevelAtObject(int level, const char *name);
	void rebindCharacters();

	void cameraToJim(bool blockLevel);
	void setJimVisible(bool visible);
	void setJimActive(bool active);
	void setJimPosition(int32 x, int32 y);
	void setJimPhase(byte phase);
	void setJimStep(byte step);
	void setObjectActive(int level, const char *name, bool active);
	void setObjectVisible(int level, const char *name, bool visible);
	void selectBank(int level, const char *name, int bank, int first, int last, bool loop, bool sum);
	void setSortValue(int level, const char *name, int16 value);
	void runDoorAction(int level, const char *name);
	void fadeOut(int effect, int speed);
	void wait(int milliseconds);
	void resetRenderQueue();
	void clearMainList();
	void clearPointerItem();

private:
	Scripts &_scripts;
	World &_w;
	ScriptRun &_run;
};

} // End of namespace Tesoro

#endif
