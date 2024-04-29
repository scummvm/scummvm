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
#include "bagel/baglib/movie_object.h"
#include "bagel/baglib/exam.h"
#include "bagel/baglib/master_win.h"
#include "bagel/baglib/pan_window.h"
#include "bagel/boflib/sound.h"
#include "bagel/baglib/fmovie.h"
#include "bagel/baglib/zoom_pda.h"
#include "bagel/boflib/file_functions.h"
#include "bagel/boflib/log.h"

namespace Bagel {

#define SOUNDFILEEXTLOWER ".wav"
#define SOUNDFILEEXTUPPER ".WAV"
#define OVERRIDESMK "$SBARDIR\\BAR\\LOG\\OVERRIDE.SMK"

CBagMovieObject::CBagMovieObject() {
	m_xObjType = MOVIEOBJ;
	SetVisible(true);
	m_xDisplayType = DISP_TYPE::MOVIE;
	m_bFlyThru = false;

	// Initialize asynch flags to be off by default.
	m_nAsynchFlags = 0;

	// Allow user to force movie not to increment (default = increment = true)
	SetIncrement();

	// Allow movie to play on a black background (default, nada).
	SetOnBlack(false);

	// Default is no associated sound.
	SetAssociatedSound(nullptr);
}

CBagMovieObject::~CBagMovieObject() {
	CBagObject::detach();

	// Could still by lying around in the pda movie queue,
	// Make sure it has been removed.
	if (m_xDisplayType == DISP_TYPE::ASYNCH_PDAMSG) {
		CBagPDA::RemoveFromMovieQueue(this);
	}

	// If there's a sound with this guy, then trash it here.
	if (GetAssociatedSound()) {
		delete GetAssociatedSound();
		SetAssociatedSound(nullptr);
	}
}

bool CBagMovieObject::runObject() {
	CBagPDA *pPDA = nullptr;
	CBofWindow *pNewWin = nullptr;
	SBZoomPda *pPDAz = (SBZoomPda *)SDEVMNGR->GetStorageDevice("BPDAZ_WLD");
	bool bZoomed = (pPDAz == nullptr ? false : pPDAz->getZoomed());

	// Get a pointer to the current game window
	CBagStorageDevWnd *pMainWin = (CBagel::getBagApp()->getMasterWnd()->GetCurrentStorageDev());

	bool rc = true;
	if (!m_bFlyThru || CBagMasterWin::GetFlyThru()) {
		rc = false;

		CBofString sFileName = GetFileName();
		int nExt = sFileName.GetLength() - 4; // ".EXT"

		if (nExt <= 0) {
			LogError("Movie does not have a file name or proper extension.  Please write better scripts.");
			return rc;
		}

#ifndef BOF_FINAL_RELEASE
		//
		// This would be much cooler if it were a cast to another object type and
		// then a run.  But this is a quicker fix.
		//
		enum class MOVFILETYPE { NONE = 0,
		                   TEXT = 1,
		                   SOUND = 2,
		                   MOVIE = 3
		                 } nMovFileType;
		CBofString sBaseStr = sFileName.Left(nExt);

		if (sFileName.Find(".smk") > 0 || sFileName.Find(".SMK") > 0) {
			nMovFileType = MOVFILETYPE::MOVIE;
		} else if (sFileName.Find(SOUNDFILEEXTLOWER) > 0 || sFileName.Find(SOUNDFILEEXTUPPER) > 0) {
			nMovFileType = MOVFILETYPE::SOUND;
		} else if (sFileName.Find(".txt") > 0 || sFileName.Find(".TXT") > 0) {
			nMovFileType = MOVFILETYPE::TEXT;
		} else {
			nMovFileType = MOVFILETYPE::NONE;
		}

		// Look for .SMK then .WAV, then .TXT
		while (!FileExists(sFileName.GetBuffer())) {
			switch (nMovFileType) {

			case MOVFILETYPE::MOVIE:
				sFileName = sBaseStr + SOUNDFILEEXTLOWER;
				nMovFileType = MOVFILETYPE::SOUND;
				break;

			case MOVFILETYPE::SOUND:
				sFileName = sBaseStr + ".txt";
				nMovFileType = MOVFILETYPE::TEXT;
				break;

			case MOVFILETYPE::TEXT:
				BofMessageBox(sFileName.GetBuffer(), "Could not find asset");
				nMovFileType = MOVFILETYPE::NONE;
				break;

			// We should never get here
			case MOVFILETYPE::NONE:
			default:
				LogError(BuildString("Movie does not have a correct file name: %s.", sFileName.GetBuffer()));
				return rc;
			}
		}

		if (nMovFileType == MOVFILETYPE::MOVIE) {
#endif

			bool isFiltered = false;
			CBagMasterWin *pWnd = CBagel::getBagApp()->getMasterWnd();
			CBagStorageDevWnd *pSDevWnd = (pWnd ? pWnd->GetCurrentStorageDev() : nullptr);

			// Get the pda here, we need it so much anyway.
			if (pPDA == nullptr) {
				pPDA = (CBagPDA *)SDEVMNGR->GetStorageDevice("BPDA_WLD");
			}
			Assert(pPDA != nullptr);

			// If we have an asnych movie to play, make sure it is a good
			// time to play it, if not, then queue it up so it can play at a much better time.
			if (m_xDisplayType == DISP_TYPE::ASYNCH_PDAMSG) {
				if (AsynchPDAMovieCanPlay() == false) {
					pPDA->AddToMovieQueue(this);
					return rc;
				}
			} else {
				// Don't need to redraw for asynch pda messages, this just
				// messes things up in the PDA redraw code
				if (pWnd != nullptr) {
					if (pSDevWnd != nullptr) {
						pSDevWnd->PaintScreen(nullptr);
						isFiltered = pSDevWnd->IsFiltered();
					}
				}
			}

			if (m_xDisplayType == DISP_TYPE::EXAMINE) {
				CBofRect r(160, 60, 480, 300);

				// Offset the rect for the movies to compensate for all screen sizes
				r.OffsetRect(((CBofWindow *)pMainWin)->GetWindowRect().TopLeft());

				// If we have a movie playing in the zoom pda, then black out
				// the background.  Examine movies will always play with a black background
				// on the mac (prevents a palette shift).
				CBagExam *pMovie = new CBagExam(CBagel::getBagApp()->getMasterWnd()->GetCurrentGameWindow(), sFileName, &r);

				if (pMovie) {
					// If there is an associated sound file, then start it up here.
					CBagSoundObject *pSObj = GetAssociatedSound();
					if (pSObj) {
						if (pSObj->isAttached() == false) {
							pSObj->attach();
						}
						pSObj->runObject();
					}

					CBofWindow *wnd = CBagel::getBagApp()->getMasterWnd();
					pMovie->Show();
					CBofApp::GetApp()->GetMainWindow()->FlushAllMessages();
					wnd->FlushAllMessages();
					pMovie->initExam();
					delete pMovie;
					rc = true;

					// As soon as we're done, detach (this will also stop the sound).
					if (pSObj) {
						pSObj->detach();
					}
				}
			} else {
				bool bActivated = false;
				CBofRect r(80, 10, 80 + 480 - 1, 10 + 360 - 1);

				// Offset the rect for the movies to compensate for all screen sizes
				r.OffsetRect(((CBofWindow *)pMainWin)->GetWindowRect().TopLeft());

				if (m_xDisplayType == DISP_TYPE::PDAMSG || m_xDisplayType == DISP_TYPE::ASYNCH_PDAMSG) {
					// Pull up the PDA (if it exists)
					// Only pull up the PDA if we're not playing an asynch movie
					if (m_xDisplayType == DISP_TYPE::PDAMSG) {
						// Increment timer one, pda message counts as one turn
						// Allow scripter to override timer increment
						if (IsIncrement()) {
							VARMNGR->IncrementTimers();
						}
					}

					if (pMainWin->GetDeviceType() == SDEV_GAMEWIN) {
						// If the pda is going up or down, then wait for it
						// To do its thing before attempting to activate it.
						if (pPDA->isActivating()) {
							((CBagPanWindow *)pMainWin)->WaitForPDA();
						}

						if (pPDA->isActivated() == false) {
							bActivated = ((CBagPanWindow *)pMainWin)->ActivatePDA();
							((CBagPanWindow *)pMainWin)->WaitForPDA();
						}
					}
				} else {

					// Pull down the PDA (if it exists)
					if (pMainWin->GetDeviceType() == SDEV_GAMEWIN) {
						((CBagPanWindow *)pMainWin)->DeactivatePDA();
						((CBagPanWindow *)pMainWin)->WaitForPDA();
					}
				}

				// Use the position scripted for the movie if it exists
				// The bottom, right is unimportant because movie won't resize anyway
				CBofPoint p = CBagObject::GetPosition();

				if (p.x != -1 && p.y != -1)
					r.OffsetRect(p);

				if (isFiltered) {
					if (bZoomed) {
						pNewWin = new CBofWindow();
						if (pNewWin) {
							pNewWin->create("BLACK", 0, 0, 640, 480, CBofApp::GetApp()->GetMainWindow(), 0);
							pNewWin->FillWindow(COLOR_BLACK);
						}
					}

					CBagFMovie *pMovie = new CBagFMovie(CBofApp::GetApp()->GetMainWindow(), sFileName, &r);

					if (pMovie != nullptr && pMovie->ErrorOccurred() == false) {
						pMovie->Show();
						CBofApp::GetApp()->GetMainWindow()->FlushAllMessages();
						pWnd->FlushAllMessages();
						pMovie->Play(false);
						delete pMovie;
						rc = true;
					} else {
						LogError(BuildString("Movie file could not be read: %s.  How? You removed that CD again didn't you", sFileName.GetBuffer()));
					}

					if (pNewWin) {
						delete pNewWin;
						pNewWin = nullptr;
					}
				} else {
					// Hack.. allow script to override some other movies.
					if ((m_xDisplayType == DISP_TYPE::PDAMSG) && pMainWin->IsCIC() && IsDontOverride() == false) {

						char szLocalBuff[256];
						CBofString cStr(szLocalBuff, 256);

						// Play the override message.
						cStr = OVERRIDESMK;
						MACROREPLACE(cStr);

						sFileName = cStr;
					}

					if (m_xDisplayType == DISP_TYPE::ASYNCH_PDAMSG) {
						// Tell our PDA to switch gears to do asynch movie time.
						if (pPDA) {
							if (pPDA->showMovie()) {       // Returns false if another movie playing
								pPDA->setMovie(sFileName); // Set the movie to play
							}
						} else {
							LogError(BuildString("Movie file could not be read: %s.  How? You removed that CD again didn't you", sFileName.GetBuffer()));
						}
					} else {
						CBofMovie *pMovie;

						if (bZoomed && m_xDisplayType != DISP_TYPE::ASYNCH_PDAMSG && m_xDisplayType != DISP_TYPE::PDAMSG) {
							pNewWin = new CBofWindow();
							if (pNewWin) {
								pNewWin->create("BLACK", 0, 0, 640, 480, CBofApp::GetApp()->GetMainWindow(), 0);
								pNewWin->Show();
								pNewWin->FillWindow(COLOR_BLACK);
							}
						}

						// If playing a PDA message while the PDA is zoomed
						if (m_xDisplayType == DISP_TYPE::PDAMSG && bZoomed) {
							// Then stretch it to fit into the PDA's viewscreen
							r.SetRect(24, 47, 28 + 600 - 1, 47 + 302 - 1);
							pMovie = new CBofMovie(CBofApp::GetApp()->GetMainWindow(), sFileName, &r, true);

						} else {
							// Otherwise, just play the movie normally
							pMovie = new CBofMovie(CBofApp::GetApp()->GetMainWindow(), sFileName, &r);
						}

						if (pMovie && pMovie->ErrorOccurred() == false) {
							// Stop any asnych movies already playing
							pPDA->stopMovie(true);
							pMovie->Show();
							CBofApp::GetApp()->GetMainWindow()->FlushAllMessages();
							pWnd->FlushAllMessages();
							pMovie->Play(false);
							delete pMovie;
							rc = true;
						} else {
							LogError(BuildString("Movie file could not be read: %s.  How? You removed that CD again didn't you", sFileName.GetBuffer()));
						}

						// If we put a black window up, then
						if (pNewWin) {
							delete pNewWin;
							pNewWin = nullptr;
						}
					}
				}

				// Put the pda down if we brought it up. (8638)
				if (m_xDisplayType != DISP_TYPE::ASYNCH_PDAMSG && bActivated) {
					((CBagPanWindow *)pMainWin)->DeactivatePDA();
					((CBagPanWindow *)pMainWin)->WaitForPDA();
				}

				// If we're asynch, then let it know to deactivate when done playing.
				if (m_xDisplayType == DISP_TYPE::ASYNCH_PDAMSG) {
					pPDA->setDeactivate(bActivated);
				}
			}

			// Movies usually mark the transition from one view to another
			// but not necessarily a change of sdev's, so make sure we repaint the
			// backdrop
			if (pMainWin) {
				pMainWin->SetPreFilterPan(true);
			}

		} else if (nMovFileType == MOVFILETYPE::SOUND) {
			CBofSound *pSound = new CBofSound(CBofApp::GetApp()->GetMainWindow(), sFileName, SOUND_WAVE);
			if (pSound) {
				pSound->Play();
				delete pSound;
				rc = true;
			} else {
				LogError(BuildString("Movie SOUND file could not be read: %s.  Where? Not in Kansas ...", sFileName.GetBuffer()));
			}
		} else if (nMovFileType == MOVFILETYPE::TEXT) {
			Common::File f;
			if (f.open(sFileName.GetBuffer())) {
				Common::String line = f.readLine();

				BofMessageBox(line.c_str(), "Incoming Message...");
				f.close();
				rc = true;
			} else {
				LogError(BuildString("Movie TEXT file could not be read: %s.  Why? because we like you ...", sFileName.GetBuffer()));
			}
		}

		rc = CBagObject::runObject();
	}

	return rc;
}

PARSE_CODES CBagMovieObject::setInfo(bof_ifstream &istr) {
	bool nObjectUpdated = false;
	char szLocalStr[256];
	CBofString sStr(szLocalStr, 256);

	while (!istr.eof()) {
		istr.EatWhite(); // Eat any white space between script elements

		char ch = (char)istr.peek();
		switch (ch) {
		//
		//  AS  - n number of slides in sprite
		//
		case 'A': {
			GetAlphaNumFromStream(istr, sStr);

			if (!sStr.Find("AS")) {
				istr.EatWhite();
				GetAlphaNumFromStream(istr, sStr);
				if (!sStr.Find("EXAMINE")) {
					m_xDisplayType = DISP_TYPE::EXAMINE;
				} else if (!sStr.Find("MOVIE")) {
					m_xDisplayType = DISP_TYPE::MOVIE;

				} else if (!sStr.Find("FLYTHRU")) {
					m_xDisplayType = DISP_TYPE::MOVIE;
					m_bFlyThru = true;
				} else if (!sStr.Find("PDAMSG")) {
					m_xDisplayType = DISP_TYPE::PDAMSG;
				} else if (!sStr.Find("ASYNCH_PDAMSG")) {
					m_xDisplayType = DISP_TYPE::ASYNCH_PDAMSG;

					// see if this improves performance any...
					SetPreload(true);
				}
			}

			nObjectUpdated = true;
		}
		break;

		// Don't queue attribute, when set, the asynch movie either plays
		// immediately or not at all.
		case 'D': {
			GetAlphaNumFromStream(istr, sStr);

			if (!sStr.Find("DONTQUEUE")) {
				SetDontQueue();
				nObjectUpdated = true;
			} else if (!sStr.Find("DONTOVERRIDE")) {
				SetDontOverride();
				nObjectUpdated = true;
			} else if (!sStr.Find("DONTINCREMENT")) {
				// Don't increment the timer when playing this movie					
				SetIncrement(false);
				nObjectUpdated = true;
			} else {
				PutbackStringOnStream(istr, sStr);
			}
		}
		break;

		// Don't queue attribute, when set, the asynch movie either plays
		// immediately or not at all.
		case 'P': {
			GetAlphaNumFromStream(istr, sStr);

			if (!sStr.Find("PLAYIMMEDIATE")) {
				SetPlayImmediate();
				nObjectUpdated = true;
			} else {
				PutbackStringOnStream(istr, sStr);
			}
		}
		break;

		// This is probably going to be much more important to the
		// mac version... give the option of playing the movie on a black
		// background.  this solves the problem of palette shifts on examine
		// movies.
		case 'O': {
			GetAlphaNumFromStream(istr, sStr);

			if (!sStr.Find("ONBLACK")) {
				SetPlayImmediate();
				nObjectUpdated = true;
			} else {
				PutbackStringOnStream(istr, sStr);
			}
		}
		break;

		// Associate a sound file with this movie (primarily for examine
		// movies).
		case 'S': {
			GetAlphaNumFromStream(istr, sStr);
			if (!sStr.Find("SND")) {
				nObjectUpdated = true;

				m_pSndObj = new CBagSoundObject();
				if (m_pSndObj && m_pSndObj->setInfo(istr) == PARSING_DONE) {
					return PARSING_DONE;
				}
			} else {
				PutbackStringOnStream(istr, sStr);
			}
		}
		break;

		//
		//  No match return from funtion
		//
		default: {
			PARSE_CODES rc = CBagObject::setInfo(istr);
			if (rc == PARSING_DONE) {
				return PARSING_DONE;
			}

			if (rc == UPDATED_OBJECT) {
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

bool CBagMovieObject::AsynchPDAMovieCanPlay() {
	// Don't play movie if we're zoomed or if we're in a CIC
	// or a sound is playing or another movie is playing...
	bool bCanPlay = true;

	// Obvious case, if it is set for immediate, return true.
	if (IsPlayImmediate()) {
		return bCanPlay;
	}

	char szLocalBuff[256];
	CBofString sStr(szLocalBuff, 256);

	sStr = "BPDAZ_WLD";
	SBZoomPda *pPDAz = (SBZoomPda *)SDEVMNGR->GetStorageDevice(sStr);

	sStr = "BPDA_WLD";
	CBagPDA *pPDA = (CBagPDA *)SDEVMNGR->GetStorageDevice(sStr);

	CBagPanWindow *pMainWin = (CBagPanWindow *)(CBagel::getBagApp()->getMasterWnd()->GetCurrentGameWindow());

	// Queue this message if any one of a variety of things is happening.
	Assert(pPDA != nullptr);
	Assert(pPDAz != nullptr);

	if (pPDA && pPDAz) {
		if (pPDAz->getZoomed() ||              // We're zoomed
		        (pMainWin->IsCIC() && !IsDontOverride()) || // We're in a character closeup
		        CBofSound::SoundsPlayingNotOver() ||        // A sound is playing
		        pPDA->getPdaMode() == MOOMODE) {            // An asynch movie is already playing
			bCanPlay = false;
		}
	}

	return bCanPlay;
}

} // namespace Bagel
