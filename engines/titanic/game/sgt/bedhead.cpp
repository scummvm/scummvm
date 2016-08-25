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

#include "titanic/game/sgt/bedhead.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CBedhead, CSGTStateRoom)
	ON_MESSAGE(TurnOn)
	ON_MESSAGE(TurnOff)
END_MESSAGE_MAP()

void BedheadEntry::load(Common::SeekableReadStream *s) {
	// TODO
}

/*------------------------------------------------------------------------*/

void BedheadEntries::load(Common::SeekableReadStream *s) {
	resize(s->readUint32LE());
	for (uint idx = 0; idx < size(); ++idx)
		(*this)[idx].load(s);
}

/*------------------------------------------------------------------------*/

void TurnOnEntries::load(Common::SeekableReadStream *s) {
	_closed.load(s);
	_restingTV.load(s);
	_restingUV.load(s);
	_closedWrong.load(s);
}

/*------------------------------------------------------------------------*/

void TurnOffEntries::load(Common::SeekableReadStream *s) {
	_open.load(s);
	_restingUTV.load(s);
	_restingV.load(s);
	_restingG.load(s);
	_openWrong.load(s);
	_restingDWrong.load(s);
}

/*------------------------------------------------------------------------*/

CBedhead::CBedhead() : CSGTStateRoom() {
	// TODO: Load data for turn on/off methods
}

void CBedhead::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	CSGTStateRoom::save(file, indent);
}

void CBedhead::load(SimpleFile *file) {
	file->readNumber();
	CSGTStateRoom::load(file);
}

bool CBedhead::TurnOn(CTurnOn *msg) {
	if (_statics->_v2 == "Closed" || _statics->_v2 == "RestingUnderTV")
		return true;

	const BedheadEntries *data = nullptr;
	if (_statics->_v1 == "Closed")
		data = &_on._closed;
	else if (_statics->_v1 == "RestingTV")
		data = &_on._restingTV;
	else if (_statics->_v1 == "RestingUV")
		data = &_on._restingUV;
	else if (_statics->_v1 == "ClosedWrong")
		data = &_on._closedWrong;
	else
		return true;

	for (uint idx = 0; idx < data->size(); ++idx) {
		const BedheadEntry &entry = (*data)[idx];
		if ((entry._name1 == _statics->_v4 || entry._name1 == "Any")
				&& (entry._name2 == _statics->_v3 || entry._name2 == "Any")
				&& (entry._name3 == _statics->_v5 || entry._name3 == "Any")) {
			CVisibleMsg visibleMsg(false);
			visibleMsg.execute("Bedfoot");
			setVisible(true);

			_statics->_v1 = entry._name4;
			playMovie(entry._startFrame, entry._endFrame, MOVIE_NOTIFY_OBJECT | MOVIE_GAMESTATE);
			playSound("b#6.wav");
			_fieldE0 = false;
		}
	}

	if (_statics->_v1 == "Open") {
		playMovie(71, 78, 0);
		playSound("196_436 bed inflate 2.wav");
	}

	return true;
}

bool CBedhead::TurnOff(CTurnOff *msg) {
	if (_statics->_v1 == "Open") {
		playMovie(78, 85, 0);
		playSound("191_436_bed inflate deflate.wav");
	}

	BedheadEntries *data = nullptr;
	if (_statics->_v1 == "Open")
		data = &_off._open;
	else if (_statics->_v1 == "RestingUTV")
		data = &_off._restingUTV;
	else if (_statics->_v1 == "RestingV")
		data = &_off._restingV;
	else if (_statics->_v1 == "RestingG")
		data = &_off._restingG;
	else if (_statics->_v1 == "OpenWrong")
		data = &_off._openWrong;
	else if (_statics->_v1 == "RestingDWrong")
		data = &_off._restingDWrong;
	else
		return true;

	for (uint idx = 0; idx < data->size(); ++idx) {
		const BedheadEntry &entry = (*data)[idx];
		if ((entry._name1 == _statics->_v4 || entry._name1 == "Any")
			&& (entry._name2 == _statics->_v3 || entry._name2 == "Any")
			&& (entry._name3 == _statics->_v5 || entry._name3 == "Any")) {
			CVisibleMsg visibleMsg(false);
			visibleMsg.execute("Bedfoot");
			setVisible(true);

			_statics->_v1 = entry._name4;
			playMovie(entry._startFrame, entry._endFrame, MOVIE_NOTIFY_OBJECT | MOVIE_GAMESTATE);
			playSound("193_436_bed fold up 1.wav");
			_fieldE0 = false;
		}
	}

	return true;
}

} // End of namespace Titanic
