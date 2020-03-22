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

#include "ultima/ultima4/game/names.h"

namespace Ultima {
namespace Ultima4 {

const char *getClassName(ClassType klass) {
	switch (klass) {
	case CLASS_MAGE:
		return "Mage";
	case CLASS_BARD:
		return "Bard";
	case CLASS_FIGHTER:
		return "Fighter";
	case CLASS_DRUID:
		return "Druid";
	case CLASS_TINKER:
		return "Tinker";
	case CLASS_PALADIN:
		return "Paladin";
	case CLASS_RANGER:
		return "Ranger";
	case CLASS_SHEPHERD:
		return "Shepherd";
	default:
		return "???";
	}
}

const char *getReagentName(Reagent reagent) {
	static const char *const reagentNames[] = {
		"Sulfur Ash", "Ginseng", "Garlic",
		"Spider Silk", "Blood Moss", "Black Pearl",
		"Nightshade", "Mandrake"
	};

	if (reagent < REAG_MAX)
		return reagentNames[reagent - REAG_ASH];
	else
		return "???";
}

const char *getVirtueName(Virtue virtue) {
	static const char *const virtueNames[] = {
		"Honesty", "Compassion", "Valor",
		"Justice", "Sacrifice", "Honor",
		"Spirituality", "Humility"
	};

	if (virtue < 8)
		return virtueNames[virtue - VIRT_HONESTY];
	else
		return "???";
}

const char *getBaseVirtueName(int virtueMask) {
	if (virtueMask == VIRT_TRUTH)           return "Truth";
	else if (virtueMask == VIRT_LOVE)       return "Love";
	else if (virtueMask == VIRT_COURAGE)    return "Courage";
	else if (virtueMask == (VIRT_TRUTH | VIRT_LOVE)) return "Truth and Love";
	else if (virtueMask == (VIRT_LOVE | VIRT_COURAGE)) return "Love and Courage";
	else if (virtueMask == (VIRT_COURAGE | VIRT_TRUTH)) return "Courage and Truth";
	else if (virtueMask == (VIRT_TRUTH | VIRT_LOVE | VIRT_COURAGE)) return "Truth, Love and Courage";
	else return "???";
}

int getBaseVirtues(Virtue virtue) {
	switch (virtue) {
	case VIRT_HONESTY:
		return VIRT_TRUTH;
	case VIRT_COMPASSION:
		return VIRT_LOVE;
	case VIRT_VALOR:
		return VIRT_COURAGE;
	case VIRT_JUSTICE:
		return VIRT_TRUTH | VIRT_LOVE;
	case VIRT_SACRIFICE:
		return VIRT_LOVE | VIRT_COURAGE;
	case VIRT_HONOR:
		return VIRT_COURAGE | VIRT_TRUTH;
	case VIRT_SPIRITUALITY:
		return VIRT_TRUTH | VIRT_LOVE | VIRT_COURAGE;
	case VIRT_HUMILITY:
		return 0;
	default:
		return 0;
	}
}

const char *getVirtueAdjective(Virtue virtue) {
	static const char *const virtueAdjectives[] = {
		"honest",
		"compassionate",
		"valiant",
		"just",
		"sacrificial",
		"honorable",
		"spiritual",
		"humble"
	};

	if (virtue < 8)
		return virtueAdjectives[virtue - VIRT_HONESTY];
	else
		return "???";
}

const char *getStoneName(Virtue virtue) {
	static const char *const virtueNames[] = {
		"Blue", "Yellow", "Red",
		"Green", "Orange", "Purple",
		"White", "Black"
	};

	if (virtue < VIRT_MAX)
		return virtueNames[virtue - VIRT_HONESTY];
	else
		return "???";
}

const char *getItemName(Item item) {
	switch (item) {
	case ITEM_SKULL:
		return "Skull";
	case ITEM_CANDLE:
		return "Candle";
	case ITEM_BOOK:
		return "Book";
	case ITEM_BELL:
		return "Bell";
	case ITEM_KEY_C:
		return "Courage";
	case ITEM_KEY_L:
		return "Love";
	case ITEM_KEY_T:
		return "Truth";
	case ITEM_HORN:
		return "Horn";
	case ITEM_WHEEL:
		return "Wheel";
	default:
		return "???";
	}
}

const char *getDirectionName(Direction dir) {
	static const char *const directionNames[] = {
		"West", "North", "East", "South"
	};

	if (dir >= DIR_WEST && dir <= DIR_SOUTH)
		return directionNames[dir - DIR_WEST];
	else
		return "???";
}

} // End of namespace Ultima4
} // End of namespace Ultima
