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

#ifndef COMMON_PLATFORM_H
#define COMMON_PLATFORM_H

#include "common/scummsys.h"
#include "common/list.h"

namespace Common {

/**
 * @defgroup common_platform Game platforms
 * @ingroup common
 *
 * @brief API for managing game platforms.
 * @{
 */

class String;

/**
 * List of game platforms. Specifying a platform for a target can be used to
 * give the game engines a hint for which platform the game data file are.
 * This may be optional or required, depending on the game engine and the
 * game in question.
 */
enum Platform : int8 {
	kPlatformDOS,
	kPlatformAmiga,
	kPlatformAmstradCPC,
	kPlatformAtari8Bit,
	kPlatformAtariST,
	kPlatformMacintosh,
	kPlatformFMTowns,
	kPlatformWindows,
	kPlatformNES,
	kPlatformC64,
	kPlatformCoCo,
	kPlatformCoCo3,
	kPlatformLinux,
	kPlatformAcorn,
	kPlatformSegaCD,
	kPlatform3DO,
	kPlatformPCEngine,
	kPlatformApple2,
	kPlatformApple2GS,
	kPlatformPC98,
	kPlatformWii,
	kPlatformPSX,
	kPlatformPS2,
	kPlatformPS3,
	kPlatformXbox,
	kPlatformCDi,
	kPlatformIOS,
	kPlatformAndroid,
	kPlatformOS2,
	kPlatformBeOS,
	kPlatformPocketPC,
	kPlatformMegaDrive,
	kPlatformSaturn,
	kPlatformPippin,
	kPlatformMacintoshII,
	kPlatformShockwave,
	kPlatformZX,
	kPlatformTI994,
	kPlatformNintendoSwitch,

	kPlatformUnknown = -1
};

struct PlatformDescription {
	const char *code;
	const char *code2;
	const char *abbrev;
	const char *description;
	Platform id;
};

extern const PlatformDescription g_platforms[];

/** Convert a string containing a platform name into a Platform enum value. */
extern Platform parsePlatform(const String &str);
extern const char *getPlatformCode(Platform id);
extern const char *getPlatformAbbrev(Platform id);
extern const char *getPlatformDescription(Platform id);

List<String> getPlatformList();

/** @} */

} // End of namespace Common

#endif
