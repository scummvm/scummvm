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
 * This code is based on original Tony Tough source code
 *
 * Copyright (c) 1997-2003 Nayma Software
 */

#include "common/textconsole.h"
#include "tony/mpal/mpalutils.h"
#include "tony/font.h"
#include "tony/input.h"
#include "tony/inventory.h"
#include "tony/loc.h"
#include "tony/tony.h"

namespace Tony {

/****************************************************************************\
*       RMFont Methods
\****************************************************************************/

RMFont::RMFont() {
	_letter = NULL;
}

RMFont::~RMFont() {
	unload();
}


/**
 * Dumps a font to a buffer
 * @param buf                   Buffer for font contents
 * @param nChars                Number of characters (max 256)
 * @param dimx                  X dimension in pixels
 * @param dimy                  Y dimension in pixels
*
\****************************************************************************/

void DumpFontBMP(const char *filename, const byte *buf, int nChars, int charX, int charY, byte *pal) {
	error("DumpFontBMP not supported in ScummVM");
}


void RMFont::load(const byte *buf, int nChars, int dimx, int dimy, uint32 palResID) {
	_letter = new RMGfxSourceBuffer8RLEByte[nChars];

	// Initialise the fonts
	for (int i = 0; i < nChars; i++) {
		// Initialise the buffer with the letters
		_letter[i].init(buf + i * (dimx * dimy + 8) + 8, dimx, dimy);
		_letter[i].loadPaletteWA(palResID);
	}

	_fontDimx = dimx;
	_fontDimy = dimy;

	nLetters = nChars;
}

void RMFont::load(uint32 resID, int nChars, int dimx, int dimy, uint32 palResID) {
	RMRes res(resID);

	if ((int)res.size() < nChars * (dimy * dimx + 8))
		nChars = res.size() / (dimy * dimx + 8);

	load(res, nChars, dimx, dimy, palResID);
}

void RMFont::unload(void) {
	if (_letter != NULL) {
		delete[] _letter;
		_letter = NULL;
	}
}


RMGfxPrimitive *RMFont::makeLetterPrimitive(byte bChar, int &nLength) {
	RMFontPrimitive *prim;
	int nLett;

	// Convert from character to glyph index
	nLett = convertToLetter(bChar);
	assert(nLett < nLetters);

	// Create primitive font
	prim = new RMFontPrimitive(this);
	prim->_nChar = nLett;

	// Get the length of the character in pixels
	nLength = letterLength(bChar);

	return prim;
}

void RMFont::draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim2) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	RMFontPrimitive *prim = (RMFontPrimitive *)prim2;

	CORO_BEGIN_CODE(_ctx);

	// Call the draw method of the letter assigned to the primitive
	if (prim->_nChar != -1)
		CORO_INVOKE_2(_letter[prim->_nChar].draw, bigBuf, prim);

	CORO_END_CODE;
}

void RMFont::close(void) {
	unload();
}

int RMFont::stringLen(const RMString &text) {
	int len, i;

	len = 0;
	for (i = 0; i < text.length() - 1; i++)
		len += letterLength(text[i], text[i + 1]);
	len += letterLength(text[i]);

	return len;
}

int RMFont::stringLen(char bChar, char bNext) {
	return letterLength(bChar, bNext);
}

/****************************************************************************\
*       Metodi di RMFontColor
\****************************************************************************/

RMFontColor::RMFontColor() : RMFont() {
	m_r = m_g = m_b = 255;
}

RMFontColor::~RMFontColor() {
}

void RMFontColor::setBaseColor(byte r1, byte g1, byte b1) {
	int r = (int)r1 << 16;
	int g = (int)g1 << 16;
	int b = (int)b1 << 16;

	int rstep = r / 14;
	int gstep = g / 14;
	int bstep = b / 14;

	int i;
	byte pal[768 * 3];

	// Check if we are already on the right colour
	if (m_r == r1 && m_g == g1 && m_b == b1)
		return;

	m_r = r1;
	m_g = g1;
	m_b = b1;

	// Constructs a new paletter for the font
	for (i = 1; i < 16; i++) {
		pal[i * 3 + 0] = r >> 16;
		pal[i * 3 + 1] = g >> 16;
		pal[i * 3 + 2] = b >> 16;

		r -= rstep;
		g -= gstep;
		b -= bstep;
	}

	pal[15 * 3 + 0] += 8;
	pal[15 * 3 + 1] += 8;
	pal[15 * 3 + 2] += 8;

	// Puts in all the letters
	for (i = 0; i < nLetters; i++)
		_letter[i].loadPaletteWA(pal);
}


/***************************************************************************\
*       RMFontParla Methods
\****************************************************************************/

