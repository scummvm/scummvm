/******************************************************************************
 *
 * File: palmstart.cpp
 *
 *****************************************************************************/

#include <PalmOS.h>
#include <SonyClie.h>
#include "StarterRsc.h"
#include "extras.h"
#include "skin.h"
#include "globals.h"
#include "pa1lib.h"
#include "scumm_globals.h"
#include "extend.h"			// for disable state

void MemExtInit();
void MemExtCleanup();
/***********************************************************************
 *
 *	Entry Points
 *
 ***********************************************************************/
//UInt8 *screen_1;
//UInt8 *screen_2;
//extern FileRef gLogFile;
//extern UInt16 gHRrefNum;
//extern UInt16 gVolRefNum;
//extern Boolean gScreenLocked;
//extern Boolean gFlipping;
//extern Boolean gVibrator;
/***********************************************************************
 *
 *	Internal Structures
 *
 ***********************************************************************/
typedef struct {
	Char nameP[32];
	UInt16 cardNo;
	LocalID dbID;
} SkinInfoType;

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
	Boolean autoRoom;
	UInt16 roomNum;
	Boolean amiga;
	Boolean subtitles;
	Boolean talkSpeed;
	UInt16 talkValue;

} GameInfoType;

typedef	struct {

	//skin params
	SkinInfoType skin;	//	card where is located the skin
	//
	Boolean vibrator;
	Boolean autoOff;

	UInt16 listPosition;
	UInt16 volRefNum;

	Boolean debug;
	UInt16 debugLevel;

	Boolean stdPalette;
	
	struct {
		UInt16 speaker;
		UInt16 headphone;
		
		UInt16 master;
		UInt16 music;
		UInt16 sfx;
	} volume;
	
	struct {
		Boolean music;
		UInt8 driver;
		UInt32 tempo;
		Boolean sfx;
	} sound;
		

} GlobalsPreferenceType;

typedef struct {
	UInt16 volRefNum;
	Char nameP[expCardInfoStringMaxLen+1];

} CardInfoType;

/***********************************************************************
 *
 *	Global variables
 *
 ***********************************************************************/
static GlobalsPreferenceType *gPrefs;
//static SkinInfoType _skin;
static DmOpenRef _dbP = NULL;
static UInt16 _lstIndex = 0;	// last index
static UInt8 __editMode__;
static UInt16 sknLastOn = skinButtonNone;

GlobalsDataType *gVars;
/***********************************************************************
 *
 *	Internal Constants
 *
 ***********************************************************************/
#define appFileCreator			'ScVM'	// register your own at http://www.palmos.com/dev/creatorid/
#define appVersionNum			0x01
#define appPrefID				0x00
#define appPrefVersionNum		0x01

// Define the minimum OS version we support (3.5 for now).
#define kOurMinVersion	sysMakeROMVersion(3,5,0,sysROMStageRelease,0)
#define kPalmOS10Version	sysMakeROMVersion(1,0,0,sysROMStageRelease,0)
#define kOS5Version		sysMakeROMVersion(5,0,0,sysROMStageRelease,0)

// edit game mode
#define edtModeAdd			0
#define edtModeEdit			1
#define edtModeParams		2

// skin 
#define	sknInfoState		0
#define	sknInfoPosX			1
#define	sknInfoPosY			2

#define	sknInfoMaxWOrH		3
#define	sknInfoDrawMode		4
#define	sknInfoKeepXOrY1	5
#define	sknInfoKeepXOrY2	7

#define sknInfoListWidth	sknInfoMaxWOrH
#define sknInfoListHeight	sknInfoDrawMode
#define sknInfoListSize		sknInfoListHeight
#define sknInfoListItemSize	12

#define	sknPosRsc			'sPos'
#define	sknColorsRsc		'sCol'

#define sknStateNormal		0
#define sknStateSelected	10
#define sknStateDisabled	20

#define sknSelectedState(bmp)	(bmp + sknStateSelected)
#define sknDisabledState(bmp)	(bmp + sknStateDisabled)

/***********************************************************************
 *
 *	Internal Functions
 *
 ***********************************************************************/
static void GBInitAll() {
#ifndef DISABLE_SCUMM
	IMuseDigital_initGlobals();
	NewGui_initGlobals();
	//Resource_initGlobals();
	Codec47_initGlobals();
	Gfx_initGlobals();
#endif
#ifndef DISABLE_SIMON
	Simon_initGlobals();
#endif
}

static void GBReleaseAll() {
#ifndef DISABLE_SCUMM
	IMuseDigital_releaseGlobals();
	NewGui_releaseGlobals();
	//Resource_releaseGlobals();
	Codec47_releaseGlobals();
	Gfx_releaseGlobals();
#endif
#ifndef DISABLE_SIMON
	Simon_releaseGlobals();
#endif
}

//TODO : use Boolean instead of void to check err
static DmOpenRef GBOpenInternal(const Char *nameP) {
	LocalID dbID = DmFindDatabase(0, nameP);
	if (dbID) {
		UInt32 dbType, dbCreator;
		Err e = DmDatabaseInfo(0, dbID, 0, 0, 0, 0, 0, 0, 0, 0, 0, &dbType, &dbCreator);
		
		if (!e && dbType == 'GLBS' && dbCreator == appFileCreator)
			return DmOpenDatabase(0, dbID, dmModeReadOnly);
	}
	return NULL;
}

static void GBOpen() {
	gVars->globals[GBVARS_SCUMM] = GBOpenInternal("Scumm-Globals");
	gVars->globals[GBVARS_SIMON] = GBOpenInternal("Simon-Globals");
	gVars->globals[GBVARS_SKY] = GBOpenInternal("Sky-Globals");
}

static void GBClose() {
	if (gVars->globals[GBVARS_SCUMM])
		DmCloseDatabase(gVars->globals[GBVARS_SCUMM]);
	if (gVars->globals[GBVARS_SIMON])
		DmCloseDatabase(gVars->globals[GBVARS_SIMON]);
	if (gVars->globals[GBVARS_SKY])
		DmCloseDatabase(gVars->globals[GBVARS_SKY]);
}

void *GBGetRecord(UInt16 index, UInt16 id) {
	if (gVars->globals[id]) {
		MemHandle recordH = DmQueryRecord(gVars->globals[id], index);
		if (recordH)
			return MemHandleLock(recordH);
	}
	return NULL;
}

void GBReleaseRecord(UInt16 index, UInt16 id) {
	if (gVars->globals[id]) {
		MemHandle recordH = DmQueryRecord(gVars->globals[id], index);
		if (recordH)
			MemHandleUnlock(recordH);
	}
}
 /////////////////////////////////////////////////////////////////////////
static DmOpenRef SknOpenSkin() {
	return DmOpenDatabase(gPrefs->skin.cardNo, gPrefs->skin.dbID, dmModeReadOnly);
}

static void SknCloseSkin(DmOpenRef skinDBP) {
	if (skinDBP)
		DmCloseDatabase(skinDBP);
}
/*
static void SknSetPalette() {
	UInt16 palIndex;
	ColorTableType *palTemp;
	MemHandle palH;

	DmOpenRef skinDBP = SknOpenSkin();

	if (skinDBP) {
		palIndex = DmFindResource (skinDBP, colorTableRsc, skinPalette, NULL);

		if (palIndex != (UInt16)-1) {
			palH = DmGetResourceIndex(skinDBP, palIndex);
			
			if (palH) {
				palTemp = (ColorTableType *)MemHandleLock(palH);
				WinPalette(winPaletteSet, 0, 256, ColorTableEntries(palTemp));
				MemPtrUnlock(palTemp);					
				DmReleaseResource(palH);
			}
		}
	}
	
	SknCloseSkin(skinDBP);
}
*/
static void SknGetListColors(DmOpenRef skinDBP, DmResID resID, UInt8 *text, UInt8 *selected, UInt8 *background) {
	UInt16 colIndex;
	MemHandle colH;
	UInt8 *colTemp;

	// default
	*text = UIColorGetTableEntryIndex (UIMenuForeground);
	*selected = UIColorGetTableEntryIndex (UIMenuSelectedForeground);
	*background = UIColorGetTableEntryIndex (UIMenuSelectedFill);

	if (skinDBP) {
		colIndex = DmFindResource (skinDBP, sknColorsRsc, resID, NULL);
		
		if (colIndex != (UInt16)-1) {
			colH = DmGetResourceIndex(skinDBP, colIndex);
			
			if (colH) {
				colTemp = (UInt8 *)MemHandleLock(colH);
				
				*text = colTemp[0];
				*selected = colTemp[1];
				*background = colTemp[2];

				MemPtrUnlock(colTemp);					
				DmReleaseResource(colH);
			}
		}
	}
}

static void SknGetObjectBounds(DmOpenRef skinDBP, DmResID resID, RectangleType *rP) {

	UInt16 bmpIndex, strIndex;
	MemHandle hBmp, hStr;
	BitmapType *bmpTemp;
	UInt8 *strTemp;

	RctSetRectangle(rP, 0, 0, 0, 0);
	
	if (skinDBP) {
		bmpIndex = DmFindResource (skinDBP, bitmapRsc, resID, NULL);
		
		if (bmpIndex != (UInt16)-1) {						// if bmp exists
			strIndex = DmFindResource (skinDBP, sknPosRsc, resID, NULL);
			
			if (strIndex != (UInt16)-1) {					// if params exist
				hBmp = DmGetResourceIndex(skinDBP,bmpIndex);

				if (hBmp) {
					hStr = DmGetResourceIndex(skinDBP,strIndex);
					
					if (hStr) {
					//	buttons : state|x|y|w/h slider|draw mode|x1/y1 keep|x2/y2 keep slider
					//	list (160mode) : state|x|y|w|h|
						bmpTemp = (BitmapType *)MemHandleLock(hBmp);
						strTemp = (UInt8 *)MemHandleLock(hStr);
						
						BmpGlueGetDimensions(bmpTemp, &(rP->extent.x), &(rP->extent.y), 0);
						rP->topLeft.x = strTemp[sknInfoPosX] * 2;
						rP->topLeft.y = strTemp[sknInfoPosY] * 2;
						
						MemPtrUnlock(strTemp);					
						DmReleaseResource(hStr);
					}

					MemPtrUnlock(bmpTemp);
					DmReleaseResource(hBmp);
				}
			}
		}
	}
}

static UInt8 SknGetState(DmOpenRef skinDBP, DmResID resID) {

	UInt16 index;
	MemHandle hStr;
	UInt8 *strTemp;
	UInt8 oldState = sknStateDisabled;

	if (skinDBP) {
		index = DmFindResource (skinDBP, sknPosRsc, resID, NULL);
		
		if (index != (UInt16)-1) {
			hStr = DmGetResourceIndex(skinDBP, index);
			
			if (hStr) {
				strTemp = (UInt8 *)MemHandleLock(hStr);
				oldState = strTemp[sknInfoState];				
				MemPtrUnlock(strTemp);					
				DmReleaseResource(hStr);				
			}
		}
	}

	return oldState;
}

static UInt8 SknSetState(DmOpenRef skinDBP, DmResID resID, UInt8 newState) {

	UInt16 index;
	MemHandle hStr;
	UInt8 *strTemp;
	UInt8 oldState = 0;

	if (skinDBP) {
		index = DmFindResource (skinDBP, sknPosRsc, resID, NULL);
		
		if (index != (UInt16)-1) {
			hStr = DmGetResourceIndex(skinDBP, index);
			
			if (hStr) {
				strTemp = (UInt8 *)MemHandleLock(hStr);
				oldState = strTemp[sknInfoState];
				
				if (oldState != newState) {
					DmWrite(strTemp, 0, &newState, 1);
				}
				
				MemPtrUnlock(strTemp);					
				DmReleaseResource(hStr);				
			}
		}
	}

	return oldState;
}

static void SknCopyBits(DmOpenRef skinDBP, DmResID bitmapID, const RectangleType *srcRect, Coord destX, Coord destY, Boolean standard) {
	MemHandle hTemp;
	BitmapPtr bmpTemp;
	UInt16 index;
	UInt8 *bmpData;
//	Err e;

	Coord cx, cy, cw, ch, bw, bh;
	UInt8 *dst, *src;

	if (skinDBP) {
		// find the bitmap
		index = DmFindResource (skinDBP, bitmapRsc, bitmapID, NULL);

		if (index != (UInt16)-1) {
			hTemp = DmGetResourceIndex(skinDBP,index);
			
			if (hTemp) {
				bmpTemp = (BitmapType *)MemHandleLock(hTemp);
				BmpGlueGetDimensions(bmpTemp, &bw, &bh, 0);

				if (!srcRect)
				{
					cx = 0;
					cy = 0;
					cw = bw;
					ch = bh;
				}
				else
				{
					cx = srcRect->topLeft.x;
					cy = srcRect->topLeft.y;
					cw = srcRect->extent.x;
					ch = srcRect->extent.y;
				}

				if (ch) {
					dst = (UInt8 *)BmpGetBits(WinGetBitmap(WinGetDisplayWindow()));
					dst+= destX + destY * 320;
					bmpData = (UInt8 *)BmpGetBits(bmpTemp);
					src	= bmpData + cx + cy * bw;

					do {
						MemMove(dst, src, cw);
						dst += 320;
						src += bw;
					} while (--ch);
				}

				MemPtrUnlock(bmpTemp);
				DmReleaseResource(hTemp);
			}
		}
	}
}

