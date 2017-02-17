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

#include "titanic/game/computer_screen.h"
#include "titanic/messages/messages.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CComputerScreen, CGameObject)
	ON_MESSAGE(ActMsg)
	ON_MESSAGE(MovieEndMsg)
	ON_MESSAGE(EnterViewMsg)
	ON_MESSAGE(TimerMsg)
END_MESSAGE_MAP()

CComputerScreen::CComputerScreen() : CGameObject() {
}

void CComputerScreen::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	CGameObject::save(file, indent);
}

void CComputerScreen::load(SimpleFile *file) {
	file->readNumber();
	CGameObject::load(file);
}

bool CComputerScreen::ActMsg(CActMsg *msg) {
	if (msg->_action == "newCD1" || msg->_action == "newCD2") {
		playMovie(27, 53, MOVIE_WAIT_FOR_FINISH);
		playMovie(19, 26, MOVIE_WAIT_FOR_FINISH);
	} else if (msg->_action == "newSTCD") {
		playMovie(0, 18, MOVIE_WAIT_FOR_FINISH | MOVIE_NOTIFY_OBJECT);
	}

	return true;
}

bool CComputerScreen::MovieEndMsg(CMovieEndMsg *msg) {
	playSound("z#47.wav");
	addTimer(0, 3000, 0);

	for (int idx = 0; idx < 10; ++idx)
		playMovie(0, 18, 0);
	return true;
}

bool CComputerScreen::EnterViewMsg(CEnterViewMsg *msg) {
	loadFrame(26);
	return true;
}

bool CComputerScreen::TimerMsg(CTimerMsg *msg) {
	int handle;

	switch (msg->_actionVal) {
	case 0:
		loadSound("a#32.wav");
		loadSound("a#31.wav");
		loadSound("a#33.wav");
		loadSound("a#30.wav");
		loadSound("a#29.wav");
		playSound("a#25.wav");
		addTimer(1, 2000, 0);
		break;

	case 1:
		playSound("a#32.wav");
		playSound("a#31.wav");
		addTimer(2, 2000, 0);
		break;

	case 2: {
		CChangeMusicMsg musicMsg(CString(), 1);
		musicMsg.execute("HomeMusicPlayer");
		playSound("a#33.wav");
		playSound("a#31.wav");
		changeView("Home.Node 4.E", "");
		playClip(51, 150);
		playSound("a#31.wav");
		playClip(151, 200);

		handle = playSound("a#27.wav");
		playClip(200, 306);
		playSound("a#30.wav");
		stopSound(handle, 0);

		playClip(306, 338);
		handle = playSound("a#28.wav");
		playClip(338, 392);
		playSound("a#29.wav");
		stopSound(handle);

		handle = playSound("y#662.wav");
		setSoundVolume(handle, 10, 2);
		playClip(392, 450);
		startTalking("Doorbot", 0x3611A);
		sleep(8000);

		playClip(450, 492);
		startTalking("Doorbot", 0x36121);
		playClip(492, 522);
		setSoundVolume(handle, 30, 2);

		playClip(523, 540);
		setSoundVolume(handle, 0, 1);

		playClip(541, 551);
		stopSound(handle);
		break;
	}

	default:
		break;
	}

	return true;
}

} // End of namespace Titanic
