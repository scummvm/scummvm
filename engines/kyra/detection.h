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

} // End of anonymous namespace

#endif // KYRA_DETECTION_H
