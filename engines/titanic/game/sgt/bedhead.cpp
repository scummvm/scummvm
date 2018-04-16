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
#include "titanic/support/files_manager.h"
#include "titanic/titanic.h"
#include "titanic/translation.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CBedhead, CSGTStateRoom)
	ON_MESSAGE(TurnOn)
	ON_MESSAGE(TurnOff)
END_MESSAGE_MAP()

void BedheadEntry::load(Common::SeekableReadStream *s) {
	_name1 = readStringFromStream(s);
	_name2 = readStringFromStream(s);
	_name3 = readStringFromStream(s);
	_name4 = readStringFromStream(s);
	_startFrame = s->readUint32LE();
	_endFrame = s->readUint32LE();
}

/*------------------------------------------------------------------------*/

void BedheadEntries::load(Common::SeekableReadStream *s, int count) {
	resize(count);
	for (int idx = 0; idx < count; ++idx)
		(*this)[idx].load(s);
}

/*------------------------------------------------------------------------*/

void TurnOnEntries::load(Common::SeekableReadStream *s) {
	_closed.load(s, 4);
	_restingTV.load(s, 2);
	_restingUV.load(s, 2);
	_closedWrong.load(s, 2);
}

/*------------------------------------------------------------------------*/

void TurnOffEntries::load(Common::SeekableReadStream *s) {
	_open.load(s, 3);
	_restingUTV.load(s, 1);
	_restingV.load(s, 1);
	_restingG.load(s, 3);
	_openWrong.load(s, 1);
	_restingDWrong.load(s, 1);
}

/*------------------------------------------------------------------------*/

CBedhead::CBedhead() : CSGTStateRoom() {
	Common::SeekableReadStream *s = g_vm->_filesManager->getResource("DATA/BEDHEAD");
	_on.load(s);
	_off.load(s);
	delete s;
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
	if (_statics->_bedfoot != "Closed" && _statics->_bedfoot != "RestingUnderTV") {
		const BedheadEntries *data = nullptr;
		if (_statics->_bedhead == "Closed")
			data = &_on._closed;
		else if (_statics->_bedhead == "RestingTV")
			data = &_on._restingTV;
		else if (_statics->_bedhead == "RestingUV")
			data = &_on._restingUV;
		else if (_statics->_bedhead == "ClosedWrong")
			data = &_on._closedWrong;
		else
			return true;

		for (uint idx = 0; idx < data->size(); ++idx) {
			const BedheadEntry &entry = (*data)[idx];
			if ((entry._name1 == _statics->_tv || entry._name1 == "Any")
					&& (entry._name2 == _statics->_vase || entry._name2 == "Any")
					&& (entry._name3 == _statics->_desk || entry._name3 == "Any")) {
				CVisibleMsg visibleMsg(false);
				visibleMsg.execute("Bedfoot");
				setVisible(true);

				_statics->_bedhead = entry._name4;
				playMovie(entry._startFrame, entry._endFrame, MOVIE_NOTIFY_OBJECT | MOVIE_WAIT_FOR_FINISH);
				playSound(TRANSLATE("b#6.wav", "b#91.wav"));
				_isClosed = false;
			}
		}
	}

	if (_statics->_bedhead == "Open") {
		playMovie(71, 78, 0);
		playSound("196_436 bed inflate 2.wav");
	}

	return true;
}

bool CBedhead::TurnOff(CTurnOff *msg) {
	if (_statics->_bedhead == "Open") {
		playMovie(78, 85, 0);
		playSound("191_436_bed inflate deflate.wav");
	}

	BedheadEntries *data = nullptr;
	if (_statics->_bedhead == "Open")
		data = &_off._open;
	else if (_statics->_bedhead == "RestingUTV")
		data = &_off._restingUTV;
	else if (_statics->_bedhead == "RestingV")
		data = &_off._restingV;
	else if (_statics->_bedhead == "RestingG")
		data = &_off._restingG;
	else if (_statics->_bedhead == "OpenWrong")
		data = &_off._openWrong;
	else if (_statics->_bedhead == "RestingDWrong")
		data = &_off._restingDWrong;
	else
		return true;

	for (uint idx = 0; idx < data->size(); ++idx) {
		const BedheadEntry &entry = (*data)[idx];
		if ((entry._name1 == _statics->_tv || entry._name1 == "Any")
			&& (entry._name2 == _statics->_vase || entry._name2 == "Any")
			&& (entry._name3 == _statics->_desk || entry._name3 == "Any")) {
			CVisibleMsg visibleMsg(false);
			visibleMsg.execute("Bedfoot");
			setVisible(true);

			_statics->_bedhead = entry._name4;
			playMovie(entry._startFrame, entry._endFrame, MOVIE_NOTIFY_OBJECT | MOVIE_WAIT_FOR_FINISH);
			playSound("193_436_bed fold up 1.wav");
			_isClosed = true;
		}
	}

	return true;
}

} // End of namespace Titanic
