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

#ifndef ULTIMA4_WEAPON_H
#define ULTIMA4_WEAPON_H

#include "ultima/ultima4/savegame.h"
#include "ultima/shared/std/containers.h"

namespace Ultima {
namespace Ultima4 {

class ConfigElement;

class Weapon {
public:
    /**< Flags affecting weapon's behavior. @see Weapon::flags */
    enum Flags {
        WEAP_LOSE					= 0x0001,	/**< lost when used */
        WEAP_LOSEWHENRANGED			= 0x0002,	/**< lost when used for ranged attack */
        WEAP_CHOOSEDISTANCE			= 0x0004,	/**< allows player to choose attack distance */
        WEAP_ALWAYSHITS				= 0x0008,	/**< always hits it's target */
        WEAP_MAGIC					= 0x0010,	/**< is magical */
        WEAP_ATTACKTHROUGHOBJECTS	= 0x0040,	/**< can attack through solid objects */
        WEAP_ABSOLUTERANGE			= 0x0080,	/**< range is absolute (only works at specific distance) */
        WEAP_RETURNS				= 0x0100,	/**< returns to user after used/thrown */
        WEAP_DONTSHOWTRAVEL			= 0x0200	/**< do not show animations when attacking */
    };

public:
    typedef Common::String string;

    static const Weapon *get(WeaponType w);
    static const Weapon *get(const string &name);

    WeaponType getType() const          {return type;}
    const string &getName() const       {return name;}
    const string &getAbbrev() const     {return abbr;}
    bool canReady(ClassType klass) const{return (canuse & (1 << klass)) != 0;}
    int getRange() const                {return range;}
    int getDamage() const               {return damage;}
    const string &getHitTile() const    {return hittile;}
    const string &getMissTile() const   {return misstile;}
    const string &leavesTile() const    {return leavetile;}
    unsigned short getFlags() const     {return flags;}

    bool loseWhenUsed() const           {return flags & WEAP_LOSE;}
    bool loseWhenRanged() const         {return flags & WEAP_LOSEWHENRANGED;}
    bool canChooseDistance() const      {return flags & WEAP_CHOOSEDISTANCE;}
    bool alwaysHits() const             {return flags & WEAP_ALWAYSHITS;}
    bool isMagic() const                {return flags & WEAP_MAGIC;}
    bool canAttackThroughObjects() const{return flags & WEAP_ATTACKTHROUGHOBJECTS;}
    bool rangeAbsolute() const          {return flags & WEAP_ABSOLUTERANGE;}
    bool returns() const                {return flags & WEAP_RETURNS;}
    bool showTravel() const             {return !(flags & WEAP_DONTSHOWTRAVEL);}

private:
    Weapon(const ConfigElement &conf);

    static void loadConf();
    static bool confLoaded;
    static Std::vector<Weapon *> weapons;

    WeaponType type;
    string name;
    string abbr;            /**< abbreviation for the weapon */
    unsigned char canuse;   /**< bitmask of classes that can use weapon */
    int range;              /**< range of weapon */
    int damage;             /**< damage of weapon */
    string hittile;         /**< tile to display a hit */
    string misstile;        /**< tile to display a miss */
    string leavetile;       /**< if the weapon leaves a tile, the tile #, zero otherwise */
    unsigned short flags;
};

} // End of namespace Ultima4
} // End of namespace Ultima

#endif