void RMFontParla::init(void) {
	int i;

	// bernie: Number of characters in the font
	int nchars =
	    112    // base
	    + 18    // polish
	    + 66    // russian
	    + 30    // czech
	    +  8    // french
	    +  5;   // deutsch

	load(RES_F_PARL, nchars, 20, 20);

	// Initialise the f**king table
	lDefault = 13;
	hDefault = 18;
	Common::fill(&l2Table[0][0], &l2Table[0][0] + (256 * 256), '\0');
	for (i = 0; i < 256; i++) {
		cTable[i] = -1;
		lTable[i] = lDefault;
	}

	for (i = 0; i < 26; i++)
		cTable['A' + i] = i + 0;

	for (i = 0; i < 26; i++)
		cTable['a' + i] = i + 26;

	for (i = 0; i < 10; i++)
		cTable['0' + i] = i + 52;

	cTable[';'] = 62;
	cTable[','] = 63;
	cTable['.'] = 64;
	cTable[':'] = 65;
	cTable['-'] = 66;
	cTable['_'] = 67;
	cTable['+'] = 68;
	cTable['<'] = 69;
	cTable['>'] = 70;
	cTable['!'] = 71;
	//cTable['!'] = 72;  Exclamation countdown
	cTable['?'] = 73;
	//cTable['?'] = 74;  Question down
	cTable['('] = 75;
	cTable[')'] = 76;
	cTable['\"'] = 77;
	cTable['^'] = 77;
	cTable['/'] = 78;
	cTable[(byte)'£'] = 79;
	cTable['$'] = 80;
	cTable['%'] = 81;
	cTable['&'] = 82;
	cTable['='] = 83;
	cTable[(byte)'«'] = 84;
	cTable[(byte)'»'] = 85;
	cTable[(byte)'®'] = 86;
	cTable[(byte)'©'] = 87;
	cTable[(byte)'à'] = 88;
	cTable[(byte)'è'] = 89;
	cTable[(byte)'é'] = 89;
	cTable[(byte)'ì'] = 90;
	cTable[(byte)'ò'] = 91;
	cTable[(byte)'ù'] = 92;
	cTable[(byte)'ä'] = 93;
	cTable[(byte)'ë'] = 94;
	cTable[(byte)'ï'] = 95;
	cTable[(byte)'ö'] = 96;
	cTable[(byte)'ü'] = 97;
	cTable[(byte)'ÿ'] = 98;
	cTable[(byte)'å'] = 99;
	//cTable[' '] = 100;  e circlet
	//cTable[' '] = 101;  i circlet
	//cTable[' '] = 102;  o circlet
	//cTable[' '] = 103;  u circlet
	cTable[(byte)'ñ'] = 104;
	cTable[(byte)'Ñ'] = 105;
	cTable[(byte)'ç'] = 106;
	cTable[(byte)'æ'] = 107;
	cTable[(byte)'Æ'] = 108;
	cTable[(byte)'ø'] = 109;
	//cTable['ƒ'] = 110;  integral
	cTable['\''] = 111;

	// Little lengths
	lTable[' '] = 9;
	lTable['\''] = 5;
	lTable['.'] = 5;
	lTable[','] = 5;
	lTable[':'] = 5;
	lTable[';'] = 5;
	lTable['!'] = 5;
	lTable['?'] = 10;
	lTable['\"'] = 5;
	lTable['^'] = 5;
	lTable['('] = 7;
	lTable[')'] = 7;

	lTable['4'] = 10;

	lTable['a'] = 14;
	lTable['b'] = 15;
	lTable['c'] = 12;
	lTable['e'] = 12;
	lTable['i'] = 6;
	lTable['ì'] = 6;
	lTable['l'] = 5;
	lTable['m'] = 16;
	lTable['n'] = 12;
	lTable['o'] = 11;
	lTable['p'] = 11;
	lTable['s'] = 12;
	lTable['u'] = 12;

	lTable['E'] = 10;
	lTable['F'] = 11;

	if (_vm->getLanguage() == Common::PL_POL) {
		// Polish characters
		//AaCcEeLlNnOoSsZzZz
		//¥¹ÆæÊê£³ÑñÓóŒœ¯¿Ÿ

		cTable[(byte)'¥'] = 112;
		cTable[(byte)'¹'] = 113;
		cTable[(byte)'Æ'] = 114;
		cTable[(byte)'æ'] = 115;
		cTable[(byte)'Ê'] = 116;
		cTable[(byte)'ê'] = 117;
		cTable[(byte)'£'] = 118;
		cTable[(byte)'³'] = 119;
		cTable[(byte)'Ñ'] = 120;
		cTable[(byte)'ñ'] = 121;
		cTable[(byte)'Ó'] = 122;
		cTable[(byte)'ó'] = 123;
		cTable[(byte)'Œ'] = 124;
		cTable[(byte)'œ'] = 125;
		cTable[(byte)'¯'] = 126;
		cTable[(byte)'¿'] = 127;
		cTable[(byte)''] = 128;
		cTable[(byte)'Ÿ'] = 129;

		lTable[(byte)'¥'] = 14;
		lTable[(byte)'¹'] = 16;
		lTable[(byte)'Æ'] = 12;
		lTable[(byte)'æ'] = 12;
		lTable[(byte)'Ê'] = 11;
		lTable[(byte)'ê'] = 12;
		lTable[(byte)'£'] = 14;
		lTable[(byte)'³'] = 9;
		lTable[(byte)'Ñ'] = 10;
		lTable[(byte)'ñ'] = 11;
		lTable[(byte)'Ó'] = 13;
		lTable[(byte)'ó'] = 11;
		lTable[(byte)'Œ'] = 12;
		lTable[(byte)'œ'] = 12;
		lTable[(byte)'¯'] = 13;
		lTable[(byte)'¿'] = 13;
		lTable[(byte)''] = 14;
		lTable[(byte)'Ÿ'] = 13;

	} else if (_vm->getLanguage() == Common::RU_RUS) {

		// Russian Characters
		// WARNING: The Russian font uses many of the ISO-Latin-1 font,
		// allowing for further translations. To support Tonyin other langauges,
		// these mappings could be used as a basis

		cTable[(byte)'À'] = 130;
		cTable[(byte)'Á'] = 131;
		cTable[(byte)'Â'] = 132;
		cTable[(byte)'Ã'] = 133;
		cTable[(byte)'Ä'] = 134;
		cTable[(byte)'Å'] = 135;
		cTable[(byte)'¨'] = 136;
		cTable[(byte)'Æ'] = 137;
		cTable[(byte)'Ç'] = 138;
		cTable[(byte)'È'] = 139;
		cTable[(byte)'É'] = 140;
		cTable[(byte)'Ê'] = 141;
		cTable[(byte)'Ë'] = 142;
		cTable[(byte)'Ì'] = 143;
		cTable[(byte)'Í'] = 144;
		cTable[(byte)'Î'] = 145;
		cTable[(byte)'Ï'] = 146;
		cTable[(byte)'Ð'] = 147;
		cTable[(byte)'Ñ'] = 148;
		cTable[(byte)'Ò'] = 149;
		cTable[(byte)'Ó'] = 150;
		cTable[(byte)'Ô'] = 151;
		cTable[(byte)'Õ'] = 152;
		cTable[(byte)'Ö'] = 153;
		cTable[(byte)'×'] = 154;
		cTable[(byte)'Ø'] = 155;
		cTable[(byte)'Ù'] = 156;
		cTable[(byte)'Ü'] = 157;
		cTable[(byte)'Ú'] = 158;
		cTable[(byte)'Û'] = 159;
		cTable[(byte)'Ý'] = 160;
		cTable[(byte)'Þ'] = 161;
		cTable[(byte)'ß'] = 162;

		cTable[(byte)'à'] = 163;
		cTable[(byte)'á'] = 164;
		cTable[(byte)'â'] = 165;
		cTable[(byte)'ã'] = 166;
		cTable[(byte)'ä'] = 167;
		cTable[(byte)'å'] = 168;
		cTable[(byte)'¸'] = 169;
		cTable[(byte)'æ'] = 170;
		cTable[(byte)'ç'] = 171;
		cTable[(byte)'è'] = 172;
		cTable[(byte)'é'] = 173;
		cTable[(byte)'ê'] = 174;
		cTable[(byte)'ë'] = 175;
		cTable[(byte)'ì'] = 176;
		cTable[(byte)'í'] = 177;
		cTable[(byte)'î'] = 178;
		cTable[(byte)'ï'] = 179;
		cTable[(byte)'ð'] = 180;
		cTable[(byte)'ñ'] = 181;
		cTable[(byte)'ò'] = 182;
		cTable[(byte)'ó'] = 183;
		cTable[(byte)'ô'] = 184;
		cTable[(byte)'õ'] = 185;
		cTable[(byte)'ö'] = 186;
		cTable[(byte)'÷'] = 187;
		cTable[(byte)'ø'] = 188;
		cTable[(byte)'ù'] = 189;
		cTable[(byte)'ü'] = 190;
		cTable[(byte)'ú'] = 191;
		cTable[(byte)'û'] = 192;
		cTable[(byte)'ý'] = 193;
		cTable[(byte)'þ'] = 194;
		cTable[(byte)'ÿ'] = 195;

		lTable[(byte)'À'] = 13;
		lTable[(byte)'Á'] = 15;
		lTable[(byte)'Â'] = 15;
		lTable[(byte)'Ã'] = 11;
		lTable[(byte)'Ä'] = 15;
		lTable[(byte)'Å'] = 11;
		lTable[(byte)'¨'] = 11;
		lTable[(byte)'Æ'] = 15;
		lTable[(byte)'Ç'] = 10;
		lTable[(byte)'È'] = 13;
		lTable[(byte)'É'] = 13;
		lTable[(byte)'Ê'] = 12;
		lTable[(byte)'Ë'] = 13;
		lTable[(byte)'Ì'] = 14;
		lTable[(byte)'Í'] = 14;
		lTable[(byte)'Î'] = 13;
		lTable[(byte)'Ï'] = 11;
		lTable[(byte)'Ð'] = 12;
		lTable[(byte)'Ñ'] = 12;
		lTable[(byte)'Ò'] = 18;
		lTable[(byte)'Ó'] = 11;
		lTable[(byte)'Ô'] = 13;
		lTable[(byte)'Õ'] = 12;
		lTable[(byte)'Ö'] = 13;
		lTable[(byte)'×'] = 12;
		lTable[(byte)'Ø'] = 17;
		lTable[(byte)'Ù'] = 18;
		lTable[(byte)'Ü'] = 16;
		lTable[(byte)'Ú'] = 18;
		lTable[(byte)'Û'] = 19;
		lTable[(byte)'Ý'] = 11;
		lTable[(byte)'Þ'] = 16;
		lTable[(byte)'ß'] = 14;

		lTable[(byte)'à'] = 14;
		lTable[(byte)'á'] = 15;
		lTable[(byte)'â'] = 10;
		lTable[(byte)'ã'] = 12;
		lTable[(byte)'ä'] = 13;
		lTable[(byte)'å'] = 12;
		lTable[(byte)'¸'] = 12;
		lTable[(byte)'æ'] = 12;
		lTable[(byte)'ç'] = 10;
		lTable[(byte)'è'] = 10;
		lTable[(byte)'é'] = 10;
		lTable[(byte)'ê'] = 11;
		lTable[(byte)'ë'] = 12;
		lTable[(byte)'ì'] = 12;
		lTable[(byte)'í'] = 12;
		lTable[(byte)'î'] = 12;
		lTable[(byte)'ï'] = 10;
		lTable[(byte)'ð'] = 11;
		lTable[(byte)'ñ'] = 10;
		lTable[(byte)'ò'] = 14;
		lTable[(byte)'ó'] =  8;
		lTable[(byte)'ô'] = 11;
		lTable[(byte)'õ'] = 11;
		lTable[(byte)'ö'] = 12;
		lTable[(byte)'÷'] = 10;
		lTable[(byte)'ø'] = 15;
		lTable[(byte)'ù'] = 16;
		lTable[(byte)'ü'] = 14;
		lTable[(byte)'ú'] = 16;
		lTable[(byte)'û'] = 16;
		lTable[(byte)'ý'] =  9;
		lTable[(byte)'þ'] = 15;
		lTable[(byte)'ÿ'] = 14;

	} else if (_vm->getLanguage() == Common::CZ_CZE) {
		// Czech
		cTable[(byte)'Ì'] = 196;
		cTable[(byte)'Š'] = 197;
		cTable[(byte)'È'] = 198;
		cTable[(byte)'Ø'] = 199;
		cTable[(byte)'Ž'] = 200;
		cTable[(byte)'Ý'] = 201;
		cTable[(byte)'Á'] = 202;
		cTable[(byte)'Í'] = 203;
		cTable[(byte)'É'] = 204;
		cTable[(byte)'Ï'] = 205;
		cTable[(byte)''] = 206;
		cTable[(byte)'Ò'] = 207;
		cTable[(byte)'Ó'] = 208;
		cTable[(byte)'Ù'] = 209;
		cTable[(byte)'Ú'] = 210;

		cTable[(byte)'ì'] = 211;
		cTable[(byte)'š'] = 212;
		cTable[(byte)'è'] = 213;
		cTable[(byte)'ø'] = 214;
		cTable[(byte)'ž'] = 215;
		cTable[(byte)'ý'] = 216;
		cTable[(byte)'á'] = 217;
		cTable[(byte)'í'] = 218;
		cTable[(byte)'é'] = 219;
		cTable[(byte)'ï'] = 220;
		cTable[(byte)''] = 221;
		cTable[(byte)'ò'] = 222;
		cTable[(byte)'ó'] = 223;
		cTable[(byte)'ù'] = 224;
		cTable[(byte)'ú'] = 225;

		lTable[(byte)'Ì'] = 11;
		lTable[(byte)'Š'] = 12;
		lTable[(byte)'È'] = 12;
		lTable[(byte)'Ø'] = 14;
		lTable[(byte)'Ž'] = 13;
		lTable[(byte)'Ý'] = 11;
		lTable[(byte)'Á'] = 13;
		lTable[(byte)'Í'] = 11;
		lTable[(byte)'É'] = 11;
		lTable[(byte)'Ï'] = 15;
		lTable[(byte)''] = 19;
		lTable[(byte)'Ò'] = 10;
		lTable[(byte)'Ó'] = 13;
		lTable[(byte)'Ù'] = 13;
		lTable[(byte)'Ú'] = 13;

		lTable[(byte)'ì'] = 12;
		lTable[(byte)'š'] = 12;
		lTable[(byte)'è'] = 12;
		lTable[(byte)'ø'] = 12;
		lTable[(byte)'ž'] = 13;
		lTable[(byte)'ý'] = 11;
		lTable[(byte)'á'] = 15;
		lTable[(byte)'í'] = 7;
		lTable[(byte)'é'] = 12;
		lTable[(byte)'ï'] = 17;
		lTable[(byte)''] = 16;
		lTable[(byte)'ò'] = 11;
		lTable[(byte)'ó'] = 11;
		lTable[(byte)'ù'] = 13;
		lTable[(byte)'ú'] = 13;

	} else if (_vm->getLanguage() == Common::FR_FRA) {
		// French

		cTable[(byte)'â'] = 226;
		cTable[(byte)'é'] = 227;
		cTable[(byte)'ê'] = 228;
		cTable[(byte)'î'] = 229;
		cTable[(byte)'ù'] = 230;
		cTable[(byte)'û'] = 231;
		cTable[(byte)'ô'] = 232;
		cTable[(byte)'ö'] = 233;

		lTable[(byte)'â'] = 15;
		lTable[(byte)'é'] = 12;
		lTable[(byte)'ê'] = 12;
		lTable[(byte)'î'] =  9;
		lTable[(byte)'ù'] = 13;
		lTable[(byte)'û'] = 13;
		lTable[(byte)'ô'] = 11;
		lTable[(byte)'ö'] = 11;

	} else if (_vm->getLanguage() == Common::DE_DEU) {
		cTable[(byte)'ß'] = 234;
		// 'SS' = 235
		cTable[(byte)'Ä'] = 236;
		cTable[(byte)'Ö'] = 237;
		cTable[(byte)'Ü'] = 238;

		lTable[(byte)'ß'] = 15;
	}
}


/***************************************************************************\
*       RMFontMacc Methods
\****************************************************************************/

