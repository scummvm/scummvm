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

#ifndef ULTIMA4_GAME_SPELL_H
#define ULTIMA4_GAME_SPELL_H

#include "ultima/ultima4/game/context.h"
#include "ultima/ultima4/map/location.h"
#include "ultima/ultima4/map/map.h"
#include "ultima/ultima4/filesys/savegame.h"
#include "ultima/ultima4/sound/sound.h"

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

/**
 * Field types for the Energy field spell
 */
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

class Spells;
typedef int (Spells::*SpellProc)(int);

struct Spell {
	enum Param {
		PARAM_NONE,             ///< None
		PARAM_PLAYER,           ///< number of a player required
		PARAM_DIR,              ///< direction required
		PARAM_TYPEDIR,          ///< type of field and direction required (energy field)
		PARAM_PHASE,            ///< phase required (gate)
		PARAM_FROMDIR           ///< direction from required (winds)
	};

	enum SpecialEffects {
		SFX_NONE,               ///< none
		SFX_INVERT,             ///< invert the screen (moongates, most normal spells)
		SFX_TREMOR              ///< tremor spell
	};

	const char *_name;
	int _components;
	LocationContext _context;
	TransportContext _transportContext;
	SpellProc _spellFunc;
	Param _paramType;
	int _mp;
};

typedef void (*SpellEffectCallback)(int spell, int player, Sound sound);
#define N_SPELLS 26

class Spells {
private:
	static const Spell SPELL_LIST[N_SPELLS];
	SpellEffectCallback spellEffectCallback;
private:
	int spellAwaken(int player);
	int spellBlink(int dir);
	int spellCure(int player);
	int spellDispel(int dir);
	int spellEField(int param);
	int spellFireball(int dir);
	int spellGate(int phase);
	int spellHeal(int player);
	int spellIceball(int dir);
	int spellJinx(int unused);
	int spellKill(int dir);
	int spellLight(int unused);
	int spellMMissle(int dir);
	int spellNegate(int unused);
	int spellOpen(int unused);
	int spellProtect(int unused);
	int spellRez(int player);
	int spellQuick(int unused);
	int spellSleep(int unused);
	int spellTremor(int unused);
	int spellUndead(int unused);
	int spellView(int unsued);
	int spellWinds(int fromdir);
	int spellXit(int unused);
	int spellYup(int unused);
	int spellZdown(int unused);
private:
	CombatController *spellCombatController();

	/**
	 * Makes a special magic ranged attack in the given direction
	 */
	void spellMagicAttack(const Common::String &tilename, Direction dir, int minDamage, int maxDamage);
	bool spellMagicAttackAt(const Coords &coords, MapTile attackTile, int attackDamage);

	LocationContext spellGetContext(uint spell) const;
	TransportContext spellGetTransportContext(uint spell) const;

	/**
	 * Returns true if the debugger is active
	 */
	bool isDebuggerActive() const;
public:
	/**
	 * Constructor
	 */
	Spells();

	/**
	 * Destructor
	 */
	~Spells();

	void spellSetEffectCallback(SpellEffectCallback callback);

	void spellEffect(int spell, int player, Sound sound) {
		(spellEffectCallback)(spell, player, sound);
	}

	/**
	 * Mix reagents for a spell.  Fails and returns false if the reagents
	 * selected were not correct.
	 */
	int spellMix(uint spell, const Ingredients *ingredients);

	/**
	 * Casts spell.  Fails and returns false if the spell cannot be cast.
	 * The error code is updated with the reason for failure.
	 */
	bool spellCast(uint spell, int character, int param, SpellCastError *error, bool spellEffect);

	Common::String spellGetErrorMessage(uint spell, SpellCastError error);

	const char *spellGetName(uint spell) const;

	/**
	 * Checks some basic prerequistes for casting a spell.  Returns an
	 * error if no mixture is available, the context is invalid, or the
	 * character doesn't have enough magic points.
	 */
	SpellCastError spellCheckPrerequisites(uint spell, int character);
	Spell::Param spellGetParamType(uint spell) const;
	int spellGetRequiredMP(uint spell) const;
	const Spell *getSpell(int i) const;
};

extern Spells *g_spells;

} // End of namespace Ultima4
} // End of namespace Ultima

#endif
