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

#include "titanic/game/glass_smasher.h"
#include "titanic/translation.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CGlassSmasher, CGameObject)
	ON_MESSAGE(StatusChangeMsg)
	ON_MESSAGE(MovieEndMsg)
END_MESSAGE_MAP()

void CGlassSmasher::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	CGameObject::save(file, indent);
}

void CGlassSmasher::load(SimpleFile *file) {
	file->readNumber();
	CGameObject::load(file);
}

bool CGlassSmasher::StatusChangeMsg(CStatusChangeMsg *msg) {
	setVisible(true);
	playSound(TRANSLATE("b#40.wav", "b#19.wav"));
	playMovie(MOVIE_NOTIFY_OBJECT | MOVIE_WAIT_FOR_FINISH);
	return true;
}

bool CGlassSmasher::MovieEndMsg(CMovieEndMsg *msg) {
	setVisible(false);
	CVisibleMsg visibleMsg(true);
	visibleMsg.execute("LongStickDispenser");
	return true;
}

} // End of namespace Titanic
