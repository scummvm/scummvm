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

#ifndef WINTERMUTE_GAME_DESCRIPTION_H
#define WINTERMUTE_GAME_DESCRIPTION_H

#include "engines/advancedDetector.h"

namespace Wintermute {

enum WMETargetExecutable {
	OLDEST_VERSION,
	WME_1_0_12, // DEAD:CODE 2003
	WME_1_0_19, // DEAD:CODE 2003
	WME_1_0_20, // DEAD:CODE 2003
	WME_1_0_22, // DEAD:CODE 2003
	WME_1_0_24, // DEAD:CODE 2003
	WME_1_0_25, // DEAD:CODE 2003
	WME_1_0_28, // DEAD:CODE 2003
	WME_1_0_30, // DEAD:CODE 2003
	WME_1_0_31, // DEAD:CODE 2003
	WME_1_1_33, // DEAD:CODE 2003
	WME_1_1_35, // DEAD:CODE 2003
	WME_1_1_37, // DEAD:CODE 2003
	WME_1_1_39, // DEAD:CODE 2004
	WME_1_2_43, // DEAD:CODE 2004
	WME_1_2_44, // DEAD:CODE 2004
	WME_1_3_0,  // DEAD:CODE 2004
	WME_1_3_2,  // DEAD:CODE 2004
	WME_1_3_3,  // DEAD:CODE 2004
	WME_1_4_0,  // DEAD:CODE 2005
	WME_1_4_1,  // DEAD:CODE 2005
	WME_1_5_0,  // DEAD:CODE 2005
	WME_1_5_2,  // DEAD:CODE 2005
	WME_1_6_0,  // DEAD:CODE 2006
	WME_1_6_1,  // DEAD:CODE 2006
	WME_1_7_0,  // DEAD:CODE 2007
	WME_1_7_1,  // DEAD:CODE 2007
	WME_1_7_2,  // DEAD:CODE 2007
	WME_1_7_3,  // DEAD:CODE 2007
	WME_1_7_93, // DEAD:CODE 2007
	WME_1_7_94, // DEAD:CODE 2007
	WME_1_8_0,  // DEAD:CODE 2007
	WME_1_8_1,  // DEAD:CODE 2007
	WME_1_8_2,  // DEAD:CODE 2008
	WME_1_8_3,  // DEAD:CODE 2008
	WME_1_8_4,  // DEAD:CODE 2008
	WME_1_8_5,  // DEAD:CODE 2008
	WME_1_8_6,  // DEAD:CODE 2008
	WME_1_8_7,  // DEAD:CODE 2008, released as "1.8.7 beta"
	WME_1_8_8,  // DEAD:CODE 2008, released as "1.8.8 beta"
	WME_1_8_9,  // DEAD:CODE 2008, released as "1.8.9 beta"
	WME_1_8_10, // DEAD:CODE 2009

	// fork of WME_1_8_10
	WME_ANDISHE_VARAN, // Andishe Varan Engine 1.0.0.0

	WME_1_8_11, // DEAD:CODE 2009
	WME_1_9_0,  // DEAD:CODE 2009, released as "1.9.0 beta"

	// fork of WME_1_9_0
	WME_KINJAL_1_0,
	WME_KINJAL_1_1,
	WME_KINJAL_1_2,
	WME_KINJAL_1_3,
	WME_KINJAL_1_4,

	// fork of WME_KINJAL_1_4
	WME_HEROCRAFT,

	WME_1_9_1,  // DEAD:CODE 2010

	// fork of WME_1_9_1
	WME_KINJAL_1_5,
	WME_KINJAL_1_6,
	WME_KINJAL_1_7,
	WME_KINJAL_1_7a,
	WME_KINJAL_1_7b,
	WME_KINJAL_1_8,
	WME_KINJAL_1_9,
	WME_KINJAL_2_0,

	WME_1_9_2,  // DEAD:CODE 2010
	WME_1_9_3,  // DEAD:CODE 2012, released as "1.10.1 beta"
	WME_LITE,
	LATEST_VERSION,

	// fork of WME_LITE
	FOXTAIL_OLDEST_VERSION,
	FOXTAIL_1_2_227,
	FOXTAIL_1_2_230,
	FOXTAIL_1_2_304,
	FOXTAIL_1_2_362,
	FOXTAIL_1_2_527,
	FOXTAIL_1_2_896,
	FOXTAIL_1_2_902,
	FOXTAIL_LATEST_VERSION
};

enum WintermuteGameFeatures {
	/** A game with low-spec resources. */
	GF_LOWSPEC_ASSETS       = 1 << 0,
	GF_IGNORE_SD_FILES      = 1 << 1,
	GF_IGNORE_HD_FILES      = 1 << 2,
	GF_3D                   = 1 << 3
};

struct WMEGameDescription {
	ADGameDescription adDesc;
	WMETargetExecutable targetExecutable;
};

} // End of namespace Wintermute

#endif  /* WINTERMUTE_GAME_DESCRIPTION_H_ */
