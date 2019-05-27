#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "po_parser.h"

PoMessageList::PoMessageList() : _list(NULL), _size(0), _allocated(0) {
}

PoMessageList::~PoMessageList() {
	for (int i = 0; i < _size; ++i)
		delete _list[i];
	delete[] _list;
}

int PoMessageList::compareString(const char* left, const char* right) {
	if (left == NULL && right == NULL)
		return 0;
	if (left == NULL)
		return -1;
	if (right == NULL)
		return 1;
	return strcmp(left, right);
}

int PoMessageList::compareMessage(const char *msgLeft, const char *contextLeft, const char *msgRight, const char *contextRight) {
	int compare = compareString(msgLeft, msgRight);
	if (compare != 0)
		return compare;
	return compareString(contextLeft, contextRight);
}

void PoMessageList::insert(const char *translation, const char *msg, const char *context) {
	if (msg == NULL || *msg == '\0' || translation == NULL || *translation == '\0')
		return;

	// binary-search for the insertion index
	int leftIndex = 0;
	int rightIndex = _size - 1;
	while (rightIndex >= leftIndex) {
		int midIndex = (leftIndex + rightIndex) / 2;
		int compareResult = compareMessage(msg, context, _list[midIndex]->msgid, _list[midIndex]->msgctxt);
		if (compareResult == 0)
			return; // The message is already in this list
		else if (compareResult < 0)
			rightIndex = midIndex - 1;
		else
			leftIndex = midIndex + 1;
	}
	// We now have rightIndex = leftIndex - 1 and we need to insert the new message
	// between the two (i.a. at leftIndex).
	if (_size + 1 > _allocated) {
		_allocated += 100;
		PoMessage **newList = new PoMessage*[_allocated];
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
	_list[leftIndex] = new PoMessage(translation, msg, context);
	++_size;
}

const char *PoMessageList::findTranslation(const char *msg, const char *context) {
	if (msg == NULL || *msg == '\0')
		return NULL;

	// binary-search for the message
	int leftIndex = 0;
	int rightIndex = _size - 1;
	while (rightIndex >= leftIndex) {
		int midIndex = (leftIndex + rightIndex) / 2;
		int compareResult = compareMessage(msg, context, _list[midIndex]->msgid, _list[midIndex]->msgctxt);
		if (compareResult == 0)
			return _list[midIndex]->msgstr;
		else if (compareResult < 0)
			rightIndex = midIndex - 1;
		else
			leftIndex = midIndex + 1;
	}
	return NULL;
}

PoMessageList *parsePoFile(const char *file) {
	FILE *inFile = fopen(file, "r");
	if (!inFile)
		return NULL;

	char msgidBuf[1024], msgctxtBuf[1024], msgstrBuf[1024];
	char line[1024], *currentBuf = msgstrBuf;

	PoMessageList *list = new PoMessageList();

	// Initialize the message attributes.
	bool fuzzy = false;
	bool fuzzy_next = false;

	// Parse the file line by line.
	// The msgstr is always the last line of an entry (i.e. msgid and msgctxt always
	// precede the corresponding msgstr).
	msgidBuf[0] = msgstrBuf[0] = msgctxtBuf[0] = '\0';
	while (!feof(inFile) && fgets(line, 1024, inFile)) {
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
				if (*msgstrBuf != '\0' && !fuzzy)
					list->insert(msgstrBuf, msgidBuf, msgctxtBuf);
				msgidBuf[0] = msgstrBuf[0] = msgctxtBuf[0] = '\0';

				// Reset the attribute flags.
				fuzzy = fuzzy_next;
				fuzzy_next = false;
			}
			strcpy(msgidBuf, stripLine(line));
			currentBuf = msgidBuf;
		} else if (strncmp(line, "msgctxt", 7) == 0) {
			if (currentBuf == msgstrBuf) {
				// add previous entry
				if (*msgstrBuf != '\0' && !fuzzy)
					list->insert(msgstrBuf, msgidBuf, msgctxtBuf);
				msgidBuf[0] = msgstrBuf[0] = msgctxtBuf[0] = '\0';

				// Reset the attribute flags
				fuzzy = fuzzy_next;
				fuzzy_next = false;
			}
			strcpy(msgctxtBuf, stripLine(line));
			currentBuf = msgctxtBuf;
		} else if (strncmp(line, "msgstr", 6) == 0) {
			strcpy(msgstrBuf, stripLine(line));
			currentBuf = msgstrBuf;
		} else {
			// concatenate the string at the end of the current buffer
			if (currentBuf)
				strcat(currentBuf, stripLine(line));
		}
	}
	if (currentBuf == msgstrBuf) {
		// add last entry
		if (*msgstrBuf != '\0' && !fuzzy)
			list->insert(msgstrBuf, msgidBuf, msgctxtBuf);
	}

	fclose(inFile);
	return list;
}

char *stripLine(char *const line) {
	// This function modifies line in place and return it.
	// Keep only the text between the first two unprotected quotes.
	// It also look for literal special characters (e.g. preceded by '\n', '\\', '\"', '\'', '\t')
	// and replace them by the special character so that strcmp() can match them at run time.
	// Look for the first quote
	char const *src = line;
	while (*src != '\0' && *src++ != '"') {}
	// shift characters until we reach the end of the string or an unprotected quote
	char *dst = line;
	while (*src != '\0' && *src != '"') {
		char c = *src++;
		if (c == '\\') {
			switch (c = *src++) {
				case  'n': c = '\n'; break;
				case  't': c = '\t'; break;
				case '\"': c = '\"'; break;
				case '\'': c = '\''; break;
				case '\\': c = '\\'; break;
				default:
					// Just skip
					fprintf(stderr, "Unsupported special character \"\\%c\" in string. Please contact ScummVM developers.\n", c);
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
	if (str == NULL)
		return NULL;
	str += strlen(field);
	// Skip spaces
	while (*str != '\0' && isspace(*str)) {
		++str;
	}
	// Find string length (stop at the first '\n')
	int len = 0;
	while (str[len] != '\0' && str[len] != '\n') {
		++len;
	}
	if (len == 0)
		return NULL;
	// Create result string
	char *result = new char[len + 1];
	strncpy(result, str, len);
	result[len] = '\0';
	return result;
}

