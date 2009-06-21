/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 * $URL$
 * $Id$
 *
 */

#include "engines/advancedDetector.h"

#include "engines/grim/grim.h"

namespace Grim {

struct GrimGameDescription {
	ADGameDescription desc;
};

static const PlainGameDescriptor grimGames[] = {
	{"grim", "Grim Fandango"},
	{"monkey", "Escape From Monkey Island"},
	{0, 0}
};

using Common::GUIO_NONE;

static const GrimGameDescription gameDescriptions[] = {
	{
		// Grim Fandago English version
		{
			"grim",
			0,
			AD_ENTRY1s("grim.tab", "cfb333d6aec260c905151b6b98ef71e8", 362212),
			Common::EN_ANY,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NONE
		},
	},
	{
		// Grim Fandago German version
		{
			"grim",
			0,
			AD_ENTRY1s("grim.tab", "464138caf47e580cbb237dee10674b16", 398592),
			Common::DE_DEU,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NONE
		},
	},
	{
		// Grim Fandago Spanish version
		{
			"grim",
			0,
			AD_ENTRY1s("grim.tab", "b1460cd029f13718f7f62c2403e047ec", 372709),
			Common::ES_ESP,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NONE
		},
	},
	{
		// Grim Fandago Italian version
		{
			"grim",
			0,
			AD_ENTRY1s("grim.tab", "2d99c796b7a4e5c421cae49dc29dab6c", 369071),
			Common::IT_ITA,
			Common::kPlatformPC,
			ADGF_NO_FLAGS,
			GUIO_NONE
		},
	},
	{
		// Grim Fandago English demo version
		{
			"grim",
			0,
			AD_ENTRY1s("gfdemo01.lab", "755cdac083f7f751bec7506402278f1a", 29489930),
			Common::EN_ANY,
			Common::kPlatformPC,
			ADGF_DEMO,
			GUIO_NONE
		},
	},


	{ AD_TABLE_END_MARKER }
};

static const GrimGameDescription fallbackGameDescriptions[] = {
	{{"grim", 0, {{0, 0, 0, 0}}, Common::UNK_LANG, Common::kPlatformPC, ADGF_NO_FLAGS, GUIO_NONE}}
};

static const ADFileBasedFallback grimFallback[] = {
	{&fallbackGameDescriptions[0], {"grim.tab"}},
	{0, {0}}
};

static const ADParams detectionParams = {
	// Pointer to ADGameDescription or its superset structure
	(const byte *)gameDescriptions,
	// Size of that superset structure
	sizeof(GrimGameDescription),
	// Number of bytes to compute MD5 sum for
	5000,
	// List of all engine targets
	grimGames,
	// Structure for autoupgrading obsolete targets
	0,
	// Name of single gameid (optional)
	"grim",
	// List of files for file-based fallback detection (optional)
	grimFallback,
	// Flags
	0,
	// Additional GUI options (for every game}
	Common::GUIO_NOMIDI
};

class GrimMetaEngine : public AdvancedMetaEngine {
public:
	GrimMetaEngine() : AdvancedMetaEngine(detectionParams) {}

	virtual const char *getName() const {
		return "Grim Engine";
	}

	virtual const char *getOriginalCopyright() const {
		return "LucasArts GrimE Games (C) LucasArts";
	}

	virtual bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const;
};

bool GrimMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *desc) const {
	const GrimGameDescription *gd = (const GrimGameDescription *)desc;
	if (gd) {
		*engine = new GrimEngine(syst, gd);
	}
	return gd != 0;
}

} // End of namespace Grim

#if PLUGIN_ENABLED_DYNAMIC(GRIM)
	REGISTER_PLUGIN_DYNAMIC(GRIM, PLUGIN_TYPE_ENGINE, Grim::GrimMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(GRIM, PLUGIN_TYPE_ENGINE, Grim::GrimMetaEngine);
#endif
