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
#include "titanic/translation.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CComputerScreen, CGameObject)
	ON_MESSAGE(ActMsg)
	ON_MESSAGE(MovieEndMsg)
	ON_MESSAGE(EnterViewMsg)
	ON_MESSAGE(TimerMsg)
	ON_MESSAGE(MovementMsg)
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
	playSound(TRANSLATE("z#47.wav", "z#578.wav"));
	addTimer(0, 3000, 0);

	for (int idx = 0; idx < 10; ++idx)
		playMovie(0, 18, 0);
	return true;
}

bool CComputerScreen::EnterViewMsg(CEnterViewMsg *msg) {
	loadFrame(26);

	// WORKAROUND: The original game leaves in a debug link that
	// allows skipping of Doorbot arrival sequence. Disable it
	static_cast<CLinkItem *>(getParent()->findByName("_TRACK,3,e-cu,4,E"))->_bounds.clear();

	return true;
}

bool CComputerScreen::MovementMsg(CMovementMsg *msg) {
	if (msg->_movement != MOVE_BACKWARDS)
		return true;

	msg->_posToUse = Common::Point(320, 50);
	return false;
}

bool CComputerScreen::TimerMsg(CTimerMsg *msg) {
	int handle;

	switch (msg->_actionVal) {
	case 0:
		if (g_language == Common::DE_DEU) {
			loadSound("a#27.wav");
			loadSound("a#26.wav");
			loadSound("a#28.wav");
			loadSound("a#25.wav");
			loadSound("a#24.wav");
			playSound("a#20.wav");
		} else {
			loadSound("a#32.wav");
			loadSound("a#31.wav");
			loadSound("a#33.wav");
			loadSound("a#30.wav");
			loadSound("a#29.wav");
			playSound("a#25.wav");
		}
		addTimer(1, 2000, 0);
		break;

	case 1:
		playMovie(23, 26, MOVIE_STOP_PREVIOUS);
		playSound(TRANSLATE("a#32.wav", "a#27.wav"));
		playSound(TRANSLATE("a#31.wav", "a#26.wav"));
		addTimer(2, 2000, 0);
		break;

	case 2: {
		CChangeMusicMsg musicMsg(CString(), MUSIC_STOP);
		musicMsg.execute("HomeMusicPlayer");
		playSound(TRANSLATE("a#33.wav", "a#28.wav"));
		playSound(TRANSLATE("a#31.wav", "a#26.wav"));
		changeView("Home.Node 4.E", "");
		playClip(51, 150);
		playSound(TRANSLATE("a#31.wav", "a#26.wav"));
		playClip(151, 200);

		handle = playSound(TRANSLATE("a#27.wav", "a#22.wav"));
		playClip(200, 306);
		playSound(TRANSLATE("a#30.wav", "a#25.wav"));
		stopSound(handle, 0);

		playClip(306, 338);
		handle = playSound(TRANSLATE("a#28.wav", "a#23.wav"));
		playClip(338, 392);
		playSound(TRANSLATE("a#29.wav", "a#24.wav"));
		stopSound(handle);

		handle = playSound(TRANSLATE("y#662.wav", "y#0.wav"));
		setSoundVolume(handle, 10, 2);
		playClip(392, 450);
		startTalking("Doorbot", 0x3611A);
		sleep(TRANSLATE(8000, 7000));

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
