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
 */

#ifndef TONY_GLOBALS
#define TONY_GLOBALS

#include "common/savefile.h"
#include "tony/adv.h"
#include "tony/gfxengine.h"
#include "tony/input.h"
#include "tony/inventory.h"
#include "tony/loc.h"
#include "tony/tonychar.h"
#include "tony/mpal/mpal.h"
#include "tony/mpal/mpaldll.h"

namespace Tony {

#define T_GRILLI 1
#define T_GRILLIOV 2
#define T_GRILLIVENTO 3
#define T_GRILLIVENTO1 4
#define T_VENTO 5
#define T_MARE 6
#define T_MAREMETA 7


struct CharacterStruct {
	uint32 code;
	RMItem *item;
	byte r, g, b;
	int talkpattern;
	int standpattern;
	int starttalkpattern, endtalkpattern;
	int numtexts;

	void Save(Common::OutSaveFile *f) {
		f->writeUint32LE(code);
		f->writeUint32LE(0);
		f->writeByte(r);
		f->writeByte(g);
		f->writeByte(b);
		f->writeUint32LE(talkpattern);
		f->writeUint32LE(standpattern);
		f->writeUint32LE(starttalkpattern);
		f->writeUint32LE(endtalkpattern);
		f->writeUint32LE(numtexts);
	}
	void Load(Common::InSaveFile *f) {
		code = f->readUint32LE();
		f->readUint32LE();
		item = NULL;
		r = f->readByte();
		g = f->readByte();
		b = f->readByte();
		talkpattern = f->readUint32LE();
		standpattern = f->readUint32LE();
		starttalkpattern = f->readUint32LE();
		endtalkpattern = f->readUint32LE();
		numtexts = f->readUint32LE();
	}
};

struct MCharacterStruct {
	uint32 code;
	RMItem *item;
	byte r, g, b;
	int x, y;
	int numtalks[10];
	int curgroup;
	int numtexts;
	bool bInTexts;
	int curTalk;
	bool bAlwaysBack;

	void Save(Common::OutSaveFile *f) {
		f->writeUint32LE(code);
		f->writeUint32LE(0);
		f->writeByte(r);
		f->writeByte(g);
		f->writeByte(b);
		f->writeUint32LE(x);
		f->writeUint32LE(y);
		for (int i = 0; i < 10; ++i)
			f->writeUint32LE(numtalks[i]);
		f->writeUint32LE(curgroup);
		f->writeUint32LE(numtexts);
		f->writeByte(bInTexts);
		f->writeUint32LE(curTalk);
		f->writeByte(bAlwaysBack);
	}
	void Load(Common::InSaveFile *f) {
		code = f->readUint32LE();
		f->readUint32LE();
		item = NULL;
		r = f->readByte();
		g = f->readByte();
		b = f->readByte();
		x = f->readUint32LE();
		y = f->readUint32LE();
		for (int i = 0; i < 10; ++i)
			numtalks[i] = f->readUint32LE();
		curgroup = f->readUint32LE();
		numtexts = f->readUint32LE();
		bInTexts = f->readByte();
		curTalk = f->readUint32LE();
		bAlwaysBack = f->readByte();
	}
};

struct ChangedHotspotStruct {
	uint32 dwCode;
	uint32 nX, nY;

	void Save(Common::OutSaveFile *f) {
		f->writeUint32LE(dwCode);
		f->writeUint32LE(nX);
		f->writeUint32LE(nY);
	}
	void Load(Common::InSaveFile *f) {
		dwCode = f->readUint32LE();
		nX = f->readUint32LE();
		nY = f->readUint32LE();
	}
};

/****************************************************************************\
*       typedef CFCALL
*       --------------
* Description: Descrizione di una chiamata a una custom function
\****************************************************************************/

typedef struct {
  int nCf;

  int arg1, arg2, arg3, arg4;
} CFCALL;
typedef CFCALL*         LPCFCALL;
typedef LPCFCALL*       LPLPCFCALL;


/****************************************************************************\
*       Global variables
\****************************************************************************/

/**
 * Globals class
 */
class Globals {
public:
	Globals();

