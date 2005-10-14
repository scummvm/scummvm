#ifndef __TextFile__
#define __TextFile__

#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include "KmpSearch.h"

class TextFile {
public:
	TextFile(char *name);
	~TextFile(void);
	char *giveLine(uint32 num);
	int32 findLine(KmpSearch *kmp, uint32 fromLine = 0);
private:
	void read(FILE *inf);
	char **_lines;
	uint32 _numLines;
};

#endif