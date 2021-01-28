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

#ifndef NANCY_GRAPHICS_H
#define NANCY_GRAPHICS_H

#include "engines/nancy/nancy.h"
#include "engines/nancy/video.h"
#include "engines/nancy/datatypes.h"

#include "common/func.h"

#include "graphics/screen.h"

namespace Nancy {

class PlaySecondaryVideo;

typedef Common::Functor0Mem<void, GraphicsManager> RenderFunction;

struct ZRenderStruct {
public:
    enum BltType { kNone, kNoTrans, kTrans };

    uint32 z = 0;
    RenderFunction *renderFunction = nullptr;
    Graphics::Surface *sourceSurface = nullptr;
    Common::Rect sourceRect;
    Common::Rect destRect;
    bool isActive = false;
    bool isInitialized = false;
    BltType bltType = kNone;
    Common::String name;
};

struct VideoChannel {
        Graphics::Surface surf;
        uint16 beginFrame = 0;
        uint16 endFrame = 0;
        bool loop = false;
        PlaySecondaryVideo *record = nullptr;
        AVFDecoder decoder;

        ~VideoChannel() { surf.free(); decoder.close(); }
};

class GraphicsManager {
public:
    GraphicsManager(NancyEngine *engine);
    virtual ~GraphicsManager();

    void init();

    void clearZRenderStruct(Common::String name);
    void clearZRenderStructs();
    ZRenderStruct &getZRenderStruct(Common::String name);
    Common::String &initZRenderStruct( char const *name,
                            uint32 z,
                            bool isActive,
                            ZRenderStruct::BltType bltType,
                            Graphics::Surface *surface = nullptr,
                            RenderFunction *func = nullptr,
                            Common::Rect *sourceRect = nullptr,
                            Common::Rect *destRect = nullptr );

    virtual void initSceneZRenderStructs(Common::Array<Common::String> &outNames);
    virtual void initMapRenderStructs(Common::Array<Common::String> &outNames);

    void renderDisplay();
    void renderDisplay(Common::Array<Common::String> ids);

    void loadBackgroundVideo(const Common::String &filename);
    const Graphics::Surface *getBackgroundFrame(uint16 frameId);
    uint32 getBackgroundFrameCount();
    uint32 getBackgroundWidth();
    uint32 getBackgroundHeight();

    void loadSecondaryVideo(uint channel, Common::String &filename, PlaySecondaryVideo *record);
    void setupSecondaryVideo(uint channel, uint16 begin, uint16 end, bool loop);
    void playSecondaryVideo(uint channel);
    void stopSecondaryVideo(uint channel);

    void loadSecondaryMovie(Common::String &filename);
    bool playSecondaryMovie(uint16 &outFrameNr);

    Graphics::Surface _background;
    Graphics::Surface _frameTextBox;
    Graphics::Surface _primaryFrameSurface;
    Graphics::Surface _object0Surface;
    Graphics::Surface _inventoryBoxIconsSurface;
    Graphics::Surface _inventoryCursorsSurface;
    Graphics::Surface _genericSurface;

    VideoChannel channels[2];
    Graphics::Surface _secMovieSurface;
    AVFDecoder _secMovieDecoder;

    View viewportDesc;

    static const Graphics::PixelFormat pixelFormat;
    static const uint transColor;

private:
    NancyEngine *_engine;
    Common::HashMap<Common::String, ZRenderStruct> _ZRender;
    Graphics::Screen _screen;
    AVFDecoder _videoDecoder;

    uint _startingZ = 1;
    uint _numTimesRenderedFrame = 0; // no idea why we're doing this

public:
    // custom render functions
    void renderFrame();
    void renderFrameInvBox();
    void renderPrimaryVideo();
    void renderSecVideo0();
    void renderSecVideo1();
    void renderSecMovie();
    void renderOrderingPuzzle();
    void renderRotatingLockPuzzle();
    void renderLeverPuzzle();
    void renderTelephone();
    void renderSliderPuzzle();
    void renderPasswordPuzzle();
    void renderMapLabels();
};

} // End of namespace Nancy

#endif // NANCY_GRAPHICS_H