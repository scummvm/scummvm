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
#include "common/config-manager.h"
#include "common/file.h"

#include "pegasus/detection.h"

static const PlainGameDescriptor pegasusGames[] = {
	{"pegasus", "The Journeyman Project: Pegasus Prime"},
	{0, 0}
};


namespace Pegasus {

static const PegasusGameDescription gameDescriptions[] = {
	{
		{
			"pegasus",
			"",
			AD_ENTRY1s("JMP PP Resources", "d13a602d2498010d720a6534f097f88b", 2009943),
			Common::EN_ANY,
			Common::kPlatformMacintosh,
			ADGF_MACRESFORK,
			GUIO1(GUIO_NOMIDI)
		},
	},

	{
		{
			"pegasus",
			"Demo",
			AD_ENTRY1s("JMP PP Resources", "d13a602d2498010d720a6534f097f88b", 360129),
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
			AD_ENTRY1s("JMP PP Resources", "d0fcda50dc75c7a81ae314e6a813f4d2", 93495),
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

	const char *getEngineId() const override {
		return "pegasus";
	}

	const char *getName() const override {
		return "The Journeyman Project: Pegasus Prime";
	}

	const char *getOriginalCopyright() const override {
		return "The Journeyman Project: Pegasus Prime (C) Presto Studios";
	}
};

REGISTER_PLUGIN_STATIC(PEGASUS_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, PegasusMetaEngineDetection);
