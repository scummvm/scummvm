/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004-2006 The ScummVM project
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

#include "common/stdafx.h"
#include "common/endian.h"
#include "common/md5.h"
#include "kyra/kyra.h"
#include "kyra/kyra2.h"
#include "kyra/kyra3.h"
#include "kyra/screen.h"
#include "kyra/resource.h"

namespace Kyra {

#define RESFILE_VERSION 17

bool StaticResource::checkKyraDat() {
	Common::File kyraDat;
	if (!kyraDat.open("KYRA.DAT"))
		return false;
	
	uint32 size = kyraDat.size() - 16;
	uint8 digest[16];
	kyraDat.seek(size, SEEK_SET);
	if (kyraDat.read(digest, 16) != 16)
		return false;
	kyraDat.close();
	
	uint8 digestCalc[16];
	if (!Common::md5_file("KYRA.DAT", digestCalc, size))
		return false;

	for (int i = 0; i < 16; ++i)
		if (digest[i] != digestCalc[i])
			return false;
	return true;
}

// used for the KYRA.DAT file which still uses
// the old flag system, we just convert it, which
// is less work than to change KYRA.DAT again
enum {
	GF_FLOPPY	= 1 <<  0,
	GF_TALKIE	= 1 <<  1,
	GF_FMTOWNS	= 1 <<  2,
	GF_DEMO		= 1 <<  3,
	GF_ENGLISH	= 1 <<  4,
	GF_FRENCH	= 1 <<  5,
	GF_GERMAN	= 1 <<  6,
	GF_SPANISH	= 1 <<  7,
	GF_ITALIAN	= 1 <<  8,
	GF_JAPANESE = 1 <<  9,
	// other languages here
	GF_LNGUNK	= 1 << 16,	// also used for multi language in kyra3
	GF_AMIGA	= 1 << 17
};

#define GAME_FLAGS (GF_FLOPPY | GF_TALKIE | GF_DEMO | GF_FMTOWNS | GF_AMIGA)
#define LANGUAGE_FLAGS (GF_ENGLISH | GF_FRENCH | GF_GERMAN | GF_SPANISH | GF_ITALIAN | GF_JAPANESE | GF_LNGUNK)

uint32 createFeatures(const GameFlags &flags) {
	if (flags.isTalkie)
		return GF_TALKIE;
	if (flags.isDemo)
		return GF_DEMO;
	if (flags.platform == Common::kPlatformFMTowns)
		return GF_FMTOWNS;
	if (flags.platform == Common::kPlatformAmiga)
		return GF_AMIGA;
	return GF_FLOPPY;
}

uint32 createLanguage(const GameFlags &flags) {
	if (flags.lang == Common::EN_ANY)
		return GF_ENGLISH;
	if (flags.lang == Common::DE_DEU)
		return GF_GERMAN;
	if (flags.lang == Common::FR_FRA)
		return GF_FRENCH;
	if (flags.lang == Common::ES_ESP)
		return GF_SPANISH;
	if (flags.lang == Common::IT_ITA)
		return GF_ITALIAN;
	if (flags.lang == Common::JA_JPN)
		return GF_JAPANESE;
	return GF_LNGUNK;
}

struct LanguageTypes {
	uint32 flags;
	const char *ext;
};

static const LanguageTypes languages[] = {
	{ GF_ENGLISH, "ENG" },	// this is the default language
	{ GF_FRENCH, "FRE" },
	{ GF_GERMAN, "GER" },
	{ GF_SPANISH, "SPA" },
	{ GF_ITALIAN, "ITA" },
	{ GF_JAPANESE, "JPN" },
	{ 0, 0 }
};

bool StaticResource::init() {
#define proc(x) &StaticResource::x
	static const FileType fileTypeTable[] = {
		{ kLanguageList, proc(loadLanguageTable), proc(freeStringTable) },
		{ kStringList, proc(loadStringTable), proc(freeStringTable) },
		{ StaticResource::kRoomList, proc(loadRoomTable), proc(freeRoomTable) },
		{ kShapeList, proc(loadShapeTable), proc(freeShapeTable) },
		{ kRawData, proc(loadRawData), proc(freeRawData) },
		{ kPaletteTable, proc(loadPaletteTable), proc(freePaletteTable) },
		{ 0, 0, 0 }
	};
#undef proc
	_fileLoader = fileTypeTable;

	// Kyrandia 1 Filenames
	static const FilenameTable kyra1StaticRes[] = {
		// INTRO / OUTRO sequences
		{ kForestSeq, kRawData, "FOREST.SEQ" },
		{ kKallakWritingSeq, kRawData, "KALLAK-WRITING.SEQ" },
		{ kKyrandiaLogoSeq, kRawData, "KYRANDIA-LOGO.SEQ" },
		{ kKallakMalcolmSeq, kRawData, "KALLAK-MALCOLM.SEQ" },
		{ kMalcolmTreeSeq, kRawData, "MALCOLM-TREE.SEQ" },
		{ kWestwoodLogoSeq, kRawData, "WESTWOOD-LOGO.SEQ" },
		{ kDemo1Seq, kRawData, "DEMO1.SEQ" },
		{ kDemo2Seq, kRawData, "DEMO2.SEQ" },
		{ kDemo3Seq, kRawData, "DEMO3.SEQ" },
		{ kDemo4Seq, kRawData, "DEMO4.SEQ" },
		{ kOutroReunionSeq, kRawData, "REUNION.SEQ" },

		// INTRO / OUTRO strings
		{ kIntroCPSStrings, kStringList, "INTRO-CPS.TXT" },
		{ kIntroCOLStrings, kStringList, "INTRO-COL.TXT" },
		{ kIntroWSAStrings, kStringList, "INTRO-WSA.TXT" },
		{ kIntroStrings, kLanguageList, "INTRO-STRINGS." },
		{ kOutroHomeString, kLanguageList, "HOME." },

		// INGAME strings
		{ kItemNames, kLanguageList, "ITEMLIST." },
		{ kTakenStrings, kLanguageList, "TAKEN." },
		{ kPlacedStrings, kLanguageList, "PLACED." },
		{ kDroppedStrings, kLanguageList, "DROPPED." },
		{ kNoDropStrings, kLanguageList, "NODROP." },
		{ kPutDownString, kLanguageList, "PUTDOWN." },
		{ kWaitAmuletString, kLanguageList, "WAITAMUL." },
		{ kBlackJewelString, kLanguageList, "BLACKJEWEL." },
		{ kPoisonGoneString, kLanguageList, "POISONGONE." },
		{ kHealingTipString, kLanguageList, "HEALINGTIP." },
		{ kThePoisonStrings, kLanguageList, "THEPOISON." },
		{ kFluteStrings, kLanguageList, "FLUTE." },
		{ kWispJewelStrings, kLanguageList, "WISPJEWEL." },
		{ kMagicJewelStrings, kLanguageList, "MAGICJEWEL." },
		{ kFlaskFullString, kLanguageList, "FLASKFULL." },
		{ kFullFlaskString, kLanguageList, "FULLFLASK." },
		{ kVeryCleverString, kLanguageList, "VERYCLEVER." },
		{ kNewGameString, kLanguageList, "NEWGAME." },

		// GUI strings table
		{ kGUIStrings, kLanguageList, "GUISTRINGS." },
		{ kConfigStrings, kLanguageList, "CONFIGSTRINGS." },
		
		// ROOM table/filenames
		{ Kyra::kRoomList, StaticResource::kRoomList, "ROOM-TABLE.ROOM" },
		{ kRoomFilenames, kStringList, "ROOM-FILENAMES.TXT" },

		// SHAPE tables
		{ kDefaultShapes, kShapeList, "SHAPES-DEFAULT.SHP" },
		{ kHealing1Shapes, kShapeList, "HEALING.SHP" },
		{ kHealing2Shapes, kShapeList, "HEALING2.SHP" },
		{ kPoisonDeathShapes, kShapeList, "POISONDEATH.SHP" },
		{ kFluteShapes, kShapeList, "FLUTE.SHP" },
		{ kWinter1Shapes, kShapeList, "WINTER1.SHP" },
		{ kWinter2Shapes, kShapeList, "WINTER2.SHP" },
		{ kWinter3Shapes, kShapeList, "WINTER3.SHP" },
		{ kDrinkShapes, kShapeList, "DRINK.SHP" },
		{ kWispShapes, kShapeList, "WISP.SHP" },
		{ kMagicAnimShapes, kShapeList, "MAGICANIM.SHP" },
		{ kBranStoneShapes, kShapeList, "BRANSTONE.SHP" },

		// IMAGE filename table
		{ kCharacterImageFilenames, kStringList, "CHAR-IMAGE.TXT" },

		// AMULET anim
		{ kAmuleteAnimSeq, kRawData, "AMULETEANIM.SEQ" },

		// PALETTE table
		{ kPaletteList, kPaletteTable, "1 33 PALTABLE" },
		
		// FM-TOWNS specific
		{ kKyra1TownsSFXTable, kRawData, "SFXTABLE" },
		{ kCreditsStrings, kRawData, "CREDITS" },

		{ 0, 0, 0 }
	};

	if (_engine->game() == GI_KYRA1) {
		_builtIn = 0;
		_filenameTable = kyra1StaticRes;
	} else if (_engine->game() == GI_KYRA2 || _engine->game() == GI_KYRA3) {
		return true;
	} else {
		error("unknown game ID");
	}

	int tempSize = 0;
	uint8 *temp = getFile("INDEX", tempSize);
	if (!temp) {
		warning("no matching INDEX file found");
		return false;
	}
	
	uint32 version = READ_BE_UINT32(temp);
	uint32 gameID = READ_BE_UINT32((temp+4));
	uint32 featuresValue = READ_BE_UINT32((temp+8));
	
	delete [] temp;
	temp = 0;
	
	if (version != RESFILE_VERSION) {
		error("invalid KYRA.DAT file version (%d, required %d)", version, RESFILE_VERSION);
	}
	if (gameID != _engine->game()) {
		error("invalid game id (%d)", gameID);
	}

	uint32 gameFeatures = createFeatures(_engine->gameFlags());
	if ((featuresValue & GAME_FLAGS) != gameFeatures) {
		error("your data file has a different game flags (0x%.08X has the data and your version has 0x%.08X)", (featuresValue & GAME_FLAGS), gameFeatures);
	}

	// load all tables for now
	if (!prefetchId(-1)) {
		error("couldn't load all needed resources from 'KYRA.DAT'");
	}
	return true;
}

void StaticResource::deinit() {
	unloadId(-1);
}

const char * const*StaticResource::loadStrings(int id, int &strings) {
	const char * const*temp = (const char* const*)getData(id, kStringList, strings);
	if (temp)
		return temp;
	return (const char* const*)getData(id, kLanguageList, strings);
}

const uint8 *StaticResource::loadRawData(int id, int &size) {
	return (const uint8*)getData(id, kRawData, size);
}

const Shape *StaticResource::loadShapeTable(int id, int &entries) {
	return (const Shape*)getData(id, kShapeList, entries);
}

const Room *StaticResource::loadRoomTable(int id, int &entries) {
	return (const Room*)getData(id, StaticResource::kRoomList, entries);
}

const uint8 * const*StaticResource::loadPaletteTable(int id, int &entries) {
	return (const uint8* const*)getData(id, kPaletteTable, entries);
}

bool StaticResource::prefetchId(int id) {
	if (id == -1) {
		for (int i = 0; _filenameTable[i].filename; ++i) {
			prefetchId(_filenameTable[i].id);
		}
		return true;
	}
	const void *ptr = 0;
	int type = -1, size = -1;

	if (checkResList(id, type, ptr, size)) {
		return true;
	}

	if (checkForBuiltin(id, type, size)) {
		return true;
	}

	const FilenameTable *filename = searchFile(id);
	if (!filename)
		return false;
	const FileType *filetype = getFiletype(filename->type);
	if (!filetype)
		return false;

	ResData data;
	data.id = id;
	data.type = filetype->type;
	if (!(this->*(filetype->load))(filename->filename, data.data, data.size)) {
		return false;
	}
	_resList.push_back(data);

	return true;
}

void StaticResource::unloadId(int id) {
	Common::List<ResData>::iterator pos = _resList.begin();
	for (; pos != _resList.end(); ++pos) {
		if (pos->id == id || id == -1) {
			const FileType *filetype = getFiletype(pos->type);
			(this->*(filetype->free))(pos->data, pos->size);
			if (id != -1)
				break;
		}
	}
}

bool StaticResource::checkResList(int id, int &type, const void *&ptr, int &size) {
	Common::List<ResData>::iterator pos = _resList.begin();
	for (; pos != _resList.end(); ++pos) {
		if (pos->id == id) {
			size = pos->size;
			type = pos->type;
			ptr = pos->data;
			return true;
		}
	}
	return false;
}

const void *StaticResource::checkForBuiltin(int id, int &type, int &size) {
	if (!_builtIn)
		return 0;

	for (int i = 0; _builtIn[i].data; ++i) {
		if (_builtIn[i].id == id) {
			size = _builtIn[i].size;
			type = _builtIn[i].type;
			return _builtIn[i].data;
		}
	}

	return 0;
}

const StaticResource::FilenameTable *StaticResource::searchFile(int id) {
	if (!_filenameTable)
		return 0;

	for (int i = 0; _filenameTable[i].filename; ++i) {
		if (_filenameTable[i].id == id)
			return &_filenameTable[i];
	}

	return 0;
}

const StaticResource::FileType *StaticResource::getFiletype(int type) {
	if (!_fileLoader)
		return 0;

	for (int i = 0; _fileLoader[i].load; ++i) {
		if (_fileLoader[i].type == type) {
			return &_fileLoader[i];
		}
	}

	return 0;
}

const void *StaticResource::getData(int id, int requesttype, int &size) {
	const void *ptr = 0;
	int type = -1;	
	size = 0;

	if (checkResList(id, type, ptr, size)) {
		if (type == requesttype)
			return ptr;
		return 0;
	}

	ptr = checkForBuiltin(id, type, size);
	if (ptr) {
		if (type == requesttype)
			return ptr;
		return 0;
	}

	if (!prefetchId(id))
		return 0;

	if (checkResList(id, type, ptr, size)) {
		if (type == requesttype)
			return ptr;
	}

	return 0;
}

bool StaticResource::loadLanguageTable(const char *filename, void *&ptr, int &size) {
	char file[64];
	for (int i = 0; languages[i].ext; ++i) {
		if (languages[i].flags != createLanguage(_engine->gameFlags())) {
			continue;
		}
			
		strcpy(file, filename);
		strcat(file, languages[i].ext);
		if (loadStringTable(file, ptr, size))
			return true;
	}

	strcpy(file, filename);
	strcat(file, languages[0].ext);
	if (loadStringTable(file, ptr, size)) {
		static bool warned = false;
		if (!warned) {
			warned = true;
			warning("couldn't find specific language table for your version, using English now");
		}
		return true;
	}

	return false;
}

bool StaticResource::loadStringTable(const char *filename, void *&ptr, int &size) {
	uint8 *filePtr = getFile(filename, size);
	if (!filePtr)
		return false;
	uint8 *src = filePtr;

	uint32 count = READ_BE_UINT32(src); src += 4;
	size = count;
	char **output = new char*[count];
	assert(output);
		
	const char *curPos = (const char*)src;
	for (uint32 i = 0; i < count; ++i) {
		int strLen = strlen(curPos);
		output[i] = new char[strLen+1];
		assert(output[i]);
		memcpy(output[i], curPos, strLen+1);
		curPos += strLen+1;
	}

	delete [] filePtr;
	ptr = output;

	return true;
}

bool StaticResource::loadRawData(const char *filename, void *&ptr, int &size) {
	ptr = getFile(filename, size);
	if (!ptr)
		return false;
	return true;
}

bool StaticResource::loadShapeTable(const char *filename, void *&ptr, int &size) {
	uint8 *filePtr = getFile(filename, size);
	if (!filePtr)
		return false;
	uint8 *src = filePtr;

	uint32 count = READ_BE_UINT32(src); src += 4;
	size = count;
	Shape *loadTo = new Shape[count];
	assert(loadTo);
	
	for (uint32 i = 0; i < count; ++i) {
		loadTo[i].imageIndex = *src++;
		loadTo[i].x = *src++;
		loadTo[i].y = *src++;
		loadTo[i].w = *src++;
		loadTo[i].h = *src++;
		loadTo[i].xOffset = *src++;
		loadTo[i].yOffset = *src++;
	}

	delete [] filePtr;
	ptr = loadTo;

	return true;
}

bool StaticResource::loadRoomTable(const char *filename, void *&ptr, int &size) {
	uint8 *filePtr = getFile(filename, size);
	if (!filePtr)
		return false;
	uint8 *src = filePtr;

	uint32 count = READ_BE_UINT32(src); src += 4;
	size = count;
	Room *loadTo = new Room[count];
	assert(loadTo);
	
	for (uint32 i = 0; i < count; ++i) {
		loadTo[i].nameIndex = *src++;
		loadTo[i].northExit = READ_BE_UINT16(src); src += 2;
		loadTo[i].eastExit = READ_BE_UINT16(src); src += 2;
		loadTo[i].southExit = READ_BE_UINT16(src); src += 2;
		loadTo[i].westExit = READ_BE_UINT16(src); src += 2;
		memset(&loadTo[i].itemsTable[0], 0xFF, sizeof(byte)*6);
		memset(&loadTo[i].itemsTable[6], 0, sizeof(byte)*6);
		memset(loadTo[i].itemsXPos, 0, sizeof(uint16)*12);
		memset(loadTo[i].itemsYPos, 0, sizeof(uint8)*12);
		memset(loadTo[i].needInit, 0, sizeof(loadTo[i].needInit));
	}

	delete [] filePtr;
	ptr = loadTo;

	return true;
}

bool StaticResource::loadPaletteTable(const char *filename, void *&ptr, int &size) {
	const char *temp = filename;
	int start = atoi(temp);
	temp = strstr(temp, " ");
	if (temp == NULL)
		return false;
	++temp;
	int end = atoi(temp);

	char **table = new char*[end-start+1];
	assert(table);

	char file[64];
	temp = filename;
	temp = strstr(temp, " ");
	++temp;
	temp = strstr(temp, " ");
	if (temp == NULL)
		return false;
	++temp;
	strncpy(file, temp, 64);

	char name[64];
	for (int i = start; i <= end; ++i) {
		snprintf(name, 64, "%s%d.PAL", file, i);
		table[(start != 0) ? (i-start) : i] = (char*)getFile(name, size);
		if (!table[(start != 0) ? (i-start) : i]) {
			delete [] table;
			return false;
		}
	}

	ptr = table;
	size = end - start + 1;
	return true;
}

void StaticResource::freeRawData(void *&ptr, int &size) {
	uint8 *data = (uint8*)ptr;
	delete [] data;
	ptr = 0;
	size = 0;
}

void StaticResource::freeStringTable(void *&ptr, int &size) {
	char **data = (char**)ptr;
	while (size--) {
		delete [] data[size];
	}
	ptr = 0;
	size = 0;
}

void StaticResource::freeShapeTable(void *&ptr, int &size) {
	Shape *data = (Shape*)ptr;
	delete [] data;
	ptr = 0;
	size = 0;
}

void StaticResource::freeRoomTable(void *&ptr, int &size) {
	Room *data = (Room*)ptr;
	delete [] data;
	ptr = 0;
	size = 0;
}

void StaticResource::freePaletteTable(void *&ptr, int &size) {
	uint8 **data = (uint8**)ptr;
	while (size--) {
		delete [] data[size];
	}
	ptr = 0;
	size = 0;
}

uint8 *StaticResource::getFile(const char *name, int &size) {
	char buffer[64];
	const char *ext = "";
	if (_engine->gameFlags().isTalkie) {
		ext = ".CD";
	} else if (_engine->gameFlags().isDemo) {
		ext = ".DEM";
	} else if (_engine->gameFlags().platform == Common::kPlatformFMTowns) {
		ext = ".TNS";
	} else if (_engine->gameFlags().platform == Common::kPlatformAmiga) {
		ext = ".AMG";
	}
	snprintf(buffer, 64, "%s%s", name, ext);
	uint32 tempSize = 0;
	uint8 *data = _engine->resource()->fileData(buffer, &tempSize);
	size = tempSize;
	return data;
}

#pragma mark -

void KyraEngine::initStaticResource() {
	int temp = 0;
	_seq_Forest = _staticres->loadRawData(kForestSeq, temp);
	_seq_KallakWriting = _staticres->loadRawData(kKallakWritingSeq, temp);
	_seq_KyrandiaLogo = _staticres->loadRawData(kKyrandiaLogoSeq, temp);
	_seq_KallakMalcolm = _staticres->loadRawData(kKallakMalcolmSeq, temp);
	_seq_MalcolmTree = _staticres->loadRawData(kMalcolmTreeSeq, temp);
	_seq_WestwoodLogo = _staticres->loadRawData(kWestwoodLogoSeq, temp);
	_seq_Demo1 = _staticres->loadRawData(kDemo1Seq, temp);
	_seq_Demo2 = _staticres->loadRawData(kDemo2Seq, temp);
	_seq_Demo3 = _staticres->loadRawData(kDemo3Seq, temp);
	_seq_Demo4 = _staticres->loadRawData(kDemo4Seq, temp);
	_seq_Reunion = _staticres->loadRawData(kOutroReunionSeq, temp);

	_seq_WSATable = _staticres->loadStrings(kIntroWSAStrings, _seq_WSATable_Size);
	_seq_CPSTable = _staticres->loadStrings(kIntroCPSStrings, _seq_CPSTable_Size);
	_seq_COLTable = _staticres->loadStrings(kIntroCOLStrings, _seq_COLTable_Size);
	_seq_textsTable = _staticres->loadStrings(kIntroStrings, _seq_textsTable_Size);

	_itemList = _staticres->loadStrings(kItemNames, _itemList_Size);
	_takenList = _staticres->loadStrings(kTakenStrings, _takenList_Size);
	_placedList = _staticres->loadStrings(kPlacedStrings, _placedList_Size);
	_droppedList = _staticres->loadStrings(kDroppedStrings, _droppedList_Size);
	_noDropList = _staticres->loadStrings(kNoDropStrings, _noDropList_Size);
	_putDownFirst = _staticres->loadStrings(kPutDownString, _putDownFirst_Size);
	_waitForAmulet = _staticres->loadStrings(kWaitAmuletString, _waitForAmulet_Size);
	_blackJewel = _staticres->loadStrings(kBlackJewelString, _blackJewel_Size);
	_poisonGone = _staticres->loadStrings(kPoisonGoneString, _poisonGone_Size);
	_healingTip = _staticres->loadStrings(kHealingTipString, _healingTip_Size);
	_thePoison = _staticres->loadStrings(kThePoisonStrings, _thePoison_Size);
	_fluteString = _staticres->loadStrings(kFluteStrings, _fluteString_Size);
	_wispJewelStrings = _staticres->loadStrings(kWispJewelStrings, _wispJewelStrings_Size);
	_magicJewelString = _staticres->loadStrings(kMagicJewelStrings, _magicJewelString_Size);
	_flaskFull = _staticres->loadStrings(kFlaskFullString, _flaskFull_Size);
	_fullFlask = _staticres->loadStrings(kFullFlaskString, _fullFlask_Size);
	_veryClever = _staticres->loadStrings(kVeryCleverString, _veryClever_Size);
	_homeString = _staticres->loadStrings(kOutroHomeString, _homeString_Size);
	_newGameString = _staticres->loadStrings(kNewGameString, _newGameString_Size);
	
	_healingShapeTable = _staticres->loadShapeTable(kHealing1Shapes, _healingShapeTableSize);
	_healingShape2Table = _staticres->loadShapeTable(kHealing2Shapes, _healingShape2TableSize);	
	_posionDeathShapeTable = _staticres->loadShapeTable(kPoisonDeathShapes, _posionDeathShapeTableSize);
	_fluteAnimShapeTable = _staticres->loadShapeTable(kFluteShapes, _fluteAnimShapeTableSize);
	_winterScrollTable = _staticres->loadShapeTable(kWinter1Shapes, _winterScrollTableSize);
	_winterScroll1Table = _staticres->loadShapeTable(kWinter2Shapes, _winterScroll1TableSize);
	_winterScroll2Table = _staticres->loadShapeTable(kWinter3Shapes, _winterScroll2TableSize);
	_drinkAnimationTable = _staticres->loadShapeTable(kDrinkShapes, _drinkAnimationTableSize);
	_brandonToWispTable = _staticres->loadShapeTable(kWispShapes, _brandonToWispTableSize);
	_magicAnimationTable = _staticres->loadShapeTable(kMagicAnimShapes, _magicAnimationTableSize);
	_brandonStoneTable = _staticres->loadShapeTable(kBranStoneShapes, _brandonStoneTableSize);

	_characterImageTable = _staticres->loadStrings(kCharacterImageFilenames, _characterImageTableSize);

	_roomFilenameTable = _staticres->loadStrings(kRoomFilenames, _roomFilenameTableSize);
	
	_amuleteAnim = _staticres->loadRawData(kAmuleteAnimSeq, temp);
	
	_specialPalettes = _staticres->loadPaletteTable(kPaletteList, temp);

	_guiStrings = _staticres->loadStrings(kGUIStrings, _guiStringsSize);
	_configStrings = _staticres->loadStrings(kConfigStrings, _configStringsSize);
	
	// copied static res

	// room list
	const Room *tempRoomList = _staticres->loadRoomTable(kRoomList, _roomTableSize);

	if (_roomTableSize > 0) {
		_roomTable = new Room[_roomTableSize];
		assert(_roomTable);

		memcpy(_roomTable, tempRoomList, _roomTableSize*sizeof(Room));
		tempRoomList = 0;

		_staticres->unloadId(kRoomList);
	}

	// default shape table
	const Shape *tempShapeTable = _staticres->loadShapeTable(kDefaultShapes, _defaultShapeTableSize);

	if (_defaultShapeTableSize > 0) {
		_defaultShapeTable = new Shape[_defaultShapeTableSize];
		assert(_defaultShapeTable);

		memcpy(_defaultShapeTable, tempShapeTable, _defaultShapeTableSize*sizeof(Shape));
		tempShapeTable = 0;

		_staticres->unloadId(kDefaultShapes);
	}
}

void KyraEngine::loadMouseShapes() {
	_screen->loadBitmap("MOUSE.CPS", 3, 3, 0);
	_screen->_curPage = 2;
	_shapes[4] = _screen->encodeShape(0, 0, 8, 10, 0);
	_shapes[5] = _screen->encodeShape(0, 0x17, 0x20, 7, 0);
	_shapes[6] = _screen->encodeShape(0x50, 0x12, 0x10, 9, 0);
	_shapes[7] = _screen->encodeShape(0x60, 0x12, 0x10, 11, 0);
	_shapes[8] = _screen->encodeShape(0x70, 0x12, 0x10, 9, 0);
	_shapes[9] = _screen->encodeShape(0x80, 0x12, 0x10, 11, 0);
	_shapes[10] = _screen->encodeShape(0x90, 0x12, 0x10, 10, 0);
	_shapes[364] = _screen->encodeShape(0x28, 0, 0x10, 13, 0);
	_screen->setMouseCursor(1, 1, 0);
	_screen->setMouseCursor(1, 1, _shapes[4]);
	_screen->setShapePages(5, 3);
}

void KyraEngine::loadCharacterShapes() {
	int curImage = 0xFF;
	int videoPage = _screen->_curPage;
	_screen->_curPage = 2;
	for (int i = 0; i < 115; ++i) {	
		assert(i < _defaultShapeTableSize);
		Shape *shape = &_defaultShapeTable[i];
		if (shape->imageIndex == 0xFF) {
			_shapes[i+7+4] = 0;
			continue;
		}
		if (shape->imageIndex != curImage) {
			assert(shape->imageIndex < _characterImageTableSize);
			_screen->loadBitmap(_characterImageTable[shape->imageIndex], 3, 3, 0);
			curImage = shape->imageIndex;
		}
		_shapes[i+7+4] = _screen->encodeShape(shape->x<<3, shape->y, shape->w<<3, shape->h, 1);
	}
	_screen->_curPage = videoPage;
}

void KyraEngine::loadSpecialEffectShapes() {
	_screen->loadBitmap("EFFECTS.CPS", 3, 3, 0);
	_screen->_curPage = 2;
 
	int currShape; 
	for (currShape = 173; currShape < 183; currShape++)
		_shapes[4 + currShape] = _screen->encodeShape((currShape-173) * 24, 0, 24, 24, 1);
 
	for (currShape = 183; currShape < 190; currShape++)
		_shapes[4 + currShape] = _screen->encodeShape((currShape-183) * 24, 24, 24, 24, 1);
 
	for (currShape = 190; currShape < 201; currShape++)
		_shapes[4 + currShape] = _screen->encodeShape((currShape-190) * 24, 48, 24, 24, 1);
 
	for (currShape = 201; currShape < 206; currShape++)
		_shapes[4 + currShape] = _screen->encodeShape((currShape-201) * 16, 106, 16, 16, 1);
}

void KyraEngine::loadItems() {
	int shape;

	_screen->loadBitmap("JEWELS3.CPS", 3, 3, 0);
	_screen->_curPage = 2;

	_shapes[327] = 0;

	for (shape = 1; shape < 6; shape++ )
		_shapes[327 + shape] = _screen->encodeShape((shape - 1) * 32, 0, 32, 17, 0);

	for (shape = 330; shape <= 334; shape++)
		_shapes[4 + shape] = _screen->encodeShape((shape-330) * 32, 102, 32, 17, 0);

	for (shape = 335; shape <= 339; shape++)
		_shapes[4 + shape] = _screen->encodeShape((shape-335) * 32, 17,  32, 17, 0);

	for (shape = 340; shape <= 344; shape++)
		_shapes[4 + shape] = _screen->encodeShape((shape-340) * 32, 34,  32, 17, 0);

	for (shape = 345; shape <= 349; shape++)
		_shapes[4 + shape] = _screen->encodeShape((shape-345) * 32, 51,  32, 17, 0);

	for (shape = 350; shape <= 354; shape++)
		_shapes[4 + shape] = _screen->encodeShape((shape-350) * 32, 68,  32, 17, 0);

	for (shape = 355; shape <= 359; shape++)
		_shapes[4 + shape] = _screen->encodeShape((shape-355) * 32, 85,  32, 17, 0);


	_screen->loadBitmap("ITEMS.CPS", 3, 3, 0);
	_screen->_curPage = 2;

	for (int i = 0; i < 107; i++) {
		shape = findDuplicateItemShape(i);

		if (shape != -1)
			_shapes[220 + i] = _shapes[220 + shape];
		else
			_shapes[220 + i] = _screen->encodeShape( (i % 20) * 16, i/20 * 16, 16, 16, 0);
	}

	uint32 size;
	uint8 *fileData = _res->fileData("_ITEM_HT.DAT", &size);
	assert(fileData);

	for (int i = 0; i < 107; i++) {
		_itemTable[i].height = fileData[i];
		_itemTable[i].unk1 = _itemTable[i].unk2 = 0;
	}

	delete[] fileData;
}

void KyraEngine::loadButtonShapes() {
	_screen->loadBitmap("BUTTONS2.CPS", 3, 3, 0);
	_screen->_curPage = 2;
	_scrollUpButton.process0PtrShape = _screen->encodeShape(0, 0, 24, 14, 1);
	_scrollUpButton.process1PtrShape = _screen->encodeShape(24, 0, 24, 14, 1);
	_scrollUpButton.process2PtrShape = _screen->encodeShape(48, 0, 24, 14, 1);
	_scrollDownButton.process0PtrShape = _screen->encodeShape(0, 15, 24, 14, 1);
	_scrollDownButton.process1PtrShape = _screen->encodeShape(24, 15, 24, 14, 1);
	_scrollDownButton.process2PtrShape = _screen->encodeShape(48, 15, 24, 14, 1);
	_screen->_curPage = 0;
}

void KyraEngine::loadMainScreen(int page) {
	_screen->clearPage(page);

	if (_flags.lang == Common::EN_ANY && !_flags.isTalkie && (_flags.platform == Common::kPlatformPC || _flags.platform == Common::kPlatformAmiga))
		_screen->loadBitmap("MAIN15.CPS", page, page, _screen->getPalette(0));
	else if (_flags.lang == Common::EN_ANY || _flags.lang == Common::JA_JPN) 
		_screen->loadBitmap("MAIN_ENG.CPS", page, page, 0);
	else if (_flags.lang == Common::FR_FRA)
		_screen->loadBitmap("MAIN_FRE.CPS", page, page, 0);
	else if (_flags.lang == Common::DE_DEU)
		_screen->loadBitmap("MAIN_GER.CPS", page, page, 0);
	else if (_flags.lang == Common::ES_ESP)
		_screen->loadBitmap("MAIN_SPA.CPS", page, page, 0);
	else if (_flags.lang == Common::IT_ITA)
		_screen->loadBitmap("MAIN_ITA.CPS", page, page, 0);
	else
		warning("no main graphics file found");

	if (_flags.platform == Common::kPlatformAmiga)
		memcpy(_screen->getPalette(1), _screen->getPalette(0), 32*3);

	_screen->copyRegion(0, 0, 0, 0, 320, 200, page, 0);
}

const ScreenDim Screen::_screenDimTable[] = {
	{ 0x00, 0x00, 0x28, 0xC8, 0x0F, 0x0C, 0x00, 0x00 },
	{ 0x08, 0x48, 0x18, 0x38, 0x0F, 0x0C, 0x00, 0x00 },
	{ 0x01, 0x08, 0x26, 0x80, 0x0F, 0x0C, 0x00, 0x00 },
	{ 0x00, 0xC2, 0x28, 0x06, 0x0F, 0x0C, 0x00, 0x00 },
	{ 0x00, 0x90, 0x28, 0x38, 0x04, 0x0C, 0x00, 0x00 },
	{ 0x01, 0x94, 0x26, 0x30, 0x04, 0x1B, 0x00, 0x00 },
	{ 0x00, 0x90, 0x28, 0x38, 0x0F, 0x0D, 0x00, 0x00 },
	{ 0x01, 0x96, 0x26, 0x32, 0x0F, 0x0D, 0x00, 0x00 },
	{ 0x00, 0x00, 0x28, 0x88, 0x0F, 0x0C, 0x00, 0x00 },
	{ 0x01, 0x20, 0x26, 0x80, 0x0F, 0x0C, 0x00, 0x00 },
	{ 0x03, 0x28, 0x22, 0x46, 0x0F, 0x0D, 0x00, 0x00 }
};

const int Screen::_screenDimTableCount = ARRAYSIZE(_screenDimTable);

const ScreenDim Screen::_screenDimTableK3[] = {
	{ 0x00, 0x00, 0x28, 0xC8, 0xFF, 0xF0, 0x00, 0x00 },
	{ 0x08, 0x48, 0x18, 0x38, 0xFF, 0xF0, 0x00, 0x00 },
	{ 0x00, 0x00, 0x28, 0xBC, 0xFF, 0xF0, 0x00, 0x00 },
	{ 0x0A, 0x96, 0x14, 0x30, 0x19, 0xF0, 0x00, 0x00 }
};

const int Screen::_screenDimTableCountK3 = ARRAYSIZE(_screenDimTableK3);

#define Opcode(x) &KyraEngine::x
void KyraEngine::setupOpcodeTable() {
	static const OpcodeProc opcodeTable[] = {
		// 0x00
		Opcode(o1_magicInMouseItem),
		Opcode(o1_characterSays),
		Opcode(o1_pauseTicks),
		Opcode(o1_drawSceneAnimShape),
		// 0x04
		Opcode(o1_queryGameFlag),
		Opcode(o1_setGameFlag),
		Opcode(o1_resetGameFlag),
		Opcode(o1_runNPCScript),
		// 0x08
		Opcode(o1_setSpecialExitList),
		Opcode(o1_blockInWalkableRegion),
		Opcode(o1_blockOutWalkableRegion),
		Opcode(o1_walkPlayerToPoint),
		// 0x0c
		Opcode(o1_dropItemInScene),
		Opcode(o1_drawAnimShapeIntoScene),
		Opcode(o1_createMouseItem),
		Opcode(o1_savePageToDisk),
		// 0x10
		Opcode(o1_sceneAnimOn),
		Opcode(o1_sceneAnimOff),
		Opcode(o1_getElapsedSeconds),
		Opcode(o1_mouseIsPointer),
		// 0x14
		Opcode(o1_destroyMouseItem),
		Opcode(o1_runSceneAnimUntilDone),
		Opcode(o1_fadeSpecialPalette),
		Opcode(o1_playAdlibSound),
		// 0x18
		Opcode(o1_playAdlibScore),
		Opcode(o1_phaseInSameScene),
		Opcode(o1_setScenePhasingFlag),
		Opcode(o1_resetScenePhasingFlag),
		// 0x1c
		Opcode(o1_queryScenePhasingFlag),
		Opcode(o1_sceneToDirection),
		Opcode(o1_setBirthstoneGem),
		Opcode(o1_placeItemInGenericMapScene),
		// 0x20
		Opcode(o1_setBrandonStatusBit),
		Opcode(o1_pauseSeconds),
		Opcode(o1_getCharactersLocation),
		Opcode(o1_runNPCSubscript),
		// 0x24
		Opcode(o1_magicOutMouseItem),
		Opcode(o1_internalAnimOn),
		Opcode(o1_forceBrandonToNormal),
		Opcode(o1_poisonDeathNow),
		// 0x28
		Opcode(o1_setScaleMode),
		Opcode(o1_openWSAFile),
		Opcode(o1_closeWSAFile),
		Opcode(o1_runWSAFromBeginningToEnd),
		// 0x2c
		Opcode(o1_displayWSAFrame),
		Opcode(o1_enterNewScene),
		Opcode(o1_setSpecialEnterXAndY),
		Opcode(o1_runWSAFrames),
		// 0x30
		Opcode(o1_popBrandonIntoScene),
		Opcode(o1_restoreAllObjectBackgrounds),
		Opcode(o1_setCustomPaletteRange),
		Opcode(o1_loadPageFromDisk),
		// 0x34
		Opcode(o1_customPrintTalkString),
		Opcode(o1_restoreCustomPrintBackground),
		Opcode(o1_hideMouse),
		Opcode(o1_showMouse),
		// 0x38
		Opcode(o1_getCharacterX),
		Opcode(o1_getCharacterY),
		Opcode(o1_changeCharactersFacing),
		Opcode(o1_copyWSARegion),
		// 0x3c
		Opcode(o1_printText),
		Opcode(o1_random),
		Opcode(o1_loadSoundFile),
		Opcode(o1_displayWSAFrameOnHidPage),
		// 0x40
		Opcode(o1_displayWSASequentialFrames),
		Opcode(o1_drawCharacterStanding),
		Opcode(o1_internalAnimOff),
		Opcode(o1_changeCharactersXAndY),
		// 0x44
		Opcode(o1_clearSceneAnimatorBeacon),
		Opcode(o1_querySceneAnimatorBeacon),
		Opcode(o1_refreshSceneAnimator),
		Opcode(o1_placeItemInOffScene),
		// 0x48
		Opcode(o1_wipeDownMouseItem),
		Opcode(o1_placeCharacterInOtherScene),
		Opcode(o1_getKey),
		Opcode(o1_specificItemInInventory),
		// 0x4c
		Opcode(o1_popMobileNPCIntoScene),
		Opcode(o1_mobileCharacterInScene),
		Opcode(o1_hideMobileCharacter),
		Opcode(o1_unhideMobileCharacter),
		// 0x50
		Opcode(o1_setCharactersLocation),
		Opcode(o1_walkCharacterToPoint),
		Opcode(o1_specialEventDisplayBrynnsNote),
		Opcode(o1_specialEventRemoveBrynnsNote),
		// 0x54
		Opcode(o1_setLogicPage),
		Opcode(o1_fatPrint),
		Opcode(o1_preserveAllObjectBackgrounds),
		Opcode(o1_updateSceneAnimations),
		// 0x58
		Opcode(o1_sceneAnimationActive),
		Opcode(o1_setCharactersMovementDelay),
		Opcode(o1_getCharactersFacing),
		Opcode(o1_bkgdScrollSceneAndMasksRight),
		// 0x5c
		Opcode(o1_dispelMagicAnimation),
		Opcode(o1_findBrightestFireberry),
		Opcode(o1_setFireberryGlowPalette),
		Opcode(o1_setDeathHandlerFlag),
		// 0x60
		Opcode(o1_drinkPotionAnimation),
		Opcode(o1_makeAmuletAppear),
		Opcode(o1_drawItemShapeIntoScene),
		Opcode(o1_setCharactersCurrentFrame),
		// 0x64
		Opcode(o1_waitForConfirmationMouseClick),
		Opcode(o1_pageFlip),
		Opcode(o1_setSceneFile),
		Opcode(o1_getItemInMarbleVase),
		// 0x68
		Opcode(o1_setItemInMarbleVase),
		Opcode(o1_addItemToInventory),
		Opcode(o1_intPrint),
		Opcode(o1_shakeScreen),
		// 0x6c
		Opcode(o1_createAmuletJewel),
		Opcode(o1_setSceneAnimCurrXY),
		Opcode(o1_poisonBrandonAndRemaps),
		Opcode(o1_fillFlaskWithWater),
		// 0x70
		Opcode(o1_getCharactersMovementDelay),
		Opcode(o1_getBirthstoneGem),
		Opcode(o1_queryBrandonStatusBit),
		Opcode(o1_playFluteAnimation),
		// 0x74
		Opcode(o1_playWinterScrollSequence),
		Opcode(o1_getIdolGem),
		Opcode(o1_setIdolGem),
		Opcode(o1_totalItemsInScene),
		// 0x78
		Opcode(o1_restoreBrandonsMovementDelay),
		Opcode(o1_setMousePos),
		Opcode(o1_getMouseState),
		Opcode(o1_setEntranceMouseCursorTrack),
		// 0x7c
		Opcode(o1_itemAppearsOnGround),
		Opcode(o1_setNoDrawShapesFlag),
		Opcode(o1_fadeEntirePalette),
		Opcode(o1_itemOnGroundHere),
		// 0x80
		Opcode(o1_queryCauldronState),
		Opcode(o1_setCauldronState),
		Opcode(o1_queryCrystalState),
		Opcode(o1_setCrystalState),
		// 0x84
		Opcode(o1_setPaletteRange),
		Opcode(o1_shrinkBrandonDown),
		Opcode(o1_growBrandonUp),
		Opcode(o1_setBrandonScaleXAndY),
		// 0x88
		Opcode(o1_resetScaleMode),
		Opcode(o1_getScaleDepthTableValue),
		Opcode(o1_setScaleDepthTableValue),
		Opcode(o1_message),
		// 0x8c
		Opcode(o1_checkClickOnNPC),
		Opcode(o1_getFoyerItem),
		Opcode(o1_setFoyerItem),
		Opcode(o1_setNoItemDropRegion),
		// 0x90
		Opcode(o1_walkMalcolmOn),
		Opcode(o1_passiveProtection),
		Opcode(o1_setPlayingLoop),
		Opcode(o1_brandonToStoneSequence),
		// 0x94
		Opcode(o1_brandonHealingSequence),
		Opcode(o1_protectCommandLine),
		Opcode(o1_pauseMusicSeconds),
		Opcode(o1_resetMaskRegion),
		// 0x98
		Opcode(o1_setPaletteChangeFlag),
		Opcode(o1_fillRect),
		Opcode(o1_vocUnload),
		Opcode(o1_vocLoad),
		Opcode(o1_dummy)
	};
	
	_opcodeTable = opcodeTable;
	_opcodeTableSize = ARRAYSIZE(opcodeTable);
}
#undef Opcode

const char *KyraEngine::_soundFiles[] = {
	"INTRO",
	"KYRA1A",
	"KYRA1B",
	"KYRA2A",
	"KYRA3A",
	"KYRA4A",
	"KYRA4B",
	"KYRA5A",
	"KYRA5B",
	"KYRAMISC"
};

const int KyraEngine::_soundFilesCount = ARRAYSIZE(KyraEngine::_soundFiles);

const char *KyraEngine::_soundFilesTowns[] = {
	"TW_INTRO.SFX",
	"TW_SCEN1.SFX",
	"TW_SCEN2.SFX",
	"TW_SCEN3.SFX",
	"TW_SCEN4.SFX",
	"TW_SCEN5.SFX",
};

const int KyraEngine::_soundFilesTownsCount = ARRAYSIZE(KyraEngine::_soundFilesTowns);

const int8 KyraEngine::_charXPosTable[] = {
	 0,  4,  4,  4,  0, -4, -4, -4
};

const int8 KyraEngine::_addXPosTable[] = {
	 4,  4,  0, -4, -4, -4,  0,  4
};

const int8 KyraEngine::_charYPosTable[] = {
	-2, -2,  0,  2,  2,  2,  0, -2
};

const int8 KyraEngine::_addYPosTable[] = {
	 0, -2, -2, -2,  0,  2,  2,  2
};

const uint16 KyraEngine::_itemPosX[] = {
	95, 115, 135, 155, 175, 95, 115, 135, 155, 175
};

const uint8 KyraEngine::_itemPosY[] = {
	160, 160, 160, 160, 160, 181, 181, 181, 181, 181
};

void KyraEngine::setupButtonData() {
	static Button buttonData[] = {
		{ 0, 0x02, /*XXX,*/0, 0, 0, /*XXX,*/ 0x0400, 0, 0, 0, 0, 0, 0, 0, 0x05D, 0x9E, 0x13, 0x14, /*XXX,*/ 0, &KyraEngine::buttonInventoryCallback/*, XXX*/ },
		{ 0, 0x01, /*XXX,*/1, 1, 1, /*XXX,*/ 0x0487, 0, 0, 0, 0, 0, 0, 0, 0x009, 0xA4, 0x36, 0x1E, /*XXX,*/ 0, &KyraEngine::buttonMenuCallback/*, XXX*/ },
		{ 0, 0x03, /*XXX,*/0, 0, 0, /*XXX,*/ 0x0400, 0, 0, 0, 0, 0, 0, 0, 0x071, 0x9E, 0x13, 0x14, /*XXX,*/ 0, &KyraEngine::buttonInventoryCallback/*, XXX*/ },
		{ 0, 0x04, /*XXX,*/0, 0, 0, /*XXX,*/ 0x0400, 0, 0, 0, 0, 0, 0, 0, 0x085, 0x9E, 0x13, 0x14, /*XXX,*/ 0, &KyraEngine::buttonInventoryCallback/*, XXX*/ },
		{ 0, 0x05, /*XXX,*/0, 0, 0, /*XXX,*/ 0x0400, 0, 0, 0, 0, 0, 0, 0, 0x099, 0x9E, 0x13, 0x14, /*XXX,*/ 0, &KyraEngine::buttonInventoryCallback/*, XXX*/ },
		{ 0, 0x06, /*XXX,*/0, 0, 0, /*XXX,*/ 0x0400, 0, 0, 0, 0, 0, 0, 0, 0x0AD, 0x9E, 0x13, 0x14, /*XXX,*/ 0, &KyraEngine::buttonInventoryCallback/*, XXX*/ },
		{ 0, 0x07, /*XXX,*/0, 0, 0, /*XXX,*/ 0x0400, 0, 0, 0, 0, 0, 0, 0, 0x05D, 0xB3, 0x13, 0x14, /*XXX,*/ 0, &KyraEngine::buttonInventoryCallback/*, XXX*/ },
		{ 0, 0x08, /*XXX,*/0, 0, 0, /*XXX,*/ 0x0400, 0, 0, 0, 0, 0, 0, 0, 0x071, 0xB3, 0x13, 0x14, /*XXX,*/ 0, &KyraEngine::buttonInventoryCallback/*, XXX*/ },
		{ 0, 0x09, /*XXX,*/0, 0, 0, /*XXX,*/ 0x0400, 0, 0, 0, 0, 0, 0, 0, 0x085, 0xB3, 0x13, 0x14, /*XXX,*/ 0, &KyraEngine::buttonInventoryCallback/*, XXX*/ },
		{ 0, 0x0A, /*XXX,*/0, 0, 0, /*XXX,*/ 0x0400, 0, 0, 0, 0, 0, 0, 0, 0x099, 0xB3, 0x13, 0x14, /*XXX,*/ 0, &KyraEngine::buttonInventoryCallback/*, XXX*/ },
		{ 0, 0x0B, /*XXX,*/0, 0, 0, /*XXX,*/ 0x0400, 0, 0, 0, 0, 0, 0, 0, 0x0AD, 0xB3, 0x13, 0x14, /*XXX,*/ 0, &KyraEngine::buttonInventoryCallback/*, XXX*/ },
		{ 0, 0x15, /*XXX,*/1, 1, 1, /*XXX,*/ 0x0487, 0, 0, 0, 0, 0, 0, 0, 0x0FD, 0x9C, 0x1A, 0x12, /*XXX,*/ 0, &KyraEngine::buttonAmuletCallback/*, XXX*/ },
		{ 0, 0x16, /*XXX,*/1, 1, 1, /*XXX,*/ 0x0487, 0, 0, 0, 0, 0, 0, 0, 0x0E7, 0xAA, 0x1A, 0x12, /*XXX,*/ 0, &KyraEngine::buttonAmuletCallback/*, XXX*/ },
		{ 0, 0x17, /*XXX,*/1, 1, 1, /*XXX,*/ 0x0487, 0, 0, 0, 0, 0, 0, 0, 0x0FD, 0xB5, 0x1A, 0x12, /*XXX,*/ 0, &KyraEngine::buttonAmuletCallback/*, XXX*/ },
		{ 0, 0x18, /*XXX,*/1, 1, 1, /*XXX,*/ 0x0487, 0, 0, 0, 0, 0, 0, 0, 0x113, 0xAA, 0x1A, 0x12, /*XXX,*/ 0, &KyraEngine::buttonAmuletCallback/*, XXX*/ }
	};

	static Button *buttonDataListPtr[] = {
		&buttonData[1],
		&buttonData[2],
		&buttonData[3],
		&buttonData[4],
		&buttonData[5],
		&buttonData[6],
		&buttonData[7],
		&buttonData[8],
		&buttonData[9],
		&buttonData[10],
		&buttonData[11],
		&buttonData[12],
		&buttonData[13],
		&buttonData[14],
		0
	};
	
	_buttonData = buttonData;
	_buttonDataListPtr = buttonDataListPtr;
}

Button KyraEngine::_scrollUpButton =  {0, 0x12, 1, 1, 1, 0x483, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x18, 0x0f, 0, 0};
Button KyraEngine::_scrollDownButton = {0, 0x13, 1, 1, 1, 0x483, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x18, 0x0f, 0, 0};

Button KyraEngine::_menuButtonData[] = {
	{ 0, 0x0c, /*XXX,*/1, 1, 1, /*XXX,*/ 0x487, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*XXX,*/ 0, 0 /*, XXX*/ },
	{ 0, 0x0d, /*XXX,*/1, 1, 1, /*XXX,*/ 0x487, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*XXX,*/ 0, 0 /*, XXX*/ },
	{ 0, 0x0e, /*XXX,*/1, 1, 1, /*XXX,*/ 0x487, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*XXX,*/ 0, 0 /*, XXX*/ },
	{ 0, 0x0f, /*XXX,*/1, 1, 1, /*XXX,*/ 0x487, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*XXX,*/ 0, 0 /*, XXX*/ },
	{ 0, 0x10, /*XXX,*/1, 1, 1, /*XXX,*/ 0x487, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*XXX,*/ 0, 0 /*, XXX*/ },
	{ 0, 0x11, /*XXX,*/1, 1, 1, /*XXX,*/ 0x487, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /*XXX,*/ 0, 0 /*, XXX*/ }
};

void KyraEngine::setupMenu() {
	static Menu menu[] = {
		{ -1, -1, 208, 136, 248, 249, 250, 0, 251, -1, 8, 0, 5, -1, -1, -1, -1, 
			{
				{1, 0, 0, 0, -1, -1, 30, 148, 15, 252, 253, 24, 0,
				248, 249, 250, &KyraEngine::gui_loadGameMenu, -1, 0, 0, 0, 0, 0},

				{1, 0, 0, 0, -1, -1, 47, 148, 15, 252, 253, 24, 0,
				248, 249, 250, &KyraEngine::gui_saveGameMenu, -1, 0, 0, 0, 0, 0},

				{1, 0, 0, 0, -1, -1, 64, 148, 15, 252, 253, 24, 0,
				248, 249, 250, &KyraEngine::gui_gameControlsMenu, -1, 0, 0, 0, 0, 0},

				{1, 0, 0, 0, -1, -1, 81, 148, 15, 252, 253, 24, 0,
				248, 249, 250, &KyraEngine::gui_quitPlaying, -1, 0, 0, 0, 0, 0},

				{1, 0, 0, 0, 86, 0, 110, 92, 15, 252, 253, -1, 255,
				248, 249, 250, &KyraEngine::gui_resumeGame, -1, 0, 0, 0, 0, 0}
			}
		},
		{ -1, -1, 288, 56, 248, 249, 250, 0, 254,-1, 8, 0, 2, -1, -1, -1, -1,
			{
				{1, 0, 0, 0, 24, 0, 30, 72, 15, 252, 253, -1, 255,
				248, 249, 250, &KyraEngine::gui_quitConfirmYes, -1, 0, 0, 0, 0, 0},

				{1, 0, 0, 0, 192, 0, 30, 72, 15, 252, 253, -1, 255,
				248, 249, 250, &KyraEngine::gui_quitConfirmNo, -1, 0, 0, 0, 0, 0}
			}
		},
		{ -1, -1, 288, 160, 248, 249, 250, 0, 251, -1, 8, 0, 6, 132, 22, 132, 124,
			{
				{1, 0, 0, 0, -1, 255, 39, 256, 15, 252, 253, 5, 0, 
				248, 249, 250, 0, -1, 0, 0, 0, 0, 0},

				{1, 0, 0, 0, -1, 255, 56, 256, 15, 252, 253, 5, 0, 
				248, 249, 250, 0, -1, 0, 0, 0, 0, 0},

				{1, 0, 0, 0, -1, 255, 73, 256, 15, 252, 253, 5, 0, 
				248, 249, 250, 0, -1, 0, 0, 0, 0, 0},

				{1, 0, 0, 0, -1, 255, 90, 256, 15, 252, 253, 5, 0, 
				248, 249, 250, 0, -1, 0, 0, 0, 0, 0},

				{1, 0, 0, 0, -1, 255, 107, 256, 15, 252, 253, 5, 0,
				248, 249, 250, 0, -1, 0, 0, 0, 0, 0},

				{1, 0, 0, 0, 184, 0, 134, 88, 15, 252, 253, -1, 255,
				248, 249, 250, &KyraEngine::gui_cancelSubMenu, -1, 0, 0, 0, 0, 0},
			}
		},
		{ -1, -1, 288, 67, 248, 249, 250, 0, 251, -1, 8, 0, 3, -1, -1, -1, -1,
			{
				{1, 0, 0, 0, 24, 0, 44, 72, 15, 252, 253, -1, 255,
				248, 249, 250, &KyraEngine::gui_savegameConfirm, -1, 0, 0, 0, 0, 0},

				{1, 0, 0, 0, 192, 0, 44, 72, 15, 252, 253, -1, 255,
				248, 249, 250, &KyraEngine::gui_cancelSubMenu, -1, 0, 0, 0, 0, 0}
			}
		},
		{ -1, -1, 208, 76, 248, 249, 250, 0, 251, -1, 8, 0, 2, -1, -1, -1, -1, 
			{
				{1, 0, 0, 0, -1, -1, 30, 148, 15, 252, 253, 24, 0,
				248, 249, 250, &KyraEngine::gui_loadGameMenu, -1, 0, 0, 0, 0, 0},

				{1, 0, 0, 0, -1, -1, 47, 148, 15, 252, 253, 24, 0,
				248, 249, 250, &KyraEngine::gui_quitPlaying, -1, 0, 0, 0, 0, 0}
			}
		},
		{ -1, -1, 208, 153, 248, 249, 250, 0, 251, -1, 8, 0, 6, -1, -1, -1, -1, 
			{
				{1, 0, 0, 0, 110, 0, 30, 64, 15, 252, 253, 5, 0,
				248, 249, 250, &KyraEngine::gui_controlsChangeMusic, -1, 0, 34, 32, 0, 0},

	 			{1, 0, 0, 0, 110, 0, 47, 64, 15, 252, 253, 5, 0,
				248, 249, 250, &KyraEngine::gui_controlsChangeSounds, -1, 0, 34, 49, 0, 0},

	 			{1, 0, 0, 0, 110, 0, 64, 64, 15, 252, 253, 5, 0,
				248, 249, 250, &KyraEngine::gui_controlsChangeWalk, -1, 0, 34, 66, 0, 0},

				{1, 0, 0, 0, 110, 0, 81, 64, 15, 252, 253, 5, 0,
				248, 249, 250, 0, -1, 0, 34, 83, 0, 0 },

				{1, 0, 0, 0, 110, 0, 98, 64, 15, 252, 253, 5, 0,
				248, 249, 250, &KyraEngine::gui_controlsChangeText, -1, 0, 34, 100, 0, 0 },

				{1, 0, 0, 0, 64, 0, 127, 92, 15, 252, 253, -1, 255,
				248, 249, 250, &KyraEngine::gui_controlsApply, -1, -0, 0, 0, 0, 0}
			}
		}
	};
	
	_menu = menu;
}

const uint8 KyraEngine::_magicMouseItemStartFrame[] = {
	0xAD, 0xB7, 0xBE, 0x00
};

const uint8 KyraEngine::_magicMouseItemEndFrame[] = {
	0xB1, 0xB9, 0xC2, 0x00
};

const uint8 KyraEngine::_magicMouseItemStartFrame2[] = {
	0xB2, 0xBA, 0xC3, 0x00
};

const uint8 KyraEngine::_magicMouseItemEndFrame2[] = {
	0xB6, 0xBD, 0xC8, 0x00
};

const uint16 KyraEngine::_amuletX[] = { 231, 275, 253, 253 };
const uint16 KyraEngine::_amuletY[] = { 170, 170, 159, 181 };

const uint16 KyraEngine::_amuletX2[] = { 0x000, 0x0FD, 0x0E7, 0x0FD, 0x113, 0x000 };
const uint16 KyraEngine::_amuletY2[] = { 0x000, 0x09F, 0x0AA, 0x0B5, 0x0AA, 0x000 };

// Kyra 2 and 3 main menu

const char *KyraEngine::_mainMenuStrings[] = {
	"Start a new game",
	"Introduction",
	"Load a game",
	"Exit the game",
	"Nouvelle Partie",
	"Introduction",
	"Charger une partie",
	"Quitter le jeu",
	"Neues Spiel starten",
	"Intro",
	"Spielstand laden",
	"Spiel beenden",
	0
};

// kyra 2 static res

const char *KyraEngine_v2::_introStrings[] = {
	"Kyrandia is disappearing!",
	"Rock by rock...",
	"...and tree by tree.",
	"Kyrandia ceases to exist!",
	"The Royal Mystics are baffled.",
	"Every reference has been consulted.",
	"Even Marko and his new valet have been allowed into the conference.",
	"Luckily, the Hand was experienced in these matters.",
	"And finally a plan was approved...",
	"...that required a magic Anchor Stone...",
	"...to be retrieved from the center of the world.",
	"Zanthia, youngest of the Kyrandian Mystics, has been selected to retrieve the Stone.",
	"Thank you for playing The Hand of Fate.",
	"This should be enough blueberries to open a portal to the center of the world.",
	" DUMMY STRING... ",
	" DUMMY STRING... ",
	"Hey! All my equipment has been stolen!",
	" DUMMY STRING... ",
	"If they think I'm going to walk all the way down there, they're nuts!",
	" DUMMY STRING... ",
	"Hurry up faun!"
};

const int KyraEngine_v2::_introStringsSize = ARRAYSIZE(KyraEngine_v2::_introStrings);

const char *KyraEngine_v2::_introSoundList[] = {
	"eintro1",
	"eintro2",
	"eintro3",
	"eintro4",
	"eintro5",
	"eintro6",
	"eintro7",
	"eintro8",
	"eintro9",
	"eintro10",
	"eintro11",
	"eintro12",
	"eglow",
	"0000210",
	"0000130",
	"0000180",
	"0000160",
	"asong",
	"crowcaw",
	"eyerub2",
	"pluck3",
	"rodnreel",
	"frog1",
	"scavmov2",
	"lambmom3",
	"lambkid1",
	"thunder2",
	"tunder3",
	"wind6",
	"h2odrop2",
	"gasleak",
	"polgulp1",
	"hndslap1",
	"burp1",
	"0000220",
	"0000230",
	"0000250",
	"0000260",
	"0000270",
	"0000280",
	"0000290",
	"0000300",
	"0000310",
	"0000320",
	"0000330",
	"scream1",
	"theend"
};

const int KyraEngine_v2::_introSoundListSize = ARRAYSIZE(KyraEngine_v2::_introSoundList);

// kyra 3 static res

const char *KyraEngine_v3::_soundList[] = {
	"ARREST1.AUD",
	"BATH1.AUD",
	"OCEAN1.AUD",
	"CLOWN1.AUD",
	"DARM2.AUD",
	"FALL1M.AUD",
	"FALL2.AUD",
	"FISH1.AUD",
	"FISHWNDR.AUD",
	"HERMAN1.AUD",
	"JAIL1.AUD",
	"JUNGLE1.AUD",
	"KATHY1.AUD",
	"NICESINE.AUD",
	"PEGASUS1.AUD",
	"PIRATE1.AUD",
	"PIRATE2.AUD",
	"PIRATE3.AUD",
	"POP3.AUD",
	"PORT1.AUD",
	"QUEEN1.AUD",
	"RUINS1.AUD",
	"SNAKES1.AUD",
	"SPRING1.AUD",
	"STATUE1.AUD",
	"STATUE2.AUD",
	"TITLE1.AUD",
	"UNDER1.AUD",
	"WALKCHP1.AUD",
	"YANK1.AUD",
	"ZAN2.AUD",
	"GROOVE2.AUD",
	"GROOVE3.AUD",
	"KING1.AUD",
	"KING2.AUD",
	"GROOVE1.AUD",
	"JAIL2.AUD",
	"SPIRIT1.AUD",
	"SPRING1A.AUD",
	"POP1.AUD",
	"POP2.AUD",
	"SQUIRL1.AUD"
};

const int KyraEngine_v3::_soundListSize = ARRAYSIZE(KyraEngine_v3::_soundList);

const char *KyraEngine_v3::_languageExtension[] = {
	"TRE",
	"TRF",
	"TRG"/*,
	"TRI",		Italian and Spanish was never included
	"TRS"*/
};

const int KyraEngine_v3::_languageExtensionSize = ARRAYSIZE(KyraEngine_v3::_languageExtension);

} // End of namespace Kyra
