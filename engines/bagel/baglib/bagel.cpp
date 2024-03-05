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

#include "bagel/baglib/bagel.h"
#include "bagel/baglib/res.h"
#include "bagel/boflib/sound.h"
#include "bagel/baglib/dialogs.h"
#include "bagel/baglib/opt_window.h"
#include "bagel/boflib/misc.h"
#include "bagel/boflib/debug.h"

#if BOF_MAC
#include <quickdraw.h>
#include <qdoffscreen.h>
#include <files.h>
#include <mac.h>
#endif

namespace Bagel {

#if BOF_MAC
#define SPINANDWAIT(n) {EventRecord event;                                              \
                        for (INT jj = 0; jj < n; jj++) {                                \
                            ::WaitNextEvent (everyEvent, &event, 0xFFFFFFFF, nullptr);      \
                        }}
#endif

// Initialize class static member variables.
//
static unsigned stringHashFunction(const CBofString &s) {
	return s.Hash();
}
CBofVHashTable<CBofString, HASHTABLESIZE> *CBagel::m_pCacheFileList = new CBofVHashTable<CBofString, HASHTABLESIZE>(&stringHashFunction);

// Initialize global variables.
//
CBofString g_cHomeDir, g_cCacheDir;
CBofWindow *g_pHackWindow;

void CBagel::initStatics() {
	g_cHomeDir = HOMEDIR_DFLT;
	g_cCacheDir = "";
	g_pHackWindow = nullptr;
}

// local functions
VOID RemoveDoubleDelimeter(CHAR *pszBuf);

#if BOF_MAC
SHORT CBagel::m_nVRefNum = 0;
#endif

CBagel::CBagel() {
	RegisterGame(nullptr);
}

CBagel::CBagel(BAGEL_REG *pGameReg) {
	Assert(pGameReg != nullptr);

	RegisterGame(pGameReg);
}

CBagel::~CBagel() {
	Assert(IsValidObject(this));

	// Release options file
	Release();

	// Empty the file cache.
	if (m_pCacheFileList) {
		delete m_pCacheFileList;
		m_pCacheFileList = nullptr;
	}

	m_szAppName[0] = '\0';
	m_pMainWnd = nullptr;
	m_pGameReg = nullptr;
}

VOID CBagel::RegisterGame(BAGEL_REG *pGameReg) {
	Assert(IsValidObject(this));

	// Inits
	m_pMainWnd = nullptr;          // no main window yet
	m_pGameReg = pGameReg;
	m_bSavedGames = FALSE;      // no saved games that we know about
	m_szAppName[0] = '\0';      // Game name not yet determined
	m_nNumRetries = 20;

	// Use registration info to init this game object
	if (pGameReg != nullptr) {
		// keep application name
		SetAppName(pGameReg->m_pszGameName);

		// Load this App's .INI file (if any)
		//
		if (pGameReg->m_pszOptionFile != nullptr) {
			CHAR szBuf[MAX_DIRPATH], szFile[MAX_DIRPATH];

#if BOF_MAC || BOF_WINMAC
			// Storing the ini file in the preferences is such
			// a better thing to do then putting it on the top level of the
			// startup drive.
			GetPrefsDir(szBuf);
#else
			GetSystemDir(szBuf);
#endif
			Common::sprintf_s(szFile, "%s%s%s", szBuf, PATH_DELIMETER, pGameReg->m_pszOptionFile);
			RemoveDoubleDelimeter(szFile);

			LoadOptionFile(szFile);
		}
	}
}

ERROR_CODE CBagel::SetOption(const CHAR *pszSection, const CHAR *pszOption, const CHAR *pszValue) {
	Assert(IsValidObject(this));
	return (WriteSetting(pszSection, pszOption, pszValue));
}

ERROR_CODE CBagel::SetOption(const CHAR *pszSection, const CHAR *pszOption, INT nValue) {
	Assert(IsValidObject(this));
	return (WriteSetting(pszSection, pszOption, nValue));
}

ERROR_CODE CBagel::GetOption(const CHAR *pszSection, const CHAR *pszOption, CHAR *pszValue, const CHAR *pszDefault, UINT nSize) {
	Assert(IsValidObject(this));
	return (ReadSetting(pszSection, pszOption, pszValue, pszDefault, nSize));
}

ERROR_CODE CBagel::GetOption(const CHAR *pszSection, const CHAR *pszOption, INT *pValue, INT nDefault) {
	Assert(IsValidObject(this));
	return (ReadSetting(pszSection, pszOption, pValue, nDefault));
}

ERROR_CODE CBagel::GetOption(const CHAR *pszSection, const CHAR *pszOption, BOOL *pValue, INT nDefault) {
	Assert(IsValidObject(this));
	return (ReadSetting(pszSection, pszOption, pValue, nDefault));
}

