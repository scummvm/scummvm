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

#include "xeen/swordsofxeen/swordsofxeen.h"

namespace Xeen {
namespace SwordsOfXeen {

SwordsOfXeenEngine::SwordsOfXeenEngine(OSystem *syst, const XeenGameDescription *gameDesc)
		: XeenEngine(syst, gameDesc) {
}

void SwordsOfXeenEngine::outerGameLoop() {
	// TODO: Implement Swords of Xeen main menu

	playGame();
}

void SwordsOfXeenEngine::death() {
	error("TODO: Swords of Xeen death screen");
}

void SwordsOfXeenEngine::dream() {
	error("TODO: Swords of Xeen dream sequence, if any");
}

void SwordsOfXeenEngine::showCutscene(const Common::String &name, int status, uint score) {
	_quitMode = QMODE_MENU;
}

} // End of namespace SwordsOfXeen
} // End of namespace Xeen
