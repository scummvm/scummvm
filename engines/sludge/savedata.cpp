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

#include "common/file.h"

#include "sludge/allfiles.h"
#include "sludge/debug.h"
#include "sludge/variable.h"
#include "sludge/newfatal.h"
#include "sludge/moreio.h"

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

void writeStringEncoded(const char *s, Common::WriteStream *stream) {
	int a, len = strlen(s);

	stream->writeUint16BE(len);
	for (a = 0; a < len; a++) {
		stream->writeByte(s[a] ^ encode1);
		encode1 += encode2;
	}
}

char *readStringEncoded(Common::File *fp) {
	int a, len = fp->readUint16BE();
	char *s = new char[len + 1];
	if (!checkNew(s))
		return NULL;
	for (a = 0; a < len; a++) {
		s[a] = (char)(fp->readByte() ^ encode1);
		encode1 += encode2;
	}
	s[len] = 0;
	return s;
}

char *readTextPlain(Common::File *fp) {
	int32_t startPos;

	uint32 stringSize = 0;
	bool keepGoing = true;
	char gotChar;
	char *reply;

	startPos = fp->pos();

	while (keepGoing) {
		gotChar = (char)fp->readByte();
		if ((gotChar == '\n') || (fp->eos())) {
			keepGoing = false;
		} else {
			stringSize++;
		}
	}

	if ((stringSize == 0) && (fp->eos())) {
		return NULL;
	} else {
		fp->seek(startPos, SEEK_SET);
		reply = new char[stringSize + 1];
		if (reply == NULL)
			return NULL;
		size_t bytes_read = fp->read(reply, stringSize);
		if (bytes_read != stringSize && fp->err()) {
			debugOut("Reading error in readTextPlain.\n");
		}
		fp->readByte();  // Skip the newline character
		reply[stringSize] = 0;
	}

	return reply;
}

bool fileToStack(char *filename, stackHandler *sH) {

	variable stringVar;
	stringVar.varType = SVT_NULL;
	const char *checker = saveEncoding ? "[Custom data (encoded)]\r\n" : "[Custom data (ASCII)]\n";

	Common::File fd;

	if (!fd.open(filename)) {
#if 0
		char currentDir[1000];
		if (!getcwd(currentDir, 998)) {
			debugOut("Can't get current directory.\n");
		}

		if (chdir(gamePath)) {
			debugOut("Error: Failed changing to directory %s\n", gamePath);
		}

		if (chdir(currentDir)) {
			debugOut("Error: Failed changing to directory %s\n", currentDir);
		}

		if (!fd.open(filename)) {
			return fatal("No such file", filename);
		}
#endif
		return fatal("No such file", filename); //TODO: false value
	}

	encode1 = (unsigned char)saveEncoding & 255;
	encode2 = (unsigned char)(saveEncoding >> 8);

	while (*checker) {
		if (fd.readByte() != *checker) {
			fd.close();
			return fatal(LOAD_ERROR "This isn't a SLUDGE custom data file:", filename);
		}
		checker++;
	}

	if (saveEncoding) {
		char *checker = readStringEncoded(&fd);
		if (strcmp(checker, "UN�LO�CKED")) {
			fd.close();
			return fatal(
			LOAD_ERROR "The current file encoding setting does not match the encoding setting used when this file was created:", filename);
		}
		delete checker;
		checker = NULL;
	}

	for (;;) {
		if (saveEncoding) {
			char i = fd.readByte() ^ encode1;

			if (fd.eos())
				break;
			switch (i) {
				case 0: {
					char *g = readStringEncoded(&fd);
					makeTextVar(stringVar, g);
					delete g;
				}
					break;

				case 1:
					setVariable(stringVar, SVT_INT, fd.readUint32LE());
					break;

				case 2:
					setVariable(stringVar, SVT_INT, fd.readByte());
					break;

				default:
					fatal(LOAD_ERROR "Corrupt custom data file:", filename);
					fd.close();
					return false;
			}
		} else {
			char *line = readTextPlain(&fd);
			if (!line)
				break;
			makeTextVar(stringVar, line);
		}

		if (sH->first == NULL) {
			// Adds to the TOP of the array... oops!
			if (!addVarToStackQuick(stringVar, sH->first))
				return false;
			sH->last = sH->first;
		} else {
			// Adds to the END of the array... much better
			if (!addVarToStackQuick(stringVar, sH->last->next))
				return false;
			sH->last = sH->last->next;
		}
	}
	fd.close();

	return true;
}

bool stackToFile(char *filename, const variable &from) {
#if 0
	FILE *fp = fopen(filename, saveEncoding ? "wb" : "wt");
	if (!fp) return fatal("Can't create file", filename);

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
					fp->writeUint32LE(hereWeAre -> thisVar.varData.intValue);
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
