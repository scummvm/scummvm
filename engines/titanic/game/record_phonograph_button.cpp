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

#include "titanic/game/record_phonograph_button.h"
#include "titanic/translation.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CRecordPhonographButton, CBackground)
	ON_MESSAGE(MouseButtonDownMsg)
	ON_MESSAGE(PhonographStopMsg)
END_MESSAGE_MAP()

void CRecordPhonographButton::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_active, indent);
	CBackground::save(file, indent);
}

void CRecordPhonographButton::load(SimpleFile *file) {
	file->readNumber();
	_active = file->readNumber();
	CBackground::load(file);
}

bool CRecordPhonographButton::MouseButtonDownMsg(CMouseButtonDownMsg *msg) {
	CPhonographRecordMsg recordMsg;
	recordMsg.execute(getParent());

	if (recordMsg._canRecord) {
		playSound(TRANSLATE("z#58.wav", "z#589.wav"));
		loadFrame(1);
		_active = true;
	}

	return true;
}

bool CRecordPhonographButton::PhonographStopMsg(CPhonographStopMsg *msg) {
	if (_active) {
		playSound(TRANSLATE("z#57.wav", "z#588.wav"));
		loadFrame(0);
		_active = false;
	}

	return true;
}

} // End of namespace Titanic
