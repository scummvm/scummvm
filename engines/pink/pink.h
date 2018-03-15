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

#ifndef PINK_PINK_H
#define PINK_PINK_H

#include "common/random.h"
#include "engines/engine.h"
#include "gui/EventRecorder.h"
#include "gui/debugger.h"
#include "file.h"


/*
 *  This is the namespace of the Pink engine.
 *
 *  Status of this engine: In Development
 *
 *  Internal name of original name: OxCart Runtime
 *
 *  Games using this engine:
 *  - The Pink Panther: Passport to Peril
 *  - The Pink Panther: Hokus Pokus Pink
 */

namespace Pink {

class Console;

enum {
    kPinkDebugGeneral = 1 << 0,
    kPinkDebugLoading = 1 << 1,
    kPinkDebugSound = 1 << 2
};

class PinkEngine : public Engine {

public:
    PinkEngine(OSystem *system, const ADGameDescription *desc);

    ~PinkEngine();

    virtual Common::Error run();

private:
    Common::Error init();

    void handleEvent(Common::Event &event);
    void update();

    Console *_console;
    Common::RandomSource _rnd;

    Common::String _nextModule;
    Common::String _nextPage;

    OrbFile  _orb;
    BroFile *_bro;

    const ADGameDescription _desc;
};

} // End of namespace Pink

#endif