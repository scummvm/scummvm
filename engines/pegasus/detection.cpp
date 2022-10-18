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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "base/plugins.h"

#include "engines/advancedDetector.h"
#include "common/config-manager.h"
#include "common/file.h"

#include "pegasus/detection.h"

static const PlainGameDescriptor pegasusGames[] = {
	{"pegasus", "The Journeyman Project: Pegasus Prime"},
	{nullptr, nullptr}
};


namespace Pegasus {

static const PegasusGameDescription gameDescriptions[] = {
	{
		{
			"pegasus",
			"",
			AD_ENTRY1s("JMP PP Resources", "d13a602d2498010d720a6534f097f88b", 2009687),
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			ADGF_MACRESFORK,
			GUIO1(GUIO_NOMIDI)
		},
	},

	{
		{
			"pegasus",
			"DVD",
			AD_ENTRY1s("JMP PP Resources", "d13a602d2498010d720a6534f097f88b", 2075081),
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			ADGF_MACRESFORK|GF_DVD,
			GUIO0()
		},
	},

	{
		{	"pegasus",
			"DVD",
			AD_ENTRY1s("JMP PP Resources", "d13a602d2498010d720a6534f097f88b", 2075081),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_MACRESFORK|GF_DVD,
			GUIO0()
		},
	},

	{
		{
			"pegasus",
			"DVD",
			AD_ENTRY1s("JMP PP Resources", "d13a602d2498010d720a6534f097f88b", 2075081),
			Common::EN_ANY,
			Common::kPlatformLinux,
			ADGF_MACRESFORK|GF_DVD,
			GUIO0()
		},
	},

	// The Next Generation Disc #31 - July 1997
	// Mac Addict #12 - August 1997
	// Publisher is Bandai Digital Entertainment Corporation
	// File modified date is Mar 29 1997 (v1.0)
	{
		{
			"pegasus",
			"v1.0 Demo",
			AD_ENTRY1s("JMP PP Resources", "d13a602d2498010d720a6534f097f88b", 365329),
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			ADGF_MACRESFORK | ADGF_DEMO,
			GUIO2(GUIO_NOLAUNCHLOAD, GUIO_NOMIDI)
		},
	},

	// Official distribution from Presto website
	// http://thejourneymanproject.com/presto/journeymanprime/demos.html
	// File modified date is date is Dec 15 2011
	{
		{
			"pegasus",
			"Demo",
			AD_ENTRY1s("JMP PP Resources", "d13a602d2498010d720a6534f097f88b", 359873),
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			ADGF_MACRESFORK | ADGF_DEMO,
			GUIO2(GUIO_NOLAUNCHLOAD, GUIO_NOMIDI)
		},
	},

	{
		{
			"pegasus",
			"DVD Demo",
			AD_ENTRY1s("JMP PP Resources", "d0fcda50dc75c7a81ae314e6a813f4d2", 93495), // FIXMEMD5
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			ADGF_MACRESFORK | ADGF_DEMO | GF_DVD,
			GUIO2(GUIO_NOLAUNCHLOAD, GUIO_NOMIDI)
		},
	},

	{
		{
			"pegasus",
			"DVD Demo",
			AD_ENTRY1s("JMP PP Resources", "d0fcda50dc75c7a81ae314e6a813f4d2", 93495),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_MACRESFORK | ADGF_DEMO | GF_DVD,
			GUIO2(GUIO_NOLAUNCHLOAD, GUIO_NOMIDI)
		},
	},

	{ AD_TABLE_END_MARKER }
};

} // End of namespace Pegasus


class PegasusMetaEngineDetection : public AdvancedMetaEngineDetection {
public:
	PegasusMetaEngineDetection() : AdvancedMetaEngineDetection(Pegasus::gameDescriptions, sizeof(Pegasus::PegasusGameDescription), pegasusGames) {
	}

	const char *getName() const override {
		return "pegasus";
	}

	const char *getEngineName() const override {
		return "The Journeyman Project: Pegasus Prime";
	}

	const char *getOriginalCopyright() const override {
		return "The Journeyman Project: Pegasus Prime (C) Presto Studios";
	}
};

REGISTER_PLUGIN_STATIC(PEGASUS_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, PegasusMetaEngineDetection);