ERROR_CODE CBagel::Initialize() {
	Assert(IsValidObject(this));

	// Game must already be registered with RegisterGame()
	Assert(m_pGameReg != nullptr);

	CBofApp::Initialize();

	GetOption("UserOptions", "WrongCDRetries", &m_nNumRetries, 20);
	if (m_nNumRetries < 1) {
		m_nNumRetries = 100;
	}

	BOOL bInit;

	LogInfo("Initializing BAGEL");

	// Turn off System cursor
	CBofCursor::Hide();

	// Initialize local game paths
	InitLocalFilePaths();

	// Check for adequate system resources
	bInit = VerifyRequirements();

	// Check for Saved Games
	m_bSavedGames = HaveSavedGames();

	// If the game is installed, Initialize the hard disk drive file cache
	if (m_nInstallCode != BAG_INSTALL_NONE && g_cCacheDir != g_cHomeDir) {
#if BOF_MAC
		m_nVRefNum = GetVolumeFromPath(g_cCacheDir.GetBuffer());
#endif
		ScanTree(g_cCacheDir.GetBuffer(), "*.*", GetCacheFileList());
	}

	// Child class must instantiate the Main Window
	return m_errCode;
}

ERROR_CODE CBagel::RunApp() {
	Assert(IsValidObject(this));

	// The main game window must have been created by now
	Assert(m_pMainWnd != nullptr);

	return CBofApp::RunApp();
}

ERROR_CODE CBagel::ShutDown() {
	Assert(IsValidObject(this));

	// Turn on System cursor
	CBofCursor::Show();

	LogInfo("Shutting down BAGEL");

	// Just in case the child class forgot to delete the main window,
	// I'll do it.
	if (m_pMainWnd != nullptr) {
		delete m_pMainWnd;
		m_pMainWnd = nullptr;
	}

	return CBofApp::ShutDown();
}


ERROR_CODE CBagel::SetActiveCursor(INT nCurs) {
	Assert(nCurs >= 0 && nCurs < MAX_CURSORS);

	CBagMasterWin::SetActiveCursor(nCurs);

	return (ERR_NONE);
}

ERROR_CODE CBagel::InitLocalFilePaths() {
	Assert(IsValidObject(this));

	CHAR szCurrentDir[MAX_DIRPATH];
	ERROR_CODE errCode;

	// Check for Installed state of game
	GetOption("Startup", "InstallCode", &m_nInstallCode, BAG_INSTALL_DEFAULT);

	// Get path to where user installed the game
	if ((errCode = GetCurrentDir(szCurrentDir)) != ERR_NONE) {
		ReportError(errCode, "CBagel::InitLocalFilePaths - GetCurrentDir() failed");
	}
	GetOption("Startup", "InstallPath", m_szInstallPath, szCurrentDir, MAX_DIRPATH);
#if BOF_MAC
	// Try to make m_szInstallPath an absolute pathname (if it isn't already)
	// everything appears to work OK if we don't do this, but I'm trying to err on the
	// side of caution.

	if (m_szInstallPath[0] == ':') {
		CHAR szPathName[256];
		UCHAR *pStr;

		// set up pathname
		Common::strcpy_s(szPathName, m_szInstallPath);

		// convert C string to Pascal String - IN PLACE
		pStr = (UCHAR *)StrCToPascal(szPathName);

		FSSpec theFSSpec;

		errCode = FSMakeFSSpec(0, 0, pStr, &theFSSpec);
		if (errCode == ERR_NONE) {
			short fullPathLength;
			Handle fullPath;

			errCode = FSpGetFullPath(&theFSSpec, &fullPathLength, &fullPath);

			if (errCode == ERR_NONE && fullPathLength < MAX_DIRPATH) {
				Assert(fullPath != nullptr);
				BlockMoveData(*fullPath, m_szInstallPath, fullPathLength);
				m_szInstallPath[fullPathLength] = 0;
				::DisposeHandle(fullPath);
			} else {
				LogError("Couldn't build full path name for relative InstallPath");
			}
		} else {
			LogError("Couldn't find relative InstallPath.");
		}
	}

	SetInstallPath(m_szInstallPath);
#endif

#if BOF_WINDOWS
	// Strip off trailing slash (if any)
	//
	if (m_szInstallPath[strlen(m_szInstallPath) - 1] == '\\') {
		m_szInstallPath[strlen(m_szInstallPath) - 1] = '\0';
	}
#endif

	// Set the cache file path from the install path.
	g_cCacheDir = m_szInstallPath;

	// Build full path to save game file
	Common::sprintf_s(m_szSaveGameFileName, "%s%s%s", m_szInstallPath, PATH_DELIMETER, m_pGameReg->m_pszSaveGameFile);
	RemoveDoubleDelimeter(m_szSaveGameFileName);

	// Get home directory for this game
	CHAR szBuf[MAX_DIRPATH];
	CHAR szDefaultHome[MAX_DIRPATH];

	// The HOMEDIR_DFLT method works fine on the windows side
	// as relative pathnames are a groovy thing... however, the MACROREPLACE
	// macro needs g_cHomeDir to be non-nullptr, and HOMEDIR_DFLT is defined
	// as the nullptr string... so when we first start the game (never played
	// before) the HomeDir will come back undefined... we must remedy this
	// situation.
#if BOF_MAC || BOF_WINMAC
	GetCurrentDir(szDefaultHome);
#else
	Common::strcpy_s(szDefaultHome, HOMEDIR_DFLT);
#endif
	GetOption("Startup", "HomeDir", szBuf, szDefaultHome, MAX_DIRPATH);
	g_cHomeDir = szBuf;

#if BOF_MAC || BOF_WINMAC
	SetCurrentDir(szBuf);
#endif

	// Build path to disk in CD_ROM drive (or current directory)
	Common::strcpy_s(m_szCDPath, g_cHomeDir.GetBuffer());

	return m_errCode;
}

