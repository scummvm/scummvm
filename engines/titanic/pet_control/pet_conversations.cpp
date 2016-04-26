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

#include "titanic/pet_control/pet_conversations.h"

namespace Titanic {

CPetConversations::CPetConversations() : CPetSection(),
		_field414(0), _field418(0) {
}

void CPetConversations::save(SimpleFile *file, int indent) const {

}

void CPetConversations::load(SimpleFile *file, int param) {
	_text2.load(file, param);
	_log.load(file, param);

	for (int idx = 0; idx < 3; ++idx)
		_valArray3[idx] = file->readNumber();
}

bool CPetConversations::isValid(CPetControl *petControl) {
	return setupControl(petControl);
}

bool CPetConversations::setupControl(CPetControl *petControl) {
	if (petControl) {
		_petControl = petControl;

		_val3.setBounds(Rect(0, 0, 21, 130));
		_val3.translate(20, 350);
		
		const Rect rect1(0, 0, 22, 36);
		_gfxList[0].setBounds(rect1);
		_gfxList[0].translate(20, 359);
		_gfxList[1].setBounds(rect1);
		_gfxList[1].translate(20, 397);
		_gfxList[2].setBounds(rect1);
		_gfxList[2].translate(20, 434);

		const Rect rect2(0, 0, 11, 24);
		_val1.setBounds(rect2);
		_val1.translate(87, 374);
		_val2.setBounds(rect2);
		_val2.translate(87, 421);

		const Rect rect3(0, 0, 39, 39);
		_val7.setBounds(rect3);
		_val7.translate(546, 372);
		_val8.setBounds(rect3);
		_val8.translate(546, 418);

		_val6.setBounds(Rect(0, 0, 37, 70));
		_val6.translate(46, 374);
		_val9.setBounds(Rect(0, 0, 435, 3));
		_val9.translate(102, 441);

		const Rect rect4(0, 0, 33, 66);
		for (int idx = 0; idx < 9; ++idx) {
			_valArray2[idx].setBounds(rect4);
			_valArray2[idx].translate(48, 376);
		}
	}

	return true;
}

} // End of namespace Titanic
