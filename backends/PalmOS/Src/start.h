/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2003 The ScummVM project
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
 
#ifndef __START_H__
#define __START_H__

#define appFileCreator 'ScVM'
#define curItemVersion sysMakeROMVersion(2,0,0,0,0)

typedef struct {
	Char nameP[32];
	UInt16 cardNo;
	LocalID dbID;
} SkinInfoType, *SkinInfoPtr;

typedef	struct {

	//skin params
	SkinInfoType skin;	//	card where is located the skin
	Boolean soundClick;	
	//
	Boolean vibrator;
	Boolean autoOff;

	UInt16 listPosition;

	struct {
		UInt16 volRefNum;
		Boolean moveDB;
		Boolean deleteDB;
		Boolean confirmMoveDB;
	} card;

	Boolean debug;
	UInt16 debugLevel;
	Boolean saveConfig;
	Boolean stdPalette;
	Boolean autoReset;
	Boolean demoMode;
	Boolean fullscreen;

	struct {
		UInt16 speaker;
		UInt16 headphone;
		
		UInt16 master;
		UInt16 music;
		UInt16 sfx;
	} volume;
	
	struct {
		// midi
		Boolean multiMidi;
		Boolean music;
		UInt8 driver;
		UInt8 tempo;
		// CD audio
		Boolean MP3;
		Boolean setDefaultTrackLength;
		UInt16 defaultTrackLength;
		UInt16 firstTrack;
		// sound FX
		Boolean sfx;
	} sound;

} GlobalsPreferenceType, *GlobalsPreferencePtr;

extern GlobalsPreferencePtr gPrefs;

#endif