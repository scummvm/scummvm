
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

#include "bagel/spacebar/baglib/dossier_object.h"
#include "bagel/spacebar/baglib/expression.h"
#include "bagel/spacebar/baglib/movie_object.h"
#include "bagel/spacebar/baglib/text_object.h"

namespace Bagel {
namespace SpaceBar {

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
	RP_NO_MODE = 0,
	RP_RESULTS,
	RP_READ_DOSSIER,
	RP_REVIEW,
	RP_MAIN_MENU
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
	bool _bOrigRectInit : true; // Original text initialized
	bool _bRPReported : true;   // been reported yet?
	bool _bResPrinted : true;   // been submitted?
	bool _bTouched : true;      // been touched?
	bool _bRPRead : true;       // been read
	bool _bRPTimeSet : true;    // had time residue printing occurred?
	bool _bCurVisible : true;   // Current one being displayed?
	bool _bInitialized : true;  // initialization state
	bool _bMoviePlayed : true;  // Has the movie played yet?

	static CBofList<CBagRPObject *> *_pRPList; // all the residue print objects
	static int32 _nLastRPQCheck;                // last time we surfed the queue.
	static CBagVar *_turnCount;               // Ptr to turnCount global
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

	ParseCodes setInfo(CBagIfstream &istr) override;

	ErrorCode attach() override;
	ErrorCode detach() override;

	bool runObject() override;

	ErrorCode update(CBofBitmap *pBmp, CBofPoint pt, CBofRect *pSrcRect, int) override;

	void onLButtonUp(uint32, CBofPoint *, void * = nullptr) override;

	void setTouchedDos(CBofString &s, CBagExpression *x);
	void setUntouchedDos(CBofString &s, CBagExpression *x);

	bool activateRPObject();
	void deactivateRPObject();

	bool getTimeSet() {
		return _bRPTimeSet;
	}
	void setTimeSet(bool b) {
		_bRPTimeSet = b;
	}

	void evaluateDossiers();

	void saveResiduePrintedVars();
	void restoreResiduePrintedVars();

	bool initialize();

	CBagDossierObject *getActiveDossier();
	void setActiveDossier(CBagDossierObject *);

	static bool addToMsgQueue(CBagRPObject *);

	static bool zoomed();

	// Command (bagcoobj) objects, activated from script
	static int runResiduePrintedQueue();
	static int updateResiduePrintedQueue();
	static void deactivateResiduePrintedQueue();

	static void activateResiduePrintedReview();
	static void deactivateResiduePrintedReview();

	static void setLogState(RPStates eLogMode);
	static RPStates getLogState();

	static void setLogPages(int);

	static void showResiduePrintedReview();
	static void hideResiduePrintedReview();

	static int residuePrintedResultsWaiting();
	static void removeAllFromMsgQueue(CBagRPObject *pRPObj);

	static void showPdaLog();

	static void synchronizeResiduePrintedObjects(bool);
};

} // namespace SpaceBar
} // namespace Bagel

#endif