void RMFontMacc::init(void) {
	int i;

	// bernie: Number of characters in the font
	int nchars =
	    102    // base
	    + 18    // polish
	    + 66    // russian
	    + 30    // czech
	    +  8    // francais
	    +  5;   // deutsch


	load(RES_F_MACC, nchars, 11, 16);

	// Default
	lDefault = 10;
	hDefault = 17;
	Common::fill(&l2Table[0][0], &l2Table[0][0] + (256 * 256), '\0');

	for (i = 0; i < 256; i++) {
		cTable[i] = -1;
		lTable[i] = lDefault;
	}

	for (i = 0; i < 26; i++)
		cTable['A' + i] = i + 0;

	for (i = 0; i < 26; i++)
		cTable['a' + i] = i + 26;

	for (i = 0; i < 10; i++)
		cTable['0' + i] = i + 52;

	cTable['!'] = 62;
	//cTable['!'] = 63;         // ! rovescia
	cTable['\"'] = 64;
	cTable['$'] = 65;
	cTable['%'] = 66;
	cTable['&'] = 67;
	cTable['/'] = 68;
	cTable['('] = 69;
	cTable[')'] = 70;
	cTable['='] = 71;
	cTable['?'] = 72;
	//cTable['?'] = 73;        // ? rovescia
	cTable['*'] = 74;
	cTable['+'] = 75;
	cTable[(byte)'ñ'] = 76;
	cTable[';'] = 77;
	cTable[','] = 78;
	cTable['.'] = 79;
	cTable[':'] = 80;
	cTable['-'] = 81;
	cTable['<'] = 82;
	cTable['>'] = 83;
	cTable['/'] = 84;
	cTable[(byte)'ÿ'] = 85;
	cTable[(byte)'à'] = 86;
	cTable[(byte)'ä'] = 87;
	cTable[(byte)'å'] = 88;
	cTable[(byte)'è'] = 89;
	cTable[(byte)'ë'] = 90;
	//cTable[(byte)''] = 91;          // e with ball
	cTable[(byte)'ì'] = 92;
	cTable[(byte)'ï'] = 93;
	//cTable[(byte)''] = 94;            // i with ball
	cTable[(byte)'ò'] = 95;
	cTable[(byte)'ö'] = 96;
	//cTable[(byte)''] = 97;          // o with ball
	cTable[(byte)'ù'] = 98;
	cTable[(byte)'ü'] = 99;
	//cTable[(byte)''] = 100;         // u with ball
	cTable[(byte)'ç'] = 101;

	if (_vm->getLanguage() == Common::PL_POL) {
		// Polish characters
		//AaCcEeLlNnOoSsZzZz
		//¥¹ÆæÊê£³ÑñÓóŒœ¯¿Ÿ

		cTable[(byte)'¥'] = 102;
		cTable[(byte)'¹'] = 103;
		cTable[(byte)'Æ'] = 104;
		cTable[(byte)'æ'] = 105;
		cTable[(byte)'Ê'] = 106;
		cTable[(byte)'ê'] = 107;
		cTable[(byte)'£'] = 108;
		cTable[(byte)'³'] = 109;
		cTable[(byte)'Ñ'] = 110;
		cTable[(byte)'ñ'] = 111;
		cTable[(byte)'Ó'] = 112;
		cTable[(byte)'ó'] = 113;
		cTable[(byte)'Œ'] = 114;
		cTable[(byte)'œ'] = 115;
		cTable[(byte)'¯'] = 116;
		cTable[(byte)'¿'] = 117;
		cTable[(byte)''] = 118;
		cTable[(byte)'Ÿ'] = 119;

		lTable[(byte)'¥'] = 14;
		lTable[(byte)'¹'] = 16;
		lTable[(byte)'Æ'] = 12;
		lTable[(byte)'æ'] = 12;
		lTable[(byte)'Ê'] = 11;
		lTable[(byte)'ê'] = 12;
		lTable[(byte)'£'] = 14;
		lTable[(byte)'³'] = 9;
		lTable[(byte)'Ñ'] = 10;
		lTable[(byte)'ñ'] = 11;
		lTable[(byte)'Ó'] = 13;
		lTable[(byte)'ó'] = 11;
		lTable[(byte)'Œ'] = 12;
		lTable[(byte)'œ'] = 12;
		lTable[(byte)'¯'] = 13;
		lTable[(byte)'¿'] = 13;
		lTable[(byte)''] = 14;
		lTable[(byte)'Ÿ'] = 13;

	} else if (_vm->getLanguage() == Common::RU_RUS) {
		// Russian Characters
		// WARNING: The Russian font uses many of the ISO-Latin-1 font,
		// allowing for further translations. To support Tonyin other langauges,
		// these mappings could be used as a basis
		cTable[(byte)'À'] = 120;
		cTable[(byte)'Á'] = 121;
		cTable[(byte)'Â'] = 122;
		cTable[(byte)'Ã'] = 123;
		cTable[(byte)'Ä'] = 124;
		cTable[(byte)'Å'] = 125;
		cTable[(byte)'¨'] = 126;
		cTable[(byte)'Æ'] = 127;
		cTable[(byte)'Ç'] = 128;
		cTable[(byte)'È'] = 129;
		cTable[(byte)'É'] = 130;
		cTable[(byte)'Ê'] = 131;
		cTable[(byte)'Ë'] = 132;
		cTable[(byte)'Ì'] = 133;
		cTable[(byte)'Í'] = 134;
		cTable[(byte)'Î'] = 135;
		cTable[(byte)'Ï'] = 136;
		cTable[(byte)'Ð'] = 137;
		cTable[(byte)'Ñ'] = 138;
		cTable[(byte)'Ò'] = 139;
		cTable[(byte)'Ó'] = 140;
		cTable[(byte)'Ô'] = 141;
		cTable[(byte)'Õ'] = 142;
		cTable[(byte)'Ö'] = 143;
		cTable[(byte)'×'] = 144;
		cTable[(byte)'Ø'] = 145;
		cTable[(byte)'Ù'] = 146;
		cTable[(byte)'Ü'] = 147;
		cTable[(byte)'Ú'] = 148;
		cTable[(byte)'Û'] = 149;
		cTable[(byte)'Ý'] = 150;
		cTable[(byte)'Þ'] = 151;
		cTable[(byte)'ß'] = 152;

		cTable[(byte)'à'] = 153;
		cTable[(byte)'á'] = 154;
		cTable[(byte)'â'] = 155;
		cTable[(byte)'ã'] = 156;
		cTable[(byte)'ä'] = 157;
		cTable[(byte)'å'] = 158;
		cTable[(byte)'¸'] = 159;
		cTable[(byte)'æ'] = 160;
		cTable[(byte)'ç'] = 161;
		cTable[(byte)'è'] = 162;
		cTable[(byte)'é'] = 163;
		cTable[(byte)'ê'] = 164;
		cTable[(byte)'ë'] = 165;
		cTable[(byte)'ì'] = 166;
		cTable[(byte)'í'] = 167;
		cTable[(byte)'î'] = 168;
		cTable[(byte)'ï'] = 169;
		cTable[(byte)'ð'] = 170;
		cTable[(byte)'ñ'] = 171;
		cTable[(byte)'ò'] = 172;
		cTable[(byte)'ó'] = 173;
		cTable[(byte)'ô'] = 174;
		cTable[(byte)'õ'] = 175;
		cTable[(byte)'ö'] = 176;
		cTable[(byte)'÷'] = 177;
		cTable[(byte)'ø'] = 178;
		cTable[(byte)'ù'] = 179;
		cTable[(byte)'ü'] = 180;
		cTable[(byte)'ú'] = 181;
		cTable[(byte)'û'] = 182;
		cTable[(byte)'ý'] = 183;
		cTable[(byte)'þ'] = 184;
		cTable[(byte)'ÿ'] = 185;

		lTable[(byte)'À'] = 11;
		lTable[(byte)'Á'] = 11;
		lTable[(byte)'Â'] = 11;
		lTable[(byte)'Ã'] = 9;
		lTable[(byte)'Ä'] = 10;
		lTable[(byte)'Å'] = 10;
		lTable[(byte)'¨'] = 10;
		lTable[(byte)'Æ'] = 11;
		lTable[(byte)'Ç'] = 10;
		lTable[(byte)'È'] = 10;
		lTable[(byte)'É'] = 10;
		lTable[(byte)'Ê'] = 11;
		lTable[(byte)'Ë'] = 9;
		lTable[(byte)'Ì'] = 11;
		lTable[(byte)'Í'] = 10;
		lTable[(byte)'Î'] = 11;
		lTable[(byte)'Ï'] = 8;
		lTable[(byte)'Ð'] = 10;
		lTable[(byte)'Ñ'] = 10;
		lTable[(byte)'Ò'] = 11;
		lTable[(byte)'Ó'] = 11;
		lTable[(byte)'Ô'] = 11;
		lTable[(byte)'Õ'] = 11;
		lTable[(byte)'Ö'] = 10;
		lTable[(byte)'×'] = 10;
		lTable[(byte)'Ø'] = 10;
		lTable[(byte)'Ù'] = 10;
		lTable[(byte)'Ü'] = 11;
		lTable[(byte)'Ú'] = 11;
		lTable[(byte)'Û'] = 11;
		lTable[(byte)'Ý'] = 11;
		lTable[(byte)'Þ'] = 11;
		lTable[(byte)'ß'] = 11;

		lTable[(byte)'à'] = 10;
		lTable[(byte)'á'] = 10;
		lTable[(byte)'â'] = 11;
		lTable[(byte)'ã'] = 10;
		lTable[(byte)'ä'] = 9;
		lTable[(byte)'å'] = 10;
		lTable[(byte)'¸'] = 9;
		lTable[(byte)'æ'] = 10;
		lTable[(byte)'ç'] = 9;
		lTable[(byte)'è'] = 11;
		lTable[(byte)'é'] = 11;
		lTable[(byte)'ê'] = 11;
		lTable[(byte)'ë'] = 11;
		lTable[(byte)'ì'] = 11;
		lTable[(byte)'í'] = 11;
		lTable[(byte)'î'] = 10;
		lTable[(byte)'ï'] = 9;
		lTable[(byte)'ð'] = 11;
		lTable[(byte)'ñ'] = 10;
		lTable[(byte)'ò'] = 9;
		lTable[(byte)'ó'] = 11;
		lTable[(byte)'ô'] = 10;
		lTable[(byte)'õ'] = 11;
		lTable[(byte)'ö'] = 10;
		lTable[(byte)'÷'] = 10;
		lTable[(byte)'ø'] = 11;
		lTable[(byte)'ù'] = 11;
		lTable[(byte)'ü'] = 10;
		lTable[(byte)'ú'] = 10;
		lTable[(byte)'û'] = 10;
		lTable[(byte)'ý'] =  9;
		lTable[(byte)'þ'] = 11;
		lTable[(byte)'ÿ'] = 11;

	} else if (_vm->getLanguage() == Common::CZ_CZE) {
		// Czech

		cTable[(byte)'Ì'] = 186;
		cTable[(byte)'Š'] = 187;
		cTable[(byte)'È'] = 188;
		cTable[(byte)'Ø'] = 189;
		cTable[(byte)'Ž'] = 190;
		cTable[(byte)'Ý'] = 191;
		cTable[(byte)'Á'] = 192;
		cTable[(byte)'Í'] = 193;
		cTable[(byte)'É'] = 194;
		cTable[(byte)'Ï'] = 195;
		cTable[(byte)''] = 196;
		cTable[(byte)'Ò'] = 197;
		cTable[(byte)'Ó'] = 198;
		cTable[(byte)'Ù'] = 199;
		cTable[(byte)'Ú'] = 200;

		cTable[(byte)'ì'] = 201;
		cTable[(byte)'š'] = 202;
		cTable[(byte)'è'] = 203;
		cTable[(byte)'ø'] = 204;
		cTable[(byte)'ž'] = 205;
		cTable[(byte)'ý'] = 206;
		cTable[(byte)'á'] = 207;
		cTable[(byte)'í'] = 208;
		cTable[(byte)'é'] = 209;
		cTable[(byte)'ï'] = 210;
		cTable[(byte)''] = 211;
		cTable[(byte)'ò'] = 212;
		cTable[(byte)'ó'] = 213;
		cTable[(byte)'ù'] = 214;
		cTable[(byte)'ú'] = 215;

		lTable[(byte)'Ì'] = 10;
		lTable[(byte)'Š'] = 10;
		lTable[(byte)'È'] = 10;
		lTable[(byte)'Ø'] = 11;
		lTable[(byte)'Ž'] = 9;
		lTable[(byte)'Ý'] = 11;
		lTable[(byte)'Á'] = 11;
		lTable[(byte)'Í'] = 9;
		lTable[(byte)'É'] = 10;
		lTable[(byte)'Ï'] = 11;
		lTable[(byte)''] = 11;
		lTable[(byte)'Ò'] = 11;
		lTable[(byte)'Ó'] = 11;
		lTable[(byte)'Ù'] = 11;
		lTable[(byte)'Ú'] = 11;

		lTable[(byte)'ì'] = 10;
		lTable[(byte)'š'] = 9;
		lTable[(byte)'è'] = 10;
		lTable[(byte)'ø'] = 10;
		lTable[(byte)'ž'] = 9;
		lTable[(byte)'ý'] = 11;
		lTable[(byte)'á'] = 10;
		lTable[(byte)'í'] = 9;
		lTable[(byte)'é'] = 10;
		lTable[(byte)'ï'] = 11;
		lTable[(byte)''] = 11;
		lTable[(byte)'ò'] = 11;
		lTable[(byte)'ó'] = 10;
		lTable[(byte)'ù'] = 11;
		lTable[(byte)'ú'] = 11;

	} else if (_vm->getLanguage() == Common::FR_FRA) {
		// French

		cTable[(byte)'â'] = 226;
		cTable[(byte)'é'] = 227;
		cTable[(byte)'è'] = 228;
		cTable[(byte)'î'] = 229;
		cTable[(byte)'ù'] = 230;
		cTable[(byte)'û'] = 231;
		cTable[(byte)'ô'] = 232;
		cTable[(byte)'ö'] = 233;

		lTable[(byte)'â'] = 10;
		lTable[(byte)'é'] = 10;
		lTable[(byte)'ê'] = 10;
		lTable[(byte)'î'] = 8;
		lTable[(byte)'ù'] = 11;
		lTable[(byte)'û'] = 11;
		lTable[(byte)'ô'] = 10;
		lTable[(byte)'ö'] = 10;

	} else if (_vm->getLanguage() == Common::DE_DEU) {
		// German

		cTable[(byte)'ß'] = 234;
		// 'SS' = 235
		cTable[(byte)'Ä'] = 236;
		cTable[(byte)'Ö'] = 237;
		cTable[(byte)'Ü'] = 238;

		lTable[(byte)'ß'] = 11;
	}
}

