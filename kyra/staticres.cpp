/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004-2005 The ScummVM project
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
 * $Header$
 *
 */

#include "common/stdafx.h"
#include "kyra/kyra.h"
#include "kyra/screen.h"
#include "kyra/resource.h"

namespace Kyra {

#define RESFILE_VERSION 11

#define GAME_FLAGS (GF_FLOPPY | GF_TALKIE | GF_DEMO | GF_AUDIOCD)
#define LANGUAGE_FLAGS (GF_ENGLISH | GF_FRENCH | GF_GERMAN | GF_SPANISH | GF_LNGUNK)

byte *getFile(PAKFile &res, const char *filename) {
	uint32 size = 0;
	size = res.getFileSize(filename);
	if (!size)
		return 0;
	return res.getFile(filename);
}

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

void KyraEngine::res_loadResources(int type) {
	debug(9, "res_loadResources(%d)", type);
	PAKFile resFile("KYRA.DAT");
	if (!resFile.isValid() || !resFile.isOpen()) {
		error("couldn't open Kyrandia resource file ('KYRA.DAT') make sure you got one file for your version");
	}
	
	uint32 version = 0;
	uint32 gameID = 0;
	uint32 featuresValue = 0;
	bool loadNativeLanguage = true;
	
	byte *temp = 0;
	
	if (_features & GF_TALKIE) {
		temp = getFile(resFile, "INDEX.CD");
	} else if (_features & GF_DEMO) {
		temp = getFile(resFile, "INDEX.DEM");
	} else {
		temp = getFile(resFile, "INDEX");
	}
	if (!temp) {
		error("no matching INDEX file found");
	}
	
	version = READ_BE_UINT32(temp);
	gameID = READ_BE_UINT32((temp+4));
	featuresValue = READ_BE_UINT32((temp+8));
	
	delete [] temp;
	temp = 0;
	
	if (version < RESFILE_VERSION) {
		error("invalid KYRA.DAT file version (%d, required %d)", version, RESFILE_VERSION);
	}
	if (gameID != _game) {
		error("invalid game id (%d)", gameID);
	}
	if ((featuresValue & GAME_FLAGS) != (_features & GAME_FLAGS)) {
		error("your data file has a different game flags (0x%.08X has the data and your version has 0x%.08X)", (featuresValue & GAME_FLAGS), (_features & GAME_FLAGS));
	}
	
	if (!((featuresValue & LANGUAGE_FLAGS) & (_features & LANGUAGE_FLAGS))) {
		char buffer[240];
		sprintf(buffer, "your data file has support for:");
		if (featuresValue & GF_ENGLISH) {
			sprintf(buffer + strlen(buffer), " English");
		}		
		if (featuresValue & GF_FRENCH) {
			sprintf(buffer + strlen(buffer), " French");
		}		
		if (featuresValue & GF_GERMAN) {
			sprintf(buffer + strlen(buffer), " German");
		}
		if (featuresValue & GF_SPANISH) {
			sprintf(buffer + strlen(buffer), " Spanish");
		}
		sprintf(buffer + strlen(buffer), " but not your language (");		
		if (_features & GF_ENGLISH) {
			sprintf(buffer + strlen(buffer), "English");
		} else if (_features & GF_FRENCH) {
			sprintf(buffer + strlen(buffer), "French");
		} else if (_features & GF_GERMAN) {
			sprintf(buffer + strlen(buffer), "German");
		} else if (_features & GF_SPANISH) {
			sprintf(buffer + strlen(buffer), "Spanish");
		} else {
			sprintf(buffer + strlen(buffer), "unknown");
		}
		sprintf(buffer + strlen(buffer), ")");		
		warning(buffer);
		loadNativeLanguage = false;
	}
	
#define getFileEx(x, y) \
	if (_features & GF_TALKIE) { \
		temp = getFile(x, y ".CD"); \
	} else if (_features & GF_DEMO) { \
		temp = getFile(x, y ".DEM"); \
	} else { \
		temp = getFile(x, y); \
	}
#define loadRawFile(x, y, z) \
	getFileEx(x, y) \
	if (temp) { \
		z = temp; \
		temp = 0; \
	}
#define loadTable(x, y, z, a) \
	getFileEx(x, y) \
	if (temp) { \
		res_loadTable(temp, z, a); \
		delete [] temp; \
		temp = 0; \
	}
#define loadRooms(x, y, z, a) \
	getFileEx(x, y) \
	if (temp) { \
		res_loadRoomTable(temp, z, a); \
		delete [] temp; \
		temp = 0; \
	}
#define loadShapes(x, y, z, a) \
	getFileEx(x, y) \
	if (temp) { \
		res_loadShapeTable(temp, z, a); \
		delete [] temp; \
		temp = 0; \
	}
	
	
	if ((type & RES_INTRO) || (type & RES_OUTRO) || type == RES_ALL) {
		loadRawFile(resFile, "FOREST.SEQ", _seq_Forest);
		loadRawFile(resFile, "KALLAK-WRITING.SEQ", _seq_KallakWriting);
		loadRawFile(resFile, "KYRANDIA-LOGO.SEQ", _seq_KyrandiaLogo);
		loadRawFile(resFile, "KALLAK-MALCOLM.SEQ", _seq_KallakMalcolm);
		loadRawFile(resFile, "MALCOLM-TREE.SEQ", _seq_MalcolmTree);
		loadRawFile(resFile, "WESTWOOD-LOGO.SEQ", _seq_WestwoodLogo);
		loadRawFile(resFile, "DEMO1.SEQ", _seq_Demo1);
		loadRawFile(resFile, "DEMO2.SEQ", _seq_Demo2);
		loadRawFile(resFile, "DEMO3.SEQ", _seq_Demo3);
		loadRawFile(resFile, "DEMO4.SEQ", _seq_Demo4);
		
		loadTable(resFile, "INTRO-CPS.TXT", (byte***)&_seq_CPSTable, &_seq_CPSTable_Size);
		loadTable(resFile, "INTRO-COL.TXT", (byte***)&_seq_COLTable, &_seq_COLTable_Size);
		loadTable(resFile, "INTRO-WSA.TXT", (byte***)&_seq_WSATable, &_seq_WSATable_Size);
		
		res_loadLangTable("INTRO-STRINGS.", &resFile, (byte***)&_seq_textsTable, &_seq_textsTable_Size, loadNativeLanguage);
		
		loadRawFile(resFile, "REUNION.SEQ", _seq_Reunion);
		
		res_loadLangTable("HOME.", &resFile, (byte***)&_homeString, &_homeString_Size, loadNativeLanguage);
	}
	
	if ((type & RES_INGAME) || type == RES_ALL) {
		loadTable(resFile, "ROOM-FILENAMES.TXT", (byte***)&_roomFilenameTable, &_roomFilenameTableSize);
		loadRooms(resFile, "ROOM-TABLE.ROOM", &_roomTable, &_roomTableSize);
		
		loadTable(resFile, "CHAR-IMAGE.TXT", (byte***)&_characterImageTable, &_characterImageTableSize);
		
		loadShapes(resFile, "SHAPES-DEFAULT.SHP", &_defaultShapeTable, &_defaultShapeTableSize);
		
		res_loadLangTable("ITEMLIST.", &resFile, (byte***)&_itemList, &_itemList_Size, loadNativeLanguage);
		res_loadLangTable("TAKEN.", &resFile, (byte***)&_takenList, &_takenList_Size, loadNativeLanguage);
		res_loadLangTable("PLACED.", &resFile, (byte***)&_placedList, &_placedList_Size, loadNativeLanguage);
		res_loadLangTable("DROPPED.", &resFile, (byte***)&_droppedList, &_droppedList_Size, loadNativeLanguage);
		res_loadLangTable("NODROP.", &resFile, (byte***)&_noDropList, &_noDropList_Size, loadNativeLanguage);
		
		loadRawFile(resFile, "AMULETEANIM.SEQ", _amuleteAnim);
		
		for (int i = 1; i <= 33; ++i) {
			char buffer[32];
			sprintf(buffer, "PALTABLE%d.PAL", i);
			if (_features & GF_TALKIE) {
				strcat(buffer, ".CD");
			} else if (_features & GF_DEMO) {
				strcat(buffer, ".DEM");
			}
			temp = getFile(resFile, buffer);
			if (temp) {
				_specialPalettes[i-1] = temp;
				temp = 0;
			}
		}
		
		res_loadLangTable("PUTDOWN.", &resFile, (byte***)&_putDownFirst, &_putDownFirst_Size, loadNativeLanguage);
		res_loadLangTable("WAITAMUL.", &resFile, (byte***)&_waitForAmulet, &_waitForAmulet_Size, loadNativeLanguage);
		res_loadLangTable("BLACKJEWEL.", &resFile, (byte***)&_blackJewel, &_blackJewel_Size, loadNativeLanguage);
		res_loadLangTable("POISONGONE.", &resFile, (byte***)&_poisonGone, &_poisonGone_Size, loadNativeLanguage);
		res_loadLangTable("HEALINGTIP.", &resFile, (byte***)&_healingTip, &_healingTip_Size, loadNativeLanguage);
		
		loadShapes(resFile, "HEALING.SHP", &_healingShapeTable, &_healingShapeTableSize);
		loadShapes(resFile, "HEALING2.SHP", &_healingShape2Table, &_healingShape2TableSize);
		
		res_loadLangTable("THEPOISON.", &resFile, (byte***)&_thePoison, &_thePoison_Size, loadNativeLanguage);
		res_loadLangTable("FLUTE.", &resFile, (byte***)&_fluteString, &_fluteString_Size, loadNativeLanguage);
		
		loadShapes(resFile, "POISONDEATH.SHP", &_posionDeathShapeTable, &_posionDeathShapeTableSize);
		loadShapes(resFile, "FLUTE.SHP", &_fluteAnimShapeTable, &_fluteAnimShapeTableSize);
		
		loadShapes(resFile, "WINTER1.SHP", &_winterScrollTable, &_winterScrollTableSize);
		loadShapes(resFile, "WINTER2.SHP", &_winterScroll1Table, &_winterScroll1TableSize);
		loadShapes(resFile, "WINTER3.SHP", &_winterScroll2Table, &_winterScroll2TableSize);
		loadShapes(resFile, "DRINK.SHP", &_drinkAnimationTable, &_drinkAnimationTableSize);
		loadShapes(resFile, "WISP.SHP", &_brandonToWispTable, &_brandonToWispTableSize);
		loadShapes(resFile, "MAGICANIM.SHP", &_magicAnimationTable, &_magicAnimationTableSize);
		loadShapes(resFile, "BRANSTONE.SHP", &_brandonStoneTable, &_brandonStoneTableSize);
		
		res_loadLangTable("WISPJEWEL.", &resFile, (byte***)&_wispJewelStrings, &_wispJewelStrings_Size, loadNativeLanguage);
		res_loadLangTable("MAGICJEWEL.", &resFile, (byte***)&_magicJewelString, &_magicJewelString_Size, loadNativeLanguage);
		
		res_loadLangTable("FLASKFULL.", &resFile, (byte***)&_flaskFull, &_fullFlask_Size, loadNativeLanguage);
		res_loadLangTable("FULLFLASK.", &resFile, (byte***)&_fullFlask, &_fullFlask_Size, loadNativeLanguage);
		
		res_loadLangTable("VERYCLEVER.", &resFile, (byte***)&_veryClever, &_veryClever_Size, loadNativeLanguage);
	}

#undef loadRooms
#undef loadTable
#undef loadRawFile
#undef getFileEx
}

void KyraEngine::res_unloadResources(int type) {
	debug(9, "res_unloadResources(%d)", type);
	if ((type & RES_INTRO) || (type & RES_OUTRO) || type & RES_ALL) {
		res_freeLangTable(&_seq_WSATable, &_seq_WSATable_Size);
		res_freeLangTable(&_seq_CPSTable, &_seq_CPSTable_Size);
		res_freeLangTable(&_seq_COLTable, &_seq_COLTable_Size);
		res_freeLangTable(&_seq_textsTable, &_seq_textsTable_Size);
		
		delete [] _seq_Forest; _seq_Forest = 0;
		delete [] _seq_KallakWriting; _seq_KallakWriting = 0;
		delete [] _seq_KyrandiaLogo; _seq_KyrandiaLogo = 0;
		delete [] _seq_KallakMalcolm; _seq_KallakMalcolm = 0;
		delete [] _seq_MalcolmTree; _seq_MalcolmTree = 0;
		delete [] _seq_WestwoodLogo; _seq_WestwoodLogo = 0;
		delete [] _seq_Demo1; _seq_Demo1 = 0;
		delete [] _seq_Demo2; _seq_Demo2 = 0;
		delete [] _seq_Demo3; _seq_Demo3 = 0;
		delete [] _seq_Demo4; _seq_Demo4 = 0;
		
		delete [] _seq_Reunion; _seq_Reunion = 0;
		res_freeLangTable(&_homeString, &_homeString_Size);
	}
	
	if ((type & RES_INGAME) || type & RES_ALL) {
		res_freeLangTable(&_roomFilenameTable, &_roomFilenameTableSize);
				
		delete [] _roomTable; _roomTable = 0;
		_roomTableSize = 0;
		
		res_freeLangTable(&_characterImageTable, &_characterImageTableSize);
				
		delete [] _defaultShapeTable;
		_defaultShapeTable = 0;
		_defaultShapeTableSize = 0;
		
		res_freeLangTable(&_itemList, &_itemList_Size);
		res_freeLangTable(&_takenList, &_takenList_Size);
		res_freeLangTable(&_placedList, &_placedList_Size);
		res_freeLangTable(&_droppedList, &_droppedList_Size);
		res_freeLangTable(&_noDropList, &_noDropList_Size);
				
		delete [] _amuleteAnim;
		_amuleteAnim = 0;
		
		for (int i = 0; i < 33; ++i) {
			delete [] _specialPalettes[i];
			_specialPalettes[i] = 0;
		}
		
		res_freeLangTable(&_putDownFirst, &_putDownFirst_Size);
		res_freeLangTable(&_waitForAmulet, &_waitForAmulet_Size);
		res_freeLangTable(&_blackJewel, &_blackJewel_Size);
		res_freeLangTable(&_poisonGone, &_poisonGone_Size);
		res_freeLangTable(&_healingTip, &_healingTip_Size);
		
		delete [] _healingShapeTable;
		_healingShapeTable = 0;
		_healingShapeTableSize = 0;
		
		delete [] _healingShape2Table;
		_healingShape2Table = 0;
		_healingShape2TableSize = 0;
		
		res_freeLangTable(&_thePoison, &_thePoison_Size);
		res_freeLangTable(&_fluteString, &_fluteString_Size);
		
		delete [] _posionDeathShapeTable;
		_posionDeathShapeTable = 0;
		_posionDeathShapeTableSize = 0;
		
		delete [] _fluteAnimShapeTable;
		_fluteAnimShapeTable = 0;
		_fluteAnimShapeTableSize = 0;
		
		delete [] _winterScrollTable;
		_winterScrollTable = 0;
		_winterScrollTableSize = 0;
		
		delete [] _winterScroll1Table;
		_winterScroll1Table = 0;
		_winterScroll1TableSize = 0;
		
		delete [] _winterScroll2Table;
		_winterScroll2Table = 0;
		_winterScroll2TableSize = 0;
		
		delete [] _drinkAnimationTable;
		_drinkAnimationTable = 0;
		_drinkAnimationTableSize = 0;
		
		delete [] _brandonToWispTable;
		_brandonToWispTable = 0;
		_brandonToWispTableSize = 0;
		
		delete [] _magicAnimationTable;
		_magicAnimationTable = 0;
		_magicAnimationTableSize = 0;
		
		delete [] _brandonStoneTable;
		_brandonStoneTable = 0;
		_brandonStoneTableSize = 0;
		
		res_freeLangTable(&_flaskFull, &_flaskFull_Size);
		res_freeLangTable(&_fullFlask, &_fullFlask_Size);
		
		res_freeLangTable(&_veryClever, &_veryClever_Size);
	}
}

void KyraEngine::res_loadLangTable(const char *filename, PAKFile *res, byte ***loadTo, int *size, bool nativ) {
	char file[36];
	for (int i = 0; languages[i].ext; ++i) {
		if (languages[i].flags != (_features & LANGUAGE_FLAGS) && nativ) {
			continue; 
		}
			
		strcpy(file, filename);
		strcat(file, languages[i].ext);
		if (_features & GF_TALKIE) {
			strcat(file, ".CD");
		} else if (_features & GF_DEMO) {
			strcat(file, ".DEM");
		}
		byte *temp = getFile(*res, file);
		if (temp) {
			res_loadTable(temp, loadTo, size);
			delete [] temp;
			temp = 0;
		} else {
			if (!nativ)
				continue;
		}
		break;
	}
}

void KyraEngine::res_loadTable(const byte *src, byte ***loadTo, int *size) {
	uint32 count = READ_BE_UINT32(src); src += 4;
	*size = count;
	*loadTo = new byte*[count];
		
	const char *curPos = (const char*)src;
	for (uint32 i = 0; i < count; ++i) {
		int strLen = strlen(curPos);
		(*loadTo)[i] = new byte[strLen+1];
		memcpy((*loadTo)[i], curPos, strLen+1);
		curPos += strLen+1;
	}
}

void KyraEngine::res_loadRoomTable(const byte *src, Room **loadTo, int *size) {
	uint32 count = READ_BE_UINT32(src); src += 4;
	*size = count;
	*loadTo = new Room[count];
	
	for (uint32 i = 0; i < count; ++i) {
		(*loadTo)[i].nameIndex = *src++;
		(*loadTo)[i].northExit = READ_BE_UINT16(src); src += 2;
		(*loadTo)[i].eastExit = READ_BE_UINT16(src); src += 2;
		(*loadTo)[i].southExit = READ_BE_UINT16(src); src += 2;
		(*loadTo)[i].westExit = READ_BE_UINT16(src); src += 2;
		memset(&(*loadTo)[i].itemsTable[0], 0xFF, sizeof(byte)*6);
		memset(&(*loadTo)[i].itemsTable[6], 0, sizeof(byte)*6);
		memset((*loadTo)[i].itemsXPos, 0, sizeof(uint16)*12);
		memset((*loadTo)[i].itemsYPos, 0, sizeof(uint8)*12);
		memset((*loadTo)[i].needInit, 0, sizeof((*loadTo)[i].needInit));
	}
}

void KyraEngine::res_loadShapeTable(const byte *src, Shape **loadTo, int *size) {
	uint32 count = READ_BE_UINT32(src); src += 4;
	*size = count;
	*loadTo = new Shape[count];
	
	for (uint32 i = 0; i < count; ++i) {
		(*loadTo)[i].imageIndex = *src++;
		(*loadTo)[i].x = *src++;
		(*loadTo)[i].y = *src++;
		(*loadTo)[i].w = *src++;
		(*loadTo)[i].h = *src++;
		(*loadTo)[i].xOffset = *src++;
		(*loadTo)[i].yOffset = *src++;
	}
}

void KyraEngine::res_freeLangTable(char ***string, int *size) {
	if (!string || !size)
		return;
	if (!*size || !*string)
		return;
	for (int i = 0; i < *size; ++i) {
		delete [] (*string)[i];
	}
	delete [] *string;
	size = 0;
	*string = 0;
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
	Opcode(cmd_dummy),
	Opcode(cmd_dummy),
	Opcode(cmd_dummy)
};
#undef Opcode

const int KyraEngine::_opcodeTableSize = ARRAYSIZE(_opcodeTable);

const char *KyraEngine::_xmidiFiles[] = {
	"INTRO.XMI",
	"KYRA1A.XMI",
	"KYRA1B.XMI",
	"KYRA2A.XMI",
	"KYRA3A.XMI",
	"KYRA4A.XMI",
	"KYRA4B.XMI",
	"KYRA5A.XMI",
	"KYRA5B.XMI",
	"KYRAMISC.XMI"
};

const int KyraEngine::_xmidiFilesCount = ARRAYSIZE(_xmidiFiles);

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
	{ 0, 0x01, /*XXX,*/1, 1, 1, /*XXX,*/ 0x0487, 0, 0, 0, 0, 0, 0, 0, 0x009, 0xA4, 0x36, 0x1E, /*XXX,*/ 0, 0/*opt_handleMenu, XXX*/ },
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
