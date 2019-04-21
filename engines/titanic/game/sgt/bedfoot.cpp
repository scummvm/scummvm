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
#include "titanic/translation.h"

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
	if (_statics->_bedfoot == "Closed" && _statics->_basin == "Closed") {
		_isClosed = false;
		_startFrame = 0;
		if (_statics->_washstand == "Open") {
			_endFrame = 13;
			_statics->_bedfoot = "Open";
			playSound(TRANSLATE("b#7.wav", "b#92.wav"));
		} else {
			_endFrame = 17;
			_statics->_bedfoot = "NotOnWashstand";
			playSound(TRANSLATE("b#4.wav", "b#89.wav"));
		}

		playMovie(_startFrame, _endFrame, MOVIE_WAIT_FOR_FINISH);
	} else if (_statics->_bedfoot == "RestingUnderTV") {
		_isClosed = false;
		_startFrame = 8;
		if (_statics->_washstand == "Open") {
			_statics->_bedfoot = "Open";
			playSound("189_436_bed down 1.wav");
		} else {
			_statics->_bedfoot = "NotOnWashstand";
			playSound("192_436_bed hits floor.wav");
		}

		playMovie(_startFrame, _endFrame, MOVIE_WAIT_FOR_FINISH);
	}

	if (_statics->_bedfoot == "Open")
		_statics->_bedhead = "Closed";
	else if (_statics->_bedfoot == "NotOnWashstand")
		_statics->_bedhead = "ClosedWrong";

	return true;
}

bool CBedfoot::TurnOff(CTurnOff *msg) {
	if (_statics->_bedhead == "Closed" || _statics->_bedhead == "ClosedWrong") {
		setVisible(true);
		CVisibleMsg visibleMsg(false);
		visibleMsg.execute("Bedhead");
	}

	if (_statics->_bedfoot == "Open" && _statics->_bedhead == "Closed") {
		_isClosed = true;
		_startFrame = 20;
		if (_statics->_tv == "Closed") {
			_statics->_bedfoot = "Closed";
			_endFrame = 30;
		} else {
			_statics->_bedfoot = "RestingUnderTV";
			_endFrame = 25;
		}

		playMovie(_startFrame, _endFrame, MOVIE_WAIT_FOR_FINISH);
		playSound(TRANSLATE("b#7.wav", "b#92.wav"));

	} else if (_statics->_bedfoot == "NotOnWashstand" && _statics->_bedhead == "ClosedWrong") {
		_isClosed = true;
		_startFrame = 17;

		if (_statics->_tv == "Closed") {
			_statics->_bedfoot = "Closed";
			_endFrame = 30;
		} else {
			_statics->_bedfoot = "RestingUnderTV";
			_endFrame = 25;
		}

		playMovie(_startFrame, _endFrame, MOVIE_WAIT_FOR_FINISH);
		playSound(TRANSLATE("b#7.wav", "b#92.wav"));

	} else if (_statics->_bedfoot == "RestingUTV" && _statics->_tv == "Closed") {
		_statics->_bedfoot = "Closed";
		_startFrame = 25;
		_endFrame = 30;
		playMovie(25, 30, MOVIE_WAIT_FOR_FINISH);
		playSound(TRANSLATE("b#7.wav", "b#92.wav"));
	}

	if (_statics->_bedfoot == "Closed")
		_statics->_bedhead = "Closed";

	return true;
}

} // End of namespace Titanic
