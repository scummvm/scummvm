/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2004 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#ifndef __GAMES_H__
#define	__GAMES_H__

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
	UInt8 language;

} GameInfoType;

// protos
Err		GamOpenDatabase		();
void	GamImportDatabase	();
void	GamCloseDatabase	(Boolean ignoreCardParams);
Err		GamSortList			();
UInt16	GamGetSelected		();
void	GamUnselect			();

extern DmOpenRef gameDB;

#endif
