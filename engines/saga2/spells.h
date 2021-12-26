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
 * aint32 with this program; if not, write to the Free Software
 *
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#ifndef SAGA2_SPELLS_H
#define SAGA2_SPELLS_H

namespace Saga2 {

enum SpellID {
	spellNone               = 0,       // none
	spellFire_Storm,                   // Yellow spells
	spellFlaming_Orb,
	spellSunburst,
	spellBolt_of_Flame,
	spellFlaming_Aura,
	spellSun_Ward,
	spellSoul_Sight,
	spellClairvoyance,
	spellVanquish_Graveborn,
	spellSummon_Flame_Wisp,
	spellSoul_Light,
	spellDisintegration,
	spellSun_Flash,
	spellFirewalk,                     // Violet spells
	spellAcid_Spray,
	spellCaustic_Rain,
	spellNether_Ward,
	spellSurestrike,
	spellWardbane,
	spellAdrenal_Fervor,
	spellMind_Tap,
	spellSearing_Thought,
	spellSpell_Barrier,
	spellInner_Balance,
	spellShadow_Walk,
	spellWill_Barrier,
	spellFireball,                     // Red
	spellMeteor_Shower,
	spellWall_of_Fire,
	spellHeat_Ward,
	spellBattle_Fever,
	spellFire_Shield,
	spellPanic,
	spellTerror,
	spellIronskin,
	spellParalysis,
	spellClumsiness,
	spellTimequake,
	spellMagma_Bolt,
	spellIncinerate,
	spellFrost_Bolt,                    // Blue
	spellCold_Wind,
	spellIce_Storm,
	spellIce_Ward,
	spellInvisibility,
	spellSeawalk,
	spellPaths_of_Mist,
	spellRejoin,
	spellMist_Jump,
	spellHaste,
	spellLevitate_Object,
	spellFlight,
	spellSummon_Wind_Wisp,
	spellFreeze,
	spellLightning_Bolt,                // Orange
	spellShocking_Touch,
	spellLightning_Storm,
	spellForce_Ward,
	spellNumbscent,
	spellLethargic_Breeze,
	spellProtect_vs_Evil,
	spellProtect_vs_Undead,
	spellProtect_vs_Ghosts,
	spellAir_of_Constraint,
	spellTime_Stop,
	spellCushion_of_Air,
	spellRing_of_Force,
	spellPoison_Cloud,                  // Green
	spellWord_of_Harm,
	spellGrasping_Earth,
	spellLife_Ward,
	spellDetect_Poison,
	spellResist_Poison,
	spellMinor_Healing,
	spellMajor_Healing,
	spellCritical_Healing,
	spellBanish_Weakness,
	spellWraith,
	spellResurrection,
	spellBounty_of_the_Earth,
	spellLife_Tap,
	skillPickpocket,                    // skills
	skillSeeHidden,
	skillLockPick,
	skillFirstAid,
	skillArchery,
	skillSwordcraft,
	skillShieldcraft,
	skillBludgeon,
	skillThrowing,
	skillSpellcraft,
	skillStealth,
	skillAgility,
	skillBrawn,
	skillVitality,
	skillCartography,
	spellCold_Blast = 96,       // had to skip because of disapeared skills
	spellThorn,
	spellLife_Shield,
	spellFlame_Shield,
	spellCold_Shield,
	spellArc_Shield,
	spellAcid_Shield,
	spellCaustic_Ward,
	spellElectric_Arc,
	spellForce_Bolt,
	spellChill,
	spellEgo_Flash,
	spellIcicles,
	spellMaelstrom,
	spellDeath_Cloud,
	spellVenom_Blast,
	totalSpellBookPages
};

#define nullSpell ((SpellID)0xFF)

} // end of namespace Saga2

#endif
