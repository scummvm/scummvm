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
#include "common/file.h"

#include "ngi/detection.h"

static const PlainGameDescriptor ngiGames[] = {
	{"ngi", 		"Nikita Game Interface game"},
	{"fullpipe",	"Full Pipe"},
	{"mdream",		"Magic Dream"},
	{0, 0}
};

namespace NGI {

static const NGIGameDescription gameDescriptions[] = {

	// Full Pipe Russian version
	{
		{
			"mdream",
			0,
			AD_ENTRY1s("0001.nl", "079d02921a938ec9740598316450d526", 11848423),
			Common::RU_RUS,
			Common::kPlatformWindows,
			ADGF_DROPPLATFORM,
			GUIO1(GUIO_NOMIDI)
		},
		GID_MDREAM
	},

	// Full Pipe Russian version
	{
		{
			"fullpipe",
			0,
			AD_ENTRY1s("4620.sc2", "a1a8f3ed731b0dfea43beaa3016fdc71", 554),
			Common::RU_RUS,
			Common::kPlatformWindows,
			ADGF_DROPPLATFORM,
			GUIO1(GUIO_NOMIDI)
		},
		GID_FULLPIPE
	},

	// Full Pipe German version
	{
		{
			"fullpipe",
			0,
			AD_ENTRY1s("4620.sc2", "e4f24ffe4dc84cafc648b951e66c1fb3", 554),
			Common::DE_DEU,
			Common::kPlatformWindows,
			ADGF_DROPPLATFORM,
			GUIO1(GUIO_NOMIDI)
		},
		GID_FULLPIPE
	},

	// Full Pipe Estonian version
	{
		{
			"fullpipe",
			0,
			AD_ENTRY1s("4620.sc2", "571f6b4b68b02003e35bc12c1a1d3fe3", 466),
			Common::ET_EST,
			Common::kPlatformWindows,
			ADGF_DROPPLATFORM,
			GUIO1(GUIO_NOMIDI)
		},
		GID_FULLPIPE
	},

	// Full Pipe English version
	{
		{
			"fullpipe",
			0,
			AD_ENTRY1s("4620.sc2", "bffea807345fece14089768fc141af83", 510),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_DROPPLATFORM,
			GUIO1(GUIO_NOMIDI)
		},
		GID_FULLPIPE
	},

	// Full Pipe Russian Demo version
	{
		{
			"fullpipe",
			0,
			AD_ENTRY1s("4620.sc2", "a0c71b47fc35a5e163fcd8d0972639bb", 70),
			Common::RU_RUS,
			Common::kPlatformWindows,
			ADGF_DROPPLATFORM | ADGF_DEMO,
			GUIO1(GUIO_NOMIDI)
		},
		GID_FULLPIPE
	},

	// Full Pipe German Demo version
	{
		{
			"fullpipe",
			0,
			AD_ENTRY1s("4620.sc2", "e5e98df537e56b39c33ae1d5c90976fe", 510),
			Common::DE_DEU,
			Common::kPlatformWindows,
			ADGF_DROPPLATFORM | ADGF_DEMO,
			GUIO1(GUIO_NOMIDI)
		},
		GID_FULLPIPE
	},

	{ AD_TABLE_END_MARKER, 0 }
};

} // End of namespace NGI

class NGIMetaEngineDetection : public AdvancedMetaEngineDetection {
public:
	NGIMetaEngineDetection() : AdvancedMetaEngineDetection(NGI::gameDescriptions, sizeof(NGI::NGIGameDescription), ngiGames) {
	}

	const char *getEngineId() const override {
		return "ngi";
	}

	const char *getName() const override {
		return "Nikita Game Interface";
	}

	const char *getOriginalCopyright() const override {
		return "Full Pipe (C) Pipe Studio";
	}
};

REGISTER_PLUGIN_STATIC(NGI_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, NGIMetaEngineDetection);
