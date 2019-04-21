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

#include "titanic/game/end_sequence_control.h"
#include "titanic/translation.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CEndSequenceControl, CGameObject)
	ON_MESSAGE(TimerMsg)
	ON_MESSAGE(MovieEndMsg)
	ON_MESSAGE(EnterRoomMsg)
	ON_MESSAGE(EnterViewMsg)
END_MESSAGE_MAP()

void CEndSequenceControl::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	CGameObject::save(file, indent);
}

void CEndSequenceControl::load(SimpleFile *file) {
	file->readNumber();
	CGameObject::load(file);
}

bool CEndSequenceControl::TimerMsg(CTimerMsg *msg) {
	switch (msg->_actionVal) {
	case 1:
		changeView("TheEnd.Node 2.N");
		break;
	case 2: {
		playSound("ShipFlyingMusic.wav");
		CActMsg actMsg("TakeOff");
		actMsg.execute("EndExplodeShip");
		break;
	}

	default:
		break;
	}

	return true;
}

bool CEndSequenceControl::MovieEndMsg(CMovieEndMsg *msg) {
	setAmbientSoundVolume(VOL_MUTE, 2, -1);
	changeView("TheEnd.Node 3.N");
	addTimer(2, 1000, 0);
	return true;
}

bool CEndSequenceControl::EnterRoomMsg(CEnterRoomMsg *msg) {
	petHide();
	disableMouse();
	addTimer(1, 1000, 0);
	playAmbientSound(TRANSLATE("a#15.wav", "a#8.wav"), VOL_NORMAL, true, true, 0, Audio::Mixer::kSpeechSoundType);
	return true;
}

bool CEndSequenceControl::EnterViewMsg(CEnterViewMsg *msg) {
	playMovie(MOVIE_NOTIFY_OBJECT | MOVIE_WAIT_FOR_FINISH);
	movieSetPlaying(true);
	return true;
}

} // End of namespace Titanic
