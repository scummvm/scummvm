#include "detect_game.h"
#include "scott.h"
#include "game_info.h"
#include "saga_draw.h"
#include "line_drawing.h"
#include "hulk.h"
#include "decompress_z80.h"
#include "globals.h"

namespace Glk {
namespace Scott {



GameIDType detectGame(Common::SeekableReadStream *f) {

	//TODO
	//for (int i = 0; i < NUMBER_OF_DIRECTIONS; i++)
	//	Directions[i] = EnglishDirections[i];
	//for (int i = 0; i < NUMBER_OF_SKIPPABLE_WORDS; i++)
	//	SkipList[i] = EnglishSkipList[i];
	//for (int i = 0; i < NUMBER_OF_DELIMITERS; i++)
	//	DelimiterList[i] = EnglishDelimiterList[i];
	//for (int i = 0; i < NUMBER_OF_EXTRA_NOUNS; i++)
	//	ExtraNouns[i] = EnglishExtraNouns[i];

	_G(_fileLength) = f->size();

	_G(_game) = new GameInfo();

	//TODO
	//SCOTTFREE Detection

	_G(_entireFile) = new uint8_t[_G(_fileLength)];
	int result = f->read(_G(_entireFile), _G(_fileLength));
	if (result != _G(_fileLength))
		g_vm->fatal("File empty or read error!");
	//TODO
	//TI99/4A Detection
	//TODO
	//C64 Detection

	//ZXSpectrum Detection
	uint8_t *uncompressed = decompressZ80(_G(_entireFile), _G(_fileLength));
	if (uncompressed != NULL) {
		delete[] _G(_entireFile);
		_G(_entireFile) = uncompressed;
		_G(_fileLength) = 0xc000;
	}

	size_t offset;
	DictionaryType dict_type = getId(&offset);
	if (dict_type == NOT_A_GAME)
		return UNKNOWN_GAME;
	for (int i = 0; i < NUMGAMES; i++) {
		if (g_games[i]._dictionary == dict_type) {
			if (tryLoading(g_games[i], offset, 0)) {
				delete _G(_game);
				_G(_game) = &g_games[i];
				break;
			}
		}
	}

	if (CURRENT_GAME == SCOTTFREE || CURRENT_GAME == TI994A)
		return CURRENT_GAME;

	/* Copy ZX Spectrum style system messages as base */
	for (int i = 6; i < MAX_SYSMESS && g_sysDictZX[i] != nullptr; i++) {
		_G(_sys)[i] = g_sysDictZX[i];
	}

	switch (CURRENT_GAME) {
	case ROBIN_OF_SHERWOOD:
		//TODO
		break;
	case ROBIN_OF_SHERWOOD_C64:
		//TODO
		break;
	case SEAS_OF_BLOOD:
		//TODO
		break;
	case SEAS_OF_BLOOD_C64:
		//TODO
		break;
	case CLAYMORGUE:
		//TODO
		break;
	case SECRET_MISSION:
	case SECRET_MISSION_C64:
		//TODO
		break;
	case ADVENTURELAND:
		//TODO
		break;
	case ADVENTURELAND_C64:
		//TODO
		break;
	case CLAYMORGUE_C64:
		//TODO
		break;
	case GREMLINS_GERMAN_C64:
		//TODO
		break;
	case SPIDERMAN_C64:
		//TODO
		break;
	case SUPERGRAN_C64:
		//TODO
		break;
	case SAVAGE_ISLAND_C64:
	case SAVAGE_ISLAND2_C64:
		//TODO
		break;
	case SAVAGE_ISLAND:
	case SAVAGE_ISLAND2:
	case GREMLINS_GERMAN:
	case GREMLINS:
	case SUPERGRAN:
		//TODO
		break;
	case GREMLINS_SPANISH:
		//TODO
		break;
	case HULK_C64:
	case HULK:
		for (int i = 0; i < 6; i++) {
			_G(_sys)[i] = g_sysDictZX[i];
		}
		break;
	default:
		//TODO
		break;
	}

	switch (CURRENT_GAME) {
	case GREMLINS_GERMAN:
		//TODO
		break;
	case GREMLINS_GERMAN_C64:
		//TODO
		break;
	case PERSEUS_ITALIAN:
		//TODO
		break;
	default:
		break;
	}

	/* If it is a C64 or a Mysterious Adventures game, we have setup the graphics already */
	if (!(_G(_game)->_subType & (C64 | MYSTERIOUS)) && _G(_game)->_numberOfPictures > 0) {
		sagaSetup(0);
	}

	return CURRENT_GAME;
}

uint8_t *seekToPos(uint8_t *buf, int offset) {
	if (offset > _G(_fileLength))
		return 0;
	return buf + offset;
}

int seekIfNeeded(int expectedStart, int *offset, uint8_t **ptr) {
	return 0;
}

int tryLoading(GameInfo info, int dictStart, int loud) {
	return 0;
}

DictionaryType getId(size_t *offset) {
	return DictionaryType();
}

int findCode(const char *x, int base) {
	return 0;
}

} // End of namespace Scott
} // End of namespace Glk
