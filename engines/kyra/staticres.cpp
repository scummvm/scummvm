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
#include "kyra/kyra.h"
#include "kyra/screen.h"
#include "kyra/resource.h"

namespace Kyra {

#define RESFILE_VERSION 11

#define GAME_FLAGS (GF_FLOPPY | GF_TALKIE | GF_DEMO | GF_AUDIOCD)
#define LANGUAGE_FLAGS (GF_ENGLISH | GF_FRENCH | GF_GERMAN | GF_SPANISH | GF_LNGUNK)

struct LanguageTypes {
	uint32 flags;
	const char *ext;
};

static LanguageTypes languages[] = {
	{ GF_ENGLISH, "ENG" },	// this is the default language
	{ GF_FRENCH, "FRE" },
	{ GF_GERMAN, "GER" },
	{ GF_SPANISH, "SPA" },
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

		{ 0, 0, 0 }
	};

	if (_engine->game() == GI_KYRA1) {
		_builtIn = 0;
		_filenameTable = kyra1StaticRes;
	} else {
		error("unknown game ID");
	}

	int tempSize = 0;
	uint8 *temp = getFile("INDEX", tempSize);
	if (!temp) {
		error("no matching INDEX file found");
	}
	
	uint32 version = READ_BE_UINT32(temp);
	uint32 gameID = READ_BE_UINT32((temp+4));
	uint32 featuresValue = READ_BE_UINT32((temp+8));
	
	delete [] temp;
	temp = 0;
	
