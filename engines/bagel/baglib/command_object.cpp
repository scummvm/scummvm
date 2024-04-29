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
#include "bagel/boflib/file_functions.h"

namespace Bagel {

extern bool g_restoreObjectListFl;
extern bool g_allowAttachActiveObjectsFl;

CBagCommandObject::CBagCommandObject() {
	m_xObjType = COMMANDOBJ;
	SetVisible(false);
	SetTimeless(true);
}

bool CBagCommandObject::runObject() {
	static int iteration = 0;
	bool rc = false;

	// Don't allow more than 10 (recursively)
	if (iteration < 10) {
		iteration++;

		CBofString srcSDev = _srcSDev;
		CBofString dstSDev = _destSDev;

		// Check if these items should be replaced by the current sdev
		if (!srcSDev.IsEmpty() && !srcSDev.Find(CURRSDEV_TOKEN)) {
			srcSDev = CBagel::getBagApp()->getMasterWnd()->GetCurrentStorageDev()->GetName();
		}

		if (!dstSDev.IsEmpty() && !dstSDev.Find(CURRSDEV_TOKEN)) {
			dstSDev = CBagel::getBagApp()->getMasterWnd()->GetCurrentStorageDev()->GetName();
		}

		// Check if these items should be replaced by the previous sdev
		if (!srcSDev.IsEmpty() && !srcSDev.Find(PREVSDEV_TOKEN)) {
			srcSDev = CBagel::getBagApp()->getMasterWnd()->GetCurrentStorageDev()->GetPrevSDev();
		}

		if (!dstSDev.IsEmpty() && !dstSDev.Find(PREVSDEV_TOKEN)) {
			dstSDev = CBagel::getBagApp()->getMasterWnd()->GetCurrentStorageDev()->GetPrevSDev();
		}

		if (GetFileName() == "REMOVE") {
			static bool removeFl = false;
			if (!removeFl) {
				removeFl = true;

				if (!_objName.IsEmpty() && !srcSDev.IsEmpty()) {
					SDEVMNGR->RemoveObject(srcSDev, _objName);
				}
				removeFl = false;
			}

		} else if (GetFileName() == "INSERT2") {
			if (!_objName.IsEmpty() && !dstSDev.IsEmpty()) {
				g_allowAttachActiveObjectsFl = false;
				SDEVMNGR->AddObject(dstSDev, _objName);
				g_allowAttachActiveObjectsFl = true;
			}

		} else if (GetFileName() == "INSERT") {
			if (!_objName.IsEmpty() && !dstSDev.IsEmpty()) {
				SDEVMNGR->AddObject(dstSDev, _objName);
			}

		} else if (GetFileName() == "TRANSFER") {

			if (!_objName.IsEmpty() && !srcSDev.IsEmpty() && !dstSDev.IsEmpty()) {
				SDEVMNGR->MoveObject(dstSDev, srcSDev, _objName);
			}

		} else if (GetFileName().Find("FLUSHQUEUE") == 0) {
			char buffer[20];
			int slot;

			slot = 0;
			Common::strcpy_s(buffer, GetFileName());
			if (strlen(buffer) == 11) {
				slot = buffer[10] - 48;
			}
			Assert(slot >= 0 && slot < 8);

		} else if (GetFileName() == "CLOSE") {
			CBagStorageDevWnd *currentSDev = CBagel::getBagApp()->getMasterWnd()->GetCurrentStorageDev();
			currentSDev->Close();

		} else if (GetFileName() == "UPDATE") {
			static bool updateFl = false;

			if (!updateFl) {
				updateFl = true;

				CBagStorageDevWnd *currentSDev = CBagel::getBagApp()->getMasterWnd()->GetCurrentStorageDev();
				currentSDev->AttachActiveObjects();

				updateFl = false;
			}

		} else if (GetFileName() == "EVENTLOOP") {
			static bool eventLoopFl = false;

			// prevent recursion
			//
			if (!eventLoopFl) {
				eventLoopFl = true;

				CBagStorageDev *currSDev = CBagel::getBagApp()->getMasterWnd()->GetCurrentStorageDev();
				if (currSDev != nullptr) {
					if (currSDev->GetDeviceType() == SDEV_GAMEWIN) {
						CBagPanWindow *panWin = (CBagPanWindow *)currSDev;
						panWin->onTimer(EVAL_EXPR);
					}
				}
				eventLoopFl = false;
			}

		} else if (GetFileName() == "STOPPAINT") {
			g_allowPaintFl = false;

		} else if (GetFileName() == "REMOVEWIELD") {
			// Remove currently wielded object from the game
			if (CBagPanWindow::m_pWieldBmp != nullptr) {
				CBagObject *currObj = CBagPanWindow::m_pWieldBmp->GetCurrObj();
				if (currObj != nullptr) {
					SDEVMNGR->RemoveObject(CBagPanWindow::m_pWieldBmp->GetName(), currObj->GetRefName());
				}
			}

		} else if (GetFileName() == "PAINT") {
			// Get a pointer to the current game window
			CBagStorageDevWnd *gameWin = CBagel::getBagApp()->getMasterWnd()->GetCurrentStorageDev();

			if (gameWin != nullptr) {
				gameWin->PaintScreen(nullptr);
			}

		} else if (GetFileName() == "STOPWAVE") {
			CBofSound::StopWaveSounds();

		} else if (GetFileName() == "STASHWIELD") {
			// Stash the currently wielded item
			if (CBagPanWindow::m_pWieldBmp != nullptr) {
				CBagObject *currObj = CBagPanWindow::m_pWieldBmp->GetCurrObj();
				if (currObj != nullptr) {
					SDEVMNGR->MoveObject("INV_WLD", CBagPanWindow::m_pWieldBmp->GetName(), currObj->GetRefName());
					CBagPanWindow::m_pWieldBmp->SetCurrObj(nullptr);
				}
			}

		} else if (GetFileName() == "DEATH") {
			CBagel::getBagApp()->getMasterWnd()->PostUserMessage(WM_DIE, 0);
			g_allowPaintFl = false;

		} else if (GetFileName() == "GOPAINT") {
			g_allowPaintFl = true;

		} else if (GetFileName() == "DEACTIVATEPDA") {

			// Get a pointer to the current game window
			CBagStorageDevWnd *gameWin = (CBagel::getBagApp()->getMasterWnd()->GetCurrentStorageDev());

			// Pull down the PDA (if it exists)
			if (gameWin->GetDeviceType() == SDEV_GAMEWIN) {
				((CBagPanWindow *)gameWin)->DeactivatePDA();
				((CBagPanWindow *)gameWin)->WaitForPDA();
			}

		} else if (GetFileName() == "ROTATETOFLY") {
			if (CBagMasterWin::GetFlyThru()) {
				CBagMasterWin::SetActiveCursor(6);
				CBagStorageDev *currSDev = CBagel::getBagApp()->getMasterWnd()->GetCurrentStorageDev();
				if ((currSDev != nullptr) && (currSDev->GetDeviceType() == SDEV_GAMEWIN)) {
					CBagPanWindow *currWin = (CBagPanWindow *)currSDev;
					currWin->RotateTo(GetPosition());
				}
			}

		} else if (GetFileName() == "ROTATETO") {
			CBagMasterWin::SetActiveCursor(6);
			CBagStorageDev *currSDev = CBagel::getBagApp()->getMasterWnd()->GetCurrentStorageDev();
			if ((currSDev != nullptr) && (currSDev->GetDeviceType() == SDEV_GAMEWIN)) {
				CBagPanWindow *currWin = (CBagPanWindow *)currSDev;
				currWin->RotateTo(GetPosition());
			}

		} else if (GetFileName() == "SETQVOL") {
			CBofSound::SetQVol(GetPosition().x, GetPosition().y);

		} else if (GetFileName() == "TURN") {

			// Cause 1 turn to go by
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
			CBagDossierObject::deactivateCurDossier();

		} else if (GetFileName() == "RP_ACTIVATE_REVIEW") {
			// Execute any waiting residue printing results.
			CBagRPObject::ActivateRPReview();

		} else if (GetFileName() == "SAVESTACK") {
			CBagMasterWin *pWin = CBagel::getBagApp()->getMasterWnd();

			if (pWin != nullptr) {
				pWin->SaveSDevStack();
			}

		} else if (GetFileName() == "DEATH2") {
			CBagel::getBagApp()->getMasterWnd()->PostUserMessage(WM_DIE, 2);
			g_allowPaintFl = false;

		} else if (GetFileName() == "ENTRYTHUD") { // Thud
			CBofRect tmpRect(60, 50, 137, 70);
			CBofString bmpName("$SBARDIR\\GENERAL\\POPUP\\THUDPOP.BMP");
			MACROREPLACE(bmpName);
			CDevDlg dlg;
			dlg.create(bmpName, CBagel::getBagApp()->getMasterWnd(), CBofApp::GetApp()->GetPalette(), &tmpRect);
			dlg.doModal();
			dlg.Destroy();

		} else if (GetFileName() == "ENTRYFLEE") { // Fleebix
			CBofRect tmpRect(35, 48, 114, 69);
			CBofString bmpName("$SBARDIR\\GENERAL\\POPUP\\FLEEDAT.BMP");
			MACROREPLACE(bmpName);
			CDevDlg dlg;
			dlg.create(bmpName, CBagel::getBagApp()->getMasterWnd(), CBofApp::GetApp()->GetPalette(), &tmpRect);
			dlg.doModal();
			dlg.Destroy();

		} else if (GetFileName() == "ENTRYCLICK") { // Click's locker combo
			CBofRect tmpRect(60, 49, 138, 68);
			CBofString bmpName("$SBARDIR\\GENERAL\\POPUP\\CLICKPOP.BMP");
			MACROREPLACE(bmpName);
			CDevDlg dlg;
			dlg.create(bmpName, CBagel::getBagApp()->getMasterWnd(), CBofApp::GetApp()->GetPalette(), &tmpRect);
			dlg.doModal();
			dlg.Destroy();

		} else if (GetFileName() == "ENTRYFRUIT") { // MegaWave the fruit
			CBofRect tmpRect(35, 49, 114, 68);
			CBofString bmpName("$SBARDIR\\GENERAL\\POPUP\\CLICKDAT.BMP");
			MACROREPLACE(bmpName);
			CDevDlg dlg;
			dlg.create(bmpName, CBagel::getBagApp()->getMasterWnd(), CBofApp::GetApp()->GetPalette(), &tmpRect);
			dlg.doModal();
			dlg.Destroy();

		} else if (GetFileName() == "D7CODE1DLG") { // Deven-7 code word
			CBofRect tmpRect(10, 48, 189, 69);
			CBofString bmpName("$SBARDIR\\GENERAL\\POPUP\\DEVENPOP.BMP");
			MACROREPLACE(bmpName);
			CDevDlg dlg;
			dlg.create(bmpName, CBagel::getBagApp()->getMasterWnd(), CBofApp::GetApp()->GetPalette(), &tmpRect, true);
			dlg.doModal();
			dlg.Destroy();

		} else if (GetFileName() == "SNAPTO") {
			CBagStorageDev *currSDev = CBagel::getBagApp()->getMasterWnd()->GetCurrentStorageDev();
			if ((currSDev != nullptr) && (currSDev->GetDeviceType() == SDEV_GAMEWIN)) {
				CBagPanWindow *currWin= (CBagPanWindow *)currSDev;

				if (currWin->GetSlideBitmap() != nullptr) {
					int x = GetPosition().x;
					int y = GetPosition().y;

					CBofRect cRect;
					cRect.SetRect(x, y, x + 480 - 1, y + 360 - 1);
					currWin->GetSlideBitmap()->SetCurrView(cRect);
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
			CBagStorageDevWnd *mainWin = (CBagel::getBagApp()->getMasterWnd()->GetCurrentStorageDev());
			// Pull up the PDA (if it exists)
			//
			if (mainWin->GetDeviceType() == SDEV_GAMEWIN) {
				((CBagPanWindow *)mainWin)->ActivatePDA();
				((CBagPanWindow *)mainWin)->WaitForPDA();
			}

		} else if (GetFileName() == "SHOWPDALOG") {
			// Get a pointer to the current game window
			CBagPDA *pda = (CBagPDA *)SDEVMNGR->GetStorageDevice("BPDA_WLD");
			if (pda) {
				pda->showLog();
			}

		} else if (GetFileName() == "OBJLIST") {
			g_restoreObjectListFl = false;

		} else if (GetFileName() == "WIN") {
			CBagMasterWin *mainWin = CBagel::getBagApp()->getMasterWnd();

			if (mainWin != nullptr) {
				CBofBitmap bmp(640, 480, CBofApp::GetApp()->GetPalette());
				bmp.FillRect(nullptr, COLOR_BLACK);

				bmp.paint(mainWin, 0, 0);
				g_allowPaintFl = false;

				CBofString smkName("$SBARDIR\\BAR\\EVGAMWIN.SMK");
				MACROREPLACE(smkName);

				// Play the movie only if it exists
				if (FileExists(smkName.GetBuffer())) {
					BofPlayMovie(mainWin, smkName.GetBuffer());
					bmp.paint(mainWin, 0, 0);
				}
			}

			// Display Credits
			mainWin->ShowCreditsDialog(mainWin);

			// Exit the game
			CBagel::getBagApp()->getMasterWnd()->Close();
			g_engine->quitGame();

		} else if (GetFileName() == "BREAK") {
			// As far as I'm aware, this can't actually happen in the game,
			// but what the heck, ScummVM has a console, let's hook it up
			g_engine->getDebugger()->attach();

		} else {
			ReportError(ERR_UNKNOWN, "Invalid RUN COMMAND = %s", (const char *)GetFileName());
		}

		rc = CBagObject::runObject();

		iteration--;
	}

	return rc;
}

PARSE_CODES CBagCommandObject::setInfo(bof_ifstream &istr) {
	bool objectUpdatedFl = false;

	while (!istr.eof()) {
		istr.EatWhite();

		char ch = (char)istr.peek();
		switch (ch) {
		//
		//  OBJECT
		//
		case 'O': {
			char localBuffer[256];
			localBuffer[0] = 0;
			CBofString curStr(localBuffer, 256);
			GetAlphaNumFromStream(istr, curStr);

			if (!curStr.Find("OBJECT")) {
				istr.EatWhite();
				GetAlphaNumFromStream(istr, _objName);
				objectUpdatedFl = true;
			} else {
				PutbackStringOnStream(istr, curStr);
			}
			break;
		}
		//
		//  FROM
		//
		case 'F': {
			char localBuffer[256];
			localBuffer[0] = 0;
			CBofString curStr(localBuffer, 256);
			GetAlphaNumFromStream(istr, curStr);

			if (!curStr.Find("FROM")) {
				istr.EatWhite();
				GetAlphaNumFromStream(istr, _srcSDev);
				objectUpdatedFl = true;
			} else {
				PutbackStringOnStream(istr, curStr);
			}
			break;
		}
		//
		//  TO
		//
		case 'T': {
			char localBuffer[256];
			localBuffer[0] = 0;
			CBofString curStr(localBuffer, 256);
			GetAlphaNumFromStream(istr, curStr);

			if (!curStr.Find("TO")) {
				istr.EatWhite();
				GetAlphaNumFromStream(istr, _destSDev);
				objectUpdatedFl = true;
			} else {
				PutbackStringOnStream(istr, curStr);
			}
			break;
		}

		//
		//  No match return from function
		//
		default: {
			PARSE_CODES rc = CBagObject::setInfo(istr);
			if (rc == PARSING_DONE) {
				return PARSING_DONE;
			}

			if (rc == UPDATED_OBJECT) {
				objectUpdatedFl = true;
			} else { // rc==UNKNOWN_TOKEN
				if (objectUpdatedFl)
					return UPDATED_OBJECT;

				return UNKNOWN_TOKEN;
			}
			break;
		}
		}
	}

	return PARSING_DONE;
}

} // namespace Bagel
