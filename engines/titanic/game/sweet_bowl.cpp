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

#include "titanic/game/sweet_bowl.h"
#include "titanic/translation.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CSweetBowl, CGameObject)
	ON_MESSAGE(MovieEndMsg)
	ON_MESSAGE(EnterViewMsg)
	ON_MESSAGE(ActMsg)
END_MESSAGE_MAP()

void CSweetBowl::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	CGameObject::save(file, indent);
}

void CSweetBowl::load(SimpleFile *file) {
	file->readNumber();
	CGameObject::load(file);
}

bool CSweetBowl::MovieEndMsg(CMovieEndMsg *msg) {
	setVisible(false);
	return true;
}

bool CSweetBowl::EnterViewMsg(CEnterViewMsg *msg) {
	setVisible(false);
	loadSound(TRANSLATE("b#43.wav", "b#26.wav"));
	loadSound(TRANSLATE("b#42.wav", "b#25.wav"));
	return true;
}

bool CSweetBowl::ActMsg(CActMsg *msg) {
	if (msg->_action == "Jiggle") {
		setVisible(true);
		playMovie(MOVIE_WAIT_FOR_FINISH | MOVIE_NOTIFY_OBJECT);
		playSound(getRandomNumber(1) == 1 ? 
			TRANSLATE("b#42.wav", "b#25.wav") :
			TRANSLATE("b#43.wav", "b#26.wav"));
	}

	petDisplayMessage(isEquals("BowlNutsRustler") ?
		BOWL_OF_NUTS : NOT_A_BOWL_OF_NUTS);
	return true;
}

} // End of namespace Titanic
