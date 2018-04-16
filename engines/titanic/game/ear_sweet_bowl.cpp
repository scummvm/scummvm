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

#include "titanic/game/ear_sweet_bowl.h"
#include "titanic/core/room_item.h"
#include "titanic/pet_control/pet_control.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CEarSweetBowl, CSweetBowl)
	ON_MESSAGE(MovieEndMsg)
	ON_MESSAGE(ReplaceBowlAndNutsMsg)
END_MESSAGE_MAP()

void CEarSweetBowl::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	CSweetBowl::save(file, indent);
}

void CEarSweetBowl::load(SimpleFile *file) {
	file->readNumber();
	CSweetBowl::load(file);
}

bool CEarSweetBowl::MovieEndMsg(CMovieEndMsg *msg) {
	setVisible(false);
	CIsEarBowlPuzzleDone doneMsg;
	doneMsg.execute(findRoom());

	if (!doneMsg._value) {
		CIsParrotPresentMsg parrotMsg;
		parrotMsg.execute(findRoom());

		if (parrotMsg._isPresent) {
			CNutPuzzleMsg nutMsg("Jiggle");
			nutMsg.execute("NutsParrotPlayer");
		}
	}

	return true;
}

bool CEarSweetBowl::ReplaceBowlAndNutsMsg(CReplaceBowlAndNutsMsg *msg) {
	setVisible(false);
	return true;
}

} // End of namespace Titanic
