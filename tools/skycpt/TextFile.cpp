#include "stdafx.h"
#include "TextFile.h"

TextFile::TextFile(char *name) {
	FILE *inf = fopen(name, "r");
	if (!inf) {
		printf("Unable to open file %s\n", name);
		getchar();
	}
	_lines = NULL;
	read(inf);
	fclose(inf);
}

uint32 crop(char *line) {
	char *start = line;
	while (*line)
		if ((*line == 0xA) || (*line == 0xD))
			*line = '\0';
		else if ((line[0] == '/') && (line[1] == '/'))
			*line = '\0';
		else
			line++;

	while (((*(line - 1) == ' ') || ((*(line - 1)) == '\t')) && (line > start)) {
		line--;
		*line = '\0';
	}
	return (uint32)(line - start);
}

char *TextFile::giveLine(uint32 num) {
	if (num >= _numLines)
		return NULL;
	else
		return _lines[num];
}

void TextFile::read(FILE *inf) {
	char *line = (char*)malloc(4096);
	_lines = (char**)malloc(4096 * sizeof(char *));
	_numLines = 0;
	uint32 linesMax = 4096;
	while (fgets(line, 4096, inf)) {
		if (_numLines >= linesMax) {
			_lines = (char**)realloc(_lines, linesMax * 2 * sizeof(char *));
			linesMax *= 2;
		}
		char *start = line;
		while ((*start == '\t') || (*start == ' '))
			start++;
		if (memcmp(start, "(int16)", 7) == 0) {
			start += 7;
			while ((*start == '\t') || (*start == ' '))
				start++;
		}
		uint32 length = crop(start);
		_lines[_numLines] = (char*)malloc(length + 1);
		memcpy(_lines[_numLines], start, length + 1);
		_numLines++;
	}
	free(line);
}

int32 TextFile::findLine(KmpSearch *kmp, uint32 fromLine) {
	for (uint32 cnt = fromLine; cnt < _numLines; cnt++) {
		if (kmp->search(_lines[cnt]))
			return cnt;
	}
	return -1;
}