static void SknShowObject(DmOpenRef skinDBP, DmResID resID) {

	RectangleType r;
	UInt8 state = SknGetState(skinDBP, resID);
	SknGetObjectBounds(skinDBP, resID, &r);
	SknCopyBits(skinDBP, resID + state, NULL, r.topLeft.x, r.topLeft.y, 0);
}

static UInt16 SknCheckClick(DmOpenRef skinDBP, Coord mx, Coord my)
{
	UInt16 resID;
	RectangleType r;

	mx *= 2;
	my *= 2;

	if (skinDBP) {
		for (resID = 1100; resID <= 7000; resID += 100) {
			if (SknGetState(skinDBP, resID) != sknStateDisabled) {
				SknGetObjectBounds(skinDBP, resID, &r);
				if (RctPtInRectangle(mx, my, &r)) {
					return resID;
				}
			}
		}
		
	}
	
	return 0;
}


// Callback for ExgDBWrite to send data with Exchange Manager
static Err WriteDBData(const void* dataP, UInt32* sizeP, void* userDataP)
{
	Err err;
	*sizeP = ExgSend((ExgSocketPtr)userDataP, (void*)dataP, *sizeP, &err);
	return err;
}

static Err SendDatabase (UInt16 cardNo, LocalID dbID, Char *nameP, Char *descriptionP)
{
	ExgSocketType exgSocket;
	Err err;

	// Create exgSocket structure
	MemSet(&exgSocket, sizeof(exgSocket), 0);
	exgSocket.description = descriptionP;
	exgSocket.name = nameP;

	// Start an exchange put operation
	err = ExgPut(&exgSocket);
	if (!err) {
		err = ExgDBWrite(WriteDBData, &exgSocket, NULL, dbID, cardNo);
		err = ExgDisconnect(&exgSocket, err);
	}

	return err;
}

static Err BeamMe() {

	UInt16 cardNo;
	LocalID dbID;
	Err err;

	err = SysCurAppDatabase(&cardNo, &dbID);
	if (dbID)
		err = SendDatabase(0, dbID, "ScummVM.prc", "\nPlay your favorite LucasArts games");
	else
		err = DmGetLastErr();
	
	return err;
}
/***********************************************************************
 *
 * FUNCTION:    RomVersionCompatible
 *
 * DESCRIPTION: This routine checks that a ROM version is meet your
 *              minimum requirement.
 *
 * PARAMETERS:  requiredVersion - minimum rom version required
 *                                (see sysFtrNumROMVersion in SystemMgr.h 
 *                                for format)
 *              launchFlags     - flags that indicate if the application 
 *                                UI is initialized.
 *
 * RETURNED:    error code or zero if rom is compatible
 *
 * REVISION HISTORY:
 *
 *
 ***********************************************************************/
static Err RomVersionCompatible(UInt32 requiredVersion, UInt16 launchFlags)
{
	UInt32 romVersion;

	// See if we're on in minimum required version of the ROM or later.
	FtrGet(sysFtrCreator, sysFtrNumROMVersion, &romVersion);
	if (romVersion < requiredVersion)
		{
		if ((launchFlags & (sysAppLaunchFlagNewGlobals | sysAppLaunchFlagUIApp)) ==
			(sysAppLaunchFlagNewGlobals | sysAppLaunchFlagUIApp))
			{
			FrmAlert (RomIncompatibleAlert);
		
			// Palm OS 1.0 will continuously relaunch this app unless we switch to 
			// another safe one.
			if (romVersion <= kPalmOS10Version)
				{
				AppLaunchWithCommand(sysFileCDefaultApp, sysAppLaunchCmdNormalLaunch, NULL);
				}
			}
		return sysErrRomIncompatible;
		}

	return errNone;
}


/***********************************************************************
 *
 * FUNCTION:    GetObjectPtr
 *
 * DESCRIPTION: This routine returns a pointer to an object in the current
 *              form.
 *
 * PARAMETERS:  formId - id of the form to display
 *
 * RETURNED:    void *
 *
 * REVISION HISTORY:
 *
 *
 ***********************************************************************/
static void * GetObjectPtr(UInt16 objectID)
{
	FormPtr frmP;

	frmP = FrmGetActiveForm();
	return FrmGetObjectPtr(frmP, FrmGetObjectIndex(frmP, objectID));
}

static void FrmList(EventPtr eventP, UInt16 objectID)
{
	ListType *listP;
	UInt16 listItem;

	listP = (ListType *)GetObjectPtr(objectID);
	listItem = LstPopupList(listP);
	CtlSetLabel(eventP->data.ctlSelect.pControl, LstGetSelectionText(listP, LstGetSelection(listP)));
}

//#############################################################################
//#############################################################################

enum {
	IcnNone = 255,
	IcnInfo = 0,
	IcnEdit = 1,
	IcnDelt = 2,
	IcnVibr = 3,
	IcnAOff = 4,
	IcnPlay = 5
};

enum {
	ArwNone = 255,
	ArwUp   = 0,
	ArwFUp  = 1,
	ArwFDwn = 2,
	ArwDown = 3
};
/*
//REMOVE
static struct {
	Boolean disabled;
	Boolean selected;
} iconState[6] = {0,0 ,0,0 ,0,0 ,1,0 ,0,0 ,0,0};
//REMOVE
static struct {
	UInt16 position;
	UInt8 last;
	Boolean disabled[4];
} ArrowManager = {0, ArwNone, 0,0,0,0};
*/

//#############################################################################
//#############################################################################

static Err GamOpenDatabase() {
	Err err = errNone;

	_dbP = DmOpenDatabaseByTypeCreator( 'DATA', appFileCreator, dmModeReadWrite);

	if (!_dbP) {
		err = DmCreateDatabase(0, "ScummVM-Data", appFileCreator, 'DATA', false);
		if (!err) {
			_dbP = DmOpenDatabaseByTypeCreator( 'DATA', appFileCreator, dmModeReadWrite);
			
			if (!_dbP)
				err = DmGetLastErr();
		}
	}

	if (err)
		FrmCustomAlert(FrmErrorAlert,"Cannot open/create games list DB !",0,0);
	return err;
}

static void GamCloseDatabase() {
	if (_dbP)
		DmCloseDatabase(_dbP);
	_dbP = NULL;
}

static Int16 GamCompare(GameInfoType *a, GameInfoType *b, SortRecordInfoPtr, SortRecordInfoPtr, MemHandle) {
	return StrCompare(a->nameP, b->nameP);
}
static Err GamSortList() {
	return DmQuickSort (_dbP, (DmComparF *)GamCompare, 0);
}


static UInt16 GamGetSelected()
{
	MemHandle record;
	GameInfoType *game;
	Boolean selected;
	UInt16 index = DmNumRecords(_dbP)-1;
	
	while (index != (UInt16)-1) {
		record = DmQueryRecord(_dbP, index);
		game = (GameInfoType *)MemHandleLock(record);
		selected = game->selected;
		MemHandleUnlock(record);

		if (selected)
			return index;

		index--;
	}

	return dmMaxRecordIndex;
}
static void GamUnselect() {
	GameInfoType modGame;
	GameInfoType *game;

	MemHandle recordH;
	UInt16 index;

	index = GamGetSelected();
	
	if (index != dmMaxRecordIndex) {
		recordH = DmGetRecord(_dbP, index);
		game = (GameInfoType *)MemHandleLock(recordH);

		MemMove(&modGame, game, sizeof(GameInfoType));	
		modGame.selected = !modGame.selected;
		DmWrite(game, 0, &modGame, sizeof(GameInfoType));

		MemHandleUnlock(recordH);
		DmReleaseRecord (_dbP, index, 0);
	}
}

static void IcnRedrawTools(DmOpenRef skinDBP)
{
	if (GamGetSelected() == dmMaxRecordIndex) {
		if (SknGetState(skinDBP, skinButtonGameDelete) == sknStateNormal) {
			SknSetState(skinDBP, skinButtonGameDelete,sknStateDisabled);
			SknShowObject(skinDBP, skinButtonGameDelete);
		}
//		if (SknGetState(skinDBP, skinButtonGameStart) == sknStateNormal) {
//			SknSetState(skinDBP, skinButtonGameStart,sknStateDisabled);
//			SknShowObject(skinDBP, skinButtonGameStart);
//		}
		if (SknGetState(skinDBP, skinButtonGameEdit) == sknStateNormal) {
			SknSetState(skinDBP, skinButtonGameEdit,sknStateDisabled);
			SknShowObject(skinDBP, skinButtonGameEdit);
		}

	} else {
		if (SknGetState(skinDBP, skinButtonGameDelete) == sknStateDisabled) {
			SknSetState(skinDBP, skinButtonGameDelete,sknStateNormal);
			SknShowObject(skinDBP, skinButtonGameDelete);
		}
//		if (SknGetState(skinDBP, skinButtonGameStart) == sknStateDisabled) {
//			SknSetState(skinDBP, skinButtonGameStart,sknStateNormal);
//			SknShowObject(skinDBP, skinButtonGameStart);
//		}
		if (SknGetState(skinDBP, skinButtonGameEdit) == sknStateDisabled) {
			SknSetState(skinDBP, skinButtonGameEdit,sknStateNormal);
			SknShowObject(skinDBP, skinButtonGameEdit);
		}
	}
}

static void ArwRedrawSlider(DmOpenRef skinDBP, UInt16 index, UInt16 maxIndex, UInt16 perPage)
{
	if (maxIndex <= perPage) {
		if (SknGetState(skinDBP,skinSliderUpArrow) != sknStateDisabled) {
			SknSetState(skinDBP,skinSliderUpArrow,sknStateDisabled);
			SknShowObject(skinDBP,skinSliderUpArrow);
		}
		if (SknGetState(skinDBP,skinSliderDownArrow) != sknStateDisabled) {
			SknSetState(skinDBP,skinSliderDownArrow,sknStateDisabled);
			SknShowObject(skinDBP,skinSliderDownArrow);
		}

	} else {
		if (SknGetState(skinDBP,skinSliderUpArrow) == sknStateDisabled) {
			SknSetState(skinDBP,skinSliderUpArrow,sknStateNormal);
			SknShowObject(skinDBP,skinSliderUpArrow);
		}
		if (SknGetState(skinDBP,skinSliderDownArrow) == sknStateDisabled) {
			SknSetState(skinDBP,skinSliderDownArrow,sknStateNormal);
			SknShowObject(skinDBP,skinSliderDownArrow);
		}
	}
}

static void GamGetListBounds(RectangleType *rAreaP, RectangleType *rArea2xP) {
	DmOpenRef skinDBP;
	UInt16 strIndex;
	MemHandle hStr;
	UInt8 *strTemp;
	UInt16 x,y,w,h;

	skinDBP = DmOpenDatabase(gPrefs->skin.cardNo, gPrefs->skin.dbID, dmModeReadOnly);
	if (skinDBP) {
		strIndex = DmFindResource (skinDBP, sknPosRsc, skinList, NULL);
		
		if (strIndex != (UInt16)-1) {					// if params exist
			hStr = DmGetResourceIndex(skinDBP,strIndex);
			if (hStr) {
				strTemp = (UInt8 *)MemHandleLock(hStr);

				x = strTemp[sknInfoPosX];
				y = strTemp[sknInfoPosY];
				w = strTemp[sknInfoListWidth];
				h = strTemp[sknInfoListSize] * sknInfoListItemSize;
				
				if (rAreaP)
					RctSetRectangle(rAreaP ,x, y, w, h);
				if (rArea2xP)
					RctSetRectangle(rArea2xP, x+x, y+y, w+w, h+h);

				MemHandleUnlock(hStr);
				DmReleaseResource(hStr);
				
			}
		}

		DmCloseDatabase(skinDBP);
	}
}

