/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef M4_RIDDLE_RIDDLE_H
#define M4_RIDDLE_RIDDLE_H

#include "m4/m4.h"
#include "m4/riddle/rooms/section2/section2.h"
#include "m4/riddle/rooms/section3/section3.h"
#include "m4/riddle/rooms/section4/section4.h"
#include "m4/riddle/rooms/section5/section5.h"
#include "m4/riddle/rooms/section6/section6.h"
#include "m4/riddle/rooms/section7/section7.h"
#include "m4/riddle/rooms/section8/section8.h"
#include "m4/riddle/rooms/section9/section9.h"
#include "m4/riddle/gui/gui_messages.h"

namespace M4 {
namespace Riddle {

class RiddleEngine : public M4Engine {
private:
	Rooms::Section1 _section1;
	Rooms::Section2 _section2;
	Rooms::Section3 _section3;
	Rooms::Section4 _section4;
	Rooms::Section5 _section5;
	Rooms::Section6 _section6;
	Rooms::Section7 _section7;
	Rooms::Section8 _section8;
	Rooms::Section9 _section9;
	GUI::MessageLog _messageLog;
	int _ripAction = 0;
	int _savedFacing = 0;

	/**
	 * Combines the items in the player _verb and _noun,
	 * removing them from the player's inventory and replacing
	 * them with the new combined item.
	 * @param newItem	New item to give to player
	 */
	void combineItems(const char *newItem);

	/**
	 * Splits apart a combined item
	*/
	void splitItems(const char *item1, const char *item2);

	void showMessageLog(int trigger);
	void lookAtInventoryItem();

protected:
	/**
	 * Creates the structure that holds all the global variables
	 */
	M4::Vars *createVars() override;

	/**
	 * Sets up the debugging console
	 */
	void setupConsole() override;

public:
	RiddleEngine(OSystem *syst, const M4GameDescription *gameDesc);
	~RiddleEngine() override {}

	/**
	 * Show the engine information
	 */
	void showEngineInfo() override;

	void syncFlags(Common::Serializer &s) override;

	void global_daemon() override;
	void global_parser() override;
};

extern void sketchInJournal(const char *digiName);

} // namespace Riddle
} // namespace M4

#endif
