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
	file->writeNumberLine(_musicActive, indent);
	file->writeNumberLine(_volume, indent);

	CGameObject::save(file, indent);
}

void CMusicPlayer::load(SimpleFile *file) {
	file->readNumber();
	_isActive = file->readNumber();
	_stopTarget = file->readString();
	_musicActive = file->readNumber();
	_volume = file->readNumber();

	CGameObject::load(file);
}

bool CMusicPlayer::StartMusicMsg(CStartMusicMsg *msg) {
	if (msg->_musicPlayer != this) {
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

		getMusicRoom()->setupMusic(_volume);
		_isActive = true;
		unlockMouse();
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
	if (_isActive && !CMusicRoom::_musicHandler->update()) {
		getMusicRoom()->stopMusic();
		_isActive = false;

		CMusicHasStoppedMsg stoppedMsg;
		stoppedMsg.execute(_stopTarget);
	}

	return true;
}

bool CMusicPlayer::EnterRoomMsg(CEnterRoomMsg *msg) {
	// Set up a timer that will create a music handler
	addTimer(100);
	return true;
}

bool CMusicPlayer::LeaveRoomMsg(CLeaveRoomMsg *msg) {
	getMusicRoom()->destroyMusicHandler();
	return true;
}

bool CMusicPlayer::CreateMusicPlayerMsg(CCreateMusicPlayerMsg *msg) {
	if (CMusicRoom::_musicHandler) {
		CMusicRoom::_musicHandler->setActive(_musicActive);
		return true;
	}

	CMusicRoomHandler *musicHandler = getMusicRoom()->createMusicHandler();
	CMusicRoomInstrument *ins;

	if (musicHandler) {
		ins = musicHandler->createInstrument(BELLS, 3);
		ins->load(0, "z#490.wav", 60);
		ins->load(1, "z#488.wav", 62);
		ins->load(2, "z#489.wav", 63);

		ins = musicHandler->createInstrument(SNAKE, 5);
		ins->load(0, "z#493.wav", 22);
		ins->load(1, "z#495.wav", 29);
		ins->load(2, "z#492.wav", 34);
		ins->load(3, "z#494.wav", 41);
		ins->load(4, "z#491.wav", 46);

		ins = musicHandler->createInstrument(PIANO, 5);
		ins->load(0, "z#499.wav", 26);
		ins->load(1, "z#497.wav", 34);
		ins->load(2, "z#498.wav", 38);
		ins->load(3, "z#496.wav", 46);
		ins->load(4, "z#500.wav", 60);

		ins = musicHandler->createInstrument(BASS, 7);
		ins->load(0, "z#504.wav", 22);
		ins->load(1, "z#507.wav", 29);
		ins->load(2, "z#503.wav", 34);
		ins->load(3, "z#506.wav", 41);
		ins->load(4, "z#502.wav", 46);
		ins->load(5, "z#505.wav", 53);
		ins->load(6, "z#501.wav", 58);

		CMusicRoom::_musicHandler->setActive(_musicActive);
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
		// Music is meant to be playing, so restart it
		CStopMusicMsg stopMsg;
		stopMsg.execute(this);
		CStartMusicMsg startMsg;
		startMsg.execute(this);
	}

	return true;
}

} // End of namespace Titanic
