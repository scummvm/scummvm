#include "stdafx.h"

void assertEmpty(CptObj *cpt);
uint16 getInfo(char *line, char *type, char *nameDest);
void dofgets(char *dest, int len, FILE *inf);
bool lineMatchSection(char *line, char *sec);
bool isEndOfSection(char *line);
bool isEndOfObject(char *line, char *type, uint16 id);