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
 */
#include <stdint.h>
#include <unistd.h>
#include <string.h>

#include "allfiles.h"
#include "debug.h"
#include "variable.h"
#include "newfatal.h"
#include "moreio.h"

#define LOAD_ERROR "Can't load custom data...\n\n"

namespace Sludge {

unsigned short saveEncoding = false;
char encode1 = 0;
char encode2 = 0;

extern char *gamePath;

/*
void loadSaveDebug (char * com) {
    FILE * ffpp = fopen ("debuggy.txt", "at");
    fprintf (ffpp, "%s\n", com);
    fclose (ffpp);
}

void loadSaveDebug (char com) {
    FILE * ffpp = fopen ("debuggy.txt", "at");
    fprintf (ffpp, "%c\n", com);
    fclose (ffpp);
}

void loadSaveDebug (int com) {
    FILE * ffpp = fopen ("debuggy.txt", "at");
    fprintf (ffpp, "%d\n", com);
    fclose (ffpp);
}
*/

#if ALLOW_FILE
void writeStringEncoded(const char *s, FILE *fp) {
	int a, len = strlen(s);

	put2bytes(len, fp);
	for (a = 0; a < len; a ++) {
		fputc(s[a] ^ encode1, fp);
		encode1 += encode2;
	}
}

char *readStringEncoded(FILE *fp) {
	int a, len = get2bytes(fp);
	char *s = new char[len + 1];
	if (! checkNew(s)) return NULL;
	for (a = 0; a < len; a ++) {
		s[a] = (char)(fgetc(fp) ^ encode1);
		encode1 += encode2;
	}
	s[len] = 0;
	return s;
}

char *readTextPlain(FILE *fp) {
	int32_t startPos;

	int stringSize = 0;
	bool keepGoing = true;
	char gotChar;
	char *reply;

	startPos = ftell(fp);

	while (keepGoing) {
		gotChar = (char) fgetc(fp);
		if ((gotChar == '\n') || (feof(fp))) {
			keepGoing = false;
		} else {
			stringSize ++;
		}
	}

	if ((stringSize == 0) && (feof(fp))) {
		return NULL;
	} else {
		fseek(fp, startPos, SEEK_SET);
		reply = new char[stringSize + 1];
		if (reply == NULL) return NULL;
		size_t bytes_read = fread(reply, stringSize, 1, fp);
		if (bytes_read != stringSize && ferror(fp)) {
			debugOut("Reading error in readTextPlain.\n");
		}
		fgetc(fp);  // Skip the newline character
		reply[stringSize] = 0;
	}

	return reply;
}
#endif

bool fileToStack(char *filename, stackHandler *sH) {
#if ALLOW_FILE
	variable stringVar;
	stringVar.varType = SVT_NULL;
	const char *checker = saveEncoding ? "[Custom data (encoded)]\r\n" : "[Custom data (ASCII)]\n";

	FILE *fp = fopen(filename, "rb");
	if (! fp) {
		char currentDir[1000];
		if (! getcwd(currentDir, 998)) {
			debugOut("Can't get current directory.\n");
		}

		if (chdir(gamePath)) {
			debugOut("Error: Failed changing to directory %s\n", gamePath);
		}
		fp = fopen(filename, "rb");
		if (chdir(currentDir)) {
			debugOut("Error: Failed changing to directory %s\n", currentDir);
		}

		if (! fp) {
			return fatal("No such file", filename);
		}
	}

	encode1 = (unsigned char) saveEncoding & 255;
	encode2 = (unsigned char)(saveEncoding >> 8);

	while (* checker) {
		if (fgetc(fp) != * checker) {
			fclose(fp);
			return fatal(LOAD_ERROR "This isn't a SLUDGE custom data file:", filename);
		}
		checker ++;
	}

	if (saveEncoding) {
		char *checker = readStringEncoded(fp);
		if (strcmp(checker, "UN�LO�CKED")) {
			fclose(fp);
			return fatal(LOAD_ERROR "The current file encoding setting does not match the encoding setting used when this file was created:", filename);
		}
		delete checker;
		checker = NULL;
	}


	for (;;) {
		if (saveEncoding) {
			char i = fgetc(fp) ^ encode1;

			if (feof(fp)) break;
			switch (i) {
			case 0: {
				char *g = readStringEncoded(fp);
				makeTextVar(stringVar, g);
				delete g;
			}
			break;

			case 1:
				setVariable(stringVar, SVT_INT, get4bytes(fp));
				break;

			case 2:
				setVariable(stringVar, SVT_INT, fgetc(fp));
				break;

			default:
				fatal(LOAD_ERROR "Corrupt custom data file:", filename);
				fclose(fp);
				return false;
			}
		} else {
			char *line = readTextPlain(fp);
			if (! line) break;
			makeTextVar(stringVar, line);
		}

		if (sH -> first == NULL) {
			// Adds to the TOP of the array... oops!
			if (! addVarToStackQuick(stringVar, sH -> first)) return false;
			sH -> last = sH -> first;
		} else {
			// Adds to the END of the array... much better
			if (! addVarToStackQuick(stringVar, sH -> last -> next)) return false;
			sH -> last = sH -> last -> next;
		}
	}
	fclose(fp);
#endif
	return true;
}

bool stackToFile(char *filename, const variable &from) {
#if ALLOW_FILE
	FILE *fp = fopen(filename, saveEncoding ? "wb" : "wt");
	if (! fp) return fatal("Can't create file", filename);

	variableStack *hereWeAre = from.varData.theStack -> first;

	encode1 = (unsigned char) saveEncoding & 255;
	encode2 = (unsigned char)(saveEncoding >> 8);

	if (saveEncoding) {
		fprintf(fp, "[Custom data (encoded)]\r\n");
		writeStringEncoded("UN�LO�CKED", fp);
	} else {
		fprintf(fp, "[Custom data (ASCII)]\n");
	}

	while (hereWeAre) {
		if (saveEncoding) {
			switch (hereWeAre -> thisVar.varType) {
			case SVT_STRING:
				fputc(encode1, fp);
				writeStringEncoded(hereWeAre -> thisVar.varData.theString, fp);
				break;

			case SVT_INT:
				// Small enough to be stored as a char
				if (hereWeAre -> thisVar.varData.intValue >= 0 && hereWeAre -> thisVar.varData.intValue < 256) {
					fputc(2 ^ encode1, fp);
					fputc(hereWeAre -> thisVar.varData.intValue, fp);
				} else {
					fputc(1 ^ encode1, fp);
					put4bytes(hereWeAre -> thisVar.varData.intValue, fp);
				}
				break;

			default:
				fatal("Can't create an encoded custom data file containing anything other than numbers and strings", filename);
				fclose(fp);
				return false;
			}
		} else {
			char *makeSureItsText = getTextFromAnyVar(hereWeAre -> thisVar);
			if (makeSureItsText == NULL) break;
			fprintf(fp, "%s\n", makeSureItsText);
			delete makeSureItsText;
		}

		hereWeAre = hereWeAre -> next;
	}
	fclose(fp);
#endif
	return true;
}

} // End of namespace Sludge
