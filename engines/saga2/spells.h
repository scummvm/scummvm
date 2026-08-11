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
	kSpellNone               = 0,       // none
	kSpellFire_Storm,                   // Yellow spells
	kSpellFlaming_Orb,
	kSpellSunburst,
	kSpellBolt_of_Flame,
	kSpellFlaming_Aura,
	kSpellSun_Ward,
	kSpellSoul_Sight,
	kSpellClairvoyance,
	kSpellVanquish_Graveborn,
	kSpellSummon_Flame_Wisp,
	kSpellSoul_Light,
	kSpellDisintegration,
	kSpellSun_Flash,
	kSpellFirewalk,                     // Violet spells
	kSpellAcid_Spray,
	kSpellCaustic_Rain,
	kSpellNether_Ward,
	kSpellSurestrike,
	kSpellWardbane,
	kSpellAdrenal_Fervor,
	kSpellMind_Tap,
	kSpellSearing_Thought,
	kSpellSpell_Barrier,
	kSpellInner_Balance,
	kSpellShadow_Walk,
	kSpellWill_Barrier,
	kSpellFireball,                     // Red
	kSpellMeteor_Shower,
	kSpellWall_of_Fire,
	kSpellHeat_Ward,
	kSpellBattle_Fever,
	kSpellFire_Shield,
	kSpellPanic,
	kSpellTerror,
	kSpellIronskin,
	kSpellParalysis,
	kSpellClumsiness,
	kSpellTimequake,
	kSpellMagma_Bolt,
	kSpellIncinerate,
	kSpellFrost_Bolt,                    // Blue
	kSpellCold_Wind,
	kSpellIce_Storm,
	kSpellIce_Ward,
	kSpellInvisibility,
	kSpellSeawalk,
	kSpellPaths_of_Mist,
	kSpellRejoin,
	kSpellMist_Jump,
	kSpellHaste,
	kSpellLevitate_Object,
	kSpellFlight,
	kSpellSummon_Wind_Wisp,
	kSpellFreeze,
	kSpellLightning_Bolt,                // Orange
	kSpellShocking_Touch,
	kSpellLightning_Storm,
	kSpellForce_Ward,
	kSpellNumbscent,
	kSpellLethargic_Breeze,
	kSpellProtect_vs_Evil,
	kSpellProtect_vs_Undead,
	kSpellProtect_vs_Ghosts,
	kSpellAir_of_Constraint,
	kSpellTime_Stop,
	kSpellCushion_of_Air,
	kSpellRing_of_Force,
	kSpellPoison_Cloud,                  // Green
	kSpellWord_of_Harm,
	kSpellGrasping_Earth,
	kSpellLife_Ward,
	kSpellDetect_Poison,
	kSpellResist_Poison,
	kSpellMinor_Healing,
	kSpellMajor_Healing,
	kSpellCritical_Healing,
	kSpellBanish_Weakness,
	kSpellWraith,
	kSpellResurrection,
	kSpellBounty_of_the_Earth,
	kSpellLife_Tap,
	kSkillPickpocket,                    // skills
	kSkillSeeHidden,
	kSkillLockPick,
	kSkillFirstAid,
	kSkillArchery,
	kSkillSwordcraft,
	kSkillShieldcraft,
	kSkillBludgeon,
	kSkillThrowing,
	kSkillSpellcraft,
	kSkillStealth,
	kSkillAgility,
	kSkillBrawn,
	kSkillVitality,
	kSkillCartography,
	kSpellCold_Blast = 96,       // had to skip because of disappeared skills
	kSpellThorn,
	kSpellLife_Shield,
	kSpellFlame_Shield,
	kSpellCold_Shield,
	kSpellArc_Shield,
	kSpellAcid_Shield,
	kSpellCaustic_Ward,
	kSpellElectric_Arc,
	kSpellForce_Bolt,
	kSpellChill,
	kSpellEgo_Flash,
	kSpellIcicles,
	kSpellMaelstrom,
	kSpellDeath_Cloud,
	kSpellVenom_Blast,
	kTotalSpellBookPages
};

#define kNullSpell ((SpellID)0xFF)

} // end of namespace Saga2

#endif
