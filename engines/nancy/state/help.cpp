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

#include "engines/nancy/nancy.h"
#include "engines/nancy/sound.h"
#include "engines/nancy/input.h"
#include "engines/nancy/cursor.h"
#include "engines/nancy/util.h"

#include "engines/nancy/state/help.h"

namespace Common {
DECLARE_SINGLETON(Nancy::State::Help);
}

namespace Nancy {
namespace State {

void Help::process() {
	switch (_state) {
	case kInit:
		init();
		// fall through
	case kBegin:
		begin();
		// fall through
	case kRun:
		run();
		break;
	case kWaitForSound:
		waitForSound();
		break;
	}
}

void Help::init() {
	Common::SeekableReadStream *chunk = g_nancy->getBootChunkStream("HELP");

	chunk->seek(0);
	Common::String imageName;
	readFilename(*chunk, imageName);
	_image.init(imageName);

	chunk->skip(20);
	_hotspot.left = chunk->readUint16LE();
	_hotspot.top = chunk->readUint16LE();
	_hotspot.right = chunk->readUint16LE();
	_hotspot.bottom = chunk->readUint16LE();

	_state = kBegin;
}

void Help::begin() {
	if (!g_nancy->_sound->isSoundPlaying("MSND")) {
		g_nancy->_sound->playSound("MSND");
	}

	_image.registerGraphics();
	_image.setVisible(true);

	g_nancy->_cursorManager->setCursorType(CursorManager::kNormalArrow);

	_state = kRun;
}

void Help::run() {
	NancyInput input = g_nancy->_input->getInput();

	if (_hotspot.contains(input.mousePos) && input.input & NancyInput::kLeftMouseButtonUp) {
		g_nancy->_sound->playSound("BUOK");
		_state = kWaitForSound;
	}
}

void Help::waitForSound() {
	if (!g_nancy->_sound->isSoundPlaying("BUOK")) {
		g_nancy->_sound->stopSound("BUOK");
		g_nancy->setToPreviousState();
	}
}

} // End of namespace State
} // End of namespace Nancy
