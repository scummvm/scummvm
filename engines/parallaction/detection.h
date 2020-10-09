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

#ifndef PARALLACTION_DETECTION_H
#define PARALLACTION_DETECTION_H

#include "engines/advancedDetector.h"

namespace Parallaction {

enum {
	GF_DEMO = 1 << 0,
	GF_LANG_EN = 1 << 1,
	GF_LANG_FR = 1 << 2,
	GF_LANG_DE = 1 << 3,
	GF_LANG_IT = 1 << 4,
	GF_LANG_MULT = 1 << 5
};

enum ParallactionGameType {
	GType_Nippon = 1,
	GType_BRA
};

struct PARALLACTIONGameDescription {
	ADGameDescription desc;

	int gameType;
	uint32 features;
};

} // End of namespace Parallaction

#endif // PARALLACTION_DETECTION_H
