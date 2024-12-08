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

#ifndef COMMON_TRANSLATION_H
#define COMMON_TRANSLATION_H

#include "common/array.h"
#include "common/fs.h"
#include "common/str.h"
#include "common/singleton.h"
#include "common/str-array.h"
#include "common/language.h"

#ifdef USE_TRANSLATION

namespace Common {

/**
 * @defgroup common_translation Message translation manager
 * @ingroup common
 *
 * @brief API related to translation.
 *
 * @{
 */

class File;

/**
 * Translation IDs.
 */
enum TranslationIDs {
	kTranslationAutodetectId = 0, /*!< ID for the default language (the current system language). */
	kTranslationBuiltinId = 1000  /*!< ID for the English language. */
};

/**
 * Structure describing a translation language.
 */
struct TLanguage {
	U32String name; /*!< Language name string. */
	int id;         /*!< Language ID. */

	TLanguage() : id(0) {}

	/**
	 * Construct a new language with name @p n and ID @p i.
	 */
	TLanguage(const U32String &n, int i) : name(n), id(i) {}
};

bool operator<(const TLanguage &l, const TLanguage &r);

typedef Array<TLanguage> TLangArray;

/**
 * Structure describing a translated message.
 */
struct PoMessageEntry {
	int msgid;         /*!< ID of the message. */
	String msgctxt;    /*!< Context of the message. It can be empty.
							Can be used to solve ambiguities. */
	String msgstr;     /*!< Message string. */
};

/**
 * Message translation manager.
 */
class TranslationManager : public NonCopyable {
public:
	/**
	 * Constructor that sets the current language to the default language.
	 *
	 * The default language is the detected system language.
	 */
	TranslationManager(const Common::String &fileName);
	~TranslationManager();

	/**
	 * Retrieve the language string from the given ID.
	 *
	 * @param id ID of the language.
	 *
	 * @return Matching string description of the language.
	 */
	String getLangById(int id) const;

	/**
	 * Set the current translation language to the one specified in the
	 * parameter.
	 *
	 * If the parameter is an empty string, it sets the translation language
	 * to the default system language.
	 *
	 * @param lang Language to set up.
	 */
	void setLanguage(const String &lang);

	/**
	 * Set the current translation language to the one specified by the
	 * @p id parameter.
	 *
	 * @param id ID of the language.
	 */
	void setLanguage(int id) {
		setLanguage(getLangById(id));
	}

	/**
	 * Get the ID for the given language string.
	 *
	 * @param lang Language string.
	 *
	 * @return ID of the language or kTranslationBuiltinId in case the
	 *         language could not be found.
	 */
	int parseLanguage(const String &lang) const;

	/**
	 * Return the translation of @p message into the current language.
	 *
	 * In case the message is not found in the translation catalog,
	 * return the original untranslated message, as a U32String.
	 */
	U32String getTranslation(const char *message) const;

	/**
	 * Return the translation of @p message into the current language.
	 *
	 * In case the message is not found in the translation catalog,
	 * return the original untranslated message, as a U32String.
	 */
	U32String getTranslation(const String &message) const;

	/**
	 * Return the translation of @p message into the current language.
	 *
	 * In case the message is not found in the translation catalog,
	 * return the original untranslated message, as a U32String.
	 *
	 * If a translation is found for the given context, return that
	 * translation. Otherwise, look for a translation for the same
	 * message without a context or with a different context.
	 */
	U32String getTranslation(const char *message, const char *context) const;

	/**
	 * Return the translation of @p message into the current language.
	 *
	 * In case the message is not found in the translation catalog,
	 * return the original untranslated message, as a U32String.
	 *
	 * If a translation is found for the given context, return that
	 * translation. Otherwise, look for a translation for the same
	 * message without a context or with a different context.
	 */
	U32String getTranslation(const String &message, const String &context) const;

	/**
	 * Return a list of supported languages.
	 *
	 * @return The list of supported languages in a user-readable format.
	 */
	const TLangArray getSupportedLanguageNames() const;

	/**
	 * Return the currently selected translation language.
	 */
	String getCurrentLanguage() const;

	/**
	 * Return the id of the selected translation language.
	 */
	int getCurrentLanguageId() const;

	/**
	 * Return true if the current language is the builtin language and false if it is another language.
	 */
	bool currentIsBuiltinLanguage() const;

private:
	/**
	 * Attempt to find the given language or a derivate of it.
	 *
	 * @param lang Language string.
	 *
	 * @return ID of the language or -1 in case no matching language could
	 *         be found.
	 */
	int32 findMatchingLanguage(const String &lang);

	/**
	 * Find the translations.dat file.
	 *
	 * First, search using the SearchMan and then, if needed, using the Themepath.
	 * If found, open the given @p File to read the translations.dat file.
	 */
	bool openTranslationsFile(File &);

	/**
	 * Find the translations.dat file in the given directory node.
	 *
	 * If found, open the given @p File to read the translations.dat file.
	 */
	bool openTranslationsFile(const FSNode &node, File &, int depth = -1);

	/**
	 * Load the list of languages from the translations.dat file.
	 */
	void loadTranslationsInfoDat(const Common::String &name);

	/**
	 * Load the translation for the given language from the translations.dat file.
	 *
	 * @param index Index of the language in the list of languages.
	 */
	void loadLanguageDat(int index);

	/**
	 * Check the header of the given file to make sure it is a valid translations data file.
	 */
	bool checkHeader(File &in);

	StringArray _langs;
	StringArray _langNames;

	StringArray _messageIds;
	Array<PoMessageEntry> _currentTranslationMessages;
	int _currentLang;
	Common::String _translationsFileName;
};

class MainTranslationManager : public TranslationManager, public Singleton<MainTranslationManager> {
public:
	MainTranslationManager() : TranslationManager("translations.dat") {}
	~MainTranslationManager() {}
};

/** @} */

} // End of namespace Common

#define TransMan Common::MainTranslationManager::instance()

#define _(str) TransMan.getTranslation(str)
#define _c(str, context) TransMan.getTranslation(str, context)

#else // !USE_TRANSLATION

#define _(str) Common::U32String(str)
#define _c(str, context) Common::U32String(str)

#endif // USE_TRANSLATION

#define _s(str) str
#define _sc(str, ctxt) str
#define DECLARE_TRANSLATION_ADDITIONAL_CONTEXT(str, ctxt)

#endif // COMMON_TRANSLATION_H