static void GamUpdateList()
{
	MemHandle record;
	UInt16 index, maxIndex, maxView;
	GameInfoType *game;
	RectangleType rArea, rField, rCopy, rArea2x;
	DmOpenRef skinDBP;

	UInt8 txtColor, norColor, selColor, bkgColor;
	UInt16 x,y;

	WinScreenLock(winLockCopy);
	GamGetListBounds(&rArea, &rArea2x);
	skinDBP = SknOpenSkin();
	// set default bg
	WinSetForeColor(UIColorGetTableEntryIndex (UIFormFill));
	WinDrawRectangle(&rArea,0);
	// copy top bg
	SknGetObjectBounds(skinDBP, skinBackgroundImageTop, &rField);
	RctGetIntersection(&rArea2x, &rField, &rCopy);
	x = rCopy.topLeft.x;
	y = rCopy.topLeft.y;
	rCopy.topLeft.x	-= rField.topLeft.x;
	rCopy.topLeft.y	-= rField.topLeft.y;	
	SknCopyBits(skinDBP, skinBackgroundImageTop, &rCopy, x, y, 0);
	// copy bottom bg
	SknGetObjectBounds(skinDBP, skinBackgroundImageBottom, &rField);
	RctGetIntersection(&rArea2x, &rField, &rCopy);
	x = rCopy.topLeft.x;
	y = rCopy.topLeft.y;
	rCopy.topLeft.x	-= rField.topLeft.x;
	rCopy.topLeft.y	-= rField.topLeft.y;	
	SknCopyBits(skinDBP, skinBackgroundImageBottom, &rCopy, x, y, 0);

	FntSetFont(stdFont);

	index = gPrefs->listPosition;
	maxIndex = DmNumRecords(_dbP);
	maxView = rArea.extent.y / sknInfoListItemSize;
	
	if (index > 0 && (index+maxView) > maxIndex) {
		index -= (index+maxView) - maxIndex;
		 gPrefs->listPosition = index;
	}	
	ArwRedrawSlider(skinDBP, index, maxIndex, maxView);
	IcnRedrawTools(skinDBP);
	SknGetListColors(skinDBP, skinColors, &norColor, &selColor, &bkgColor);

	SknCloseSkin(skinDBP);

	while (index < (gPrefs->listPosition + maxView) && index < maxIndex)
	{
		record = DmQueryRecord(_dbP, index);
		game = (GameInfoType *)MemHandleLock(record);
		
		// text box
		RctSetRectangle(&rField, rArea.topLeft.x, (rArea.topLeft.y + 12 * (index - gPrefs->listPosition)), rArea.extent.x, sknInfoListItemSize);
		WinSetClip(&rField);

		if (game->selected) {
			WinSetDrawMode(winPaint);
			WinSetForeColor(bkgColor);
			WinDrawRectangle(&rField,0);
			txtColor = selColor;
		}
		else
			txtColor = norColor;

		// clipping
		rField.topLeft.x += 2;
		rField.extent.x -= 4;
		WinSetClip(&rField);
		// draw text mask
		WinSetTextColor(255);
		WinSetDrawMode(winMask);
		WinPaintChars(game->nameP, StrLen(game->nameP), rField.topLeft.x, rField.topLeft.y);
		// draw text
		if (txtColor) {
			WinSetTextColor(txtColor);
			WinSetDrawMode(winOverlay);
			WinPaintChars(game->nameP, StrLen(game->nameP), rField.topLeft.x, rField.topLeft.y);
		}
		MemHandleUnlock(record);
		index++;
	}

	RctSetRectangle(&rArea,0,0,160,160);
	WinSetClip(&rArea);
	WinScreenUnlock();
}

static Boolean ArwProcessAction(UInt16 button)
{
	Boolean handled = false;

	switch (button) {
		case skinSliderUpArrow:
			if (gPrefs->listPosition > 0) {
				gPrefs->listPosition--;
				GamUpdateList();
			}
			handled = true;
			break;

		case skinSliderDownArrow:
			RectangleType rArea;
			UInt16 maxView;

			GamGetListBounds(&rArea, 0);
			maxView = rArea.extent.y / sknInfoListItemSize;

			if (gPrefs->listPosition < DmNumRecords(_dbP)-maxView) {
				gPrefs->listPosition++;
				GamUpdateList();
			}
			handled = true;
			break;
	}

	return handled;
}

static void GamSelect(Coord x, Coord y) {
	RectangleType rArea;
	UInt16 x1,y1,x2,y2;

	GamGetListBounds(&rArea,0);
	x1 = rArea.topLeft.x;
	x2 = rArea.topLeft.x + rArea.extent.x - 1;
	y1 = rArea.topLeft.y;
	y2 = rArea.topLeft.y + rArea.extent.y - 1;

	if (y >= y1 && y <= y2 && x >= x1 && x <= x2) {
		UInt16 index;
		MemHandle record;
		GameInfoType *game;
		UInt16 oldIndex;

		index = (y - y1) / sknInfoListItemSize + gPrefs->listPosition;

		if (index == _lstIndex)
			return;

		if (index < DmNumRecords(_dbP))
		{
			GameInfoType modGame;

			oldIndex = GamGetSelected();
			record = DmGetRecord(_dbP, index);
			game = (GameInfoType *)MemHandleLock(record);

			MemMove(&modGame, game, sizeof(GameInfoType));	
			modGame.selected = !modGame.selected;
			DmWrite(game, 0, &modGame, sizeof(GameInfoType));

			MemHandleUnlock(record);
			DmReleaseRecord (_dbP, index, 0);

			if (oldIndex != index && oldIndex != dmMaxRecordIndex)	{
				record = DmGetRecord(_dbP, oldIndex);
				game = (GameInfoType *)MemHandleLock(record);

				MemMove(&modGame, game, sizeof(GameInfoType));	
				modGame.selected = false;
				DmWrite(game, 0, &modGame, sizeof(GameInfoType));

				MemHandleUnlock(record);
				DmReleaseRecord (_dbP, oldIndex, 0);
			}
			
			_lstIndex = index;
			GamUpdateList();
		}
	}
}

static void EditGameFormDelete(Boolean direct) {
	UInt16 index = GamGetSelected();

	if (index == dmMaxRecordIndex) {
		FrmCustomAlert(FrmWarnAlert, "Select an entry first.",0,0);
		return;

	} else if (FrmAlert(FrmDeleteAlert) == 0) {
		DmRemoveRecord(_dbP, index);
		if (!direct)
			FrmReturnToForm(MainForm);
		GamSortList();
		GamUpdateList();
	}
}

static void FldTrimText(FieldType * fldP) {
	MemHandle tmpH;
	Char *tmpP;

	tmpH = FldGetTextHandle(fldP);
	FldSetTextHandle(fldP, NULL);
	tmpP = (Char *)MemHandleLock(tmpH);
	TxtGlueStripSpaces(tmpP, true, true);
	MemHandleUnlock(tmpH);
	FldSetTextHandle(fldP, tmpH);
}


/***********************************************************************
 *
 * FUNCTION:    EditGameFormSave
 * FUNCTION:    EditGameFormInit
 * FUNCTION:    EditGameFormHandleEvent
 *
 * DESCRIPTION: 
 *
 * REVISION HISTORY:
 *
 *
 ***********************************************************************/

static void EditGameFormSave(UInt16 index) {

	FieldType *fld1P, *fld2P, *fld3P, *fld4P, *fld5P, *fld6P;	// need to change this with good names
	ControlType *cck1P, *cck2P, *cck3P, *cck4P, *cck5P;	
	ListType *listP;
	FormPtr frmP;

	MemHandle recordH;
	GameInfoType *gameInfo, newGameInfo;
//	UInt16 index;

	listP = (ListType *)GetObjectPtr(EditGameGfxListList);
	fld1P = (FieldType *)GetObjectPtr(EditGameEntryNameField);
	fld2P = (FieldType *)GetObjectPtr(EditGamePathField);
	fld3P = (FieldType *)GetObjectPtr(EditGameGameField);
	fld4P = (FieldType *)GetObjectPtr(EditGameLoadSlotField);
	fld5P = (FieldType *)GetObjectPtr(EditGameStartRoomField);
	fld6P = (FieldType *)GetObjectPtr(EditGameTalkSpeedField);
	
	cck1P = (ControlType *)GetObjectPtr(EditGameLoadSlotCheckbox);
	cck2P = (ControlType *)GetObjectPtr(EditGameStartRoomCheckbox);
	cck3P = (ControlType *)GetObjectPtr(EditGameAmigaCheckbox);
	cck4P = (ControlType *)GetObjectPtr(EditGameSubtitlesCheckbox);
	cck5P = (ControlType *)GetObjectPtr(EditGameTalkSpeedCheckbox);

	frmP = FrmGetActiveForm();

	FldTrimText(fld1P);
	FldTrimText(fld2P);
	FldTrimText(fld3P);

	if (FldGetTextLength(fld1P) == 0) {
		FrmCustomAlert(FrmWarnAlert,"You must specified an entry name.",0,0);
		FrmSetFocus(frmP, FrmGetObjectIndex(frmP, EditGameEntryNameField));
		return;
	} else if (FldGetTextLength(fld2P) == 0) {
		FrmCustomAlert(FrmWarnAlert,"You must specified a path.",0,0);
		FrmSetFocus(frmP, FrmGetObjectIndex(frmP, EditGamePathField));
		return;
	} else if (FldGetTextLength(fld3P) == 0) {
		FrmCustomAlert(FrmWarnAlert,"You must specified a game.",0,0);
		FrmSetFocus(frmP, FrmGetObjectIndex(frmP, EditGameGameField));
		return;
	} else if (FldGetTextLength(fld5P) == 0 && CtlGetValue(cck2P) == 1) {
		FrmCustomAlert(FrmWarnAlert,"You must specified a room number.",0,0);
		FrmSetFocus(frmP, FrmGetObjectIndex(frmP, EditGameStartRoomField));
		return;
	} else if (FldGetTextLength(fld6P) == 0 && CtlGetValue(cck5P) == 1) {
		FrmCustomAlert(FrmWarnAlert,"You must specified a talk speed.",0,0);
		FrmSetFocus(frmP, FrmGetObjectIndex(frmP, EditGameTalkSpeedField));
		return;
	}

	if (FldGetTextPtr(fld2P)[FldGetTextLength(fld2P)-1] != '/') {
		FldGetTextPtr(fld2P)[FldGetTextLength(fld2P)+0] = '/';
		FldGetTextPtr(fld2P)[FldGetTextLength(fld2P)+1] = 0;
	}

//	index = GamGetSelected();

	if (index != dmMaxRecordIndex) {
		recordH = DmGetRecord(_dbP, index);
	} else {
		index = dmMaxRecordIndex;
		GamUnselect();
		recordH = DmNewRecord(_dbP, &index, sizeof(GameInfoType));
	}

	gameInfo = (GameInfoType *)MemHandleLock(recordH);

	StrCopy(newGameInfo.nameP, FldGetTextPtr(fld1P));
	StrCopy(newGameInfo.pathP, FldGetTextPtr(fld2P));
	StrCopy(newGameInfo.gameP, FldGetTextPtr(fld3P));
	newGameInfo.gfxMode = LstGetSelection(listP);
	newGameInfo.selected = true;
	newGameInfo.autoLoad = CtlGetValue(cck1P);
	newGameInfo.loadSlot = StrAToI(FldGetTextPtr(fld4P));
	newGameInfo.autoRoom = CtlGetValue(cck2P);
	newGameInfo.roomNum = StrAToI(FldGetTextPtr(fld5P));
	newGameInfo.amiga = CtlGetValue(cck3P);
	newGameInfo.subtitles = !(CtlGetValue(cck4P));
	newGameInfo.talkValue = StrAToI(FldGetTextPtr(fld6P));
	newGameInfo.talkSpeed = CtlGetValue(cck5P);

	DmWrite(gameInfo, 0, &newGameInfo, sizeof(GameInfoType));

	MemHandleUnlock(recordH);
	DmReleaseRecord (_dbP, index, 0);
	GamSortList();
	// update list position
	{
		RectangleType rArea;
		UInt16 posIndex, maxIndex, maxView;
		
		// get the sorted index
		index = GamGetSelected();
		// if new item is out of the list bounds, change current list pos
		GamGetListBounds(&rArea, NULL);
		maxView = rArea.extent.y / sknInfoListItemSize;
		posIndex = gPrefs->listPosition;
		maxIndex = DmNumRecords(_dbP);
		
		if (index == 0 && posIndex > 0) {
			gPrefs->listPosition = 0;
		} else if ((maxView + posIndex) <= index) {
			posIndex = index - (maxView - posIndex) + 1;
			gPrefs->listPosition = posIndex;
		}
	}

	FrmReturnToForm (MainForm);
	GamUpdateList();
}

