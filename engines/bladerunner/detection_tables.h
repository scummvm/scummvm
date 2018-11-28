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

#ifndef BLADERUNNER_DETECTION_TABLES_H
#define BLADERUNNER_DETECTION_TABLES_H

#include "engines/advancedDetector.h"

namespace BladeRunner {

static const ADGameDescription gameDescriptions[] = {
	// BladeRunner
	{
		"bladerunner",
		0,
		AD_ENTRY1s("STARTUP.MIX", "5643b53306ca7764cf1ec7b79c9630a3", 2312374),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO0()
	},

	// BladeRunner (German)
	{
		"bladerunner",
		0,
		AD_ENTRY1s("STARTUP.MIX", "57d674ed860148a530b7f4957cbe65ec", 2314301),
		Common::DE_DEU,
		Common::kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO0()
	},

	// BladeRunner (French) - Bug #9722
	{
		"bladerunner",
		0,
		AD_ENTRY1s("STARTUP.MIX", "39d1901df50935d58aee252707134952", 2314526),
		Common::FR_FRA,
		Common::kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO0()
	},

	// BladeRunner (Italian)
	{
		"bladerunner",
		0,
		AD_ENTRY1s("STARTUP.MIX", "c7ceb9c691223d25e78516aa519ff504", 2314461),
		Common::IT_ITA,
		Common::kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO0()
	},

	// BladeRunner (Russian)
	{
		"bladerunner",
		0,
		AD_ENTRY1s("STARTUP.MIX", "c198b54a5366b88b1734bbca21d3b192", 2678672),
		Common::RU_RUS,
		Common::kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO0()
	},

	// BladeRunner (Spanish)
	{
		"bladerunner",
		0,
		AD_ENTRY1s("STARTUP.MIX", "54cad53da9e4ae03a85648834ac6765d", 2312976),
		Common::ES_ESP,
		Common::kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO0()
	},

	AD_TABLE_END_MARKER
};

} // End of namespace BladeRunner

#endif
