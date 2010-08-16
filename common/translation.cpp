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

#include "translation.h"

DECLARE_SINGLETON(Common::TranslationManager)

#ifdef USE_DETECTLANG
#ifndef WIN32
#include <locale.h>
#endif // !WIN32
#endif

#ifdef USE_TRANSLATION
#include "messages.cpp"
#endif

namespace Common {


#ifdef USE_TRANSLATION

// Translation enabled

TranslationManager::TranslationManager() {
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
	if (*lang == '\0')
		po2c_setlang(_syslang.c_str());
	else
		po2c_setlang(lang);
}

const char *TranslationManager::getTranslation(const char *message) {
	return po2c_gettext(message);
}

const char *TranslationManager::getCurrentCharset() {
	return po2c_getcharset();
}

String TranslationManager::getTranslation(const String &message) {
	return po2c_gettext(message.c_str());
}

const TLangArray TranslationManager::getSupportedLanguageNames() const {
	TLangArray languages;

	int total = po2c_getnumlangs();
	for (int i = 0; i < total; i++) {
		TLanguage lng(po2c_getlangname(i), i + 1);
		languages.push_back(lng);
	}

	//sort(languages.begin(), languages.end());

	return languages;
}

int TranslationManager::parseLanguage(const String lang) {
	int total = po2c_getnumlangs();

	for (int i = 0; i < total; i++) {
		if (lang == po2c_getlang(i))
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
		if (id >= 0 && id - 1 < po2c_getnumlangs())
			return po2c_getlang(id - 1);
	}

	// In case an invalid ID was specified, we will output a warning
	// and return the same value as the auto detection id.
	warning("Invalid language id %d passed to TranslationManager::getLangById", id);
	return "";
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

}	// End of namespace Common
