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

// Don't play sound files on mac if we're using wave conversion
#if (BOF_MAC || BOF_WINMAC) && !PLAYWAVONMAC
#define SOUNDFILEEXTLOWER ".snd"
#define SOUNDFILEEXTUPPER ".SND"
#define OVERRIDESMK "$SBARDIR:BAR:LOG:OVERRIDE.SMK"
#else
#define SOUNDFILEEXTLOWER ".wav"
#define SOUNDFILEEXTUPPER ".WAV"
#define OVERRIDESMK "$SBARDIR\\BAR\\LOG\\OVERRIDE.SMK"
#endif

//
// CBagMovieObject -
//  CBagMovieObject is an object that can be place within the slide window.
//
CBagMovieObject::CBagMovieObject() {
	m_xObjType = MOVIEOBJ;
	SetVisible(TRUE);
	m_xDisplayType = MOVIE;
	m_bFlyThru = FALSE;

	// jwl 12.02.96 initialize asynch flags to be off by default.
	m_nAsynchFlags = 0;

	// jwl 12.05.96 allow user to force movie not to increment (default = increment = true)
	SetIncrement();

	// jwl 12.17.96 allow movie to play on a black background (default, nada).
	SetOnBlack(FALSE);

	// jwl 1.2.96 default is no associated sound.
	SetAssociatedSound(NULL);
}

