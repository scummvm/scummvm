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

#include "ultima/ultima4/aura.h"

namespace Ultima {
namespace Ultima4 {

Aura::Aura() : type(NONE), duration(0) {}

void Aura::setDuration(int d) {
    duration = d;
    setChanged();
    notifyObservers(NULL);
}

void Aura::set(Type t, int d) {
    type = t;
    duration = d;
    setChanged();
    notifyObservers(NULL);
}

void Aura::setType(Type t) {
    type = t;
    setChanged();
    notifyObservers(NULL);
}

void Aura::passTurn() {
    if (duration > 0) {
        duration--;
        
        if (duration == 0) {
            type = NONE;

            setChanged();
            notifyObservers(NULL);
        }
    }
}

} // End of namespace Ultima4
} // End of namespace Ultima
