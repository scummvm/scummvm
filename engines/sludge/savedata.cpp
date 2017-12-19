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

#include "common/file.h"

#include "sludge/allfiles.h"
#include "sludge/moreio.h"
#include "sludge/newfatal.h"
#include "sludge/variable.h"

#define LOAD_ERROR "Can't load custom data...\n\n"

namespace Sludge {

const char UTF8_CHECKER[] = {'U', 'N', '\xef', '\xbf', '\xbd', 'L', 'O', '\xef', '\xbf', '\xbd', 'C', 'K', 'E', 'D', '\0'};
uint16 saveEncoding = false;
char encode1 = 0;
char encode2 = 0;

void writeStringEncoded(const Common::String &s, Common::WriteStream *stream) {
	int len = s.size();

	stream->writeUint16BE(len);
	for (int a = 0; a < len; a++) {
		stream->writeByte(s[a] ^ encode1);
		encode1 += encode2;
	}
}

Common::String readStringEncoded(Common::File *fp) {
	int len = fp->readUint16BE();
	Common::String res = "";

	for (int a = 0; a < len; a++) {
		res += (char)(fp->readByte() ^ encode1);
		encode1 += encode2;
	}
	return res;
}

char *readTextPlain(Common::File *fp) {
	int32 startPos;

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
		uint bytes_read = fp->read(reply, stringSize);
		if (bytes_read != stringSize && fp->err()) {
			warning("Reading error in readTextPlain.");
		}
		fp->readByte();  // Skip the newline character
		reply[stringSize] = 0;
	}

	return reply;
}

bool fileToStack(const Common::String &filename, StackHandler *sH) {

	Variable stringVar;
	stringVar.varType = SVT_NULL;
	Common::String checker = saveEncoding ? "[Custom data (encoded)]\r\n" : "[Custom data (ASCII)]\n";

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

	encode1 = (byte)saveEncoding & 255;
	encode2 = (byte)(saveEncoding >> 8);

	for (uint i = 0; i < checker.size(); ++i) {
		if (fd.readByte() != checker[i]) {
			fd.close();
			return fatal(LOAD_ERROR "This isn't a SLUDGE custom data file:", filename);
		}
	}

	if (saveEncoding) {
		checker = readStringEncoded(&fd);
		if (checker == UTF8_CHECKER) {
			fd.close();
			return fatal(
			LOAD_ERROR "The current file encoding setting does not match the encoding setting used when this file was created:", filename);
		}
	}

	for (;;) {
		if (saveEncoding) {
			char i = fd.readByte() ^ encode1;

			if (fd.eos())
				break;
			switch (i) {
				case 0: {
					Common::String g = readStringEncoded(&fd);
					makeTextVar(stringVar, g);
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

bool stackToFile(const Common::String &filename, const Variable &from) {
#if 0
	FILE *fp = fopen(filename, saveEncoding ? "wb" : "wt");
	if (!fp) return fatal("Can't create file", filename);

	VariableStack *hereWeAre = from.varData.theStack -> first;

	encode1 = (byte)saveEncoding & 255;
	encode2 = (byte)(saveEncoding >> 8);

	if (saveEncoding) {
		fprintf(fp, "[Custom data (encoded)]\r\n");
		writeStringEncoded(UTF8_CHECKER, fp);
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
