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

#include "titanic/game/nut_replacer.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CNutReplacer, CGameObject)
	ON_MESSAGE(ReplaceBowlAndNutsMsg)
	ON_MESSAGE(MovieEndMsg)
END_MESSAGE_MAP()

void CNutReplacer::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	CGameObject::save(file, indent);
}

void CNutReplacer::load(SimpleFile *file) {
	file->readNumber();
	CGameObject::load(file);
}

bool CNutReplacer::ReplaceBowlAndNutsMsg(CReplaceBowlAndNutsMsg *msg) {
	setVisible(true);
	playMovie(MOVIE_WAIT_FOR_FINISH | MOVIE_NOTIFY_OBJECT);
	return true;
}

bool CNutReplacer::MovieEndMsg(CMovieEndMsg *msg) {
	setVisible(false);
	return true;
}

} // End of namespace Titanic