#if BOF_MAC || BOF_WINMAC
#define LOADINGBMP          "$SBARDIR:GENERAL:SYSTEM:LOADING.BMP"
#else
#define LOADINGBMP          "$SBARDIR\\GENERAL\\SYSTEM\\LOADING.BMP"
#endif

ERROR_CODE CBagel::VerifyCDInDrive(INT nDiskID, const CHAR *pszWaveFile) {
	Assert(IsValidObject(this));
#if BOF_MAC
	BOOL        bEjectDisk = FALSE;
	VolumeParam vInfo;
	OSErr       oserr = noErr;
	CBofWindow *pBlackWin = nullptr;
#endif

	if (m_pGameReg->m_nNumberOfCDs > 0) {
		CHAR szBuf[MAX_DIRPATH], szCD[MAX_DIRPATH];
		INT i;

		// Find the drive that this disk is in
		Common::sprintf_s(szBuf, "DISK%d", nDiskID);
		GetOption(m_szAppName, szBuf, szCD, g_cHomeDir, MAX_DIRPATH - 1);

		// Build new path to CD
		if (szCD[0] != '\0') {
			Common::strcpy_s(m_szCDPath, szCD);

		} else {
			Common::strcpy_s(m_szCDPath, m_szInstallPath);
		}

		Common::sprintf_s(szBuf, "$SBARDIR%sDISK%d", PATH_DELIMETER, nDiskID);

		CBofString cString(szBuf, MAX_DIRPATH);
		MACROREPLACE(cString);

		// If the disk is not in drive, then inform the user
		i = 0;
		while (!FileExists(cString.GetBuffer())) {
			// display a dialog box that tells the user to put the CD back into
			// the drive, or Abort.
			LogInfo(BuildString("Unable to find game's DiskID as '%s'", cString.GetBuffer()));

			if (i++ > m_nNumRetries) {
				ReportError(ERR_FFIND, "Could not recover from missing CD");
				break;
			}

			// Play the Zelda "Alias, you should insert disk 1 at this time."
			if (i == 1 && pszWaveFile != nullptr) {
				BofPlaySound(pszWaveFile, SOUND_WAVE | SOUND_ASYNCH);
			}

#if BOF_MAC
			SHORT nVRefNum = GetVolumeFromPath(g_cHomeDir.GetBuffer());
			bEjectDisk = TRUE;

			// If we get no disk to eject, then just check and see
			// if a disk that we know about is present.  If it is, then spit it
			// out for the user.
			if (nVRefNum == 0) {
				for (INT i = DISK_1; i <= DISK_3; i++) {
					if (i != nDiskID) {
						CHAR szDiskName[256];

						Common::sprintf_s(szDiskName, "SBDISK%d:", i);
						nVRefNum = GetVolumeFromPath(szDiskName);
						if (nVRefNum != 0) {
							break;
						}
					}
				}
			}

			if (nVRefNum) {
				oserr = UnmountAndEject(nullptr, nVRefNum);

				if (oserr != noErr) {
					LogError(BuildString("Unable to eject mac volume, err = %d", oserr));
				}
			}
#endif

#if BOF_MAC
			if (pBlackWin == nullptr) {
				pBlackWin = new CBofWindow();
				if (pBlackWin) {
					pBlackWin->Create("BLACK", 0, 0, 640, 480, CBofApp::GetApp()->GetMainWindow(), 0);
					pBlackWin->FillWindow(COLOR_BLACK);
				}
			}
			ShowNextCDDialog(pBlackWin, nDiskID);
#else
			if (g_pHackWindow == nullptr) {
				ShowNextCDDialog(m_pMainWnd, nDiskID);
			} else {
				ShowNextCDDialog(g_pHackWindow, nDiskID);
			}
#endif

#if BOF_MAC
			if (m_pMainWnd) {
				pBlackWin->FillWindow(COLOR_BLACK);
#if PALETTESHIFTFIX
				::ShowWindow(pBlackWin->GetMacWindow());
				::SelectWindow(pBlackWin->GetMacWindow());
#else
				pBlackWin->Show();
#endif
			}

			short attempts = 0;
			SetWatchCursor();
			do {
				long        finalTicks;
				CHAR        szNewVol[MAX_DIRPATH];

				Common::strcpy_s(szNewVol, g_cHomeDir.GetBuffer());
				StrCToPascal(szNewVol);

				// wait a half a second, give some time to the system and check
				// and see if our disk is mounted yet.
				::Delay(30, &finalTicks);
				SPINANDWAIT(10);
				vInfo.ioCompletion = 0;
				vInfo.ioVolIndex = -1;
				vInfo.ioNamePtr = (StringPtr)szNewVol;
				vInfo.ioVRefNum = 0;
				oserr = ::PBGetVInfoSync((ParmBlkPtr)&vInfo);
				attempts++;

			} while (oserr != noErr && attempts < 20);
			::InitCursor();
#endif
		}
	}

#if BOF_MAC
	if (bEjectDisk) {
		if (pBlackWin) {

			CHAR szBuf[256];
			Common::strcpy_s(szBuf, LOADINGBMP);
			CBofString cStr(szBuf, 256);
			MACROREPLACE(cStr);

			CBofRect cRect;
			cRect.left = (640 - 180) / 2;
			cRect.top = (480 - 50) / 2;
			cRect.right = cRect.left + 180 - 1;
			cRect.bottom = cRect.top + 50 - 1;

			PaintBitmap(pBlackWin, cStr, &cRect);

			delete pBlackWin;
			pBlackWin = nullptr;
		}

		CBagMasterWin::SetActiveCursor(6);
	}

	// new home dir...
	SetCurrentDisk(nDiskID);
	DISKREPLACE(m_szCDPath);

	g_cHomeDir = m_szCDPath;
	SetCurrentDir(m_szCDPath);

#endif

	return m_errCode;
}

