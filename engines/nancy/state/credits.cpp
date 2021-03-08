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
    Common::SeekableReadStream *cred = _engine->getBootChunkStream("CRED");
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
    _engine->_res->loadImage("ciftree", buf, surf);
    _fullTextSurface.create(surf.w, surf.h + _text._screenPosition.height() * 2, GraphicsManager::pixelFormat);
    _fullTextSurface.clear(GraphicsManager::transColor);
    _fullTextSurface.blitFrom(surf, Common::Point(0, _text._screenPosition.height()));
    surf.free();
    
    Common::Rect src = _text._screenPosition;
    src.moveTo(Common::Point());
    _text._drawSurface.create(_fullTextSurface, src);
    _text.init();

    _engine->sound->loadSound(_sound);
    _engine->sound->playSound(_sound);

    _returnToState = _engine->getPreviousState();

    _background.registerGraphics();
    _text.registerGraphics();

    _engine->cursorManager->showCursor(false);

    _state = kRun;
}

void Credits::run() {
    NancyInput input = _engine->input->getInput();

    if (input.input & NancyInput::kLeftMouseButtonDown) {
        _state = kInit;
        _engine->sound->stopSound(_sound);
        if (_returnToState == NancyEngine::kMainMenu) {
            _engine->setState((NancyEngine::GameState)_returnToState, NancyEngine::kScene);
        } else {
            _engine->setState((NancyEngine::GameState)_returnToState);
        }
        _engine->cursorManager->showCursor(true);
        _fullTextSurface.free();
    }

    Time currentTime = _engine->getTotalPlayTime();
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
