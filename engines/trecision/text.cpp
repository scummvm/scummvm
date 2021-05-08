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

#include "trecision/nl/message.h"
#include "trecision/nl/proto.h"
#include "trecision/trecision.h"
#include "trecision/sound.h"
#include "trecision/actor.h"
#include "trecision/text.h"

namespace Trecision {

TextManager::TextManager(TrecisionEngine *vm) : _vm(vm) {
	_someoneSpeakTime = 0;
	_superStringLen = 0;
	_subStringAgain = false;
	_talkTime = 0;
	_talkingPersonAnimId = 0;
	for (int i = 0; i < MAXSUBSTRING; ++i) {
		for (int j = 0; j < MAXLENSUBSTRING; ++j) {
			_subString[i][j] = 0;
		}
	}
	_subStringUsed = 0;
	_superString = nullptr;
	_subStringStart = 0;
	_curSentenceId = 0;
	_curSubString = 0;
	for (int i = 0; i < 13; ++i)
		_lastFilename[i] = 0;
	_talkingPersonId = 0;
}

TextManager::~TextManager() {
}

void TextManager::PositionString(uint16 x, uint16 y, const char *string, uint16 *posx, uint16 *posy, bool characterFl) {
	uint16 lenText = _vm->textLength(string, 0);
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

void TextManager::FormattingSuperString() {
	_subStringUsed = 0;
	_subStringAgain = true;
	while (_subStringAgain) {
		FormattingOneString();
		_subStringUsed++;
	}
}

void TextManager::FormattingOneString() {
	uint16 i;
	memset(_subString[_subStringUsed], '\0', MAXLENSUBSTRING);

	const uint16 available = (_superStringLen - _subStringStart);
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

void TextManager::CharacterTalk(const char *s) {
	_vm->_flagSomeoneSpeaks = true;
	_vm->_flagCharacterSpeak = true;
	_vm->_flagSkipTalk = false;

	_superString = s;
	_superStringLen = strlen(_superString);
	_subStringStart = 0;
	_curSubString = 0;
	FormattingSuperString();

	CharacterContinueTalk();

	_vm->_characterQueue.initQueue();
	_vm->_actor->actorStop();
}

void TextManager::CharacterContinueTalk() {
	_vm->_flagSkipTalk = false;
	_vm->_characterSpeakTime = _vm->_curTime;

	_subStringAgain = (_curSubString < (_subStringUsed - 1));

	uint16 posx, posy;
	if (_vm->_flagCharacterExists)
		PositionString(_vm->_actor->_lim[0], _vm->_actor->_lim[2], _subString[_curSubString], &posx, &posy, true);
	else
		PositionString(MAXX / 2, 30, _subString[_curSubString], &posx, &posy, false);

	_vm->clearText();
	if (ConfMan.getBool("subtitles"))
		_vm->addText(posx, posy, _subString[_curSubString], COLOR_OBJECT, MASKCOL);

	if (!_vm->_flagDialogActive) {
		if (_curSubString)
			sprintf(_lastFilename, "s%04d%c.wav", _curSentenceId, _curSubString + 'a');
		else
			sprintf(_lastFilename, "s%04d.wav", _curSentenceId);
	}

	_talkTime = _vm->_soundMgr->talkStart(_lastFilename);
	if (!_talkTime)
		_talkTime = (strlen(_subString[_curSubString]) * 5) / 2 + 50;

	_curSubString++;

	doEvent(MC_STRING, ME_CHARACTERSPEAKING, MP_DEFAULT, 0, 0, 0, 0);
}

void TextManager::CharacterMute() {
	_vm->_flagSomeoneSpeaks = false;
	_vm->_flagCharacterSpeak = false;
	_vm->_flagSkipTalk = false;
	_vm->_characterSpeakTime = 0;

	_vm->clearText();
	_vm->_lastObj = 0;
	_vm->_lastInv = 0;

	_vm->redrawString();
	_vm->_soundMgr->talkStop();

	if ((_vm->_curRoom == kRoom12CU) || (_vm->_curRoom == kRoom13CU))
		doEvent(MC_SYSTEM, ME_CHANGEROOM, MP_SYSTEM, _vm->_oldRoom, 0, 0, _vm->_curObj);
}

void TextManager::SomeoneContinueTalk() {
	_someoneSpeakTime = _vm->_curTime;
	_vm->_flagSkipTalk = false;

	_subStringAgain = (_curSubString < (_subStringUsed - 1));

	uint16 posx, posy;
	if (_talkingPersonId)
		PositionString(_vm->_obj[_talkingPersonId]._lim.left, _vm->_obj[_talkingPersonId]._lim.top, _subString[_curSubString], &posx, &posy, false);
	else
		PositionString(_vm->_actor->_lim[0], _vm->_actor->_lim[2], _subString[_curSubString], &posx, &posy, true);

	_vm->clearText();
	if (ConfMan.getBool("subtitles"))
		_vm->addText(posx, posy, _subString[_curSubString], HYELLOW, MASKCOL);

	if (_curSubString)
		sprintf(_lastFilename, "s%04d%c.wav", _curSentenceId, _curSubString + 'a');
	else
		sprintf(_lastFilename, "s%04d.wav", _curSentenceId);

	_talkTime = _vm->_soundMgr->talkStart(_lastFilename);
	if (!_talkTime)
		_talkTime = (strlen(_subString[_curSubString]) * 5) / 2 + 50;

	_curSubString++;
	doEvent(MC_STRING, ME_SOMEONESPEAKING, MP_DEFAULT, 0, 0, 0, 0);
}

void TextManager::someoneMute() {
	_vm->_flagCharacterSpeak = false;
	_vm->_flagSkipTalk = false;
	_vm->_flagSomeoneSpeaks = false;
	_someoneSpeakTime = 0;

	_vm->clearText();
	_vm->_lastObj = 0;
	_vm->_lastInv = 0;

	_vm->redrawString();
	_vm->_soundMgr->talkStop();
}

// ******************************************************* //

void TextManager::doString() {
	switch (_vm->_curMessage->_event) {
	case ME_CHARACTERSPEAK:
		CharacterSay(_vm->_curMessage->_u16Param1);
		break;

	case ME_CHARACTERSPEAKING:
		if (_vm->_flagCharacterSpeak) {
			if (_vm->_flagSkipTalk || (_vm->_curTime > _talkTime + _vm->_characterSpeakTime)) {
				if (_subStringAgain)
					CharacterContinueTalk();
				else
					CharacterMute();
			} else
				_vm->reEvent();
		}
		break;

	case ME_SOMEONEWAIT2SPEAK:
		if (!_vm->_curMessage->_u16Param1)
			SomeoneContinueTalk();
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
					SomeoneContinueTalk();
				else {
					if (_talkingPersonAnimId)
						doEvent(MC_ANIMATION, ME_DELANIM, MP_SYSTEM, _talkingPersonAnimId, true, 0, 0);
					doEvent(MC_STRING, ME_SOMEONEWAIT2MUTE, MP_DEFAULT, _talkingPersonAnimId, 0, 0, 0);
				}
			} else
				_vm->reEvent();
		}
		break;
	default:
		break;
	}
}

void TextManager::ShowObjName(uint16 obj, bool show) {
	static const char *dunno = "?";

	Common::String locsent;

	if (_vm->_flagSomeoneSpeaks)
		return;

	if (_vm->_lastInv) {
		_vm->clearText();
		_vm->_lastInv = 0;
	}

	if (_vm->_flagUseWithStarted) {
		if (!show) {
			_vm->clearText();
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
		uint16 lenText = _vm->textLength(locsent.c_str(), 0);

		uint16 posx = CLIP(320 - (lenText / 2), 2, MAXX - 2 - lenText);
		uint16 posy = MAXY - CARHEI;

		if (_vm->_lastObj)
			_vm->clearText();
		_vm->addText(posx, posy, locsent.c_str(), COLOR_INVENTORY, MASKCOL);
	} else {
		if (!obj || !show) {
			_vm->clearText();
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

		PositionString(posx, posy, locsent.c_str(), &posx, &posy, false);
		if (_vm->_lastObj)
			_vm->clearText();
		_vm->_lastObj = obj;
		_vm->addText(posx, posy, locsent.c_str(), COLOR_OBJECT, MASKCOL);
	}
}

void TextManager::SomeoneSay(uint16 s, uint16 Person, uint16 NewAnim) {
	_talkingPersonAnimId = NewAnim;
	_talkingPersonId = Person;
	_vm->_flagSomeoneSpeaks = true;
	_vm->_flagSkipTalk = false;

	_curSentenceId = s;
	_superString = _vm->_sentence[s];
	_superStringLen = strlen(_superString);
	_subStringStart = 0;
	_curSubString = 0;

	FormattingSuperString();

	if (_talkingPersonAnimId)
		doEvent(MC_ANIMATION, ME_ADDANIM, MP_SYSTEM, _talkingPersonAnimId, 0, 0, 0);
	doEvent(MC_STRING, ME_SOMEONEWAIT2SPEAK, MP_DEFAULT, _talkingPersonAnimId, 0, 0, 0);
}

void TextManager::CharacterSay(uint16 i) {
	_curSentenceId = i;

	//	if he took some action
	if (_vm->_sentence[i][0] == '*' && !_vm->_animMgr->_playingAnims[kSmackerAction])
		_vm->StartCharacterAction(hBOH, 0, 0, 0);
	else
		CharacterTalk(_vm->_sentence[i]);
}

void TextManager::CharacterSayInAction(uint16 ss) {
	const char *s = _vm->_sentence[ss];

	if (s[0] == '*')
		return;
	_curSentenceId = ss;

	_vm->_flagSomeoneSpeaks = true;
	_vm->_flagCharacterSpeak = true;
	_vm->_flagSkipTalk = false;

	_superString = s;
	_superStringLen = strlen(_superString);
	_subStringStart = 0;
	_curSubString = 0;
	FormattingSuperString();

	CharacterContinueTalk();
}

} // End of namespace Trecision
