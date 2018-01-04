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
#include "titanic/translation.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CBridgeView, CBackground)
	ON_MESSAGE(ActMsg)
	ON_MESSAGE(MovieEndMsg)
END_MESSAGE_MAP()

void CBridgeView::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_action, indent);
	CBackground::save(file, indent);
}

void CBridgeView::load(SimpleFile *file) {
	file->readNumber();
	_action = (BridgeAction)file->readNumber();
	CBackground::load(file);
}

bool CBridgeView::ActMsg(CActMsg *msg) {
	CTurnOn onMsg;
	CSetVolumeMsg volumeMsg;
	volumeMsg._secondsTransition = 1;

	if (msg->_action == "End") {
		_action = BA_ENDING2;
		petLockInput();
		petHide();
		setVisible(true);
		playMovie(MOVIE_NOTIFY_OBJECT);
	} else if (msg->_action == "Go") {
		_action = BA_GO;
		setVisible(true);
		hideMouse();
		volumeMsg._volume = 100;
		volumeMsg.execute("EngineSounds");
		onMsg.execute("EngineSounds");
		playMovie(MOVIE_NOTIFY_OBJECT);
	} else {
		volumeMsg._volume = 50;
		volumeMsg.execute("EngineSounds");
		onMsg.execute("EngineSounds");

		if (msg->_action == "Cruise") {
			_action = BA_CRUISE;
			setVisible(true);
			hideMouse();
			playMovie(MOVIE_NOTIFY_OBJECT);
		} else if (msg->_action == "GoEnd") {
			_action = BA_ENDING1;
			setVisible(true);
			hideMouse();

			CChangeMusicMsg musicMsg;
			musicMsg._action = MUSIC_STOP;
			musicMsg.execute("BridgeAutoMusicPlayer");
			playSound(TRANSLATE("a#42.wav", "a#35.wav"));
			playMovie(MOVIE_NOTIFY_OBJECT);
		}
	}

	return true;
}

bool CBridgeView::MovieEndMsg(CMovieEndMsg *msg) {
	CTurnOff offMsg;
	offMsg.execute("EngineSounds");

	switch (_action) {
	case BA_GO:
	case BA_CRUISE:
		setVisible(false);
		showMouse();
		decTransitions();
		break;

	case BA_ENDING1: {
		setVisible(false);
		CActMsg actMsg("End");
		actMsg.execute("HomeSequence");
		break;
	}

	case BA_ENDING2:
		setVisible(false);
		changeView("TheEnd.Node 3.N");
		break;

	default:
		break;
	}

	return true;
}

} // End of namespace Titanic
