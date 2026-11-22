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

#include "common/textconsole.h"
#include "tesoro/core/archive.h"
#include "tesoro/core/camera.h"
#include "tesoro/kinds/cameraframe.h"
#include "tesoro/kinds/cursor.h"
#include "tesoro/kinds/jim.h"
#include "tesoro/core/level.h"
#include "tesoro/core/namekey.h"
#include "tesoro/core/scenes.h"
#include "tesoro/core/scriptblock.h"
#include "tesoro/core/scriptrunner.h"
#include "tesoro/world.h"
#include "tesoro/tesoro.h"

namespace Tesoro {

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
	~Scripts() {
		for (uint i = 0; i < _tasks.size(); ++i)
			delete _tasks[i];
		delete _runner;
	}

	bool load() {
		static const char *const kFiles[] = { "ANIMS.DLG", "CONVERS.DLG", "ICONS.DLG", "OBJETOS.DLG" };
		for (uint i = 0; i < 4; ++i) {
			const uint32 offset = _w._engine._archive.findEntry(kFiles[i]);
			if (!offset) {
				warning("Tesoro: %s is not in the archive", kFiles[i]);
				return false;
			}
			Common::SeekableReadStream &dat = *_w._engine._archive.seekEntry(offset - 4);
			const uint32 size = dat.readUint32LE();
			_text[i].resize(size);
			if (dat.read(_text[i].data(), size) != size || !_library.load(_text[i].data(), size, _w._speech)) {
				warning("Tesoro: %s could not be compiled", kFiles[i]);
				return false;
			}
		}
		_library.resolve();
		_runner = new ScriptRunner(*this, _library);
		for (uint32 i = 0; i < _library.blockCount(); ++i)
			_tasks.push_back(new ScriptTask(*_runner, i));
		return true;
	}

	int start(Level *level, const char *block, const char *label, byte *variables) {
		const ScriptBlock *found = _library.find(nameKey64(block));
		if (!found) {
			warning("Tesoro: there is no script block %s", block);
			return 0;
		}
		uint32 index = 0;
		while (&_library.block(index) != found)
			++index;
		if (!variables) {
			memset(_scratch, 0, sizeof(_scratch));
			variables = _scratch;
		}
		return _runner->start(index, nameKey64(label), variables, level);
	}

	uint32 now() { return _w.now(); }
	Object *jim() { return _w.jim(); }
	Level *levelFromRef(uint32 ref) { return !ref ? nullptr : ref == kInventoryRef ? _w._engine._interface : _w.level((int)ref - 1); }
	Object *findObject(Level *level, uint64 key) { return level ? level->findObject(key) : nullptr; }
	// A new script goes in front of equal priorities, so a script it starts waits before the one that started it.
	void insertScript(uint32 block, int priority) { _w._main.insertByPriorityFront(_tasks[block], priority); }
	void removeScript(uint32 block) { _w._main.remove(_tasks[block]); }
	void removeObject(Object *object) { _w._main.remove(object); }
	void setCursorActive(bool enabled) { _w.setPointerActive(enabled); }
	void say(Object *actor, uint32 phrase, uint32 extra) {
		const SpeechTable::Line &line = _w._speech.lineOf(phrase);
		if (actor == _w._jim)
			_w._jim->sayAnswer(line);
		else if (actor && actor->kindId() == 5)
			static_cast<Character *>(actor)->say(line, static_cast<Character *>(actor)->textColour(), extra);
		else
			warning("Tesoro: a script makes something other than Jim speak");
	}

	void sayJim(uint32 phrase) { _w.jim()->sayNarrationLine(_w._speech.lineOf(phrase)); }
	bool isSpeaking(Object *actor) {
		if (actor == _w._jim)
			return _w._jim->speaker().speaking();
		return actor && actor->kindId() == 5 && static_cast<Character *>(actor)->speaker().speaking();
	}

	void animate(Object *object, byte first, int16 last, int32 third, bool loop) {
		if (object)
			object->animation().selectBank(first, (uint16)last, third, loop, false, _w.now());
	}

	void activate(Object *object, bool on) {
		if (object)
			object->setActive(on);
	}
	void show(Object *object, bool on) {
		if (object)
			object->setVisible(on);
	}

