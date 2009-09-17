/* ScummVM Tools
 * Copyright (C) 2009 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */


#include "extract.h"

// Filename creation

void createFilename(char *dstFilename, const int gid, const int lang, const int platform, const int special, const char *filename);

namespace {

void createLangFilename(char *dstFilename, const int gid, const int lang, const int platform, const int special, const char *filename);

// Extraction function prototypes 

bool extractRaw(PAKFile &out, const Game *g, const byte *data, const uint32 size, const char *filename, int id, int lang);
bool extractStrings(PAKFile &out, const Game *g, const byte *data, const uint32 size, const char *filename, int id, int lang);
bool extractStrings10(PAKFile &out, const Game *g, const byte *data, const uint32 size, const char *filename, int id, int lang);
bool extractRooms(PAKFile &out, const Game *g, const byte *data, const uint32 size, const char *filename, int id, int lang);
bool extractShapes(PAKFile &out, const Game *g, const byte *data, const uint32 size, const char *filename, int id, int lang);
bool extractAmigaSfx(PAKFile &out, const Game *g, const byte *data, const uint32 size, const char *filename, int id, int lang);
bool extractWdSfx(PAKFile &out, const Game *g, const byte *data, const uint32 size, const char *filename, int id, int lang);

bool extractHofSeqData(PAKFile &out, const Game *g, const byte *data, const uint32 size, const char *filename, int id, int lang);
bool extractHofShapeAnimDataV1(PAKFile &out, const Game *g, const byte *data, const uint32 size, const char *filename, int id, int lang);
bool extractHofShapeAnimDataV2(PAKFile &out, const Game *g, const byte *data, const uint32 size, const char *filename, int id, int lang);

bool extractStringsWoSuffix(PAKFile &out, const Game *g, const byte *data, const uint32 size, const char *filename, int id, int lang);
bool extractPaddedStrings(PAKFile &out, const Game *g, const byte *data, const uint32 size, const char *filename, int id, int lang);
bool extractRaw16to8(PAKFile &out, const Game *g, const byte *data, const uint32 size, const char *filename, int id, int lang);
bool extractMrShapeAnimData(PAKFile &out, const Game *g, const byte *data, const uint32 size, const char *filename, int id, int lang);
bool extractRaw16(PAKFile &out, const Game *g, const byte *data, const uint32 size, const char *filename, int id, int lang);
bool extractRaw32(PAKFile &out, const Game *g, const byte *data, const uint32 size, const char *filename, int id, int lang);
bool extractLolButtonDefs(PAKFile &out, const Game *g, const byte *data, const uint32 size, const char *filename, int id, int lang);

// Extraction type table

const ExtractType extractTypeTable[] = {
	{ kTypeLanguageList, extractStrings, createLangFilename },
	{ kTypeStringList, extractStrings, createFilename },
	{ kTypeRoomList, extractRooms, createFilename },
	{ kTypeShapeList, extractShapes, createFilename },
	{ kTypeRawData, extractRaw, createFilename },
	{ kTypeAmigaSfxTable, extractAmigaSfx, createFilename },
	{ kTypeTownsWDSfxTable, extractWdSfx, createFilename },

	{ k2TypeSeqData, extractHofSeqData, createFilename },
	{ k2TypeShpDataV1, extractHofShapeAnimDataV1, createFilename },
	{ k2TypeShpDataV2, extractHofShapeAnimDataV2, createFilename },
	{ k2TypeSoundList, extractStringsWoSuffix, createFilename },
	{ k2TypeLangSoundList, extractStringsWoSuffix, createLangFilename },
	{ k2TypeSize10StringList, extractStrings10, createFilename },
	{ k2TypeSfxList, extractPaddedStrings, createFilename },
	{ k3TypeRaw16to8, extractRaw16to8, createFilename },
	{ k3TypeShpData, extractMrShapeAnimData, createFilename },

	{ kLolTypeRaw16, extractRaw16, createFilename },
	{ kLolTypeRaw32, extractRaw32, createFilename },
	{ kLolTypeButtonDef, extractLolButtonDefs, createFilename },

	{ -1, 0, 0}
};

} // end of anonymous namespace

