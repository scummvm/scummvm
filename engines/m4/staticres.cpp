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

#include "m4/staticres.h"

namespace M4 {

const char *englishMADSArticleList[9] = {
	NULL, "with", "to", "at", "from", "on", "in", "under", "behind"
};

const char *cheatingEnabledDesc[3] = {
	"CHEATING ENABLED",
	"(For your convenience).",
	NULL
};

const char *atStr = "at";
const char *lookAroundStr = "Look around";
const char *toStr = "to ";
const char *useStr = "Use ";
const char *walkToStr = "Walk to ";
const char *fenceStr = "fence";
const char *overStr = "over";

const char *GameReleaseInfoStr = "ScummVM rev: 8.43 14-Sept-92";
const char *GameReleaseTitleStr = "GAME RELASE VERSION INFO";

VerbInit verbList[10] = {
	{kVerbLook, 2, 0},
	{kVerbTake, 2, 0},
	{kVerbPush, 2, 0},
	{kVerbOpen, 2, 0},
	{kVerbPut, 1, -1},
	{kVerbTalkTo, 2, 0},
	{kVerbGive, 1, 2},
	{kVerbPull, 2, 0},
	{kVerbClose, 2, 0},
	{kVerbThrow, 1, 3}
};

} // End of namespace M4
