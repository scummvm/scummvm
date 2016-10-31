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

const char *const kLookAroundStr = "Look around";
const char *const kToStr = "to ";
const char *const kUseStr = "Use ";
const char *const kWalkToStr = "Walk to ";
const char *const kFenceStr = "fence";
const char *const kOverStr = "over";

const char *const kGameReleaseInfoStr = "ScummVM rev: 8.43 14-Sept-92";
const char *const kGameReleaseTitleStr = "GAME RELEASE VERSION INFO";

const uint32 DEFAULT_VGA_LOW_PALETTE[16] = {
	0x000000, 0x0000a8, 0x00a800, 0x00a8a8, 0xa80000, 0xa800a8, 0xa85400, 0xa8a8a8,
	0x545454, 0x5454fc, 0x54fc54, 0x54fcfc, 0xfc5454, 0xfc54fc, 0xfcfc54, 0xfcfcfc
};

const uint32 DEFAULT_VGA_HIGH_PALETTE[16] = {
	0x2c402c, 0x2c4030, 0x2c4034, 0x2c403c, 0x2c4040, 0x2c3c40, 0x2c3440, 0x2c3040,
	0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000, 0x000000
};

} // End of namespace MADS