CBagMovieObject::~CBagMovieObject() {
	Detach();

	// jwl 11.30.96 could still by lying around in the pda movie queue,
	// make sure it has been removed.
	if (m_xDisplayType == ASYNCH_PDAMSG) {
		CBagPDA::RemoveFromMovieQueue(this);
	}

	// jwl 1.2.96 if there's a sound with this guy, then trash it here.
	if (GetAssociatedSound()) {
		delete GetAssociatedSound();
		SetAssociatedSound(NULL);
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


CBofRect CBagMovieObject::GetRect(VOID)
{
    CBofPoint p = GetPosition();
    CBofSize  s = GetSize();
    CBofRect r = CBofRect(p,s);
    return(r);
}*/

#if BOF_MAC
#pragma profile off // jwl 08.13.96 movies are skewing our profiling
// data, since we don't have control over it, don't
// include it.
#endif
BOOL CBagMovieObject::RunObject() {
	BOOL rc;
	CBagPDA *pPDA = NULL;
	CBofWindow *pNewWin = NULL;
	SBZoomPda *pPDAz = (SBZoomPda *)SDEVMNGR->GetStorageDevice("BPDAZ_WLD");
	BOOL bZoomed = (pPDAz == NULL ? FALSE : pPDAz->GetZoomed());

	// MDM 7/22
	// Get a pointer to the current game window
	CBagStorageDevWnd *pMainWin = (CBagel::GetBagApp()->GetMasterWnd()->GetCurrentStorageDev());
	// MDM 7/22
	// Get a pointer to the current game window
	// CBagPanWindow* pMainWin=(CBagPanWindow*)(CBagel::GetBagApp()->GetMasterWnd()->GetCurrentGameWindow());

	rc = TRUE;
	if (!m_bFlyThru || CBagMasterWin::GetFlyThru()) {

		rc = FALSE;

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
		enum MOVFILETYPE { NONE = 0,
		                   TEXT = 1,
		                   SOUND = 2,
		                   MOVIE = 3
		                 } nMovFileType;
		CBofString sBaseStr = sFileName.Left(nExt);

#if BOF_MAC
		// jwl just temporary, trying to get the diskid right for multiple
		// game disks
		INT nDiskID = CBagel::GetBagApp()->GetMasterWnd()->GetDiskID();
		if (nDiskID > 0 && nDiskID < 4) {
			CHAR szFirst6[7];

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
			nMovFileType = MOVIE;
		} else if (sFileName.Find(SOUNDFILEEXTLOWER) > 0 || sFileName.Find(SOUNDFILEEXTUPPER) > 0) {
			nMovFileType = SOUND;
		} else if (sFileName.Find(".txt") > 0 || sFileName.Find(".TXT") > 0) {
			nMovFileType = TEXT;
		} else {
			nMovFileType = NONE;
		}

		// Look for .SMK then .WAV, then .TXT
		//
		while (!FileExists(sFileName.GetBuffer())) {

			switch (nMovFileType) {

			case MOVIE:
				sFileName = sBaseStr + SOUNDFILEEXTLOWER;
				nMovFileType = SOUND;
				break;

			case SOUND:
				sFileName = sBaseStr + ".txt";
				nMovFileType = TEXT;
				break;

			case TEXT:
				// sFileName = sBaseStr + ".ARF"
				BofMessageBox(sFileName.GetBuffer(), "Could not find asset");
				nMovFileType = NONE;
				break;

			// We should never get here
			//
			case NONE:
			default:
				LogError(BuildString("Movie does not have a correct file name: %s.", sFileName.GetBuffer()));
				return (rc);
			}
		}

		if (nMovFileType == MOVIE) {
#endif

			BOOL isFiltered = FALSE;
			CBagMasterWin *pWnd = CBagel::GetBagApp()->GetMasterWnd();
			CBagStorageDevWnd *pSDevWnd = (pWnd ? pWnd->GetCurrentStorageDev() : NULL);

			// Get the pda here, we need it so much anyway.
			if (pPDA == NULL) {
				pPDA = (CBagPDA *)SDEVMNGR->GetStorageDevice("BPDA_WLD");
			}
			Assert(pPDA != NULL);

			// jwl 11.29.96 If we have an asnych movie to play, make sure it is a good
			// time to play it, if not, then queue it up so it can play at a much better time.
			if (m_xDisplayType == ASYNCH_PDAMSG) {
				if (AsynchPDAMovieCanPlay() == FALSE) {
					pPDA->AddToMovieQueue(this);
					return rc;
				}
			} else {
				// jwl 11.29.96 don't need to redraw for asynch pda messages, this just
				// messes things up in the PDA redraw code (trust me)
				//
				// BCW 08/27/96 04:29 pm
				// Hide the cursor by causing 1 more paint (and specifying no cursor)
				//
				if (pWnd != NULL) {
					if (pSDevWnd != NULL) {
						pSDevWnd->PaintScreen(NULL, FALSE);
						isFiltered = pSDevWnd->IsFiltered();
					}
				}
			}

			if (m_xDisplayType == EXAMINE) {
				CBofRect r(160, 60, 480, 300);

				// Offset the rect for the movies to compensate for all screen sizes
				r.OffsetRect(((CBofWindow *)pMainWin)->GetWindowRect().TopLeft());
				// MDM 7/22
				//
				// jwl 12.18.96 If we have a movie playing in the zoom pda, then black out
				// the background.  Examine movies will always play with a black background
				// on the mac (prevents a palette shift).

#if BOF_MAC
				pNewWin = new CBofWindow();
				if (pNewWin) {
					pNewWin->Create("BLACK", 0, 0, 640, 480, CBofApp::GetApp()->GetMainWindow(), 0);
					pNewWin->FillWindow(COLOR_BLACK);
				}

				// jwl 1.22.97 set the first frame to be black so that we don't get a palette shift.
				CBagExam *pMovie = new CBagExam(CBagel::GetBagApp()->GetMasterWnd()->GetCurrentGameWindow(), sFileName, &r, FALSE, TRUE, m_xDisplayType == EXAMINE);
#else
				CBagExam *pMovie = new CBagExam(CBagel::GetBagApp()->GetMasterWnd()->GetCurrentGameWindow(), sFileName, &r);
#endif
				if (pMovie) {
					// jwl 1.2.96 if there is an associated sound file, then start it up here.
					CBagSoundObject *pSObj = GetAssociatedSound();
					if (pSObj) {
						if (pSObj->IsAttached() == FALSE) {
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
					rc = TRUE;

					// As soon as we're done, detach (this will also stop the sound).
					if (pSObj) {
						pSObj->Detach();
					}
				}

#if BOF_MAC
				if (pNewWin) {
					delete pNewWin;
					pNewWin = NULL;
				}
#endif
			} else {
				BOOL bActivated = FALSE;
				CBofRect r(80, 10, 80 + 480 - 1, 10 + 360 - 1);

				// Offset the rect for the movies to compensate for all screen sizes
				r.OffsetRect(((CBofWindow *)pMainWin)->GetWindowRect().TopLeft());

				if (m_xDisplayType == PDAMSG || m_xDisplayType == ASYNCH_PDAMSG) {

					// Pull up the PDA (if it exists)
					//
					// jwl 11.21.96 only pull up the PDA if we're not playing an asynch movie
					if (m_xDisplayType == PDAMSG) {

						// jwl 10.23.96 increment timer one, pda message counts as one turn
						// jwl 12.05.96 allow scripter to override timer increment
						if (IsIncrement()) {
							VARMNGR->IncrementTimers();
						}
					}

					if (pMainWin->GetDeviceType() == SDEV_GAMEWIN) {
						// jwl 1.10.97 if the pda is going up or down, then wait for it
						// to do its thing before attempting to activate it.
						if (pPDA->IsActivating()) {
							((CBagPanWindow *)pMainWin)->WaitForPDA();
						}

						if (pPDA->IsActivated() == FALSE) {
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
				// MDM 7/22

				// MDM 7/17
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

					if (pMovie != NULL && pMovie->ErrorOccurred() == FALSE) {
						pMovie->Show();
						CBofApp::GetApp()->GetMainWindow()->FlushAllMessages();
						pWnd->FlushAllMessages();
						pMovie->Play(FALSE);
						delete pMovie;
						rc = TRUE;
					} else {
						LogError(BuildString("Movie file could not be read: %s.  How? You removed that CD again didn't you", sFileName.GetBuffer()));
					}

					if (pNewWin) {
						delete pNewWin;
						pNewWin = NULL;
					}
				} else {

					// jwl 12.13.96 temp fix... maybe, allow script to override some
					// other movies.
					if ((m_xDisplayType == PDAMSG) && pMainWin->IsCIC() && IsDontOverride() == FALSE) {

						CHAR szLocalBuff[256];
						CBofString cStr(szLocalBuff, 256);

						// Play the override message.
						cStr = OVERRIDESMK;
						MACROREPLACE(cStr);

						sFileName = cStr;
					}

					if (m_xDisplayType == ASYNCH_PDAMSG) {
						// Tell our PDA to switch gears to do asynch movie time.
						// jwl 11.29.96
						if (pPDA) {
							if (pPDA->ShowMovie()) {       // returns false if another movie playing
								pPDA->SetMovie(sFileName); // set the movie to play
							}
						} else {
							LogError(BuildString("Movie file could not be read: %s.  How? You removed that CD again didn't you", sFileName.GetBuffer()));
						}
					} else {
						CBofMovie *pMovie;

						if (bZoomed && m_xDisplayType != ASYNCH_PDAMSG && m_xDisplayType != PDAMSG) {
							pNewWin = new CBofWindow();
							if (pNewWin) {
								pNewWin->Create("BLACK", 0, 0, 640, 480, CBofApp::GetApp()->GetMainWindow(), 0);
								pNewWin->Show();
								pNewWin->FillWindow(COLOR_BLACK);
							}
						}

						// If playing a PDA message while the PDA is zoomed
						//
						if (m_xDisplayType == PDAMSG && bZoomed) {

							// Then stretch it to fit into the PDA's viewscreen
							// r.SetRect(24, 47, 24 + 600 - 1, 47 + 302 - 1);

							r.SetRect(24, 47, 28 + 600 - 1, 47 + 302 - 1);
							pMovie = new CBofMovie(CBofApp::GetApp()->GetMainWindow(), sFileName, &r, TRUE);

							// Otherwise, just play the movie normally
							//
						} else {
#if BOF_MAC
							pMovie = new CBofMovie(CBofApp::GetApp()->GetMainWindow(), sFileName, &r, FALSE, (m_xDisplayType != PDAMSG), (m_xDisplayType == EXAMINE || m_bFlyThru == TRUE));
#else
							pMovie = new CBofMovie(CBofApp::GetApp()->GetMainWindow(), sFileName, &r);
#endif
						}

						if (pMovie && pMovie->ErrorOccurred() == FALSE) {
							// jwl 11.21.96 stop any asnych movies already playing
							pPDA->StopMovie(TRUE);
							pMovie->Show();
							CBofApp::GetApp()->GetMainWindow()->FlushAllMessages();
							pWnd->FlushAllMessages();
							pMovie->Play(FALSE);
							delete pMovie;
							rc = TRUE;
						} else {
							LogError(BuildString("Movie file could not be read: %s.  How? You removed that CD again didn't you", sFileName.GetBuffer()));
						}

						// jwl 12.24.96 if we put a black window up, then
						if (pNewWin) {
							delete pNewWin;
							pNewWin = NULL;
						}
					}
				}

				// jwl 12.30.96 put the pda down if we brought it up. (8638)
				if (m_xDisplayType != ASYNCH_PDAMSG && bActivated) {
#if BOF_MAC
					// jwl 10.14.96 make sure the window is up before we drop the pda
					((CBagPanWindow *)pMainWin)->Show();
#endif
					((CBagPanWindow *)pMainWin)->DeactivatePDA();
					((CBagPanWindow *)pMainWin)->WaitForPDA();
				}

				// jwl 12.30.96 if we're asynch, then let it know to deactivate when
				// done playing.
				if (m_xDisplayType == ASYNCH_PDAMSG) {
					pPDA->SetDeactivate(bActivated);
				}
			}

			// jwl 10.11.96 movies usually mark the transition from one view to another
			// but not necessarily a change of sdev's, so make sure we repaint the
			// backdrop
			if (pMainWin) {
				pMainWin->SetPreFilterPan(TRUE);
			}
#ifndef BOF_FINAL_RELEASE
		} else if (nMovFileType == SOUND) {
			CBofSound *pSound = new CBofSound(CBofApp::GetApp()->GetMainWindow(), sFileName, SOUND_WAVE);
			if (pSound) {
				pSound->Play();
				delete pSound;
				rc = TRUE;
			} else {
				LogError(BuildString("Movie SOUND file could not be read: %s.  Where? Not in Kansas ...", sFileName.GetBuffer()));
			}
		} else if (nMovFileType == TEXT) {
#if BOF_MAC
			BOOL isOpen = FALSE;
			if (FileExists(sFileName)) {
				ifstream fpTest(sFileName, 0);
				if (fpTest.is_open()) {
					isOpen = TRUE;
					char str[255];
					fpTest.getline(str, 255);

					BofMessageBox(str, "Incoming Message...");
					fpTest.close();
					rc = TRUE;
				}
			}

			if (isOpen == FALSE) {
				LogError(BuildString("Movie TEXT file could not be read: %s.  Why? because we like you ...", sFileName.GetBuffer()));
			}
#else
			Common::File f;
			if (f.open(sFileName.GetBuffer())) {
				Common::String line = f.readLine();

				BofMessageBox(line.c_str(), "Incoming Message...");
				f.close();
				rc = TRUE;
			} else {
				LogError(BuildString("Movie TEXT file could not be read: %s.  Why? because we like you ...", sFileName.GetBuffer()));
			}
#endif
		}

#endif
		rc = CBagObject::RunObject();
	}

	return (rc);
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
	BOOL nObjectUpdated = FALSE;
	char ch;
	CHAR szLocalStr[256];
	CBofString sStr(szLocalStr, 256); // jwl 08.28.96 performance improvement

	while (!istr.eof()) {
		nChanged = 0;
		istr.EatWhite(); // Eat any white space between script elements jwl 12.02.96

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
					m_xDisplayType = EXAMINE;
				} else if (!sStr.Find("MOVIE")) {
					m_xDisplayType = MOVIE;

				} else if (!sStr.Find("FLYTHRU")) {
					m_xDisplayType = MOVIE;
					m_bFlyThru = TRUE;
				} else if (!sStr.Find("PDAMSG")) {
					m_xDisplayType = PDAMSG;
				} else if (!sStr.Find("ASYNCH_PDAMSG")) {
					m_xDisplayType = ASYNCH_PDAMSG;

					// jwl 12.13.96 see if this improves performance any...
					SetPreload(TRUE);
				}
			}

			nObjectUpdated = TRUE;
			nChanged++;
		}
		break;
		// jwl 12.02.96 dont queue attribute, when set, the asynch movie either plays
		// immediately or not at all.
		case 'D': {
			GetAlphaNumFromStream(istr, sStr);

			if (!sStr.Find("DONTQUEUE")) {
				SetDontQueue();
				nObjectUpdated = TRUE;
				nChanged++;
			} else {
				if (!sStr.Find("DONTOVERRIDE")) {
					SetDontOverride();
					nObjectUpdated = TRUE;
					nChanged++;
				} else {
					// Don't increment the timer when playing this movie
					if (!sStr.Find("DONTINCREMENT")) {
						SetIncrement(FALSE);
						nObjectUpdated = TRUE;
						nChanged++;
					} else {
						PutbackStringOnStream(istr, sStr);
					}
				}
			}
		}
		break;
		// jwl 12.02.96 dont queue attribute, when set, the asynch movie either plays
		// immediately or not at all.
		case 'P': {
			GetAlphaNumFromStream(istr, sStr);

			if (!sStr.Find("PLAYIMMEDIATE")) {
				SetPlayImmediate();
				nObjectUpdated = TRUE;
				nChanged++;
			} else {
				PutbackStringOnStream(istr, sStr);
			}
		}
		break;
		// jwl 12.17.96 This is probably going to be much more important to the
		// mac version... give the option of playing the movie on a black
		// background.  this solves the problem of palette shifts on examine
		// movies.
		case 'O': {
			GetAlphaNumFromStream(istr, sStr);

			if (!sStr.Find("ONBLACK")) {
				SetPlayImmediate();
				nObjectUpdated = TRUE;
				nChanged++;
			} else {
				PutbackStringOnStream(istr, sStr);
			}
		}
		break;

		// jwl 1.2.97 associate a sound file with this movie (primarily for examine
		// movies).
		case 'S': {
			GetAlphaNumFromStream(istr, sStr);
			if (!sStr.Find("SND")) {
				nObjectUpdated = TRUE;
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
				nObjectUpdated = TRUE;
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

// jwl 11.21.96 Don't play movie if we're zoomed or if we're in a CIC
// or a sound is playing or another movie is playing...
BOOL CBagMovieObject::AsynchPDAMovieCanPlay() {
	BOOL bCanPlay = TRUE;

	// Obvious case, if it is set for immediate, return true.
	if (IsPlayImmediate()) {
		return bCanPlay;
	}

	CHAR szLocalBuff[256];
	CBofString sStr(szLocalBuff, 256);

	sStr = "BPDAZ_WLD";
	SBZoomPda *pPDAz = (SBZoomPda *)SDEVMNGR->GetStorageDevice(sStr);

	sStr = "BPDA_WLD";
	CBagPDA *pPDA = (CBagPDA *)SDEVMNGR->GetStorageDevice(sStr);

	CBagPanWindow *pMainWin = (CBagPanWindow *)(CBagel::GetBagApp()->GetMasterWnd()->GetCurrentGameWindow());

	// jwl 11.29.96 queue this message if any one of a variety of things is happening.
	Assert(pPDA != NULL);
	Assert(pPDAz != NULL);

	if (pPDA && pPDAz) {
		if ((pPDAz && pPDAz->GetZoomed()) ||              // we're zoomed
		        (pMainWin->IsCIC() && !IsDontOverride()) || // we're in a character closeup
		        CBofSound::SoundsPlayingNotOver() ||        // a sound is playing
		        pPDA->GetPDAMode() == MOOMODE) {            // an asynch movie is already playing
			bCanPlay = FALSE;
		}
	}

	return bCanPlay;
}

} // namespace Bagel
