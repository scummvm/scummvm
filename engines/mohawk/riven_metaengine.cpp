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

#include "mohawk/riven_metaengine.h"
#include "mohawk/riven_graphics_detection_enums.h"

void Mohawk::MohawkMetaEngine_Riven::registerDefaultSettings() {
	ConfMan.registerDefault("zip_mode", false);
	ConfMan.registerDefault("water_effects", true);
	ConfMan.registerDefault("transition_mode", kRivenTransitionModeFastest);
}

const Mohawk::RivenLanguage *Mohawk::MohawkMetaEngine_Riven::listLanguages() {
	static const RivenLanguage languages[] = {
	    { Common::EN_ANY,   "english"  },
	    { Common::FR_FRA,   "french"   },
	    { Common::DE_DEU,   "german"   },
	    { Common::IT_ITA,   "italian"  },
	    { Common::JA_JPN,   "japanese" },
	    { Common::PL_POL,   "polish"   },
	    { Common::RU_RUS,   "russian"  },
	    { Common::ES_ESP,   "spanish"  },
	    { Common::UNK_LANG, nullptr    }
	};
	return languages;
}
