/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 */

#include "common/util.h"
#include "common/textconsole.h"
#include "common/stream.h"

#include "engines/grim/textsplit.h"

namespace Grim {

static bool isCodeSeparator(char c) {
	return (c == ' ' || c == ',' || c == '.' || c == '%' || c == '\'' || c == ':');
}

static bool isSeparator(char c) {
	return (c == ' ' || c == ',' || c == ':');
}

int power(int base, int exp) {
	int res = 1;
	for (int i = 0; i < exp; ++i) {
		res *= base;
	}
	return res;
}

static float str2float(const char *str) {
	int len = strlen(str);
	int dotpos = len;
	char *int_part = new char[len + 1];
	int j = 0;
	for (int i = 0; i < len; ++i) {
		if (str[i] != '.') {
			int_part[j++] = str[i];
		} else {
			dotpos = i;
			break;
		}
	}
	int_part[j++] = '\0';

	float num = atoi(int_part);
	int sign = (str[0] == '-' ? -1 : 1);
	j = 0;
	for (int i = dotpos + 1; i < len; ++i) {
		float part = (float)(str[i] - 48) / (float)power(10, ++j);
		num += part * sign;
	}

	delete[] int_part;

	return num;
}

static void parse(const char *line, const char *fmt, int field_count, va_list va) {
	char *str = strdup(line);
	const int len = strlen(str);
	for (int i = 0; i < len; ++i) {
		if (str[i] == '\t')
			str[i] = ' ';
	}

	char *format = strdup(fmt);
	const int formatlen = strlen(format);
	for (int i = 0; i < formatlen; ++i) {
		if (format[i] == '\t')
			format[i] = ' ';
	}

	int count = 0;
	const char *src = str;
	const char *end = str + len;
	for (int i = 0; i < formatlen; ++i) {
		if (format[i] == '%') {
			char code[10];
			int j = 0;
			while (++i < formatlen && !isCodeSeparator(format[i])) {
				code[j++] = format[i];
			}
			code[j++] = '\0';

			void *var = va_arg(va, void *);
			if (strcmp(code, "n") == 0) {
				*(int*)var = src - str;
				continue;
			}

			char s[2000];

			j = 0;
			if (code[0] != 'c') {
				char nextchar = format[i];
				while (src[0] == ' ') { //skip initial whitespace
					++src;
				}
				while (src != end && src[0] != nextchar && !isSeparator(src[0])) {
					s[j++] = src[0];
					++src;
				}
			} else {
				s[j++] = src[0];
				++src;
			}
			s[j++] = '\0';
			--i;

			if (strcmp(code, "d") == 0) {
				*(int*)var = atoi(s);
			} else if (strcmp(code, "x") == 0) {
				*(int*)var = strtol(s, (char **) NULL, 16);
			} else if (strcmp(code, "f") == 0) {
				*(float*)var = str2float(s);
			} else if (strcmp(code, "c") == 0) {
				*(char*)var = s[0];
			} else if (strcmp(code, "s") == 0) {
				char *string = (char*)var;
				strcpy(string, s);
			} else if (code[strlen(code) - 1] == 's') {
				char *string = (char*)var;
				strncpy(string, s, 10);
				string[10] = '\0';
			} else {
				error("Code not handled: \"%s\" \"%s\"\n\"%s\" \"%s\"", code, s, line, fmt);
			}

			++count;
			continue;
		}

		while (src[0] == ' ') {
			++src;
		}
		if (src == end)
			break;

		if (src[0] != format[i] && format[i] != ' ') {
			error("Expected line of format '%s', got '%s'", fmt, line);
		}

		if (src == end)
			break;
		if (format[i] != ' ') {
			++src;
			if (src == end)
				break;
		}
	}
	free(str);
	free(format);

	if (count < field_count) {
		error("Expected line of format '%s', got '%s'", fmt, line);
	}
}


TextSplitter::TextSplitter(Common::SeekableReadStream *data) {
	char *line;
	int i;
	uint32 len = data->size();

	_stringData = new char[len + 1];
	data->read(_stringData, len);
	_stringData[len] = '\0';
	// Find out how many lines of text there are
	_numLines = _lineIndex = 0;
	line = (char *)_stringData;
	while (line) {
		line = strchr(line, '\n');
		if (line) {
			_numLines++;
			line++;
		}
	}
	// Allocate an array of the lines
	_lines = new char *[_numLines];
	line = (char *)_stringData;
	for (i = 0; i < _numLines;i++) {
		char *lastLine = line;
		line = strchr(lastLine, '\n');
		*line = '\0';
		_lines[i] = lastLine;
		line++;
	}
	_currLine = NULL;
	processLine();
}

TextSplitter::~TextSplitter() {
	delete[] _stringData;
	delete[] _lines;
}

bool TextSplitter::checkString(const char *needle) {
	// checkString also needs to check for extremely optional
	// components like "object_art" which can be missing entirely
	if (!getCurrentLine())
		return false;
	else if (strstr(getCurrentLine(), needle))
		return true;
	else
		return false;
}

void TextSplitter::expectString(const char *expected) {
	if (!_currLine)
		error("Expected `%s', got EOF", expected);
	if (scumm_stricmp(getCurrentLine(), expected) != 0)
		error("Expected `%s', got '%s'", expected, getCurrentLine());
	nextLine();
}

void TextSplitter::scanString(const char *fmt, int field_count, ...) {
	if (!_currLine)
		error("Expected line of format '%s', got EOF", fmt);

	va_list va;
	va_start(va, field_count);

	parse(getCurrentLine(), fmt, field_count, va);

	va_end(va);

	nextLine();
}

void TextSplitter::scanStringAtOffset(int offset, const char *fmt, int field_count, ...) {
	if (!_currLine)
		error("Expected line of format '%s', got EOF", fmt);

	va_list va;
	va_start(va, field_count);

	parse(getCurrentLine() + offset, fmt, field_count, va);

	va_end(va);

	nextLine();
}

void TextSplitter::scanStringNoNewLine(const char *fmt, int field_count, ...) {
	if (!_currLine)
		error("Expected line of format '%s', got EOF", fmt);

	va_list va;
	va_start(va, field_count);

	parse(getCurrentLine(), fmt, field_count, va);

	va_end(va);
}

void TextSplitter::scanStringAtOffsetNoNewLine(int offset, const char *fmt, int field_count, ...) {
	if (!_currLine)
		error("Expected line of format '%s', got EOF", fmt);

	va_list va;
	va_start(va, field_count);

	parse(getCurrentLine() + offset, fmt, field_count, va);

	va_end(va);
}

void TextSplitter::processLine() {
	if (isEof())
		return;

	_currLine = _lines[_lineIndex++];

	// Cut off comments
	char *comment_start = strchr(_currLine, '#');
	if (comment_start)
		*comment_start = '\0';

	// Cut off trailing whitespace (including '\r')
	char *strend = strchr(_currLine, '\0');
	while (strend > _currLine && Common::isSpace(strend[-1]))
		strend--;
	*strend = '\0';

	// Skip blank lines
	if (*_currLine == '\0')
		nextLine();

	// Convert to lower case
	if (!isEof())
		for (char *s = _currLine; *s != '\0'; s++)
			*s = tolower(*s);
}

} // end of namespace Grim
