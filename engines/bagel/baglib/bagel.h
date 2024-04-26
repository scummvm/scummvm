
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
#include "bagel/boflib/gfx/cursor.h"
#include "bagel/boflib/list.h"
#include "bagel/boflib/string.h"
#include "bagel/boflib/vhash_table.h"

namespace Bagel {

// Set a default hash table size here.
// Note: Best performance is achieved if this value is a prime number!
//
#define HASH_TABLE_SIZE 131

#define MAX_APP_NAME 128

#define PATH_DELIMETER "/"

#define DISK_1 1

#define BAG_INSTALL_NONE 0 /* play entire game from where it was executed */
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
	const char *_gameName;		// Game Name. Ex: "The Space Bar"
	const char *_gamePath;		// Relative path for the CD: "\\SPACEBAR"
	const char *_optionFile;	// This game's INI file name
	const char *_saveGameFile;	// Name of save game Index file.
	uint32 _ramRequired;		// Amount of free RAM needed to play game
	int32 _numberOfCDs;			// # of CDs used by this game
	int _requiredDepth;			// Required bits per pixel to play game
	int _requiredWidth;			// Minimum screen width for game
	int _requiredHeight;		// Minimum screen height for game
};

class CBagel : public CBofOptions, public CBofApp {
public:
	CBagel(const BagelReg *pGameReg);
	~CBagel();

	/**
	 * Registers game information for this game object
	 * @param pGameReg      Game registration info
	 */
	void registerGame(const BagelReg *pGameReg);

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
	ErrorCode runApp() override;

	/**
	 * Performs cleanup and destruction of Bagel object
	 * @return          Error return Code.
	 */
	ErrorCode shutdown() override;

	/**
	 * Sets specified user option in associated .INI file
	 * @param pszSection        .INI section to write to
	 * @param pszOption         Option to add/update
	 * @param pszValue          New value of option
	 * @return                  Error return code
	 */
	ErrorCode setOption(const char *pszSection, const char *pszOption, const char *pszValue);

	/**
	 * Sets specified user option in associated .INI file
	 * @param pszSection        .INI section to write to
	 * @param pszOption         Option to add/update
	 * @param nValue            New value of option
	 * @return                  Error return code
	 */
	ErrorCode setOption(const char *pszSection, const char *pszOption, int nValue);

	/**
	 * Gets specified user option from associated .INI file
	 * @param pszSection        .INI section to read from
	 * @param pszOption         Option to retrieve
	 * @param pszValue          Buffer to hold value
	 * @param pszDefault        Default value if option not found
	 * @param nSize             Length of the pszValue buffer
	 * @return                  Error return code
	*/
	ErrorCode getOption(const char *pszSection, const char *pszOption, char *pszValue, const char *pszDefault, uint32 nSize);

	/**
	 * Gets specified user option from associated .INI file
	 * @param pszSection        .INI section to read from
	 * @param pszOption         Option to retrieve
	 * @param nValue            Buffer to hold value
	 * @param nDefault          Default value if option not found
	 * @return                  Error return code
	 */
	ErrorCode getOption(const char *pszSection, const char *pszOption, int *nValue, int nDefault);

	/**
	 * Gets specified user option from associated .INI file
	 * @param pszSection        .INI section to read from
	 * @param pszOption         Option to retrieve
	 * @param nValue            Buffer to hold value
	 * @param nDefault          Default value if option not found
	 * @return                  Error return code
	 */
	ErrorCode getOption(const char *pszSection, const char *pszOption, bool *nValue, int nDefault);

	void setAppName(const char *pszNewAppName) {
		Common::strcpy_s(m_szAppName, pszNewAppName);
	}

	CBagMasterWin *getMasterWnd() {
		return (CBagMasterWin *)m_pMainWnd;
	}

	static CBagel *GetBagApp() {
		return (CBagel *)m_pBofApp;
	}

	int getChromaColor() {
		return DEFAULT_CHROMA_COLOR;
	}

	/**
	 * Checks to make sure the Game CD is in the drive
	 * @param nDiskID       Disk number
	 * @param pszWaveFile   Filename
	 * @return              Error return code
	 */
	ErrorCode VerifyCDInDrive(int nDiskID = DISK_1, const char *pszWaveFile = nullptr);

	static void ShowNextCDDialog(CBofWindow *pParentWin, int nCDID);

	static CBofVHashTable<CBofString, HASH_TABLE_SIZE> *GetCacheFileList() {
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
	static CBofVHashTable<CBofString, HASH_TABLE_SIZE> *m_pCacheFileList;
};

} // namespace Bagel

#endif
