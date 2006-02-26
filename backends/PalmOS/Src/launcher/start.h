/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2006 The ScummVM project
 * Copyright (C) 2002-2006 Chris Apers - PalmOS Backend
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
 
#ifndef __START_H__
#define __START_H__

typedef struct {
	Char nameP[32];
	UInt16 cardNo;
	LocalID dbID;
} SkinInfoType, *SkinInfoPtr;

typedef	struct {

	//skin params
	SkinInfoType skin;	//	card where is located the skin
	Boolean soundClick;	

	Boolean vibrator;
	Boolean autoOff;
	Boolean setStack;
	Boolean exitLauncher;
	Boolean goLCD;
	Boolean stylusClick;

	UInt16 listPosition;
	UInt16 autoSavePeriod;

	struct {
		UInt16 volRefNum;
		UInt32 cacheSize;
		Boolean useCache;
		Boolean showLED;
		Boolean moveDB;
		Boolean deleteDB;
		Boolean confirmMoveDB;
	} card;

	Boolean debug;
	UInt16 debugLevel;
	Boolean stdPalette;
	Boolean demoMode;
	Boolean copyProtection;
	Boolean oldarm;// TODO : free slot...
	Boolean altIntro;
	Boolean autoSave;
	Boolean advancedMode;

	struct {
		Boolean enable;
		UInt8 mode;
	} lightspeed;

} GlobalsPreferenceType, *GlobalsPreferencePtr;

extern GlobalsPreferencePtr gPrefs;

extern Boolean bDirectMode;
extern Boolean bStartScumm;
extern Boolean bLaunched;

#define appPrefID				0x00
#define appVersionNum			0x01
#define appPrefVersionNum		0x02

#define STACK_DEFAULT			8192
#define STACK_LARGER			16384
#define STACK_GET				0

Err AppStart(void);
void AppStop(void);
Boolean StartScummVM();
void SavePrefs();
Err SendDatabase (UInt16 cardNo, LocalID dbID, Char *nameP, Char *descriptionP);
#endif
