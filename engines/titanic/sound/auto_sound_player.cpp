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

#include "titanic/sound/auto_sound_player.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CAutoSoundPlayer, CGameObject)
	ON_MESSAGE(TurnOn)
	ON_MESSAGE(TurnOff)
	ON_MESSAGE(SignalObject)
	ON_MESSAGE(SetVolumeMsg)
	ON_MESSAGE(LoadSuccessMsg)
END_MESSAGE_MAP()

CAutoSoundPlayer::CAutoSoundPlayer() : CGameObject(),
	_unused(0), _volume(70), _balance(0), _repeated(false), _soundHandle(-1),
	_stopSeconds(0), _startSeconds(-1), _active(false), _isVectorSound(false) {
}

void CAutoSoundPlayer::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_unused, indent);
	file->writeQuotedLine(_filename, indent);
	file->writeNumberLine(_volume, indent);
	file->writeNumberLine(_balance, indent);
	file->writeNumberLine(_repeated, indent);
	file->writeNumberLine(_soundHandle, indent);
	file->writeNumberLine(_stopSeconds, indent);
	file->writeNumberLine(_startSeconds, indent);
	file->writeNumberLine(_active, indent);
	file->writeNumberLine(_isVectorSound, indent);

	CGameObject::save(file, indent);
}

void CAutoSoundPlayer::load(SimpleFile *file) {
	file->readNumber();
	_unused = file->readNumber();
	_filename = file->readString();
	_volume = file->readNumber();
	_balance = file->readNumber();
	_repeated = file->readNumber();
	_soundHandle = file->readNumber();
	_stopSeconds = file->readNumber();
	_startSeconds = file->readNumber();
	_active = file->readNumber();
	_isVectorSound = file->readNumber();

	CGameObject::load(file);
}

bool CAutoSoundPlayer::TurnOn(CTurnOn *msg) {
	if (_soundHandle == -1) {
		CProximity prox;
		prox._balance = _balance;
		prox._repeated = _repeated;
		if (_isVectorSound)
			prox._positioningMode = POSMODE_VECTOR;
		prox._channelVolume = (_startSeconds == -1) ? _volume : 0;

		_soundHandle = playSound(_filename, prox);
		if (_startSeconds != -1)
			setSoundVolume(_soundHandle, _volume, _startSeconds);

		_active = true;
	}

	return true;
}

bool CAutoSoundPlayer::TurnOff(CTurnOff *msg) {
	if (_soundHandle != -1) {
		if (isSoundActive(_soundHandle))
			stopSound(_soundHandle, _stopSeconds);

		_soundHandle = -1;
		_active = false;
	}

	return true;
}

bool CAutoSoundPlayer::SignalObject(CSignalObject *msg) {
	if (_soundHandle != -1) {
		if (isSoundActive(_soundHandle))
			stopSound(_soundHandle, msg->_numValue);

		_soundHandle = -1;
		_active = false;
	}

	return true;
}

bool CAutoSoundPlayer::SetVolumeMsg(CSetVolumeMsg *msg) {
	if (_soundHandle != -1 && isSoundActive(_soundHandle))
		setSoundVolume(_soundHandle, msg->_volume, msg->_secondsTransition);

	return true;
}

bool CAutoSoundPlayer::LoadSuccessMsg(CLoadSuccessMsg *msg) {
	if (_active) {
		_soundHandle = -1;
		_active = false;

		CTurnOn onMsg;
		onMsg.execute(this);
	}

	return true;
}

} // End of namespace Titanic
