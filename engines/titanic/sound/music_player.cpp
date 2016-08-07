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

#include "titanic/sound/music_player.h"
#include "titanic/sound/music_room.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CMusicPlayer, CGameObject)
	ON_MESSAGE(StartMusicMsg)
	ON_MESSAGE(StopMusicMsg)
	ON_MESSAGE(FrameMsg)
	ON_MESSAGE(EnterRoomMsg)
	ON_MESSAGE(LeaveRoomMsg)
	ON_MESSAGE(CreateMusicPlayerMsg)
	ON_MESSAGE(LoadSuccessMsg)
END_MESSAGE_MAP()

void CMusicPlayer::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_isActive, indent);
	file->writeQuotedLine(_stopTarget, indent);
	file->writeNumberLine(_fieldCC, indent);
	file->writeNumberLine(_musicId, indent);

	CGameObject::save(file, indent);
}

void CMusicPlayer::load(SimpleFile *file) {
	file->readNumber();
	_isActive = file->readNumber();
	_stopTarget = file->readString();
	_fieldCC = file->readNumber();
	_musicId = file->readNumber();

	CGameObject::load(file);
}

bool CMusicPlayer::StartMusicMsg(CStartMusicMsg *msg) {
	if (msg->_musicPlayer == this) {
		if (_isActive) {
			CStopMusicMsg stopMusicMsg;
			stopMusicMsg.execute(this);
		}

		return false;
	}

	if (!_isActive) {
		lockMouse();

		CCreateMusicPlayerMsg createMsg;
		createMsg.execute(this);
		CSetMusicControlsMsg controlsMsg;
		controlsMsg.execute(this, nullptr, MSGFLAG_SCAN);

		getMusicRoom()->startMusic(_musicId);
		_isActive = true;
	}

	return true;
}

bool CMusicPlayer::StopMusicMsg(CStopMusicMsg *msg) {
	if (!_isActive)
		// Player isn't playing, so ignore message
		return false;

	// Stop the music
	CMusicRoom *musicRoom = getMusicRoom();
	if (musicRoom)
		musicRoom->stopMusic();
	_isActive = false;

	CMusicHasStoppedMsg stoppedMsg;
	stoppedMsg.execute(_stopTarget, nullptr, MSGFLAG_SCAN);
	return true;
}

bool CMusicPlayer::FrameMsg(CFrameMsg *msg) {
	if (_isActive && !CMusicRoom::_musicHandler->isBusy()) {
		getMusicRoom()->stopMusic();
		_isActive = false;

		CMusicHasStoppedMsg stoppedMsg;
		stoppedMsg.execute(_stopTarget);
	}

	return true;
}

bool CMusicPlayer::EnterRoomMsg(CEnterRoomMsg *msg) {
	addTimer(100);
	return true;
}

bool CMusicPlayer::LeaveRoomMsg(CLeaveRoomMsg *msg) {
	getMusicRoom()->destroyMusicHandler();
	return true;
}

bool CMusicPlayer::CreateMusicPlayerMsg(CCreateMusicPlayerMsg *msg) {
	if (CMusicRoom::_musicHandler) {
		CMusicRoom::_musicHandler->set124(_fieldCC);
		return true;
	}

	CMusicHandler *musicHandler = getMusicRoom()->createMusicHandler();
	if (musicHandler) {
		// TODO

		CMusicRoom::_musicHandler->set124(_fieldCC);
	}

	return true;
}

bool CMusicPlayer::TimerMsg(CTimerMsg *msg) {
	CCreateMusicPlayerMsg playerMsg;
	playerMsg.execute(this);
	return true;
}

bool CMusicPlayer::LoadSuccessMsg(CLoadSuccessMsg *msg) {
	if (_isActive) {
		CStopMusicMsg stopMsg;
		stopMsg.execute(this);
		CStartMusicMsg startMsg;
		startMsg.execute(this);
	}

	return true;
}

} // End of namespace Titanic
