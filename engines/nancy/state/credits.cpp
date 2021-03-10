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

#include "engines/nancy/state/credits.h"

#include "engines/nancy/nancy.h"
#include "engines/nancy/resource.h"
#include "engines/nancy/util.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/sound.h"
#include "engines/nancy/input.h"
#include "engines/nancy/cursor.h"

#include "common/stream.h"

namespace Common {
DECLARE_SINGLETON(Nancy::State::Credits);
}

namespace Nancy {
namespace State {

void Credits::process() {
    switch (_state) {
    case kInit:
        init();
        // fall through
    case kRun:
        run();
        break;
    }
}

void Credits::init() {
    Common::SeekableReadStream *cred = NanEngine.getBootChunkStream("CRED");
    cred->seek(0);

    char buf[10];
    cred->read(buf, 10);
    _background.init(buf);

    cred->read(buf, 10);
    cred->skip(0x20); // Skip the src and dest rectangles
    readRect(*cred, _text._screenPosition);
    cred->skip(0x10);
    _updateTime = cred->readUint16LE();
    _pixelsToScroll = cred->readUint16LE();
    _sound.read(*cred, SoundDescription::kMenu);

    Graphics::Surface surf;
    NanEngine.resource->loadImage(buf, surf);
    _fullTextSurface.create(surf.w, surf.h + _text._screenPosition.height() * 2, GraphicsManager::pixelFormat);
    _fullTextSurface.clear(GraphicsManager::transColor);
    _fullTextSurface.blitFrom(surf, Common::Point(0, _text._screenPosition.height()));
    surf.free();
    
    Common::Rect src = _text._screenPosition;
    src.moveTo(Common::Point());
    _text._drawSurface.create(_fullTextSurface, src);
    _text.init();

    NanEngine.sound->loadSound(_sound);
    NanEngine.sound->playSound(_sound);

    _background.registerGraphics();
    _text.registerGraphics();

    NanEngine.cursorManager->showCursor(false);

    _state = kRun;
}

void Credits::run() {
    NancyInput input = NanEngine.input->getInput();

    if (input.input & NancyInput::kLeftMouseButtonDown) {
        _state = kInit;
        NanEngine.sound->stopSound(_sound);
        NanEngine.setState(NancyEngine::kMainMenu);
        NanEngine.cursorManager->showCursor(true);
        _fullTextSurface.free();
    }

    Time currentTime = NanEngine.getTotalPlayTime();
    if (currentTime >= _nextUpdateTime) {
        _nextUpdateTime = currentTime + _updateTime;

        Common::Rect newSrc = _text._screenPosition;
        newSrc.moveTo(_text._drawSurface.getOffsetFromOwner());
        newSrc.translate(0, _pixelsToScroll);

        if (newSrc.bottom > _fullTextSurface.h) {
            newSrc.moveTo(Common::Point());
        }

        _text._drawSurface.create(_fullTextSurface, newSrc);
        _text._needsRedraw = true;
    }
}

} // End of namespace State
} // End of namespace Nancy