ERROR_CODE CBagel::VerifyRequirements() {
	Assert(IsValidObject(this));
	Assert(m_pGameReg != nullptr);

	VOID *pBuf;

	for (;;) {

#if BOF_MAC
		ULONG nFreeMem = GetFreeMem();
		if (nFreeMem < m_pGameReg->m_lRamRequired) {

			// Inform the user that there is not enough RAM to play this game

			ReportError(ERR_MEMORY, "There is only %ld bytes of free memory available, but this game requires %ld bytes.  Please close all other applications before running %s.", nFreeMem, m_pGameReg->m_lRamRequired);
			break;
		}

#else
		// Check for minimum ram required to play the game
		if ((pBuf = malloc(m_pGameReg->m_lRamRequired)) == nullptr) {
			// Inform the user that there is not enough RAM to play this game

			ReportError(ERR_MEMORY, "This game requires %ld bytes of free memory.  You should close all other applications before running %s.  If this does not work, you may want to try increasing your virtual memory space.",
			            m_pGameReg->m_lRamRequired, m_pGameReg->m_pszGameName);

			break;

		} else {
			free(pBuf);
		}
#endif

		//
		// Check disk space (need about 12Mb for saved games)
		//

#ifndef DEMO
#if BOF_WINDOWS
		CHAR szBuf[10];

		szBuf[0] = m_szSaveGameFileName[0];
		szBuf[1] = m_szSaveGameFileName[1];
		szBuf[2] = m_szSaveGameFileName[2];
		szBuf[3] = '\0';

		LogInfo(BuildString("Free Disk Space on '%s' is %ld bytes", szBuf, GetFreeDiskSpace(szBuf)));
		if (GetFreeDiskSpace(szBuf) < (LONG)(MAX_SAVEDGAMES * sizeof(ST_BAGEL_SAVE)) + 100000) {

			ReportError(ERR_SPACE, "The SpaceBar requires approximately %ld bytes of free disk space for Saved Games.  Please free up some more space, and re-run The SpaceBar.", (MAX_SAVEDGAMES * sizeof(ST_BAGEL_SAVE)) + 100000);
			break;
		}
#endif
#endif  // !DEMO

		// check virtual memory
		//

		// check for min OS version
		//

		// check for minimum graphics color depth and resolution
		//

		BOOL bUseDirectDraw;

		// Assume we are not using direct draw
		bUseDirectDraw = FALSE;

#if 0 //BOF_WINDOWS
		GetOption("UserOptions", "UseDirectDraw", &bUseDirectDraw, TRUE);
#endif

		//INT nActualDepth = 8;
#if !BOF_MAC
		if (!bUseDirectDraw && ScreenDepth() < m_pGameReg->m_nRequiredDepth) {
#endif
			// Make sure that we are operating in 256 colors (i.e. 8 bit).

#if BOF_MAC
			GDHandle    screenGD;
			GWorldPtr   curWorld;

			GetGWorld(&curWorld, &screenGD);

			//  First off, handle the case where 8 bit color isn't even
			//  supported.

			if (!HasDepth(screenGD, 8, 0, 0)) {
				ReportError(ERR_UNKNOWN, "The SpaceBar requires a monitor that can support 256 colors.");
				ReportMacError(kNot8BitDevice, nullptr);
				break;
			}

			// if our pixel depth is not 8 bit, then see if the user wants to switch, if
			// they don't, then bomb out of the game.  If they do, then use set depth
			// to switch it over.

			if ((*(*screenGD)->gdPMap)->pixelSize != 8) {
				if (QueryMacUser(kSwitchTo8BitColor)) {
					OSErr oserr = SetDepth(screenGD, 8, 0, 0);
					if (OS_ERROR(oserr)) {
						ReportError(ERR_UNKNOWN, "The SpaceBar can not run in this color mode.");
						ReportMacError(kErrorSwitchingTo8Bit, nullptr);
						break;
					}
				} else {
					ReportError(ERR_UNKNOWN, "User foolishly chose not to switch to 256 colors.");
					break;
				}
			}

			//nActualDepth = 8;
#elif BOF_WINDOWS
			// Inform the user that they must change the pixel depth in order
			// to play this game, or we must change it for them.
			if (::MessageBox(nullptr,
			                 "Your display is currently set to 256 colors, but The Space Bar requires that your display be set to 16bit Color.  If you continue now, some parts of the game may not display correctly.  Continue anyway?",
			                 "Display requirement failure",
			                 MB_YESNO | MB_ICONEXCLAMATION) == IDNO) {
				m_errCode = ERR_UNKNOWN;
				break;
			}
#else
			error("We want 256 colors in ScummVM");
#endif
#if !BOF_MAC
		}
#endif

		if (!bUseDirectDraw && (ScreenHeight() != m_pGameReg->m_nRequiredHeight || ScreenWidth() != m_pGameReg->m_nRequiredWidth)) {
#if BOF_MAC
#if !USEDRAWSPROCKET
			BofMessageBox(BuildString("For best performance, please change your video display to a resolution of (%d x %d).", m_pGameReg->m_nRequiredWidth, m_pGameReg->m_nRequiredHeight), "Invalid screen size");
#endif
#elif BOF_WINDOWS
			if (::MessageBox(nullptr, BuildString("It is strongly recommended that this game be played in a video mode of %d x %d.  Your current video mode is %d x %d.  Continue anyway?",
			                                      m_pGameReg->m_nRequiredWidth, m_pGameReg->m_nRequiredHeight, ScreenWidth(), ScreenHeight()),
			                 "Invalid video mode", MB_YESNO | MB_ICONEXCLAMATION) == IDNO) {
				m_errCode = ERR_UNKNOWN;
				break;
			}
#else
			error("Bad video mode");
#endif
		}


		if (!bUseDirectDraw && (ScreenHeight() < m_pGameReg->m_nRequiredHeight || ScreenWidth() < m_pGameReg->m_nRequiredWidth)) {
			// Inform the user that they must change the screen resolution in
			// order to play this game, or we must change it for them.
			//
			ReportError(ERR_UNKNOWN, "This game requires a graphics resolution of (%d x %d)", m_pGameReg->m_nRequiredHeight, m_pGameReg->m_nRequiredWidth);
			break;
		}
		break;
	}

	return m_errCode;
}


