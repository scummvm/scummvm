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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "engines/nancy/ui/button.h"

#include "engines/nancy/state/scene.h"

#include "engines/nancy/input.h"
#include "engines/nancy/cursor.h"
#include "engines/nancy/nancy.h"
#include "engines/nancy/util.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/sound.h"

namespace Nancy {
namespace UI {

void Button::handleInput(NancyInput &input) {
	if (_screenPosition.contains(input.mousePos)) {
		g_nancy->_cursorManager->setCursorType(CursorManager::kHotspotArrow);

		if (input.input & NancyInput::kLeftMouseButtonUp) {
			onClick();
		}
	}
}

void MenuButton::init() {
	Common::SeekableReadStream *bsum = g_nancy->getBootChunkStream("BSUM");

	bsum->seek(0x184, SEEK_SET);
	Common::Rect src;
	readRect(*bsum, src);
	_drawSurface.create(g_nancy->_graphicsManager->_object0, src);
	bsum->skip(16);
	readRect(*bsum, _screenPosition);
	setVisible(false);

	RenderObject::init();
}

void MenuButton::onClick() {
	NancySceneState.requestStateChange(NancyState::kMainMenu);
	g_nancy->_sound->playSound(0x18);
	setVisible(true);
}

void HelpButton::init() {
	Common::SeekableReadStream *bsum = g_nancy->getBootChunkStream("BSUM");

	bsum->seek(0x194, SEEK_SET);
	Common::Rect src;
	readRect(*bsum, src);
	_drawSurface.create(g_nancy->_graphicsManager->_object0, src);
	bsum->skip(16);
	readRect(*bsum, _screenPosition);
	setVisible(false);

	RenderObject::init();
}

void HelpButton::onClick() {
	NancySceneState.requestStateChange(NancyState::kHelp);
	g_nancy->_sound->playSound(0x18);
	setVisible(true);
}

} // End of namespace UI
} // End of namespace Nancy