	bool animationBusy(Object *object) {
		if (!object)
			return false;
		object->updateBounds(_w.now());
		AnimPlayer *player = object->currentPlayer();
		return player && (player->flags() & AnimPlayer::kPlaying);
	}

	void walk(Object *object, int16 x, int16 y, uint32 notifyBlock) {
		if (object == _w._jim)
			_w._jim->walkTo(x, y, _tasks[notifyBlock], false);
		else if (object && object->kindId() == 5)
			static_cast<Character *>(object)->startWalk();
	}

	void look(Object *object, int8 direction) {
		if (object == _w._jim)
			_w._jim->setFacing(direction);
		else if (object && object->kindId() == 5)
			static_cast<Character *>(object)->face(direction);
	}

	// An object held in a speaking pose goes back to what it was doing.
	void releaseIfBusy(Object *character) {
		if (character && character->animation().bankCount() > 0 && character->animation().isStashed()) {
			character->animation().setStashed(false);
			character->animation().restoreStashed();
			character->animation().current().resume(_w.now());
		}
	}

	void beginMenu() {
		Cursor *cursor = _w._engine._cursor;
		if (!cursor)
			return;
		_savedStep = cursor->currentPlayer()->current();
		// The held item is put back here once the inventory exists (9e).
		cursor->animation().selectBank(0, 0, 0, false, false, _w.now());
	}

	int showMenu(const uint32 *phrases, uint32 count) {
		const SpeechTable::Line *lines[kMaxChoices];
		for (uint32 i = 0; i < count && i < kMaxChoices; ++i)
			lines[i] = &_w._speech.lineOf(phrases[i]);
		return DialogueChoiceMenu::run(*this, _w._engine._display, lines, count);
	}

	void endMenu() {
		if (_w._engine._cursor)
			_w._engine._cursor->currentPlayer()->setCurrent(_savedStep);
	}

	void runDo(ScriptRun &run, uint32 ordinal);

	void boardGame(ScriptRun &run, uint32 type, uint32 ordinal) {
		Jim *player = _w.jim();
		if (type == 8) {
			if (ordinal == 0) {
				moveToLevel(run, 14, player->x(), player->y());
				player->setActive(false);
				_w.setPointerActive(true);
				_w._game.start(_w, run.variables);
				_w.startScriptAt(run.level, "FCONECTA", "main", run.variables);
			} else if (ordinal == 1) {
				_w.setPointerActive(false);
			}
		} else if (ordinal == 0) {
			_w._game.turn(_w, run.variables);
		} else if (ordinal == 1) {
			moveToLevel(run, 9, player->x(), player->y());
			_w.setPointerActive(false);
			player->setActive(true);
		}
	}

	Level *moveToLevel(ScriptRun &run, int index, int32 x, int32 y) {
		run.level = _w.level(index);
		_w.putJimInLevel(run.level, x, y);
		return run.level;
	}
	/** Places the level's camera on Jim at once. */
	void cameraToJim(Level *where) {
		if (where)
			where->cameraMoveTo(_w._jim->x(), _w._jim->y(), 0, _w.now());
	}
	/** Looks the block's characters up again after a level change: the cached ones belong to the old level. */
	void rebindCharacters(ScriptRun &run) {
		const ScriptBlock &b = _library.block(run.blockIndex);
		for (uint32 i = 0; i < b.characterCount(); ++i)
			run.characters[i] = findObject(run.level, b.character(i));
	}

	void playVideo(uint32 number) { _w._engine._video.playVideo((int)number); }

	void resumeJim() {
		_w._jim->stopWalking();
		_w._jim->setFacing(_w._jim->facing());
	}

	void playSound(const char *name) {
		Common::String file(name);
		file.toUppercase();
		file += ".WAV";
		const uint32 offset = _w._engine._archive.findEntry(file.c_str());
		if (offset)
			_w.playVoice(offset);
		else
			warning("Tesoro: the sound %s is not in the archive", file.c_str());
	}

