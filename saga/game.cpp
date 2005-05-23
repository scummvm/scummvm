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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

// Game detection, general game parameters

#include "saga/saga.h"

#include "common/file.h"
#include "common/md5.h"
#include "common/map.h"
#include "base/plugins.h"
#include "base/gameDetector.h"
#include "backends/fs/fs.h"

#include "saga/rscfile_mod.h"
#include "saga/interface.h"
#include "saga/scene.h"

namespace Saga {

static int detectGame(const FSList &fslist, bool mode = false);

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
	{kPanelButtonConverseText,	52,6 + CONVERSE_TEXT_HEIGHT * 0, CONVERSE_MAX_TEXT_WIDTH,CONVERSE_TEXT_HEIGHT,	0,'1',0,	0,0,0},
	{kPanelButtonConverseText,	52,6 + CONVERSE_TEXT_HEIGHT * 1, CONVERSE_MAX_TEXT_WIDTH,CONVERSE_TEXT_HEIGHT,	1,'2',0,	0,0,0},
	{kPanelButtonConverseText,	52,6 + CONVERSE_TEXT_HEIGHT * 2, CONVERSE_MAX_TEXT_WIDTH,CONVERSE_TEXT_HEIGHT,	2,'3',0,	0,0,0},
	{kPanelButtonConverseText,	52,6 + CONVERSE_TEXT_HEIGHT * 3, CONVERSE_MAX_TEXT_WIDTH,CONVERSE_TEXT_HEIGHT,	3,'4',0,	0,0,0},
	{kPanelButtonArrow,			257,6,	9,6,	-1,'u',0,	0,4,2},
	{kPanelButtonArrow,			257,41,	9,6,	1,'d',0,	1,5,3}, 
};

static PanelButton ITE_OptionPanelButtons[] = {
	{kPanelButtonSlider,	284,19, 13,75,	0,'-',0,	0,0,0}, //slider-scroller
	{kPanelButtonOption,	113,18, 45,17,	13,'r',0,	0,0,0}, //read speed
	{kPanelButtonOption,	113,37, 45,17,	16,'m',0,	0,0,0}, //music
	{kPanelButtonOption,	113,56, 45,17,	16,'n',0,	0,0,0}, //sound-noise
	{kPanelButtonOption,	13,79, 135,17,	12,'q',0,	0,0,0}, //quit
	{kPanelButtonOption,	13,98, 135,17,	17,'c',0,	0,0,0}, //continue
	{kPanelButtonOption,	164,98, 57,17,	18,'l',0,	0,0,0}, //load
	{kPanelButtonOption,	241,98, 57,17,	19,'s',0,	0,0,0},	//save
	{kPanelButtonOption,	166,20, 112,74,	0,'-',0,	0,0,0},	//slider
};

