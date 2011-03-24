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
	int flags;
	GrimGameType gameType;
};

static const PlainGameDescriptor grimGames[] = {
	{"grim", "Grim Fandango"},
	{"monkey4", "Escape From Monkey Island"},
	{0, 0}
};

using Common::GUIO_NONE;

static const GrimGameDescription gameDescriptions[] = {
	{
		// Grim Fandango English version
		{
			"grim",
			"",
			AD_ENTRY1s("grim.tab", "cfb333d6aec260c905151b6b98ef71e8", 362212),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO_NONE
		},
		0,
		GType_GRIM
	},
	{
		// Grim Fandango German version
		{
			"grim",
			"",
			AD_ENTRY1s("grim.tab", "464138caf47e580cbb237dee10674b16", 398592),
			Common::DE_DEU,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO_NONE
		},
		0,
		GType_GRIM
	},
	{
		// Grim Fandango Spanish version
		{
			"grim",
			"",
			AD_ENTRY1s("grim.tab", "b1460cd029f13718f7f62c2403e047ec", 372709),
			Common::ES_ESP,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO_NONE
		},
		0,
		GType_GRIM
	},
	{
		// Grim Fandango Italian version
		{
			"grim",
			"",
			AD_ENTRY1s("grim.tab", "2d99c796b7a4e5c421cae49dc29dab6c", 369071),
			Common::IT_ITA,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO_NONE
		},
		0,
		GType_GRIM
	},
	{
		// Grim Fandango English demo version
		{
			"grim",
			"Demo",
			AD_ENTRY1s("gfdemo01.lab", "755cdac083f7f751bec7506402278f1a", 29489930),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_DEMO,
			GUIO_NONE
		},
		GF_DEMO,
		GType_GRIM
	},
	{
		// Escape from Monkey Island English PS2
		{
			"monkey4",
			"",
			AD_ENTRY1s("local.m4b", "00c4eb73f6b6607ba3d4e8d3f956b37b", 3804862),
			Common::EN_ANY,
			Common::kPlatformPS2,
			ADGF_NO_FLAGS,
			GUIO_NONE
		},
		0,
		GType_MONKEY4
	},
	{
		// Escape from Monkey Island demo
		{
			"monkey4",
			"Demo",
			AD_ENTRY1s("magdemo.lab", "9e7eaa1b9317ff47d5deeda0b2c42ce3", 19826116),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_DEMO,
			GUIO_NONE
		},
		GF_DEMO,
		GType_MONKEY4
	},


	{ AD_TABLE_END_MARKER, 0, GType_GRIM }
};

static const GrimGameDescription fallbackGameDescriptions[] = {
	{
		{
			"grim",
			"",
			{{0, 0, 0, 0}},
			Common::UNK_LANG,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO_NONE
		},
		0,
		GType_GRIM
	}
};

static const ADFileBasedFallback grimFallback[] = {
	{&fallbackGameDescriptions[0], {"grim.tab"}},
	{0, {0}}
};

static const ADObsoleteGameID obsoleteGameIDsTable[] = {
	{"grimdemo", "grim", Common::kPlatformWindows},
	{0, 0, Common::kPlatformUnknown}
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
	obsoleteGameIDsTable,
	// Name of single gameid (optional)
	0,
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

	if (gd)
		*engine = new GrimEngine(syst, gd->flags, gd->gameType);

	return gd != 0;
}

} // End of namespace Grim

#if PLUGIN_ENABLED_DYNAMIC(GRIM)
	REGISTER_PLUGIN_DYNAMIC(GRIM, PLUGIN_TYPE_ENGINE, Grim::GrimMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(GRIM, PLUGIN_TYPE_ENGINE, Grim::GrimMetaEngine);
#endif