/***************************************************************************\
*       RMFontCredits Methods
\****************************************************************************/

void RMFontCredits::init(void) {
	int i;

	// bernie: Number of characters in the font
	int nchars =
	    112    // base
	    + 18    // polish
	    + 66    // russian
	    + 30    // czech
	    +  8    // french
	    +  2;   // deutsch


	load(RES_F_CREDITS, nchars, 27, 28, RES_F_CPAL);

	// Default
	lDefault = 10;
	hDefault = 28;
	Common::fill(&l2Table[0][0], &l2Table[0][0] + (256 * 256), '\0');

	for (i = 0; i < 256; i++) {
		cTable[i] = -1;
		lTable[i] = lDefault;
	}

	for (i = 0; i < 26; i++)
		cTable['A' + i] = i + 0;

	for (i = 0; i < 26; i++)
		cTable['a' + i] = i + 26;



	cTable[(byte)'à'] = 52;
	cTable[(byte)'á'] = 53;
//	cTable[''] = 54; // a ^
//	cTable[''] = 55; // a pallini
	cTable[(byte)'è'] = 56;
	cTable[(byte)'é'] = 57;
//	cTable[''] = 58; // e ^
//	cTable[''] = 59; // e pallini
	cTable[(byte)'ì'] = 60;
	cTable[(byte)'í'] = 61;
//	cTable[''] = 62; // i ^
//	cTable[''] = 63; // i pallini
	cTable[(byte)'ò'] = 64;
	cTable[(byte)'ó'] = 65;
//	cTable[''] = 66; // o ^
//	cTable[''] = 67; // o pallini
	cTable[(byte)'ù'] = 68;
	cTable[(byte)'ú'] = 69;
//	cTable[''] = 70; // u ^
//	cTable[''] = 71; // u pallini
//	cTable[''] = 72; // y pallini
	cTable[(byte)'ñ'] = 73;
	cTable[(byte)'ç'] = 74;
//	cTable[''] = 75; // o barrato
//	cTable[''] = 76; // ac
	cTable[(byte)'©'] = 77;
//	cTable[''] = 78; // ? rovesciato
	cTable['?'] = 79;
//	cTable[''] = 80; // ! rovesciato
	cTable['!'] = 81;
//	cTable[''] = 82; // 1/2
//	cTable[''] = 83; // 1/4
	cTable['('] = 84;
	cTable[')'] = 85;
	cTable[(byte)'«'] = 86;
	cTable[(byte)'»'] = 87;
//	cTable[''] = 88; // AE
	cTable[':'] = 89;
	cTable['%'] = 90;
	cTable['&'] = 91;
	cTable['/'] = 92;
	cTable['+'] = 93;
	cTable[';'] = 94;
	cTable[','] = 95;
	cTable['^'] = 96;
	cTable['='] = 97;
	cTable['_'] = 98;
	cTable['*'] = 99;
	cTable['.'] = 100;

	for (i = 0; i < 10; i++)
		cTable['0' + i] = i + 101;
	cTable['\''] = 111;

	lTable[' '] = 11;
	lTable[(byte)'Ä'] = lTable['A'] = 19;
	lTable['B'] = 15;
	lTable['C'] = 14;
	lTable['D'] = 13;
	lTable['E'] = 14;
	lTable['F'] = 13;
	lTable['G'] = 16;
	lTable['H'] = 15;
	lTable['I'] = 5;
	lTable['J'] = 8;
	lTable['K'] = 15;
	lTable['L'] = 13;
	lTable['M'] = 17;
	lTable['N'] = 15;
	lTable['Ö'] = lTable['O'] = 14;
	lTable['P'] = 12;
	lTable['Q'] = 14;
	lTable['R'] = 14;
	lTable['S'] = 15;
	lTable['T'] = 11;
	lTable['Ü'] = lTable['U'] = 12;
	lTable['V'] = 12;
	lTable['W'] = 16;
	lTable['X'] = 12;
	lTable['Y'] = 13;
	lTable['Z'] = 14;

	lTable['a'] = 11;
	lTable['b'] = 9;
	lTable['c'] = 9;
	lTable['d'] = 10;
	lTable['e'] = 9;
	lTable['f'] = 8;
	lTable['g'] = 9;
	lTable['h'] = 10;
	lTable['i'] = 5;
	lTable['j'] = 6;
	lTable['k'] = 12;
	lTable['l'] = 6;
	lTable['m'] = 14;
	lTable['n'] = 10;
	lTable['o'] = 11;
	lTable['p'] = 11;
	lTable['q'] = 9;
	lTable['r'] = 9;
	lTable['s'] = 9;
	lTable['t'] = 6;
	lTable['u'] = 9;
	lTable['v'] = 10;
	lTable['w'] = 14;
	lTable['x'] = 9;
	lTable['y'] = 10;
	lTable['z'] = 9;

	lTable['0'] = 12;
	lTable['1'] = 8;
	lTable['2'] = 10;
	lTable['3'] = 11;
	lTable['4'] = 12;
	lTable['5'] = 11;
	lTable['6'] = 12;
	lTable['7'] = 10;
	lTable['8'] = 11;
	lTable['9'] = 10;

	lTable['/'] = 10;
	lTable['^'] = 9;
	lTable[','] = 5;
	lTable['.'] = 5;
	lTable[';'] = 5;
	lTable[':'] = 5;
	lTable['\''] = 5;

	if (_vm->getLanguage() == Common::PL_POL) {
		// Polish characters
		//AaCcEeLlNnOoSsZzZz
		//¥¹ÆæÊê£³ÑñÓóŒœ¯¿Ÿ

		cTable[(byte)'¥'] = 112;
		cTable[(byte)'¹'] = 113;
		cTable[(byte)'Æ'] = 114;
		cTable[(byte)'æ'] = 115;
		cTable[(byte)'Ê'] = 116;
		cTable[(byte)'ê'] = 117;
		cTable[(byte)'£'] = 118;
		cTable[(byte)'³'] = 119;
		cTable[(byte)'Ñ'] = 120;
		cTable[(byte)'ñ'] = 121;
		cTable[(byte)'Ó'] = 122;
		cTable[(byte)'ó'] = 123;
		cTable[(byte)'Œ'] = 124;
		cTable[(byte)'œ'] = 125;
		cTable[(byte)'¯'] = 126;
		cTable[(byte)'¿'] = 127;
		cTable[(byte)''] = 128;
		cTable[(byte)'Ÿ'] = 129;

		lTable[(byte)'¥'] = 20;
		lTable[(byte)'¹'] = 12;
		lTable[(byte)'Æ'] = 15;
		lTable[(byte)'æ'] = 10;
		lTable[(byte)'Ê'] = 15;
		lTable[(byte)'ê'] = 10;
		lTable[(byte)'£'] = 14;
		lTable[(byte)'³'] = 11;
		lTable[(byte)'Ñ'] = 16;
		lTable[(byte)'ñ'] = 10;
		lTable[(byte)'Ó'] = 15;
		lTable[(byte)'ó'] = 11;
		lTable[(byte)'Œ'] = 15;
		lTable[(byte)'œ'] = 10;
		lTable[(byte)'¯'] = 15;
		lTable[(byte)'¿'] = 10;
		lTable[(byte)''] = 15;
		lTable[(byte)'Ÿ'] = 10;

	} else if (_vm->getLanguage() == Common::RU_RUS) {
		// Russian Characters
		// WARNING: The Russian font uses many of the ISO-Latin-1 font,
		// allowing for further translations. To support Tonyin other langauges,
		// these mappings could be used as a basis
		cTable[(byte)'À'] = 130;
		cTable[(byte)'Á'] = 131;
		cTable[(byte)'Â'] = 132;
		cTable[(byte)'Ã'] = 133;
		cTable[(byte)'Ä'] = 134;
		cTable[(byte)'Å'] = 135;
		cTable[(byte)'¨'] = 136;
		cTable[(byte)'Æ'] = 137;
		cTable[(byte)'Ç'] = 138;
		cTable[(byte)'È'] = 139;
		cTable[(byte)'É'] = 140;
		cTable[(byte)'Ê'] = 141;
		cTable[(byte)'Ë'] = 142;
		cTable[(byte)'Ì'] = 143;
		cTable[(byte)'Í'] = 144;
		cTable[(byte)'Î'] = 145;
		cTable[(byte)'Ï'] = 146;
		cTable[(byte)'Ð'] = 147;
		cTable[(byte)'Ñ'] = 148;
		cTable[(byte)'Ò'] = 149;
		cTable[(byte)'Ó'] = 150;
		cTable[(byte)'Ô'] = 151;
		cTable[(byte)'Õ'] = 152;
		cTable[(byte)'Ö'] = 153;
		cTable[(byte)'×'] = 154;
		cTable[(byte)'Ø'] = 155;
		cTable[(byte)'Ù'] = 156;
		cTable[(byte)'Ü'] = 157;
		cTable[(byte)'Ú'] = 158;
		cTable[(byte)'Û'] = 159;
		cTable[(byte)'Ý'] = 160;
		cTable[(byte)'Þ'] = 161;
		cTable[(byte)'ß'] = 162;

		cTable[(byte)'à'] = 163;
		cTable[(byte)'á'] = 164;
		cTable[(byte)'â'] = 165;
		cTable[(byte)'ã'] = 166;
		cTable[(byte)'ä'] = 167;
		cTable[(byte)'å'] = 168;
		cTable[(byte)'¸'] = 169;
		cTable[(byte)'æ'] = 170;
		cTable[(byte)'ç'] = 171;
		cTable[(byte)'è'] = 172;
		cTable[(byte)'é'] = 173;
		cTable[(byte)'ê'] = 174;
		cTable[(byte)'ë'] = 175;
		cTable[(byte)'ì'] = 176;
		cTable[(byte)'í'] = 177;
		cTable[(byte)'î'] = 178;
		cTable[(byte)'ï'] = 179;
		cTable[(byte)'ð'] = 180;
		cTable[(byte)'ñ'] = 181;
		cTable[(byte)'ò'] = 182;
		cTable[(byte)'ó'] = 183;
		cTable[(byte)'ô'] = 184;
		cTable[(byte)'õ'] = 185;
		cTable[(byte)'ö'] = 186;
		cTable[(byte)'÷'] = 187;
		cTable[(byte)'ø'] = 188;
		cTable[(byte)'ù'] = 189;
		cTable[(byte)'ü'] = 190;
		cTable[(byte)'ú'] = 191;
		cTable[(byte)'û'] = 192;
		cTable[(byte)'ý'] = 193;
		cTable[(byte)'þ'] = 194;
		cTable[(byte)'ÿ'] = 195;

		lTable[(byte)'À'] = 20;
		lTable[(byte)'Á'] = 16;
		lTable[(byte)'Â'] = 16;
		lTable[(byte)'Ã'] = 14;
		lTable[(byte)'Ä'] = 22;
		lTable[(byte)'Å'] = 15;
		lTable[(byte)'¨'] = 15;
		lTable[(byte)'Æ'] = 20;
		lTable[(byte)'Ç'] = 12;
		lTable[(byte)'È'] = 16;
		lTable[(byte)'É'] = 16;
		lTable[(byte)'Ê'] = 16;
		lTable[(byte)'Ë'] = 22;
		lTable[(byte)'Ì'] = 18;
		lTable[(byte)'Í'] = 16;
		lTable[(byte)'Î'] = 15;
		lTable[(byte)'Ï'] = 14;
		lTable[(byte)'Ð'] = 13;
		lTable[(byte)'Ñ'] = 15;
		lTable[(byte)'Ò'] = 12;
		lTable[(byte)'Ó'] = 14;
		lTable[(byte)'Ô'] = 15;
		lTable[(byte)'Õ'] = 13;
		lTable[(byte)'Ö'] = 16;
		lTable[(byte)'×'] = 14;
		lTable[(byte)'Ø'] = 23;
		lTable[(byte)'Ù'] = 23;
		lTable[(byte)'Ü'] = 10;
		lTable[(byte)'Ú'] = 12;
		lTable[(byte)'Û'] = 16;
		lTable[(byte)'Ý'] = 12;
		lTable[(byte)'Þ'] = 20;
		lTable[(byte)'ß'] = 15;

		lTable[(byte)'à'] = 12;
		lTable[(byte)'á'] = 10;
		lTable[(byte)'â'] = 10;
		lTable[(byte)'ã'] = 11;
		lTable[(byte)'ä'] = 16;
		lTable[(byte)'å'] = 10;
		lTable[(byte)'¸'] = 11;
		lTable[(byte)'æ'] = 13;
		lTable[(byte)'ç'] = 12;
		lTable[(byte)'è'] = 13;
		lTable[(byte)'é'] = 13;
		lTable[(byte)'ê'] = 12;
		lTable[(byte)'ë'] = 13;
		lTable[(byte)'ì'] = 14;
		lTable[(byte)'í'] = 11;
		lTable[(byte)'î'] = 11;
		lTable[(byte)'ï'] = 11;
		lTable[(byte)'ð'] = 12;
		lTable[(byte)'ñ'] = 10;
		lTable[(byte)'ò'] = 10;
		lTable[(byte)'ó'] = 10;
		lTable[(byte)'ô'] = 11;
		lTable[(byte)'õ'] = 10;
		lTable[(byte)'ö'] = 11;
		lTable[(byte)'÷'] = 10;
		lTable[(byte)'ø'] = 15;
		lTable[(byte)'ù'] = 15;
		lTable[(byte)'ü'] = 10;
		lTable[(byte)'ú'] = 12;
		lTable[(byte)'û'] = 16;
		lTable[(byte)'ý'] = 11;
		lTable[(byte)'þ'] = 13;
		lTable[(byte)'ÿ'] = 11;

	} else if (_vm->getLanguage() == Common::CZ_CZE) {
		// CZECH Language

		cTable[(byte)'Ì'] = 196;
		cTable[(byte)'Š'] = 197;
		cTable[(byte)'È'] = 198;
		cTable[(byte)'Ø'] = 199;
		cTable[(byte)'Ž'] = 200;
		cTable[(byte)'Ý'] = 201;
		cTable[(byte)'Á'] = 202;
		cTable[(byte)'Í'] = 203;
		cTable[(byte)'É'] = 204;
		cTable[(byte)'Ï'] = 205;
		cTable[(byte)''] = 206;
		cTable[(byte)'Ò'] = 207;
		cTable[(byte)'Ó'] = 208;
		cTable[(byte)'Ù'] = 209;
		cTable[(byte)'Ú'] = 210;

		cTable[(byte)'ì'] = 211;
		cTable[(byte)'š'] = 212;
		cTable[(byte)'è'] = 213;
		cTable[(byte)'ø'] = 214;
		cTable[(byte)'ž'] = 215;
		cTable[(byte)'ý'] = 216;
		cTable[(byte)'á'] = 217;
		cTable[(byte)'í'] = 218;
		cTable[(byte)'é'] = 219;
		cTable[(byte)'ï'] = 220;
		cTable[(byte)''] = 221;
		cTable[(byte)'ò'] = 222;
		cTable[(byte)'ó'] = 223;
		cTable[(byte)'ù'] = 224;
		cTable[(byte)'ú'] = 225;

		lTable[(byte)'Ì'] = 15;
		lTable[(byte)'Š'] = 15;
		lTable[(byte)'È'] = 15;
		lTable[(byte)'Ø'] = 15;
		lTable[(byte)'Ž'] = 15;
		lTable[(byte)'Ý'] = 14;
		lTable[(byte)'Á'] = 20;
		lTable[(byte)'Í'] = 7;
		lTable[(byte)'É'] = 15;
		lTable[(byte)'Ï'] = 20;
		lTable[(byte)''] = 19;
		lTable[(byte)'Ò'] = 16;
		lTable[(byte)'Ó'] = 15;
		lTable[(byte)'Ù'] = 13;
		lTable[(byte)'Ú'] = 13;

		lTable[(byte)'ì'] = 11;
		lTable[(byte)'š'] = 11;
		lTable[(byte)'è'] = 11;
		lTable[(byte)'ø'] = 11;
		lTable[(byte)'ž'] = 11;
		lTable[(byte)'ý'] = 10;
		lTable[(byte)'á'] = 12;
		lTable[(byte)'í'] = 6;
		lTable[(byte)'é'] = 10;
		lTable[(byte)'ï'] = 15;
		lTable[(byte)''] = 12;
		lTable[(byte)'ò'] = 11;
		lTable[(byte)'ó'] = 11;
		lTable[(byte)'ù'] = 10;
		lTable[(byte)'ú'] = 10;

	} else if (_vm->getLanguage() == Common::FR_FRA) {
		// French

		cTable[(byte)'â'] = 226;
		cTable[(byte)'é'] = 227;
		cTable[(byte)'ê'] = 228;
		cTable[(byte)'î'] = 229;
		cTable[(byte)'ù'] = 230;
		cTable[(byte)'û'] = 231;
		cTable[(byte)'ô'] = 232;
		cTable[(byte)'ö'] = 233;

		lTable[(byte)'â'] = 12;
		lTable[(byte)'é'] = 10;
		lTable[(byte)'ê'] = 10;
		lTable[(byte)'î'] = 6;
		lTable[(byte)'ù'] = 10;
		lTable[(byte)'û'] = 10;
		lTable[(byte)'ô'] = 11;
		lTable[(byte)'ö'] = 11;

	} else if (_vm->getLanguage() == Common::DE_DEU) {
		// German

		cTable[(byte)'ß'] = 234;
		// 'SS' = 235

		// old chars overrides
		cTable[(byte)'Ä'] = cTable[(byte)'ä'] = 55;
		cTable[(byte)'Ö'] = cTable[(byte)'ö'] = 67;
		cTable[(byte)'Ü'] = cTable[(byte)'ü'] = 71;

		lTable[(byte)'ß'] = 11;
	}
}



