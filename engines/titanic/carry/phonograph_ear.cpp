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

#include "titanic/carry/phonograph_ear.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CPhonographEar, CEar)
	ON_MESSAGE(CorrectMusicPlayedMsg)
	ON_MESSAGE(PETGainedObjectMsg)
	ON_MESSAGE(TimerMsg)
END_MESSAGE_MAP()

void CPhonographEar::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_replacementEar, indent);
	CEar::save(file, indent);
}

void CPhonographEar::load(SimpleFile *file) {
	file->readNumber();
	_replacementEar = file->readNumber();
	CEar::load(file);
}

bool CPhonographEar::CorrectMusicPlayedMsg(CCorrectMusicPlayedMsg *msg) {
	_canTake = true;
	return true;
}

bool CPhonographEar::PETGainedObjectMsg(CPETGainedObjectMsg *msg) {
	if (_replacementEar) {
		// Start a timer to add a replacement ear to the Phonograph
		_replacementEar = false;
		addTimer(1000);
	}

	return CEar::PETGainedObjectMsg(msg);
}

bool CPhonographEar::TimerMsg(CTimerMsg *msg) {
	CVisibleMsg visibleMsg;
	visibleMsg.execute("Replacement Phonograph Ear");
	return true;
}

} // End of namespace Titanic
