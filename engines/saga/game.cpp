/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004-2006 The ScummVM project
 *
 * The ReInherit Engine is (C)2000-2003 by Daniel Balsom.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

// Game detection, general game parameters

#include "saga/saga.h"

#include "common/file.h"
#include "common/md5.h"
#include "common/hashmap.h"
#include "common/config-manager.h"
#include "base/plugins.h"
#include "backends/fs/fs.h"

#include "saga/rscfile.h"
#include "saga/interface.h"
#include "saga/scene.h"
#include "saga/resnames.h"

#define ITE_CONVERSE_MAX_TEXT_WIDTH (256 - 60)
#define ITE_CONVERSE_TEXT_HEIGHT	10
#define ITE_CONVERSE_TEXT_LINES     4

//TODO: ihnm
#define IHNM_CONVERSE_MAX_TEXT_WIDTH (256 - 60)
#define IHNM_CONVERSE_TEXT_HEIGHT	10
#define IHNM_CONVERSE_TEXT_LINES	10

namespace Saga {
static DetectedGameList GAME_detectGames(const FSList &fslist);
}

static const PlainGameDescriptor saga_games[] = {
	{"ite", "Inherit the Earth: Quest for the Orb"},
	{"ihnm", "I Have No Mouth and I Must Scream"},
	{0, 0}
};

GameList Engine_SAGA_gameIDList() {
	GameList games;
	const PlainGameDescriptor *g = saga_games;

	while (g->gameid) {
		games.push_back(*g);
		g++;
	}

	return games;
}

GameDescriptor Engine_SAGA_findGameID(const char *gameid) {
	const PlainGameDescriptor *g = saga_games;
	while (g->gameid) {
		if (0 == scumm_stricmp(gameid, g->gameid))
			break;
		g++;
	}
	return *g;
}

DetectedGameList Engine_SAGA_detectGames(const FSList &fslist) {
	return Saga::GAME_detectGames(fslist);
}

PluginError Engine_SAGA_create(OSystem *syst, Engine **engine) {
	assert(engine);
	*engine = new Saga::SagaEngine(syst);
	return kNoError;
}

REGISTER_PLUGIN(SAGA, "SAGA Engine");

