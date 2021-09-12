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

#include "base/plugins.h"
#include "engines/advancedDetector.h"

namespace Chamber {
static const PlainGameDescriptor ChamberGames[] = {
	{"chamber", "Chamber of the Sci-Mutant Priestess"},
	{ 0, 0 }
};


static const ADGameDescription gameDescriptions[] = {
	{
		"chamber",
		"",
		AD_ENTRY1s("desce.bin", "d6b2b07bbb6b6d5a292c17536ad7dd44", 10419),
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_UNSTABLE,
		GUIO0()
	},

	AD_TABLE_END_MARKER
};
} // End of namespace Chamber

class ChamberMetaEngineDetection : public AdvancedMetaEngineDetection {
public:
	ChamberMetaEngineDetection() : AdvancedMetaEngineDetection(Chamber::gameDescriptions, sizeof(ADGameDescription), Chamber::ChamberGames) {
	}

	const char *getName() const override {
		return "chamber";
	}

	const char *getEngineName() const override {
		return "chamber";
	}

	const char *getOriginalCopyright() const override {
		return "Chamber (C) 1989 ERE Informatique";
	}
};

REGISTER_PLUGIN_STATIC(CHAMBER_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, ChamberMetaEngineDetection);
