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
	WME_1_0_0,
	WME_1_1_0,
	WME_1_2_0,
	WME_1_3_0,
	WME_1_4_0,
	WME_1_5_0,
	WME_1_6_0,
	WME_1_7_0,
	WME_1_8_0,
	WME_1_8_6,
	WME_1_9_0,
	LATEST_VERSION
};

struct WMEGameDescription {
	ADGameDescription adDesc;
	WMETargetExecutable targetExecutable;
};

}

#endif  /* WINTERMUTE_GAME_DESCRIPTION_H_ */
