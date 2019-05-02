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

#include "titanic/game/end_credit_text.h"
#include "titanic/translation.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CEndCreditText, CGameObject)
	ON_MESSAGE(ActMsg)
	ON_MESSAGE(FrameMsg)
	ON_MESSAGE(TimerMsg)
END_MESSAGE_MAP()

void CEndCreditText::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_flag, indent);
	CGameObject::save(file, indent);
}

void CEndCreditText::load(SimpleFile *file) {
	file->readNumber();
	_flag = file->readNumber();
	CGameObject::load(file);
}

bool CEndCreditText::ActMsg(CActMsg *msg) {
	playAmbientSound(TRANSLATE("z#41.wav", "z#573.wav"), VOL_NORMAL, false, false, 0);
	createCredits();
	_flag = true;
	return true;
}

bool CEndCreditText::FrameMsg(CFrameMsg *msg) {
	if (_flag) {
		if (_credits) {
			makeDirty();
		} else {
			addTimer(5000);
			_flag = false;
		}
	}

	return true;
}

bool CEndCreditText::TimerMsg(CTimerMsg *msg) {
	setAmbientSoundVolume(VOL_MUTE, 2, -1);
	sleep(1000);
	quitGame();
	return true;
}

} // End of namespace Titanic
