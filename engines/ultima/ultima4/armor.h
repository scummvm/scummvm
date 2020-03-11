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

#ifndef ULTIMA4_ARMOR_H
#define ULTIMA4_ARMOR_H

#include "ultima/ultima4/savegame.h"
#include "ultima/shared/std/containers.h"
#include "ultima/shared/std/string.h"

namespace Ultima {
namespace Ultima4 {

class ConfigElement;

class Armor {
public:
    typedef Common::String string;

    static const Armor *get(ArmorType a);
    static const Armor *get(const string &name);

    // Getters
    ArmorType getType() const       {return type;   } /**< Returns the ArmorType of the armor */
    const string &getName() const   {return name;   } /**< Returns the name of the armor */
    int getDefense() const          {return defense;} /**< Returns the defense value of the armor */
                                                      /** Returns true if the class given can wear the armor */
    bool canWear(ClassType klass) const {return canuse & (1 << klass);}

private:
    Armor(const ConfigElement &conf);

    static void loadConf();
    static bool confLoaded;
    static Std::vector<Armor *> armors;

    ArmorType type;
    string name;
    unsigned char canuse;
    int defense;
    unsigned short mask;
};

} // End of namespace Ultima4
} // End of namespace Ultima

#endif
