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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#include "engines/wintermute/ad/ad_game.h"
#include "engines/wintermute/ad/ad_scene.h"
#include "engines/wintermute/ad/ad_sentence.h"
#include "engines/wintermute/ad/ad_talk_def.h"
#include "engines/wintermute/ad/ad_talk_node.h"
#include "engines/wintermute/utils/path_util.h"
#include "engines/wintermute/base/base_game.h"
#include "engines/wintermute/base/base_sprite.h"
#include "engines/wintermute/base/base_file_manager.h"
#include "engines/wintermute/base/font/base_font.h"
#include "engines/wintermute/base/gfx/base_renderer.h"
#include "engines/wintermute/base/sound/base_sound.h"

namespace Wintermute {

IMPLEMENT_PERSISTENT(AdSentence, false)

//////////////////////////////////////////////////////////////////////////
AdSentence::AdSentence(BaseGame *inGame) : BaseClass(inGame) {
	_text = nullptr;
	_stances = nullptr;
	_tempStance = nullptr;

	_duration = 0;
	_startTime = 0;
	_currentStance = 0;

	_font = nullptr;

	_pos.x = _pos.y = 0;
	_width = _gameRef->_renderer->getWidth();

	_align = (TTextAlign)TAL_CENTER;

	_sound = nullptr;
	_soundStarted = false;

	_talkDef = nullptr;
	_currentSprite = nullptr;
	_currentSkelAnim = nullptr;
	_fixedPos = false;
	_freezable = true;
}


//////////////////////////////////////////////////////////////////////////
AdSentence::~AdSentence() {
	delete _sound;
	delete[] _text;
	delete[] _stances;
	delete[] _tempStance;
	delete _talkDef;
	_sound = nullptr;
	_text = nullptr;
	_stances = nullptr;
	_tempStance = nullptr;
	_talkDef = nullptr;

	_currentSprite = nullptr; // ref only
	_currentSkelAnim = nullptr;
	_font = nullptr; // ref only
}


//////////////////////////////////////////////////////////////////////////
void AdSentence::setText(const char *text) {
	if (_text) {
		delete[] _text;
	}
	_text = new char[strlen(text) + 1];
	if (_text) {
		strcpy(_text, text);
	}
}


//////////////////////////////////////////////////////////////////////////
void AdSentence::setStances(const char *stances) {
	if (_stances) {
		delete[] _stances;
	}
	if (stances) {
		_stances = new char[strlen(stances) + 1];
		if (_stances) {
			strcpy(_stances, stances);
		}
	} else {
		_stances = nullptr;
	}
}


//////////////////////////////////////////////////////////////////////////
char *AdSentence::getCurrentStance() {
	return getStance(_currentStance);
}


//////////////////////////////////////////////////////////////////////////
char *AdSentence::getNextStance() {
	_currentStance++;
	return getStance(_currentStance);
}


//////////////////////////////////////////////////////////////////////////
char *AdSentence::getStance(int stance) {
	if (_stances == nullptr) {
		return nullptr;
	}

	if (_tempStance) {
		delete[] _tempStance;
	}
	_tempStance = nullptr;

	char *start;
	char *curr;
	int pos;

	if (stance == 0) {
		start = _stances;
	} else {
		pos = 0;
		start = nullptr;
		curr = _stances;
		while (pos < stance) {
			if (*curr == '\0') {
				break;
			}
			if (*curr == ',') {
				pos++;
			}
			curr++;
		}
		if (pos == stance) {
			start = curr;
		}
	}

	if (start == nullptr) {
		return nullptr;
	}

	while (*start == ' ' && *start != ',' && *start != '\0') {
		start++;
	}

	curr = start;
	while (*curr != '\0' && *curr != ',') {
		curr++;
	}

	while (curr > start && *(curr - 1) == ' ') {
		curr--;
	}

	_tempStance = new char [curr - start + 1];
	if (_tempStance) {
		Common::strlcpy(_tempStance, start, curr - start + 1);
	}

	return _tempStance;
}


//////////////////////////////////////////////////////////////////////////
bool AdSentence::display() {
	if (!_font || !_text) {
		return STATUS_FAILED;
	}

	if (_sound && !_soundStarted) {
		_sound->play();
		_soundStarted = true;
	}

	if (_gameRef->_subtitles) {
		int32 x = _pos.x;
		int32 y = _pos.y;

		if (!_fixedPos) {
			x = x - ((AdGame *)_gameRef)->_scene->getOffsetLeft();
			y = y - ((AdGame *)_gameRef)->_scene->getOffsetTop();
		}


		x = MAX<int32>(x, 0);
		x = MIN(x, _gameRef->_renderer->getWidth() - _width);
		y = MAX<int32>(y, 0);

		_font->drawText((byte *)_text, x, y, _width, _align);
	}

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
void AdSentence::setSound(BaseSound *sound) {
	if (!sound) {
		return;
	}
	delete _sound;
	_sound = sound;
	_soundStarted = false;
}


//////////////////////////////////////////////////////////////////////////
bool AdSentence::finish() {
	if (_sound) {
		_sound->stop();
	}
	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool AdSentence::persist(BasePersistenceManager *persistMgr) {

	persistMgr->transferPtr(TMEMBER_PTR(_gameRef));

	persistMgr->transferSint32(TMEMBER_INT(_align));
	persistMgr->transferSint32(TMEMBER(_currentStance));
	persistMgr->transferPtr(TMEMBER_PTR(_currentSprite));
	persistMgr->transferCharPtr(TMEMBER(_currentSkelAnim));
	persistMgr->transferUint32(TMEMBER(_duration));
	persistMgr->transferPtr(TMEMBER_PTR(_font));
	persistMgr->transferPoint32(TMEMBER(_pos));
	persistMgr->transferPtr(TMEMBER_PTR(_sound));
	persistMgr->transferBool(TMEMBER(_soundStarted));
	persistMgr->transferCharPtr(TMEMBER(_stances));
	persistMgr->transferUint32(TMEMBER(_startTime));
	persistMgr->transferPtr(TMEMBER_PTR(_talkDef));
	persistMgr->transferCharPtr(TMEMBER(_tempStance));
	persistMgr->transferCharPtr(TMEMBER(_text));
	persistMgr->transferSint32(TMEMBER(_width));
	persistMgr->transferBool(TMEMBER(_fixedPos));
	persistMgr->transferBool(TMEMBER(_freezable));

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool AdSentence::setupTalkFile(const char *soundFilename) {
	delete _talkDef;
	_talkDef = nullptr;
	_currentSprite = nullptr;

	if (!soundFilename) {
		return STATUS_OK;
	}


	AnsiString path = PathUtil::getDirectoryName(soundFilename);
	AnsiString name = PathUtil::getFileNameWithoutExtension(soundFilename);

	AnsiString talkDefFileName = PathUtil::combine(path, name + ".talk");

	if (!BaseFileManager::getEngineInstance()->hasFile(talkDefFileName)) {
		return STATUS_OK;    // no talk def file found
	}

	_talkDef = new AdTalkDef(_gameRef);
	if (!_talkDef || DID_FAIL(_talkDef->loadFile(talkDefFileName.c_str()))) {
		delete _talkDef;
		_talkDef = nullptr;
		return STATUS_FAILED;
	}
	//_gameRef->LOG(0, "Using .talk file: %s", TalkDefFile);

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool AdSentence::update(TDirection dir) {
	if (!_talkDef) {
		return STATUS_OK;
	}

	uint32 currentTime;
	// if sound is available, synchronize with sound, otherwise use timer

	/*
	if (_sound) CurrentTime = _sound->GetPositionTime();
	else CurrentTime = _gameRef->getTimer()->getTime() - _startTime;
	*/
	currentTime = _gameRef->getTimer()->getTime() - _startTime;

	bool talkNodeFound = false;
	for (uint32 i = 0; i < _talkDef->_nodes.size(); i++) {
		if (_talkDef->_nodes[i]->isInTimeInterval(currentTime, dir)) {
			talkNodeFound = true;

			BaseSprite *newSprite = _talkDef->_nodes[i]->getSprite(dir);
			if (newSprite != _currentSprite) {
				newSprite->reset();
			}
			_currentSprite = newSprite;

			if (!_talkDef->_nodes[i]->_playToEnd) {
				break;
			}
		}
	}


	// no talk node, try to use default sprite instead (if any)
	if (!talkNodeFound) {
		BaseSprite *newSprite = _talkDef->getDefaultSprite(dir);
		if (newSprite) {
			if (newSprite != _currentSprite) {
				newSprite->reset();
			}
			_currentSprite = newSprite;
		} else {
			_currentSprite = nullptr;
		}
	}

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool AdSentence::canSkip() {
	// prevent accidental sentence skipping (TODO make configurable)
	return (_gameRef->getTimer()->getTime() - _startTime) > 300;
}

} // End of namespace Wintermute
