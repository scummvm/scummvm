#include "stdafx.h"
#include "TextFile.h"
#include "KmpSearch.h"

uint16 findCptId(char *name, TextFile *cptFile) {
	KmpSearch *kmp = new KmpSearch();
	kmp->init(name);
	int cLine = 0;
	do {
		cLine = cptFile->findLine(kmp, cLine);
		if (cLine >= 0) {
			char *line = cptFile->giveLine(cLine);
			if ((strncmp(line, "COMPACT::", 9) == 0) || 
				(strncmp(line, "SCRATCH::", 9) == 0) ||
				(strncmp(line, "GET_TOS::", 9) == 0)) {
				char *stopCh;
				uint16 resId = (uint16)strtoul(line + 9, &stopCh, 16);
				if ((stopCh[0] == ':') && (stopCh[1] == ':') && (strcmp(stopCh + 2, name) == 0)) {
					delete kmp;
					return resId;
				}
			}
			cLine++;
		}
	} while (cLine != -1);
	delete kmp;
	return 0;
}
