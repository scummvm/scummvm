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

#include "engines/advancedDetector.h"
#include "engines/agds/detection.h"

namespace AGDS {

static const ADGameDescription gameDescriptions[] = {

	// Black Mirror
	{
		"black-mirror",
		0,
		AD_ENTRY2s(
			"gfx1.grp", "6665ce103cf12a362fd55f863d1ec9e6", 907820240,
			"data.adb", "3b2d85f16e24ac81c4ede7a1da55f786", 2169482),
		Common::EN_USA,
		Common::kPlatformWindows,
		ADGF_DROPPLATFORM,
		GUIO1(GUIO_NONE)},
	{"black-mirror",
	 "CD version protected with StarForce",
	 AD_ENTRY2s(
		 "gfx1.grp", "6665ce103cf12a362fd55f863d1ec9e6", 907820240,
		 "data.adb", "98d6a1e673e1d3f21be8c89fff25bc4a", 2169482),
	 Common::EN_USA,
	 Common::kPlatformWindows,
	 ADGF_DROPPLATFORM | ADGF_CD | ADGF_UNSUPPORTED,
	 GUIO1(GUIO_NONE)},
	{"black-mirror",
	 0,
	 AD_ENTRY2s(
		 "gfx1.grp", "a5f84365d1e15a8403237fa3ad339f86", 850840170,
		 "data.adb", "5f98eabecf94569be7046d3813edc49a", 2169111),
	 Common::EN_GRB,
	 Common::kPlatformWindows,
	 ADGF_DROPPLATFORM | ADGF_CD,
	 GUIO1(GUIO_NONE)},
	{"black-mirror",
	 0,
	 AD_ENTRY2s(
		 "gfx1.grp", "652f931f02c5a79fb9bcbe32abafbdf7", 907732355,
		 "data.adb", "d8706b17fb89d58d4dba094a73e5490a", 2152794),
	 Common::RU_RUS,
	 Common::kPlatformWindows,
	 ADGF_DROPPLATFORM,
	 GUIO1(GUIO_NONE)},

	// NiBiRu
	{
		"nibiru",
		0,
		AD_ENTRY2s(
			"gfx1.grp", "40a7a88f77c35305b6aba0329ed8a9ac", 381768750,
			"data.adb", "40a7a88f77c35305b6aba0329ed8a9ac", 1391440),
		Common::EN_USA,
		Common::kPlatformWindows,
		ADGF_DROPPLATFORM | AGDS_V2 | ADGF_UNSTABLE,
		GUIO1(GUIO_NONE)},
	{"nibiru",
	 0,
	 AD_ENTRY2s(
		 "gfx1.grp", "c8e711bc01b16cd82849cbd996d02642", 381768360,
		 "data.adb", "40a7a88f77c35305b6aba0329ed8a9ac", 1391440),
	 Common::RU_RUS,
	 Common::kPlatformWindows,
	 ADGF_DROPPLATFORM | AGDS_V2 | ADGF_UNSTABLE,
	 GUIO1(GUIO_NONE)},

	AD_TABLE_END_MARKER};

} // End of namespace AGDS
