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

#include "common/scummsys.h"
#include "mads/staticres.h"

namespace MADS {

const char *const kArticleList[9] = {
	nullptr, "with", "to", "at", "from", "on", "in", "under", "behind"
};

const char *const kCheatingEnabledDesc[3] = {
	"CHEATING ENABLED",
	"(For your convenience).",
	nullptr
};

const char *const kAtStr = "at";
const char *const kLookAroundStr = "Look around";
const char *const kToStr = "to ";
const char *const kUseStr = "Use ";
const char *const kWalkToStr = "Walk to ";
const char *const kFenceStr = "fence";
const char *const kOverStr = "over";

const char *const kGameReleaseInfoStr = "ScummVM rev: 8.43 14-Sept-92";
const char *const kGameReleaseTitleStr = "GAME RELASE VERSION INFO";

} // End of namespace MADS
