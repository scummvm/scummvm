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
#include "glk/level9/os_glk.h"
#include "glk/blorb.h"
#include "common/debug.h"
#include "common/file.h"
#include "common/md5.h"
#include "engines/game.h"

namespace Glk {
namespace Level9 {

GameDetection::GameDetection(byte *&startData, size_t &fileSize) :
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
		? _startData[28] | _startData[29] << BITS_PER_CHAR
		: _startData[0] | _startData[1] << BITS_PER_CHAR;
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
			for (bit = 0; bit < BITS_PER_CHAR; bit++)
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
		crc = _crcTable[(crc ^ buffer[index]) & BYTE_MAX] ^ (crc >> BITS_PER_CHAR);

	/* Add in any requested NUL padding bytes. */
	for (index = 0; index < padding; index++)
		crc = _crcTable[crc & BYTE_MAX] ^ (crc >> BITS_PER_CHAR);

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
	for (const PlainGameDescriptor *pd = LEVEL9_GAME_LIST; pd->gameId; ++pd) {
		games.push_back(*pd);
	}
}

GameDescriptor Level9MetaEngine::findGame(const char *gameId) {
	for (const PlainGameDescriptor *pd = LEVEL9_GAME_LIST; pd->gameId; ++pd) {
		if (!strcmp(gameId, pd->gameId))
			return *pd;
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
		if (!filename.hasSuffixIgnoreCase(".l9"))
			continue;

		// Open up the file and calculate the md5
		Common::File gameFile;
		if (!gameFile.open(*file))
			continue;

		Common::String md5 = Common::computeStreamMD5AsString(gameFile, 5000);
		size_t _fileSize = gameFile.size();
		gameFile.seek(0);
		bool isBlorb = Blorb::isBlorb(gameFile, ID_ADRI);
		gameFile.close();

		if (!isBlorb && Blorb::hasBlorbExt(filename))
			continue;

		// Check for known games
		const GlkDetectionEntry *p = LEVEL9_GAMES;
		while (p->_gameId && (md5 != p->_md5 || _fileSize != p->_filesize))
			++p;

		if (!p->_gameId) {
			const PlainGameDescriptor &desc = LEVEL9_GAME_LIST[0];
			gameList.push_back(GlkDetectedGame(desc.gameId, desc.description, filename, md5, _fileSize));
		} else {
			PlainGameDescriptor gameDesc = findGame(p->_gameId);
			gameList.push_back(GlkDetectedGame(p->_gameId, gameDesc.description, p->_extra, filename, p->_language));
		}
	}

	return !gameList.empty();
}

void Level9MetaEngine::detectClashes(Common::StringMap &map) {
	// No implementation
}

} // End of namespace Level9
} // End of namespace Glk
