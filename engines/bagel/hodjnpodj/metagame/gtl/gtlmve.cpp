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

#include "bagel/hodjnpodj/metagame/bgen/stdafx.h"
#include "bagel/boflib/misc.h"
#include "bagel/boflib/sound.h"
#include "bagel/hodjnpodj/metagame/gtl/gtl.h"
#include "bagel/hodjnpodj/metagame/gtl/gtldoc.h"
#include "bagel/hodjnpodj/metagame/gtl/gtlview.h"
#include "bagel/hodjnpodj/metagame/gtl/gtlfrm.h"
#include "bagel/hodjnpodj/metagame/gtl/spinner.h"
#include "bagel/hodjnpodj/metagame/gtl/cmapdlg.h"
#include "bagel/hodjnpodj/metagame/bgen/note.h"
#include "bagel/hodjnpodj/metagame/bgen/notebook.h"
#include "bagel/hodjnpodj/metagame/bgen/notelist.h"
#include "bagel/hodjnpodj/metagame/gtl/citemdlg.h"
#include "bagel/hodjnpodj/metagame/bgen/c1btndlg.h"
#include "bagel/hodjnpodj/metagame/bgen/c2btndlg.h"
#include "bagel/hodjnpodj/metagame/gtl/encount.h"
#include "bagel/hodjnpodj/hodjnpodj.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {
namespace Gtl {

extern CGtlFrame    *pMainWindow;
extern CBfcMgr      *lpMetaGameStruct;
extern CGtlView     *gpMyView;
extern bool         bJustReturned;

struct CLUE_LOC {
	int iLocID;
	int iNoteID;
	int iPersonID;
	int iPlaceID;
};

const char *szGameSounds[MG_SOUND_COUNT] = {
	".\\SOUND\\PK1.WAV",
	".\\SOUND\\PK2.WAV",
	".\\SOUND\\PG1.WAV",
	".\\SOUND\\PG2.WAV",
	".\\SOUND\\PG3.WAV",
	".\\SOUND\\PG4.WAV",
	".\\SOUND\\PG5.WAV",
	".\\SOUND\\PG6.WAV",
	".\\SOUND\\FG1.WAV",
	".\\SOUND\\FG2.WAV",
	".\\SOUND\\FG3.WAV",
	".\\SOUND\\FG4.WAV",
	".\\SOUND\\FG5.WAV",
	".\\SOUND\\TG1.WAV",
	".\\SOUND\\TG2.WAV",
	".\\SOUND\\TG3.WAV",
	".\\SOUND\\TG4.WAV",
	".\\SOUND\\TG5.WAV",
	".\\SOUND\\TG6.WAV",
	".\\SOUND\\DF1.WAV",
	".\\SOUND\\DF2.WAV",
	".\\SOUND\\DF3.WAV",
	".\\SOUND\\DF4.WAV",
	".\\SOUND\\DF5.WAV",
	".\\SOUND\\GF1B.WAV",
	".\\SOUND\\GF2B.WAV",
	".\\SOUND\\GF3B.WAV",
	".\\SOUND\\CR1.WAV",
	".\\SOUND\\CR2.WAV",
	".\\SOUND\\CR3.WAV",
	".\\SOUND\\CR4.WAV",
	".\\SOUND\\CR5.WAV",
	".\\SOUND\\BC1.WAV",
	".\\SOUND\\BC2.WAV",
	".\\SOUND\\BC3.WAV",
	".\\SOUND\\BC4.WAV",
	".\\SOUND\\BC5.WAV",
	".\\SOUND\\RR1.WAV",
	".\\SOUND\\RR2.WAV",
	".\\SOUND\\RR3.WAV",
	".\\SOUND\\RR4.WAV",
	".\\SOUND\\BF1.WAV",
	".\\SOUND\\BF2.WAV",
	".\\SOUND\\BF3.WAV",
	".\\SOUND\\PR1B.WAV",
	".\\SOUND\\PR2B.WAV",
	".\\SOUND\\PR3B.WAV",
	".\\SOUND\\PR4B.WAV",
	".\\SOUND\\MD1.WAV",
	".\\SOUND\\MD2.WAV",
	".\\SOUND\\MD3.WAV",
	".\\SOUND\\MD4.WAV",
	".\\SOUND\\WS1.WAV",
	".\\SOUND\\WS2.WAV",
	".\\SOUND\\WS3.WAV",
	".\\SOUND\\WS4.WAV",
	".\\SOUND\\BQ1.WAV",
	".\\SOUND\\BQ2.WAV",
	".\\SOUND\\MK1.WAV",
	".\\SOUND\\MK6.WAV",
	".\\SOUND\\AR1.WAV",
	".\\SOUND\\AR2.WAV",
	".\\SOUND\\NV1.WAV",
	".\\SOUND\\NV2.WAV",
	".\\SOUND\\NV3.WAV",
	".\\SOUND\\AP1.WAV",
	".\\SOUND\\AP2.WAV",
	".\\SOUND\\LF1.WAV",
	".\\SOUND\\LF2.WAV",
	".\\SOUND\\EU1.WAV",
	".\\SOUND\\EU2.WAV",
	".\\SOUND\\OLN1.WAV",
	".\\SOUND\\OLN2.WAV",
	".\\SOUND\\OLN3.WAV",
	".\\SOUND\\OLN4.WAV",
	".\\SOUND\\OLN5.WAV",
	".\\SOUND\\OLN6.WAV",
	".\\SOUND\\OLN7.WAV",
	".\\SOUND\\OLN8.WAV",
	".\\SOUND\\OLN9.WAV",
	".\\SOUND\\GSPS1.WAV",
	".\\SOUND\\GSPS2.WAV",
	".\\SOUND\\GSPS3.WAV",
	".\\SOUND\\GSPS4.WAV",
	".\\SOUND\\GSPS5.WAV",
	".\\SOUND\\GSPS6.WAV",
	".\\SOUND\\GSPS7.WAV",
	".\\SOUND\\GSPS8.WAV",
	".\\SOUND\\UTP1.WAV",
	".\\SOUND\\UTP2.WAV",
	".\\SOUND\\UTP3.WAV",
	".\\SOUND\\UTP4.WAV",
	".\\SOUND\\UTP5.WAV",
	".\\SOUND\\UTP6.WAV",
	".\\SOUND\\UTP7.WAV",
	".\\SOUND\\UTP8.WAV",
	".\\SOUND\\TRN1.WAV",
	".\\SOUND\\TRN2.WAV",
	".\\SOUND\\TRN3.WAV",
	".\\SOUND\\TRN4.WAV",
	".\\SOUND\\TRN5.WAV",
	".\\SOUND\\TRN6.WAV",
	".\\SOUND\\TRN7.WAV",
	".\\SOUND\\TRN8.WAV",
	".\\SOUND\\TRN9.WAV",
	".\\SOUND\\TRN10.WAV",
	".\\SOUND\\TRN11.WAV",
	".\\SOUND\\TRN12.WAV",
	".\\SOUND\\BB61.WAV",
	".\\SOUND\\BB62.WAV",
	".\\SOUND\\BB63.WAV",
	".\\SOUND\\BB64.WAV",
	".\\SOUND\\BB65.WAV",
	".\\SOUND\\BB66.WAV",
	".\\SOUND\\BB67.WAV",
	".\\SOUND\\BB68.WAV",
	".\\SOUND\\BB69.WAV",
	".\\SOUND\\BB70.WAV",
	".\\SOUND\\BB71.WAV",
	".\\SOUND\\FM1.WAV",
	".\\SOUND\\FM2.WAV",
	".\\SOUND\\PM1.WAV",
	".\\SOUND\\PM2.WAV",
	".\\SOUND\\WC1.WAV",
	".\\SOUND\\WC2.WAV",
	".\\SOUND\\AC1.WAV",
	".\\SOUND\\AC2.WAV",
	".\\SOUND\\WT1.WAV",
	".\\SOUND\\WT2.WAV",
	".\\SOUND\\WR1.WAV",
	".\\SOUND\\WR2.WAV",
	".\\SOUND\\BR1.WAV",
	".\\SOUND\\BR2.WAV",
	".\\SOUND\\MM1.WAV",
	".\\SOUND\\MM2.WAV",
	".\\SOUND\\PI1.WAV",
	".\\SOUND\\PI2.WAV",
	".\\SOUND\\PI3.WAV",
	".\\SOUND\\HM1.WAV",
	".\\SOUND\\HM2.WAV"
};

static const CLUE_LOC nClueLocation[NOTE_COUNT] = {
	{ MG_LOC_INN,        NOTE_MANKALA,            NOTE_ICON_CRAB,       NOTE_ICON_INN},
	{ MG_LOC_TOURNAMENT, NOTE_ARCHEROIDS,         NOTE_ICON_ARCHER,     NOTE_ICON_FIELD},
	{ MG_LOC_BOARDING,   NOTE_NOVACANCY,          NOTE_ICON_INNKEEPER,  NOTE_ICON_HOUSE},
	{ MG_LOC_ARTISTCOTT, NOTE_ARTPARTS,           NOTE_ICON_ARTIST,     NOTE_ICON_ARTISTCOTTAGE},
	{ MG_LOC_TEMPLE,     NOTE_LIFE,               NOTE_ICON_PRIEST,     NOTE_ICON_TEMPLE},
	{ MG_LOC_BANDITHIDE, NOTE_EVERYTHINGUNDERSUN, NOTE_ICON_BANDIT,     NOTE_ICON_HIDEOUT},
	{ MG_LOC_FARMHOUSE,  NOTE_FARMHOUSE,          NOTE_ICON_FARMER,     NOTE_ICON_FARM},
	{ MG_LOC_MANSION,    NOTE_MANSION,            NOTE_ICON_ARISTOCRAT, NOTE_ICON_MANSION},
	{ MG_LOC_WOODCUTTER, NOTE_WOODCUTTERSCOTTAGE, NOTE_ICON_WOODCUTTER, NOTE_ICON_CUTTERCOTTAGE},
	{ MG_LOC_POSTOFFICE, NOTE_POSTOFFICE,         NOTE_ICON_POSTMASTER, NOTE_ICON_POSTOFFICE},
	{ MG_LOC_AMPHI,      NOTE_GARFUNKLE,          NOTE_ICON_LEOPOLD,    NOTE_ICON_THEATRE}
};

bool                bExitMetaDLL = false;

bool st_bExitDll = false ;  // static DLL exit flag

bool bLacksMoney = false;   // 1 - lacks money for eligibility, 0 - doesn't lack money

int nMissTurn = 0;  // 0 - nobody misses turn; 1 - Hodj misses turn; 2 - Podj misses turn;

///DEFS gtldat.h


// destructor deletes strategy information object
CXodj::~CXodj() {
	if (m_xpStrategyInfo != nullptr)
		delete m_xpStrategyInfo ;

	if (m_pThemeSound != nullptr) {
		m_pThemeSound->stop();
		delete m_pThemeSound;
		m_pThemeSound = nullptr;
	}
}

//* CGtlData::InitProblem -- initialize character problem
bool CGtlData::InitProblem()
// returns: true if error, false otherwise
{
	JXENTER(CGtlData::InitProblem) ;
	int iError = 0 ;        // error code
	bool bMishMoshLoc = false ; // true when Mish/Mosh location known
	CXodj * xpXodj ;
	int iK, iL, iIndex ;        // loop variables
	CGtlApp * xpGtlApp = (CGtlApp *)AfxGetApp() ; // get application
	int iLengthCode ;   // game length: MG_LENGTH_xxxx
	int iOption ;

//    if (lpMetaGameStruct && lpMetaGameStruct->m_bInventories)
//  GTB - Ichanged this

	if (lpMetaGameStruct != nullptr)
		iLengthCode =
		    (lpMetaGameStruct->m_iGameTime == SHORT_GAME)
		    ? MG_LENGTH_SHORT
		    : (lpMetaGameStruct->m_iGameTime == MEDIUM_GAME)
		    ? MG_LENGTH_MEDIUM
		    : MG_LENGTH_LONG ;
	else                // no initialize code
		iLengthCode = MG_LENGTH_LONG ;  // long game default

	if ((iOption = xpGtlApp->m_cBdbgMgr. GetDebugInt("gamelength", -1)) >= 0)
		iLengthCode = (iOption == 0) ? MG_LENGTH_SHORT
		              : (iOption == 1) ? MG_LENGTH_MEDIUM
		              : MG_LENGTH_LONG ;

	const CLengthTable * xpLengthTable = CMgStatic::cLengthTable ;
	for (; xpLengthTable->m_iLengthCode != iLengthCode
	        && xpLengthTable->m_iLengthCode ; ++xpLengthTable)
		;   // null loop body

	int iNumObjects = xpLengthTable->m_iNumObjects ;
	// number of objects player needs
	int iNumInfo = xpLengthTable->m_iNumInfo ;
	// total number of pieces of information player needs
	int iPrimaryInfo = iNumObjects + 1 ;
	// #pieces primary information (where object
	// or mishmosh are)
	int iSecondaryInfo = iNumInfo - iPrimaryInfo ;
	// #pieces secondary info (where to get primary info)
	//bool bMoneyRequired = xpLengthTable->m_bMoneyRequired ;
	// if true, then one object must be money

	if (iSecondaryInfo < 0) // only possible if argument bad
		iSecondaryInfo = 0 ;

	int iObjectClues[MAX_CLUES], iSecondaryClues[MAX_CLUES],
	    iMishMoshClues[MAX_CLUES];
	int iObjectCount, iSecondaryCount, iMishMoshCount, iMoneyCount ;
	//int iMoneyClue = 0 ;        // number of chosen money clue
	const CClueTable * xpClueTable, *xpClueEntry ;

	for (xpXodj = m_xpXodjChain ; xpXodj ; xpXodj = xpXodj->m_xpXodjNext) {

		xpXodj->m_iWinInfoWon = xpXodj->m_iSecondaryInfoWon = 0 ;
//      xpXodj->m_bHaveMishMosh = false ;
		xpXodj->m_iWinInfoNeed = iPrimaryInfo ;
		xpXodj->m_iSecondaryInfoNeed = iSecondaryInfo ;
		xpXodj->m_iRequiredObjectsCount = iNumObjects ;
		xpXodj->m_iRequiredMoney = 0 ;  // no money required yet

		if (lpMetaGameStruct && lpMetaGameStruct->m_bInventories) {

			if (xpXodj->m_bHodj) {

				xpXodj->m_pInventory = lpMetaGameStruct->m_cHodj.m_pInventory;
				xpXodj->m_pGenStore = lpMetaGameStruct->m_cHodj.m_pGenStore;
				xpXodj->m_pBlackMarket = lpMetaGameStruct->m_cHodj.m_pBlackMarket;
				xpXodj->m_iFurlongs = lpMetaGameStruct->m_cHodj.m_iFurlongs;
				xpXodj->m_nTurns = lpMetaGameStruct->m_cHodj.m_nTurns;

				// test debugging override
				if ((iOption = xpGtlApp->m_cBdbgMgr.GetDebugInt("hodjcomputer", -1)) >= 0)
					xpXodj->m_bComputer = iOption ;
//                xpXodj->m_bComputer = true ;
				xpXodj->m_bComputer = lpMetaGameStruct->m_cHodj.m_bComputer ;

			} else {

				xpXodj->m_pInventory = lpMetaGameStruct->m_cPodj.m_pInventory;
				xpXodj->m_pGenStore = lpMetaGameStruct->m_cPodj.m_pGenStore;
				xpXodj->m_pBlackMarket = lpMetaGameStruct->m_cPodj.m_pBlackMarket;
				xpXodj->m_iFurlongs = lpMetaGameStruct->m_cPodj.m_iFurlongs;
				xpXodj->m_nTurns = lpMetaGameStruct->m_cPodj.m_nTurns;

				// test debugging override
				if ((iOption = xpGtlApp->m_cBdbgMgr.GetDebugInt("podjcomputer", -1)) >= 0)
					xpXodj->m_bComputer = iOption ;
				xpXodj->m_bComputer = true ;
//                xpXodj->m_bComputer = lpMetaGameStruct->m_cPodj.m_bComputer ;
			}
		}

		if (xpXodj->m_bHodj == false) {
			xpXodj->m_bComputer = lpMetaGameStruct->m_cPodj.m_bComputer ;
		} else {
			xpXodj->m_bComputer = lpMetaGameStruct->m_cHodj.m_bComputer ;
		}

		iObjectCount = iSecondaryCount = iMishMoshCount = iMoneyCount = 0 ;

		xpClueTable = xpXodj->m_bHodj ? CMgStatic::cHodjClueTable : CMgStatic::cPodjClueTable;

		for (xpClueEntry = xpClueTable ; xpClueEntry->m_iCluePicCode ; ++xpClueEntry) {
			iIndex = xpClueEntry - xpClueTable ;    // get index
			switch (xpClueEntry->m_iClueCode) {
			case MG_CLUE_NEED:  /* need an object */
				if (xpClueEntry->m_iArgument != MG_OBJ_HERRING)
					iObjectClues[iObjectCount++] = iIndex ;
				break ;

			case MG_CLUE_MISHMOSH:  /* where to find Mish/Mosh */
				if (xpClueEntry->m_iArgument != MG_LOC_HERRING
				        && LocationToNode(xpClueEntry->m_iArgument))
					iMishMoshClues[iMishMoshCount++] = iIndex ;
				break ;

			case MG_CLUE_GOTO:  /* go for information */
				if (xpClueEntry->m_iArgument != MG_LOC_HERRING)
					iSecondaryClues[iSecondaryCount++] = iIndex ;
				break ;

			case MG_CLUE_CROWNS:    /* need some crowns */
				//iMoneyClues[iMoneyCount++] = iIndex ;
				break;

			default:
				break ;
			}
		}

		if (iObjectCount < iNumObjects || iMishMoshCount < 1
		        || iMoneyCount < 1
		        || iSecondaryCount < iSecondaryInfo) {
			iError = 100 ;
			goto cleanup ;
		}

		m_cGenUtil.RandomSelection(iObjectClues, iObjectCount,
		                           iNumObjects) ;

		for (iK = 0 ; iK < iNumObjects ; ++iK) {
			xpClueEntry = xpClueTable + iObjectClues[iK] ;
			if (xpClueEntry->m_iClueCode == MG_CLUE_NEED)
				// this clue needs an object
				xpXodj->m_iRequiredObjectsTable[iK]
				    = xpClueEntry->m_iArgument ;
			else {
				xpXodj->m_iRequiredObjectsTable[iK] = MG_OBJ_CROWN ;
				xpXodj->m_iRequiredMoney = xpClueEntry->m_iArgument ;
			}
		}

		xpXodj->m_iRequiredObjectsCount = iNumObjects ;


		if (bMishMoshLoc) { // if MishMosh location known
			for (iK = 0 ; iK < iMishMoshCount &&
			        xpClueTable[iMishMoshClues[iK]].m_iArgument
			        != m_iMishMoshLoc ; ++iK)
				;   // null loop body

			if (iK >= iMishMoshCount) {
				iError = 101 ;
				goto cleanup ;
			}
		} else {
			iK = m_cGenUtil.RandomInteger(0, iMishMoshCount - 1) ;
			m_iMishMoshLoc = xpClueTable[iMishMoshClues[iK]].m_iArgument ;
			bMishMoshLoc = true;    //  Mish/Mosh loc now known
		}

		iObjectClues[iNumObjects] = iMishMoshClues[iK] ;
		// add MishMosh clue to object clues

		m_cGenUtil.RandomSelection(iObjectClues, iPrimaryInfo,
		                           iPrimaryInfo) ;
		m_cGenUtil.RandomSelection(iSecondaryClues, iSecondaryCount,
		                           iSecondaryInfo) ;

		// exchange the secondary clues with the object clues in
		// the corresponding positions, so that only object clues
		// are given at the secondary locations.

		for (iK = 0 ; iK < iSecondaryInfo ; ++iK) {
			xpXodj->m_iSecondaryInfoTable[iK] = iObjectClues[iK] ;
			iObjectClues[iK] = iSecondaryClues[iK] ;
		}

		// now scramble up all the object and secondary clues to be
		// given in primary locations.
		m_cGenUtil.RandomSelection(iObjectClues,
		                           iPrimaryInfo, iPrimaryInfo) ;

		for (iK = 0 ; iK < iPrimaryInfo ; ++iK)
			xpXodj->m_iWinInfoTable[iK] = iObjectClues[iK] ;

		// no secondary locations have been assigned yet
		xpXodj->m_iSecondaryLoc[0] = 0 ;

		// if one of the required objects is money, we remove it
		// from the required objects table
		for (iK = iL = 0 ; iK < xpXodj->m_iRequiredObjectsCount; ++iK)
			if (xpXodj->m_iRequiredObjectsTable[iK] != MG_OBJ_CROWN)
				xpXodj->m_iRequiredObjectsTable[iL++] = xpXodj->m_iRequiredObjectsTable[iK] ;
		xpXodj->m_iRequiredObjectsCount = iL;
	}

cleanup:

	JXELEAVE(CGtlData::InitProblem) ;
	RETURN(iError != 0) ;
}

//* CGtlData::EndMoveProcessing -- handle things at end of a move
bool CGtlData::EndMoveProcessing()
// returns: true if error, false otherwise
{
	JXENTER(CGtlData::EndMoveProcessing) ;
	int iError = 0 ;        // error code
	CGtlView * xpGtlView = (CGtlView *)m_cBgbMgr.m_xpcView ;
	CXodj * xpXodj = m_xpCurXodj ;  // character who just moved
	CNode FAR * lpNode ;
	CMap *lpMap;
	const CLocTable * xpLocTable = CMgStatic::cLocTable, *xpLocEntry ;
	int iLocFunctionCode ;  // MG_GAME_xxxx or MG_VISIT_xxxx
	bool bExitDll = false ;
	static bool bFirstTime = true;
	CSound *pSound;

	if (xpXodj && xpXodj->m_iCharNode >= 0 && xpXodj->m_iCharNode < m_iNodes && !(lpNode = m_lpNodes + xpXodj->m_iCharNode)->m_bDeleted && lpNode->m_bRelocatable) {

		PositionCharacters() ;

		#if 1
		if (lpNode->m_iWeight == 0) {
			ErrorLog("ERROR.LOG", "Error: %s stopped on Passthru Node #%d", xpXodj->m_szName, xpXodj->m_iCharNode);
		}
		#endif

		if (lpNode->m_iLocationCode) {
			for (xpLocEntry = xpLocTable ; xpLocEntry->m_iLocCode && xpLocEntry->m_iLocCode != lpNode->m_iLocationCode; ++xpLocEntry)
				;   // null loop body

			// if Hodj and Podj are on the same node and
			// if one of them has mish and mosh and it's the other's turn

			if ((((lpMetaGameStruct->m_cHodj.m_bHaveMishMosh) && (lpMetaGameStruct->m_cPodj.m_bMoving) && (xpXodj->m_iCharNode == lpMetaGameStruct->m_cHodj.m_iNode)) ||
			        ((lpMetaGameStruct->m_cPodj.m_bHaveMishMosh) && (lpMetaGameStruct->m_cHodj.m_bMoving) && (xpXodj->m_iCharNode == lpMetaGameStruct->m_cPodj.m_iNode))) &&
			        (!m_bJustPlayedMiniGame)) {

				// then set the the game code to the MG_GAME_CHALLENGE

				pSound = new CSound(xpGtlView, (lpMetaGameStruct->m_cPodj.m_bHaveMishMosh ? ".\\sound\\msc18.wav" : ".\\sound\\msc19.wav"), SOUND_WAVE | SOUND_QUEUE | SOUND_ASYNCH | SOUND_AUTODELETE);
				pSound->setDrivePath(lpMetaGameStruct->m_chCDPath);
				pSound->play();

				if (lpMetaGameStruct->m_cHodj.m_bHaveMishMosh) {
					C1ButtonDialog dlg1Button((CWnd *)pMainWindow, m_cBgbMgr.m_xpGamePalette, "&OK", " ", "Podj challenges Hodj!");
					dlg1Button.DoModal();
				} else {
					C1ButtonDialog dlg1Button((CWnd *)pMainWindow, m_cBgbMgr.m_xpGamePalette, "&OK", " ", "Hodj challenges Podj!");
					dlg1Button.DoModal();
				}

				CSound::waitWaveSounds();

				InitInterface(MG_GAME_CHALLENGE, bExitDll) ;
			} else {
				// if this location is in table
				if (xpLocEntry->m_iLocCode) {

					// GTB enters into location - special movement ( check eligibility )
					// dispatch to InitInterface ( set the m_iSpecialTravelCode to the correct thing


					if (m_bJustPlayedMiniGame) {
						xpXodj->m_bGatherInformation = true ;
						xpXodj->m_iFurlongs = 0 ;
						SetFurlongs(xpXodj);
					} else {
						// if we're eligible for something at this
						// location
						if (DetermineEligibility(xpXodj, lpNode->m_iLocationCode, iLocFunctionCode, true)) {

							// go launch game or take other action
							xpXodj->m_bGatherInformation = true ;
							xpXodj->m_iFurlongs = 0 ;
							SetFurlongs(xpXodj);
							InitInterface(iLocFunctionCode, bExitDll) ;

							// not eligible
							//
						} else {
							if (xpXodj->m_bComputer) {
								xpXodj->m_bGatherInformation = true ;
								xpXodj->m_iFurlongs = 0 ;
								SetFurlongs(xpXodj);
							}
							TakeIneligibleAction(xpXodj, xpLocEntry->m_iFunctionCode, lpNode->m_iLocationCode);
						}
					}
				}
			}
		} else {

			// if Hodj and Podj are on the same node and
			// if one of them has mish and mosh and it's the other's turn

			if ((((lpMetaGameStruct->m_cHodj.m_bHaveMishMosh) && (lpMetaGameStruct->m_cPodj.m_bMoving) && (xpXodj->m_iCharNode == lpMetaGameStruct->m_cHodj.m_iNode)) ||
			        ((lpMetaGameStruct->m_cPodj.m_bHaveMishMosh) && (lpMetaGameStruct->m_cHodj.m_bMoving) && (xpXodj->m_iCharNode == lpMetaGameStruct->m_cPodj.m_iNode))) &&
			        (!m_bJustPlayedMiniGame)) {

				// then set the the game code to the MG_GAME_CHALLENGE

				pSound = new CSound(xpGtlView, (lpMetaGameStruct->m_cPodj.m_bHaveMishMosh ? ".\\sound\\msc18.wav" : ".\\sound\\msc19.wav"), SOUND_WAVE | SOUND_QUEUE | SOUND_ASYNCH | SOUND_AUTODELETE);
				pSound->setDrivePath(lpMetaGameStruct->m_chCDPath);
				pSound->play();

				if (lpMetaGameStruct->m_cHodj.m_bHaveMishMosh) {
					C1ButtonDialog dlg1Button((CWnd *)pMainWindow, m_cBgbMgr.m_xpGamePalette, "&OK", " ", "Podj challenges Hodj!");
					dlg1Button.DoModal();
				} else {
					C1ButtonDialog dlg1Button((CWnd *)pMainWindow, m_cBgbMgr.m_xpGamePalette, "&OK", " ", "Hodj challenges Podj!");
					dlg1Button.DoModal();
				}

				CSound::waitWaveSounds();

				InitInterface(MG_GAME_CHALLENGE, bExitDll) ;
			}
		}
		// update the furlongs at end of move
		//
		if (!lpMetaGameStruct->m_bScrolling) {

			if ((m_xpCurXodj->m_iFurlongs >= 0) && (m_xpCurXodj->m_iFurlongs < 24) && ((lpMap = m_lpFurlongMaps[m_xpCurXodj->m_iFurlongs]) != nullptr) && lpMap->m_lpcBgbObject)
				DrawABitmap(nullptr, lpMap->m_lpcBgbObject);
		}

		if (!bExitDll && (xpXodj->m_iFurlongs == 0) && !m_bGameOver) {

			// can't do booby trap on first time thru
			//
			if (bFirstTime) {

				bFirstTime = false;

			} else {

				// do booby trap if we did not just come back from a mini-game
				//
				if ((!m_bJustPlayedMiniGame) || (lpMetaGameStruct->m_bRestart == false)) {
					InitInterface(MG_DLLX_COLOR, bExitDll);
				}
			}

			bJustReturned = false;
			m_bJustPlayedMiniGame = false;

			// switch between Hodj and Podj
			while (SwitchPlayers())
				;
		}
	}

// GTB put a Windows message traffic chec here

// cleanup:

	JXELEAVE(CGtlData::EndMoveProcessing) ;
	RETURN(iError != 0) ;
}

#define MAX_NODE_LIST 40

//* CGtlData::CheckForTransport -- Transports a player to another sector
// after a booby trap was hit
void CGtlData::CheckForTransport(CXodj *pXodj, int nNewSector)
// GTB - I wrote this function
{
	CPoint  ptNew;
	CNode *pNode;
	int i, j, iNodeList[MAX_NODE_LIST];
	int iNode;

	// must be a valid sector number
	assert(nNewSector >= MG_SECTOR_BASE && nNewSector <= MG_SECTOR_MAX);

	// go thru the node chain, and create a list of all nodes with this sector
	//
	j = 0;
	for (i = 0; i < m_iNodes; i++) {
		pNode = m_lpNodes + i;
		if (pNode->m_iSector == nNewSector) {

			// only use normal nodes
			if ((pNode->m_iLocationCode == 0) && (pNode->m_iWeight == 1)) {
				iNodeList[j++] = i;
				if (j >= MAX_NODE_LIST)
					break;
			}
		}
	}

	assert(j != 0);

	#if 1

	iNode = iNodeList[brand() % j];

	DoTransport(pXodj, iNode);

	#else
	pXodj->m_iCharNode = iNodeList[brand() % j];

	ptNew = NodeToPoint(m_lpNodes + pXodj->m_iCharNode, &pXodj->m_lpcCharSprite->m_cSize);
	m_cBgbMgr.SetPosition(pXodj->m_lpcCharSprite, ptNew);

	// Re-center new current player
	m_xpcGtlDoc->m_xpcLastFocusView->m_cViewBsuSet.EdgeToCenter(pXodj->m_lpcCharSprite->m_crPosition, true);


	// pause at this location for 1 sec.
	Sleep(1000);
	#endif

	// if both players on same node, then re-position them
	PositionCharacters();

	pXodj->m_iTargetLocation = 0;
}


void CGtlData::DoTransport(CXodj *pXodj, int iNode) {
	char szOldFileName[128];
	CBgbObject *pCurPlayer;
	CPoint point;
	int i;

	point = NodeToPoint(m_lpNodes + iNode, &pXodj->m_lpcCharSprite->m_cSize);

	pCurPlayer = pXodj->m_lpcCharSprite;

	// object must be in memory to get access to the sprite
	m_cBgbMgr.CacheLoadObject(pCurPlayer);

	// if there is no DC, then get one
	if (m_cBgbMgr.m_xpDc == nullptr) {
		m_cBgbMgr.ReInitDc();
	}

	assert(m_cBgbMgr.m_xpBsuSet != nullptr);
	if (pCurPlayer->IfRelocatable()) {
		m_cBgbMgr.m_xpBsuSet->PrepareDc(m_cBgbMgr.m_xpDc, true);
	} else {
		m_cBgbMgr.m_xpBsuSet->PrepareDc(m_cBgbMgr.m_xpDc, false);
	}

	// erase old image
	((CSprite *)pCurPlayer->m_pObject)->EraseSprite(m_cBgbMgr.m_xpDc);

	// set character's new position
	//
	pXodj->m_iCharNode = iNode;
	m_cBgbMgr.SetPosition(pXodj->m_lpcCharSprite, point);

	// load new cel strip
	//

	// save old filename
	Common::strcpy_s(szOldFileName, pCurPlayer->m_szFileName);

	// use stars animation bitmap
	Common::strcpy_s(pCurPlayer->m_szFileName, pXodj->m_pszStarsFile);
	((CSprite *)pCurPlayer->m_pObject)->LoadCels(m_cBgbMgr.m_xpDc, pCurPlayer->m_szFileName, pCurPlayer->m_nCels);

	((CSprite *)pCurPlayer->m_pObject)->SetAnimated(true);

	// Re-center where current player will end up
	m_xpcGtlDoc->m_xpcLastFocusView->m_cViewBsuSet.EdgeToCenter(point, true);

	// paint each cel in animation strip
	//
	for (i = 1; i < pCurPlayer->m_nCels * 2; i++) {
		m_cBgbMgr.PaintBitmapObject(pCurPlayer);
		Sleep(200);
	}

	// object must be in memory to get access to the sprite
	m_cBgbMgr.CacheLoadObject(pCurPlayer);

	// if there is no DC, then get one
	if (m_cBgbMgr.m_xpDc == nullptr) {
		m_cBgbMgr.ReInitDc();
	}

	assert(m_cBgbMgr.m_xpBsuSet != nullptr);
	if (pCurPlayer->IfRelocatable()) {
		m_cBgbMgr.m_xpBsuSet->PrepareDc(m_cBgbMgr.m_xpDc, true);
	} else {
		m_cBgbMgr.m_xpBsuSet->PrepareDc(m_cBgbMgr.m_xpDc, false);
	}

	// erase old image
	((CSprite *)pCurPlayer->m_pObject)->EraseSprite(m_cBgbMgr.m_xpDc);

	// put back old filename
	Common::strcpy_s(pCurPlayer->m_szFileName, szOldFileName);
	((CSprite *)pCurPlayer->m_pObject)->LoadCels(m_cBgbMgr.m_xpDc, pCurPlayer->m_szFileName, pCurPlayer->m_nCels);

	m_cBgbMgr.PaintBitmapObject(pXodj->m_lpcCharSprite);
}


//* CGtlData::SwitchPlayers -- switch between Hodj and Podj
bool CGtlData::SwitchPlayers()
//// int FAR PASCAL CGtlData::SwitchPlayers()
// returns: true if we need to switch players again, false otherwise
{
	CHodjPodj *pCurPlayer;
	CMap FAR * lpMap ;
	CNode *pNode;
	char szBuf[80];
	bool bExitDll = false ; // if true, exit from DLL
	//int iFurlongs ; // number of furlongs to move
	int iSpecialCode;
	int iLocFunctionCode;
	bool bSpinner, bAgain;

	CSound::waitWaveSounds();

	m_xpcGtlDoc->m_xpcLastFocusView->FlushInputEvents();

	bAgain = false;

	// switch to next player
	//
	if (m_xpCurXodj->m_nTurns <= 0) {

		// current player has no turns left
		m_xpCurXodj->m_nTurns = 0;

		// stop this character's theme song
		//
		if (m_xpCurXodj->m_pThemeSound != nullptr) {
			m_xpCurXodj->m_pThemeSound->stop();
			delete m_xpCurXodj->m_pThemeSound;
			m_xpCurXodj->m_pThemeSound = nullptr;
		}

		// do the switch
		if ((m_xpCurXodj = m_xpCurXodj->m_xpXodjNext) == nullptr)
			m_xpCurXodj = m_xpXodjChain;

		// new player has 1 more turn
		m_xpCurXodj->m_nTurns++;

		// show turn indicator
		DrawABitmap(nullptr, m_xpCurXodj->m_lpcIcon, true);

		// Re-center new current player
		m_xpcGtlDoc->m_xpcLastFocusView->m_cViewBsuSet.EdgeToCenter(m_xpCurXodj->m_lpcCharSprite->m_crPosition, true);
	}

	// if music is enabled
	//
	if (lpMetaGameStruct->m_stGameStruct.bMusicEnabled) {

		// start new character's theme song
		//
		if (m_xpCurXodj->m_pThemeSound == nullptr) {

			m_xpCurXodj->m_pThemeSound = new CSound(m_xpGtlView, m_xpCurXodj->m_pszThemeFile, SOUND_MIDI | SOUND_LOOP | SOUND_DONT_LOOP_TO_END);
			m_xpCurXodj->m_pThemeSound->midiLoopPlaySegment(m_xpCurXodj->m_nThemeStart, m_xpCurXodj->m_nThemeEnd);
		}
	}

	bSpinner = false;

	pCurPlayer = &lpMetaGameStruct->m_cPodj;
	if (m_xpCurXodj->m_bHodj) {
		pCurPlayer = &lpMetaGameStruct->m_cHodj;
	}

	pNode = m_lpNodes + m_xpCurXodj->m_iCharNode;

	switch (pNode->m_iLocationCode) {

	case MG_LOC_BOAT1:

		if (pCurPlayer->m_bComputer) {
			bSpinner = true;
			break;
		}

		if (pCurPlayer->m_iNumberBoatTries != 0) {

			C2ButtonDialog dlg2Button((CWnd *)pMainWindow, m_cBgbMgr.m_xpGamePalette, "&Yes", "&No", "Would you like to try", "taking the ferry again?");

			if (dlg2Button.DoModal() == CBUTTON1) {

				iLocFunctionCode = MG_VISIT_BOAT1;
				if (DetermineEligibility(m_xpCurXodj, pNode->m_iLocationCode, iLocFunctionCode, true)) {

					// go launch game or take other action
					InitInterface(MG_VISIT_BOAT1, bExitDll);
				} else {
					TakeIneligibleAction(m_xpCurXodj, MG_VISIT_BOAT1, MG_VISIT_BOAT1);
				}
				bAgain = true;
			} else {
				bSpinner = true;
			}
		} else {
			iSpecialCode = pCurPlayer->m_iSpecialTravelCode;
			assert(iSpecialCode != MG_VISIT_NONE);

			if (iSpecialCode == MG_VISIT_BOAT2) {

				// continue upstream ?
				Common::strcpy_s(szBuf, "Continue Upstream?");

				iSpecialCode = MG_VISIT_BOAT4;

			} else {

				assert(iSpecialCode == MG_VISIT_BOAT3);

				// continue downstream ?
				Common::strcpy_s(szBuf, "Continue Downstream?");
				iSpecialCode = MG_VISIT_BOAT1;
			}
			C2ButtonDialog dlg2Button((CWnd *)pMainWindow, m_cBgbMgr.m_xpGamePalette, "&Yes", "&No", " ", szBuf);

			if (dlg2Button.DoModal() == CBUTTON1) {

				pCurPlayer->m_iSpecialTravelCode = iSpecialCode;

				m_xpCurXodj->m_iCharNode = DoSpecialTravel(iSpecialCode, m_xpCurXodj->m_bHodj);

				// Reposition characters if they are on the same node
				PositionCharacters();

				bAgain = true;
			} else {
				bSpinner = true;
			}
		}
		break;

	case MG_LOC_BOAT2:

		if (pCurPlayer->m_bComputer) {
			bSpinner = true;
			break;
		}

		if (pCurPlayer->m_iNumberBoatTries != 0) {

			C2ButtonDialog dlg2Button((CWnd *)pMainWindow, m_cBgbMgr.m_xpGamePalette, "&Yes", "&No", "Would you like to try", "taking the ferry again?");

			if (dlg2Button.DoModal() == CBUTTON1) {

				iLocFunctionCode = MG_VISIT_BOAT2;
				if (DetermineEligibility(m_xpCurXodj, pNode->m_iLocationCode, iLocFunctionCode, true)) {

					// go launch game or take other action
					InitInterface(MG_VISIT_BOAT2, bExitDll);
				} else {
					TakeIneligibleAction(m_xpCurXodj, MG_VISIT_BOAT2, MG_VISIT_BOAT2);
				}
				bAgain = true;
			} else {
				bSpinner = true;
			}

		} else {
			bSpinner = true;
		}

		break;

	case MG_LOC_BOAT3:

		if (pCurPlayer->m_bComputer) {
			bSpinner = true;
			break;
		}

		if (pCurPlayer->m_iNumberBoatTries != 0) {

			C2ButtonDialog dlg2Button((CWnd *)pMainWindow, m_cBgbMgr.m_xpGamePalette, "&Yes", "&No", "Would you like to try", "taking the ferry again?");

			if (dlg2Button.DoModal() == CBUTTON1) {

				iLocFunctionCode = MG_VISIT_BOAT3;
				if (DetermineEligibility(m_xpCurXodj, pNode->m_iLocationCode, iLocFunctionCode, true)) {

					// go launch game or take other action
					InitInterface(MG_VISIT_BOAT3, bExitDll);
				} else {
					TakeIneligibleAction(m_xpCurXodj, MG_VISIT_BOAT3, MG_VISIT_BOAT3);
				}
				bAgain = true;
			} else {
				bSpinner = true;
			}

		} else {
			bSpinner = true;
		}
		break;

	default:
		bSpinner = true;
		break;
	}

	m_xpCurXodj->m_nTurns--;
	m_xpCurXodj->m_iFurlongs = 0;
	SetFurlongs(m_xpCurXodj);
	if (bSpinner) {
		InitInterface(MG_DLLX_SPINNER, bExitDll) ;
	}

	SetFurlongs(m_xpCurXodj);

	if (!bExitDll && m_xpCurXodj->m_iFurlongs >= 0 && (uint)m_xpCurXodj->m_iFurlongs < DIMENSION(m_lpFurlongMaps) && ((lpMap = m_lpFurlongMaps[m_xpCurXodj->m_iFurlongs]) != nullptr) && lpMap->m_lpcBgbObject)
		DrawABitmap(nullptr, lpMap->m_lpcBgbObject);

	return (bAgain);
}


//* CGtlData::InitInterface -- init data for interface manager
bool CGtlData::InitInterface(int iCode, bool & bExitDll)
// iCode -- MG_GAME_xxxx to launch specified game;
//      MG_DLLX_xxxx to perform specified function
//      MG_VISIT_xxxx to perform specified visit code
// bExitDll (output) -- if true, exit from meta game DLL
// returns: true if error, false otherwise
{
	JXENTER(CGtlData::InitInterface) ;
	int iError = 0 ;        // error code
	CXodj * xpXodj, *xpHXodj = nullptr, *xpPXodj = nullptr ;    // hodj/podj
	CHodjPodj FAR * lpCodj, FAR * lpHodj, FAR * lpPodj ;
	// interface hodj/podj
	int iCount ;
	CSpinner    *pSpinner = nullptr;
	CGtlFrame * xpGtlFrame = m_xpcGtlDoc->m_xpGtlFrame ;
	CBfcMgr FAR * lpBfcMgr ;
	CNode FAR * lpNode ;
	const CGameTable * xpGameTable = CMgStatic::cGameTable, *xpGameEntry ;
	CPoint point;
	CGtlView * xpGtlView = (CGtlView *)m_cBgbMgr.m_xpcView ;
	CBsuInfo cBsuInfo ; // scroll set information
	CHodjPodj *pCurPlayer;
	CXodj  *pOtherXodj;
	int     nPSector = MG_SECTOR_BASE;
	int     nOSector = MG_SECTOR_BASE;
	int     iLocCode;
	int     i;
	CSound *pSound;
	int iSoundCode;
	int iLocationCode;
	const CLocTable * xpLocTable;
	int     nItems;
	long    lCrowns;
	int     j;
	bool    bSellItem = true;
	CItem   *pItem = nullptr;

	// check to see if this is true and if so then exit the function
	bExitDll = false ;  // assume no exit from DLL

	if ((xpGtlFrame == nullptr) || ((lpBfcMgr = xpGtlFrame->m_lpBfcMgr) == nullptr)) {
		iError = 100 ;
		goto cleanup ;
	}

	lpBfcMgr->m_bRestart = false ;
	lpBfcMgr->m_iFunctionCode = iCode ;

	lpBfcMgr->m_iMishMoshLoc = m_iMishMoshLoc ;

	lpHodj = &lpBfcMgr->m_cHodj ;
	lpPodj = &lpBfcMgr->m_cPodj ;

	for (xpXodj = m_xpXodjChain ; xpXodj ; xpXodj = xpXodj->m_xpXodjNext) {

		if (scumm_stricmp(xpXodj->m_szName, "Hodj") == 0)
			xpHXodj = xpXodj  ;
		else if (scumm_stricmp(xpXodj->m_szName, "Podj") == 0)
			xpPXodj = xpXodj  ;
	}

	if (!xpHXodj || !xpPXodj) {
		iError = 101 ;
		goto cleanup ;
	}

	for (xpXodj = xpHXodj, lpCodj = lpHodj, iCount = 0 ; ++iCount <= 2 ; xpXodj = xpPXodj, lpCodj = lpPodj) {

		lpCodj->m_bMoving = (xpXodj == m_xpCurXodj) ;
		lpCodj->m_bComputer = xpXodj->m_bComputer ;
		lpCodj->m_iNode = xpXodj->m_iCharNode ;
		lpNode = m_lpNodes + xpXodj->m_iCharNode ;
		lpCodj->m_iSectorCode = lpNode->m_iSector ;

		lpCodj->m_pInventory = xpXodj->m_pInventory ;
		lpCodj->m_pGenStore = xpXodj->m_pGenStore ;
		lpCodj->m_pBlackMarket = xpXodj->m_pBlackMarket ;

		lpCodj->m_iFurlongs = xpXodj->m_iFurlongs;
		lpCodj->m_nTurns = xpXodj->m_nTurns;

		// list of clue numbers for clues given by winning mini-game
		lpCodj->m_iWinInfoWon = xpXodj->m_iWinInfoWon ;
		lpCodj->m_iWinInfoNeed = xpXodj->m_iWinInfoNeed ;
		memcpy(lpCodj->m_iWinInfoTable, xpXodj->m_iWinInfoTable, sizeof(lpCodj->m_iWinInfoTable)) ;

		// list of clue numbers for clues given by farmer, etc.
		lpCodj->m_iSecondaryInfoWon = xpXodj->m_iSecondaryInfoWon ;
		lpCodj->m_iSecondaryInfoNeed = xpXodj->m_iSecondaryInfoNeed ;
		memcpy(lpCodj->m_iSecondaryInfoTable, xpXodj->m_iSecondaryInfoTable, sizeof(lpCodj->m_iSecondaryInfoTable)) ;

		// list of objects required to get Mish/Mosh
		lpCodj->m_iRequiredObjectsCount = xpXodj->m_iRequiredObjectsCount ;
		memcpy(lpCodj->m_iRequiredObjectsTable, xpXodj->m_iRequiredObjectsTable, sizeof(lpCodj->m_iRequiredObjectsTable)) ;
		lpCodj->m_iRequiredMoney = xpXodj->m_iRequiredMoney ;

		// list of secondary information location we still have to visit
		memcpy(lpCodj->m_iSecondaryLoc, xpXodj->m_iSecondaryLoc, sizeof(lpCodj->m_iSecondaryLoc)) ;

		//  lpCodj->m_bHaveMishMosh = xpXodj->m_bHaveMishMosh ;
		memcpy(lpCodj->m_iGameHistory, xpXodj->m_iGameHistory, sizeof(lpCodj->m_iGameHistory)) ;
	}

	xpXodj = m_xpCurXodj;

	lpBfcMgr->m_stGameStruct.bPlayingMetagame = true;
	lpBfcMgr->m_stGameStruct.bPlayingHodj = lpHodj->m_bMoving;
	lpBfcMgr->m_stGameStruct.lScore = 0;
	if (lpHodj->m_bMoving) {
		lpBfcMgr->m_stGameStruct.lCrowns = lpBfcMgr->m_cHodj.m_pInventory->FindItem(MG_OBJ_CROWN)->GetQuantity();
		lpBfcMgr->m_stGameStruct.nSkillLevel = lpBfcMgr->m_cHodj.m_iSkillLevel;
	} else {
		lpBfcMgr->m_stGameStruct.lCrowns = lpBfcMgr->m_cPodj.m_pInventory->FindItem(MG_OBJ_CROWN)->GetQuantity();
		lpBfcMgr->m_stGameStruct.nSkillLevel = lpBfcMgr->m_cPodj.m_iSkillLevel;
	}

	for (xpGameEntry = xpGameTable ; xpGameEntry->m_iGameCode && iCode != xpGameEntry->m_iGameCode ; ++xpGameEntry)
		;   // null loop body

	// if this game is in table
	if (xpGameEntry->m_iGameCode) {

		if (m_xpCurXodj->m_pThemeSound != nullptr) {
			m_xpCurXodj->m_pThemeSound->stop();
			delete m_xpCurXodj->m_pThemeSound;
			m_xpCurXodj->m_pThemeSound = nullptr;
		}
		xpGtlFrame->m_nReturnCode = xpGameEntry->m_iGameCode;

		// if either this player is the computer
		// or we're not in a DLL
		if (((m_xpCurXodj->m_bComputer) && (xpGameEntry->m_iGameCode != MG_GAME_CHALLENGE)) || !m_bGtlDll) {
			// generate random game result, and process
			ProcessGameResult(m_xpCurXodj, xpGameEntry->m_iGameCode, nullptr);

		} else {
			bExitDll = true ;   // end DLL and process game
		}
	}

	else
		switch (lpBfcMgr->m_iFunctionCode) {

		case MG_DLLX_QUIT:  /* quit game (game is over) */
			if (m_xpCurXodj->m_pThemeSound != nullptr) {
				m_xpCurXodj->m_pThemeSound->stop();
				delete m_xpCurXodj->m_pThemeSound;
				m_xpCurXodj->m_pThemeSound = nullptr;
			}
			bExitDll = true ;
			break ;

		case MG_VISIT_CASTLE:
		case MG_DLLX_HODJ_WINS:
		case MG_DLLX_PODJ_WINS:
			m_bGameOver = true ;    // meta game is over
			if (lpBfcMgr->m_iFunctionCode == MG_DLLX_HODJ_WINS || m_xpCurXodj->m_bHodj) {
				lpBfcMgr->m_iFunctionCode = MG_DLLX_HODJ_WINS ;

			} else {

				lpBfcMgr->m_iFunctionCode = MG_DLLX_PODJ_WINS ;
			}
			bExitDll = true ;
			break ;

		case MG_VISIT_MISHMOSH:

			iSoundCode = -1;
			switch ((m_lpNodes + m_xpCurXodj->m_iCharNode)->m_iLocationCode) {

			case MG_LOC_WITCHHOVEL:
				iSoundCode = MG_SOUND_WT2;
				break;

			case MG_LOC_WARLOCKLAIR:
				iSoundCode = MG_SOUND_WR2;
				break;

			case MG_LOC_BEARCAVE:
				iSoundCode = MG_SOUND_BR2;
				break;

			case MG_LOC_MERMAIDROCK:
				iSoundCode = MG_SOUND_MM2;
				break;

			case MG_LOC_PIRATECOVE:
				iSoundCode = MG_SOUND_PI3;
				break;

			case MG_LOC_HERMITTENT:
				iSoundCode = MG_SOUND_HM2;
				break;
			}

			if (iSoundCode >= 0) {
				assert(iSoundCode >= MG_SOUND_BASE && iSoundCode <= MG_SOUND_MAX);
				iSoundCode -= MG_SOUND_BASE;
				pSound = new CSound(xpGtlView, szGameSounds[iSoundCode], SOUND_WAVE | SOUND_QUEUE | SOUND_ASYNCH | SOUND_AUTODELETE);
				pSound->setDrivePath(lpMetaGameStruct->m_chCDPath);
				pSound->play();
				CSound::waitWaveSounds();
			}

			GainMishMosh(m_xpCurXodj, 0) ;
			if (xpXodj->m_bHodj)
				lpMetaGameStruct->m_cHodj.m_bHaveMishMosh = true;
			else
				lpMetaGameStruct->m_cPodj.m_bHaveMishMosh = true;

			pSound = new CSound(xpGtlView, ".\\sound\\rsc1.wav", SOUND_WAVE | SOUND_QUEUE | SOUND_AUTODELETE);
			pSound->setDrivePath(lpMetaGameStruct->m_chCDPath);
			pSound->play();
			pSound = new CSound(xpGtlView, ".\\sound\\rsc2.wav", SOUND_WAVE | SOUND_QUEUE | SOUND_AUTODELETE);
			pSound->setDrivePath(lpMetaGameStruct->m_chCDPath);
			pSound->play();
			pSound = new CSound(xpGtlView, ".\\sound\\rsc3.wav", SOUND_WAVE | SOUND_QUEUE | SOUND_ASYNCH | SOUND_AUTODELETE);
			pSound->setDrivePath(lpMetaGameStruct->m_chCDPath);
			pSound->play();
			break ;

		case MG_DLLX_SAVE:  /* save game (game is not over) */
			bExitDll = true ;
			break ;

		case MG_DLLX_RESTORE:  /* restore game (game is not over) */
			bExitDll = true ;
			break ;

		case MG_DLLX_COLOR: /* color narration or booby trap */

			m_cBgbMgr.CacheOptimize(0);

			// find the other player
			//
			pOtherXodj = m_xpXodjChain;
			if (pOtherXodj == m_xpCurXodj)
				pOtherXodj = m_xpCurXodj->m_xpXodjNext;

			nPSector = (m_lpNodes + m_xpCurXodj->m_iCharNode)->m_iSector;
			nOSector = (m_lpNodes + pOtherXodj->m_iCharNode)->m_iSector;

			if (!bJustReturned) {

				nItems = m_xpCurXodj->m_pInventory->ItemCount();
				lCrowns = m_xpCurXodj->m_pInventory->FindItem(MG_OBJ_CROWN)->GetQuantity();

				i = DoEncounter((CWnd *)pMainWindow, m_cBgbMgr.m_xpGamePalette,
				                m_xpCurXodj->m_bHodj, m_xpCurXodj->m_pInventory,
				                m_xpCurXodj->m_pBlackMarket, m_xpCurXodj->m_pGenStore,
				                ENC_BOOBYTRAP, m_xpCurXodj->m_iFurlongs,
				                nPSector, nOSector,
				                lpMetaGameStruct->m_bTraps);

				// if this is a transport code
				//
				if (i >= MG_SECTOR_BASE && i <= MG_SECTOR_MAX) {
					CheckForTransport(m_xpCurXodj, i);

					// current player gains a turn
					//
				} else if (i == ENC_GAIN_TURN) {

					m_xpCurXodj->m_nTurns++;
					pOtherXodj->m_nTurns = 0;

					// current player loses a turn (same as other player gaining turn)
					//
				} else if (i == ENC_LOSE_TURN) {

					if (--m_xpCurXodj->m_nTurns < 0)
						pOtherXodj->m_nTurns++;
				}

				if ((nItems != m_xpCurXodj->m_pInventory->ItemCount()) || (lCrowns != m_xpCurXodj->m_pInventory->FindItem(MG_OBJ_CROWN)->GetQuantity())) {
					m_xpCurXodj->m_iTargetLocation = 0;
				}
			}

			bExitDll = false ;
			break ;

		//
		// zoom map
		//
		case MG_DLLX_ZOOM: {
			m_cBgbMgr.CacheOptimize(3000000L);
			m_cBgbMgr.PauseAnimations();
			CMapDialog dlgZoomMap(pMainWindow,
			                      m_cBgbMgr.m_xpGamePalette,
			                      xpHXodj->m_lpcCharSprite->m_crPosition,
			                      xpPXodj->m_lpcCharSprite->m_crPosition);
			m_cBgbMgr.ResumeAnimations();
		}
		bExitDll = false;
		break;


		case MG_DLLX_INVENTORY: /* display inventory */
			m_cBgbMgr.CacheOptimize(3000000L);
			pMainWindow->ShowInventory(m_cBgbMgr.m_xpGamePalette);

			bExitDll = false ;
			break ;

		case MG_DLLX_SCROLL:    /* display inventory */
			if (lpMetaGameStruct->m_stGameStruct.bMusicEnabled) {
				if (m_xpCurXodj->m_pThemeSound != nullptr) {
					//m_xpCurXodj->m_pThemeSound->Pause();
				}
			}
			bExitDll = false ;
			m_cBgbMgr.CacheOptimize(3000000L);
			m_cBgbMgr.PauseAnimations();
			pMainWindow->ShowOptions(m_cBgbMgr.m_xpGamePalette);
			m_cBgbMgr.ResumeAnimations();
			if (bExitMetaDLL) {
				bExitDll = true;
				xpGtlFrame->m_nReturnCode = MG_DLLX_QUIT;
				lpMetaGameStruct->m_iFunctionCode = MG_DLLX_QUIT;
				break;
			}
			if (lpMetaGameStruct->m_stGameStruct.bMusicEnabled) {

				if (m_xpCurXodj->m_pThemeSound != nullptr) {
					//m_xpCurXodj->m_pThemeSound->Resume();

				} else {

					m_xpCurXodj->m_pThemeSound = new CSound(m_xpGtlView, m_xpCurXodj->m_pszThemeFile, SOUND_MIDI | SOUND_LOOP | SOUND_DONT_LOOP_TO_END);
					m_xpCurXodj->m_pThemeSound->midiLoopPlaySegment(m_xpCurXodj->m_nThemeStart, m_xpCurXodj->m_nThemeEnd);
				}

			} else {
				if (m_xpCurXodj->m_pThemeSound != nullptr) {
					m_xpCurXodj->m_pThemeSound->stop();
					delete m_xpCurXodj->m_pThemeSound;
					m_xpCurXodj->m_pThemeSound = nullptr;
				}
			}
			break ;

		case MG_VISIT_PAWNSHOP:
		case MG_DLLX_PAWNSHOP: /* trade at pawn shop */

			if (m_xpCurXodj->m_bComputer == false) {
				m_cBgbMgr.CacheOptimize(3000000L);
				lpBfcMgr->m_iFunctionCode = MG_DLLX_PAWNSHOP ;
				pMainWindow->ShowInventory(m_cBgbMgr.m_xpGamePalette, 3);
			} else {
				for (i = 0; i < xpXodj->m_pInventory->ItemCount(); i++) {
					pItem = xpXodj->m_pInventory->FetchItem(i);
					bSellItem = true;
					if ((pItem->m_nID != MG_OBJ_CROWN) &&
					        (pItem->m_nID != MG_OBJ_HODJ_NOTEBOOK) &&
					        (pItem->m_nID != MG_OBJ_PODJ_NOTEBOOK) &&
					        (pItem->m_nID != MG_OBJ_MISH) &&
					        (pItem->m_nID != MG_OBJ_MOSH)) {


						for (j = 0; j < xpXodj->m_iRequiredObjectsCount; j++) {
							if (xpXodj->m_iRequiredObjectsTable[j] == pItem->m_nID) {
								bSellItem = false;
								break;
							}
						}

						if (bSellItem) {
							xpXodj->m_pInventory->RemoveItem(pItem);
							if (ProbableTrue(50)) {
								xpXodj->m_pGenStore->AddItem(pItem);
							} else {
								xpXodj->m_pBlackMarket->AddItem(pItem);
							}
							xpXodj->m_pInventory->FindItem(MG_OBJ_CROWN)->SetQuantity((long)pItem->GetValue());
						}
					}
					pItem = nullptr;
				}
			}
			bExitDll = false ;
			break ;

		case MG_VISIT_GENERALSTR:
		case MG_DLLX_GENERALSTR:   /* buy from general store */

			// GTB if no objects in store then don't invoke
			//
			lpBfcMgr->m_iFunctionCode = MG_DLLX_GENERALSTR ;

			{
				bool    bCantBuy = true;
				int     nWhichObjToBuy = 0;

				if (xpXodj->m_pGenStore->ItemCount() > 0) {
					bCantBuy = false;
				}
				if (bCantBuy == false) {

					if (m_xpCurXodj->m_bComputer == false) {
						m_cBgbMgr.CacheOptimize(3000000L);
						pMainWindow->ShowInventory(m_cBgbMgr.m_xpGamePalette, 2);
					} else {

						lpMetaGameStruct->m_bVisitedStore = true;

						if (xpXodj->m_iWinInfoWon > 0) {
							bool    bHasItem = false;
							int     l = 0;

							for (i = 0; i < xpXodj->m_iWinInfoWon; i++) {
								int     k = xpXodj->m_iWinInfoTable[i];

								l = 0;
								if (xpXodj->m_bHodj) {
									l = CMgStatic::cHodjClueTable[k].m_iArgument;
								} else {
									l = CMgStatic::cPodjClueTable[k].m_iArgument;
								}

								if ((l >= MG_OBJ_BASE) && (l <= MG_OBJ_MAX)) {
									CItem   *item = xpXodj->m_pInventory->FindItem(l);

									if (item != nullptr) {
										if (item->GetQuantity() > 0) {
											bHasItem = true;
										}
									} else {
										bHasItem = false;
										break;
									}
								} else {
									bHasItem = true;
								}
							}
							if (bHasItem == false) {
								CItem   *item = xpXodj->m_pGenStore->FindItem(l);
								if (item != nullptr) {
									if (item->GetQuantity() > 0) {
										if (item->GetValue() < xpXodj->m_pInventory->FindItem(MG_OBJ_CROWN)->GetQuantity()) {
											long m = xpXodj->m_pInventory->FindItem(MG_OBJ_CROWN)->GetQuantity() - item->GetValue();
											// Buy item
											xpXodj->m_pInventory->FindItem(MG_OBJ_CROWN)->SetQuantity(m);
											xpXodj->m_pGenStore->RemoveItem(item);
											xpXodj->m_pInventory->AddItem(item);
											bHasItem = true;
										}
									}
								}
								if (bHasItem == false) {
									if (brand() % 2) {
										item = xpXodj->m_pGenStore->FetchItem(nWhichObjToBuy);
										long m = xpXodj->m_pInventory->FindItem(MG_OBJ_CROWN)->GetQuantity() - item->GetValue();
										// Buy item
										xpXodj->m_pInventory->FindItem(MG_OBJ_CROWN)->SetQuantity(m);
										xpXodj->m_pGenStore->RemoveItem(item);
										xpXodj->m_pInventory->AddItem(item);
										bHasItem = true;
									}
								}
							}
						}
					}
				}
			}
			bExitDll = false ;
			break ;

		case MG_VISIT_BLACKMARKET:
		case MG_DLLX_BLACKMARKET:
			// GTB if no objects in store then don't invoke
			//
			lpBfcMgr->m_iFunctionCode = MG_DLLX_BLACKMARKET ;

			{
				bool    bCantBuy = true;
				int     nWhichObjToBuy = 0;

				if (xpXodj->m_pBlackMarket->ItemCount() > 0) {
					bCantBuy = false;
				}
				if (bCantBuy == false) {

					if (m_xpCurXodj->m_bComputer == false) {
						m_cBgbMgr.CacheOptimize(3000000L);
						pMainWindow->ShowInventory(m_cBgbMgr.m_xpGamePalette, 4);
					} else {

						lpMetaGameStruct->m_bVisitedStore = true;

						if (xpXodj->m_iWinInfoWon > 0) {
							bool    bHasItem = false;
							int     l = 0;

							for (i = 0; i < xpXodj->m_iWinInfoWon; i++) {
								int     k = xpXodj->m_iWinInfoTable[i];

								l = 0;
								if (xpXodj->m_bHodj) {
									l = CMgStatic::cHodjClueTable[k].m_iArgument;
								} else {
									l = CMgStatic::cPodjClueTable[k].m_iArgument;
								}

								if ((l >= MG_OBJ_BASE) && (l <= MG_OBJ_MAX)) {
									pItem = xpXodj->m_pInventory->FindItem(l);

									if (pItem != nullptr) {
										if (pItem->GetQuantity() > 0) {
											bHasItem = true;
										}
									} else {
										bHasItem = false;
										break;
									}
								} else {
									bHasItem = true;
								}
							}
							if (bHasItem == false) {
								pItem = xpXodj->m_pBlackMarket->FindItem(l);
								if (pItem != nullptr) {
									if (pItem->GetQuantity() > 0) {
										if (pItem->GetValue() < xpXodj->m_pInventory->FindItem(MG_OBJ_CROWN)->GetQuantity()) {
											long m = xpXodj->m_pInventory->FindItem(MG_OBJ_CROWN)->GetQuantity() - pItem->GetValue();
											// Buy item
											xpXodj->m_pInventory->FindItem(MG_OBJ_CROWN)->SetQuantity(m);
											xpXodj->m_pBlackMarket->RemoveItem(pItem);
											xpXodj->m_pInventory->AddItem(pItem);
											bHasItem = true;
										}
									}
								}
								if (bHasItem == false) {
									if (brand() % 2) {
										pItem = xpXodj->m_pBlackMarket->FetchItem(nWhichObjToBuy);
										long m = xpXodj->m_pInventory->FindItem(MG_OBJ_CROWN)->GetQuantity() - pItem->GetValue();
										// Buy item
										xpXodj->m_pInventory->FindItem(MG_OBJ_CROWN)->SetQuantity(m);
										xpXodj->m_pBlackMarket->RemoveItem(pItem);
										xpXodj->m_pInventory->AddItem(pItem);
										bHasItem = true;
									}
								}
							}
						}
					}
				}
			}
			bExitDll = false ;
			break ;

		case MG_VISIT_TRADINGPOST:
		case MG_DLLX_TRADINGPOST:
			lpBfcMgr->m_iFunctionCode = MG_DLLX_TRADINGPOST ;
			bExitDll = false ;
			break ;

		//
		// run spinner
		//
		case MG_DLLX_SPINNER: {

			if (m_xpCurXodj->m_pThemeSound != nullptr) {
				m_xpCurXodj->m_pThemeSound->pause();
			}
			m_cBgbMgr.PauseAnimations();
			m_cBgbMgr.CacheOptimize(0);

			CDC         *pDC = nullptr;
			if ((pDC = gpMyView->GetDC()) != nullptr) {

				m_cBgbMgr.m_xpBsuSet->PrepareDc(pDC, true);
				if ((pSpinner = new CSpinner(xpGtlView, pDC, 0, 0, m_xpCurXodj->m_bHodj)) != nullptr) {
					point = pDC->GetViewportOrg();
					point.x = abs(point.x);
					point.y = abs(point.y);
					m_xpCurXodj->m_iFurlongs = pSpinner->Animate(point.x + 240, point.y + (true ? 50 : 480 - 85 - 40));
					SetFurlongs(m_xpCurXodj);
					delete pSpinner;
				}
				m_cBgbMgr.m_xpBsuSet->PrepareDc(pDC, false);
				gpMyView->ReleaseDC(pDC);

				// find the other player
				//
				pOtherXodj = m_xpXodjChain;
				if (pOtherXodj == m_xpCurXodj)
					pOtherXodj = m_xpCurXodj->m_xpXodjNext;

				nPSector = (m_lpNodes + m_xpCurXodj->m_iCharNode)->m_iSector;
				nOSector = (m_lpNodes + pOtherXodj->m_iCharNode)->m_iSector;

				// don't play Narrations if on special nodes
				//
				iLocCode = (m_lpNodes + m_xpCurXodj->m_iCharNode)->m_iLocationCode;
				if (iLocCode < MG_LOC_BASE || iLocCode > MG_LOC_MAX) {

					DoEncounter((CWnd *)pMainWindow, m_cBgbMgr.m_xpGamePalette,
					            m_xpCurXodj->m_bHodj,
					            m_xpCurXodj->m_pInventory,
					            m_xpCurXodj->m_pBlackMarket,
					            m_xpCurXodj->m_pGenStore,
					            ENC_NARRATION, m_xpCurXodj->m_iFurlongs,
					            nPSector, nOSector,
					            lpMetaGameStruct->m_bTraps);
				}
			}

			m_cBgbMgr.ResumeAnimations();
			if (m_xpCurXodj->m_pThemeSound != nullptr) {
				m_xpCurXodj->m_pThemeSound->resume();
			}

			bExitDll = false ;
			break ;
		}

		case MG_DLLX_INFO:      /* provide a piece of info */
		case MG_VISIT_INFO:
			iLocationCode = (m_lpNodes + m_xpCurXodj->m_iCharNode)->m_iLocationCode;

			lpBfcMgr->m_iFunctionCode = MG_DLLX_INFO ;

			iSoundCode = -1;
			switch (iLocationCode) {

			case MG_LOC_WOODCUTTER:
				iSoundCode = MG_SOUND_WC2;
				break;

			case MG_LOC_POSTOFFICE:
				iSoundCode = MG_SOUND_PM2;
				break;

			case MG_LOC_MANSION:
				iSoundCode = MG_SOUND_AC2;
				break;

			case MG_LOC_FARMHOUSE:
				iSoundCode = MG_SOUND_FM2;
				break;
			}

			if ((iSoundCode >= 0) && !xpXodj->m_bComputer) {
				assert(iSoundCode >= MG_SOUND_BASE && iSoundCode <= MG_SOUND_MAX);
				iSoundCode -= MG_SOUND_BASE;
				pSound = new CSound(xpGtlView, szGameSounds[iSoundCode], SOUND_WAVE | SOUND_QUEUE | SOUND_ASYNCH | SOUND_AUTODELETE);
				pSound->setDrivePath(lpMetaGameStruct->m_chCDPath);
				pSound->play();
			}

			// divulge secondary information
			DivulgeInformation(xpXodj, true) ;

			xpLocTable = CMgStatic::FindLoc(iLocationCode);
			if (xpLocTable->m_iCost > 0) {
				char blurb[128];
				Common::sprintf_s(blurb, "%d %s for the clue.", xpLocTable->m_iCost, (xpLocTable->m_iCost == 1 ? "crown" : "crowns"));
				C1ButtonDialog dlg1Button((CWnd *)pMainWindow, m_cBgbMgr.m_xpGamePalette, "&OK", (m_xpCurXodj->m_bHodj ? "Hodj generously offers" : "Podj generously offers"), blurb);
				dlg1Button.DoModal();
				m_xpCurXodj->m_pInventory->DiscardItem(MG_OBJ_CROWN, xpLocTable->m_iCost);
			}

			CSound::waitWaveSounds();

			bExitDll = false ;
			break;

		// GTB set special move flag
		//
		case MG_VISIT_BOAT1:
		case MG_VISIT_BOAT2:
		case MG_VISIT_BOAT3:
		case MG_VISIT_SKY1:
		case MG_VISIT_SKY2:
		case MG_VISIT_CAR1:
		case MG_VISIT_OASIS:
		case MG_VISIT_SECRET1:
		case MG_VISIT_SECRET2:

			// GTB do special movement here after I've returned from the spinner.
			//
			pCurPlayer = &lpMetaGameStruct->m_cPodj;
			if (m_xpCurXodj->m_bHodj) {
				pCurPlayer = &lpMetaGameStruct->m_cHodj;
			}

			if (pCurPlayer->m_iSpecialTravelCode == MG_VISIT_BOAT1) {

				C2ButtonDialog dlg2Button((CWnd *)pMainWindow, m_cBgbMgr.m_xpGamePalette, "&Upstream", "&Downstream", " ", "Which direction?");

				if (dlg2Button.DoModal() == CBUTTON1) {
					pCurPlayer->m_iSpecialTravelCode = MG_VISIT_BOAT4;
				}
			}

			// move player between special travel nodes
			// and set players new location
			//
			Sleep(750);
			xpXodj->m_iCharNode = DoSpecialTravel(pCurPlayer->m_iSpecialTravelCode, xpXodj->m_bHodj);

			// Reposition characters if they are on the same node
			PositionCharacters();

			//pCurPlayer->m_iSpecialTravelCode = MG_VISIT_NONE;

			break;

		default:
			bExitDll = false ;

			break ;
		}

	// if exiting DLL
	//
	if (bExitDll) {

		CSound::waitWaveSounds();

		if (m_bGtlDll)          // if we're in a DLL
			xpGtlFrame->PostMessage(WM_COMMAND, ID_CALL_EXIT) ;
		else
			bExitDll = false ;
	}

cleanup:

	xpGtlFrame->m_bExitDll = bExitDll ; // set frame flag for
	bExitMetaDLL = bExitDll;            // exiting DLL

	JXELEAVE(CGtlData::InitInterface) ;
	RETURN(iError != 0) ;
}

//* CGtlData::ReturnFromInterface -- reset everything after interface return
bool CGtlData::ReturnFromInterface()
// returns: true if error, false otherwise
{
	JXENTER(CGtlData::ReturnFromInterface) ;
	int iError = 0 ;        // error code
	CXodj * xpXodj, *xpHXodj = nullptr, *xpPXodj = nullptr ;    // hodj/podj
	CHodjPodj FAR * lpCodj, FAR * lpHodj, FAR * lpPodj ;
	// interface hodj/podj
	int iCount ;
	CGtlFrame * xpGtlFrame = m_xpcGtlDoc->m_xpGtlFrame ;
	CBfcMgr FAR * lpBfcMgr ;
	//CNode FAR * lpNode ;
	int iFunctionCode ;

	if ((xpGtlFrame == nullptr) || ((lpBfcMgr = xpGtlFrame->m_lpBfcMgr) == nullptr)) {
		iError = 100 ;
		goto cleanup ;
	}

	// if any information is valid
	//
	if (lpBfcMgr->m_bRestart || lpBfcMgr->m_bInventories) {
		lpHodj = &lpBfcMgr->m_cHodj ;
		lpPodj = &lpBfcMgr->m_cPodj ;

		for (xpXodj = m_xpXodjChain ; xpXodj ; xpXodj = xpXodj->m_xpXodjNext) {
			if (scumm_stricmp(xpXodj->m_szName, "Hodj") == 0)
				xpHXodj = xpXodj  ;
			else if (scumm_stricmp(xpXodj->m_szName, "Podj") == 0)
				xpPXodj = xpXodj  ;
		}

		if (!xpHXodj || !xpPXodj) {
			iError = 101 ;
			goto cleanup ;
		}

		for (xpXodj = xpHXodj, lpCodj = lpHodj, iCount = 0 ; ++iCount <= 2 ; xpXodj = xpPXodj, lpCodj = lpPodj) {

			xpXodj->m_pInventory = lpCodj->m_pInventory ;
			xpXodj->m_pGenStore = lpCodj->m_pGenStore ;
			xpXodj->m_pBlackMarket = lpCodj->m_pBlackMarket ;
			xpXodj->m_iFurlongs = lpCodj->m_iFurlongs;
			xpXodj->m_nTurns = lpCodj->m_nTurns;

			if (lpBfcMgr->m_bRestart) {

				if (lpCodj->m_bMoving)
					m_xpCurXodj = xpXodj;

				xpXodj->m_bComputer = lpCodj->m_bComputer ;
				xpXodj->m_iCharNode = lpCodj->m_iNode ;
				//lpNode = m_lpNodes + xpXodj->m_iCharNode ;

				// list of clue numbers for clues given by winning mini-game
				xpXodj->m_iWinInfoWon = lpCodj->m_iWinInfoWon ;
				xpXodj->m_iWinInfoNeed = lpCodj->m_iWinInfoNeed ;
				memcpy(xpXodj->m_iWinInfoTable, lpCodj->m_iWinInfoTable, sizeof(lpCodj->m_iWinInfoTable)) ;

				// list of clue numbers for clues given by farmer, etc.
				xpXodj->m_iSecondaryInfoWon = lpCodj->m_iSecondaryInfoWon ;
				xpXodj->m_iSecondaryInfoNeed = lpCodj->m_iSecondaryInfoNeed ;
				memcpy(xpXodj->m_iSecondaryInfoTable, lpCodj->m_iSecondaryInfoTable, sizeof(lpCodj->m_iSecondaryInfoTable)) ;

				// list of objects required to get Mish/Mosh
				xpXodj->m_iRequiredObjectsCount = lpCodj->m_iRequiredObjectsCount ;
				memcpy(xpXodj->m_iRequiredObjectsTable, lpCodj->m_iRequiredObjectsTable, sizeof(lpCodj->m_iRequiredObjectsTable)) ;
				xpXodj->m_iRequiredMoney = lpCodj->m_iRequiredMoney ;

				// list of secondary information location we still have to visit
				memcpy(xpXodj->m_iSecondaryLoc, lpCodj->m_iSecondaryLoc, sizeof(lpCodj->m_iSecondaryLoc)) ;

				memcpy(xpXodj->m_iGameHistory, lpCodj->m_iGameHistory, sizeof(lpCodj->m_iGameHistory)) ;
			}
		}

		xpXodj = m_xpCurXodj;

		if (lpBfcMgr->m_bRestart) {

			m_iMishMoshLoc = lpBfcMgr->m_iMishMoshLoc ;
			iFunctionCode = lpBfcMgr->m_iFunctionCode ;

		} else {

			// no valid function code unless we're restarting
			iFunctionCode = 0;
		}

		// Take any necessary terminating action after a DLL exit
		// has been completed.  This can be switching players, or
		// setting the "gather information" flag, if necessary

		// if we've just played a mini-game
		if (iFunctionCode >= MG_GAME_BASE && iFunctionCode <= MG_GAME_MAX) {

			// generate random game result, and process
			ProcessGameResult(m_xpCurXodj, iFunctionCode, &lpBfcMgr->m_stGameStruct) ;

			// set this flag because collection of objects
			// or information may change
			m_xpCurXodj->m_bGatherInformation = true ;
		}

		else if (iFunctionCode) {

			switch (iFunctionCode) {

			case MG_DLLX_QUIT:  /* quit game (game is over) */
				break ;

			case MG_VISIT_CASTLE:
			case MG_DLLX_HODJ_WINS:
			case MG_DLLX_PODJ_WINS:
				break ;

			case MG_DLLX_SAVE:  /* save game (game is not over) */
				break ;

			case MG_DLLX_COLOR: /* color narration or booby trap */

				// *** xpXodj->m_bGatherInformation = true ;
				// set this flag because collection of objects
				// may change
				break ;

			case MG_DLLX_ZOOM:  /* zoom map */
				break ;

			case MG_DLLX_INVENTORY: /* display inventory */
				break ;

			case MG_DLLX_SCROLL:    /* display inventory */
				break ;

			case MG_VISIT_GENERALSTR:
			case MG_DLLX_GENERALSTR:   /* buy from general store */
				m_xpCurXodj->m_bGatherInformation = true ;
				// set this flag because collection of objects
				// may change
				break ;

			case MG_VISIT_PAWNSHOP:
			case MG_DLLX_PAWNSHOP: /* trade at pawn shop */
				m_xpCurXodj->m_bGatherInformation = true ;
				// set this flag because collection of objects
				// may change
				break ;

			case MG_DLLX_SPINNER:   /* run spinner */
				break ;

			case MG_DLLX_INFO:      /* provide a piece of info */
			case MG_VISIT_INFO:
				break ;

			default:
				break ;
			}
		}

		// make sure we don't
		// do this again
		lpBfcMgr->m_bRestart = lpBfcMgr->m_bInventories = false;
		AdjustToView((CGtlView *)m_cBgbMgr.m_xpcView);
	}

cleanup:

	JXELEAVE(CGtlData::ReturnFromInterface) ;
	RETURN(iError != 0) ;
}


//* CGtlData::TakeIneligibleAction -- take action if character is at
//      a location but is not eligible to perform function
bool CGtlData::TakeIneligibleAction(CXodj *xpXodj, int iFunctionCode, int iLocationCode)
// xpXodj -- character / player
// iFunctionCode -- MG_GAME_xxxx or MG_VISIT_xxxx -- function that
//      the player was ineligible for
// returns: true if error, false otherwise
{
	JXENTER(CGtlData::TakeIneligibleAction) ;
	int iError = 0 ;        // error code
	int iSoundCode = -1;
	CSound  *pSound;
	CXodj *pOpponent;
	CGtlView * xpGtlView = (CGtlView *)m_cBgbMgr.m_xpcView ;

	// if ineligible to play a mini-game
	if (iFunctionCode >= MG_GAME_BASE && iFunctionCode <= MG_GAME_MAX) {

		switch (iLocationCode) {

		case MG_LOC_TEMPLE:
		case MG_LOC_SHERIFF:
		case MG_LOC_BOARDING:
		case MG_LOC_ARTISTCOTT:
		case MG_LOC_BARBERSHOP:
		case MG_LOC_TAVERN:
		case MG_LOC_INN:
		case MG_LOC_FISHMARKET:
			iSoundCode = (xpXodj->m_bHodj ? MG_SOUND_OLN1 : MG_SOUND_OLN2);
			break;

		case MG_LOC_CORRAL:
			iSoundCode = MG_SOUND_OLN9;
			break;

		case MG_LOC_BEAVER:
		case MG_LOC_LIGHTHOUSE:
		case MG_LOC_AMPHI:
		case MG_LOC_BARN:
		case MG_LOC_CRYPT:
		case MG_LOC_ANCIENTRUIN:
			iSoundCode = (xpXodj->m_bHodj ? MG_SOUND_OLN7 : MG_SOUND_OLN8);
			break;

		case MG_LOC_TOURNAMENT:
		case MG_LOC_HEDGEMAZE:
		case MG_LOC_FERRISWHEEL:
			iSoundCode = (xpXodj->m_bHodj ? MG_SOUND_OLN3 : MG_SOUND_OLN4);
			break;

		case MG_LOC_MINE:
			iSoundCode = (xpXodj->m_bHodj ? MG_SOUND_OLN5 : MG_SOUND_OLN6);
			break;
		}
	} else {

		// non-game function code
		switch (iFunctionCode) {

		case MG_VISIT_CASTLE:
			break ;

		case MG_VISIT_GENERALSTR:
			iSoundCode = (xpXodj->m_bHodj ? MG_SOUND_OLN1 : MG_SOUND_OLN2);
			break ;

		case MG_VISIT_PAWNSHOP:
			iSoundCode = (xpXodj->m_bHodj ? MG_SOUND_OLN1 : MG_SOUND_OLN2);
			break ;

		case MG_VISIT_TRADINGPOST:
			iSoundCode = (xpXodj->m_bHodj ? MG_SOUND_OLN1 : MG_SOUND_OLN2);
			break;

		case MG_VISIT_BLACKMARKET:
			iSoundCode = (xpXodj->m_bHodj ? MG_SOUND_OLN1 : MG_SOUND_OLN2);
			break;

		case MG_VISIT_BOAT1:
		case MG_VISIT_BOAT2:
		case MG_VISIT_BOAT3:
		case MG_VISIT_BOAT4:

			xpXodj->m_iFurlongs = 0 ;

			switch (brand() % 6) {
			case 0:
				iSoundCode = (xpXodj->m_bHodj ? MG_SOUND_BB66 : MG_SOUND_BB67);
				break;

			case 1:
				iSoundCode = MG_SOUND_BB61;
				break;

			case 2:
				iSoundCode = MG_SOUND_BB62;
				break;

			case 3:
				iSoundCode = MG_SOUND_BB63;
				break;

			case 4:
				iSoundCode = MG_SOUND_BB64;
				break;

			case 5:
				iSoundCode = MG_SOUND_BB65;
				break;

			default:
				assert(0);
				break;
			}
			break;

		case MG_VISIT_SKY1:
		case MG_VISIT_SKY2:
			iSoundCode = (xpXodj->m_bHodj ? MG_SOUND_TRAN7 : MG_SOUND_TRAN8);
			break;

		case MG_VISIT_MISHMOSH:

			//
			// if Hodj or Podj already has Mish/Mosh, then play the
			// "Unseen force prevents Hodj from entering" sound
			//

			// find opponent
			//
			if ((pOpponent = xpXodj->m_xpXodjNext) == nullptr)
				pOpponent = m_xpXodjChain;

			if (GetGameObjectCount(xpXodj, MG_OBJ_MISH) || GetGameObjectCount(pOpponent, MG_OBJ_MISH)) {
				iSoundCode = (xpXodj->m_bHodj ? MG_SOUND_OLN7 : MG_SOUND_OLN8);

			} else {

				switch (iLocationCode) {

				case MG_LOC_WITCHHOVEL:
					iSoundCode = MG_SOUND_WT1;
					break;

				case MG_LOC_WARLOCKLAIR:
					iSoundCode = MG_SOUND_WR1;
					break;

				case MG_LOC_BEARCAVE:
					iSoundCode = MG_SOUND_BR1;
					break;

				case MG_LOC_MERMAIDROCK:
					iSoundCode = MG_SOUND_MM1;
					break;

				case MG_LOC_PIRATECOVE:
					iSoundCode = (xpXodj->m_bHodj ? MG_SOUND_PI1 : MG_SOUND_PI2);
					break;

				case MG_LOC_HERMITTENT:
					iSoundCode = MG_SOUND_HM1;
					break;
				}
			}
			break;

		case MG_VISIT_INFO:
			switch (iLocationCode) {

			case MG_LOC_WOODCUTTER:
				if (bLacksMoney)
					iSoundCode = (xpXodj->m_bHodj ? MG_SOUND_UTP1 : MG_SOUND_UTP2);
				else
					iSoundCode = MG_SOUND_WC1;
				break;

			case MG_LOC_POSTOFFICE:
				if (bLacksMoney)
					iSoundCode = (xpXodj->m_bHodj ? MG_SOUND_UTP3 : MG_SOUND_UTP4);
				else
					iSoundCode = MG_SOUND_PM1;
				break;

			case MG_LOC_MANSION:
				if (bLacksMoney)
					iSoundCode = (xpXodj->m_bHodj ? MG_SOUND_UTP5 : MG_SOUND_UTP6);
				else
					iSoundCode = MG_SOUND_AC1;
				break;

			case MG_LOC_FARMHOUSE:
				if (bLacksMoney)
					iSoundCode = (xpXodj->m_bHodj ? MG_SOUND_UTP7 : MG_SOUND_UTP8);
				else
					iSoundCode = MG_SOUND_FM1;
				break;
			}

		default:
			break;
		}
	}

	if (iSoundCode >= 0) {
		iSoundCode -= MG_SOUND_BASE;
		pSound = new CSound(xpGtlView, szGameSounds[iSoundCode], SOUND_WAVE | SOUND_QUEUE | SOUND_ASYNCH | SOUND_AUTODELETE);
		pSound->setDrivePath(lpMetaGameStruct->m_chCDPath);
		pSound->play();
		CSound::waitWaveSounds();
		if ((iFunctionCode >= MG_GAME_BASE) && (iFunctionCode <= MG_GAME_MAX) &&
		        (CMgStatic::cGameTable[iFunctionCode - MG_GAME_BASE].m_iWinCode == MG_WIN_INFO)) {
			if ((xpXodj->m_iWinInfoWon == xpXodj->m_iWinInfoNeed) &&
			        (xpXodj->m_iSecondaryInfoWon == xpXodj->m_iSecondaryInfoNeed)) {
				C1ButtonDialog dlg1Button((CWnd *)pMainWindow, m_cBgbMgr.m_xpGamePalette, "&OK", (xpXodj->m_bHodj ? "Hodj has all the" : "Podj has all the"), "clues he needs!");
				dlg1Button.DoModal();
			} else if (xpXodj->m_iWinInfoWon == xpXodj->m_iWinInfoNeed) {
				C1ButtonDialog dlg1Button((CWnd *)pMainWindow, m_cBgbMgr.m_xpGamePalette, "&OK", (xpXodj->m_bHodj ? "Review Hodj's clues" : "Review Podj's clues"), "for what to do next.");
				dlg1Button.DoModal();
			}
		} else if (iFunctionCode == MG_VISIT_INFO) {
			if (bLacksMoney) {
				CSound::waitWaveSounds();
				const CLocTable *xpLocTable;
				//int  iLocationCode;
				char blurb[128];
				iLocationCode = (m_lpNodes + m_xpCurXodj->m_iCharNode)->m_iLocationCode;
				xpLocTable = CMgStatic::FindLoc(iLocationCode);
				Common::sprintf_s(blurb, (m_xpCurXodj->m_bHodj ? "Hodj needs %ld more" : "Podj needs %ld more"), xpLocTable->m_iCost - m_xpCurXodj->m_pInventory->FindItem(MG_OBJ_CROWN)->GetQuantity());
				C1ButtonDialog dlg1Button((CWnd *)pMainWindow, m_cBgbMgr.m_xpGamePalette, "&OK", blurb, "crowns for the clue.");
				dlg1Button.DoModal();
			} else if ((xpXodj->m_iWinInfoWon == xpXodj->m_iWinInfoNeed) &&
			           (xpXodj->m_iSecondaryInfoWon == xpXodj->m_iSecondaryInfoNeed)) {
				C1ButtonDialog dlg1Button((CWnd *)pMainWindow, m_cBgbMgr.m_xpGamePalette, "&OK", (xpXodj->m_bHodj ? "Hodj has all the" : "Podj has all the"), "clues he needs!");
				dlg1Button.DoModal();
			}
		}
	}


// cleanup:

	JXELEAVE(CGtlData::TakeIneligibleAction) ;
	RETURN(iError != 0) ;
}

//* CGtlData::DumpGameStatus -- (debugging) dump status of game
bool CGtlData::DumpGameStatus(int iOptionFlags)
// iOptionFlags -- combination of DUMPSTAT_xxxx
// returns: true if error, false otherwise
{
	JXENTER(CGtlData::DumpGameStatus) ;
	int iError = 0 ;        // error code

// cleanup:

	JXELEAVE(CGtlData::DumpGameStatus) ;
	RETURN(iError != 0) ;
}

//* CGtlData::ProcessGameResult -- process result of game, optionally generating a random win
bool CGtlData::ProcessGameResult(CXodj *xpXodj, int iGameCode, LPGAMESTRUCT lpGameStruct)
// xpXodj -- player
// iGameCode -- GM_GAME_xxxx -- game code
// lpGameStruct -- game structure; if nullptr, then generate random win
// returns: true if error, false otherwise
{
	JXENTER(CGtlData::ProcessGameResult) ;
	int iError = 0 ;            // error code
	bool bWin = false ;         // won game
	const CGameTable * xpGameEntry ;  // pointer to game table entry
	long lMoneyWon = 0 ;         // amount of money won
	int iWinCode ;              // MG_WIN_xxxx (win money, object, info)
	CSound *pSound = nullptr;
	int iSoundCode = -1;
	CGtlView * xpGtlView = (CGtlView *)m_cBgbMgr.m_xpcView ;

	// if not a legal game code
	//
	if ((xpGameEntry = CMgStatic::FindGame(iGameCode)) == nullptr) {
		iError = 100 ;
		goto cleanup ;
	}

	xpXodj->m_bGatherInformation = true ;
	iWinCode = xpGameEntry->m_iWinCode ;        // get type of game win

	// if generating random win
	if (!lpGameStruct) {
// OINK MOO
		if (xpXodj->m_bHodj) {
			if (lpMetaGameStruct->m_cHodj.m_iSkillLevel == SKILLLEVEL_LOW) {
				bWin = ProbableTrue(40);
			} else {
				if (lpMetaGameStruct->m_cHodj.m_iSkillLevel == SKILLLEVEL_MEDIUM) {
					bWin = ProbableTrue(60);
				} else {
					bWin = ProbableTrue(80);
				}
			}
		} else {
			if (lpMetaGameStruct->m_cPodj.m_iSkillLevel == SKILLLEVEL_LOW) {
				bWin = ProbableTrue(40);
			} else {
				if (lpMetaGameStruct->m_cPodj.m_iSkillLevel == SKILLLEVEL_MEDIUM) {
					bWin = ProbableTrue(60);
				} else {
					bWin = ProbableTrue(80);
				}
			}
		}

		//
		// GTB put in code here to process POKER loss of MONEY
		//

		if (iGameCode == MG_GAME_VIDEOPOKER) {
			long i;

			// if Podj has 0 crowns then he should not have played Poker
			assert(xpXodj->m_pInventory->FindItem(MG_OBJ_CROWN)->GetQuantity() != 0L);

			if (bWin) {
				bWin = false;
				i = xpXodj->m_pInventory->FindItem(MG_OBJ_CROWN)->GetQuantity();
				i += brand() % i;
				xpXodj->m_pInventory->FindItem(MG_OBJ_CROWN)->SetQuantity(i);
			} else {
				i = xpXodj->m_pInventory->FindItem(MG_OBJ_CROWN)->GetQuantity();
				i -= brand() % i;
				xpXodj->m_pInventory->FindItem(MG_OBJ_CROWN)->SetQuantity(i);
			}

			// generate win with probability 1/2
			//
		} else  {
			// if it's a money game
			//
			if (bWin && iWinCode == MG_WIN_MONEY) {
				// win from 1 to 20 crowns
				lMoneyWon = m_cGenUtil.RandomInteger(1, 20) ;
			}
		}

		// actual return from game
		//
	} else {

		// stop next booby trap
		m_bJustPlayedMiniGame = true;

		// mini-game specific win logic
		//
		iSoundCode = -1;
		switch (iGameCode) {

		case MG_GAME_ANAGRAMS:      /* O. Anagrams */
			break ;

		case MG_GAME_ARCHEROIDS:    /* A. Archeroids */
			bWin = ((lpGameStruct->lScore == 1) ? true : false);
			iSoundCode = (bWin ? MG_SOUND_AR2 : MG_SOUND_AR1);
			break ;

		case MG_GAME_ARTPARTS:      /* S. Art Parts */
			bWin = ((lpGameStruct->lScore == 1) ? true : false);
			iSoundCode = (bWin ? MG_SOUND_AP2 : MG_SOUND_AP1);
			break ;

		case MG_GAME_BARBERSHOP:    /* U. Barbershop Quintet */
			bWin = false;
			switch (lpGameStruct->nSkillLevel) {
			case SKILLLEVEL_LOW:
				if (lpGameStruct->lScore >= 56)
					bWin = true;
				break;
			case SKILLLEVEL_MEDIUM:
				if (lpGameStruct->lScore >= 58)
					bWin = true;
				break;
			case SKILLLEVEL_HIGH:
				if (lpGameStruct->lScore >= 60)
					bWin = true;
				break;
			}
			iSoundCode = (bWin ? MG_SOUND_BQ2 : MG_SOUND_BQ1);
			break ;

		case MG_GAME_BATTLEFISH:    /* X. Battlefish */
			bWin = ((lpGameStruct->lScore == 1) ? true : false);
			iSoundCode = (bWin ? (xpXodj->m_bHodj ? MG_SOUND_BF2 : MG_SOUND_BF3) : MG_SOUND_BF1);
			break ;

		case MG_GAME_BEACON:        /* H. Beacon */
			bWin = true;
			lMoneyWon = lpGameStruct->lScore;

			// determine which sound to play
			//
			iSoundCode = (xpXodj->m_bHodj ? MG_SOUND_BC3 : MG_SOUND_BC4);
			if (lpGameStruct->lScore < 5) {
				iSoundCode = (xpXodj->m_bHodj ? MG_SOUND_BC1 : MG_SOUND_BC2);
			} else if (lpGameStruct->lScore >= 10) {
				iSoundCode = MG_SOUND_BC5;
			}

			break ;

		case MG_GAME_CRYPTOGRAMS:   /* Q. Cryptograms */
			bWin = true;
			if (lpGameStruct->lScore < 24)
				lMoneyWon = 0;
			else
				lMoneyWon = (lpGameStruct->lScore - 24) / 3;

			// determine which sound to play
			//
			iSoundCode = MG_SOUND_CR3;
			if (lMoneyWon <= 0) {
				iSoundCode = (xpXodj->m_bHodj ? MG_SOUND_CR1 : MG_SOUND_CR2);
			} else if (lpGameStruct->lScore >= 100) {
				iSoundCode = (xpXodj->m_bHodj ? MG_SOUND_CR4 : MG_SOUND_CR5);
			}
			break ;

		case MG_GAME_DAMFURRY:      /* G. Dam Furry Animals */
			bWin = true;
			lMoneyWon = lpGameStruct->lScore / 3;

			// determine which sound to play
			//
			iSoundCode = (xpXodj->m_bHodj ? MG_SOUND_DF2 : MG_SOUND_DF3);
			if (lpGameStruct->lScore <= 0) {
				iSoundCode = MG_SOUND_DF1;
			} else if (lpGameStruct->lScore >= 12) {
				iSoundCode = (xpXodj->m_bHodj ? MG_SOUND_DF4 : MG_SOUND_DF5);
			}
			break ;

		case MG_GAME_EVERYTHING:    /* Y. Everything Under the Sun */
			break ;

		case MG_GAME_FUGE:  /* D. Fuge */

			bWin = true;
			lMoneyWon = (lpGameStruct->lScore - 3) / 2;

			// determine which sound to play
			//
			iSoundCode = (xpXodj->m_bHodj ? MG_SOUND_FG2 : MG_SOUND_FG3);
			if (lMoneyWon <= 0) {
				bWin = false;
				iSoundCode = MG_SOUND_FG1;
			} else if (lMoneyWon >= 25) {
				iSoundCode = (xpXodj->m_bHodj ? MG_SOUND_FG4 : MG_SOUND_FG5);
			}
			break;

		case MG_GAME_GARFUNKEL:     /* Garfunkel */
			bWin = false;
			switch (lpGameStruct->nSkillLevel) {
			case SKILLLEVEL_LOW:
				if (lpGameStruct->lScore >= 8)
					bWin = true;
				break;
			case SKILLLEVEL_MEDIUM:
				if (lpGameStruct->lScore >= 10)
					bWin = true;
				break;
			case SKILLLEVEL_HIGH:
				if (lpGameStruct->lScore >= 12)
					bWin = true;
				break;
			}

			// determine which sound to play
			//
			if (bWin)
				iSoundCode = MG_SOUND_GF3B;
			else
				iSoundCode = (xpXodj->m_bHodj ? MG_SOUND_GF1B : MG_SOUND_GF2B);
			break ;

		case MG_GAME_LIFE:  /* F. Life */
			bWin = false;
			switch (lpGameStruct->nSkillLevel) {
			case SKILLLEVEL_LOW:
				if (lpGameStruct->lScore >= 15)
					bWin = true;
				break;
			case SKILLLEVEL_MEDIUM:
				if (lpGameStruct->lScore >= 19)
					bWin = true;
				break;
			case SKILLLEVEL_HIGH:
				if (lpGameStruct->lScore >= 23)
					bWin = true;
				break;
			}
			iSoundCode = (bWin ? MG_SOUND_LF2 : MG_SOUND_LF1);
			break ;

		case MG_GAME_MANKALA:       /* W. Mankala */
			bWin = ((lpGameStruct->lScore == 1) ? true : false);

			iSoundCode = (bWin ? MG_SOUND_MK6 : MG_SOUND_MK1);
			break ;

		case MG_GAME_MAZEODOOM:     /* L. Maze o'Doom */
			bWin = ((lpGameStruct->lScore == 1) ? true : false);

			if (bWin) {
				iSoundCode = (xpXodj->m_bHodj ? MG_SOUND_MD3 : MG_SOUND_MD4);
			} else {
				iSoundCode = (xpXodj->m_bHodj ? MG_SOUND_MD1 : MG_SOUND_MD2);
			}
			break ;

		case MG_GAME_NOVACANCY:     /* M. No Vacancy */
			bWin = false;
			switch (lpGameStruct->nSkillLevel) {
			case SKILLLEVEL_LOW:
				if (lpGameStruct->lScore <= 8)
					bWin = true;
				break;
			case SKILLLEVEL_MEDIUM:
				if (lpGameStruct->lScore <= 6)
					bWin = true;
				break;
			case SKILLLEVEL_HIGH:
				if (lpGameStruct->lScore <= 4)
					bWin = true;
				break;
			}

			iSoundCode = (xpXodj->m_bHodj ? MG_SOUND_NV1 : MG_SOUND_NV2);
			if (bWin)
				iSoundCode = MG_SOUND_NV3;
			break ;

		case MG_GAME_PACRAT:        /* C. Pac-Rat */
			bWin = false;
			switch (lpGameStruct->nSkillLevel) {
			case SKILLLEVEL_LOW:
				if (lpGameStruct->lScore >= (350 * 1))
					bWin = true;
				break;
			case SKILLLEVEL_MEDIUM:
				if (lpGameStruct->lScore >= (350 * 6))
					bWin = true;
				break;
			case SKILLLEVEL_HIGH:
				if (lpGameStruct->lScore >= (350 * 12))
					bWin = true;
				break;
			}

			if (bWin) {
				iSoundCode = (xpXodj->m_bHodj ? MG_SOUND_PR3B : MG_SOUND_PR4B);
			} else {
				iSoundCode = (xpXodj->m_bHodj ? MG_SOUND_PR1B : MG_SOUND_PR2B);
			}
			break ;

		case MG_GAME_PEGGLEBOZ:     /* V. Peggleboz */
			iSoundCode = (xpXodj->m_bHodj ? MG_SOUND_PG3 : MG_SOUND_PG4);
			if (lpGameStruct->lScore > 1) {
				if (lMoneyWon >= 5) {
					iSoundCode = (xpXodj->m_bHodj ? MG_SOUND_PG5 : MG_SOUND_PG6);
				}
				bWin = true;
			} else {
				bWin = false;
				iSoundCode = (xpXodj->m_bHodj ? MG_SOUND_PG1 : MG_SOUND_PG2);
			}

			if (bWin) {
				switch (lpGameStruct->nSkillLevel) {
				case SKILLLEVEL_HIGH:
					switch (lpGameStruct->lScore) {
					case 25:
						lMoneyWon = 20;
						break;
					case 10:
						lMoneyWon = 16;
						break;
					case 5:
						lMoneyWon = 12;
						break;
					case 4:
						lMoneyWon = 8;
						break;
					case 3:
						lMoneyWon = 4;
						break;
					case 2:
						lMoneyWon = 2;
						break;
					}
					break;
				case SKILLLEVEL_MEDIUM:
					switch (lpGameStruct->lScore) {
					case 25:
						lMoneyWon = 25;
						break;
					case 10:
						lMoneyWon = 20;
						break;
					case 5:
						lMoneyWon = 15;
						break;
					case 4:
						lMoneyWon = 10;
						break;
					case 3:
						lMoneyWon = 6;
						break;
					case 2:
						lMoneyWon = 3;
						break;
					}
					break;
				case SKILLLEVEL_LOW:
					switch (lpGameStruct->lScore) {
					case 25:
						lMoneyWon = 30;
						break;
					case 10:
						lMoneyWon = 24;
						break;
					case 5:
						lMoneyWon = 18;
						break;
					case 4:
						lMoneyWon = 12;
						break;
					case 3:
						lMoneyWon = 8;
						break;
					case 2:
						lMoneyWon = 4;
						break;
					}
					break;
				}
			}
			break;

		case MG_GAME_RIDDLES:       /* T. Riddles */
			bWin = ((lpGameStruct->lScore == 1) ? true : false);

			if (bWin) {
				iSoundCode = (xpXodj->m_bHodj ? MG_SOUND_RR3 : MG_SOUND_RR4);
			} else {
				iSoundCode = (xpXodj->m_bHodj ? MG_SOUND_RR1 : MG_SOUND_RR2);
			}

			break ;

		case MG_GAME_ROULETTE:      /* J. Roulette */
			break ;

		case MG_GAME_SHOTMACHINE:   /* I. Shot Machine */
			break ;

		case MG_GAME_SPINBALL:      /* E. Spinball */
			break ;

		case MG_GAME_THGESNGGME:    /* N. TH GESNG GME */
			bWin = true;
			lMoneyWon = lpGameStruct->lScore / 8;

			// determine which sound to play
			//
			iSoundCode = (xpXodj->m_bHodj ? MG_SOUND_TG3 : MG_SOUND_TG4);
			if (lMoneyWon <= 0) {
				iSoundCode = (xpXodj->m_bHodj ? MG_SOUND_TG1 : MG_SOUND_TG2);
			} else if (lMoneyWon >= 10) {
				iSoundCode = (xpXodj->m_bHodj ? MG_SOUND_TG5 : MG_SOUND_TG6);
			}

			break ;

		case MG_GAME_TRIVIA:        /* R. Trivia */
			break ;

		case MG_GAME_VIDEOPOKER:    /* K. Video Poker */
			bWin = false;
			xpXodj->m_pInventory->FindItem(MG_OBJ_CROWN)->SetQuantity(lpGameStruct->lCrowns);
			iSoundCode = (xpXodj->m_bHodj ? MG_SOUND_PK1 : MG_SOUND_PK2);
			break ;

		case MG_GAME_WORDSEARCH:    /* P. Word Search */
			bWin = ((lpGameStruct->lScore >= 5) ? true : false);

			if (bWin) {
				iSoundCode = (xpXodj->m_bHodj ? MG_SOUND_WS3 : MG_SOUND_WS4);
			} else {
				iSoundCode = (xpXodj->m_bHodj ? MG_SOUND_WS1 : MG_SOUND_WS2);
			}

			break ;

		case MG_GAME_CHALLENGE:    /* Q. Challenge for Mish and Mosh */
			bWin = false;
			break;

		default:                    // can't recognize game
			assert(0);
			break ;
		}
	}

	if ((iSoundCode != -1) && !m_xpCurXodj->m_bComputer) {

		// play the You Win Narration (game and hodj/podj dependent)
		//
		iSoundCode -= MG_SOUND_BASE;
		assert(iSoundCode >= 0 && iSoundCode <= MG_SOUND_MAX);
		if ((pSound = new CSound(xpGtlView, szGameSounds[iSoundCode], SOUND_WAVE | SOUND_QUEUE | SOUND_ASYNCH | SOUND_AUTODELETE)) != nullptr) {
			pSound->setDrivePath(lpMetaGameStruct->m_chCDPath);
			pSound->play();
		}
	}

	if (bWin) {

		switch (iWinCode) {

		// win information
		//
		case MG_WIN_INFO:
			DivulgeInformation(m_xpCurXodj, false);
			break ;

		// give the player an object from storage
		//
		case MG_WIN_OBJECT:
			GainRandomItem(m_xpCurXodj);
			break ;

		// player gains
		//
		case MG_WIN_MONEY:

			if (lMoneyWon > 0)
				GainMoney(m_xpCurXodj, lMoneyWon);
			break ;

		default:
			break ;
		}
	}

cleanup:

	JXELEAVE(CGtlData::ProcessGameResult) ;
	RETURN(iError != 0) ;
}


//* CGtlData::GainRandomItem -- player receives a random item from store
bool CGtlData::GainRandomItem(CXodj * xpXodj)
// xpXodj -- player getting the random item
// returns: true if error, false otherwise
{
	JXENTER(CGtlData::GainRandomItem) ;
	int iError = 0 ;            // error code
	CItem * xpItem ;
	CInventory * xpInventory ;

	int iTotalItems = xpXodj->m_pGenStore->ItemCount() + xpXodj->m_pBlackMarket->ItemCount();
	int iItemNumber ;   // item number of item

	// no items left in store
	if (iTotalItems <= 0) {

		// there are some items in the store
		//
	} else {

		iItemNumber = m_cGenUtil.RandomInteger(1, iTotalItems);

		xpInventory = xpXodj->m_pGenStore ;     // assume it's from
		// the general store
		if (iItemNumber > xpXodj->m_pGenStore->ItemCount()) {

			// use pawn shop, and reduce item number
			xpInventory = xpXodj->m_pBlackMarket;
			iItemNumber -= xpXodj->m_pGenStore->ItemCount();
		}

		xpItem = xpInventory->FetchItem(iItemNumber - 1);

		if (xpItem == nullptr) {
			iError = 100 ;
			goto cleanup ;
		}

		xpInventory->RemoveItem(xpItem);
		xpXodj->m_pInventory->AddItem(xpItem);

		// Pause Meta Game
		m_bInhibitDraw = true;

		m_cBgbMgr.PauseAnimations();
		CItemDialog dlgItem((CGtlView *)m_cBgbMgr.m_xpcView, m_cBgbMgr.m_xpGamePalette, xpItem, lpMetaGameStruct->m_cHodj.m_bMoving, true, 1);
		//m_cBgbMgr.ResumeAnimations();

		// Resume Meta Game
		m_bInhibitDraw = false;
	}

cleanup:

	JXELEAVE(CGtlData::GainRandomItem);
	RETURN(iError != 0) ;
}

//* CGtlData::GainMoney -- player has won some money
bool CGtlData::GainMoney(CXodj * xpXodj, long lCrowns)
// xpXodj -- player
// lCrowns -- number of crowns won
// returns: true if error, false otherwise
{
	JXENTER(CGtlData::GainMoney) ;
	int iError = 0 ;        // error code
	long lCount = 0 ;        // total number of crowns
	CItem * xpItem ;        // inventory item structure

	if (xpXodj->m_pInventory && ((xpItem = xpXodj->m_pInventory->FindItem(MG_OBJ_CROWN)) != nullptr)) {

		lCount = xpItem->m_nQuantity ;  // get quantity
		lCount += lCrowns ;             // increment
		xpItem->SetQuantity(lCount) ;

		// Pause Meta Game
		m_bInhibitDraw = true;

		xpItem = new CItem(MG_OBJ_CROWN);

		m_cBgbMgr.PauseAnimations();
		CItemDialog dlgItem((CWnd *)m_cBgbMgr.m_xpcView, m_cBgbMgr.m_xpGamePalette, xpItem, lpMetaGameStruct->m_cHodj.m_bMoving, true, lCrowns);
		//m_cBgbMgr.ResumeAnimations();

		delete xpItem;

		// Resume Meta Game
		m_bInhibitDraw = false;
	}

// cleanup:

	JXELEAVE(CGtlData::GainMoney) ;
	RETURN(iError != 0) ;
}

//* CGtlData::GainMishMosh -- player gets MishMosh
bool CGtlData::GainMishMosh(CXodj *pXodj, long)
// pXodj -- player
// lCrowns -- number of crowns won
// returns: true if error, false otherwise
{
	JXENTER(CGtlData::GainMishMosh) ;
	int iError = 0 ;        // error code

	if (pXodj->m_pInventory != nullptr) {

		// add Mish/Mosh to player's inventory
		//
		assert(lpMetaGameStruct->m_pMishItem != nullptr);
		pXodj->m_pInventory->AddItem(lpMetaGameStruct->m_pMishItem);
		lpMetaGameStruct->m_pMishItem = nullptr;

		assert(lpMetaGameStruct->m_pMoshItem != nullptr);
		pXodj->m_pInventory->AddItem(lpMetaGameStruct->m_pMoshItem);
		lpMetaGameStruct->m_pMoshItem = nullptr;
	}

// cleanup:

	JXELEAVE(CGtlData::GainMishMosh) ;
	RETURN(iError != 0) ;
}

//* CGtlData::DivulgeInformation -- divulge information to player
bool CGtlData::DivulgeInformation(CXodj * pXodj, bool bSecondary)
// pXodj -- player
// bSecondary -- divulge secondary information (in this case,
//      this means getting info from farmer, etc.)
// returns: true if error, false otherwise
{
	JXENTER(CGtlData::DivulgeInformation) ;
	int iError = 0 ;                    // error code
	int iClueCode = 0 ;                 // clue number
	const CClueTable * pClueTable ;
	int iK ;                            // loop variable
	CHodjPodj *pCurXodj;
	CNote *pNote;
	int i;
	int iNoteID, iPersonID, iPlaceID, iLocID;

	// make sure there is a valid player object
	assert(pXodj != nullptr);

	// winning a mini-game
	if (!bSecondary) {

		// if there are no more primary msgs left
		if (pXodj->m_iWinInfoWon >= pXodj->m_iWinInfoNeed) {
//			::MessageBox(nullptr,"Primary clue needed but all already given out","Debug Info",MB_OK);
			iError = 100 ;      // no more primary messages
			goto cleanup ;
		}
		iClueCode = pXodj->m_iWinInfoTable[pXodj->m_iWinInfoWon++] ;
	} else {                    // secondary information

		// if there are no more secondary msgs left
		if (pXodj->m_iSecondaryInfoWon >= pXodj->m_iSecondaryInfoNeed) {
//			::MessageBox(nullptr,"Secondary clue needed but all already given out","Debug Info",MB_OK);
			iError = 101 ;      // no more primary messages
			goto cleanup ;
		}
		iClueCode = pXodj->m_iSecondaryInfoTable[pXodj->m_iSecondaryInfoWon++] ;
	}

	if (((pClueTable = CMgStatic::FindClue(pXodj->m_bHodj, iClueCode)) != nullptr) && (pClueTable->m_iCluePicCode != 0)) {

		if (pClueTable->m_iClueCode == MG_CLUE_GOTO) {
			for (iK = DIMENSION(pXodj->m_iSecondaryLoc) - 1 ; iK > 0 ; --iK)
				pXodj->m_iSecondaryLoc[iK] = pXodj->m_iSecondaryLoc[iK - 1] ;
			// move everything up one position
			pXodj->m_iSecondaryLoc[0] = pClueTable->m_iArgument ;
		}

		// Player gains a clue
		//

		// determine where this clue was found
		//
		iNoteID = -1;
		iPersonID = -1;
		iPlaceID = -1;
		iLocID = (m_lpNodes + (pXodj->m_iCharNode))->m_iLocationCode;
		for (i = 0; i < NOTE_COUNT; i++) {

			if (nClueLocation[i].iLocID == iLocID) {
				iNoteID = nClueLocation[i].iNoteID;
				iPersonID = nClueLocation[i].iPersonID;
				iPlaceID = nClueLocation[i].iPlaceID;
				break;
			}
		}
		assert(iNoteID != -1);

		pCurXodj = &m_xpcGtlDoc->m_xpGtlFrame->m_lpBfcMgr->m_cPodj;
		if (pXodj->m_bHodj)
			pCurXodj = &m_xpcGtlDoc->m_xpGtlFrame->m_lpBfcMgr->m_cHodj;

		assert(pCurXodj->m_aClueArray[iClueCode].bUsed == false);
		assert(pCurXodj->m_aClueArray[iClueCode].pNote != nullptr);

		pCurXodj->m_aClueArray[iClueCode].bUsed = true;
		pNote = pCurXodj->m_aClueArray[iClueCode].pNote;

		// set this clue's attributes
		//
		pNote->SetID(iNoteID);
		pNote->SetPersonID(iPersonID);
		pNote->SetPlaceID(iPlaceID);

		// show clue
		//
		// Pause Meta Game
		m_bInhibitDraw = true;
		m_cBgbMgr.PauseAnimations();

		// show the clue in dialog box
		if (pXodj->m_bComputer) {
			C1ButtonDialog dlg1Button((CWnd *)pMainWindow, m_cBgbMgr.m_xpGamePalette, "&OK", " ", (pXodj->m_bHodj ? "Hodj gets a clue!" : "Podj gets a clue!"));
			dlg1Button.DoModal();
		} else
			pMainWindow->ShowClue(m_cBgbMgr.m_xpGamePalette, pNote);

		// resume Meta Game
		//m_cBgbMgr.ResumeAnimations();
		m_bInhibitDraw = false;

		// add clue to players notebook
		//
		pXodj->m_pInventory->FindItem(pXodj->m_bHodj ? MG_OBJ_HODJ_NOTEBOOK : MG_OBJ_PODJ_NOTEBOOK)->AddNote(pNote);

	} else {
//		::MessageBox(nullptr,"Invalid clue number detected","Debug Info",MB_OK);
		iError = 102 ;      // invalid clue number
		goto cleanup ;
	}

cleanup:

	JXELEAVE(CGtlData::DivulgeInformation) ;
	RETURN(iError != 0) ;
}


//* CGtlData::DetermineEligibility -- determine whether
//  player has objects/money needed for current location
bool CGtlData::DetermineEligibility(CXodj *xpXodj, int iLocationCode, int &iLocFunctionCode, bool bExecute)
// xpXodj -- player
// iLocationCode -- MG_LOC_xxxx location code
// iLocFunctionCode (output) -- MG_GAME_xxxx or MG_VISIT_xxxx
// bExecute -- if true, then execute on the eligibility
// returns: true if eligible, false otherwise
{
	JXENTER(CGtlData::DetermineEligibility) ;
	//int iError = 0 ;        // error code
	const CLocTable * xpLocEntry ;    // location table entry
	bool bEligibility = false ; // return value
	int iSoundCode = -1;
	CGtlView * xpGtlView = (CGtlView *)m_cBgbMgr.m_xpcView ;
	CHodjPodj *pCurPlayer;
	CSound *pSound;

	if ((xpLocEntry = CMgStatic::FindLoc(iLocationCode)) == nullptr) {
		//iError = 100 ;
		goto cleanup ;
	}

	// set up current player 1st so we don't have to do all
	// the other if...else's later on in this function
	// OPT
	//
	pCurPlayer = &lpMetaGameStruct->m_cPodj;
	if (xpXodj->m_bHodj)
		pCurPlayer = &lpMetaGameStruct->m_cHodj;

	iLocFunctionCode = xpLocEntry->m_iFunctionCode ;

	// Since MishMosh can be in the Pawn Shop (which has other purposes),
	// we assume that MishMosh can be in any location, and make that
	// test first; failing it, we try other things.

	if ((bEligibility = DetermineMishMoshEligibility(xpXodj, iLocationCode)) != false)
		iLocFunctionCode = MG_VISIT_MISHMOSH ;  // change function
	// code to mish/mosh

	else if (iLocFunctionCode >= MG_GAME_BASE && iLocFunctionCode <= MG_GAME_MAX)
		bEligibility = DetermineGameEligibility(xpXodj, iLocFunctionCode, bExecute);

	else {
		switch (iLocFunctionCode) {

		case MG_VISIT_CASTLE:
			bEligibility = (GetGameObjectCount(xpXodj, MG_OBJ_MISH) > 0);
			// eligibile for castle if player has
			// Mish (and hence Mosh)
			break;

		case MG_VISIT_SECRET1:
		case MG_VISIT_SECRET2:

			if (xpXodj->m_bComputer) {
				bEligibility = false;
				break;
			}

			if (bExecute) {
				// always eligible for secret passage
				bEligibility = true;
				if (xpXodj->m_bHodj) {
					lpMetaGameStruct->m_cHodj.m_iSpecialTravelCode = iLocFunctionCode;
				} else {
					lpMetaGameStruct->m_cPodj.m_iSpecialTravelCode = iLocFunctionCode;
				}
			}
			break;

		case MG_VISIT_OASIS:
			if (bExecute) {

				// always eligible for Oasis
				bEligibility = true;

				if (xpXodj->m_bHodj) {
					lpMetaGameStruct->m_cHodj.m_iSpecialTravelCode = iLocFunctionCode;
					iSoundCode = MG_SOUND_TRAN3;
				} else {
					lpMetaGameStruct->m_cPodj.m_iSpecialTravelCode = iLocFunctionCode;
					iSoundCode = MG_SOUND_TRAN4;
				}
			}
			break;

		case MG_VISIT_CAR1:

			if (xpXodj->m_bComputer) {
				bEligibility = false;
				break;
			}

			if (bExecute) {

				// always eligible for mining car
				bEligibility = true;

				if (xpXodj->m_bHodj) {
					lpMetaGameStruct->m_cHodj.m_iSpecialTravelCode = iLocFunctionCode;
					iSoundCode = MG_SOUND_TRAN11;
				} else {
					lpMetaGameStruct->m_cPodj.m_iSpecialTravelCode = iLocFunctionCode;
					iSoundCode = MG_SOUND_TRAN12;
				}
			}
			break;

		case MG_VISIT_SKY1:
		case MG_VISIT_SKY2:

			if (xpXodj->m_bComputer) {
				bEligibility = false;
				break;
			}

			if (bExecute) {

				long b = xpXodj->m_pInventory->FindItem(MG_OBJ_CROWN)->GetQuantity();

				if (b >= 5) {

					bEligibility = true;
					xpXodj->m_pInventory->FindItem(MG_OBJ_CROWN)->SetQuantity(b - 5);
					if (xpXodj->m_bHodj) {
						lpMetaGameStruct->m_cHodj.m_iSpecialTravelCode = iLocFunctionCode;
						iSoundCode = MG_SOUND_TRAN9;
					} else {
						lpMetaGameStruct->m_cPodj.m_iSpecialTravelCode = iLocFunctionCode;
						iSoundCode = MG_SOUND_TRAN10;
					}
				}
			}
			break;

		case MG_VISIT_BOAT1:
		case MG_VISIT_BOAT2:
		case MG_VISIT_BOAT3:

			if (xpXodj->m_bComputer) {
				bEligibility = false;
				break;
			}

			if (bExecute == false)
				break;

			// increasing chances that hodj will get a boat ride
			// 30, 60, 90, 100%
			//
			bEligibility = false;

			pCurPlayer->m_iNumberBoatTries++;
			if (ProbableTrue(33 * pCurPlayer->m_iNumberBoatTries)) {
				pCurPlayer->m_iSpecialTravelCode = iLocFunctionCode;
				pCurPlayer->m_iNumberBoatTries = 0;
				bEligibility = true;

				// randomly select a boat narration
				//
				switch (brand() % 4) {

				case 0:
					iSoundCode = MG_SOUND_BB68;
					break;

				case 1:
					iSoundCode = MG_SOUND_BB69;
					break;

				case 2:
					iSoundCode = MG_SOUND_BB70;
					break;

				case 3:
					iSoundCode = MG_SOUND_BB71;
					break;
				}
			}
			break;


		case MG_VISIT_INFO:
			bEligibility = DetermineInfoEligibility(xpXodj, iLocationCode, bExecute);
			break ;

		case MG_VISIT_BLACKMARKET: {
			bool    bCantBuy = true;

			if (xpXodj->m_pBlackMarket->ItemCount() > 0) {
				bCantBuy = false;
			}
			if (bCantBuy == false) {
				if (xpXodj->m_bComputer == false) {
					bEligibility = true;
				} else {
					bEligibility = false;

					if (xpXodj->m_iWinInfoWon > 0) {
						int i;

						for (i = 0; i < xpXodj->m_iWinInfoWon; i++) {
							int     k = xpXodj->m_iWinInfoTable[i];
							int     l = 0;

							if (xpXodj->m_bHodj) {
								l = CMgStatic::cHodjClueTable[k].m_iArgument;
							} else {
								l = CMgStatic::cPodjClueTable[k].m_iArgument;
							}

							if ((l >= MG_OBJ_BASE) && (l <= MG_OBJ_MAX)) {
								CItem   *pItem = xpXodj->m_pInventory->FindItem(l);

								if (pItem != nullptr) {
									if (pItem->GetQuantity() == 0) {
										bEligibility = true;
										break;
									}
								} else {
									bEligibility = true;
								}
							}
						}
					}
				}
			} else {
				bEligibility = false ;
			}
		}
		break ;

		case MG_VISIT_GENERALSTR: {
			bool    bCantBuy = true;

			if (xpXodj->m_pGenStore->ItemCount() > 0) {
				bCantBuy = false;
			}
			if (bCantBuy == false) {
				if (xpXodj->m_bComputer == false) {
					bEligibility = true;
				} else {
					bEligibility = false;

					if (xpXodj->m_iWinInfoWon > 0) {
						int i;

						for (i = 0; i < xpXodj->m_iWinInfoWon; i++) {
							int     k = xpXodj->m_iWinInfoTable[i];
							int     l = 0;

							if (xpXodj->m_bHodj) {
								l = CMgStatic::cHodjClueTable[k].m_iArgument;
							} else {
								l = CMgStatic::cPodjClueTable[k].m_iArgument;
							}

							if ((l >= MG_OBJ_BASE) && (l <= MG_OBJ_MAX)) {
								CItem   *pItem = xpXodj->m_pInventory->FindItem(l);

								if (pItem != nullptr) {
									if (pItem->GetQuantity() == 0) {
										bEligibility = true;
										break;
									}
								} else {
									bEligibility = true;
								}
							}
						}
					}
				}
			} else {
				bEligibility = false ;
			}
		}
		break ;

		case MG_VISIT_PAWNSHOP:
			bEligibility = true ;
			// always eligible, even if broke
			break ;

		case MG_VISIT_MISHMOSH:
		default:
			break ;     // default to "not eligible"
		}
	}

	if (iSoundCode >= 0) {
		assert(iSoundCode >= MG_SOUND_BASE && iSoundCode <= MG_SOUND_MAX);
		iSoundCode -= MG_SOUND_BASE;
		pSound = new CSound(xpGtlView, szGameSounds[iSoundCode], SOUND_WAVE | SOUND_QUEUE | SOUND_ASYNCH | SOUND_AUTODELETE);
		pSound->setDrivePath(lpMetaGameStruct->m_chCDPath);
		pSound->play();
	}
	if (!bEligibility)
		iLocFunctionCode = 0 ;  // no function code if not eligible

cleanup:

	JXELEAVE(CGtlData::DetermineEligibility) ;
	RETURN(bEligibility) ;
}


//* CGtlData::DetermineGameEligibility -- determine if eligible to play
//  a game, and update game history if so (and bExecute)
// ***** modify this function to determine how often a user can
//  play the same game over and over
bool CGtlData::DetermineGameEligibility(CXodj *xpXodj, int iGameCode, bool bExecute)
// xpXodj -- player
// iGameCode -- MG_GAME_xxxx -- game whose eligibility
//      is to be determined
// bExecute -- if true, then execute on the eligibility
// returns: true if eligible, false otherwise
{
	JXENTER(CGtlData::DetermineGameEligibility) ;
	int iK ;        // loop variable
	int iLast = -1 ;    // how long ago last time game played
	int iGame ;     // game code in table
	bool bEligibility = false ; // return value

	if (iGameCode >= MG_GAME_BASE && iGameCode <= MG_GAME_MAX) {

		for (iK = 0 ; iLast < 0 && (uint)iK < DIMENSION(xpXodj->m_iGameHistory) && ((iGame = xpXodj->m_iGameHistory[iK]) != 0); ++iK)
			if (iGame == iGameCode)
				iLast = iK ;

		// the following tests whether this game has been played
		// any of the last three times.  Change this test to determine
		// how often a user can play the ame game over and over.
		if (iLast < 0 || iLast > 2)
			bEligibility = true ;

		// if eligible to play and we want to execute on eligibility
		//
		if (bEligibility && bExecute) {

			// move everything up one space in game history array,
			// and add the new game in front
			for (iK = DIMENSION(xpXodj->m_iGameHistory) - 1 ; iK > 0 ; --iK)
				xpXodj->m_iGameHistory[iK] = xpXodj->m_iGameHistory[iK - 1] ;

			xpXodj->m_iGameHistory[0] = iGameCode ;
		}

		if (bEligibility) {
			int i;
			int j = -1;

			for (i = 0; i < MG_GAME_COUNT; i ++) {
				if (iGameCode == CMgStatic::cGameTable[i].m_iGameCode) {
					j = CMgStatic::cGameTable[i].m_iWinCode;
					break;
				}
			}

			if (j > 0) {
				if (j == MG_WIN_INFO) {
					if (xpXodj->m_iWinInfoWon == xpXodj->m_iWinInfoNeed)
						bEligibility = false ;
				} else {
					if (j == MG_WIN_OBJECT) {
						bEligibility = false;
						if ((xpXodj->m_pGenStore->ItemCount() > 0) || (xpXodj->m_pBlackMarket->ItemCount() > 0))
							bEligibility = true;
					}
				}
			}
		}
		if ((bEligibility) && (iGameCode == MG_GAME_VIDEOPOKER)) {
			if (GetGameObjectCount(xpXodj, MG_OBJ_CROWN) <= 0)
				bEligibility = false;
		} else if ((bEligibility) && !xpXodj->m_bComputer && (iGameCode == MG_GAME_BEACON)) {
			int nDevCaps;
			CDC *pDC = nullptr;
			if ((pDC = gpMyView->GetDC()) != nullptr) {
				m_cBgbMgr.m_xpBsuSet->PrepareDc(pDC, true);
				nDevCaps = (*pDC).GetDeviceCaps(RASTERCAPS);
				m_cBgbMgr.m_xpBsuSet->PrepareDc(pDC, false);
				gpMyView->ReleaseDC(pDC);
				if (!(nDevCaps & RC_PALETTE)) {
					C1ButtonDialog dlg1Button((CWnd *)pMainWindow, m_cBgbMgr.m_xpGamePalette, "&OK", "To play Beacon,", "your display must be", "set to 256 colors.");
					dlg1Button.DoModal();
					bEligibility = false;
				}
			}
		}

	}

// cleanup:

	JXELEAVE(CGtlData::DetermineGameEligibility) ;
	RETURN(bEligibility) ;
}


//* CGtlData::GetGameObjectCount -- get the count of the specified
//      object in the player's inventory
long CGtlData::GetGameObjectCount(CXodj *xpXodj, int iObjectCode)
// xpXodj -- player
// iObjectCode -- MG_OBJ_xxxx -- object code
// returns: count of # of objects
{
	JXENTER(CGtlData::GetGameObjectCount) ;
	long lCount = 0 ;        // count of number of objects
	CItem * xpItem ;        // inventory item structure

	if (xpXodj->m_pInventory && ((xpItem = xpXodj->m_pInventory->FindItem(iObjectCode)) != nullptr))
		lCount = xpItem->m_nQuantity ;

// cleanup:

	JXELEAVE(CGtlData::GetGameObjectCount) ;
	RETURN(lCount) ;
}

//* CGtlData::DetermineInfoEligibility -- determine if eligible to
//  get information at this location
bool CGtlData::DetermineInfoEligibility(CXodj * xpXodj, int iLocationCode, bool bExecute)
// xpXodj -- player
// iLocationCode -- MG_LOC_xxxx -- game where information might
//      be available, if we're eligible
// bExecute -- if true, then execute on the eligibility
// returns: true if eligible, false otherwise
{
	JXENTER(CGtlData::DetermineInfoEligibility) ;
	int iK ;        // loop variable
	int iFound = -1 ;   // location in secondary location table
	int iLoc ;      // location code in table
	bool bEligibility = false ; // return value
	const CLocTable * xpLocTable = CMgStatic::FindLoc(iLocationCode) ;

	// if there's a location table entry, and it's an
	// information location, and if I have at least
	// as many crowns as are necessary

	bLacksMoney = false;

	if (xpLocTable && (xpLocTable->m_iFunctionCode == MG_VISIT_INFO)) {

		// check to see if the location is in my secondary location
		// table -- which means that I've received a clue telling me
		// to go there to get information

		for (iK = 0; iFound < 0 && (uint)iK < DIMENSION(xpXodj->m_iSecondaryLoc) && ((iLoc = xpXodj->m_iSecondaryLoc[iK]) != 0); ++iK)
			if (iLoc == iLocationCode)
				iFound = iK ;

		// location found in secondary loc table
		//
		if (iFound >= 0) {
			if (GetGameObjectCount(xpXodj, MG_OBJ_CROWN) < xpLocTable->m_iCost)
				bLacksMoney = true;
			else {
				bEligibility = true ;   // then we're indeed eligible
				// to get information from this location
				if (bExecute)   // if we want to execute on eligibility
					for (iK = iFound ; (uint)iK < DIMENSION(xpXodj->m_iSecondaryLoc)
					        ; ++iK)
						xpXodj->m_iSecondaryLoc[iK] =
						    xpXodj->m_iSecondaryLoc[iK + 1] ;
				// remove location from table
			}
		}
	}

// cleanup:

	JXELEAVE(CGtlData::DetermineInfoEligibility) ;
	RETURN(bEligibility) ;
}

//* CGtlData::DetermineMishMoshEligibility -- determine if eligible to
//  grab Mish and Mosh from current location
bool CGtlData::DetermineMishMoshEligibility(CXodj *xpXodj, int iLocationCode)
// xpXodj -- player
// iLocationCode -- MG_LOC_xxxx -- location where Mish/Mosh
//      might lie (or lay, if we get lucky)
// returns: true if eligible, false otherwise
{
	JXENTER(CGtlData::DetermineMishMoshEligibility) ;
	int iK ;        // loop variable
//  int iLoc ;      // location code in table
	bool bEligibility = true ;  // return value: assume eligible
	// until disproven
	CXodj * xpOpponent ;    // opponent
//    bool bWinMishMosh = xpGtlApp->m_cBdbgMgr.GetDebugInt("winmishmosh") ;
	bool bWinMishMosh = false;
	// debugging flag means no need for objects/info

	// we can grab Mish/Mosh if (1) this is where we put them,
	// (2) we have all the required objects and information,
	// and (3) I don't already have them, and (4) my opponent
	// doesn't already have them.

	if (iLocationCode != m_iMishMoshLoc)
		bEligibility = false ;      // not eligible, since we
	// don't have all the required information

	if (!bWinMishMosh) {

		if (xpXodj->m_iWinInfoWon != xpXodj->m_iWinInfoNeed || xpXodj->m_iSecondaryInfoWon != xpXodj->m_iSecondaryInfoNeed)
			bEligibility = false ;      // not eligible, since
		// we don't have all the required additional
		// information

		for (iK = 0 ; bEligibility && iK < xpXodj->m_iRequiredObjectsCount ; ++iK) {

			// if we don't have one of the required objects
			if (!GetGameObjectCount(xpXodj, xpXodj->m_iRequiredObjectsTable[iK])) {
				bEligibility = false ;
			}
		}

		/*
		        if (xpXodj->m_iRequiredMoney && GetGameObjectCount(xpXodj, MG_OBJ_CROWN) < xpXodj->m_iRequiredMoney) {
		            bEligibility = false ;  // don't have enough crowns
		        }
		*/

		if ((xpOpponent = xpXodj->m_xpXodjNext) == nullptr)
			xpOpponent = m_xpXodjChain ;    // find opponent

		if (GetGameObjectCount(xpXodj, MG_OBJ_MISH) || GetGameObjectCount(xpOpponent, MG_OBJ_MISH)) {
			bEligibility = false ;  // one of  us already has Mish/Mosh
		}
	}

// cleanup:

	JXELEAVE(CGtlData::DetermineMishMoshEligibility) ;
	RETURN(bEligibility) ;
}

} // namespace Gtl
} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel
