
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

#ifndef BAGEL_BOFLIB_OPTIONS_H
#define BAGEL_BOFLIB_OPTIONS_H

#include "common/stream.h"

#include "bagel/boflib/stdinc.h"
#include "bagel/boflib/llist.h"
#include "bagel/boflib/object.h"
#include "bagel/boflib/error.h"

namespace Bagel {
namespace SpaceBar {

#define MAX_OPTION_LEN 100

class COption : public CLList, public CBofObject {
public:
	COption(const char *pszInit = nullptr);

	char _szBuf[MAX_OPTION_LEN];
};

class CBofOptions : public CBofObject { // CCache
private:
	/**
	 * Finds the specified INI section in current file
	 * @param pszSection    INI section
	 * @return              Pointer to option where this section starts
	 */
	COption *findSection(const char *pszSection);

	/**
	 * Finds the variable (option) in specified section
	 * @param pszSection    INI section
	 * @param pszVar        Variable name for this option
	 * @return              Pointer to option containing this variable
	 */
	COption *findOption(const char *pszSection, const char *pszVar);

	/**
	 * Reads one line of text from specified file
	 * @param pFile     Pointer to open file for reading
	 * @param pszBuf    Buffer to fill with text read
	 */
	bool readLine(Common::SeekableReadStream *pFile, char *pszBuf);

protected:
	char _szFileName[MAX_FNAME];
	COption *_pOptionList;
	bool _bDirty;

public:
	/**
	 * Constructor
	 * @brief       Loads specified .INI file which contains user options.
	 * @param pszOptionsFile    Name of .INI settings file
	**/
	CBofOptions(const char *pszOptionsFile = nullptr);

	/**
	 * Destructor
	*/
	virtual ~CBofOptions();

	/**
	 * Updates current option list file
	 * @return      Error return code
	 */
	ErrorCode commit();

	/**
	 * Loads specified .INI options file
	 * @brief           Loads and builds Option list
	 * @param pszFile   Name of .INI file to load
	 * @return          Error return code
	**/
	ErrorCode loadOptionFile(const char *pszFile);

	const char *getFileName() const {
		return ((const char *)_szFileName);
	}

	/**
	 * Adds or modifies 1 option in list
	 * @param pszSection        Section in .INI file
	 * @param pszOption         Option to update
	 * @param pszValue          New value
	 * @return                  Error return code
	 */
	ErrorCode writeSetting(const char *pszSection, const char *pszOption, const char *pszValue);

	/**
	 * Adds or modifies 1 option in list
	 * @param pszSection        Section in .INI file
	 * @param pszOption         Option to update
	 * @param nValue            New value
	 * @return                  Error return code
	 */
	ErrorCode writeSetting(const char *pszSection, const char *pszOption, int nValue);

	/**
	 * Reads value for the specified option
	 * @param section        Section in .INI file
	 * @param option         Option to update
	 * @param stringValue    Destination buffer for read value
	 * @param defaultValue   Default value if not exists
	 * @param nSize          Max length of stringValue buffer
	 * @return               Error return code
	 */
	ErrorCode readSetting(const char *section, const char *option, char *stringValue, const char *defaultValue, uint32 nSize);

	/**
	 * Reads value for the specified option
	 * @param section        Section in .INI file
	 * @param option         Option to update
	 * @param nValue            Pointer to write value to
	 * @param defaultValue          Default value if not exists
	 * @return                  Error return code
	 */
	ErrorCode readSetting(const char *section, const char *option, int *nValue, int defaultValue);

	/**
	 * Reads value for the specified option
	 * @param section        Section in .INI file
	 * @param option         Option to update
	 * @param boolValue            Pointer to write value to
	 * @param defaultValue          Default value if not exists
	 * @return                  Error return code
	 */
	ErrorCode readSetting(const char *section, const char *option, bool *boolValue, bool defaultValue);

	/**
	 * Loads current .INI options file
	 * @return          Error return code
	 */
	ErrorCode load();

	/**
	 * Updates and Releases current option list
	 */
	void release();
};

} // namespace SpaceBar
} // namespace Bagel

#endif
