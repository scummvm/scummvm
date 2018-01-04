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

#include "titanic/game/useless_lever.h"
#include "titanic/translation.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CUselessLever, CToggleButton)
	ON_MESSAGE(MouseButtonDownMsg)
	ON_MESSAGE(EnterViewMsg)
END_MESSAGE_MAP()

void CUselessLever::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	CToggleButton::save(file, indent);
}

void CUselessLever::load(SimpleFile *file) {
	file->readNumber();
	CToggleButton::load(file);
}

bool CUselessLever::MouseButtonDownMsg(CMouseButtonDownMsg *msg) {
	if (_fieldE0) {
		playMovie(15, 30, 0);
		playSound(TRANSLATE("z#56.wav", "z#587.wav"));
		_fieldE0 = false;
	} else {
		playMovie(0, 14, 0);
		playSound(TRANSLATE("z#56.wav", "z#587.wav"));
		_fieldE0 = true;
	}

	return true;
}

bool CUselessLever::EnterViewMsg(CEnterViewMsg *msg) {
	loadFrame(_fieldE0 ? 15 : 0);
	return true;
}

} // End of namespace Titanic
