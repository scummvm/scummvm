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

#ifndef COMMON_TRANSLATION_H
#define COMMON_TRANSLATION_H

#include "common/singleton.h"
#include "common/str-array.h"

#ifdef TERMCONV
#include <iconv.h>
#endif

namespace Common {

enum TranslationIDs {
	kTranslationAutodetectId = 0,
	kTranslationBuiltinId = 1000
};

struct TLanguage {
	const char *name;
	int id;

	TLanguage() {
		name = 0;
		id = 0;
	}

	TLanguage(const char *n, int i) {
		name = n;
		id = i;
	}
};

typedef Array<TLanguage> TLangArray;

/**
 * Message translation manager.
 */
class TranslationManager : public Singleton<TranslationManager> {
private:
	char _syslang[6];

#ifdef TERMCONV
	iconv_t _conversion;
	char *_convmsg;
	int _sizeconv;

	bool convert(const char *message);
#endif // TERMCONV

public:
	/**
	 * The constructor detects the system language and sets default
	 * language to English.
	 */
	TranslationManager();
	~TranslationManager();

	const char *getLangById(int id);

	/**
	 * Sets the current translation language to the one specified in the
	 * parameter. If the parameter is an empty string, it sets the default
	 * system language.
	 */
	void setLanguage(const char *);
	void setLanguage(int id) {
		setLanguage(getLangById(id));
	}

	int parseLanguage(const String lang);

	/**
	 * Returns the translation into the current language of the parameter
	 * message. In case the message isn't found in the translation catalog,
	 * it returns the original untranslated message.
	 */
	const char *getTranslation(const char *message);

	/**
	 * Converts the message into the terminal character set (which may be
	 * different than the GUI's "native" one.
	 */
	const char *convertTerm(const char *message);

	const TLangArray getSupportedLanguages() const;
};

}	// End of namespace Common

#define TransMan Common::TranslationManager::instance()

#ifdef TRANSLATION
#define _(str) TransMan.getTranslation(str)
#define _t(str) TransMan.convertTerm(_(str))
#else
#define _(str) str
#define _t(str) str
#endif

#define _s(str) str

#endif
