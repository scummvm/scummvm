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

#include "titanic/game/play_music_button.h"
#include "titanic/sound/music_room.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CPlayMusicButton, CBackground)
	ON_MESSAGE(MouseButtonDownMsg)
	ON_MESSAGE(FrameMsg)
END_MESSAGE_MAP()

void CPlayMusicButton::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_flag, indent);
	file->writeNumberLine(_ticks, indent);

	CBackground::save(file, indent);
}

void CPlayMusicButton::load(SimpleFile *file) {
	file->readNumber();
	_flag = file->readNumber();
	_ticks = file->readNumber();

	CBackground::load(file);
}

bool CPlayMusicButton::MouseButtonDownMsg(CMouseButtonDownMsg *msg) {
	CMusicRoom *musicRoom = getMusicRoom();
	if (_flag) {
		musicRoom->stopMusic();
		stopMovie();
		loadFrame(0);
		_flag = false;
	} else {
		musicRoom->setupMusic(100);
		playMovie(MOVIE_REPEAT);
		_ticks = getTicksCount();
		_flag = true;
	}

	return true;
}

bool CPlayMusicButton::FrameMsg(CFrameMsg *msg) {
	if (_flag && !CMusicRoom::_musicHandler->update()) {
		CMusicRoom *musicRoom = getMusicRoom();
		musicRoom->stopMusic();
		stopMovie();
		loadFrame(0);
		_flag = false;
	}

	return true;
}

} // End of namespace Titanic
