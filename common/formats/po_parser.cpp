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
 * This is a utility for create the translations.dat file from all the po files.
 * The generated files is used by ScummVM to propose translation of its GUI.
 */

#include "common/formats/po_parser.h"

namespace Common {

PlainPoMessageList::PlainPoMessageList() : _messages(nullptr), _size(0), _allocated(0) {
}

PlainPoMessageList::~PlainPoMessageList() {
	for (int i = 0; i < _size; ++i)
		delete[] _messages[i];
	delete[] _messages;
}

void PlainPoMessageList::insert(const char *msg) {
	if (msg == nullptr || *msg == '\0')
		return;

	// binary-search for the insertion index
	int leftIndex = 0;
	int rightIndex = _size - 1;
	while (rightIndex >= leftIndex) {
		int midIndex = (leftIndex + rightIndex) / 2;
		int compareResult = strcmp(msg, _messages[midIndex]);
		if (compareResult == 0)
			return;
		else if (compareResult < 0)
			rightIndex = midIndex - 1;
		else
			leftIndex = midIndex + 1;
	}
	// We now have rightIndex = leftIndex - 1 and we need to insert the new message
	// between the two (i.a. at leftIndex).
	if (_size + 1 > _allocated) {
		_allocated += 100;
		char **newMessages = new char *[_allocated];
		for (int i = 0; i < leftIndex; ++i)
			newMessages[i] = _messages[i];
		for (int i = leftIndex; i < _size; ++i)
			newMessages[i + 1] = _messages[i];
		delete[] _messages;
		_messages = newMessages;
	} else {
		for (int i = _size - 1; i >= leftIndex; --i)
			_messages[i + 1] = _messages[i];
	}
	size_t len = 1 + strlen(msg);
	_messages[leftIndex] = new char[len];
	Common::strlcpy(_messages[leftIndex], msg, len);
	++_size;
}

int PlainPoMessageList::findIndex(const char *msg) {
	if (msg == nullptr || *msg == '\0')
		return -1;

	// binary-search for the message
	int leftIndex = 0;
	int rightIndex = _size - 1;

	while (rightIndex >= leftIndex) {
		const int midIndex = (leftIndex + rightIndex) / 2;
		const int compareResult = strcmp(msg, _messages[midIndex]);
		if (compareResult == 0)
			return midIndex;
		else if (compareResult < 0)
			rightIndex = midIndex - 1;
		else
			leftIndex = midIndex + 1;
	}

	return -1;
}

int PlainPoMessageList::size() const {
	return _size;
}

const char *PlainPoMessageList::operator[](int index) const {
	if (index < 0 || index >= _size)
		return nullptr;
	return _messages[index];
}

PlainPoMessageEntryList::PlainPoMessageEntryList(const char *lang) : _lang(nullptr), _langName(nullptr), _langNameAlt(nullptr), _useUTF8(true),
														   _list(nullptr), _size(0), _allocated(0) {
	size_t len = 1 + strlen(lang);
	_lang = new char[len];
	Common::strlcpy(_lang, lang, len);
	// Set default langName to lang
	_langNameAlt = new char[len];
	Common::strlcpy(_langNameAlt, lang, len);
}

PlainPoMessageEntryList::~PlainPoMessageEntryList() {
	delete[] _lang;
	delete[] _langName;
	delete[] _langNameAlt;
	for (int i = 0; i < _size; ++i)
		delete _list[i];
	delete[] _list;
}

void PlainPoMessageEntryList::addMessageEntry(const char *translation, const char *message, const char *context) {
	if (*message == '\0') {
		// This is the header.
		// We get the charset and the language name from the translation string
		char *str = parseLine(translation, "X-Language-name:");
		if (str != nullptr) {
			delete[] _langName;
			_langName = str;
		}
		str = parseLine(translation, "Language:");
		if (str != nullptr) {
			delete[] _langNameAlt;
			_langNameAlt = str;
		}
		str = parseLine(translation, "charset=");
		if (strcmp(str, "utf-8") != 0 && strcmp(str, "UTF-8") != 0) {
			_useUTF8 = false;
		}
		delete[] str;
		return;
	}

	// binary-search for the insertion index
	int leftIndex = 0;
	int rightIndex = _size - 1;
	while (rightIndex >= leftIndex) {
		int midIndex = (leftIndex + rightIndex) / 2;
		int compareResult = strcmp(message, _list[midIndex]->msgid);
		if (compareResult == 0) {
			if (context == nullptr) {
				if (_list[midIndex]->msgctxt == nullptr)
					return;
				compareResult = -1;
			} else {
				if (_list[midIndex]->msgctxt == nullptr)
					compareResult = 1;
				else {
					compareResult = strcmp(context, _list[midIndex]->msgctxt);
					if (compareResult == 0)
						return;
				}
			}
		}
		if (compareResult < 0)
			rightIndex = midIndex - 1;
		else
			leftIndex = midIndex + 1;
	}
	// We now have rightIndex = leftIndex - 1 and we need to insert the new message
	// between the two (i.a. at leftIndex).
	// However since the TranslationManager will pick the translation associated to no
	// context if it is not present for a specific context, we can optimize the file
	// size, memory used at run-time and performances (less strings to read from the file
	// and less strings to look for) by avoiding duplicate.
	if (context != nullptr && *context != '\0') {
		// Check if we have the same translation for no context
		int contextIndex = leftIndex - 1;
		while (contextIndex >= 0 && strcmp(message, _list[contextIndex]->msgid) == 0) {
			--contextIndex;
		}
		++contextIndex;
		if (contextIndex < leftIndex && _list[contextIndex]->msgctxt == nullptr && strcmp(translation, _list[contextIndex]->msgstr) == 0)
			return;
	}

	if (_size + 1 > _allocated) {
		_allocated += 100;
		PlainPoMessageEntry **newList = new PlainPoMessageEntry *[_allocated];
		for (int i = 0; i < leftIndex; ++i)
			newList[i] = _list[i];
		for (int i = leftIndex; i < _size; ++i)
			newList[i + 1] = _list[i];
		delete[] _list;
		_list = newList;
	} else {
		for (int i = _size - 1; i >= leftIndex; --i)
			_list[i + 1] = _list[i];
	}
	_list[leftIndex] = new PlainPoMessageEntry(translation, message, context);
	++_size;

	if (context == nullptr || *context == '\0') {
		// Remove identical translations for a specific context (see comment above)
		int contextIndex = leftIndex + 1;
		int removed = 0;
		while (contextIndex < _size && strcmp(message, _list[contextIndex]->msgid) == 0) {
			if (strcmp(translation, _list[contextIndex]->msgstr) == 0) {
				delete _list[contextIndex];
				++removed;
			} else {
				_list[contextIndex - removed] = _list[contextIndex];
			}
			++contextIndex;
		}
		if (removed > 0) {
			while (contextIndex < _size) {
				_list[contextIndex - removed] = _list[contextIndex];
				++contextIndex;
			}
		}
		_size -= removed;
	}
}

const char *PlainPoMessageEntryList::language() const {
	return _lang;
}

const char *PlainPoMessageEntryList::languageName() const {
	return _langName ? _langName : _langNameAlt;
}

bool PlainPoMessageEntryList::useUTF8() const {
	return _useUTF8;
}

int PlainPoMessageEntryList::size() const {
	return _size;
}

const PlainPoMessageEntry *PlainPoMessageEntryList::entry(int index) const {
	if (index < 0 || index >= _size)
		return nullptr;
	return _list[index];
}

PlainPoMessageEntryList *parsePoFile(const char *file, PlainPoMessageList &messages) {
	Common::File inFile;
	Common::FSNode node(file);

	if (!inFile.open(node))
		return nullptr;

	char msgidBuf[20480], msgctxtBuf[20480], msgstrBuf[20480];
	char line[20480], *currentBuf = msgstrBuf;

	// Get language from file name and create PlainPoMessageEntryList
	int index = 0, start_index = strlen(file) - 1;
	while (start_index > 0 && file[start_index - 1] != '/' && file[start_index - 1] != '\\') {
		--start_index;
	}
	while (file[start_index + index] != '.' && file[start_index + index] != '\0') {
		msgidBuf[index] = file[start_index + index];
		++index;
	}
	msgidBuf[index] = '\0';
	PlainPoMessageEntryList *list = new PlainPoMessageEntryList(msgidBuf);

	// Initialize the message attributes.
	bool fuzzy = false;
	bool fuzzy_next = false;

	// Parse the file line by line.
	// The msgstr is always the last line of an entry (i.e. msgid and msgctxt always
	// precede the corresponding msgstr).
	msgidBuf[0] = msgstrBuf[0] = msgctxtBuf[0] = '\0';
	while (!inFile.eos() && inFile.readLine(line, 1024)) {
		if (line[0] == '#' && line[1] == ',') {
			// Handle message attributes.
			if (strstr(line, "fuzzy")) {
				fuzzy_next = true;
				continue;
			}
		}
		// Skip empty and comment line
		if (*line == '\n' || *line == '#')
			continue;
		if (strncmp(line, "msgid", 5) == 0) {
			if (currentBuf == msgstrBuf) {
				// add previous entry
				if (*msgstrBuf != '\0' && !fuzzy) {
					messages.insert(msgidBuf);
					list->addMessageEntry(msgstrBuf, msgidBuf, msgctxtBuf);
				}
				msgidBuf[0] = msgstrBuf[0] = msgctxtBuf[0] = '\0';

				// Reset the attribute flags.
				fuzzy = fuzzy_next;
				fuzzy_next = false;
			}
			Common::strcpy_s(msgidBuf, stripLine(line));
			currentBuf = msgidBuf;
		} else if (strncmp(line, "msgctxt", 7) == 0) {
			if (currentBuf == msgstrBuf) {
				// add previous entry
				if (*msgstrBuf != '\0' && !fuzzy) {
					messages.insert(msgidBuf);
					list->addMessageEntry(msgstrBuf, msgidBuf, msgctxtBuf);
				}
				msgidBuf[0] = msgstrBuf[0] = msgctxtBuf[0] = '\0';

				// Reset the attribute flags
				fuzzy = fuzzy_next;
				fuzzy_next = false;
			}
			Common::strcpy_s(msgctxtBuf, stripLine(line));
			currentBuf = msgctxtBuf;
		} else if (strncmp(line, "msgstr", 6) == 0) {
			Common::strcpy_s(msgstrBuf, stripLine(line));
			currentBuf = msgstrBuf;
		} else {
			// concatenate the string at the end of the current buffer
			if (currentBuf)
				Common::strlcat(currentBuf, stripLine(line), 20480);
		}
	}
	if (currentBuf == msgstrBuf) {
		// add last entry
		if (*msgstrBuf != '\0' && !fuzzy) {
			messages.insert(msgidBuf);
			list->addMessageEntry(msgstrBuf, msgidBuf, msgctxtBuf);
		}
	}

	inFile.close();
	return list;
}

char *stripLine(char *const line) {
	// This function modifies line in place and return it.
	// Keep only the text between the first two unprotected quotes.
	// It also look for literal special characters (e.g. preceded by '\n', '\\', '\"', '\'', '\t')
	// and replace them by the special character so that strcmp() can match them at run time.
	// Look for the first quote
	char const *src = line;
	while (*src != '\0' && *src++ != '"') {
	}
	// shift characters until we reach the end of the string or an unprotected quote
	char *dst = line;
	while (*src != '\0' && *src != '"') {
		char c = *src++;
		if (c == '\\') {
			switch (c = *src++) {
			case 'n':
				c = '\n';
				break;
			case 't':
				c = '\t';
				break;
			case '\"':
				c = '\"';
				break;
			case '\'':
				c = '\'';
				break;
			case '\\':
				c = '\\';
				break;
			default:
				// Just skip
				// fprintf(stderr, "Unsupported special character \"\\%c\" in string. Please contact ScummVM developers.\n", c);
				continue;
			}
		}
		*dst++ = c;
	}
	*dst = '\0';
	return line;
}

char *parseLine(const char *line, const char *field) {
	// This function allocate and return a new char*.
	// It will return a NULL pointer if the field is not found.
	// It is used to parse the header of the po files to find the language name
	// and the charset.
	const char *str = strstr(line, field);
	if (str == nullptr)
		return nullptr;
	str += strlen(field);
	// Skip spaces
	while (*str != '\0' && Common::isSpace(*str)) {
		++str;
	}
	// Find string length (stop at the first '\n')
	int len = 0;
	while (str[len] != '\0' && str[len] != '\n') {
		++len;
	}
	if (len == 0)
		return nullptr;
	// Create result string
	char *result = new char[len + 1];
	strncpy(result, str, len);
	result[len] = '\0';
	return result;
}

} // End of namespace Common