BOOL CBagel::HaveSavedGames() {
	m_bSavedGames = FALSE;

	if (FileExists(m_szSaveGameFileName)) {
		CBagSaveGameFile cSaveFile(m_szSaveGameFileName);

		m_bSavedGames = cSaveFile.AnySavedGames();
	}

	return m_bSavedGames;
}

ERROR_CODE CBagel::InitializeSoundSystem(WORD nChannels, DWORD nFreq, WORD nBitsPerSample) {
#if BOF_WINDOWS
	BOOL bDebugAudio;

	bDebugAudio = TRUE;
#ifdef _DEBUG
	GetOption("UserOptions", "DebugAudio", &bDebugAudio, TRUE);
#endif

	if (bDebugAudio) {

		// Initialize Smacker and the Miles Sound System
		//
		PCMWAVEFORMAT stFormat;
		INT nError;

		stFormat.wf.wFormatTag = WAVE_FORMAT_PCM;
		stFormat.wf.nChannels = nChannels;
		stFormat.wf.nSamplesPerSec = nFreq;
		stFormat.wf.nAvgBytesPerSec = nFreq * 1 * nChannels;
		stFormat.wf.nBlockAlign = (WORD)((nBitsPerSample / 8) * nChannels);
		stFormat.wBitsPerSample = nBitsPerSample;

		AIL_startup();

		// If DirectSound is installed, then use it
		//
		if ((nError = AIL_waveOutOpen(&m_hDriver, 0, 0, (LPWAVEFORMAT)&stFormat)) == 0) {
			SmackSoundUseMSS(m_hDriver);

			// Otherwise, fallback to Windows MCI
			//
		} else {
			CHAR *pszError;

			pszError = AIL_last_error();

			LogWarning("DirectSound NOT installed, falling back to WAVEOUT");

			AIL_set_preference(DIG_USE_WAVEOUT, YES);
			if ((nError = AIL_waveOutOpen(&m_hDriver, 0, WAVE_MAPPER, (LPWAVEFORMAT)&stFormat)) == 0) {
				SmackSoundUseMSS(m_hDriver);

			} else {
				ReportError(ERR_UNKNOWN, "Could not find a valid WAVE output Driver. AIL_Error: %s", AIL_last_error());
			}
		}

		// Attach the MIDI driver
		//
		if ((nError = AIL_midiOutOpen(&m_hMidiDriver, nullptr, MIDI_MAPPER)) == 0) {

		} else {

			if ((nError = AIL_midiOutOpen(&m_hMidiDriver, nullptr, 0)) == 0) {

			} else {
				ReportError(ERR_UNKNOWN, "Could not find a valid MIDI output Driver. AIL_Error: %s", AIL_last_error());
			}
		}

		CBofSound::SetVolume(CBagMasterWin::GetMidiVolume(), CBagMasterWin::GetWaveVolume());
	}

#endif

	return m_errCode;
}


