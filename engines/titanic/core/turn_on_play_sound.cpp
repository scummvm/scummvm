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

#include "titanic/core/turn_on_play_sound.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CTurnOnPlaySound, CTurnOnObject)
	ON_MESSAGE(MouseButtonUpMsg)
END_MESSAGE_MAP()

CTurnOnPlaySound::CTurnOnPlaySound() : CTurnOnObject(),
		_soundName("NULL"), _soundVolume(80), _soundVal3(0) {
}

void CTurnOnPlaySound::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeQuotedLine(_soundName, indent);
	file->writeNumberLine(_soundVolume, indent);
	file->writeNumberLine(_soundVal3, indent);

	CTurnOnObject::save(file, indent);
}

void CTurnOnPlaySound::load(SimpleFile *file) {
	file->readNumber();
	_soundName = file->readString();
	_soundVolume = file->readNumber();
	_soundVal3 = file->readNumber();

	CTurnOnObject::load(file);
}

bool CTurnOnPlaySound::MouseButtonUpMsg(CMouseButtonUpMsg *msg) {
	if (_soundName != "NULL")
		playSound(_soundName, _soundVolume, _soundVal3);

	return CTurnOnObject::MouseButtonUpMsg(msg);
}

} // End of namespace Titanic
