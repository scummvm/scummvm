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
#include "adl/graphics.h"
#include "adl/disk.h"

namespace Common {
class ReadStream;
struct Point;
}

namespace Adl {

#define IDS_HR1_EXE_0    "AUTO LOAD OBJ"
#define IDS_HR1_EXE_1    "ADVENTURE"
#define IDS_HR1_LOADER   "MYSTERY.HELLO"
#define IDS_HR1_MESSAGES "MESSAGES"

#define IDI_HR1_NUM_ROOMS         41
#define IDI_HR1_NUM_PICS          97
#define IDI_HR1_NUM_VARS          20
#define IDI_HR1_NUM_ITEM_OFFSETS  21
#define IDI_HR1_NUM_MESSAGES     167

// Messages used outside of scripts
#define IDI_HR1_MSG_CANT_GO_THERE      137
#define IDI_HR1_MSG_DONT_UNDERSTAND     37
#define IDI_HR1_MSG_ITEM_DOESNT_MOVE   151
#define IDI_HR1_MSG_ITEM_NOT_HERE      152
#define IDI_HR1_MSG_THANKS_FOR_PLAYING 140
#define IDI_HR1_MSG_DONT_HAVE_IT       127
#define IDI_HR1_MSG_GETTING_DARK         7

#define IDI_HR1_OFS_STR_ENTER_COMMAND   0x5bbc
#define IDI_HR1_OFS_STR_VERB_ERROR      0x5b4f
#define IDI_HR1_OFS_STR_NOUN_ERROR      0x5b8e
#define IDI_HR1_OFS_STR_PLAY_AGAIN      0x5f1e
#define IDI_HR1_OFS_STR_CANT_GO_THERE   0x6c0a
#define IDI_HR1_OFS_STR_DONT_HAVE_IT    0x6c31
#define IDI_HR1_OFS_STR_DONT_UNDERSTAND 0x6c51
#define IDI_HR1_OFS_STR_GETTING_DARK    0x6c7c
#define IDI_HR1_OFS_STR_PRESS_RETURN    0x5f68

#define IDI_HR1_OFS_PD_TEXT_0    0x005d
#define IDI_HR1_OFS_PD_TEXT_1    0x012b
#define IDI_HR1_OFS_PD_TEXT_2    0x016d
#define IDI_HR1_OFS_PD_TEXT_3    0x0259

#define IDI_HR1_OFS_INTRO_TEXT   0x0066
#define IDI_HR1_OFS_GAME_OR_HELP 0x000f

#define IDI_HR1_OFS_LOGO_0       0x1003
#define IDI_HR1_OFS_LOGO_1       0x1800

#define IDI_HR1_OFS_ITEMS        0x0100
#define IDI_HR1_OFS_ROOMS        0x050a
#define IDI_HR1_OFS_PICS         0x4b03
#define IDI_HR1_OFS_CMDS_0       0x3c00
#define IDI_HR1_OFS_CMDS_1       0x3d00

#define IDI_HR1_OFS_ITEM_OFFSETS 0x68ff
#define IDI_HR1_OFS_CORNERS      0x4f00

#define IDI_HR1_OFS_VERBS        0x3800
#define IDI_HR1_OFS_NOUNS        0x0f00

class HiRes1Engine : public AdlEngine {
public:
	HiRes1Engine(OSystem *syst, const AdlGameDescription *gd) :
			AdlEngine(syst, gd),
			_files(nullptr),
			_messageDelay(true) { }
	~HiRes1Engine() { delete _files; }

private:
	// AdlEngine
	void runIntro() const;
	void init();
	void initState();
	void restartGame();
	void drawPic(byte pic, Common::Point pos = Common::Point()) const;
	void printString(const Common::String &str);
	void printMessage(uint idx);
	void drawItem(const Item &item, const Common::Point &pos) const;
	void loadRoom(byte roomNr);
	void showRoom();

	void wordWrap(Common::String &str) const;

	Files *_files;
	Common::File _exe;
	Common::Array<DataBlockPtr> _corners;
	Common::Array<byte> _roomDesc;
	bool _messageDelay;
};

} // End of namespace Adl

#endif
