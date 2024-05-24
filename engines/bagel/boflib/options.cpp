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
#include "bagel/boflib/misc.h"
#include "bagel/boflib/string_functions.h"
#include "bagel/boflib/log.h"

namespace Bagel {

COption::COption(const char *pszInit) {
	_szBuf[0] = '\0';

	if (pszInit != nullptr) {
		assert(strlen(pszInit) < MAX_OPTION_LEN);
		Common::strcpy_s(_szBuf, pszInit);
	}
}

CBofOptions::CBofOptions(const char *pszOptionFile) {
	_szFileName[0] = '\0';
	_pOptionList = nullptr;
	_bDirty = false;

	if (pszOptionFile != nullptr) {
		loadOptionFile(pszOptionFile);
	}
}

CBofOptions::~CBofOptions() {
	assert(isValidObject(this));

	release();

	_szFileName[0] = '\0';
}

ErrorCode CBofOptions::loadOptionFile(const char *pszOptionFile) {
	assert(isValidObject(this));
	assert(pszOptionFile != nullptr);
	assert(*pszOptionFile != '\0');
	assert(strlen(pszOptionFile) < MAX_FNAME);

	release();

	Common::strcpy_s(_szFileName, pszOptionFile);

	return load();
}

ErrorCode CBofOptions::load() {
	assert(isValidObject(this));

	// Assume no error
	ErrorCode errorCode = ERR_NONE;

	// Free any previous option info
	release();

	Common::File f;
	if (Common::File::exists(_szFileName) && f.open(_szFileName)) {
		char szBuf[MAX_OPTION_LEN];

		assert(_pOptionList == nullptr);

		while (readLine(&f, szBuf)) {
			COption *pNewOption = new COption(szBuf);
			if (_pOptionList != nullptr) {
				_pOptionList->addToTail(pNewOption);
			} else {
				_pOptionList = pNewOption;
			}
		}

		if (_pOptionList != nullptr) {
			// _pOptionList must always be the head of the list!
			assert(_pOptionList == _pOptionList->getHead());
		}

		f.close();

	} else {
		errorCode = ERR_FOPEN;
	}

	return errorCode;
}

void CBofOptions::release() {
	assert(isValidObject(this));

	commit();

	// Release each item in the list
	while (_pOptionList != nullptr) {
		COption *pNextItem = (COption *)_pOptionList->getNext();

		delete _pOptionList;
		_pOptionList = pNextItem;
	}
}

ErrorCode CBofOptions::commit() {
	assert(isValidObject(this));
	ErrorCode errorCode = ERR_NONE;

	if ((_pOptionList != nullptr) && _bDirty) {
		// _pOptionList must always be the head of the list!
		assert(_pOptionList == _pOptionList->getHead());

		warning("TODO: Look into refactoring options to ConfMan if needed");
	}

	return errorCode;
}

ErrorCode CBofOptions::writeSetting(const char *pszSection, const char *pszVar, const char *pszNewValue) {
	// Can't access nullptr pointers
	assert(pszSection != nullptr);
	assert(pszVar != nullptr);
	assert(pszNewValue != nullptr);

	char szValueBuf[MAX_OPTION_LEN];

	// Assume no error
	ErrorCode errorCode = ERR_NONE;

	// Indicate that the options file needs to be updated
	_bDirty = true;

	Common::sprintf_s(szValueBuf, "%s=%s", pszVar, pszNewValue);

	// Find this option based on it's section
	COption *pOption = findOption(pszSection, pszVar);
	if (pOption != nullptr) {
		// Update option with new value
		Common::strcpy_s(pOption->_szBuf, szValueBuf);

	} else {
		// Did not find option (or possibly also did not find section)

		// If this section is not in the file
		COption *pSection = findSection(pszSection);
		if (pSection == nullptr) {
			char szSectionBuf[MAX_OPTION_LEN];
			// Then create a new section
			Common::sprintf_s(szSectionBuf, "[%s]", pszSection);

			pSection = new COption(szSectionBuf);

			if (_pOptionList != nullptr) {
				_pOptionList->addToTail(pSection);
			} else {
				_pOptionList = pSection;
			}
		}

		// Add this option to the specified section
		pOption = new COption(szValueBuf);
		pSection->Insert(pOption);
	}

	return errorCode;
}

ErrorCode CBofOptions::writeSetting(const char *pszSection, const char *pszVar, int nNewValue) {
	// Can't access nullptr pointers
	assert(pszSection != nullptr);
	assert(pszVar != nullptr);

	char szBuf[20];

	Common::sprintf_s(szBuf, "%d", nNewValue);
	ErrorCode errorCode = writeSetting(pszSection, pszVar, szBuf);

	return errorCode;
}

ErrorCode CBofOptions::readSetting(const char *section, const char *option, char *stringValue, const char *defaultValue, uint32 maxLen) {
	// Can't access nullptr pointers
	assert(section != nullptr);
	assert(option != nullptr);
	assert(stringValue != nullptr);
	assert(defaultValue != nullptr);

	// If ConfMan has a key of a given name, no matter the section,
	// than it takes precedence over any INI file value
	if (ConfMan.hasKey(option)) {
		Common::String str = ConfMan.get(option);
		Common::strcpy_s(stringValue, maxLen, str.c_str());
		return ERR_NONE;
	}

	char szBuf[MAX_OPTION_LEN];

	// Assume no error
	ErrorCode errorCode = ERR_NONE;

	// Assume we will need to use the default setting
	Common::strcpy_s(stringValue, maxLen, defaultValue);

	// Try to find this option
	COption *pOption = findOption(section, option);
	if (pOption != nullptr) {
		assert(strlen(pOption->_szBuf) < MAX_OPTION_LEN);

		Common::strcpy_s(szBuf, pOption->_szBuf);

		// Strip out any comments
		strreplaceChar(szBuf, ';', '\0');

		// Find 1st equal sign
		char *p = strchr(szBuf, '=');

		// Error in .INI file if we can't find the equal sign
		if (p != nullptr) {
			p++;

			if (strlen(p) > 0)
				Common::strcpy_s(stringValue, maxLen, p);

		} else {
			logError(buildString("Error in %s, section: %s, entry: %s", _szFileName, section, option));
			errorCode = ERR_FTYPE;
		}
	}

	return errorCode;
}

ErrorCode CBofOptions::readSetting(const char *section, const char *option, int *intValue, int defaultValue) {
	assert(section != nullptr);
	assert(option != nullptr);
	assert(intValue != nullptr);

	// If ConfMan has a key of a given name, no matter the section,
	// than it takes precedence over any INI file value
	if (ConfMan.hasKey(option)) {
		*intValue = ConfMan.getInt(option);
		return ERR_NONE;
	}

	char szDefault[20], szBuf[20];

	Common::sprintf_s(szDefault, "%d", defaultValue);
	ErrorCode errorCode = readSetting(section, option, szBuf, szDefault, 20);
	*intValue = atoi(szBuf);

	return errorCode;
}

ErrorCode CBofOptions::readSetting(const char *section, const char *option, bool *boolValue, bool defaultValue) {
	assert(section != nullptr);
	assert(option != nullptr);
	assert(boolValue != nullptr);

	// If ConfMan has a key of a given name, no matter the section,
	// than it takes precedence over any INI file value
	if (ConfMan.hasKey(option)) {
		*boolValue = ConfMan.getBool(option);
		return ERR_NONE;
	}

	int v;
	ErrorCode errorCode = readSetting(section, option, &v, defaultValue);
	*boolValue = v != 0;
	return errorCode;
}

COption *CBofOptions::findSection(const char *pszSection) {
	assert(isValidObject(this));
	assert(pszSection != nullptr);
	assert(*pszSection != '\0');

	char szSectionBuf[MAX_OPTION_LEN];

	Common::sprintf_s(szSectionBuf, "[%s]", pszSection);
	int nLength = strlen(szSectionBuf);

	COption *pOption = _pOptionList;
	while (pOption != nullptr) {
		if (!scumm_strnicmp(pOption->_szBuf, szSectionBuf, nLength)) {
			break;
		}

		pOption = (COption *)pOption->getNext();
	}

	return pOption;
}

COption *CBofOptions::findOption(const char *pszSection, const char *pszVar) {
	assert(isValidObject(this));
	assert(pszSection != nullptr);
	assert(pszVar != nullptr);
	assert(*pszSection != '\0');
	assert(*pszVar != '\0');

	// Assume we won't find the option
	COption *pFound = nullptr;

	int nLength = strlen(pszVar);
	COption *pStart = findSection(pszSection);

	if (pStart != nullptr) {
		COption *pOption = (COption *)pStart->getNext();
		while (pOption != nullptr) {
			if (pOption->_szBuf[0] == '[') {
				// this option was not found
				pFound = nullptr;
				break;
			}

			if (!scumm_strnicmp(pOption->_szBuf, pszVar, nLength)) {
				pFound = pOption;
				break;
			}

			pOption = (COption *)pOption->getNext();
		}
	}

	return pFound;
}

bool CBofOptions::readLine(Common::SeekableReadStream *pFile, char *pszBuf) {
	assert(pFile != nullptr);
	assert(pszBuf != nullptr);

	if (pFile->eos())
		return false;

	Common::String line = pFile->readLine();
	Common::strcpy_s(pszBuf, 256, line.c_str());

	return true;
}

} // namespace Bagel
