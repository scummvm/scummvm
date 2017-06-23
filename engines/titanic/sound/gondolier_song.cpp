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

#include "titanic/sound/gondolier_song.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CGondolierSong, CRoomAutoSoundPlayer)
	ON_MESSAGE(TurnOn)
	ON_MESSAGE(SignalObject)
	ON_MESSAGE(SetVolumeMsg)
	ON_MESSAGE(StatusChangeMsg)
END_MESSAGE_MAP()

void CGondolierSong::save(SimpleFile *file, int indent) {
	file->writeNumberLine(_enabled, indent);
	file->writeNumberLine(_value, indent);
	CRoomAutoSoundPlayer::save(file, indent);
}

void CGondolierSong::load(SimpleFile *file) {
	_enabled = file->readNumber();
	_value = file->readNumber();
	CRoomAutoSoundPlayer::load(file);
}

bool CGondolierSong::TurnOn(CTurnOn *msg) {
	if (_enabled) {
		if (_soundHandle != -1) {
			int volume = _value * _volume / 100;

			if (_startSeconds == -1) {
				_soundHandle = playSound(_filename, volume, _balance, _repeated);
			} else {
				_soundHandle = playSound(_filename, 0, _balance, _repeated);
				setSoundVolume(_soundHandle, _volume, _startSeconds);
			}

			_active = true;
		}
	}

	return true;
}

bool CGondolierSong::SignalObject(CSignalObject *msg) {
	_enabled = false;
	CAutoSoundPlayer::SignalObject(msg);
	return true;
}

bool CGondolierSong::SetVolumeMsg(CSetVolumeMsg *msg) {
	if (_enabled) {
		_volume = msg->_volume;

		if (_soundHandle != -1 && isSoundActive(_soundHandle)) {
			int newVolume = _value * _volume / 100;
			setSoundVolume(_soundHandle, newVolume, msg->_secondsTransition);
		}
	}

	return true;
}

bool CGondolierSong::StatusChangeMsg(CStatusChangeMsg *msg) {
	if (_enabled) {
		_value = CLIP(msg->_newStatus, 0, 100);
		CSetVolumeMsg volumeMsg(_volume, _stopSeconds);
		volumeMsg.execute(this);
	}

	return true;
}

} // End of namespace Titanic
