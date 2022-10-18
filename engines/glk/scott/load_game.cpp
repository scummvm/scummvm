/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/*
 * Based on ScottFree interpreter version 1.14 developed by Swansea
 * University Computer Society without disassembly of any other game
 * drivers, only of game databases as permitted by EEC law (for purposes
 * of compatibility).
 *
 * Licensed under GPLv2
 *
 * https://github.com/angstsmurf/spatterlight/tree/master/terps/scott
 */

#include "common/str.h"
#include "glk/scott/scott.h"
#include "glk/scott/globals.h"
#include "glk/scott/command_parser.h"
#include "glk/scott/decompress_text.h"
#include "glk/scott/decompress_z80.h"
#include "glk/scott/detection.h"
#include "glk/scott/detection_tables.h"
#include "glk/scott/game_info.h"
#include "glk/scott/hulk.h"
#include "glk/scott/line_drawing.h"
#include "glk/scott/saga_draw.h"
#include "glk/scott/c64_checksums.h"
#include "glk/scott/game_specific.h"
#include "glk/scott/resource.h"
#include "glk/scott/load_game.h"
#include "glk/scott/robin_of_sherwood.h"
#include "glk/scott/gremlins.h"
#include "glk/scott/seas_of_blood.h"
#include "glk/scott/load_ti99_4a.h"

