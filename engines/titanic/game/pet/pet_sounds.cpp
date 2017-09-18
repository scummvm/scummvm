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

#include "titanic/game/pet/pet_sounds.h"
#include "titanic/translation.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CPETSounds, CGameObject)
	ON_MESSAGE(PETPlaySoundMsg)
	ON_MESSAGE(LoadSuccessMsg)
END_MESSAGE_MAP()

void CPETSounds::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_ticks, indent);
	CGameObject::save(file, indent);
}

void CPETSounds::load(SimpleFile *file) {
	file->readNumber();
	_ticks = file->readNumber();
	CGameObject::load(file);
}

bool CPETSounds::PETPlaySoundMsg(CPETPlaySoundMsg *msg) {
	if (msg->_soundNum == 1) {
		playSound(TRANSLATE("z#65.wav", "z#596.wav"));
	} else if (msg->_soundNum == 2 && stateGetParrotMet()) {
		uint ticks = getTicksCount();
		if (!_ticks || ticks > (_ticks + 12000)) {
			playSound(TRANSLATE("z#36.wav", "z#568.wav"));
			_ticks = ticks;
		}
	}

	return true;
}

bool CPETSounds::LoadSuccessMsg(CLoadSuccessMsg *msg) {
	_ticks = 0;
	return true;
}

} // End of namespace Titanic
