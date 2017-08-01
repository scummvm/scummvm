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

#include "titanic/sound/seasonal_music_player.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CSeasonalMusicPlayer, CAutoMusicPlayerBase)
	ON_MESSAGE(ChangeSeasonMsg)
	ON_MESSAGE(ArboretumGateMsg)
	ON_MESSAGE(ChangeMusicMsg)
END_MESSAGE_MAP()

CSeasonalMusicPlayer::CSeasonalMusicPlayer() : CAutoMusicPlayerBase() {
	_isSpring = false;
	_isSummer = true;
	_isAutumn = false;
	_isWinter = false;
	_springMode = VOL_MUTE;
	_summerMode = VOL_QUIET;
	_autumnMode = VOL_MUTE;
	_winterMode = VOL_MUTE;
}

void CSeasonalMusicPlayer::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_isSpring, indent);
	file->writeNumberLine(_isSummer, indent);
	file->writeNumberLine(_isAutumn, indent);
	file->writeNumberLine(_isWinter, indent);
	file->writeNumberLine(_springMode, indent);
	file->writeNumberLine(_summerMode, indent);
	file->writeNumberLine(_autumnMode, indent);
	file->writeNumberLine(_winterMode, indent);

	CAutoMusicPlayerBase::save(file, indent);
}

void CSeasonalMusicPlayer::load(SimpleFile *file) {
	file->readNumber();
	_isSpring = file->readNumber();
	_isSummer = file->readNumber();
	_isAutumn = file->readNumber();
	_isWinter = file->readNumber();
	_springMode = (VolumeMode)file->readNumber();
	_summerMode = (VolumeMode)file->readNumber();
	_autumnMode = (VolumeMode)file->readNumber();
	_winterMode = (VolumeMode)file->readNumber();

	CAutoMusicPlayerBase::load(file);
}

bool CSeasonalMusicPlayer::ChangeSeasonMsg(CChangeSeasonMsg *msg) {
	_isSpring = msg->_season == "spring";
	_isSummer = msg->_season == "summer";
	_isAutumn = msg->_season == "autumn";
	_isWinter = msg->_season == "winter";

	_springMode = _isSpring ? VOL_QUIET : VOL_MUTE;
	_summerMode = _isSummer ? VOL_QUIET : VOL_MUTE;
	_autumnMode = _isAutumn ? VOL_QUIET : VOL_MUTE;
	_winterMode = _isWinter ? VOL_QUIET : VOL_MUTE;

	CChangeMusicMsg changeMsg;
	changeMsg._filename = msg->_season;
	changeMsg.execute(this);

	return true;
}

bool CSeasonalMusicPlayer::ArboretumGateMsg(CArboretumGateMsg *msg) {
	CChangeMusicMsg changeMsg;
	changeMsg._flags = msg->_value ? 2 : 1;
	changeMsg.execute(this);

	return true;
}

bool CSeasonalMusicPlayer::ChangeMusicMsg(CChangeMusicMsg *msg) {
	if (_isRepeated && msg->_flags == 1) {
		_isRepeated = false;
		stopGlobalSound(_transition, -1);
	}

	if (!msg->_filename.empty()) {
		if (_isSummer) {
			setGlobalSoundVolume(VOL_MUTE, 2, 0);
			setGlobalSoundVolume(VOL_QUIET, 2, 1);
		} else if (_isAutumn) {
			setGlobalSoundVolume(VOL_MUTE, 2, 1);
			setGlobalSoundVolume(VOL_QUIET, 2, 2);
		} else if (_isWinter) {
			setGlobalSoundVolume(VOL_MUTE, 2, 2);
			setGlobalSoundVolume(VOL_QUIET, 2, 3);
		} else if (_isSpring) {
			setGlobalSoundVolume(VOL_MUTE, 2, 3);
			setGlobalSoundVolume(VOL_QUIET, 2, 0);
		}
	}

	if (!_isRepeated && msg->_flags == 2) {
		_isRepeated = true;
		loadSound("c#64.wav");
		loadSound("c#63.wav");
		loadSound("c#65.wav");
		loadSound("c#62.wav");
		playGlobalSound("c#64.wav", _springMode, _isSpring, true, 0);
		playGlobalSound("c#63.wav", _summerMode, _isSummer, true, 1);
		playGlobalSound("c#65.wav", _autumnMode, _isAutumn, true, 2);
		playGlobalSound("c#62.wav", _winterMode, _isWinter, true, 3);
	}

	return true;
}

} // End of namespace Titanic
