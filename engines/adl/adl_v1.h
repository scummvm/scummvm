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

#ifndef ADL_ADL_V1_H
#define ADL_ADL_V1_H

#include "adl/adl.h"

namespace Common {
class ReadStream;
}

namespace Adl {

enum {
	// Some of these are probably common
	STR_MH_DIRERR = STR_CUSTOM_START,
	STR_MH_DONTHAVEIT,
	STR_MH_DONTUNDERSTAND,
	STR_MH_GETTINGDARK,
	STR_MH_PLAYAGAIN,

	STR_MH_TOTAL
};

class HiRes1Engine : public AdlEngine {
public:
	HiRes1Engine(OSystem *syst, const AdlGameDescription *gd);
	Common::String getExeString(uint idx);

protected:
	void runGame();

private:
	enum {
		MH_ROOMS = 42,
		MH_PICS = 98,
		MH_ITEM_OFFSETS = 21
	};

	void runIntro();
	void drawPic(Common::ReadStream &stream, byte xOffset, byte yOffset);
	void showRoom();
	void printMessage(uint idx, bool wait = true);
	void wordWrap(Common::String &str);
	void readCommands(Common::ReadStream &stream, Commands &commands);
	bool checkCommand(const Command &command, byte verb, byte noun);
	bool doOneCommand(const Commands &commands, byte verb, byte noun);
	void doAllCommands(const Commands &commands, byte verb, byte noun);
	void doActions(const Command &command, byte noun, byte offset);
	void clearScreen();
	void takeItem(byte noun);
	void dropItem(byte noun);
	void drawItems();
	void drawPic(byte pic, byte xOffset, byte yOffset);

	Common::Array<Common::String> _exeStrings;
};

} // End of namespace Adl

#endif
