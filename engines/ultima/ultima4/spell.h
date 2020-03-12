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

#ifndef ULTIMA4_SPELL_H
#define ULTIMA4_SPELL_H

#include "ultima/ultima4/context.h"
#include "ultima/ultima4/location.h"
#include "ultima/ultima4/map.h"
#include "ultima/ultima4/savegame.h"
#include "ultima/ultima4/sound.h"

namespace Ultima {
namespace Ultima4 {

enum SpellCastError {
    CASTERR_NOERROR,            /* success */
    CASTERR_NOMIX,              /* no mixture available */    
    CASTERR_MPTOOLOW,           /* caster doesn't have enough mp */
    CASTERR_FAILED,             /* the spell failed */
    CASTERR_WRONGCONTEXT,       /* generic 'wrong-context' error (generrally finds the correct
                                   context error message on its own) */
    CASTERR_COMBATONLY,         /* e.g. spell must be cast in combat */
    CASTERR_DUNGEONONLY,        /* e.g. spell must be cast in dungeons */
    CASTERR_WORLDMAPONLY       /* e.g. spell must be cast on the world map */
};

/* Field types for the Energy field spell */
enum EnergyFieldType {
    ENERGYFIELD_NONE,
    ENERGYFIELD_FIRE,
    ENERGYFIELD_LIGHTNING,
    ENERGYFIELD_POISON,
    ENERGYFIELD_SLEEP
};

/**
 * The ingredients for a spell mixture.
 */
class Ingredients {
public:
    Ingredients();
    bool addReagent(Reagent reagent);
    bool removeReagent(Reagent reagent);
    int getReagent(Reagent reagent) const;
    void revert();
    bool checkMultiple(int mixes) const;
    void multiply(int mixes);

private:
    unsigned short _reagents[REAG_MAX];
};

struct Spell {
    enum Param {
        PARAM_NONE,             /* none */
        PARAM_PLAYER,           /* number of a player required */
        PARAM_DIR,              /* direction required */
        PARAM_TYPEDIR,          /* type of field and direction required (energy field) */
        PARAM_PHASE,            /* phase required (gate) */
        PARAM_FROMDIR           /* direction from required (winds) */
    };

    enum SpecialEffects {
        SFX_NONE,               /* none */
        SFX_INVERT,             /* invert the screen (moongates, most normal spells) */
        SFX_TREMOR              /* tremor spell */
    };

    const char *_name;
    int _components;
    LocationContext _context;
    TransportContext _transportContext;
    int (*_spellFunc)(int);
    Param _paramType;
    int _mp;
};

typedef void (*SpellEffectCallback)(int spell, int player, Sound sound);

void spellSetEffectCallback(SpellEffectCallback callback);
const char *spellGetName(unsigned int spell);
int spellGetRequiredMP(unsigned int spell);
LocationContext spellGetContext(unsigned int spell);
TransportContext spellGetTransportContext(unsigned int spell);
Common::String spellGetErrorMessage(unsigned int spell, SpellCastError error);
int spellMix(unsigned int spell, const Ingredients *ingredients);
Spell::Param spellGetParamType(unsigned int spell);
SpellCastError spellCheckPrerequisites(unsigned int spell, int character);
bool spellCast(unsigned int spell, int character, int param, SpellCastError *error, bool spellEffect);
const Spell* getSpell(int i);

} // End of namespace Ultima4
} // End of namespace Ultima

#endif