ERROR_CODE CBagel::ShutDownSoundSystem() {
#if BOF_WINDOWS
	if (m_hDriver != nullptr) {
		AIL_waveOutClose(m_hDriver);
		m_hDriver = nullptr;
	}
	AIL_shutdown();
#endif

	return m_errCode;
}

void CBagel::ScanTree(const char *pszRoot, const char *pszFilename, CBofVHashTable<CBofString, HASHTABLESIZE> *pCacheFileList) {
#if BOF_MAC
	CInfoPBRec      cipbr;
	HFileInfo *fpb = (HFileInfo *)&cipbr;
	DirInfo *dpb = (DirInfo *)&cipbr;
	Str255          szOrigDir;
	OSErr           oserr = noErr;
	INT             nIndex = 0;
	Str255          pstrRoot;

	// preserve the original
	//
	// Use p-strings, they're much more mac friendly.  We'll convert back
	// to c-string for lists and recursing.
	Common::strcpy_s(PCHS(szOrigDir), pszRoot);
	szOrigDir[0] = strlen(pszRoot);
	pCommon::strcpy_s(pstrRoot, szOrigDir);

	// Must always assume that the root does not end with the path delimeter
	if (*(PLCHS(pstrRoot)) == *PATH_DELIMETER) {
		pstrRoot[0]--;
		szOrigDir[0]--;
	}

	// we're passed in a folder, index through it.
	memset(&cipbr, 0, sizeof(CInfoPBRec));
	dpb->ioFDirIndex = 0;
	dpb->ioNamePtr = szOrigDir;

	oserr = PBGetCatInfo(&cipbr, FALSE);
	if (oserr != noErr)
		return;

	// Save this folders directory id
	LONG nDirID = dpb->ioDrDirID;

	// now loop through the children.
	Str255          pszFileName;
	Str255          szFullFilePath;

	CHAR szNewFile[256];
	CBofString sNewFileName(szNewFile, 256);

	while (true) {
		memset(&cipbr, 0, sizeof(CInfoPBRec));
		nIndex++;
		pszFileName[0] = 0;

		dpb->ioDrDirID = nDirID;
		dpb->ioFDirIndex = nIndex;
		dpb->ioNamePtr = pszFileName;
		dpb->ioVRefNum = m_nVRefNum;

		oserr = PBGetCatInfo(&cipbr, FALSE);
		if (oserr != noErr) {
			break;
		}

		// Build the full file spec

		pCommon::strcpy_s(szFullFilePath, pstrRoot);
		pstrcat(szFullFilePath, "\p:");
		pstrcat(szFullFilePath, pszFileName);

		// If folder, recurse
		if (dpb->ioFlAttrib & 0x0010) {
			ptocstr(szFullFilePath);
			ScanTree((const char *)szFullFilePath, nullptr, pCacheFileList);
		} else {
			// upper case it
			::UprString(szFullFilePath, false);

			ptocstr(szFullFilePath);
			sNewFileName = (CHAR *)szFullFilePath;
			pCacheFileList->insert(sNewFileName);
		}
	}

#elif BOF_WINDOWS
	HANDLE hFind;
	WIN32_FIND_DATA xFindData;
	char szTargetFilename[MAX_PATH] = "";

	// Build up the file name.
	//
	Common::strcpy_s(szTargetFilename, pszRoot);
	strcat(szTargetFilename, PATH_DELIMETER);
	strcat(szTargetFilename, pszFilename);

	// See if any files can be found.
	//
	hFind = ::FindFirstFile(szTargetFilename, &xFindData);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			if (!strcmp(xFindData.cFileName, ".")) {
				// Skip the "." directory entry.
				//
				;
			} else if (!strcmp(xFindData.cFileName, "..")) {
				// Skip the ".." directory entry.
				//
				;
			} else if (xFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				// We have a directory - recurse in that directory.
				//
				char szRecursiveScanPath[MAX_PATH] = "";

				Common::strcpy_s(szRecursiveScanPath, pszRoot);
				strcat(szRecursiveScanPath, PATH_DELIMETER);
				strcat(szRecursiveScanPath, xFindData.cFileName);

				ScanTree(szRecursiveScanPath, "*.*", pCacheFileList);
			} else {
				// Just an ordinary file (not a directory and not . or ..).
				//
				char szNewFile[MAX_PATH];

				Common::strcpy_s(szNewFile, pszRoot);
				strcat(szNewFile, PATH_DELIMETER);
				strcat(szNewFile, xFindData.cFileName);

				CBofString sNewFileName(szNewFile, MAX_PATH);
				pCacheFileList->insert(sNewFileName);
			}
		} while (::FindNextFile(hFind, &xFindData));

		::FindClose(hFind);
	}

