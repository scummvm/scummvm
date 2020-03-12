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

#ifndef ULTIMA4_SAVEGAME_H
#define ULTIMA4_SAVEGAME_H

#include "common/stream.h"

namespace Ultima {
namespace Ultima4 {

#define PARTY_SAV_BASE_FILENAME         "party.sav"
#define MONSTERS_SAV_BASE_FILENAME      "monsters.sav"
#define OUTMONST_SAV_BASE_FILENAME      "outmonst.sav"

#define MONSTERTABLE_SIZE                32
#define MONSTERTABLE_CREATURES_SIZE        8
#define MONSTERTABLE_OBJECTS_SIZE        (MONSTERTABLE_SIZE - MONSTERTABLE_CREATURES_SIZE)

class Object;

/**
 * The list of all weapons.  These values are used in both the
 * inventory fields and character records of the savegame.
 */
enum WeaponType {
    WEAP_HANDS,
    WEAP_STAFF, 
    WEAP_DAGGER,
    WEAP_SLING,
    WEAP_MACE,
    WEAP_AXE,
    WEAP_SWORD,
    WEAP_BOW,
    WEAP_CROSSBOW,
    WEAP_OIL,
    WEAP_HALBERD,
    WEAP_MAGICAXE,
    WEAP_MAGICSWORD,
    WEAP_MAGICBOW,
    WEAP_MAGICWAND,
    WEAP_MYSTICSWORD,
    WEAP_MAX
};

/**
 * The list of all armor types.  These values are used in both the
 * inventory fields and character records of the savegame.
 */
enum ArmorType {
    ARMR_NONE,
    ARMR_CLOTH,
    ARMR_LEATHER,
    ARMR_CHAIN,
    ARMR_PLATE,
    ARMR_MAGICCHAIN,
    ARMR_MAGICPLATE,
    ARMR_MYSTICROBES,
    ARMR_MAX
};

/**
 * The list of sex values for the savegame character records.  The
 * values match the male and female symbols in the character set.
 */
enum SexType {
    SEX_MALE = 0xb,
    SEX_FEMALE = 0xc
};

/**
 * The list of class types for the savegame character records.
 */
enum ClassType {
    CLASS_MAGE,
    CLASS_BARD,
    CLASS_FIGHTER,
    CLASS_DRUID,
    CLASS_TINKER,
    CLASS_PALADIN,
    CLASS_RANGER,
    CLASS_SHEPHERD
};

/**
 * The list of status values for the savegame character records.  The
 * values match the letter thats appear in the ztats area.
 */
enum StatusType {
    STAT_GOOD = 'G',
    STAT_POISONED = 'P',
    STAT_SLEEPING = 'S',
    STAT_DEAD = 'D'
};

enum Virtue {
    VIRT_HONESTY,
    VIRT_COMPASSION,
    VIRT_VALOR,
    VIRT_JUSTICE,
    VIRT_SACRIFICE,
    VIRT_HONOR,
    VIRT_SPIRITUALITY,
    VIRT_HUMILITY,
    VIRT_MAX
};

enum BaseVirtue {
    VIRT_NONE       = 0x00,
    VIRT_TRUTH      = 0x01,
    VIRT_LOVE       = 0x02,
    VIRT_COURAGE    = 0x04
};

enum Reagent {
    REAG_ASH,
    REAG_GINSENG,
    REAG_GARLIC,
    REAG_SILK,
    REAG_MOSS,
    REAG_PEARL,
    REAG_NIGHTSHADE,
    REAG_MANDRAKE,
    REAG_MAX
};

#define SPELL_MAX 26

enum Item {
    ITEM_SKULL  = 0x01,
    ITEM_SKULL_DESTROYED = 0x02,
    ITEM_CANDLE = 0x04,
    ITEM_BOOK   = 0x08,
    ITEM_BELL   = 0x10,
    ITEM_KEY_C  = 0x20,
    ITEM_KEY_L  = 0x40,
    ITEM_KEY_T  = 0x80,
    ITEM_HORN   = 0x100,
    ITEM_WHEEL  = 0x200,
    ITEM_CANDLE_USED = 0x400,
    ITEM_BOOK_USED = 0x800,
    ITEM_BELL_USED = 0x1000
};

enum Stone {
    STONE_BLUE   = 0x01,
    STONE_YELLOW = 0x02,
    STONE_RED    = 0x04,
    STONE_GREEN  = 0x08,
    STONE_ORANGE = 0x10,
    STONE_PURPLE = 0x20,
    STONE_WHITE  = 0x40,
    STONE_BLACK  = 0x80
};

enum Rune {
    RUNE_HONESTY      = 0x01,
    RUNE_COMPASSION   = 0x02,
    RUNE_VALOR        = 0x04,
    RUNE_JUSTICE      = 0x08,
    RUNE_SACRIFICE    = 0x10,
    RUNE_HONOR        = 0x20,
    RUNE_SPIRITUALITY = 0x40,
    RUNE_HUMILITY     = 0x80
};

/**
 * The Ultima IV savegame player record data.  
 */
struct SaveGamePlayerRecord {
    int write(Common::WriteStream *f) const;
    int read(Common::ReadStream *f);
    void init();

    unsigned short _hp;
    unsigned short _hpMax;
    unsigned short _xp;
    unsigned short _str, _dex, _intel;
    unsigned short _mp;
    unsigned short _unknown;
    WeaponType _weapon;
    ArmorType armor;
    char name[16];
    SexType _sex;
    ClassType _class;
    StatusType _status;
};

/**
 * How Ultima IV stores monster information
 */
struct SaveGameMonsterRecord {
    byte _tile;
    byte _x;
    byte _y;
    byte _prevTile;
    byte _prevX;
    byte _prevY;
    byte _unused1;
    byte _unused2;
};

/**
 * Represents the on-disk contents of PARTY.SAV.
 */
struct SaveGame {
    int write(Common::WriteStream *f) const;
    int read(Common::ReadStream *f);
    void init(const SaveGamePlayerRecord *avatarInfo);

    unsigned int _unknown1;
    unsigned int _moves;
    SaveGamePlayerRecord _players[8];
    int _food;
    short _gold;
    short _karma[VIRT_MAX];
    short _torches;
    short _gems;
    short _keys;
    short _sextants;
    short _armor[ARMR_MAX];
    short _weapons[WEAP_MAX];
    short _reagents[REAG_MAX];
    short _mixtures[SPELL_MAX];
    unsigned short _items;
    byte _x, _y;
    byte _stones;
    byte _runes;
    unsigned short _members;
    unsigned short _transport;
    union {
        unsigned short _balloonstate;
        unsigned short _torchduration;
    };
    unsigned short _trammelPhase;
    unsigned short _feluccaPhase;
    unsigned short _shipHull;
    unsigned short _lbIntro;
    unsigned short _lastCamp;
    unsigned short _lastReagent;
    unsigned short _lastMeditation;
    unsigned short _lastVirtue;
    byte _dngX, _dngY;
    unsigned short _orientation;
    unsigned short _dngLevel;
    unsigned short _location;
};

int saveGameMonstersWrite(SaveGameMonsterRecord *monsterTable, Common::WriteStream *f);
int saveGameMonstersRead(SaveGameMonsterRecord *monsterTable, Common::ReadStream *f);

} // End of namespace Ultima4
} // End of namespace Ultima

#endif
