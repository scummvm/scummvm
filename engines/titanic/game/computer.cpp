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

#include "titanic/game/computer.h"
#include "titanic/translation.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CComputer, CBackground)
	ON_MESSAGE(ActMsg)
	ON_MESSAGE(MouseButtonDownMsg)
	ON_MESSAGE(MovieEndMsg)
END_MESSAGE_MAP()

void CComputer::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeQuotedLine(_currentCD, indent);
	file->writeNumberLine(_state, indent);
	CBackground::save(file, indent);
}

void CComputer::load(SimpleFile *file) {
	file->readNumber();
	_currentCD = file->readString();
	_state = file->readNumber();
	CBackground::load(file);
}

bool CComputer::ActMsg(CActMsg *msg) {
	if (_state) {
		playSound(TRANSLATE("a#35.wav", "a#30.wav"));
		playMovie(32, 42, 0);

		if (msg->_action == "CD1")
			playMovie(43, 49, 0);
		else if (msg->_action == "CD2")
			playMovie(50, 79, 0);
		else if (msg->_action == "STCD")
			playMovie(80, 90, MOVIE_NOTIFY_OBJECT);

		_currentCD = msg->_action;
		_state = 0;
	}

	return true;
}

bool CComputer::MouseButtonDownMsg(CMouseButtonDownMsg *msg) {
	if (_currentCD == "None") {
		if (_state) {
			playSound(TRANSLATE("a#35.wav", "a#30.wav"));
			playMovie(11, 21, 0);
			_state = 0;
		} else {
			playSound(TRANSLATE("a#34.wav", "a#29.wav"));
			playMovie(0, 10, 0);
			_state = 1;
		}
	} else {
		if (_state) {
			loadFrame(11);
			CActMsg actMsg("EjectCD");
			actMsg.execute(_currentCD);
			_currentCD = "None";
		} else {
			playSound(TRANSLATE("a#34.wav", "a#29.wav"));
			playMovie(21, 31, 0);
			_state = 1;
		}
	}

	return true;
}

bool CComputer::MovieEndMsg(CMovieEndMsg *msg) {
	if (msg->_endFrame == 90) {
		playSound(TRANSLATE("a#32.wav", "a#27.wav"));
		playSound(TRANSLATE("a#33.wav", "a#28.wav"));
		playSound(TRANSLATE("a#31.wav", "a#26.wav"));
		playSound(TRANSLATE("a#0.wav", "a#52.wav"));

		gotoView("Home.Node 4.E", "_TRACK,3,e-cu,4,E");
	}

	return true;
}

} // End of namespace Titanic
