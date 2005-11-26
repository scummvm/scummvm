/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004-2005 The ScummVM project
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
 * $Header$
 *
 */

// Game detection, general game parameters

#include "saga/saga.h"

#include "common/file.h"
#include "common/md5.h"
#include "common/map.h"
#include "common/config-manager.h"
#include "base/plugins.h"
#include "base/gameDetector.h"
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

static int detectGame(const FSList &fslist, bool mode = false, int start = -1);

// ITE section
static PanelButton ITE_MainPanelButtons[] = {
	{kPanelButtonVerb,		52,4,	57,10,	kVerbWalkTo,'w',0,	0,1,0},
	{kPanelButtonVerb,		52,15,	57,10,	kVerbLookAt,'l',0,	2,3,0},
	{kPanelButtonVerb,		52,26,	57,10,	kVerbPickUp,'p',0,	4,5,0},
	{kPanelButtonVerb,		52,37,	57,10,	kVerbTalkTo,'t',0,	0,1,0},
	{kPanelButtonVerb,		110,4,	56,10,	kVerbOpen,'o',0,	6,7,0},
	{kPanelButtonVerb,		110,15,	56,10,	kVerbClose,'c',0,	8,9,0},
	{kPanelButtonVerb,		110,26,	56,10,	kVerbUse,'u',0,		10,11,0},
	{kPanelButtonVerb,		110,37,	56,10,	kVerbGive,'g',0,	12,13,0},
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
static GameFileDescription ITEDEMO_GameFiles[] = {
	{"ite.rsc", GAME_RESOURCEFILE},
	//{"ite.dmo", GAME_DEMOFILE},
	{"scripts.rsc", GAME_SCRIPTFILE},
	{"voices.rsc", GAME_SOUNDFILE | GAME_VOICEFILE}
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
static GameFileDescription ITEWINDEMO_GameFiles[] = {
	{"ited.rsc", GAME_RESOURCEFILE},
	{"scriptsd.rsc", GAME_SCRIPTFILE},
	{"soundsd.rsc", GAME_SOUNDFILE},
	{"voicesd.rsc", GAME_VOICEFILE}
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
static GameFileDescription ITEMACDEMO_GameFiles[] = {
	{"ited.rsc", GAME_RESOURCEFILE},
	{"scriptsd.rsc", GAME_SCRIPTFILE},
	{"soundsd.rsc", GAME_SOUNDFILE},
	{"voicesd.rsc", GAME_VOICEFILE},
	{"musicd.rsc", GAME_MUSICFILE}
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
static GameFileDescription ITELINDEMO_GameFiles[] = {
	{"ited.rsc", GAME_RESOURCEFILE},
	{"scriptsd.rsc", GAME_SCRIPTFILE},
	{"soundsd.rsc", GAME_SOUNDFILE},
	{"voicesd.rsc", GAME_VOICEFILE},
	{"musicd.rsc", GAME_MUSICFILE}
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
static GameFileDescription ITELINCD_GameFiles[] = {
	{"ite.rsc", GAME_RESOURCEFILE},
	{"scripts.rsc", GAME_SCRIPTFILE},
	{"sounds.rsc", GAME_SOUNDFILE},
	{"voices.rsc", GAME_VOICEFILE},
	{"music.rsc", GAME_MUSICFILE}
};

// Inherit the Earth - Wyrmkeep combined Windows/Mac/Linux version. This
// version is different from the other Wyrmkeep re-releases in that it does
// not have any substitute files. Presumably the ite.rsc file has been
// modified to include the Wyrmkeep changes. The resource files are little-
// endian, except for the voice file which is big-endian.

static GameFileDescription ITEMULTICD_GameFiles[] = {
	{"ite.rsc", GAME_RESOURCEFILE},
	{"scripts.rsc", GAME_SCRIPTFILE},
	{"sounds.rsc", GAME_SOUNDFILE},
	{"Inherit the Earth Voices", GAME_VOICEFILE | GAME_SWAPENDIAN},
	{"music.rsc", GAME_MUSICFILE}
};

static GameFileDescription ITEMACCD_G_GameFiles[] = {
	{"ITE Resources.bin", GAME_RESOURCEFILE | GAME_MACBINARY},
	{"ITE Scripts.bin", GAME_SCRIPTFILE | GAME_MACBINARY},
	{"ITE Sounds.bin", GAME_SOUNDFILE | GAME_MACBINARY},
	{"ITE Music.bin", GAME_MUSICFILE_GM | GAME_MACBINARY},
	{"ITE Voices.bin", GAME_VOICEFILE | GAME_MACBINARY}
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
static GameFileDescription ITEMACCD_GameFiles[] = {
	{"ite.rsc", GAME_RESOURCEFILE},
	{"scripts.rsc", GAME_SCRIPTFILE},
	{"sounds.rsc", GAME_SOUNDFILE},
	{"Inherit the Earth Voices", GAME_VOICEFILE},
	{"music.rsc", GAME_MUSICFILE}
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
static GameFileDescription ITEDISK_GameFiles[] = {
	{"ite.rsc", GAME_RESOURCEFILE},
	{"scripts.rsc", GAME_SCRIPTFILE},
	{"voices.rsc", GAME_SOUNDFILE | GAME_VOICEFILE}
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
static GameFileDescription ITECD_GameFiles[] = {
	{"ite.rsc", GAME_RESOURCEFILE},
	{"scripts.rsc", GAME_SCRIPTFILE},
	{"sounds.rsc", GAME_SOUNDFILE},
	{"voices.rsc", GAME_VOICEFILE}
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
	{kPanelButtonVerb,		106,12,		114,30,	kVerbWalkTo,'w',0,	0,1,0},
	{kPanelButtonVerb,		106,44,		114,30,	kVerbLookAt,'l',0,	2,3,0},
	{kPanelButtonVerb,		106,76,		114,30, kVerbTake,'k',0,	4,5,0},
	{kPanelButtonVerb,		106,108,	114,30, kVerbUse,'u',0,		6,7,0},
	{kPanelButtonVerb,		223,12,		114,30, kVerbTalkTo,'t',0,	8,9,0},
	{kPanelButtonVerb,		223,44,		114,30, kVerbSwallow,'s',0,	10,11,0},
	{kPanelButtonVerb,		223,76,		114,30, kVerbGive,'g',0,	12,13,0},
	{kPanelButtonVerb,		223,108,	114,30, kVerbPush,'p',0,	14,15,0},
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
static GameFileDescription IHNMDEMO_GameFiles[] = {
	{"scream.res", GAME_RESOURCEFILE},
	{"scripts.res", GAME_SCRIPTFILE},
	{"sfx.res", GAME_SOUNDFILE},
	{"voicesd.res", GAME_VOICEFILE}
};

// I Have No Mouth and I Must Scream - Retail CD version
static GameFileDescription IHNMCD_GameFiles[] = {
	{"musicfm.res", GAME_MUSICFILE_FM},
	{"musicgm.res", GAME_MUSICFILE_GM},
	{"scream.res", GAME_RESOURCEFILE},
	{"patch.re_", GAME_PATCHFILE | GAME_RESOURCEFILE},
	{"scripts.res", GAME_SCRIPTFILE},
	{"sfx.res", GAME_SOUNDFILE},
	{"voicess.res", GAME_VOICEFILE}, //order of voice bank file is important
	{"voices1.res", GAME_VOICEFILE},
	{"voices2.res", GAME_VOICEFILE},
	{"voices3.res", GAME_VOICEFILE},
	{"voices4.res", GAME_VOICEFILE},
	{"voices5.res", GAME_VOICEFILE},
	{"voices6.res", GAME_VOICEFILE}
};

// I Have No Mouth and I Must Scream - Censored CD version (without Nimdok)
static GameFileDescription IHNMCD_Censored_GameFiles[] = {
	{"musicfm.res", GAME_MUSICFILE_FM},
	{"musicgm.res", GAME_MUSICFILE_GM},
	{"scream.res", GAME_RESOURCEFILE},
	{"scripts.res", GAME_SCRIPTFILE},
	{"patch.re_", GAME_PATCHFILE | GAME_RESOURCEFILE},
	{"sfx.res", GAME_SOUNDFILE},
	{"voicess.res", GAME_VOICEFILE}, //order of voice bank file is important
	{"voices1.res", GAME_VOICEFILE},
	{"voices2.res", GAME_VOICEFILE},
	{"voices3.res", GAME_VOICEFILE},
	{"voices5.res", GAME_VOICEFILE},
	{"voices6.res", GAME_VOICEFILE}
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

struct GameMD5 {
	GameIds id;
	const char *md5;
	const char *filename;
	bool caseSensitive;
};

#define FILE_MD5_BYTES 5000

static GameMD5 gameMD5[] = {
	{ GID_ITE_DISK_G,   "8f4315a9bb10ec839253108a032c8b54", "ite.rsc", false },
	{ GID_ITE_DISK_G,   "516f7330f8410057b834424ea719d1ef", "scripts.rsc", false },
	{ GID_ITE_DISK_G,   "c46e4392fcd2e89bc91e5567db33b62d", "voices.rsc", false },

	{ GID_ITE_CD_G,     "8f4315a9bb10ec839253108a032c8b54", "ite.rsc", false },
	{ GID_ITE_CD_G,     "50a0d2d7003c926a3832d503c8534e90", "scripts.rsc", false },
	{ GID_ITE_CD_G,     "e2ccb61c325d6d1ead3be0e731fe29fe", "sounds.rsc", false },
	{ GID_ITE_CD_G,     "41bb6b95d792dde5196bdb78740895a6", "voices.rsc", false },

	{ GID_ITE_CD,       "8f4315a9bb10ec839253108a032c8b54", "ite.rsc", false },
	{ GID_ITE_CD,       "a891405405edefc69c9d6c420c868b84", "scripts.rsc", false },
	{ GID_ITE_CD,       "e2ccb61c325d6d1ead3be0e731fe29fe", "sounds.rsc", false },
	{ GID_ITE_CD,       "41bb6b95d792dde5196bdb78740895a6", "voices.rsc", false },

	// reported by mld. Bestsellergamers cover disk
	{ GID_ITE_CD_DE,    "869fc23c8f38f575979ec67152914fee", "ite.rsc", false },
	{ GID_ITE_CD_DE,    "a891405405edefc69c9d6c420c868b84", "scripts.rsc", false },
	{ GID_ITE_CD_DE,    "e2ccb61c325d6d1ead3be0e731fe29fe", "sounds.rsc", false },
	{ GID_ITE_CD_DE,    "2fbad5d10b9b60a3415dc4aebbb11718", "voices.rsc", false },

	{ GID_ITE_DEMO_G,   "986c79c4d2939dbe555576529fd37932", "ite.rsc", false },
	{ GID_ITE_DEMO_G,   "d5697dd3240a3ceaddaa986c47e1a2d7", "scripts.rsc", false },
	{ GID_ITE_DEMO_G,   "c58e67c506af4ffa03fd0aac2079deb0", "voices.rsc", false },
	{ GID_ITE_DEMO_G,   "0b9a70eb4e120b6f00579b46c8cae29e", "ite.dmo", false },

	{ GID_ITE_WINCD,    "8f4315a9bb10ec839253108a032c8b54", "ite.rsc", false },
	{ GID_ITE_WINCD,    "a891405405edefc69c9d6c420c868b84", "scripts.rsc", false },
	{ GID_ITE_WINCD,    "e2ccb61c325d6d1ead3be0e731fe29fe", "sounds.rsc", false },
	{ GID_ITE_WINCD,    "41bb6b95d792dde5196bdb78740895a6", "voices.rsc", false },

	{ GID_ITE_MACCD,    "4f7fa11c5175980ed593392838523060", "ite.rsc", false },
	{ GID_ITE_MACCD,    "adf1f46c1d0589083996a7060c798ad0", "scripts.rsc", false },
	{ GID_ITE_MACCD,    "1a91cd60169f367ecb6c6e058d899b2f", "music.rsc", false },
	{ GID_ITE_MACCD,    "95863b89a0916941f6c5e1789843ba14", "sounds.rsc", false },
	{ GID_ITE_MACCD,    "c14c4c995e7a0d3828e3812a494301b7", "Inherit the Earth Voices", true },

	{ GID_ITE_MACCD_G,  "0bd506aa887bfc7965f695c6bd28237d", "ITE Resources.bin", true },
	{ GID_ITE_MACCD_G,  "af0d7a2588e09ad3ecbc5b474ea238bf", "ITE Scripts.bin", true },
	{ GID_ITE_MACCD_G,  "c1d20324b7cdf1650e67061b8a93251c", "ITE Music.bin", true },
	{ GID_ITE_MACCD_G,  "441426c6bb2a517f65c7e49b57f7a345", "ITE Sounds.bin", true },
	{ GID_ITE_MACCD_G,  "dba92ae7d57e942250fe135609708369", "ITE Voices.bin", true },

	{ GID_ITE_LINCD,    "8f4315a9bb10ec839253108a032c8b54", "ite.rsc", false },
	{ GID_ITE_LINCD,    "a891405405edefc69c9d6c420c868b84", "scripts.rsc", false },
	{ GID_ITE_LINCD,    "e2ccb61c325d6d1ead3be0e731fe29fe", "sounds.rsc", false },
	{ GID_ITE_LINCD,    "41bb6b95d792dde5196bdb78740895a6", "voices.rsc", false },
	{ GID_ITE_LINCD,    "d6454756517f042f01210458abe8edd4", "music.rsc", false },

	{ GID_ITE_MULTICD,  "a6433e34b97b15e64fe8214651012db9", "ite.rsc", false },
	{ GID_ITE_MULTICD,  "a891405405edefc69c9d6c420c868b84", "scripts.rsc", false },
	{ GID_ITE_MULTICD,  "e2ccb61c325d6d1ead3be0e731fe29fe", "sounds.rsc", false },
	{ GID_ITE_MULTICD,  "c14c4c995e7a0d3828e3812a494301b7", "Inherit the Earth Voices", true },
	{ GID_ITE_MULTICD,  "d6454756517f042f01210458abe8edd4", "music.rsc", false },

	{ GID_ITE_DISK_DE,  "869fc23c8f38f575979ec67152914fee", "ite.rsc", false },
	{ GID_ITE_DISK_DE,  "516f7330f8410057b834424ea719d1ef", "scripts.rsc", false },
	{ GID_ITE_DISK_DE,  "0c9113e630f97ef0996b8c3114badb08", "voices.rsc", false },

	{ GID_ITE_WINDEMO2, "3a450852cbf3c80773984d565647e6ac", "ited.rsc", false },
	{ GID_ITE_WINDEMO2, "3f12b67fa93e56e1a6be39d2921d80bb", "scriptsd.rsc", false },
	{ GID_ITE_WINDEMO2, "95a6c148e22e99a8c243f2978223583c", "soundsd.rsc", false },
	{ GID_ITE_WINDEMO2, "e139d86bab2ee8ba3157337f894a92d4", "voicesd.rsc", false },

	{ GID_ITE_LINDEMO,  "3a450852cbf3c80773984d565647e6ac", "ited.rsc", false },
	{ GID_ITE_LINDEMO,  "3f12b67fa93e56e1a6be39d2921d80bb", "scriptsd.rsc", false },
	{ GID_ITE_LINDEMO,  "d6454756517f042f01210458abe8edd4", "musicd.rsc", false },
	{ GID_ITE_LINDEMO,  "95a6c148e22e99a8c243f2978223583c", "soundsd.rsc", false },
	{ GID_ITE_LINDEMO,  "e139d86bab2ee8ba3157337f894a92d4", "voicesd.rsc", false },

	{ GID_ITE_MACDEMO2, "addfc9d82bc2fa1f4cab23743c652c08", "ited.rsc", false },
	{ GID_ITE_MACDEMO2, "fded5c59b8b7c5976229f960d21e6b0b", "scriptsd.rsc", false },
	{ GID_ITE_MACDEMO2, "495bdde51fd9f4bea2b9c911091b1ab2", "musicd.rsc", false },
	{ GID_ITE_MACDEMO2, "b3a831fbed337d1f1300fee1dd474f6c", "soundsd.rsc", false },
	{ GID_ITE_MACDEMO2, "e139d86bab2ee8ba3157337f894a92d4", "voicesd.rsc", false },

	{ GID_ITE_WINDEMO1, "3a450852cbf3c80773984d565647e6ac", "ited.rsc", false },
	{ GID_ITE_WINDEMO1, "3f12b67fa93e56e1a6be39d2921d80bb", "scriptsd.rsc", false },
	{ GID_ITE_WINDEMO1, "a741139dd7365a13f463cd896ff9969a", "soundsd.rsc", false },
	{ GID_ITE_WINDEMO1, "0759eaf5b64ae19fd429920a70151ad3", "voicesd.rsc", false },

	{ GID_ITE_MACDEMO1, "addfc9d82bc2fa1f4cab23743c652c08", "ited.rsc", false },
	{ GID_ITE_MACDEMO1, "fded5c59b8b7c5976229f960d21e6b0b", "scriptsd.rsc", false },
	{ GID_ITE_MACDEMO1, "1a91cd60169f367ecb6c6e058d899b2f", "musicd.rsc", false },
	{ GID_ITE_MACDEMO1, "b3a831fbed337d1f1300fee1dd474f6c", "soundsd.rsc", false },
	{ GID_ITE_MACDEMO1, "e139d86bab2ee8ba3157337f894a92d4", "voicesd.rsc", false },

	{ GID_IHNM_CD, "0439083e3dfdc51b486071d45872ae52", "musicfm.res", false },
	{ GID_IHNM_CD, "80f875a1fb384160d1f4b27166eef583", "musicgm.res", false },
	{ GID_IHNM_CD, "46bbdc65d164ba7e89836a0935eec8e6", "scream.res", false },
	{ GID_IHNM_CD, "be38bbc5a26be809dbf39f13befebd01", "scripts.res", false },
	{ GID_IHNM_CD, "58b79e61594779513c7f2d35509fa89e", "patch.re_", false },
	{ GID_IHNM_CD, "1c610d543f32ec8b525e3f652536f269", "sfx.res", false },
	{ GID_IHNM_CD, "fc6440b38025f4b2cc3ff55c3da5c3eb", "voices1.res", false },
	{ GID_IHNM_CD, "b37f10fd1696ade7d58704ccaaebceeb", "voices2.res", false },
	{ GID_IHNM_CD, "3bbc16a8f741dbb511da506c660a0b54", "voices3.res", false },
	{ GID_IHNM_CD, "ebfa160122d2247a676ca39920e5d481", "voices4.res", false },
	{ GID_IHNM_CD, "1f501ce4b72392bdd1d9ec38f6eec6da", "voices5.res", false },
	{ GID_IHNM_CD, "f580ed7568c7d6ef34e934ba20adf834", "voices6.res", false },
	{ GID_IHNM_CD, "54b1f2013a075338ceb0e258d97808bd", "voicess.res", false },

	// Reported by mld. German Retail
	{ GID_IHNM_CD_DE, "0439083e3dfdc51b486071d45872ae52", "musicfm.res", false },
	{ GID_IHNM_CD_DE, "80f875a1fb384160d1f4b27166eef583", "musicgm.res", false },
	{ GID_IHNM_CD_DE, "c92370d400e6f2a3fc411c3729d09224", "scream.res", false },
	{ GID_IHNM_CD_DE, "32aa01a89937520fe0ea513950117292", "scripts.res", false },
	{ GID_IHNM_CD_DE, "58b79e61594779513c7f2d35509fa89e", "patch.re_", false },
	{ GID_IHNM_CD_DE, "1c610d543f32ec8b525e3f652536f269", "sfx.res", false },
	{ GID_IHNM_CD_DE, "424971e1e2373187c3f5734fe36071a2", "voices1.res", false },
	{ GID_IHNM_CD_DE, "c270e0980782af43641a86e4a14e2a32", "voices2.res", false },
	{ GID_IHNM_CD_DE, "49e42befea883fd101ec3d0f5d0647b9", "voices3.res", false },
	{ GID_IHNM_CD_DE, "c477443c52a0aa56e686ebd8d051e4ab", "voices5.res", false },
	{ GID_IHNM_CD_DE, "2b9aea838f74b4eecfb29a8f205a2bd4", "voices6.res", false },
	{ GID_IHNM_CD_DE, "8b09a196a52627cacb4eab13bfe0b2c3", "voicess.res", false },

	{ GID_IHNM_CD_ES, "0439083e3dfdc51b486071d45872ae52", "musicfm.res", false },
	{ GID_IHNM_CD_ES, "80f875a1fb384160d1f4b27166eef583", "musicgm.res", false },
	{ GID_IHNM_CD_ES, "58b79e61594779513c7f2d35509fa89e", "patch.re_", false },
	{ GID_IHNM_CD_ES, "c92370d400e6f2a3fc411c3729d09224", "scream.res", false },
	{ GID_IHNM_CD_ES, "be38bbc5a26be809dbf39f13befebd01", "scripts.res", false },
	{ GID_IHNM_CD_ES, "1c610d543f32ec8b525e3f652536f269", "sfx.res", false },
	{ GID_IHNM_CD_ES, "dc6a34e3d1668730ea46815a92c7847f", "voices1.res", false },
	{ GID_IHNM_CD_ES, "dc6a5fa7a4cdc2ca5a6fd924e969986c", "voices2.res", false },
	{ GID_IHNM_CD_ES, "dc6a5fa7a4cdc2ca5a6fd924e969986c", "voices3.res", false },
	{ GID_IHNM_CD_ES, "0f87400b804232a58dd22e404420cc45", "voices4.res", false },
	{ GID_IHNM_CD_ES, "172668cfc5d8c305cb5b1a9b4d995fc0", "voices5.res", false },
	{ GID_IHNM_CD_ES, "96c9bda9a5f41d6bc232ed7bf6d371d9", "voices6.res", false },
	{ GID_IHNM_CD_ES, "d869de9883c8faea7f687217a9ec7057", "voicess.res", false },

	{ GID_IHNM_CD_RU, "0439083e3dfdc51b486071d45872ae52", "musicfm.res", false },
	{ GID_IHNM_CD_RU, "80f875a1fb384160d1f4b27166eef583", "musicgm.res", false },
	{ GID_IHNM_CD_RU, "46bbdc65d164ba7e89836a0935eec8e6", "scream.res", false },
	{ GID_IHNM_CD_RU, "be38bbc5a26be809dbf39f13befebd01", "scripts.res", false },
	{ GID_IHNM_CD_RU, "58b79e61594779513c7f2d35509fa89e", "patch.re_", false },
	{ GID_IHNM_CD_RU, "1c610d543f32ec8b525e3f652536f269", "sfx.res", false },
	{ GID_IHNM_CD_RU, "d6100d2dc3b2b9f2e1ad247f613dce9b", "voices1.res", false },
	{ GID_IHNM_CD_RU, "84f6f48ecc2832841ea6417a9a379430", "voices2.res", false },
	{ GID_IHNM_CD_RU, "ebb9501283047f27a0f54e27b3c8ba1e", "voices3.res", false },
	{ GID_IHNM_CD_RU, "4c145da5fa6d1306162a7ca8ce5a4f2e", "voices4.res", false },
	{ GID_IHNM_CD_RU, "871a559644281917677eca4af1b05620", "voices5.res", false },
	{ GID_IHNM_CD_RU, "211be5c24f066d69a2f6cfa953acfba6", "voices6.res", false },
	{ GID_IHNM_CD_RU, "9df7cd3b18ddaa16b5291b3432567036", "voicess.res", false },

	{ GID_IHNM_CD_FR, "0439083e3dfdc51b486071d45872ae52", "musicfm.res", false },
	{ GID_IHNM_CD_FR, "80f875a1fb384160d1f4b27166eef583", "musicgm.res", false },
	{ GID_IHNM_CD_FR, "58b79e61594779513c7f2d35509fa89e", "patch.re_", false },
	{ GID_IHNM_CD_FR, "c92370d400e6f2a3fc411c3729d09224", "scream.res", false },
	{ GID_IHNM_CD_FR, "32aa01a89937520fe0ea513950117292", "scripts.res", false },
	{ GID_IHNM_CD_FR, "1c610d543f32ec8b525e3f652536f269", "sfx.res", false },
	{ GID_IHNM_CD_FR, "424971e1e2373187c3f5734fe36071a2", "voices1.res", false },
	{ GID_IHNM_CD_FR, "c2d93a35d2c2def9c3d6d242576c794b", "voices2.res", false },
	{ GID_IHNM_CD_FR, "49e42befea883fd101ec3d0f5d0647b9", "voices3.res", false },
	{ GID_IHNM_CD_FR, "f4c415de7c03de86b73f9a12b8bd632f", "voices5.res", false },
	{ GID_IHNM_CD_FR, "3fc5358a5d8eee43bdfab2740276572e", "voices6.res", false },
	{ GID_IHNM_CD_FR, "b8642e943bbebf89cef2f48b31cb4305", "voicess.res", false },

	{ GID_IHNM_DEMO, "46bbdc65d164ba7e89836a0935eec8e6", "scream.res", false },
	{ GID_IHNM_DEMO, "9626bda8978094ff9b29198bc1ed5f9a", "scripts.res", false },
	{ GID_IHNM_DEMO, "1c610d543f32ec8b525e3f652536f269", "sfx.res", false },
	{ GID_IHNM_DEMO, "3bbc16a8f741dbb511da506c660a0b54", "voicesd.res", false },
};

static GameDescription gameDescriptions[] = {
	// Inherit the earth - DOS Demo version
	// sound unchecked
	{
		"ite",
		GType_ITE,
		GID_ITE_DEMO_G, // Game id
		"Inherit the Earth (DOS Demo)", // Game title
		&ITE_DisplayInfo,
		ITE_DEFAULT_SCENE, // Starting scene number
		&ITEDemo_Resources,
		ARRAYSIZE(ITEDEMO_GameFiles), // Game datafiles
		ITEDEMO_GameFiles,
		ARRAYSIZE(ITEDEMO_GameFonts),
		ITEDEMO_GameFonts,
		&ITEDEMO_GameSound,
		&ITEDEMO_GameSound,
		NULL,
		0,
		NULL,
		0, // features
		Common::EN_USA,
		Common::kPlatformPC,
	},

	// Inherit the earth - MAC Demo version
	{
		"ite",
		GType_ITE,
		GID_ITE_MACDEMO2,
		"Inherit the Earth (MAC Demo)",
		&ITE_DisplayInfo,
		ITE_DEFAULT_SCENE,
		&ITE_Resources,
		ARRAYSIZE(ITEMACDEMO_GameFiles),
		ITEMACDEMO_GameFiles,
		ARRAYSIZE(ITEWINDEMO_GameFonts),
		ITEWINDEMO_GameFonts,
		&ITEMACDEMO_GameVoice,
		&ITEMACDEMO_GameSound,
		&ITEMACDEMO_GameMusic,
		ARRAYSIZE(ITEMacPatch_Files),
		ITEMacPatch_Files,
		GF_BIG_ENDIAN_DATA | GF_WYRMKEEP | GF_CD_FX | GF_SCENE_SUBSTITUTES,
		Common::EN_USA,
		Common::kPlatformMacintosh,
	},

	// Inherit the earth - early MAC Demo version
	{
		"ite",
		GType_ITE,
		GID_ITE_MACDEMO1,
		"Inherit the Earth (early MAC Demo)",
		&ITE_DisplayInfo,
		ITE_DEFAULT_SCENE,
		&ITE_Resources,
		ARRAYSIZE(ITEMACDEMO_GameFiles),
		ITEMACDEMO_GameFiles,
		ARRAYSIZE(ITEWINDEMO_GameFonts),
		ITEWINDEMO_GameFonts,
		&ITEMACDEMO_GameVoice,
		&ITEMACDEMO_GameSound,
		&ITEMACCD_GameMusic,
		ARRAYSIZE(ITEMacPatch_Files),
		ITEMacPatch_Files,
		GF_BIG_ENDIAN_DATA | GF_WYRMKEEP | GF_CD_FX,
		Common::EN_USA,
		Common::kPlatformMacintosh,
	},

	// Inherit the earth - MAC CD Guild version
	{
		"ite",
		GType_ITE,
		GID_ITE_MACCD_G,
		"Inherit the Earth (MAC CD)",
		&ITE_DisplayInfo,
		ITE_DEFAULT_SCENE,
		&ITE_Resources,
		ARRAYSIZE(ITEMACCD_G_GameFiles),
		ITEMACCD_G_GameFiles,
		ARRAYSIZE(ITEWINDEMO_GameFonts),
		ITEWINDEMO_GameFonts,
		&ITEMACCD_G_GameSound,
		&ITEMACCD_G_GameSound,
		NULL,
		0,
		NULL,
		GF_BIG_ENDIAN_DATA | GF_CD_FX,
		Common::EN_USA,
		Common::kPlatformMacintosh,
	},

	// Inherit the earth - MAC CD Wyrmkeep version
	{
		"ite",
		GType_ITE,
		GID_ITE_MACCD,
		"Inherit the Earth (Wyrmkeep MAC CD)",
		&ITE_DisplayInfo,
		ITE_DEFAULT_SCENE,
		&ITE_Resources,
		ARRAYSIZE(ITEMACCD_GameFiles),
		ITEMACCD_GameFiles,
		ARRAYSIZE(ITEWINDEMO_GameFonts),
		ITEWINDEMO_GameFonts,
		&ITEMACCD_GameSound,
		&ITEMACCD_GameSound,
		&ITEMACCD_GameMusic,
		ARRAYSIZE(ITEMacPatch_Files),
		ITEMacPatch_Files,
		GF_BIG_ENDIAN_DATA | GF_WYRMKEEP | GF_CD_FX,
		Common::EN_USA,
		Common::kPlatformMacintosh,
	},

	// Inherit the earth - Linux Demo version
	// Note: it should be before GID_ITE_WINDEMO2 version
	{
		"ite",
		GType_ITE,
		GID_ITE_LINDEMO,
		"Inherit the Earth (Linux Demo)",
		&ITE_DisplayInfo,
		ITE_DEFAULT_SCENE,
		&ITE_Resources,
		ARRAYSIZE(ITELINDEMO_GameFiles),
		ITELINDEMO_GameFiles,
		ARRAYSIZE(ITEWINDEMO_GameFonts),
		ITEWINDEMO_GameFonts,
		&ITEWINDEMO2_GameVoice,
		&ITEWINDEMO2_GameSound,
		&ITELINDEMO_GameMusic,
		ARRAYSIZE(ITELinPatch_Files),
		ITELinPatch_Files,
		GF_WYRMKEEP | GF_CD_FX | GF_SCENE_SUBSTITUTES,
		Common::EN_USA,
		Common::kPlatformLinux,
	},

	// Inherit the earth - Win32 Demo version
	{
		"ite",
		GType_ITE,
		GID_ITE_WINDEMO2,
		"Inherit the Earth (Win32 Demo)",
		&ITE_DisplayInfo,
		ITE_DEFAULT_SCENE,
		&ITE_Resources,
		ARRAYSIZE(ITEWINDEMO_GameFiles),
		ITEWINDEMO_GameFiles,
		ARRAYSIZE(ITEWINDEMO_GameFonts),
		ITEWINDEMO_GameFonts,
		&ITEWINDEMO2_GameVoice,
		&ITEWINDEMO2_GameSound,
		NULL,
		ARRAYSIZE(ITEWinPatch2_Files),
		ITEWinPatch2_Files,
		GF_WYRMKEEP | GF_CD_FX | GF_SCENE_SUBSTITUTES,
		Common::EN_USA,
		Common::kPlatformWindows,
	},

	// Inherit the earth - early Win32 Demo version
	{
		"ite",
		GType_ITE,
		GID_ITE_WINDEMO1,
		"Inherit the Earth (early Win32 Demo)",
		&ITE_DisplayInfo,
		ITE_DEFAULT_SCENE,
		&ITE_Resources,
		ARRAYSIZE(ITEWINDEMO_GameFiles),
		ITEWINDEMO_GameFiles,
		ARRAYSIZE(ITEWINDEMO_GameFonts),
		ITEWINDEMO_GameFonts,
		&ITEWINDEMO1_GameSound,
		&ITEWINDEMO1_GameSound,
		NULL,
		ARRAYSIZE(ITEWinPatch1_Files),
		ITEWinPatch1_Files,
		GF_WYRMKEEP | GF_CD_FX,
		Common::EN_USA,
		Common::kPlatformWindows,
	},

	// Inherit the earth - Wyrmkeep combined Windows/Mac/Linux CD
	{
		"ite",
		GType_ITE,
		GID_ITE_MULTICD,
		"Inherit the Earth (Multi-OS CD Version)",
		&ITE_DisplayInfo,
		ITE_DEFAULT_SCENE,
		&ITE_Resources,
		ARRAYSIZE(ITEMULTICD_GameFiles),
		ITEMULTICD_GameFiles,
		ARRAYSIZE(ITECD_GameFonts),
		ITECD_GameFonts,
		&ITEMACCD_GameSound,
		&ITECD_GameSound,
		&ITEMACCD_GameMusic,
		0,
		NULL,
		GF_WYRMKEEP | GF_CD_FX,
		Common::EN_USA,
		Common::kPlatformUnknown,
	},

	// Inherit the earth - Wyrmkeep Linux CD version
	{
		"ite",
		GType_ITE,
		GID_ITE_LINCD,
		"Inherit the Earth (Linux CD Version)",
		&ITE_DisplayInfo,
		ITE_DEFAULT_SCENE,
		&ITE_Resources,
		ARRAYSIZE(ITELINCD_GameFiles),
		ITELINCD_GameFiles,
		ARRAYSIZE(ITECD_GameFonts),
		ITECD_GameFonts,
		&ITECD_GameSound,
		&ITECD_GameSound,
		&ITEMACCD_GameMusic,
		ARRAYSIZE(ITELinPatch_Files),
		ITELinPatch_Files,
		GF_WYRMKEEP | GF_CD_FX,
		Common::EN_USA,
		Common::kPlatformLinux,
	},

	// Inherit the earth - Wyrmkeep Windows CD version
	{
		"ite",
		GType_ITE,
		GID_ITE_WINCD,
		"Inherit the Earth (Win32 CD Version)",
		&ITE_DisplayInfo,
		ITE_DEFAULT_SCENE,
		&ITE_Resources,
		ARRAYSIZE(ITECD_GameFiles),
		ITECD_GameFiles,
		ARRAYSIZE(ITECD_GameFonts),
		ITECD_GameFonts,
		&ITECD_GameSound,
		&ITECD_GameSound,
		NULL,
		ARRAYSIZE(ITEWinPatch1_Files),
		ITEWinPatch1_Files,
		GF_WYRMKEEP | GF_CD_FX,
		Common::EN_USA,
		Common::kPlatformWindows,
	},

	// Inherit the earth - DOS CD version
	{
		"ite",
		GType_ITE,
		GID_ITE_CD_G,
		"Inherit the Earth (DOS CD Version)",
		&ITE_DisplayInfo,
		ITE_DEFAULT_SCENE,
		&ITE_Resources,
		ARRAYSIZE(ITECD_GameFiles),
		ITECD_GameFiles,
		ARRAYSIZE(ITECD_GameFonts),
		ITECD_GameFonts,
		&ITECD_GameSound,
		&ITECD_GameSound,
		NULL,
		0,
		NULL,
		GF_CD_FX,
		Common::EN_USA,
		Common::kPlatformPC,
	},

	// Inherit the earth - DOS CD German version
	{
		"ite",
		GType_ITE,
		GID_ITE_CD_DE,
		"Inherit the Earth (De DOS CD Version)",
		&ITE_DisplayInfo,
		ITE_DEFAULT_SCENE,
		&ITE_Resources,
		ARRAYSIZE(ITECD_GameFiles),
		ITECD_GameFiles,
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

	// Inherit the earth - CD version
	{
		"ite",
		GType_ITE,
		GID_ITE_CD,
		"Inherit the Earth (DOS CD Version)",
		&ITE_DisplayInfo,
		ITE_DEFAULT_SCENE,
		&ITE_Resources,
		ARRAYSIZE(ITECD_GameFiles),
		ITECD_GameFiles,
		ARRAYSIZE(ITECD_GameFonts),
		ITECD_GameFonts,
		&ITECD_GameSound,
		&ITECD_GameSound,
		NULL,
		0,
		NULL,
		GF_CD_FX,
		Common::EN_USA,
		Common::kPlatformPC,
	},

	// Inherit the earth - German Floppy version
	{
		"ite",
		GType_ITE,
		GID_ITE_DISK_DE,
		"Inherit the Earth (De DOS Floppy)",
		&ITE_DisplayInfo,
		ITE_DEFAULT_SCENE,
		&ITE_Resources,
		ARRAYSIZE(ITEDISK_GameFiles),
		ITEDISK_GameFiles,
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

	// Inherit the earth - Disk version
	{
		"ite",
		GType_ITE,
		GID_ITE_DISK_G,
		"Inherit the Earth (DOS Floppy)",
		&ITE_DisplayInfo,
		ITE_DEFAULT_SCENE,
		&ITE_Resources,
		ARRAYSIZE(ITEDISK_GameFiles),
		ITEDISK_GameFiles,
		ARRAYSIZE(ITEDISK_GameFonts),
		ITEDISK_GameFonts,
		&ITEDISK_GameSound,
		&ITEDISK_GameSound,
		NULL,
		0,
		NULL,
		0,
		Common::EN_USA,
		Common::kPlatformPC,
	},

	// I Have No Mouth And I Must Scream - Demo version
	{
		"ihnm",
		GType_IHNM,
		GID_IHNM_DEMO,
		"I Have No Mouth and I Must Scream (DOS Demo)",
		&IHNM_DisplayInfo,
		0,
		&IHNM_Resources,
		ARRAYSIZE(IHNMDEMO_GameFiles),
		IHNMDEMO_GameFiles,
		ARRAYSIZE(IHNMDEMO_GameFonts),
		IHNMDEMO_GameFonts,
		&IHNM_GameSound,
		&IHNM_GameSound,
		NULL,
		0,
		NULL,
		GF_DEFAULT_TO_1X_SCALER,
		Common::EN_USA,
		Common::kPlatformPC,
	},

	// I Have No Mouth And I Must Scream - CD version
	{
		"ihnm",
		GType_IHNM,
		GID_IHNM_CD,
		"I Have No Mouth and I Must Scream (DOS)",
		&IHNM_DisplayInfo,
		IHNM_DEFAULT_SCENE,
		&IHNM_Resources,
		ARRAYSIZE(IHNMCD_GameFiles),
		IHNMCD_GameFiles,
		ARRAYSIZE(IHNMCD_GameFonts),
		IHNMCD_GameFonts,
		&IHNM_GameSound,
		&IHNM_GameSound,
		NULL,
		0,
		NULL,
		GF_DEFAULT_TO_1X_SCALER,
		Common::EN_USA,
		Common::kPlatformPC,
	},

	// I Have No Mouth And I Must Scream - De CD version
	{
		"ihnm",
		GType_IHNM,
		GID_IHNM_CD_DE,
		"I Have No Mouth and I Must Scream (DE DOS)",
		&IHNM_DisplayInfo,
		IHNM_DEFAULT_SCENE,
		&IHNM_Resources,
		ARRAYSIZE(IHNMCD_Censored_GameFiles),
		IHNMCD_Censored_GameFiles,
		ARRAYSIZE(IHNMCD_GameFonts),
		IHNMCD_GameFonts,
		&IHNM_GameSound,
		&IHNM_GameSound,
		NULL,
		0,
		NULL,
		GF_DEFAULT_TO_1X_SCALER,
		Common::DE_DEU,
		Common::kPlatformPC,
	},
	// I Have No Mouth And I Must Scream - Sp CD version
	{
		"ihnm",
		GType_IHNM,
		GID_IHNM_CD_ES,
		"I Have No Mouth and I Must Scream (Sp DOS)",
		&IHNM_DisplayInfo,
		IHNM_DEFAULT_SCENE,
		&IHNM_Resources,
		ARRAYSIZE(IHNMCD_GameFiles),
		IHNMCD_GameFiles,
		ARRAYSIZE(IHNMCD_GameFonts),
		IHNMCD_GameFonts,
		&IHNM_GameSound,
		&IHNM_GameSound,
		NULL,
		0,
		NULL,
		GF_DEFAULT_TO_1X_SCALER,
		Common::ES_ESP,
		Common::kPlatformPC,
	},
	// I Have No Mouth And I Must Scream - Ru CD version
	{
		"ihnm",
		GType_IHNM,
		GID_IHNM_CD_RU,
		"I Have No Mouth and I Must Scream (Ru DOS)",
		&IHNM_DisplayInfo,
		IHNM_DEFAULT_SCENE,
		&IHNM_Resources,
		ARRAYSIZE(IHNMCD_GameFiles),
		IHNMCD_GameFiles,
		ARRAYSIZE(IHNMCD_GameFonts),
		IHNMCD_GameFonts,
		&IHNM_GameSound,
		&IHNM_GameSound,
		NULL,
		0,
		NULL,
		GF_DEFAULT_TO_1X_SCALER,
		Common::RU_RUS,
		Common::kPlatformPC,
	},
	// I Have No Mouth And I Must Scream - Fr CD version
	{
		"ihnm",
		GType_IHNM,
		GID_IHNM_CD_FR,
		"I Have No Mouth and I Must Scream (Fr DOS)",
		&IHNM_DisplayInfo,
		IHNM_DEFAULT_SCENE,
		&IHNM_Resources,
		ARRAYSIZE(IHNMCD_Censored_GameFiles),
		IHNMCD_Censored_GameFiles,
		ARRAYSIZE(IHNMCD_GameFonts),
		IHNMCD_GameFonts,
		&IHNM_GameSound,
		&IHNM_GameSound,
		NULL,
		0,
		NULL,
		GF_DEFAULT_TO_1X_SCALER,
		Common::FR_FRA,
		Common::kPlatformPC,
	},
};

bool SagaEngine::initGame() {
	uint16 gameCount = ARRAYSIZE(gameDescriptions);
	int gameNumber = -1;
	FSList dummy;
	DetectedGameList detectedGames;
	int *matches;
	Common::Language language = Common::UNK_LANG;
	Common::Platform platform = Common::kPlatformUnknown;

	if (ConfMan.hasKey("language"))
		language = Common::parseLanguage(ConfMan.get("language"));
	if (ConfMan.hasKey("platform"))
		platform = Common::parsePlatform(ConfMan.get("platform"));


	detectedGames = GAME_ProbeGame(dummy, &matches);

	if (detectedGames.size() == 0) {
		warning("No valid games were found in the specified directory.");
		return false;
	}

	// If we have more than one match then try to match by platform and
	// language
	int count = 0;
	if (detectedGames.size() > 1) {
		for (int i = 0; i < ARRAYSIZE(gameDescriptions); i++)
			if (matches[i] != -1) {
				if ((gameDescriptions[matches[i]].language != language &&
					 language != Common::UNK_LANG) ||
					(gameDescriptions[matches[i]].platform != platform &&
					 platform != Common::kPlatformUnknown)) {
					debug(2, "Purged (pass 2) %s", gameDescriptions[matches[i]].title);
					matches[i] = -1;
				}
				else
					count++;
			}
	} else
		count = 1;

	if (count != 1)
		warning("Conflicting targets detected (%d)", count);

	for (int i = 0; i < ARRAYSIZE(gameDescriptions); i++)
		if (matches[i] != -1) {
			gameNumber = matches[i];
			break;
		}

	free(matches);

	if (gameNumber >= gameCount || gameNumber == -1) {
		error("SagaEngine::loadGame wrong gameNumber");
	}

	debug(2, "Running %s", gameDescriptions[gameNumber].title);

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

DetectedGameList GAME_ProbeGame(const FSList &fslist, int **retmatches) {
	DetectedGameList detectedGames;
	int game_n;
	int index = 0, i, j;
	int matches[ARRAYSIZE(gameDescriptions)];
	bool mode = retmatches ? false : true;

	game_n = -1;
	for (i = 0; i < ARRAYSIZE(gameDescriptions); i++)
		matches[i] = -1;

	while (1) {
		game_n = detectGame(fslist, mode, game_n);
		if (game_n == -1)
			break;
		matches[index++] = game_n;
	}

	// We have some resource sets which are superpositions of other
	// Particularly it is ite-demo-linux vs ite-demo-win
	// Now remove lesser set if bigger matches too

	if (index > 1) {
		// Search max number
		int maxcount = 0;
		for (i = 0; i < index; i++) {
			int count = 0;
			for (j = 0; j < ARRAYSIZE(gameMD5); j++)
				if (gameMD5[j].id == gameDescriptions[matches[i]].gameId)
					count++;
			maxcount = MAX(maxcount, count);
		}

		// Now purge targets with number of files lesser than max
		for (i = 0; i < index; i++) {
			int count = 0;
			for (j = 0; j < ARRAYSIZE(gameMD5); j++)
				if (gameMD5[j].id == gameDescriptions[matches[i]].gameId)
					count++;
			if (count < maxcount) {
				debug(2, "Purged: %s", gameDescriptions[matches[i]].title);
				matches[i] = -1;
			}
		}

	}

	// and now push them into list of detected games
	for (i = 0; i < index; i++)
		if (matches[i] != -1)
			detectedGames.push_back(DetectedGame(gameDescriptions[matches[i]].toGameSettings(),
							 gameDescriptions[matches[i]].language,
							 gameDescriptions[matches[i]].platform));
		
	if (retmatches) {
		*retmatches = (int *)calloc(ARRAYSIZE(gameDescriptions), sizeof(int));
		for (i = 0; i < ARRAYSIZE(gameDescriptions); i++)
			(*retmatches)[i] = matches[i];
	}

	return detectedGames;
}

int detectGame(const FSList &fslist, bool mode, int start) {
	int game_count = ARRAYSIZE(gameDescriptions);
	int game_n = -1;
	typedef Common::Map<Common::String, Common::String> StringMap;
	StringMap filesMD5;

	typedef Common::Map<Common::String, bool> StringSet;
	StringSet filesList;

	uint16 file_count;
	uint16 file_n;
	Common::File test_file;
	bool file_missing;

	Common::String tstr, tstr1;
	char md5str[32+1];
	uint8 md5sum[16];

	// First we compose list of files which we need MD5s for
	for (int i = 0; i < ARRAYSIZE(gameMD5); i++) {
		tstr = Common::String(gameMD5[i].filename);
		tstr.toLowercase();

		if (gameMD5[i].caseSensitive && !mode)
			filesList[Common::String(gameMD5[i].filename)] = true;
		else
			filesList[tstr] = true;
	}

	if (mode) {
		// Now count MD5s for required files
		for (FSList::const_iterator file = fslist.begin(); file != fslist.end(); ++file) {
			if (!file->isDirectory()) {
				tstr = file->displayName();
				// FIXME: there is a bug in String class. tstr1 = tstr; tstr.toLowercase()
				// makes tstr1 lowercase as well
				tstr1 = Common::String(file->displayName().c_str());
				tstr.toLowercase();

				if (filesList.contains(tstr) || filesList.contains(tstr1)) {
					if (Common::md5_file(file->path().c_str(), md5sum, NULL, FILE_MD5_BYTES)) {
						for (int j = 0; j < 16; j++) {
							sprintf(md5str + j*2, "%02x", (int)md5sum[j]);
						}
						filesMD5[tstr] = Common::String(md5str);
						filesMD5[tstr1] = Common::String(md5str);
					}
				}
			}
		}
	} else {
		Common::File testFile;

		for (StringSet::const_iterator file = filesList.begin(); file != filesList.end(); ++file) {
			if (testFile.open(file->_key.c_str())) {
				testFile.close();
				if (Common::md5_file(file->_key.c_str(), md5sum, NULL, FILE_MD5_BYTES)) {
					for (int j = 0; j < 16; j++) {
						sprintf(md5str + j*2, "%02x", (int)md5sum[j]);
					}
					filesMD5[file->_key] = Common::String(md5str);
				}
			}
		}
	}

	for (game_n = start + 1; game_n < game_count; game_n++) {
		file_count = gameDescriptions[game_n].filesCount;
		file_missing = false;

		// Try to open all files for this game
		for (file_n = 0; file_n < file_count; file_n++) {
			tstr = gameDescriptions[game_n].filesDescriptions[file_n].fileName;

			if (!filesMD5.contains(tstr)) {
				file_missing = true;
				break;
			}
		}

		// Try the next game, couldn't find all files for the current
		// game
		if (file_missing) {
			continue;
		} else {
			bool match = true;

			debug(2, "Probing game: %s", gameDescriptions[game_n].title);

			for (int i = 0; i < ARRAYSIZE(gameMD5); i++) {
				if (gameMD5[i].id == gameDescriptions[game_n].gameId) {
					tstr = gameMD5[i].filename;

					if (strcmp(gameMD5[i].md5, filesMD5[tstr].c_str())) {
						match = false;
						break;
					}
				}
			}
			if (!match)
				continue;

			debug(2, "Found game: %s", gameDescriptions[game_n].title);

			return game_n;
		}
	}

	if (!filesMD5.isEmpty() && start == -1) {
		printf("MD5s of your game version are unknown. Please, report following data to\n");
		printf("ScummVM team along with your game name and version:\n");

		for (StringMap::const_iterator file = filesMD5.begin(); file != filesMD5.end(); ++file)
			printf("%s: %s\n", file->_key.c_str(), file->_value.c_str());
	}

	return -1;
}

} // End of namespace Saga
