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
	GType_LIVINGBOOKSV5
};

#define GAMEOPTION_ME   GUIO_GAMEOPTIONS1
#define GAMEOPTION_25TH GUIO_GAMEOPTIONS2
#define GAMEOPTION_DEMO GUIO_GAMEOPTIONS3

enum MohawkGameFeatures {
	GF_ME             = (1 << 0), // Myst Masterpiece Edition
	GF_25TH           = (1 << 1), // Myst and Riven 25th Anniversary
	GF_DVD            = (1 << 2),
	GF_DEMO           = (1 << 3),
	GF_LB_10          = (1 << 4)  // very early Living Books 1.0 games
};

struct MohawkGameDescription {
	ADGameDescription desc;

	uint8 gameType;
	uint32 features;
	const char *appName;
};

} // End of namespace Mohawk

#endif // MOHAWK_DETECTION_H
