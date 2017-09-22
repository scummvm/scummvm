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

#include "titanic/game/speech_dispensor.h"
#include "titanic/translation.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CSpeechDispensor, CBackground)
	ON_MESSAGE(FrameMsg)
	ON_MESSAGE(MouseButtonUpMsg)
	ON_MESSAGE(StatusChangeMsg)
	ON_MESSAGE(ChangeSeasonMsg)
END_MESSAGE_MAP()

CSpeechDispensor::CSpeechDispensor() : CBackground(), _dragItem(nullptr),
		_hitCounter(0), _state(0), _speechFallen(false), _failureType(false),
		_seasonNum(SEASON_SUMMER) {
}

void CSpeechDispensor::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_hitCounter, indent);
	file->writeNumberLine(_state, indent);
	file->writeNumberLine(_speechFallen, indent);
	file->writeNumberLine(_itemPos.x, indent);
	file->writeNumberLine(_itemPos.y, indent);
	file->writeNumberLine(_failureType, indent);
	file->writeNumberLine(_seasonNum, indent);

	CBackground::save(file, indent);
}

void CSpeechDispensor::load(SimpleFile *file) {
	file->readNumber();
	_hitCounter = file->readNumber();
	_state = file->readNumber();
	_speechFallen = file->readNumber();
	_itemPos.x = file->readNumber();
	_itemPos.y = file->readNumber();
	_failureType = file->readNumber();
	_seasonNum = (Season)file->readNumber();

	CBackground::load(file);
}

bool CSpeechDispensor::FrameMsg(CFrameMsg *msg) {
	if (_speechFallen || _seasonNum == SEASON_SUMMER || _seasonNum == SEASON_SPRING)
		return true;

	CGameObject *dragObject = getDraggingObject();
	if (!_dragItem && dragObject && getView() == findView()) {
		if (dragObject->isEquals("Perch")) {
			petDisplayMessage(1, TOO_SHORT_TO_REACH_BRANCHES);
			return true;
		}

		if (dragObject->isEquals("LongStick"))
			_dragItem = dragObject;
	}

	if (_dragItem) {
		Point pt(_itemPos.x + _dragItem->_bounds.left,
			_itemPos.y + _dragItem->_bounds.top);
		if (!checkPoint(pt, true))
			return true;

		switch (_state) {
		case 0:
			playSound(TRANSLATE("z#93.wav", "z#624.wav"));
			if (_seasonNum == SEASON_WINTER) {
				petDisplayMessage(1, FROZEN_TO_BRANCH);
				_hitCounter = 0;
				_state = 1;
			} else {
				if (++_hitCounter >= 5) {
					CActMsg actMsg("PlayerGetsSpeechCentre");
					actMsg.execute("SeasonalAdjust");
					CSpeechFallsFromTreeMsg fallMsg(pt);
					fallMsg.execute("SpeechCentre");

					_speechFallen = true;
					_hitCounter = 0;
				}

				_state = 1;
			}
			break;

		case 1:
			_state = 0;
			++_hitCounter;
			break;

		default:
			break;
		}
	}

	return true;
}

bool CSpeechDispensor::MouseButtonUpMsg(CMouseButtonUpMsg *msg) {
	if (!_speechFallen) {
		playSound(TRANSLATE("z#93.wav", "z#624.wav"));
		if (_failureType) {
			petDisplayMessage(1, OUT_OF_REACH);
		} else {
			petDisplayMessage(1, STUCK_TO_BRANCH);
		}
	}

	return true;
}

bool CSpeechDispensor::StatusChangeMsg(CStatusChangeMsg *msg) {
	_failureType = msg->_newStatus == 1;
	return true;
}

bool CSpeechDispensor::ChangeSeasonMsg(CChangeSeasonMsg *msg) {
	_seasonNum = (Season)(((int)_seasonNum + 1) % 4);
	return true;
}

} // End of namespace Titanic
