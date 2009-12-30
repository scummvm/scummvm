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

#include "mohawk/myst.h"
#include "mohawk/myst_vars.h"

namespace Mohawk {

// The Myst variable system is complex, and the structure is fairly
// unknown. The idea of this class is to abstract the variable references
// from the storage structure until the structure is clear enough that
// the complexity of this abstraction can be removed.

// The exact organization of local/global, persistent/non-persistent
// age-specific mapping etc. is currently unknown.

MystVarEntry introVars[] = {
	{   0, 7, "Age To Link To" } // 0 to 7
		// 0 = Selenitic
		// 1 = Stoneship
		// 2 = Myst (Library Ceiling)
		// 3 = Mechanical
		// 4 = Channelwood
		// 5 = Myst (Start of Movie - Over Sea)
		// 6 = D'ni
		// 7 = Myst (End of Movie - On Dock)
};

MystVarEntry seleniticVars[] = {
	{   0, 0, "Sound Pickup At Windy Tunnel" }, // 0 to 1 // TODO: Multiple Uses of Var 0?
	{   1, 0, "Sound Pickup At Volcanic Crack" }, // 0 to 1 
	{   2, 0, "Sound Pickup At Clock" }, // 0 to 1 
	{   3, 0, "Sound Pickup At Water Pool" }, // 0 to 1 
	{   4, 0, "Sound Pickup At Crystal Rocks" }, // 0 to 1 
	{   5, 0, "Sound Receiver Doors" }, // 0 to 1 
	{   6, 0, "Windy Tunnel Lights" }, // 0 to 1 
	{   7, 0, "Maze Runner Porthole View" }, // 0 to 3
	{   8, 0, "Sound Receiver Screen (Control Variable?)" },
	{   9, 0, "Sound Receiver Water Pool Button Selected" },
	{  10, 0, "Sound Receiver Volcanic Crack Button Selected" },
	{  11, 0, "Sound Receiver Clock Button Selected" },
	{  12, 0, "Sound Receiver Crystal Rocks Button Selected" },
	{  13, 0, "Sound Receiver Windy Tunnel Button Selected" },
	{  14, 0, "Sound Receiver LED Digit #0 (Left)" },
	{  15, 0, "Sound Receiver LED Digit #1" },
	{  16, 0, "Sound Receiver LED Digit #2" },
	{  17, 0, "Sound Receiver LED Digit #3 (Right)" },
	{  18, 0, "Sound Receiver Green Arrow - Right" },
	{  19, 0, "Sound Receiver Green Arrow - Left" },
	{  20, 0, "Sound Lock Slider #1 (Left) (Position?)" },
	{  21, 0, "Sound Lock Slider #2 (Position?)" },
	{  22, 0, "Sound Lock Slider #3 (Position?)" },
	{  23, 0, "Sound Lock Slider #4 (Position?)" },
	{  24, 0, "Sound Lock Slider #5 (Right) (Position?)" },
	{  25, 0, "Maze Runner Compass Heading" }, // 0 to 8
	{  26, 0, "Sound Receiver Sum Button Selected" },
	{  27, 0, "Maze Runner Red Warning Light" },
	{  28, 0, "Sound Lock Button State" },
	{  29, 0, "Maze Runner Door Button State" },
	{  30, 0, "Maze Runner Door State" },
	{  31, 0, "Maze Runner Forward Button Lit" }, // 0 to 2
	{  32, 0, "Maze Runner Left & Right Button Lit" }, // 0 to 2
	{  33, 0, "Maze Runner Backtrack Button Lit" }, // 0 to 2
	{ 102, 1, "Red Page in Age" }, // 0 to 1
	{ 103, 1, "Blue Page in Age" } // 0 to 1
};

MystVarEntry stoneshipVars[] = {
	{   0, 0, "Water Pump Button #3 (Right) / Lighthouse Water Drained" }, // 0 to 2 = Button Up & Unlit, Button Down & Lit, Button Up & Lit
	{   1, 0, "Water Pump Button #2 / Tunnels To Brothers Rooms Drained" }, // 0 to 2 = Button Up & Unlit, Button Down & Lit, Button Up & Lit
	{   2, 0, "Water Pump Button #1 (Left) / Ship Cabin Water Drained" }, // 0 to 2 = Button Up & Unlit, Button Down & Lit, Button Up & Lit
	{   3, 0, "Lighthouse (Water or Chest Floating?)" }, // 0 to 1, Used by Far View
	{   4, 0, "Lighthouse Water/Chest State" }, // 0 to 2 = Water, No Water, Water & Chest Floating
	{   5, 2, "Lighthouse Trapdoor State" }, // 0 to 2 = Closed & Unlocked, Open, Closed & Locked
	{   6, 0, "Lighthouse Chest Valve Position" }, // 0 to 1
	{   7, 0, "Lighthouse Chest Unlocked" }, // 0 to 1
	{   8, 0, "Lighthouse Chest Key Position?" }, // 0 to 2, 2 = Bottom of Lighthouse
// Var 9 Unused
	{  10, 1, "Lighthouse Chest Full Of Water" }, // 0 to 1
	{  11, 3, "Lighthouse Key State" }, // 0 to 3 = Closed?, Open & No Key, Open & No Key, Open & Key
	{  12, 0, "Lighthouse Trapdoor - Holding Key" }, // 0 to 1
	{  13, 0, "State of Water in Tunnels to Brothers' Rooms" }, // 0 to 2 = Dark & Water, Dark & Drained, Lit & Water
	{  14, 0, "Tunnels to Brothers' Rooms Lit" }, // 0 to 1
	{  15, 0, "Side Door in Tunnels To Brother's Rooms Open" }, // 0 to 1
	{  16, 0, "Underwater Light Lit" }, // 0 to 1
	{  17, 0, "Sirrus' Room Drawer with Drugs Open" }, // 0 to 1
	{  18, 0, "Brother Room Door Open" }, // 0 to 1, Used for Door Slam
	{  19, 0, "Brother Room Door State" }, // 0 to 2 = Closed, Open & Dark, Open & Lit
	{  20, 0, "Ship Cabin Myst Book Present" }, // 0 to 1
	{  21, 0, "Brothers Rooms' Chest Of Drawers Drawer State" }, // 0 to 6 (Card 2197) or 0 to 7 (Card 2004)
	{  22, 0, "Sirrus' Chest of Drawers Drawer #1 (Top) Open" }, // 0 to 1
	{  23, 0, "Sirrus' Chest of Drawers Drawer #2 Open" }, // 0 to 1
	{  24, 0, "Sirrus' Chest of Drawers Drawer #3 Open" }, // 0 to 1
// Var 25 Unused - Replaced by Var 35.
	{  26, 0, "Sirrus' Chest of Drawers Left Small Drawer Open" }, // 0 to 1
	{  27, 0, "Sirrus' Chest of Drawers Right Small Drawer Open" }, // 0 to 1
	{  28, 0, "Telescope View Position" }, // Range Unknown.. 0 to 360?
	{  29, 0, "Achenar's Room Rose/Skull Hologram Button Lit" }, // 0 to 1
	{  30, 2, "Light State in Tunnel to Compass Rose Room" }, // 0 to 2 = Lit & Underwater Light Lit, Lit, Dark
	{  31, 0, "Lighthouse Lamp Room Battery Pack Indicator Light" }, // 0 to 1
	{  32, 0, "Lighthouse Lamp Room Battery Pack Meter Level" }, // Range Unknown.. // Must be 1 to vertical size of image...
	{  33, 0, "State of Side Door in Tunnels to Compass Rose Room (Power?)" }, // 0 to 2 = Closed (No Power), Closed (Power), Open
	{  34, 1, "Achenar's Room Drawer with Torn Note Closed" }, // 0 to 1
	{  35, 2, "Sirrus' Room Drawer #4 (Bottom) Open and Red Page State" }, // 0 to 2 = Open, Open with Page, Closed
	{  36, 0, "Ship Cabin Door State" }, // 0 to 2 = Closed, Open & Dark, Open & Lit
	{ 102, 1, "Red Page in Age" }, // 0 to 1
	{ 103, 1, "Blue Page in Age" }, // 0 to 1
	{ 105, 0, "Ship Cabin Door State" }
};

MystVarEntry mystVars[] = {
	{   0, 1, "Myst Library Bookcase Closed / Library Exit Open" }, // 0 to 1 // TODO: Multiple Uses of Var 0?
	{   1, 0, "Myst Library Bookcase Open / Library Exit Blocked" }, // 0 to 1
	{   2, 0, "Marker Switch Near Cabin" }, // 0 to 1
	{   3, 0, "Marker Switch Near Clock Tower" }, // 0 to 1
	{   4, 0, "Marker Switch on Dock" }, // 0 to 1
	{   5, 0, "Marker Switch Near Ship Pool" }, // 0 to 1
	{   6, 0, "Marker Switch Near Cogs" }, // 0 to 1
	{   7, 0, "Marker Switch Near Generator Room" }, // 0 to 1
	{   8, 0, "Marker Switch Near Stellar Observatory" }, // 0 to 1
	{   9, 0, "Marker Switch Near Rocket Ship" }, // 0 to 1
	{  10, 0, "Ship Floating State" }, // 0 to 1
	{  11, 0, "Cabin Door Open State" },
	{  12, 0, "Clock Tower Gear Bridge" }, // 0 to 1
	{  13, 0, "Tower Elevator Sound Control" }, // 0 to 1
	{  14, 0, "Tower Solution (Key) Plaque" }, // 0 to 4
		// 0 = None
		// 1 = 59 V
		// 2 = 2:40 2-2-1
		// 3 = October  11, 1984 10:04 AM
		//     January  17, 1207  5:46 AM
		//     November 23, 9791  6:57 PM
		// 4 = 7, 2, 4
	{  15, 0, "Tower Window (Book) View" }, // 0 to 6
		// 0 = Wall
		// 1 = Rocketship
		// 2 = Cogs Closed
		// 3 = Ship Sunk
		// 4 = Channelwood Tree
		// 5 = Ship Floating
		// 6 = Cogs Open
	{  16, 0, "Tower Window (Book) View From Ladder Top" }, // 0 to 2
		// 0 = Wall
		// 1 = Sky
		// 2 = Sky with Channelwood Tree
	{  17, 0, "Fireplace Grid Row #1 (Top)" }, // Bitfield 0x00 to 0xFF
	{  18, 0, "Fireplace Grid Row #2" }, // Bitfield 0x00 to 0xFF
	{  19, 0, "Fireplace Grid Row #3" }, // Bitfield 0x00 to 0xFF
	{  20, 0, "Fireplace Grid Row #4" }, // Bitfield 0x00 to 0xFF
	{  21, 0, "Fireplace Grid Row #5" }, // Bitfield 0x00 to 0xFF
	{  22, 0, "Fireplace Grid Row #6 (Bottom)" }, // Bitfield 0x00 to 0xFF
	{  23, 0, "Fireplace Pattern Correct" }, // 0 to 1
	{  24, 1, "Fireplace Blue Page Present" }, // 0 to 1
	{  25, 1, "Fireplace Red Page Present" }, // 0 to 1
	{  26, 0, "Ship Puzzle Box Image (Cross)" }, // 0 to 2
	{  27, 0, "Ship Puzzle Box Image (Leaf)" }, // 0 to 2
	{  28, 0, "Ship Puzzle Box Image (Arrow)" }, // 0 to 2
	{  29, 0, "Ship Puzzle Box Image (Eye)" }, // 0 to 2
	{  30, 0, "Ship Puzzle Box Image (Snake)" }, // 0 to 2
	{  31, 0, "Ship Puzzle Box Image (Spider)" }, // 0 to 2
	{  32, 0, "Ship Puzzle Box Image (Anchor)" }, // 0 to 2
	{  33, 0, "Ship Puzzle Box Image (Ostrich)" }, // 0 to 2
	{  34, 2, "Dock Forechamber Imager State" }, // 0 to 2 = Off, Mountain, Water
	{  35, 5, "Dock Forechamber Imager Control Left Digit" }, // 0 to 9
	{  36, 6, "Dock Forechamber Imager Control Right Digit" }, // 0 to 9
	{  37, 0, "Clock Tower Control Wheels Position" }, // 0 to 8
// Var 38 Unused
// 39 = TODO: ?
	{  40, 0, "Cog Close/Open State" },
	{  41, 0, "Dock Marker Switch Vault State" }, // 0 to 2 = Closed, Open & Page Taken, Open & Page Present
// Var 42 Unused
	{  43, 0, "Clock Tower Time" }, // 0 to 143
	{  44, 0, "Rocket Ship Power State" }, // 0 to 2 = None, Insufficient, Correct
	{  45, 1, "Dock Forechamber Imager Water Effect Enabled" }, // 0 to 1
	{  46, 0, "Number Of Pages in Red Book" }, // 0 to 6 = 0-5, Extra
	{  47, 0, "Number Of Pages in Blue Book" }, // 0 to 6 = 0-5, Extra
	{  48, 0, "Marker Switch on Dock - Duplicate of Var #4?" }, // 0 to 2
	{  49, 0, "Generator Running" }, // Boolean used for Sound..
// 50 = TODO: ?
	{  51, 2, "Forechamber Imager Movie Control Variable" }, // 0 to 4 = Blank, No Function? / Mountain?, Water, Atrus, Marker Switch
	{  52, 0, "Generator Switch #1" },
	{  53, 0, "Generator Switch #2" },
	{  54, 0, "Generator Switch #3" },
	{  55, 0, "Generator Switch #4" },
	{  56, 0, "Generator Switch #5" },
	{  57, 0, "Generator Switch #6" },
	{  58, 0, "Generator Switch #7" },
	{  59, 0, "Generator Switch #8" },
	{  60, 0, "Generator Switch #9" },
	{  61, 0, "Generator Switch #10" },
	{  62, 0, "Generator Power Dial Left LED Digit" }, // 0 to 9
	{  63, 0, "Generator Power Dial Right LED Digit" }, // 0 to 9
	{  64, 0, "Generator Power To Spaceship Dial Left LED Digit" }, // 0 to 9
	{  65, 0, "Generator Power To Spaceship Dial Right LED Digit" }, // 0 to 9
	{  66, 0, "Generator Room Lights On" }, // Boolean
	{  67, 9, "Cabin Safe Lock Number #1 - Left" },
	{  68, 9, "Cabin Safe Lock Number #2" },
	{  69, 9, "Cabin Safe Lock Number #3 - Right" },
	{  70, 0, "Cabin Safe Matchbox State" }, // 0 to 2
	{  71, 1, "Stellar Observatory Lights" },
	{  72, 0, "Channelwood Tree Position" }, // 0 to 12, 4 for Alcove
	{  73, 9, "Stellar Observatory Telescope Control - Month" }, // 0 to 11, Not in order...
		// 0 = OCT, 1 = NOV, 2 = DEC, 3 = JUL,  4 = AUG,  5 = SEP
		// 6 = APR, 7 = MAY, 8 = JUN, 9 = JAN, 10 = FEB, 11 = MAR
	{  74, 10, "Stellar Observatory Telescope Control - Day Digit #1 (Left)" }, // 0 to 10 = 0 to 9, Blank
	{  75, 1, "Stellar Observatory Telescope Control - Day Digit #2 (Right)" }, // 0 to 10 = 0 to 9, Blank
	{  76, 10, "Stellar Observatory Telescope Control - Year Digit #1 (Left)" }, // 0 to 10 = 0 to 9, Blank
	{  77, 10, "Stellar Observatory Telescope Control - Year Digit #2" }, // 0 to 10 = 0 to 9, Blank
	{  78, 10, "Stellar Observatory Telescope Control - Year Digit #3" }, // 0 to 10 = 0 to 9, Blank
	{  79, 0, "Stellar Observatory Telescope Control - Year Digit #4 (Right)" }, // 0 to 10 = 0 to 9, Blank
	{  80, 1, "Stellar Observatory Telescope Control - Hour Digit #1 (Left)" }, // 0 to 10 = 0 to 9, Blank
	{  81, 2, "Stellar Observatory Telescope Control - Hour Digit #2 (Right)" }, // 0 to 10 = 0 to 9, Blank
	{  82, 0, "Stellar Observatory Telescope Control - Minute Digit #1 (Left)" }, // 0 to 10 = 0 to 9, Blank
	{  83, 0, "Stellar Observatory Telescope Control - Minute Digit #2 (Right)" }, // 0 to 10 = 0 to 9, Blank
// 84 to 87 = TODO: ?
	{  88, 0, "Stellar Observatory Telescope Control - AM/PM Indicator" }, // 0 = AM, 1 = PM
	{  89, 0, "Stellar Observatory Telescope Control - Slider #1 (Left)" }, // 0 to 2 = Not Present, Dark, Lit
	{  90, 0, "Stellar Observatory Telescope Control - Slider #2" }, // 0 to 2 = Not Present, Dark, Lit
	{  91, 0, "Stellar Observatory Telescope Control - Slider #3" }, // 0 to 2 = Not Present, Dark, Lit
	{  92, 0, "Stellar Observatory Telescope Control - Slider #4 (Right)" }, // 0 to 2 = Not Present, Dark, Lit
	{  93, 0, "Breaker nearest Generator Room Blown" },
	{  94, 0, "Breaker nearest Rocket Ship Blown" },
// 95 = TODO: ?
	{  96, 0, "Generator Power Dial Needle Position" }, // 0 to 24
	{  97, 0, "Generator Power To Spaceship Dial Needle Position" }, // 0 to 24
	{  98, 0, "Cabin Boiler Pilot Light Lit" },
	{  99, 0, "Cabin Boiler Gas Valve Position" }, // 0 to 5
	{ 100, 0, "Red Book Page State" }, // Bitfield
	{ 101, 0, "Blue Book Page State" }, // Bitfield
	{ 102, 1, "Red Page in Age" }, // 0 to 1
	{ 103, 1, "Blue Page in Age" }, // 0 to 1
// 104 = TODO: ?
	{ 105, 0, "Clock Tower Door / Ship Box Temp Value" },
	{ 106, 0, "Red / Blue Book State" }, // 0 to 4, 0-3 = Books Present 4 = Books Burnt
	{ 300, 0, "Rocket Ship Music Puzzle Slider State" },  // 0 to 2 = Not Present, Dark, Lit
	{ 301, 0, "Rocket Ship Piano Key Depressed" }, // 0 to 1
	{ 302, 0, "Green Book Opened Before Flag" }, // 0 to 1
	{ 303, 1, "Myst Library Bookcase Closed / Library Exit Open" },
	{ 304, 0, "Myst Library Image Present on Tower Rotation Map" }, // 0 to 1
	{ 305, 0, "Cabin Boiler Lit" },
	{ 306, 0, "Cabin Boiler Steam Sound Control" }, // 0 to 27
	{ 307, 0, "Cabin Boiler Needle Position i.e. Fully Pressurised" }, // 0 to 1
	{ 308, 0, "Cabin Safe Handle Position / Matchbox Temp Value" },
	{ 309, 0, "Red/Blue/Green Book Open" }, // 0 to 1
	{ 310, 0, "Dock Forechamber Imager Control Temp Value?" }
};

MystVarEntry mechVars[] = {
	{   0, 0, "Achenar's Room Secret Panel State" }, // TODO: Multiple Uses of Var 0?
	{   1, 0, "Sirrus's Room Secret Panel State" },
	{   2, 0, "Achenar's Secret Room Crate Lid Open and Blue Page Present" }, // 0 to 4
		// 0 = Lid Closed, Blue Page Present
		// 1 = Lid Closed, Blue Page Not Present
		// 2 = Lid Open, Blue Page Not Present
		// 3 = Lid Open, Blue Page Present
	{   3, 0, "Achenar's Secret Room Crate Lid Open" }, // 0 to 1
	{   4, 0, "Myst Book Staircase State" },
	{   5, 0, "Fortress Telescope View" },
	{   6, 0, "Large Cog Visible Through Distant Fortress Doorway" }, // 0 to 1
	{   7, 0, "Fortress Elevator Door Rotated to Open" }, // 0 to 1
// Var 8 Not Used
// Var 9 Not Used
	{  10, 0, "Fortress Staircase State" }, // 0 to 1
	{  11, 0, "Fortress Elevator Rotation Control - Position Indicator" }, // 0 to 9, 4 = Red Open Position
	{  12, 0, "Fortress Elevator Rotation Control - Top Cog Position" }, // 0 to 5
	{  13, 0, "Fortress Elevator Vertical Position - View" }, // 0 to 2, Used for View Logic on change from Card 6150
	{  14, 0, "Fortress Elevator Vertical Position - Button" }, // 0 to 2, Used for Button Logic on Card 6120
	{  15, 0, "Code Lock State" }, // 0 to 2
	{  16, 0, "Code Lock Shape #1 (Left)" },
	{  17, 0, "Code Lock Shape #2" },
	{  18, 0, "Code Lock Shape #3" },
	{  19, 0, "Code Lock Shape #4 (Right)" },
	{  20, 0, "Red Dodecahedron Lit" },
	{  21, 0, "Green Dodecahedron Lit" },
	{  22, 0, "Yellow Tetrahedron Lit" },
	{  23, 0, "In Elevator" }, // 0 to 1
	{ 102, 1, "Red Page in Age" }, // 0 to 1
	{ 103, 1, "Blue Page in Age" } // 0 to 1
};

MystVarEntry channelwoodVars[] = {
	{   0, 0, "Multiple Uses..." }, // TODO: Multiple Uses of Var 0?
	{   1, 0, "Water Pump Bridge State" }, // 0 to 1
	{   2, 0, "Lower Walkway to Upper Walkway Elevator State" }, // 0 to 1
	{   3, 0, "Water Flowing (R) to Pump for Upper Walkway to Temple Elevator" }, // 0 to 1
	{   4, 0, "Water Flowing (L, R, R, L, Pipe Extended) to Pump for Book Room Elevator" }, // 0 to 1
	{   5, 0, "Lower Walkway to Upper Walkway Spiral Stair Lower Door Open" }, // 0 to 1
	{   6, 0, "Pipe Bridge Extended" }, // 0 to 1
	{   7, 0, "Bridge Pump Running" }, // 0 to 1
	{   8, 0, "Water Tank Valve State" },  // 0 to 1 
	{   9, 0, "First Water Valve State" },  // 0 to 1 
	{  10, 0, "Second (L) Water Valve State" },  // 0 to 1
	{  11, 0, "Third (L, R) Water Valve State" },  // 0 to 1
	{  12, 0, "Fourth (L, R, R) Water Valve State" }, // 0 to 1
	{  13, 0, "Fourth (L, R, L) Water Valve State" }, // 0 to 1
	{  14, 0, "Third (L, L) Water Valve State" },  // 0 to 1
	{  15, 0, "Water Flowing (L, R, R, R) to Pump for Lower Walkway to Upper Walkway Elevator" }, // 0 to 1
	{  16, 0, "Lower Walkway to Upper Walkway Spiral Stair Upper Door Open" }, // 0 to 1
	{  17, 0, "Achenar's Holoprojector Selection" }, // 0 to 3
	{  18, 0, "Drawer in Sirrus' Room with Wine Bottles and Torn Note Open" }, // 0 to 1
	{  19, 0, "Water Flowing to First Water Valve" }, // 0 to 1
	{  20, 0, "Water Flowing to Second (L) Water Valve" }, // 0 to 1
	{  21, 0, "Water Flowing to Third (L, R) Water Valve" }, // 0 to 1
	{  22, 0, "Water Flowing to Fourth (L, R, R) Water Valve" }, // 0 to 1
	{  23, 0, "Water Flowing to Fourth (L, R, L) Water Valve" }, // 0 to 1
	{  24, 0, "Water Flowing to Third (L, L) Water Valve" }, // 0 to 1
	{  25, 0, "Water Flowing to Pipe Bridge (L, R, R, L)" }, // 0 to 1
	{  26, 0, "Water Flowing to Pipe At Entry Point (L, R, L, R)" }, // 0 to 1
	{  27, 0, "Water Flowing to Join and Pump Bridge (L, R, L, L)" }, // 0 to 1
	{  28, 0, "Water Flowing to Join and Pump Bridge (L, L, R)" }, // 0 to 1
	{  29, 0, "Water Flowing to Pipe In Water (L, L, L)" }, // 0 to 1
	{  30, 0, "Lower Walkway to Upper Walkway Elevator Door Open" }, // 0 to 1
	{  31, 0, "Water Flowing to Join (L, L, R)" }, // 0 to 2 = Stop Sound, Background, Background with Water Flow
	{  32, 0, "Water Flowing (L, R, R, L, Pipe) State" }, // 0 to 2 = Stop Sound, Background, Background with Water Flow
	{  33, 0, "Lower Walkway to Upper Walkway Spiral Stair Upper Door State" }, // 0 to 2 = Closed, Open, Open but slams behind you.
	{ 102, 1, "Red Page in Age" }, // 0 to 1
	{ 103, 1, "Blue Page in Age" }, // 0 to 1
	{ 105, 0, "Upper Walkway to Temple Elevator Door Open / Temple Iron Door Open" } // 0 to 1, used for slam sound
};

MystVarEntry dniVars[] = {
	{   0, 0, "Atrus Gone" }, // 0 to 1 // TODO: Multiple Uses of Var 0?
	{   1, 0, "Myst Book State" }, // 0 to 2 = Book Inactive, Book Unlinkable, Book Linkable
	{   2, 0, "Sound Control" }, // 0 to 2 = Win Tune, Lose Tune, Stop Sound
	{ 106, 0, "Atrus State Control" } // 0 to 4 = Atrus Writing, Atrus Holding Out Hand, Atrus Gone, Atrus #2, Atrus #2
};

MystVarEntry creditsVars[] = {
	{   0, 0, "Image Control" }, // 0 to 6
	{   1, 1, "Sound Control" } // 0 to 1 = Win Tune, Lose Tune
};

MystVar::MystVar(MohawkEngine_Myst *vm) {
	_vm = vm;
}

MystVar::~MystVar() {
}

// Only for use by Save/Load, all other code should use getVar()
uint16 MystVar::saveGetVar(uint16 stack, uint16 v) {
	uint16 value = 0;
	MystVarEntry unknownVar = { v, 0, "Unknown" };
	const char *desc = NULL;
	uint16 i;
	
	switch (stack) {
	case kIntroStack:
		for (i = 0; i < ARRAYSIZE(introVars); i++) {
			if (introVars[i].refNum == v) {
				value = introVars[i].storage;
				desc = introVars[i].description;
				break;
			}
		}
		break;
	case kSeleniticStack:
		for (i = 0; i < ARRAYSIZE(seleniticVars); i++) {
			if (seleniticVars[i].refNum == v) {
				value = seleniticVars[i].storage;
				desc = seleniticVars[i].description;
				break;
			}
		}
		break;
	case kStoneshipStack:
		for (i = 0; i < ARRAYSIZE(stoneshipVars); i++) {
			if (stoneshipVars[i].refNum == v) {
				value = stoneshipVars[i].storage;
				desc = stoneshipVars[i].description;
				break;
			}
		}
		break;
	case kDemoPreviewStack:
	case kMystStack:
		for (i = 0; i < ARRAYSIZE(mystVars); i++) {
			if (mystVars[i].refNum == v) {
				value = mystVars[i].storage;
				desc = mystVars[i].description;
				break;
			}
		}
		break;
	case kMechanicalStack:
		for (i = 0; i < ARRAYSIZE(mechVars); i++) {
			if (mechVars[i].refNum == v) {
				value = mechVars[i].storage;
				desc = mechVars[i].description;
				break;
			}
		}
		break;
	case kChannelwoodStack:
		for (i = 0; i < ARRAYSIZE(channelwoodVars); i++) {
			if (channelwoodVars[i].refNum == v) {
				value = channelwoodVars[i].storage;
				desc = channelwoodVars[i].description;
				break;
			}
		}
		break;
	case kDniStack:
		for (i = 0; i < ARRAYSIZE(dniVars); i++) {
			if (dniVars[i].refNum == v) {
				value = dniVars[i].storage;
				desc = dniVars[i].description;
				break;
			}
		}
		break;
	case kCreditsStack:
		for (i = 0; i < ARRAYSIZE(creditsVars); i++) {
			if (creditsVars[i].refNum == v) {
				value = creditsVars[i].storage;
				desc = creditsVars[i].description;
				break;
			}
		}
		break;
	default:
		break;
	}
	
	if (desc == NULL) {
		for (i = 0; i < _unknown.size(); i++) {
			if (_unknown[i].refNum == v) {
				value = _unknown[i].storage;
				desc = _unknown[i].description;
				break;
			}
		}

		if (desc == NULL) {
			warning("MystVar::getVar(%d): Unknown variable reference", v);			
			_unknown.push_back(unknownVar);
			desc = _unknown.back().description;
		}
	}

	debugC(kDebugVariable, "MystVar::getVar(%d = %s): %d", v, desc, value);
	return value;
}

// Only for use by Save/Load, all other code should use setVar()
void MystVar::loadSetVar(uint16 stack, uint16 v, uint16 value) {
	const char *desc = NULL;
	MystVarEntry unknownVar = { v, value, "Unknown" };
	uint16 i;
	
	switch (stack) {
	case kIntroStack:
		for (i = 0; i < ARRAYSIZE(introVars); i++) {
			if (introVars[i].refNum == v) {
				introVars[i].storage = value;
				desc = introVars[i].description;
				break;
			}
		}
		break;
	case kSeleniticStack:
		for (i = 0; i < ARRAYSIZE(seleniticVars); i++) {
			if (seleniticVars[i].refNum == v) {
				seleniticVars[i].storage = value;
				desc = seleniticVars[i].description;
				break;
			}
		}
		break;
	case kStoneshipStack:
		for (i = 0; i < ARRAYSIZE(stoneshipVars); i++) {
			if (stoneshipVars[i].refNum == v) {
				stoneshipVars[i].storage = value;
				desc = stoneshipVars[i].description;
				break;
			}
		}
		break;
	case kDemoPreviewStack:
	case kMystStack:
		for (i = 0; i < ARRAYSIZE(mystVars); i++) {
			if (mystVars[i].refNum == v) {
				mystVars[i].storage = value;
				desc = mystVars[i].description;
				break;
			}
		}
		break;
	case kMechanicalStack:
		for (i = 0; i < ARRAYSIZE(mechVars); i++) {
			if (mechVars[i].refNum == v) {
				mechVars[i].storage = value;
				desc = mechVars[i].description;
				break;
			}
		}
		break;
	case kChannelwoodStack:
		for (i = 0; i < ARRAYSIZE(channelwoodVars); i++) {
			if (channelwoodVars[i].refNum == v) {
				channelwoodVars[i].storage = value;
				desc = channelwoodVars[i].description;
				break;
			}
		}
		break;
	case kDniStack:
		for (i = 0; i < ARRAYSIZE(dniVars); i++) {
			if (dniVars[i].refNum == v) {
				dniVars[i].storage = value;
				desc = dniVars[i].description;
				break;
			}
		}
		break;
	case kCreditsStack:
		for (i = 0; i < ARRAYSIZE(creditsVars); i++) {
			if (creditsVars[i].refNum == v) {
				creditsVars[i].storage = value;
				desc = creditsVars[i].description;
				break;
			}
		}
		break;
	default:
		break;
	}

	if (desc == NULL) {
		for (i = 0; i < _unknown.size(); i++) {
			if (_unknown[i].refNum == v) {
				_unknown[i].storage = value;
				desc = _unknown[i].description;
				break;
			}
		}

		if (desc == NULL) {
			warning("MystVar::setVar(%d): Unknown variable reference", v);
			_unknown.push_back(unknownVar);
			desc = _unknown.back().description;
		}
	}

	debugC(kDebugVariable, "MystVar::setVar(%d = %s): %d", v, desc, value);
}

uint16 MystVar::getVar(uint16 v) {
	return this->saveGetVar(_vm->getCurStack(), v);
}

void MystVar::setVar(uint16 v, uint16 value) {
	this->loadSetVar(_vm->getCurStack(), v, value);
}

} // End of namespace Mohawk
