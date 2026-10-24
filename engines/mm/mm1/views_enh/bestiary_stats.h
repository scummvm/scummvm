/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef MM1_VIEWS_ENH_BESTIARY_STATS_H
#define MM1_VIEWS_ENH_BESTIARY_STATS_H

#include "common/array.h"
#include "mm/mm1/data/monsters.h"

namespace MM {
namespace MM1 {
namespace ViewsEnh {

/**
 * Returns the stat lines for a monster's database record, not a live combat
 * instance. Monsters that haven't been seen yield no lines
 */
inline Common::Array<Common::String> makeBestiaryStats(const Monster &m, bool seen, int page) {
	Common::Array<Common::String> lines;
	if (!seen)
		return lines;

	if (page == 0) {
		lines.push_back(Common::String::format("Base HP: %u", (uint)m._defaultHP));
		lines.push_back(Common::String::format("Armor class: %u", (uint)m._defaultAC));
		lines.push_back(Common::String::format("Speed: %u", (uint)m._speed));
		lines.push_back(Common::String::format("Attacks: %u", (uint)m._numberOfAttacks));
		lines.push_back(Common::String::format("Max damage/hit: %u", (uint)m._maxDamage));
		lines.push_back(Common::String::format("Experience: %u", (uint)m._experience));

		// Combat compares this rating with a roll depending on caster level
		lines.push_back(Common::String::format("Magic resist rating: %u",
			(uint)(m._resistUndead & MAGIC_RESISTANCE)));
		lines.push_back(Common::String::format("Undead: %s",
			(m._resistUndead & IS_UNDEAD) ? "Yes" : "No"));
		lines.push_back(Common::String::format("Regenerates: %s",
			(m._counterFlags & COUNTER_REGENERATE) ? "Yes" : "No"));
		return lines;
	}

	static const char *const LABELS[8] = {
		"Fire resistance", "Cold resistance", "Electricity resistance",
		"Energy resistance", "Sleep immunity", "Fear immunity",
		"Paralysis immunity", "Normal weapon immunity"
	};
	static const byte FLAGS[8] = {
		MONRES_FIRE, MONRES_COLD, MONRES_ELECTRICITY, MONRES_ENERGY,
		MONRES_ASLEEP, MONRES_FEAR, MONRES_PARALYSIS, MONRES_PHYSICAL_ATTACK
	};
	for (int i = 0; i < 8; ++i) {
		lines.push_back(Common::String::format("%s: %s", LABELS[i],
			(m._resistances & FLAGS[i]) ? "Yes" : "No"));
	}

	// Same 1-based order as SpellsMonsters::SPELLS
	static const char *const SPELLS[33] = {
		"None", "Curse", "Energy blast", "Fire breath", "Blindness",
		"Poison spray", "Acid spray", "Sleep", "Paralyze", "Dispel",
		"Lightning bolt", "Strange gas", "Explosion", "Fireball",
		"Fire breath", "Gaze", "Acid arrow", "Elements", "Cold beam",
		"Dancing sword", "Magic drain", "Finger of death", "Sun ray",
		"Disintegration", "Commands energy", "Poison", "Lightning",
		"Frost", "Spikes", "Acid", "Fire", "Energy", "Swarm"
	};
	if (m._specialAbility & HAS_RANGED_ATTACK) {
		lines.push_back(Common::String::format("Special: Ranged (max damage %u)",
			(uint)(m._specialAbility & ATTACK_VALUE)));
	} else {
		lines.push_back(Common::String::format("Special: %s",
			m._specialAbility < 33 ? SPELLS[m._specialAbility] : "Unknown"));
	}
	lines.push_back(Common::String::format("Special uses/battle: %u",
		(uint)(m._counterFlags & COUNTER_BITS)));

	// MonsterTouch dispatches byte value N to ACTIONS[N - 1]
	static const char *const TOUCHES[26] = {
		"None", "Takes food", "Disease", "Paralysis", "Poison",
		"Steals gems", "Reduces endurance", "Sleep", "Paralysis", "Disease",
		"Steals gold", "Steals an item", "Poison", "Blindness",
		"Drains 1 level", "Petrification", "Aging", "Drains 2 levels",
		"Death", "Unconsciousness", "Drains might", "Drains attributes",
		"Steals backpack", "Steals gold and gems", "Eradication",
		"Drains spell points"
	};
	const char *touch = (m._bonusOnTouch & 0x80) ? "None" :
		(m._bonusOnTouch < 26 ? TOUCHES[m._bonusOnTouch] : "Unknown");
	lines.push_back(Common::String::format("Touch: %s", touch));
	lines.push_back(Common::String::format("Advances into melee: %s",
		(m._counterFlags & COUNTER_ADVANCES) ? "Yes" : "No"));
	return lines;
}

/**
 * Returns the effect a monster spell inflicts, from the actual spell
 * handlers. Poison breath is damage, unlike poison spray, and Commands
 * Energy also deals damage rather than setting a status
 */
inline const char *bestiarySpellStatus(byte spell) {
	switch (spell) {
	case 1:
		return "Curse";
	case 4:
		return "Blindness";
	case 5:
		return "Poison";
	case 7:
		return "Sleep";
	case 8:
		return "Paralysis";
	case 11:
	case 15:
		return "Petrification";
	case 21:
		return "Death";
	case 23:
		return "Eradication";
	default:
		return "None";
	}
}

/**
 * Returns the text lines for one of the two columns beside a monster's
 * picture. The left column holds stats or defenses depending on the page,
 * and the right column holds the special abilities
 */
inline Common::Array<Common::String> bestiaryColumn(const Monster &m, bool seen, bool specials, int page) {
	Common::Array<Common::String> out;
	if (!seen)
		return out;

	const Common::Array<Common::String> stats = makeBestiaryStats(m, true, 0);
	const Common::Array<Common::String> abilities = makeBestiaryStats(m, true, 1);

	if (specials) {
		if (m._specialAbility & HAS_RANGED_ATTACK) {
			out.push_back("Ranged attack");
			out.push_back(Common::String::format("Max damage: %u",
				(uint)(m._specialAbility & ATTACK_VALUE)));
		} else {
			out.push_back("Ability:");
			out.push_back(abilities[8].substr(9));

			const char *status = bestiarySpellStatus(m._specialAbility);
			if (Common::String(status) != "None") {
				out.push_back("Inflicts:");
				out.push_back(status);
			}

			if (m._specialAbility == 9)
				out.push_back("Dispels buffs");

			if (m._specialAbility == 20) {
				out.push_back("Drains SP");
				out.push_back("Dispels buffs");
			}
		}

		out.push_back(Common::String::format("Uses/battle: %u",
			(uint)(m._counterFlags & COUNTER_BITS)));
		out.push_back(abilities[10]);
		out.push_back(Common::String::format("Regen: %s",
			(m._counterFlags & COUNTER_REGENERATE) ? "Yes" : "No"));
		out.push_back(stats[7]);
		out.push_back(Common::String::format("Advances: %s",
			(m._counterFlags & COUNTER_ADVANCES) ? "Yes" : "No"));

	} else if (page == 0) {
		for (uint i = 0; i < 4; ++i)
			out.push_back(stats[i]);

		out.push_back(Common::String::format("Max dmg/hit: %u", (uint)m._maxDamage));
		out.push_back(Common::String::format("XP: %u", (uint)m._experience));
		out.push_back(Common::String::format("Magic resist: %u",
			(uint)(m._resistUndead & MAGIC_RESISTANCE)));

	} else {
		static const char *const LABELS[8] = {
			"Fire", "Cold", "Electricity", "Energy",
			"Sleep", "Fear", "Paralysis", "Normal weapons"
		};
		static const byte FLAGS[8] = {
			MONRES_FIRE, MONRES_COLD, MONRES_ELECTRICITY, MONRES_ENERGY,
			MONRES_ASLEEP, MONRES_FEAR, MONRES_PARALYSIS, MONRES_PHYSICAL_ATTACK
		};

		out.push_back("Resists damage:");
		for (int i = 0; i < 8; ++i) {
			if (i == 4)
				out.push_back("Immune to:");

			out.push_back(Common::String::format("%s: %s", LABELS[i],
				(m._resistances & FLAGS[i]) ? "Yes" : "No"));
		}
	}

	return out;
}

} // namespace ViewsEnh
} // namespace MM1
} // namespace MM

#endif