namespace Glk {
namespace Scott {

void loadZXSpectrum(Common::SeekableReadStream *f, Common::String md5) {
	_G(_entireFile) = new uint8_t[_G(_fileLength)];
	size_t result = f->read(_G(_entireFile), _G(_fileLength));
	if (result != _G(_fileLength))
		g_scott->fatal("File empty or read error!");

	uint8_t *uncompressed = decompressZ80(_G(_entireFile), _G(_fileLength));
	if (uncompressed != nullptr) {
		delete[] _G(_entireFile);
		_G(_entireFile) = uncompressed;
		_G(_fileLength) = 0xc000;
	}

	int offset;
	DictionaryType dict_type = getId(&offset);
	if (dict_type == NOT_A_GAME)
		return;

	int index = _G(_md5Index)[md5];
	if (tryLoading(_G(_games)[index], offset, 0)) {
		_G(_game) = &_G(_games)[index];
	}
}

void loadC64(Common::SeekableReadStream* f, Common::String md5) {
	_G(_entireFile) = new uint8_t[_G(_fileLength)];
	size_t result = f->read(_G(_entireFile), _G(_fileLength));
	if (result != _G(_fileLength))
		g_scott->fatal("File empty or read error!");

	_G(_fallbackGame)._gameID = static_cast<GameIDType>(detectC64(&_G(_entireFile), &_G(_fileLength)));
}

void loadTI994A(Common::SeekableReadStream *f) {
	_G(_entireFile) = new uint8_t[_G(_fileLength)];
	size_t result = f->read(_G(_entireFile), _G(_fileLength));
	if (result != _G(_fileLength))
		g_scott->fatal("File empty or read error!");

	_G(_fallbackGame)._gameID = detectTI994A(f, &_G(_entireFile), &_G(_fileLength));
}

void loadGameFile(Common::SeekableReadStream *f) {

	for (int i = 0; i < NUMBER_OF_DIRECTIONS; i++)
		_G(_directions)[i] = _G(_englishDirections)[i];
	for (int i = 0; i < NUMBER_OF_SKIPPABLE_WORDS; i++)
		_G(_skipList)[i] = _G(_englishSkipList)[i];
	for (int i = 0; i < NUMBER_OF_DELIMITERS; i++)
		_G(_delimiterList)[i] = _G(_englishDelimiterList)[i];
	for (int i = 0; i < NUMBER_OF_EXTRA_NOUNS; i++)
		_G(_extraNouns)[i] = _G(_englishExtraNouns)[i];

	_G(_fileLength) = f->size();

	_G(_game) = &_G(_fallbackGame);

	Common::String md5 = g_vm->getGameMD5();
	const GlkDetectionEntry *p = SCOTT_GAMES;

	while (p->_md5) {
		if (md5.equalsC(p->_md5)) {
			if (!scumm_stricmp(p->_extra, "")) {
				_G(_fallbackGame)._gameID = SCOTTFREE;
				break;
			} else if (!scumm_stricmp(p->_extra, "ZXSpectrum")) {
				loadZXSpectrum(f, md5);
				break;
			} else if (!scumm_stricmp(p->_extra, "C64")) {
				loadC64(f, md5);
				break;
			} else {
				loadTI994A(f);
				break;
			}
		}
		++p;
	}

	if (CURRENT_GAME == SCOTTFREE || CURRENT_GAME == TI994A)
		return;

	/* Copy ZX Spectrum style system messages as base */
	for (int i = 6; i < MAX_SYSMESS && g_sysDictZX[i] != nullptr; i++) {
		_G(_sys)[i] = g_sysDictZX[i];
	}

	switch (CURRENT_GAME) {
	case ROBIN_OF_SHERWOOD:
		loadExtraSherwoodData();
		break;
	case ROBIN_OF_SHERWOOD_C64:
		loadExtraSherwoodData64();
		break;
	case SEAS_OF_BLOOD:
		loadExtraSeasOfBloodData();
		break;
	case SEAS_OF_BLOOD_C64:
		loadExtraSeasOfBlood64Data();
		break;
	case CLAYMORGUE:
		for (int i = OK; i <= RESUME_A_SAVED_GAME; i++)
			_G(_sys)[i] = _G(_systemMessages)[6 - OK + i];
		for (int i = PLAY_AGAIN; i <= ON_A_SCALE_THAT_RATES; i++)
			_G(_sys)[i] = _G(_systemMessages)[2 - PLAY_AGAIN + i];
		break;
	case ADVENTURELAND:
		for (int i = PLAY_AGAIN; i <= ON_A_SCALE_THAT_RATES; i++)
			_G(_sys)[i] = _G(_systemMessages)[2 - PLAY_AGAIN + i];
		for (int i = OK; i <= YOU_HAVENT_GOT_IT; i++)
			_G(_sys)[i] = _G(_systemMessages)[6 - OK + i];
		for (int i = YOU_DONT_SEE_IT; i <= RESUME_A_SAVED_GAME; i++)
			_G(_sys)[i] = _G(_systemMessages)[13 - YOU_DONT_SEE_IT + i];
		break;
	case ADVENTURELAND_C64:
		adventureland64Sysmess();
		break;
	case CLAYMORGUE_C64:
		claymorgue64Sysmess();
		break;
	case GREMLINS_GERMAN_C64:
		loadExtraGermanGremlinsc64Data();
		break;
	case SPIDERMAN_C64:
		spiderman64Sysmess();
		break;
	case SUPERGRAN_C64:
		supergran64Sysmess();
		break;
	case SAVAGE_ISLAND_C64:
		_G(_items)[20]._image = 13;
		// fallthrough
	case SAVAGE_ISLAND2_C64:
		_G(_sys)[IM_DEAD] = "I'm DEAD!! ";
		if (CURRENT_GAME == SAVAGE_ISLAND2_C64)
			_G(_rooms)[30]._image = 20;
		break;
	case SAVAGE_ISLAND:
		_G(_items)[20]._image = 13;
		// fallthrough
	case SAVAGE_ISLAND2:
		MY_LOC = 30; /* Both parts of Savage Island begin in room 30 */
		// fallthrough
	case GREMLINS_GERMAN:
	case GREMLINS:
	case SUPERGRAN:
		for (int i = DROPPED; i <= OK; i++)
			_G(_sys)[i] = _G(_systemMessages)[2 - DROPPED + i];
		for (int i = I_DONT_UNDERSTAND; i <= THATS_BEYOND_MY_POWER; i++)
			_G(_sys)[i] = _G(_systemMessages)[6 - I_DONT_UNDERSTAND + i];
		for (int i = YOU_ARE; i <= HIT_ENTER; i++)
			_G(_sys)[i] = _G(_systemMessages)[17 - YOU_ARE + i];
		_G(_sys)[PLAY_AGAIN] = _G(_systemMessages)[5];
		_G(_sys)[YOURE_CARRYING_TOO_MUCH] = _G(_systemMessages)[24];
		_G(_sys)[IM_DEAD] = _G(_systemMessages)[25];
		_G(_sys)[YOU_CANT_GO_THAT_WAY] = _G(_systemMessages)[14];
		break;
	case GREMLINS_SPANISH:
		loadExtraSpanishGremlinsData();
		break;
	case HULK_C64:
	case HULK:
		for (int i = 0; i < 6; i++) {
			_G(_sys)[i] = g_sysDictZX[i];
		}
		break;
	default:
		if (!(_G(_game)->_subType & C64)) {
			if (_G(_game)->_subType & MYSTERIOUS) {
				for (int i = PLAY_AGAIN; i <= YOU_HAVENT_GOT_IT; i++)
					_G(_sys)[i] = _G(_systemMessages)[2 - PLAY_AGAIN + i];
				for (int i = YOU_DONT_SEE_IT; i <= WHAT_NOW; i++)
					_G(_sys)[i] = _G(_systemMessages)[15 - YOU_DONT_SEE_IT + i];
				for (int i = LIGHT_HAS_RUN_OUT; i <= RESUME_A_SAVED_GAME; i++)
					_G(_sys)[i] = _G(_systemMessages)[31 - LIGHT_HAS_RUN_OUT + i];
				_G(_sys)[ITEM_DELIMITER] = " - ";
				_G(_sys)[MESSAGE_DELIMITER] = "\n";
				_G(_sys)[YOU_SEE] = "\nThings I can see:\n";
				break;
			} else {
				for (int i = PLAY_AGAIN; i <= RESUME_A_SAVED_GAME; i++)
					_G(_sys)[i] = _G(_systemMessages)[2 - PLAY_AGAIN + i];
			}
		}
		break;
	}

	switch (CURRENT_GAME) {
	case GREMLINS_GERMAN:
		loadExtraGermanGremlinsData();
		break;
	case GREMLINS_GERMAN_C64:
		loadExtraGermanGremlinsc64Data();
		break;
	default:
		break;
	}

	if (!(_G(_game)->_subType & (C64 | MYSTERIOUS))) {
		mysterious64Sysmess();
	}

	/* If it is a C64 or a Mysterious Adventures game, we have setup the graphics already */
	if (!(_G(_game)->_subType & (C64 | MYSTERIOUS)) && _G(_game)->_numberOfPictures > 0) {
		sagaSetup(0);
	}
}

} // End of namespace Scott
} // End of namespace Glk
