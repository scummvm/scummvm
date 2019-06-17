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

#include "common/savefile.h"

#include "sludge/allfiles.h"
#include "sludge/moreio.h"
#include "sludge/newfatal.h"
#include "sludge/savedata.h"
#include "sludge/variable.h"

#define LOAD_ERROR "Can't load custom data...\n\n"

namespace Sludge {

const char CustomSaveHelper::UTF8_CHECKER[] = {'U', 'N', '\xef', '\xbf', '\xbd', 'L', 'O', '\xef', '\xbf', '\xbd', 'C', 'K', 'E', 'D', '\0'};
uint16 CustomSaveHelper::_saveEncoding = false;
char CustomSaveHelper::_encode1 = 0;
char CustomSaveHelper::_encode2 = 0;

void CustomSaveHelper::writeStringEncoded(const Common::String checker, Common::WriteStream *stream) {
	int len = checker.size();

	stream->writeUint16BE(len);
	for (int a = 0; a < len; a++) {
		stream->writeByte(checker[a] ^ _encode1);
		_encode1 += _encode2;
	}
}

Common::String CustomSaveHelper::readStringEncoded(Common::SeekableReadStream *fp) {
	int len = fp->readUint16BE();
	Common::String res = "";

	for (int a = 0; a < len; a++) {
		res += (char)(fp->readByte() ^ _encode1);
		_encode1 += _encode2;
	}
	return res;
}

char *CustomSaveHelper::readTextPlain(Common::SeekableReadStream *fp) {
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

bool CustomSaveHelper::fileToStack(const Common::String &filename, StackHandler *sH) {
	Variable stringVar;
	stringVar.varType = SVT_NULL;
	Common::String checker = _saveEncoding ? "[Custom data (encoded)]\r\n" : "[Custom data (ASCII)]\n";

	Common::InSaveFile *fp = g_system->getSavefileManager()->openForLoading(filename);

	if (fp == NULL) {
		return fatal("No such file", filename); //TODO: false value
	}

	_encode1 = (byte)_saveEncoding & 255;
	_encode2 = (byte)(_saveEncoding >> 8);

	for (uint i = 0; i < checker.size(); ++i) {
		if (fp->readByte() != checker[i]) {
			delete fp;
			return fatal(LOAD_ERROR "This isn't a SLUDGE custom data file:", filename);
		}
	}

	if (_saveEncoding) {
		checker = readStringEncoded(fp);
		if (checker != UTF8_CHECKER) {
			delete fp;
			return fatal(LOAD_ERROR "The current file encoding setting does not match the encoding setting used when this file was created:", filename);
		}
	}

	for (;;) {
		if (_saveEncoding) {
			char i = fp->readByte() ^ _encode1;

			if (fp->eos())
				break;
			switch (i) {
				case 0: {
					Common::String g = readStringEncoded(fp);
					stringVar.makeTextVar(g);
				}
					break;

				case 1:
					stringVar.setVariable(SVT_INT, fp->readUint32LE());
					break;

				case 2:
					stringVar.setVariable(SVT_INT, fp->readByte());
					break;

				default:
					fatal(LOAD_ERROR "Corrupt custom data file:", filename);
					delete fp;
					return false;
			}
		} else {
			char *line = readTextPlain(fp);
			if (!line)
				break;
			stringVar.makeTextVar(line);
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

	delete fp;

	return true;
}

bool CustomSaveHelper::stackToFile(const Common::String &filename, const Variable &from) {
	Common::OutSaveFile *fp = g_system->getSavefileManager()->openForSaving(filename);
	if (fp == NULL) {
		return fatal("Can't create file", filename);
	}

	VariableStack *hereWeAre = from.varData.theStack -> first;

	_encode1 = (byte)_saveEncoding & 255;
	_encode2 = (byte)(_saveEncoding >> 8);

	if (_saveEncoding) {
		fp->writeString("[Custom data (encoded)]\r\n");
		writeStringEncoded(UTF8_CHECKER, fp);
	} else {
		fp->writeString("[Custom data (ASCII)]\n");
	}

	while (hereWeAre) {
		if (_saveEncoding) {
			switch (hereWeAre -> thisVar.varType) {
				case SVT_STRING:
					fp->writeByte(_encode1);
					writeStringEncoded(hereWeAre -> thisVar.varData.theString, fp);
					break;

				case SVT_INT:
					// Small enough to be stored as a char
					if (hereWeAre -> thisVar.varData.intValue >= 0 && hereWeAre -> thisVar.varData.intValue < 256) {
						fp->writeByte(2 ^ _encode1);
						fp->writeByte(hereWeAre -> thisVar.varData.intValue);
					} else {
						fp->writeByte(1 ^ _encode1);
						fp->writeUint32LE(hereWeAre -> thisVar.varData.intValue);
					}
					break;

				default:
					fatal("Can't create an encoded custom data file containing anything other than numbers and strings", filename);
					delete fp;
					return false;
			}
		} else {
			Common::String makeSureItsText = hereWeAre->thisVar.getTextFromAnyVar();
			if (makeSureItsText.empty())
				break;
			fp->writeString((makeSureItsText + "\n").c_str());
		}

		hereWeAre = hereWeAre -> next;
	}

	delete fp;

	return true;
}

} // End of namespace Sludge
