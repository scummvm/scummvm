// AsciiCptCompile.cpp : Definiert den Einstiegspunkt für die Konsolenanwendung.
//

#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "TextFile.h"

typedef unsigned char byte;
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned long uint32;
typedef unsigned int uint;
typedef signed char int8;
typedef signed short int16;
typedef signed long int32;

void doCompile(FILE *inf, FILE *debOutf, FILE *resOutf, TextFile *cptDef, FILE *sve);

int main(int argc, char* argv[])
{
	uint8 testBuf[4] = { 0x11, 0x22, 0x33, 0x44 };
	if (*(uint32*)testBuf != 0x44332211) {
		printf("Sorry, this program only works on little endian systems.\nGoodbye.\n");
		return 0;
	}
	TextFile *cptDef = new TextFile("compact.txt");
	FILE *inf = fopen("compact.txt", "r");
	FILE *dbg = fopen("compact.dbg", "wb");
	FILE *out = fopen("compact.bin", "wb");
	FILE *sve = fopen("savedata.txt", "r");
	assert(inf && dbg && out && sve);
	doCompile(inf, dbg, out, cptDef, sve);
	fclose(inf);
	fclose(dbg);
	fclose(out);
	fclose(sve);
	printf("done\n");
	return 0;
}