/***************************************************************************\
*       Metodi di RMFontObj
\****************************************************************************/

#define TOUPPER(a)  ((a) >= 'a' && (a) <= 'z' ? (a) + 'A' - 'a' : (a))
#define TOLOWER(a)  ((a) >= 'A' && (a) <= 'Z' ? (a) + 'a' - 'A' : (a))

void RMFontObj::setBothCase(int nChar, int nNext, signed char spiazz) {
	l2Table[TOUPPER(nChar)][TOUPPER(nNext)] = spiazz;
	l2Table[TOUPPER(nChar)][TOLOWER(nNext)] = spiazz;
	l2Table[TOLOWER(nChar)][TOUPPER(nNext)] = spiazz;
	l2Table[TOLOWER(nChar)][TOLOWER(nNext)] = spiazz;
}


void RMFontObj::init(void) {
	int i;

	//bernie: Number of characters in the font (solo maiuscolo)
	int nchars =
	    85    // base
	    +  9    // polish
	    + 33    // russian
	    + 15    // czech
	    +  0    // francais (no uppercase chars)
	    +  1;   // deutsch


	load(RES_F_OBJ, nchars, 25, 30);

	// Initialise the f**king table
	lDefault = 26;
	hDefault = 30;
	Common::fill(&l2Table[0][0], &l2Table[0][0] + (256 * 256), '\0');

	for (i = 0; i < 256; i++) {
		cTable[i] = -1;
		lTable[i] = lDefault;
	}

	for (i = 0; i < 26; i++) {
		cTable['A' + i] = i + 0;
		cTable['a' + i] = i + 0;
	}

	for (i = 0; i < 10; i++)
		cTable['0' + i] = i + 26;

	cTable[','] = 36;
	cTable[';'] = 37;
	cTable['.'] = 38;
	cTable[':'] = 39;
	cTable['-'] = 40;
	cTable['+'] = 41;
	cTable['!'] = 42;
	// cTable['!'] = 43; Exclamation countdown
	cTable['?'] = 44;
	//cTable['?'] = 45;  Interrogativo alla rovescia
	cTable['/'] = 46;
	cTable['('] = 47;
	cTable[')'] = 48;
	cTable['='] = 49;
	cTable['\''] = 50;
	cTable['\"'] = 51;
	cTable[(byte)'£'] = 52;
	cTable[(byte)'$'] = 53;
	cTable[(byte)'%'] = 54;
	cTable[(byte)'&'] = 55;
	cTable[(byte)'^'] = 56;
	cTable[(byte)'*'] = 57;
	cTable[(byte)'<'] = 58;
	cTable[(byte)'>'] = 59;
	cTable[(byte)'«'] = 60;
	cTable[(byte)'»'] = 61;
	cTable[(byte)'ø'] = 62;
	cTable[(byte)'ç'] = 63;
	//cTable[(byte)'ƒ'] = 64;   integral
	cTable[(byte)'Ñ'] = 65;
	cTable[(byte)'®'] = 66;
	cTable[(byte)'©'] = 67;
	cTable[(byte)'Æ'] = 68;
	cTable[(byte)'Á'] = 69;
	cTable[(byte)'Ä'] = cTable[(byte)'ä'] = 70;
	cTable[(byte)'Å'] = 71;
	cTable[(byte)'È'] = 72;
	cTable[(byte)'Ë'] = 73;
	//cTable[(byte)' '] = 74;   e circlet
	cTable[(byte)'Ì'] = 75;
	cTable[(byte)'Ï'] = 76;
	//cTable[(byte)' '] = 77;     i circlet
	cTable[(byte)'Ò'] = 78;
	cTable[(byte)'Ö'] = cTable[(byte)'ö'] = 79;
	//cTable[(byte)' '] = 80;       o circlet
	cTable[(byte)'Ù'] = 81;
	cTable[(byte)'Ü'] = cTable[(byte)'ü'] = 82;
	//cTable[' '] = 83;     u circlet
	//cTable[' '] = 84;   y dieresi

	/* Little lengths */
	lTable[' '] = 11;
	lTable['.'] = 8;
	lTable['-'] = 12;
	lTable['\''] = 8;
	lTable['0'] = 20;
	lTable['1'] = 20;
	lTable['2'] = 15;
	lTable['3'] = 20;
	lTable['4'] = 20;
	lTable['5'] = 20;
	lTable['6'] = 20;
	lTable['7'] = 20;
	lTable['8'] = 20;
	lTable['9'] = 20;


	lTable['a'] = lTable['A'] = lTable['ä'] = lTable['Ä'] = 17;
	lTable['b'] = lTable['B'] = 17;
	lTable['c'] = lTable['C'] = 19;
	lTable['d'] = lTable['D'] = 17;
	lTable['e'] = lTable['E'] = 15;
	lTable['f'] = lTable['F'] = 17;
	lTable['g'] = lTable['G'] = 19;
	lTable['i'] = lTable['I'] = 16;
	lTable['h'] = lTable['H'] = 17;
	lTable['k'] = lTable['K'] = 17;
	lTable['l'] = lTable['L'] = 14;
	lTable['m'] = lTable['M'] = 19;
	lTable['n'] = lTable['N'] = 17;
	lTable['o'] = lTable['O'] = lTable['ö'] = lTable['Ö'] = 19;
	lTable['p'] = lTable['P'] = 17;
	lTable['q'] = lTable['Q'] = 19;
	lTable['r'] = lTable['R'] = 14;
	lTable['s'] = lTable['S'] = 13;
	lTable['t'] = lTable['T'] = 15;
	lTable['u'] = lTable['U'] = lTable['ü'] = lTable['Ü'] = 15;
	lTable['v'] = lTable['V'] = 13;
	lTable['x'] = lTable['X'] = 15;
	lTable['y'] = lTable['Y'] = 13;
	lTable['w'] = lTable['W'] = 19;
	lTable['z'] = lTable['Z'] = 20;
	lTable[(byte)'Ñ'] = 17;

	/* Casi particolari */
	setBothCase('C', 'C', 2);
	setBothCase('A', 'T', -2);
	setBothCase('R', 'S', 2);
	setBothCase('H', 'I', -2);
	setBothCase('T', 'S', 2);
	setBothCase('O', 'R', 2);
	setBothCase('O', 'L', 2);
	setBothCase('O', 'G', 2);
	setBothCase('Z', 'A', -1);
	setBothCase('R', 'R', 1);
	setBothCase('R', 'U', 3);

	if (_vm->getLanguage() == Common::PL_POL) {
		// Polish characters
		//¥¹ÆæÊê£³ÑñÓóŒœ¯¿Ÿ
		//AaCcEeLlNnOoSsZzZz
		cTable[(byte)'¥'] = cTable[(byte)'¹'] = 85;
		lTable[(byte)'¥'] = lTable[(byte)'¹'] = 20;

		cTable[(byte)'Æ'] = cTable[(byte)'æ'] = 86;
		lTable[(byte)'Æ'] = lTable[(byte)'æ'] = 22;

		cTable[(byte)'Ê'] = cTable[(byte)'ê'] = 87;
		lTable[(byte)'Ê'] = lTable[(byte)'ê'] = 17;

		cTable[(byte)'£'] = cTable[(byte)'³'] = 88;
		lTable[(byte)'£'] = lTable[(byte)'³'] = 19;

		cTable[(byte)'Ñ'] = cTable[(byte)'ñ'] = 89;
		lTable[(byte)'Ñ'] = lTable[(byte)'ñ'] = 17;

		cTable[(byte)'Ó'] = cTable[(byte)'ó'] = 90;
		lTable[(byte)'Ó'] = lTable[(byte)'ó'] = 22;

		cTable[(byte)'Œ'] = cTable[(byte)'œ'] = 91;
		lTable[(byte)'Œ'] = lTable[(byte)'œ'] = 15;

		cTable[(byte)'¯'] = cTable[(byte)'¿'] = 92;
		lTable[(byte)'¯'] = lTable[(byte)'¿'] = 21;

		cTable[(byte)''] = cTable[(byte)'Ÿ'] = 93;
		lTable[(byte)''] = lTable[(byte)'Ÿ'] = 21;

	} else if (_vm->getLanguage() == Common::RU_RUS) {
		// Russian Characters
		// WARNING: The Russian font uses many of the ISO-Latin-1 font,
		// allowing for further translations. To support Tonyin other langauges,
		// these mappings could be used as a basis

		cTable[(byte)'¥'] = cTable[(byte)'¹'] = 85;
		lTable[(byte)'¥'] = lTable[(byte)'¹'] = 20;

		cTable[(byte)'À'] = cTable[(byte)'à'] = 94;
		cTable[(byte)'Á'] = cTable[(byte)'á'] = 95;
		cTable[(byte)'Â'] = cTable[(byte)'â'] = 96;
		cTable[(byte)'Ã'] = cTable[(byte)'ã'] = 97;
		cTable[(byte)'Ä'] = cTable[(byte)'ä'] = 98;
		cTable[(byte)'Å'] = cTable[(byte)'å'] = 99;
		cTable[(byte)'¨'] = cTable[(byte)'¸'] = 100;
		cTable[(byte)'Æ'] = cTable[(byte)'æ'] = 101;
		cTable[(byte)'Ç'] = cTable[(byte)'ç'] = 102;
		cTable[(byte)'È'] = cTable[(byte)'è'] = 103;
		cTable[(byte)'É'] = cTable[(byte)'é'] = 104;
		cTable[(byte)'Ê'] = cTable[(byte)'ê'] = 105;
		cTable[(byte)'Ë'] = cTable[(byte)'ë'] = 106;
		cTable[(byte)'Ì'] = cTable[(byte)'ì'] = 107;
		cTable[(byte)'Í'] = cTable[(byte)'í'] = 108;
		cTable[(byte)'Î'] = cTable[(byte)'î'] = 109;
		cTable[(byte)'Ï'] = cTable[(byte)'ï'] = 110;
		cTable[(byte)'Ð'] = cTable[(byte)'ð'] = 111;
		cTable[(byte)'Ñ'] = cTable[(byte)'ñ'] = 112;
		cTable[(byte)'Ò'] = cTable[(byte)'ò'] = 113;
		cTable[(byte)'Ó'] = cTable[(byte)'ó'] = 114;
		cTable[(byte)'Ô'] = cTable[(byte)'ô'] = 115;
		cTable[(byte)'Õ'] = cTable[(byte)'õ'] = 116;
		cTable[(byte)'Ö'] = cTable[(byte)'ö'] = 117;
		cTable[(byte)'×'] = cTable[(byte)'÷'] = 118;
		cTable[(byte)'Ø'] = cTable[(byte)'ø'] = 119;
		cTable[(byte)'Ù'] = cTable[(byte)'ù'] = 120;
		cTable[(byte)'Ü'] = cTable[(byte)'ü'] = 121;
		cTable[(byte)'Ú'] = cTable[(byte)'ú'] = 122;
		cTable[(byte)'Û'] = cTable[(byte)'û'] = 123;
		cTable[(byte)'Ý'] = cTable[(byte)'ý'] = 124;
		cTable[(byte)'Þ'] = cTable[(byte)'þ'] = 125;
		cTable[(byte)'ß'] = cTable[(byte)'ÿ'] = 126;


		lTable[(byte)'À'] = lTable[(byte)'à'] = 18;
		lTable[(byte)'Á'] = lTable[(byte)'á'] = 18;
		lTable[(byte)'Â'] = lTable[(byte)'â'] = 18;
		lTable[(byte)'Ã'] = lTable[(byte)'ã'] = 17;
		lTable[(byte)'Ä'] = lTable[(byte)'ä'] = 16;
		lTable[(byte)'Å'] = lTable[(byte)'å'] = 18;
		lTable[(byte)'¨'] = lTable[(byte)'¸'] = 18;
		lTable[(byte)'Æ'] = lTable[(byte)'æ'] = 20;
		lTable[(byte)'Ç'] = lTable[(byte)'ç'] = 18;
		lTable[(byte)'È'] = lTable[(byte)'è'] = 18;
		lTable[(byte)'É'] = lTable[(byte)'é'] = 18;
		lTable[(byte)'Ê'] = lTable[(byte)'ê'] = 18;
		lTable[(byte)'Ë'] = lTable[(byte)'ë'] = 16;
		lTable[(byte)'Ì'] = lTable[(byte)'ì'] = 18;
		lTable[(byte)'Í'] = lTable[(byte)'í'] = 15;
		lTable[(byte)'Î'] = lTable[(byte)'î'] = 22;
		lTable[(byte)'Ï'] = lTable[(byte)'ï'] = 15;
		lTable[(byte)'Ð'] = lTable[(byte)'ð'] = 18;
		lTable[(byte)'Ñ'] = lTable[(byte)'ñ'] = 22;
		lTable[(byte)'Ò'] = lTable[(byte)'ò'] = 19;
		lTable[(byte)'Ó'] = lTable[(byte)'ó'] = 16;
		lTable[(byte)'Ô'] = lTable[(byte)'ô'] = 21;
		lTable[(byte)'Õ'] = lTable[(byte)'õ'] = 20;
		lTable[(byte)'Ö'] = lTable[(byte)'ö'] = 16;
		lTable[(byte)'×'] = lTable[(byte)'÷'] = 16;
		lTable[(byte)'Ø'] = lTable[(byte)'ø'] = 19;
		lTable[(byte)'Ù'] = lTable[(byte)'ù'] = 22;
		lTable[(byte)'Ü'] = lTable[(byte)'ü'] = 18;
		lTable[(byte)'Ú'] = lTable[(byte)'ú'] = 19;
		lTable[(byte)'Û'] = lTable[(byte)'û'] = 19;
		lTable[(byte)'Ý'] = lTable[(byte)'ý'] = 15;
		lTable[(byte)'Þ'] = lTable[(byte)'þ'] = 18;
		lTable[(byte)'ß'] = lTable[(byte)'ÿ'] = 18;

	} else if (_vm->getLanguage() == Common::CZ_CZE) {
		// Czech

		cTable[(byte)'Ì'] = cTable[(byte)'ì'] = 127;
		cTable[(byte)'Š'] = cTable[(byte)'š'] = 128;
		cTable[(byte)'È'] = cTable[(byte)'è'] = 129;
		cTable[(byte)'Ø'] = cTable[(byte)'ø'] = 130;
		cTable[(byte)'Ž'] = cTable[(byte)'ž'] = 131;
		cTable[(byte)'Ý'] = cTable[(byte)'ý'] = 132;
		cTable[(byte)'Á'] = cTable[(byte)'á'] = 133;
		cTable[(byte)'Í'] = cTable[(byte)'í'] = 134;
		cTable[(byte)'É'] = cTable[(byte)'é'] = 135;
		cTable[(byte)'Ï'] = cTable[(byte)'ï'] = 136;
		cTable[(byte)''] = cTable[(byte)''] = 137;
		cTable[(byte)'Ò'] = cTable[(byte)'ò'] = 138;
		cTable[(byte)'Ó'] = cTable[(byte)'ó'] = 139;
		cTable[(byte)'Ù'] = cTable[(byte)'ù'] = 140;
		cTable[(byte)'Ú'] = cTable[(byte)'ú'] = 141;

		lTable[(byte)'Ì'] = lTable[(byte)'ì'] = 17;
		lTable[(byte)'Š'] = lTable[(byte)'š'] = 15;
		lTable[(byte)'È'] = lTable[(byte)'è'] = 22;
		lTable[(byte)'Ø'] = lTable[(byte)'ø'] = 18;
		lTable[(byte)'Ž'] = lTable[(byte)'ž'] = 21;
		lTable[(byte)'Ý'] = lTable[(byte)'ý'] = 16;
		lTable[(byte)'Á'] = lTable[(byte)'á'] = 18;
		lTable[(byte)'Í'] = lTable[(byte)'í'] = 19;
		lTable[(byte)'É'] = lTable[(byte)'é'] = 17;
		lTable[(byte)'Ï'] = lTable[(byte)'ï'] = 23;
		lTable[(byte)''] = lTable[(byte)''] = 24;
		lTable[(byte)'Ò'] = lTable[(byte)'ò'] = 17;
		lTable[(byte)'Ó'] = lTable[(byte)'ó'] = 22;
		lTable[(byte)'Ù'] = lTable[(byte)'ù'] = 16;
		lTable[(byte)'Ú'] = lTable[(byte)'ú'] = 16;

	} else if (_vm->getLanguage() == Common::FR_FRA) {
		// French

		// Translate accented characters as normal letters

		cTable[(byte)'à'] = cTable[(byte)'á'] = cTable[(byte)'â'] = 0; // a
		lTable[(byte)'à'] = lTable[(byte)'á'] = lTable[(byte)'â'] = 17;

		cTable[(byte)'é'] = cTable[(byte)'è'] = 4; // e
		lTable[(byte)'é'] = lTable[(byte)'è'] = 15;

		cTable[(byte)'ì'] = cTable[(byte)'í'] = cTable[(byte)'î'] = 8; // i
		lTable[(byte)'ì'] = lTable[(byte)'í'] = lTable[(byte)'î'] = 16;

		cTable[(byte)'ò'] = cTable[(byte)'ó'] = cTable[(byte)'ô'] = cTable[(byte)'ö'] = 14; // o
		lTable[(byte)'ò'] = lTable[(byte)'ó'] = lTable[(byte)'ô'] = lTable[(byte)'ö'] = 19;

		cTable[(byte)'ù'] = cTable[(byte)'û'] = 20; // u
		lTable[(byte)'ù'] = lTable[(byte)'û'] = 15;

	} else if (_vm->getLanguage() == Common::DE_DEU) {
		// German

		cTable['ß'] = 142;
		// SS = 143

		lTable['ß'] = 24;
	}
}


/****************************************************************************\
*       RMText Methods
\****************************************************************************/

RMFontColor *RMText::_fonts[4] = { NULL, NULL, NULL, NULL };
RMGfxClearTask RMText::_clear;

void RMText::initStatics() {
	Common::fill(&_fonts[0], &_fonts[4], (RMFontColor *)NULL);
}

RMText::RMText() {
	// Default colour: white
	m_r = m_g = m_b = 255;

	// Default length
	maxLineLength = 350;

	_bTrasp0 = true;
	aHorType = HCENTER;
	aVerType = VTOP;
	setPriority(150);
}

RMText::~RMText() {

}

void RMText::unload() {
	if (_fonts[0] != NULL) {
		delete _fonts[0];
		delete _fonts[1];
		delete _fonts[2];
		delete _fonts[3];
		_fonts[0] =  _fonts[1] = _fonts[2] = _fonts[3] = 0;
	}
}

void RMText::setMaxLineLength(int max) {
	maxLineLength = max;
}

void RMText::removeThis(CORO_PARAM, bool &result) {
	// Here we can do checks on the number of frames, time spent, etc.
	result = true;
}


void RMText::writeText(const RMString &text, int nFont, int *time) {
	// Initialises the font (only once)
	if (_fonts[0] == NULL) {
		_fonts[0] = new RMFontParla;
		_fonts[0]->init();
		_fonts[1] = new RMFontObj;
		_fonts[1]->init();
		_fonts[2] = new RMFontMacc;
		_fonts[2]->init();
		_fonts[3] = new RMFontCredits;
		_fonts[3]->init();
	}

	writeText(text, _fonts[nFont], time);
}


