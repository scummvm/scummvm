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
extern bool g_allowattachActiveObjectsFl;

CBagCommandObject::CBagCommandObject() {
	_xObjType = COMMANDOBJ;
	setVisible(false);
	setTimeless(true);
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
		if (!srcSDev.isEmpty() && !srcSDev.find(CURRSDEV_TOKEN)) {
			srcSDev = CBagel::getBagApp()->getMasterWnd()->getCurrentStorageDev()->GetName();
		}

		if (!dstSDev.isEmpty() && !dstSDev.find(CURRSDEV_TOKEN)) {
			dstSDev = CBagel::getBagApp()->getMasterWnd()->getCurrentStorageDev()->GetName();
		}

		// Check if these items should be replaced by the previous sdev
		if (!srcSDev.isEmpty() && !srcSDev.find(PREVSDEV_TOKEN)) {
			srcSDev = CBagel::getBagApp()->getMasterWnd()->getCurrentStorageDev()->getPrevSDev();
		}

		if (!dstSDev.isEmpty() && !dstSDev.find(PREVSDEV_TOKEN)) {
			dstSDev = CBagel::getBagApp()->getMasterWnd()->getCurrentStorageDev()->getPrevSDev();
		}

		if (getFileName() == "REMOVE") {
			static bool removeFl = false;
			if (!removeFl) {
				removeFl = true;

				if (!_objName.isEmpty() && !srcSDev.isEmpty()) {
					SDEV_MANAGER->removeObject(srcSDev, _objName);
				}
				removeFl = false;
			}

		} else if (getFileName() == "INSERT2") {
			if (!_objName.isEmpty() && !dstSDev.isEmpty()) {
				g_allowattachActiveObjectsFl = false;
				SDEV_MANAGER->AddObject(dstSDev, _objName);
				g_allowattachActiveObjectsFl = true;
			}

		} else if (getFileName() == "INSERT") {
			if (!_objName.isEmpty() && !dstSDev.isEmpty()) {
				SDEV_MANAGER->AddObject(dstSDev, _objName);
			}

		} else if (getFileName() == "TRANSFER") {

			if (!_objName.isEmpty() && !srcSDev.isEmpty() && !dstSDev.isEmpty()) {
				SDEV_MANAGER->MoveObject(dstSDev, srcSDev, _objName);
			}

		} else if (getFileName().find("FLUSHQUEUE") == 0) {
			char buffer[20];
			int slot;

			slot = 0;
			Common::strcpy_s(buffer, getFileName());
			if (strlen(buffer) == 11) {
				slot = buffer[10] - 48;
			}
			assert(slot >= 0 && slot < 8);

		} else if (getFileName() == "CLOSE") {
			CBagStorageDevWnd *currentSDev = CBagel::getBagApp()->getMasterWnd()->getCurrentStorageDev();
			currentSDev->close();

		} else if (getFileName() == "UPDATE") {
			static bool updateFl = false;

			if (!updateFl) {
				updateFl = true;

				CBagStorageDevWnd *currentSDev = CBagel::getBagApp()->getMasterWnd()->getCurrentStorageDev();
				currentSDev->attachActiveObjects();

				updateFl = false;
			}

		} else if (getFileName() == "EVENTLOOP") {
			static bool eventLoopFl = false;

			// prevent recursion
			//
			if (!eventLoopFl) {
				eventLoopFl = true;

				CBagStorageDev *currSDev = CBagel::getBagApp()->getMasterWnd()->getCurrentStorageDev();
				if (currSDev != nullptr) {
					if (currSDev->GetDeviceType() == SDEV_GAMEWIN) {
						CBagPanWindow *panWin = (CBagPanWindow *)currSDev;
						panWin->onTimer(EVAL_EXPR);
					}
				}
				eventLoopFl = false;
			}

		} else if (getFileName() == "STOPPAINT") {
			g_allowPaintFl = false;

		} else if (getFileName() == "REMOVEWIELD") {
			// Remove currently wielded object from the game
			if (CBagPanWindow::_pWieldBmp != nullptr) {
				CBagObject *currObj = CBagPanWindow::_pWieldBmp->getCurrObj();
				if (currObj != nullptr) {
					SDEV_MANAGER->removeObject(CBagPanWindow::_pWieldBmp->GetName(), currObj->getRefName());
				}
			}

		} else if (getFileName() == "PAINT") {
			// Get a pointer to the current game window
			CBagStorageDevWnd *gameWin = CBagel::getBagApp()->getMasterWnd()->getCurrentStorageDev();

			if (gameWin != nullptr) {
				gameWin->PaintScreen(nullptr);
			}

		} else if (getFileName() == "STOPWAVE") {
			CBofSound::stopWaveSounds();

		} else if (getFileName() == "STASHWIELD") {
			// Stash the currently wielded item
			if (CBagPanWindow::_pWieldBmp != nullptr) {
				CBagObject *currObj = CBagPanWindow::_pWieldBmp->getCurrObj();
				if (currObj != nullptr) {
					SDEV_MANAGER->MoveObject("INV_WLD", CBagPanWindow::_pWieldBmp->GetName(), currObj->getRefName());
					CBagPanWindow::_pWieldBmp->setCurrObj(nullptr);
				}
			}

		} else if (getFileName() == "DEATH") {
			CBagel::getBagApp()->getMasterWnd()->postUserMessage(WM_DIE, 0);
			g_allowPaintFl = false;

		} else if (getFileName() == "GOPAINT") {
			g_allowPaintFl = true;

		} else if (getFileName() == "DEACTIVATEPDA") {

			// Get a pointer to the current game window
			CBagStorageDevWnd *gameWin = (CBagel::getBagApp()->getMasterWnd()->getCurrentStorageDev());

			// Pull down the PDA (if it exists)
			if (gameWin->GetDeviceType() == SDEV_GAMEWIN) {
				((CBagPanWindow *)gameWin)->deactivatePDA();
				((CBagPanWindow *)gameWin)->waitForPDA();
			}

		} else if (getFileName() == "ROTATETOFLY") {
			if (CBagMasterWin::getFlyThru()) {
				CBagMasterWin::setActiveCursor(6);
				CBagStorageDev *currSDev = CBagel::getBagApp()->getMasterWnd()->getCurrentStorageDev();
				if ((currSDev != nullptr) && (currSDev->GetDeviceType() == SDEV_GAMEWIN)) {
					CBagPanWindow *currWin = (CBagPanWindow *)currSDev;
					currWin->rotateTo(getPosition());
				}
			}

		} else if (getFileName() == "ROTATETO") {
			CBagMasterWin::setActiveCursor(6);
			CBagStorageDev *currSDev = CBagel::getBagApp()->getMasterWnd()->getCurrentStorageDev();
			if ((currSDev != nullptr) && (currSDev->GetDeviceType() == SDEV_GAMEWIN)) {
				CBagPanWindow *currWin = (CBagPanWindow *)currSDev;
				currWin->rotateTo(getPosition());
			}

		} else if (getFileName() == "SETQVOL") {
			CBofSound::setQVol(getPosition().x, getPosition().y);

		} else if (getFileName() == "TURN") {

			// Cause 1 turn to go by
			VAR_MANAGER->IncrementTimers();

		} else if (getFileName() == "RP_UPDATE_QUEUE") {
			// Execute any waiting residue printing results.
			CBagRPObject::updateRPQueue();

		} else if (getFileName() == "RP_RUN_QUEUE") {
			// Execute any waiting residue printing results.
			CBagRPObject::runRPQueue();

		} else if (getFileName() == "RP_DEACTIVATE_RESULTS") {
			// Execute any waiting residue printing results.
			CBagRPObject::deactivateRPQueue();

		} else if (getFileName() == "RP_DEACTIVATE_REVIEW") {
			// Execute any waiting residue printing results.
			CBagRPObject::deactivateRPReview();

		} else if (getFileName() == "RP_DEACTIVATE_DOSSIER") {
			// Execute any waiting residue printing results.
			CBagDossierObject::deactivateCurDossier();

		} else if (getFileName() == "RP_ACTIVATE_REVIEW") {
			// Execute any waiting residue printing results.
			CBagRPObject::activateRPReview();

		} else if (getFileName() == "SAVESTACK") {
			CBagMasterWin *pWin = CBagel::getBagApp()->getMasterWnd();

			if (pWin != nullptr) {
				pWin->saveSDevStack();
			}

		} else if (getFileName() == "DEATH2") {
			CBagel::getBagApp()->getMasterWnd()->postUserMessage(WM_DIE, 2);
			g_allowPaintFl = false;

		} else if (getFileName() == "ENTRYTHUD") { // Thud
			CBofRect tmpRect(60, 50, 137, 70);
			CBofString bmpName("$SBARDIR\\GENERAL\\POPUP\\THUDPOP.BMP");
			MACROREPLACE(bmpName);
			CDevDlg dlg;
			dlg.create(bmpName, CBagel::getBagApp()->getMasterWnd(), CBofApp::getApp()->getPalette(), &tmpRect);
			dlg.doModal();
			dlg.destroy();

		} else if (getFileName() == "ENTRYFLEE") { // Fleebix
			CBofRect tmpRect(35, 48, 114, 69);
			CBofString bmpName("$SBARDIR\\GENERAL\\POPUP\\FLEEDAT.BMP");
			MACROREPLACE(bmpName);
			CDevDlg dlg;
			dlg.create(bmpName, CBagel::getBagApp()->getMasterWnd(), CBofApp::getApp()->getPalette(), &tmpRect);
			dlg.doModal();
			dlg.destroy();

		} else if (getFileName() == "ENTRYCLICK") { // Click's locker combo
			CBofRect tmpRect(60, 49, 138, 68);
			CBofString bmpName("$SBARDIR\\GENERAL\\POPUP\\CLICKPOP.BMP");
			MACROREPLACE(bmpName);
			CDevDlg dlg;
			dlg.create(bmpName, CBagel::getBagApp()->getMasterWnd(), CBofApp::getApp()->getPalette(), &tmpRect);
			dlg.doModal();
			dlg.destroy();

		} else if (getFileName() == "ENTRYFRUIT") { // MegaWave the fruit
			CBofRect tmpRect(35, 49, 114, 68);
			CBofString bmpName("$SBARDIR\\GENERAL\\POPUP\\CLICKDAT.BMP");
			MACROREPLACE(bmpName);
			CDevDlg dlg;
			dlg.create(bmpName, CBagel::getBagApp()->getMasterWnd(), CBofApp::getApp()->getPalette(), &tmpRect);
			dlg.doModal();
			dlg.destroy();

		} else if (getFileName() == "D7CODE1DLG") { // Deven-7 code word
			CBofRect tmpRect(10, 48, 189, 69);
			CBofString bmpName("$SBARDIR\\GENERAL\\POPUP\\DEVENPOP.BMP");
			MACROREPLACE(bmpName);
			CDevDlg dlg;
			dlg.create(bmpName, CBagel::getBagApp()->getMasterWnd(), CBofApp::getApp()->getPalette(), &tmpRect, true);
			dlg.doModal();
			dlg.destroy();

		} else if (getFileName() == "SNAPTO") {
			CBagStorageDev *currSDev = CBagel::getBagApp()->getMasterWnd()->getCurrentStorageDev();
			if ((currSDev != nullptr) && (currSDev->GetDeviceType() == SDEV_GAMEWIN)) {
				CBagPanWindow *currWin= (CBagPanWindow *)currSDev;

				if (currWin->getSlideBitmap() != nullptr) {
					int x = getPosition().x;
					int y = getPosition().y;

					CBofRect cRect;
					cRect.setRect(x, y, x + 480 - 1, y + 360 - 1);
					currWin->getSlideBitmap()->setCurrView(cRect);
				}
			}

		} else if (getFileName() == "RESUMESOUND") {
			CBofSound::resumeSounds();

		} else if (getFileName() == "PAUSESOUND") {
			CBofSound::pauseSounds();

		} else if (getFileName() == "STOPMIDI") {
			CBofSound::stopMidiSounds();

		} else if (getFileName() == "WAITWAVE") {
			CBofSound::waitWaveSounds();

		} else if (getFileName() == "WAITMIDI") {
			CBofSound::waitMidiSounds();

		} else if (getFileName() == "ACTIVATEPDA") {
			// Get a pointer to the current game window
			CBagStorageDevWnd *mainWin = (CBagel::getBagApp()->getMasterWnd()->getCurrentStorageDev());
			// Pull up the PDA (if it exists)
			//
			if (mainWin->GetDeviceType() == SDEV_GAMEWIN) {
				((CBagPanWindow *)mainWin)->activatePDA();
				((CBagPanWindow *)mainWin)->waitForPDA();
			}

		} else if (getFileName() == "SHOWPDALOG") {
			// Get a pointer to the current game window
			CBagPDA *pda = (CBagPDA *)SDEV_MANAGER->GetStorageDevice("BPDA_WLD");
			if (pda) {
				pda->showLog();
			}

		} else if (getFileName() == "OBJLIST") {
			g_restoreObjectListFl = false;

		} else if (getFileName() == "WIN") {
			CBagMasterWin *mainWin = CBagel::getBagApp()->getMasterWnd();

			if (mainWin != nullptr) {
				CBofBitmap bmp(640, 480, CBofApp::getApp()->getPalette());
				bmp.fillRect(nullptr, COLOR_BLACK);

				bmp.paint(mainWin, 0, 0);
				g_allowPaintFl = false;

				CBofString smkName("$SBARDIR\\BAR\\EVGAMWIN.SMK");
				MACROREPLACE(smkName);

				// Play the movie only if it exists
				if (fileExists(smkName.getBuffer())) {
					bofPlayMovie(mainWin, smkName.getBuffer());
					bmp.paint(mainWin, 0, 0);
				}
			}

			// Display Credits
			mainWin->showCreditsDialog(mainWin);

			// Exit the game
			CBagel::getBagApp()->getMasterWnd()->close();
			g_engine->quitGame();

		} else if (getFileName() == "BREAK") {
			// As far as I'm aware, this can't actually happen in the game,
			// but what the heck, ScummVM has a console, let's hook it up
			g_engine->getDebugger()->attach();

		} else {
			reportError(ERR_UNKNOWN, "Invalid RUN COMMAND = %s", (const char *)getFileName());
		}

		rc = CBagObject::runObject();

		iteration--;
	}

	return rc;
}

PARSE_CODES CBagCommandObject::setInfo(CBagIfstream &istr) {
	bool objectUpdatedFl = false;

	while (!istr.eof()) {
		istr.eatWhite();

		char ch = (char)istr.peek();
		switch (ch) {
		//
		//  OBJECT
		//
		case 'O': {
			char localBuffer[256];
			localBuffer[0] = 0;
			CBofString curStr(localBuffer, 256);
			getAlphaNumFromStream(istr, curStr);

			if (!curStr.find("OBJECT")) {
				istr.eatWhite();
				getAlphaNumFromStream(istr, _objName);
				objectUpdatedFl = true;
			} else {
				putbackStringOnStream(istr, curStr);
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
			getAlphaNumFromStream(istr, curStr);

			if (!curStr.find("FROM")) {
				istr.eatWhite();
				getAlphaNumFromStream(istr, _srcSDev);
				objectUpdatedFl = true;
			} else {
				putbackStringOnStream(istr, curStr);
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
			getAlphaNumFromStream(istr, curStr);

			if (!curStr.find("TO")) {
				istr.eatWhite();
				getAlphaNumFromStream(istr, _destSDev);
				objectUpdatedFl = true;
			} else {
				putbackStringOnStream(istr, curStr);
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
