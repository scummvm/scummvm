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

#include "titanic/gfx/music_control.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CMusicControl, CBackground)
	ON_MESSAGE(MouseButtonDownMsg)
	ON_MESSAGE(MouseDoubleClickMsg)
END_MESSAGE_MAP()

CMusicControl::CMusicControl() : CBackground(),
	_controlArea(BELLS), _controlVal(0), _controlMax(1), _enabled(true) {
}

void CMusicControl::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_controlArea, indent);
	file->writeNumberLine(_controlVal, indent);
	file->writeNumberLine(_controlMax, indent);
	file->writeNumberLine(_enabled, indent);

	CBackground::save(file, indent);
}

void CMusicControl::load(SimpleFile *file) {
	file->readNumber();
	_controlArea = (MusicInstrument)file->readNumber();
	_controlVal = file->readNumber();
	_controlMax = file->readNumber();
	_enabled = file->readNumber();

	CBackground::load(file);
}

bool CMusicControl::MouseButtonDownMsg(CMouseButtonDownMsg *msg) {
	CMusicSettingChangedMsg changedMsg;
	changedMsg.execute(this);
	return true;
}

bool CMusicControl::MouseDoubleClickMsg(CMouseDoubleClickMsg *msg) {
	CMusicSettingChangedMsg changedMsg;
	changedMsg.execute(this);
	return true;
}

} // End of namespace Titanic
