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

#ifndef KYRA_DETECTION_H
#define KYRA_DETECTION_H

#include "engines/advancedDetector.h"

namespace Kyra {

enum {
	GI_KYRA1 = 0,
	GI_KYRA2 = 1,
	GI_KYRA3 = 2,
	GI_LOL = 4,
	GI_EOB1 = 5,
	GI_EOB2 = 6
};

struct GameFlags {
	Common::Language lang;

	// Only for MR versions which have 4 languages instead of 3 (Simplified Chinese/Traditional Chinese)
	Common::Language extraLang;

	// language overwrites of fan translations (only needed for multilingual games)
	Common::Language fanLang;
	Common::Language replacedLang;

	Common::Platform platform;

	bool isDemo               : 1;
	bool useAltShapeHeader    : 1;    // alternative shape header (uses 2 bytes more, those are unused though)
	bool isTalkie             : 1;
	bool isOldFloppy          : 1;
	bool useHiRes             : 1;
	bool use16ColorMode       : 1;
	bool useHiColorMode       : 1;
	bool useDigSound          : 1;
	bool useInstallerPackage  : 1;

	byte gameID;
};

} // End of namespace Kyra

namespace {

struct KYRAGameDescription {
	ADGameDescription desc;

	Kyra::GameFlags flags;
};

#define GAMEOPTION_KYRA3_AUDIENCE GUIO_GAMEOPTIONS1
#define GAMEOPTION_KYRA3_SKIP     GUIO_GAMEOPTIONS2
#define GAMEOPTION_KYRA3_HELIUM   GUIO_GAMEOPTIONS3

#define GAMEOPTION_LOL_SCROLLING  GUIO_GAMEOPTIONS4
#define GAMEOPTION_LOL_CURSORS    GUIO_GAMEOPTIONS5
#define GAMEOPTION_LOL_SAVENAMES  GUIO_GAMEOPTIONS8

#define GAMEOPTION_EOB_HPGRAPHS   GUIO_GAMEOPTIONS6
#define GAMEOPTION_EOB_MOUSESWAP  GUIO_GAMEOPTIONS7

} // End of anonymous namespace

#endif // KYRA_DETECTION_H