static void EditGameFormInit(UInt16 index) {

	FieldType *fld1P, *fld2P, *fld3P, *fld4P, *fld5P, *fld6P;
	FormPtr frmP;
	ListType *listP;

	Char *nameP, *pathP, *gameP, *loadP, *roomP, *talkP;
	MemHandle nameH, pathH, gameH, loadH, roomH, talkH;

	MemHandle recordH = NULL;
	GameInfoType *game;

	listP = (ListType *)GetObjectPtr(EditGameGfxListList);
	fld1P = (FieldType *)GetObjectPtr(EditGameEntryNameField);
	fld2P = (FieldType *)GetObjectPtr(EditGamePathField);
	fld3P = (FieldType *)GetObjectPtr(EditGameGameField);
	fld4P = (FieldType *)GetObjectPtr(EditGameLoadSlotField);
	fld5P = (FieldType *)GetObjectPtr(EditGameStartRoomField);
	fld6P = (FieldType *)GetObjectPtr(EditGameTalkSpeedField);

	nameH = MemHandleNew(FldGetMaxChars(fld1P));
	pathH = MemHandleNew(FldGetMaxChars(fld2P));
	gameH = MemHandleNew(FldGetMaxChars(fld3P));
	loadH = MemHandleNew(FldGetMaxChars(fld4P));
	roomH = MemHandleNew(FldGetMaxChars(fld5P));
	talkH = MemHandleNew(FldGetMaxChars(fld6P));

	nameP = (Char *)MemHandleLock(nameH);
	pathP = (Char *)MemHandleLock(pathH);
	gameP = (Char *)MemHandleLock(gameH);
	loadP = (Char *)MemHandleLock(loadH);
	roomP = (Char *)MemHandleLock(roomH);
	talkP = (Char *)MemHandleLock(talkH);

//	index = GamGetSelected();

	if (index != dmMaxRecordIndex) {
		recordH = DmQueryRecord(_dbP, index);
		game = (GameInfoType *)MemHandleLock(recordH);
		StrCopy(nameP, game->nameP);
		StrCopy(pathP, game->pathP);
		StrCopy(gameP, game->gameP);

		LstSetSelection(listP, game->gfxMode);
		
		StrIToA(loadP, game->loadSlot);
		StrIToA(roomP, game->roomNum);
		StrIToA(talkP, game->talkValue);

		CtlSetValue((ControlType *)GetObjectPtr(EditGameLoadSlotCheckbox), game->autoLoad);
		CtlSetValue((ControlType *)GetObjectPtr(EditGameStartRoomCheckbox), game->autoRoom);
		CtlSetValue((ControlType *)GetObjectPtr(EditGameAmigaCheckbox), game->amiga);
		CtlSetValue((ControlType *)GetObjectPtr(EditGameSubtitlesCheckbox), !game->subtitles);
		CtlSetValue((ControlType *)GetObjectPtr(EditGameTalkSpeedCheckbox), game->talkSpeed);
		
		MemHandleUnlock(recordH);
		CtlSetUsable((ControlType *)GetObjectPtr(EditGameDeleteButton),true);
	}
	else {
		MemSet(nameP,MemHandleSize(nameH),0);
		MemSet(pathP,MemHandleSize(pathH),0);
		MemSet(gameP,MemHandleSize(gameH),0);

		StrIToA(loadP, 0);
		StrIToA(roomP, 0);
		StrIToA(talkP, 60);

		CtlSetValue((ControlType *)GetObjectPtr(EditGameLoadSlotCheckbox), 0);
		CtlSetValue((ControlType *)GetObjectPtr(EditGameStartRoomCheckbox), 0);
		CtlSetValue((ControlType *)GetObjectPtr(EditGameAmigaCheckbox), 0);
		CtlSetValue((ControlType *)GetObjectPtr(EditGameSubtitlesCheckbox), 0);
		CtlSetValue((ControlType *)GetObjectPtr(EditGameTalkSpeedCheckbox), 0);

		LstSetSelection(listP, 1);	
		CtlSetUsable((ControlType *)GetObjectPtr(EditGameDeleteButton),false);
	}

	MemHandleUnlock(nameH);
	MemHandleUnlock(pathH);
	MemHandleUnlock(gameH);
	MemHandleUnlock(loadH);
	MemHandleUnlock(roomH);
	MemHandleUnlock(talkH);

	FldSetTextHandle(fld1P, nameH);
	FldSetTextHandle(fld2P, pathH);
	FldSetTextHandle(fld3P, gameH);
	FldSetTextHandle(fld4P, loadH);
	FldSetTextHandle(fld5P, roomH);
	FldSetTextHandle(fld6P, talkH);

	CtlSetLabel((ControlType *)GetObjectPtr(EditGameGfxPopupPopTrigger), LstGetSelectionText(listP, LstGetSelection(listP)));

	frmP = FrmGetActiveForm();
	FrmDrawForm(frmP);
}

static Boolean EditGameFormHandleEvent(EventPtr eventP)
{
	Boolean handled = false;

	switch (eventP->eType) {
		case frmOpenEvent:
			switch (__editMode__) {
				case edtModeAdd:
					EditGameFormInit(dmMaxRecordIndex);
					break;
				case edtModeEdit:
				case edtModeParams:
				default :
					EditGameFormInit(GamGetSelected());
					break;
			}
			handled = true;
			break;

		case ctlSelectEvent:
			switch (eventP->data.ctlSelect.controlID)
			{
				case EditGameOKButton:
					switch (__editMode__) {
						case edtModeAdd:
							EditGameFormSave(dmMaxRecordIndex);
							break;
						case edtModeEdit:
						case edtModeParams:
						default :
							EditGameFormSave(GamGetSelected());
							break;
					}
					break;

				case EditGameCancelButton:
					FrmReturnToForm(MainForm);
					break;
				
				case EditGameDeleteButton:
					EditGameFormDelete(false);
					break;

				case EditGameGfxPopupPopTrigger:
					FrmList(eventP, EditGameGfxListList);
					break;
			}
			handled = true;
			break;

		default:
			break;
	}
	
	return handled;
}

/***********************************************************************
 *
 * FUNCTION:    VolumeFormSave
 * FUNCTION:    VolumeFormInit
 * FUNCTION:    VolumeFormHandleEvent
 *
 * DESCRIPTION: 
 *
 * REVISION HISTORY:
 *
 *
 ***********************************************************************/

static void VolumeFormSave() {
	SliderControlType *slid1P, *slid2P, *slid3P, *slid4P, *slid5P;
	
	slid1P = (SliderControlType *)GetObjectPtr(VolumeSpeakerSliderControl);
	slid2P = (SliderControlType *)GetObjectPtr(VolumeHeadphoneSliderControl);

	slid3P = (SliderControlType *)GetObjectPtr(VolumeMasterSliderControl);
	slid4P = (SliderControlType *)GetObjectPtr(VolumeMusicSliderControl);
	slid5P = (SliderControlType *)GetObjectPtr(VolumeSfxSliderControl);
	
	CtlGetSliderValues ((ControlType *)slid1P, 0, 0, 0, &gPrefs->volume.speaker) ;
	CtlGetSliderValues ((ControlType *)slid2P, 0, 0, 0, &gPrefs->volume.headphone) ;

	CtlGetSliderValues ((ControlType *)slid3P, 0, 0, 0, &gPrefs->volume.master);
	CtlGetSliderValues ((ControlType *)slid4P, 0, 0, 0, &gPrefs->volume.music);
	CtlGetSliderValues ((ControlType *)slid5P, 0, 0, 0, &gPrefs->volume.sfx);
	
	FrmReturnToForm (MainForm);
}

static void VolumeFormInit() {
	SliderControlType *slid1P, *slid2P, *slid3P, *slid4P, *slid5P;
	FormPtr frmP;
	UInt16 value;
	
	slid1P = (SliderControlType *)GetObjectPtr(VolumeSpeakerSliderControl);
	slid2P = (SliderControlType *)GetObjectPtr(VolumeHeadphoneSliderControl);

	slid3P = (SliderControlType *)GetObjectPtr(VolumeMasterSliderControl);
	slid4P = (SliderControlType *)GetObjectPtr(VolumeMusicSliderControl);
	slid5P = (SliderControlType *)GetObjectPtr(VolumeSfxSliderControl);

	value = gPrefs->volume.speaker;
	CtlSetSliderValues ((ControlType *)slid1P, 0, 0, 0, &value);
	value = gPrefs->volume.headphone;
	CtlSetSliderValues ((ControlType *)slid2P, 0, 0, 0, &value);

	value = gPrefs->volume.master;
	CtlSetSliderValues ((ControlType *)slid3P, 0, 0, 0, &value);
	value = gPrefs->volume.music;
	CtlSetSliderValues ((ControlType *)slid4P, 0, 0, 0, &value);
	value = gPrefs->volume.sfx;
	CtlSetSliderValues ((ControlType *)slid5P, 0, 0, 0, &value);

	frmP = FrmGetActiveForm();
	FrmDrawForm(frmP);
}

static Boolean VolumeFormHandleEvent(EventPtr eventP) {
	Boolean handled = false;

	switch (eventP->eType) {
		case frmOpenEvent:
			VolumeFormInit();
			handled = true;
			break;

		case ctlSelectEvent:
			switch (eventP->data.ctlSelect.controlID)
			{
				case VolumeOKButton:
					VolumeFormSave();
					break;

				case VolumeCancelButton:
					FrmReturnToForm(MainForm);
					break;
			}
			handled = true;
			break;

		default:
			break;
	}
	
	return handled;
}

static void SoundFormSave() {
	ControlType *cck1P, *cck2P;
	ListType *listP;
	FieldType *fld1P;

	fld1P = (FieldType *)GetObjectPtr(SoundTempoField);
	cck1P = (ControlType *)GetObjectPtr(SoundMusicCheckbox);
	cck2P = (ControlType *)GetObjectPtr(SoundSfxCheckbox);
	listP = (ListType *)GetObjectPtr(SoundDriverList);

	gPrefs->sound.music = CtlGetValue(cck1P);
	gPrefs->sound.sfx = CtlGetValue(cck2P);
	gPrefs->sound.driver = LstGetSelection(listP);

	FrmReturnToForm (MainForm);
}

static void SoundFormInit() {
	ControlType *cck1P, *cck2P;
	ListType *listP;
	FieldType *fld1P;
	FormPtr frmP;

	fld1P = (FieldType *)GetObjectPtr(SoundTempoField);
	cck1P = (ControlType *)GetObjectPtr(SoundMusicCheckbox);
	cck2P = (ControlType *)GetObjectPtr(SoundSfxCheckbox);
	listP = (ListType *)GetObjectPtr(SoundDriverList);

	CtlSetValue(cck1P, gPrefs->sound.music);
	CtlSetValue(cck2P, gPrefs->sound.sfx);

	LstSetSelection(listP, gPrefs->sound.driver);
	CtlSetLabel((ControlType *)GetObjectPtr(SoundDriverPopTrigger), LstGetSelectionText(listP, LstGetSelection(listP)));

	frmP = FrmGetActiveForm();
	FrmDrawForm(frmP);
}

static Boolean SoundFormHandleEvent(EventPtr eventP) {
	Boolean handled = false;

	switch (eventP->eType) {
		case frmOpenEvent:
			SoundFormInit();
			handled = true;
			break;

		case ctlSelectEvent:
			switch (eventP->data.ctlSelect.controlID)
			{
				case SoundOKButton:
					SoundFormSave();
					break;

				case SoundCancelButton:
					FrmReturnToForm(MainForm);
					break;

				case SoundDriverPopTrigger:
					FrmList(eventP, SoundDriverList);
					break;

			}
			handled = true;
			break;

		default:
			break;
	}
	
	return handled;
}
/***********************************************************************
 *
 * FUNCTION:    MiscOptionsFormSave
 * FUNCTION:    MiscOptionsFormInit
 * FUNCTION:    MiscOptionsFormHandleEvent
 *
 * DESCRIPTION: Misc. Options form functions
 *
 * REVISION HISTORY:
 *
 *
 ***********************************************************************/

static void MiscOptionsFormSave() {

	FieldType *fld1P;
	ControlType *cck1P, *cck2P, *cck3P, *cck4P;	
	FormPtr frmP;

	fld1P = (FieldType *)GetObjectPtr(MiscOptionsDebugLevelField);
	
	cck1P = (ControlType *)GetObjectPtr(MiscOptionsVibratorCheckbox);
	cck2P = (ControlType *)GetObjectPtr(MiscOptionsNoAutoOffCheckbox);
	cck3P = (ControlType *)GetObjectPtr(MiscOptionsStdPaletteCheckbox);
	cck4P = (ControlType *)GetObjectPtr(MiscOptionsDebugCheckbox);

	frmP = FrmGetActiveForm();

	if (FldGetTextLength(fld1P) == 0 && CtlGetValue(cck4P) == 1) {
		FrmCustomAlert(FrmWarnAlert,"You must specified a debug level.",0,0);
		FrmSetFocus(frmP, FrmGetObjectIndex(frmP, MiscOptionsDebugLevelField));
		return;
	}

	gPrefs->vibrator = CtlGetValue(cck1P);
	gPrefs->autoOff = !CtlGetValue(cck2P);
	gPrefs->stdPalette = CtlGetValue(cck3P);
	gPrefs->debug = CtlGetValue(cck4P);
	gPrefs->debugLevel = StrAToI(FldGetTextPtr(fld1P));
	
	FrmReturnToForm (MainForm);
}