void createFilename(char *dstFilename, const int gid, const int lang, const int platform, const int special, const char *filename) {
	strcpy(dstFilename, filename);

	static const char *gidExtensions[] = { "", ".K2", ".K3", 0, ".LOL" };
	strcat(dstFilename, gidExtensions[gid]);

	for (const SpecialExtension *specialE = specialTable; specialE->special != -1; ++specialE) {
		if (specialE->special == special) {
			strcat(dstFilename, ".");
			strcat(dstFilename, specialE->ext);
			break;
		}
	}

	for (const PlatformExtension *platformE = platformTable; platformE->platform != -1; ++platformE) {
		if (platformE->platform == platform) {
			strcat(dstFilename, ".");
			strcat(dstFilename, platformE->ext);
		}
	}
}

namespace {

void createLangFilename(char *dstFilename, const int gid, const int lang, const int platform, const int special, const char *filename) {
	strcpy(dstFilename, filename);

	for (const Language *langE = languageTable; langE->lang != -1; ++langE) {
		if (langE->lang == lang) {
			strcat(dstFilename, ".");
			strcat(dstFilename, langE->ext);
			break;
		}
	}

	static const char *gidExtensions[] = { "", ".K2", ".K3", 0, ".LOL" };
	strcat(dstFilename, gidExtensions[gid]);

	for (const SpecialExtension *specialE = specialTable; specialE->special != -1; ++specialE) {
		if (specialE->special == special) {
			strcat(dstFilename, ".");
			strcat(dstFilename, specialE->ext);
			break;
		}
	}

	for (const PlatformExtension *platformE = platformTable; platformE->platform != -1; ++platformE) {
		if (platformE->platform == platform) {
			strcat(dstFilename, ".");
			strcat(dstFilename, platformE->ext);
		}
	}
}

} // end of anonymous namespace

// misc

const ExtractType *findExtractType(const int type) {
	for (const ExtractType *i = extractTypeTable; i->type != -1; ++i) {
		if (i->type == type)
			return i;
	}
	return 0;
}

// Extractor implementation

