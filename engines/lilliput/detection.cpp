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

#include "base/plugins.h"
#include "engines/advancedDetector.h"
#include "common/textconsole.h"

#include "lilliput/detection.h"

namespace Lilliput {

static const PlainGameDescriptor lilliputGames[] = {
	// Games
	{"robin", "Adventures of Robin Hood"},
	{"rome", "Rome: Pathway to Power"},
	{0, 0}
};

static const LilliputGameDescription gameDescriptions[] = {

	// Robin Hood English
	{
		{
			"robin", 0,
			AD_ENTRY2s("erules.prg", "92aaf84693a8948497ad57864fa31c2a", 71010,
					   "isomap.dta", "bad97eae03a4db3e99565e39b0b3c06a", 16384),
			Common::EN_ANY,
			Common::kPlatformDOS,
			ADGF_UNSTABLE,
			GUIO0()
		},
		kGameTypeRobin
	},
	// Robin Hood French
	{
		{
			"robin", 0,
			AD_ENTRY2s("frules.prg", "cf076c5ebfe8b3571e74a6a46d79426f", 76660,
					   "isomap.dta", "bad97eae03a4db3e99565e39b0b3c06a", 16384),
			Common::FR_FRA,
			Common::kPlatformDOS,
			ADGF_UNSTABLE,
			GUIO0()
		},
		kGameTypeRobin
	},
	// Robin Hood German
	{
		{
			"robin", 0,
			AD_ENTRY2s("grules.prg", "b53b7353dc1e841b206a64851e7bc58c", 78050,
					   "isomap.dta", "bad97eae03a4db3e99565e39b0b3c06a", 16384),
			Common::DE_DEU,
			Common::kPlatformDOS,
			ADGF_UNSTABLE,
			GUIO0()
		},
		kGameTypeRobin
	},
	// Robin Hood Italian
	{
		{
			"robin", 0,
			AD_ENTRY2s("irules.prg", "4d69ed3cda1e1d73585905517ea705d1", 75654,
					   "isomap.dta", "bad97eae03a4db3e99565e39b0b3c06a", 16384),
			Common::IT_ITA,
			Common::kPlatformDOS,
			ADGF_UNSTABLE,
			GUIO0()
		},
		kGameTypeRobin
	},
	{AD_TABLE_END_MARKER, kGameTypeNone}
};

class LilliputMetaEngineDetection : public AdvancedMetaEngineDetection {
public:
	LilliputMetaEngineDetection() : AdvancedMetaEngineDetection(gameDescriptions, sizeof(LilliputGameDescription), lilliputGames) {
	}

	const char *getEngineId() const override {
		return "lilliput";
	}

	const char *getName() const override {
		return "Lilliput";
	}

	const char *getOriginalCopyright() const override {
		return "Lilliput (C) S.L.Grand, Brainware, 1991-1992";
	}
};

} // End of namespace Lilliput

REGISTER_PLUGIN_STATIC(LILLIPUT_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, Lilliput::LilliputMetaEngineDetection);
