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

#include "titanic/core/turn_on_turn_off.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CTurnOnTurnOff, CBackground)
	ON_MESSAGE(TurnOn)
	ON_MESSAGE(TurnOff)
END_MESSAGE_MAP()

CTurnOnTurnOff::CTurnOnTurnOff() : CBackground(), _startFrameOn(0),
	_endFrameOn(0), _startFrameOff(0), _endFrameOff(0), _isOn(false) {
}

void CTurnOnTurnOff::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_startFrameOn, indent);
	file->writeNumberLine(_endFrameOn, indent);
	file->writeNumberLine(_startFrameOff, indent);
	file->writeNumberLine(_endFrameOff, indent);
	file->writeNumberLine(_isOn, indent);

	CBackground::save(file, indent);
}

void CTurnOnTurnOff::load(SimpleFile *file) {
	file->readNumber();
	_startFrameOn = file->readNumber();
	_endFrameOn = file->readNumber();
	_startFrameOff = file->readNumber();
	_endFrameOff = file->readNumber();
	_isOn = file->readNumber();

	CBackground::load(file);
}

bool CTurnOnTurnOff::TurnOn(CTurnOn *msg) {
	if (!_isOn) {
		if (_isBlocking)
			playMovie(_startFrameOn, _endFrameOn, MOVIE_WAIT_FOR_FINISH);
		else
			playMovie(_startFrameOn, _endFrameOn, MOVIE_NOTIFY_OBJECT);
		_isOn = true;
	}

	return true;
}

bool CTurnOnTurnOff::TurnOff(CTurnOff *msg) {
	if (_isOn) {
		if (_isBlocking)
			playMovie(_startFrameOff, _endFrameOff, MOVIE_WAIT_FOR_FINISH);
		else
			playMovie(_startFrameOff, _endFrameOff, MOVIE_NOTIFY_OBJECT);
		_isOn = false;
	}

	return true;
}

} // End of namespace Titanic
