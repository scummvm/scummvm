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

#include "titanic/sound/titania_speech.h"
#include "titanic/translation.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CTitaniaSpeech, CGameObject)
	ON_MESSAGE(ActMsg)
	ON_MESSAGE(MovieEndMsg)
	ON_MESSAGE(MovieFrameMsg)
	ON_MESSAGE(TimerMsg)
	ON_MESSAGE(EnterRoomMsg)
END_MESSAGE_MAP()

void CTitaniaSpeech::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_actionNum, indent);
	file->writeNumberLine(_backgroundFrame, indent);

	CGameObject::save(file, indent);
}

void CTitaniaSpeech::load(SimpleFile *file) {
	file->readNumber();
	_actionNum = file->readNumber();
	_backgroundFrame = file->readNumber();

	CGameObject::load(file);
}

bool CTitaniaSpeech::ActMsg(CActMsg *msg) {
	CSetFrameMsg frameMsg;
	CVisibleMsg visibleMsg;
	CActMsg actMsg;

	if (msg->_action == "TitaniaSpeech") {
		CProximity prox(Audio::Mixer::kSpeechSoundType);
		switch (_actionNum) {
		case 1:
			loadSound(TRANSLATE("a#12.wav", "a#0.wav"));
			sleep(1000);
			playMovie(TRANSLATE(0, 584), TRANSLATE(187, 761),
				MOVIE_WAIT_FOR_FINISH | MOVIE_NOTIFY_OBJECT);
			if (g_language == Common::EN_ANY) {
				movieSetPlaying(true);
				movieEvent(0);
			} else {
				playSound("a#0.wav", prox);
			}
			break;

		case 2:
			loadSound(TRANSLATE("a#11.wav", "a#4.wav"));
			addTimer(0);
			startAnimTimer("Para2", 300);
			addTimer(6000);
			addTimer(12000);
			addTimer(18000);
			addTimer(24000);
			startAnimTimer("NextPara", TRANSLATE(30000, 33000));
			break;

		case 3:
			visibleMsg._visible = false;
			visibleMsg.execute("TitaniaStillControl");
			loadSound(TRANSLATE("a#10.wav", "a#2.wav"));
			playMovie(585, TRANSLATE(706, 748), MOVIE_WAIT_FOR_FINISH | MOVIE_NOTIFY_OBJECT);
			playSound(TRANSLATE("a#10.wav", "a#2.wav"), prox);
			break;

		case 4:
			visibleMsg._visible = false;
			visibleMsg.execute("TitaniaStillControl");
			loadSound(TRANSLATE("a#9.wav", "a#3.wav"));
			playMovie(707, 905, MOVIE_WAIT_FOR_FINISH | MOVIE_NOTIFY_OBJECT);
			playSound(TRANSLATE("a#9.wav", "a#3.wav"), prox);
			break;

		case 5:
			visibleMsg._visible = false;
			visibleMsg.execute("TitaniaStillControl");
			loadSound(TRANSLATE("a#8.wav", "a#1.wav"));
			playMovie(906, TRANSLATE(938, 943), MOVIE_WAIT_FOR_FINISH | MOVIE_NOTIFY_OBJECT);
			playSound(TRANSLATE("a#8.wav", "a#1.wav"), prox);
			break;

		default:
			sleep(3000);
			actMsg._action = "SleepTitania";
			actMsg.execute("TitaniaControl");
		}
	}

	return true;
}

bool CTitaniaSpeech::MovieEndMsg(CMovieEndMsg *msg) {
	if (_actionNum == 5) {
		startAnimTimer("NextPara", 0);
	} else {
		if (_actionNum != 1)
			addTimer(0);
		startAnimTimer("NextPara", 3000);
	}

	return true;
}

bool CTitaniaSpeech::MovieFrameMsg(CMovieFrameMsg *msg) {
	int frame = getMovieFrame();
	if (frame == 0) {
		CProximity prox(Audio::Mixer::kSpeechSoundType);
		playSound(TRANSLATE("a#12.wav", "a#0.wav"), prox);
	}

	return true;
}

bool CTitaniaSpeech::TimerMsg(CTimerMsg *msg) {
	CSetFrameMsg frameMsg;
	CVisibleMsg visibleMsg;
	CActMsg actMsg("TitaniaSpeech");

	if (msg->_action == "NextPara") {
		visibleMsg.execute("TitaniaStillControl");
		++_actionNum;
		actMsg.execute(this);
	} else if (msg->_action == "Para2") {
		CProximity prox(Audio::Mixer::kSpeechSoundType);
		playSound(TRANSLATE("a#11.wav", "a#4.wav"), prox);
	} else {
		frameMsg._frameNumber = _backgroundFrame++;
		frameMsg.execute("TitaniaStillControl");
	}

	return true;
}

bool CTitaniaSpeech::EnterRoomMsg(CEnterRoomMsg *msg) {
	CActMsg actMsg("Disable");
	actMsg.execute("ShipAnnouncements");

	return true;
}

} // End of namespace Titanic
