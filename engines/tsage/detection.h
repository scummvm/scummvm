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

#ifndef TSAGE_DETECTION_H
#define TSAGE_DETECTION_H

#include "engines/advancedDetector.h"

namespace TsAGE {

enum {
	GType_Ringworld = 0,
	GType_BlueForce = 1,
	GType_Ringworld2 = 2,
	GType_Sherlock1 = 5
};

enum {
	GF_DEMO			= 1 << 0,
	GF_CD			= 1 << 1,
	GF_FLOPPY		= 1 << 2,
	GF_ALT_REGIONS	= 1 << 3,
	GF_UNINSTALLED	= 1 << 4
};

struct tSageGameDescription {
	ADGameDescription desc;

	int gameID;
	uint32 features;
};

} // End of namespace TsAGE

#endif // TSAGE_DETECTION_H
