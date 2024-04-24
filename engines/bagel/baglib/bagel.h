
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

#ifndef BAGEL_BAGLIB_BAGEL_H
#define BAGEL_BAGLIB_BAGEL_H

#include "bagel/boflib/app.h"
#include "bagel/boflib/gui/window.h"
#include "bagel/boflib/options.h"
#include "bagel/boflib/error.h"
#include "bagel/baglib/master_win.h"
#include "bagel/baglib/res.h"
#include "bagel/boflib/gfx/cursor.h"
#include "bagel/boflib/list.h"
#include "bagel/boflib/string.h"
#include "bagel/boflib/vhash_table.h"

namespace Bagel {

// If a definition of the hash table size for the file cache has not been supplied
// via the compiler's command line, set a default hash table size here.  Note:
// Best performance is achieved if this value is a prime number!
//
#ifndef HASHTABLESIZE
#define HASHTABLESIZE 131
#endif

#define MAX_APPNAME 128

#define PATH_DELIMETER "/"

#define DISK_1 1
#define DISK_2 2
#define DISK_3 3
#define DISK_4 4

#define BAG_INSTALL_NONE 0 /* play entire game from where it was executed */
#define BAG_INSTALL_MIN 1  /* minimal use of hard disk */
#define BAG_INSTALL_MED 2  /* medium use of hard disk */
#define BAG_INSTALL_MAX 3  /* substantial use of hard disk */

#define BAG_INSTALL_DEFAULT BAG_INSTALL_NONE

#define HOMEDIR_TOKEN "$SBARDIR" /* Change this to $HOMEDIR  */
// THE CURRENT STORAGE DEVICE OF THE GAME
#define CURRSDEV_TOKEN "$CURRENT_SDEV"
#define PREVSDEV_TOKEN "$PREVIOUS_SDEV"

// Defines default chroma color to be palette index 1
#define DEFAULT_CHROMA_COLOR 1

extern bool MACROREPLACE(CBofString &s);

class CBagMasterWin;

/**
 * Initialization structure fot CBagel app
 */
struct BagelReg {
	const char *m_pszGameName;		// Game Name. Ex: "The Space Bar"
	const char *m_pszGamePath;		// Relative path for the CD: "\\SPACEBAR"
	const char *m_pszOptionFile;	// This game's INI file name
	const char *m_pszSaveGameFile;	// name of save game Index file.
	uint32 m_lRamRequired;			// ammount of free RAM needed to play game
	int32 m_nNumberOfCDs;			// # of CDs used by this game
	int m_nRequiredDepth;			// Required bits per pixel to play game
	int m_nRequiredWidth;			// minimum screen width for game
	int m_nRequiredHeight;			// minimum screen height for game
};

class CBagel : public CBofOptions, public CBofApp {
public:
	CBagel(const BagelReg *pGameReg);
	~CBagel();

	/**
	 * Registers game information for this game object
	 * @param pGameReg      Game registration info
	 */
	void RegisterGame(const BagelReg *pGameReg);

	// these functions must be provided by the child class
	//
	/**
	 * Initializes BAGEL, checks system resources, etc...
	 * @return          Error return code
	 */
	ErrorCode initialize() override;

	/**
	 * Provides main message loop (MainEventLoop)
	 * @return          Error return Code.
	 */
	ErrorCode RunApp() override;

	/**
	 * Performs cleanup and destruction of Bagel object
	 * @return          Error return Code.
	 */
	ErrorCode shutdown() override;

	/**
	 * Sets specified user option in assocciated .INI file
	 * @param pszSection        .INI section to write to
	 * @param pszOption         Option to add/update
	 * @param pszValue          New value of option
	 * @return                  Error return code
	 */
	ErrorCode SetOption(const char *pszSection, const char *pszOption, const char *pszValue);

	/**
	 * Sets specified user option in assocciated .INI file
	 * @param pszSection        .INI section to write to
	 * @param pszOption         Option to add/update
	 * @param nValue            New value of option
	 * @return                  Error return code
	 */
	ErrorCode SetOption(const char *pszSection, const char *pszOption, int nValue);

	/**
	 * Gets specified user option from assocciated .INI file
	 * @param pszSection        .INI section to read from
	 * @param pszOption         Option to retrieve
	 * @param pszValue          Buffer to hold value
	 * @param pszDefault        Default value if option not found
	 * @param nSize             Length of the pszValue buffer
	 * @return                  Error return code
	*/
	ErrorCode GetOption(const char *pszSection, const char *pszOption, char *pszValue, const char *pszDefault, uint32 nSize);

	/**
	 * Gets specified user option from assocciated .INI file
	 * @param pszSection        .INI section to read from
	 * @param pszOption         Option to retrieve
	 * @param nValue            Buffer to hold value
	 * @param nDefault          Default value if option not found
	 * @return                  Error return code
	 */
	ErrorCode GetOption(const char *pszSection, const char *pszOption, int *nValue, int nDefault);

	/**
	 * Gets specified user option from assocciated .INI file
	 * @param pszSection        .INI section to read from
	 * @param pszOption         Option to retrieve
	 * @param nValue            Buffer to hold value
	 * @param nDefault          Default value if option not found
	 * @return                  Error return code
	 */
	ErrorCode GetOption(const char *pszSection, const char *pszOption, bool *nValue, int nDefault);

	void SetAppName(const char *pszNewAppName) {
		Common::strcpy_s(m_szAppName, pszNewAppName);
	}

	const char *GetAppName() {
		return (const char *)m_szAppName;
	}

	CBofWindow *GetMainWindow() {
		return m_pMainWnd;
	}

	const char *GetOptionsFileName() {
		return (const char *)m_szFileName;
	}

	CBagMasterWin *GetMasterWnd() {
		return (CBagMasterWin *)m_pMainWnd;
	}

	static CBagel *GetBagApp() {
		return (CBagel *)m_pBofApp;
	}

	int GetChromaColor() {
		return DEFAULT_CHROMA_COLOR;
	}

	static ErrorCode SetActiveCursor(int nCurs);

	/**
	 * Checks to make sure the Game CD is in the drive
	 * @param nDiskID       Disk number
	 * @param pszWaveFile   Filename
	 * @return              Error return code
	 */
	ErrorCode VerifyCDInDrive(int nDiskID = DISK_1, const char *pszWaveFile = nullptr);

	static void ShowNextCDDialog(CBofWindow *pParentWin, int nCDID);

	static CBofVHashTable<CBofString, HASHTABLESIZE> *GetCacheFileList() {
		return m_pCacheFileList;
	}

protected:
	/**
	 * initialize full path names to files stored on local disk
	 * @return          Error return code
	 */
	ErrorCode InitLocalFilePaths();
	ErrorCode InitGraphics();

	/**
	 * Checks system resources, determining if user has the minimum
	 * system requirements to play this game.
	 * @return          Error return code
	 */
	ErrorCode VerifyRequirements();

	// Data members
	const BagelReg *m_pGameReg = nullptr;

	int m_nNumRetries = 20;
	int m_nInstallCode = 0;

	CBofString m_lpCmdLine;
	CBofCursor m_cCursor;

private:
	static CBofVHashTable<CBofString, HASHTABLESIZE> *m_pCacheFileList;
};

} // namespace Bagel

#endif
