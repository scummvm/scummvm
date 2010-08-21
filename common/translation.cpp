/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 */

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
// winnt.h defines ARRAYSIZE, but we want our own one... - this is needed before including util.h
#undef ARRAYSIZE
#endif

#define TRANSLATIONS_DAT_VER 1

#include "translation.h"

DECLARE_SINGLETON(Common::TranslationManager)

#ifdef USE_DETECTLANG
#ifndef WIN32
#include <locale.h>
#endif // !WIN32
#endif

namespace Common {

bool operator<(const TLanguage &l, const TLanguage &r) {
	return strcmp(l.name, r.name) < 0;
}

#ifdef USE_TRANSLATION

// Translation enabled

TranslationManager::TranslationManager() : _currentLang(-1) {
	loadTranslationsInfoDat();

#ifdef USE_DETECTLANG
#ifdef WIN32
	// We can not use "setlocale" (at least not for MSVC builds), since it
	// will return locales like: "English_USA.1252", thus we need a special
	// way to determine the locale string for Win32.
	char langName[9];
	char ctryName[9];

	const LCID languageIdentifier = GetThreadLocale();

	// GetLocalInfo is only supported starting from Windows 2000, according to this:
	// http://msdn.microsoft.com/en-us/library/dd318101%28VS.85%29.aspx
	// On the other hand the locale constants used, seem to exist on Windows 98 too,
	// check this for that: http://msdn.microsoft.com/en-us/library/dd464799%28v=VS.85%29.aspx
	//
	// I am not exactly sure what is the truth now, it might be very well that this breaks
	// support for systems older than Windows 2000....
	//
	// TODO: Check whether this (or ScummVM at all ;-) works on a system with Windows 98 for
	// example and if it does not and we still want Windows 9x support, we should definitly
	// think of another solution.
	if (GetLocaleInfo(languageIdentifier, LOCALE_SISO639LANGNAME, langName, sizeof(langName)) != 0 &&
		GetLocaleInfo(languageIdentifier, LOCALE_SISO3166CTRYNAME, ctryName, sizeof(ctryName)) != 0) {
		_syslang = langName;
		_syslang += "_";
		_syslang += ctryName;
	} else {
		_syslang = "C";
	}
#else // WIN32
	// Activating current locale settings
	const char *locale = setlocale(LC_ALL, "");

	// Detect the language from the locale
	if (!locale) {
		_syslang = "C";
	} else {
		int length = 0;

		// Strip out additional information, like
		// ".UTF-8" or the like. We do this, since
		// our translation languages are usually
		// specified without any charset information.
		for (int i = 0; locale[i]; ++i) {
			// TODO: Check whether "@" should really be checked
			// here.
			if (locale[i] == '.' || locale[i] == ' ' || locale[i] == '@') {
				length = i;
				break;
			}

			length = i;
		}

		_syslang = String(locale, length);
	}
#endif // WIN32
#else // USE_DETECTLANG
	_syslang = "C";
#endif // USE_DETECTLANG

	// Set the default language
	setLanguage("");
}

TranslationManager::~TranslationManager() {
}

void TranslationManager::setLanguage(const char *lang) {
	// Get lang index
	int langIndex = -1;
	String langStr(lang);
	if (langStr.empty())
		langStr = _syslang;

	// Searching for a valid language
	for (unsigned int i = 0; i < _langs.size() && langIndex == -1; ++i) {
		if (langStr == _langs[i])
			langIndex = i;
	}

	// Try partial match
	for (unsigned int i = 0; i < _langs.size() && langIndex == -1; ++i) {
		if (strncmp(langStr.c_str(), _langs[i].c_str(), 2) == 0)
			langIndex = i;
	}

	// Load messages for that lang
	// Call it even if the index is -1 to unload previously loaded translations
	if (langIndex != _currentLang) {
		loadLanguageDat(langIndex);
		_currentLang = langIndex;
	}
}

const char *TranslationManager::getTranslation(const char *message) {
	// if no language is set or message is empty, return msgid as is
	if (_currentTranslationMessages.empty() || *message == '\0')
		return message;

	// binary-search for the msgid
	int leftIndex = 0;
	int rightIndex = _currentTranslationMessages.size() - 1;

	while (rightIndex >= leftIndex) {
		const int midIndex = (leftIndex + rightIndex) / 2;
		const PoMessageEntry * const m = &_currentTranslationMessages[midIndex];

		const int compareResult = strcmp(message, _messageIds[m->msgid].c_str());

		if (compareResult == 0)
			return m->msgstr.c_str();
		else if (compareResult < 0)
			rightIndex = midIndex - 1;
		else
			leftIndex = midIndex + 1;
	}

	return message;
}

const char *TranslationManager::getCurrentCharset() {
	if (_currentCharset.empty())
		return "ASCII";
	return _currentCharset.c_str();
}

String TranslationManager::getTranslation(const String &message) {
	return getTranslation(message.c_str());
}

const TLangArray TranslationManager::getSupportedLanguageNames() const {
	TLangArray languages;

	for (unsigned int i = 0; i < _langNames.size(); i++) {
		TLanguage lng(_langNames[i].c_str(), i + 1);
		languages.push_back(lng);
	}

	sort(languages.begin(), languages.end());

	return languages;
}

int TranslationManager::parseLanguage(const String lang) {
	for (unsigned int i = 0; i < _langs.size(); i++) {
		if (lang == _langs[i])
			return i + 1;
	}

	return kTranslationBuiltinId;
}

const char *TranslationManager::getLangById(int id) {
	switch (id) {
	case kTranslationAutodetectId:
		return "";
	case kTranslationBuiltinId:
		return "C";
	default:
		if (id >= 0 && id - 1 < (int)_langs.size())
			return _langs[id - 1].c_str();
	}

	// In case an invalid ID was specified, we will output a warning
	// and return the same value as the auto detection id.
	warning("Invalid language id %d passed to TranslationManager::getLangById", id);
	return "";
}

void TranslationManager::loadTranslationsInfoDat() {
	File in;
	in.open("translations.dat");

	if (!checkHeader(in))
		return;

	char buf[256];
	int len;

	// Get number of translations
	int nbTranslations = in.readUint16BE();
	
	// Skip all the block sizes
	for (int i = 0; i < nbTranslations + 2; ++i)
		in.readUint16BE();

	// Read list of languages
	_langs.resize(nbTranslations);
	_langNames.resize(nbTranslations);
	for (int i = 0; i < nbTranslations; ++i) {
		len = in.readUint16BE();
		in.read(buf, len);
		_langs[i] = String(buf, len);
		len = in.readUint16BE();
		in.read(buf, len);
		_langNames[i] = String(buf, len);
	}

	// Read messages
	int numMessages = in.readUint16BE();
	_messageIds.resize(numMessages);
	for (int i = 0; i < numMessages; ++i) {
		len = in.readUint16BE();
		in.read(buf, len);
		_messageIds[i] = String(buf, len);
	}
}

void TranslationManager::loadLanguageDat(int index) {
	_currentTranslationMessages.clear();
	_currentCharset.clear();
	// Sanity check
	if (index < 0 || index >= (int)_langs.size()) {
		if (index != -1)
			warning("Invalid language index %d passed to TranslationManager::loadLanguageDat", index);
		return;
	}

	File in;
	in.open("translations.dat");

	if (!checkHeader(in))
		return;

	char buf[1024];
	int len;

	// Get number of translations
	int nbTranslations = in.readUint16BE();
	if (nbTranslations != (int)_langs.size()) {
		warning("The 'translations.dat' file has changed since starting ScummVM. GUI translation will not be available");
		return;
	}

	// Get size of blocks to skip.
	int skipSize = 0;
	for (int i = 0; i < index + 2; ++i)
		skipSize += in.readUint16BE();
	// We also need to skip the remaining block sizes
	skipSize += 2 * (nbTranslations - index);

	// Seek to start of block we want to read
	in.seek(skipSize, SEEK_CUR);

	// Read number of translated messages
	int nbMessages = in.readUint16BE();
	_currentTranslationMessages.resize(nbMessages);

	// Read charset
	len = in.readUint16BE();
	in.read(buf, len);
	_currentCharset = String(buf, len);

	// Read messages
	for (int i = 0; i < nbMessages; ++i) {
		_currentTranslationMessages[i].msgid = in.readUint16BE();
		len = in.readUint16BE();
		in.read(buf, len);
		_currentTranslationMessages[i].msgstr = String(buf, len);
	}
}

bool TranslationManager::checkHeader(File &in) {
	char buf[13];
	int ver;

	if (!in.isOpen()) {
		warning("You re missing the 'translations.dat' file. GUI translation will not be available");
		return false;
	}

	in.read(buf, 12);
	buf[12] = '\0';

	// Check header
	if (strcmp(buf, "TRANSLATIONS")) {
		warning("File 'translations.dat' is corrupt. GUI translation will not be available");
		return false;
	}

	// Check version
	ver = in.readByte();

	if (ver != TRANSLATIONS_DAT_VER) {
		warning("File 'translations.dat' is wrong version. Expected %d but got %d. GUI translation will not be available", TRANSLATIONS_DAT_VER, ver);
		return false;
	}

	return true;
}

#else // USE_TRANSLATION

// Translation disabled


TranslationManager::TranslationManager() {}

TranslationManager::~TranslationManager() {}

void TranslationManager::setLanguage(const char *lang) {}

const char *TranslationManager::getLangById(int id) {
	return "";
}

int TranslationManager::parseLanguage(const String lang) {
	return kTranslationBuiltinId;
}

const char *TranslationManager::getTranslation(const char *message) {
	return message;
}

String TranslationManager::getTranslation(const String &message) {
	return message;
}

const TLangArray TranslationManager::getSupportedLanguageNames() const {
	return TLangArray();
}
	
const char *TranslationManager::getCurrentCharset() {
	return "ASCII";
}

#endif // USE_TRANSLATION

} // End of namespace Common

