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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/config-manager.h"
#include "common/str.h"

#include "trecision/actor.h"
#include "trecision/animmanager.h"
#include "trecision/graphics.h"
#include "trecision/sound.h"
#include "trecision/trecision.h"
#include "trecision/text.h"
#include "trecision/scheduler.h"
#include "trecision/video.h"

namespace Trecision {

TextManager::TextManager(TrecisionEngine *vm) : _vm(vm) {
	_someoneSpeakTime = 0;
	_subStringAgain = false;
	_talkTime = 0;
	for (int i = 0; i < MAXSUBSTRING; ++i) {
		for (int j = 0; j < MAXLENSUBSTRING; ++j) {
			_subString[i][j] = 0;
		}
	}
	_subStringUsed = 0;
	_subStringStart = 0;
	_curSentenceId = 0;
	_curSubString = 0;
	_talkingPersonId = 0;
}

TextManager::~TextManager() {
}

Common::Point TextManager::positionString(uint16 x, uint16 y, const char *string, bool characterFl) {
	uint16 lenText = _vm->textLength(string);
	Common::Point pos;

	if (lenText > 960)
		lenText = (lenText * 2 / 5);
	else if (lenText > 320)
		lenText = (lenText * 3 / 5);

	if (x > (lenText >> 1))
		x -= (lenText >> 1);
	else
		x = 0;

	pos.x = CLIP<uint16>(x, 5, MAXX - lenText - 5);

	pos.y = characterFl ? 0 : VIDEOTOP;
	pos.y += y - 1; //15
	if (pos.y <= VIDEOTOP)
		pos.y = VIDEOTOP + 1;

	return pos;
}

void TextManager::formattingSuperString() {
	_subStringUsed = 0;
	_subStringAgain = true;
	while (_subStringAgain) {
		formattingOneString();
		++_subStringUsed;
	}
}

void TextManager::formattingOneString() {
	uint16 i;
	memset(_subString[_subStringUsed], '\0', MAXLENSUBSTRING);

	const uint16 available = (_superString.size() - _subStringStart);
	for (i = 0; i < available; ++i) {
		switch (_superString[i + _subStringStart]) {
		case '\0':
			_subStringAgain = false;
			return;

		case '@':
			_subStringAgain = true;
			_subStringStart += (i + 1);
			return;

		default:
			_subString[_subStringUsed][i] = _superString[i + _subStringStart];
			break;
		}
	}

	_subString[_subStringUsed][i] = '\0';
	_subStringAgain = false;
}

void TextManager::characterTalk(Common::String s) {
	_vm->_flagSomeoneSpeaks = true;
	_vm->_flagCharacterSpeak = true;
	_vm->_flagSkipTalk = false;

	_superString = s;
	_subStringStart = 0;
	_curSubString = 0;
	formattingSuperString();

	characterContinueTalk();

	_vm->_scheduler->initCharacterQueue();
	_vm->_actor->actorStop();
}

void TextManager::characterContinueTalk() {
	Common::Point pos;

	_vm->_flagSkipTalk = false;
	_vm->_characterSpeakTime = _vm->_curTime;

	_subStringAgain = (_curSubString < (_subStringUsed - 1));

	if (_vm->_flagShowCharacter || _vm->_animMgr->isActionActive())
		pos = positionString(_vm->_actor->_area[0], _vm->_actor->_area[2], _subString[_curSubString], true);
	else
		pos = positionString(MAXX / 2, 30, _subString[_curSubString], false);

	clearLastText();
	if (ConfMan.getBool("subtitles"))
		addText(pos, _subString[_curSubString], COLOR_OBJECT);

	if (!_vm->_flagDialogActive) {
		if (_curSubString)
			_lastFilename = Common::String::format("s%04d%c.wav", _curSentenceId, _curSubString + 'a');
		else
			_lastFilename = Common::String::format("s%04d.wav", _curSentenceId);
	}

	_talkTime = _vm->_soundMgr->talkStart(_lastFilename);
	if (!_talkTime)
		_talkTime = Common::String(_subString[_curSubString]).size() * 5 / 2 + 50;

	++_curSubString;

	_vm->_scheduler->doEvent(MC_STRING, ME_CHARACTERSPEAKING, MP_DEFAULT, 0, 0, 0, 0);
}

void TextManager::characterMute() {
	_vm->_flagSomeoneSpeaks = false;
	_vm->_flagCharacterSpeak = false;
	_vm->_flagSkipTalk = false;
	_vm->_characterSpeakTime = 0;

	clearLastText();
	_vm->_lastObj = 0;
	_vm->_lastInv = 0;

	redrawString();
	_vm->_soundMgr->stopSoundType(kSoundTypeSpeech);

	if ((_vm->_curRoom == kRoom12CU) || (_vm->_curRoom == kRoom13CU))
		_vm->changeRoom(_vm->_oldRoom);
}

void TextManager::someoneContinueTalk() {
	_someoneSpeakTime = _vm->_curTime;
	_vm->_flagSkipTalk = false;

	_subStringAgain = (_curSubString < (_subStringUsed - 1));

	Common::Point pos;
	if (_talkingPersonId)
		pos = positionString(_vm->_obj[_talkingPersonId]._area.left, _vm->_obj[_talkingPersonId]._area.top, _subString[_curSubString], false);
	else
		pos = positionString(_vm->_actor->_area[0], _vm->_actor->_area[2], _subString[_curSubString], true);

	clearLastText();
	if (ConfMan.getBool("subtitles"))
		addText(pos, _subString[_curSubString], HYELLOW);

	if (_curSubString)
		_lastFilename = Common::String::format("s%04d%c.wav", _curSentenceId, _curSubString + 'a');
	else
		_lastFilename = Common::String::format("s%04d.wav", _curSentenceId);

	_talkTime = _vm->_soundMgr->talkStart(_lastFilename);
	if (!_talkTime)
		_talkTime = Common::String(_subString[_curSubString]).size() * 5 / 2 + 50;

	++_curSubString;
	_vm->_scheduler->doEvent(MC_STRING, ME_SOMEONESPEAKING, MP_DEFAULT, 0, 0, 0, 0);
}

void TextManager::someoneMute() {
	_vm->_flagCharacterSpeak = false;
	_vm->_flagSkipTalk = false;
	_vm->_flagSomeoneSpeaks = false;
	_someoneSpeakTime = 0;

	clearLastText();
	_vm->_lastObj = 0;
	_vm->_lastInv = 0;

	redrawString();
	_vm->_soundMgr->stopSoundType(kSoundTypeSpeech);
}

// ******************************************************* //

void TextManager::doString() {
	switch (_vm->_curMessage->_event) {

	case ME_CHARACTERSPEAKING:
		if (_vm->_flagCharacterSpeak) {
			if (_vm->_flagSkipTalk || (_vm->_curTime > _talkTime + _vm->_characterSpeakTime)) {
				if (_subStringAgain)
					characterContinueTalk();
				else
					characterMute();
			} else
				_vm->reEvent();
		}
		break;

	case ME_SOMEONESPEAKING:
		if (_vm->_flagSomeoneSpeaks) {
			if (_vm->_flagSkipTalk || (_vm->_curTime >= (_talkTime + _someoneSpeakTime))) {
				if (_subStringAgain)
					someoneContinueTalk();
				else {
					someoneMute();
				}
			} else
				_vm->reEvent();
		}
		break;
	default:
		break;
	}
}

void TextManager::showObjName(uint16 obj, bool show) {
	static const char *dunno = "?";
	Common::String desc;

	if (_vm->_flagSomeoneSpeaks)
		return;

	if (_vm->_lastInv) {
		clearLastText();
		_vm->_lastInv = 0;
	}

	if (_vm->_flagUseWithStarted) {
		if (!show) {
			clearLastText();
			_vm->_lastObj = obj;
			return;
		}

		if ((_vm->_obj[_vm->_curObj].isFlagRoomOut() || _vm->_obj[_vm->_curObj].isFlagRoomIn()) && !_vm->_obj[_vm->_curObj].isFlagExamine())
			return;

		desc = _vm->_sysText[kMessageUse];
	
		if (_vm->_useWithInv[USED])
			desc += _vm->_objName[_vm->_inventoryObj[_vm->_useWith[USED]]._name];
		else if (_vm->_obj[_vm->_useWith[USED]].isModeHidden())
			desc += dunno;
		else
			desc += _vm->_objName[_vm->_obj[_vm->_useWith[USED]]._name];

		desc += _vm->_sysText[kMessageWith];
		if (obj && (_vm->_useWithInv[USED] || (obj != _vm->_useWith[USED]))) {
			if (_vm->_obj[obj].isModeHidden())
				desc += dunno;
			else
				desc += _vm->_objName[_vm->_obj[obj]._name];
		}

		_vm->_lastObj = (obj | 0x8000);
		const uint16 lenText = _vm->textLength(desc);
		const Common::Point pos(CLIP(320 - (lenText / 2), 2, MAXX - 2 - lenText), MAXY - CARHEI);

		if (_vm->_lastObj)
			clearLastText();
		addText(pos, desc.c_str(), COLOR_INVENTORY);
	} else {
		if (!obj || !show) {
			clearLastText();
			_vm->_lastObj = obj;
			return;
		}

		if (obj == _vm->_lastObj)
			return;
		if (!_vm->_obj[obj].isFlagExamine()) {
			if (_vm->_obj[obj].isFlagDone() || _vm->_room[_vm->_obj[obj]._goRoom].isDone()) {
				desc = _vm->_sysText[kMessageGoto];
				if (_vm->_obj[obj].isModeHidden())
					desc += dunno;
				else
					desc += _vm->_objName[_vm->_obj[obj]._name];
			} else
				desc = _vm->_sysText[kMessageGoto2];
		} else if (_vm->_obj[obj].isModeHidden())
			desc = dunno;
		else
			desc = _vm->_objName[_vm->_obj[obj]._name];

		const uint16 x = (_vm->_obj[obj]._area.left + _vm->_obj[obj]._area.right) / 2;
		const uint16 y = (obj == oWHEELS2C) ? 187 : _vm->_obj[obj]._area.top;
		Common::Point pos = positionString(x, y, desc.c_str(), false);

		if (_vm->_lastObj)
			clearLastText();
		_vm->_lastObj = obj;
		addText(pos, desc.c_str(), COLOR_OBJECT);
	}
}

void TextManager::someoneSay(uint16 sentence, uint16 person) {
	_talkingPersonId = person;
	_vm->_flagSomeoneSpeaks = true;
	_vm->_flagSkipTalk = false;

	_curSentenceId = sentence;
	_superString = _vm->_sentence[sentence];
	_subStringStart = 0;
	_curSubString = 0;

	formattingSuperString();
	someoneContinueTalk();
}

void TextManager::characterSay(uint16 i) {
	_curSentenceId = i;

	//	if he took some action
	if (_vm->_sentence[i][0] == '*' && !_vm->_animMgr->isActionActive())
		_vm->startCharacterAction(hBOH, 0, 0, 0);
	else
		characterTalk(_vm->_sentence[i]);
}

void TextManager::characterSayInAction(uint16 ss) {
	const char *s = _vm->_sentence[ss];

	if (s[0] == '*')
		return;
	_curSentenceId = ss;

	_vm->_flagSomeoneSpeaks = true;
	_vm->_flagCharacterSpeak = true;
	_vm->_flagSkipTalk = false;

	_superString = s;
	_subStringStart = 0;
	_curSubString = 0;
	formattingSuperString();

	characterContinueTalk();
}

void TextManager::addText(Common::Point pos, const char *text, uint16 textCol) {
	StackText t;
	t._x = pos.x;
	t._y = pos.y;
	t._textColor = textCol;
	t._clear = false;
	t._text = text;

	_textStack.push_back(t);
}

void TextManager::clearLastText() {
	if (!_textStack.empty()) {
		if (!_textStack.back()._clear)
			// The last entry is a string to be shown, remove it
			_textStack.pop_back();
	} else {
		StackText t;
		t._clear = true;
		_textStack.push_back(t);
	}
}

void TextManager::drawText(StackText *text) {
	_curString._rect.left = text->_x;
	_curString._rect.top = text->_y;
	_curString._rect.setWidth(_vm->textLength(text->_text));
	int16 w = _curString._rect.width();

	if (text->_y == MAXY - CARHEI && w > 600)
		w = w * 3 / 5;
	else if (text->_y != MAXY - CARHEI && w > 960)
		w = w * 2 / 5;
	else if (text->_y != MAXY - CARHEI && w > 320)
		w = w * 3 / 5;

	_curString._rect.setWidth(w);

	_curString._text = text->_text;
	uint16 height = _curString.calcHeight(_vm);
	_curString._subtitleRect = Common::Rect(_curString._rect.width(), height);
	_curString._rect.setHeight(height);
	_curString._textColor = text->_textColor;

	if (_curString._rect.top <= height)
		_curString._rect.top += height;
	else
		_curString._rect.top -= height;

	if (_curString._rect.top <= VIDEOTOP)
		_curString._rect.top = VIDEOTOP + 1;

	_vm->_textStatus |= TEXT_DRAW;
}

void TextManager::clearText() {
	if (_oldString._text.empty() && !_curString._text.empty()) {
		_oldString.set(&_curString);
		_curString._text.clear();

		_vm->_textStatus |= TEXT_DEL;
	}
}

void TextManager::drawTexts() {
	for (Common::List<StackText>::iterator it = _textStack.begin(); it != _textStack.end(); ++it) {
		if (it->_clear)
			clearText();
		else
			drawText(&*it);
	}
}

void TextManager::redrawString() {
	if (!_vm->_flagDialogActive && !_vm->_flagDialogMenuActive && !_vm->_flagSomeoneSpeaks && !_vm->_flagScriptActive && _vm->_graphicsMgr->isCursorVisible()) {
		if (_vm->isInventoryArea(_vm->_mousePos))
			_vm->showIconName();
		else {
			_vm->checkMask(_vm->_mousePos);
			showObjName(_vm->_curObj, true);
		}
	}
}

void TextManager::clearTextStack() {
	_textStack.clear();
}

void TextManager::drawCurString() {
	_curString.draw(_vm);
	_vm->_graphicsMgr->addDirtyRect(_curString._rect, false);
}

Common::Rect TextManager::getOldTextRect() const {
	return _oldString._rect;
}

void TextManager::clearOldText() {
	_oldString._text.clear();
}

} // End of namespace Trecision
