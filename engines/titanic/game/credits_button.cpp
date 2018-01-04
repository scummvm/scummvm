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

#include "titanic/game/credits_button.h"
#include "titanic/translation.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CCreditsButton, CBackground)
	ON_MESSAGE(MouseButtonUpMsg)
	ON_MESSAGE(MouseButtonDownMsg)
END_MESSAGE_MAP()

CCreditsButton::CCreditsButton() : CBackground(), _fieldE0(1) {
}

void CCreditsButton::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_fieldE0, indent);
	CBackground::save(file, indent);
}

void CCreditsButton::load(SimpleFile *file) {
	file->readNumber();
	_fieldE0 = file->readNumber();
	CBackground::load(file);
}

bool CCreditsButton::MouseButtonUpMsg(CMouseButtonUpMsg *msg) {
	return true;
}

bool CCreditsButton::MouseButtonDownMsg(CMouseButtonDownMsg *msg) {
	if (_fieldE0) {
		playSound(TRANSLATE("a#20.wav", "a#15.wav"));
		CSignalObject signalMsg;
		signalMsg._numValue = 1;
		signalMsg.execute("CreditsPlayer");
	}

	return true;
}

} // End of namespace Titanic
