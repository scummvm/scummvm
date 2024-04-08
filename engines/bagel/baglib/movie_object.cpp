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

namespace Bagel {

#define SOUNDFILEEXTLOWER ".wav"
#define SOUNDFILEEXTUPPER ".WAV"
#define OVERRIDESMK "$SBARDIR\\BAR\\LOG\\OVERRIDE.SMK"

//
// CBagMovieObject -
//  CBagMovieObject is an object that can be place within the slide window.
//
CBagMovieObject::CBagMovieObject() {
	m_xObjType = MOVIEOBJ;
	SetVisible(true);
	m_xDisplayType = DISP_TYPE::MOVIE;
	m_bFlyThru = false;

	// Initialize asynch flags to be off by default.
	m_nAsynchFlags = 0;

	// allow user to force movie not to increment (default = increment = true)
	SetIncrement();

	// allow movie to play on a black background (default, nada).
	SetOnBlack(false);

	// default is no associated sound.
	SetAssociatedSound(nullptr);
}

CBagMovieObject::~CBagMovieObject() {
	Detach();

	// could still by lying around in the pda movie queue,
	// make sure it has been removed.
	if (m_xDisplayType == DISP_TYPE::ASYNCH_PDAMSG) {
		CBagPDA::RemoveFromMovieQueue(this);
	}

	// if there's a sound with this guy, then trash it here.
	if (GetAssociatedSound()) {
		delete GetAssociatedSound();
		SetAssociatedSound(nullptr);
	}
}

/*ERROR_CODE CBagMovieObject::Attach()
{
    return CBagObject::Attach();
}


ERROR_CODE CBagMovieObject::Detach()
{
    return CBagObject::Detach();
}


CBofRect CBagMovieObject::GetRect()
{
    CBofPoint p = GetPosition();
    CBofSize  s = GetSize();
    CBofRect r = CBofRect(p,s);
    return(r);
}*/

#if BOF_MAC
#pragma profile off // movies are skewing our profiling
// data, since we don't have control over it, don't
// include it.
#endif
bool CBagMovieObject::RunObject() {
	bool rc;
	CBagPDA *pPDA = nullptr;
	CBofWindow *pNewWin = nullptr;
	SBZoomPda *pPDAz = (SBZoomPda *)SDEVMNGR->GetStorageDevice("BPDAZ_WLD");
	bool bZoomed = (pPDAz == nullptr ? false : pPDAz->GetZoomed());

	// Get a pointer to the current game window
	CBagStorageDevWnd *pMainWin = (CBagel::GetBagApp()->GetMasterWnd()->GetCurrentStorageDev());

	// Get a pointer to the current game window
	// CBagPanWindow* pMainWin=(CBagPanWindow*)(CBagel::GetBagApp()->GetMasterWnd()->GetCurrentGameWindow());

	rc = true;
	if (!m_bFlyThru || CBagMasterWin::GetFlyThru()) {

		rc = false;

		CBofString sFileName = GetFileName();
		int nExt = sFileName.GetLength() - 4; // ".EXT"

		if (nExt <= 0) {
			LogError("Movie does not have a file name or proper extention.  Please write better scripts.");
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

#if BOF_MAC
		// just temporary, trying to get the diskid right for multiple
		// game disks
		int nDiskID = CBagel::GetBagApp()->GetMasterWnd()->GetDiskID();
		if (nDiskID > 0 && nDiskID < 4) {
			char szFirst6[7];

			BofMemCopy(szFirst6, sBaseStr.GetBuffer(), 6);
			szFirst6[6] = 0;
			::UpperText(szFirst6, 6);
			if (memcmp(szFirst6, "SBDISK", 6) == 0) {
				sBaseStr.ReplaceCharAt(6, nDiskID + '0');
			}
		}
#endif

		// We shouldn't need this, but you never know what a QA person is going to type
		//
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
		//
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
				// sFileName = sBaseStr + ".ARF"
				BofMessageBox(sFileName.GetBuffer(), "Could not find asset");
				nMovFileType = MOVFILETYPE::NONE;
				break;

			// We should never get here
			//
			case MOVFILETYPE::NONE:
			default:
				LogError(BuildString("Movie does not have a correct file name: %s.", sFileName.GetBuffer()));
				return rc;
			}
		}

		if (nMovFileType == MOVFILETYPE::MOVIE) {
#endif

			bool isFiltered = false;
			CBagMasterWin *pWnd = CBagel::GetBagApp()->GetMasterWnd();
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
				// don't need to redraw for asynch pda messages, this just
				// messes things up in the PDA redraw code (trust me)
				//
				// Hide the cursor by causing 1 more paint (and specifying no cursor)
				//
				if (pWnd != nullptr) {
					if (pSDevWnd != nullptr) {
						pSDevWnd->PaintScreen(nullptr, false);
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

#if BOF_MAC
				pNewWin = new CBofWindow();
				if (pNewWin) {
					pNewWin->Create("BLACK", 0, 0, 640, 480, CBofApp::GetApp()->GetMainWindow(), 0);
					pNewWin->FillWindow(COLOR_BLACK);
				}

				// set the first frame to be black so that we don't get a palette shift.
				CBagExam *pMovie = new CBagExam(CBagel::GetBagApp()->GetMasterWnd()->GetCurrentGameWindow(), sFileName, &r, false, true, m_xDisplayType == EXAMINE);
#else
				CBagExam *pMovie = new CBagExam(CBagel::GetBagApp()->GetMasterWnd()->GetCurrentGameWindow(), sFileName, &r);
#endif
				if (pMovie) {
					// if there is an associated sound file, then start it up here.
					CBagSoundObject *pSObj = GetAssociatedSound();
					if (pSObj) {
						if (pSObj->IsAttached() == false) {
							pSObj->Attach();
						}
						pSObj->RunObject();
					}

					CBofWindow *wnd = CBagel::GetBagApp()->GetMasterWnd();
					pMovie->Show();
					CBofApp::GetApp()->GetMainWindow()->FlushAllMessages();
					wnd->FlushAllMessages();
					pMovie->Exam();
					delete pMovie;
					rc = true;

					// As soon as we're done, detach (this will also stop the sound).
					if (pSObj) {
						pSObj->Detach();
					}
				}

#if BOF_MAC
				if (pNewWin) {
					delete pNewWin;
					pNewWin = nullptr;
				}
#endif
			} else {
				bool bActivated = false;
				CBofRect r(80, 10, 80 + 480 - 1, 10 + 360 - 1);

				// Offset the rect for the movies to compensate for all screen sizes
				r.OffsetRect(((CBofWindow *)pMainWin)->GetWindowRect().TopLeft());

				if (m_xDisplayType == DISP_TYPE::PDAMSG || m_xDisplayType == DISP_TYPE::ASYNCH_PDAMSG) {

					// Pull up the PDA (if it exists)
					//
					// only pull up the PDA if we're not playing an asynch movie
					if (m_xDisplayType == DISP_TYPE::PDAMSG) {

						// increment timer one, pda message counts as one turn
						// allow scripter to override timer increment
						if (IsIncrement()) {
							VARMNGR->IncrementTimers();
						}
					}

					if (pMainWin->GetDeviceType() == SDEV_GAMEWIN) {
						// if the pda is going up or down, then wait for it
						// to do its thing before attempting to activate it.
						if (pPDA->IsActivating()) {
							((CBagPanWindow *)pMainWin)->WaitForPDA();
						}

						if (pPDA->IsActivated() == false) {
							bActivated = ((CBagPanWindow *)pMainWin)->ActivatePDA();
							((CBagPanWindow *)pMainWin)->WaitForPDA();
						}
					}
				} else {

					// Pull down the PDA (if it exists)
					//
					if (pMainWin->GetDeviceType() == SDEV_GAMEWIN) {
						((CBagPanWindow *)pMainWin)->DeactivatePDA();
						((CBagPanWindow *)pMainWin)->WaitForPDA();
					}
				}

				// Use the position scripted for the movie if it exists
				// the bottom, right is unimportant because movie won't resize anyway
				CBofPoint p = CBagObject::GetPosition();

				if (p.x != -1 && p.y != -1)
					r.OffsetRect(p);

				if (isFiltered) {
					if (bZoomed) {
						pNewWin = new CBofWindow();
						if (pNewWin) {
							pNewWin->Create("BLACK", 0, 0, 640, 480, CBofApp::GetApp()->GetMainWindow(), 0);
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

					// temp fix... maybe, allow script to override some
					// other movies.
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
							if (pPDA->ShowMovie()) {       // returns false if another movie playing
								pPDA->SetMovie(sFileName); // set the movie to play
							}
						} else {
							LogError(BuildString("Movie file could not be read: %s.  How? You removed that CD again didn't you", sFileName.GetBuffer()));
						}
					} else {
						CBofMovie *pMovie;

						if (bZoomed && m_xDisplayType != DISP_TYPE::ASYNCH_PDAMSG && m_xDisplayType != DISP_TYPE::PDAMSG) {
							pNewWin = new CBofWindow();
							if (pNewWin) {
								pNewWin->Create("BLACK", 0, 0, 640, 480, CBofApp::GetApp()->GetMainWindow(), 0);
								pNewWin->Show();
								pNewWin->FillWindow(COLOR_BLACK);
							}
						}

						// If playing a PDA message while the PDA is zoomed
						//
						if (m_xDisplayType == DISP_TYPE::PDAMSG && bZoomed) {

							// Then stretch it to fit into the PDA's viewscreen
							// r.SetRect(24, 47, 24 + 600 - 1, 47 + 302 - 1);

							r.SetRect(24, 47, 28 + 600 - 1, 47 + 302 - 1);
							pMovie = new CBofMovie(CBofApp::GetApp()->GetMainWindow(), sFileName, &r, true);

							// Otherwise, just play the movie normally
							//
						} else {
#if BOF_MAC
							pMovie = new CBofMovie(CBofApp::GetApp()->GetMainWindow(), sFileName, &r, false, (m_xDisplayType != PDAMSG), (m_xDisplayType == EXAMINE || m_bFlyThru == true));
#else
							pMovie = new CBofMovie(CBofApp::GetApp()->GetMainWindow(), sFileName, &r);
#endif
						}

						if (pMovie && pMovie->ErrorOccurred() == false) {
							// stop any asnych movies already playing
							pPDA->StopMovie(true);
							pMovie->Show();
							CBofApp::GetApp()->GetMainWindow()->FlushAllMessages();
							pWnd->FlushAllMessages();
							pMovie->Play(false);
							delete pMovie;
							rc = true;
						} else {
							LogError(BuildString("Movie file could not be read: %s.  How? You removed that CD again didn't you", sFileName.GetBuffer()));
						}

						// if we put a black window up, then
						if (pNewWin) {
							delete pNewWin;
							pNewWin = nullptr;
						}
					}
				}

				// put the pda down if we brought it up. (8638)
				if (m_xDisplayType != DISP_TYPE::ASYNCH_PDAMSG && bActivated) {
#if BOF_MAC
					// make sure the window is up before we drop the pda
					((CBagPanWindow *)pMainWin)->Show();
#endif
					((CBagPanWindow *)pMainWin)->DeactivatePDA();
					((CBagPanWindow *)pMainWin)->WaitForPDA();
				}

				// if we're asynch, then let it know to deactivate when
				// done playing.
				if (m_xDisplayType == DISP_TYPE::ASYNCH_PDAMSG) {
					pPDA->SetDeactivate(bActivated);
				}
			}

			// movies usually mark the transition from one view to another
			// but not necessarily a change of sdev's, so make sure we repaint the
			// backdrop
			if (pMainWin) {
				pMainWin->SetPreFilterPan(true);
			}
#ifndef BOF_FINAL_RELEASE
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
#if BOF_MAC
			bool isOpen = false;
			if (FileExists(sFileName)) {
				ifstream fpTest(sFileName, 0);
				if (fpTest.is_open()) {
					isOpen = true;
					char str[255];
					fpTest.getline(str, 255);

					BofMessageBox(str, "Incoming Message...");
					fpTest.close();
					rc = true;
				}
			}

			if (isOpen == false) {
				LogError(BuildString("Movie TEXT file could not be read: %s.  Why? because we like you ...", sFileName.GetBuffer()));
			}
#else
			Common::File f;
			if (f.open(sFileName.GetBuffer())) {
				Common::String line = f.readLine();

				BofMessageBox(line.c_str(), "Incoming Message...");
				f.close();
				rc = true;
			} else {
				LogError(BuildString("Movie TEXT file could not be read: %s.  Why? because we like you ...", sFileName.GetBuffer()));
			}
#endif
		}

#endif
		rc = CBagObject::RunObject();
	}

	return rc;
}

#if BOF_MAC
#pragma profile reset
#endif

//
// SetInfo(bof_ifstream& istr)
//   Takes in info and then removes the relative information and returns the info
//   without the relevant info.
PARSE_CODES CBagMovieObject::SetInfo(bof_ifstream &istr) {
	int nChanged;
	bool nObjectUpdated = false;
	char ch;
	char szLocalStr[256];
	CBofString sStr(szLocalStr, 256); // performance improvement

	while (!istr.eof()) {
		nChanged = 0;
		istr.EatWhite(); // Eat any white space between script elements

		switch (ch = (char)istr.peek()) {
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
			nChanged++;
		}
		break;
		// dont queue attribute, when set, the asynch movie either plays
		// immediately or not at all.
		case 'D': {
			GetAlphaNumFromStream(istr, sStr);

			if (!sStr.Find("DONTQUEUE")) {
				SetDontQueue();
				nObjectUpdated = true;
				nChanged++;
			} else {
				if (!sStr.Find("DONTOVERRIDE")) {
					SetDontOverride();
					nObjectUpdated = true;
					nChanged++;
				} else {
					// Don't increment the timer when playing this movie
					if (!sStr.Find("DONTINCREMENT")) {
						SetIncrement(false);
						nObjectUpdated = true;
						nChanged++;
					} else {
						PutbackStringOnStream(istr, sStr);
					}
				}
			}
		}
		break;
		// dont queue attribute, when set, the asynch movie either plays
		// immediately or not at all.
		case 'P': {
			GetAlphaNumFromStream(istr, sStr);

			if (!sStr.Find("PLAYIMMEDIATE")) {
				SetPlayImmediate();
				nObjectUpdated = true;
				nChanged++;
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
				nChanged++;
			} else {
				PutbackStringOnStream(istr, sStr);
			}
		}
		break;

		// associate a sound file with this movie (primarily for examine
		// movies).
		case 'S': {
			GetAlphaNumFromStream(istr, sStr);
			if (!sStr.Find("SND")) {
				nObjectUpdated = true;
				nChanged++;

				m_pSndObj = new CBagSoundObject();
				if (m_pSndObj && m_pSndObj->SetInfo(istr) == PARSING_DONE) {
					return PARSING_DONE;
				}
			} else {
				PutbackStringOnStream(istr, sStr);
			}
		}
		break;

		//
		//  no match return from funtion
		//
		default: {
			PARSE_CODES rc;
			if ((rc = CBagObject::SetInfo(istr)) == PARSING_DONE) {
				return PARSING_DONE;
			} else if (rc == UPDATED_OBJECT) {
				nObjectUpdated = true;
			} else if (!nChanged) { // rc==UNKNOWN_TOKEN
				if (nObjectUpdated)
					return UPDATED_OBJECT;
				else
					return UNKNOWN_TOKEN;
			}
		}
		break;
		}
	}

	return PARSING_DONE;
}

// Don't play movie if we're zoomed or if we're in a CIC
// or a sound is playing or another movie is playing...
bool CBagMovieObject::AsynchPDAMovieCanPlay() {
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

	CBagPanWindow *pMainWin = (CBagPanWindow *)(CBagel::GetBagApp()->GetMasterWnd()->GetCurrentGameWindow());

	// queue this message if any one of a variety of things is happening.
	Assert(pPDA != nullptr);
	Assert(pPDAz != nullptr);

	if (pPDA && pPDAz) {
		if ((pPDAz && pPDAz->GetZoomed()) ||              // we're zoomed
		        (pMainWin->IsCIC() && !IsDontOverride()) || // we're in a character closeup
		        CBofSound::SoundsPlayingNotOver() ||        // a sound is playing
		        pPDA->GetPDAMode() == MOOMODE) {            // an asynch movie is already playing
			bCanPlay = false;
		}
	}

	return bCanPlay;
}

} // namespace Bagel