void RMText::writeText(const RMString &text, RMFontColor *font, int *time) {
	RMGfxPrimitive *prim;
	char *p, *old_p;
	int i, j, x, y;
	int len;
	int numchar;
	int width, height;
	char *string;
	int numlines;

	// Set the base colour
	font->setBaseColor(m_r, m_g, m_b);

	// Destroy the buffer before starting
	destroy();

	// If the string is empty, do nothing
	if (text == NULL || text[0] == '\0')
		return;

	// Divide the words into lines. In this cycle, X contains the maximum length reached by a line,
	// and the number of lines
	string = p = text;
	i = j = x = 0;
	while (*p != '\0') {
		j += font->stringLen(*p);
		if (j > (((aHorType == HLEFTPAR) && (i > 0)) ? maxLineLength - 25 : maxLineLength)) {
			j -= font->stringLen(*p, p[1]);
			if (j > x)
				x = j;

			// Back to the first usable space
			//
			// BERNIE: In the original, sentences containing words that exceed the
			// width of a line caused discontinuation of the whole sentence.
			// This workaround has the partial word broken up so it will still display
			//
			old_p = p;
			while (*p != ' ' && *p != '-' && p > string)
				p--;

			if (p == string)
				p = old_p;

			// Check if there are any blanks to end
			while (*p == ' ' && *p != '\0')
				p++;
			if (*p == '\0')
				break;
			p--;
			i++;
			*p = '\0';
			j = 0;
		}
		p++;
	}

	if (j > x)
		x = j;

	i++;
	numlines = i;

	x += 8;

	// Starting position for the surface: X1, Y
	width = x;
	height = (numlines - 1) * font->letterHeight() + font->_fontDimy;

	// Create the surface
	create(width, height);
	//AddPrim(new RMGfxPrimitive(&m_clear));
	Common::fill(_buf, _buf + width * height * 2, 0);

	p = string;

	y = 0;
	numchar = 0;
	for (; i > 0; i--) {
		// Measure the length of the line
		x = 0;
		j = font->stringLen(RMString(p));

		switch (aHorType) {
		case HLEFT:
			x = 0;
			break;

		case HLEFTPAR:
			if (i == numlines)
				x = 0;
			else
				x = 25;
			break;

		case HCENTER:
			x = width / 2 - j / 2;
			break;

		case HRIGHT:
			x = width - j - 1;
			break;
		}

		while (*p != '\0') {
			if (*p == ' ') {
				x += font->stringLen(*p);
				p++;
				continue;
			}

			prim = font->makeLetterPrimitive(*p, len);
			prim->getDst().x1 = x;
			prim->getDst().y1 = y;
			addPrim(prim);

			numchar++;

			x += font->stringLen(*p, p[1]);
			p++;
		}
		p++;
		y += font->letterHeight();
	}

	if (time != NULL)
		*time = 1000 + numchar * (11 - GLOBALS.nCfgTextSpeed) * 14;
}

void RMText::clipOnScreen(RMGfxPrimitive *prim) {
	// Don't let it go outside the screen
	if (prim->getDst().x1 < 5)
		prim->getDst().x1 = 5;
	if (prim->getDst().y1 < 5)
		prim->getDst().y1 = 5;
	if (prim->getDst().x1 + _dimx > 635)
		prim->getDst().x1 = 635 - _dimx;
	if (prim->getDst().y1 + _dimy > 475)
		prim->getDst().y1 = 475 - _dimy;
}

void RMText::draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);
	// Horizontally
	if (aHorType == HCENTER)
		prim->getDst().topLeft() -= RMPoint(_dimx / 2, 0);
	else if (aHorType == HRIGHT)
		prim->getDst().topLeft() -= RMPoint(_dimx, 0);


	// Vertically
	if (aVerType == VTOP) {

	} else if (aVerType == VCENTER) {
		prim->getDst().y1 -= _dimy / 2;

	} else if (aVerType == VBOTTOM) {
		prim->getDst().y1 -= _dimy;
	}

	clipOnScreen(prim);

	CORO_INVOKE_2(RMGfxWoodyBuffer::draw, bigBuf, prim);

	CORO_END_CODE;
}

/****************************************************************************\
*       RMTextDialog Methods
\****************************************************************************/

RMTextDialog::RMTextDialog() : RMText() {
	_startTime = 0;
	dst = RMPoint(0, 0);

	_bSkipStatus = true;
	_bShowed = true;
	_bForceTime = false;
	_bForceNoTime = false;
	_bAlwaysDisplay = false;
	_bNoTab = false;
	hCustomSkip = CORO_INVALID_PID_VALUE;
	hCustomSkip2 = CORO_INVALID_PID_VALUE;
	_input = NULL;

	// Create the event for displaying the end
	hEndDisplay = CoroScheduler.createEvent(false, false);
}

RMTextDialog::~RMTextDialog() {
	CoroScheduler.closeEvent(hEndDisplay);
}

void RMTextDialog::show(void) {
	_bShowed = true;
}

void RMTextDialog::hide(CORO_PARAM) {
	_bShowed = false;
}

void RMTextDialog::writeText(const RMString &text, int font, int *time) {
	RMText::writeText(text, font, &_time);

	if (time != NULL)
		*time = _time;
}

void RMTextDialog::writeText(const RMString &text, RMFontColor *font, int *time) {
	RMText::writeText(text, font, &_time);

	if (time != NULL)
		*time = _time;
}


void RMTextDialog::setSkipStatus(bool bEnabled) {
	_bSkipStatus = bEnabled;
}

void RMTextDialog::forceTime(void) {
	_bForceTime = true;
}

void RMTextDialog::forceNoTime(void) {
	_bForceNoTime = true;
}

void RMTextDialog::setNoTab(void) {
	_bNoTab = true;
}

void RMTextDialog::setForcedTime(uint32 dwTime) {
	_time = dwTime;
}

void RMTextDialog::setAlwaysDisplay(void) {
	_bAlwaysDisplay = true;
}

void RMTextDialog::removeThis(CORO_PARAM, bool &result) {
	CORO_BEGIN_CONTEXT;
	bool expired;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	// Presume successful result
	result = true;

	// Don't erase the background
	if (_bSkipStatus) {
		if (!(GLOBALS.bCfgDubbing && hCustomSkip2 != CORO_INVALID_PID_VALUE)) {
			if (GLOBALS.bCfgTimerizedText) {
				if (!_bForceNoTime) {
					if (_vm->getTime() > (uint32)_time + _startTime)
						return;
				}
			}
		}

		if (!_bNoTab) {
			if (_vm->getEngine()->getInput().getAsyncKeyState(Common::KEYCODE_TAB))
				return;
		}

		if (!_bNoTab) {
			if (_input) {
				if (_input->mouseLeftClicked() || _input->mouseRightClicked())
					return;
			}
		}
	}
	// Erase the background
	else if (!(GLOBALS.bCfgDubbing && hCustomSkip2 != CORO_INVALID_PID_VALUE)) {
		if (!_bForceNoTime) {
			if (_vm->getTime() > (uint32)_time + _startTime)
				return;
		}
	}

	// If time is forced
	if (_bForceTime) {
		if (_vm->getTime() > (uint32)_time + _startTime)
			return;
	}

	if (hCustomSkip != CORO_INVALID_PID_VALUE) {
		CORO_INVOKE_3(CoroScheduler.waitForSingleObject, hCustomSkip, 0, &_ctx->expired);
		// == WAIT_OBJECT_0
		if (!_ctx->expired)
			return;
	}

	if (GLOBALS.bCfgDubbing && hCustomSkip2 != CORO_INVALID_PID_VALUE) {
		CORO_INVOKE_3(CoroScheduler.waitForSingleObject, hCustomSkip2, 0, &_ctx->expired);
		// == WAIT_OBJECT_0
		if (!_ctx->expired)
			return;
	}

	result = false;

	CORO_END_CODE;
}

void RMTextDialog::Unregister(void) {
	RMGfxTask::Unregister();
	assert(_nInList == 0);
	CoroScheduler.setEvent(hEndDisplay);
}

void RMTextDialog::draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	if (_startTime == 0)
		_startTime = _vm->getTime();

	if (_bShowed) {
		if (GLOBALS.bCfgSottotitoli || _bAlwaysDisplay) {
			prim->getDst().topLeft() = dst;
			CORO_INVOKE_2(RMText::draw, bigBuf, prim);
		}
	}

	CORO_END_CODE;
}

void RMTextDialog::setCustomSkipHandle(uint32 hCustom) {
	hCustomSkip = hCustom;
}

void RMTextDialog::setCustomSkipHandle2(uint32 hCustom) {
	hCustomSkip2 = hCustom;
}

void RMTextDialog::waitForEndDisplay(CORO_PARAM) {
	CoroScheduler.waitForSingleObject(coroParam, hEndDisplay, CORO_INFINITE);
}

void RMTextDialog::setInput(RMInput *input) {
	_input = input;
}

/****************************************************************************\
*       RMTextDialogScrolling Methods
\****************************************************************************/

RMTextDialogScrolling::RMTextDialogScrolling() {
	curLoc = NULL;
}

RMTextDialogScrolling::RMTextDialogScrolling(RMLocation *loc) {
	curLoc = loc;
	startScroll = loc->scrollPosition();
}

RMTextDialogScrolling::~RMTextDialogScrolling() {
}