#else
	error("TODO: ScummVM ScanTree");
#endif
}


VOID RemoveDoubleDelimeter(CHAR *pszBuf) {
	Assert(pszBuf != nullptr);

	StrReplaceStr(pszBuf, BuildString("%s%s", PATH_DELIMETER, PATH_DELIMETER), PATH_DELIMETER);
}


BOOL MACROREPLACE(CBofString &s) {
	CHAR *p;
#if BOF_MAC
	p = s.GetBuffer();
	for (INT i = 0; i < s.GetLength(); i++) {
		if (p[i] == '\\') {
			p[i] = *PATH_DELIMETER;
		}
	}
#endif
	// Just found that this routine is consuming approximately
	// 30% of of the load time as the string manipulation routines call malloc
	// and free 5 times each (not kidding).  Going to use simple string routines
	// instead.

#if 1 //BOF_MAC
	if ((p = strstr(s.GetBuffer(), HOMEDIR_TOKEN)) != nullptr) {

		// BCW - 09/29/96 12:33 pm
		// Need to remove any double delimeters
		//
		s.ReplaceStr(BuildString("%s%s", PATH_DELIMETER, PATH_DELIMETER), PATH_DELIMETER);

		char newStr[256];
		char *oldBuffp = s.GetBuffer();
		int oldLen = s.GetLength();
		int newLen = p - oldBuffp;
		int homeDirTokenLen = strlen(HOMEDIR_TOKEN);

		BofMemCopy(newStr, oldBuffp, newLen);

		if (newLen && newStr[newLen - 1] != *PATH_DELIMETER) {
			newStr[newLen++] = *PATH_DELIMETER;
		}

		// Look for the file on the hard disk.
		//
		if (g_cCacheDir != "") {

			BofMemCopy(&newStr[newLen], g_cCacheDir.GetBuffer(), g_cCacheDir.GetLength());
			newLen += g_cCacheDir.GetLength();

			if (newStr[newLen - 1] != *PATH_DELIMETER) {
				newStr[newLen++] = *PATH_DELIMETER;
			}

			p += homeDirTokenLen;
			if (*p == *PATH_DELIMETER) {
				p++;
			}

			BofMemCopy(&newStr[newLen], p, s.GetLength() + oldBuffp - p);
			newLen += (s.GetLength() + oldBuffp - p);
			newStr[newLen] = '\0';

			Assert(newLen < 256);

			// We must strip off any characters in the filename that do not
			// belong to a filename for the cache, and then put them
			// back after we check the cache.
			//
			CHAR *pEnd, chSave;
			BOOL bStripped, bInCache;

			chSave = '\0';
			bStripped = FALSE;
			if ((pEnd = StriStr(newStr, ".")) != nullptr) {
				pEnd += 4;
				if (*pEnd != '\0') {
					chSave = *pEnd;
					*pEnd = '\0';
					bStripped = TRUE;
				}
			}

			CBofVHashTable<CBofString, HASHTABLESIZE> *pCacheFileList = CBagel::GetCacheFileList();

			CBofString newString(newStr, 256);

#if BOF_MAC && !PLAYWAVONMAC
			// Stuff is cached by the name on the disk.  So the mac
			// will have .snd files on the disk.
			BOOL bReplaceExtension = FALSE;
			if (newString.Find(".WAV") >= 0) {
				newString.ReplaceStr(".WAV", ".SND");
				bReplaceExtension = TRUE;
			}
#endif
#if BOF_MAC
			BOOL bReplaceMidiExt = FALSE;
			if (newString.Find(".MID") >= 0) {
				newString.ReplaceStr(".MID", ".SMS");
				bReplaceMidiExt = TRUE;
			}
#endif
			bInCache = pCacheFileList->contains(newString);

#if BOF_MAC && !PLAYWAVONMAC
			if (bReplaceExtension) {
				newString.ReplaceStr(".SND", ".WAV");
			}
#endif
#if BOF_MAC
			if (bReplaceMidiExt) {
				newString.ReplaceStr(".SMS", ".MID");
			}
#endif

			// Put it back
			if (bStripped)
				*pEnd = chSave;

			// If this file is installed on user's hard drive, then use it.
			if (bInCache) {
				s = newStr;
				return TRUE;
			}
		}

		// The file is not in the disk cache.  Return a conventional file spec.
		p = strstr(s.GetBuffer(), HOMEDIR_TOKEN);
		oldBuffp = s.GetBuffer();
		oldLen = s.GetLength();
		newLen = p - oldBuffp;
		homeDirTokenLen = strlen(HOMEDIR_TOKEN);

		BofMemCopy(newStr, oldBuffp, newLen);

		if (newLen && newStr[newLen - 1] != *PATH_DELIMETER) {
			newStr[newLen++] = *PATH_DELIMETER;
		}

		BofMemCopy(&newStr[newLen], g_cHomeDir.GetBuffer(), g_cHomeDir.GetLength());
		newLen += g_cHomeDir.GetLength();

		if (newStr[newLen - 1] != *PATH_DELIMETER) {
			newStr[newLen++] = *PATH_DELIMETER;
		}

		// advance past the token
		p += homeDirTokenLen;

		if (*p == *PATH_DELIMETER)
			p++;

		BofMemCopy(&newStr[newLen], p, s.GetLength() + oldBuffp - p);
		newLen += (s.GetLength() + oldBuffp - p);

		newStr[newLen] = 0;

#if BOF_MAC
		// Make sure we are at the correct volume.
		DISKREPLACE(newStr);
#endif
		Assert(newLen < 256);

		s = newStr;

		// There had better not be a double delimeter in here
#if BOF_MAC
		Assert(strstr(s.GetBuffer(), "::") == nullptr);
#else
		Assert(strstr(s.GetBuffer(), "\\\\") == nullptr);
#endif

		return TRUE;
	}
#else
	if (!s.Find(HOMEDIR_TOKEN)) {
		s = g_cHomeDir + s.Mid(strlen(HOMEDIR_TOKEN));
		s.ReplaceStr(BuildString("%s%s", PATH_DELIMETER, PATH_DELIMETER), PATH_DELIMETER);
		return TRUE;
	}
#endif
	return FALSE;
}


