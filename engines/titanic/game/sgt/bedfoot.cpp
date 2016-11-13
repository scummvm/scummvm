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

#include "titanic/game/sgt/bedfoot.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CBedfoot, CSGTStateRoom)
	ON_MESSAGE(TurnOn)
	ON_MESSAGE(TurnOff)
END_MESSAGE_MAP()

void CBedfoot::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	CSGTStateRoom::save(file, indent);
}

void CBedfoot::load(SimpleFile *file) {
	file->readNumber();
	CSGTStateRoom::load(file);
}

bool CBedfoot::TurnOn(CTurnOn *msg) {
	if (_statics->_v2 == "Closed" && _statics->_v11 == "Closed") {
		_isClosed = false;
		_startFrame = 0;
		if (_statics->_v10 == "Open") {
			_endFrame = 13;
			_statics->_v2 = "Open";
			playSound("b#7.wav");
		} else {
			_endFrame = 17;
			_statics->_v2 = "NotOnWashstand";
			playSound("b#4.wav");
		}

		playMovie(_startFrame, _endFrame, MOVIE_GAMESTATE);
	} else if (_statics->_v2 == "RestingUnderTV") {
		_isClosed = false;
		_startFrame = 8;
		if (_statics->_v10 == "Open") {
			_statics->_v2 = "Open";
			playSound("189_436_bed down 1.wav");
		} else {
			_statics->_v2 = "NotOnWashstand";
			playSound("192_436_bed hits floor.wav");
		}

		playMovie(_startFrame, _endFrame, MOVIE_GAMESTATE);
	}

	if (_statics->_v2 == "Open")
		_statics->_v1 = "Closed";
	else if (_statics->_v2 == "NotOnWashstand")
		_statics->_v1 = "ClosedWrong";

	return true;
}

bool CBedfoot::TurnOff(CTurnOff *msg) {
	if (_statics->_v1 == "Closed" || _statics->_v1 == "ClosedWrong") {
		setVisible(true);
		CVisibleMsg visibleMsg(false);
		visibleMsg.execute("Bedhead");
	}

	if (_statics->_v2 == "Open" && _statics->_v1 == "Closed") {
		_isClosed = false;
		_startFrame = 20;
		if (_statics->_v4 == "Closed") {
			_statics->_v2 = "Closed";
			_endFrame = 30;
		} else {
			_statics->_v2 = "RestingUnderTV";
			_endFrame = 25;
		}

		playMovie(_startFrame, _endFrame, MOVIE_GAMESTATE);
		playSound("b#7.wav");

	} else if (_statics->_v2 == "NotOnWashstand" && _statics->_v1 == "ClosedWrong") {
		_isClosed = false;
		_startFrame = 17;

		if (_statics->_v4 == "Closed") {
			_statics->_v2 = "Closed";
			_endFrame = 30;
		} else {
			_statics->_v2 = "RestingUnderTV";
			_endFrame = 25;
		}

		playMovie(_startFrame, _endFrame, MOVIE_GAMESTATE);
		playSound("b#7.wav");

	} else if (_statics->_v2 == "RestingUTV" && _statics->_v4 == "Closed") {
		_statics->_v2 = "Closed";
		_startFrame = 25;
		_endFrame = 30;
		playMovie(25, 30, MOVIE_GAMESTATE);
		playSound("b#7.wav");
	}

	if (_statics->_v2 == "Closed")
		_statics->_v1 = "Closed";

	return true;
}

} // End of namespace Titanic
