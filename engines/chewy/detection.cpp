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

#include "chewy/detection.h"

static const PlainGameDescriptor chewyGames[] = {
	{"chewy", "Chewy: Esc from F5"},
	{0, 0}
};

static const char *directoryGlobs[] = {
	"txt",
	0
};

namespace Chewy {

static const ChewyGameDescription gameDescriptions[] = {

	{
		{
			"chewy",
			0,
			AD_ENTRY1s("atds.tap", "e6050c144dd4f23d79ea4f89a8ef306e", 218857),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOMIDI)
		},
	},

	{
		// Chewy - ESC von F5 - German
		// Master version 1.1 (CHEWY.EXE - offset 0x8AB28)
		// The source CD-ROM has the Matrix code SONOPRESS R-7885 B
		// The disc contains several demos and files from 1996
		// Provided by lotharsm
		{
			"chewy",
			0,
			AD_ENTRY1s("atds.tap", "c117e884cc5b4bbe50ae1217d13916c4", 231071),
			Common::DE_DEU,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOMIDI)
		},
	},

	{
		// Chewy - ESC von F5 - German
		// Master version 1.0 (CHEWY.EXE - offset 0x8AB10)
		// The source CD-ROM has the Matrix code SONOPRESS M-2742 A
		// CD-ROM has the label "CHEWY_V1_0"
		// Provided by lotharsm
		{
			"chewy",
			0,
			AD_ENTRY1s("atds.tap", "e22f97761c0e7772ec99660f2277b1a4", 231001),
			Common::DE_DEU,
			Common::kPlatformDOS,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOMIDI)
		},
	},

	{ AD_TABLE_END_MARKER }
};

} // End of namespace Chewy

class ChewyMetaEngineDetection : public AdvancedMetaEngineDetection {
public:
	ChewyMetaEngineDetection() : AdvancedMetaEngineDetection(Chewy::gameDescriptions, sizeof(Chewy::ChewyGameDescription), chewyGames) {
		_maxScanDepth = 2;
		_directoryGlobs = directoryGlobs;
	}

	const char *getEngineId() const override {
		return "chewy";
	}

	const char *getName() const override {
		return "Chewy: Esc from F5";
	}

	const char *getOriginalCopyright() const override {
		return "Chewy: Esc from F5 (C) 1995 New Generation Software";
	}
};

REGISTER_PLUGIN_STATIC(CHEWY_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, ChewyMetaEngineDetection);