namespace {

bool extractRaw(PAKFile &out, const Game *g, const byte *data, const uint32 size, const char *filename, int id, int lang) {
	uint8 *buffer = new uint8[size];
	assert(buffer);
	memcpy(buffer, data, size);

	return out.addFile(filename, buffer, size);
}

bool extractStrings(PAKFile &out, const Game *g, const byte *data, const uint32 size, const char *filename, int id, int lang) {
	int fmtPatch = 0;
	// FM Towns files that need addional patches
	if (g->platform == kPlatformFMTowns) {
		if (id == kTakenStrings || id == kNoDropStrings || id == kPoisonGoneString ||
			id == kThePoisonStrings || id == kFluteStrings || id == kWispJewelStrings)
			fmtPatch = 1;
		else if (id == kIntroStrings)
			fmtPatch = 2;
		else if (id == k2SeqplayStrings)
			fmtPatch = 3;
	}

	if (g->special == k2FloppyFile2) {
		if (id == k2IngamePakFiles)
			fmtPatch = 4;
	}

	uint32 entries = 0;
	uint32 targetsize = size + 4;
	for (uint32 i = 0; i < size; ++i) {
		if (!data[i]) {
			if (g->platform == kPlatformAmiga) {
				if (i + 1 >= size)
					++entries;
				else if (!data[i+1] && !(i & 1))
					continue;
				else
					++entries;
			} else {
				++entries;
			}

			if (g->platform == kPlatformFMTowns) {
				// prevents creation of empty entries (which we have mostly between all strings in the FM-TOWNS version)
				while (!data[++i]) {
					if (i == size)
						break;
					targetsize--;
				}
				if (fmtPatch == 1) {
					// Here is the first step of the extra treatment for all FM-TOWNS string arrays that
					// contain more than one string and which the original code
					// addresses via stringname[boolJapanese].
					// We simply skip every other string
					if (i == size)
						continue;
					uint32 len = strlen((const char*) data + i);
					i += len;

					targetsize = targetsize - 1 - len;

					while (!data[++i]) {
						if (i == len)
							break;
						targetsize--;
					}
				}
			}
		}
	}

	if (fmtPatch == 2) {
		if (g->special == kFMTownsVersionE) {
			targetsize--;
			entries += 1;
		} else if (g->special == kFMTownsVersionJ) {
			targetsize += 2;
			entries += 2;
		}
	}

	if (fmtPatch == 3) {
		entries++;
		targetsize++;
	}

	if (fmtPatch == 4) {
		targetsize -= 9;
	}

	uint8 *buffer = new uint8[targetsize];
	assert(buffer);
	memset(buffer, 0, targetsize);
	uint8 *output = buffer;
	const uint8 *input = (const uint8*) data;

	WRITE_BE_UINT32(output, entries); output += 4;
	if (g->platform == kPlatformFMTowns) {
		const byte *c = data + size;
		do {
			if (fmtPatch == 2 && input - data == 0x3C0 && input[0x10] == 0x32) {
				memcpy(output, input, 0x0F);
				input += 0x11; output += 0x0F;
			}

			strcpy((char*) output, (const char*) input);
			uint32 stringsize = strlen((const char*)output) + 1;
			input += stringsize; output += stringsize;
			// skip empty entries
			while (!*input) {
				// Write one empty string into intro strings file
				if (fmtPatch == 2) {
					if ((g->special == kFMTownsVersionE && input - data == 0x260) ||
						(g->special == kFMTownsVersionJ && input - data == 0x2BD) ||
						(g->special == kFMTownsVersionJ && input - data == 0x2BE))
							*output++ = *input;
				}

				// insert one dummy string at hof sequence strings position 59
				if (fmtPatch == 3) {
					if ((g->special == k2TownsFile1E && input - data == 0x695) ||
						(g->special == k2TownsFile1J && input - data == 0x598))
							*output++ = *input;
				}

				if (++input == c)
					break;
			}

			if (fmtPatch == 1) {
				// Here is the extra treatment for all FM-TOWNS string arrays that
				// contain more than one string and which the original code
				// addresses via stringname[boolJapanese].
				// We simply skip every other string
				if (input == c)
					continue;
				input += strlen((const char*)input);
				while (!*input) {
					if (++input == c)
						break;
				}
			}

		} while (input < c);
	} else if (g->platform == kPlatformAmiga) {
		// we need to strip some aligment zeros out here
		int dstPos = 0;
		for (uint32 i = 0; i < size; ++i) {
			if (!data[i] && !(i & 1)) {
				if (i + 1 > size)
					continue;
				else if (i + 1 < size && !data[i+1])
					continue;
			}

			*output++ = data[i];
			++dstPos;
		}
		targetsize = dstPos + 4;
	} else {
		uint32 copySize = size;
		if (fmtPatch == 4) {
			memcpy(output, data, 44);
			output += 44;
			data += 44;
			for (int t = 1; t != 10; t++) {
				sprintf((char*) output, "COST%d_SH.PAK", t);
				output += 13;
			}
			data += 126;
			copySize -= 170;
		}
		memcpy(output, data, copySize);
	}

	return out.addFile(filename, buffer, targetsize);
}

bool extractStrings10(PAKFile &out, const Game *g, const byte *data, const uint32 size, const char *filename, int id, int lang) {
	const int strSize = 10;
	uint32 entries = (size + (strSize - 1)) / strSize;

	uint8 *buffer = new uint8[size + 4];
	assert(buffer);
	uint8 *output = buffer;
	WRITE_BE_UINT32(output, entries); output += 4;

	for (uint32 i = 0; i < entries; ++i) {
		const byte *src = data + i * strSize;

		while (*src)
			*output++ = *src++;
		*output++ = '\0';
	}

	return out.addFile(filename, buffer, output - buffer);
}

bool extractRooms(PAKFile &out, const Game *g, const byte *data, const uint32 size, const char *filename, int id, int lang) {
	// different entry size for the FM-TOWNS version
	const int roomEntrySize = (g->platform == kPlatformFMTowns) ? (0x69) : ((g->platform == kPlatformAmiga) ? 0x52 : 0x51);
	const int countRooms = size / roomEntrySize;

	uint8 *buffer = new uint8[countRooms * 9 + 4];
	assert(buffer);
	uint8 *output = buffer;

	WRITE_BE_UINT32(output, countRooms); output += 4;

	const byte *src = data;
	if (g->platform == kPlatformAmiga) {
		for (int i = 0; i < countRooms; ++i) {
			*output++ = *src++; assert(*src == 0); ++src;
			memcpy(output, src, 8); output += 0x8;
			src += roomEntrySize - 0x2;
		}
	} else {
		for (int i = 0; i < countRooms; ++i) {
			*output++ = *src++;
			WRITE_BE_UINT16(output, READ_LE_UINT16(src)); output += 2; src += 2;
			WRITE_BE_UINT16(output, READ_LE_UINT16(src)); output += 2; src += 2;
			WRITE_BE_UINT16(output, READ_LE_UINT16(src)); output += 2; src += 2;
			WRITE_BE_UINT16(output, READ_LE_UINT16(src)); output += 2; src += 2;
			src += roomEntrySize - 0x9;
		}
	}

	return out.addFile(filename, buffer, countRooms * 9 + 4);
}

bool extractShapes(PAKFile &out, const Game *g, const byte *data, const uint32 size, const char *filename, int id, int lang) {
	byte *buffer = new byte[size + 1 * 4];
	assert(buffer);
	byte *output = buffer;

	const int count = size / 0x07;
	WRITE_BE_UINT32(output, count); output += 4;
	memcpy(output, data, size);

	return out.addFile(filename, buffer, size + 1 * 4);
}

bool extractAmigaSfx(PAKFile &out, const Game *g, const byte *data, const uint32 size, const char *filename, int id, int lang) {
	const uint32 entries = size / 8;
	byte *buffer = new byte[entries * 6 + 1 * 4];

	byte *output = buffer;
	WRITE_BE_UINT32(output, entries); output += 4;

	for (uint32 i = 0; i < entries; ++i) {
		*output++ = *data++;	// Note
		*output++ = *data++;	// Patch
		data += 2;				// Unused
		WRITE_BE_UINT16(output, READ_BE_UINT16(data)); output += 2; data += 2; // Duration
		*output++ = *data++;	// Volume
		*output++ = *data++;	// Pan
	}

	return out.addFile(filename, buffer, entries * 6 + 1 * 4);
}

bool extractWdSfx(PAKFile &out, const Game *g, const byte *data, const uint32 size, const char *filename, int id, int lang) {
	const int bufferSize = 0x12602;

	uint8 *buffer = new uint8[0x12602];
	assert(buffer);
	memcpy(buffer, data, 0x7EE5);
	memcpy(buffer + 0x7EE5, data + 0x7EE7, 0x7FFF);
	memcpy(buffer + 0xFEE4, data + 0xFEE8, 0x271E);

	return out.addFile(filename, buffer, bufferSize);
}

int extractHofSeqData_checkString(const void *ptr, uint8 checkSize);
int extractHofSeqData_isSequence(const void *ptr, const Game *g, uint32 maxCheckSize);
int extractHofSeqData_isControl(const void *ptr, uint32 size);

bool extractHofSeqData(PAKFile &out, const Game *g, const byte *data, const uint32 size, const char *filename, int id, int lang) {
	int numSequences = 0;
	int numNestedSequences = 0;

	uint16 headerSize = 50 * sizeof(uint16);
	uint16 bufferSize = size + headerSize;
	byte *buffer = new byte[bufferSize];
	assert(buffer);
	memset(buffer, 0, bufferSize );
	uint16 *header = (uint16*) buffer;
	byte *output = buffer + headerSize;
	uint16 *hdout = header;

	//debug(1, "\nProcessing Hand of Fate sequence data:\n--------------------------------------\n");
	for (int cycle = 0; cycle < 2; cycle++) {
		const byte *ptr = data;
		hdout++;

		const byte * endOffs = (const byte *)(data + size);

		// detect sequence structs
		while (ptr < endOffs) {
			if (ptr[1]) {
				error("invalid sequence data encountered");
				delete[] buffer;
				return false;
			}

			int v = extractHofSeqData_isSequence(ptr, g, endOffs - ptr);

			if (cycle == 0 && v == 1) {
				if ((g->special == k2FloppyFile1 && *ptr == 5) || (g->special == k2DemoVersion && (ptr - data == 312))) {
					// patch for floppy version: skips invalid ferb sequence
					// patch for demo: skips invalid title sequence
					ptr += 54;
					continue;
				}

				numSequences++;
				uint16 relOffs = (uint16) (output - buffer);
				WRITE_BE_UINT16(hdout, relOffs);
				hdout++;

				WRITE_BE_UINT16(output, READ_LE_UINT16(ptr)); // flags
				ptr += 2;
				output += 2;

				memcpy(output, ptr, 28); // wsa and cps file names
				ptr += 28;
				output += 28;

				if (g->special == k2TownsFile1E) { // startupCommand + finalCommand
					memcpy(output , ptr, 2);
					ptr += 2;
					output += 2;
				} else {
					*output++ = READ_LE_UINT16(ptr) & 0xff;
					ptr += 2;
					*output++ = READ_LE_UINT16(ptr) & 0xff;
					ptr += 2;
				}

				for (int w = 0; w < 7; w++) { //stringIndex1 to yPos
					WRITE_BE_UINT16(output, READ_LE_UINT16(ptr));
					ptr += 2;
					output += 2;
				}

				ptr += 4;
				WRITE_BE_UINT16(output, READ_LE_UINT16(ptr)); // duration
				ptr += 2;
				output+= 2;

			} else if (cycle == 1 && v != 1 && v != -2) {
				uint16 controlOffs = 0;
				uint16 ctrSize = 0;
				if (v) {
					const byte *ctrStart = ptr;
					while (v && v != -2) {
						ptr++;
						v = extractHofSeqData_isSequence(ptr, g, endOffs - ptr);
					}

					if (v == -2)
						break;

					ctrSize = (uint16)(ptr - ctrStart);

					if (g->special != k2DemoVersion &&
						extractHofSeqData_isControl(ctrStart, ctrSize)) {
						controlOffs = (uint16) (output - buffer);
						*output++ = ctrSize >> 2;

						for (int cc = 0; cc < ctrSize; cc += 2)
							WRITE_BE_UINT16(output + cc, READ_LE_UINT16(ctrStart + cc)); // frame control
						output += ctrSize;
					}
				}

				numNestedSequences++;
				uint16 relOffs = (uint16) (output - buffer);
				WRITE_BE_UINT16(hdout, relOffs);
				hdout++;

				WRITE_BE_UINT16(output, READ_LE_UINT16(ptr)); // flags
				ptr += 2;
				output += 2;

				memcpy(output, ptr, 14); // wsa file name
				ptr += 14;
				output += 14;

				// startframe
				WRITE_BE_UINT16(output, READ_LE_UINT16(ptr));
				ptr += 2;
				output += 2;

				// endFrame
				WRITE_BE_UINT16(output, (ctrSize && ((ctrSize >> 2)  < READ_LE_UINT16(ptr))) ? (ctrSize >> 2) : READ_LE_UINT16(ptr));
				ptr += 2;
				output += 2;

				// frameDelay
				WRITE_BE_UINT16(output, READ_LE_UINT16(ptr));
				ptr += 2;
				output += 2;

				ptr += 4;

				for (int w = 0; w < 2; w++) { //x, y
					WRITE_BE_UINT16(output, READ_LE_UINT16(ptr));
					ptr += 2;
					output += 2;
				}

				if (!READ_LE_UINT32(ptr))
					controlOffs = 0;

				WRITE_BE_UINT16(output, controlOffs);
				if (g->special != k2DemoVersion && g->special != k2DemoLol)
					ptr += 4;
				output += 2;

				if (g->special != k2DemoVersion && g->special != k2DemoLol) {
					for (int w = 0; w < 2; w++) { //startupCommand, finalCommand
						WRITE_BE_UINT16(output, READ_LE_UINT16(ptr));
						ptr += 2;
						output += 2;
					}
				} else {
					memset(output, 0, 4);
					output += 4;
				}

				if (g->special == k2TownsFile1E)
					ptr += 2;

			} else if (cycle == 0) {
				while (v != 1 && v != -2) {
					ptr++;
					v = extractHofSeqData_isSequence(ptr, g, endOffs - ptr);
				}

				if (v == -2)
					break;


			} else if (cycle == 1) {
				while (v == 1 && v != -2) {
					ptr++;
					v = extractHofSeqData_isSequence(ptr, g, endOffs - ptr);
				}

				if (v == -2)
					break;
			}
		}
	}

	uint16 finHeaderSize = (2 + numSequences + numNestedSequences) * sizeof(uint16);
	uint16 finBufferSize = ((output - buffer) - headerSize) + finHeaderSize;
	byte *finBuffer = new byte[finBufferSize];
	assert(finBuffer);
	uint16 diff = headerSize - finHeaderSize;
	uint16 *finHeader = (uint16*) finBuffer;

	for (int i = 1; i < finHeaderSize; i++)
		WRITE_BE_UINT16(&finHeader[i], (READ_BE_UINT16(&header[i]) - diff));
	WRITE_BE_UINT16(finHeader, numSequences);
	WRITE_BE_UINT16(&finHeader[numSequences + 1], numNestedSequences);
	memcpy (finBuffer + finHeaderSize, buffer + headerSize, finBufferSize - finHeaderSize);
	delete[] buffer;

	finHeader = (uint16*) (finBuffer + ((numSequences + 2) * sizeof(uint16)));
	for (int i = 0; i < numNestedSequences; i++) {
		uint8 * offs = finBuffer + READ_BE_UINT16(finHeader++) + 26;
		uint16 ctrl = READ_BE_UINT16(offs);
		if (ctrl)
			ctrl -= diff;
		WRITE_BE_UINT16(offs, ctrl);
	}

	return out.addFile(filename, finBuffer, finBufferSize);
}

int extractHofSeqData_checkString(const void *ptr, uint8 checkSize) {
	// return values: 1 = text; 0 = zero string; -1 = other

	int t = 0;
	int c = checkSize;
	const uint8 *s = (const uint8*)ptr;

	// check for character string
	while (c--) {
		if (*s > 31 && *s < 123)
			t++;
		s++;
	}

	if (t == checkSize)
		return 1;

	// check for zero string
	c = checkSize;
	uint32 sum = 0;
	s = (const uint8*)ptr;
	while (c--)
		sum += *s++;

	return (sum) ? -1 : 0;
}

int extractHofSeqData_isSequence(const void *ptr, const Game *g, uint32 maxCheckSize) {
	// return values: 1 = Sequence; 0 = Nested Sequence; -1 = other; -2 = overflow

	if (maxCheckSize < 30)
		return -2;

	const uint8 * s = (const uint8*)ptr;
	int c1 = extractHofSeqData_checkString(s + 2, 6);
	int c2 = extractHofSeqData_checkString(s + 16, 6);
	int c3 = extractHofSeqData_checkString(s + 2, 14);
	int c4 = extractHofSeqData_checkString(s + 16, 14);
	int c0 = s[1];
	int c5 = s[0];

	if (c0 == 0 && c5 && ((c1 + c2) >= 1) && (!(c3 == 0 && c2 != 1)) && (!(c4 == 0 && c1 != 1))) {
		if (maxCheckSize < 41)
			return -2;

		if (g->special == k2TownsFile1E) {
			if (!(s[37] | s[39]) && s[38] > s[36])
				return 1;
		} else {
			if (!(s[39] | s[41]) && s[40] > s[38])
				return 1;
		}
	}

	if (c0 == 0 && c5 == 4 && c3 == 0 && c4 == 0) {
		if (maxCheckSize >= 41 && READ_LE_UINT32(s + 34) && !(s[39] | s[41]) && s[40] > s[38])
			return 1;
	}

	if (c0 == 0 && c5 && c1 == 1 && c4 == -1 && s[20])
		return 0;

	return -1;
}

int extractHofSeqData_isControl(const void *ptr, uint32 size) {
	// return values: 1 = possible frame control data; 0 = definitely not frame control data

	const uint8 *s = (const uint8*)ptr;
	for (uint32 i = 2; i < size; i += 4) {
		if (!s[i])
			return 0;
	}

	for (uint32 i = 1; i < size; i += 2) {
		if (s[i])
			return 0;
	}
	return 1;
}

bool extractHofShapeAnimDataV1(PAKFile &out, const Game *g, const byte *data, const uint32 size, const char *filename, int id, int lang) {
	int outsize = 1;
	uint8 *buffer = new uint8[size + 1];
	const uint8 *src = data;
	uint8 *dst = buffer + 1;

	for (int i = 0; i < 4; i++) {
		WRITE_BE_UINT16(dst, READ_LE_UINT16(src));
		src += 2;
		dst += 2;
		WRITE_BE_UINT16(dst, READ_LE_UINT16(src));
		src += 4;
		dst += 2;
		outsize += 4;

		for (int j = 0; j < 20; j++) {
			WRITE_BE_UINT16(dst, READ_LE_UINT16(src));
			src += 2;
			dst += 2;
			outsize += 2;
		}

	};

	*buffer = 4; // number of items

	return out.addFile(filename, buffer, outsize);
}

bool extractHofShapeAnimDataV2(PAKFile &out, const Game *g, const byte *data, const uint32 size, const char *filename, int id, int lang) {
	int outsize = 1;
	uint8 *buffer = new uint8[size + 1];
	const uint8 *src = data;
	uint8 *dst = buffer + 1;
	const uint8 *fin = data + size;
	int count = 0;

	do {
		if (READ_LE_UINT16(src) == 0xffff)
			break;

		count++;

		WRITE_BE_UINT16(dst, READ_LE_UINT16(src));
		src += 2;
		dst += 2;

		uint8 numFrames = *src;
		*dst++ = numFrames;
		src += 6;
		outsize += 3;

		for (int i = 0; i < (numFrames << 1); i++) {
			WRITE_BE_UINT16(dst, READ_LE_UINT16(src));
			src += 2;
			dst += 2;
			outsize += 2;
		}

		src += (48 - (numFrames << 2));

	} while (src < fin);

	*buffer = count; // number of items

	return out.addFile(filename, buffer, outsize);
}

bool extractStringsWoSuffix(PAKFile &out, const Game *g, const byte *data, const uint32 size, const char *filename, int id, int lang) {
	int outsize = size + 4;
	uint8 *buffer = new uint8[outsize];
	const uint8 *src = data;
	uint8 *dst = buffer + 4;
	const uint8 *fin = src + size;
	int entries = 0;

	while (src < fin) {
		while (!*src && src < fin)
			src++;
		while (*src && *src != '.' && src < fin)
			*dst++ = *src++;

		*dst++ = '\0';
		entries++;

		if (*src == '.') {
			while (*src && src < fin)
				src++;
		}
	}

	WRITE_BE_UINT32(buffer, entries);
	outsize = dst - buffer;

	return out.addFile(filename, buffer, outsize);
}

bool extractPaddedStrings(PAKFile &out, const Game *g, const byte *data, const uint32 size, const char *filename, int id, int lang) {
	int outsize = size + 4;
	uint8 *buffer = new uint8[outsize];
	const uint8 *src = data;
	uint8 *dst = buffer + 4;
	const uint8 *fin = src + size;
	int entries = 0;

	while (src < fin) {
		while (!*src && src < fin)
			src++;
		while (*src && src < fin)
			*dst++ = *src++;

		*dst++ = '\0';
		entries++;
	}

	WRITE_BE_UINT32(buffer, entries);
	outsize = dst - buffer;

	return out.addFile(filename, buffer, outsize);
}

bool extractRaw16to8(PAKFile &out, const Game *g, const byte *data, const uint32 size, const char *filename, int id, int lang) {
	int outsize = size >> 1;
	uint8 *buffer = new uint8[outsize];
	const uint8 *src = data;
	uint8 *dst = buffer;

	for (int i = 0; i < outsize; i++) {
		*dst++ = *src++;
		*src++;
	}

	return out.addFile(filename, buffer, outsize);
}

bool extractRaw16(PAKFile &out, const Game *g, const byte *data, const uint32 size, const char *filename, int id, int lang) {
	uint8 *buffer = new uint8[size];
	const uint8 *src = data;
	uint8 *dst = buffer;

	for (uint32 i = 0; i < (size >> 1); i++) {
		WRITE_BE_UINT16(dst, READ_LE_UINT16(src));
		src += 2;
		dst += 2;
	}

	return out.addFile(filename, buffer, size);
}

bool extractRaw32(PAKFile &out, const Game *g, const byte *data, const uint32 size, const char *filename, int id, int lang) {
	uint8 *buffer = new uint8[size];
	const uint8 *src = data;
	uint8 *dst = buffer;

	for (uint32 i = 0; i < (size >> 2); i++) {
		WRITE_BE_UINT32(dst, READ_LE_UINT32(src));
		src += 4;
		dst += 4;
	}

	return out.addFile(filename, buffer, size);
}

bool extractLolButtonDefs(PAKFile &out, const Game *g, const byte *data, const uint32 size, const char *filename, int id, int lang) {
	int num = size / 22;
	uint8 *buffer = new uint8[size];
	uint32 outsize = num * 18;
	const uint8 *src = data;
	uint8 *dst = buffer;

	for (int i = 0; i < num; i++) {
		WRITE_BE_UINT16(dst, READ_LE_UINT16(src));
		src += 2; dst += 2;
		WRITE_BE_UINT16(dst, READ_LE_UINT16(src));
		src += 2; dst += 2;
		WRITE_BE_UINT16(dst, READ_LE_UINT16(src));
		src += 6; dst += 2;
		WRITE_BE_UINT16(dst, READ_LE_UINT16(src));
		src += 2; dst += 2;
		WRITE_BE_UINT16(dst, READ_LE_UINT16(src));
		src += 2; dst += 2;
		WRITE_BE_UINT16(dst, READ_LE_UINT16(src));
		src += 2; dst += 2;
		WRITE_BE_UINT16(dst, READ_LE_UINT16(src));
		src += 2; dst += 2;
		WRITE_BE_UINT16(dst, READ_LE_UINT16(src));
		src += 2; dst += 2;
		WRITE_BE_UINT16(dst, READ_LE_UINT16(src));
		src += 2; dst += 2;
	}

	return out.addFile(filename, buffer, outsize);
}

bool extractMrShapeAnimData(PAKFile &out, const Game *g, const byte *data, const uint32 size, const char *filename, int id, int lang) {
	int outsize = 1;
	uint8 *buffer = new uint8[size + 1];
	const uint8 *src2 = data;
	const uint8 *src1 = data + 324;
	uint8 *dst = buffer + 1;
	const uint8 *fin = data + size;
	int count = 0;

	do {
		if (READ_LE_UINT16(src1) == 0xffff)
			break;

		count++;

		WRITE_BE_UINT16(dst, READ_LE_UINT16(src1));
		src1 += 2;
		dst += 2;

		uint8 numFrames = *src1;
		*dst++ = numFrames;
		src1 += 10;
		outsize += 3;

		for (int i = 0; i < (numFrames << 1); i++) {
			WRITE_BE_UINT16(dst, READ_LE_UINT16(src2));
			src2 += 2;
			dst += 2;
			outsize += 2;
		}
	} while (src1 < fin);

	*buffer = count; // number of items

	return out.addFile(filename, buffer, outsize);
}

} // end of anonymous namespace

