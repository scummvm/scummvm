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

#ifndef GLK_LEVEL9_DETECTION
#define GLK_LEVEL9_DETECTION

#include "common/fs.h"
#include "common/hash-str.h"
#include "engines/game.h"
#include "glk/detection.h"

namespace Glk {
namespace Level9 {

enum L9GameTypes { L9_V1, L9_V2, L9_V3, L9_V4 };

struct gln_game_table_t {
	const size_t length;        ///< Datafile length in bytes
	const byte checksum;        ///< 8-bit checksum, last datafile byte
	const uint16 crc;           ///< 16-bit CRC, L9cut-internal
	const char *const gameId;   ///< Game Id
	const char *const name;     ///< Game title
	const char *const extra;    ///< Disk number, platform, etc.
};
typedef const gln_game_table_t *gln_game_tableref_t;

struct gln_patch_table_t {
	const size_t length;        ///< Datafile length in bytes
	const byte orig_checksum;   ///< 8-bit checksum, last datafile byte
	const uint16 orig_crc;      ///< 16-bit CRC, L9cut-internal
	const byte patch_checksum;  ///< 8-bit checksum, last datafile byte
	const uint16 patch_crc;     ///< 16-bit CRC, L9cut-internal
};
typedef const gln_patch_table_t *gln_patch_tableref_t;

struct L9V1GameInfo {
	byte dictVal1, dictVal2;
	int dictStart, L9Ptrs[5], absData, msgStart, msgLen;
};

/**
 * Scanner for game data
 */
class Scanner {
private:
	long scan(byte *StartFile, uint32 size);
	long ScanV2(byte *StartFile, uint32 size);
	long ScanV1(byte *StartFile, uint32 size);

	bool ValidateSequence(byte *Base, byte *Image, uint32 iPos, uint32 acode, uint32 *Size, uint32 size, uint32 *Min, uint32 *Max, bool Rts, bool *JumpKill, bool *DriverV4);
	uint16 scanmovewa5d0(byte *Base, uint32 *Pos);
	uint32 scangetaddr(int Code, byte *Base, uint32 *Pos, uint32 acode, int *Mask);
	void scangetcon(int Code, uint32 *Pos, int *Mask);
	bool CheckCallDriverV4(byte *Base, uint32 Pos);
#ifdef FULLSCAN
	void Scanner::fullScan(byte *StartFile, uint32 size);
#endif
private:
	byte **_dictData;
	byte **_aCodePtr;
public:
	L9GameTypes _gameType;
	int _l9V1Game;
public:
	Scanner() : _dictData(nullptr), _aCodePtr(nullptr), _gameType(L9_V1), _l9V1Game(-1) {}

	/**
	 * Scan passed file for a valid game, and if found return it's offset
	 */
	long scanner(byte *StartFile, uint32 size, byte **dictData = nullptr,
		byte **aCodePtr = nullptr);

	/**
	 * Returns the info for a V1 game
	 */
	const L9V1GameInfo &v1Game() const;
};

/**
 * Detection manager for specific games
 */
class GameDetection : public Scanner {
private:
	byte *&_startData;
	uint32 &_fileSize;
	bool _crcInitialized;
	uint16 _crcTable[256];
public:
	const char *_gameName;
public:
	/**
	 * Constructor
	 */
	GameDetection(byte *&startData, uint32 &fileSize);

	/**
	 * Identify a game from its data length, checksum, and CRC.  Returns the
	 * entry of the game in the game table, or nullptr if not found.
	 *
	 * This function uses startdata and FileSize from the core interpreter.
	 * These aren't advertised symbols, so be warned.
	 */
	gln_game_tableref_t gln_gameid_identify_game();

	/**
	 * Return the CRC of the bytes buffer[0..length-1].
	 *
	 * This algorithm is selected to match the CRCs used in L9cut.  Because of
	 * the odd way CRCs are padded when L9cut calculates the CRC, this function
	 * allows a count of NUL padding bytes to be included within the return CRC.
	 */
	uint16 gln_get_buffer_crc(const void *void_buffer, size_t length, size_t padding = 0);

	/*
	 * Look up and return game table and patch table entries given a game's
	 * length, checksum, and CRC.  Returns the entry, or nullptr if not found.
	 */
	gln_game_tableref_t gln_gameid_lookup_game(uint16 length, byte checksum, uint16 crc, int ignore_crc) const;

	/**
	 * Look up and return patch table entries given a game's length, checksum, and CRC.
	 * Returns the entry, or nullptr if not found
	 */
	gln_patch_tableref_t gln_gameid_lookup_patch(uint16 length, byte checksum, uint16 crc) const;

	/**
	 * Return the name of the game, or nullptr if not identifiable.
	 *
	 * This function uses startdata from the core interpreter.  This isn't an
	 * advertised symbol, so be warned.
	 */
	const char *gln_gameid_get_game_name();

	/**
	 * Clear the saved game name, forcing a new lookup when next queried.  This
	 * function should be called by actions that may cause the interpreter to
	 * change game file, for example os_set_filenumber().
	 */
	void gln_gameid_game_name_reset();
};

/**
 * Meta engine for Level 9 interpreter
 */
class Level9MetaEngine {
public:
	/**
	 * Get a list of supported games
	 */
	static void getSupportedGames(PlainGameList &games);

	/**
	 * Returns a game description for the given game Id, if it's supported
	 */
	static GameDescriptor findGame(const char *gameId);

	/**
	 * Detect supported games
	 */
	static bool detectGames(const Common::FSList &fslist, DetectedGames &gameList);

	/**
	 * Check for game Id clashes with other sub-engines
	 */
	static void detectClashes(Common::StringMap &map);
};

} // End of namespace Level9
} // End of namespace Glk

#endif
