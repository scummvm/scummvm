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
 */

#include "common/config-manager.h"
#include "common/str.h"

#include "trecision/actor.h"
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
	_talkingPersonAnimId = 0;
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

void TextManager::positionString(uint16 x, uint16 y, const char *string, uint16 *posx, uint16 *posy, bool characterFl) {
	uint16 lenText = _vm->textLength(string);
	if (lenText > 960)
		lenText = (lenText * 2 / 5);
	else if (lenText > 320)
		lenText = (lenText * 3 / 5);

	if (x > (lenText >> 1))
		x -= (lenText >> 1);
	else
		x = 0;

	*posx = CLIP<uint16>(x, 5, MAXX - lenText - 5);

	*posy = characterFl ? 0 : VIDEOTOP;
	*posy += y - 1; //15
	if (*posy <= VIDEOTOP)
		*posy = VIDEOTOP + 1;
}

void TextManager::formattingSuperString() {
	_subStringUsed = 0;
	_subStringAgain = true;
	while (_subStringAgain) {
		formattingOneString();
		_subStringUsed++;
	}
}

void TextManager::formattingOneString() {
	uint16 i;
	memset(_subString[_subStringUsed], '\0', MAXLENSUBSTRING);

	const uint16 available = (_superString.size() - _subStringStart);
	for (i = 0; i < available; i++) {
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

void TextManager::characterTalk(const char *s) {
	_vm->_flagSomeoneSpeaks = true;
	_vm->_flagCharacterSpeak = true;
	_vm->_flagSkipTalk = false;

	_superString = s;
	_subStringStart = 0;
	_curSubString = 0;
	formattingSuperString();

	characterContinueTalk();

	_vm->_characterQueue.initQueue();
	_vm->_actor->actorStop();
}

void TextManager::characterContinueTalk() {
	uint16 posx, posy;
	
	_vm->_flagSkipTalk = false;
	_vm->_characterSpeakTime = _vm->_curTime;

	_subStringAgain = (_curSubString < (_subStringUsed - 1));

	if (_vm->_flagCharacterExists)
		positionString(_vm->_actor->_lim[0], _vm->_actor->_lim[2], _subString[_curSubString], &posx, &posy, true);
	else
		positionString(MAXX / 2, 30, _subString[_curSubString], &posx, &posy, false);

	clearLastText();
	if (ConfMan.getBool("subtitles"))
		addText(posx, posy, _subString[_curSubString], COLOR_OBJECT, MASKCOL);

	if (!_vm->_flagDialogActive) {
		if (_curSubString)
			_lastFilename = Common::String::format("s%04d%c.wav", _curSentenceId, _curSubString + 'a');
		else
			_lastFilename = Common::String::format("s%04d.wav", _curSentenceId);
	}

	_talkTime = _vm->_soundMgr->talkStart(_lastFilename);
	if (!_talkTime)
		_talkTime = (strlen(_subString[_curSubString]) * 5) / 2 + 50;

	_curSubString++;

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
		_vm->_scheduler->doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, _vm->_oldRoom, 0, 0, _vm->_curObj);
}

void TextManager::someoneContinueTalk() {
	_someoneSpeakTime = _vm->_curTime;
	_vm->_flagSkipTalk = false;

	_subStringAgain = (_curSubString < (_subStringUsed - 1));

	uint16 posx, posy;
	if (_talkingPersonId)
		positionString(_vm->_obj[_talkingPersonId]._lim.left, _vm->_obj[_talkingPersonId]._lim.top, _subString[_curSubString], &posx, &posy, false);
	else
		positionString(_vm->_actor->_lim[0], _vm->_actor->_lim[2], _subString[_curSubString], &posx, &posy, true);

	clearLastText();
	if (ConfMan.getBool("subtitles"))
		addText(posx, posy, _subString[_curSubString], HYELLOW, MASKCOL);

	if (_curSubString)
		_lastFilename = Common::String::format("s%04d%c.wav", _curSentenceId, _curSubString + 'a');
	else
		_lastFilename = Common::String::format("s%04d.wav", _curSentenceId);

	_talkTime = _vm->_soundMgr->talkStart(_lastFilename);
	if (!_talkTime)
		_talkTime = (strlen(_subString[_curSubString]) * 5) / 2 + 50;

	_curSubString++;
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
	case ME_CHARACTERSPEAK:
		characterSay(_vm->_curMessage->_u16Param1);
		break;

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

	case ME_SOMEONEWAIT2SPEAK:
		if (!_vm->_curMessage->_u16Param1)
			someoneContinueTalk();
		else
			_vm->reEvent();
		break;

	case ME_SOMEONEWAIT2MUTE:
		if (!_vm->_curMessage->_u16Param1)
			someoneMute();
		else
			_vm->reEvent();
		break;

	case ME_SOMEONESPEAKING:
		if (_vm->_flagSomeoneSpeaks) {
			if (_vm->_flagSkipTalk || (_vm->_curTime >= (_talkTime + _someoneSpeakTime))) {
				if (_subStringAgain)
					someoneContinueTalk();
				else {
					if (_talkingPersonAnimId)
						_vm->_scheduler->doEvent(MC_ANIMATION, ME_DELANIM, MP_SYSTEM, _talkingPersonAnimId, true, 0, 0);
					_vm->_scheduler->doEvent(MC_STRING, ME_SOMEONEWAIT2MUTE, MP_DEFAULT, _talkingPersonAnimId, 0, 0, 0);
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

	Common::String locsent;

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

		if ((_vm->_obj[_vm->_curObj]._flag & (kObjFlagRoomOut | kObjFlagRoomIn)) && !(_vm->_obj[_vm->_curObj]._flag & kObjFlagExamine))
			return;

		locsent += _vm->_sysText[kMessageUse];
		if (_vm->_useWithInv[USED])
			locsent += _vm->_objName[_vm->_inventoryObj[_vm->_useWith[USED]]._name];
		else if (_vm->_obj[_vm->_useWith[USED]]._mode & OBJMODE_HIDDEN)
			locsent += dunno;
		else
			locsent += _vm->_objName[_vm->_obj[_vm->_useWith[USED]]._name];

		locsent += _vm->_sysText[kMessageWith];
		if (obj && (_vm->_useWithInv[USED] || (obj != _vm->_useWith[USED]))) {
			if (_vm->_obj[obj]._mode & OBJMODE_HIDDEN)
				locsent += dunno;
			else
				locsent += _vm->_objName[_vm->_obj[obj]._name];
		}

		_vm->_lastObj = (obj | 0x8000);
		uint16 lenText = _vm->textLength(locsent);

		uint16 posx = CLIP(320 - (lenText / 2), 2, MAXX - 2 - lenText);
		uint16 posy = MAXY - CARHEI;

		if (_vm->_lastObj)
			clearLastText();
		addText(posx, posy, locsent.c_str(), COLOR_INVENTORY, MASKCOL);
	} else {
		if (!obj || !show) {
			clearLastText();
			_vm->_lastObj = obj;
			return;
		}

		if (obj == _vm->_lastObj)
			return;
		if (!(_vm->_obj[obj]._flag & kObjFlagExamine)) {
			if ((_vm->_obj[obj]._flag & kObjFlagDone) || (_vm->_room[_vm->_obj[obj]._goRoom]._flag & kObjFlagDone)) {
				locsent = _vm->_sysText[kMessageGoto];
				if (_vm->_obj[obj]._mode & OBJMODE_HIDDEN)
					locsent += dunno;
				else
					locsent += _vm->_objName[_vm->_obj[obj]._name];
			} else
				locsent = _vm->_sysText[kMessageGoto2];
		} else if (_vm->_obj[obj]._mode & OBJMODE_HIDDEN)
			locsent = dunno;
		else
			locsent = _vm->_objName[_vm->_obj[obj]._name];

		uint16 posx = (_vm->_obj[obj]._lim.left + _vm->_obj[obj]._lim.right) / 2;
		uint16 posy = (obj == oWHEELS2C) ? 187 : _vm->_obj[obj]._lim.top;

		positionString(posx, posy, locsent.c_str(), &posx, &posy, false);
		if (_vm->_lastObj)
			clearLastText();
		_vm->_lastObj = obj;
		addText(posx, posy, locsent.c_str(), COLOR_OBJECT, MASKCOL);
	}
}

void TextManager::someoneSay(uint16 s, uint16 Person, uint16 NewAnim) {
	_talkingPersonAnimId = NewAnim;
	_talkingPersonId = Person;
	_vm->_flagSomeoneSpeaks = true;
	_vm->_flagSkipTalk = false;

	_curSentenceId = s;
	_superString = _vm->_sentence[s];
	_subStringStart = 0;
	_curSubString = 0;

	formattingSuperString();

	if (_talkingPersonAnimId)
		_vm->_scheduler->doEvent(MC_ANIMATION, ME_ADDANIM, MP_SYSTEM, _talkingPersonAnimId, 0, 0, 0);
	_vm->_scheduler->doEvent(MC_STRING, ME_SOMEONEWAIT2SPEAK, MP_DEFAULT, _talkingPersonAnimId, 0, 0, 0);
}

void TextManager::characterSay(uint16 i) {
	_curSentenceId = i;

	//	if he took some action
	if (_vm->_sentence[i][0] == '*' && !_vm->_animMgr->_playingAnims[kSmackerAction])
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

void TextManager::addText(uint16 x, uint16 y, const char *text, uint16 tcol, uint16 scol) {
	StackText t;
	t.x = x;
	t.y = y;
	t.tcol = tcol;
	t.scol = scol;
	t.clear = false;
	strcpy(t.text, text);

	_textStack.push_back(t);
}

void TextManager::clearLastText() {
	if (!_textStack.empty()) {
		if (!_textStack.back().clear)
			// The last entry is a string to be shown, remove it
			_textStack.pop_back();
	} else {
		StackText t;
		t.clear = true;
		_textStack.push_back(t);
	}
}

void TextManager::drawText(StackText text) {
	_curString._rect.left = text.x;
	_curString._rect.top = text.y;
	_curString._rect.setWidth(_vm->textLength(text.text));
	int16 w = _curString._rect.width();

	if (text.y == MAXY - CARHEI && w > 600)
		w = w * 3 / 5;
	else if (text.y != MAXY - CARHEI && w > 960)
		w = w * 2 / 5;
	else if (text.y != MAXY - CARHEI && w > 320)
		w = w * 3 / 5;

	_curString._rect.setWidth(w);

	_curString.text = text.text;
	uint16 height = _curString.calcHeight(_vm);
	_curString._subtitleRect = Common::Rect(_curString._rect.width(), height);
	_curString._rect.setHeight(height);
	_curString.tcol = text.tcol;
	_curString.scol = text.scol;

	if (_curString._rect.top <= height)
		_curString._rect.top += height;
	else
		_curString._rect.top -= height;

	if (_curString._rect.top <= VIDEOTOP)
		_curString._rect.top = VIDEOTOP + 1;

	_vm->_textStatus |= TEXT_DRAW;
}

void TextManager::clearText() {
	if (_oldString.text.empty() && !_curString.text.empty()) {
		_oldString.set(_curString);
		_curString.text.clear();

		_vm->_textStatus |= TEXT_DEL;
	}
}

void TextManager::drawTexts() {
	for (Common::List<StackText>::iterator i = _textStack.begin(); i != _textStack.end(); ++i) {
		if (i->clear)
			clearText();
		else
			drawText(*i);
	}
}

void TextManager::redrawString() {
	if (!_vm->_flagDialogActive && !_vm->_flagDialogMenuActive && !_vm->_flagSomeoneSpeaks && !_vm->_flagScriptActive && _vm->_graphicsMgr->isCursorVisible()) {
		if (_vm->isInventoryArea(_vm->_mousePos))
			_vm->_scheduler->doEvent(MC_INVENTORY, ME_SHOWICONNAME, MP_DEFAULT, 0, 0, 0, 0);
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
	_vm->_dirtyRects.push_back(_curString._rect);
}

Common::Rect TextManager::getOldTextRect() const {
	return _oldString._rect;
}

void TextManager::clearOldText() {
	_oldString.text.clear();
}

} // End of namespace Trecision
