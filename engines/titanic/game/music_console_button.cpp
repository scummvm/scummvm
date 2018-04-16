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

#include "titanic/game/music_console_button.h"
#include "titanic/core/room_item.h"
#include "titanic/sound/music_room.h"
#include "titanic/sound/music_room_handler.h"
#include "titanic/titanic.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CMusicConsoleButton, CMusicPlayer)
	ON_MESSAGE(MouseButtonDownMsg)
	ON_MESSAGE(LeaveViewMsg)
	ON_MESSAGE(SetMusicControlsMsg)
END_MESSAGE_MAP()

void CMusicConsoleButton::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	CMusicPlayer::save(file, indent);
}

void CMusicConsoleButton::load(SimpleFile *file) {
	file->readNumber();
	CMusicPlayer::load(file);
}

bool CMusicConsoleButton::MouseButtonDownMsg(CMouseButtonDownMsg *msg) {
	if (_isActive) {
		CStopMusicMsg stopMsg(this);
		stopMsg.execute(this);
		stopMovie();
		loadFrame(0);
	} else {
		CStartMusicMsg startMsg(this);
		startMsg.execute(this);
		playMovie(MOVIE_REPEAT);

		CMusicHasStartedMsg startedMsg;
		startedMsg.execute("Music Room Phonograph");

		if (CMusicRoom::_musicHandler->checkInstrument(SNAKE)
				&& CMusicRoom::_musicHandler->checkInstrument(PIANO)
				&& CMusicRoom::_musicHandler->checkInstrument(BASS)) {
			// All three instruments have the correct settings
			CCorrectMusicPlayedMsg correctMsg;
			correctMsg.execute(findRoom());
		}
	}

	return true;
}

bool CMusicConsoleButton::LeaveViewMsg(CLeaveViewMsg *msg) {
	if (_isActive) {
		// Stop playing the active music
		CStopMusicMsg stopMsg(this);
		stopMsg.execute(this);
		stopMovie();
		loadFrame(0);
	}

	return true;
}

bool CMusicConsoleButton::SetMusicControlsMsg(CSetMusicControlsMsg *msg) {
	CMusicRoom *musicRoom = getMusicRoom();
	CQueryMusicControlSettingMsg queryMsg;

	queryMsg.execute("Bells Pitch Control");
	musicRoom->setPitchControl(BELLS, queryMsg._value);
	queryMsg.execute("Bells Speed Control");
	musicRoom->setSpeedControl(BELLS, queryMsg._value);
	queryMsg.execute("Bells Inversion Control");
	musicRoom->setInversionControl(BELLS, queryMsg._value == 1);
	queryMsg.execute("Bells Direction Control");
	musicRoom->setDirectionControl(BELLS, queryMsg._value == 1);
	queryMsg.execute("Bells Mute Control");
	musicRoom->setMuteControl(BELLS, queryMsg._value == 1);

	queryMsg.execute("Snake Pitch Control");
	musicRoom->setPitchControl(SNAKE, queryMsg._value);
	queryMsg.execute("Snake Speed Control");
	musicRoom->setSpeedControl(SNAKE, queryMsg._value);
	queryMsg.execute("Snake Inversion Control");
	musicRoom->setInversionControl(SNAKE, queryMsg._value == 1);
	queryMsg.execute("Snake Direction Control");
	musicRoom->setDirectionControl(SNAKE, queryMsg._value == 1);
	queryMsg.execute("Snake Mute Control");
	musicRoom->setMuteControl(SNAKE, queryMsg._value == 1);

	queryMsg.execute("Piano Pitch Control");
	musicRoom->setPitchControl(PIANO, queryMsg._value);
	queryMsg.execute("Piano Speed Control");
	musicRoom->setSpeedControl(PIANO, queryMsg._value);
	queryMsg.execute("Piano Inversion Control");
	musicRoom->setInversionControl(PIANO, queryMsg._value == 1);
	queryMsg.execute("Piano Direction Control");
	musicRoom->setDirectionControl(PIANO, queryMsg._value == 1);
	queryMsg.execute("Piano Mute Control");
	musicRoom->setMuteControl(PIANO, queryMsg._value == 1);

	queryMsg.execute("Bass Pitch Control");
	musicRoom->setPitchControl(BASS, queryMsg._value);
	queryMsg.execute("Bass Speed Control");
	musicRoom->setSpeedControl(BASS, queryMsg._value);
	queryMsg.execute("Bass Inversion Control");
	musicRoom->setInversionControl(BASS, queryMsg._value == 1);
	queryMsg.execute("Bass Direction Control");
	musicRoom->setDirectionControl(BASS, queryMsg._value == 1);
	queryMsg.execute("Bass Mute Control");
	musicRoom->setMuteControl(BASS, queryMsg._value == 1);

	return true;
}

} // End of namespace Titanic
