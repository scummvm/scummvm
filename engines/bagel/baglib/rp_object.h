
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
#include "bagel/baglib/variable_object.h"

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

	CBagDossierObject *m_pDossier;
	CBofString m_sDossier;
	CBagExpression *m_xDosExp;
	bool m_bDisplayDossier;
};

enum RPSTATES {
	RP_NOMODE = 0,
	RP_RESULTS,
	RP_READ_DOSSIER,
	RP_REVIEW,
	RP_MAINMENU
};

class CBagRPObject : public CBagObject {
private:
	CBofList<DossierObj *> *m_pTouchedList;   // dossiers to show if alias touched it
	CBofList<DossierObj *> *m_pUntouchedList; // dossiers to show if alias did not touch it
	CBagMovieObject *m_pMovieObj;             // a movie to play
	CBofString m_sMovieObj;                   // Name of movie
	CBagVar *m_pVarObj;                       // associated global var
	CBagTextObject *m_pDescObj;               // description of residue printed object
	CBagTextObject *m_pObjectName;            // description of residue printed object
	CBofString m_sObjectName;                 // description of residue printed object

	CBagVar *m_pSaveVar;    // Var to save info to
	CBagVar *m_pTouchedVar; // variable indicating touched or not
	INT m_nRPTime;          // time to execute

	CBofRect m_cOrigRect; // Original text rect

	int16 m_nCurDossier;      // index of current dossier
	// member data boolS
	bool m_bOrigRectInit : 1; // Original text initialized
	bool m_bRPReported : 1;   // been reported yet?
	bool m_bResPrinted : 1;   // been submitted?
	bool m_bTouched : 1;      // been touched?
	bool m_bRPRead : 1;       // been read
	bool m_bRPTimeSet : 1;    // had time residue printing occurred?
	bool m_bCurVisible : 1;   // Current one being displayed?
	bool m_bInitialized : 1;  // initialization state
	bool m_bMoviePlayed : 1;  // Has the movie played yet?

	static CBofList<CBagRPObject *> *m_pRPList; // all the residue print objects
	static int32 m_nLastRPQCheck;                // last time we surfed the queue.
	static CBagVar *m_pTurncount;               // Ptr to turncount global
	static CBagVar *m_pLogStateVar;             // Ptr to log state global
	static CBagVar *m_pPrevLogStateVar;         // Ptr to prev log state global
	static CBagVar *m_pBarLogPages;             // Ptr to total pages global
	static CBagVar *m_pPrevBarPage;             //
	static CBagVar *m_pCurBarPage;              // current bar page
	static CBagRPObject *m_pCurRPObject;        // Most recent residue print object.
	static CBagRPObject *m_pActivateThisGuy;    // The one we want to activate
	static RPSTATES m_eRPMode;                  // Current state of display
public:
	CBagRPObject();
	virtual ~CBagRPObject();

	PARSE_CODES SetInfo(bof_ifstream &istr);

	ERROR_CODE Attach();
	ERROR_CODE Detach();

	virtual bool RunObject();

	ERROR_CODE Update(CBofBitmap *pBmp, CBofPoint pt, CBofRect *pSrcRect, INT);

	virtual void OnLButtonUp(uint32, CBofPoint *, void * = nullptr);

	void SetTouchedDos(CBofString &s, CBagExpression *x);
	void SetUntouchedDos(CBofString &s, CBagExpression *x);

	bool ActivateRPObject();
	void DeactivateRPObject();

	bool GetTimeSet() {
		return m_bRPTimeSet;
	}
	void SetTimeSet(bool b = TRUE) {
		m_bRPTimeSet = b;
	}

	void EvaluateDossiers();

	void SaveRPVars();
	void RestoreRPVars();

	bool initialize();

	CBagDossierObject *GetActiveDossier();
	void SetActiveDossier(CBagDossierObject *);

	static bool AddToMsgQueue(CBagRPObject *);

	static bool Zoomed();

	// Command (bagcoobj) objects, activated from script
	static INT RunRPQueue();
	static INT UpdateRPQueue();
	static void DeactivateRPQueue();

	static void ActivateRPReview();
	static void DeactivateRPReview();

	static void SetLogState(RPSTATES eLogMode);
	static RPSTATES GetLogState();

	static void SetLogPages(INT);

	static void ShowRPReview();
	static void HideRPReview();

	static INT RPResultsWaiting();
	static void RemoveAllFromMsgQueue(CBagRPObject *pRPObj);

	static void ShowPDALog();

	static void SynchronizeRPObjects(bool);
};

} // namespace Bagel

#endif
