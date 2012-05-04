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
/**************************************************************************
 *                                     ÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ *
 *                                             Nayma Software srl         *
 *                    e                -= We create much MORE than ALL =- *
 *        u-        z$$$c        '.    ÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ *
 *      .d"        d$$$$$b        "b.                                     *
 *   .z$*         d$$$$$$$L        ^*$c.                                  *
 *  #$$$.         $$$$$$$$$         .$$$" Project: Roasted Moths........  *
 *    ^*$b       4$$$$$$$$$F      .d$*"                                   *
 *      ^$$.     4$$$$$$$$$F     .$P"     Module:  Font.CPP.............  *
 *        *$.    '$$$$$$$$$     4$P 4                                     *
 *     J   *$     "$$$$$$$"     $P   r    Author:  Giovanni Bajo........  *
 *    z$   '$$$P*4c.*$$$*.z@*R$$$    $.                                   *
 *   z$"    ""       #$F^      ""    '$c                                  *
 *  z$$beu     .ue="  $  "=e..    .zed$$c                                 *
 *      "#$e z$*"   .  `.   ^*Nc e$""                                     *
 *         "$$".  .r"   ^4.  .^$$"                                        *
 *          ^.@*"6L=\ebu^+C$"*b."                                         *
 *        "**$.  "c 4$$$  J"  J$P*"    OS:  [ ] DOS  [X] WIN95  [ ] PORT  *
 *            ^"--.^ 9$"  .--""      COMP:  [ ] WATCOM  [X] VISUAL C++    *
 *                    "                     [ ] EIFFEL  [ ] GCC/GXX/DJGPP *
 *                                                                        *
 * This source code is Copyright (C) Nayma Software.  ALL RIGHTS RESERVED *
 *                                                                        *
 **************************************************************************/

#include "common/textconsole.h"
#include "tony/mpal/mpalutils.h"
#include "tony/mpal/stubs.h"
#include "tony/font.h"
#include "tony/input.h"
#include "tony/inventory.h"
#include "tony/loc.h"
#include "tony/tony.h"

