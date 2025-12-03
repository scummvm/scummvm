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

#include "common/config-manager.h"
#include "common/file.h"
#include "common/system.h"
#include "zvision/detection.h"
#include "zvision/graphics/render_manager.h"
#include "zvision/scripting/script_manager.h"
#include "zvision/text/subtitle_manager.h"
#include "zvision/text/text.h"

namespace ZVision {

SubtitleManager::SubtitleManager(ZVision *engine, const ScreenLayout layout, const Graphics::PixelFormat pixelFormat, bool doubleFPS) :
	_engine(engine),
	_system(engine->_system),
	_renderManager(engine->getRenderManager()),
	_pixelFormat(pixelFormat),
	_textOffset(layout.workingArea.origin() - layout.textArea.origin()),
	_textArea(layout.textArea.width(), layout.textArea.height()),
	_redraw(false),
	_doubleFPS(doubleFPS),
	_subId(0) {
}

SubtitleManager::~SubtitleManager() {
	// Delete all subtitles referenced in subslist
	for (auto &it : _subsList)
		delete it._value;
}

void SubtitleManager::process(int32 deltatime) {
	for (SubtitleMap::iterator it = _subsList.begin(); it != _subsList.end(); it++) {
		// Update all automatic subtitles
		if (it->_value->selfUpdate())
			_redraw = true;
		// Update all subtitles' respective deletion timers
		if (it->_value->process(deltatime)) {
			debugC(4, kDebugSubtitle, "Deleting subtitle, subId=%d", it->_key);
			_subsFocus.remove(it->_key);
			delete it->_value;
			_subsList.erase(it);
			_redraw = true;
		}
	}
	if (_subsList.size() == 0)
		if (_subId != 0) {
			debugC(4, kDebugSubtitle, "Resetting subId to 0");
			_subId = 0;
			_subsFocus.clear();
		}
	if (_redraw) {
		debugC(4, kDebugSubtitle, "Redrawing subtitles");
		// Blank subtitle buffer
		_renderManager->clearTextSurface();
		// Render just the most recent subtitle
		if (_subsFocus.size()) {
			uint16 curSub = _subsFocus.front();
			debugC(4, kDebugSubtitle, "Rendering subtitle %d", curSub);
			Subtitle *sub = _subsList[curSub];
			if (sub->_lineId >= 0) {
				Graphics::Surface textSurface;
				//TODO - make this surface a persistent member of the manager; only call create() when currently displayed subtitle is changed.
				textSurface.create(sub->_textArea.width(), sub->_textArea.height(), _engine->_resourcePixelFormat);
				textSurface.fillRect(Common::Rect(sub->_textArea.width(), sub->_textArea.height()), (uint32)-1); // TODO Unnecessary operation?  Check later.
				_engine->getTextRenderer()->drawTextWithWordWrapping(sub->_lines[sub->_lineId].subStr, textSurface, _engine->isWidescreen());
				_renderManager->blitSurfaceToText(textSurface, sub->_textArea.left, sub->_textArea.top, -1);
				textSurface.free();
				sub->_redraw = false;
			}
		}
		_redraw = false;
	}
}

void SubtitleManager::update(int32 count, uint16 subid) {
	if (_subsList.contains(subid))
		if (_subsList[subid]->update(count)) {
			// _subsFocus.set(subid);
			_redraw = true;
		}
}

uint16 SubtitleManager::create(const Common::Path &subname, bool vob) {
	_subId++;
	debugC(2, kDebugSubtitle, "Creating scripted subtitle, subId=%d", _subId);
	_subsList[_subId] = new Subtitle(_engine, subname, vob);
	_subsFocus.set(_subId);
	return _subId;
}

uint16 SubtitleManager::create(const Common::Path &subname, Audio::SoundHandle handle) {
	_subId++;
	debugC(2, kDebugSubtitle, "Creating scripted subtitle, subId=%d", _subId);
	_subsList[_subId] = new AutomaticSubtitle(_engine, subname, handle);
	_subsFocus.set(_subId);
	return _subId;
}

uint16 SubtitleManager::create(const Common::String &str) {
	_subId++;
	debugC(2, kDebugSubtitle, "Creating simple subtitle, subId=%d, message %s", _subId, str.c_str());
	_subsList[_subId] = new Subtitle(_engine, str, _textArea);
	_subsFocus.set(_subId);
	return _subId;
}

void SubtitleManager::destroy(uint16 id) {
	if (_subsList.contains(id)) {
		debugC(2, kDebugSubtitle, "Marking subtitle %d for immediate deletion", id);
		_subsList[id]->_toDelete = true;
	}
}

void SubtitleManager::destroy(uint16 id, int16 delay) {
	if (_subsList.contains(id)) {
		debugC(2, kDebugSubtitle, "Marking subtitle %d for deletion in %dms", id, delay);
		_subsList[id]->_timer = delay;
	}
}

void SubtitleManager::timedMessage(const Common::String &str, uint16 milsecs) {
	uint16 msgid = create(str);
	debugC(1, kDebugSubtitle, "initiating timed message: %s to subtitle id %d, time %d", str.c_str(), msgid, milsecs);
	update(0, msgid);
	process(0);
	destroy(msgid, milsecs);
}

bool SubtitleManager::askQuestion(const Common::String &str, bool streaming, bool safeDefault) {
	uint16 msgid = create(str);
	debugC(1, kDebugSubtitle, "initiating user question: %s to subtitle id %d", str.c_str(), msgid);
	update(0, msgid);
	process(0);
	if(streaming)
		_renderManager->renderSceneToScreen(true,true,true);
	else
		_renderManager->renderSceneToScreen(true);
	_engine->stopClock();
	int result = 0;
	while (result == 0) {
		Common::Event evnt;
		while (_engine->getEventManager()->pollEvent(evnt)) {
			switch (evnt.type) {
				case Common::EVENT_CUSTOM_ENGINE_ACTION_START:
					if ((ZVisionAction)evnt.customType != kZVisionActionQuit)
						break;
					// fall through
				case Common::EVENT_QUIT:
					debugC(1, kDebugEvent, "Attempting to quit within quit dialog!");
					_engine->quit(false);
					return safeDefault;
					break;
				case Common::EVENT_KEYDOWN:
					// English: yes/no
					// German: ja/nein
					// Spanish: si/no
					// French Nemesis: F4/any other key  _engine(engine),
					// French ZGI: oui/non
					// TODO: Handle this using the keymapper
					switch (evnt.kbd.keycode) {
					case Common::KEYCODE_y:
						if (_engine->getLanguage() == Common::EN_ANY)
							result = 2;
						break;
					case Common::KEYCODE_j:
						if (_engine->getLanguage() == Common::DE_DEU)
							result = 2;
						break;
					case Common::KEYCODE_s:
						if (_engine->getLanguage() == Common::ES_ESP)
							result = 2;
						break;
					case Common::KEYCODE_o:
						if (_engine->getLanguage() == Common::FR_FRA && _engine->getGameId() == GID_GRANDINQUISITOR)
							result = 2;
						break;
					case Common::KEYCODE_F4:
						if (_engine->getLanguage() == Common::FR_FRA && _engine->getGameId() == GID_NEMESIS)
							result = 2;
						break;
					case Common::KEYCODE_n:
						result = 1;
						break;
					default:
						if (_engine->getLanguage() == Common::FR_FRA && _engine->getGameId() == GID_NEMESIS)
							result = 1;
						break;
					}
					break;
				default:
					break;
			}
		}
		if(streaming)
			_renderManager->renderSceneToScreen(true,true,false);
		else
			_renderManager->renderSceneToScreen(true);
		if (_doubleFPS)
			_system->delayMillis(33);
		else
			_system->delayMillis(66);
	}
	destroy(msgid);
	_engine->startClock();
	return result == 2;
}

void SubtitleManager::delayedMessage(const Common::String &str, uint16 milsecs) {
	uint16 msgid = create(str);
	debugC(1, kDebugSubtitle, "initiating delayed message: %s to subtitle id %d, delay %dms", str.c_str(), msgid, milsecs);
	update(0, msgid);
	process(0);
	_renderManager->renderSceneToScreen(true);
	_engine->stopClock();

	uint32 stopTime = _system->getMillis() + milsecs;
	while (_system->getMillis() < stopTime) {
		Common::Event evnt;
		while (_engine->getEventManager()->pollEvent(evnt)) {
			switch (evnt.type) {
			case Common::EVENT_KEYDOWN:
				switch (evnt.kbd.keycode) {
				case Common::KEYCODE_SPACE:
				case Common::KEYCODE_RETURN:
				case Common::KEYCODE_ESCAPE:
					goto skip_delayed_message;
					break;
				default:
					break;
				}
				break;
			case Common::EVENT_CUSTOM_ENGINE_ACTION_START:
				if ((ZVisionAction)evnt.customType != kZVisionActionQuit)
					break;
				// fall through
			case Common::EVENT_QUIT:
				if (ConfMan.hasKey("confirm_exit") && ConfMan.getBool("confirm_exit"))
					_engine->quit(true);
				else
					_engine->quit(false);
				break;
			default:
				break;
			}
		}
		skip_delayed_message:
		
		_renderManager->renderSceneToScreen(true);
		if (_doubleFPS)
			_system->delayMillis(17);
		else
			_system->delayMillis(33);
	}
	destroy(msgid);
	_engine->startClock();
}

void SubtitleManager::showDebugMsg(const Common::String &msg, int16 delay) {
	uint16 msgid = create(msg);
	debugC(1, kDebugSubtitle, "initiating in-game debug message: %s to subtitle id %d, delay %dms", msg.c_str(), msgid, delay);
	update(0, msgid);
	process(0);
	destroy(msgid, delay);
}

Subtitle::Subtitle(ZVision *engine, const Common::Path &subname, bool vob) :
	_engine(engine),
	_lineId(-1),
	_timer(-1),
	_toDelete(false),
	_redraw(false) {
	Common::File subFile;
	Common::Point _textOffset = _engine->getSubtitleManager()->getTextOffset();
	if (!subFile.open(subname)) {
		warning("Failed to open subtitle %s", subname.toString().c_str());
		_toDelete = true;
		return;
	}
	// Parse subtitle parameters from script
	while (!subFile.eos()) {
		Common::String str = subFile.readLine();
		if (str.lastChar() == '~')
			str.deleteLastChar();
		if (str.matchString("*Initialization*", true)) {
			// Not used
		} else if (str.matchString("*Rectangle*", true)) {
			int32 x1, y1, x2, y2;
			if (sscanf(str.c_str(), "%*[^:]:%d %d %d %d", &x1, &y1, &x2, &y2) == 4) {
				_textArea = Common::Rect(x1, y1, x2, y2);
				debugC(1, kDebugSubtitle, "Original subtitle script rectangle coordinates: l%d, t%d, r%d, b%d", x1, y1, x2, y2);
				// Original game subtitle scripts appear to define subtitle rectangles relative to origin of working area.
				// To allow arbitrary aspect ratios, we need to instead place these relative to origin of text area.
				// This will allow the managed text area to then be arbitrarily placed on the screen to suit different aspect ratios.
				_textArea.translate(_textOffset.x, _textOffset.y);  // Convert working area coordinates to text area coordinates
				debugC(1, kDebugSubtitle, "Text area coordinates: l%d, t%d, r%d, b%d", _textArea.left, _textArea.top, _textArea.right, _textArea.bottom);
			}
		} else if (str.matchString("*TextFile*", true)) {
			char filename[64];
			if (sscanf(str.c_str(), "%*[^:]:%s", filename) == 1) {
				Common::File txtFile;
				if (txtFile.open(Common::Path(filename))) {
					while (!txtFile.eos()) {
						Common::String txtline = readWideLine(txtFile).encode();
						Line curLine;
						curLine.start = -1;
						curLine.stop = -1;
						curLine.subStr = txtline;
						_lines.push_back(curLine);
					}
					txtFile.close();
				}
			}
		} else {
			int32 st; // Line start time
			int32 en; // Line end time
			int32 sb; // Line number
			if (sscanf(str.c_str(), "%*[^:]:(%d,%d)=%d", &st, &en, &sb) == 3) {
				if (sb <= (int32)_lines.size()) {
					if (vob) {
						// Convert frame number from 15FPS (AVI) to 29.97FPS (VOB) to synchronise with video
						// st = st * 2997 / 1500;
						// en = en * 2997 / 1500;
						st = st * 2900 / 1500;  // TODO: Subtitles only synchronise correctly at 29fps, but vob files should be 29.97fps; check if video codec is rounding this value down!
						en = en * 2900 / 1500;
					}
					_lines[sb].start = st;
					_lines[sb].stop = en;
				}
			}
		}
	}
	subFile.close();
}

Subtitle::Subtitle(ZVision *engine, const Common::String &str, const Common::Rect &textArea) :
	_engine(engine),
	_lineId(-1),
	_timer(-1),
	_toDelete(false),
	_redraw(false) {
	_textArea = textArea;
	debugC(1, kDebugSubtitle, "Text area coordinates: l%d, t%d, r%d, b%d", _textArea.left, _textArea.top, _textArea.right, _textArea.bottom);
	Line curLine;
	curLine.start = -1;
	curLine.stop = 0;
	curLine.subStr = str;
	_lines.push_back(curLine);
}

Subtitle::~Subtitle() {
	_lines.clear();
}

bool Subtitle::process(int32 deltatime) {
	if (_timer != -1) {
		_timer -= deltatime;
		if (_timer <= 0)
			_toDelete = true;
	}
	return _toDelete;
}

bool Subtitle::update(int32 count) {
	int16 j = -1;
	// Search all lines to find first line that encompasses current time/framecount, set j to this
	for (uint16 i = (_lineId >= 0 ? _lineId : 0); i < _lines.size(); i++)
		if (count >= _lines[i].start && count <= _lines[i].stop) {
			j = i;
			break;
		}
	if (j == -1) {
		// No line exists for current time/framecount
		if (_lineId != -1) {
			// Line is set
			_lineId = -1; // Unset line
			_redraw = true;
		}
	} else {
		// Line exists for current time/framecount
		if (j != _lineId && _lines[j].subStr.size()) {
			// Set line is not equal to current line & current line is not blank
			_lineId = j;  // Set line to current
			_redraw = true;
		}
	}
	return _redraw;
}

AutomaticSubtitle::AutomaticSubtitle(ZVision *engine, const Common::Path &subname, Audio::SoundHandle handle) :
	Subtitle(engine, subname, false),
	_handle(handle) {
}

bool AutomaticSubtitle::selfUpdate() {
	if (_engine->_mixer->isSoundHandleActive(_handle) && _engine->getScriptManager()->getStateValue(StateKey_Subtitles) == 1)
		return update(_engine->_mixer->getSoundElapsedTime(_handle) / 100);
	else {
		_toDelete = true;
		return false;
	}
}

bool AutomaticSubtitle::process(int32 deltatime) {
	Subtitle::process(deltatime);
	if (!_engine->_mixer->isSoundHandleActive(_handle))
		_toDelete = true;
	return _toDelete;
}

} // End of namespace ZVision
