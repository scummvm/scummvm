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

#include "engines/advancedDetector.h"

#include "prince/detection.h"

static const PlainGameDescriptor princeGames[] = {
	{"prince", "The Prince and the Coward"},
	{0, 0}
};

namespace Prince {
static const PrinceGameDescription gameDescriptions[] = {
	{
		{
			"prince",
			"Galador: Der Fluch des Prinzen",
			AD_ENTRY1s("databank.ptc", "5fa03833177331214ec1354761b1d2ee", 3565031),
			Common::DE_DEU,
			Common::kPlatformWindows,
			ADGF_USEEXTRAASTITLE | ADGF_DROPPLATFORM,
			GUIO1(GUIO_NONE)
		},
		kPrinceDataDE
	},
	{
		{
			"prince",
			"Ksiaze i Tchorz",
			AD_ENTRY1s("databank.ptc", "48ec9806bda9d152acbea8ce31c93c49", 3435298),
			Common::PL_POL,
			Common::kPlatformWindows,
			ADGF_USEEXTRAASTITLE | ADGF_DROPPLATFORM,
			GUIO1(GUIO_NONE)
		},
		kPrinceDataPL
	},
	{
		{
			"prince",
			"",
			AD_ENTRY1s("talktxt.dat", "02bb2372f19aca3c65896ed81b2cefb3", 125702),
			Common::RU_RUS,
			Common::kPlatformWindows,
			GF_EXTRACTED | ADGF_DROPPLATFORM,
			GUIO1(GUIO_NONE)
		},
		kPrinceDataDE
	},
	{
		{
			"prince",
			"",
			AD_ENTRY1s("databank.ptc", "a67b55730f3d7064921bd2a59e1063a3", 3892982),
			Common::RU_RUS,
			Common::kPlatformWindows,
			GF_NOVOICES | ADGF_DROPPLATFORM,
			GUIO1(GUIO_NONE)
		},
		kPrinceDataDE
	},
    {
        {
            "prince",
            "",
            AD_ENTRY1s("databank.ptc", "eb702d16e88c8c41f963d449287c8023", 3730152),
            Common::RU_RUS,
            Common::kPlatformWindows,
            GF_RUSPROJEDITION | ADGF_USEEXTRAASTITLE | ADGF_DROPPLATFORM,
            GUIO1(GUIO_NONE)
        },
        kPrinceDataDE
    },
	{
		{
			"prince",
			"w/translation",
			AD_ENTRY2s("databank.ptc", "5fa03833177331214ec1354761b1d2ee", 3565031,
					   "prince_translation.dat", 0, -1),
			Common::EN_ANY,
			Common::kPlatformWindows,
			GF_TRANSLATED | ADGF_DROPPLATFORM,
			GUIO1(GUIO_NONE)
		},
		kPrinceDataDE
	},
	{
		{
			"prince",
			"w/translation",
			AD_ENTRY2s("databank.ptc", "48ec9806bda9d152acbea8ce31c93c49", 3435298,
					   "prince_translation.dat", 0, -1),
			Common::EN_ANY,
			Common::kPlatformWindows,
			GF_TRANSLATED | ADGF_DROPPLATFORM,
			GUIO1(GUIO_NONE)
		},
		kPrinceDataPL
	},
	{ AD_TABLE_END_MARKER, kPrinceDataUNK }
};

} // End of namespace Prince

const static char *directoryGlobs[] = {
	"all",
	0
};

class PrinceMetaEngineDetection : public AdvancedMetaEngineDetection {
public:
	PrinceMetaEngineDetection() : AdvancedMetaEngineDetection(Prince::gameDescriptions, sizeof(Prince::PrinceGameDescription), princeGames) {
		_maxScanDepth = 2;
		_directoryGlobs = directoryGlobs;
	}

	const char *getEngineId() const override {
		return "prince";
	}

	const char *getName() const override {
		return "The Prince and the Coward";
	}

	const char *getOriginalCopyright() const override {
		return "The Prince and the Coward (C) 1996-97 Metropolis";
	}
};

REGISTER_PLUGIN_STATIC(PRINCE_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, PrinceMetaEngineDetection);