VOID CBagel::ShowNextCDDialog(CBofWindow *pParentWin, INT nCDID) {
	CBagNextCDDialog cNextCDDialog;
	CBofRect cRect;

	// Use specified bitmap as this dialog's image
	CBofBitmap *pBmp;
	CBofPalette *pPal;

	pPal = nullptr;
	pBmp = nullptr;

	switch (nCDID) {

	case 1:
		pBmp = Bagel::LoadBitmap(BuildSysDir("DISK1.BMP"), pPal);
		break;

	case 2:
		pBmp = Bagel::LoadBitmap(BuildSysDir("DISK2.BMP"), pPal);
		break;

	case 3:
		pBmp = Bagel::LoadBitmap(BuildSysDir("DISK3.BMP"), pPal);
		break;

	// Shouldn't ever get here
	default:
		Assert(FALSE);
		break;
	}

	cNextCDDialog.SetBackdrop(pBmp);

	cRect = cNextCDDialog.GetBackdrop()->GetRect();

	// Create the dialog box
	cNextCDDialog.Create("NextCD", cRect.left, cRect.top, cRect.Width(), cRect.Height(), pParentWin);
	cNextCDDialog.Center();

	cNextCDDialog.DoModal();
#if BOF_MAC
	cNextCDDialog.Hide();
#if PALETTESHIFTFIX
	CBofWindow::CheckPaletteShiftList();
#endif
#endif
}

} // namespace Bagel
