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

#include "gui/debugger.h"
#include "bagel/baglib/command_object.h"
#include "bagel/baglib/dossier_object.h"
#include "bagel/baglib/rp_object.h"
#include "bagel/baglib/master_win.h"
#include "bagel/baglib/storage_dev_win.h"
#include "bagel/boflib/gui/movie.h"
#include "bagel/baglib/dev_dlg.h"
#include "bagel/baglib/pan_window.h"
#include "bagel/boflib/sound.h"
#include "bagel/bagel.h"

namespace Bagel {

extern BOOL g_bRestoreObjList;
extern BOOL g_bAllowAAO;

CBagCommandObject::CBagCommandObject() {
	m_xObjType = COMMANDOBJ;
	SetVisible(FALSE);
	SetTimeless(TRUE);
}

BOOL CBagCommandObject::RunObject() {
	static INT nIteration = 0;
	BOOL rc = FALSE;

	// Don't allow more than 10 (recursively)
	//
	if (nIteration < 10) {
		nIteration++;

		CBofString sSrcSDev = m_sSrcSDev;
		CBofString sDstSDev = m_sDstSDev;

		// Check if these items should be replaced by the current sdev
		if (!sSrcSDev.IsEmpty() && !sSrcSDev.Find(CURRSDEV_TOKEN)) {
			sSrcSDev = CBagel::GetBagApp()->GetMasterWnd()->GetCurrentStorageDev()->GetName();
		}

		if (!sDstSDev.IsEmpty() && !sDstSDev.Find(CURRSDEV_TOKEN)) {
			sDstSDev = CBagel::GetBagApp()->GetMasterWnd()->GetCurrentStorageDev()->GetName();
		}

		// Check if these items should be replaced by the previous sdev
		if (!sSrcSDev.IsEmpty() && !sSrcSDev.Find(PREVSDEV_TOKEN)) {
			sSrcSDev = CBagel::GetBagApp()->GetMasterWnd()->GetCurrentStorageDev()->GetPrevSDev();
		}

		if (!sDstSDev.IsEmpty() && !sDstSDev.Find(PREVSDEV_TOKEN)) {
			sDstSDev = CBagel::GetBagApp()->GetMasterWnd()->GetCurrentStorageDev()->GetPrevSDev();
		}

		if (GetFileName() == "REMOVE") {
			static BOOL bRemove = FALSE;
			if (!bRemove) {
				bRemove = TRUE;

				if (!m_sObjName.IsEmpty() && !sSrcSDev.IsEmpty()) {
					SDEVMNGR->RemoveObject(sSrcSDev, m_sObjName);
				}
				bRemove = FALSE;
			}

		} else if (GetFileName() == "INSERT2") {
			if (!m_sObjName.IsEmpty() && !sDstSDev.IsEmpty()) {
				g_bAllowAAO = FALSE;
				SDEVMNGR->AddObject(sDstSDev, m_sObjName);
				g_bAllowAAO = TRUE;
			}

		} else if (GetFileName() == "INSERT") {
			if (!m_sObjName.IsEmpty() && !sDstSDev.IsEmpty()) {
				SDEVMNGR->AddObject(sDstSDev, m_sObjName);
			}

		} else if (GetFileName() == "TRANSFER") {

			if (!m_sObjName.IsEmpty() && !sSrcSDev.IsEmpty() && !sDstSDev.IsEmpty()) {
				SDEVMNGR->MoveObject(sDstSDev, sSrcSDev, m_sObjName);
			}

		} else if (GetFileName().Find("FLUSHQUEUE") == 0) {
			CHAR szBuf[20];
			INT nSlot;

			nSlot = 0;
			Common::strcpy_s(szBuf, GetFileName());
			if (strlen(szBuf) == 11) {
				nSlot = szBuf[10] - 48;
			}
			Assert(nSlot >= 0 && nSlot < 8);

#if BOF_WINDOWS
			CBofSound::FlushQueue(nSlot);
#endif
		} else if (GetFileName() == "CLOSE") {
			CBagStorageDevWnd *pSDev = CBagel::GetBagApp()->GetMasterWnd()->GetCurrentStorageDev();
			pSDev->Close();

		} else if (GetFileName() == "UPDATE") {
			static BOOL bUpdate = FALSE;

			if (!bUpdate) {
				bUpdate = TRUE;

				CBagStorageDevWnd *pSDev = CBagel::GetBagApp()->GetMasterWnd()->GetCurrentStorageDev();
				pSDev->AttachActiveObjects();

				bUpdate = FALSE;
			}

		} else if (GetFileName() == "EVENTLOOP") {
			static BOOL bEventLoop = FALSE;

			// prevent recursion
			//
			if (!bEventLoop) {
				bEventLoop = TRUE;

				CBagStorageDev *pSDev;
				CBagPanWindow *pWin;

				if ((pSDev = CBagel::GetBagApp()->GetMasterWnd()->GetCurrentStorageDev()) != nullptr) {
					if (pSDev->GetDeviceType() == SDEV_GAMEWIN) {
						pWin = (CBagPanWindow *)pSDev;
						pWin->OnTimer(EVAL_EXPR);
					}
				}
				bEventLoop = FALSE;
			}

		} else if (GetFileName() == "STOPPAINT") {
			g_bAllowPaint = FALSE;

		} else if (GetFileName() == "REMOVEWIELD") {
			// Remove currently wielded object from the game
			CBagObject *pObj;

			if (CBagPanWindow::m_pWieldBmp != nullptr) {

				if ((pObj = CBagPanWindow::m_pWieldBmp->GetCurrObj()) != nullptr) {
					SDEVMNGR->RemoveObject(CBagPanWindow::m_pWieldBmp->GetName(), pObj->GetRefName());
				}
			}

		} else if (GetFileName() == "PAINT") {
			// Get a pointer to the current game window
			CBagStorageDevWnd *pWin;

			if ((pWin = CBagel::GetBagApp()->GetMasterWnd()->GetCurrentStorageDev()) != nullptr) {
				pWin->PaintScreen(nullptr, TRUE);
			}

		} else if (GetFileName() == "STOPWAVE") {
			CBofSound::StopWaveSounds();

		} else if (GetFileName() == "STASHWIELD") {
			// Stash the currently wielded item
			CBagObject *pObj;

			if (CBagPanWindow::m_pWieldBmp != nullptr) {
				if ((pObj = CBagPanWindow::m_pWieldBmp->GetCurrObj()) != nullptr) {
					SDEVMNGR->MoveObject("INV_WLD", CBagPanWindow::m_pWieldBmp->GetName(), pObj->GetRefName());
					CBagPanWindow::m_pWieldBmp->SetCurrObj(nullptr);
				}
			}

		} else if (GetFileName() == "DEATH") {
			CBagel::GetBagApp()->GetMasterWnd()->PostUserMessage(WM_DIE, 0);
			g_bAllowPaint = FALSE;

		} else if (GetFileName() == "GOPAINT") {
			g_bAllowPaint = TRUE;

		} else if (GetFileName() == "DEACTIVATEPDA") {

			// Get a pointer to the current game window
			CBagStorageDevWnd *pMainWin = (CBagel::GetBagApp()->GetMasterWnd()->GetCurrentStorageDev());
			// Pull down the PDA (if it exists)
			//
			if (pMainWin->GetDeviceType() == SDEV_GAMEWIN) {
				((CBagPanWindow *)pMainWin)->DeactivatePDA();
				((CBagPanWindow *)pMainWin)->WaitForPDA();
			}

		} else if (GetFileName() == "ROTATETOFLY") {
			if (CBagMasterWin::GetFlyThru()) {
				CBagStorageDev *pSDev;
				CBagPanWindow *pWin;

				CBagMasterWin::SetActiveCursor(6);
				if ((pSDev = CBagel::GetBagApp()->GetMasterWnd()->GetCurrentStorageDev()) != nullptr) {
					if (pSDev->GetDeviceType() == SDEV_GAMEWIN) {
						pWin = (CBagPanWindow *)pSDev;
						pWin->RotateTo(GetPosition());
					}
				}
			}

		} else if (GetFileName() == "ROTATETO") {
			CBagStorageDev *pSDev;
			CBagPanWindow *pWin;

			CBagMasterWin::SetActiveCursor(6);
			if ((pSDev = CBagel::GetBagApp()->GetMasterWnd()->GetCurrentStorageDev()) != nullptr) {
				if (pSDev->GetDeviceType() == SDEV_GAMEWIN) {
					pWin = (CBagPanWindow *)pSDev;
					pWin->RotateTo(GetPosition());
				}
			}

		} else if (GetFileName() == "SETQVOL") {
#if BOF_WINDOWS
			CBofSound::SetQVol(GetPosition().x, GetPosition().y);
#endif
		} else if (GetFileName() == "TURN") {

			// Cause 1 turn to go by
			//
			VARMNGR->IncrementTimers();

		} else if (GetFileName() == "RP_UPDATE_QUEUE") {
			// Execute any waiting residue printing results.
			CBagRPObject::UpdateRPQueue();

		} else if (GetFileName() == "RP_RUN_QUEUE") {
			// Execute any waiting residue printing results.
			CBagRPObject::RunRPQueue();

		} else if (GetFileName() == "RP_DEACTIVATE_RESULTS") {
			// Execute any waiting residue printing results.
			CBagRPObject::DeactivateRPQueue();

		} else if (GetFileName() == "RP_DEACTIVATE_REVIEW") {
			// Execute any waiting residue printing results.
			CBagRPObject::DeactivateRPReview();

		} else if (GetFileName() == "RP_DEACTIVATE_DOSSIER") {
			// Execute any waiting residue printing results.
			CBagDossierObject::DeactivateCurDossier();

		} else if (GetFileName() == "RP_ACTIVATE_REVIEW") {
			// Execute any waiting residue printing results.
			CBagRPObject::ActivateRPReview();

		} else if (GetFileName() == "SAVESTACK") {
			CBagMasterWin *pWin;

			if ((pWin = CBagel::GetBagApp()->GetMasterWnd()) != nullptr) {
				pWin->SaveSDevStack();
			}

		} else if (GetFileName() == "DEATH2") {
			CBagel::GetBagApp()->GetMasterWnd()->PostUserMessage(WM_DIE, 2);
			g_bAllowPaint = FALSE;

		} else if (GetFileName() == "ENTRYTHUD") { // Thud
			CDevDlg dlg;
			CBofRect tmpRect(60, 50, 137, 70);
			CBofString cStr("$SBARDIR\\GENERAL\\POPUP\\THUDPOP.BMP");
			MACROREPLACE(cStr);
			dlg.Create(cStr, CBagel::GetBagApp()->GetMasterWnd(), CBofApp::GetApp()->GetPalette(), &tmpRect);
			dlg.DoModal();
			dlg.Destroy();

		} else if (GetFileName() == "ENTRYFLEE") { // Fleebix
			CDevDlg dlg;
			CBofRect tmpRect(35, 48, 114, 69);
			CBofString cStr("$SBARDIR\\GENERAL\\POPUP\\FLEEDAT.BMP");
			MACROREPLACE(cStr);
			dlg.Create(cStr, CBagel::GetBagApp()->GetMasterWnd(), CBofApp::GetApp()->GetPalette(), &tmpRect);
			dlg.DoModal();
			dlg.Destroy();

		} else if (GetFileName() == "ENTRYCLICK") { // Click's locker combo
			CDevDlg dlg;
			CBofRect tmpRect(60, 49, 138, 68);
			CBofString cStr("$SBARDIR\\GENERAL\\POPUP\\CLICKPOP.BMP");
			MACROREPLACE(cStr);
			dlg.Create(cStr, CBagel::GetBagApp()->GetMasterWnd(), CBofApp::GetApp()->GetPalette(), &tmpRect);
			dlg.DoModal();
			dlg.Destroy();

		} else if (GetFileName() == "ENTRYFRUIT") { // MegaWave the fruit
			CDevDlg dlg;
			CBofRect tmpRect(35, 49, 114, 68);
			CBofString cStr("$SBARDIR\\GENERAL\\POPUP\\CLICKDAT.BMP");
			MACROREPLACE(cStr);
			dlg.Create(cStr, CBagel::GetBagApp()->GetMasterWnd(), CBofApp::GetApp()->GetPalette(), &tmpRect);
			dlg.DoModal();
			dlg.Destroy();

		} else if (GetFileName() == "D7CODE1DLG") { // Deven-7 code word
			CDevDlg dlg;
			CBofRect tmpRect(10, 48, 189, 69);
			CBofString cStr("$SBARDIR\\GENERAL\\POPUP\\DEVENPOP.BMP");
			MACROREPLACE(cStr);
			dlg.Create(cStr, CBagel::GetBagApp()->GetMasterWnd(), CBofApp::GetApp()->GetPalette(), &tmpRect, TRUE);
			dlg.DoModal();
			dlg.Destroy();

		} else if (GetFileName() == "SNAPTO") {
			CBagStorageDev *pSDev;
			CBagPanWindow *pWin;

			if ((pSDev = CBagel::GetBagApp()->GetMasterWnd()->GetCurrentStorageDev()) != nullptr) {
				if (pSDev->GetDeviceType() == SDEV_GAMEWIN) {
					pWin = (CBagPanWindow *)pSDev;

					if (pWin->GetSlideBitmap() != nullptr) {
						INT x, y;
						CBofRect cRect;
						x = GetPosition().x;
						y = GetPosition().y;

						cRect.SetRect(x, y, x + 480 - 1, y + 360 - 1);

						pWin->GetSlideBitmap()->SetCurrView(cRect);
					}
				}
			}

		} else if (GetFileName() == "RESUMESOUND") {
			CBofSound::ResumeSounds();

		} else if (GetFileName() == "PAUSESOUND") {
			CBofSound::PauseSounds();

		} else if (GetFileName() == "STOPMIDI") {
			CBofSound::StopMidiSounds();

		} else if (GetFileName() == "WAITWAVE") {
			CBofSound::WaitWaveSounds();

		} else if (GetFileName() == "WAITMIDI") {
			CBofSound::WaitMidiSounds();

		} else if (GetFileName() == "ACTIVATEPDA") {
			// Get a pointer to the current game window
			CBagStorageDevWnd *pMainWin = (CBagel::GetBagApp()->GetMasterWnd()->GetCurrentStorageDev());
			// Pull up the PDA (if it exists)
			//
			if (pMainWin->GetDeviceType() == SDEV_GAMEWIN) {
				((CBagPanWindow *)pMainWin)->ActivatePDA();
				((CBagPanWindow *)pMainWin)->WaitForPDA();
			}

		} else if (GetFileName() == "SHOWPDALOG") {
			// Get a pointer to the current game window
			CBagPDA *pPDA = (CBagPDA *)SDEVMNGR->GetStorageDevice("BPDA_WLD");
			if (pPDA) {
				pPDA->ShowLog();
			}

		} else if (GetFileName() == "OBJLIST") {
			g_bRestoreObjList = FALSE;

		} else if (GetFileName() == "WIN") {
			CBagMasterWin *pMainWin;

			if ((pMainWin = CBagel::GetBagApp()->GetMasterWnd()) != nullptr) {
				CBofBitmap cBmp(640, 480, CBofApp::GetApp()->GetPalette());
				cBmp.FillRect(nullptr, COLOR_BLACK);

				cBmp.Paint(pMainWin, 0, 0);
				g_bAllowPaint = FALSE;

				CBofString cString("$SBARDIR\\BAR\\EVGAMWIN.SMK");
				MACROREPLACE(cString);

				// Play the movie only if it exists
				//
				if (FileExists(cString.GetBuffer())) {
					BofPlayMovie(pMainWin, cString.GetBuffer());
					cBmp.Paint(pMainWin, 0, 0);
				}
			}

			// Display Credits
			pMainWin->ShowCreditsDialog(pMainWin);

			// Exit the game
			CBagel::GetBagApp()->GetMasterWnd()->Close();

		} else if (GetFileName() == "BREAK") {

#ifdef _DEBUG

#if BOF_MAC
			Debugger();
#else
			g_engine->getDebugger()->attach();
#endif

#endif // !_DEBUG

		} else {
			ReportError(ERR_UNKNOWN, "Invalid RUN COMMAND = %s", (const CHAR *)GetFileName());
		}

		rc = CBagObject::RunObject();

		nIteration--;
	}

	return rc;
}

PARSE_CODES CBagCommandObject::SetInfo(bof_ifstream &istr) {
	int nChanged;
	BOOL nObjectUpdated = FALSE;
	char ch;

	while (!istr.eof()) {
		nChanged = 0;
		istr.EatWhite();

		switch (ch = (char)istr.peek()) {
		//
		//  OBJECT
		//
		case 'O': {
			CHAR szLocalStr[256];
			szLocalStr[0] = 0;
			CBofString sStr(szLocalStr, 256);
			GetAlphaNumFromStream(istr, sStr);

			if (!sStr.Find("OBJECT")) {
				istr.EatWhite();
				GetAlphaNumFromStream(istr, m_sObjName);
				nChanged++;
				nObjectUpdated = TRUE;
			} else {
				PutbackStringOnStream(istr, sStr);
			}
			break;
		}
		//
		//  FROM
		//
		case 'F': {
			CHAR szLocalStr[256];
			szLocalStr[0] = 0;
			CBofString sStr(szLocalStr, 256);
			GetAlphaNumFromStream(istr, sStr);

			if (!sStr.Find("FROM")) {
				istr.EatWhite();
				GetAlphaNumFromStream(istr, m_sSrcSDev);
				nChanged++;
				nObjectUpdated = TRUE;
			} else {
				PutbackStringOnStream(istr, sStr);
			}
			break;
		}
		//
		//  TO
		//
		case 'T': {
			CHAR szLocalStr[256];
			szLocalStr[0] = 0;
			CBofString sStr(szLocalStr, 256);
			GetAlphaNumFromStream(istr, sStr);

			if (!sStr.Find("TO")) {
				istr.EatWhite();
				GetAlphaNumFromStream(istr, m_sDstSDev);
				nChanged++;
				nObjectUpdated = TRUE;
			} else {
				PutbackStringOnStream(istr, sStr);
			}
			break;
		}

		//
		//  no match return from funtion
		//
		default: {
			PARSE_CODES rc;
			if ((rc = CBagObject::SetInfo(istr)) == PARSING_DONE) {
				return PARSING_DONE;
			} else if (rc == UPDATED_OBJECT) {
				nObjectUpdated = TRUE;
			} else if (!nChanged) { // rc==UNKNOWN_TOKEN
				if (nObjectUpdated)
					return UPDATED_OBJECT;
				else
					return UNKNOWN_TOKEN;
			}
			break;
		}
		}
	}

	return PARSING_DONE;
}

} // namespace Bagel
