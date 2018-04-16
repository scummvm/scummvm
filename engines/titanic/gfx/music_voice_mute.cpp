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

#include "titanic/gfx/music_voice_mute.h"
#include "titanic/sound/music_room.h"
#include "titanic/translation.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CMusicVoiceMute, CMusicControl)
	ON_MESSAGE(MusicSettingChangedMsg)
	ON_MESSAGE(EnterViewMsg)
	ON_MESSAGE(QueryMusicControlSettingMsg)
END_MESSAGE_MAP()

bool CMusicVoiceMute::MusicSettingChangedMsg(CMusicSettingChangedMsg *msg) {
	if (++_controlVal > _controlMax)
		_controlVal = 0;

	CMusicRoom *musicRoom = getMusicRoom();
	musicRoom->setMuteControl(_controlArea, _controlVal == 1 ? 1 : 0);
	loadFrame(1 - _controlVal);
	playSound(TRANSLATE("z#55.wav", "z#586.wav"), 50);

	return true;
}

bool CMusicVoiceMute::EnterViewMsg(CEnterViewMsg *msg) {
	loadFrame(1 - _controlVal);
	CMusicRoom *musicRoom = getMusicRoom();
	musicRoom->setMuteControl(_controlArea, _controlVal == 1 ? 1 : 0);

	return true;
}

bool CMusicVoiceMute::QueryMusicControlSettingMsg(CQueryMusicControlSettingMsg *msg) {
	msg->_value = _controlVal;
	return true;
}

} // End of namespace Titanic
