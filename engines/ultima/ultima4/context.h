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

#ifndef ULTIMA4_CONTEXT_H
#define ULTIMA4_CONTEXT_H

#include "ultima/ultima4/location.h"
#include "ultima/ultima4/aura.h"
#include "ultima/ultima4/names.h"
#include "ultima/ultima4/person.h"
#include "ultima/ultima4/script.h"
#include "ultima/ultima4/types.h"
#include "ultima/ultima4/savegame.h"
#include "ultima/shared/std/containers.h"

namespace Ultima {
namespace Ultima4 {

class Object;
class Party;
class Person;
class Script;
class StatsArea;

typedef enum {
    TRANSPORT_FOOT      = 0x1,
    TRANSPORT_HORSE     = 0x2,
    TRANSPORT_SHIP      = 0x4,
    TRANSPORT_BALLOON		= 0x8,
    TRANSPORT_FOOT_OR_HORSE	= TRANSPORT_FOOT | TRANSPORT_HORSE,
    TRANSPORT_ANY			= 0xffff
} TransportContext;

/**
 * Context class
 */
class Context : public Script::Provider {
public:
    Context();
    ~Context();

    Party *party;
    SaveGame *saveGame;
    class Location *location;
    int line, col;
    StatsArea *stats;
    int moonPhase;
    int windDirection;
    int windCounter;
    bool windLock;
    Aura *aura;    
    int horseSpeed;
    int opacity;
    TransportContext transportContext;
    uint32 lastCommandTime;
    class Object *lastShip;

    /**
     * Provides scripts with information
     */
    virtual Common::String translate(Std::vector<Common::String> &parts) {
        if (parts.size() == 1) {
            if (parts[0] == "wind")
                return getDirectionName(static_cast<Direction>(windDirection));
        }
        return "";
    }
};

extern Context *c;

} // End of namespace Ultima4
} // End of namespace Ultima

#endif
