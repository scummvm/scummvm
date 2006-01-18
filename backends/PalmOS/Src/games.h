/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2006 The ScummVM project
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
 * $Header$
 *
 */

#ifndef __GAMES_H__
#define	__GAMES_H__

#define curItemVersion sysMakeROMVersion(3,4,0,0,0)

#define itemVersion_33 sysMakeROMVersion(3,3,0,0,0)
#define itemVersion_32 sysMakeROMVersion(3,2,0,0,0)
#define itemVersion_31 sysMakeROMVersion(3,1,0,0,0)
#define itemVersion_30 sysMakeROMVersion(3,0,0,0,0)
#define itemVersion_27 sysMakeROMVersion(2,7,0,0,0)
#define itemVersion_26 sysMakeROMVersion(2,6,0,0,0)
#define itemVersion_25 sysMakeROMVersion(2,5,0,0,0)
#define itemVersion_20 sysMakeROMVersion(2,0,0,0,0)

// old config structs
typedef struct {
	UInt32	version;
	UInt16	icnID;			// icon to display on the list
	Boolean	selected;

	Char 	nameP[50];		// game name to display in list
	Char 	pathP[150];		// path to the game files
	Char 	gameP[10];		// scumm name of the game
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

	Char 	nameP[50];		// game name to display in list
	Char 	pathP[150];		// path to the game files
	Char 	gameP[10];		// scumm name of the game
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
		UInt16 sfx;
		UInt16 speech;
		UInt16 audiocd;
	} volume;

	struct {
		// midi
		Boolean multiMidi;
		Boolean music;
		UInt8 drvMusic;
		UInt8 tempo;
		// sound FX
		Boolean sfx;
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

	Char 	nameP[50];		// game name to display in list
	Char 	pathP[150];		// path to the game files
	Char 	gameP[15];		// scumm name of the game
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
	UInt8 language;			// |- 	v2.5

	Boolean filter;			// 		v2.6
	Boolean fullscreen;		// |
	Boolean aspectRatio;	// |-	v2.7

	MusicInfoType musicInfo;// 		v3.0

	UInt8 engine;			// |-	v3.1
	UInt8 renderMode;		// |-	v3.2
//	Boolean use16Bit;		// |-	v3.3	// removed
	UInt8 fmQuality;		// |-	v3.4	// replace use16Bit
} GameInfoType;
/*
enum {
	ENGINE_SCUMM = 0,
	ENGINE_SIMON,
	ENGINE_QUEEN,
	ENGINE_SWORD1,
	ENGINE_SKY,
	ENGINE_SAGA,
	ENGINE_GOB,
	ENGINE_KYRA,
	ENGINE_SWORD2
};
*/

enum {
	ENGINE_SKY = 0,
	ENGINE_SWORD1,
	ENGINE_SWORD2,
	ENGINE_QUEEN,
	ENGINE_GOB,
	ENGINE_KYRA,
	ENGINE_SAGA,
	ENGINE_SIMON,
	ENGINE_SCUMM,
	ENGINE_COUNT
};

static const struct {
	const char *fileP;
	const char *nameP;
} engines[] = {
	{ "sky",	"Beneath a Steel Sky" },
	{ "sword1",	"Broken Sword 1" },
	{ "sword2",	"Broken Sword 2" },
	{ "queen",	"Flight of the Amazon Queen" },
	{ "gob",	"Gobliiins" },
	{ "kyra",	"Kyrandia" },
	{ "saga",	"SAGA Engine" },
	{ "simon",	"Simon the Sorcerer" },
	{ "scumm",	"Scumm Games" },
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
