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

#include "common/file.h"
#include "bagel/spacebar/baglib/movie_object.h"
#include "bagel/spacebar/baglib/exam.h"
#include "bagel/spacebar/baglib/master_win.h"
#include "bagel/spacebar/baglib/pan_window.h"
#include "bagel/boflib/sound.h"
#include "bagel/spacebar/baglib/fmovie.h"
#include "bagel/spacebar/baglib/zoom_pda.h"
#include "bagel/boflib/file_functions.h"
#include "bagel/boflib/log.h"

namespace Bagel {
namespace SpaceBar {

#define SOUND_FILE_EXT_LOWER ".wav"
#define SOUND_FILE_EXT_UPPER ".WAV"
#define OVERRIDE_SMK "$SBARDIR\\BAR\\LOG\\OVERRIDE.SMK"

CBagMovieObject::CBagMovieObject() {
	_xObjType = MOVIE_OBJ;
	setVisible(true);
	_xDisplayType = dispType::MOVIE;
	_bFlyThru = false;

	// Initialize asynch flags to be off by default.
	_nAsynchFlags = 0;

	// Allow user to force movie not to increment (default = increment = true)
	setIncrement();

	// Allow movie to play on a black background (default, nada).
	setOnBlack(false);

	// Default is no associated sound.
	setAssociatedSound(nullptr);
}

CBagMovieObject::~CBagMovieObject() {
	CBagObject::detach();

	// Could still by lying around in the pda movie queue,
	// Make sure it has been removed.
	if (_xDisplayType == dispType::ASYNCH_PDA_MSG) {
		CBagPDA::removeFromMovieQueue(this);
	}

	// If there's a sound with this guy, then trash it here.
	if (getAssociatedSound()) {
		delete getAssociatedSound();
		setAssociatedSound(nullptr);
	}
}

bool CBagMovieObject::runObject() {
	CBofWindow *pNewWin = nullptr;
	SBZoomPda *pPDAz = (SBZoomPda *)g_SDevManager->getStorageDevice("BPDAZ_WLD");
	const bool bZoomed = (pPDAz == nullptr ? false : pPDAz->getZoomed());

	// Get a pointer to the current game window
	CBagStorageDevWnd *pMainWin = CBagel::getBagApp()->getMasterWnd()->getCurrentStorageDev();

	bool rc = true;
	if (!_bFlyThru || CBagMasterWin::getFlyThru()) {
		rc = false;

		CBofString sFileName = getFileName();
		const int nExt = sFileName.getLength() - 4; // ".EXT"

		if (nExt <= 0) {
			logError("Movie does not have a file name or proper extension.  Please write better scripts.");
			return rc;
		}

		//
		// This would be much cooler if it were a cast to another object type and
		// then a run.  But this is a quicker fix.
		//
		enum class MovieFileType {
			NONE = 0,
			TEXT = 1,
			SOUND = 2,
			MOVIE = 3
		} nMovFileType;
		const CBofString sBaseStr = sFileName.left(nExt);

		if (sFileName.find(".smk") > 0 || sFileName.find(".SMK") > 0) {
			nMovFileType = MovieFileType::MOVIE;
		} else if (sFileName.find(SOUND_FILE_EXT_LOWER) > 0 || sFileName.find(SOUND_FILE_EXT_UPPER) > 0) {
			nMovFileType = MovieFileType::SOUND;
		} else if (sFileName.find(".txt") > 0 || sFileName.find(".TXT") > 0) {
			nMovFileType = MovieFileType::TEXT;
		} else {
			nMovFileType = MovieFileType::NONE;
		}

		// Look for .SMK then .WAV, then .TXT
		while (!fileExists(sFileName.getBuffer())) {
			switch (nMovFileType) {

			case MovieFileType::MOVIE:
				sFileName = sBaseStr + SOUND_FILE_EXT_LOWER;
				nMovFileType = MovieFileType::SOUND;
				break;

			case MovieFileType::SOUND:
				sFileName = sBaseStr + ".txt";
				nMovFileType = MovieFileType::TEXT;
				break;

			case MovieFileType::TEXT:
				bofMessageBox(sFileName.getBuffer(), "Could not find asset");
				nMovFileType = MovieFileType::NONE;
				break;

			// We should never get here
			case MovieFileType::NONE:
			default:
				logError(buildString("Movie does not have a correct file name: %s.", sFileName.getBuffer()));
				return rc;
			}
		}

		if (nMovFileType == MovieFileType::MOVIE) {
			bool isFiltered = false;
			CBagMasterWin *pWnd = CBagel::getBagApp()->getMasterWnd();
			CBagStorageDevWnd *pSDevWnd = (pWnd ? pWnd->getCurrentStorageDev() : nullptr);

			// Get the pda here, we need it so much anyway.
			CBagPDA *pPDA = (CBagPDA *)g_SDevManager->getStorageDevice("BPDA_WLD");

			assert(pPDA != nullptr);

			// If we have an asnych movie to play, make sure it is a good
			// time to play it, if not, then queue it up so it can play at a much better time.
			if (_xDisplayType == dispType::ASYNCH_PDA_MSG) {
				if (asynchPDAMovieCanPlay() == false) {
					pPDA->addToMovieQueue(this);
					return rc;
				}
			} else if ((pWnd != nullptr) && (pSDevWnd != nullptr)) {
				// Don't need to redraw for asynch pda messages, this just
				// messes things up in the PDA redraw code
				pSDevWnd->paintScreen(nullptr);
				isFiltered = pSDevWnd->isFiltered();
			}

			if (_xDisplayType == dispType::EXAMINE) {
				CBofRect r(160, 60, 480, 300);

				// Offset the rect for the movies to compensate for all screen sizes
				r.offsetRect(((CBofWindow *)pMainWin)->getWindowRect().topLeft());

				// If we have a movie playing in the zoom pda, then black out
				// the background.  Examine movies will always play with a black background
				// on the mac (prevents a palette shift).
				CBagExam *pMovie = new CBagExam(CBagel::getBagApp()->getMasterWnd()->getCurrentGameWindow(), sFileName, &r);

				// If there is an associated sound file, then start it up here.
				CBagSoundObject *pSObj = getAssociatedSound();
				if (pSObj) {
					if (pSObj->isAttached() == false) {
						pSObj->attach();
					}
					pSObj->runObject();
				}

				CBofWindow *wnd = CBagel::getBagApp()->getMasterWnd();
				pMovie->show();
				CBofApp::getApp()->getMainWindow()->flushAllMessages();
				wnd->flushAllMessages();
				pMovie->initExam();
				delete pMovie;

				// As soon as we're done, detach (this will also stop the sound).
				if (pSObj) {
					pSObj->detach();
				}
			} else {
				bool bActivated = false;
				CBofRect r(80, 10, 80 + 480 - 1, 10 + 360 - 1);

				// Offset the rect for the movies to compensate for all screen sizes
				r.offsetRect(((CBofWindow *)pMainWin)->getWindowRect().topLeft());

				if (_xDisplayType == dispType::PDA_MSG || _xDisplayType == dispType::ASYNCH_PDA_MSG) {
					// Pull up the PDA (if it exists)
					// Only pull up the PDA if we're not playing an asynch movie
					if (_xDisplayType == dispType::PDA_MSG) {
						// Increment timer one, pda message counts as one turn
						// Allow scripter to override timer increment
						if (isIncrement()) {
							g_VarManager->incrementTimers();
						}
					}

					if (pMainWin->getDeviceType() == SDEV_GAMEWIN) {
						// If the pda is going up or down, then wait for it
						// To do its thing before attempting to activate it.
						if (pPDA->isActivating()) {
							((CBagPanWindow *)pMainWin)->waitForPDA();
						}

						if (pPDA->isActivated() == false) {
							bActivated = ((CBagPanWindow *)pMainWin)->activatePDA();
							((CBagPanWindow *)pMainWin)->waitForPDA();
						}
					}
				} else {

					// Pull down the PDA (if it exists)
					if (pMainWin->getDeviceType() == SDEV_GAMEWIN) {
						((CBagPanWindow *)pMainWin)->deactivatePDA();
						((CBagPanWindow *)pMainWin)->waitForPDA();
					}
				}

				// Use the position scripted for the movie if it exists
				// The bottom, right is unimportant because movie won't resize anyway
				CBofPoint p = CBagObject::getPosition();

				if (p.x != -1 && p.y != -1)
					r.offsetRect(p);

				if (isFiltered) {
					if (bZoomed) {
						pNewWin = new CBofWindow();
						pNewWin->create("BLACK", 0, 0, 640, 480, CBofApp::getApp()->getMainWindow(), 0);
						pNewWin->fillWindow(COLOR_BLACK);
					}

					CBagFMovie *pMovie = new CBagFMovie(CBofApp::getApp()->getMainWindow(), sFileName, &r);

					if (pMovie->errorOccurred())
						logError(buildString("Movie file could not be read: %s.  How? You removed that CD again didn't you", sFileName.getBuffer()));
					else {
						pMovie->show();
						CBofApp::getApp()->getMainWindow()->flushAllMessages();
						pWnd->flushAllMessages();
						pMovie->play(false);
					}
					delete pMovie;
					pMovie = nullptr;

					delete pNewWin;
					pNewWin = nullptr;
				} else {
					// Hack.. allow script to override some other movies.
					if ((_xDisplayType == dispType::PDA_MSG) && pMainWin->isCIC() && isDontOverride() == false) {

						char szLocalBuff[256];
						CBofString cStr(szLocalBuff, 256);

						// Play the override message.
						cStr = OVERRIDE_SMK;
						fixPathName(cStr);

						sFileName = cStr;
					}

					if (_xDisplayType == dispType::ASYNCH_PDA_MSG) {
						// Tell our PDA to switch gears to do asynch movie time.
						if (pPDA->showMovie()) {       // Returns false if another movie playing
							pPDA->setMovie(sFileName); // Set the movie to play
						}
					} else {
						CBofMovie *pMovie;

						if (bZoomed && _xDisplayType != dispType::PDA_MSG) {
							pNewWin = new CBofWindow();
							pNewWin->create("BLACK", 0, 0, 640, 480, CBofApp::getApp()->getMainWindow(), 0);
							pNewWin->show();
							pNewWin->fillWindow(COLOR_BLACK);
						}

						// If playing a PDA message while the PDA is zoomed
						if (bZoomed && _xDisplayType == dispType::PDA_MSG) {
							// Then stretch it to fit into the PDA's viewscreen
							r.setRect(24, 47, 28 + 600 - 1, 47 + 302 - 1);
							pMovie = new CBofMovie(CBofApp::getApp()->getMainWindow(), sFileName, &r, true);

						} else {
							// Otherwise, just play the movie normally
							pMovie = new CBofMovie(CBofApp::getApp()->getMainWindow(), sFileName, &r);
						}

						if (pMovie->errorOccurred())
							logError(buildString("Movie file could not be read: %s.  How? You removed that CD again didn't you", sFileName.getBuffer()));
						else {
							// Stop any asnych movies already playing
							pPDA->stopMovie(true);
							pMovie->show();
							CBofApp::getApp()->getMainWindow()->flushAllMessages();
							pWnd->flushAllMessages();
							pMovie->play(false);
						}

						delete pMovie;
						pMovie = nullptr;

						// If we put a black window up, then
						delete pNewWin;
						pNewWin = nullptr;
					}
				}

				// Put the pda down if we brought it up. (8638)
				if (_xDisplayType != dispType::ASYNCH_PDA_MSG && bActivated) {
					((CBagPanWindow *)pMainWin)->deactivatePDA();
					((CBagPanWindow *)pMainWin)->waitForPDA();
				}

				// If we're asynch, then let it know to deactivate when done playing.
				if (_xDisplayType == dispType::ASYNCH_PDA_MSG) {
					pPDA->setDeactivate(bActivated);
				}
			}

			// Movies usually mark the transition from one view to another
			// but not necessarily a change of sdev's, so make sure we repaint the
			// backdrop
			pMainWin->setPreFilterPan(true);

		} else if (nMovFileType == MovieFileType::SOUND) {
			CBofSound *pSound = new CBofSound(CBofApp::getApp()->getMainWindow(), sFileName, SOUND_WAVE);
			pSound->play();
			delete pSound;
		} else if (nMovFileType == MovieFileType::TEXT) {
			Common::File f;
			if (f.open(sFileName.getBuffer())) {
				Common::String line = f.readLine();

				bofMessageBox(line.c_str(), "Incoming Message...");
				f.close();
			} else {
				logError(buildString("Movie TEXT file could not be read: %s.  Why? because we like you ...", sFileName.getBuffer()));
			}
		}

		rc = CBagObject::runObject();
	}

	return rc;
}

ParseCodes CBagMovieObject::setInfo(CBagIfstream &istr) {
	bool nObjectUpdated = false;
	char szLocalStr[256];
	CBofString sStr(szLocalStr, 256);

	while (!istr.eof()) {
		istr.eatWhite(); // Eat any white space between script elements

		const char ch = (char)istr.peek();
		switch (ch) {
		//
		//  AS  - n number of slides in sprite
		//
		case 'A': {
			getAlphaNumFromStream(istr, sStr);

			if (!sStr.find("AS")) {
				istr.eatWhite();
				getAlphaNumFromStream(istr, sStr);
				if (!sStr.find("EXAMINE")) {
					_xDisplayType = dispType::EXAMINE;
				} else if (!sStr.find("MOVIE")) {
					_xDisplayType = dispType::MOVIE;

				} else if (!sStr.find("FLYTHRU")) {
					_xDisplayType = dispType::MOVIE;
					_bFlyThru = true;
				} else if (!sStr.find("PDAMSG")) {
					_xDisplayType = dispType::PDA_MSG;
				} else if (!sStr.find("ASYNCH_PDAMSG")) {
					_xDisplayType = dispType::ASYNCH_PDA_MSG;

					// see if this improves performance any...
					setPreload(true);
				}
			}

			nObjectUpdated = true;
		}
		break;

		// Don't queue attribute, when set, the asynch movie either plays
		// immediately or not at all.
		case 'D': {
			getAlphaNumFromStream(istr, sStr);

			if (!sStr.find("DONTQUEUE")) {
				setDontQueue();
				nObjectUpdated = true;
			} else if (!sStr.find("DONTOVERRIDE")) {
				setDontOverride();
				nObjectUpdated = true;
			} else if (!sStr.find("DONTINCREMENT")) {
				// Don't increment the timer when playing this movie
				setIncrement(false);
				nObjectUpdated = true;
			} else {
				putbackStringOnStream(istr, sStr);
			}
		}
		break;

		// Don't queue attribute, when set, the asynch movie either plays
		// immediately or not at all.
		case 'P': {
			getAlphaNumFromStream(istr, sStr);

			if (!sStr.find("PLAYIMMEDIATE")) {
				setPlayImmediate();
				nObjectUpdated = true;
			} else {
				putbackStringOnStream(istr, sStr);
			}
		}
		break;

		// This is probably going to be much more important to the
		// mac version... give the option of playing the movie on a black
		// background.  this solves the problem of palette shifts on examine
		// movies.
		case 'O': {
			getAlphaNumFromStream(istr, sStr);

			if (!sStr.find("ONBLACK")) {
				setPlayImmediate();
				nObjectUpdated = true;
			} else {
				putbackStringOnStream(istr, sStr);
			}
		}
		break;

		// Associate a sound file with this movie (primarily for examine
		// movies).
		case 'S': {
			getAlphaNumFromStream(istr, sStr);
			if (!sStr.find("SND")) {
				nObjectUpdated = true;

				_pSndObj = new CBagSoundObject();
				if (_pSndObj->setInfo(istr) == PARSING_DONE) {
					return PARSING_DONE;
				}
			} else {
				putbackStringOnStream(istr, sStr);
			}
		}
		break;

		//
		//  No match return from function
		//
		default: {
			const ParseCodes parseCode = CBagObject::setInfo(istr);
			if (parseCode == PARSING_DONE) {
				return PARSING_DONE;
			}

			if (parseCode == UPDATED_OBJECT) {
				nObjectUpdated = true;
			} else { // rc==UNKNOWN_TOKEN
				if (nObjectUpdated)
					return UPDATED_OBJECT;

				return UNKNOWN_TOKEN;
			}
		}
		break;
		}
	}

	return PARSING_DONE;
}

bool CBagMovieObject::asynchPDAMovieCanPlay() {
	// Don't play movie if we're zoomed or if we're in a CIC
	// or a sound is playing or another movie is playing...
	bool bCanPlay = true;

	// Obvious case, if it is set for immediate, return true.
	if (isPlayImmediate()) {
		return bCanPlay;
	}

	char szLocalBuff[256];
	CBofString sStr(szLocalBuff, 256);

	sStr = "BPDAZ_WLD";
	SBZoomPda *pPDAz = (SBZoomPda *)g_SDevManager->getStorageDevice(sStr);

	sStr = "BPDA_WLD";
	CBagPDA *pPDA = (CBagPDA *)g_SDevManager->getStorageDevice(sStr);

	CBagPanWindow *pMainWin = (CBagPanWindow *)(CBagel::getBagApp()->getMasterWnd()->getCurrentGameWindow());

	// Queue this message if any one of a variety of things is happening.
	assert(pPDA != nullptr);
	assert(pPDAz != nullptr);

	if (pPDA && pPDAz) {
		if (pPDAz->getZoomed() ||              // We're zoomed
		        (pMainWin->isCIC() && !isDontOverride()) || // We're in a character closeup
		        CBofSound::soundsPlayingNotOver() ||        // A sound is playing
		        pPDA->getPdaMode() == PDA_MOO_MODE) {            // An asynch movie is already playing
			bCanPlay = false;
		}
	}

	return bCanPlay;
}

} // namespace SpaceBar
} // namespace Bagel