static void MiscOptionsFormInit() {

	FieldType *fld1P;
	FormPtr frmP;

	Char *levelP;
	MemHandle levelH;

	CtlSetValue((ControlType *)GetObjectPtr(MiscOptionsVibratorCheckbox), gPrefs->vibrator);
	CtlSetValue((ControlType *)GetObjectPtr(MiscOptionsNoAutoOffCheckbox), !gPrefs->autoOff);
	CtlSetValue((ControlType *)GetObjectPtr(MiscOptionsStdPaletteCheckbox), gPrefs->stdPalette);
	CtlSetValue((ControlType *)GetObjectPtr(MiscOptionsDebugCheckbox), gPrefs->debug);

	fld1P = (FieldType *)GetObjectPtr(MiscOptionsDebugLevelField);

	levelH = MemHandleNew(FldGetMaxChars(fld1P));
	levelP = (Char *)MemHandleLock(levelH);
	StrIToA(levelP, gPrefs->debugLevel);
	MemHandleUnlock(levelH);

	FldSetTextHandle(fld1P, levelH);
	frmP = FrmGetActiveForm();
	FrmDrawForm(frmP);
}

static Boolean MiscOptionsFormHandleEvent(EventPtr eventP) {
	Boolean handled = false;

	switch (eventP->eType) {
		case frmOpenEvent:
			MiscOptionsFormInit();
			handled = true;
			break;

		case ctlSelectEvent:
			switch (eventP->data.ctlSelect.controlID)
			{
				case MiscOptionsOKButton:
					MiscOptionsFormSave();
					break;

				case MiscOptionsCancelButton:
					FrmReturnToForm(MainForm);
					break;
			}
			handled = true;
			break;

		default:
			break;
	}
	
	return handled;
}

static UInt16 parseCards(Boolean forceDisplay) {

	Err err;
	UInt16 volRefNum;
	UInt32 volIterator = vfsIteratorStart;
	UInt8 counter = 0;

	MemHandle cards = NULL;
	CardInfoType *cardsInfo;
	MemHandle items = NULL;
	Char **itemsText = NULL;
	UInt32 other = 1;
	
	while (volIterator != vfsIteratorStop) {
		err = VFSVolumeEnumerate(&volRefNum, &volIterator);

		if (!err)
		{	Char labelP[expCardInfoStringMaxLen+1];
			err = VFSVolumeGetLabel(volRefNum, labelP, expCardInfoStringMaxLen+1);

			if (!err) {
				if (StrLen(labelP) == 0) {	// if no label try to retreive card type
					VolumeInfoType volInfo;
					err = VFSVolumeInfo(volRefNum, &volInfo);
					
					if (!err) {
						ExpCardInfoType info;
						err = ExpCardInfo(volInfo.slotRefNum, &info);
						StrCopy(labelP, info.deviceClassStr);
					}
					
					if (err != errNone)	// if err default name
						StrPrintF(labelP,"Other Card %ld", other++);
				}
			
				if (!cards)
					cards = MemHandleNew(sizeof(CardInfoType));
				else
					MemHandleResize(cards, MemHandleSize(cards) + sizeof(CardInfoType));
					
				cardsInfo = (CardInfoType *)MemHandleLock(cards);
				cardsInfo[counter].volRefNum = volRefNum;
				StrCopy(cardsInfo[counter].nameP, labelP);
				MemHandleUnlock(cards);
				counter++;
			}
		}
	}

	if (counter>0) {
		cardsInfo = (CardInfoType *)MemHandleLock(cards);

		if (forceDisplay) {
			FormPtr frmP;
			ListPtr listP;
			Int16 selected = 0;
			UInt16 index;

			frmP = FrmInitForm (CardSlotForm);
			listP = (ListType *)FrmGetObjectPtr(frmP, FrmGetObjectIndex(frmP, CardSlotSlotList));

			for (index = 0; index < counter; index++)
			{
				if (!items)
					items = MemHandleNew(sizeof(Char *));
				else
					MemHandleResize(items, MemHandleSize(items) + sizeof(Char *));

				itemsText = (Char **)MemHandleLock(items);
				itemsText[index] = cardsInfo[index].nameP;
				MemHandleUnlock(items);
				
				if (cardsInfo[index].volRefNum == gPrefs->volRefNum)
					selected = index;
			}

			itemsText = (Char **)MemHandleLock(items);
			LstSetListChoices (listP, itemsText, counter);
			LstSetSelection(listP, selected);
			FrmDoDialog (frmP);
			selected = LstGetSelection(listP);
			MemHandleUnlock(items);
			MemHandleFree(items);
			FrmDeleteForm(frmP);
			
			volRefNum = cardsInfo[selected].volRefNum;

		} else {
			volRefNum = cardsInfo[0].volRefNum;	// return the first volref
		}
		
		MemHandleUnlock(cards);
		MemHandleFree(cards);

	} else if (forceDisplay) {
		FrmCustomAlert(FrmWarnAlert, "No card found.\nPlease insert a memory card.", 0, 0);
		volRefNum = sysInvalidRefNum;

	} else {
		volRefNum = sysInvalidRefNum;
	}

	FormPtr frmP = FrmGetActiveForm();

	if (volRefNum != sysInvalidRefNum) { // if found try to create folders
		VFSDirCreate(volRefNum, "/PALM");
		VFSDirCreate(volRefNum, "/PALM/Programs");
		VFSDirCreate(volRefNum, "/PALM/Programs/ScummVM");
		VFSDirCreate(volRefNum, "/PALM/Programs/ScummVM/Games");
		VFSDirCreate(volRefNum, "/PALM/Programs/ScummVM/Saved");

		if (frmP)
			FrmShowObject(frmP, FrmGetObjectIndex (frmP, MainMSBitMap));
	} else {
		if (frmP)
			FrmShowObject(frmP, FrmGetObjectIndex (frmP, MainMSNoneBitMap));
	}

	return volRefNum;
}

//#############################################################################
//#############################################################################
// Skin manager
//#############################################################################

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#define AppLastVersion	"v0.2.97"
#define AppLastYear		"2003"

static void SknApplySkin()
{
	DmOpenRef skinDBP;
	RectangleType r;
	FormPtr frmP = FrmGetActiveForm();

	WinScreenLock(winLockErase);
//	SknSetPalette();
	FrmDrawForm(frmP);

	if (gPrefs->volRefNum != sysInvalidRefNum)
		FrmShowObject(frmP, FrmGetObjectIndex (frmP, MainMSBitMap));
	else
		FrmShowObject(frmP, FrmGetObjectIndex (frmP, MainMSNoneBitMap));

	WinSetForeColor(255);
	WinSetDrawMode(winPaint);
	WinDrawLine (0, 14, 159, 14);
	WinDrawLine (0, 13, 159, 13);

	skinDBP = SknOpenSkin();
	SknGetObjectBounds(skinDBP, skinBackgroundImageTop, &r);
	SknCopyBits(skinDBP, skinBackgroundImageTop, 0, r.topLeft.x, r.topLeft.y, 0);
	SknGetObjectBounds(skinDBP, skinBackgroundImageBottom, &r);
	SknCopyBits(skinDBP, skinBackgroundImageBottom, 0, r.topLeft.x, r.topLeft.y, 0);

	for (UInt16 resID = 1100; resID <= 7000; resID += 100) {
		SknSetState(skinDBP, resID, sknStateNormal);
		SknShowObject(skinDBP, resID);
	}

	SknCloseSkin(skinDBP);
	WinScreenUnlock();
	GamUpdateList();
}

//#############################################################################
//#############################################################################

/***********************************************************************
 *
 * FUNCTION:    MainFormInit
 *
 * DESCRIPTION: This routine initializes the MainForm form.
 *
 * PARAMETERS:  frm - pointer to the MainForm form.
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *
 *
 ***********************************************************************/
static void MainFormInit()
{
	SknApplySkin();
}

/***********************************************************************
 *
 * FUNCTION:    MainFormDoCommand
 *
 * DESCRIPTION: This routine performs the menu command specified.
 *
 * PARAMETERS:  command  - menu item id
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *
 *
 ***********************************************************************/


static Int16 SkinsFormCompare(SkinInfoType *a, SkinInfoType *b, SortRecordInfoPtr, SortRecordInfoPtr, MemHandle) {
	return StrCompare(a->nameP, b->nameP);
}

static Char **itemsText = NULL;
static void  *itemsList = NULL;

