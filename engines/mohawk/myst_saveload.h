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
 * $URL$
 * $Id$
 *
 */

#ifndef MYST_SAVELOAD_H
#define MYST_SAVELOAD_H

#include "common/savefile.h"
#include "common/file.h"
#include "common/str.h"

namespace Mohawk {

// These are left as uint16 currently, rather than
// being changed to bool etc. to save memory.
// This is because the exact structure
// is subject to change and the code to implement
// opcodes to access them is simpler this way..
struct MystVariables {
	MystVariables() { memset(this, 0, sizeof(MystVariables)); }

	/* 8 Game Global Variables :
	   0 = Unknown - Fixed at 2
	   1 = Current Age / Stack
	   2 = Page Being Held
	   3 = Unknown - Fixed at 1
	   4 = Slide Transistions
	   5 = Zip Mode
	   6 = Red Pages in Book
	   7 = Blue Pages in Book
	*/
	uint16 game_globals[8];

	/* 50 Myst Specific Variables :
	   0  = Marker Switch Near Cabin
	   1  = Marker Switch Near Clock Tower
	   2  = Marker Switch on Dock
	   3  = Marker Switch Near Ship Pool
	   4  = Marker Switch Near Cogs
	   5  = Marker Switch Near Generator Room
	   6  = Marker Switch Near Stellar Observatory
	   7  = Marker Switch Near Rocket Ship
	   8  = Fireplace, Opened Green Book Before
	   9  = Ship State
	   10 = Cabin Gas Valve Position
	   11 = Clock Tower Hour Hand Position
	   12 = Clock Tower Minute Hand Position
	   13 = Clock Tower Puzzle Solved / Cogs Open
	   14 = Clock Tower Gear Bridge
	   15 = Generator Breaker State
	   16 = Generator Button State
	   17 = Generator Voltage State
	   18 = Library Bookcase Door
	   19 = Dock Imager Numeric Selection
	   20 = Dock Imager Active
	   21 = Unknown #1 - Fixed at 0
	   22 = Unknown #2 - Fixed at 0
	   23 = Unknown #3 - Fixed at 0
	   24 = Unknown #4 - Fixed at 0
	   25 = Tower Rotation Angle
	   26 = Boxes For Ship Float Puzzle
	   27 = Tree Boiler Pilot Light Lit
	   28 = Stellar Observatory Viewer, Control Setting Day
	   29 = Stellar Observatory Lights
	   30 = Stellar Observatory Viewer, Control Setting Month
	   31 = Stellar Observatory Viewer, Control Setting Time
	   32 = Stellar Observatory Viewer, Control Setting Year
	   33 = Stellar Observatory Viewer, Target Day
	   34 = Stellar Observatory Viewer, Target Month
	   35 = Stellar Observatory Viewer, Target Time
	   36 = Stellar Observatory Viewer, Target Year
 	   37 = Cabin Safe Combination
	   38 = Channelwood Tree Position
	   39 = Checksum? #1
	   40 = Checksum? #2
	   41 = Rocketship Music Puzzle Slider #1 Position
	   42 = Rocketship Music Puzzle Slider #2 Position
	   43 = Rocketship Music Puzzle Slider #3 Position
	   44 = Rocketship Music Puzzle Slider #4 Position
	   45 = Rocketship Music Puzzle Slider #5 Position
	   46 = Unknown #5
	   47 = Unknown #6
	   48 = Unknown #7
	   49 = Unknown #8
	*/
	uint16 myst_vars[50];

	/* 7 Channelwood Specific Variables :
	    0 = Water Pump Bridge State
	    1 = Lower Walkway to Upper Walkway Elevator State
	    2 = Lower Walkway to Upper Walkway Spiral Stair Lower Door State
	    3 = Extendable Pipe State
	    4 = Water Valve States
	    5 = Achenar's Holoprojector Selection
	    6 = Lower Walkway to Upper Walkway Spiral Stair Upper Door State
	*/
	uint16 channelwood_vars[7];

	/* 8 Mech Specific Variables :
	    0 = Achenar's Room Secret Panel State
	    1 = Sirrus' Room Secret Panel State
	    2 = Fortress Staircase State
	    3 = Fortress Elevator Rotation
	    4 = Code Lock Shape #1 (Left)
	    5 = Code Lock Shape #2
	    6 = Code Lock Shape #3
	    7 = Code Lock Shape #4 (Right)
	*/
	uint16 mech_vars[8];

	/* 18 Selenitic Specific Variables :
	    0 = Sound Pickup At Water Pool
	    1 = Sound Pickup At Volcanic Crack
	    2 = Sound Pickup At Clock
	    3 = Sound Pickup At Crystal Rocks
	    4 = Sound Pickup At Windy Tunnel
	    5 = Sound Receiver Doors
	    6 = Windy Tunnel Lights
	    7 = Sound Receiver Current Input
	    8 = Sound Receiver Input #0 (Water Pool) Angle Value
	    9 = Sound Receiver Input #1 (Volcanic Crack) Angle Value
	   10 = Sound Receiver Input #2 (Clock) Angle Value
	   11 = Sound Receiver Input #3 (Crystal Rocks) Angle Value
	   12 = Sound Receiver Input #4 (Windy Tunnel) Angle Value
	   13 = Sound Lock Slider #1 (Left) Position
	   14 = Sound Lock Slider #2 Position
	   15 = Sound Lock Slider #3 Position
	   16 = Sound Lock Slider #4 Position
	   17 = Sound Lock Slider #5 (Right) Position
	*/
	uint16 selenitic_vars[18];

	/* 14 Stoneship Specific Variables :
	    0 = Light State
	    1 = Unknown #1
	    2 = Unknown #2
	    3 = Water Pump State
	    4 = Lighthouse Trapdoor State
	    5 = Lighthouse Chest Water State
	    6 = Lighthouse Chest Valve State
	    7 = Lighthouse Chest Open State
	    8 = Lighthouse Trapdoor Key State
	    9 = Lighthouse Generator Power Level(?)
	   10 = Lighthouse Generator Power...?
	   11 = Lighthouse Generator Power Good
	   12 = Lighthouse Generator Power #1 ?
	   13 = Lighthouse Generator Power #2?
	*/
	uint16 stoneship_vars[14];

	/* 1 Dunny Specific Variable :
	    0 = Outcome State
	*/
	uint16 dunny_vars[1];

    // The values in these regions seem to be lists of resource IDs
    // which correspond to VIEW resources i.e. cards
	uint16 unknown_myst[31];

	uint16 unknown_channelwood[37];

	uint16 unknown_mech[18];

	uint16 unknown_selenitic[30];

	uint16 unknown_stoneship[22];
};

class MohawkEngine_Myst;

class MystSaveLoad {
public:
	MystSaveLoad(MohawkEngine_Myst*, Common::SaveFileManager*);
	~MystSaveLoad();

	Common::StringList generateSaveGameList();
	bool loadGame(Common::String);
	bool saveGame(Common::String);
	void deleteSave(Common::String);

	void initMystVariables(MystVariables *_tv);
	void debug_printMystVariables(MystVariables *_tv);
private:
	MohawkEngine_Myst *_vm;
	Common::SaveFileManager *_saveFileMan;
	MystVariables *_v;
};

} // End of namespace Mohawk

#endif
