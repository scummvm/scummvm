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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#include "engines/wintermute/base/font/base_font.h"
#include "engines/wintermute/base/font/base_font_bitmap.h"
#include "engines/wintermute/base/font/base_font_truetype.h"
#include "engines/wintermute/base/base_parser.h"
#include "engines/wintermute/base/base_file_manager.h"

namespace WinterMute {

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMPLEMENT_PERSISTENT(BaseFont, false)

//////////////////////////////////////////////////////////////////////
BaseFont::BaseFont(BaseGame *inGame) : BaseObject(inGame) {

}


//////////////////////////////////////////////////////////////////////
BaseFont::~BaseFont() {
}


//////////////////////////////////////////////////////////////////////
void BaseFont::drawText(byte *text, int x, int y, int width, TTextAlign align, int maxHeight, int maxLength) {
}


//////////////////////////////////////////////////////////////////////
int BaseFont::getTextHeight(byte *text, int width) {
	return 0;
}


//////////////////////////////////////////////////////////////////////
int BaseFont::getTextWidth(byte *text, int maxLength) {
	return 0;
}

/*
//////////////////////////////////////////////////////////////////////
bool BaseFont::loadFile(const char * Filename)
{
    BYTE* Buffer = BaseFileManager::getEngineInstance()->readWholeFile(filename);
    if (Buffer==NULL){
        _gameRef->LOG(0, "BaseFont::LoadFile failed for file '%s'", filename);
        return STATUS_FAILED;
    }

    bool ret;

    _filename = new char [strlen(filename)+1];
    strcpy(_filename, filename);

    if (DID_FAIL(ret = loadBuffer(Buffer))) _gameRef->LOG(0, "Error parsing FONT file '%s'", filename);

    delete[] Buffer;

    return ret;
}


TOKEN_DEF_START
  TOKEN_DEF (FONT)
TOKEN_DEF_END
//////////////////////////////////////////////////////////////////////
bool BaseFont::loadBuffer(byte * Buffer)
{
    TOKEN_TABLE_START(commands)
        TOKEN_TABLE (FONT)
    TOKEN_TABLE_END

    char* params;
    int cmd;
    BaseParser parser;

    if (parser.GetCommand ((char**)&Buffer, commands, (char**)&params)!=TOKEN_FONT){
        _gameRef->LOG(0, "'FONT' keyword expected.");
        return STATUS_FAILED;
    }
    Buffer = (byte *)params;

    while ((cmd = parser.GetCommand ((char**)&Buffer, commands, (char**)&params)) > 0)
    {
        switch (cmd)
        {
            case TOKEN_IMAGE:
                surface_file = (char*)params;
            break;

            case TOKEN_TRANSPARENT:
                parser.scanStr(params, "%d,%d,%d", &r, &g, &b);
                custo_trans = true;
            break;
        }


    }
    if (cmd == PARSERR_TOKENNOTFOUND){
        _gameRef->LOG(0, "Syntax error in FONT definition");
        return STATUS_FAILED;
    }

    return STATUS_OK;
}
*/

//////////////////////////////////////////////////////////////////////////
int BaseFont::getLetterHeight() {
	return 0;
}


//////////////////////////////////////////////////////////////////////////
bool BaseFont::persist(BasePersistenceManager *persistMgr) {

	BaseObject::persist(persistMgr);
	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
BaseFont *BaseFont::createFromFile(BaseGame *gameRef, const Common::String &filename) {
	if (isTrueType(gameRef,  filename)) {
		BaseFontTT *font = new BaseFontTT(gameRef);
		if (font) {
			if (DID_FAIL(font->loadFile(filename))) {
				delete font;
				return NULL;
			}
		}
		return font;
	} else {
		BaseFontBitmap *font = new BaseFontBitmap(gameRef);
		if (font) {
			if (DID_FAIL(font->loadFile(filename))) {
				delete font;
				return NULL;
			}
		}
		return font;
	}
}


TOKEN_DEF_START
TOKEN_DEF(FONT)
TOKEN_DEF(TTFONT)
TOKEN_DEF_END
//////////////////////////////////////////////////////////////////////////
bool BaseFont::isTrueType(BaseGame *gameRef, const Common::String &filename) {
	TOKEN_TABLE_START(commands)
	TOKEN_TABLE(FONT)
	TOKEN_TABLE(TTFONT)
	TOKEN_TABLE_END


	byte *buffer = BaseFileManager::getEngineInstance()->readWholeFile(filename);
	if (buffer == NULL) {
		return false;
	}

	byte *workBuffer = buffer;

	char *params;
	BaseParser parser;

	bool ret = false;
	if (parser.getCommand((char **)&workBuffer, commands, (char **)&params) == TOKEN_TTFONT) {
		ret = true;
	}

	delete[] buffer;
	return ret;
}

} // end of namespace WinterMute
