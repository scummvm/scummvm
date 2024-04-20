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
#include "bagel/boflib/string_functions.h"

namespace Bagel {

#define USE_REGISTRY 1

CBofOptions::CBofOptions(const char *pszOptionFile) {
	m_szFileName[0] = '\0';
	m_pOptionList = nullptr;
	m_bDirty = false;

	if (pszOptionFile != nullptr) {
		LoadOptionFile(pszOptionFile);
	}
}

CBofOptions::~CBofOptions() {
	Assert(IsValidObject(this));

	Release();

	m_szFileName[0] = '\0';
}

ErrorCode CBofOptions::LoadOptionFile(const char *pszOptionFile) {
	Assert(IsValidObject(this));

	Assert(pszOptionFile != nullptr);
	Assert(*pszOptionFile != '\0');
	Assert(strlen(pszOptionFile) < MAX_FNAME);

	Release();

	Common::strcpy_s(m_szFileName, pszOptionFile);

	return Load();
}

ErrorCode CBofOptions::Load() {
	Assert(IsValidObject(this));

	// Assume no error
	 ErrorCode errCode = ERR_NONE;

	// Free any previous option info
	Release();

	Common::File f;
	if (Common::File::exists(m_szFileName) && f.open(m_szFileName)) {
		char szBuf[MAX_OPTION_LEN];

		Assert(m_pOptionList == nullptr);

		while (ReadLine(&f, szBuf)) {
			COption *pNewOption = new COption(szBuf);
			if (pNewOption != nullptr) {
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

void CBofOptions::Release() {
	Assert(IsValidObject(this));

	Commit();

	// Release each item in the list
	while (m_pOptionList != nullptr) {
		COption *pNextItem = (COption *)m_pOptionList->GetNext();

		delete m_pOptionList;
		m_pOptionList = pNextItem;
	}
}

ErrorCode CBofOptions::Commit() {
	Assert(IsValidObject(this));
	ErrorCode errCode = ERR_NONE;

	if ((m_pOptionList != nullptr) && m_bDirty) {
		// m_pOptionList must always be the head of the list!
		Assert(m_pOptionList == m_pOptionList->GetHead());

		warning("TODO: Look into refactoring options to ConfMan if needed");
	}

	return errCode;
}

ErrorCode CBofOptions::WriteSetting(const char *pszSection, const char *pszVar, const char *pszNewValue) {
	// Can't access nullptr pointers
	Assert(pszSection != nullptr);
	Assert(pszVar != nullptr);
	Assert(pszNewValue != nullptr);

	char szValueBuf[MAX_OPTION_LEN];

	// Assume no error
	ErrorCode errCode = ERR_NONE;

	// Indicate that the options file needs to be updated
	m_bDirty = true;

	Common::sprintf_s(szValueBuf, "%s=%s", pszVar, pszNewValue);

	// Find this option based on it's section
	COption *pOption = FindOption(pszSection, pszVar);
	if (pOption != nullptr) {
		// Update option with new value
		Common::strcpy_s(pOption->m_szBuf, szValueBuf);

	} else {
		// Did not find option (or possibly also did not find section)
		char szSectionBuf[MAX_OPTION_LEN];

		// If this section is not in the file
		COption *pSection = FindSection(pszSection);
		if (pSection == nullptr) {
			// Then create a new section
			Common::sprintf_s(szSectionBuf, "[%s]", pszSection);

			pSection = new COption(szSectionBuf);
			if (pSection != nullptr) {
				if (m_pOptionList != nullptr) {
					m_pOptionList->AddToTail(pSection);
				} else {
					m_pOptionList = pSection;
				}
			} else {
				errCode = ERR_MEMORY;
			}
		}

		// Add this option to the specified section
		pOption = new COption(szValueBuf);
		if (pOption != nullptr) {
			Assert(pSection != nullptr);
			pSection->Insert(pOption);
		} else {
			errCode = ERR_MEMORY;
		}
	}

	return errCode;
}

ErrorCode CBofOptions::WriteSetting(const char *pszSection, const char *pszVar, int nNewValue) {
	// Can't access nullptr pointers
	Assert(pszSection != nullptr);
	Assert(pszVar != nullptr);

	char szBuf[20];

	// Assume no error
	ErrorCode errCode = ERR_NONE;

	Common::sprintf_s(szBuf, "%d", nNewValue);
	errCode = WriteSetting(pszSection, pszVar, szBuf);

	return errCode;
}

ErrorCode CBofOptions::ReadSetting(const char *pszSection, const char *pszOption, char *pszValue, const char *pszDefault, uint32 nMaxLen) {
	// Can't acess nullptr pointers
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

	char szBuf[MAX_OPTION_LEN];

	// Assume no error
	ErrorCode errCode = ERR_NONE;

	// Assume we will need to use the default setting
	Common::strcpy_s(pszValue, nMaxLen, pszDefault);

	// Try to find this option
	COption *pOption = FindOption(pszSection, pszOption);
	if (pOption != nullptr) {
		Assert(strlen(pOption->m_szBuf) < MAX_OPTION_LEN);

		Common::strcpy_s(szBuf, pOption->m_szBuf);

		// Strip out any comments
		StrReplaceChar(szBuf, ';', '\0');

		// Find 1st equal sign
		char *p = strchr(szBuf, '=');

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

ErrorCode CBofOptions::ReadSetting(const char *pszSection, const char *pszOption, int *pValue, int nDefault) {
	Assert(pszSection != nullptr);
	Assert(pszOption != nullptr);
	Assert(pValue != nullptr);

	// If ConfMan has a key of a given name, no matter the section,
	// than it takes precedence over any INI file value
	if (ConfMan.hasKey(pszOption)) {
		*pValue = ConfMan.getInt(pszOption);
		return ERR_NONE;
	}

	char szDefault[20], szBuf[20];

	Common::sprintf_s(szDefault, "%d", nDefault);
	ErrorCode errCode = ReadSetting(pszSection, pszOption, szBuf, szDefault, 20);

	if (pValue != nullptr)
		*pValue = atoi(szBuf);

	return errCode;
}

ErrorCode CBofOptions::ReadSetting(const char *pszSection, const char *pszOption, bool *nValue, bool nDefault) {
	Assert(pszSection != nullptr);
	Assert(pszOption != nullptr);
	Assert(nValue != nullptr);

	// If ConfMan has a key of a given name, no matter the section,
	// than it takes precedence over any INI file value
	if (ConfMan.hasKey(pszOption)) {
		*nValue = ConfMan.getBool(pszOption);
		return ERR_NONE;
	}

	int v;
	ErrorCode errCode = ReadSetting(pszSection, pszOption, &v, nDefault);
	*nValue = v != 0;
	return errCode;
}

COption *CBofOptions::FindSection(const char *pszSection) {
	Assert(IsValidObject(this));
	Assert(pszSection != nullptr);
	Assert(*pszSection != '\0');

	char szSectionBuf[MAX_OPTION_LEN];

	Common::sprintf_s(szSectionBuf, "[%s]", pszSection);
	int nLength = strlen(szSectionBuf);

	COption *pOption = m_pOptionList;
	while (pOption != nullptr) {
		if (!scumm_strnicmp(pOption->m_szBuf, szSectionBuf, nLength)) {
			break;
		}

		pOption = (COption *)pOption->GetNext();
	}

	return pOption;
}

COption *CBofOptions::FindOption(const char *pszSection, const char *pszVar) {
	Assert(IsValidObject(this));
	Assert(pszSection != nullptr);
	Assert(pszVar != nullptr);
	Assert(*pszSection != '\0');
	Assert(*pszVar != '\0');

	COption *pOption;

	// Assume we won't find the option
	COption *pFound = nullptr;

	int nLength = strlen(pszVar);
	COption *pStart;

	if ((pStart = FindSection(pszSection)) != nullptr) {

		pOption = (COption *)pStart->GetNext();
		while (pOption != nullptr) {
			if (pOption->m_szBuf[0] == '[') {
				// this option was not found
				pFound = nullptr;
				break;
			}

			if (!scumm_strnicmp(pOption->m_szBuf, pszVar, nLength)) {
				pFound = pOption;
				break;
			}

			pOption = (COption *)pOption->GetNext();
		}
	}

	return pFound;
}

bool CBofOptions::ReadLine(Common::SeekableReadStream *pFile, char *pszBuf) {
	Assert(pFile != nullptr);
	Assert(pszBuf != nullptr);

	if (pFile->eos())
		return false;

	Common::String line = pFile->readLine();
	Common::strcpy_s(pszBuf, 256, line.c_str());

	return true;
}

} // namespace Bagel
