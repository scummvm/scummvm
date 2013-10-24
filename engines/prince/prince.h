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

#ifndef PRINCE_H
#define PRINCE_H

#include "common/random.h"
#include "common/system.h"
#include "common/debug.h"
#include "common/debug-channels.h"
#include "common/textconsole.h"
#include "common/rect.h"
#include "common/events.h"

#include "graphics/decoders/bmp.h"

#include "gui/debugger.h"

#include "engines/engine.h"
#include "engines/util.h"

#include "audio/mixer.h"

#include "video/flic_decoder.h"

#include "prince/font.h"
#include "prince/mhwanh.h"

namespace Prince {

struct PrinceGameDescription;

class PrinceEngine;
class GraphicsMan;
class Script;
class Debugger;

class PrinceEngine : public Engine {
protected:
    Common::Error run();

public:
    PrinceEngine(OSystem *syst, const PrinceGameDescription *gameDesc);
    virtual ~PrinceEngine();

    virtual bool hasFeature(EngineFeature f) const;

    int getGameType() const;
    const char *getGameId() const;
    uint32 getFeatures() const;
    Common::Language getLanguage() const;

    const PrinceGameDescription *_gameDescription;
    Video::FlicDecoder _flicPlayer;

    bool loadLocation(uint16 locationNr);
    bool loadAnim(uint16 animNr);

    virtual GUI::Debugger *getDebugger();

private:
    bool playNextFrame();
    void keyHandler(Common::Event event);

    Common::RandomSource *_rnd;
    Graphics::BitmapDecoder _roomBmp;
    uint16 _locationNr;
    MhwanhDecoder _walizkaBmp;

    Debugger *_debugger;
    GraphicsMan *_graph;
    Script *_script;
    Font _font;
    
    void mainLoop();

};

} // End of namespace Prince

#endif