static void SkinsFormInit(Boolean bDraw) {
	MemHandle skins = NULL;
	SkinInfoType *skinsInfo;
	UInt16 numSkins = 0;

	FormPtr frmP;
	ListType *listP;
	MemHandle items = NULL;
	
	DmSearchStateType stateInfo;
	UInt16 cardNo;
	LocalID dbID;

	Err errInfo;
	Char nameP[32];

	itemsText = NULL;
	
	// parse and save skins
	Err err = DmGetNextDatabaseByTypeCreator(true, &stateInfo, 'skin', appFileCreator, false, &cardNo, &dbID);
	while (!err && dbID) {
		errInfo = DmDatabaseInfo (cardNo, dbID, nameP, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
		if (!errInfo)
		{
			if (!skins)
				skins = MemHandleNew(sizeof(SkinInfoType));
			else
				MemHandleResize(skins, MemHandleSize(skins) + sizeof(SkinInfoType));
			
			skinsInfo = (SkinInfoType *)MemHandleLock(skins);
			StrCopy(skinsInfo[numSkins].nameP, nameP);
			skinsInfo[numSkins].cardNo = cardNo;
			skinsInfo[numSkins].dbID = dbID;
			MemHandleUnlock(skins);
			numSkins++;
		}		
		err = DmGetNextDatabaseByTypeCreator(false, &stateInfo, 'skin', appFileCreator, false, &cardNo, &dbID);
	}

	Int16 selected = -1;

	listP = (ListType *)GetObjectPtr(SkinsSkinList);
	skinsInfo = (SkinInfoType *)MemHandleLock(skins);
	SysQSort(skinsInfo, numSkins, sizeof(SkinInfoType), (CmpFuncPtr)SkinsFormCompare, 0);

	// create itemsText (TODO: create a custom draw function)
	for (UInt16 index=0; index < numSkins; index++)
	{
		if (!items)
			items = MemHandleNew(sizeof(Char *));
		else
			MemHandleResize(items, MemHandleSize(items) + sizeof(Char *));

		itemsText = (Char **)MemHandleLock(items);
		itemsText[index] = skinsInfo[index].nameP;
		MemHandleUnlock(items);
		
		if (	gPrefs->skin.cardNo == skinsInfo[index].cardNo &&
				gPrefs->skin.dbID == skinsInfo[index].dbID &&
				StrCompare(gPrefs->skin.nameP, skinsInfo[index].nameP) == 0)
			selected = index;
	}
	// save globals and set list
	itemsText = (Char **)MemHandleLock(items);
	itemsList = (void *)skinsInfo;
	LstSetListChoices (listP, itemsText, numSkins);
	LstSetSelection(listP, selected);

	if (bDraw) {
		frmP = FrmGetActiveForm();
		FrmDrawForm(frmP);
	} else {
		WinScreenLock(winLockCopy);
		LstDrawList(listP);
		WinScreenUnlock();
//		LstSetSelection(listP, 0);
	}
}



static void SkinsFormExit(Boolean bSave) {
	MemHandle skins;
	MemHandle items;
	SkinInfoType *skinsInfo;

	ListType *listP;
	Int16 selected;

	listP = (ListType *)GetObjectPtr(SkinsSkinList);
	selected = LstGetSelection(listP);
	
	if (bSave && selected == -1)	{	// may never occured...
		FrmCustomAlert(FrmWarnAlert, "You didn't select a skin.", 0, 0);
		return;
	}

	skinsInfo = (SkinInfoType *)itemsList;
	skins = MemPtrRecoverHandle(skinsInfo);
	items = MemPtrRecoverHandle(itemsText);
	
	itemsText = NULL;
	itemsList = NULL;

	if (bSave) {
		StrCopy(gPrefs->skin.nameP, skinsInfo[selected].nameP);
		gPrefs->skin.cardNo = skinsInfo[selected].cardNo;
		gPrefs->skin.dbID =  skinsInfo[selected].dbID;
	}

	FrmReturnToForm (MainForm);

	MemHandleUnlock(items);
	MemHandleUnlock(skins);
	MemHandleFree(items);
	MemHandleFree(skins);

	if (bSave)
		SknApplySkin();
}

static void SkinsFormBeam() {
	SkinInfoType *skinsInfo;

	ListType *listP;
	Int16 selected;
	Err err;

	listP = (ListType *)GetObjectPtr(SkinsSkinList);
	selected = LstGetSelection(listP);

	if (selected == -1)	{	// may never occured...
		FrmCustomAlert(FrmWarnAlert, "You didn't select a skin.", 0, 0);
		return;
	}

	skinsInfo = (SkinInfoType *)itemsList;
	err = SendDatabase(0, skinsInfo[selected].dbID, "skin.pdb", "\nScummVM Skin");

//	if (err)
//		FrmCustomAlert(FrmErrorAlert, "Unable to beam this skin.",0,0);
}

static void SkinsFormDelete() {
	MemHandle skins;
	MemHandle items;
	SkinInfoType *skinsInfo;

	ListType *listP;
	Int16 selected;

	listP = (ListType *)GetObjectPtr(SkinsSkinList);
	selected = LstGetSelection(listP);

	if (selected == -1)	{	// may never occured...
		FrmCustomAlert(FrmInfoAlert, "You didn't select a skin.", 0, 0);
		return;
	}

	skinsInfo = (SkinInfoType *)itemsList;
	skins = MemPtrRecoverHandle(skinsInfo);
	items = MemPtrRecoverHandle(itemsText);

	if (	gPrefs->skin.cardNo == skinsInfo[selected].cardNo &&
			gPrefs->skin.dbID == skinsInfo[selected].dbID &&
			StrCompare(gPrefs->skin.nameP, skinsInfo[selected].nameP) == 0) {
			FrmCustomAlert(FrmInfoAlert, "You cannot delete the active skin.",0,0);
			return;

	} else {
		Err err = DmDeleteDatabase(0, skinsInfo[selected].dbID);
		if (!err) {

			itemsText = NULL;
			itemsList = NULL;

			MemHandleUnlock(items);
			MemHandleUnlock(skins);
			MemHandleFree(items);
			MemHandleFree(skins);

			SkinsFormInit(false);
		} else {
			FrmCustomAlert(FrmErrorAlert, "Skin deletion failed.",0,0);
		}
	}

}
static Boolean SkinsFormHandleEvent(EventPtr eventP) {
	Boolean handled = false;

	switch (eventP->eType) {
	
		case frmOpenEvent:
			SkinsFormInit(true);
			handled = true;
			break;

		case frmCloseEvent:
			SkinsFormExit(false);
			handled = true;
			break;

		case ctlSelectEvent:
			switch (eventP->data.ctlSelect.controlID)
			{
				case SkinsOKButton:
					SkinsFormExit(true);
					break;

				case SkinsCancelButton:
					SkinsFormExit(false);
					break;
					
				case SkinsBeamButton:
					SkinsFormBeam();
					break;
				
				case SkinsDeleteButton:
					SkinsFormDelete();
					break;
			}
			handled = true;
			break;

		default:
			break;
	}
	
	return handled;
}
/*

static void SkinsFormDoDialog() {
	MemHandle skins = NULL;
	SkinInfoType *skinsInfo;
	UInt16 numSkins = 0;

	FormPtr frmP;
	ListType *listP;
	MemHandle items = NULL;
	Char **itemsText = NULL;
	
	DmSearchStateType stateInfo;
	UInt16 cardNo;
	LocalID dbID;

	Err errInfo;
	Char nameP[32];
	
	Err err = DmGetNextDatabaseByTypeCreator(true, &stateInfo, 'skin', appFileCreator, false, &cardNo, &dbID);
	while (!err && dbID) {
		errInfo = DmDatabaseInfo (cardNo, dbID, nameP, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
		if (!errInfo)
		{
			if (!skins)
				skins = MemHandleNew(sizeof(SkinInfoType));
			else
				MemHandleResize(skins, MemHandleSize(skins) + sizeof(SkinInfoType));
			
			skinsInfo = (SkinInfoType *)MemHandleLock(skins);
			StrCopy(skinsInfo[numSkins].nameP, nameP);
			skinsInfo[numSkins].cardNo = cardNo;
			skinsInfo[numSkins].dbID = dbID;
			MemHandleUnlock(skins);
			numSkins++;
		}		
		err = DmGetNextDatabaseByTypeCreator(false, &stateInfo, 'skin', appFileCreator, false, &cardNo, &dbID);
	}

	UInt16 button;
	Int16 selected = -1;

	frmP = FrmInitForm (SkinsForm);
	listP = (ListType *)FrmGetObjectPtr(frmP, FrmGetObjectIndex(frmP, SkinsSkinList));

	skinsInfo = (SkinInfoType *)MemHandleLock(skins);
	SysQSort(skinsInfo, numSkins, sizeof(SkinInfoType), (CmpFuncPtr)SkinsFormCompare, 0);
	for (UInt16 index=0; index < numSkins; index++)
	{
		if (!items)
			items = MemHandleNew(sizeof(Char *));
		else
			MemHandleResize(items, MemHandleSize(items) + sizeof(Char *));

		itemsText = (Char **)MemHandleLock(items);
		itemsText[index] = skinsInfo[index].nameP;
		MemHandleUnlock(items);
		
		if (	gPrefs->skin.cardNo == skinsInfo[index].cardNo &&
				gPrefs->skin.dbID == skinsInfo[index].dbID &&
				StrCompare(gPrefs->skin.nameP, skinsInfo[index].nameP) == 0)
			selected = index;
	}
	itemsText = (Char **)MemHandleLock(items);
	LstSetListChoices (listP, itemsText, numSkins);
	LstSetSelection(listP, selected);
	button = FrmDoDialog (frmP);
	selected = LstGetSelection(listP);

	switch (button) {
		case SkinsOKButton:
			StrCopy(gPrefs->skin.nameP, skinsInfo[selected].nameP);
			gPrefs->skin.cardNo = skinsInfo[selected].cardNo;
			gPrefs->skin.dbID =  skinsInfo[selected].dbID;
			SknApplySkin();
			break;

		case SkinsSkinDeleteButton:
			FrmCustomAlert(FrmWarnAlert,"Not implemented !",0,0);
			break;
	}

	FrmDeleteForm (frmP);
	MemHandleUnlock(items);
	MemHandleUnlock(skins);
	MemHandleFree(items);
	MemHandleFree(skins);
}
*/
static Boolean MainFormDoCommand(UInt16 command)
{
	Boolean handled = false;
	FormPtr frmP;

	switch (command)
		{
		case MainGamesChooseaCard:
			MenuEraseStatus(0);
			gPrefs->volRefNum = parseCards(true);
			handled = true;
			break;

		case MainGamesNewEdit:
			MenuEraseStatus(0);
			__editMode__ = edtModeParams;
			FrmPopupForm(EditGameForm);
			handled = true;
			break;

		case MainGamesBeamScummVM:
			MenuEraseStatus(0);
			BeamMe();
			//if (BeamMe())
				//FrmCustomAlert(FrmErrorAlert,"Unable to beam ScummVM for PalmOS.",0,0);
			handled = true;
			break;

		case MainOptionsAbout:
			MenuEraseStatus(0);					// Clear the menu status from the display.
			frmP = FrmInitForm (AboutForm);
			FrmDoDialog (frmP);					// Display the About Box.
			FrmDeleteForm (frmP);
			handled = true;
			break;
		
		case MainOptionsVolumeControl:
			FrmPopupForm(VolumeForm);
			handled = true;
			break;

		case MainOptionsSoundPrefs:
			FrmPopupForm(SoundForm);
			handled = true;
			break;
		
		case MainOptionsSkins:
			MenuEraseStatus(0);					// Clear the menu status from the display.
			//SkinsFormDoDialog();
			FrmPopupForm(SkinsForm);
			handled = true;
			break;

		case MainOptionsMiscellaneous:			
			MenuEraseStatus(0);					// Clear the menu status from the display.
			FrmPopupForm(MiscOptionsForm);
			handled = true;
			break;
		}
	
	return handled;
}


static void SavePrefs() {
	if (gPrefs) {
		PrefSetAppPreferences(appFileCreator, appPrefID, appPrefVersionNum, gPrefs, sizeof (GlobalsPreferenceType), true);
		MemPtrFree(gPrefs);
		gPrefs = NULL;
	}
}

static Boolean CheckVibratorExists() {
	UInt32 romVersion;
	Err err;
	Boolean exists = false;

	err = FtrGet(sysFtrCreator, sysFtrNumROMVersion, &romVersion);
	if (!err) {
		if (romVersion >= sysMakeROMVersion(4,0,0,sysROMStageRelease,0)) {
			Boolean active = false;
			err = HwrVibrateAttributes(0, kHwrVibrateActive, &active);
			exists = (!err) ? true : exists;
		}
	}

	return exists;
}

static void AddArg(Char **argvP, const Char *argP, const Char *parmP, UInt8 *countArgP)
{
	if (argP)
	{
		UInt16 len2 = 0;
		UInt16 len1 = StrLen(argP);

		if (len1 > 0)
		{	
			if (parmP)
				len2 = StrLen(parmP);

			(*countArgP)++;
			*argvP = (Char *)MemPtrNew(len1 + len2 + 1);
			StrCopy(*argvP, argP);
			
			if (parmP)
				StrCat(*argvP, parmP);
		}
	}
}

// need to move this on a .h file
#define sonySysFileCSystem      'SsYs'  /* Sony overall System */
#define sonySysFtrCreatorSystem     sonySysFileCSystem

#define sonySysFtrNumSystemBase         10000
#define sonySysFtrNumSystemAOutSndStateOnHandlerP   (sonySysFtrNumSystemBase + 4)
#define sonySysFtrNumSystemAOutSndStateOffHandlerP  (sonySysFtrNumSystemBase + 5)

typedef void (*sndStateOnType)(UInt8 /* kind */, UInt8 /* L volume 0-31 */, UInt8 /* R volume 0-31 */);
typedef void (*sndStateOffType)(UInt8 /* kind */);

/* kind */
#define aOutSndKindSp       (0) /* Speaker volume */
#define aOutSndKindHp       (2) /* HeadPhone volume */
////////////////////////////////////////////////////////////
#define MAX_ARG	20
static void StartScummVM()
{
	Char *argvP[MAX_ARG];
	UInt8 argc	= 0;
	UInt8 count;

	Boolean autoOff;
	UInt16 autoOffDelay;
	Boolean debug;
	UInt16 musicDriver = sysInvalidRefNum; // for launch call

	UInt16 index = GamGetSelected();
/*	
	if (index == dmMaxRecordIndex) {
		// TODO : enable "Continue anyway ?" to use ScummVM selector
		FrmCustomAlert(FrmWarnAlert,"Error : No game was specified !",0,0);
		return;
	}
*/
	for(count = 0; count < MAX_ARG; count++)
		argvP[count] = 0;

	if (index != dmMaxRecordIndex) {
		Char pathP[256];
		Char num[4];
		MemHandle recordH;
		GameInfoType *gameInfoP;

		recordH = DmQueryRecord(_dbP,index);
		gameInfoP = (GameInfoType *)MemHandleLock(recordH);

		// check path
		StrCopy(pathP,"/Palm/Programs/ScummVM/Games/");
		if (gameInfoP->pathP[0]=='/')
			StrCopy(pathP,gameInfoP->pathP);
		else
			StrCat(pathP,gameInfoP->pathP);
	/*	{
			FileRef tmpRef;
			// TODO : enable empty path -> /Palm/Programs/ScummVM/Games/ as default
			if (VFSFileOpen(gPrefs->volRefNum, pathP, vfsModeRead, &tmpRef) != errNone) {
				MemHandleUnlock(recordH);
				FrmCustomAlert(FrmErrorAlert,"The specified path was not found !",0,0);
				return;
			} else {
				VFSFileClose(tmpRef);
			}
		}
	*/
		AddArg(&argvP[argc], "ScummVM", NULL, &argc);
	//	AddArg(&argvP[argc], "-w", NULL, &argc);

		// path
		AddArg(&argvP[argc], "-p", pathP, &argc);

		// gfx mode
		gVars->flipping.pageAddr1 = (UInt8 *)(BmpGetBits(WinGetBitmap(WinGetDisplayWindow())));
		gVars->flipping.pageAddr2 = gVars->flipping.pageAddr1; // default if not flipping mode
		switch (gameInfoP->gfxMode)
		{
			case 1:
				AddArg(&argvP[argc], "-g", "flipping", &argc);
				gVars->flipping.pageAddr1 = (UInt8 *)WinScreenLock(winLockErase);
				WinScreenUnlock();
				break;
			case 2:
				AddArg(&argvP[argc], "-g", "dbuffer", &argc);
				break;
			default:
				AddArg(&argvP[argc], "-g", "normal", &argc);
				break;
		}

		// load state
		if (gameInfoP->autoLoad) {
			StrIToA(num, gameInfoP->loadSlot);
			AddArg(&argvP[argc], "-x", num, &argc);
		}
		// start in room
		if (gameInfoP->autoRoom) {
			StrIToA(num, gameInfoP->roomNum);
			AddArg(&argvP[argc], "-b", num, &argc);
		}
		// amiga palette
		if (gameInfoP->amiga) {
			AddArg(&argvP[argc], "-a", NULL, &argc);
		}
		// subtitles
		if (!gameInfoP->subtitles) {
			AddArg(&argvP[argc], "-n", NULL, &argc);
		}
		// talk speed
		if (gameInfoP->talkSpeed) {
			StrIToA(num, gameInfoP->talkValue);
			AddArg(&argvP[argc], "-y", num, &argc);
		}
		// debug level
		debug = gPrefs->debug;
		if (gPrefs->debug) {
			StrIToA(num, gPrefs->debugLevel);
			AddArg(&argvP[argc], "-d", num, &argc);
		}
		// music driver
		musicDriver = gPrefs->sound.music;
		if (musicDriver) {
			switch (gPrefs->sound.driver) {
				case 0:	// NULL
					AddArg(&argvP[argc], "-e", "null", &argc);
					break;
				case 1:	// yam	ha Pa1
					AddArg(&argvP[argc], "-e", "ypa1", &argc);
					break;
			}		
		}
		else	// NULL as default
			AddArg(&argvP[argc], "-e", "null", &argc);

		// volume control
		StrIToA(num, gPrefs->volume.master);
		AddArg(&argvP[argc], "-o", num, &argc);
		StrIToA(num, gPrefs->volume.sfx);
		AddArg(&argvP[argc], "-s", num, &argc);
		StrIToA(num, gPrefs->volume.music);
		AddArg(&argvP[argc], "-m", num, &argc);

		// game name
		AddArg(&argvP[argc], gameInfoP->gameP, NULL, &argc);

		MemHandleUnlock(recordH);
	}

	GamCloseDatabase();
	FrmCloseAllForms();

	autoOff = gPrefs->autoOff;
	if (!autoOff) {
		autoOffDelay = SysSetAutoOffTime(0);
		EvtResetAutoOffTimer();
	}

	// gVars values
	//gVars->HRrefNum defined in checkHRmode on Clié OS4
	//gVars->logFile defined bellow, must be defined only if debug option is checked
	gVars->screenLocked = false;
	gVars->volRefNum = gPrefs->volRefNum;
	gVars->vibrator = gPrefs->vibrator;
	gVars->stdPalette = gPrefs->stdPalette;
//	gVars->volume.speaker = gPrefs->volume.speaker;
//	gVars->volume.headphone = gPrefs->volume.headphone;

	if (gVars->vibrator)
	{
		if (CheckVibratorExists()) {
			UInt16 cycle	= (SysTicksPerSecond())/2;
			UInt32 pattern	= 0xFF000000;
			UInt16 delay	= 1;
			UInt16 repeat	= 1;

			HwrVibrateAttributes(1, kHwrVibrateRate, &cycle);
			HwrVibrateAttributes(1, kHwrVibratePattern, &pattern);
			HwrVibrateAttributes(1, kHwrVibrateDelay, &delay);
			HwrVibrateAttributes(1, kHwrVibrateRepeatCount, &repeat);

		} else {
			gVars->vibrator = false;
		}
	}

	if (debug) {
		VFSFileDelete(gVars->volRefNum,"PALM/Programs/ScummVM/scumm.log");
		VFSFileCreate(gVars->volRefNum,"PALM/Programs/ScummVM/scumm.log");
		VFSFileOpen(gVars->volRefNum,"PALM/Programs/ScummVM/scumm.log",vfsModeWrite, &gVars->logFile);
	}

	void *sndStateOnFuncP = 0, *sndStateOffFuncP = 0;

	if (musicDriver == 1 || musicDriver == sysInvalidRefNum) {

		Pa1Lib_Open();

		FtrGet(sonySysFtrCreatorSystem, sonySysFtrNumSystemAOutSndStateOnHandlerP, (UInt32*) &sndStateOnFuncP);
		FtrGet(sonySysFtrCreatorSystem, sonySysFtrNumSystemAOutSndStateOffHandlerP, (UInt32*) &sndStateOffFuncP);

		if (sndStateOnFuncP && sndStateOffFuncP) {
			((sndStateOnType)(sndStateOnFuncP))(aOutSndKindSp, gPrefs->volume.headphone, gPrefs->volume.headphone);
			((sndStateOnType)(sndStateOnFuncP))(aOutSndKindHp, gPrefs->volume.speaker, gPrefs->volume.speaker);

		}

		Pa1Lib_devHpVolume(gPrefs->volume.headphone, gPrefs->volume.headphone);
		Pa1Lib_devSpVolume(gPrefs->volume.speaker);
	//	Pa1Lib_devEqVolume(gPrefs->volume.speaker);
	}
	SavePrefs();	// free globals pref memory
	GBOpen();
	GBInitAll();

//	MemExtInit();
	main(argc, argvP);
//	MemExtCleanup();

	GBReleaseAll();
	GBClose();

	if (musicDriver == 1 || musicDriver == sysInvalidRefNum) {
		if (sndStateOnFuncP && sndStateOffFuncP) {
			((sndStateOffType)(sndStateOffFuncP))(aOutSndKindSp);
			((sndStateOffType)(sndStateOffFuncP))(aOutSndKindHp);
		}
		Pa1Lib_Close();
	}

	if (debug)
		VFSFileClose(gVars->logFile);

	for(count = 0; count < MAX_ARG; count++)
		if (argvP[count])
			MemPtrFree(argvP[count]);
	
	if (gVars->vibrator) {
		Boolean active = false;
		HwrVibrateAttributes(1, kHwrVibrateActive, &active);
	}

	if (!autoOff) {
		SysSetAutoOffTime(autoOffDelay);SystemPreferencesChoice
		EvtResetAutoOffTimer();
	}
}

void PalmFatalError(const Char *err)
{
	WinPalette(winPaletteSetToDefault,0,0,0);

	if (gVars->screenLocked)
		WinScreenUnlock();

//	MemExtCleanup();
	WinEraseWindow();
	FrmCustomAlert(FrmFatalErrorAlert, err, 0,0);
	SysReset();
}

/*
void DrawBitmap (DmResID resID, Coord x, Coord y, WinDrawOperation newMode)
{
	MemHandle hTemp;
	BitmapType* bmTemp;
	WinDrawOperation oldMode;

	hTemp	= DmGetResource(bitmapRsc,resID);
	bmTemp	= MemHandleLock(hTemp);

	oldMode	= WinSetDrawMode(newMode);
	WinPaintBitmap(bmTemp, x, y);
	WinSetDrawMode(oldMode);

	MemHandleUnlock(hTemp);
	DmReleaseResource((MemPtr)bmTemp);
}
*/

/***********************************************************************
 *
 * FUNCTION:    MainFormHandleEvent
 *
 * DESCRIPTION: This routine is the event handler for the 
 *              "MainForm" of this application.
 *
 * PARAMETERS:  eventP  - a pointer to an EventType structure
 *
 * RETURNED:    true if the event has handle and should not be passed
 *              to a higher level handler.
 *
 * REVISION HISTORY:
 *
 *
 ***********************************************************************/
/*
static void CheckCardPresent() {

	if (gVolRefNum) {
		Err err;
		VolumeInfoType volInfo;
		err = VFSVolumeInfo(gVolRefNum, &volInfo);

		if (!err) {
			err = ExpCardPresent(volInfo.slotRefNum);

			if (err != errNone && err != expErrInvalidSlotRefNum) // expErrInvalidSlotRefNum -> error on palmSim with hostFS
				gVolRefNum = 0;
		}
	}

	if (!gVolRefNum) {
		FormPtr frmP = FrmGetActiveForm();
		
		if (frmP)
			FrmHideObject(frmP, FrmGetObjectIndex (frmP, MainMSBitMap));
	}
}
*/
static Boolean penDownRepeat() {
	Coord x,y;
	Boolean penDown, handled = false;
	EvtGetPen(&x, &y, &penDown);

	if (penDown && sknLastOn) {
		RectangleType r;
		DmOpenRef skinDBP;
	
		skinDBP = SknOpenSkin();
		SknGetObjectBounds(skinDBP, sknLastOn, &r);

		if (RctPtInRectangle(x*2, y*2, &r)) {
			if (SknGetState(skinDBP, sknLastOn) != sknStateSelected) {
				SknSetState(skinDBP, sknLastOn, sknStateSelected);
				SknShowObject(skinDBP, sknLastOn);
			}
			
			switch (sknLastOn) {
				case skinSliderUpArrow:
				case skinSliderDownArrow:
					handled = ArwProcessAction(sknLastOn);
					break;
			}
			
		} else {
			if (SknGetState(skinDBP, sknLastOn) != sknStateNormal) {
				SknSetState(skinDBP, sknLastOn, sknStateNormal);
				SknShowObject(skinDBP, sknLastOn);
			}
		}

		SknCloseSkin(skinDBP);
	}
	
	return handled;
}

static Boolean MainFormHandleEvent(EventPtr eventP)
{
	Boolean handled = false;
	FormPtr frmP;
	Coord x,y;
	DmOpenRef skinDBP;
	
	switch (eventP->eType) {
		case menuEvent:
			return MainFormDoCommand(eventP->data.menu.itemID);

		case frmOpenEvent:
			MainFormInit();
			handled = true;
			break;

		case ctlSelectEvent:
			switch (eventP->data.ctlSelect.controlID)
			{
				case MainCardsButton:
					gPrefs->volRefNum = parseCards(true);
					break;
			
				case MainAboutButton:
					frmP = FrmInitForm (AboutForm);
					FrmDoDialog (frmP);					// Display the About Box.
					FrmDeleteForm (frmP);
					break;
				
//				case MainListTypeSelTrigger:
//					FrmList(eventP, MainListTypeList);
//					break;
			}
			handled = true;
			break;

		case frmUpdateEvent:
			int a= 0;
			// To do any custom drawing here, first call FrmDrawForm(), then do your
			// drawing, and then set handled to true.
			break;

		case penUpEvent:
			x = eventP->screenX;
			y = eventP->screenY;

			_lstIndex = dmMaxRecordIndex;
			
			if (sknLastOn != skinButtonNone) {
				RectangleType r;
				skinDBP = SknOpenSkin();
				SknGetObjectBounds(skinDBP, sknLastOn, &r);
				SknSetState(skinDBP, sknLastOn, sknStateNormal);
				SknShowObject(skinDBP, sknLastOn);
				SknCloseSkin(skinDBP);
				
				if (RctPtInRectangle(x*2, y*2, &r)) {
					switch (sknLastOn) {
						case skinButtonGameAdd:
							__editMode__ = edtModeAdd;
							FrmPopupForm(EditGameForm);
							handled = true;
							break;

						case skinButtonGameEdit:
						case skinButtonGameParams:
							__editMode__ = edtModeParams;
							FrmPopupForm(EditGameForm);
							handled = true;
							break;

						case skinButtonGameStart:
							if (gPrefs->volRefNum == sysInvalidRefNum)
								FrmCustomAlert(FrmWarnAlert,"Please select/insert a memory card.", 0, 0);
							else
								StartScummVM();
							handled = true;
							break;

						case skinButtonGameDelete:
							EditGameFormDelete(true);
							break;
					}
				}
				sknLastOn = skinButtonNone;
			}
			break;

		case penDownEvent:
		case penMoveEvent:
			if (sknLastOn == skinButtonNone) {
				x = eventP->screenX;
				y = eventP->screenY;
				skinDBP = SknOpenSkin();

				switch (sknLastOn = SknCheckClick(skinDBP, x,y)) {
					case skinButtonNone:
						break;
					case skinSliderUpArrow:
					case skinSliderDownArrow:
					case skinButtonGameAdd:
					case skinButtonGameEdit:
					case skinButtonGameParams:
					case skinButtonGameStart:
					case skinButtonGameDelete:
						SknSetState(skinDBP, sknLastOn, sknStateSelected);
						SknShowObject(skinDBP, sknLastOn);
						handled = true;
						break;
					default:
						FrmCustomAlert(FrmWarnAlert,"Unknown button !",0,0);
				}
				SknCloseSkin(skinDBP);
				GamSelect(x, y);
			}
			break;
		
		default:
			handled = penDownRepeat();
	}

	return handled;
}

/***********************************************************************
 *
 * FUNCTION:    AppHandleEvent
 *
 * DESCRIPTION: This routine loads form resources and set the event
 *              handler for the form loaded.
 *
 * PARAMETERS:  event  - a pointer to an EventType structure
 *
 * RETURNED:    true if the event has handle and should not be passed
 *              to a higher level handler.
 *
 * REVISION HISTORY:
 *
 *
 ***********************************************************************/
static Boolean AppHandleEvent(EventPtr eventP)
{
	UInt16 formId;
	FormPtr frmP;

	if (eventP->eType == frmLoadEvent)
		{
		// Load the form resource.
		formId = eventP->data.frmLoad.formID;
		frmP = FrmInitForm(formId);
		FrmSetActiveForm(frmP);

		// Set the event handler for the form.  The handler of the currently
		// active form is called by FrmHandleEvent each time is receives an
		// event.
		switch (formId)
			{
			case MainForm:
				FrmSetEventHandler(frmP, MainFormHandleEvent);
				break;

			case SkinsForm:
				FrmSetEventHandler(frmP, SkinsFormHandleEvent);
				break;

			case EditGameForm:
				FrmSetEventHandler(frmP, EditGameFormHandleEvent);
				break;

			case MiscOptionsForm:
				FrmSetEventHandler(frmP, MiscOptionsFormHandleEvent);
				break;

			case VolumeForm:
				FrmSetEventHandler(frmP, VolumeFormHandleEvent);
				break;
			
			case SoundForm:
				FrmSetEventHandler(frmP, SoundFormHandleEvent);
				break;

			default:
//				ErrFatalDisplay("Invalid Form Load Event");
				break;

			}
		return true;
		}
	
	return false;
}


/***********************************************************************
 *
 * FUNCTION:    AppEventLoop
 *
 * DESCRIPTION: This routine is the event loop for the application.  
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *
 *
 ***********************************************************************/
static void AppEventLoop(void)
{
	UInt16 error;
	EventType event;

	do {
		EvtGetEvent(&event, evtNoWait);

		if (! SysHandleEvent(&event))
			if (! MenuHandleEvent(0, &event, &error))
				if (! AppHandleEvent(&event))
					FrmDispatchEvent(&event);

	} while (event.eType != appStopEvent);
}


/***********************************************************************
 *
 * FUNCTION:     AppStart
 *
 * DESCRIPTION:  Get the current application's preferences.
 *
 * PARAMETERS:   nothing
 *
 * RETURNED:     Err value 0 if nothing went wrong
 *
 * REVISION HISTORY:
 *
 *
 ***********************************************************************/
static Err AppStartCheckHRmode()
{
	SonySysFtrSysInfoP sonySysFtrSysInfoP;
	UInt32 version;
	Err error = errNone;

	// test if sonyHR is present
	if (!(error = FtrGet(sonySysFtrCreator, sonySysFtrNumSysInfoP, (UInt32*)&sonySysFtrSysInfoP)))
	{
		if (sonySysFtrSysInfoP->libr & sonySysFtrSysInfoLibrHR) {		/* HR available */

			if ((error = SysLibFind(sonySysLibNameHR, &gVars->HRrefNum))) {
				if (error == sysErrLibNotFound)							/* couldn't find lib */
					error = SysLibLoad( 'libr', sonySysFileCHRLib, &gVars->HRrefNum );
			}

			if (!error )
			{	/* Now we can use HR lib. Executes Open library. */
				error = HROpen(gVars->HRrefNum);
			}
		}
	}
	// if not Hi-Density ?
	if (error && !(error = FtrGet(sysFtrCreator, sysFtrNumWinVersion, &version)))
	{
		gVars->HRrefNum = sysInvalidRefNum;	// Not sony HR
		if (version<4)
			error = 1;
	}

	if (!error) { /* Not error processing */
		UInt32 width, height, depth;
		Boolean color;

		width = hrWidth;
		height= hrHeight;
		depth = 8;
		color = true;

		if (gVars->HRrefNum != sysInvalidRefNum)
			error = HRWinScreenMode ( gVars->HRrefNum, winScreenModeSet, &width, &height, &depth, &color );
		else
			error = WinScreenMode ( winScreenModeSet, &width, &height, &depth, &color );

		/* high-resolution mode entered from here if no error */
		if (error != errNone)
			FrmCustomAlert(FrmErrorAlert,"Your device doesn't seems to support 256 colors mode.",0,0);
	}
	else
		FrmCustomAlert(FrmErrorAlert,"This device doesn't seems to support\nHi-Res mode.",0,0);

	return error;
}

static void AppStopHRMode() {
	if (gVars->HRrefNum != sysInvalidRefNum) {
			HRClose(gVars->HRrefNum);
			//SysLibRemove(gVars->HRrefNum);
	}
}

static Err AppStartCheckNotify() {
	UInt32 romVersion;
	Err err;

	err = FtrGet(sysFtrCreator, sysFtrNumNotifyMgrVersion, &romVersion); 
	if (!err) {
		UInt16 cardNo;
		LocalID dbID;

		err = SysCurAppDatabase(&cardNo, &dbID);
		if (!err) {
			SysNotifyRegister(cardNo, dbID, sysNotifyVolumeMountedEvent, NULL, sysNotifyNormalPriority, NULL);
			SysNotifyRegister(cardNo, dbID, sysNotifyVolumeUnmountedEvent, NULL, sysNotifyNormalPriority, NULL);
		}
	}

	return err;
}

static Err AppStartLoadSkin() {
	Err err = errNone;

	// if skin defined, check if the db still exists
	if (gPrefs->skin.dbID) {
		UInt32 type, creator;

		// remember to check version for next revision of the skin
		err = DmDatabaseInfo (gPrefs->skin.cardNo, gPrefs->skin.dbID, gPrefs->skin.nameP, 0, 0, 0, 0, 0, 0, 0,0, &type, &creator);
		if (!err)
			if (type != 'skin' || creator != appFileCreator)
				err = dmErrInvalidParam;

		if (err)
			MemSet(&(gPrefs->skin),sizeof(SkinInfoType),0);
	}

	// No skin ? try to get the first one
	if (!gPrefs->skin.dbID) {
		DmSearchStateType stateInfo;

		err = DmGetNextDatabaseByTypeCreator(true, &stateInfo, 'skin', appFileCreator, false, &gPrefs->skin.cardNo, &gPrefs->skin.dbID);
		if (!err)
			err = DmDatabaseInfo (gPrefs->skin.cardNo, gPrefs->skin.dbID, gPrefs->skin.nameP, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	}

	if (err)
		FrmCustomAlert(FrmErrorAlert,"No skin found.\nPlease install a skin and restart ScummVM.",0,0);
	
	return err;
}

static Err AppStartCheckMathLib()
{
	Err error = errNone;

	if ((error = SysLibFind(MathLibName, &MathLibRef)))
		if (error == sysErrLibNotFound)									// couldn't find lib
			error = SysLibLoad( LibType, MathLibCreator, &MathLibRef);

	if (!error) {// Now we can use lib. Executes Open library.
		error = MathLibOpen(MathLibRef, MathLibVersion);
		if (error)
			FrmCustomAlert(FrmErrorAlert,"Can't open MathLib !",0,0);

	} else {
		FrmCustomAlert(FrmErrorAlert,"Can't find MathLib.\nPlease install it first.",0,0);
	}

	return error;
}

static void AppStopMathLib() {
	UInt16 useCount;

	if (MathLibRef != sysInvalidRefNum) {
		MathLibClose(MathLibRef, &useCount);
		
		if (!useCount)
			SysLibRemove(MathLibRef);
	}
}


static Err AppStart(void)
{
	UInt16 dataSize;
	Err error;

	// allocate global variables space
	dataSize = sizeof(GlobalsDataType);
	gVars = (GlobalsDataType *)MemPtrNew(dataSize);
	MemSet(gVars, dataSize, 0);
	
	gVars->indicator.on	= 255;
	gVars->indicator.off = 0;
	gVars->HRrefNum = sysInvalidRefNum;

	// allocate prefs space
	dataSize = sizeof(GlobalsPreferenceType);
	gPrefs	= (GlobalsPreferenceType *)MemPtrNew(dataSize);
	MemSet(gPrefs, dataSize, 0);

	// Read the saved preferences / saved-state information.
	if (PrefGetAppPreferences(appFileCreator, appPrefID, gPrefs, &dataSize, true) == noPreferenceFound)
	{
		UInt32 romVersion;
		FtrGet(sysFtrCreator, sysFtrNumROMVersion, &romVersion);

		gPrefs->volRefNum = sysInvalidRefNum;

		gPrefs->autoOff = true;
		gPrefs->vibrator = CheckVibratorExists();
		gPrefs->debug = false;
		gPrefs->stdPalette = (romVersion >= kOS5Version);
		
		gPrefs->volume.speaker = 16;
		gPrefs->volume.headphone = 16;
		
		gPrefs->volume.master = 192;
		gPrefs->volume.music = 192;
		gPrefs->volume.sfx = 192;

/*		if (gPrefs->skin.nameP)
			StrCopy(_skin.nameP, prefs.skin.nameP);

		_skin.cardNo = prefs.skin.cardNo;
		_skin.dbID = prefs.skin.dbID;

		gVibrator = prefs.vibrator;
		gAutoOff = prefs.autoOff;
		ArrowManager.position = prefs.listPosition;*/
	}

	error = AppStartCheckMathLib();
	if (error) return (error);

	error = AppStartCheckHRmode();
	if (error) return (error);

	error = AppStartLoadSkin();
	if (error) return (error);

	error = GamOpenDatabase();
	if (error) return (error);

	if (gPrefs->volRefNum != sysInvalidRefNum) {	// if volref prviously defined, check if it's a valid one
		VolumeInfoType volInfo;
		Err err = VFSVolumeInfo(gPrefs->volRefNum, &volInfo);
		if (err)
			gPrefs->volRefNum = sysInvalidRefNum;
	}
	else
		gPrefs->volRefNum = parseCards(0);	// get first volref

	AppStartCheckNotify();		// not fatal error if not avalaible

	return error;
}

/***********************************************************************
 *
 * FUNCTION:    AppStop
 *
 * DESCRIPTION: Save the current state of the application.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    nothing
 *
 * REVISION HISTORY:
 *
 *
 ***********************************************************************/
static Err AppStopCheckNotify()
{
	UInt32 romVersion;
	Err err;
	
	err = FtrGet(sysFtrCreator, sysFtrNumNotifyMgrVersion, &romVersion); 
	if (!err) {
		UInt16 cardNo;
		LocalID dbID;

		err = SysCurAppDatabase(&cardNo, &dbID);
		if (!err) {
			SysNotifyUnregister(cardNo, dbID, sysNotifyVolumeUnmountedEvent, sysNotifyNormalPriority);
			SysNotifyUnregister(cardNo, dbID, sysNotifyVolumeMountedEvent, sysNotifyNormalPriority);
		}
	}
	
	return err;
}

static void AppStop(void)
{
	// Write the saved preferences / saved-state information.  This data 
	// will saved during a HotSync backup.
/*
	StrCopy(prefs.skin.nameP,gPrefsskin.nameP);
	prefs.skin.cardNo = _skin.cardNo;
	prefs.skin.dbID = _skin.dbID;

	prefs.vibrator = iconState[IcnVibr].selected;
	prefs.autoOff = iconState[IcnAOff].selected;
	prefs.listPosition = ArrowManager.position;
*/
	SavePrefs();
	AppStopCheckNotify();
	AppStopMathLib();
	AppStopHRMode();
/*
	// Close all the open forms.
*/	FrmCloseAllForms();
	GamCloseDatabase();

	if (gVars)
		MemPtrFree(gVars);

	WinPalette(winPaletteSetToDefault, 0, 256, NULL);
}


/***********************************************************************
 *
 * FUNCTION:    ScummVMPalmMain
 *
 * DESCRIPTION: This is the main entry point for the application.
 *
 * PARAMETERS:  cmd - word value specifying the launch code. 
 *              cmdPB - pointer to a structure that is associated with the launch code. 
 *              launchFlags -  word value providing extra information about the launch.
 *
 * RETURNED:    Result of launch
 *
 * REVISION HISTORY:
 *
 *
 ***********************************************************************/
static void AppLaunchCmdNotify(UInt16 LaunchFlags, SysNotifyParamType * pData)
{
	switch (pData->notifyType)
	{
		case sysNotifyVolumeMountedEvent:
			pData->handled = true;	// don't switch

			if (gPrefs->volRefNum == sysInvalidRefNum) {
				FormPtr frmP = FrmGetActiveForm();
				VFSAnyMountParamType *notifyDetailsP = (VFSAnyMountParamType *)pData->notifyDetailsP;
				gPrefs->volRefNum = notifyDetailsP->volRefNum;

				if (frmP && gPrefs->volRefNum != sysInvalidRefNum) {
					MenuEraseStatus(0);
					FrmShowObject(frmP, FrmGetObjectIndex (frmP, MainMSBitMap));
				}
			}
		
		case sysNotifyVolumeUnmountedEvent:
			if (gPrefs->volRefNum == (UInt16)pData->notifyDetailsP) {
				FormPtr frmP = FrmGetActiveForm();
				gPrefs->volRefNum = sysInvalidRefNum;

				if (frmP) {
					MenuEraseStatus(0);
					FrmShowObject(frmP, FrmGetObjectIndex (frmP, MainMSNoneBitMap));
				}
			}
			break;
	}
}

static UInt32 ScummVMPalmMain(UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags)
{
	Err error;

	error = RomVersionCompatible (kOurMinVersion, launchFlags);
	if (error) return (error);

	switch (cmd)
		{
		case sysAppLaunchCmdNotify:
			AppLaunchCmdNotify(launchFlags, (SysNotifyParamType *) cmdPBP);
			break;

		case sysAppLaunchCmdNormalLaunch:
			error = AppStart();
			if (error) 
				goto end;
				
			FrmGotoForm(MainForm);
			AppEventLoop();
end:
			AppStop();
			break;

		default:
			break;

		}

	return error;
}
/***********************************************************************
 *
 * FUNCTION:    PilotMain
 *
 * DESCRIPTION: This is the main entry point for the application.
 *
 * PARAMETERS:  cmd - word value specifying the launch code. 
 *              cmdPB - pointer to a structure that is associated with the launch code. 
 *              launchFlags -  word value providing extra information about the launch.
 * RETURNED:    Result of launch
 *
 * REVISION HISTORY:
 *
 *
 ***********************************************************************/

UInt32 PilotMain( UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags)
{
	return ScummVMPalmMain(cmd, cmdPBP, launchFlags);
}
