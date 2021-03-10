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

#include "engines/nancy/state/help.h"

#include "engines/nancy/nancy.h"
#include "engines/nancy/commontypes.h"
#include "engines/nancy/sound.h"
#include "engines/nancy/input.h"
#include "engines/nancy/cursor.h"

#include "common/stream.h"

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
    Common::SeekableReadStream *chunk = NanEngine.getBootChunkStream("HELP");

    chunk->seek(0);
    char buf[10];
    chunk->read(buf, 10);
    _image.init(buf);

    chunk->skip(20);
    _hotspot.left = chunk->readUint16LE();
    _hotspot.top = chunk->readUint16LE();
    _hotspot.right = chunk->readUint16LE();
    _hotspot.bottom = chunk->readUint16LE();
    
    chunk = NanEngine.getBootChunkStream("MSND");
    chunk->seek(0);
	_sound.read(*chunk, SoundDescription::kMenu);

    _state = kBegin;
}

void Help::begin() {
	NanEngine.sound->loadSound(_sound);
	NanEngine.sound->playSound(_sound);
    
    _image.registerGraphics();
    _image.setVisible(true);

    NanEngine.cursorManager->setCursorType(CursorManager::kNormalArrow);
    
    _state = kRun;
}

void Help::run() {
    NancyInput input = NanEngine.input->getInput();

    if (_hotspot.contains(input.mousePos) && input.input & NancyInput::kLeftMouseButtonUp) {
        NanEngine.sound->playSound(0x18); // Hardcoded by original engine
        _state = kWaitForSound;
    }
}

void Help::waitForSound() {
    if (!NanEngine.sound->isSoundPlaying(18)) {
	    NanEngine.sound->stopSound(_sound);
        NanEngine.setPreviousState();
    }
}

} // End of namespace State
} // End of namespace Nancy
