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

#include "common/textconsole.h"
#include "titanic/pet_control/pet_section.h"
#include "titanic/pet_control/pet_control.h"

namespace Titanic {

static const uint PALETTE1[6] = {
	0xA7C0DB, 0x9CFFFE, 0x73AEFF, 0xA7C0DB, 0x9CFFFE, 0
};

static const uint PALETTE2[6] = {
	0x10101, 0x1013C, 0xC80101, 0x10101, 0x800101, 0
};

static const uint PALETTE3[5] = {
	0x10101, 0x1013C, 0xC80101, 0x10101, 0x800101
};

void CPetSection::displayMessage(const CString &msg) {
	CTextControl *text = getText();

	if (text) {
		text->setColor(getColor(1));
		text->setText(msg);
		_petControl->makeDirty();
		removeText(5000);
	}
}

void CPetSection::timerExpired(int val) {
	if (!val) {
		removeText();
		_petControl->makeDirty();
	}
}

void CPetSection::removeText(int duration) {
	if (duration > 0)
		_petControl->startPetTimer(0, duration, 0, this);
	else
		removeText();
}

void CPetSection::removeText() {
	CTextControl *text = getText();
	if (text)
		text->setup();
}

void CPetSection::stopTextTimer() {
	_petControl->stopPetTimer(0);
}

uint CPetSection::getColor(uint index) {
	return getColorTable()[index];
}

const uint *CPetSection::getColorTable(int tableNum) {
	if (tableNum == -1) {
		CPetControl *pet = getPetControl();
		tableNum = pet ? pet->getPassengerClass() : 3;
	}

	switch (tableNum) {
	case 1: return PALETTE1;
	case 2: return PALETTE2;
	default: return PALETTE3;
	}
}

void CPetSection::areaChanged(PetArea area) {
	if (_petControl && _petControl->_currentArea == area)
		_petControl->makeDirty();
}

CString CPetSection::getActiveNPCName() const {
	if (_petControl && _petControl->_activeNPC)
		return _petControl->_activeNPC->getName();
	else
		return CString();
}

void CPetSection::copyColors(uint tableNum, uint colors[5]) {
	const uint *src = getColorTable(tableNum);
	Common::copy(src, src + 5, colors);
}

} // End of namespace Titanic
