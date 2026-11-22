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

#include "tesoro/core/display.h"
#include "tesoro/core/namekey.h"
#include "tesoro/core/scriptblock.h"
#include "tesoro/core/scriptrunner.h"
#include "tesoro/scripts.h"

namespace Tesoro {

// A spoken line is followed by this long a pause before the script goes on.
static const uint32 kSpeechPause = 201;
static const int kScriptPriority = 30;

ScriptRunner::ScriptRunner(Scripts &scripts, const ScriptLibrary &library) : _scripts(scripts), _library(library) {
	_runs.resize(library.blockCount());
	for (uint32 i = 0; i < library.blockCount(); ++i) {
		_runs[i].blockIndex = i;
		_runs[i].characters.resize(library.block(i).characterCount());
	}
}

const ScriptBlock &ScriptRunner::block(const ScriptRun &run) const {
	return _library.block(run.blockIndex);
}

Object *ScriptRunner::character(ScriptRun &run, int32 index) {
	Object *&cached = run.characters[index];
	if (!cached)
		cached = _scripts.findObject(run.level, block(run).character(index));
	return cached;
}

int ScriptRunner::start(uint32 blockIndex, uint64 label, byte *variables, Level *level) {
	ScriptRun &r = _runs[blockIndex];
	const ScriptBlock &b = block(r);
	for (uint32 i = 0; i < b.characterCount(); ++i)
		r.characters[i] = _scripts.findObject(level, b.character(i));
	r.menuStart = -1;
	r.pc = b.findLabel(label);
	r.level = level;
	r.variables = variables;
	_scripts.insertScript(blockIndex, kScriptPriority);
	_scripts.setCursorActive(false);
	execute(r);
	return 3;
}

void ScriptRunner::resume(uint32 blockIndex) {
	ScriptRun &r = _runs[blockIndex];
	_scripts.removeObject(r.actor);
	_scripts.resumeJim();
	++r.pc;
	execute(r);
}

int ScriptRunner::finish(ScriptRun &r) {
	_scripts.removeScript(r.blockIndex);
	_scripts.setCursorActive(true);
	for (uint32 i = 0; i < r.characters.size(); ++i) {
		if (r.characters[i])
			_scripts.releaseIfBusy(r.characters[i]);
	}
	return 3;
}

int ScriptRunner::collectMenu(ScriptRun &r) {
	const ScriptBlock &b = block(r);
	_menuCount = 0;
	for (;;) {
		_menuShown = false;
		const ScriptEntry &item = b.entry(r.pc);
		const int8 variable = (int8)item.byteAt(6);
		bool listed;
		if (variable < 0)
			listed = true;
		else if (item.byteAt(7) == 0)
			listed = r.variables[variable] == 1;
		else
			listed = r.variables[variable] == 0;
		if (listed) {
			// The choice's text is the line after the item.
			if (_menuCount == kMaxChoices)
				error("Script block %u: more than %u choices in a menu", r.blockIndex, kMaxChoices);
			_menuPhrases[_menuCount] = b.entry(r.pc + 1).word(4);
			_menuResume[_menuCount] = r.pc + 2;
			++_menuCount;
		}
		const int16 next = (int16)item.half(4);
		if (next < 1) {
			r.actor = _scripts.jim();
			return 3;
		}
		r.pc = next;
	}
}

int ScriptRunner::execute(ScriptRun &r) {
	const ScriptBlock &b = block(r);
	for (;;) {
		const ScriptEntry &e = b.entry(r.pc);
		switch (e.op()) {
		case kScriptReturn:
		case kScriptEnd:
			return finish(r);
		case kScriptSay: {
			_speechEnd = 0;
			const int32 who = (int32)e.word(8);
			if (who > 0) {
				r.actor = character(r, who - 1);
				_scripts.say(r.actor, e.word(4), e.word(12));
			} else {
				r.actor = _scripts.jim();
				_scripts.sayJim(e.word(4));
			}
			return 3;
		}
		case kScriptSleep:
			r.sleepStart = _scripts.now();
			return 2;
		case kScriptBreak:
			if (r.menuStart < 0)
				return finish(r);
			r.pc = r.menuStart;
			continue;
		case kScriptJump:
			r.pc = e.word(4);
			continue;
		case kScriptMenuBegin:
			r.menuStart = r.pc;
			++r.pc;
			return collectMenu(r);
		case kScriptMenuItem:
			// Reached by walking into a menu from outside: the item is skipped.
			++r.pc;
			continue;
		case kScriptIfEquals:
			if (r.variables[e.byteAt(8)] == e.byteAt(9)) {
				r.pc = e.word(4);
				continue;
			}
			++r.pc;
			continue;
		case kScriptRun:
			if (e.word(4) != 0) {
				start(e.word(4) - 1, e.key(8), r.variables, r.level);
				return 3;
			}
			_scripts.throneScene(r, e.key(8) == nameKey64("ir") ? 0 : 1);
			++r.pc;
			continue;
		case kScriptSendMessage: {
			Level *level = e.word(12) ? _scripts.levelFromRef(e.word(12)) : r.level;
			Object *object = _scripts.findObject(level, e.key(4));
			r.actor = object;
			switch (e.word(16)) {
			case kMessagePlay: {
				const byte mode = e.byteAt(22);
				const int32 third = mode == 0 ? (int8)e.byteAt(21) : -1;
				_scripts.animate(object, e.byteAt(20), (int8)e.byteAt(21), third, mode == 2);
				if (mode == 3)
					return 3;
				break;
			}
			case kMessageWalk:
				_scripts.walk(object, (int16)e.half(20), (int16)e.half(22), r.blockIndex);
				return 3;
			case kMessageLook:
				_scripts.look(object, (int8)e.byteAt(20));
				break;
			case kMessageActivate:
				_scripts.activate(object, e.byteAt(20) != 0);
				break;
			case kMessageShow:
				_scripts.show(object, e.byteAt(20) != 0);
				break;
			default:
				break;
			}
			++r.pc;
			continue;
		}
		case kScriptSetVariable:
			r.variables[e.byteAt(4)] = e.byteAt(5);
			++r.pc;
			continue;
		case kScriptDo:
			_scripts.runDo(r, e.word(4));
			++r.pc;
			continue;
		case kScriptPlayVideo:
			_scripts.playVideo(e.word(4));
			++r.pc;
			continue;
		case kScriptPlaySound:
			_scripts.playSound((const char *)e.data + 4);
			++r.pc;
			continue;
		case kScriptPlayTrack:
			_scripts.playTrack(e.byteAt(4));
			++r.pc;
			continue;
		default:
			// The original loops forever on a command it cannot run.
			error("Script block %u: command %u at %u cannot be run", r.blockIndex, e.op(), r.pc);
		}
	}
}

int ScriptRunner::tickMenu(ScriptRun &r) {
	if (!_menuShown) {
		_scripts.beginMenu();
		_menuChoice = _scripts.showMenu(_menuPhrases, _menuCount);
		if (_menuChoice >= 0) {
			_menuShown = true;
			r.actor = _scripts.jim();
			_scripts.say(r.actor, _menuPhrases[_menuChoice], 0);
		}
	} else if (!_scripts.isSpeaking(_scripts.jim())) {
		_scripts.endMenu();
		r.pc = _menuResume[_menuChoice];
		execute(r);
		return 3;
	}
	return 2;
}

int ScriptRunner::tick(uint32 blockIndex) {
	ScriptRun &r = _runs[blockIndex];
	const ScriptEntry &e = block(r).entry(r.pc);
	const uint32 before = r.pc;

	switch (e.op()) {
	case kScriptSay:
		if (_scripts.isSpeaking(r.actor))
			return 2;
		if (_speechEnd == 0)
			_speechEnd = _scripts.now();
		if (_scripts.now() - _speechEnd < kSpeechPause)
			return 2;
		++r.pc;
		break;
	case kScriptSleep:
		if (_scripts.now() - r.sleepStart < e.word(4))
			return 2;
		++r.pc;
		break;
	case kScriptMenuBegin:
	case kScriptMenuItem:
		return tickMenu(r);
	case kScriptRun:
		_scripts.setCursorActive(false);
		++r.pc;
		break;
	case kScriptSendMessage:
		// Only an animation order waits, and only while the animation runs.
		if (e.word(16) != 0 || _scripts.animationBusy(r.actor))
			return 2;
		++r.pc;
		break;
	default:
		break;
	}
	if (r.pc != before)
		return execute(r);
	return 2;
}

int DialogueChoiceMenu::run(Scripts &scripts, Display &display, const SpeechTable::Line *const *lines, uint32 count) {
	static const uint32 kWhite = 0xffff, kYellow = 0xffe0;
	static const uint16 kLayer = 5;

	scripts.clearLabel();
	scripts.showCursorAt(scripts.mouseX() + 320, 240 - scripts.mouseY());

	TextQueue &texts = display.texts();
	const uint32 first = texts.lineCount();
	Common::Array<int32> top;
	int32 y = 50;
	for (uint32 i = 0; i < count; ++i) {
		top.push_back(y);
		display.queueText((const byte *)lines[i]->str(), kWhite, 320, y, 640, kLayer, false, 0x12);
		y = (texts.lineCount() ? texts.line(texts.lineCount() - 1).y : 0) + 30;
	}
	top.push_back(480);

	// Push the block down so that it ends near the bottom of the screen.
	const int32 shift = 465 - y;
	texts.shiftLines(first, shift);
	top[0] += shift;
	if (count < 1)
		return -1;

	for (uint32 i = 0; i < count; ++i) {
		top[i + 1] += shift;
		const int32 mouseY = scripts.mouseY();
		if (top[i] - 240 <= mouseY && mouseY < top[i + 1] - 240) {
			if (scripts.leftButton()) {
				scripts.removeJimFromMainList();
				return i;
			}
			display.queueText((const byte *)lines[i]->str(), kYellow, 320, top[i], 640, kLayer, false, 0x12);
		}
	}
	return -1;
}

} // End of namespace Tesoro
