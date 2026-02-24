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

#include "common/str.h"
#include "common/file.h"

#ifndef COMMON_FORMATS_PO_PARSER_H
#define COMMON_FORMATS_PO_PARSER_H

namespace Common {

/**
 * List of english messages.
 */
class PlainPoMessageList {
public:
	PlainPoMessageList();
	~PlainPoMessageList();

	void insert(const char *msg);
	int findIndex(const char *msg);

	int size() const;
	const char *operator[](int) const;

private:
	char **_messages;
	int _size;
	int _allocated;
};

/**
 * Describes a translation entry.
 */
struct PlainPoMessageEntry {
	char *msgstr;
	char *msgid;
	char *msgctxt;

	PlainPoMessageEntry(const char *translation, const char *message, const char *context = NULL) : msgstr(NULL), msgid(NULL), msgctxt(NULL) {
		if (translation != NULL && *translation != '\0') {
			size_t len = 1 + strlen(translation);
			msgstr = new char[len];
			Common::strlcpy(msgstr, translation, len);
		}
		if (message != NULL && *message != '\0') {
			size_t len = 1 + strlen(translation);
			msgid = new char[len];
			Common::strlcpy(msgid, message, len);
		}
		if (context != NULL && *context != '\0') {
			size_t len = 1 + strlen(translation);
			msgctxt = new char[len];
			Common::strlcpy(msgctxt, context, len);
		}
	}
	~PlainPoMessageEntry() {
		delete[] msgstr;
		delete[] msgid;
		delete[] msgctxt;
	}
};
/**
 * List of translation entries for one language.
 */
class PlainPoMessageEntryList {
public:
	PlainPoMessageEntryList(const char *language);
	~PlainPoMessageEntryList();

	void addMessageEntry(const char *translation, const char *message, const char *context = NULL);

	const char *language() const;
	const char *languageName() const;
	bool useUTF8() const;

	int size() const;
	const PlainPoMessageEntry *entry(int) const;

private:
	char *_lang;
	char *_langName;
	char *_langNameAlt;

	bool _useUTF8;

	PlainPoMessageEntry **_list;
	int _size;
	int _allocated;
};

PlainPoMessageEntryList *parsePoFile(const char *file, PlainPoMessageList &);
char *stripLine(char *);
char *parseLine(const char *line, const char *field);

} // End of namespace Common

#endif // COMMON_FORMATS_PO_PARSER_H
