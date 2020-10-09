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

#include "glk/level9/detection.h"
#include "glk/level9/detection_tables.h"
#include "glk/level9/level9_main.h"
#include "glk/level9/os_glk.h"
#include "glk/blorb.h"
#include "glk/detection.h"
#include "common/debug.h"
#include "common/file.h"
#include "common/md5.h"
#include "engines/game.h"

namespace Glk {
namespace Level9 {

long Scanner::scanner(byte *startFile, uint32 size, byte **dictData, byte **aCodePtr) {
	_dictData = dictData;
	_aCodePtr = aCodePtr;

#ifdef FULLSCAN
	FullScan(startfile, FileSize);
#endif

	int offset = scan(startFile, size);
	if (offset < 0) {
		offset = ScanV2(startFile, size);
		_gameType = L9_V2;
		if (offset < 0) {
			offset = ScanV1(startFile, size);
			_gameType = L9_V1;
			if (offset < 0) {
				return -1;
			}
		}
	}

	return offset;
}

const L9V1GameInfo &Scanner::v1Game() const {
	assert(_gameType == L9_V1);
	return L9_V1_GAMES[_l9V1Game];
}

long Scanner::scan(byte *startFile, uint32 size) {
	uint32 i, num, Size, MaxSize = 0;
	int j;
	uint16 d0 = 0, l9, md, ml, dd, dl;
	uint32 Min, Max;
	long offset = -1;
	bool JumpKill, DriverV4;

	if (size < 33)
		return -1;

	byte *Chk = (byte *)malloc(size + 1);
	byte *Image = (byte *)calloc(size, 1);

	if ((Chk == nullptr) || (Image == nullptr)) {
		error("Unable to allocate memory for game scan! Exiting...");
	}

	Chk[0] = 0;
	for (i = 1; i <= size; i++)
		Chk[i] = Chk[i - 1] + startFile[i - 1];

	for (i = 0; i < size - 33; i++) {
		num = L9WORD(startFile + i) + 1;
		/*
		        Chk[i] = 0 +...+ i-1
		        Chk[i+n] = 0 +...+ i+n-1
		        Chk[i+n] - Chk[i] = i + ... + i+n
		*/
		if (num > 0x2000 && i + num <= size && Chk[i + num] == Chk[i]) {
			md = L9WORD(startFile + i + 0x2);
			ml = L9WORD(startFile + i + 0x4);
			dd = L9WORD(startFile + i + 0xa);
			dl = L9WORD(startFile + i + 0xc);

			if (ml > 0 && md > 0 && i + md + ml <= size && dd > 0 && dl > 0 && i + dd + dl * 4 <= size) {
				/* v4 files may have acodeptr in 8000-9000, need to fix */
				for (j = 0; j < 12; j++) {
					d0 = L9WORD(startFile + i + 0x12 + j * 2);
					if (j != 11 && d0 >= 0x8000 && d0 < 0x9000) {
						if (d0 >= 0x8000 + LISTAREASIZE) break;
					} else if (i + d0 > size) break;
				}
				/* list9 ptr must be in listarea, acode ptr in data */
				if (j < 12 /*|| (d0>=0x8000 && d0<0x9000)*/) continue;

				l9 = L9WORD(startFile + i + 0x12 + 10 * 2);
				if (l9 < 0x8000 || l9 >= 0x8000 + LISTAREASIZE) continue;

				Size = 0;
				Min = Max = i + d0;
				DriverV4 = 0;
				if (ValidateSequence(startFile, Image, i + d0, i + d0, &Size, size, &Min, &Max, false, &JumpKill, &DriverV4)) {
					if (Size > MaxSize && Size > 100) {
						offset = i;
						MaxSize = Size;
						_gameType = DriverV4 ? L9_V4 : L9_V3;
					}
				}
			}
		}
	}

	free(Chk);
	free(Image);
	return offset;
}

long Scanner::ScanV2(byte *startFile, uint32 size) {
	uint32 i, Size, MaxSize = 0, num;
	int j;
	uint16 d0 = 0, l9;
	uint32 Min, Max;
	long offset = -1;
	bool JumpKill;

	if (size < 28)
		return -1;

	byte *Chk = (byte *)malloc(size + 1);
	byte *Image = (byte *)calloc(size, 1);

	if ((Chk == nullptr) || (Image == nullptr)) {
		error("Unable to allocate memory for game scan! Exiting...");
	}

	Chk[0] = 0;
	for (i = 1; i <= size; i++)
		Chk[i] = Chk[i - 1] + startFile[i - 1];

	for (i = 0; i < size - 28; i++) {
		num = L9WORD(startFile + i + 28) + 1;
		if ((i + num) <= size && i < (size - 32) && ((Chk[i + num] - Chk[i + 32]) & 0xff) == startFile[i + 0x1e]) {
			for (j = 0; j < 14; j++) {
				d0 = L9WORD(startFile + i + j * 2);
				if (j != 13 && d0 >= 0x8000 && d0 < 0x9000) {
					if (d0 >= 0x8000 + LISTAREASIZE) break;
				} else if (i + d0 > size) break;
			}
			/* list9 ptr must be in listarea, acode ptr in data */
			if (j < 14 /*|| (d0>=0x8000 && d0<0x9000)*/) continue;

			l9 = L9WORD(startFile + i + 6 + 9 * 2);
			if (l9 < 0x8000 || l9 >= 0x8000 + LISTAREASIZE) continue;

			Size = 0;
			Min = Max = i + d0;
			if (ValidateSequence(startFile, Image, i + d0, i + d0, &Size, size, &Min, &Max, false, &JumpKill, nullptr)) {
#ifdef L9DEBUG
				printf("Found valid V2 header at %ld, code size %ld", i, Size);
#endif
				if (Size > MaxSize && Size > 100) {
					offset = i;
					MaxSize = Size;
				}
			}
		}
	}
	free(Chk);
	free(Image);
	return offset;
}

long Scanner::ScanV1(byte *startFile, uint32 size) {
	uint32 i, Size;
	int Replace;
	byte *ImagePtr;
	long MaxPos = -1;
	uint32 MaxCount = 0;
	uint32 Min, Max; //, MaxMax, MaxMin;
	bool JumpKill; // , MaxJK;

	int dictOff1 = 0, dictOff2 = 0;
	byte dictVal1 = 0xff, dictVal2 = 0xff;

	if (size < 20)
		return -1;

	byte *Image = (byte *)calloc(size, 1);
	if (Image == nullptr) {
		error("Unable to allocate memory for game scan! Exiting...");
	}

	for (i = 0; i < size; i++) {
		if ((startFile[i] == 0 && startFile[i + 1] == 6) || (startFile[i] == 32 && startFile[i + 1] == 4)) {
			Size = 0;
			Min = Max = i;
			Replace = 0;
			if (ValidateSequence(startFile, Image, i, i, &Size, size, &Min, &Max, false, &JumpKill, nullptr)) {
				if (Size > MaxCount && Size > 100 && Size < 10000) {
					MaxCount = Size;
					//MaxMin = Min;
					//MaxMax = Max;

					MaxPos = i;
					//MaxJK = JumpKill;
				}
				Replace = 0;
			}
			for (ImagePtr = Image + Min; ImagePtr <= Image + Max; ImagePtr++) {
				if (*ImagePtr == 2)
					*ImagePtr = Replace;
			}
		}
	}

	/* V1 dictionary detection from L9Cut by Paul David Doherty */
	for (i = 0; i < size - 20; i++) {
		if (startFile[i] == 'A') {
			if (startFile[i + 1] == 'T' && startFile[i + 2] == 'T' && startFile[i + 3] == 'A' && startFile[i + 4] == 'C' && startFile[i + 5] == 0xcb) {
				dictOff1 = i;
				dictVal1 = startFile[dictOff1 + 6];
				break;
			}
		}
	}
	for (i = dictOff1; i < size - 20; i++) {
		if (startFile[i] == 'B') {
			if (startFile[i + 1] == 'U' && startFile[i + 2] == 'N' && startFile[i + 3] == 'C' && startFile[i + 4] == 0xc8) {
				dictOff2 = i;
				dictVal2 = startFile[dictOff2 + 5];
				break;
			}
		}
	}
	_l9V1Game = -1;
	if (_dictData && (dictVal1 != 0xff || dictVal2 != 0xff)) {
		for (i = 0; i < sizeof L9_V1_GAMES / sizeof L9_V1_GAMES[0]; i++) {
			if ((L9_V1_GAMES[i].dictVal1 == dictVal1) && (L9_V1_GAMES[i].dictVal2 == dictVal2)) {
				_l9V1Game = i;
				(*_dictData) = startFile + dictOff1 - L9_V1_GAMES[i].dictStart;
			}
		}
	}

	free(Image);

	if (MaxPos > 0 && _aCodePtr) {
		(*_aCodePtr) = startFile + MaxPos;
		return 0;
	}

	return -1;
}

bool Scanner::ValidateSequence(byte *Base, byte *Image, uint32 iPos, uint32 acode, uint32 *Size, uint32 size, uint32 *Min, uint32 *Max, bool Rts, bool *JumpKill, bool *DriverV4) {
	uint32 Pos;
	bool Finished = false, Valid;
	uint32 Strange = 0;
	int ScanCodeMask;
	int Code;
	*JumpKill = false;

	if (iPos >= size)
		return false;
	Pos = iPos;
	if (Pos < *Min) *Min = Pos;

	if (Image[Pos]) return true; /* hit valid code */

	do {
		Code = Base[Pos];
		Valid = true;
		if (Image[Pos]) break; /* converged to found code */
		Image[Pos++] = 2;
		if (Pos > *Max) *Max = Pos;

		ScanCodeMask = 0x9f;
		if (Code & 0x80) {
			ScanCodeMask = 0xff;
			if ((Code & 0x1f) > 0xa)
				Valid = false;
			Pos += 2;
		}
		else switch (Code & 0x1f) {
		case 0: { /* goto */
			uint32 Val = scangetaddr(Code, Base, &Pos, acode, &ScanCodeMask);
			Valid = ValidateSequence(Base, Image, Val, acode, Size, size, Min, Max, true/*Rts*/, JumpKill, DriverV4);
			Finished = true;
			break;
		}
		case 1: { /* intgosub */
			uint32 Val = scangetaddr(Code, Base, &Pos, acode, &ScanCodeMask);
			Valid = ValidateSequence(Base, Image, Val, acode, Size, size, Min, Max, true, JumpKill, DriverV4);
			break;
		}
		case 2: /* intreturn */
			Valid = Rts;
			Finished = true;
			break;
		case 3: /* printnumber */
			Pos++;
			break;
		case 4: /* messagev */
			Pos++;
			break;
		case 5: /* messagec */
			scangetcon(Code, &Pos, &ScanCodeMask);
			break;
		case 6: /* function */
			switch (Base[Pos++]) {
			case 2:/* random */
				Pos++;
				break;
			case 1:/* calldriver */
				if (DriverV4) {
					if (CheckCallDriverV4(Base, Pos - 2))
						*DriverV4 = true;
				}
				break;
			case 3:/* save */
			case 4:/* restore */
			case 5:/* clearworkspace */
			case 6:/* clear stack */
				break;
			case 250: /* printstr */
				while (Base[Pos++]);
				break;

			default:
				Valid = false;
				break;
			}
			break;
		case 7: /* input */
			Pos += 4;
			break;
		case 8: /* varcon */
			scangetcon(Code, &Pos, &ScanCodeMask);
			Pos++;
			break;
		case 9: /* varvar */
			Pos += 2;
			break;
		case 10: /* _add */
			Pos += 2;
			break;
		case 11: /* _sub */
			Pos += 2;
			break;
		case 14: /* jump */
			*JumpKill = true;
			Finished = true;
			break;
		case 15: /* exit */
			Pos += 4;
			break;
		case 16: /* ifeqvt */
		case 17: /* ifnevt */
		case 18: /* ifltvt */
		case 19: { /* ifgtvt */
			uint32 Val;
			Pos += 2;
			Val = scangetaddr(Code, Base, &Pos, acode, &ScanCodeMask);
			Valid = ValidateSequence(Base, Image, Val, acode, Size, size, Min, Max, Rts, JumpKill, DriverV4);
			break;
		}
		case 20: /* screen */
			if (Base[Pos++]) Pos++;
			break;
		case 21: /* cleartg */
			Pos++;
			break;
		case 22: /* picture */
			Pos++;
			break;
		case 23: /* getnextobject */
			Pos += 6;
			break;
		case 24: /* ifeqct */
		case 25: /* ifnect */
		case 26: /* ifltct */
		case 27: { /* ifgtct */
			uint32 Val;
			Pos++;
			scangetcon(Code, &Pos, &ScanCodeMask);
			Val = scangetaddr(Code, Base, &Pos, acode, &ScanCodeMask);
			Valid = ValidateSequence(Base, Image, Val, acode, Size, size, Min, Max, Rts, JumpKill, DriverV4);
			break;
		}
		case 28: /* printinput */
			break;
		case 12: /* ilins */
		case 13: /* ilins */
		case 29: /* ilins */
		case 30: /* ilins */
		case 31: /* ilins */
			Valid = false;
			break;
		}
		if (Valid && (Code & ~ScanCodeMask))
			Strange++;
	} while (Valid && !Finished && Pos < size); /* && Strange==0); */
	(*Size) += Pos - iPos;
	return Valid; /* && Strange==0; */
}

uint16 Scanner::scanmovewa5d0(byte *Base, uint32 *Pos) {
	uint16 ret = L9WORD(Base + *Pos);
	(*Pos) += 2;
	return ret;
}

uint32 Scanner::scangetaddr(int Code, byte *Base, uint32 *Pos, uint32 acode, int *Mask) {
	(*Mask) |= 0x20;
	if (Code & 0x20) {
		/* getaddrshort */
		signed char diff = Base[*Pos];
		(*Pos)++;
		return (*Pos) + diff - 1;
	} else {
		return acode + scanmovewa5d0(Base, Pos);
	}
}

void Scanner::scangetcon(int Code, uint32 *Pos, int *Mask) {
	(*Pos)++;
	if (!(Code & 64))(*Pos)++;
	(*Mask) |= 0x40;
}

bool Scanner::CheckCallDriverV4(byte *Base, uint32 Pos) {
	int i, j;

	// Look back for an assignment from a variable to list9[0], which is used
	// to specify the driver call.
	for (i = 0; i < 2; i++) {
		int x = Pos - ((i + 1) * 3);
		if ((Base[x] == 0x89) && (Base[x + 1] == 0x00)) {
			// Get the variable being copied to list9[0]
			int var = Base[x + 2];

			// Look back for an assignment to the variable
			for (j = 0; j < 2; j++) {
				int y = x - ((j + 1) * 3);
				if ((Base[y] == 0x48) && (Base[y + 2] == var)) {
					// If this a V4 driver call?
					switch (Base[y + 1]) {
					case 0x0E:
					case 0x20:
					case 0x22:
						return TRUE;
					}
					return FALSE;
				}
			}
		}
	}
	return FALSE;
}

#ifdef FULLSCAN
void Scanner::fullScan(byte *startFile, uint32 size) {
	byte *Image = (byte *)calloc(size, 1);
	uint32 i, Size;
	int Replace;
	byte *ImagePtr;
	uint32 MaxPos = 0;
	uint32 MaxCount = 0;
	uint32 Min, Max, MaxMin, MaxMax;
	int offset;
	bool JumpKill, MaxJK;
	for (i = 0; i < size; i++) {
		Size = 0;
		Min = Max = i;
		Replace = 0;
		if (ValidateSequence(startFile, Image, i, i, &Size, size, &Min, &Max, FALSE, &JumpKill, nullptr)) {
			if (Size > MaxCount) {
				MaxCount = Size;
				MaxMin = Min;
				MaxMax = Max;

				MaxPos = i;
				MaxJK = JumpKill;
			}
			Replace = 0;
		}
		for (ImagePtr = Image + Min; ImagePtr <= Image + Max; ImagePtr++) {
			if (*ImagePtr == 2)
				*ImagePtr = Replace;
		}
	}
	printf("%ld %ld %ld %ld %s", MaxPos, MaxCount, MaxMin, MaxMax, MaxJK ? "jmp killed" : "");
	/* search for reference to MaxPos */
	offset = 0x12 + 11 * 2;
	for (i = 0; i < size - offset - 1; i++) {
		if ((L9WORD(startFile + i + offset)) + i == MaxPos) {
			printf("possible v3,4 Code reference at : %ld", i);
			/* startdata=startFile+i; */
		}
	}
	offset = 13 * 2;
	for (i = 0; i < size - offset - 1; i++) {
		if ((L9WORD(startFile + i + offset)) + i == MaxPos)
			printf("possible v2 Code reference at : %ld", i);
	}
	free(Image);
}
#endif

/*----------------------------------------------------------------------*/

GameDetection::GameDetection(byte *&startData, uint32 &fileSize) :
		_startData(startData), _fileSize(fileSize), _crcInitialized(false), _gameName(nullptr) {
	Common::fill(&_crcTable[0], &_crcTable[256], 0);
}

gln_game_tableref_t GameDetection::gln_gameid_identify_game() {
	uint16 length, crc;
	byte checksum;
	int is_version2;
	gln_game_tableref_t game;
	gln_patch_tableref_t patch;

	/* If the data file appears too short for a header, give up now. */
	if (_fileSize < 30)
		return nullptr;

	/*
	 * Find the version of the game, and the length of game data.  This logic
	 * is taken from L9cut, with calcword() replaced by simple byte comparisons.
	 * If the length exceeds the available data, fail.
	 */
	assert(_startData);
	is_version2 = _startData[4] == 0x20 && _startData[5] == 0x00
		&& _startData[10] == 0x00 && _startData[11] == 0x80
		&& _startData[20] == _startData[22]
		&& _startData[21] == _startData[23];

	length = is_version2
		? _startData[28] | _startData[29] << BITS_PER_BYTE
		: _startData[0] | _startData[1] << BITS_PER_BYTE;
	if (length >= _fileSize)
		return nullptr;

	/* Calculate or retrieve the checksum, in a version specific way. */
	if (is_version2) {
		int index;

		checksum = 0;
		for (index = 0; index < length + 1; index++)
			checksum += _startData[index];
	}
	else
		checksum = _startData[length];

	/*
	 * Generate a CRC for this data.  When L9cut calculates a CRC, it's using a
	 * copy taken up to length + 1 and then padded with two NUL bytes, so we
	 * mimic that here.
	 */
	crc = gln_get_buffer_crc(_startData, length + 1, 2);

	/*
	 * See if this is a patched file.  If it is, look up the game based on the
	 * original CRC and checksum.  If not, use the current CRC and checksum.
	 */
	patch = gln_gameid_lookup_patch(length, checksum, crc);
	game = gln_gameid_lookup_game(length,
		patch ? patch->orig_checksum : checksum,
		patch ? patch->orig_crc : crc,
		false);

	/* If no game identified, retry without the CRC.  This is guesswork. */
	if (!game)
		game = gln_gameid_lookup_game(length, checksum, crc, true);

	return game;
}

// CRC table initialization polynomial
static const uint16 GLN_CRC_POLYNOMIAL = 0xa001;

uint16 GameDetection::gln_get_buffer_crc(const void *void_buffer, size_t length, size_t padding) {
	const char *buffer = (const char *)void_buffer;
	uint16 crc;
	size_t index;

	/* Build the static CRC lookup table on first call. */
	if (!_crcInitialized) {
		for (index = 0; index < BYTE_MAX + 1; index++) {
			int bit;

			crc = (uint16)index;
			for (bit = 0; bit < BITS_PER_BYTE; bit++)
				crc = crc & 1 ? GLN_CRC_POLYNOMIAL ^ (crc >> 1) : crc >> 1;

			_crcTable[index] = crc;
		}

		_crcInitialized = true;

		/* CRC lookup table self-test, after is_initialized set -- recursion. */
		assert(gln_get_buffer_crc("123456789", 9, 0) == 0xbb3d);
	}

	/* Start with zero in the crc, then update using table entries. */
	crc = 0;
	for (index = 0; index < length; index++)
		crc = _crcTable[(crc ^ buffer[index]) & BYTE_MAX] ^ (crc >> BITS_PER_BYTE);

	/* Add in any requested NUL padding bytes. */
	for (index = 0; index < padding; index++)
		crc = _crcTable[crc & BYTE_MAX] ^ (crc >> BITS_PER_BYTE);

	return crc;
}

gln_game_tableref_t GameDetection::gln_gameid_lookup_game(uint16 length, byte checksum, uint16 crc, int ignore_crc) const {
	gln_game_tableref_t game;

	for (game = GLN_GAME_TABLE; game->length; game++) {
		if (game->length == length && game->checksum == checksum
			&& (ignore_crc || game->crc == crc))
			break;
	}

	return game->length ? game : nullptr;
}

gln_patch_tableref_t GameDetection::gln_gameid_lookup_patch(uint16 length, byte checksum, uint16 crc) const {
	gln_patch_tableref_t patch;

	for (patch = GLN_PATCH_TABLE; patch->length; patch++) {
		if (patch->length == length && patch->patch_checksum == checksum
			&& patch->patch_crc == crc)
			break;
	}

	return patch->length ? patch : nullptr;
}

const char *GameDetection::gln_gameid_get_game_name() {
	/*
	 * If no game name yet known, attempt to identify the game.  If it can't
	 * be identified, set the cached game name to "" -- this special value
	 * indicates that the game is an unknown one, but suppresses repeated
	 * attempts to identify it on successive calls.
	 */
	if (!_gameName) {
		gln_game_tableref_t game;

		/*
		 * If the interpreter hasn't yet loaded a game, startdata is nullptr
		 * (uninitialized, global).  In this case, we return nullptr, allowing
		 * for retries until a game is loaded.
		 */
		if (!_startData)
			return nullptr;

		game = gln_gameid_identify_game();
		_gameName = game ? game->name : "";
	}

	/* Return the game's name, or nullptr if it was unidentifiable. */
	assert(_gameName);
	return strlen(_gameName) > 0 ? _gameName : nullptr;
}

/**
 * Clear the saved game name, forcing a new lookup when next queried.  This
 * function should be called by actions that may cause the interpreter to
 * change game file, for example os_set_filenumber().
 */
void GameDetection::gln_gameid_game_name_reset() {
	_gameName = nullptr;
}

/*----------------------------------------------------------------------*/

void Level9MetaEngine::getSupportedGames(PlainGameList &games) {
	const char *prior_id = nullptr;

	for (const gln_game_table_t *pd = GLN_GAME_TABLE; pd->name; ++pd) {
		if (prior_id == nullptr || strcmp(pd->gameId, prior_id)) {
			PlainGameDescriptor gd;
			gd.gameId = pd->gameId;
			gd.description = pd->name;
			games.push_back(gd);

			prior_id = pd->gameId;
		}
	}
}

GameDescriptor Level9MetaEngine::findGame(const char *gameId) {
	for (const gln_game_table_t *pd = GLN_GAME_TABLE; pd->gameId; ++pd) {
		if (!strcmp(gameId, pd->gameId)) {
			GameDescriptor gd(pd->gameId, pd->name, 0);
			return gd;
		}
	}

	return PlainGameDescriptor();
}

bool Level9MetaEngine::detectGames(const Common::FSList &fslist, DetectedGames &gameList) {
	// Loop through the files of the folder
	for (Common::FSList::const_iterator file = fslist.begin(); file != fslist.end(); ++file) {
		// Check for a recognised filename
		if (file->isDirectory())
			continue;
		Common::String filename = file->getName();
		if (!filename.hasSuffixIgnoreCase(".l9") && !filename.hasSuffixIgnoreCase(".dat"))
			continue;

		// Open up the file so we can get it's size
		Common::File gameFile;
		if (!gameFile.open(*file))
			continue;

		uint32 fileSize = gameFile.size();
		if (fileSize == 0 || fileSize > 0xffff) {
			// Too big or too small to possibly be a Level 9 game
			gameFile.close();
			continue;
		}

		// Read in the game data
		Common::Array<byte> data;
		data.resize(fileSize + 1);
		gameFile.read(&data[0], fileSize);
		gameFile.close();

		// Check if it's a valid Level 9 game
		byte *startFile = &data[0];
		Scanner scanner;
		int offset = scanner.scanner(&data[0], fileSize) < 0;
		if (offset < 0)
			continue;

		// Check for the specific game
		byte *startData = startFile + offset;
		GameDetection detection(startData, fileSize);

		const gln_game_tableref_t game = detection.gln_gameid_identify_game();
		if (!game)
			continue;

		// Found the game, add a detection entry
		DetectedGame gd = DetectedGame("glk", game->gameId, game->name, Common::UNK_LANG,
			Common::kPlatformUnknown, game->extra);
		gd.addExtraEntry("filename", filename);
		gameList.push_back(gd);
	}

	return !gameList.empty();
}

void Level9MetaEngine::detectClashes(Common::StringMap &map) {
	const char *prior_id = nullptr;

	for (const gln_game_table_t *pd = GLN_GAME_TABLE; pd->name; ++pd) {
		if (prior_id == nullptr || strcmp(pd->gameId, prior_id)) {
			prior_id = pd->gameId;

			if (map.contains(pd->gameId))
				error("Duplicate game Id found - %s", pd->gameId);
			map[pd->gameId] = "";
		}
	}
}

} // End of namespace Level9
} // End of namespace Glk
