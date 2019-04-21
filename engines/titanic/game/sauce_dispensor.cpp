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

#include "titanic/game/sauce_dispensor.h"
#include "titanic/carry/chicken.h"
#include "titanic/carry/glass.h"
#include "titanic/translation.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CSauceDispensor, CBackground)
	ON_MESSAGE(Use)
	ON_MESSAGE(MovieEndMsg)
	ON_MESSAGE(ActMsg)
	ON_MESSAGE(LeaveViewMsg)
	ON_MESSAGE(MouseButtonDownMsg)
	ON_MESSAGE(StatusChangeMsg)
END_MESSAGE_MAP()

CSauceDispensor::CSauceDispensor() : CBackground(),
		_pouringCondiment(false), _starlingsDead(false),
		_field104(0), _field108(0) {
}

void CSauceDispensor::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeQuotedLine(_condimentName, indent);
	file->writeNumberLine(_pouringCondiment, indent);
	file->writeNumberLine(_starlingsDead, indent);
	file->writePoint(_pos1, indent);
	file->writePoint(_pos2, indent);
	file->writeNumberLine(_field104, indent);
	file->writeNumberLine(_field108, indent);

	CBackground::save(file, indent);
}

void CSauceDispensor::load(SimpleFile *file) {
	file->readNumber();
	_condimentName = file->readString();
	_pouringCondiment = file->readNumber();
	_starlingsDead = file->readNumber();
	_pos1 = file->readPoint();
	_pos2 = file->readPoint();
	_field104 = file->readNumber();
	_field108 = file->readNumber();

	CBackground::load(file);
}

bool CSauceDispensor::Use(CUse *msg) {
	CVisibleMsg visibleMsg(true);

	if (msg->_item->isEquals("Chicken")) {
		CChicken *chicken = static_cast<CChicken *>(msg->_item);
		_field104 = true;
		if (_starlingsDead) {
			playSound(TRANSLATE("b#15.wav", "z#562.wav"), 50);

			if (chicken->_condiment != "None") {
				petDisplayMessage(1, FOODSTUFF_ALREADY_GARNISHED);
				msg->execute("Chicken");
			} else {
				setVisible(true);
				if (chicken->_greasy) {
					_pouringCondiment = true;
					playMovie(_pos1.x, _pos1.y, MOVIE_NOTIFY_OBJECT);
				} else {
					CActMsg actMsg(_condimentName);
					actMsg.execute("Chicken");
					playMovie(_pos2.x, _pos2.y, MOVIE_NOTIFY_OBJECT);
				}
			}

			if (_starlingsDead)
				return true;
		}

		CMovieEndMsg endMsg(0, 0);
		endMsg.execute(this);
		playSound(TRANSLATE("z#120.wav", "z#651.wav"));

		petDisplayMessage(1, DISPENSOR_IS_EMPTY);
	} else if (msg->_item->isEquals("BeerGlass")) {
		CGlass *glass = dynamic_cast<CGlass *>(msg->_item);
		assert(glass);
		_field108 = true;

		if (_field104 != 1 || !_starlingsDead) {
			glass->petAddToInventory();
		} else if (glass->_condiment != "None") {
			visibleMsg.execute("BeerGlass");
		} else if (_pouringCondiment) {
			glass->setPosition(Point(
				_bounds.left + (_bounds.width() / 2) - (glass->_bounds.width() / 2),
				300));
			setVisible(true);

			CActMsg actMsg(_condimentName);
			actMsg.execute("BeerGlass");
		}
	}

	return true;
}

bool CSauceDispensor::MovieEndMsg(CMovieEndMsg *msg) {
	setVisible(false);
	_pouringCondiment = false;

	CActMsg actMsg("GoToPET");
	if (_field104)
		actMsg.execute("Chicken");
	if (_field108)
		actMsg.execute("BeerGlass");

	_field104 = false;
	_field108 = false;
	return true;
}

bool CSauceDispensor::ActMsg(CActMsg *msg) {
	if (msg->_action == "StarlingsDead")
		_starlingsDead = true;

	return true;
}

bool CSauceDispensor::LeaveViewMsg(CLeaveViewMsg *msg) {
	setVisible(false);
	loadFrame(0);

	if (_field108) {
		CGameObject *glass = findRoomObject("Beerglass");
		if (glass)
			glass->petAddToInventory();
	}

	_field104 = false;
	_field108 = false;
	return true;
}

bool CSauceDispensor::MouseButtonDownMsg(CMouseButtonDownMsg *msg) {
	petDisplayMessage(1, PUT_FOOD_UNDER_DISPENSOR);
	return true;
}

bool CSauceDispensor::StatusChangeMsg(CStatusChangeMsg *msg) {
	petDisplayMessage(1, PUT_FOOD_UNDER_DISPENSOR);
	return true;
}

} // End of namespace Titanic
