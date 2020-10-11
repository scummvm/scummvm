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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "engines/advancedDetector.h"

#include "base/plugins.h"

#include "illusions/detection.h"

static const PlainGameDescriptor illusionsGames[] = {
	{ "bbdou", "Beavis and Butt-head Do U" },
	{ "duckman", "Duckman" },
	{ 0, 0 }
};

namespace Illusions {

static const IllusionsGameDescription gameDescriptions[] = {
	{
		{
			"bbdou",
			0,
			AD_ENTRY1s("000D0001.scr", "d0c846d5dccc5607a482c7dcbdf06973", 601980),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_UNSTABLE | ADGF_DROPPLATFORM,
			GUIO0()
		},
		kGameIdBBDOU
	},

	{
		{
			"duckman",
			0,
			AD_ENTRY1s("duckman.gam", "172c0514f3793041718159cf9cf9935f", 29560832),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_DROPPLATFORM,
			GUIO0()
		},
		kGameIdDuckman
	},

	{
		{
			"duckman",
			"Demo",
			AD_ENTRY1s("duckman.gam", "71d01e3f3d9d4e51cd69f71028745610", 7127040),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_DROPPLATFORM | ADGF_DEMO,
			GUIO0()
		},
		kGameIdDuckman
	},

	{
		{
			"duckman",
			0,
			AD_ENTRY1s("duckman.gam", "64d16922ffb46b746fc2c12a14d75bcc", 29779968),
			Common::DE_DEU,
			Common::kPlatformWindows,
			ADGF_DROPPLATFORM,
			GUIO0()
		},
		kGameIdDuckman
	},

	{
		{
			"duckman",
			0,
			AD_ENTRY1s("duckman.gam", "a65f9b58427b3543882bddf36abe5878", 29564928),
			Common::RU_RUS,
			Common::kPlatformWindows,
			ADGF_DROPPLATFORM,
			GUIO0()
		},
		kGameIdDuckman
	},

	{AD_TABLE_END_MARKER, 0}
};

} // End of namespace Illusions

static const char * const directoryGlobs[] = {
	"resource",
	0
};

class IllusionsMetaEngineDetection : public AdvancedMetaEngineDetection {
public:
	IllusionsMetaEngineDetection() : AdvancedMetaEngineDetection(Illusions::gameDescriptions, sizeof(Illusions::IllusionsGameDescription), illusionsGames) {
		_maxScanDepth = 2;
		_directoryGlobs = directoryGlobs;
	}

	const char *getEngineId() const override {
		return "illusions";
	}

	const char *getName() const override {
		return "Illusions";
	}

	const char *getOriginalCopyright() const override {
		return "(C) The Illusions Gaming Company";
	}
};

REGISTER_PLUGIN_STATIC(ILLUSIONS_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, IllusionsMetaEngineDetection);
