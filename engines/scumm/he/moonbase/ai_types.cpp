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

#include "common/textconsole.h"
#include "scumm/scumm.h"
#include "scumm/he/moonbase/ai_types.h"

namespace Scumm {

AIEntity::AIEntity(int id) {
	switch (id) {
	default:
	case BRUTAKAS:
		debugC(DEBUG_MOONBASE_AI, "BRUTAKAS");
		_id = id;
		_nameString = new char[64];
		strcpy(_nameString, "BRUTAKAS");
		_behaviorVariation = AI_VAR_SMALL;
		_targetVariation = AI_VAR_SMALL;
		_angleVariation = AI_VAR_SMALL;
		_powerVariation = AI_VAR_SMALL;
		break;

	case AGI:
		debugC(DEBUG_MOONBASE_AI, "Agi");
		_id = id;
		_nameString = new char[64];
		strcpy(_nameString, "Agi");
		_behaviorVariation = AI_VAR_SMALL;
		_targetVariation = AI_VAR_MEDIUM;
		_angleVariation = AI_VAR_MEDIUM;
		_powerVariation = AI_VAR_LARGE;
		break;

	case EL_GATO:
		debugC(DEBUG_MOONBASE_AI, "El Gato de la Noche");
		_id = id;
		_nameString = new char[64];
		strcpy(_nameString, "El Gato de la Noche");
		_behaviorVariation = AI_VAR_SMALL;
		_targetVariation = AI_VAR_SMALL;
		_angleVariation = AI_VAR_SMALL;
		_powerVariation = AI_VAR_MEDIUM;
		break;

	case PIXELAHT:
		debugC(DEBUG_MOONBASE_AI, "Pixelaht");
		_id = id;
		_nameString = new char[64];
		strcpy(_nameString, "Pixelaht");
		_behaviorVariation = AI_VAR_SMALL;
		_targetVariation = AI_VAR_LARGE;
		_angleVariation = AI_VAR_MEDIUM;
		_powerVariation = AI_VAR_SMALL;
		break;

	case CYBALL:
		debugC(DEBUG_MOONBASE_AI, "cYbaLL");
		_id = id;
		_nameString = new char[64];
		strcpy(_nameString, "cYbaLL");
		_behaviorVariation = AI_VAR_LARGE;
		_targetVariation = AI_VAR_LARGE;
		_angleVariation = AI_VAR_SMALL;
		_powerVariation = AI_VAR_SMALL;
		break;

	case NEEP:
		debugC(DEBUG_MOONBASE_AI, "Neep! Neep!");
		_id = id;
		_nameString = new char[64];
		strcpy(_nameString, "Neep! Neep!");
		_behaviorVariation = AI_VAR_MEDIUM;
		_targetVariation = AI_VAR_SMALL;
		_angleVariation = AI_VAR_SMALL;
		_powerVariation = AI_VAR_LARGE;
		break;

	case WARCUPINE:
		debugC(DEBUG_MOONBASE_AI, "WARcupine");
		_id = id;
		_nameString = new char[64];
		strcpy(_nameString, "WARcupine");
		_behaviorVariation = AI_VAR_SMALL;
		_targetVariation = AI_VAR_SMALL;
		_angleVariation = AI_VAR_LARGE;
		_powerVariation = AI_VAR_MEDIUM;
		break;

	case AONE:
		debugC(DEBUG_MOONBASE_AI, "aone");
		_id = id;
		_nameString = new char[64];
		strcpy(_nameString, "aone");
		_behaviorVariation = AI_VAR_MEDIUM;
		_targetVariation = AI_VAR_MEDIUM;
		_angleVariation = AI_VAR_MEDIUM;
		_powerVariation = AI_VAR_MEDIUM;
		break;

	case SPANDO:
		debugC(DEBUG_MOONBASE_AI, "S p a n d o");
		_id = id;
		_nameString = new char[64];
		strcpy(_nameString, "S p a n d o");
		_behaviorVariation = AI_VAR_LARGE;
		_targetVariation = AI_VAR_LARGE;
		_angleVariation = AI_VAR_SMALL;
		_powerVariation = AI_VAR_SMALL;
		break;

	case ORBNU_LUNATEK:
		debugC(DEBUG_MOONBASE_AI, "Bonur J Lunatek");
		_id = id;
		_nameString = new char[64];
		strcpy(_nameString, "Bonur J Lunatek");
		_behaviorVariation = AI_VAR_HUGE;
		_targetVariation = AI_VAR_HUGE;
		_angleVariation = AI_VAR_HUGE;
		_powerVariation = AI_VAR_HUGE;
		break;

	case CRAWLER_CHUCKER:
		debugC(DEBUG_MOONBASE_AI, "Le Chuckre des Crawlres");
		_id = id;
		_nameString = new char[64];
		strcpy(_nameString, "Le Chuckre des Crawlres");
		_behaviorVariation = AI_VAR_SMALL;
		_targetVariation = AI_VAR_MEDIUM;
		_angleVariation = AI_VAR_MEDIUM;
		_powerVariation = AI_VAR_LARGE;
		break;

	case ENERGY_HOG:
		debugC(DEBUG_MOONBASE_AI, "Energy Hog");
		_id = id;
		_nameString = new char[64];
		strcpy(_nameString, "Energy Hog\n");
		_behaviorVariation = AI_VAR_SMALL;
		_targetVariation = AI_VAR_SMALL;
		_angleVariation = AI_VAR_SMALL;
		_powerVariation = AI_VAR_SMALL;
		break;

	case RANGER:
		debugC(DEBUG_MOONBASE_AI, "Ranger");
		_id = id;
		_nameString = new char[64];
		strcpy(_nameString, "Ranger\n");
		_behaviorVariation = AI_VAR_SMALL;
		_targetVariation = AI_VAR_SMALL;
		_angleVariation = AI_VAR_SMALL;
		_powerVariation = AI_VAR_SMALL;
		break;
	}
}

} // End of namespace Scumm
