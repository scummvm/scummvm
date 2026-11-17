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
#include "tesoro/core/cameraframe.h"
#include "tesoro/core/cursor.h"
#include "tesoro/core/jim.h"
#include "tesoro/core/level.h"
#include "tesoro/core/namekey.h"
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
		else
			warning("Tesoro: a script makes something other than Jim speak");
	}

	void sayJim(uint32 phrase) { _w.jim()->sayNarrationLine(_w._speech.lineOf(phrase)); }
	bool isSpeaking(Object *actor) {
		if (actor == _w._jim)
			return _w._jim->speaker().speaking();
		return actor && actor->kindId() == 5;
	}

	void animate(Object *object, byte first, int16 last, int32 third, bool loop) {
		if (object)
			object->animation().selectBank(first, (uint16)last, third, loop, false, _w.now());
	}

	void look(Object *object, int8 direction) {
		if (object == _w._jim)
			_w._jim->setFacing(direction);
		else if (object)
			warning("Tesoro: a script turns something other than Jim");
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
		else if (object)
			warning("Tesoro: a script makes something other than Jim walk");
	}

	// An object held in a speaking pose goes back to what it was doing.
	void releaseIfBusy(Object *object) {
		if (object && object->animation().bankCount() > 0 && object->animation().isStashed()) {
			object->animation().setStashed(false);
			object->animation().restoreStashed();
			object->animation().current().resume(_w.now());
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


	void runDo(ScriptRun &run, uint32 ordinal) { warning("Tesoro: DO line %u of block %u is not ported yet", ordinal, run.blockIndex); }
	void throneScene(ScriptRun &run, int variant) { warning("Tesoro: the throne scene %d of block %u is not ported yet", variant, run.blockIndex); }
	void playVideo(uint32 number) { warning("Tesoro: movie %u cannot be played yet", number); }

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
	World &_w;
	ScriptLibrary _library;
	ScriptRunner *_runner = nullptr;
	Common::Array<ScriptTask *> _tasks;
	Common::Array<byte> _text[4];
	byte _scratch[256];
	uint16 _savedStep = 0;
};

} // End of namespace Tesoro

#endif
