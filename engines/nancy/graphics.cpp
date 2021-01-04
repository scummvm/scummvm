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

#include "engines/nancy/graphics.h"

#include "common/error.h"
#include "common/system.h"

#include "graphics/managed_surface.h"

namespace Nancy {

// TODO the original engine uses a sixth byte for the green; 
// so this should be Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0)
// and transColor should be 0x7C0
// so the colors are gonna be slightly wrong for now
//const Graphics::PixelFormat GraphicsManager::pixelFormat = Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0);
//const uint GraphicsManager::transColor = 0x7C0;
const Graphics::PixelFormat GraphicsManager::pixelFormat = Graphics::PixelFormat(2, 5, 5, 5, 0, 10, 5, 0, 0);
const uint GraphicsManager::transColor = 0x3E0;

GraphicsManager::GraphicsManager(NancyEngine *engine) :
        _engine(engine) {
    _screen.create(640, 480, pixelFormat);
}

void GraphicsManager::init() {  
    Common::SeekableReadStream *chunk = _engine->getBootChunkStream("VIEW");
    viewportDesc = View(chunk);
    uint32 width = viewportDesc.destRight - viewportDesc.destLeft;
    uint32 height = viewportDesc.destBottom - viewportDesc.destTop;
    _background.create(width, height, pixelFormat);

    // TODO make a TBOX struct
    chunk = _engine->getBootChunkStream("TBOX");
    chunk->seek(0x28);
    width = chunk->readUint32LE();
    height = chunk->readUint32LE();
    chunk->seek(0x20);
    width -= chunk->readUint32LE();
    height -= chunk->readUint32LE();
    _frameTextBox.create(width, height, pixelFormat);
}

GraphicsManager::~GraphicsManager() {
    _background.free();
    _frameTextBox.free();
    _screen.free();

    for (auto st : _ZRender) {
        delete st.sourceRect;
        delete st.destPoint;
        delete st.renderFunction;
    }
}

void GraphicsManager::clearGenericZRenderStruct(uint id) {
    ZRenderStruct *st = getGenericZRenderStruct(id);
    st->isActive = false;
    st->isInitialized = false;
}

ZRenderStruct *GraphicsManager::getGenericZRenderStruct(uint id) {
    if (id > 60)
        error("Bad ZRender ID!");
    
    return &_ZRender[id];
}

void GraphicsManager::initGenericZRenderStruct(uint id, char const *name, uint32 z, bool isActive, ZRenderStruct::BltType bltType, Graphics::Surface *surface, RenderFunction *func, Common::Rect *sourceRect, Common::Point *destPoint) {
    clearGenericZRenderStruct(id);
    ZRenderStruct *st = getGenericZRenderStruct(id);
    delete st->sourceRect;
    delete st->destPoint;
    delete st->renderFunction;
    st->name = name;
    st->z = z;
    st->isActive = isActive;
    st->isInitialized = true;
    st->bltType = bltType;
    st->sourceSurface = surface;
    st->sourceRect = sourceRect;
    st->destPoint = destPoint;
    st->renderFunction = func;
}

void GraphicsManager::renderDisplay(uint last) {
    int *mask = new int[last+2];
    for (uint i = 0; i <= last; ++i) {
        mask[i] = i;
    }
    // set final member to -1
    mask[last+1] = -1;
    renderDisplay(mask);
    delete[] mask;
}

void GraphicsManager::renderDisplay(int *idMask) {
    char hasBeenRendered[60]{};
    if (!idMask)
        error("Bad ZRender ID Mask!");

    for (uint currentZ = _startingZ; currentZ < 12; ++currentZ) {
        for (uint i = 0; idMask[i] != -1; ++i) {
            int maskCur = idMask[i];
            ZRenderStruct &current = _ZRender[maskCur];
            if (!hasBeenRendered[i] && current.isActive && current.isInitialized && current.z == currentZ) {
                if (current.renderFunction && current.renderFunction->isValid()) {
                    current.renderFunction->operator()();
                }
                else {
                    switch (current.bltType) {
                        // making some assumptions here
                        case ZRenderStruct::BltType::kNoTrans:
                            _screen.blitFrom(*current.sourceSurface, *current.sourceRect, *current.destPoint);
                            break;
                        case ZRenderStruct::BltType::kTrans: {
                            
                            _screen.transBlitFrom(*current.sourceSurface, *current.sourceRect, *current.destPoint, transColor);
                            break; }
                        default:
                            error("Bad ZRender Blt type!");
                    }
                }
                hasBeenRendered[i] = 1;
            }
        }
    }
    _screen.update();
}

void GraphicsManager::loadBackgroundVideo(const Common::String &filename) {
    if (_videoDecoder.isVideoLoaded()) {
        _videoDecoder.close();
    }
    _videoDecoder.loadFile(filename + ".avf");
}

const Graphics::Surface *GraphicsManager::getBackgroundFrame(uint16 frameId)  {
    if (!_videoDecoder.isVideoLoaded()) {
        error("No video loaded");
        return nullptr;
    }
    return _videoDecoder.decodeFrame(frameId);
}

uint32 GraphicsManager::getBackgroundFrameCount() {
    return _videoDecoder.getFrameCount();
}

uint32 GraphicsManager::getBackgroundWidth() {
    return _videoDecoder.getWidth();
}

uint32 GraphicsManager::getBackgroundHeight() {
    return _videoDecoder.getHeight();
}

void GraphicsManager::renderFrame() {
    Graphics::Surface n = *_ZRender[0].sourceSurface->convertTo(pixelFormat);
    _screen.blitFrom(n, *_ZRender[0].sourceRect, *_ZRender[0].destPoint);
    n.free();

    // not sure why we do this
    _numTimesRenderedFrame += 1;
    if (_numTimesRenderedFrame > 1) {
        _startingZ = 2;
        _numTimesRenderedFrame = 0;
    } else {
        _startingZ = 1;
    }
}

void GraphicsManager::renderResTBBatSlider() {
    // not sure why this is its own function
    ZRenderStruct &st = _ZRender[10];
    _screen.blitFrom(*st.sourceSurface, *st.sourceRect, *st.destPoint);
}

void GraphicsManager::renderFrameInvBox() {
    // TODO
}

void GraphicsManager::renderPrimaryVideo() {
    // TODO
}

void GraphicsManager::renderSecVideo0() {
    // TODO
}

void GraphicsManager::renderSecVideo1() {
    // TODO
}

void GraphicsManager::renderSecMovie() {
    // TODO
}

void GraphicsManager::renderOrderingPuzzle() {
    // TODO
}

void GraphicsManager::renderRotatingLockPuzzle() {
    // TODO
}

void GraphicsManager::renderLeverPuzzle() {
    // TODO
}

void GraphicsManager::renderTelephone() {
    // TODO
}

void GraphicsManager::renderSliderPuzzle() {
    // TODO
}

void GraphicsManager::renderPasswordPuzzle() {
    // TODO
}

} // End of namespace Nancy