namespace Saga {

// ITE section
static PanelButton ITE_MainPanelButtons[] = {
	{kPanelButtonVerb,		52,4,	57,10,	kVerbITEWalkTo,'w',0,	0,1,0},
	{kPanelButtonVerb,		52,15,	57,10,	kVerbITELookAt,'l',0,	2,3,0},
	{kPanelButtonVerb,		52,26,	57,10,	kVerbITEPickUp,'p',0,	4,5,0},
	{kPanelButtonVerb,		52,37,	57,10,	kVerbITETalkTo,'t',0,	0,1,0},
	{kPanelButtonVerb,		110,4,	56,10,	kVerbITEOpen,'o',0,	6,7,0},
	{kPanelButtonVerb,		110,15,	56,10,	kVerbITEClose,'c',0,	8,9,0},
	{kPanelButtonVerb,		110,26,	56,10,	kVerbITEUse,'u',0,		10,11,0},
	{kPanelButtonVerb,		110,37,	56,10,	kVerbITEGive,'g',0,	12,13,0},
	{kPanelButtonArrow,		306,6,	8,5,	-1,'U',0,			0,4,2},
	{kPanelButtonArrow,		306,41,	8,5,	1,'D',0,			1,5,3},

	{kPanelButtonInventory,	181 + 32*0,6,	27,18,	0,'-',0,	0,0,0},
	{kPanelButtonInventory,	181 + 32*1,6,	27,18,	1,'-',0,	0,0,0},
	{kPanelButtonInventory,	181 + 32*2,6,	27,18,	2,'-',0,	0,0,0},
	{kPanelButtonInventory,	181 + 32*3,6,	27,18,	3,'-',0,	0,0,0},

	{kPanelButtonInventory,	181 + 32*0,27,	27,18,	4,'-',0,	0,0,0},
	{kPanelButtonInventory,	181 + 32*1,27,	27,18,	5,'-',0,	0,0,0},
	{kPanelButtonInventory,	181 + 32*2,27,	27,18,	6,'-',0,	0,0,0},
	{kPanelButtonInventory,	181 + 32*3,27,	27,18,	7,'-',0,	0,0,0}
};

static PanelButton ITE_ConversePanelButtons[] = {
	{kPanelButtonConverseText,	52,6 + ITE_CONVERSE_TEXT_HEIGHT * 0, ITE_CONVERSE_MAX_TEXT_WIDTH,ITE_CONVERSE_TEXT_HEIGHT,	0,'1',0,	0,0,0},
	{kPanelButtonConverseText,	52,6 + ITE_CONVERSE_TEXT_HEIGHT * 1, ITE_CONVERSE_MAX_TEXT_WIDTH,ITE_CONVERSE_TEXT_HEIGHT,	1,'2',0,	0,0,0},
	{kPanelButtonConverseText,	52,6 + ITE_CONVERSE_TEXT_HEIGHT * 2, ITE_CONVERSE_MAX_TEXT_WIDTH,ITE_CONVERSE_TEXT_HEIGHT,	2,'3',0,	0,0,0},
	{kPanelButtonConverseText,	52,6 + ITE_CONVERSE_TEXT_HEIGHT * 3, ITE_CONVERSE_MAX_TEXT_WIDTH,ITE_CONVERSE_TEXT_HEIGHT,	3,'4',0,	0,0,0},
	{kPanelButtonArrow,			257,6,	9,6,	-1,'u',0,	0,4,2},
	{kPanelButtonArrow,			257,41,	9,6,	1,'d',0,	1,5,3},
};

static PanelButton ITE_OptionPanelButtons[] = {
	{kPanelButtonOptionSlider,	284,19, 13,75,	0,'-',0,	0,0,0}, //slider-scroller
	{kPanelButtonOption,	113,18, 45,17,	kTextReadingSpeed,'r',0,	0,0,0}, //read speed
	{kPanelButtonOption,	113,37, 45,17,	kTextMusic,'m',0,	0,0,0}, //music
	{kPanelButtonOption,	113,56, 45,17,	kTextSound,'n',0,	0,0,0}, //sound-noise
	{kPanelButtonOption,	13,79, 135,17,	kTextQuitGame,'q',0,	0,0,0}, //quit
	{kPanelButtonOption,	13,98, 135,17,	kTextContinuePlaying,'c',0,	0,0,0}, //continue
	{kPanelButtonOption,	164,98, 57,17,	kTextLoad,'l',0,	0,0,0}, //load
	{kPanelButtonOption,	241,98, 57,17,	kTextSave,'s',0,	0,0,0},	//save
	{kPanelButtonOptionSaveFiles,	166,20, 112,74,	0,'-',0,	0,0,0},	//savefiles

	{kPanelButtonOptionText,106,4, 0,0,	kTextGameOptions,'-',0,	0,0,0},	// text: game options
	{kPanelButtonOptionText,11,22, 0,0,	kTextReadingSpeed,'-',0, 0,0,0},	// text: read speed
	{kPanelButtonOptionText,28,22, 0,0,	kTextShowDialog,'-',0, 0,0,0},	// text: read speed
	{kPanelButtonOptionText,73,41, 0,0,	kTextMusic,'-',0, 0,0,0},	// text: music
	{kPanelButtonOptionText,69,60, 0,0,	kTextSound,'-',0, 0,0,0},	// text: noise
};

static PanelButton ITE_QuitPanelButtons[] = {
	{kPanelButtonQuit, 11,17, 60,16, kTextQuit,'q',0, 0,0,0},
	{kPanelButtonQuit, 121,17, 60,16, kTextCancel,'c',0, 0,0,0},
	{kPanelButtonQuitText, -1,5, 0,0, kTextQuitTheGameQuestion,'-',0, 0,0,0},
};

static PanelButton ITE_LoadPanelButtons[] = {
	{kPanelButtonLoad, 101,19, 60,16, kTextOK,'o',0, 0,0,0},
	{kPanelButtonLoadText, -1,5, 0,0, kTextLoadSuccessful,'-',0, 0,0,0},
};

static PanelButton ITE_SavePanelButtons[] = {
	{kPanelButtonSave, 11,37, 60,16, kTextSave,'s',0, 0,0,0},
	{kPanelButtonSave, 101,37, 60,16, kTextCancel,'c',0, 0,0,0},
	{kPanelButtonSaveEdit, 26,17, 119,17, 0,'-',0, 0,0,0},
	{kPanelButtonSaveText, -1,5, 0,0, kTextEnterSaveGameName,'-',0, 0,0,0},
};

static PanelButton ITE_ProtectPanelButtons[] = {
	{kPanelButtonProtectEdit, 26,17, 119,17, 0,'-',0, 0,0,0},
	{kPanelButtonProtectText, -1,5, 0,0, kTextEnterProtectAnswer,'-',0, 0,0,0},
};

/*
static PanelButton ITE_ProtectionPanelButtons[] = {
	{kPanelButtonArrow, 0,0, 0,0, 0,'-',0, 0,0,0}, //TODO
};*/

static GameDisplayInfo ITE_DisplayInfo = {
	320, 200,		// logical width&height

	35,				// scene path y offset
	137,			// scene height

	0,				// status x offset
	137,			// status y offset
	320,			// status width
	11,				// status height
	2,				// status text y offset
	186,			// status text color
	15,				// status BG color
	308,137,		// save reminder pos
	12,12,			// save reminder w & h
	6,7,			// save reminder sprite numbers

	5, 4,			// left portrait x, y offset
	274, 4,			// right portrait x, y offset

	8, 9,			// inventory Up & Down button indexies
	2, 4,			// inventory rows, columns

	0, 148,			// main panel offsets
	ARRAYSIZE(ITE_MainPanelButtons),
	ITE_MainPanelButtons,

	ITE_CONVERSE_MAX_TEXT_WIDTH,
	ITE_CONVERSE_TEXT_HEIGHT,
	ITE_CONVERSE_TEXT_LINES,
	4, 5,			// converse Up & Down button indexies
	0, 148,			// converse panel offsets
	ARRAYSIZE(ITE_ConversePanelButtons),
	ITE_ConversePanelButtons,

	8, 0,			// save file index
	8,				// optionSaveFileVisible
	8, 8,			// option panel offsets
	ARRAYSIZE(ITE_OptionPanelButtons),
	ITE_OptionPanelButtons,

	64,54,			// quit panel offsets
	192,38,			// quit panel width & height
	ARRAYSIZE(ITE_QuitPanelButtons),
	ITE_QuitPanelButtons,

	74, 53,			// load panel offsets
	172, 40,		// load panel width & height
	ARRAYSIZE(ITE_LoadPanelButtons),
	ITE_LoadPanelButtons,

	2,				// save edit index
	74, 44,			// save panel offsets
	172, 58,		// save panel width & height
	ARRAYSIZE(ITE_SavePanelButtons),
	ITE_SavePanelButtons,

	0,				// protect edit index
	74, 44,			// protect panel offsets
	172, 58,		// protect panel width & height
	ARRAYSIZE(ITE_ProtectPanelButtons),
	ITE_ProtectPanelButtons
};

static GameResourceDescription ITE_Resources = {
	RID_ITE_SCENE_LUT,  // Scene lookup table RN
	RID_ITE_SCRIPT_LUT, // Script lookup table RN
	RID_ITE_MAIN_PANEL,
	RID_ITE_CONVERSE_PANEL,
	RID_ITE_OPTION_PANEL,
	RID_ITE_MAIN_SPRITES,
	RID_ITE_MAIN_PANEL_SPRITES,
	RID_ITE_DEFAULT_PORTRAITS,
	RID_ITE_MAIN_STRINGS,
	RID_ITE_ACTOR_NAMES
};

static GameResourceDescription ITEDemo_Resources = {
	RID_ITEDEMO_SCENE_LUT,  // Scene lookup table RN
	RID_ITEDEMO_SCRIPT_LUT, // Script lookup table RN
	RID_ITEDEMO_MAIN_PANEL,
	RID_ITEDEMO_CONVERSE_PANEL,
	RID_ITEDEMO_OPTION_PANEL,
	RID_ITEDEMO_MAIN_SPRITES,
	RID_ITEDEMO_MAIN_PANEL_SPRITES,
	RID_ITEDEMO_DEFAULT_PORTRAITS,
	RID_ITEDEMO_MAIN_STRINGS,
	RID_ITEDEMO_ACTOR_NAMES
};

// Inherit the Earth - DOS Demo version
static GameFileDescription ITE_DEMO_G_GameFiles[] = {
	{"ite.rsc",		GAME_RESOURCEFILE,					"986c79c4d2939dbe555576529fd37932"},
	//{"ite.dmo",	GAME_DEMOFILE},						"0b9a70eb4e120b6f00579b46c8cae29e"
	{"scripts.rsc", GAME_SCRIPTFILE,					"d5697dd3240a3ceaddaa986c47e1a2d7"},
	{"voices.rsc",	GAME_SOUNDFILE | GAME_VOICEFILE,	"c58e67c506af4ffa03fd0aac2079deb0"}
};

static GameFontDescription ITEDEMO_GameFonts[] = {
	{0},
	{1}
};

static GameSoundInfo ITEDEMO_GameSound = {
	kSoundVOC,
	-1,
	-1,
	false,
	false,
	true
};

// Inherit the Earth - Wyrmkeep Win32 Demo version

static GameFileDescription ITE_WINDEMO2_GameFiles[] = {
	{"ited.rsc",		GAME_RESOURCEFILE,	"3a450852cbf3c80773984d565647e6ac"},
	{"scriptsd.rsc",	GAME_SCRIPTFILE,	"3f12b67fa93e56e1a6be39d2921d80bb"},
	{"soundsd.rsc",		GAME_SOUNDFILE,		"95a6c148e22e99a8c243f2978223583c"},
	{"voicesd.rsc",		GAME_VOICEFILE,		"e139d86bab2ee8ba3157337f894a92d4"}
};

static GameFileDescription ITE_WINDEMO1_GameFiles[] = {
	{"ited.rsc",		GAME_RESOURCEFILE,	"3a450852cbf3c80773984d565647e6ac"},
	{"scriptsd.rsc",	GAME_SCRIPTFILE,	"3f12b67fa93e56e1a6be39d2921d80bb"},
	{"soundsd.rsc",		GAME_SOUNDFILE,		"a741139dd7365a13f463cd896ff9969a"},
	{"voicesd.rsc",		GAME_VOICEFILE,		"0759eaf5b64ae19fd429920a70151ad3"}
};

static GameFontDescription ITEWINDEMO_GameFonts[] = {
	{2},
	{0}
};

static GameSoundInfo ITEWINDEMO1_GameSound = {
	kSoundPCM,
	22050,
	8,
	false,
	false,
	false
};

static GameSoundInfo ITEWINDEMO2_GameVoice = {
	kSoundVOX,
	22050,
	16,
	false,
	false,
	true
};

static GameSoundInfo ITEWINDEMO2_GameSound = {
	kSoundPCM,
	22050,
	16,
	false,
	false,
	true
};

// Inherit the Earth - Wyrmkeep Mac Demo version
static GameFileDescription ITE_MACDEMO2_GameFiles[] = {
	{"ited.rsc",		GAME_RESOURCEFILE,	"addfc9d82bc2fa1f4cab23743c652c08"},
	{"scriptsd.rsc",	GAME_SCRIPTFILE,	"fded5c59b8b7c5976229f960d21e6b0b"},
	{"soundsd.rsc",		GAME_SOUNDFILE,		"b3a831fbed337d1f1300fee1dd474f6c"},
	{"voicesd.rsc",		GAME_VOICEFILE,		"e139d86bab2ee8ba3157337f894a92d4"},
	{"musicd.rsc",		GAME_MUSICFILE,		"495bdde51fd9f4bea2b9c911091b1ab2"}
};

static GameFileDescription ITE_MACDEMO1_GameFiles[] = {
	{"ited.rsc",		GAME_RESOURCEFILE,	"addfc9d82bc2fa1f4cab23743c652c08"},
	{"scriptsd.rsc",	GAME_SCRIPTFILE,	"fded5c59b8b7c5976229f960d21e6b0b"},
	{"soundsd.rsc",		GAME_SOUNDFILE,		"b3a831fbed337d1f1300fee1dd474f6c"},
	{"voicesd.rsc",		GAME_VOICEFILE,		"e139d86bab2ee8ba3157337f894a92d4"},
	{"musicd.rsc",		GAME_MUSICFILE,		"1a91cd60169f367ecb6c6e058d899b2f"}
};

static GameSoundInfo ITEMACDEMO_GameVoice = {
	kSoundVOX,
	22050,
	16,
	false,
	false,
	true
};

static GameSoundInfo ITEMACDEMO_GameSound = {
	kSoundPCM,
	22050,
	16,
	false,
	true,
	true
};

static GameSoundInfo ITEMACDEMO_GameMusic = {
	kSoundPCM,
	11025,
	16,
	false,
	false,
	true
};

// Inherit the Earth - Wyrmkeep Linux Demo version
static GameFileDescription ITE_LINDEMO_GameFiles[] = {
	{"ited.rsc",		GAME_RESOURCEFILE,	"3a450852cbf3c80773984d565647e6ac"},
	{"scriptsd.rsc",	GAME_SCRIPTFILE,	"3f12b67fa93e56e1a6be39d2921d80bb"},
	{"soundsd.rsc",		GAME_SOUNDFILE,		"95a6c148e22e99a8c243f2978223583c"},
	{"voicesd.rsc",		GAME_VOICEFILE,		"e139d86bab2ee8ba3157337f894a92d4"},
	{"musicd.rsc",		GAME_MUSICFILE,		"d6454756517f042f01210458abe8edd4"}
};

static GameSoundInfo ITELINDEMO_GameMusic = {
	kSoundPCM,
	11025,
	16,
	true,
	false,
	true
};

// Inherit the Earth - Wyrmkeep Linux version

static GameFileDescription ITE_LINCD_GameFiles[] = {
	{"ite.rsc",		GAME_RESOURCEFILE,	"8f4315a9bb10ec839253108a032c8b54"},
	{"scripts.rsc",	GAME_SCRIPTFILE,	"a891405405edefc69c9d6c420c868b84"},
	{"sounds.rsc",	GAME_SOUNDFILE,		"e2ccb61c325d6d1ead3be0e731fe29fe"},
	{"voices.rsc",	GAME_VOICEFILE,		"41bb6b95d792dde5196bdb78740895a6"},
	{"music.rsc",	GAME_MUSICFILE,		"d6454756517f042f01210458abe8edd4"}
};

// Inherit the Earth - Wyrmkeep combined Windows/Mac/Linux version. This
// version is different from the other Wyrmkeep re-releases in that it does
// not have any substitute files. Presumably the ite.rsc file has been
// modified to include the Wyrmkeep changes. The resource files are little-
// endian, except for the voice file which is big-endian.

static GameFileDescription ITE_MULTICD_GameFiles[] = {
	{"ite.rsc",						GAME_RESOURCEFILE,					"a6433e34b97b15e64fe8214651012db9"},
	{"scripts.rsc",					GAME_SCRIPTFILE,					"a891405405edefc69c9d6c420c868b84"},
	{"sounds.rsc",					GAME_SOUNDFILE,						"e2ccb61c325d6d1ead3be0e731fe29fe"},
	{"inherit the earth voices",	GAME_VOICEFILE | GAME_SWAPENDIAN,	"c14c4c995e7a0d3828e3812a494301b7"},
	{"music.rsc",					GAME_MUSICFILE,						"d6454756517f042f01210458abe8edd4"}
};

static GameFileDescription ITE_MACCD_G_GameFiles[] = {
	{"ite resources.bin",	GAME_RESOURCEFILE | GAME_MACBINARY,	"0bd506aa887bfc7965f695c6bd28237d"},
	{"ite scripts.bin",		GAME_SCRIPTFILE | GAME_MACBINARY,	"af0d7a2588e09ad3ecbc5b474ea238bf"},
	{"ite sounds.bin",		GAME_SOUNDFILE | GAME_MACBINARY,	"441426c6bb2a517f65c7e49b57f7a345"},
	{"ite music.bin",		GAME_MUSICFILE_GM | GAME_MACBINARY,	"c1d20324b7cdf1650e67061b8a93251c"},
	{"ite voices.bin",		GAME_VOICEFILE | GAME_MACBINARY,	"dba92ae7d57e942250fe135609708369"}
};

static GameSoundInfo ITEMACCD_G_GameSound = {
	kSoundMacPCM,
	22050,
	8,
	false,
	false,
	false
};

// Inherit the Earth - Mac Wyrmkeep version
static GameFileDescription ITE_MACCD_GameFiles[] = {
	{"ite.rsc",						GAME_RESOURCEFILE,	"4f7fa11c5175980ed593392838523060"},
	{"scripts.rsc",					GAME_SCRIPTFILE,	"adf1f46c1d0589083996a7060c798ad0"},
	{"sounds.rsc",					GAME_SOUNDFILE,		"95863b89a0916941f6c5e1789843ba14"},
	{"inherit the earth voices",	GAME_VOICEFILE,		"c14c4c995e7a0d3828e3812a494301b7"},
	{"music.rsc",					GAME_MUSICFILE,		"1a91cd60169f367ecb6c6e058d899b2f"}
};

static GameSoundInfo ITEMACCD_GameSound = {
	kSoundPCM,
	22050,
	16,
	false,
	true,
	true
};

static GameSoundInfo ITEMACCD_GameMusic = {
	kSoundPCM,
	11025,
	16,
	true,
	false,
	true
};

// Inherit the Earth - Diskette version
static GameFileDescription ITE_DISK_DE_GameFiles[] = {
	{"ite.rsc",		GAME_RESOURCEFILE,					"869fc23c8f38f575979ec67152914fee"},
	{"scripts.rsc",	GAME_SCRIPTFILE,					"516f7330f8410057b834424ea719d1ef"},
	{"voices.rsc",	GAME_SOUNDFILE | GAME_VOICEFILE,	"0c9113e630f97ef0996b8c3114badb08"}
};

static GameFileDescription ITE_DISK_G_GameFiles[] = {
	{"ite.rsc",		GAME_RESOURCEFILE,					"8f4315a9bb10ec839253108a032c8b54"},
	{"scripts.rsc",	GAME_SCRIPTFILE,					"516f7330f8410057b834424ea719d1ef"},
	{"voices.rsc",	GAME_SOUNDFILE | GAME_VOICEFILE,	"c46e4392fcd2e89bc91e5567db33b62d"}
};

static GameFileDescription ITE_DISK_DE2_GameFiles[] = {
	{"ite.rsc",		GAME_RESOURCEFILE,					"869fc23c8f38f575979ec67152914fee"},
	{"scripts.rsc",	GAME_SCRIPTFILE,					"516f7330f8410057b834424ea719d1ef"},
	{"voices.rsc",	GAME_SOUNDFILE | GAME_VOICEFILE,	"0c9113e630f97ef0996b8c3114badb08"},
	{"music.rsc",	GAME_MUSICFILE,						"d6454756517f042f01210458abe8edd4"}
};

static GameFileDescription ITE_DISK_G2_GameFiles[] = {
	{"ite.rsc",		GAME_RESOURCEFILE,					"8f4315a9bb10ec839253108a032c8b54"},
	{"scripts.rsc",	GAME_SCRIPTFILE,					"516f7330f8410057b834424ea719d1ef"},
	{"voices.rsc",	GAME_SOUNDFILE | GAME_VOICEFILE,	"c46e4392fcd2e89bc91e5567db33b62d"},
	{"music.rsc",	GAME_MUSICFILE,						"d6454756517f042f01210458abe8edd4"}
};

static GameFontDescription ITEDISK_GameFonts[] = {
	{2},
	{0},
	{1}
};

static GameSoundInfo ITEDISK_GameSound = {
	kSoundVOC,
	-1,
	-1,
	false,
	false,
	true
};

// Inherit the Earth - CD Enhanced version
static GameFileDescription ITE_WINCD_GameFiles[] = {
	{"ite.rsc",		GAME_RESOURCEFILE,	"8f4315a9bb10ec839253108a032c8b54"},
	{"scripts.rsc",	GAME_SCRIPTFILE,	"a891405405edefc69c9d6c420c868b84"},
	{"sounds.rsc",	GAME_SOUNDFILE,		"e2ccb61c325d6d1ead3be0e731fe29fe"},
	{"voices.rsc",	GAME_VOICEFILE,		"41bb6b95d792dde5196bdb78740895a6"}
};

static GameFileDescription ITE_CD_G_GameFiles[] = {
	{"ite.rsc",		GAME_RESOURCEFILE,	"8f4315a9bb10ec839253108a032c8b54"},
	{"scripts.rsc",	GAME_SCRIPTFILE,	"50a0d2d7003c926a3832d503c8534e90"},
	{"sounds.rsc",	GAME_SOUNDFILE,		"e2ccb61c325d6d1ead3be0e731fe29fe"},
	{"voices.rsc",	GAME_VOICEFILE,		"41bb6b95d792dde5196bdb78740895a6"}
};

// reported by mld. Bestsellergamers cover disk
static GameFileDescription ITE_CD_DE_GameFiles[] = {
	{"ite.rsc",		GAME_RESOURCEFILE,	"869fc23c8f38f575979ec67152914fee"},
	{"scripts.rsc",	GAME_SCRIPTFILE,	"a891405405edefc69c9d6c420c868b84"},
	{"sounds.rsc",	GAME_SOUNDFILE,		"e2ccb61c325d6d1ead3be0e731fe29fe"},
	{"voices.rsc",	GAME_VOICEFILE,		"2fbad5d10b9b60a3415dc4aebbb11718"}
};

static GameFileDescription ITE_CD_GameFiles[] = {
	{"ite.rsc",		GAME_RESOURCEFILE,	"8f4315a9bb10ec839253108a032c8b54"},
	{"scripts.rsc",	GAME_SCRIPTFILE,	"a891405405edefc69c9d6c420c868b84"},
	{"sounds.rsc",	GAME_SOUNDFILE,		"e2ccb61c325d6d1ead3be0e731fe29fe"},
	{"voices.rsc",	GAME_VOICEFILE,		"41bb6b95d792dde5196bdb78740895a6"}
};


static GameFileDescription ITE_CD_G2_GameFiles[] = {
	{"ite.rsc",		GAME_RESOURCEFILE,	"8f4315a9bb10ec839253108a032c8b54"},
	{"scripts.rsc",	GAME_SCRIPTFILE,	"50a0d2d7003c926a3832d503c8534e90"},
	{"sounds.rsc",	GAME_SOUNDFILE,		"e2ccb61c325d6d1ead3be0e731fe29fe"},
	{"voices.rsc",	GAME_VOICEFILE,		"41bb6b95d792dde5196bdb78740895a6"},
	{"music.rsc",	GAME_MUSICFILE,		"d6454756517f042f01210458abe8edd4"}
};

static GameFileDescription ITE_CD_DE2_GameFiles[] = {
	{"ite.rsc",		GAME_RESOURCEFILE,	"869fc23c8f38f575979ec67152914fee"},
	{"scripts.rsc",	GAME_SCRIPTFILE,	"a891405405edefc69c9d6c420c868b84"},
	{"sounds.rsc",	GAME_SOUNDFILE,		"e2ccb61c325d6d1ead3be0e731fe29fe"},
	{"voices.rsc",	GAME_VOICEFILE,		"2fbad5d10b9b60a3415dc4aebbb11718"},
	{"music.rsc",	GAME_MUSICFILE,		"d6454756517f042f01210458abe8edd4"}
};


static GameFontDescription ITECD_GameFonts[] = {
	{2},
	{0},
	{1}
};

static GameSoundInfo ITECD_GameSound = {
	kSoundPCM,
	22050,
	16,
	false,
	false,
	true
};

static GamePatchDescription ITEWinPatch1_Files[] = {
	{ "cave.mid", GAME_RESOURCEFILE, 9, NULL},
	{ "intro.mid", GAME_RESOURCEFILE, 10, NULL},
	{ "fvillage.mid", GAME_RESOURCEFILE, 11, NULL},
	{ "elkhall.mid", GAME_RESOURCEFILE, 12, NULL},
	{ "mouse.mid", GAME_RESOURCEFILE, 13, NULL},
	{ "darkclaw.mid", GAME_RESOURCEFILE, 14, NULL},
	{ "birdchrp.mid", GAME_RESOURCEFILE, 15, NULL},
	{ "orbtempl.mid", GAME_RESOURCEFILE, 16, NULL},
	{ "spooky.mid", GAME_RESOURCEFILE, 17, NULL},
	{ "catfest.mid", GAME_RESOURCEFILE, 18, NULL},
	{ "elkfanfare.mid", GAME_RESOURCEFILE, 19, NULL},
	{ "bcexpl.mid", GAME_RESOURCEFILE, 20, NULL},
	{ "boargtnt.mid", GAME_RESOURCEFILE, 21, NULL},
	{ "boarking.mid", GAME_RESOURCEFILE, 22, NULL},
	{ "explorea.mid", GAME_RESOURCEFILE, 23, NULL},
	{ "exploreb.mid", GAME_RESOURCEFILE, 24, NULL},
	{ "explorec.mid", GAME_RESOURCEFILE, 25, NULL},
	{ "sunstatm.mid", GAME_RESOURCEFILE, 26, NULL},
	{ "nitstrlm.mid", GAME_RESOURCEFILE, 27, NULL},
	{ "humruinm.mid", GAME_RESOURCEFILE, 28, NULL},
	{ "damexplm.mid", GAME_RESOURCEFILE, 29, NULL},
	{ "tychom.mid", GAME_RESOURCEFILE, 30, NULL},
	{ "kitten.mid", GAME_RESOURCEFILE, 31, NULL},
	{ "sweet.mid", GAME_RESOURCEFILE, 32, NULL},
	{ "brutalmt.mid", GAME_RESOURCEFILE, 33, NULL},
	{ "shiala.mid", GAME_RESOURCEFILE, 34, NULL},

	{ "wyrm.pak", GAME_RESOURCEFILE, 1529, NULL},
	{ "wyrm1.dlt", GAME_RESOURCEFILE, 1530, NULL},
	{ "wyrm2.dlt", GAME_RESOURCEFILE, 1531, NULL},
	{ "wyrm3.dlt", GAME_RESOURCEFILE, 1532, NULL},
	{ "wyrm4.dlt", GAME_RESOURCEFILE, 1533, NULL},
	{ "credit3n.dlt", GAME_RESOURCEFILE, 1796, NULL},
	{ "credit4n.dlt", GAME_RESOURCEFILE, 1797, NULL},
	{ "p2_a.voc", GAME_VOICEFILE, 4, NULL}
};

static GamePatchDescription ITEWinPatch2_Files[] = {
	{ "cave.mid", GAME_RESOURCEFILE, 9, NULL},
	{ "intro.mid", GAME_RESOURCEFILE, 10, NULL},
	{ "fvillage.mid", GAME_RESOURCEFILE, 11, NULL},
	{ "elkfanfare.mid", GAME_RESOURCEFILE, 19, NULL},
	{ "bcexpl.mid", GAME_RESOURCEFILE, 20, NULL},
	{ "boargtnt.mid", GAME_RESOURCEFILE, 21, NULL},
	{ "explorea.mid", GAME_RESOURCEFILE, 23, NULL},
	{ "sweet.mid", GAME_RESOURCEFILE, 32, NULL},

	{ "wyrm.pak", GAME_RESOURCEFILE, 1529, NULL},
	{ "wyrm1.dlt", GAME_RESOURCEFILE, 1530, NULL},
	{ "wyrm2.dlt", GAME_RESOURCEFILE, 1531, NULL},
	{ "wyrm3.dlt", GAME_RESOURCEFILE, 1532, NULL},
	{ "p2_a.iaf", GAME_VOICEFILE, 4, &ITECD_GameSound}
/*	boarhall.bbm
	elkenter.bbm
	ferrets.bbm
	ratdoor.bbm
	sanctuar.bbm
	tycho.bbm*/
};

static GamePatchDescription ITEMacPatch_Files[] = {
	{ "wyrm.pak", GAME_RESOURCEFILE, 1529, NULL},
	{ "wyrm1.dlt", GAME_RESOURCEFILE, 1530, NULL},
	{ "wyrm2.dlt", GAME_RESOURCEFILE, 1531, NULL},
	{ "wyrm3.dlt", GAME_RESOURCEFILE, 1532, NULL},
	{ "wyrm4.dlt", GAME_RESOURCEFILE, 1533, NULL},
	{ "credit3m.dlt", GAME_RESOURCEFILE, 1796, NULL},
	{ "credit4m.dlt", GAME_RESOURCEFILE, 1797, NULL},
	{ "p2_a.iaf", GAME_VOICEFILE, 4, &ITEMACCD_GameSound}
};

static GamePatchDescription ITELinPatch_Files[] = {
	{ "wyrm.pak", GAME_RESOURCEFILE, 1529, NULL},
	{ "wyrm1.dlt", GAME_RESOURCEFILE, 1530, NULL},
	{ "wyrm2.dlt", GAME_RESOURCEFILE, 1531, NULL},
	{ "wyrm3.dlt", GAME_RESOURCEFILE, 1532, NULL},
	{ "credit3n.dlt", GAME_RESOURCEFILE, 1796, NULL},
	{ "credit4n.dlt", GAME_RESOURCEFILE, 1797, NULL},
	{ "P2_A.iaf", GAME_VOICEFILE, 4, &ITECD_GameSound}
};

// IHNM section

static PanelButton IHNM_MainPanelButtons[] = {
	{kPanelButtonVerb,		106,12,		114,30,	kVerbIHNMWalk,'w',0,	0,1,0},
	{kPanelButtonVerb,		106,44,		114,30,	kVerbIHNMLookAt,'l',0,	2,3,0},
	{kPanelButtonVerb,		106,76,		114,30, kVerbIHNMTake,'k',0,	4,5,0},
	{kPanelButtonVerb,		106,108,	114,30, kVerbIHNMUse,'u',0,		6,7,0},
	{kPanelButtonVerb,		223,12,		114,30, kVerbIHNMTalkTo,'t',0,	8,9,0},
	{kPanelButtonVerb,		223,44,		114,30, kVerbIHNMSwallow,'s',0,	10,11,0},
	{kPanelButtonVerb,		223,76,		114,30, kVerbIHNMGive,'g',0,	12,13,0},
	{kPanelButtonVerb,		223,108,	114,30, kVerbIHNMPush,'p',0,	14,15,0},
	{kPanelButtonArrow,		606,22,		20,25,	-1,'[',0,			0,0,0}, //TODO: arrow Sprite Numbers
	{kPanelButtonArrow,		606,108,	20,25,	1,']',0,			0,0,0},

	{kPanelButtonInventory,	357 + 64*0,18,	54,54,	0,'-',0,	0,0,0},
	{kPanelButtonInventory,	357 + 64*1,18,	54,54,	1,'-',0,	0,0,0},
	{kPanelButtonInventory,	357 + 64*2,18,	54,54,	2,'-',0,	0,0,0},
	{kPanelButtonInventory,	357 + 64*3,18,	54,54,	3,'-',0,	0,0,0},

	{kPanelButtonInventory,	357 + 64*0,80,	54,54,	4,'-',0,	0,0,0},
	{kPanelButtonInventory,	357 + 64*1,80,	54,54,	5,'-',0,	0,0,0},
	{kPanelButtonInventory,	357 + 64*2,80,	54,54,	6,'-',0,	0,0,0},
	{kPanelButtonInventory,	357 + 64*3,80,	54,54,	7,'-',0,	0,0,0}
};

static PanelButton IHNM_ConversePanelButtons[] = {
	{kPanelButtonConverseText,	117,18 + IHNM_CONVERSE_TEXT_HEIGHT * 0, IHNM_CONVERSE_MAX_TEXT_WIDTH,IHNM_CONVERSE_TEXT_HEIGHT,	0,'1',0,	0,0,0},
	{kPanelButtonConverseText,	52,18 + IHNM_CONVERSE_TEXT_HEIGHT * 1, IHNM_CONVERSE_MAX_TEXT_WIDTH,IHNM_CONVERSE_TEXT_HEIGHT,	1,'2',0,	0,0,0},
	{kPanelButtonConverseText,	52,18 + IHNM_CONVERSE_TEXT_HEIGHT * 2, IHNM_CONVERSE_MAX_TEXT_WIDTH,IHNM_CONVERSE_TEXT_HEIGHT,	2,'3',0,	0,0,0},
	{kPanelButtonConverseText,	52,18 + IHNM_CONVERSE_TEXT_HEIGHT * 3, IHNM_CONVERSE_MAX_TEXT_WIDTH,IHNM_CONVERSE_TEXT_HEIGHT,	3,'4',0,	0,0,0},
	//.....
	{kPanelButtonArrow,			606,22,		20,25,	-1,'[',0,	0,0,0}, //TODO: arrow Sprite Numbers
	{kPanelButtonArrow,			606,108,	20,25,	1,']',0,	0,0,0}
};

static PanelButton IHNM_OptionPanelButtons[] = {
	{kPanelButtonArrow, 0,0, 0,0, 0,'-',0, 0,0,0}, //TODO
};

static PanelButton IHNM_QuitPanelButtons[] = {
	{kPanelButtonArrow, 0,0, 0,0, 0,'-',0, 0,0,0}, //TODO
};

static PanelButton IHNM_LoadPanelButtons[] = {
	{kPanelButtonArrow, 0,0, 0,0, 0,'-',0, 0,0,0}, //TODO
};

static PanelButton IHNM_SavePanelButtons[] = {
	{kPanelButtonArrow, 0,0, 0,0, 0,'-',0, 0,0,0}, //TODO
};


static GameDisplayInfo IHNM_DisplayInfo = { //TODO: fill it all
	640, 480,	// logical width&height

	0,			// scene path y offset
	304,		// scene height

	0,			// status x offset
	304,		// status y offset
	616,		// status width
	24,			// status height
	8,			// status text y offset
	253,		// status text color
	250,		// status BG color
	616, 303,	// save reminder pos
	24, 24,		// save reminder w&h
	0,1,		// save reminder sprite numbers

	11, 12,		// left portrait x, y offset
	-1, -1,		// right portrait x, y offset

	-1, -1,		// inventory Up & Down button indexies
	2, 4,		// inventory rows, columns

	0, 328,		// main panel offsets
	ARRAYSIZE(IHNM_MainPanelButtons),
	IHNM_MainPanelButtons,

	-1, -1,		// converse Up & Down button indexies

	IHNM_CONVERSE_MAX_TEXT_WIDTH,
	IHNM_CONVERSE_TEXT_HEIGHT,
	IHNM_CONVERSE_TEXT_LINES,
	0, 328,		// converse panel offsets
	ARRAYSIZE(IHNM_ConversePanelButtons),
	IHNM_ConversePanelButtons,

	-1, -1,		// save file index
	0,			// optionSaveFileVisible
	0, 0,		// option panel offsets
	ARRAYSIZE(IHNM_OptionPanelButtons),
	IHNM_OptionPanelButtons,

	0,0,			// quit panel offsets
	0,0,			// quit panel width & height
	ARRAYSIZE(IHNM_QuitPanelButtons),
	IHNM_QuitPanelButtons,

	0, 0,			// load panel offsets
	0, 0,			// load panel width & height
	ARRAYSIZE(IHNM_LoadPanelButtons),
	IHNM_LoadPanelButtons,

	-1,				// save edit index
	0, 0,			// save panel offsets
	0, 0,			// save panel width & height
	ARRAYSIZE(IHNM_SavePanelButtons),
	IHNM_SavePanelButtons,

	// No protection panel in IHNM
	-1,				// protect edit index
	0, 0,			// protect panel offsets
	0, 0,			// protect panel width & height
	ARRAYSIZE(IHNM_SavePanelButtons),
	IHNM_SavePanelButtons
};

static GameResourceDescription IHNM_Resources = {
	RID_IHNM_SCENE_LUT,  // Scene lookup table RN
	RID_IHNM_SCRIPT_LUT, // Script lookup table RN
	RID_IHNM_MAIN_PANEL,
	RID_IHNM_CONVERSE_PANEL,
	RID_IHNM_OPTION_PANEL,
	RID_IHNM_MAIN_SPRITES,
	RID_IHNM_MAIN_PANEL_SPRITES,
	0,
	RID_IHNM_MAIN_STRINGS,
	0
};

// I Have No Mouth and I Must Scream - Demo version
static GameFileDescription IHNM_DEMO_GameFiles[] = {
	{"scream.res",		GAME_RESOURCEFILE,	"46bbdc65d164ba7e89836a0935eec8e6"},
	{"scripts.res",		GAME_SCRIPTFILE,	"9626bda8978094ff9b29198bc1ed5f9a"},
	{"sfx.res",			GAME_SOUNDFILE,		"1c610d543f32ec8b525e3f652536f269"},
	{"voicesd.res",		GAME_VOICEFILE,		"3bbc16a8f741dbb511da506c660a0b54"}
};

// I Have No Mouth and I Must Scream - Retail CD version

static GameFileDescription IHNM_CD_GameFiles[] = {
	{"musicfm.res",	GAME_MUSICFILE_FM,					"0439083e3dfdc51b486071d45872ae52"},
	{"musicgm.res",	GAME_MUSICFILE_GM,					"80f875a1fb384160d1f4b27166eef583"},
	{"scream.res",	GAME_RESOURCEFILE,					"46bbdc65d164ba7e89836a0935eec8e6"},
	{"patch.re_",	GAME_PATCHFILE | GAME_RESOURCEFILE,	"58b79e61594779513c7f2d35509fa89e"},
	{"scripts.res",	GAME_SCRIPTFILE,					"be38bbc5a26be809dbf39f13befebd01"},
	{"sfx.res",		GAME_SOUNDFILE,						"1c610d543f32ec8b525e3f652536f269"},
	{"voicess.res",	GAME_VOICEFILE,						"54b1f2013a075338ceb0e258d97808bd"}, //order of voice bank file is important
	{"voices1.res",	GAME_VOICEFILE,						"fc6440b38025f4b2cc3ff55c3da5c3eb"},
	{"voices2.res",	GAME_VOICEFILE,						"b37f10fd1696ade7d58704ccaaebceeb"},
	{"voices3.res",	GAME_VOICEFILE,						"3bbc16a8f741dbb511da506c660a0b54"},
	{"voices4.res",	GAME_VOICEFILE,						"ebfa160122d2247a676ca39920e5d481"},
	{"voices5.res",	GAME_VOICEFILE,						"1f501ce4b72392bdd1d9ec38f6eec6da"},
	{"voices6.res",	GAME_VOICEFILE,						"f580ed7568c7d6ef34e934ba20adf834"}
};

static GameFileDescription IHNM_CD_ES_GameFiles[] = {
	{"musicfm.res",	GAME_MUSICFILE_FM,					"0439083e3dfdc51b486071d45872ae52"},
	{"musicgm.res",	GAME_MUSICFILE_GM,					"80f875a1fb384160d1f4b27166eef583"},
	{"scream.res",	GAME_RESOURCEFILE,					"c92370d400e6f2a3fc411c3729d09224"},
	{"patch.re_",	GAME_PATCHFILE | GAME_RESOURCEFILE,	"58b79e61594779513c7f2d35509fa89e"},
	{"scripts.res",	GAME_SCRIPTFILE,					"be38bbc5a26be809dbf39f13befebd01"},
	{"sfx.res",		GAME_SOUNDFILE,						"1c610d543f32ec8b525e3f652536f269"},
	{"voicess.res",	GAME_VOICEFILE,						"d869de9883c8faea7f687217a9ec7057"}, //order of voice bank file is important
	{"voices1.res",	GAME_VOICEFILE,						"dc6a34e3d1668730ea46815a92c7847f"},
	{"voices2.res",	GAME_VOICEFILE,						"dc6a5fa7a4cdc2ca5a6fd924e969986c"},
	{"voices3.res",	GAME_VOICEFILE,						"dc6a5fa7a4cdc2ca5a6fd924e969986c"},
	{"voices4.res",	GAME_VOICEFILE,						"0f87400b804232a58dd22e404420cc45"},
	{"voices5.res",	GAME_VOICEFILE,						"172668cfc5d8c305cb5b1a9b4d995fc0"},
	{"voices6.res",	GAME_VOICEFILE,						"96c9bda9a5f41d6bc232ed7bf6d371d9"}
};

static GameFileDescription IHNM_CD_RU_GameFiles[] = {
	{"musicfm.res",	GAME_MUSICFILE_FM,					"0439083e3dfdc51b486071d45872ae52"},
	{"musicgm.res",	GAME_MUSICFILE_GM,					"80f875a1fb384160d1f4b27166eef583"},
	{"scream.res",	GAME_RESOURCEFILE,					"46bbdc65d164ba7e89836a0935eec8e6"},
	{"patch.re_",	GAME_PATCHFILE | GAME_RESOURCEFILE,	"58b79e61594779513c7f2d35509fa89e"},
	{"scripts.res",	GAME_SCRIPTFILE,					"be38bbc5a26be809dbf39f13befebd01"},
	{"sfx.res",		GAME_SOUNDFILE,						"1c610d543f32ec8b525e3f652536f269"},
	{"voicess.res",	GAME_VOICEFILE,						"9df7cd3b18ddaa16b5291b3432567036"}, //order of voice bank file is important
	{"voices1.res",	GAME_VOICEFILE,						"d6100d2dc3b2b9f2e1ad247f613dce9b"},
	{"voices2.res",	GAME_VOICEFILE,						"84f6f48ecc2832841ea6417a9a379430"},
	{"voices3.res",	GAME_VOICEFILE,						"ebb9501283047f27a0f54e27b3c8ba1e"},
	{"voices4.res",	GAME_VOICEFILE,						"4c145da5fa6d1306162a7ca8ce5a4f2e"},
	{"voices5.res",	GAME_VOICEFILE,						"871a559644281917677eca4af1b05620"},
	{"voices6.res",	GAME_VOICEFILE,						"211be5c24f066d69a2f6cfa953acfba6"}
};

// I Have No Mouth and I Must Scream - Censored CD version (without Nimdok)

// Reported by mld. German Retail
static GameFileDescription IHNM_CD_DE_GameFiles[] = {
	{"musicfm.res",	GAME_MUSICFILE_FM,					"0439083e3dfdc51b486071d45872ae52"},
	{"musicgm.res",	GAME_MUSICFILE_GM,					"80f875a1fb384160d1f4b27166eef583"},
	{"scream.res",	GAME_RESOURCEFILE,					"c92370d400e6f2a3fc411c3729d09224"},
	{"scripts.res",	GAME_SCRIPTFILE,					"32aa01a89937520fe0ea513950117292"},
	{"patch.re_",	GAME_PATCHFILE | GAME_RESOURCEFILE,	"58b79e61594779513c7f2d35509fa89e"},
	{"sfx.res",		GAME_SOUNDFILE,						"1c610d543f32ec8b525e3f652536f269"},
	{"voicess.res",	GAME_VOICEFILE,						"8b09a196a52627cacb4eab13bfe0b2c3"}, //order of voice bank file is important
	{"voices1.res",	GAME_VOICEFILE,						"424971e1e2373187c3f5734fe36071a2"},
	{"voices2.res",	GAME_VOICEFILE,						"c270e0980782af43641a86e4a14e2a32"},
	{"voices3.res",	GAME_VOICEFILE,						"49e42befea883fd101ec3d0f5d0647b9"},
	{"voices5.res",	GAME_VOICEFILE,						"c477443c52a0aa56e686ebd8d051e4ab"},
	{"voices6.res",	GAME_VOICEFILE,						"2b9aea838f74b4eecfb29a8f205a2bd4"}
};

static GameFileDescription IHNM_CD_FR_GameFiles[] = {
	{"musicfm.res",	GAME_MUSICFILE_FM,					"0439083e3dfdc51b486071d45872ae52"},
	{"musicgm.res",	GAME_MUSICFILE_GM,					"80f875a1fb384160d1f4b27166eef583"},
	{"scream.res",	GAME_RESOURCEFILE,					"c92370d400e6f2a3fc411c3729d09224"},
	{"scripts.res",	GAME_SCRIPTFILE,					"32aa01a89937520fe0ea513950117292"},
	{"patch.re_",	GAME_PATCHFILE | GAME_RESOURCEFILE,	"58b79e61594779513c7f2d35509fa89e"},
	{"sfx.res",		GAME_SOUNDFILE,						"1c610d543f32ec8b525e3f652536f269"},
	{"voicess.res",	GAME_VOICEFILE,						"b8642e943bbebf89cef2f48b31cb4305"}, //order of voice bank file is important
	{"voices1.res",	GAME_VOICEFILE,						"424971e1e2373187c3f5734fe36071a2"},
	{"voices2.res",	GAME_VOICEFILE,						"c2d93a35d2c2def9c3d6d242576c794b"},
	{"voices3.res",	GAME_VOICEFILE,						"49e42befea883fd101ec3d0f5d0647b9"},
	{"voices5.res",	GAME_VOICEFILE,						"f4c415de7c03de86b73f9a12b8bd632f"},
	{"voices6.res",	GAME_VOICEFILE,						"3fc5358a5d8eee43bdfab2740276572e"}
};

static GameFontDescription IHNMDEMO_GameFonts[] = {
	{2},
	{3},
	{4}
};

static GameFontDescription IHNMCD_GameFonts[] = {
	{2},
	{3},
	{4},
	{5},
	{6},  // kIHNMFont8
	{7},
	{8}   // kIHNMMainFont
};

static GameSoundInfo IHNM_GameSound = {
	kSoundWAV,
	-1,
	-1,
	false,
	false,
	true
};

#define FILE_MD5_BYTES 5000

static GameDescription gameDescriptions[] = {
	// Inherit the earth - DOS Demo version
	// sound unchecked
	{
		"ite",
		GType_ITE,
		GID_ITE_DEMO_G, // Game id
		"Demo", // Game title
		&ITE_DisplayInfo,
		ITE_DEFAULT_SCENE, // Starting scene number
		&ITEDemo_Resources,
		ARRAYSIZE(ITE_DEMO_G_GameFiles), // Game datafiles
		ITE_DEMO_G_GameFiles,
		ARRAYSIZE(ITEDEMO_GameFonts),
		ITEDEMO_GameFonts,
		&ITEDEMO_GameSound,
		&ITEDEMO_GameSound,
		NULL,
		0,
		NULL,
		0, // features
		Common::EN_ANY,
		Common::kPlatformPC,
	},

	// Inherit the earth - MAC Demo version
	{
		"ite",
		GType_ITE,
		GID_ITE_MACDEMO2,
		"Demo",
		&ITE_DisplayInfo,
		ITE_DEFAULT_SCENE,
		&ITE_Resources,
		ARRAYSIZE(ITE_MACDEMO2_GameFiles),
		ITE_MACDEMO2_GameFiles,
		ARRAYSIZE(ITEWINDEMO_GameFonts),
		ITEWINDEMO_GameFonts,
		&ITEMACDEMO_GameVoice,
		&ITEMACDEMO_GameSound,
		&ITEMACDEMO_GameMusic,
		ARRAYSIZE(ITEMacPatch_Files),
		ITEMacPatch_Files,
		GF_BIG_ENDIAN_DATA | GF_WYRMKEEP | GF_CD_FX | GF_SCENE_SUBSTITUTES,
		Common::EN_ANY,
		Common::kPlatformMacintosh,
	},

	// Inherit the earth - early MAC Demo version
	{
		"ite",
		GType_ITE,
		GID_ITE_MACDEMO1,
		"early Demo",
		&ITE_DisplayInfo,
		ITE_DEFAULT_SCENE,
		&ITE_Resources,
		ARRAYSIZE(ITE_MACDEMO1_GameFiles),
		ITE_MACDEMO1_GameFiles,
		ARRAYSIZE(ITEWINDEMO_GameFonts),
		ITEWINDEMO_GameFonts,
		&ITEMACDEMO_GameVoice,
		&ITEMACDEMO_GameSound,
		&ITEMACCD_GameMusic,
		ARRAYSIZE(ITEMacPatch_Files),
		ITEMacPatch_Files,
		GF_BIG_ENDIAN_DATA | GF_WYRMKEEP | GF_CD_FX,
		Common::EN_ANY,
		Common::kPlatformMacintosh,
	},

	// Inherit the earth - MAC CD Guild version
	{
		"ite",
		GType_ITE,
		GID_ITE_MACCD_G,
		"CD",
		&ITE_DisplayInfo,
		ITE_DEFAULT_SCENE,
		&ITE_Resources,
		ARRAYSIZE(ITE_MACCD_G_GameFiles),
		ITE_MACCD_G_GameFiles,
		ARRAYSIZE(ITEWINDEMO_GameFonts),
		ITEWINDEMO_GameFonts,
		&ITEMACCD_G_GameSound,
		&ITEMACCD_G_GameSound,
		NULL,
		0,
		NULL,
		GF_BIG_ENDIAN_DATA | GF_CD_FX,
		Common::EN_ANY,
		Common::kPlatformMacintosh,
	},

	// Inherit the earth - MAC CD Wyrmkeep version
	{
		"ite",
		GType_ITE,
		GID_ITE_MACCD,
		"Wyrmkeep CD",
		&ITE_DisplayInfo,
		ITE_DEFAULT_SCENE,
		&ITE_Resources,
		ARRAYSIZE(ITE_MACCD_GameFiles),
		ITE_MACCD_GameFiles,
		ARRAYSIZE(ITEWINDEMO_GameFonts),
		ITEWINDEMO_GameFonts,
		&ITEMACCD_GameSound,
		&ITEMACCD_GameSound,
		&ITEMACCD_GameMusic,
		ARRAYSIZE(ITEMacPatch_Files),
		ITEMacPatch_Files,
		GF_BIG_ENDIAN_DATA | GF_WYRMKEEP | GF_CD_FX,
		Common::EN_ANY,
		Common::kPlatformMacintosh,
	},

	// Inherit the earth - Linux Demo version
	// Note: it should be before GID_ITE_WINDEMO2 version
	{
		"ite",
		GType_ITE,
		GID_ITE_LINDEMO,
		"Demo",
		&ITE_DisplayInfo,
		ITE_DEFAULT_SCENE,
		&ITE_Resources,
		ARRAYSIZE(ITE_LINDEMO_GameFiles),
		ITE_LINDEMO_GameFiles,
		ARRAYSIZE(ITEWINDEMO_GameFonts),
		ITEWINDEMO_GameFonts,
		&ITEWINDEMO2_GameVoice,
		&ITEWINDEMO2_GameSound,
		&ITELINDEMO_GameMusic,
		ARRAYSIZE(ITELinPatch_Files),
		ITELinPatch_Files,
		GF_WYRMKEEP | GF_CD_FX | GF_SCENE_SUBSTITUTES,
		Common::EN_ANY,
		Common::kPlatformLinux,
	},

	// Inherit the earth - Win32 Demo version
	{
		"ite",
		GType_ITE,
		GID_ITE_WINDEMO2,
		"Demo",
		&ITE_DisplayInfo,
		ITE_DEFAULT_SCENE,
		&ITE_Resources,
		ARRAYSIZE(ITE_WINDEMO2_GameFiles),
		ITE_WINDEMO2_GameFiles,
		ARRAYSIZE(ITEWINDEMO_GameFonts),
		ITEWINDEMO_GameFonts,
		&ITEWINDEMO2_GameVoice,
		&ITEWINDEMO2_GameSound,
		NULL,
		ARRAYSIZE(ITEWinPatch2_Files),
		ITEWinPatch2_Files,
		GF_WYRMKEEP | GF_CD_FX | GF_SCENE_SUBSTITUTES,
		Common::EN_ANY,
		Common::kPlatformWindows,
	},

	// Inherit the earth - early Win32 Demo version
	{
		"ite",
		GType_ITE,
		GID_ITE_WINDEMO1,
		"early Demo",
		&ITE_DisplayInfo,
		ITE_DEFAULT_SCENE,
		&ITE_Resources,
		ARRAYSIZE(ITE_WINDEMO1_GameFiles),
		ITE_WINDEMO1_GameFiles,
		ARRAYSIZE(ITEWINDEMO_GameFonts),
		ITEWINDEMO_GameFonts,
		&ITEWINDEMO1_GameSound,
		&ITEWINDEMO1_GameSound,
		NULL,
		ARRAYSIZE(ITEWinPatch1_Files),
		ITEWinPatch1_Files,
		GF_WYRMKEEP | GF_CD_FX,
		Common::EN_ANY,
		Common::kPlatformWindows,
	},

	// Inherit the earth - Wyrmkeep combined Windows/Mac/Linux CD
	{
		"ite",
		GType_ITE,
		GID_ITE_MULTICD,
		"Multi-OS CD Version",
		&ITE_DisplayInfo,
		ITE_DEFAULT_SCENE,
		&ITE_Resources,
		ARRAYSIZE(ITE_MULTICD_GameFiles),
		ITE_MULTICD_GameFiles,
		ARRAYSIZE(ITECD_GameFonts),
		ITECD_GameFonts,
		&ITEMACCD_GameSound,
		&ITECD_GameSound,
		&ITEMACCD_GameMusic,
		0,
		NULL,
		GF_WYRMKEEP | GF_CD_FX,
		Common::EN_ANY,
		Common::kPlatformUnknown,
	},

	// Inherit the earth - Wyrmkeep Linux CD version
	{
		"ite",
		GType_ITE,
		GID_ITE_LINCD,
		"CD Version",
		&ITE_DisplayInfo,
		ITE_DEFAULT_SCENE,
		&ITE_Resources,
		ARRAYSIZE(ITE_LINCD_GameFiles),
		ITE_LINCD_GameFiles,
		ARRAYSIZE(ITECD_GameFonts),
		ITECD_GameFonts,
		&ITECD_GameSound,
		&ITECD_GameSound,
		&ITEMACCD_GameMusic,
		ARRAYSIZE(ITELinPatch_Files),
		ITELinPatch_Files,
		GF_WYRMKEEP | GF_CD_FX,
		Common::EN_ANY,
		Common::kPlatformLinux,
	},

	// Inherit the earth - Wyrmkeep Windows CD version
	{
		"ite",
		GType_ITE,
		GID_ITE_WINCD,
		"CD Version",
		&ITE_DisplayInfo,
		ITE_DEFAULT_SCENE,
		&ITE_Resources,
		ARRAYSIZE(ITE_WINCD_GameFiles),
		ITE_WINCD_GameFiles,
		ARRAYSIZE(ITECD_GameFonts),
		ITECD_GameFonts,
		&ITECD_GameSound,
		&ITECD_GameSound,
		NULL,
		ARRAYSIZE(ITEWinPatch1_Files),
		ITEWinPatch1_Files,
		GF_WYRMKEEP | GF_CD_FX,
		Common::EN_ANY,
		Common::kPlatformWindows,
	},

	// Inherit the earth - DOS CD version
	{
		"ite",
		GType_ITE,
		GID_ITE_CD_G,
		"CD Version",
		&ITE_DisplayInfo,
		ITE_DEFAULT_SCENE,
		&ITE_Resources,
		ARRAYSIZE(ITE_CD_G_GameFiles),
		ITE_CD_G_GameFiles,
		ARRAYSIZE(ITECD_GameFonts),
		ITECD_GameFonts,
		&ITECD_GameSound,
		&ITECD_GameSound,
		NULL,
		0,
		NULL,
		GF_CD_FX,
		Common::EN_ANY,
		Common::kPlatformPC,
	},

	// Inherit the earth - DOS CD version with digital music
	{
		"ite",
		GType_ITE,
		GID_ITE_CD_G2,
		"CD Version",
		&ITE_DisplayInfo,
		ITE_DEFAULT_SCENE,
		&ITE_Resources,
		ARRAYSIZE(ITE_CD_G2_GameFiles),
		ITE_CD_G2_GameFiles,
		ARRAYSIZE(ITECD_GameFonts),
		ITECD_GameFonts,
		&ITECD_GameSound,
		&ITECD_GameSound,
		&ITEMACCD_GameMusic,
		0,
		NULL,
		GF_CD_FX,
		Common::EN_ANY,
		Common::kPlatformPC,
	},

	// Inherit the earth - DOS CD German version
	{
		"ite",
		GType_ITE,
		GID_ITE_CD_DE,
		"CD Version",
		&ITE_DisplayInfo,
		ITE_DEFAULT_SCENE,
		&ITE_Resources,
		ARRAYSIZE(ITE_CD_DE_GameFiles),
		ITE_CD_DE_GameFiles,
		ARRAYSIZE(ITECD_GameFonts),
		ITECD_GameFonts,
		&ITECD_GameSound,
		&ITECD_GameSound,
		NULL,
		0,
		NULL,
		GF_CD_FX,
		Common::DE_DEU,
		Common::kPlatformPC,
	},

	// Inherit the earth - DOS CD German version with digital music
	{
		"ite",
		GType_ITE,
		GID_ITE_CD_DE2,
		"CD Version",
		&ITE_DisplayInfo,
		ITE_DEFAULT_SCENE,
		&ITE_Resources,
		ARRAYSIZE(ITE_CD_DE2_GameFiles),
		ITE_CD_DE2_GameFiles,
		ARRAYSIZE(ITECD_GameFonts),
		ITECD_GameFonts,
		&ITECD_GameSound,
		&ITECD_GameSound,
		&ITEMACCD_GameMusic,
		0,
		NULL,
		GF_CD_FX,
		Common::DE_DEU,
		Common::kPlatformPC,
	},

	// Inherit the earth - CD version
	{
		"ite",
		GType_ITE,
		GID_ITE_CD,
		"CD Version",
		&ITE_DisplayInfo,
		ITE_DEFAULT_SCENE,
		&ITE_Resources,
		ARRAYSIZE(ITE_CD_GameFiles),
		ITE_CD_GameFiles,
		ARRAYSIZE(ITECD_GameFonts),
		ITECD_GameFonts,
		&ITECD_GameSound,
		&ITECD_GameSound,
		NULL,
		0,
		NULL,
		GF_CD_FX,
		Common::EN_ANY,
		Common::kPlatformPC,
	},

	// Inherit the earth - German Floppy version
	{
		"ite",
		GType_ITE,
		GID_ITE_DISK_DE,
		"Floppy",
		&ITE_DisplayInfo,
		ITE_DEFAULT_SCENE,
		&ITE_Resources,
		ARRAYSIZE(ITE_DISK_DE_GameFiles),
		ITE_DISK_DE_GameFiles,
		ARRAYSIZE(ITEDISK_GameFonts),
		ITEDISK_GameFonts,
		&ITEDISK_GameSound,
		&ITEDISK_GameSound,
		NULL,
		0,
		NULL,
		0,
		Common::DE_DEU,
		Common::kPlatformPC,
	},

	// Inherit the earth - German Floppy version with digital music
	{
		"ite",
		GType_ITE,
		GID_ITE_DISK_DE2,
		"Floppy",
		&ITE_DisplayInfo,
		ITE_DEFAULT_SCENE,
		&ITE_Resources,
		ARRAYSIZE(ITE_DISK_DE2_GameFiles),
		ITE_DISK_DE2_GameFiles,
		ARRAYSIZE(ITEDISK_GameFonts),
		ITEDISK_GameFonts,
		&ITEDISK_GameSound,
		&ITEDISK_GameSound,
		&ITEMACCD_GameMusic,
		0,
		NULL,
		0,
		Common::DE_DEU,
		Common::kPlatformPC,
	},

	// Inherit the earth - Disk version
	{
		"ite",
		GType_ITE,
		GID_ITE_DISK_G,
		"Floppy",
		&ITE_DisplayInfo,
		ITE_DEFAULT_SCENE,
		&ITE_Resources,
		ARRAYSIZE(ITE_DISK_G_GameFiles),
		ITE_DISK_G_GameFiles,
		ARRAYSIZE(ITEDISK_GameFonts),
		ITEDISK_GameFonts,
		&ITEDISK_GameSound,
		&ITEDISK_GameSound,
		NULL,
		0,
		NULL,
		0,
		Common::EN_ANY,
		Common::kPlatformPC,
	},

	// Inherit the earth - Disk version with digital music
	{
		"ite",
		GType_ITE,
		GID_ITE_DISK_G2,
		"Floppy",
		&ITE_DisplayInfo,
		ITE_DEFAULT_SCENE,
		&ITE_Resources,
		ARRAYSIZE(ITE_DISK_G2_GameFiles),
		ITE_DISK_G2_GameFiles,
		ARRAYSIZE(ITEDISK_GameFonts),
		ITEDISK_GameFonts,
		&ITEDISK_GameSound,
		&ITEDISK_GameSound,
		&ITEMACCD_GameMusic,
		0,
		NULL,
		0,
		Common::EN_ANY,
		Common::kPlatformPC,
	},

	// I Have No Mouth And I Must Scream - Demo version
	{
		"ihnm",
		GType_IHNM,
		GID_IHNM_DEMO,
		"Demo",
		&IHNM_DisplayInfo,
		0,
		&IHNM_Resources,
		ARRAYSIZE(IHNM_DEMO_GameFiles),
		IHNM_DEMO_GameFiles,
		ARRAYSIZE(IHNMDEMO_GameFonts),
		IHNMDEMO_GameFonts,
		&IHNM_GameSound,
		&IHNM_GameSound,
		NULL,
		0,
		NULL,
		0,
		Common::EN_ANY,
		Common::kPlatformPC,
	},

	// I Have No Mouth And I Must Scream - CD version
	{
		"ihnm",
		GType_IHNM,
		GID_IHNM_CD,
		"",
		&IHNM_DisplayInfo,
		IHNM_DEFAULT_SCENE,
		&IHNM_Resources,
		ARRAYSIZE(IHNM_CD_GameFiles),
		IHNM_CD_GameFiles,
		ARRAYSIZE(IHNMCD_GameFonts),
		IHNMCD_GameFonts,
		&IHNM_GameSound,
		&IHNM_GameSound,
		NULL,
		0,
		NULL,
		0,
		Common::EN_ANY,
		Common::kPlatformPC,
	},

	// I Have No Mouth And I Must Scream - De CD version
	{
		"ihnm",
		GType_IHNM,
		GID_IHNM_CD_DE,
		"",
		&IHNM_DisplayInfo,
		IHNM_DEFAULT_SCENE,
		&IHNM_Resources,
		ARRAYSIZE(IHNM_CD_DE_GameFiles),
		IHNM_CD_DE_GameFiles,
		ARRAYSIZE(IHNMCD_GameFonts),
		IHNMCD_GameFonts,
		&IHNM_GameSound,
		&IHNM_GameSound,
		NULL,
		0,
		NULL,
		0,
		Common::DE_DEU,
		Common::kPlatformPC,
	},
	// I Have No Mouth And I Must Scream - Sp CD version
	{
		"ihnm",
		GType_IHNM,
		GID_IHNM_CD_ES,
		"",
		&IHNM_DisplayInfo,
		IHNM_DEFAULT_SCENE,
		&IHNM_Resources,
		ARRAYSIZE(IHNM_CD_ES_GameFiles),
		IHNM_CD_ES_GameFiles,
		ARRAYSIZE(IHNMCD_GameFonts),
		IHNMCD_GameFonts,
		&IHNM_GameSound,
		&IHNM_GameSound,
		NULL,
		0,
		NULL,
		0,
		Common::ES_ESP,
		Common::kPlatformPC,
	},
	// I Have No Mouth And I Must Scream - Ru CD version
	{
		"ihnm",
		GType_IHNM,
		GID_IHNM_CD_RU,
		"",
		&IHNM_DisplayInfo,
		IHNM_DEFAULT_SCENE,
		&IHNM_Resources,
		ARRAYSIZE(IHNM_CD_RU_GameFiles),
		IHNM_CD_RU_GameFiles,
		ARRAYSIZE(IHNMCD_GameFonts),
		IHNMCD_GameFonts,
		&IHNM_GameSound,
		&IHNM_GameSound,
		NULL,
		0,
		NULL,
		0,
		Common::RU_RUS,
		Common::kPlatformPC,
	},
	// I Have No Mouth And I Must Scream - Fr CD version
	{
		"ihnm",
		GType_IHNM,
		GID_IHNM_CD_FR,
		"",
		&IHNM_DisplayInfo,
		IHNM_DEFAULT_SCENE,
		&IHNM_Resources,
		ARRAYSIZE(IHNM_CD_FR_GameFiles),
		IHNM_CD_FR_GameFiles,
		ARRAYSIZE(IHNMCD_GameFonts),
		IHNMCD_GameFonts,
		&IHNM_GameSound,
		&IHNM_GameSound,
		NULL,
		0,
		NULL,
		0,
		Common::FR_FRA,
		Common::kPlatformPC,
	},
};

DetectedGame toDetectedGame(const GameDescription &g) {
	const char *title;
	title = saga_games[g.gameType].description;
	DetectedGame dg(g.name, title, g.language, g.platform);
	dg.updateDesc(g.extra);
	return dg;
}

static int detectGame(const FSList *fslist, Common::Language language, Common::Platform platform, int*& returnMatches) {
	int gamesCount = ARRAYSIZE(gameDescriptions);
	int filesCount;

	typedef Common::HashMap<Common::String, bool> StringSet;
	StringSet filesList;

	typedef Common::HashMap<Common::String, Common::String> StringMap;
	StringMap filesMD5;

	Common::String tstr;
	
	int i, j;
	char md5str[32+1];
	uint8 md5sum[16];

	int matched[ARRAYSIZE(gameDescriptions)];
	int matchedCount = 0;
	bool fileMissing;
	GameFileDescription *fileDesc;

	// First we compose list of files which we need MD5s for
	for (i = 0; i < gamesCount; i++) {
		for (j = 0; j < gameDescriptions[i].filesCount; j++) {
			tstr = Common::String(gameDescriptions[i].filesDescriptions[j].fileName);
			tstr.toLowercase();
			filesList[tstr] = true;
		}
	}
	
	if (fslist != NULL) {
		for (FSList::const_iterator file = fslist->begin(); file != fslist->end(); ++file) {
			if (file->isDirectory()) continue;
			tstr = file->displayName();
			tstr.toLowercase();

			if (!filesList.contains(tstr)) continue;

			if (!Common::md5_file(file->path().c_str(), md5sum, FILE_MD5_BYTES)) continue;
			for (j = 0; j < 16; j++) {
				sprintf(md5str + j*2, "%02x", (int)md5sum[j]);
			}
			filesMD5[tstr] = Common::String(md5str);
		}
	} else {
		Common::File testFile;

		for (StringSet::const_iterator file = filesList.begin(); file != filesList.end(); ++file) {
			tstr = file->_key;
			tstr.toLowercase();

			if(!filesMD5.contains(tstr)) {
				if (testFile.open(file->_key)) {
					testFile.close();

					if (Common::md5_file(file->_key.c_str(), md5sum, FILE_MD5_BYTES)) {
						for (j = 0; j < 16; j++) {
							sprintf(md5str + j*2, "%02x", (int)md5sum[j]);
						}
						filesMD5[tstr] = Common::String(md5str);
					}
				}
			}
		}
	}

	for (i = 0; i < gamesCount; i++) {
		filesCount = gameDescriptions[i].filesCount;		
		fileMissing = false;

		// Try to open all files for this game
		for (j = 0; j < filesCount; j++) {
			fileDesc = &gameDescriptions[i].filesDescriptions[j];
			tstr = fileDesc->fileName;
			tstr.toLowercase();

			if (!filesMD5.contains(tstr)) {

				if ((fileDesc->fileType & (GAME_SOUNDFILE | GAME_VOICEFILE | GAME_MUSICFILE)) != 0) {
					//TODO: find recompressed files
				}
				fileMissing = true;
				break;
			}
			if (strcmp(fileDesc->md5, filesMD5[tstr].c_str())) {
				fileMissing = true;
				break;
			}
		}
		if (!fileMissing) {
			debug(2, "Found game: %s", toDetectedGame(gameDescriptions[i]).description.c_str());
			matched[matchedCount++] = i;
		}
	}

	if (!filesMD5.empty() && (matchedCount == 0)) {
		printf("MD5s of your game version are unknown. Please, report following data to\n");
		printf("ScummVM team along with your game name and version:\n");

		for (StringMap::const_iterator file = filesMD5.begin(); file != filesMD5.end(); ++file)
			printf("%s: %s\n", file->_key.c_str(), file->_value.c_str());
	}

	// We have some resource sets which are superpositions of other
	// Particularly it is ite-demo-linux vs ite-demo-win
	// Now remove lesser set if bigger matches too

	if (matchedCount > 1) {
		// Search max number
		int maxcount = 0;
		for (i = 0; i < matchedCount; i++) {
			maxcount = MAX(gameDescriptions[matched[i]].filesCount, maxcount);
		}

		// Now purge targets with number of files lesser than max
		for (i = 0; i < matchedCount; i++) {
			if ((gameDescriptions[matched[i]].language != language && language != Common::UNK_LANG) ||
				(gameDescriptions[matched[i]].platform != platform && platform != Common::kPlatformUnknown)) {
				debug(2, "Purged %s", toDetectedGame(gameDescriptions[matched[i]]).description.c_str());
				matched[i] = -1;
				continue;
			}

			if (gameDescriptions[matched[i]].filesCount < maxcount) {
				debug(2, "Purged: %s", toDetectedGame(gameDescriptions[matched[i]]).description.c_str());
				matched[i] = -1;
			}
		}
	}


	returnMatches = (int *)malloc(matchedCount * sizeof(int));
	j = 0;
	for (i = 0; i < matchedCount; i++)
		if (matched[i] != -1)
			returnMatches[j++] = matched[i];
	return j;
}

bool SagaEngine::initGame() {
	uint16 gameCount = ARRAYSIZE(gameDescriptions);
	int gameNumber = -1;
	
	DetectedGameList detectedGames;
	int count;
	int* matches;
	Common::Language language = Common::UNK_LANG;
	Common::Platform platform = Common::kPlatformUnknown;

	if (ConfMan.hasKey("language"))
		language = Common::parseLanguage(ConfMan.get("language"));
	if (ConfMan.hasKey("platform"))
		platform = Common::parsePlatform(ConfMan.get("platform"));


	count = detectGame(NULL, language, platform, matches);

	if (count == 0) {
		warning("No valid games were found in the specified directory.");
		return false;
	}

	if (count != 1)
		warning("Conflicting targets detected (%d)", count);

	gameNumber = matches[0];

	free(matches);

	if (gameNumber >= gameCount || gameNumber == -1) {
		error("SagaEngine::loadGame wrong gameNumber");
	}

	_gameTitle = toDetectedGame(gameDescriptions[gameNumber]).description;
	debug(2, "Running %s", _gameTitle.c_str());

	_gameNumber = gameNumber;
	_gameDescription = &gameDescriptions[gameNumber];
	_gameDisplayInfo = *_gameDescription->gameDisplayInfo;
	_displayClip.right = _gameDisplayInfo.logicalWidth;
	_displayClip.bottom = _gameDisplayInfo.logicalHeight;

	if (!_resource->createContexts()) {
		return false;
	}
	return true;
}

DetectedGameList GAME_detectGames(const FSList &fslist) {
	DetectedGameList detectedGames;
	int count;
	int* matches;
	count = detectGame(&fslist, Common::UNK_LANG, Common::kPlatformUnknown, matches);

	for (int i = 0; i < count; i++)
		detectedGames.push_back(toDetectedGame(gameDescriptions[matches[i]]));
	free(matches);
	return detectedGames;
}

} // End of namespace Saga