void RMTextDialogScrolling::draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim) {
	CORO_BEGIN_CONTEXT;
	RMPoint curDst;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	_ctx->curDst = dst;

	if (curLoc != NULL)
		dst -= curLoc->scrollPosition() - startScroll;

	CORO_INVOKE_2(RMTextDialog::draw, bigBuf, prim);

	dst = _ctx->curDst;

	CORO_END_CODE;
}

void RMTextDialogScrolling::clipOnScreen(RMGfxPrimitive *prim) {
	// We must not do anything!
}


/****************************************************************************\
*       RMTextItemName Methods
\****************************************************************************/

RMTextItemName::RMTextItemName() : RMText() {
	_item = NULL;
	setPriority(220);
}

RMTextItemName::~RMTextItemName() {

}

void RMTextItemName::doFrame(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMLocation &loc, RMPointer &ptr, RMInventory &inv) {
	CORO_BEGIN_CONTEXT;
	RMItem *lastItem;
	uint32 hThread;
	CORO_END_CONTEXT(_ctx);

	RMString itemName;

	CORO_BEGIN_CODE(_ctx);

	_ctx->lastItem = _item;

	// Adds to the list if there is need
	if (!_nInList)
		bigBuf.addPrim(new RMGfxPrimitive(this));

	// Update the scrolling co-ordinates
	_curscroll = loc.scrollPosition();

	// Check if we are on the inventory
	if (inv.itemInFocus(_mpos))
		_item = inv.whichItemIsIn(_mpos);
	else
		_item = loc.whichItemIsIn(_mpos);

	itemName = "";

	// If there an item, get its name
	if (_item != NULL)
		_item->getName(itemName);

	// Write it
	writeText(itemName, 1);

	// Handle the change If the selected item is different from the previous one
	if (_ctx->lastItem != _item) {
		if (_item == NULL)
			ptr.setSpecialPointer(RMPointer::PTR_NONE);
		else {
			_ctx->hThread = mpalQueryDoAction(20, _item->mpalCode(), 0);
			if (_ctx->hThread == CORO_INVALID_PID_VALUE)
				ptr.setSpecialPointer(RMPointer::PTR_NONE);
			else
				CORO_INVOKE_2(CoroScheduler.waitForSingleObject, _ctx->hThread, CORO_INFINITE);
		}
	}

	CORO_END_CODE;
}


void RMTextItemName::draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	// If there is no text, it's pointless to continue
	if (_buf == NULL)
		return;

	// Set the destination coordinates of the mouse
	prim->getDst().topLeft() = _mpos - RMPoint(0, 30);

	CORO_INVOKE_2(RMText::draw, bigBuf, prim);

	CORO_END_CODE;
}

RMPoint RMTextItemName::getHotspot() {
	if (_item == NULL)
		return _mpos + _curscroll;
	else
		return _item->hotspot();
}

RMItem *RMTextItemName::getSelectedItem() {
	return _item;
}

bool RMTextItemName::isItemSelected() {
	return _item != NULL;
}

bool RMTextItemName::isNormalItemSelected() {
	return _item != NULL && _itemName.length() > 0;
}


/****************************************************************************\
*       RMDialogChoice Methods
\****************************************************************************/

RMDialogChoice::RMDialogChoice() {
	RMResRaw dlg1(RES_I_DLGTEXT);
	RMResRaw dlg2(RES_I_DLGTEXTLINE);
	RMRes dlgpal(RES_I_DLGTEXTPAL);

	DlgText.init(dlg1, dlg1.width(), dlg1.height());
	DlgTextLine.init(dlg2, dlg2.width(), dlg2.height());

	DlgText.loadPaletteWA(dlgpal);
	DlgTextLine.loadPaletteWA(dlgpal);

	hUnreg = CoroScheduler.createEvent(false, false);
	bRemoveFromOT = false;
}

RMDialogChoice::~RMDialogChoice() {
	CoroScheduler.closeEvent(hUnreg);
}

void RMDialogChoice::Unregister(void) {
	RMGfxWoodyBuffer::Unregister();
	assert(!_nInList);
	CoroScheduler.pulseEvent(hUnreg);

	bRemoveFromOT = false;
}

void RMDialogChoice::init(void) {
	_numChoices = 0;
	_drawedStrings = NULL;
	_ptDrawStrings = NULL;
	_curSelection = -1;

	create(640, 477);
	setPriority(140);
}


void RMDialogChoice::close(void) {
	if (_drawedStrings != NULL) {
		delete[] _drawedStrings;
		_drawedStrings = NULL;
	}

	if (_ptDrawStrings != NULL) {
		delete[] _ptDrawStrings;
		_ptDrawStrings = NULL;
	}

	destroy();
}

void RMDialogChoice::setNumChoices(int num) {
	int i;

	_numChoices = num;
	_curAdded = 0;

	// Allocate space for drawn strings
	_drawedStrings = new RMText[num];
	_ptDrawStrings = new RMPoint[num];

	// Initialisation
	for (i = 0; i < _numChoices; i++) {
		_drawedStrings[i].setColor(0, 255, 0);
		_drawedStrings[i].setAlignType(RMText::HLEFTPAR, RMText::VTOP);
		_drawedStrings[i].setMaxLineLength(600);
		_drawedStrings[i].setPriority(10);
	}
}

void RMDialogChoice::addChoice(const RMString &string) {
	// Draw the string
	assert(_curAdded < _numChoices);
	_drawedStrings[_curAdded++].writeText(string, 0);
}

void RMDialogChoice::prepare(CORO_PARAM) {
	CORO_BEGIN_CONTEXT;
	int i;
	RMPoint ptPos;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	addPrim(new RMGfxPrimitive(&DlgText, RMPoint(0, 0)));
	addPrim(new RMGfxPrimitive(&DlgTextLine, RMPoint(0, 155)));
	addPrim(new RMGfxPrimitive(&DlgTextLine, RMPoint(0, 155 + 83)));
	addPrim(new RMGfxPrimitive(&DlgTextLine, RMPoint(0, 155 + 83 + 83)));
	addPrim(new RMGfxPrimitive(&DlgTextLine, RMPoint(0, 155 + 83 + 83 + 83)));

	_ctx->ptPos.set(20, 90);

	for (_ctx->i = 0; _ctx->i < _numChoices; _ctx->i++) {
		addPrim(new RMGfxPrimitive(&_drawedStrings[_ctx->i], _ctx->ptPos));
		_ptDrawStrings[_ctx->i] = _ctx->ptPos;
		_ctx->ptPos.offset(0, _drawedStrings[_ctx->i].getDimy() + 15);
	}

	CORO_INVOKE_0(drawOT);
	clearOT();

	_ptDrawPos.set(0, 480 - _ctx->ptPos.y);

	CORO_END_CODE;
}

void RMDialogChoice::setSelected(CORO_PARAM, int pos) {
	CORO_BEGIN_CONTEXT;
	RMGfxBox box;
	RMRect rc;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	if (pos == _curSelection)
		return;

	_ctx->box.setPriority(5);

	if (_curSelection != -1) {
		_ctx->box.setColor(0xCC, 0xCC, 0xFF);
		_ctx->rc.topLeft() = RMPoint(18, _ptDrawStrings[_curSelection].y);
		_ctx->rc.bottomRight() = _ctx->rc.topLeft() + RMPoint(597, _drawedStrings[_curSelection].getDimy());
		addPrim(new RMGfxPrimitive(&_ctx->box, _ctx->rc));

		addPrim(new RMGfxPrimitive(&_drawedStrings[_curSelection], _ptDrawStrings[_curSelection]));
		CORO_INVOKE_0(drawOT);
		clearOT();
	}

	if (pos != -1) {
		_ctx->box.setColor(100, 100, 100);
		_ctx->rc.topLeft() = RMPoint(18, _ptDrawStrings[pos].y);
		_ctx->rc.bottomRight() = _ctx->rc.topLeft() + RMPoint(597, _drawedStrings[pos].getDimy());
		addPrim(new RMGfxPrimitive(&_ctx->box, _ctx->rc));
		addPrim(new RMGfxPrimitive(&_drawedStrings[pos], _ptDrawStrings[pos]));
	}

	CORO_INVOKE_0(drawOT);
	clearOT();

	_curSelection = pos;

	CORO_END_CODE;
}

void RMDialogChoice::show(CORO_PARAM, RMGfxTargetBuffer *bigBuf) {
	CORO_BEGIN_CONTEXT;
	RMPoint destpt;
	int deltay;
	int starttime;
	int elaps;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	CORO_INVOKE_0(prepare);
	_bShow = false;

	if (!_nInList && bigBuf != NULL)
		bigBuf->addPrim(new RMGfxPrimitive(this));

	if (0) {
		_bShow = true;
	} else {
		_ctx->starttime = _vm->getTime();
		_ctx->deltay = 480 - _ptDrawPos.y;
		_ctx->destpt = _ptDrawPos;
		_ptDrawPos.set(0, 480);

		if (!_nInList && bigBuf != NULL)
			bigBuf->addPrim(new RMGfxPrimitive(this));
		_bShow = true;

		_ctx->elaps = 0;
		while (_ctx->elaps < 700) {
			CORO_INVOKE_0(mainWaitFrame);
			mainFreeze();
			_ctx->elaps = _vm->getTime() - _ctx->starttime;
			_ptDrawPos.y = 480 - ((_ctx->deltay * 100) / 700 * _ctx->elaps) / 100;
			mainUnfreeze();
		}

		_ptDrawPos.y = _ctx->destpt.y;
	}

	CORO_END_CODE;
}

void RMDialogChoice::draw(CORO_PARAM, RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim) {
	CORO_BEGIN_CONTEXT;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	if (_bShow == false)
		return;

	prim->setDst(_ptDrawPos);
	CORO_INVOKE_2(RMGfxSourceBuffer16::draw, bigBuf, prim);

	CORO_END_CODE;
}


void RMDialogChoice::hide(CORO_PARAM) {
	CORO_BEGIN_CONTEXT;
	int deltay;
	int starttime;
	int elaps;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	if (1) {
		_ctx->starttime = _vm->getTime();

		_ctx->deltay = 480 - _ptDrawPos.y;
		_ctx->elaps = 0;
		while (_ctx->elaps < 700) {
			CORO_INVOKE_0(mainWaitFrame);
			mainFreeze();
			_ctx->elaps = _vm->getTime() - _ctx->starttime;
			_ptDrawPos.y = 480 - ((_ctx->deltay * 100) / 700 * (700 - _ctx->elaps)) / 100;
			mainUnfreeze();
		}
	}

	_bShow = false;
	bRemoveFromOT = true;
	CORO_INVOKE_2(CoroScheduler.waitForSingleObject, hUnreg, CORO_INFINITE);

	CORO_END_CODE;
}


void RMDialogChoice::removeThis(CORO_PARAM, bool &result) {
	result = bRemoveFromOT;
}

void RMDialogChoice::doFrame(CORO_PARAM, RMPoint ptMousePos) {
	CORO_BEGIN_CONTEXT;
	int i;
	CORO_END_CONTEXT(_ctx);

	CORO_BEGIN_CODE(_ctx);

	if (ptMousePos.y > _ptDrawPos.y) {
		for (_ctx->i = 0; _ctx->i < _numChoices; _ctx->i++) {
			if ((ptMousePos.y >= _ptDrawPos.y + _ptDrawStrings[_ctx->i].y) && (ptMousePos.y < _ptDrawPos.y + _ptDrawStrings[_ctx->i].y + _drawedStrings[_ctx->i].getDimy())) {
				CORO_INVOKE_1(setSelected, _ctx->i);
				break;
			}
		}

		if (_ctx->i == _numChoices)
			CORO_INVOKE_1(setSelected, -1);
	}

	CORO_END_CODE;
}

int RMDialogChoice::getSelection(void) {
	return _curSelection;
}

} // End of namespace Tony
