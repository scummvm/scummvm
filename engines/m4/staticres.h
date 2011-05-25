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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef M4_STATICRES_H
#define M4_STATICRES_H

#include "common/scummsys.h"
#include "m4/m4.h"

namespace M4 {

extern const char *englishMADSArticleList[9];

extern const char *cheatingEnabledDesc[3];

extern const char *atStr;
extern const char *lookAroundStr;
extern const char *toStr;
extern const char *useStr;
extern const char *walkToStr;
extern const char *fenceStr;
extern const char *overStr;

extern const char *GameReleaseInfoStr;
extern const char *GameReleaseTitleStr;

struct VerbInit {
	int verb;
	int8 flag1;
	int8 flag2;
};

extern VerbInit verbList[10];

} // End of namespace M4

#endif
