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

#include "common/platform.h"
#include "common/gui_options.h"
#include "common/str.h"
#include "common/algorithm.h"

namespace Common {

const PlatformDescription g_platforms[] = {
	{ GUIO_PLAT_APPLE2GS, "2gs", "2gs", "2gs", "Apple IIgs", kPlatformApple2GS },
	{ GUIO_PLAT_APPLE2, "apple2", "apple2", "apple2", "Apple II", kPlatformApple2 },
	{ GUIO_PLAT_3DO, "3do", "3do", "3do", "3DO", kPlatform3DO },
	{ GUIO_PLAT_ACORN, "acorn", "acorn", "acorn", "Acorn", kPlatformAcorn },
	{ GUIO_PLAT_AMIGA, "amiga", "ami", "amiga", "Amiga", kPlatformAmiga },
	{ GUIO_PLAT_ATARI8BIT, "atari8", "atari8", "atari8", "Atari 8-bit", kPlatformAtari8Bit },
	{ GUIO_PLAT_ATARIST, "atari", "atari-st", "st", "Atari ST", kPlatformAtariST },
	{ GUIO_PLAT_C64, "c64", "c64", "c64", "Commodore 64", kPlatformC64 },
	{ GUIO_PLAT_AMSTRADCPC, "cpc", "cpc", "cpc", "Amstrad CPC", kPlatformAmstradCPC },
	{ GUIO_PLAT_DOS, "pc", "dos", "ibm", "DOS", kPlatformDOS },
	{ GUIO_PLAT_PC98, "pc98", "pc98", "pc98", "PC-98", kPlatformPC98 },
	{ GUIO_PLAT_WII, "wii", "wii", "wii", "Nintendo Wii", kPlatformWii },
	{ GUIO_PLAT_COCO, "coco", "coco", "coco", "CoCo", kPlatformCoCo },		// CoCo 1/2
	{ GUIO_PLAT_COCO3, "coco3", "coco3", "coco3", "CoCo3", kPlatformCoCo3 },	// CoCo 3 only
	// The 'official' spelling seems to be "FM-TOWNS" (e.g. in the Indy4 demo).
	// However, on the net many variations can be seen, like "FMTOWNS",
	// "FM TOWNS", "FmTowns", etc.
	{ GUIO_PLAT_FMTOWNS, "fmtowns", "towns", "fm", "FM-TOWNS", kPlatformFMTowns },
	{ GUIO_PLAT_LINUX, "linux", "linux", "linux", "Linux", kPlatformLinux },
	{ GUIO_PLAT_MACINTOSH, "macintosh", "mac", "mac", "Macintosh", kPlatformMacintosh },
	{ GUIO_PLAT_PCENGINE, "pce", "pce", "pce", "PC-Engine", kPlatformPCEngine },
	{ GUIO_PLAT_NES, "nes", "nes", "nes", "NES", kPlatformNES },
	{ GUIO_PLAT_SEGACD, "segacd", "segacd", "sega", "SegaCD", kPlatformSegaCD },
	{ GUIO_PLAT_WINDOWS, "windows", "win", "win", "Windows", kPlatformWindows },
	{ GUIO_PLAT_PSX, "playstation", "psx", "psx", "Sony PlayStation", kPlatformPSX },
	{ GUIO_PLAT_PS2, "playstation2", "ps2", "ps2", "Sony PlayStation 2", kPlatformPS2 },
	{ GUIO_PLAT_PS3, "playstation3", "ps3", "ps3", "Sony PlayStation 3", kPlatformPS3 },
	{ GUIO_PLAT_XBOX, "xbox", "xbox", "xbox", "Microsoft Xbox", kPlatformXbox },
	{ GUIO_PLAT_CDI, "cdi", "cdi", "cdi", "Philips CD-i", kPlatformCDi },
	{ GUIO_PLAT_IOS, "ios", "ios", "ios", "Apple iOS", kPlatformIOS },
	{ GUIO_PLAT_ANDROID, "android", "android", "android", "Android", kPlatformAndroid },
	{ GUIO_PLAT_OS2, "os2", "os2", "os2", "OS/2", kPlatformOS2 },
	{ GUIO_PLAT_BEOS, "beos", "beos", "beos", "BeOS", kPlatformBeOS },
	{ GUIO_PLAT_POCKETPC, "ppc", "ppc", "ppc", "PocketPC", kPlatformPocketPC },
	{ GUIO_PLAT_MEGADRIVE, "megadrive", "genesis", "md", "Mega Drive/Genesis", kPlatformMegaDrive },
	{ GUIO_PLAT_SATURN, "saturn", "saturn", "saturn", "Sega Saturn", kPlatformSaturn },
	{ GUIO_PLAT_PIPPIN, "pippin", "pippin", "pippin", "Pippin", kPlatformPippin },
	{ GUIO_PLAT_MACINTOSHII, "macintosh2", "macintosh2", "mac2", "Macintosh II", kPlatformMacintoshII },
	{ GUIO_PLAT_SHOCKWAVE, "shockwave", "shockwave", "shock", "Shockwave", kPlatformShockwave },
	{ GUIO_PLAT_ZX, "zx", "zx", "zx", "ZX Spectrum", kPlatformZX },
	{ GUIO_PLAT_TI994, "ti994", "ti994", "ti994", "TI-99/4A", kPlatformTI994 },
	{ GUIO_PLAT_NINTENDOSWITCH, "switch", "switch", "switch", "Nintendo Switch", kPlatformNintendoSwitch },

	{ nullptr, nullptr, nullptr, nullptr, "Default", kPlatformUnknown }
};

Platform parsePlatform(const String &str) {
	if (str.empty())
		return kPlatformUnknown;

	// Handle some special case separately, for compatibility with old config
	// files.
	if (str == "1")
		return kPlatformAmiga;
	else if (str == "2")
		return kPlatformAtariST;
	else if (str == "3")
		return kPlatformMacintosh;

	const PlatformDescription *l = g_platforms;
	for (; l->code; ++l) {
		if (str.equalsIgnoreCase(l->code) || str.equalsIgnoreCase(l->code2) || str.equalsIgnoreCase(l->abbrev))
			return l->id;
	}

	return kPlatformUnknown;
}


const char *getPlatformCode(Platform id) {
	const PlatformDescription *l = g_platforms;
	for (; l->code; ++l) {
		if (l->id == id)
			return l->code;
	}
	return nullptr;
}

const char *getPlatformAbbrev(Platform id) {
	const PlatformDescription *l = g_platforms;
	for (; l->code; ++l) {
		if (l->id == id)
			return l->abbrev;
	}
	return nullptr;
}

const char *getPlatformDescription(Platform id) {
	const PlatformDescription *l = g_platforms;
	for (; l->code; ++l) {
		if (l->id == id)
			return l->description;
	}
	return l->description;
}

bool checkGameGUIOptionPlatform(Platform plat, const String &str) {
	if (!str.contains("plat_")) // If no platforms are specified
		return true;

	if (str.contains(getGameGUIOptionsDescriptionPlatform(plat)))
		return true;

	return false;
}

const String parseGameGUIOptionsPlatforms(const String &str) {
	String res;

	for (int i = 0; g_platforms[i].code; i++)
		if (str.contains("plat_" + String(g_platforms[i].code)))
			res += String(g_platforms[i].GUIOption);

	return res;
}

const String getGameGUIOptionsDescriptionPlatforms(const String &str) {
	String res;

	for (int i = 0; g_platforms[i].GUIOption; i++)
		if (str.contains(g_platforms[i].GUIOption))
			res += "plat_" + String(g_platforms[i].code) + " ";

	res.trim();

	return res;
}

const String getGameGUIOptionsDescriptionPlatform(Platform plat) {
	if (plat == kPlatformUnknown)
		return "";

	// Using platform code as description for GUI options to avoid spaces in the name
	return String("plat_") + getPlatformCode(plat);
}

List<String> getPlatformList() {
	List<String> list;

	for (const PlatformDescription *l = g_platforms; l->code; ++l)
		list.push_back(l->code2);

	 Common::sort(list.begin(), list.end());

	 return list;
}

} // End of namespace Common
