#include "stdafx.h"
#include "cpthelp.h"

void assertEmpty(CptObj *cpt) {
	assert((cpt->len == 0) && (cpt->data == NULL) && (cpt->dbgName == NULL) && (cpt->type == 0));
}

uint16 getInfo(char *line, char *type, char *nameDest) {
	assert(*line == '\t');
	line++;
	assert(!memcmp(line, type, strlen(type)));
	line += strlen(type);
	assert((line[0] == ':') && (line[1] == ':'));
	line += 2;
	char *stopCh;
	uint16 res = (uint16)strtoul(line, &stopCh, 16);
	assert(line != stopCh);
	assert((stopCh[0] == ':') && (stopCh[1] == ':'));
	stopCh += 2;
	strcpy(nameDest, stopCh);
	assert(strlen(nameDest) < 32);
	assert(res);
	return res;
}

void dofgets(char *dest, int len, FILE *inf) {
	fgets(dest, len, inf);
	while (*dest)
		if ((*dest == 0xA) || (*dest == 0xD))
			*dest = '\0';
		else
			dest++;
}

bool lineMatchSection(char *line, char *sec) {
	if (memcmp(line, "SECTION::", 9))
		return false;
	if (memcmp(line + 9, sec, strlen(sec)))
		return false;
	return true;
}

bool isEndOfSection(char *line) {
	if (strcmp(line, "SECTION::ENDS"))
		return false;
	return true;
}

bool isEndOfObject(char *line, char *type, uint16 id) {
	if (*line != '\t')
		return false;
	line++;
	if (memcmp(line, type, strlen(type)))
		return false;
	line += strlen(type);
	if ((line[0] != ':') || (line[1] != ':'))
		return false;
	line += 2;
	char *stopCh;
	uint16 idVal = (uint16)strtoul(line, &stopCh, 16);
	assert(stopCh != line);
	if (strcmp(stopCh, "::ENDS"))
		return false;
	assert(id == idVal);
	return true;
}
