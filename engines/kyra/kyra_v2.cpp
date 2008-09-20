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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "common/config-manager.h"

#include "kyra/kyra_v2.h"
#include "kyra/screen_v2.h"
#include "kyra/debugger.h"

namespace Kyra {

KyraEngine_v2::KyraEngine_v2(OSystem *system, const GameFlags &flags, const EngineDesc &desc) : KyraEngine_v1(system, flags), _desc(desc) {
	memset(&_sceneAnims, 0, sizeof(_sceneAnims));
	memset(&_sceneAnimMovie, 0, sizeof(_sceneAnimMovie));

	_lastProcessedSceneScript = 0;
	_specialSceneScriptRunFlag = false;

	_itemList = 0;
	_itemListSize = 0;
	
	_characterShapeFile = -1;

	_updateCharPosNextUpdate = 0;

	memset(&_sceneScriptState, 0, sizeof(_sceneScriptState));
	memset(&_sceneScriptData, 0, sizeof(_sceneScriptData));

	_animObjects = 0;

	_runFlag = true;
	_showOutro = false;
	_deathHandler = -1;
	_animNeedUpdate = false;

	_animShapeCount = 0;
	_animShapeFiledata = 0;

	_vocHigh = -1;
	_chatVocHigh = -1;
	_chatVocLow = -1;
	_chatText = 0;
	_chatObject = -1;

	memset(_hiddenItems, -1, sizeof(_hiddenItems));

	_screenBuffer = 0;

	memset(&_mainCharacter, 0, sizeof(_mainCharacter));
	memset(&_mainCharacter.inventory, -1, sizeof(_mainCharacter.inventory));

	_pauseStart = 0;

	_lang = 0;
	Common::Language lang = Common::parseLanguage(ConfMan.get("language"));
	if (lang == _flags.fanLang && _flags.replacedLang != Common::UNK_LANG)
		lang = _flags.replacedLang;

	switch (lang) {
	case Common::EN_ANY:
	case Common::EN_USA:
	case Common::EN_GRB:
		_lang = 0;
		break;

	case Common::FR_FRA:
		_lang = 1;
		break;

	case Common::DE_DEU:
		_lang = 2;
		break;

	case Common::JA_JPN:
		_lang = 3;
		break;

	default:
		warning("unsupported language, switching back to English");
		_lang = 0;
		break;
	}
}

KyraEngine_v2::~KyraEngine_v2() {
	if (!(_flags.isDemo && !_flags.isTalkie)) {
		for (ShapeMap::iterator i = _gameShapes.begin(); i != _gameShapes.end(); ++i) {
			delete[] i->_value;
			i->_value = 0;
		}
		_gameShapes.clear();
	}

	delete[] _itemList;
	delete[] _sceneList;

	_emc->unload(&_sceneScriptData);

	delete[] _animObjects;

	for (Common::Array<const Opcode*>::iterator i = _opcodesAnimation.begin(); i != _opcodesAnimation.end(); ++i)
		delete *i;
	_opcodesAnimation.clear();

	delete[] _screenBuffer;
}

void KyraEngine_v2::pauseEngineIntern(bool pause) {
	KyraEngine_v1::pauseEngineIntern(pause);

	if (!pause) {
		uint32 pausedTime = _system->getMillis() - _pauseStart;

		for (int i = 0; i < ARRAYSIZE(_sceneSpecialScriptsTimer); ++i) {
			if (_sceneSpecialScriptsTimer[i])
				_sceneSpecialScriptsTimer[i] += pausedTime;
		}

	} else {
		_pauseStart = _system->getMillis();
	}
}

void KyraEngine_v2::delay(uint32 amount, bool updateGame, bool isMainLoop) {
	debugC(9, kDebugLevelMain, "KyraEngine_v2::delay(%u, %d, %d)", amount, updateGame, isMainLoop);

	uint32 start = _system->getMillis();
	do {
		if (updateGame) {
			if (_chatText)
				updateWithText();
			else
				update();
		} else {
			updateInput();
		}

		if (amount > 0)
			_system->delayMillis(amount > 10 ? 10 : amount);
	} while (!skipFlag() && _system->getMillis() < start + amount && !quit());
}

int KyraEngine_v2::checkInput(Button *buttonList, bool mainLoop) {
	debugC(9, kDebugLevelMain, "KyraEngine_v2::checkInput(%p, %d)", (const void*)buttonList, mainLoop);
	updateInput();

	int keys = 0;
	int8 mouseWheel = 0;

	while (_eventList.size()) {
		Common::Event event = *_eventList.begin();
		bool breakLoop = false;

		switch (event.type) {
		case Common::EVENT_KEYDOWN:
			if (event.kbd.keycode >= '1' && event.kbd.keycode <= '9' &&
					(event.kbd.flags == Common::KBD_CTRL || event.kbd.flags == Common::KBD_ALT) && mainLoop) {
				const char *saveLoadSlot = getSavegameFilename(9 - (event.kbd.keycode - '0') + 990);

				if (event.kbd.flags == Common::KBD_CTRL) {
					loadGame(saveLoadSlot);
					_eventList.clear();
					breakLoop = true;
				} else {
					char savegameName[14];
					sprintf(savegameName, "Quicksave %d", event.kbd.keycode - '0');
					saveGame(saveLoadSlot, savegameName, 0);
				}
			} else if (event.kbd.flags == Common::KBD_CTRL) {
				if (event.kbd.keycode == 'd')
					_debugger->attach();
			}
			break;

		case Common::EVENT_MOUSEMOVE: {
			Common::Point pos = getMousePos();
			_mouseX = pos.x;
			_mouseY = pos.y;
			} break;

		case Common::EVENT_LBUTTONDOWN:
		case Common::EVENT_LBUTTONUP: {
			Common::Point pos = getMousePos();
			_mouseX = pos.x;
			_mouseY = pos.y;
			keys = (event.type == Common::EVENT_LBUTTONDOWN ? 199 : (200 | 0x800));
			breakLoop = true;
			} break;

		case Common::EVENT_WHEELUP:
			mouseWheel = -1;
			break;

		case Common::EVENT_WHEELDOWN:
			mouseWheel = 1;
			break;

		default:
			break;
		}

		if (_debugger->isAttached())
			_debugger->onFrame();

		if (breakLoop)
			break;

		_eventList.erase(_eventList.begin());
	}

	return gui_v2()->processButtonList(buttonList, keys | 0x8000, mouseWheel);
}

void KyraEngine_v2::updateInput() {
	Common::Event event;

	while (_eventMan->pollEvent(event)) {
		switch (event.type) {
		case Common::EVENT_KEYDOWN:
			if (event.kbd.keycode == '.' || event.kbd.keycode == Common::KEYCODE_ESCAPE)
				_eventList.push_back(Event(event, true));
			else if (event.kbd.keycode == 'q' && event.kbd.flags == Common::KBD_CTRL)
				quitGame();
			else
				_eventList.push_back(event);
			break;

		case Common::EVENT_LBUTTONDOWN:
			_eventList.push_back(Event(event, true));
			break;

		case Common::EVENT_MOUSEMOVE:
			screen_v2()->updateScreen();
			// fall through

		case Common::EVENT_LBUTTONUP:
		case Common::EVENT_WHEELUP:
		case Common::EVENT_WHEELDOWN:
			_eventList.push_back(event);
			break;

		default:
			break;
		}
	}
}

void KyraEngine_v2::removeInputTop() {
	if (!_eventList.empty())
		_eventList.erase(_eventList.begin());
}

bool KyraEngine_v2::skipFlag() const {
	for (Common::List<Event>::const_iterator i = _eventList.begin(); i != _eventList.end(); ++i) {
		if (i->causedSkip)
			return true;
	}
	return false;
}

void KyraEngine_v2::resetSkipFlag(bool removeEvent) {
	for (Common::List<Event>::iterator i = _eventList.begin(); i != _eventList.end(); ++i) {
		if (i->causedSkip) {
			if (removeEvent)
				_eventList.erase(i);
			else
				i->causedSkip = false;
			return;
		}
	}
}

bool KyraEngine_v2::checkSpecialSceneExit(int num, int x, int y) {
	if (_specialExitTable[0+num] > x || _specialExitTable[5+num] > y ||
		_specialExitTable[10+num] < x || _specialExitTable[15+num] < y)
		return false;
	return true;
}

void KyraEngine_v2::addShapeToPool(const uint8 *data, int realIndex, int shape) {
	debugC(9, kDebugLevelMain, "KyraEngine_v2::addShapeToPool(%p, %d, %d)", data, realIndex, shape);
	remShapeFromPool(realIndex);
	_gameShapes[realIndex] = screen_v2()->makeShapeCopy(data, shape);
}

void KyraEngine_v2::addShapeToPool(uint8 *shpData, int index) {
	debugC(9, kDebugLevelMain, "KyraEngine_v2::addShapeToPool(%p, %d)", shpData, index);
	remShapeFromPool(index);
	_gameShapes[index] = shpData;
}

void KyraEngine_v2::remShapeFromPool(int idx) {
	ShapeMap::iterator iter = _gameShapes.find(idx);
	if (iter != _gameShapes.end()) {
		delete[] iter->_value;
		iter->_value = 0;
	}
}

uint8 *KyraEngine_v2::getShapePtr(int shape) const {
	debugC(9, kDebugLevelMain, "KyraEngine_v2::getShapePtr(%d)", shape);
	ShapeMap::iterator iter = _gameShapes.find(shape);
	if (iter == _gameShapes.end())
		return 0;
	return iter->_value;
}

void KyraEngine_v2::moveCharacter(int facing, int x, int y) {
	debugC(9, kDebugLevelMain, "KyraEngine_v2::moveCharacter(%d, %d, %d)", facing, x, y);
	x &= ~3;
	y &= ~1;
	_mainCharacter.facing = facing;

	Screen *scr = screen();
	scr->hideMouse();
	switch (facing) {
	case 0:
		while (_mainCharacter.y1 > y)
			updateCharPosWithUpdate();
		break;

	case 2:
		while (_mainCharacter.x1 < x)
			updateCharPosWithUpdate();
		break;

	case 4:
		while (_mainCharacter.y1 < y)
			updateCharPosWithUpdate();
		break;

	case 6:
		while (_mainCharacter.x1 > x)
			updateCharPosWithUpdate();
		break;

	default:
		break;
	}
	scr->showMouse();
}

void KyraEngine_v2::updateCharPosWithUpdate() {
	debugC(9, kDebugLevelMain, "KyraEngine_v2::updateCharPosWithUpdate()");
	updateCharPos(0, 0);
	update();
}

int KyraEngine_v2::updateCharPos(int *table, int force) {
	debugC(9, kDebugLevelMain, "KyraEngine_v2::updateCharPos(%p, %d)", (const void*)table, force);
	if (_updateCharPosNextUpdate > _system->getMillis() && !force)
		return 0;
	_mainCharacter.x1 += _charAddXPosTable[_mainCharacter.facing];
	_mainCharacter.y1 += _charAddYPosTable[_mainCharacter.facing];
	updateCharAnimFrame(0, table);
	_updateCharPosNextUpdate = _system->getMillis() + getCharacterWalkspeed() * _tickLength;
	return 1;
}

} // end of namespace Kyra

