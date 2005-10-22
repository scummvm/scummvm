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

#define RESFILE_VERSION 1

#define GAME_FLAGS (GF_FLOPPY | GF_TALKIE | GF_DEMO | GF_AUDIOCD)
#define LANGUAGE_FLAGS (GF_ENGLISH | GF_FRENCH | GF_GERMAN | GF_SPANISH | GF_LNGUNK)

byte *getFile(PAKFile &res, const char *filename) {
	uint32 size = 0;
	size = res.getFileSize(filename);
	if (!size)
		return 0;
	const byte *src = res.getFile(filename);
	byte *dst = new byte[size];
	memcpy(dst, src, size);
	return dst;
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
		error("invalid resource file version (%d)", version);
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
	
#define loadRawFile(x, y, z) \
	if (_features & GF_TALKIE) { \
		temp = getFile(x, y ".CD"); \
	} else if (_features & GF_DEMO) { \
		temp = getFile(x, y ".DEM"); \
	} else { \
		temp = getFile(x, y); \
	} \
	if (temp) { \
		z = temp; \
		temp = 0; \
	}
#define loadTable(x, y, z, a) \
	if (_features & GF_TALKIE) { \
		temp = getFile(x, y ".CD"); \
	} else if (_features & GF_DEMO) { \
		temp = getFile(x, y ".DEM"); \
	} else { \
		temp = getFile(x, y); \
	} \
	if (temp) { \
		res_loadTable(temp, z, a); \
		delete [] temp; \
		temp = 0; \
	}
	
	if ((type & RES_INTRO) || type == RES_ALL) {
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
	}
	
#undef loadTable
#undef loadRawFile
}

void KyraEngine::res_unloadResources(int type) {
	debug(9, "res_unloadResources(%d)", type);
	if ((type & RES_INTRO) || type == RES_ALL) {
		for (int i = 0; i < _seq_WSATable_Size; ++i) {
			delete [] _seq_WSATable[i];
		}
		delete [] _seq_WSATable;
		_seq_WSATable_Size = 0;
		_seq_WSATable = 0;
		
		for (int i = 0; i < _seq_CPSTable_Size; ++i) {
			delete [] _seq_CPSTable[i];
		}
		delete [] _seq_CPSTable;
		_seq_CPSTable_Size = 0;
		_seq_CPSTable = 0;
		
		for (int i = 0; i < _seq_COLTable_Size; ++i) {
			delete [] _seq_COLTable[i];
		}
		delete [] _seq_COLTable;
		_seq_COLTable_Size = 0;
		_seq_COLTable = 0;
		
		for (int i = 0; i < _seq_textsTable_Size; ++i) {
			delete [] _seq_textsTable[i];
		}
		delete [] _seq_textsTable;
		_seq_textsTable_Size = 0;
		_seq_textsTable = 0;
		
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

const Screen::DrawShapePlotPixelCallback Screen::_drawShapePlotPixelTable[] = {
	&Screen::drawShapePlotPixelCallback1
	// XXX
};

const int Screen::_drawShapePlotPixelCount = ARRAYSIZE(_drawShapePlotPixelTable);

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

} // End of namespace Kyra