static GameDisplayInfo ITE_DisplayInfo = {
	320, 200,		// logical width&height
	
	35,				// scene path y offset
	137,			// scene height
	
	0,				// status x offset
	137,			// status y offset
	320,			// status width
	12,				// status height
	2,				// status text y offset
	186,			// status text color
	15,				// status BG color
	308,138,		// save reminder pos
	12,12,			// save reminder w & h
	6,7,			// save reminder sprite numbers

	147,			// verb text color
	15,				// verb text shadow color
	96,				// verb text active color
	
	5, 4,			// left portrait x, y offset
	274, 4,			// right portrait x, y offset

	8, 9,			// inventory Up & Down button indexies
	2, 4,			// inventory rows, columns

	0, 149,			// main panel offsets
	ARRAYSIZE(ITE_MainPanelButtons),
	ITE_MainPanelButtons,

	4, 5,			// converse Up & Down button indexies

	0, 149,			// converse panel offsets
	ARRAYSIZE(ITE_ConversePanelButtons),
	ITE_ConversePanelButtons,
	
	8, 8,			// option panel offsets
	ARRAYSIZE(ITE_OptionPanelButtons),
	ITE_OptionPanelButtons
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

// Inherit the Earth - DOS Demo version
static GameFileDescription ITEDEMO_GameFiles[] = {
	{"ite.rsc", GAME_RESOURCEFILE},
	{"ite.dmo", GAME_DEMOFILE},
	{"scripts.rsc", GAME_SCRIPTFILE},
	{"voices.rsc", GAME_SOUNDFILE | GAME_VOICEFILE}
};

static GameFontDescription ITEDEMO_GameFonts[] = {
	{0},
	{1}
}; 

static GameSoundInfo ITEDEMO_GameSound = {
	GAME_SOUND_VOC, 0, 0, 0
};

static GameFontDescription ITEMACDEMO_GameFonts[] = {
	{2},
	{0}
}; 

// Inherit the Earth - Wyrmkeep Win32 Demo version
static GameFileDescription ITEWINDEMO_GameFiles[] = {
	{"ited.rsc", GAME_RESOURCEFILE},
	{"scriptsd.rsc", GAME_SCRIPTFILE},
	{"soundsd.rsc", GAME_SOUNDFILE},
	{"voicesd.rsc", GAME_VOICEFILE}
};

// Inherit the Earth -  Wyrmkeep Linux Demo version
static GameFileDescription ITELINDEMO_GameFiles[] = {
	{"ited.rsc", GAME_RESOURCEFILE},
	{"scriptsd.rsc", GAME_SCRIPTFILE},
	{"soundsd.rsc", GAME_SOUNDFILE},
	{"voicesd.rsc", GAME_VOICEFILE},
	{"musicd.rsc", GAME_MUSICFILE}
};

// Inherit the Earth -  Wyrmkeep Linux version
static GameFileDescription ITELINCD_GameFiles[] = {
	{"ite.rsc", GAME_RESOURCEFILE},
	{"scripts.rsc", GAME_SCRIPTFILE},
	{"sounds.rsc", GAME_SOUNDFILE},
	{"voices.rsc", GAME_VOICEFILE},
	{"music.rsc", GAME_MUSICFILE}
};

static GameFontDescription ITEWINDEMO_GameFonts[] = {
	{0},
	{2}
}; 

// Inherit the Earth - Mac Wyrmkeep version
static GameFileDescription ITEMACCD_GameFiles[] = {
	{"ite.rsc", GAME_RESOURCEFILE},
	{"scripts.rsc", GAME_SCRIPTFILE},
	{"sounds.rsc", GAME_SOUNDFILE},
	{"Inherit the Earth Voices", GAME_VOICEFILE},
	{"music.rsc", GAME_MUSICFILE}
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

static GameSoundInfo ITE_GameSound = {
	GAME_SOUND_VOC, 0, 0, 0
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
	GAME_SOUND_PCM,
	22050,
	16,
	0
};

// IHNM section

static PanelButton IHNM_MainPanelButtons[] = {
	{kPanelButtonVerb, 0,0, 0,0, kVerbWalkTo,' ',0, 0,0,0}, //TODO
};

static PanelButton IHNM_ConversePanelButtons[] = {
	{kPanelButtonConverseText, 0,0, 0,0, 0,'-',0, 0,0,0}, //TODO
};

static PanelButton IHNM_OptionPanelButtons[] = {
	{kPanelButtonArrow, 0,0, 0,0, 0,'-',0, 0,0,0}, //TODO
};

static GameDisplayInfo IHNM_DisplayInfo = { //TODO: fill it all
	640, 480,	// logical width&height
	
	0,			// scene path y offset
	304,		// scene height

	0,			// status x offset
	304,		// status y offset
	640,		// status width
	24,			// status height
	8,			// status text y offset
	186,		// status text color
	11,			// status BG color
	0,0,		// save reminder pos
	0,0,		// save reminder w&h
	0,0,		// save reminder sprite numbers

	147,		// verb text color
	15,			// verb text shadow color
	96,			// verb text active color

	5, 4,		// left portrait x, y offset
	-1, -1,		// right portrait x, y offset

	-1, -1,		// inventory Up & Down button indexies
	0, 0,		// inventory rows, columns

	0, 149,		// main panel offsets
	ARRAYSIZE(IHNM_MainPanelButtons),
	IHNM_MainPanelButtons,

	-1, -1,		// converse Up & Down button indexies

	0, 0,		// converse panel offsets
	ARRAYSIZE(IHNM_ConversePanelButtons),
	IHNM_ConversePanelButtons,
	
	0, 0,		// option panel offsets
	ARRAYSIZE(IHNM_OptionPanelButtons),
	IHNM_OptionPanelButtons
};

static GameResourceDescription IHNM_Resources = {
	RID_IHNM_SCENE_LUT,  // Scene lookup table RN
	RID_IHNM_SCRIPT_LUT, // Script lookup table RN
	RID_IHNM_MAIN_PANEL,
	RID_IHNM_CONVERSE_PANEL,
	0,
	0,
	0,
	0,
	0,
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
	{"patch.re_", GAME_PATCHFILE},
	{"scripts.res", GAME_SCRIPTFILE},
	{"sfx.res", GAME_SOUNDFILE},
	{"voicess.res", GAME_VOICEFILE},
	{"voices1.res", GAME_VOICEFILE},
	{"voices2.res", GAME_VOICEFILE},
	{"voices3.res", GAME_VOICEFILE},
	{"voices4.res", GAME_VOICEFILE},
	{"voices5.res", GAME_VOICEFILE},
	{"voices6.res", GAME_VOICEFILE}
};

// I Have No Mouth and I Must Scream - Retail German CD version
static GameFileDescription IHNMCDDE_GameFiles[] = {
	{"musicfm.res", GAME_MUSICFILE_FM},
	{"musicgm.res", GAME_MUSICFILE_GM},
	{"scream.res", GAME_RESOURCEFILE},
	{"scripts.res", GAME_SCRIPTFILE},
	{"patch.re_", GAME_PATCHFILE},
	{"sfx.res", GAME_SOUNDFILE},
	{"voicess.res", GAME_VOICEFILE},
	{"voices1.res", GAME_VOICEFILE},
	{"voices2.res", GAME_VOICEFILE},
	{"voices3.res", GAME_VOICEFILE},
	{"voices5.res", GAME_VOICEFILE},
	{"voices6.res", GAME_VOICEFILE}
};

static GameFontDescription IHNMCD_GameFonts[] = {
	{2},
	{3},
	{4},
	{5},
	{6},
	{7},
	{8}
};

static GameSoundInfo IHNM_GameSound = {
	GAME_SOUND_WAV, 0, 0, 0
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

	{ GID_ITE_MACCD,    "4f7fa11c5175980ed593392838523060", "ite.rsc", false },
	{ GID_ITE_MACCD,    "adf1f46c1d0589083996a7060c798ad0", "scripts.rsc", false },
	{ GID_ITE_MACCD,    "1a91cd60169f367ecb6c6e058d899b2f", "music.rsc", false },
	{ GID_ITE_MACCD,    "95863b89a0916941f6c5e1789843ba14", "sounds.rsc", false },
	{ GID_ITE_MACCD,    "c14c4c995e7a0d3828e3812a494301b7", "Inherit the Earth Voices", true },

	{ GID_ITE_LINCD,    "8f4315a9bb10ec839253108a032c8b54", "ite.rsc", false },
	{ GID_ITE_LINCD,    "a891405405edefc69c9d6c420c868b84", "scripts.rsc", false },
	{ GID_ITE_LINCD,    "e2ccb61c325d6d1ead3be0e731fe29fe", "sounds.rsc", false },
	{ GID_ITE_LINCD,    "41bb6b95d792dde5196bdb78740895a6", "voices.rsc", false },
	{ GID_ITE_LINCD,    "d6454756517f042f01210458abe8edd4", "music.rsc", false },

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

	{ GID_IHNM_DEMO, "46bbdc65d164ba7e89836a0935eec8e6", "scream.res", false },
	{ GID_IHNM_DEMO, "9626bda8978094ff9b29198bc1ed5f9a", "scripts.res", false },
	{ GID_IHNM_DEMO, "1c610d543f32ec8b525e3f652536f269", "sfx.res", false },
	{ GID_IHNM_DEMO, "3bbc16a8f741dbb511da506c660a0b54", "voicesd.res", false },
};

static GameDescription gameDescriptions[] = {
	// Inherit the earth - DOS Demo version
	{
		"ite-demo",
		GType_ITE,
		GID_ITE_DEMO_G, // Game id
		"Inherit the Earth (DOS Demo)", // Game title
		&ITE_DisplayInfo,
		ITE_DEFAULT_SCENE, // Starting scene number
		&ITE_Resources,
		ARRAYSIZE(ITEDEMO_GameFiles), // Game datafiles
		ITEDEMO_GameFiles,
		ARRAYSIZE(ITEDEMO_GameFonts),
		ITEDEMO_GameFonts,
		&ITEDEMO_GameSound,
		0, // features
	},

	// Inherit the earth - MAC Demo version
	{
		"ite-demo",
		GType_ITE,
		GID_ITE_MACDEMO2,
		"Inherit the Earth (MAC Demo)",
		&ITE_DisplayInfo,
		ITE_DEFAULT_SCENE,
		&ITE_Resources,
		ARRAYSIZE(ITEWINDEMO_GameFiles),
		ITEWINDEMO_GameFiles,
		ARRAYSIZE(ITEMACDEMO_GameFonts),
		ITEMACDEMO_GameFonts,
		&ITECD_GameSound,
		GF_VOX_VOICES | GF_BIG_ENDIAN_DATA | GF_MAC_RESOURCES | GF_WYRMKEEP | GF_CD_FX
	},

	// Inherit the earth - early MAC Demo version
	{
		"ite-demo",
		GType_ITE,
		GID_ITE_MACDEMO1,
		"Inherit the Earth (early MAC Demo)",
		&ITE_DisplayInfo,
		ITE_DEFAULT_SCENE,
		&ITE_Resources,
		ARRAYSIZE(ITEWINDEMO_GameFiles),
		ITEWINDEMO_GameFiles,
		ARRAYSIZE(ITEMACDEMO_GameFonts),
		ITEMACDEMO_GameFonts,
		&ITECD_GameSound,
		GF_VOX_VOICES | GF_BIG_ENDIAN_DATA | GF_MAC_RESOURCES | GF_WYRMKEEP | GF_CD_FX
	},

	// Inherit the earth - MAC CD Wyrmkeep version
	{
		"ite",
		GType_ITE,
		GID_ITE_MACCD,
		"Inherit the Earth (MAC CD)",
		&ITE_DisplayInfo,
		ITE_DEFAULT_SCENE,
		&ITE_Resources,
		ARRAYSIZE(ITEMACCD_GameFiles),
		ITEMACCD_GameFiles,
		ARRAYSIZE(ITEMACDEMO_GameFonts),
		ITEMACDEMO_GameFonts,
		&ITECD_GameSound,
		GF_BIG_ENDIAN_DATA | GF_MAC_RESOURCES | GF_WYRMKEEP | GF_CD_FX
	},

	// Inherit the earth - Linux Demo version
	// Note: it should be before GID_ITE_WINDEMO2 version
	{
		"ite-demo",
		GType_ITE,
		GID_ITE_LINDEMO,
		"Inherit the Earth (Linux Demo)",
		&ITE_DisplayInfo,
		ITE_DEFAULT_SCENE,
		&ITE_Resources,
		ARRAYSIZE(ITELINDEMO_GameFiles),
		ITELINDEMO_GameFiles,
		ARRAYSIZE(ITECD_GameFonts),
		ITECD_GameFonts,
		&ITECD_GameSound,
		GF_VOX_VOICES | GF_WYRMKEEP | GF_CD_FX
	},
	
	// Inherit the earth - Linux Demo version
	{
		"ite-demo",
		GType_ITE,
		GID_ITE_WINDEMO2,
		"Inherit the Earth (Win32 Demo)",
		&ITE_DisplayInfo,
		ITE_DEFAULT_SCENE,
		&ITE_Resources,
		ARRAYSIZE(ITEWINDEMO_GameFiles),
		ITEWINDEMO_GameFiles,
		ARRAYSIZE(ITECD_GameFonts),
		ITECD_GameFonts,
		&ITECD_GameSound,
		GF_VOX_VOICES | GF_WYRMKEEP | GF_CD_FX
	},
	
	// Inherit the earth - Win32 Demo version
	{
		"ite-demo",
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
		&ITECD_GameSound,
		GF_VOX_VOICES | GF_WYRMKEEP | GF_CD_FX
	},
	
	// Inherit the earth - Wyrmkeep Linux CD version
	// should be before GID_ITE_CD_G
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
		GF_WYRMKEEP | GF_CD_FX
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
		GF_CD_FX
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
		GF_LANG_DE | GF_CD_FX
	},

	// Inherit the earth - CD version
	{
		"ite",
		GType_ITE,
		GID_ITE_CD,
		"Inherit the Earth (DOS/Win32 CD Version)",
		&ITE_DisplayInfo,
		ITE_DEFAULT_SCENE,
		&ITE_Resources,
		ARRAYSIZE(ITECD_GameFiles),
		ITECD_GameFiles,
		ARRAYSIZE(ITECD_GameFonts),
		ITECD_GameFonts,
		&ITECD_GameSound,
		GF_CD_FX
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
		&ITE_GameSound,
		GF_LANG_DE
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
		&ITE_GameSound,
		0
	},

	// I Have No Mouth And I Must Scream - Demo version
	{
		"ihnm-demo",
		GType_IHNM,
		GID_IHNM_DEMO,
		"I Have No Mouth and I Must Scream (DOS Demo)",
		&IHNM_DisplayInfo,
		0,
		&IHNM_Resources,
		ARRAYSIZE(IHNMDEMO_GameFiles),
		IHNMDEMO_GameFiles,
		ARRAYSIZE(IHNMCD_GameFonts),
		IHNMCD_GameFonts,
		&IHNM_GameSound,
		GF_DEFAULT_TO_1X_SCALER
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
		GF_DEFAULT_TO_1X_SCALER
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
		ARRAYSIZE(IHNMCDDE_GameFiles),
		IHNMCDDE_GameFiles,
		ARRAYSIZE(IHNMCD_GameFonts),
		IHNMCD_GameFonts,
		&IHNM_GameSound,
		GF_DEFAULT_TO_1X_SCALER
	}
};

int SagaEngine::initGame(void) {
	int gameNumber;
	FSList dummy;

	if ((gameNumber = detectGame(dummy)) == -1) {
		warning("No valid games were found in the specified directory.");
		return FAILURE;
	}

	if (loadGame(gameNumber) != SUCCESS) {
		warning("Error loading game resource files.");
		return FAILURE;
	}

	return SUCCESS;
}

RSCFILE_CONTEXT *SagaEngine::getFileContext(uint16 type, int param) {
	uint16 i;

	for (i = 0; i < _gameDescription->filesCount; i++) {
		if ( _gameDescription->filesDescriptions[i].fileType & type) {
			return _gameFileContexts[i];
		}
	}

	return NULL;
}

DetectedGameList GAME_ProbeGame(const FSList &fslist) {
	DetectedGameList detectedGames;
	int game_n;

	if ((game_n = detectGame(fslist, true)) != -1)
		detectedGames.push_back(gameDescriptions[game_n].toGameSettings());

	return detectedGames;
}

int detectGame(const FSList &fslist, bool mode) {
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

	for (game_n = 0; game_n < game_count; game_n++) {
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

			debug(5, "Probing game: %s", gameDescriptions[game_n].title);

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

			debug(5, "Found game: %s", gameDescriptions[game_n].title);

			return game_n;
		}
	}

	if (!filesMD5.isEmpty()) {
		printf("MD5s of your ITE version are unknown. Please, report following data to\n");
		printf("ScummVM team along with your ITE version:\n");

		for (StringMap::const_iterator file = filesMD5.begin(); file != filesMD5.end(); ++file)
			printf("%s: %s\n", file->_key.c_str(), file->_value.c_str());

	}

	return -1;
}

