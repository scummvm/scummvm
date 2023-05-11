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

#ifndef VCRUISE_DETECTION_TABLES_H
#define VCRUISE_DETECTION_TABLES_H

#include "engines/advancedDetector.h"

#include "vcruise/detection.h"

namespace VCruise {

static const VCruiseGameDescription gameDescriptions[] = {
	{ // Reah: Face the Unknown, English DVD version
		{
			"reah",
			"English DVD",
			AD_ENTRY2s("Reah.exe", "69aa9832338db3f40f616386e54857e8", 304128,
					   "0170_b.wav", "5b705300b4fee3ceb821a1b55884a722", 129248),
			Common::UNK_LANG,
			Common::kPlatformWindows,
			ADGF_TESTING | VCRUISE_GF_WANT_MP3,
			GUIO0()
		},
		GID_REAH,
		Common::EN_ANY,
	},
	{ // Reah: Face the Unknown, English digital (GOG) version
		{
			"reah",
			"English Digital + German Community Patch",
			AD_ENTRY2s("Reah.exe", "60ec19c53f1323cc7f0314f98d396283", 304128,
					   "0170_b.wav", "5b705300b4fee3ceb821a1b55884a722", 129248),
			Common::UNK_LANG,
			Common::kPlatformWindows,
			ADGF_TESTING | VCRUISE_GF_WANT_MP3,
			GUIO0()
		},
		GID_REAH,
		Common::EN_ANY,
	},
	{ // Reah: Face the Unknown, English 6 CD Version
		{
			"reah",
			"English CD",
			AD_ENTRY2s("Reah.exe", "77bc7f7819cdd443f52b193529138c87", 305664,
					   "0170_b.wav", "5b705300b4fee3ceb821a1b55884a722", 129248),
			Common::UNK_LANG,
			Common::kPlatformWindows,
			ADGF_TESTING,
			GUIO0()
		},
		GID_REAH,
		Common::EN_ANY,
	},
	{ // Reah: Face the Unknown, German 6 CD Version
		{
			"reah",
			"German CD",
			AD_ENTRY2s("Reah.exe", "be29f9f9fc9a454488f9d2fb68e26326", 305664,
					   "0170_b.wav", "5f84af02d1193ce9c47fe0dc821f0613", 121220),
			Common::UNK_LANG,
			Common::kPlatformWindows,
			ADGF_TESTING,
			GUIO0()
		},
		GID_REAH,
		Common::DE_DEU,
	},
	{ // Reah: Face the Unknown, English digital (GOG) version + German VO community patch
		{
			"reah",
			"English Digital + German Community Patch",
			AD_ENTRY2s("Reah.exe", "60ec19c53f1323cc7f0314f98d396283", 304128,
					   "0170_b.wav", "5f84af02d1193ce9c47fe0dc821f0613", 121220),
			Common::UNK_LANG,
			Common::kPlatformWindows,
			ADGF_TESTING | VCRUISE_GF_WANT_MP3,
			GUIO0()
		},
		GID_REAH,
		Common::EN_ANY,
	},
	{ // Schizm: Mysterious Journey, English DVD/digital Version
		{
			"schizm",
			"DVD",
			AD_ENTRY1s("Schizm.exe", "296edd26d951c3bdc4d303c4c88b27cd", 364544),
			Common::UNK_LANG,
			Common::kPlatformWindows,
			ADGF_UNSTABLE | VCRUISE_GF_WANT_OGG_VORBIS | VCRUISE_GF_NEED_JPEG,
			GUIO0()
		},
		GID_SCHIZM,
		Common::EN_GRB,
	},
	{ AD_TABLE_END_MARKER, GID_UNKNOWN, Common::UNK_LANG }
};

} // End of namespace VCruise

#endif
