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

// Extra level of indirection required to force macro expansion on some compilers
#define GET_ENUM_VAL_IMPL(val, hex) val
#define GET_ENUM_VAL_EXPAND(x) GET_ENUM_VAL_IMPL x
#define GET_ENUM_VAL(name) GET_ENUM_VAL_EXPAND((name))

// List of platforms values as int and string literals, to be used in platform enum and gui options
// Starting GUI options at 0x80 to fit the numbering plan
#define kPlatformApple2GS_VAL          0x00, "\x80"
#define kPlatformApple2_VAL            0x01, "\x81"
#define kPlatform3DO_VAL               0x02, "\x82"
#define kPlatformAcorn_VAL             0x03, "\x83"
#define kPlatformAmiga_VAL             0x04, "\x84"
#define kPlatformAtari8Bit_VAL         0x05, "\x85"
#define kPlatformAtariST_VAL           0x06, "\x86"
#define kPlatformC64_VAL               0x07, "\x87"
#define kPlatformAmstradCPC_VAL        0x08, "\x88"
#define kPlatformDOS_VAL               0x09, "\x89"
#define kPlatformPC98_VAL              0x0A, "\x8A"
#define kPlatformWii_VAL               0x0B, "\x8B"
#define kPlatformCoCo_VAL              0x0C, "\x8C"
#define kPlatformCoCo3_VAL             0x0D, "\x8D"
#define kPlatformFMTowns_VAL           0x0E, "\x8E"
#define kPlatformLinux_VAL             0x0F, "\x8F"
#define kPlatformMacintosh_VAL         0x10, "\x90"
#define kPlatformPCEngine_VAL          0x11, "\x91"
#define kPlatformNES_VAL               0x12, "\x92"
#define kPlatformSegaCD_VAL            0x13, "\x93"
#define kPlatformWindows_VAL           0x14, "\x94"
#define kPlatformPSX_VAL               0x15, "\x95"
#define kPlatformPS2_VAL               0x16, "\x96"
#define kPlatformPS3_VAL               0x17, "\x97"
#define kPlatformXbox_VAL              0x18, "\x98"
#define kPlatformCDi_VAL               0x19, "\x99"
#define kPlatformIOS_VAL               0x1A, "\x9A"
#define kPlatformAndroid_VAL           0x1B, "\x9B"
#define kPlatformOS2_VAL               0x1C, "\x9C"
#define kPlatformBeOS_VAL              0x1D, "\x9D"
#define kPlatformPocketPC_VAL          0x1E, "\x9E"
#define kPlatformMegaDrive_VAL         0x1F, "\x9F"
#define kPlatformSaturn_VAL            0x20, "\xA0"
#define kPlatformPippin_VAL            0x21, "\xA1"
#define kPlatformMacintoshII_VAL       0x22, "\xA2"
#define kPlatformShockwave_VAL         0x23, "\xA3"
#define kPlatformZX_VAL                0x24, "\xA4"
#define kPlatformTI994_VAL             0x25, "\xA5"
#define kPlatformNintendoSwitch_VAL    0x26, "\xA6"

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
#undef GET_ENUM_VAL_EXPAND

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