	void playTrack(byte track) { _w.playMusicTrack(track); }
	void throneScene(ScriptRun &run, int variant);
	// Choice menu
	void clearLabel() { _w.setHoverText(nullptr); }
	int32 mouseX() { return _w._engine._pointer.x; }
	int32 mouseY() { return _w._engine._pointer.y; }
	bool leftButton() { return (_w._engine._pointer.buttonState(PointerState::kLeft) & PointerState::kPressed) != 0; }
	void showCursorAt(int32 x, int32 y) {
		if (!_w._engine._cursor || !_w._engine._interface)
			return;
		_w._engine._cursor->setPosition(x, y);
		_w._engine._cursor->draw(_w._engine._interface->camera(), _w._engine._display);
	}
	void removeJimFromMainList() { _w._main.remove(_w._jim); }

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
	SceneAdapter(Scripts &scripts, ScriptRun &run) : _s(scripts), _w(scripts._w), _run(run) {}
	void enterLevel(int level, int32 x, int32 y) { _s.moveToLevel(_run, level, x, y); }
	void enterLevelAtJim(int level) { _s.moveToLevel(_run, level, _w._jim->x(), _w._jim->y()); }
	void enterLevelAtFloor(int level) {
		Level *where = _w.level(level);
		const CameraFrame *frame = where ? where->cameraFrame() : nullptr;
		Object *floor = frame ? frame->target() : nullptr;
		_s.moveToLevel(_run, level, floor ? floor->x() : 0, floor ? floor->y() : 0);
	}
	void enterLevelAtObject(int level, const char *name) {
		Object *object = _w.findObject(level, name);
		_s.moveToLevel(_run, level, object ? object->x() : 0, object ? object->y() : 0);
	}
	void rebindCharacters() { _s.rebindCharacters(_run); }

	void cameraToJim(bool blockLevel) { _s.cameraToJim(blockLevel ? _run.level : _w._jim->level()); }
	void setJimVisible(bool visible) { _w._jim->setVisible(visible); }
	void setJimActive(bool active) { _w._jim->setActive(active); }
	void setJimPosition(int32 x, int32 y) { _w._jim->setPosition(x, y); }
	void setJimPhase(byte phase) { _w._jimPhase = phase; }
	void setJimStep(byte step) { _w._jimStep = step; }
	void setObjectActive(int level, const char *name, bool active) {
		if (Object *object = _w.findObject(level, name))
			object->setActive(active);
	}
	void setObjectVisible(int level, const char *name, bool visible) {
		if (Object *object = _w.findObject(level, name))
			object->setVisible(visible);
	}
	void selectBank(int level, const char *name, int bank, int first, int last, bool loop, bool sum) {
		if (Object *object = _w.findObject(level, name))
			object->animation().selectBank(bank, first, last, loop, sum, _w.now());
	}
	void setSortValue(int level, const char *name, int16 value) {
		if (Object *object = _w.findObject(level, name))
			object->setSortValue(value);
	}
	void runDoorAction(int level, const char *name) {
		if (Object *door = _w.findObject(level, name))
			door->verb(0);
	}
	void fadeOut(int effect, int speed) { _w.fadeOut(effect, speed); }
	void wait(int milliseconds) { _w.wait(milliseconds); }
	void resetRenderQueue() { _w.resetRenderQueue(); }
	void clearMainList() { _w._main.clear(); }
	void clearPointerItem() { _w.clearPointerItem(); }

private:
	Scripts &_s;
	World &_w;
	ScriptRun &_run;
};

inline void Scripts::runDo(ScriptRun &run, uint32 ordinal) {
	const uint32 type = _library.block(run.blockIndex).type();
	if (type == 8 || type == 14) {
		boardGame(run, type, ordinal);
		return;
	}
	// The blocks of type 0 have no routine of their own: their DO lines only describe the scene.
	SceneAdapter scene(*this, run);
	if (!SceneRoutines::run(scene, type, ordinal) && type)
		warning("Tesoro: DO line %u of the scene routine of block type %u is not ported yet", ordinal, type);
}

inline void Scripts::throneScene(ScriptRun &run, int variant) {
	// To the throne room (variant 0, Jim hidden) or back to the village (variant 1, Jim shown), where he stands now.
	SceneAdapter scene(*this, run);
	SceneRoutines::run(scene, 20, (uint32)variant);
}


} // End of namespace Tesoro


#endif
