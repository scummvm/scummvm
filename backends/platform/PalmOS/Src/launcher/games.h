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

#ifndef __GAMES_H__
#define	__GAMES_H__

#define curItemVersion sysMakeROMVersion(3,5,8,0,0)	// Parallaction engine


#define itemVersion_357 sysMakeROMVersion(3,5,7,0,0)	// Added : Apple IIgs
#define itemVersion_356 sysMakeROMVersion(3,5,6,0,0)	// Changed : Simon -> AGOS
#define itemVersion_355 sysMakeROMVersion(3,5,5,0,0)	// Added : AGI engine
#define itemVersion_354 sysMakeROMVersion(3,5,4,0,0)	// Added : Default/Auto music driver
#define itemVersion_353 sysMakeROMVersion(3,5,3,0,0)	// Added : CinE engine and 3DO platform
#define itemVersion_352 sysMakeROMVersion(3,5,2,0,0)	// Added : Lure engine
#define itemVersion_351 sysMakeROMVersion(3,5,1,0,0)	// Added : Sega CD platform
#define itemVersion_350 sysMakeROMVersion(3,5,0,0,0)
#define itemVersion_340 sysMakeROMVersion(3,4,0,0,0)
#define itemVersion_330 sysMakeROMVersion(3,3,0,0,0)
#define itemVersion_320 sysMakeROMVersion(3,2,0,0,0)
#define itemVersion_310 sysMakeROMVersion(3,1,0,0,0)
#define itemVersion_300 sysMakeROMVersion(3,0,0,0,0)
#define itemVersion_270 sysMakeROMVersion(2,7,0,0,0)
#define itemVersion_260 sysMakeROMVersion(2,6,0,0,0)
#define itemVersion_250 sysMakeROMVersion(2,5,0,0,0)
#define itemVersion_200 sysMakeROMVersion(2,0,0,0,0)

// old config structs
typedef struct {
	UInt32	version;
	UInt16	icnID;			// icon to display on the list
	Boolean	selected;

	Char	nameP[50];		// game name to display in list
	Char	pathP[150];		// path to the game files
	Char	gameP[10];		// scumm name of the game
	UInt16	gfxMode;

	Boolean autoLoad;
	UInt16 loadSlot;
	Boolean bootParam;
	UInt16 bootValue;
	Boolean amiga;
	Boolean subtitles;
	Boolean talkSpeed;
	UInt16 talkValue;
	UInt8 language;

} GameInfoTypeV0;

typedef struct {
	UInt32	version;
	UInt16	icnID;			// icon to display on the list
	Boolean	selected;

	Char	nameP[50];		// game name to display in list
	Char	pathP[150];		// path to the game files
	Char	gameP[10];		// scumm name of the game
	UInt16	gfxMode;

	Boolean autoLoad;
	Boolean bootParam;
	Boolean setPlatform;
	Boolean subtitles;
	Boolean talkSpeed;

	UInt16 loadSlot;
	UInt16 bootValue;
	UInt16 talkValue;
	UInt8 platform;
	UInt8 language;

} GameInfoTypeV2;


// Current config
typedef struct {
	struct {
		UInt16 palm;
		UInt16 music;
		UInt16 sfx;		// TODO : remove
		UInt16 speech;
		UInt16 audiocd;
	} volume;

	struct {
		// midi
		Boolean multiMidi;
		Boolean music;		// TODO : rename this, it is enable audio option
		UInt8 drvMusic;
		UInt8 tempo;
		// sound FX
		Boolean sfx;		// TODO : remove this
		UInt8 rate;
		// CD audio
		Boolean CD;
		UInt8 drvCD, frtCD;
		UInt16 defaultTrackLength;
		UInt16 firstTrack;
	} sound;
} MusicInfoType;

typedef struct {
	UInt32	version;
	UInt16	icnID;			// icon to display on the list
	Boolean	selected;

	Char	nameP[50];		// game name to display in list
	Char	pathP[150];		// path to the game files
	Char	gameP[15];		// scumm name of the game
	UInt16	gfxMode;

	Boolean autoLoad;
	Boolean bootParam;
	Boolean setPlatform;
	Boolean subtitles;
	Boolean talkSpeed;

	UInt16 loadSlot;
	UInt16 bootValue;
	UInt16 talkValue;
	UInt8 platform;
	UInt8 language;			// |-	v2.5

	Boolean filter;			//		v2.6
	Boolean fullscreen;		// |
	Boolean aspectRatio;	// |-	v2.7

	MusicInfoType musicInfo;//		v3.0

	UInt8 engine;			// |-	v3.1
	UInt8 renderMode;		// |-	v3.2
//	Boolean use16Bit;		// |-	v3.3	// removed
	UInt8 fmQuality;		// |-	v3.4	// replace use16Bit
} GameInfoType;

enum {
	ENGINE_AGOS = 0,
	ENGINE_SKY,
	ENGINE_SWORD1,
	ENGINE_SWORD2,
	ENGINE_CINE,
	ENGINE_QUEEN,
	ENGINE_LURE,
	ENGINE_GOB,
	ENGINE_KYRA,
	ENGINE_PARALLACTION,
	ENGINE_SAGA,
	ENGINE_SCUMM,
	ENGINE_AGI,
	ENGINE_TOUCHE,
	ENGINE_CRUISE,
	ENGINE_DRASCULA,
	ENGINE_COUNT
};

static const struct {
	const char *fileP;
	const char *nameP;
} engines[] = {
	{ "agos",			"AGOS Engine" },
	{ "sky",			"Beneath a Steel Sky" },
	{ "sword1",			"Broken Sword I" },
	{ "sword2",			"Broken Sword II" },
	{ "cine",			"Delphine Cinematique v1.0" },
	{ "queen",			"Flight of the Amazon Queen" },
	{ "lure",			"Lure of the Tempress" },
	{ "gob",			"Gobliiins, Bargon Attack and more" },
	{ "kyra",			"Kyrandia" },
	{ "parallaction",	"Parallaction" },
	{ "saga",			"SAGA Engine" },
	{ "scumm",			"Scumm Games" },
	{ "agi",			"Sierra AGI" },
	{ "touche",			"Touche: The Adventures of the Fifth Musketeer" },
	{ "cruise",			"Beta -> Cruise for a Corpse" },
	{ "drascula",		"Beta -> Drascula" },
};

// protos
Err		GamOpenDatabase		();
void	GamImportDatabase	();
void	GamCloseDatabase	(Boolean ignoreCardParams);
Err		GamSortList			();
UInt16	GamGetSelected		();
void	GamUnselect			();
Boolean GamJumpTo			(Char letter);

extern DmOpenRef gameDB;

#endif
