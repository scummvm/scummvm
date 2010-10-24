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
 * $URL$
 * $Id$
 *
 */

#include "lastexpress/lastexpress.h"

namespace LastExpress {

static const PlainGameDescriptor lastExpressGames[] = {
	// Games
	{"lastexpress", "The Last Express"},
	{0, 0}
};

static const ADGameDescription gameDescriptions[] = {

	// The Last Express (English)
	{
		"lastexpress",
		"",
		{
			{"HD.HPF",  0, "2d331459e0e68cf277ef4e4043750413", 29865984},
			{"CD1.HPF", 0, "8c86db47304033fcff32c69fddd5a920", 525522944},
			{"CD2.HPF", 0, "58aa26e782d10ec5d2231e539d2fe6a2", 669581312},
			{"CD3.HPF", 0, "00554fbf78a2ad391d98578fbbbe1c48", 641128448},
		},
		Common::EN_ANY,
		Common::kPlatformUnknown,
		ADGF_NO_FLAGS,
		Common::GUIO_NONE
	},

	// The Last Express (English) - UK Broderbund Release
	{
		"lastexpress",
		"",
		{
			{"HD.HPF",  0, "2d331459e0e68cf277ef4e4043750413", 29865984},
			{"CD1.HPF", 0, "8c86db47304033fcff32c69fddd5a920", 525522944},
			{"CD2.HPF", 0, "2672348691e1ae22d37d9f46f3683a07", 669509632},
			{"CD3.HPF", 0, "33f5e35f51063cb90f6bed9974475aa6", 641056768},
		},
		Common::EN_ANY,
		Common::kPlatformUnknown,
		ADGF_NO_FLAGS,
		Common::GUIO_NONE
	},

	// The Last Express (English) - Interplay Release
	{
		"lastexpress",
		"",
		{
			{"HD.HPF",  0, "bcc32d977f92bb52c060a0b4e8589cac", 30715904},
			{"CD1.HPF", 0, "8c86db47304033fcff32c69fddd5a920", 525522944},
			{"CD2.HPF", 0, "58aa26e782d10ec5d2231e539d2fe6a2", 669581312},
			{"CD3.HPF", 0, "00554fbf78a2ad391d98578fbbbe1c48", 641128448},
		},
		Common::EN_ANY,
		Common::kPlatformUnknown,
		ADGF_NO_FLAGS,
		Common::GUIO_NONE
	},

	// The Last Express (French)
	{
		"lastexpress",
		"",
		{
			{"HD.HPF",  0, "c14c6d685d9bf8705d9f659062e6c5c2", 29505536},
			{"CD1.HPF", 0, "b4277b22bc5cd6ad3b00c2ec04d4645d", 522924032},
			{"CD2.HPF", 0, "8c9610aa4cb707ab51f61c30feb22c1a", 665710592},
			{"CD3.HPF", 0, "411c1bab57b3e8da4fb359c5b40ef5d7", 640884736},
		},
		Common::FR_FRA,
		Common::kPlatformUnknown,
		ADGF_NO_FLAGS,
		Common::GUIO_NONE
	},

	// The Last Express (German)
	{
		"lastexpress",
		"",
		{
			{"HD.HPF", 0, "7cdd70fc0b1555785f1e9e8d371ea85c", 31301632},
			{"CD1.HPF", 0, "6d74cc861d172466bc745ff8bf0e59c5", 522971136},
			{"CD2.HPF", 0, "b71ac9391de415807c74ff078f4fab22", 655702016},
			{"CD3.HPF", 0, "ee55d4310546dd2a38560b096d1c2771", 641144832},
		},
		Common::DE_DEU,
		Common::kPlatformUnknown,
		ADGF_NO_FLAGS,
		Common::GUIO_NONE
	},

	// The Last Express (Spanish)
	{
		"lastexpress",
		"",
		{
			{"HD.HPF",  0, "46bed8832f06cf7160883a2aae2d667f", 29657088},
			{"CD1.HPF", 0, "367a3a8581f6f88ddc51af7cde105ba9", 519927808},
			{"CD2.HPF", 0, "af5566df3000472852ec182c9ec57797", 662210560},
			{"CD3.HPF", 0, "0d1901662f4d063a5c250c9fbf64b771", 639504384},
		},
		Common::ES_ESP,
		Common::kPlatformUnknown,
		ADGF_NO_FLAGS,
		Common::GUIO_NONE
	},

	// The Last Express (Demo)
	{
		"lastexpress",
		"Demo",
		{
			{"Demo.HPF",  0, "baf3b1f64155d34872896e61c3d3cb78", 58191872},
		},
		Common::EN_ANY,
		Common::kPlatformUnknown,
		ADGF_DEMO,
		Common::GUIO_NONE
	},
	AD_TABLE_END_MARKER
};

static const ADParams detectionParams = {
	// Pointer to ADGameDescription or its superset structure
	(const byte *)gameDescriptions,
	// Size of that superset structure
	sizeof(ADGameDescription),
	// Number of bytes to compute MD5 sum for
	5000,
	// List of all engine targets
	lastExpressGames,
	// Structure for autoupgrading obsolete targets
	0,
	// Name of single gameid (optional)
	"lastexpress",
	// List of files for file-based fallback detection (optional)
	0,
	// Flags
	0,
	// Additional GUI options (for every game}
	Common::GUIO_NOSUBTITLES | Common::GUIO_NOSFX,
	// Maximum directory depth
	1,
	// List of directory globs
	0
};


class LastExpressMetaEngine : public AdvancedMetaEngine {
public:
	LastExpressMetaEngine() : AdvancedMetaEngine(detectionParams) {}

	const char *getName() const {
		return "LastExpress Engine";
	}

	const char *getOriginalCopyright() const {
		return "LastExpress Engine (C) 1997 Smoking Car Productions";
	}

	bool createInstance(OSystem *syst, Engine **engine, const ADGameDescription *gd) const;
};

bool LastExpressMetaEngine::createInstance(OSystem *syst, Engine **engine, const ADGameDescription *gd) const {
	if (gd) {
		*engine = new LastExpressEngine(syst, (const ADGameDescription *)gd);
	}
	return gd != 0;
}

} // End of namespace LastExpress

#if PLUGIN_ENABLED_DYNAMIC(LASTEXPRESS)
	REGISTER_PLUGIN_DYNAMIC(LASTEXPRESS, PLUGIN_TYPE_ENGINE, LastExpress::LastExpressMetaEngine);
#else
	REGISTER_PLUGIN_STATIC(LASTEXPRESS, PLUGIN_TYPE_ENGINE, LastExpress::LastExpressMetaEngine);
#endif
