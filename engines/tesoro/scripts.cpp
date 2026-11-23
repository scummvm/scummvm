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

#include "common/textconsole.h"

#include "tesoro/core/archive.h"
#include "tesoro/core/camera.h"
#include "tesoro/core/level.h"
#include "tesoro/core/namekey.h"
#include "tesoro/core/scenes.h"
#include "tesoro/kinds/cameraframe.h"
#include "tesoro/kinds/cursor.h"
#include "tesoro/kinds/jim.h"
#include "tesoro/scripts.h"
#include "tesoro/tesoro.h"
#include "tesoro/world.h"

namespace Tesoro {

Scripts::~Scripts() {
	for (uint i = 0; i < _tasks.size(); ++i)
		delete _tasks[i];
	delete _runner;
}

bool Scripts::load() {
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

int Scripts::start(Level *level, const char *block, const char *label, byte *variables) {
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

uint32 Scripts::now() { return _w.now(); }

Object *Scripts::jim() { return _w.jim(); }

Level *Scripts::levelFromRef(uint32 ref) { return !ref ? nullptr : ref == kInventoryRef ? _w._engine._interface : _w.level((int)ref - 1); }

Object *Scripts::findObject(Level *level, uint64 key) { return level ? level->findObject(key) : nullptr; }

void Scripts::insertScript(uint32 block, int priority) { _w._main.insertByPriorityFront(_tasks[block], priority); }

void Scripts::removeScript(uint32 block) { _w._main.remove(_tasks[block]); }

void Scripts::removeObject(Object *object) { _w._main.remove(object); }

void Scripts::setCursorActive(bool enabled) { _w.setPointerActive(enabled); }

void Scripts::say(Object *actor, uint32 phrase, uint32 extra) {
	const SpeechTable::Line &line = _w._speech.lineOf(phrase);
	if (actor == _w._jim)
		_w._jim->sayAnswer(line);
	else if (actor && actor->kindId() == 5)
		static_cast<Character *>(actor)->say(line, static_cast<Character *>(actor)->textColour(), extra);
	else
		warning("Tesoro: a script makes something other than Jim speak");
}

void Scripts::sayJim(uint32 phrase) { _w.jim()->sayNarrationLine(_w._speech.lineOf(phrase)); }

bool Scripts::isSpeaking(Object *actor) {
	if (actor == _w._jim)
		return _w._jim->speaker().speaking();
	return actor && actor->kindId() == 5 && static_cast<Character *>(actor)->speaker().speaking();
}

void Scripts::animate(Object *object, byte first, int16 last, int32 third, bool loop) {
	if (object)
		object->animation().selectBank(first, (uint16)last, third, loop, false, _w.now());
}

void Scripts::activate(Object *object, bool on) {
	if (object)
		object->setActive(on);
}

void Scripts::show(Object *object, bool on) {
	if (object)
		object->setVisible(on);
}

bool Scripts::animationBusy(Object *object) {
	if (!object)
		return false;
	object->updateBounds(_w.now());
	AnimPlayer *player = object->currentPlayer();
	return player && (player->flags() & AnimPlayer::kPlaying);
}

void Scripts::walk(Object *object, int16 x, int16 y, uint32 notifyBlock) {
	if (object == _w._jim)
		_w._jim->walkTo(x, y, _tasks[notifyBlock], false);
	else if (object && object->kindId() == 5)
		static_cast<Character *>(object)->startWalk();
}

void Scripts::look(Object *object, int8 direction) {
	if (object == _w._jim)
		_w._jim->setFacing(direction);
	else if (object && object->kindId() == 5)
		static_cast<Character *>(object)->face(direction);
}

void Scripts::releaseIfBusy(Object *character) {
	if (character && character->animation().bankCount() > 0 && character->animation().isStashed()) {
		character->animation().setStashed(false);
		character->animation().restoreStashed();
		character->animation().current().resume(_w.now());
	}
}

void Scripts::beginMenu() {
	if (!_w._cursor)
		return;
	_savedStep = _w._cursor->currentPlayer()->current();
	if (InventoryItem *held = _w.pointerItem())
		held->setVisible(true);
	_w.setPointerItem(nullptr);
	_w.cursorFrame(0, 0);
}

int Scripts::showMenu(const uint32 *phrases, uint32 count) {
	const SpeechTable::Line *lines[kMaxChoices];
	for (uint32 i = 0; i < count && i < kMaxChoices; ++i)
		lines[i] = &_w._speech.lineOf(phrases[i]);
	return DialogueChoiceMenu::run(*this, _w._engine._display, lines, count);
}

void Scripts::endMenu() {
	if (_w._engine._cursor)
		_w._engine._cursor->currentPlayer()->setCurrent(_savedStep);
}

void Scripts::boardGame(ScriptRun &run, uint32 type, uint32 ordinal) {
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

Level *Scripts::moveToLevel(ScriptRun &run, int index, int32 x, int32 y) {
	run.level = _w.level(index);
	_w.putJimInLevel(run.level, x, y);
	return run.level;
}

void Scripts::cameraToJim(Level *where) {
	if (where)
		where->cameraMoveTo(_w._jim->x(), _w._jim->y(), 0, _w.now());
}

void Scripts::rebindCharacters(ScriptRun &run) {
	const ScriptBlock &b = _library.block(run.blockIndex);
	for (uint32 i = 0; i < b.characterCount(); ++i)
		run.characters[i] = findObject(run.level, b.character(i));
}

void Scripts::playVideo(uint32 number) { _w._engine._video.playVideo((int)number); }

void Scripts::resumeJim() {
	_w._jim->stopWalking();
	_w._jim->setFacing(_w._jim->facing());
}

void Scripts::playSound(const char *name) {
	Common::String file(name);
	file.toUppercase();
	file += ".WAV";
	const uint32 offset = _w._engine._archive.findEntry(file.c_str());
	if (offset)
		_w.playVoice(offset);
	else
		warning("Tesoro: the sound %s is not in the archive", file.c_str());
}

void Scripts::playTrack(byte track) { _w.playMusicTrack(track); }

void Scripts::clearLabel() { _w.setHoverText(nullptr); }

int32 Scripts::mouseX() { return _w._engine._pointer.x; }

int32 Scripts::mouseY() { return _w._engine._pointer.y; }

bool Scripts::leftButton() { return (_w._engine._pointer.buttonState(PointerState::kLeft) & PointerState::kPressed) != 0; }

void Scripts::showCursorAt(int32 x, int32 y) {
	if (!_w._engine._cursor || !_w._engine._interface)
		return;
	_w._engine._cursor->setPosition(x, y);
	_w._engine._cursor->draw(_w._engine._interface->camera(), _w._engine._display);
}

void Scripts::removeJimFromMainList() { _w._main.remove(_w._jim); }

SceneAdapter::SceneAdapter(Scripts &scripts, ScriptRun &run) : _scripts(scripts), _w(scripts._w), _run(run) {}

void SceneAdapter::enterLevel(int level, int32 x, int32 y) { _scripts.moveToLevel(_run, level, x, y); }

void SceneAdapter::enterLevelAtJim(int level) { _scripts.moveToLevel(_run, level, _w._jim->x(), _w._jim->y()); }

void SceneAdapter::enterLevelAtFloor(int level) {
	Level *where = _w.level(level);
	const CameraFrame *frame = where ? where->cameraFrame() : nullptr;
	Object *floor = frame ? frame->target() : nullptr;
	_scripts.moveToLevel(_run, level, floor ? floor->x() : 0, floor ? floor->y() : 0);
}

void SceneAdapter::enterLevelAtObject(int level, const char *name) {
	Object *object = _w.findObject(level, name);
	_scripts.moveToLevel(_run, level, object ? object->x() : 0, object ? object->y() : 0);
}

void SceneAdapter::rebindCharacters() { _scripts.rebindCharacters(_run); }

void SceneAdapter::cameraToJim(bool blockLevel) { _scripts.cameraToJim(blockLevel ? _run.level : _w._jim->level()); }

void SceneAdapter::setJimVisible(bool visible) { _w._jim->setVisible(visible); }

void SceneAdapter::setJimActive(bool active) { _w._jim->setActive(active); }

void SceneAdapter::setJimPosition(int32 x, int32 y) { _w._jim->setPosition(x, y); }

void SceneAdapter::setJimPhase(byte phase) { _w._jimPhase = phase; }

void SceneAdapter::setJimStep(byte step) { _w._jimStep = step; }

void SceneAdapter::setObjectActive(int level, const char *name, bool active) {
	if (Object *object = _w.findObject(level, name))
		object->setActive(active);
}

void SceneAdapter::setObjectVisible(int level, const char *name, bool visible) {
	if (Object *object = _w.findObject(level, name))
		object->setVisible(visible);
}

void SceneAdapter::selectBank(int level, const char *name, int bank, int first, int last, bool loop, bool sum) {
	if (Object *object = _w.findObject(level, name))
		object->animation().selectBank(bank, first, last, loop, sum, _w.now());
}

void SceneAdapter::setSortValue(int level, const char *name, int16 value) {
	if (Object *object = _w.findObject(level, name))
		object->setSortValue(value);
}

void SceneAdapter::runDoorAction(int level, const char *name) {
	if (Object *door = _w.findObject(level, name))
		door->verb(0);
}

void SceneAdapter::fadeOut(int effect, int speed) { _w.fadeOut(effect, speed); }

void SceneAdapter::wait(int milliseconds) { _w.wait(milliseconds); }

void SceneAdapter::resetRenderQueue() { _w.resetRenderQueue(); }

void SceneAdapter::clearMainList() { _w._main.clear(); }

void SceneAdapter::clearPointerItem() { _w.clearPointerItem(); }

void Scripts::runDo(ScriptRun &run, uint32 ordinal) {
	const uint32 type = _library.block(run.blockIndex).type();
	if (type == 8 || type == 14) {
		boardGame(run, type, ordinal);
		return;
	}
	// The blocks of type 0 have no routine of their own: their DO lines only describe the scene.
	SceneAdapter scene(*this, run);
	if (!SceneRoutines::run(scene, type, ordinal) && type)
		warning("Tesoro: DO line %u of the scene routine of block type %u has no scene routine", ordinal, type);
}

void Scripts::throneScene(ScriptRun &run, int variant) {
	// To the throne room (variant 0, Jim hidden) or back to the village (variant 1, Jim shown), where he stands now.
	SceneAdapter scene(*this, run);
	SceneRoutines::run(scene, 20, (uint32)variant);
}

} // End of namespace Tesoro
