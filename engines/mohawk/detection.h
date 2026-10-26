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

#ifndef MOHAWK_DETECTION_H
#define MOHAWK_DETECTION_H

#include "engines/advancedDetector.h"

namespace Mohawk {

enum MohawkGameType {
	GType_MYST,
	GType_MAKINGOF,
	GType_RIVEN,
	GType_CSTIME,
	GType_LIVINGBOOKSV1,
	GType_LIVINGBOOKSV2,
	GType_LIVINGBOOKSV3,
	GType_LIVINGBOOKSV4,
	GType_LIVINGBOOKSV5,
	GType_ZOOMBINI,
	GType_CSWORLD,
	GType_CSAMTRAK,
	GType_JAMESMATH,
	GType_TREEHOUSE,
	GType_1STDEGREE,
	GType_CSUSA,
	GType_ORLY,
	GType_ALIENTALES,
	GType_MATHSWORKSHOP,
	GType_WRICAMACT,
	GType_AMAZINGWRITING,
};

#define GAMEOPTION_ME   GUIO_GAMEOPTIONS1
#define GAMEOPTION_25TH GUIO_GAMEOPTIONS2
#define GAMEOPTION_DEMO GUIO_GAMEOPTIONS3

enum MohawkGameFeatures {
	GF_ME             = (1 << 0), // Myst Masterpiece Edition
	GF_25TH           = (1 << 1), // Myst and Riven 25th Anniversary
	GF_DVD            = (1 << 2),
	GF_DEMO           = (1 << 3),
	GF_LB_10          = (1 << 4), // very early Living Books 1.0 games
	/**
	 * Logical Journey of the Zoombinis v1.0 (Europe)
	 * - v1.0BR
	 * - v1.0FR
	 * - v1.0GE
	 * - v1.0IT
	 * @remarks
	 * Europe v1.x releases with localized resource directories.
	 * The CD contains only 16-bit installer and executable.
	 */
	GF_ZMB_10_EU      = (1 << 5),
	/**
	 * Logical Journey of the Zoombinis v1.1 (Europe)
	 * - v1.1BR
	 * - v1.1FR
	 * - v1.1GE
	 * @remarks
	 * Europe v1.x releases with ENGLISH/FRENCH/GERMAN resource directories.
	 * European v1.1 releases are just a bugfix release of Europe v1.0.
	 * The CD contains only 16-bit installer and executable.
	 */
	GF_ZMB_11_EU      = (1 << 6),
	/**
	 * 2002 Repackaged version for Logical Journey of the Zoombinis v1.1 (France)
	 * - v1.1FR_2002
	 * @remarks
	 * Europe v1.1 is just a bugfix release of Europe v1.0.
	 * The executable is still 16-bit, but the installer is now 32-bit and the resource directories are now in French only.
	 */
	GF_ZMB_11_FR_2002 = (1 << 7),
	/** USv1.0 Windows and Macintosh retail releases (January 1996), with one-clear difficulty advancement. */
	GF_ZMB_10_US      = (1 << 8),
	/**
	 * Logical Journey of the Zoombinis v1.0 (Spain and Portugal)
	 * - v1.0ES
	 * - v1.0PT
	 * @remarks
	 * These localizations retain the US v1.0 game-state and UI behavior,
	 * but ships 16/32-bit executables and keeps its resources under DATA.
	 * Localized UI text is embedded in the executable instead of TEXTSTR.MHK.
	 */
	GF_ZMB_10_ESPT    = (1 << 9),
	/**
	 * Logical Journey of the Zoombinis v1.0 (Japan).
	 * - v1.0JP
	 * @remarks
	 * This release retains the Europe v1.0 game-state, but has patched UI layouts.
	 * It carries Japanese resources under JAPANESE/, while retaining the Europe v1.x resources.
	 * Localized UI text is still stored in JAPANESE/TEXTSTR.MHK.
	 * The CD contains a 16-bit installer and 16/32-bit executable.
	 */
	GF_ZMB_10_JP      = (1 << 10),
	/**
	 * Versions based on Logical Journey of the Zoombinis v1.1 (US)
	 * - v1.1US
	 * - v1.11KR
	 * @remarks
	 * v1.1US adds features and adjusts the balance patched on US v1.0 release.
	 * v1.11KR is a Korean localization based on v1.1US.
	 * It added IME support for Hangul input and word dictionary for Korean Zoombini names.
	 * The CD contains a 16-bit installer and 16/32-bit executable.
	 */
	GF_ZMB_11_US      = (1 << 11),
	/**
	 * Logical Journey of the Zoombinis v2.0 (The Learning Company rebuild)
	 * - v2.0US
	 * - v2.0PL
	 * @remarks
	 * The v2.0 TLC releases updated the engine with a more modern backend,
	 * and have better compatibility with modern NT-based Windows.
	 * The CD contains only 32-bit installer and executable.
	 */
	GF_ZMB_20_US      = (1 << 12),
};

struct MohawkGameDescription {
	ADGameDescription desc;

	uint8 gameType;
	uint32 features;
	const char *appName;

	uint32 sizeBuffer() const {
		uint32 ret = desc.sizeBuffer();
		ret += ADDynamicDescription::strSizeBuffer(appName);
		return ret;
	}

	void *toBuffer(void *buffer) {
		buffer = desc.toBuffer(buffer);
		buffer = ADDynamicDescription::strToBuffer(buffer, appName);
		return buffer;
	}
};

} // End of namespace Mohawk

#endif // MOHAWK_DETECTION_H
