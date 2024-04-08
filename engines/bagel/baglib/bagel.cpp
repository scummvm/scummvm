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
#include "bagel/dialogs/next_cd_dialog.h"
#include "bagel/dialogs/opt_window.h"
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
                        for (int jj = 0; jj < n; jj++) {                                \
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

// local functions
void RemoveDoubleDelimeter(char *pszBuf);

#if BOF_MAC
int16 CBagel::m_nVRefNum = 0;
#endif

CBagel::CBagel() {
	RegisterGame(nullptr);
}

CBagel::CBagel(const BagelReg *pGameReg) {
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

void CBagel::RegisterGame(const BagelReg *pGameReg) {
	Assert(IsValidObject(this));
	m_pGameReg = pGameReg;

	// Init statics
	g_cHomeDir = HOMEDIR_DFLT;
	g_cCacheDir = "";
	g_pHackWindow = nullptr;

	// Use registration info to init this game object
	if (pGameReg != nullptr) {
		// Keep application name
		SetAppName(pGameReg->m_pszGameName);

		// Load this game's .ini file
		if (pGameReg->m_pszOptionFile != nullptr)
			LoadOptionFile(pGameReg->m_pszOptionFile);
	}
}

ERROR_CODE CBagel::SetOption(const char *pszSection, const char *pszOption, const char *pszValue) {
	Assert(IsValidObject(this));
	return WriteSetting(pszSection, pszOption, pszValue);
}

ERROR_CODE CBagel::SetOption(const char *pszSection, const char *pszOption, int nValue) {
	Assert(IsValidObject(this));
	return WriteSetting(pszSection, pszOption, nValue);
}

ERROR_CODE CBagel::GetOption(const char *pszSection, const char *pszOption, char *pszValue, const char *pszDefault, uint32 nSize) {
	Assert(IsValidObject(this));
	return ReadSetting(pszSection, pszOption, pszValue, pszDefault, nSize);
}

ERROR_CODE CBagel::GetOption(const char *pszSection, const char *pszOption, int *pValue, int nDefault) {
	Assert(IsValidObject(this));
	return ReadSetting(pszSection, pszOption, pValue, nDefault);
}

ERROR_CODE CBagel::GetOption(const char *pszSection, const char *pszOption, bool *pValue, int nDefault) {
	Assert(IsValidObject(this));
	return ReadSetting(pszSection, pszOption, pValue, nDefault);
}

ERROR_CODE CBagel::initialize() {
	Assert(IsValidObject(this));

	// Game must already be registered with RegisterGame()
	Assert(m_pGameReg != nullptr);

	CBofApp::initialize();

	GetOption("UserOptions", "WrongCDRetries", &m_nNumRetries, 20);
	if (m_nNumRetries < 1) {
		m_nNumRetries = 100;
	}

	LogInfo("Initializing BAGEL");

	// Turn off System cursor
	CBofCursor::Hide();

	// Initialize local game paths
	InitLocalFilePaths();

	// Check for adequate system resources
	VerifyRequirements();

	// If the game is installed, initialize the hard disk drive file cache
	if (m_nInstallCode != BAG_INSTALL_NONE && g_cCacheDir != g_cHomeDir) {
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

ERROR_CODE CBagel::shutdown() {
	Assert(IsValidObject(this));

	LogInfo("Shutting down BAGEL");

	// Just in case the child class forgot to delete the main window,
	// I'll do it.
	if (m_pMainWnd != nullptr) {
		delete m_pMainWnd;
		m_pMainWnd = nullptr;
	}

	return CBofApp::shutdown();
}


ERROR_CODE CBagel::SetActiveCursor(int nCurs) {
	Assert(nCurs >= 0 && nCurs < MAX_CURSORS);

	CBagMasterWin::SetActiveCursor(nCurs);

	return ERR_NONE;
}

ERROR_CODE CBagel::InitLocalFilePaths() {
	Assert(IsValidObject(this));

	char szCurrentDir[MAX_DIRPATH];
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
		char szPathName[256];
		byte *pStr;

		// set up pathname
		Common::strcpy_s(szPathName, m_szInstallPath);

		// convert C string to Pascal String - IN PLACE
		pStr = (byte *)StrCToPascal(szPathName);

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

	// Get home directory for this game
	char szBuf[MAX_DIRPATH];
	char szDefaultHome[MAX_DIRPATH];

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

#define LOADINGBMP          "$SBARDIR\\GENERAL\\SYSTEM\\LOADING.BMP"

ERROR_CODE CBagel::VerifyCDInDrive(int nDiskID, const char *pszWaveFile) {
	Assert(IsValidObject(this));
#if BOF_MAC
	bool        bEjectDisk = FALSE;
	VolumeParam vInfo;
	OSErr       oserr = noErr;
	CBofWindow *pBlackWin = nullptr;
#endif

	if (m_pGameReg->m_nNumberOfCDs > 0) {
		char szBuf[MAX_DIRPATH], szCD[MAX_DIRPATH];

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
		int i = 0;
		while (!shouldQuit() && !FileExists(cString.GetBuffer())) {
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
			int16 nVRefNum = GetVolumeFromPath(g_cHomeDir.GetBuffer());
			bEjectDisk = true;

			// If we get no disk to eject, then just check and see
			// if a disk that we know about is present.  If it is, then spit it
			// out for the user.
			if (nVRefNum == 0) {
				for (int ii = DISK_1; ii <= DISK_3; ii++) {
					if (ii != nDiskID) {
						char szDiskName[256];

						Common::sprintf_s(szDiskName, "SBDISK%d:", ii);
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
				char        szNewVol[MAX_DIRPATH];

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

			char szBuf[256];
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

	return m_errCode;
}

void CBagel::ScanTree(const char *pszRoot, const char *pszFilename, CBofVHashTable<CBofString, HASHTABLESIZE> *pCacheFileList) {
#if BOF_MAC
	CInfoPBRec      cipbr;
	// HFileInfo *fpb = (HFileInfo *)&cipbr;
	DirInfo *dpb = (DirInfo *)&cipbr;
	Str255          szOrigDir;
	OSErr           oserr = noErr;
	int             nIndex = 0;
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
	int32 nDirID = dpb->ioDrDirID;

	// now loop through the children.
	Str255          pszFileName;
	Str255          szFullFilePath;

	char szNewFile[256];
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
			sNewFileName = (char *)szFullFilePath;
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


void RemoveDoubleDelimeter(char *pszBuf) {
	Assert(pszBuf != nullptr);

	StrReplaceStr(pszBuf, BuildString("%s%s", PATH_DELIMETER, PATH_DELIMETER), PATH_DELIMETER);
}


bool MACROREPLACE(CBofString &s) {
	char *p;

	// Remove any homedir prefix. In ScummVM, all paths are relative
	// to the game folder automatically
	if ((p = strstr(s.GetBuffer(), HOMEDIR_TOKEN)) != nullptr)
		s = p + strlen(HOMEDIR_TOKEN) + 1;

	// Replace any backslashes with forward slashes
	while ((p = strchr(s.GetBuffer(), '\\')) != nullptr)
		*p = '/';

	return true;
}


void CBagel::ShowNextCDDialog(CBofWindow *pParentWin, int nCDID) {
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
