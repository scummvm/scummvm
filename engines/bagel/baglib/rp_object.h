
/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef BAGEL_BAGLIB_RP_OBJECT_H
#define BAGEL_BAGLIB_RP_OBJECT_H

#include "bagel/baglib/character_object.h"
#include "bagel/baglib/dossier_object.h"
#include "bagel/baglib/expression.h"
#include "bagel/baglib/movie_object.h"
#include "bagel/baglib/text_object.h"

namespace Bagel {

// Locals
#define LOGWLD "LOG_WLD"
#define LOGZWLD "LOGZ_WLD"
#define PDAWLD "BPDA_WLD"
#define PDAZWLD "BPDAZ_WLD"

class DossierObj {
public:
	DossierObj();
	virtual ~DossierObj();

	CBagDossierObject *_pDossier;
	CBofString _sDossier;
	CBagExpression *_xDosExp;
	bool _bDisplayDossier;
};

enum RPStates {
	RP_NOMODE = 0,
	RP_RESULTS,
	RP_READ_DOSSIER,
	RP_REVIEW,
	RP_MAINMENU
};

class CBagRPObject : public CBagObject {
private:
	CBofList<DossierObj *> *_pTouchedList;   // dossiers to show if alias touched it
	CBofList<DossierObj *> *_pUntouchedList; // dossiers to show if alias did not touch it
	CBagMovieObject *_pMovieObj;             // a movie to play
	CBofString _sMovieObj;                   // Name of movie
	CBagVar *_pVarObj;                       // associated global var
	CBagTextObject *_pDescObj;               // description of residue printed object
	CBagTextObject *_pObjectName;            // description of residue printed object
	CBofString _sObjectName;                 // description of residue printed object

	CBagVar *_pSaveVar;    // Var to save info to
	CBagVar *_pTouchedVar; // variable indicating touched or not
	int _nRPTime;          // time to execute

	CBofRect _cOrigRect; // Original text rect

	int16 _nCurDossier;      // index of current dossier
	// member data boolS
	bool _bOrigRectInit : 1; // Original text initialized
	bool _bRPReported : 1;   // been reported yet?
	bool _bResPrinted : 1;   // been submitted?
	bool _bTouched : 1;      // been touched?
	bool _bRPRead : 1;       // been read
	bool _bRPTimeSet : 1;    // had time residue printing occurred?
	bool _bCurVisible : 1;   // Current one being displayed?
	bool _bInitialized : 1;  // initialization state
	bool _bMoviePlayed : 1;  // Has the movie played yet?

	static CBofList<CBagRPObject *> *_pRPList; // all the residue print objects
	static int32 _nLastRPQCheck;                // last time we surfed the queue.
	static CBagVar *_pTurncount;               // Ptr to turncount global
	static CBagVar *_pLogStateVar;             // Ptr to log state global
	static CBagVar *_pPrevLogStateVar;         // Ptr to prev log state global
	static CBagVar *_pBarLogPages;             // Ptr to total pages global
	static CBagVar *_pPrevBarPage;             //
	static CBagVar *_pCurBarPage;              // current bar page
	static CBagRPObject *_pCurRPObject;        // Most recent residue print object.
	static CBagRPObject *_pActivateThisGuy;    // The one we want to activate
	static RPStates _eRPMode;                  // Current state of display
public:
	CBagRPObject();
	virtual ~CBagRPObject();

	ParseCodes setInfo(CBagIfstream &istr);

	ErrorCode attach();
	ErrorCode detach();

	virtual bool runObject();

	ErrorCode update(CBofBitmap *pBmp, CBofPoint pt, CBofRect *pSrcRect, int);

	virtual void onLButtonUp(uint32, CBofPoint *, void * = nullptr);

	void setTouchedDos(CBofString &s, CBagExpression *x);
	void setUntouchedDos(CBofString &s, CBagExpression *x);

	bool activateRPObject();
	void deactivateRPObject();

	bool getTimeSet() {
		return _bRPTimeSet;
	}
	void setTimeSet(bool b = true) {
		_bRPTimeSet = b;
	}

	void evaluateDossiers();

	void saveRPVars();
	void restoreRPVars();

	bool initialize();

	CBagDossierObject *getActiveDossier();
	void setActiveDossier(CBagDossierObject *);

	static bool addToMsgQueue(CBagRPObject *);

	static bool zoomed();

	// Command (bagcoobj) objects, activated from script
	static int runRPQueue();
	static int updateRPQueue();
	static void deactivateRPQueue();

	static void activateRPReview();
	static void deactivateRPReview();

	static void setLogState(RPStates eLogMode);
	static RPStates getLogState();

	static void setLogPages(int);

	static void showRPReview();
	static void hideRPReview();

	static int RPResultsWaiting();
	static void removeAllFromMsgQueue(CBagRPObject *pRPObj);

	static void showPDALog();

	static void synchronizeRPObjects(bool);
};

} // namespace Bagel

#endif