	if (version < RESFILE_VERSION) {
		error("invalid KYRA.DAT file version (%d, required %d)", version, RESFILE_VERSION);
	}
	if (gameID != _engine->game()) {
		error("invalid game id (%d)", gameID);
	}
	if ((featuresValue & GAME_FLAGS) != (_engine->features() & GAME_FLAGS)) {
		error("your data file has a different game flags (0x%.08X has the data and your version has 0x%.08X)", (featuresValue & GAME_FLAGS), (_engine->features() & GAME_FLAGS));
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

const char **StaticResource::loadStrings(int id, int &strings) {
	const char **temp = (const char**)getData(id, kStringList, strings);
	if (temp)
		return temp;
	return (const char**)getData(id, kLanguageList, strings);
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

const uint8 **StaticResource::loadPaletteTable(int id, int &entries) {
	return (const uint8**)getData(id, kPaletteTable, entries);
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
		if (languages[i].flags != (_engine->features() & LANGUAGE_FLAGS)) {
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
		warning("coudln't find specific language table for your version, using English now");
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
	if (_engine->features() & GF_TALKIE) {
		ext = ".CD";
	} else if (_engine->features() & GF_DEMO) {
		ext = ".DEM";
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
	loadBitmap("MOUSE.CPS", 3, 3, 0);
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
			loadBitmap(_characterImageTable[shape->imageIndex], 3, 3, 0);
			curImage = shape->imageIndex;
		}
		_shapes[i+7+4] = _screen->encodeShape(shape->x<<3, shape->y, shape->w<<3, shape->h, 1);
	}
	_screen->_curPage = videoPage;
}

void KyraEngine::loadSpecialEffectShapes() {
	loadBitmap("EFFECTS.CPS", 3, 3, 0);
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

	loadBitmap("JEWELS3.CPS", 3, 3, 0);
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


	loadBitmap("ITEMS.CPS", 3, 3, 0);
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
	loadBitmap("BUTTONS2.CPS", 3, 3, 0);
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
	if ((_features & GF_ENGLISH) && (_features & GF_TALKIE)) 
		loadBitmap("MAIN_ENG.CPS", page, page, 0);
	else if(_features & GF_FRENCH)
		loadBitmap("MAIN_FRE.CPS", page, page, 0);
	else if(_features & GF_GERMAN)
		loadBitmap("MAIN_GER.CPS", page, page, 0);
	else if ((_features & GF_ENGLISH) && (_features & GF_FLOPPY))
		loadBitmap("MAIN15.CPS", page, page, 0);
	else if (_features & GF_SPANISH)
		loadBitmap("MAIN_SPA.CPS", page, page, 0);
	else
		warning("no main graphics file found");
	
	uint8 *_pageSrc = _screen->getPagePtr(page);
	uint8 *_pageDst = _screen->getPagePtr(0);
	memcpy(_pageDst, _pageSrc, 320*200);
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

// CD Version *could* use an different opcodeTable
#define Opcode(x) &KyraEngine::x
KyraEngine::OpcodeProc KyraEngine::_opcodeTable[] = {
	// 0x00
	Opcode(cmd_magicInMouseItem),
	Opcode(cmd_characterSays),
	Opcode(cmd_pauseTicks),
	Opcode(cmd_drawSceneAnimShape),
	// 0x04
	Opcode(cmd_queryGameFlag),
	Opcode(cmd_setGameFlag),
	Opcode(cmd_resetGameFlag),
	Opcode(cmd_runNPCScript),
	// 0x08
	Opcode(cmd_setSpecialExitList),
	Opcode(cmd_blockInWalkableRegion),
	Opcode(cmd_blockOutWalkableRegion),
	Opcode(cmd_walkPlayerToPoint),
	// 0x0c
	Opcode(cmd_dropItemInScene),
	Opcode(cmd_drawAnimShapeIntoScene),
	Opcode(cmd_createMouseItem),
	Opcode(cmd_savePageToDisk),
	// 0x10
	Opcode(cmd_sceneAnimOn),
	Opcode(cmd_sceneAnimOff),
	Opcode(cmd_getElapsedSeconds),
	Opcode(cmd_mouseIsPointer),
	// 0x14
	Opcode(cmd_destroyMouseItem),
	Opcode(cmd_runSceneAnimUntilDone),
	Opcode(cmd_fadeSpecialPalette),
	Opcode(cmd_playAdlibSound),
	// 0x18
	Opcode(cmd_playAdlibScore),
	Opcode(cmd_phaseInSameScene),
	Opcode(cmd_setScenePhasingFlag),
	Opcode(cmd_resetScenePhasingFlag),
	// 0x1c
	Opcode(cmd_queryScenePhasingFlag),
	Opcode(cmd_sceneToDirection),
	Opcode(cmd_setBirthstoneGem),
	Opcode(cmd_placeItemInGenericMapScene),
	// 0x20
	Opcode(cmd_setBrandonStatusBit),
	Opcode(cmd_pauseSeconds),
	Opcode(cmd_getCharactersLocation),
	Opcode(cmd_runNPCSubscript),
	// 0x24
	Opcode(cmd_magicOutMouseItem),
	Opcode(cmd_internalAnimOn),
	Opcode(cmd_forceBrandonToNormal),
	Opcode(cmd_poisonDeathNow),
	// 0x28
	Opcode(cmd_setScaleMode),
	Opcode(cmd_openWSAFile),
	Opcode(cmd_closeWSAFile),
	Opcode(cmd_runWSAFromBeginningToEnd),
	// 0x2c
	Opcode(cmd_displayWSAFrame),
	Opcode(cmd_enterNewScene),
	Opcode(cmd_setSpecialEnterXAndY),
	Opcode(cmd_runWSAFrames),
	// 0x30
	Opcode(cmd_popBrandonIntoScene),
	Opcode(cmd_restoreAllObjectBackgrounds),
	Opcode(cmd_setCustomPaletteRange),
	Opcode(cmd_loadPageFromDisk),
	// 0x34
	Opcode(cmd_customPrintTalkString),
	Opcode(cmd_restoreCustomPrintBackground),
	Opcode(cmd_hideMouse),
	Opcode(cmd_showMouse),
	// 0x38
	Opcode(cmd_getCharacterX),
	Opcode(cmd_getCharacterY),
	Opcode(cmd_changeCharactersFacing),
	Opcode(cmd_copyWSARegion),
	// 0x3c
	Opcode(cmd_printText),
	Opcode(cmd_random),
	Opcode(cmd_loadSoundFile),
	Opcode(cmd_displayWSAFrameOnHidPage),
	// 0x40
	Opcode(cmd_displayWSASequentialFrames),
	Opcode(cmd_drawCharacterStanding),
	Opcode(cmd_internalAnimOff),
	Opcode(cmd_changeCharactersXAndY),
	// 0x44
	Opcode(cmd_clearSceneAnimatorBeacon),
	Opcode(cmd_querySceneAnimatorBeacon),
	Opcode(cmd_refreshSceneAnimator),
	Opcode(cmd_placeItemInOffScene),
	// 0x48
	Opcode(cmd_wipeDownMouseItem),
	Opcode(cmd_placeCharacterInOtherScene),
	Opcode(cmd_getKey),
	Opcode(cmd_specificItemInInventory),
	// 0x4c
	Opcode(cmd_popMobileNPCIntoScene),
	Opcode(cmd_mobileCharacterInScene),
	Opcode(cmd_hideMobileCharacter),
	Opcode(cmd_unhideMobileCharacter),
	// 0x50
	Opcode(cmd_setCharactersLocation),
	Opcode(cmd_walkCharacterToPoint),
	Opcode(cmd_specialEventDisplayBrynnsNote),
	Opcode(cmd_specialEventRemoveBrynnsNote),
	// 0x54
	Opcode(cmd_setLogicPage),
	Opcode(cmd_fatPrint),
	Opcode(cmd_preserveAllObjectBackgrounds),
	Opcode(cmd_updateSceneAnimations),
	// 0x58
	Opcode(cmd_sceneAnimationActive),
	Opcode(cmd_setCharactersMovementDelay),
	Opcode(cmd_getCharactersFacing),
	Opcode(cmd_bkgdScrollSceneAndMasksRight),
	// 0x5c
	Opcode(cmd_dispelMagicAnimation),
	Opcode(cmd_findBrightestFireberry),
	Opcode(cmd_setFireberryGlowPalette),
	Opcode(cmd_setDeathHandlerFlag),
	// 0x60
	Opcode(cmd_drinkPotionAnimation),
	Opcode(cmd_makeAmuletAppear),
	Opcode(cmd_drawItemShapeIntoScene),
	Opcode(cmd_setCharactersCurrentFrame),
	// 0x64
	Opcode(cmd_waitForConfirmationMouseClick),
	Opcode(cmd_pageFlip),
	Opcode(cmd_setSceneFile),
	Opcode(cmd_getItemInMarbleVase),
	// 0x68
	Opcode(cmd_setItemInMarbleVase),
	Opcode(cmd_addItemToInventory),
	Opcode(cmd_intPrint),
	Opcode(cmd_shakeScreen),
	// 0x6c
	Opcode(cmd_createAmuletJewel),
	Opcode(cmd_setSceneAnimCurrXY),
	Opcode(cmd_poisonBrandonAndRemaps),
	Opcode(cmd_fillFlaskWithWater),
	// 0x70
	Opcode(cmd_getCharactersMovementDelay),
	Opcode(cmd_getBirthstoneGem),
	Opcode(cmd_queryBrandonStatusBit),
	Opcode(cmd_playFluteAnimation),
	// 0x74
	Opcode(cmd_playWinterScrollSequence),
	Opcode(cmd_getIdolGem),
	Opcode(cmd_setIdolGem),
	Opcode(cmd_totalItemsInScene),
	// 0x78
	Opcode(cmd_restoreBrandonsMovementDelay),
	Opcode(cmd_setMousePos),
	Opcode(cmd_getMouseState),
	Opcode(cmd_setEntranceMouseCursorTrack),
	// 0x7c
	Opcode(cmd_itemAppearsOnGround),
	Opcode(cmd_setNoDrawShapesFlag),
	Opcode(cmd_fadeEntirePalette),
	Opcode(cmd_itemOnGroundHere),
	// 0x80
	Opcode(cmd_queryCauldronState),
	Opcode(cmd_setCauldronState),
	Opcode(cmd_queryCrystalState),
	Opcode(cmd_setCrystalState),
	// 0x84
	Opcode(cmd_setPaletteRange),
	Opcode(cmd_shrinkBrandonDown),
	Opcode(cmd_growBrandonUp),
	Opcode(cmd_setBrandonScaleXAndY),
	// 0x88
	Opcode(cmd_resetScaleMode),
	Opcode(cmd_getScaleDepthTableValue),
	Opcode(cmd_setScaleDepthTableValue),
	Opcode(cmd_message),
	// 0x8c
	Opcode(cmd_checkClickOnNPC),
	Opcode(cmd_getFoyerItem),
	Opcode(cmd_setFoyerItem),
	Opcode(cmd_setNoItemDropRegion),
	// 0x90
	Opcode(cmd_walkMalcolmOn),
	Opcode(cmd_passiveProtection),
	Opcode(cmd_setPlayingLoop),
	Opcode(cmd_brandonToStoneSequence),
	// 0x94
	Opcode(cmd_brandonHealingSequence),
	Opcode(cmd_protectCommandLine),
	Opcode(cmd_pauseMusicSeconds),
	Opcode(cmd_resetMaskRegion),
	// 0x98
	Opcode(cmd_setPaletteChangeFlag),
	Opcode(cmd_fillRect),
	Opcode(cmd_vocUnload),
	Opcode(cmd_vocLoad),
	Opcode(cmd_dummy)
};
#undef Opcode

const int KyraEngine::_opcodeTableSize = ARRAYSIZE(_opcodeTable);

const char *KyraEngine::_musicFiles[] = {
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

const int KyraEngine::_musicFilesCount = ARRAYSIZE(_musicFiles);

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

Button KyraEngine::_buttonData[] = {
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

Button *KyraEngine::_buttonDataListPtr[] = {
	&_buttonData[1],
	&_buttonData[2],
	&_buttonData[3],
	&_buttonData[4],
	&_buttonData[5],
	&_buttonData[6],
	&_buttonData[7],
	&_buttonData[8],
	&_buttonData[9],
	&_buttonData[10],
	&_buttonData[11],
	&_buttonData[12],
	&_buttonData[13],
	&_buttonData[14],
	0
};

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

Menu KyraEngine::_menu[] = {
	{ -1, -1, 208, 136, 248, 249, 250, "The Legend of Kyrandia", 251, -1, 8, 0, 5, -1, -1, -1, -1, 
		{
			{1, 0, 0, "Load a Game", -1, -1, 30, 148, 15, 252, 253, 24, 0,
			248, 249, 250, &KyraEngine::gui_loadGameMenu, -1, 0, 0, 0, 0, 0},

			{1, 0, 0, "Save this Game", -1, -1, 47, 148, 15, 252, 253, 24, 0,
			248, 249, 250, &KyraEngine::gui_saveGameMenu, -1, 0, 0, 0, 0, 0},

			{1, 0, 0, "Game Controls", -1, -1, 64, 148, 15, 252, 253, 24, 0,
			248, 249, 250, &KyraEngine::gui_gameControlsMenu, -1, 0, 0, 0, 0, 0},

			{1, 0, 0, "Quit playing", -1, -1, 81, 148, 15, 252, 253, 24, 0,
			248, 249, 250, &KyraEngine::gui_quitPlaying, -1, 0, 0, 0, 0, 0},

			{1, 0, 0, "Resume game", 86, 0, 110, 92, 15, 252, 253, -1, 255,
			248, 249, 250, &KyraEngine::gui_resumeGame, -1, 0, 0, 0, 0, 0}
		}
	},
	{ -1, -1, 288, 56, 248, 249, 250, 0, 254,-1, 8, 0, 2, -1, -1, -1, -1,
		{
			{1, 0, 0, "Yes", 24, 0, 30, 72, 15, 252, 253, -1, 255,
			248, 249, 250, &KyraEngine::gui_quitConfirmYes, -1, 0, 0, 0, 0, 0},

			{1, 0, 0, "No", 192, 0, 30, 72, 15, 252, 253, -1, 255,
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

			{1, 0, 0, "Cancel", 184, 0, 134, 88, 15, 252, 253, -1, 255,
			248, 249, 250, &KyraEngine::gui_cancelSubMenu, -1, 0, 0, 0, 0, 0},
		}
	},
	{ -1, -1, 288, 67, 248, 249, 250, "Enter a description of your saved game:", 251, -1, 8, 0, 3, -1, -1, -1, -1,
		{
			{1, 0, 0, "Save", 24, 0, 44, 72, 15, 252, 253, -1, 255,
			248, 249, 250, &KyraEngine::gui_savegameConfirm, -1, 0, 0, 0, 0, 0},

			{1, 0, 0, "Cancel", 192, 0, 44, 72, 15, 252, 253, -1, 255,
			248, 249, 250, &KyraEngine::gui_cancelSubMenu, -1, 0, 0, 0, 0, 0}
		}
	},
	{ -1, -1, 208, 76, 248, 249, 250, "Rest in peace, Brandon.", 251, -1, 8, 0, 2, -1, -1, -1, -1, 
		{
			{1, 0, 0, "Load a game", -1, -1, 30, 148, 15, 252, 253, 24, 0,
			248, 249, 250, &KyraEngine::gui_loadGameMenu, -1, 0, 0, 0, 0, 0},

			{1, 0, 0, "Quit playing", -1, -1, 47, 148, 15, 252, 253, 24, 0,
			248, 249, 250, &KyraEngine::gui_quitPlaying, -1, 0, 0, 0, 0, 0}
		}
	},
	{ -1, -1, 208, 153, 248, 249, 250, "Game Controls", 251, -1, 8, 0, 6, -1, -1, -1, -1, 
		{
			{1, 0, 0, 0, 110, 0, 30, 64, 15, 252, 253, 5, 0,
			248, 249, 250, &KyraEngine::gui_controlsChangeMusic, -1, "Music is ", 34, 32, 0, 0},

 			{1, 0, 0, 0, 110, 0, 47, 64, 15, 252, 253, 5, 0,
			248, 249, 250, &KyraEngine::gui_controlsChangeSounds, -1, "Sounds are ", 34, 49, 0, 0},

 			{1, 0, 0, 0, 110, 0, 64, 64, 15, 252, 253, 5, 0,
			248, 249, 250, &KyraEngine::gui_controlsChangeWalk, -1, "Walk speed ", 34, 66, 0, 0},

			{1, 0, 0, 0, 110, 0, 81, 64, 15, 252, 253, 5, 0,
			248, 249, 250, 0, -1, 0, 34, 83, 0, 0 },

			{1, 0, 0, 0, 110, 0, 98, 64, 15, 252, 253, 5, 0,
			248, 249, 250, &KyraEngine::gui_controlsChangeText, -1, "Text speed ", 34, 100, 0, 0 },

			{1, 0, 0, "Main Menu", 64, 0, 127, 92, 15, 252, 253, -1, 255,
			248, 249, 250, &KyraEngine::gui_cancelSubMenu, -1, -0, 0, 0, 0, 0}
		}
	}
};

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
} // End of namespace Kyra
