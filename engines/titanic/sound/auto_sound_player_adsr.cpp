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

#include "titanic/sound/auto_sound_player_adsr.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CAutoSoundPlayerADSR, CAutoSoundPlayer)
	ON_MESSAGE(TurnOn)
	ON_MESSAGE(TurnOff)
END_MESSAGE_MAP()

void CAutoSoundPlayerADSR::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeQuotedLine(_soundName1, indent);
	file->writeQuotedLine(_soundName2, indent);
	file->writeQuotedLine(_soundName3, indent);
	CAutoSoundPlayer::save(file, indent);
}

void CAutoSoundPlayerADSR::load(SimpleFile *file) {
	file->readNumber();
	_soundName1 = file->readString();
	_soundName2 = file->readString();
	_soundName3 = file->readString();
	CAutoSoundPlayer::load(file);
}

bool CAutoSoundPlayerADSR::TurnOn(CTurnOn *msg) {
	if (_soundHandle == -1) {
		if (!_soundName1.empty()) {
			_soundHandle = playSound(_soundName1, _volume, _balance);

			if (!_soundName2.empty())
				_soundHandle = queueSound(_soundName2, _soundHandle, _volume, _balance);

			_soundHandle = queueSound(_filename, _soundHandle, _volume, _balance);
			_active = true;
		}
	}

	return true;
}

bool CAutoSoundPlayerADSR::TurnOff(CTurnOff *msg) {
	if (_soundHandle != -1) {
		if (!_soundName3.empty())
			queueSound(_soundName3, _soundHandle, _volume, _balance);

		if (isSoundActive(_soundHandle))
			stopSound(_soundHandle);

		_soundHandle = -1;
		_active = false;
	}

	return true;
}

} // End of namespace Titanic
