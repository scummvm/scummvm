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

#include "titanic/moves/exit_bridge.h"
#include "titanic/translation.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CExitBridge, CMovePlayerTo)
	ON_MESSAGE(MouseButtonDownMsg)
END_MESSAGE_MAP()

CExitBridge::CExitBridge() : CMovePlayerTo(), _viewName("Titania.Node 1.S") {
}

void CExitBridge::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeQuotedLine(_viewName, indent);

	CMovePlayerTo::save(file, indent);
}

void CExitBridge::load(SimpleFile *file) {
	file->readNumber();
	_viewName = file->readString();

	CMovePlayerTo::load(file);
}

bool CExitBridge::MouseButtonDownMsg(CMouseButtonDownMsg *msg) {
	if (getGameManager()) {
		changeView(_destination);
		playSound(TRANSLATE("a#53.wav", "a#46.wav"));
		changeView(_viewName);
	}

	return true;
}

} // End of namespace Titanic
