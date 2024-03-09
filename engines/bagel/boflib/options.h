
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
#include "bagel/boflib/boffo.h"
#include "bagel/boflib/stdinc.h"
#include "bagel/boflib/llist.h"
#include "bagel/boflib/object.h"
#include "bagel/boflib/error.h"

namespace Bagel {

#define MAX_OPTION_LEN 100

class COption : public CLList, public CBofObject {
public:
	COption(const CHAR *pszInit = nullptr) {
		m_szBuf[0] = '\0';

		if (pszInit != nullptr) {
			Assert(strlen(pszInit) < MAX_OPTION_LEN);
			Common::strcpy_s(m_szBuf, pszInit);
		}
	}

	CHAR m_szBuf[MAX_OPTION_LEN];
};

class CBofOptions : public CBofObject { // CCache
private:
	/**
	 * Finds the specified INI section in current file
	 * @param pszSection    INI section
	 * @return              Pointer to option where this section starts
	 */
	COption *FindSection(const CHAR *pszSection);

	/**
	 * Finds the variable (option) in specified section
	 * @param pszSection    INI section
	 * @param pszVar        Variable name for this option
	 * @return              Pointer to option containing this variable
	 */
	COption *FindOption(const CHAR *pszSection, const CHAR *pszVar);

	/**
	 * Reads one line of text from specified file
	 * @param pFile     Pointer to open file for reading
	 * @param pszBuf    Buffer to fill with text read
	 */
	BOOL ReadLine(Common::SeekableReadStream *pFile, CHAR *pszBuf);

protected:
	CHAR m_szFileName[MAX_FNAME];
	COption *m_pOptionList;
	BOOL m_bDirty;

public:
	/**
	 * Constructor
	 * @brief       Loads specified .INI file which contains user options.
	 * @param pszOptionsFile    Name of .INI settings file
	**/
	CBofOptions(const CHAR *pszOptionsFile = nullptr);

	/**
	 * Destructor
	*/
	virtual ~CBofOptions();

	/**
	 * Updates current option list file
	 * @return      Error return code
	 */
	ERROR_CODE Commit();

	/**
	 * Loads specified .INI options file
	 * @brief           Loads and builds Option list
	 * @param pszFile   Name of .INI file to load
	 * @return          Error return code
	**/
	ERROR_CODE LoadOptionFile(const CHAR *pszFile);

	const CHAR *GetFileName() const {
		return ((const CHAR *)m_szFileName);
	}

	/**
	 * Adds or modifies 1 option in list
	 * @param pszSection        Section in .INI file
	 * @param pszOption         Option to update
	 * @param pszValue          New value
	 * @return                  Error return code
	 */
	ERROR_CODE WriteSetting(const CHAR *pszSection, const CHAR *pszOption, const CHAR *pszValue);

	/**
	 * Adds or modifies 1 option in list
	 * @param pszSection        Section in .INI file
	 * @param pszOption         Option to update
	 * @param pszValue          New value
	 * @return                  Error return code
	 */
	ERROR_CODE WriteSetting(const CHAR *pszSection, const CHAR *pszOption, INT nValue);

	/**
	 * Reads value for the specified option
	 * @param pszSection        Section in .INI file
	 * @param pszOption         Option to update
	 * @param pszValue          destination buffer for read value
	 * @param pszDefault        Default value if not exists
	 * @param nSize             Max length of pszValue buffer
	 * @return                  Error return code
	 */
	ERROR_CODE ReadSetting(const CHAR *pszSection, const CHAR *pszOption, CHAR *pszValue, const CHAR *pszDefault, UINT nSize);

	/**
	 * Reads value for the specified option
	 * @param pszSection        Section in .INI file
	 * @param pszOption         Option to update
	 * @param nValue            Pointer to write value to
	 * @param nDefault          Default value if not exists
	 * @return                  Error return code
	 */
	ERROR_CODE ReadSetting(const CHAR *pszSection, const CHAR *pszOption, INT *nValue, INT nDefault);

	/**
	 * Reads value for the specified option
	 * @param pszSection        Section in .INI file
	 * @param pszOption         Option to update
	 * @param nValue            Pointer to write value to
	 * @param nDefault          Default value if not exists
	 * @return                  Error return code
	 */
	ERROR_CODE ReadSetting(const CHAR *pszSection, const CHAR *pszOption, BOOL *nValue, BOOL nDefault);

	/**
	 * Loads current .INI options file
	 * @return          Error return code
	 */
	ERROR_CODE Load();

	/**
	 * Updates and Releases current option list
	 */
	VOID Release();
};

} // namespace Bagel

#endif
