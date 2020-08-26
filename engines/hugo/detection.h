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

#ifndef HUGO_DETECTION_H
#define HUGO_DETECTION_H

#include "engines/advancedDetector.h"

namespace Hugo {

enum HugoGameFeatures {
	GF_PACKED = (1 << 0) // Database
};

enum GameType {
	kGameTypeNone  = 0,
	kGameTypeHugo1,
	kGameTypeHugo2,
	kGameTypeHugo3
};

enum GameVariant {
	kGameVariantH1Win = 0,
	kGameVariantH2Win,
	kGameVariantH3Win,
	kGameVariantH1Dos,
	kGameVariantH2Dos,
	kGameVariantH3Dos,
	kGameVariantNone
};

struct HugoGameDescription {
	ADGameDescription desc;
	GameType gameType;
};

} // End of namespace Hugo

#endif // HUGO_DETECTION_H
