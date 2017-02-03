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

#include "titanic/sound/auto_music_player_base.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CAutoMusicPlayerBase, CGameObject)
	ON_MESSAGE(StatusChangeMsg)
	ON_MESSAGE(TimerMsg)
	ON_MESSAGE(LoadSuccessMsg)
	ON_MESSAGE(ChangeMusicMsg)
END_MESSAGE_MAP()

CAutoMusicPlayerBase::CAutoMusicPlayerBase() : CGameObject(),
	_initialMute(true), _isRepeated(false), _volumeMode(-1), _transition(1) {
}
void CAutoMusicPlayerBase::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeQuotedLine(_filename, indent);
	file->writeNumberLine(_initialMute, indent);
	file->writeNumberLine(_isRepeated, indent);
	file->writeNumberLine(_volumeMode, indent);
	file->writeNumberLine(_transition, indent);

	CGameObject::save(file, indent);
}

void CAutoMusicPlayerBase::load(SimpleFile *file) {
	file->readNumber();
	_filename = file->readString();
	_initialMute = file->readNumber();
	_isRepeated = file->readNumber();
	_volumeMode = file->readNumber();
	_transition = file->readNumber();

	CGameObject::load(file);
}

bool CAutoMusicPlayerBase::StatusChangeMsg(CStatusChangeMsg *msg) {
	return true;
}

bool CAutoMusicPlayerBase::TimerMsg(CTimerMsg *msg) {
	CChangeMusicMsg musicMsg;
	musicMsg._flags = 2;
	musicMsg.execute(this);

	return true;
}

bool CAutoMusicPlayerBase::LoadSuccessMsg(CLoadSuccessMsg *msg) {
	if (_isRepeated)
		playGlobalSound(_filename, _volumeMode, _initialMute, true, 0);

	return true;
}

bool CAutoMusicPlayerBase::ChangeMusicMsg(CChangeMusicMsg *msg) {
	if (_isRepeated && msg->_flags == 1) {
		_isRepeated = false;
		stopGlobalSound(_transition, -1);
	}

	if (!msg->_filename.empty()) {
		_filename = msg->_filename;

		if (_isRepeated) {
			stopGlobalSound(_transition, -1);
			playGlobalSound(_filename, _volumeMode, _initialMute, true, 0);
		}
	}

	if (!_isRepeated && msg->_flags == 2) {
		_isRepeated = true;
		playGlobalSound(_filename, _volumeMode, _initialMute, true, 0);
	}

	return true;
}

} // End of namespace Titanic