namespace Tony {

/****************************************************************************\
*       Metodi di RMFont
\****************************************************************************/

RMFont::RMFont() {
	m_letter = NULL;
}

RMFont::~RMFont() {
	Unload();
}

/****************************************************************************\
*
* Function:     void RMFont::Load(byte *buf, int nChars, int dimx, int dimy);
*
* Description:  Carica un font da buffer
*
* Input:        byte *buf              Buffer contenente il font
*               int nChars              Numero di caratteri (max 256)
*               int dimx,dimy           Dimensione in pixel di un carattere
*
\****************************************************************************/

void DumpFontBMP(const char *filename, const byte *buf, int nChars, int charX, int charY, byte *pal) {
	error("DumpFontBMP not supported in ScummVM");
}


void RMFont::Load(const byte *buf, int nChars, int dimx, int dimy, uint32 palResID) {
	m_letter = new RMGfxSourceBuffer8RLEByte[nChars];

#if 0
	if (nChars == 112 && palResID == RES_F_PAL) {
		// Font parla
		DumpFontBMP("font_parla.bmp", buf, nChars, dimx, dimy, RMRes(palResID));
	}
	else if (nChars == 102 && palResID == RES_F_PAL) {
		// Font macc
		DumpFontBMP("font_macc.bmp", buf, nChars, dimx, dimy, RMRes(palResID));
	} else if (nChars == 85 && palResID == RES_F_PAL) {
		// Font obj
		DumpFontBMP("font_obj.bmp", buf, nChars, dimx, dimy, RMRes(palResID));
	} else if (nChars == 112 && palResID == RES_F_CPAL) {
		// Font credits
		DumpFontBMP("font_credits.bmp", buf, nChars, dimx, dimy, RMRes(palResID));		
	}
#endif

	// Carichiamoce 'sto font
	for (int i = 0; i < nChars; i++) {
		// Inizializza il buffer con le lettere
		m_letter[i].Init(buf + i * (dimx * dimy + 8) + 8, dimx, dimy);
		m_letter[i].LoadPaletteWA(palResID);
	}

	m_fontDimx = dimx;
	m_fontDimy = dimy;

	nLetters=nChars;
}

void RMFont::Load(uint32 resID, int nChars, int dimx, int dimy, uint32 palResID) {
	RMRes res(resID);

	if ((int)res.Size() < nChars * (dimy * dimx + 8))
		nChars = res.Size() / (dimy * dimx + 8);

	Load(res, nChars, dimx, dimy, palResID);
}

void RMFont::Unload(void) {
	if (m_letter != NULL) {
		delete[] m_letter;
		m_letter = NULL;
	}
}


RMGfxPrimitive *RMFont::MakeLetterPrimitive(byte bChar, int &nLength) {
	RMFontPrimitive *prim;
	int nLett;

	// Converte da carattere a lettera
	nLett = ConvertToLetter(bChar);

	// Crea la primitiva per il font
	prim = new RMFontPrimitive(this);
	prim->m_nChar = nLett;

	// Si fa' dare la lunghezza della lettera in pixel
	nLength = LetterLength(bChar);

	return prim;
}

void RMFont::Draw(RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim2) {
  RMFontPrimitive *prim = (RMFontPrimitive *)prim2;

  // Richiama la Draw della lettera assegnata alla primitiva
  if (prim->m_nChar != -1)
	  m_letter[prim->m_nChar].Draw(bigBuf,prim);
}

void RMFont::Close(void) {
	Unload();
}

int RMFont::StringLen(RMString text) {
	int len, i;

	len = 0;
	for (i = 0; i < text.Length() - 1; i++)
		len += LetterLength(text[i], text[i + 1]);
	len += LetterLength(text[i]);

	return len;
}

int RMFont::StringLen(char bChar, char bNext) {
	return LetterLength(bChar, bNext);
}

/****************************************************************************\
*       Metodi di RMFontColor
\****************************************************************************/

RMFontColor::RMFontColor() : RMFont() {
 m_r = m_g = m_b = 255;
}

RMFontColor::~RMFontColor() {

}

void RMFontColor::SetBaseColor(byte r1, byte g1, byte b1) {
	int r = (int)r1 << 16;
	int g = (int)g1 << 16;
	int b = (int)b1 << 16;

	int rstep = r / 14;
	int gstep = g / 14;
	int bstep = b / 14;

	int i;
	byte pal[768*3];

	// Controlla se siamo gia' sul colore giusto
	if (m_r == r1 && m_g == g1 && m_b == b1)
		return;

	m_r = r1;
	m_g = g1;
	m_b = b1;

	// Costruisce la nuova palette per il font
	for (i = 1; i < 16; i++) {
		pal[i * 3 + 0] = r >> 16;	
		pal[i * 3 + 1] = g >> 16;	
		pal[i * 3 + 2] = b >> 16;	

		r -= rstep;
		g -= gstep;
		b -= bstep;
	}

	pal[15*3 + 0] += 8;
	pal[15*3 + 1] += 8;
	pal[15*3 + 2] += 8;

	// La mette in tutte le lettere
	for (i = 0; i < nLetters; i++)
		m_letter[i].LoadPaletteWA(pal);
}


/***************************************************************************\
*       Metodi di RMFontParla
\****************************************************************************/

void RMFontParla::Init(void) {
	int i;

	// bernie: numero di caratteri nel font
	int nchars =
		 112	// base
		+ 18	// polish
		+ 66	// russian
		+ 30	// czech
		+  8	// french
		+  5;	// deutsch

	Load(RES_F_PARL, nchars, 20, 20);

	// Inizializziamo le tabelline del cazzo
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
	//cTable['!'] = 72;  Esclamativo alla rovescia
	cTable['?'] = 73;
	//cTable['?'] = 74;  Interrogativo alla rovescia
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
	//cTable[' '] = 100;  e cerchietto
	//cTable[' '] = 101;  i cerchietto
	//cTable[' '] = 102;  o cerchietto
	//cTable[' '] = 103;  u cerchietto
	cTable[(byte)'ñ'] = 104;
	cTable[(byte)'Ñ'] = 105;
	cTable[(byte)'ç'] = 106;
	cTable[(byte)'æ'] = 107;
	cTable[(byte)'Æ'] = 108;
	cTable[(byte)'ø'] = 109;
	//cTable['ƒ'] = 110;  integrale 
	cTable['\''] = 111; 

	// Un po' di lunghezze
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

#ifdef FONT_RUSSIAN
	// Russian Characters
	// WARNING: Il russo usa molti dei caratteri ISO-Latin-1 che servono
	// per le altre traduzioni. Per compilare Tony in altre lingue,
	// commentare via queste definizioni.

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
	cTable[(byte)'Ğ'] = 147;
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
	cTable[(byte)'İ'] = 160;
	cTable[(byte)'Ş'] = 161;
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
	cTable[(byte)'ğ'] = 180;
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
	cTable[(byte)'ı'] = 193;
	cTable[(byte)'ş'] = 194;
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
	lTable[(byte)'Ğ'] = 12;
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
	lTable[(byte)'İ'] = 11;
	lTable[(byte)'Ş'] = 16;
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
	lTable[(byte)'ğ'] = 11;
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
	lTable[(byte)'ı'] =  9;
	lTable[(byte)'ş'] = 15;
	lTable[(byte)'ÿ'] = 14;

#endif // FONT_RUSSIAN

#ifdef FONT_CZECH

	cTable[(byte)'Ì'] = 196;
	cTable[(byte)'Š'] = 197;
	cTable[(byte)'È'] = 198;
	cTable[(byte)'Ø'] = 199;
	cTable[(byte)''] = 200;
	cTable[(byte)'İ'] = 201;
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
	cTable[(byte)''] = 215;
	cTable[(byte)'ı'] = 216;
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
	lTable[(byte)''] = 13;
	lTable[(byte)'İ'] = 11;
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
	lTable[(byte)''] = 13;
	lTable[(byte)'ı'] = 11;
	lTable[(byte)'á'] = 15;
	lTable[(byte)'í'] = 7;
	lTable[(byte)'é'] = 12;
	lTable[(byte)'ï'] = 17;
	lTable[(byte)''] = 16;
	lTable[(byte)'ò'] = 11;
	lTable[(byte)'ó'] = 11;
	lTable[(byte)'ù'] = 13;
	lTable[(byte)'ú'] = 13;

#endif // FONT_CZECH

#ifdef FONT_FRENCH
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

#endif // FONT_FRENCH

#ifdef FONT_GERMAN
	cTable[(byte)'ß'] = 234;
	// 'SS' = 235
	cTable[(byte)'Ä'] = 236;
	cTable[(byte)'Ö'] = 237;
	cTable[(byte)'Ü'] = 238;

	lTable[(byte)'ß'] = 15;

#endif // FONT_GERMAN
}


/***************************************************************************\
*       Metodi di RMFontMacc
\****************************************************************************/

void RMFontMacc::Init(void) {
	int i;

	// bernie: numero di caratteri nel font
	int nchars =
		 102	// base
		+ 18	// polish
		+ 66	// russian
		+ 30	// czech
		+  8	// francais
		+  5;	// deutsch


	Load(RES_F_MACC, nchars, 11, 16);

	// Default
	lDefault = 10;
	hDefault = 17;
	Common::fill(&l2Table[0][0], &l2Table[0][0] + (256 * 256), '\0');
	
	for (i = 0; i < 256; i++) {
		cTable[i] = -1;
		lTable[i] = lDefault;
	}

	for (i = 0; i < 26; i++)
		cTable['A'+i] = i + 0;

	for (i = 0; i < 26; i++)
		cTable['a'+i] = i + 26;

	for (i = 0; i < 10; i++)
		cTable['0'+i] = i + 52;

	cTable['!'] = 62;
	//cTable['!'] = 63;			// ! rovescia
	cTable['\"'] = 64;		
	cTable['$'] = 65;		
	cTable['%'] = 66;		
	cTable['&'] = 67;		
	cTable['/'] = 68;		
	cTable['('] = 69;		
	cTable[')'] = 70;		
	cTable['='] = 71;		
	cTable['?'] = 72;		
	//cTable['?'] = 73;		   // ? rovescia
	cTable['*'] = 74;		
	cTable['+'] = 75;		
	cTable['ñ'] = 76;		
	cTable[';'] = 77;		
	cTable[','] = 78;		
	cTable['.'] = 79;
	cTable[':'] = 80;		
	cTable['-'] = 81;		
	cTable['<'] = 82;
	cTable['>'] = 83;
	cTable['/'] = 84;
	cTable['ÿ'] = 85;
	cTable['à'] = 86;
	cTable['ä'] = 87;
	cTable['å'] = 88;
	cTable['è'] = 89;
	cTable['ë'] = 90;
	//cTable[''] = 91;		  // e col pallino
	cTable['ì'] = 92;		
	cTable['ï'] = 93;		
	//cTable[''] = 94;			// i col pallino
	cTable['ò'] = 95;		
	cTable['ö'] = 96;		
	//cTable[''] = 97;		  // o col pallino
	cTable['ù'] = 98;		
	cTable['ü'] = 99;		
	//cTable[''] = 100;		  // u col pallino
	cTable['ç'] = 101;		

	// Polish characters
	//AaCcEeLlNnOoSsZzZz
	//¥¹ÆæÊê£³ÑñÓóŒœ¯¿Ÿ
	
	cTable['¥'] = 102;
	cTable['¹'] = 103;
	cTable['Æ'] = 104;
	cTable['æ'] = 105;
	cTable['Ê'] = 106;
	cTable['ê'] = 107;
	cTable['£'] = 108;
	cTable['³'] = 109;
	cTable['Ñ'] = 110;
	cTable['ñ'] = 111;
	cTable['Ó'] = 112;
	cTable['ó'] = 113;
	cTable['Œ'] = 114;
	cTable['œ'] = 115;
	cTable['¯'] = 116;
	cTable['¿'] = 117;
	cTable[''] = 118;
	cTable['Ÿ'] = 119;
	
	lTable['¥'] = 14;
	lTable['¹'] = 16;
	lTable['Æ'] = 12;
	lTable['æ'] = 12;
	lTable['Ê'] = 11;
	lTable['ê'] = 12;
	lTable['£'] = 14;
	lTable['³'] = 9;
	lTable['Ñ'] = 10;
	lTable['ñ'] = 11;
	lTable['Ó'] = 13;
	lTable['ó'] = 11;
	lTable['Œ'] = 12;
	lTable['œ'] = 12;
	lTable['¯'] = 13;
	lTable['¿'] = 13;
	lTable[''] = 14;
	lTable['Ÿ'] = 13;


#ifdef FONT_RUSSIAN
	// Russian Characters
	// WARNING: Il russo usa molti dei caratteri ISO-Latin-1 che servono
	// per le altre traduzioni. Per compilare Tony in altre lingue,
	// commentare via queste definizioni.

	cTable['À'] = 120;
	cTable['Á'] = 121;
	cTable['Â'] = 122;
	cTable['Ã'] = 123;
	cTable['Ä'] = 124;
	cTable['Å'] = 125;
	cTable['¨'] = 126;
	cTable['Æ'] = 127;
	cTable['Ç'] = 128;
	cTable['È'] = 129;
	cTable['É'] = 130;
	cTable['Ê'] = 131;
	cTable['Ë'] = 132;
	cTable['Ì'] = 133;
	cTable['Í'] = 134;
	cTable['Î'] = 135;
	cTable['Ï'] = 136;
	cTable['Ğ'] = 137;
	cTable['Ñ'] = 138;
	cTable['Ò'] = 139;
	cTable['Ó'] = 140;
	cTable['Ô'] = 141;
	cTable['Õ'] = 142;
	cTable['Ö'] = 143;
	cTable['×'] = 144;
	cTable['Ø'] = 145;
	cTable['Ù'] = 146;
	cTable['Ü'] = 147;
	cTable['Ú'] = 148;
	cTable['Û'] = 149;
	cTable['İ'] = 150;
	cTable['Ş'] = 151;
	cTable['ß'] = 152;

	cTable['à'] = 153;
	cTable['á'] = 154;
	cTable['â'] = 155;
	cTable['ã'] = 156;
	cTable['ä'] = 157;
	cTable['å'] = 158;
	cTable['¸'] = 159;
	cTable['æ'] = 160;
	cTable['ç'] = 161;
	cTable['è'] = 162;
	cTable['é'] = 163;
	cTable['ê'] = 164;
	cTable['ë'] = 165;
	cTable['ì'] = 166;
	cTable['í'] = 167;
	cTable['î'] = 168;
	cTable['ï'] = 169;
	cTable['ğ'] = 170;
	cTable['ñ'] = 171;
	cTable['ò'] = 172;
	cTable['ó'] = 173;
	cTable['ô'] = 174;
	cTable['õ'] = 175;
	cTable['ö'] = 176;
	cTable['÷'] = 177;
	cTable['ø'] = 178;
	cTable['ù'] = 179;
	cTable['ü'] = 180;
	cTable['ú'] = 181;
	cTable['û'] = 182;
	cTable['ı'] = 183;
	cTable['ş'] = 184;
	cTable['ÿ'] = 185;

	lTable['À'] = 11;
	lTable['Á'] = 11;
	lTable['Â'] = 11;
	lTable['Ã'] = 9;
	lTable['Ä'] = 10;
	lTable['Å'] = 10;
	lTable['¨'] = 10;
	lTable['Æ'] = 11;
	lTable['Ç'] = 10;
	lTable['È'] = 10;
	lTable['É'] = 10;
	lTable['Ê'] = 11;
	lTable['Ë'] = 9;
	lTable['Ì'] = 11;
	lTable['Í'] = 10;
	lTable['Î'] = 11;
	lTable['Ï'] = 8;
	lTable['Ğ'] = 10;
	lTable['Ñ'] = 10;
	lTable['Ò'] = 11;
	lTable['Ó'] = 11;
	lTable['Ô'] = 11;
	lTable['Õ'] = 11;
	lTable['Ö'] = 10;
	lTable['×'] = 10;
	lTable['Ø'] = 10;
	lTable['Ù'] = 10;
	lTable['Ü'] = 11;
	lTable['Ú'] = 11;
	lTable['Û'] = 11;
	lTable['İ'] = 11;
	lTable['Ş'] = 11;
	lTable['ß'] = 11;

	lTable['à'] = 10;
	lTable['á'] = 10;
	lTable['â'] = 11;
	lTable['ã'] = 10;
	lTable['ä'] = 9;
	lTable['å'] = 10;
	lTable['¸'] = 9;
	lTable['æ'] = 10;
	lTable['ç'] = 9;
	lTable['è'] = 11;
	lTable['é'] = 11;
	lTable['ê'] = 11;
	lTable['ë'] = 11;
	lTable['ì'] = 11;
	lTable['í'] = 11;
	lTable['î'] = 10;
	lTable['ï'] = 9;
	lTable['ğ'] = 11;
	lTable['ñ'] = 10;
	lTable['ò'] = 9;
	lTable['ó'] = 11;
	lTable['ô'] = 10;
	lTable['õ'] = 11;
	lTable['ö'] = 10;
	lTable['÷'] = 10;
	lTable['ø'] = 11;
	lTable['ù'] = 11;
	lTable['ü'] = 10;
	lTable['ú'] = 10;
	lTable['û'] = 10;
	lTable['ı'] =  9;
	lTable['ş'] = 11;
	lTable['ÿ'] = 11;

#endif // FONT_RUSSIAN

#ifdef FONT_CZECH
	
	cTable['Ì'] = 186;
	cTable['Š'] = 187;
	cTable['È'] = 188;
	cTable['Ø'] = 189;
	cTable[''] = 190;
	cTable['İ'] = 191;
	cTable['Á'] = 192;
	cTable['Í'] = 193;
	cTable['É'] = 194;
	cTable['Ï'] = 195;
	cTable[''] = 196;
	cTable['Ò'] = 197;
	cTable['Ó'] = 198;
	cTable['Ù'] = 199;
	cTable['Ú'] = 200;

	cTable['ì'] = 201;
	cTable['š'] = 202;
	cTable['è'] = 203;
	cTable['ø'] = 204;
	cTable[''] = 205;
	cTable['ı'] = 206;
	cTable['á'] = 207;
	cTable['í'] = 208;
	cTable['é'] = 209;
	cTable['ï'] = 210;
	cTable[''] = 211;
	cTable['ò'] = 212;
	cTable['ó'] = 213;
	cTable['ù'] = 214;
	cTable['ú'] = 215;

	lTable['Ì'] = 10;
	lTable['Š'] = 10;
	lTable['È'] = 10;
	lTable['Ø'] = 11;
	lTable[''] = 9;
	lTable['İ'] = 11;
	lTable['Á'] = 11;
	lTable['Í'] = 9;
	lTable['É'] = 10;
	lTable['Ï'] = 11;
	lTable[''] = 11;
	lTable['Ò'] = 11;
	lTable['Ó'] = 11;
	lTable['Ù'] = 11;
	lTable['Ú'] = 11;

	lTable['ì'] = 10;
	lTable['š'] = 9;
	lTable['è'] = 10;
	lTable['ø'] = 10;
	lTable[''] = 9;
	lTable['ı'] = 11;
	lTable['á'] = 10;
	lTable['í'] = 9;
	lTable['é'] = 10;
	lTable['ï'] = 11;
	lTable[''] = 11;
	lTable['ò'] = 11;
	lTable['ó'] = 10;
	lTable['ù'] = 11;
	lTable['ú'] = 11;

#endif // FONT_CZECH

#ifdef FONT_FRENCH

	cTable['â'] = 226;
	cTable['é'] = 227;
	cTable['è'] = 228;
	cTable['î'] = 229;
	cTable['ù'] = 230;
	cTable['û'] = 231;
	cTable['ô'] = 232;
	cTable['ö'] = 233;

	lTable['â'] = 10;
	lTable['é'] = 10;
	lTable['ê'] = 10;
	lTable['î'] = 8;
	lTable['ù'] = 11;
	lTable['û'] = 11;
	lTable['ô'] = 10;
	lTable['ö'] = 10;

#endif // FONT_FRENCH

#ifdef FONT_GERMAN
	cTable['ß'] = 234;
	// 'SS' = 235
	cTable['Ä'] = 236;
	cTable['Ö'] = 237;
	cTable['Ü'] = 238;

	lTable['ß'] = 11;
#endif // FONT_GERMAN
}

/***************************************************************************\
*       Metodi di RMFontCredits
\****************************************************************************/

void RMFontCredits::Init(void) {
	int i;

	// bernie: numero di caratteri nel font
	int nchars =
		 112	// base
		+ 18	// polish
		+ 66	// russian
		+ 30	// czech
		+  8	// french
		+  2;	// deutsch


	Load(RES_F_CREDITS, nchars, 27, 28, RES_F_CPAL);

	// Default
	lDefault=10;
	hDefault=28;
	Common::fill(&l2Table[0][0], &l2Table[0][0] + (256 * 256), '\0');
	
	for (i = 0; i < 256; i++) {
		cTable[i] = -1;
		lTable[i] = lDefault;
	}

	for (i = 0; i < 26; i++)
		cTable['A' + i] = i + 0;

	for (i = 0; i < 26; i++)
		cTable['a' + i] = i + 26;



	cTable['à'] = 52;
	cTable['á'] = 53;
//	cTable[''] = 54; // a ^
//	cTable[''] = 55; // a pallini
	cTable['è'] = 56;
	cTable['é'] = 57; 
//	cTable[''] = 58; // e ^
//	cTable[''] = 59; // e pallini
	cTable['ì'] = 60;
	cTable['í'] = 61;
//	cTable[''] = 62; // i ^
//	cTable[''] = 63; // i pallini
	cTable['ò'] = 64;
	cTable['ó'] = 65;
//	cTable[''] = 66; // o ^
//	cTable[''] = 67; // o pallini
	cTable['ù'] = 68;
	cTable['ú'] = 69;
//	cTable[''] = 70; // u ^
//	cTable[''] = 71; // u pallini
//	cTable[''] = 72; // y pallini
	cTable['ñ'] = 73;
	cTable['ç'] = 74; 
//	cTable[''] = 75; // o barrato
//	cTable[''] = 76; // ac
	cTable['©'] = 77;
//	cTable[''] = 78; // ? rovesciato
	cTable['?'] = 79; 
//	cTable[''] = 80; // ! rovesciato
	cTable['!'] = 81; 
//	cTable[''] = 82; // 1/2
//	cTable[''] = 83; // 1/4
	cTable['('] = 84;
	cTable[')'] = 85;
	cTable['«'] = 86;
	cTable['»'] = 87;
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
		cTable['0'+i] = i+101;
	cTable['\''] = 111;  

	lTable[' '] = 11;
	lTable['Ä'] = lTable['A'] = 19;
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



	// Polish characters
	//AaCcEeLlNnOoSsZzZz
	//¥¹ÆæÊê£³ÑñÓóŒœ¯¿Ÿ
	
	cTable['¥'] = 112;
	cTable['¹'] = 113;
	cTable['Æ'] = 114;
	cTable['æ'] = 115;
	cTable['Ê'] = 116;
	cTable['ê'] = 117;
	cTable['£'] = 118;
	cTable['³'] = 119;
	cTable['Ñ'] = 120;
	cTable['ñ'] = 121;
	cTable['Ó'] = 122;
	cTable['ó'] = 123;
	cTable['Œ'] = 124;
	cTable['œ'] = 125;
	cTable['¯'] = 126;
	cTable['¿'] = 127;
	cTable[''] = 128;
	cTable['Ÿ'] = 129;
	
	lTable['¥'] = 20;
	lTable['¹'] = 12;
	lTable['Æ'] = 15;
	lTable['æ'] = 10;
	lTable['Ê'] = 15;
	lTable['ê'] = 10;
	lTable['£'] = 14;
	lTable['³'] = 11;
	lTable['Ñ'] = 16;
	lTable['ñ'] = 10;
	lTable['Ó'] = 15;
	lTable['ó'] = 11;
	lTable['Œ'] = 15;
	lTable['œ'] = 10;
	lTable['¯'] = 15;
	lTable['¿'] = 10;
	lTable[''] = 15;
	lTable['Ÿ'] = 10;


#ifdef FONT_RUSSIAN
	// Russian Characters
	// WARNING: Il russo usa molti dei caratteri ISO-Latin-1 che servono
	// per le altre traduzioni. Per compilare Tony in altre lingue,
	// commentare via queste definizioni.

	cTable['À'] = 130;
	cTable['Á'] = 131;
	cTable['Â'] = 132;
	cTable['Ã'] = 133;
	cTable['Ä'] = 134;
	cTable['Å'] = 135;
	cTable['¨'] = 136;
	cTable['Æ'] = 137;
	cTable['Ç'] = 138;
	cTable['È'] = 139;
	cTable['É'] = 140;
	cTable['Ê'] = 141;
	cTable['Ë'] = 142;
	cTable['Ì'] = 143;
	cTable['Í'] = 144;
	cTable['Î'] = 145;
	cTable['Ï'] = 146;
	cTable['Ğ'] = 147;
	cTable['Ñ'] = 148;
	cTable['Ò'] = 149;
	cTable['Ó'] = 150;
	cTable['Ô'] = 151;
	cTable['Õ'] = 152;
	cTable['Ö'] = 153;
	cTable['×'] = 154;
	cTable['Ø'] = 155;
	cTable['Ù'] = 156;
	cTable['Ü'] = 157;
	cTable['Ú'] = 158;
	cTable['Û'] = 159;
	cTable['İ'] = 160;
	cTable['Ş'] = 161;
	cTable['ß'] = 162;

	cTable['à'] = 163;
	cTable['á'] = 164;
	cTable['â'] = 165;
	cTable['ã'] = 166;
	cTable['ä'] = 167;
	cTable['å'] = 168;
	cTable['¸'] = 169;
	cTable['æ'] = 170;
	cTable['ç'] = 171;
	cTable['è'] = 172;
	cTable['é'] = 173;
	cTable['ê'] = 174;
	cTable['ë'] = 175;
	cTable['ì'] = 176;
	cTable['í'] = 177;
	cTable['î'] = 178;
	cTable['ï'] = 179;
	cTable['ğ'] = 180;
	cTable['ñ'] = 181;
	cTable['ò'] = 182;
	cTable['ó'] = 183;
	cTable['ô'] = 184;
	cTable['õ'] = 185;
	cTable['ö'] = 186;
	cTable['÷'] = 187;
	cTable['ø'] = 188;
	cTable['ù'] = 189;
	cTable['ü'] = 190;
	cTable['ú'] = 191;
	cTable['û'] = 192;
	cTable['ı'] = 193;
	cTable['ş'] = 194;
	cTable['ÿ'] = 195;

	lTable['À'] = 20;
	lTable['Á'] = 16;
	lTable['Â'] = 16;
	lTable['Ã'] = 14;
	lTable['Ä'] = 22;
	lTable['Å'] = 15;
	lTable['¨'] = 15;
	lTable['Æ'] = 20;
	lTable['Ç'] = 12;
	lTable['È'] = 16;
	lTable['É'] = 16;
	lTable['Ê'] = 16;
	lTable['Ë'] = 22;
	lTable['Ì'] = 18;
	lTable['Í'] = 16;
	lTable['Î'] = 15;
	lTable['Ï'] = 14;
	lTable['Ğ'] = 13;
	lTable['Ñ'] = 15;
	lTable['Ò'] = 12;
	lTable['Ó'] = 14;
	lTable['Ô'] = 15;
	lTable['Õ'] = 13;
	lTable['Ö'] = 16;
	lTable['×'] = 14;
	lTable['Ø'] = 23;
	lTable['Ù'] = 23;
	lTable['Ü'] = 10;
	lTable['Ú'] = 12;	
	lTable['Û'] = 16;
	lTable['İ'] = 12;
	lTable['Ş'] = 20;
	lTable['ß'] = 15;

	lTable['à'] = 12;
	lTable['á'] = 10;
	lTable['â'] = 10;
	lTable['ã'] = 11;
	lTable['ä'] = 16;
	lTable['å'] = 10;
	lTable['¸'] = 11;
	lTable['æ'] = 13;
	lTable['ç'] = 12;
	lTable['è'] = 13;
	lTable['é'] = 13;
	lTable['ê'] = 12;
	lTable['ë'] = 13;
	lTable['ì'] = 14;
	lTable['í'] = 11;
	lTable['î'] = 11;
	lTable['ï'] = 11;
	lTable['ğ'] = 12;
	lTable['ñ'] = 10;
	lTable['ò'] = 10;
	lTable['ó'] = 10;
	lTable['ô'] = 11;
	lTable['õ'] = 10;
	lTable['ö'] = 11;
	lTable['÷'] = 10;
	lTable['ø'] = 15;
	lTable['ù'] = 15;
	lTable['ü'] = 10;
	lTable['ú'] = 12;
	lTable['û'] = 16;
	lTable['ı'] = 11;
	lTable['ş'] = 13;
	lTable['ÿ'] = 11;

#endif // FONT_RUSSIAN

#ifdef FONT_CZECH

	cTable['Ì'] = 196;
	cTable['Š'] = 197;
	cTable['È'] = 198;
	cTable['Ø'] = 199;
	cTable[''] = 200;
	cTable['İ'] = 201;
	cTable['Á'] = 202;
	cTable['Í'] = 203;
	cTable['É'] = 204;
	cTable['Ï'] = 205;
	cTable[''] = 206;
	cTable['Ò'] = 207;
	cTable['Ó'] = 208;
	cTable['Ù'] = 209;
	cTable['Ú'] = 210;

	cTable['ì'] = 211;
	cTable['š'] = 212;
	cTable['è'] = 213;
	cTable['ø'] = 214;
	cTable[''] = 215;
	cTable['ı'] = 216;
	cTable['á'] = 217;
	cTable['í'] = 218;
	cTable['é'] = 219;
	cTable['ï'] = 220;
	cTable[''] = 221;
	cTable['ò'] = 222;
	cTable['ó'] = 223;
	cTable['ù'] = 224;
	cTable['ú'] = 225;

	lTable['Ì'] = 15;
	lTable['Š'] = 15;
	lTable['È'] = 15;
	lTable['Ø'] = 15;
	lTable[''] = 15;
	lTable['İ'] = 14;
	lTable['Á'] = 20;
	lTable['Í'] = 7;
	lTable['É'] = 15;
	lTable['Ï'] = 20;
	lTable[''] = 19;
	lTable['Ò'] = 16;
	lTable['Ó'] = 15;
	lTable['Ù'] = 13;
	lTable['Ú'] = 13;

	lTable['ì'] = 11;
	lTable['š'] = 11;
	lTable['è'] = 11;
	lTable['ø'] = 11;
	lTable[''] = 11;
	lTable['ı'] = 10;
	lTable['á'] = 12;
	lTable['í'] = 6;
	lTable['é'] = 10;
	lTable['ï'] = 15;
	lTable[''] = 12;
	lTable['ò'] = 11;
	lTable['ó'] = 11;
	lTable['ù'] = 10;
	lTable['ú'] = 10;

#endif // FONT_CZECH

#ifdef FONT_FRENCH

	cTable['â'] = 226;
	cTable['é'] = 227;
	cTable['ê'] = 228;
	cTable['î'] = 229;
	cTable['ù'] = 230;
	cTable['û'] = 231;
	cTable['ô'] = 232;
	cTable['ö'] = 233;

	lTable['â'] = 12;
	lTable['é'] = 10;
	lTable['ê'] = 10;
	lTable['î'] = 6;
	lTable['ù'] = 10;
	lTable['û'] = 10;
	lTable['ô'] = 11;
	lTable['ö'] = 11;

#endif // FONT_FRENCH

#ifdef FONT_GERMAN
	cTable['ß'] = 234;
	// 'SS' = 235

	// old chars overrides
	cTable['Ä'] = cTable['ä'] = 55;
	cTable['Ö'] = cTable['ö'] = 67;
	cTable['Ü'] = cTable['ü'] = 71;

	lTable['ß'] = 11;

#endif // FONT_GERMAN
}



/***************************************************************************\
*       Metodi di RMFontObj
\****************************************************************************/

#define TOUPPER(a)	((a) >='a'&&(a)<='z'?(a)+'A'-'a':(a))
#define TOLOWER(a)	((a) >='A'&&(a)<='Z'?(a)+'a'-'A':(a))

void RMFontObj::SetBothCase(int nChar, int nNext, signed char spiazz) {
	l2Table[TOUPPER(nChar)][TOUPPER(nNext)] = spiazz;
	l2Table[TOUPPER(nChar)][TOLOWER(nNext)] = spiazz;
	l2Table[TOLOWER(nChar)][TOUPPER(nNext)] = spiazz;
	l2Table[TOLOWER(nChar)][TOLOWER(nNext)] = spiazz;
}


void RMFontObj::Init(void) {
	int i;

	//bernie: numero di caratteri nel font (solo maiuscolo)
	int nchars =
		  85	// base
		+  9	// polish
		+ 33	// russian
		+ 15	// czech
		+  0	// francais (no uppercase chars)
		+  1;	// deutsch


	Load(RES_F_OBJ, nchars, 25, 30);

	// Inizializziamo le tabelline del cazzo
	lDefault = 26;
	hDefault = 30;
	Common::fill(&l2Table[0][0], &l2Table[0][0] + (256 * 256), '\0');

	for (i = 0; i < 256; i++) {
		cTable[i] = -1;
		lTable[i] = lDefault;
	}

	for (i = 0; i < 26; i++) {
		cTable['A' + i] = i+0;
		cTable['a' + i] = i+0;
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
	// cTable['!'] = 43; Esclamativo alla rovescia
	cTable['?'] = 44;
	//cTable['?'] = 45;  Interrogativo alla rovescia
	cTable['/'] = 46;
	cTable['('] = 47;
	cTable[')'] = 48;
	cTable['='] = 49;
	cTable['\''] = 50;
	cTable['\"'] = 51;
	cTable['£'] = 52;
	cTable['$'] = 53;
	cTable['%'] = 54;
	cTable['&'] = 55;
	cTable['^'] = 56;
	cTable['*'] = 57;
	cTable['<'] = 58;
	cTable['>'] = 59;
	cTable['«'] = 60;
	cTable['»'] = 61;
	cTable['ø'] = 62;
	cTable['ç'] = 63;
	//cTable['ƒ'] = 64;   integrale
	cTable['Ñ'] = 65;
	cTable['®'] = 66;
	cTable['©'] = 67;
	cTable['Æ'] = 68;
	cTable['Á'] = 69;
	cTable['Ä'] = cTable['ä'] = 70;
	cTable['Å'] = 71;
	cTable['È'] = 72;
	cTable['Ë'] = 73;
	//cTable[' '] = 74;   e cerchietto
	cTable['Ì'] = 75;
	cTable['Ï'] = 76;
	//cTable[' '] = 77;	  i cerchietto
	cTable['Ò'] = 78;
	cTable['Ö'] = cTable['ö'] = 79;
	//cTable[' '] = 80;		o cerchietto
	cTable['Ù'] = 81;
	cTable['Ü'] = cTable['ü'] = 82;
	//cTable[' '] = 83;		u cerchietto
	//cTable[' '] = 84;   y dieresi

	/* Un po' di lunghezze */
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
	lTable['Ñ'] = 17;

	/* Casi particolari */
	SetBothCase('C','C',2);
	SetBothCase('A','T',-2);
	SetBothCase('R','S',2);
	SetBothCase('H','I',-2);
	SetBothCase('T','S',2);
	SetBothCase('O','R',2);
	SetBothCase('O','L',2);
	SetBothCase('O','G',2);
	SetBothCase('Z','A',-1);
	SetBothCase('R','R',1);
	SetBothCase('R','U',3);


	// Polish characters
	//¥¹ÆæÊê£³ÑñÓóŒœ¯¿Ÿ
	//AaCcEeLlNnOoSsZzZz
	cTable['¥'] = cTable['¹'] = 85;
	lTable['¥'] = lTable['¹'] = 20;

	cTable['Æ'] = cTable['æ'] = 86;
	lTable['Æ'] = lTable['æ'] = 22;
	
	cTable['Ê'] = cTable['ê'] = 87;
	lTable['Ê'] = lTable['ê'] = 17;
	
	cTable['£'] = cTable['³'] = 88;
	lTable['£'] = lTable['³'] = 19;

	cTable['Ñ'] = cTable['ñ'] = 89;
	lTable['Ñ'] = lTable['ñ'] = 17;

	cTable['Ó'] = cTable['ó'] = 90;
	lTable['Ó'] = lTable['ó'] = 22;

	cTable['Œ'] = cTable['œ'] = 91;
	lTable['Œ'] = lTable['œ'] = 15;

	cTable['¯'] = cTable['¿'] = 92;
	lTable['¯'] = lTable['¿'] = 21;

	cTable[''] = cTable['Ÿ'] = 93;
	lTable[''] = lTable['Ÿ'] = 21;


#ifdef FONT_RUSSIAN
	// Russian Characters
	// WARNING: Il russo usa molti dei caratteri ISO-Latin-1 che servono
	// per le altre traduzioni. Per compilare Tony in altre lingue,
	// commentare via queste definizioni.

	cTable['¥'] = cTable['¹'] = 85;
	lTable['¥'] = lTable['¹'] = 20;

	cTable['À'] = cTable['à'] =94;
	cTable['Á'] = cTable['á'] =95;
	cTable['Â'] = cTable['â'] =96;
	cTable['Ã'] = cTable['ã'] =97;
	cTable['Ä'] = cTable['ä'] =98;
	cTable['Å'] = cTable['å'] =99;
	cTable['¨'] = cTable['¸'] =100;
	cTable['Æ'] = cTable['æ'] =101;
	cTable['Ç'] = cTable['ç'] =102;
	cTable['È'] = cTable['è'] =103;
	cTable['É'] = cTable['é'] =104;
	cTable['Ê'] = cTable['ê'] =105;
	cTable['Ë'] = cTable['ë'] =106;
	cTable['Ì'] = cTable['ì'] =107;
	cTable['Í'] = cTable['í'] =108;
	cTable['Î'] = cTable['î'] =109;
	cTable['Ï'] = cTable['ï'] =110;
	cTable['Ğ'] = cTable['ğ'] =111;
	cTable['Ñ'] = cTable['ñ'] =112;
	cTable['Ò'] = cTable['ò'] =113;
	cTable['Ó'] = cTable['ó'] =114;
	cTable['Ô'] = cTable['ô'] =115;
	cTable['Õ'] = cTable['õ'] =116;
	cTable['Ö'] = cTable['ö'] =117;
	cTable['×'] = cTable['÷'] =118;
	cTable['Ø'] = cTable['ø'] =119;
	cTable['Ù'] = cTable['ù'] =120;
	cTable['Ü'] = cTable['ü'] =121;
	cTable['Ú'] = cTable['ú'] =122;
	cTable['Û'] = cTable['û'] =123;
	cTable['İ'] = cTable['ı'] =124;
	cTable['Ş'] = cTable['ş'] =125;
	cTable['ß'] = cTable['ÿ'] =126;


	lTable['À'] = lTable['à'] = 18;
	lTable['Á'] = lTable['á'] = 18;
	lTable['Â'] = lTable['â'] = 18;
	lTable['Ã'] = lTable['ã'] = 17;
	lTable['Ä'] = lTable['ä'] = 16;
	lTable['Å'] = lTable['å'] = 18;
	lTable['¨'] = lTable['¸'] = 18;
	lTable['Æ'] = lTable['æ'] = 20;
	lTable['Ç'] = lTable['ç'] = 18;
	lTable['È'] = lTable['è'] = 18;
	lTable['É'] = lTable['é'] = 18;
	lTable['Ê'] = lTable['ê'] = 18;
	lTable['Ë'] = lTable['ë'] = 16;
	lTable['Ì'] = lTable['ì'] = 18;
	lTable['Í'] = lTable['í'] = 15;
	lTable['Î'] = lTable['î'] = 22;
	lTable['Ï'] = lTable['ï'] = 15;
	lTable['Ğ'] = lTable['ğ'] = 18;
	lTable['Ñ'] = lTable['ñ'] = 22;
	lTable['Ò'] = lTable['ò'] = 19;
	lTable['Ó'] = lTable['ó'] = 16;
	lTable['Ô'] = lTable['ô'] = 21;
	lTable['Õ'] = lTable['õ'] = 20;
	lTable['Ö'] = lTable['ö'] = 16;
	lTable['×'] = lTable['÷'] = 16;
	lTable['Ø'] = lTable['ø'] = 19;
	lTable['Ù'] = lTable['ù'] = 22;
	lTable['Ü'] = lTable['ü'] = 18;
	lTable['Ú'] = lTable['ú'] = 19;	
	lTable['Û'] = lTable['û'] = 19;
	lTable['İ'] = lTable['ı'] = 15;
	lTable['Ş'] = lTable['ş'] = 18;
	lTable['ß'] = lTable['ÿ'] = 18;

#endif // FONT_RUSSIAN

#ifdef FONT_CZECH
	// rep. ceca characters	

	cTable['Ì'] = cTable['ì'] = 127;
	cTable['Š'] = cTable['š'] = 128;
	cTable['È'] = cTable['è'] = 129;
	cTable['Ø'] = cTable['ø'] = 130;
	cTable[''] = cTable[''] = 131;
	cTable['İ'] = cTable['ı'] = 132;
	cTable['Á'] = cTable['á'] = 133;
	cTable['Í'] = cTable['í'] = 134;
	cTable['É'] = cTable['é'] = 135;
	cTable['Ï'] = cTable['ï'] = 136;
	cTable[''] = cTable[''] = 137;
	cTable['Ò'] = cTable['ò'] = 138;
	cTable['Ó'] = cTable['ó'] = 139;
	cTable['Ù'] = cTable['ù'] = 140;
	cTable['Ú'] = cTable['ú'] = 141;

	lTable['Ì'] = lTable['ì'] =17;
	lTable['Š'] = lTable['š'] =15;
	lTable['È'] = lTable['è'] =22;
	lTable['Ø'] = lTable['ø'] =18;
	lTable[''] = lTable[''] =21;
	lTable['İ'] = lTable['ı'] =16;
	lTable['Á'] = lTable['á'] =18;
	lTable['Í'] = lTable['í'] =19;
	lTable['É'] = lTable['é'] =17;
	lTable['Ï'] = lTable['ï'] =23;
	lTable[''] = lTable[''] =24;
	lTable['Ò'] = lTable['ò'] =17;
	lTable['Ó'] = lTable['ó'] =22;
	lTable['Ù'] = lTable['ù'] =16;
	lTable['Ú'] = lTable['ú'] =16;

#endif // FONT_CZECH

#ifdef FONT_FRENCH

	// traduci le lettere accentate in lettere normali

	cTable['à'] = cTable['á'] = cTable['â'] = 0; // a
	lTable['à'] = lTable['á'] = lTable['â'] = 17;

	cTable['é'] = cTable['è'] = 4; // e
	lTable['é'] = lTable['è'] = 15;

	cTable['ì'] = cTable['í'] = cTable['î'] = 8; // i
	lTable['ì'] = lTable['í'] = lTable['î'] = 16;

	cTable['ò'] = cTable['ó'] = cTable['ô'] = cTable['ö'] = 14; // o
	lTable['ò'] = lTable['ó'] = lTable['ô'] = lTable['ö'] = 19;

	cTable['ù'] = cTable['û'] = 20; // u
	lTable['ù'] = lTable['û'] = 15;

#endif // FONT_FRENCH

#ifdef FONT_GERMAN
	cTable['ß'] = 142;
	// SS = 143

	lTable['ß'] = 24;
#endif // FONT_GERMAN
}


/****************************************************************************\
*       Metodi di RMText
\****************************************************************************/

RMFontColor *RMText::m_fonts[4] = { NULL, NULL, NULL, NULL };
OSystem::MutexRef RMText::m_cs;
RMGfxClearTask RMText::m_clear;

RMText::RMText() {
	// Colore di default: bianco
	m_r = m_g = m_b = 255;

	// Lunghezza di default
	maxLineLength = 350;

	m_bTrasp0 = true;
	aHorType = HCENTER;
	aVerType = VTOP;
	SetPriority(150);
}

RMText::~RMText() {
	
}

void RMText::SetMaxLineLength(int max) {
	maxLineLength = max;
}

bool RMText::RemoveThis() {
 // Qui possiamo fare i controlli sul numero di frame, sul tempo trascorso
 // etc.
	return true;
}


void RMText::WriteText(RMString text, int nFont, int *time) {
	// Inizializza i font (una volta sola)	
	if (m_fonts[0] == NULL) {
		m_fonts[0] = new RMFontParla; m_fonts[0]->Init();
		m_fonts[1] = new RMFontObj;   m_fonts[1]->Init();
		m_fonts[2] = new RMFontMacc;  m_fonts[2]->Init();
		m_fonts[3] = new RMFontCredits;  m_fonts[3]->Init();

		m_cs = g_system->createMutex();
	}

	g_system->lockMutex(m_cs);
	WriteText(text,m_fonts[nFont],time);
	g_system->unlockMutex(m_cs);
}


void RMText::WriteText(RMString text, RMFontColor *font, int *time) {
	RMGfxPrimitive *prim;
	char *p, *old_p;
	int i, j, x, y;
	int len;
	int numchar;
	int width, height;
	char *string;
	int numlines;

	// Setta il colore di base
	font->SetBaseColor(m_r, m_g, m_b);

	// Si autodistrugge il buffer prima di iniziare
	Destroy();

	// Se la stringa è vuota, non fare nulla	
	if (text == NULL || text[0] == '\0')
		return;
	
	// Divide la frase in linee. In questo ciclo, X contiene la lunghezza massima raggiunta da una linea
	// e I il numero delle linee
	string=p = text;
	i = j = x = 0;
	while (*p != '\0') {
		j += font->StringLen(*p);
		if (j > (((aHorType == HLEFTPAR) && (i > 0)) ? maxLineLength - 25 : maxLineLength)) {
			j -= font->StringLen(*p, p[1]);
			if (j > x) x = j;

			// Torna indietro al primo spazio utile
			//
			// BERNIE: nella versione originale le frasi contenenti
			// parole che superano la larghezza di una riga causavano
			// l'interruzione dell'intera frase.
			// Questo workaround e' parziale: la parola troppo lunga
			// viene spezzata bruscamente e si perde una lettera.
			// Oltre allo spazio e' ammesso il wrap sul carattere '-'.
			//
			old_p = p;
			while (*p != ' ' && *p != '-' && p > string) p--;

			if (p == string)
				p = old_p;

			// Controlla se sono tutti spazi fino alla fine
			while (*p == ' ' && *p != '\0') p++;
			if (*p == '\0')
				break;
			p--;
			i++;
			*p = '\0';
			j = 0;
		}
		p++;
	}

	if (j > x) x = j;
 
	i++;
	numlines = i;

	// X=Lunghezza della linea piu' lunga. Controlla se puo' essere puttata a X1
	//x+=font->StringLen(-1)+1;          // Meglio esagerare per sicurezza
	x += 8;

	// Posizione di partenza per la surface: X1,Y
	width = x;
	height = (numlines - 1) * font->LetterHeight() + font->m_fontDimy;

	// Crea la surface
	Create(width, height);
	//AddPrim(new RMGfxPrimitive(&m_clear));
	Common::fill(m_buf, m_buf + width * height * 2, 0);

	p = string;

	y = 0;
	numchar = 0;
	for (; i > 0; i--) {
		// Misura la lunghezza della linea
		x = 0;
		j = font->StringLen(RMString(p));

		switch (aHorType) {
		case HLEFT:
			x = 0;
			break;

		case HLEFTPAR:
			if (i == numlines)
				x=0;
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
				x += font->StringLen(*p);
				p++;
				continue;
			}

			prim = font->MakeLetterPrimitive(*p, len);
			prim->Dst().x1 = x;
			prim->Dst().y1 = y;
			AddPrim(prim);

			numchar++;

			x += font->StringLen(*p, p[1]);
			p++;
		}
		p++;
		y += font->LetterHeight();
	}

	if (time != NULL)
		*time = 1000 + numchar * (11 - nCfgTextSpeed) * 14;
}

void RMText::ClipOnScreen(RMGfxPrimitive *prim) {
	// Cerca di non farlo uscire dallo schermo
	if (prim->Dst().x1 < 5) prim->Dst().x1 = 5;
	if (prim->Dst().y1 < 5) prim->Dst().y1 = 5;
	if (prim->Dst().x1+m_dimx > 635) prim->Dst().x1 = 635 - m_dimx;
	if (prim->Dst().y1+m_dimy > 475) prim->Dst().y1 = 475 - m_dimy;
}

void RMText::Draw(RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim) {
	// Allinea orizzontalmente
	if (aHorType == HCENTER)
		prim->Dst().TopLeft() -= RMPoint(m_dimx / 2, 0);
	else if (aHorType == HRIGHT)
		prim->Dst().TopLeft() -= RMPoint(m_dimx, 0);


	// Alinea verticalemente
	switch (aVerType) {
	case VTOP:
		break;

	case VCENTER:
		prim->Dst().y1-=m_dimy/2;
		break;

	case VBOTTOM:
		prim->Dst().y1-=m_dimy;
		break;
	}

	ClipOnScreen(prim);

	RMGfxWoodyBuffer::Draw(bigBuf,prim);
}

/****************************************************************************\
*       Metodi di RMTextDialog
\****************************************************************************/

RMTextDialog::RMTextDialog() : RMText() {
	m_startTime = 0;
	dst = RMPoint(0,0);

	m_bSkipStatus = true;
	m_bShowed = true;
	m_bForceTime = false;
	m_bForceNoTime = false;
	m_bAlwaysDisplay = false;
	m_bNoTab = false;
	hCustomSkip = INVALID_HANDLE_VALUE;
	hCustomSkip2 = INVALID_HANDLE_VALUE;
	m_input = NULL;

	// Crea l'evento di fine displaying
	hEndDisplay = CreateEvent(NULL, false, false, NULL);
}

RMTextDialog::~RMTextDialog() {
	CloseHandle(hEndDisplay);
}

void RMTextDialog::Show(void) {
	m_bShowed = true;
}

void RMTextDialog::Hide(void) {
	m_bShowed = false;
}

void RMTextDialog::WriteText(RMString text, int font, int *time) {
	RMText::WriteText(text,font,&m_time);

	if (time != NULL)
		*time = m_time;
}

void RMTextDialog::WriteText(RMString text, RMFontColor *font, int *time) {
	RMText::WriteText(text,font,&m_time);

	if (time != NULL)
		*time = m_time;
}


void RMTextDialog::SetSkipStatus(bool bEnabled) {
	m_bSkipStatus=bEnabled;
}

void RMTextDialog::ForceTime(void) {
	m_bForceTime = true;
}

void RMTextDialog::ForceNoTime(void) {
	m_bForceNoTime = true;
}

void RMTextDialog::SetNoTab(void) {
	m_bNoTab = true;
}

void RMTextDialog::SetForcedTime(uint32 dwTime) {
	m_time = dwTime;	
}

void RMTextDialog::SetAlwaysDisplay(void) {
	m_bAlwaysDisplay = true;
}

bool RMTextDialog::RemoveThis(void) {
	// Frase NON di background
	if (m_bSkipStatus) {
		if (!(bCfgDubbing && hCustomSkip2 != INVALID_HANDLE_VALUE))
			if (bCfgTimerizedText) {
				if (!m_bForceNoTime)
					if (_vm->GetTime() > (uint32)m_time + m_startTime)
						return true;
			}

		if (!m_bNoTab)
			if ((GetAsyncKeyState(Common::KEYCODE_TAB) & 0x8001) == 0x8001)
				return true;

		if (!m_bNoTab)
			if (m_input)
				if (m_input->MouseLeftClicked() || m_input->MouseRightClicked())
					return true;
	}
	// Frase di background
	else {
		if (!(bCfgDubbing && hCustomSkip2 != INVALID_HANDLE_VALUE))
			if (!m_bForceNoTime)
				if (_vm->GetTime() > (uint32)m_time + m_startTime)
					return true;
	}

	// Se il tempo è forzato
	if (m_bForceTime)
		if (_vm->GetTime() > (uint32)m_time + m_startTime)
			return true;

	if (hCustomSkip != INVALID_HANDLE_VALUE)
		if (WaitForSingleObject(hCustomSkip, 0) == WAIT_OBJECT_0)
			return true;

	if (bCfgDubbing && hCustomSkip2 != INVALID_HANDLE_VALUE)
		if (WaitForSingleObject(hCustomSkip2,0) == WAIT_OBJECT_0)
			return true;

	return false;
}

void RMTextDialog::Unregister(void) {
	RMGfxTask::Unregister();
	assert(m_nInList == 0);
	SetEvent(hEndDisplay);
}

void RMTextDialog::Draw(RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim) {
	if (m_startTime == 0)
		m_startTime = _vm->GetTime();
	
	if (m_bShowed) {
		if (bCfgSottotitoli || m_bAlwaysDisplay) {
			prim->Dst().TopLeft() = dst;
			RMText::Draw(bigBuf, prim);
		}
	}
}

void RMTextDialog::SetCustomSkipHandle(HANDLE hCustom) {
	hCustomSkip = hCustom;
}

void RMTextDialog::SetCustomSkipHandle2(HANDLE hCustom) {
	hCustomSkip2 = hCustom;
}

void RMTextDialog::WaitForEndDisplay(void) {
	WaitForSingleObject(hEndDisplay, INFINITE);
}

void RMTextDialog::SetInput(RMInput *input) {
	m_input = input;
}

/****************************************************************************\
*       Metodi di RMTextDialogScrolling
\****************************************************************************/

RMTextDialogScrolling::RMTextDialogScrolling() {
	curLoc = NULL;
}

RMTextDialogScrolling::RMTextDialogScrolling(RMLocation *loc) {
	curLoc = loc;
	startScroll = loc->ScrollPosition();
}

RMTextDialogScrolling::~RMTextDialogScrolling() {
}

void RMTextDialogScrolling::Draw(RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim) {
	RMPoint curDst;

	curDst = dst;

	if (curLoc != NULL)
		dst -= curLoc->ScrollPosition() - startScroll;

	RMTextDialog::Draw(bigBuf, prim);

	dst = curDst;
}

void RMTextDialogScrolling::ClipOnScreen(RMGfxPrimitive *prim) {
	// Non dobbiamo fare nulla!
}


/****************************************************************************\
*       Metodi di RMTextItemName
\****************************************************************************/

RMTextItemName::RMTextItemName() : RMText() {
	m_item = NULL;
	SetPriority(220);
}

RMTextItemName::~RMTextItemName() {

}

void RMTextItemName::DoFrame(RMGfxTargetBuffer& bigBuf, RMLocation &loc, RMPointer &ptr, RMInventory &inv) {
	RMString itemName;
	RMItem *lastItem = m_item;

	// Si aggiunge alla lista se c'e' bisogno
	if (!m_nInList)
		bigBuf.AddPrim(new RMGfxPrimitive(this));
	
	// Aggiorna le coordinate di scrolling
	m_curscroll = loc.ScrollPosition();

	// Controlla se siamo sopra l'inventario
	if (inv.ItemInFocus(m_mpos))
		m_item = inv.WhichItemIsIn(m_mpos);
	else
		m_item = loc.WhichItemIsIn(m_mpos);
	
	itemName = "";

	// Si fa dare il nuovo nome
	if (m_item != NULL)
		m_item->GetName(itemName);

	// Se lo scrive
	WriteText(itemName, 1);

	// Se e' diverso dal precedente, e' il caso di aggiornare anche il puntatore con la WhichPointer
	if (lastItem != m_item) {
		if (m_item == NULL)
			ptr.SetSpecialPointer(RMPointer::PTR_NONE);
		else {
			HANDLE hThread = mpalQueryDoAction(20, m_item->MpalCode(), 0);		
			if (hThread == INVALID_HANDLE_VALUE)
				ptr.SetSpecialPointer(RMPointer::PTR_NONE);
			else
				WaitForSingleObject(hThread,INFINITE);
		}
	}
}


void RMTextItemName::Draw(RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim) {
	// Se non c'e' testo, e' inutile continuare
	if (m_buf == NULL)
		return;

	// Setta come coordinate destinazione quelle del mouse
	prim->Dst().TopLeft() = m_mpos-RMPoint(0, 30);

	RMText::Draw(bigBuf,prim);
}

RMPoint RMTextItemName::GetHotspot() { 
	if (m_item == NULL) 
		return m_mpos + m_curscroll; 
	else 
		return m_item->Hotspot();  
}

RMItem *RMTextItemName::GetSelectedItem() { 
	return m_item; 
}

bool RMTextItemName::IsItemSelected() { 
	return m_item != NULL; 
}

bool RMTextItemName::IsNormalItemSelected() { 
	return m_item != NULL && m_itemName.Length() > 0; 
}


/****************************************************************************\
*       Metodi di RMDialogChoice
\****************************************************************************/

RMDialogChoice::RMDialogChoice() {
	RMResRaw dlg1(RES_I_DLGTEXT);
	RMResRaw dlg2(RES_I_DLGTEXTLINE);
	RMRes dlgpal(RES_I_DLGTEXTPAL);
	
	DlgText.Init(dlg1, dlg1.Width(), dlg1.Height());
	DlgTextLine.Init(dlg2, dlg2.Width(), dlg2.Height());

	DlgText.LoadPaletteWA(dlgpal);
	DlgTextLine.LoadPaletteWA(dlgpal);
	
	hUnreg=CreateEvent(NULL, false, false, NULL);
	bRemoveFromOT = false;
}

RMDialogChoice::~RMDialogChoice() {
	CloseHandle(hUnreg);
}

void RMDialogChoice::Unregister(void) {
	RMGfxWoodyBuffer::Unregister();
	assert(!m_nInList);
	PulseEvent(hUnreg);

	bRemoveFromOT = false;
}

void RMDialogChoice::Init(void)
{
	m_numChoices = 0;
	m_drawedStrings = NULL;
	m_ptDrawStrings = NULL;
	m_curSelection = -1;

	Create(640, 477);
	SetPriority(140);
}


void RMDialogChoice::Close(void) {
	if (m_drawedStrings != NULL) {
		delete[] m_drawedStrings;
		m_drawedStrings = NULL;
	}

	if (m_ptDrawStrings != NULL) {
		delete[] m_ptDrawStrings;
		m_ptDrawStrings = NULL;
	}

	Destroy();
}

void RMDialogChoice::SetNumChoices(int num) {
	int i;

	m_numChoices = num;
	m_curAdded = 0;
	
	// Alloca lo spazio per le stringhe disegnate
	m_drawedStrings = new RMText[num];
	m_ptDrawStrings = new RMPoint[num];

	// Le inizializza
	for (i = 0; i < m_numChoices; i++) {
		m_drawedStrings[i].SetColor(0, 255, 0);
		m_drawedStrings[i].SetAlignType(RMText::HLEFTPAR, RMText::VTOP);
		m_drawedStrings[i].SetMaxLineLength(600);
		m_drawedStrings[i].SetPriority(10);
	}
}

void RMDialogChoice::AddChoice(RMString string) {
	// Si disegna la stringa
	assert(m_curAdded < m_numChoices);
	m_drawedStrings[m_curAdded++].WriteText(string,0);	
}

void RMDialogChoice::Prepare(void) {
	int i;
	RMPoint ptPos;

	AddPrim(new RMGfxPrimitive(&DlgText,RMPoint(0,0)));
	AddPrim(new RMGfxPrimitive(&DlgTextLine,RMPoint(0,155)));
	AddPrim(new RMGfxPrimitive(&DlgTextLine,RMPoint(0,155+83)));
	AddPrim(new RMGfxPrimitive(&DlgTextLine,RMPoint(0,155+83+83)));
	AddPrim(new RMGfxPrimitive(&DlgTextLine,RMPoint(0,155+83+83+83)));

	ptPos.Set(20,90);

	for (i = 0; i < m_numChoices; i++) {
		AddPrim(new RMGfxPrimitive(&m_drawedStrings[i], ptPos));
		m_ptDrawStrings[i] = ptPos;
		ptPos.Offset(0,m_drawedStrings[i].Dimy() + 15);
	}

	DrawOT();
	ClearOT();

	m_ptDrawPos.Set(0,480-ptPos.y);
}

void RMDialogChoice::SetSelected(int pos) {
	//uint16 * buf = (uint16 *)m_buf;
	RMGfxBox box;
	RMRect rc;

	if (pos == m_curSelection)
		return;

	box.SetPriority(5);

	if (m_curSelection != -1) {
		box.SetColor(0xCC, 0xCC, 0xFF);
		rc.TopLeft()=RMPoint(18, m_ptDrawStrings[m_curSelection].y); 
		rc.BottomRight() = rc.TopLeft() + RMPoint(597, m_drawedStrings[m_curSelection].Dimy());
		AddPrim(new RMGfxPrimitive(&box, rc));

		AddPrim(new RMGfxPrimitive(&m_drawedStrings[m_curSelection], m_ptDrawStrings[m_curSelection]));
		DrawOT();
		ClearOT();
	}

	if (pos != -1) {
		box.SetColor(100, 100, 100);
		rc.TopLeft()=RMPoint(18, m_ptDrawStrings[pos].y); 
		rc.BottomRight() = rc.TopLeft()+RMPoint(597, m_drawedStrings[pos].Dimy());
		AddPrim(new RMGfxPrimitive(&box, rc));
		AddPrim(new RMGfxPrimitive(&m_drawedStrings[pos], m_ptDrawStrings[pos]));
	}

	DrawOT();
	ClearOT();

	m_curSelection = pos;
}

void RMDialogChoice::Show(RMGfxTargetBuffer *bigBuf) {
	Prepare();
	m_bShow = false;

	if (!m_nInList && bigBuf != NULL)
		bigBuf->AddPrim(new RMGfxPrimitive(this));

	if (0) {
		m_bShow = true;
	} else {
		RMPoint destpt;
		int deltay;
		int starttime = _vm->GetTime();
		int elaps;

		deltay=480 - m_ptDrawPos.y;
		destpt = m_ptDrawPos;
		m_ptDrawPos.Set(0, 480);

  	if (!m_nInList && bigBuf != NULL)
	  	bigBuf->AddPrim(new RMGfxPrimitive(this));
		m_bShow = true;

		elaps = 0;
		while (elaps < 700) {
			MainWaitFrame();
			MainFreeze();
			elaps = _vm->GetTime() - starttime;
			m_ptDrawPos.y = 480 - ((deltay * 100) / 700 * elaps) / 100;
			MainUnfreeze();
		}

		m_ptDrawPos.y = destpt.y;
	}
}

void RMDialogChoice::Draw(RMGfxTargetBuffer &bigBuf, RMGfxPrimitive *prim) {
	if (m_bShow == false)
		return;

	prim->SetDst(m_ptDrawPos);
	RMGfxSourceBuffer16::Draw(bigBuf, prim);
}


void RMDialogChoice::Hide(void) {
	if (1) {
		int deltay;
		int starttime = _vm->GetTime();
		int elaps;

		deltay=480 - m_ptDrawPos.y;
		elaps = 0;
		while (elaps < 700) {
			MainWaitFrame();
			MainFreeze();
			elaps=_vm->GetTime()-starttime;
			m_ptDrawPos.y=480-((deltay*100)/700*(700-elaps))/100;
			MainUnfreeze();
		}
	}

	m_bShow = false;
	bRemoveFromOT = true;
	WaitForSingleObject(hUnreg, INFINITE);
}


bool RMDialogChoice::RemoveThis(void) {
	return bRemoveFromOT;
}

void RMDialogChoice::DoFrame(RMPoint ptMousePos) {
	int i;
	
	if (ptMousePos.y > m_ptDrawPos.y) {		
		for (i = 0; i < m_numChoices; i++) {
			if ((ptMousePos.y >= m_ptDrawPos.y+m_ptDrawStrings[i].y) && (ptMousePos.y < m_ptDrawPos.y+m_ptDrawStrings[i].y+m_drawedStrings[i].Dimy())) {
				SetSelected(i);
				break;
			}
		}

		if (i == m_numChoices)
			SetSelected(-1);
	}
}

int RMDialogChoice::GetSelection(void) {
	return m_curSelection;
}

} // End of namespace Tony