	char nextMusic[MAX_PATH];
	bool nextLoop;
	int nextChannel;
	int nextSync;
	int curChannel;
	int flipflop;
	// OSystem::MutexRef csMusic;
	CharacterStruct Character[16];
	MCharacterStruct MCharacter[10];
	ChangedHotspotStruct ChangedHotspot[256];
	bool IsMChar[16];
	bool bAlwaysDisplay;
	RMPoint saveTonyPos;
	int saveTonyLoc;
	RMTextDialog *curBackText;
	bool bTonyIsSpeaking;
	int curChangedHotspot;
	bool bPatIrqFreeze;
	bool bCfgInvLocked;
	bool bCfgInvNoScroll;
	bool bCfgTimerizedText;
	bool bCfgInvUp;
	bool bCfgAnni30;
	bool bCfgAntiAlias;
	bool bCfgSottotitoli;
	bool bCfgTransparence;
	bool bCfgInterTips;
	bool bCfgDubbing;
	bool bCfgMusic;
	bool bCfgSFX;
	int  nCfgTonySpeed;
	int  nCfgTextSpeed;
	int	 nCfgDubbingVolume;
	int	 nCfgMusicVolume;
	int	 nCfgSFXVolume;
	bool bSkipSfxNoLoop;
	bool bIdleExited;
	bool bNoOcchioDiBue;
	int curDialog;
	int curSonoriz;
	bool bFadeOutStop;

	RMTony *Tony;
	RMPointer *Pointer;
	RMGameBoxes *Boxes;
	RMLocation *Loc;
	RMInventory *Inventory;
	RMInput *Input;
	RMGfxEngine *GfxEngine;

	uint32 (*LoadLocation)(int, RMPoint, RMPoint start);
	void (*UnloadLocation)(CORO_PARAM, bool bDoOnExit, uint32 *result);
	void (*LinkGraphicTask)(RMGfxTask *task);
	void (*Freeze)(void); 
	void (*Unfreeze)(void); 
	void (*WaitFrame)(CORO_PARAM); 
	void (*PlayMusic)(int nChannel, const char *fileName, int nFX, bool bLoop, int nSync);
	void (*WaitWipeEnd)(CORO_PARAM); 
	void (*CloseWipe)(void); 
	void (*InitWipe)(int type); 
	void (*EnableGUI)(void); 
	void (*DisableGUI)(void); 
	void (*SetPalesati)(bool bpal);

	uint32 dwTonyNumTexts;
	bool bTonyInTexts;
	bool bStaticTalk;
	RMTony::TALKTYPE nTonyNextTalkType;

	RMPoint StartLocPos[256];
	OSystem::MutexRef cs[10];
	OSystem::MutexRef vdb;
	HANDLE mut[10];

	bool bSkipIdle;
	uint32 hSkipIdle;

	int lastMusic, lastTappeto;

	int tappeti[200];
	RMPoint SFM_pt;
	int SFM_nLoc;

	/**
	 * @defgroup MPAL variables
	 *
	*/
	uint32                  mpalError;
	LPITEMIRQFUNCTION       lpiifCustom;
	LPLPCUSTOMFUNCTION      lplpFunctions;
	Common::String *		lplpFunctionStrings;
	uint16                   nObjs;
	uint16                   nVars;
	HGLOBAL                  hVars;
	LPMPALVAR                lpmvVars;
	uint16                   nMsgs;
	HGLOBAL                  hMsgs;
	LPMPALMSG                lpmmMsgs;
	uint16                   nDialogs;
	HGLOBAL                  hDialogs;
	LPMPALDIALOG             lpmdDialogs;
	uint16                   nItems;
	HGLOBAL                  hItems;
	LPMPALITEM               lpmiItems;
	uint16                   nLocations;
	HGLOBAL                  hLocations;
	LPMPALLOCATION           lpmlLocations;
	uint16                   nScripts;
	HGLOBAL                  hScripts;
	LPMPALSCRIPT             lpmsScripts;
	Common::File             hMpr;
	uint16                   nResources;
	uint32 *                 lpResources;
	bool                     bExecutingAction;
	bool                     bExecutingDialog;
	uint32					nPollingLocations[MAXPOLLINGLOCATIONS];
	uint32					hEndPollingLocations[MAXPOLLINGLOCATIONS];
	uint32					PollingThreads[MAXPOLLINGLOCATIONS];
	uint32					hAskChoice;
	uint32					hDoneChoice;
	uint32                    nExecutingAction;
	uint32                    nExecutingDialog;
	uint32                    nExecutingChoice;
	uint32                    nSelectedChoice;
};

} // End of namespace Tony

#endif // TONY_GLOBALS
