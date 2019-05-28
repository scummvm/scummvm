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
 * This is a utility for generating a data file for the supernova engine.
 * It contains strings extracted from the original executable as well
 * as translations and is required for the engine to work properly.
 */

#ifndef PO_PARSER_H
#define PO_PARSER_H

struct PoMessage {
	char *msgstr;
	char *msgid;
	char *msgctxt;

	PoMessage(const char *translation, const char *message, const char *context = NULL) :
	msgstr(NULL), msgid(NULL), msgctxt(NULL)
	{
		if (translation != NULL && *translation != '\0') {
			msgstr = new char[1 + strlen(translation)];
			strcpy(msgstr, translation);
		}
		if (message != NULL && *message != '\0') {
			msgid = new char[1 + strlen(message)];
			strcpy(msgid, message);
		}
		if (context != NULL && *context != '\0') {
			msgctxt = new char[1 + strlen(context)];
			strcpy(msgctxt, context);
		}
	}
	~PoMessage() {
		delete[] msgstr;
		delete[] msgid;
		delete[] msgctxt;
	}
};

class PoMessageList {
public:
	PoMessageList();
	~PoMessageList();

	void insert(const char *translation, const char *msg, const char *context = NULL);
	const char *findTranslation(const char *msg, const char *context = NULL);

private:
	int compareString(const char *left, const char *right);
	int compareMessage(const char *msgLeft, const char *contextLeft, const char *msgRight, const char *contextRight);

	PoMessage **_list;
	int _size;
	int _allocated;
};

PoMessageList *parsePoFile(const char *file);
char *stripLine(char *);
char *parseLine(const char *line, const char *field);

#endif /* PO_PARSER_H */
