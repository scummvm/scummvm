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


#define MAX_PATH    255


struct CharacterStruct {
	uint32 _code;
	RMItem *_item;
	byte _r, _g, _b;
	int _talkPattern;
	int _standPattern;
	int _startTalkPattern, _endTalkPattern;
	int _numTexts;

	void save(Common::OutSaveFile *f) {
		f->writeUint32LE(_code);
		f->writeUint32LE(0);
		f->writeByte(_r);
		f->writeByte(_g);
		f->writeByte(_b);
		f->writeUint32LE(_talkPattern);
		f->writeUint32LE(_standPattern);
		f->writeUint32LE(_startTalkPattern);
		f->writeUint32LE(_endTalkPattern);
		f->writeUint32LE(_numTexts);
	}
	void load(Common::InSaveFile *f) {
		_code = f->readUint32LE();
		f->readUint32LE();
		_item = NULL;
		_r = f->readByte();
		_g = f->readByte();
		_b = f->readByte();
		_talkPattern = f->readUint32LE();
		_standPattern = f->readUint32LE();
		_startTalkPattern = f->readUint32LE();
		_endTalkPattern = f->readUint32LE();
		_numTexts = f->readUint32LE();
	}
};

struct MCharacterStruct {
	uint32 _code;
	RMItem *_item;
	byte _r, _g, _b;
	int _x, _y;
	int _numTalks[10];
	int _curGroup;
	int _numTexts;
	bool _bInTexts;
	int _curTalk;
	bool _bAlwaysBack;

	void save(Common::OutSaveFile *f) {
		f->writeUint32LE(_code);
		f->writeUint32LE(0);
		f->writeByte(_r);
		f->writeByte(_g);
		f->writeByte(_b);
		f->writeUint32LE(_x);
		f->writeUint32LE(_y);
		for (int i = 0; i < 10; ++i)
			f->writeUint32LE(_numTalks[i]);
		f->writeUint32LE(_curGroup);
		f->writeUint32LE(_numTexts);
		f->writeByte(_bInTexts);
		f->writeUint32LE(_curTalk);
		f->writeByte(_bAlwaysBack);
	}
	void load(Common::InSaveFile *f) {
		_code = f->readUint32LE();
		f->readUint32LE();
		_item = NULL;
		_r = f->readByte();
		_g = f->readByte();
		_b = f->readByte();
		_x = f->readUint32LE();
		_y = f->readUint32LE();
		for (int i = 0; i < 10; ++i)
			_numTalks[i] = f->readUint32LE();
		_curGroup = f->readUint32LE();
		_numTexts = f->readUint32LE();
		_bInTexts = f->readByte();
		_curTalk = f->readUint32LE();
		_bAlwaysBack = f->readByte();
	}
};

struct ChangedHotspotStruct {
	uint32 _dwCode;
	uint32 _nX, _nY;

	void save(Common::OutSaveFile *f) {
		f->writeUint32LE(_dwCode);
		f->writeUint32LE(_nX);
		f->writeUint32LE(_nY);
	}
	void load(Common::InSaveFile *f) {
		_dwCode = f->readUint32LE();
		_nX = f->readUint32LE();
		_nY = f->readUint32LE();
	}
};


/**
 * Description of a call to a custom function.
 */
typedef struct {
	int nCf;

	int arg1, arg2, arg3, arg4;
} CFCALL;
typedef CFCALL         *LPCFCALL;
typedef LPCFCALL       *LPLPCFCALL;


/****************************************************************************\
*       Global variables
\****************************************************************************/

/**
 * Globals class
 */
class Globals {
public:
	Globals();

	char _nextMusic[MAX_PATH];
	bool _nextLoop;
	int _nextChannel;
	int _nextSync;
	int _curChannel;
	int _flipflop;
	// OSystem::MutexRef csMusic;
	CharacterStruct _character[16];
	MCharacterStruct _mCharacter[10];
	ChangedHotspotStruct _changedHotspot[256];
	bool _isMChar[16];
	bool _bAlwaysDisplay;
	RMPoint _saveTonyPos;
	int _saveTonyLoc;
	RMTextDialog *_curBackText;
	bool _bTonyIsSpeaking;
	int _curChangedHotspot;
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
	int  nCfgDubbingVolume;
	int  nCfgMusicVolume;
	int  nCfgSFXVolume;
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

	uint32(*LoadLocation)(int, RMPoint, RMPoint start);
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
	uint32 mut[10];

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
	Common::String         *lplpFunctionStrings;
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
	uint32                  *lpResources;
	bool                     bExecutingAction;
	bool                     bExecutingDialog;
	uint32                  nPollingLocations[MAXPOLLINGLOCATIONS];
	uint32                  hEndPollingLocations[MAXPOLLINGLOCATIONS];
	uint32                  PollingThreads[MAXPOLLINGLOCATIONS];
	uint32                  hAskChoice;
	uint32                  hDoneChoice;
	uint32                    nExecutingAction;
	uint32                    nExecutingDialog;
	uint32                    nExecutingChoice;
	uint32                    nSelectedChoice;
};

} // End of namespace Tony

#endif // TONY_GLOBALS
