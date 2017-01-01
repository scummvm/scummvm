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

#include "titanic/game/bridge_view.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CBridgeView, CBackground)
	ON_MESSAGE(ActMsg)
	ON_MESSAGE(MovieEndMsg)
END_MESSAGE_MAP()

void CBridgeView::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_mode, indent);
	CBackground::save(file, indent);
}

void CBridgeView::load(SimpleFile *file) {
	file->readNumber();
	_mode = file->readNumber();
	CBackground::load(file);
}

bool CBridgeView::ActMsg(CActMsg *msg) {
	CTurnOn onMsg;
	CSetVolumeMsg volumeMsg;
	volumeMsg._secondsTransition = 1;

	if (msg->_action == "End") {
		_mode = 4;
		petLockInput();
		petHide();
		setVisible(true);
		playMovie(MOVIE_NOTIFY_OBJECT);
	} else if (msg->_action == "Go") {
		_mode = 1;
		setVisible(true);
		volumeMsg._volume = 100;
		volumeMsg.execute("EngineSounds");
		onMsg.execute("EngineSounds");
		playMovie(MOVIE_NOTIFY_OBJECT);
	} else {
		volumeMsg._volume = 50;
		volumeMsg.execute("EngineSounds");
		onMsg.execute("EngineSounds");

		if (msg->_action == "Cruise") {
			_mode = 2;
			setVisible(true);
			playMovie(MOVIE_NOTIFY_OBJECT);
		} else if (msg->_action == "GoENd") {
			_mode = 3;
			setVisible(true);
			CChangeMusicMsg musicMsg;
			musicMsg._flags = 1;
			musicMsg.execute("BridgeAutoMusicPlayer");
			playSound("a#42.wav");
			playMovie(MOVIE_NOTIFY_OBJECT);
		}
	}

	return true;
}

bool CBridgeView::MovieEndMsg(CMovieEndMsg *msg) {
	CTurnOff offMsg;
	offMsg.execute("EngineSounds");

	switch (_mode) {
	case 0:
	case 1:
		setVisible(false);
		decTransitions();
		break;

	case 2: {
		setVisible(false);
		CActMsg actMsg("End");
		actMsg.execute("HomeSequence");
		break;
	}

	case 3:
		setVisible(false);
		changeView("TheEnd.Node 3.N");
		break;

	default:
		break;
	}

	return true;
}

} // End of namespace Titanic
