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

#include "titanic/sound/dome_from_top_of_well.h"

namespace Titanic {

EMPTY_MESSAGE_MAP(CDomeFromTopOfWell, CViewAutoSoundPlayer);

CDomeFromTopOfWell::CDomeFromTopOfWell() : CViewAutoSoundPlayer() {
	_filename = "z#227.wav";
	_volume = 25;
	_repeated = true;
	_stopSeconds = 1;
	_startSeconds = 1;
}

void CDomeFromTopOfWell::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	CViewAutoSoundPlayer::save(file, indent);
}

void CDomeFromTopOfWell::load(SimpleFile *file) {
	file->readNumber();
	CViewAutoSoundPlayer::load(file);
}

} // End of namespace Titanic
