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

#include "titanic/carry/bowl_ear.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CBowlEar, CEar)
	ON_MESSAGE(PETGainedObjectMsg)
	ON_MESSAGE(ReplaceBowlAndNutsMsg)
	ON_MESSAGE(NutPuzzleMsg)
	ON_MESSAGE(MouseDragStartMsg)
END_MESSAGE_MAP()

void CBowlEar::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	CEar::save(file, indent);
}

void CBowlEar::load(SimpleFile *file) {
	file->readNumber();
	CEar::load(file);
}

bool CBowlEar::PETGainedObjectMsg(CPETGainedObjectMsg *msg) {
	CBowlStateChangeMsg changeMsg(3);
	changeMsg.execute("ParrotNutBowlActor");

	return CEar::PETGainedObjectMsg(msg);
}

bool CBowlEar::ReplaceBowlAndNutsMsg(CReplaceBowlAndNutsMsg *msg) {
	setVisible(false);
	return true;
}

bool CBowlEar::NutPuzzleMsg(CNutPuzzleMsg *msg) {
	if (msg->_action == "BowlUnlocked")
		_canTake = true;

	return true;
}

bool CBowlEar::MouseDragStartMsg(CMouseDragStartMsg *msg) {
	setVisible(true);
	return CEar::MouseDragStartMsg(msg);
}

} // End of namespace Titanic
