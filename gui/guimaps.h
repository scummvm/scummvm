/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 */

#ifndef GUI_GUIMAPS_H
#define GUI_GUIMAPS_H

struct ResString {
	int num;
	char string[80];
};

// String maps
static const char* string_map_table_custom[] = { 
	"Master Volume :",	//0
	"Music Volume :",	//1
	"SFX Volume :",		//2
	"+",				//3
	"-",				//4
	"Sound",			//5
	"Keys",				//6
	"About",			//7
	"Pocket ScummVM",	//8
	"Build " SCUMMVM_VERSION " (" SCUMMVM_CVS ")",	//9
	"ScummVM http://scummvm.sourceforge.net",		//10
	"All games (c) LucasArts",						//11
	"Quit",											//12
	"Pause",										//13
	"Save",											//14
	"Skip",											//15
	"Hide",											//16
	"Options",									//17
	"Misc",											//18
	"Show speech subtitles",		//19
	"Amiga palette conversion",	//20
	"Except:",									//21
	"Simon the Sorcerer (c) Adventuresoft", //22
	"Close"											//23
};

static ResString string_map_table_v7[] = {
	{96, "game name and version"}, //that's how it's supposed to be
	{77, "Select a game to LOAD"},
	{76, "Name your SAVE game"},
        {70, "save"}, //boot8
	{71, "load"}, //boot9
	{72, "play"}, //boot10
	{73, "cancel"}, //boot11
	{74, "quit"}, //boot12
	{75, "ok"}, //boot13
	{85, "game paused"}, // boot3					
	
	/* this is the almost complete string map for v7
	{63, "how may I serve you?"},
	{64, "the dig v1.0"}, //(game name/version)
	{67, "text display only"},
	{68, "c:\\dig"}, //boot007 (save path ?)
	{69, "the dig"}, //boot21 (game name)
	{70, "save"}, //boot8
	{71, "load"}, //boot9
	{72, "play"}, //boot10
	{73, "cancel"}, //boot11
	{74, "quit"}, //boot12
	{75, "ok"}, //boot13
	{76, "name your save game"}, //boot19
	{77, "select a game to load"}, //boot20
	{78, "you must enter a name"},//boot14
	{79, "saving '%s'"}, //boot17
	{80, "loading '%s'"}, //boot18
	{81, "the game was NOT saved"}, //boot15
	{82, "the game was NOT loaded"}, //boot16
	{83, "how may I serve you?"},
	{84, "how may I serve you?"},
	{85, "game paused"}, // boot3
	{86, "Are you sure you want to restart"},
	{87, "Are you sure you want to quit?"}, //boot05
	{89, "how may I serve you?"},
	{90, "music"}, //boot22
	{91, "voice"}, //boot23
	{92, "sfx"}, //boot24
	{93, "disabled"}, //boot25
	{94, "text speed"}, //boot26
	{95, "text display"}, //boot27
	{96, "the dig v1.0"},*/
	
};

static ResString string_map_table_v6[] = {
	{117, "How may I serve you?"}, 
	{109, "Select a game to LOAD"}, 
	{108, "Name your SAVE game"}, 
	{96, "Save"}, 
	{97, "Load"}, 
	{98, "Play"}, 
	{99, "Cancel"}, 
	{100, "Quit"}, 
	{101, "Ok"}, 
	{93, "Game paused"}, 
};

static ResString string_map_table_v5[] = {
	{28, "How may I serve you?"}, 
	{20, "Select a game to LOAD"},
	{19, "Name your SAVE game"},
	{7, "Save"},
	{8, "Load"},
	{9, "Play"},
	{10, "Cancel"},
	{11, "Quit"},
	{12, "Ok"},
	{4, "Game paused"}
};

#endif
