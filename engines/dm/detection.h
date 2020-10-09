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

/*
* Based on the Reverse Engineering work of Christophe Fontanel,
* maintainer of the Dungeon Master Encyclopaedia (http://dmweb.free.fr/)
*/

#ifndef DM_DETECTION_H
#define DM_DETECTION_H

#include "engines/advancedDetector.h"

namespace DM {

enum OriginalSaveFormat {
	kDMSaveFormatAcceptAny = -1,
	kDMSaveFormatEndOfList = 0,
	kDMSaveFormatNone = 0,
	kDMSaveFormatAtari = 1,
	kDMSaveFormatAmigaPC98FmTowns = 2,
	kCSBSaveFormatAtari = 2,
	kDMSaveFormatAppleIIgs = 3,
	kDMSaveFormatAmiga36PC = 5,
	kCSBSaveFormatAmigaPC98FmTowns = 5,
	kDMSaveFormatTotal
};

enum OriginalSavePlatform {
	kDMSavePlatformAcceptAny = -1,
	kDMSavePlatformEndOfList = 0,
	kDMSavePlatformNone = 0,
	kDMSavePlatformAtariSt = 1, // @ C1_PLATFORM_ATARI_ST
	kDMSavePlatformAppleIIgs = 2, // @ C2_PLATFORM_APPLE_IIGS
	kDMSavePlatformAmiga = 3, // @ C3_PLATFORM_AMIGA
	kDMSavePlatformPC98 = 5, // @ C5_PLATFORM_PC98
	kDMSavePlatformX68000 = 6, // @ C6_PLATFORM_X68000
	kDMSavePlatformFmTownsEN = 7, // @ C7_PLATFORM_FM_TOWNS_EN
	kDMSavePlatformFmTownsJP = 8, // @ C8_PLATFORM_FM_TOWNS_JP
	kDMSavePlatformPC = 9, // @ C9_PLATFORM_PC
	kDMSavePlatformTotal
};

enum SaveTarget {
	kDMSaveTargetAcceptAny = -1,
	kDMSaveTargetEndOfList = 0,
	kDMSaveTargetNone = 0,
	kDMSaveTargetDM21 = 1,
	kDMSaveTargetTotal
};

struct DMADGameDescription {
	ADGameDescription _desc;

	SaveTarget _saveTargetToWrite;
	OriginalSaveFormat _origSaveFormatToWrite;
	OriginalSavePlatform _origPlatformToWrite;

	SaveTarget _saveTargetToAccept[kDMSaveTargetTotal + 1];
	OriginalSaveFormat _saveFormatToAccept[kDMSaveFormatTotal + 1];
	OriginalSavePlatform _origPlatformToAccept[kDMSavePlatformTotal + 1];
};

} // End of namespace DM

#endif // DM_DETECTION_H
