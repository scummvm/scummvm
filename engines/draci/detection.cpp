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
#include "engines/metaengine.h"

static const PlainGameDescriptor draciGames[] = {
	{ "draci", "Draci Historie" },
	{ 0, 0 }
};

namespace Draci {

const ADGameDescription gameDescriptions[] = {
	{
		"draci",
		0,
		AD_ENTRY1s("INIT.DFW", "b890a5aeebaf16af39219cba2416b0a3", 906),
		Common::EN_ANY,
		Common::kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO0()
	},

	{
		"draci",
		0,
		AD_ENTRY1s("INIT.DFW", "9921c8f0045679a8f37eca8d41c5ec02", 906),
		Common::CZ_CZE,
		Common::kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO0()
	},

	{
		"draci",
		0,
		AD_ENTRY1s("INIT.DFW", "76b9b78a8a8809a240acc395df4d0715", 906),
		Common::PL_POL,
		Common::kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO0()
	},

	{
		"draci",
		0,
		AD_ENTRY1s("INIT.DFW", "9a7115b91cdea361bcaff3e046ac7ded", 906),
		Common::DE_DEU,
		Common::kPlatformDOS,
		ADGF_NO_FLAGS,
		GUIO0()
	},

	AD_TABLE_END_MARKER
};

} // End of namespace Draci

class DraciMetaEngineDetection : public AdvancedMetaEngineDetection {
public:
	DraciMetaEngineDetection() : AdvancedMetaEngineDetection(Draci::gameDescriptions, sizeof(ADGameDescription), draciGames) {
	}

	const char *getEngineId() const override {
		return "draci";
	}

	const char *getName() const override {
		return "Draci Historie";
	}

	const char *getOriginalCopyright() const override {
		return "Draci Historie (C) 1995 NoSense";
	}
};

REGISTER_PLUGIN_STATIC(DRACI_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, DraciMetaEngineDetection);
