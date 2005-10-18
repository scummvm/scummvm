#include <PalmOS.h>
#include "b_globals.h"

typedef struct {
	Int32 num;
	char string[80];
} ResString;

static void addDialogs_string_map_table_v7() {
	ResString string_map_table_v7[] = {
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
	writeRecord(string_map_table_v7, sizeof(string_map_table_v7), GBVARS_STRINGMAPTABLEV7_INDEX , GBVARS_SCUMM);
}

static void addDialogs_string_map_table_v6() {
	ResString string_map_table_v6[] = {
		{117, "How may I serve you?"}, 
		{109, "Select a game to LOAD"}, 
		{108, "Name your SAVE game"}, 
		{96, "Save"}, 
		{97, "Load"}, 
		{98, "Play"}, 
		{99, "Cancel"}, 
		{100, "Quit"}, 
		{101, "OK"}, 
		{93, "Game paused"}, 
	};
	writeRecord(string_map_table_v6, sizeof(string_map_table_v6), GBVARS_STRINGMAPTABLEV6_INDEX , GBVARS_SCUMM);
}

static void addDialogs_string_map_table_v5() {
	ResString string_map_table_v5[] = {
		{28, "How may I serve you?"}, 
		{20, "Select a game to LOAD"},
		{19, "Name your SAVE game"},
		{7, "Save"},
		{8, "Load"},
		{9, "Play"},
		{10, "Cancel"},
		{11, "Quit"},
		{12, "OK"},
		{4, "Game paused"}
	};
	writeRecord(string_map_table_v5, sizeof(string_map_table_v5), GBVARS_STRINGMAPTABLEV5_INDEX , GBVARS_SCUMM);
}
///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

void addDialogs() {
	addDialogs_string_map_table_v7();
	addDialogs_string_map_table_v6();
	addDialogs_string_map_table_v5();

}
