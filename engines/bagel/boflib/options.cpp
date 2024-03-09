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

#include "common/config-manager.h"
#include "common/file.h"
#include "bagel/boflib/options.h"
#include "bagel/boflib/debug.h"
#include "bagel/boflib/misc.h"

namespace Bagel {

#define USE_REGISTRY 1

CBofOptions::CBofOptions(const CHAR *pszOptionFile) {
	m_szFileName[0] = '\0';
	m_pOptionList = nullptr;
	m_bDirty = FALSE;

	if (pszOptionFile != nullptr) {
		LoadOptionFile(pszOptionFile);
	}
}

CBofOptions::~CBofOptions() {
	Assert(IsValidObject(this));

	Release();

	m_szFileName[0] = '\0';
}

ERROR_CODE CBofOptions::LoadOptionFile(const CHAR *pszOptionFile) {
	Assert(IsValidObject(this));

	Assert(pszOptionFile != nullptr);
	Assert(*pszOptionFile != '\0');
	Assert(strlen(pszOptionFile) < MAX_FNAME);

	Release();

	Common::strcpy_s(m_szFileName, pszOptionFile);

	return Load();
}

ERROR_CODE CBofOptions::Load() {
	Assert(IsValidObject(this));

	COption *pNewOption;
	ERROR_CODE errCode;

	// Assume no error
	errCode = ERR_NONE;

	// free any previous option info
	Release();

	Common::File f;
	if (Common::File::exists(m_szFileName) && f.open(m_szFileName)) {
		CHAR szBuf[MAX_OPTION_LEN];

		Assert(m_pOptionList == nullptr);

		while (ReadLine(&f, szBuf)) {
			if ((pNewOption = new COption(szBuf)) != nullptr) {
				if (m_pOptionList != nullptr) {
					m_pOptionList->AddToTail(pNewOption);
				} else {
					m_pOptionList = pNewOption;
				}

			} else {
				errCode = ERR_MEMORY;
				break;
			}
		}

		if (m_pOptionList != nullptr) {
			// m_pOptionList must always be the head of the list!
			Assert(m_pOptionList == m_pOptionList->GetHead());
		}

		f.close();

	} else {
		errCode = ERR_FOPEN;
	}

	return errCode;
}

VOID CBofOptions::Release() {
	Assert(IsValidObject(this));

	COption *pNextItem;

	Commit();

	// release each item in the list
	//
	while (m_pOptionList != nullptr) {

		pNextItem = (COption *)m_pOptionList->GetNext();

		delete m_pOptionList;

		m_pOptionList = pNextItem;
	}
}

ERROR_CODE CBofOptions::Commit() {
	Assert(IsValidObject(this));
	ERROR_CODE errCode = ERR_NONE;

	if ((m_pOptionList != nullptr) && m_bDirty) {
		// m_pOptionList must always be the head of the list!
		Assert(m_pOptionList == m_pOptionList->GetHead());

#if 0
		FILE *pFile;
		COption *pOption;

		if ((pFile = fopen(m_szFileName, "wt")) != nullptr) {

			pOption = m_pOptionList;
			while (pOption != nullptr) {

				fprintf(pFile, "%s\n", pOption->m_szBuf);

				pOption = (COption *)pOption->GetNext();
			}
			fclose(pFile);

			// options file is now up-to-date
			m_bDirty = FALSE;

		} else {
			errCode = ERR_FOPEN;
		}
#else
		error("TODO: Look into refactoring options to ConfMan if needed");
#endif
	}

	return errCode;
}

ERROR_CODE CBofOptions::WriteSetting(const CHAR *pszSection, const CHAR *pszVar, const CHAR *pszNewValue) {
	// can't acess nullptr pointers
	//
	Assert(pszSection != nullptr);
	Assert(pszVar != nullptr);
	Assert(pszNewValue != nullptr);

	CHAR szValueBuf[MAX_OPTION_LEN];
	COption *pOption;
	ERROR_CODE errCode;

	// assume no error
	errCode = ERR_NONE;

	// indicate that the options file needs to be updated
	m_bDirty = TRUE;

	Common::sprintf_s(szValueBuf, "%s=%s", pszVar, pszNewValue);

	// find this option based on it's section
	//
	if ((pOption = FindOption(pszSection, pszVar)) != nullptr) {

		// update option with new value
		Common::strcpy_s(pOption->m_szBuf, szValueBuf);

		// did not find option (or possibly also did not find section)
		//
	} else {
		CHAR szSectionBuf[MAX_OPTION_LEN];
		COption *pSection;

		// if this section is not in the file
		//
		if ((pSection = FindSection(pszSection)) == nullptr) {

			// then create a new section
			//
			Common::sprintf_s(szSectionBuf, "[%s]", pszSection);

			if ((pSection = new COption(szSectionBuf)) != nullptr) {

				if (m_pOptionList != nullptr) {

					m_pOptionList->AddToTail(pSection);

				} else {
					m_pOptionList = pSection;
				}
			} else {
				errCode = ERR_MEMORY;
			}
		}

		// add this option to the specified section
		//
		if ((pOption = new COption(szValueBuf)) != nullptr) {

			Assert(pSection != nullptr);

			pSection->Insert(pOption);

		} else {
			errCode = ERR_MEMORY;
		}
	}

	return errCode;
}

ERROR_CODE CBofOptions::WriteSetting(const CHAR *pszSection, const CHAR *pszVar, INT nNewValue) {
	// can't acess nullptr pointers
	//
	Assert(pszSection != nullptr);
	Assert(pszVar != nullptr);

	CHAR szBuf[20];
	ERROR_CODE errCode;

	// assume no error
	errCode = ERR_NONE;

	Common::sprintf_s(szBuf, "%d", nNewValue);
	errCode = WriteSetting(pszSection, pszVar, szBuf);

	return errCode;
}

ERROR_CODE CBofOptions::ReadSetting(const CHAR *pszSection, const CHAR *pszOption, CHAR *pszValue, const CHAR *pszDefault, UINT nMaxLen) {
	//  Can't acess nullptr pointers
	Assert(pszSection != nullptr);
	Assert(pszOption != nullptr);
	Assert(pszValue != nullptr);
	Assert(pszDefault != nullptr);

	// If ConfMan has a key of a given name, no matter the section,
	// than it takes precedence over any INI file value
	if (ConfMan.hasKey(pszOption)) {
		Common::String str = ConfMan.get(pszOption);
		Common::strcpy_s(pszValue, nMaxLen, str.c_str());
		return ERR_NONE;
	}

	CHAR szBuf[MAX_OPTION_LEN];
	CHAR *p;
	COption *pOption;
	ERROR_CODE errCode;

	// Assume no error
	errCode = ERR_NONE;

	// Assume we will need to use the default setting
	Common::strcpy_s(pszValue, nMaxLen, pszDefault);

	// Try to find this option
	if ((pOption = FindOption(pszSection, pszOption)) != nullptr) {
		Assert(strlen(pOption->m_szBuf) < MAX_OPTION_LEN);

		Common::strcpy_s(szBuf, pOption->m_szBuf);

		// Strip out any comments
		StrReplaceChar(szBuf, ';', '\0');

		// Find 1st equal sign
		p = strchr(szBuf, '=');

		// Error in .INI file if we can't find the equal sign
		if (p != nullptr) {
			p++;

			if (strlen(p) > 0)
				Common::strcpy_s(pszValue, nMaxLen, p);

		} else {
			LogError(BuildString("Error in %s, section: %s, entry: %s", m_szFileName, pszSection, pszOption));
			errCode = ERR_FTYPE;
		}
	}

	return errCode;
}

ERROR_CODE CBofOptions::ReadSetting(const CHAR *pszSection, const CHAR *pszOption, INT *pValue, INT nDefault) {
	Assert(pszSection != nullptr);
	Assert(pszOption != nullptr);
	Assert(pValue != nullptr);

	// If ConfMan has a key of a given name, no matter the section,
	// than it takes precedence over any INI file value
	if (ConfMan.hasKey(pszOption)) {
		*pValue = ConfMan.getInt(pszOption);
		return ERR_NONE;
	}

	CHAR szDefault[20], szBuf[20];
	ERROR_CODE errCode;

	Common::sprintf_s(szDefault, "%d", nDefault);

	errCode = ReadSetting(pszSection, pszOption, szBuf, szDefault, 20);

	if (pValue != nullptr)
		*pValue = atoi(szBuf);

	return errCode;
}

ERROR_CODE CBofOptions::ReadSetting(const CHAR *pszSection, const CHAR *pszOption, BOOL *nValue, BOOL nDefault) {
	Assert(pszSection != nullptr);
	Assert(pszOption != nullptr);
	Assert(nValue != nullptr);

	// If ConfMan has a key of a given name, no matter the section,
	// than it takes precedence over any INI file value
	if (ConfMan.hasKey(pszOption)) {
		*nValue = ConfMan.getBool(pszOption);
		return ERR_NONE;
	}

	INT v;
	ERROR_CODE errCode = ReadSetting(pszSection, pszOption, &v, nDefault);
	*nValue = v != 0;
	return errCode;
}

COption *CBofOptions::FindSection(const CHAR *pszSection) {
	Assert(IsValidObject(this));
	Assert(pszSection != nullptr);
	Assert(*pszSection != '\0');

	CHAR szSectionBuf[MAX_OPTION_LEN];
	COption *pOption;
	INT nLength;

	Common::sprintf_s(szSectionBuf, "[%s]", pszSection);

	nLength = strlen(szSectionBuf);

	pOption = m_pOptionList;
	while (pOption != nullptr) {

		if (!scumm_strnicmp(pOption->m_szBuf, szSectionBuf, nLength)) {
			break;
		}

		pOption = (COption *)pOption->GetNext();
	}

	return pOption;
}

COption *CBofOptions::FindOption(const CHAR *pszSection, const CHAR *pszVar) {
	Assert(IsValidObject(this));
	Assert(pszSection != nullptr);
	Assert(pszVar != nullptr);
	Assert(*pszSection != '\0');
	Assert(*pszVar != '\0');

	COption *pOption, *pFound;
	INT nLength;

	// Assume we won't find the option
	pFound = nullptr;

	nLength = strlen(pszVar);

#if BOF_WINDOWS && USE_REGISTRY

	pOption = m_pOptionList;
	while (pOption != nullptr) {

		if (!scumm_strnicmp(pOption->m_szBuf, pszVar, nLength)) {

			pFound = pOption;
			break;
		}

		pOption = (COption *)pOption->GetNext();
	}
#else

	COption *pStart;

	if ((pStart = FindSection(pszSection)) != nullptr) {

		pOption = (COption *)pStart->GetNext();
		while (pOption != nullptr) {

			if (pOption->m_szBuf[0] == '[') {

				// this option was not found
				pFound = nullptr;
				break;

			} else if (!scumm_strnicmp(pOption->m_szBuf, pszVar, nLength)) {

				pFound = pOption;
				break;
			}

			pOption = (COption *)pOption->GetNext();
		}
	}

#endif

	return pFound;
}

BOOL CBofOptions::ReadLine(Common::SeekableReadStream *pFile, CHAR *pszBuf) {
	Assert(pFile != nullptr);
	Assert(pszBuf != nullptr);

	if (pFile->eos())
		return false;

	Common::String line = pFile->readLine();
	Common::strcpy_s(pszBuf, 256, line.c_str());

	return TRUE;
}

} // namespace Bagel
