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

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#include "engines/wintermute/dcgf.h"
#include "engines/wintermute/Ad/AdSentence.h"
#include "engines/wintermute/Ad/AdTalkDef.h"
#include "engines/wintermute/Ad/AdTalkNode.h"
#include "engines/wintermute/Ad/AdGame.h"
#include "engines/wintermute/PathUtil.h"
#include "engines/wintermute/Base/BGame.h"
#include "engines/wintermute/Base/BSound.h"
#include "engines/wintermute/Ad/AdScene.h"
#include "engines/wintermute/Base/BFont.h"
#include "engines/wintermute/Base/BSprite.h"
#include "engines/wintermute/Base/BFileManager.h"

namespace WinterMute {

IMPLEMENT_PERSISTENT(CAdSentence, false)

//////////////////////////////////////////////////////////////////////////
CAdSentence::CAdSentence(CBGame *inGame): CBBase(inGame) {
	_text = NULL;
	_stances = NULL;
	_tempStance = NULL;

	_duration = 0;
	_startTime = 0;
	_currentStance = 0;

	_font = NULL;

	_pos.x = _pos.y = 0;
	_width = Game->_renderer->_width;

	_align = (TTextAlign)TAL_CENTER;

	_sound = NULL;
	_soundStarted = false;

	_talkDef = NULL;
	_currentSprite = NULL;
	_currentSkelAnim = NULL;
	_fixedPos = false;
	_freezable = true;
}


//////////////////////////////////////////////////////////////////////////
CAdSentence::~CAdSentence() {
	delete _sound;
	delete[] _text;
	delete[] _stances;
	delete[] _tempStance;
	delete _talkDef;
	_sound = NULL;
	_text = NULL;
	_stances = NULL;
	_tempStance = NULL;
	_talkDef = NULL;

	_currentSprite = NULL; // ref only
	_currentSkelAnim = NULL;
	_font = NULL; // ref only
}


//////////////////////////////////////////////////////////////////////////
void CAdSentence::SetText(const char *Text) {
	if (_text) delete [] _text;
	_text = new char[strlen(Text) + 1];
	if (_text) strcpy(_text, Text);
}


//////////////////////////////////////////////////////////////////////////
void CAdSentence::SetStances(const char *Stances) {
	if (_stances) delete [] _stances;
	if (Stances) {
		_stances = new char[strlen(Stances) + 1];
		if (_stances) strcpy(_stances, Stances);
	} else _stances = NULL;
}


//////////////////////////////////////////////////////////////////////////
char *CAdSentence::GetCurrentStance() {
	return GetStance(_currentStance);
}


//////////////////////////////////////////////////////////////////////////
char *CAdSentence::GetNextStance() {
	_currentStance++;
	return GetStance(_currentStance);
}


//////////////////////////////////////////////////////////////////////////
char *CAdSentence::GetStance(int Stance) {
	if (_stances == NULL) return NULL;

	if (_tempStance) delete [] _tempStance;
	_tempStance = NULL;

	char *start;
	char *curr;
	int pos;

	if (Stance == 0) start = _stances;
	else {
		pos = 0;
		start = NULL;
		curr = _stances;
		while (pos < Stance) {
			if (*curr == '\0') break;
			if (*curr == ',') pos++;
			curr++;
		}
		if (pos == Stance) start = curr;
	}

	if (start == NULL) return NULL;

	while (*start == ' ' && *start != ',' && *start != '\0') start++;

	curr = start;
	while (*curr != '\0' && *curr != ',') curr++;

	while (curr > start && *(curr - 1) == ' ') curr--;

	_tempStance = new char [curr - start + 1];
	if (_tempStance) {
		_tempStance[curr - start] = '\0';
		strncpy(_tempStance, start, curr - start);
	}

	return _tempStance;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdSentence::Display() {
	if (!_font || !_text) return E_FAIL;

	if (_sound && !_soundStarted) {
		_sound->Play();
		_soundStarted = true;
	}

	if (Game->_subtitles) {
		int x = _pos.x;
		int y = _pos.y;

		if (!_fixedPos) {
			x = x - ((CAdGame *)Game)->_scene->GetOffsetLeft();
			y = y - ((CAdGame *)Game)->_scene->GetOffsetTop();
		}


		x = MAX(x, 0);
		x = MIN(x, Game->_renderer->_width - _width);
		y = MAX(y, 0);

		_font->DrawText((byte *)_text, x, y, _width, _align);
	}

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
void CAdSentence::SetSound(CBSound *Sound) {
	if (!Sound) return;
	delete _sound;
	_sound = Sound;
	_soundStarted = false;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdSentence::Finish() {
	if (_sound) _sound->Stop();
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdSentence::Persist(CBPersistMgr *PersistMgr) {

	PersistMgr->Transfer(TMEMBER(Game));

	PersistMgr->Transfer(TMEMBER_INT(_align));
	PersistMgr->Transfer(TMEMBER(_currentStance));
	PersistMgr->Transfer(TMEMBER(_currentSprite));
	PersistMgr->Transfer(TMEMBER(_currentSkelAnim));
	PersistMgr->Transfer(TMEMBER(_duration));
	PersistMgr->Transfer(TMEMBER(_font));
	PersistMgr->Transfer(TMEMBER(_pos));
	PersistMgr->Transfer(TMEMBER(_sound));
	PersistMgr->Transfer(TMEMBER(_soundStarted));
	PersistMgr->Transfer(TMEMBER(_stances));
	PersistMgr->Transfer(TMEMBER(_startTime));
	PersistMgr->Transfer(TMEMBER(_talkDef));
	PersistMgr->Transfer(TMEMBER(_tempStance));
	PersistMgr->Transfer(TMEMBER(_text));
	PersistMgr->Transfer(TMEMBER(_width));
	PersistMgr->Transfer(TMEMBER(_fixedPos));
	PersistMgr->Transfer(TMEMBER(_freezable));

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdSentence::SetupTalkFile(const char *SoundFilename) {
	delete _talkDef;
	_talkDef = NULL;
	_currentSprite = NULL;

	if (!SoundFilename) return S_OK;


	AnsiString path = PathUtil::GetDirectoryName(SoundFilename);
	AnsiString name = PathUtil::GetFileNameWithoutExtension(SoundFilename);

	AnsiString talkDefFileName = PathUtil::Combine(path, name + ".talk");

	CBFile *file = Game->_fileManager->OpenFile(talkDefFileName.c_str());
	if (file) {
		Game->_fileManager->CloseFile(file);
	} else return S_OK; // no talk def file found


	_talkDef = new CAdTalkDef(Game);
	if (!_talkDef || FAILED(_talkDef->LoadFile(talkDefFileName.c_str()))) {
		delete _talkDef;
		_talkDef = NULL;
		return E_FAIL;
	}
	//Game->LOG(0, "Using .talk file: %s", TalkDefFile);

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdSentence::Update(TDirection Dir) {
	if (!_talkDef) return S_OK;

	uint32 CurrentTime;
	// if sound is available, synchronize with sound, otherwise use timer

	/*
	if(_sound) CurrentTime = _sound->GetPositionTime();
	else CurrentTime = Game->_timer - _startTime;
	*/
	CurrentTime = Game->_timer - _startTime;

	bool TalkNodeFound = false;
	for (int i = 0; i < _talkDef->_nodes.GetSize(); i++) {
		if (_talkDef->_nodes[i]->IsInTimeInterval(CurrentTime, Dir)) {
			TalkNodeFound = true;

			CBSprite *NewSprite = _talkDef->_nodes[i]->GetSprite(Dir);
			if (NewSprite != _currentSprite) NewSprite->Reset();
			_currentSprite = NewSprite;

			if (!_talkDef->_nodes[i]->_playToEnd) break;
		}
	}


	// no talk node, try to use default sprite instead (if any)
	if (!TalkNodeFound) {
		CBSprite *NewSprite = _talkDef->GetDefaultSprite(Dir);
		if (NewSprite) {
			if (NewSprite != _currentSprite) NewSprite->Reset();
			_currentSprite = NewSprite;
		} else _currentSprite = NULL;
	}

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
bool CAdSentence::CanSkip() {
	// prevent accidental sentence skipping (TODO make configurable)
	return (Game->_timer - _startTime) > 300;
}

} // end of namespace WinterMute
