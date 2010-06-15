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

#include "translation.h"

DECLARE_SINGLETON(Common::TranslationManager)

#ifdef DETECTLANG
#include <locale.h>
#endif

#ifdef TERMCONV
#include <langinfo.h>
#endif

#ifdef TRANSLATION
#include "messages.cpp"
#endif

namespace Common {


#ifdef TRANSLATION

// Translation enabled


TranslationManager::TranslationManager() {
#ifdef DETECTLANG
	// Activating current locale settings
	const char *locale = setlocale(LC_ALL, "");

	// Detect the language from the locale
	if (!locale) {
		strcpy(_syslang, "C");
	} else {
		int len = strlen(locale);
		if (len > 5)
			len = 5;
		strncpy(_syslang, locale, len);
		_syslang[len] = 0;
	}
#else // DETECTLANG
	strcpy(_syslang, "C");
#endif // DETECTLANG

#ifdef TERMCONV
	_convmsg = NULL;
	_conversion = NULL;
#endif // TERMCONV

	// Set the default language
	setLanguage("");
}

TranslationManager::~TranslationManager() {
#ifdef TERMCONV
	iconv_close(_conversion);
	if (_convmsg)
		delete [] _convmsg;
#endif // TERMCONV
}

void TranslationManager::setLanguage(const char *lang) {
	if (*lang == '\0')
		po2c_setlang(_syslang);
	else
		po2c_setlang(lang);

#ifdef TERMCONV
	// Get the locale character set (for terminal output)
	const char *charset_term = nl_langinfo(CODESET);

	// Get the messages character set
	const char *charset_po = po2c_getcharset();

	// Delete previous conversion
	if (_conversion)
		iconv_close(_conversion);

	// Initialize the conversion
	_conversion = iconv_open(charset_term, charset_po);
#endif // TERMCONV
}

const char *TranslationManager::getTranslation(const char *message) {
	return po2c_gettext(message);
}

String TranslationManager::getTranslation(const String &message) {
	return po2c_gettext(message.c_str());
}

#ifdef TERMCONV
bool TranslationManager::convert(const char *message) {
	// Preparing conversion origin
	size_t len = strlen(message);
	char *msgcpy = new char[len + 1];
	strcpy(msgcpy, message);
	char *msg = msgcpy;
	char **pmsg = &msg;

	// Preparing conversion destination
	size_t len2 = _sizeconv;
	char *conv = _convmsg;
	char **pconv = &conv;

	// Clean previous conversions
	iconv(_conversion, NULL, NULL, pconv, &len2);

	// Do the real conversion
	size_t result = iconv(_conversion, pmsg, &len, pconv, &len2);

	delete [] msgcpy;

	return result != ((size_t)-1);
}
#endif // TERMCONV

const char *TranslationManager::convertTerm(const char *message) {
#ifdef TERMCONV
	size_t len = strlen(message);
	if (!_convmsg) {
		_sizeconv = len * 2;
		_convmsg = new char[_sizeconv];
	}

	if (!convert(message)) {
		// Resizing the buffer
		delete [] _convmsg;
		_sizeconv = len * 2;
		_convmsg = new char[_sizeconv];

		if (!convert(message)) {
			printf("Error while converting character sets\n");
			return "Error while converting character sets";
		}
	}

	return _convmsg;
#else // TERMCONV
	return message;
#endif // TERMCONV
}

const TLangArray TranslationManager::getSupportedLanguages() const {
	TLangArray languages;

	int total = po2c_getnumlangs();
	for (int i = 0; i < total; i++) {
		TLanguage lng(po2c_getlang(i), i + 1);
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
		return po2c_getlang(id - 1);
	}
}

#else // TRANSLATION

// Translation disabled


TranslationManager::TranslationManager() {}

TranslationManager::~TranslationManager() {}

void TranslationManager::setLanguage(const char *lang) {}

const char *TranslationManager::getTranslation(const char *message) {
	return message;
}

const char *TranslationManager::convertTerm(const char *message) {
	return message;
}

#endif // TRANSLATION

}	// End of namespace Common