int SagaEngine::loadGame(int gameNumber) {
	RSCFILE_CONTEXT *loadContext;
	uint16 gameCount = ARRAYSIZE(gameDescriptions);
	const char *gameFileName;
	uint16 gameFileCount;
	uint16 i;

	if (gameNumber >= gameCount) {
		error("SagaEngine::loadGame wrong gameNumber");
	}

	_gameNumber = gameNumber;
	_gameDescription = &gameDescriptions[gameNumber];
	_gameDisplayInfo = *_gameDescription->gameDisplayInfo;

	gameFileCount = _gameDescription->filesCount;
	
	_gameFileContexts = (RSCFILE_CONTEXT **)realloc(_gameFileContexts, gameFileCount * sizeof(*_gameFileContexts));
	//TODO: on exit - FREE!
	if (_gameFileContexts == NULL) {
		memoryError("SagaEngine::loadGame");
	}


	// Load game resource files
	for (i = 0; i < gameFileCount; i++) {
		loadContext = RSC_CreateContext();
		gameFileName = _gameDescription->filesDescriptions[i].fileName;
		if (RSC_OpenContext(loadContext, gameFileName) != SUCCESS) {
			return FAILURE;
		}

		debug(0, "Opened resource file: %s", gameFileName);
		_gameFileContexts[i] = loadContext;
	}

	return SUCCESS;
}

} // End of namespace Saga
