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

#ifndef SAGA2_LOCALIZE_H
#define SAGA2_LOCALIZE_H

namespace Saga2 {

#define ALLOCATION_ERROR    "Required allocation failed"
#define RESHANDLE_ERROR     "Memory heap exhausted"

#define WOUNDED_STATUS      "%s is badly wounded!"
#define HURT_STATUS         "%s is hurt."
#define CALENDAR_STATUS     "It is %s, day %d, week %d."

#define HOUR00_TIME         "midnight"
#define HOUR02_TIME         "morning"
#define HOUR06_TIME         "mid-morning"
#define HOUR12_TIME         "noon"
#define HOUR18_TIME         "mid-afternoon"
#define HOUR22_TIME         "evening"


#define IDEAS_MENTAL        "Ideas"
#define SPELL_MENTAL        "Spells"
#define SKILL_MENTAL        "Skills"

#define CLOSE_MOUSE         "Close"
#define SCROLL_MOUSE        "Scroll Inventory"

#define ASLEEP_STATE        "Asleep"
#define PARALY_STATE        "Paralysed"
#define BLIND_STATE         "Blind"
#define AFRAID_STATE        "Afraid"
#define ANGRY_STATE         "Angry"
#define BADWND_STATE        "Badly Wounded"
#define HURT_STATE          "Hurt"
#define POISON_STATE        "Poisoned"
#define DISEAS_STATE        "Diseased"
#define NORMAL_STATE        "Normal"
#define DEAD_STATE          "Dead"

#define JULIAN_BROSTATE     "Julian is: "
#define PHILLIP_BROSTATE    "Phillip is: "
#define KEVIN_BROSTATE      "Kevin is: "

#define ON_AGRESS           "Agression: On"
#define OFF_AGRESS          "Agression: Off"

#define NO_ARMOR            "No Armor Worn"
#define DESC_ARMOR          "Absorb:%d, Reduce:/%d, Defense:+%d"

#define ON_CENTER           "Center: On"
#define OFF_CENTER          "Center: Off"

#define ON_BANDED           "Banding: On"
#define OFF_BANDED          "Banding: Off"

#define OPTIONS_PANEL       "Options Panel"

#define HEALTH_HINT         "Health:"
#define WEIGHT_HINT         "Weight:"
#define BULK_HINT           "Bulk:"
#define UNK_WEIGHT_HINT     "N/A"
#define UNK_BULK_HINT       "N/A"

#define RED_MANA            "Red Mana:"
#define ORANGE_MANA         "Orange Mana:"
#define YELLOW_MANA         "Yellow Mana:"
#define GREEN_MANA          "Green Mana:"
#define BLUE_MANA           "Blue Mana:"
#define VIOLET_MANA         "Purple Mana:"


#define DOS_CMDLINE         "Syntax : FTA2 \n"
#define WIN_CMDLINE         "Syntax : FTA2 \n"

#define SINGLE_CHARGE       "%s, %d Charge"
#define MULTI_CHARGE        "%s, %d Charges"

#define PLURAL_DESC         "%d %ss"

#define IDEAS_INVENT        "Mental Inventory: Ideas"
#define SPELL_INVENT        "Mental Inventory: Spells"
#define SKILL_INVENT        "Mental Inventory: Skills"

#define ERROR_HEADING       "Error"
#define ERROR_OK_BUTTON     "OK"
#define ERROR_RE_BUTTON     "_Retry"
#define ERROR_CA_BUTTON     "_Cancel"
#define ERROR_YE_BUTTON     "_Yes"
#define ERROR_NO_BUTTON     "_No"

#define ARCHERY_SKILL       "archery skill"
#define SWORD_SKILL         "swordcraft skill"
#define SHIELD_SKILL        "shieldcraft skill"
#define BLUDGEON_SKILL      "bludgeon skill"
#define DEAD_SKILL          ""
#define SPELL_SKILL         "spellcraft"
#define DEAD2_SKILL         ""
#define AGILITY_SKILL       "agility"
#define BRAWN_SKILL         "brawn"

#define SKILL_STATUS         "%s's %s has increased."
#define VITALITY_STATUS      "%s's vitality has increased."

#define ATTACK_STATUS        "%s is under attack!"
#define DEATH_STATUS         "%s has died!"

} // end of namespace Saga2

#endif
