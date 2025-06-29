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

// Helper macros to get enum value for the platform
#define GET_ENUM_VAL_IMPL(val, hex) val
#define GET_ENUM_VAL(name) GET_ENUM_VAL_IMPL(name)

// List of platforms values as int and string literals, to be used in platform enum and gui options
#define kPlatformApple2GS_VAL 0x40, "\x40"
#define kPlatformApple2_VAL 0x41, "\x41"
#define kPlatform3DO_VAL 0x42, "\x42"
#define kPlatformAcorn_VAL 0x43, "\x43"
#define kPlatformAmiga_VAL 0x44, "\x44"
#define kPlatformAtari8Bit_VAL 0x45, "\x45"
#define kPlatformAtariST_VAL 0x46, "\x46"
#define kPlatformC64_VAL 0x47, "\x47"
#define kPlatformAmstradCPC_VAL 0x48, "\x48"
#define kPlatformDOS_VAL 0x49, "\x49"
#define kPlatformPC98_VAL 0x4A, "\x4A"
#define kPlatformWii_VAL 0x4B, "\x4B"
#define kPlatformCoCo_VAL 0x4C, "\x4C"
#define kPlatformCoCo3_VAL 0x4D, "\x4D"
#define kPlatformFMTowns_VAL 0x4E, "\x4E"
#define kPlatformLinux_VAL 0x4F, "\x4F"
#define kPlatformMacintosh_VAL 0x50, "\x50"
#define kPlatformPCEngine_VAL 0x51, "\x51"
#define kPlatformNES_VAL 0x52, "\x52"
#define kPlatformSegaCD_VAL 0x53, "\x53"
#define kPlatformWindows_VAL 0x54, "\x54"
#define kPlatformPSX_VAL 0x55, "\x55"
#define kPlatformPS2_VAL 0x56, "\x56"
#define kPlatformPS3_VAL 0x57, "\x57"
#define kPlatformXbox_VAL 0x58, "\x58"
#define kPlatformCDi_VAL 0x59, "\x59"
#define kPlatformIOS_VAL 0x5A, "\x5A"
#define kPlatformAndroid_VAL 0x5B, "\x5B"
#define kPlatformOS2_VAL 0x5C, "\x5C"
#define kPlatformBeOS_VAL 0x5D, "\x5D"
#define kPlatformPocketPC_VAL 0x5E, "\x5E"
#define kPlatformMegaDrive_VAL 0x5F, "\x5F"
#define kPlatformSaturn_VAL 0x60, "\x60"
#define kPlatformPippin_VAL 0x61, "\x61"
#define kPlatformMacintoshII_VAL 0x62, "\x62"
#define kPlatformShockwave_VAL 0x63, "\x63"
#define kPlatformZX_VAL 0x64, "\x64"
#define kPlatformTI994_VAL 0x65, "\x65"
#define kPlatformNintendoSwitch_VAL 0x66, "\x66"

/**
 * List of game platforms. Specifying a platform for a target can be used to
 * give the game engines a hint for which platform the game data file are.
 * This may be optional or required, depending on the game engine and the
 * game in question.
 */
enum Platform : int8 {
    kPlatformDOS = GET_ENUM_VAL(kPlatformDOS_VAL),
    kPlatformAmiga = GET_ENUM_VAL(kPlatformAmiga_VAL),
    kPlatformAmstradCPC = GET_ENUM_VAL(kPlatformAmstradCPC_VAL),
    kPlatformAtari8Bit = GET_ENUM_VAL(kPlatformAtari8Bit_VAL),
    kPlatformAtariST = GET_ENUM_VAL(kPlatformAtariST_VAL),
    kPlatformMacintosh = GET_ENUM_VAL(kPlatformMacintosh_VAL),
    kPlatformFMTowns = GET_ENUM_VAL(kPlatformFMTowns_VAL),
    kPlatformWindows = GET_ENUM_VAL(kPlatformWindows_VAL),
    kPlatformNES = GET_ENUM_VAL(kPlatformNES_VAL),
    kPlatformC64 = GET_ENUM_VAL(kPlatformC64_VAL),
    kPlatformCoCo = GET_ENUM_VAL(kPlatformCoCo_VAL),
    kPlatformCoCo3 = GET_ENUM_VAL(kPlatformCoCo3_VAL),
    kPlatformLinux = GET_ENUM_VAL(kPlatformLinux_VAL),
    kPlatformAcorn = GET_ENUM_VAL(kPlatformAcorn_VAL),
    kPlatformSegaCD = GET_ENUM_VAL(kPlatformSegaCD_VAL),
    kPlatform3DO = GET_ENUM_VAL(kPlatform3DO_VAL),
    kPlatformPCEngine = GET_ENUM_VAL(kPlatformPCEngine_VAL),
    kPlatformApple2 = GET_ENUM_VAL(kPlatformApple2_VAL),
    kPlatformApple2GS = GET_ENUM_VAL(kPlatformApple2GS_VAL),
    kPlatformPC98 = GET_ENUM_VAL(kPlatformPC98_VAL),
    kPlatformWii = GET_ENUM_VAL(kPlatformWii_VAL),
    kPlatformPSX = GET_ENUM_VAL(kPlatformPSX_VAL),
    kPlatformPS2 = GET_ENUM_VAL(kPlatformPS2_VAL),
    kPlatformPS3 = GET_ENUM_VAL(kPlatformPS3_VAL),
    kPlatformXbox = GET_ENUM_VAL(kPlatformXbox_VAL),
    kPlatformCDi = GET_ENUM_VAL(kPlatformCDi_VAL),
    kPlatformIOS = GET_ENUM_VAL(kPlatformIOS_VAL),
    kPlatformAndroid = GET_ENUM_VAL(kPlatformAndroid_VAL),
    kPlatformOS2 = GET_ENUM_VAL(kPlatformOS2_VAL),
    kPlatformBeOS = GET_ENUM_VAL(kPlatformBeOS_VAL),
    kPlatformPocketPC = GET_ENUM_VAL(kPlatformPocketPC_VAL),
    kPlatformMegaDrive = GET_ENUM_VAL(kPlatformMegaDrive_VAL),
    kPlatformSaturn = GET_ENUM_VAL(kPlatformSaturn_VAL),
    kPlatformPippin = GET_ENUM_VAL(kPlatformPippin_VAL),
    kPlatformMacintoshII = GET_ENUM_VAL(kPlatformMacintoshII_VAL),
    kPlatformShockwave = GET_ENUM_VAL(kPlatformShockwave_VAL),
    kPlatformZX = GET_ENUM_VAL(kPlatformZX_VAL),
    kPlatformTI994 = GET_ENUM_VAL(kPlatformTI994_VAL),
    kPlatformNintendoSwitch = GET_ENUM_VAL(kPlatformNintendoSwitch_VAL),

    // This is a special case, unknown platform won't be added to gui options string
    kPlatformUnknown = -1
};

// Do not pollute namespace
#undef GET_ENUM_VAL_IMPL
#undef GET_ENUM_VAL

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

/**
* Return string containing platform description based on platform enum
*/
const String getGameGUIOptionsDescriptionPlatform(Platform plat);

/**
* Check if given platform option is present in a string
*/
bool checkGameGUIOptionPlatform(Platform plat, const String &str);

/**
* Parse gui options string to GUIO platform literals
*/
const String parseGameGUIOptionsPlatforms(const String &str);

/**
* Return string containing platform(s) description based on gui options string
*/
const String getGameGUIOptionsDescriptionPlatforms(const String &str);

List<String> getPlatformList();

/** @} */

} // End of namespace Common

#endif
