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

#include "engines/wintermute/Base/BSound.h"
#include "engines/wintermute/Base/BGame.h"
#include "engines/wintermute/Base/BSoundMgr.h"

namespace WinterMute {

IMPLEMENT_PERSISTENT(CBSound, false)

//////////////////////////////////////////////////////////////////////////
CBSound::CBSound(CBGame *inGame): CBBase(inGame) {
	_sound = NULL;
	_soundFilename = NULL;

	_soundType = SOUND_SFX;
	_soundStreamed = false;
	_soundLooping = false;
	_soundPlaying = false;
	_soundPaused = false;
	_soundFreezePaused = false;
	_soundPosition = 0;
	_soundPrivateVolume = 0;
	_soundLoopStart = 0;

	_sFXType = SFX_NONE;
	_sFXParam1 = _sFXParam2 = _sFXParam3 = _sFXParam4 = 0;
}


//////////////////////////////////////////////////////////////////////////
CBSound::~CBSound() {
	if (_sound) Game->_soundMgr->removeSound(_sound);
	_sound = NULL;

	delete[] _soundFilename;
	_soundFilename = NULL;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBSound::SetSound(const char *Filename, TSoundType Type, bool Streamed) {
	if (_sound) {
		Game->_soundMgr->removeSound(_sound);
		_sound = NULL;
	}
	delete[] _soundFilename;
	_soundFilename = NULL;

	_sound = Game->_soundMgr->addSound(Filename, Type, Streamed);
	if (_sound) {
		_soundFilename = new char[strlen(Filename) + 1];
		strcpy(_soundFilename, Filename);

		_soundType = Type;
		_soundStreamed = Streamed;

		return S_OK;
	} else return E_FAIL;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBSound::SetSoundSimple() {
	_sound = Game->_soundMgr->addSound(_soundFilename, _soundType, _soundStreamed);
	if (_sound) {
		if (_soundPosition) _sound->SetPosition(_soundPosition);
		_sound->SetLooping(_soundLooping);
		_sound->SetPrivateVolume(_soundPrivateVolume);
		_sound->SetLoopStart(_soundLoopStart);
		_sound->_freezePaused = _soundFreezePaused;
		if (_soundPlaying) return _sound->Resume();
		else return S_OK;
	} else return E_FAIL;
}



//////////////////////////////////////////////////////////////////////////
uint32 CBSound::GetLength() {
	if (_sound) return _sound->GetLength();
	else return 0;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBSound::Play(bool Looping) {
	if (_sound) {
		_soundPaused = false;
		return _sound->Play(Looping, _soundPosition);
	} else return E_FAIL;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBSound::Stop() {
	if (_sound) {
		_soundPaused = false;
		return _sound->Stop();
	} else return E_FAIL;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBSound::Pause(bool FreezePaused) {
	if (_sound) {
		_soundPaused = true;
		if (FreezePaused) _sound->_freezePaused = true;
		return _sound->Pause();
	} else return E_FAIL;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBSound::Resume() {
	if (_sound && _soundPaused) {
		_soundPaused = false;
		return _sound->Resume();
	} else return E_FAIL;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBSound::persist(CBPersistMgr *persistMgr) {
	if (persistMgr->_saving && _sound) {
		_soundPlaying = _sound->IsPlaying();
		_soundLooping = _sound->_looping;
		_soundPrivateVolume = _sound->_privateVolume;
		if (_soundPlaying) _soundPosition = _sound->GetPosition();
		_soundLoopStart = _sound->_loopStart;
		_soundFreezePaused = _sound->_freezePaused;
	}

	if (persistMgr->_saving) {
		_sFXType = SFX_NONE;
		_sFXParam1 = _sFXParam2 = _sFXParam3 = _sFXParam4 = 0;
	}

	persistMgr->transfer(TMEMBER(Game));

	persistMgr->transfer(TMEMBER(_soundFilename));
	persistMgr->transfer(TMEMBER(_soundLooping));
	persistMgr->transfer(TMEMBER(_soundPaused));
	persistMgr->transfer(TMEMBER(_soundFreezePaused));
	persistMgr->transfer(TMEMBER(_soundPlaying));
	persistMgr->transfer(TMEMBER(_soundPosition));
	persistMgr->transfer(TMEMBER(_soundPrivateVolume));
	persistMgr->transfer(TMEMBER(_soundStreamed));
	persistMgr->transfer(TMEMBER_INT(_soundType));
	persistMgr->transfer(TMEMBER(_soundLoopStart));

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
bool CBSound::IsPlaying() {
	return _sound && _sound->IsPlaying();
}


//////////////////////////////////////////////////////////////////////////
bool CBSound::IsPaused() {
	return _sound && _soundPaused;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBSound::SetPositionTime(uint32 Time) {
	if (!_sound) return E_FAIL;
	_soundPosition = Time;
	HRESULT ret = _sound->SetPosition(_soundPosition);
	if (_sound->IsPlaying()) _soundPosition = 0;
	return ret;
}


//////////////////////////////////////////////////////////////////////////
uint32 CBSound::GetPositionTime() {
	if (!_sound) return 0;

	if (!_sound->IsPlaying()) return 0;
	else return _sound->GetPosition();
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBSound::SetVolume(int Volume) {
	if (!_sound) return E_FAIL;
	else return _sound->SetPrivateVolume(Volume);
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBSound::SetPrivateVolume(int Volume) {
	if (!_sound) return E_FAIL;
	else return _sound->_privateVolume = Volume;
}


//////////////////////////////////////////////////////////////////////////
int CBSound::GetVolume() {
	if (!_sound) return 0;
	else return _sound->_privateVolume;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBSound::SetLoopStart(uint32 Pos) {
	if (!_sound) return E_FAIL;
	else {
		_sound->SetLoopStart(Pos);
		return S_OK;
	}
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBSound::SetPan(float Pan) {
	if (_sound) return _sound->SetPan(Pan);
	else return E_FAIL;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBSound::ApplyFX(TSFXType Type, float Param1, float Param2, float Param3, float Param4) {
	if (!_sound) return S_OK;

	if (Type != _sFXType || Param1 != _sFXParam1 || Param2 != _sFXParam2 || Param3 != _sFXParam3 || Param4 != _sFXParam4) {
		HRESULT Ret = _sound->ApplyFX(Type, Param1, Param2, Param3, Param4);

		_sFXType = Type;
		_sFXParam1 = Param1;
		_sFXParam2 = Param2;
		_sFXParam3 = Param3;
		_sFXParam4 = Param4;

		return Ret;
	}
	return S_OK;
}

} // end of namespace WinterMute
