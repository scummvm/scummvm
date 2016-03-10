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

#ifndef ADL_HIRES1_H
#define ADL_HIRES1_H

#include "common/str.h"

#include "adl/adl.h"

namespace Common {
class ReadStream;
class Point;
}

namespace Adl {

#define IDS_HR2_DISK_IMAGE "WIZARD.DSK"

// Track, sector, offset
#define TSO(TRACK, SECTOR, OFFSET) (((TRACK) * 16 + (SECTOR)) * 256 + (OFFSET))
#define TS(TRACK, SECTOR) TSO(TRACK, SECTOR, 0)
#define T(TRACK) TS(TRACK, 0)

#define IDI_HR2_OFS_INTRO_TEXT TSO(0x00, 0xd, 0x17)
#define IDI_HR2_OFS_VERBS        T(0x19)
#define IDI_HR2_OFS_NOUNS       TS(0x22, 0x2)
#define IDI_HR2_OFS_ROOMS      TSO(0x21, 0x5, 0x0e) // Skip bogus room 0
#define IDI_HR2_OFS_MESSAGES   TSO(0x1f, 0x2, 0x04) // Skip bogus message 0

#define IDI_HR2_NUM_ROOMS 135
#define IDI_HR2_NUM_MESSAGES 254

// Messages used outside of scripts
#define IDI_HR2_MSG_CANT_GO_THERE      123
#define IDI_HR2_MSG_DONT_UNDERSTAND     19
#define IDI_HR2_MSG_ITEM_DOESNT_MOVE   242
#define IDI_HR2_MSG_ITEM_NOT_HERE        4
#define IDI_HR2_MSG_THANKS_FOR_PLAYING 239

#define IDI_HR2_OFS_STR_ENTER_COMMAND TSO(0x1a, 0x1, 0xbc)
#define IDI_HR2_OFS_STR_VERB_ERROR    TSO(0x1a, 0x1, 0x4f)
#define IDI_HR2_OFS_STR_NOUN_ERROR    TSO(0x1a, 0x1, 0x8e)
#define IDI_HR2_OFS_STR_PLAY_AGAIN    TSO(0x1a, 0x8, 0x25)
#define IDI_HR2_OFS_STR_PRESS_RETURN  TSO(0x1a, 0x8, 0x5f)

class HiRes2Engine : public AdlEngine {
public:
	HiRes2Engine(OSystem *syst, const AdlGameDescription *gd) : AdlEngine(syst, gd) { }

private:
	// AdlEngine
	void runIntro() const;
	void loadData();
	void initState();
	void restartGame();
	void drawPic(byte pic, Common::Point pos) const;
	void showRoom() const;
};

} // End of namespace Adl

#endif
