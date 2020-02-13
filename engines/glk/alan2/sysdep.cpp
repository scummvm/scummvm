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

#include "glk/alan2/alan2.h"
#include "glk/alan2/sysdep.h"

namespace Glk {
namespace Alan2 {

void syserr(char str[]);

void fprintf(Common::WriteStream *ws, const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	Common::String s = Common::String::vformat(fmt, args);
	va_end(args);

	ws->write(s.c_str(), s.size());
}

/* Note to Glk maintainers: 'native' characters are used for output, in this
   case, Glk's Latin-1.  ISO characters are Alan's internal representation,
   stored in the .DAT file, and must be converted to native before printing.
   Glk could just use the ISO routines directly, but its safer to maintain
   its own tables to guard against future changes in either Alan or Glk (ie. a
   move to Unicode).
 */

static char spcChrs[] = {
	'\x0A', /* linefeed */
	'\x20', /* space */
	'\xA0', /* non-breaking space */
	'\x00'
};

static char lowChrs[] = {
	'\x61', /* a */  '\x62', /* b */  '\x63', /* c */  '\x64', /* d */
	'\x65', /* e */  '\x66', /* f */  '\x67', /* g */  '\x68', /* h */
	'\x69', /* i */  '\x6A', /* j */  '\x6B', /* k */  '\x6C', /* l */
	'\x6D', /* m */  '\x6E', /* n */  '\x6F', /* o */  '\x70', /* p */
	'\x71', /* q */  '\x72', /* r */  '\x73', /* s */  '\x74', /* t */
	'\x75', /* u */  '\x76', /* v */  '\x77', /* w */  '\x78', /* x */
	'\x79', /* y */  '\x7A', /* z */  '\xDF', /* ss <small sharp s> */
	'\xE0', /* a grave */             '\xE1', /* a acute */
	'\xE2', /* a circumflex */        '\xE3', /* a tilde */
	'\xE4', /* a diaeresis */         '\xE5', /* a ring */
	'\xE6', /* ae */                  '\xE7', /* c cedilla */
	'\xE8', /* e grave */             '\xE9', /* e acute */
	'\xEA', /* e circumflex */        '\xEB', /* e diaeresis */
	'\xEC', /* i grave */             '\xED', /* i acute */
	'\xEE', /* i circumflex */        '\xEF', /* i diaeresis */
	'\xF0', /* <small eth> */         '\xF1', /* n tilde */
	'\xF2', /* o grave */             '\xF3', /* o acute */
	'\xF4', /* o circumflex */        '\xF5', /* o tilde */
	'\xF6', /* o diaeresis */         '\xF8', /* o slash */
	'\xF9', /* u grave */             '\xFA', /* u acute */
	'\xFB', /* u circumflex */        '\xFC', /* u diaeresis */
	'\xFD', /* y acute */             '\xFE', /* <small thorn> */
	'\xFF', /* y diaeresis */         '\x00'
};

/* FIXME: ss <small sharp s> and y diaeresis have no UC analogues
   Are they really considered LC?
 */

static char uppChrs[] = {
	'\x41', /* A */  '\x42', /* B */  '\x43', /* C */  '\x44', /* D */
	'\x45', /* E */  '\x46', /* F */  '\x47', /* G */  '\x48', /* H */
	'\x49', /* I */  '\x4A', /* J */  '\x4B', /* K */  '\x4C', /* L */
	'\x4D', /* M */  '\x4E', /* N */  '\x4F', /* O */  '\x50', /* P */
	'\x51', /* Q */  '\x52', /* R */  '\x53', /* S */  '\x54', /* T */
	'\x55', /* U */  '\x56', /* V */  '\x57', /* W */  '\x58', /* X */
	'\x59', /* Y */  '\x5A', /* Z */
	'\xC0', /* A grave */           '\xC1', /* A acute */
	'\xC2', /* A circumflex */      '\xC3', /* A tilde */
	'\xC4', /* A diaeresis */       '\xC5', /* A ring */
	'\xC6', /* AE */                '\xC7', /* C cedilla */
	'\xC8', /* E grave */           '\xC9', /* E acute */
	'\xCA', /* E circumflex */      '\xCB', /* E diaeresis */
	'\xCC', /* I grave */           '\xCD', /* I acute */
	'\xCE', /* I circumflex */      '\xCF', /* I diaeresis */
	'\xD0', /* <capital eth> */     '\xD1', /* N tilde */
	'\xD2', /* O grave */           '\xD3', /* O acute */
	'\xD4', /* O circumflex */      '\xD5', /* O tilde */
	'\xD6', /* O diaeresis */       '\xD8', /* O slash */
	'\xD9', /* U grave */           '\xDA', /* U acute */
	'\xDB', /* U circumflex */      '\xDC', /* U diaeresis */
	'\xDD', /* Y acute */           '\xDE', /* <capital thorn> */
	'\x00'
};


int isSpace(int c) {            /* IN - Native character to test */
	return (c != '\0' && strchr(spcChrs, c) != 0);
}


int isLower(int c) {            /* IN - Native character to test */
	return (c != '\0' && strchr(lowChrs, c) != 0);
}


int isUpper(int c) {            /* IN - Native character to test */
	return (c != '\0' && strchr(uppChrs, c) != 0);
}

int isLetter(int c) {           /* IN - Native character to test */
	return (c != '\0' && (isLower(c) ? !0 : isUpper(c)));
}


int toLower(int c) {            /* IN - Native character to convert */
	return g_vm->glk_char_to_lower(c);
}

int toUpper(int c) {            /* IN - Native character to convert */
	return g_vm->glk_char_to_upper(c);
}

char *strlow(char str[]) {      /* INOUT - Native string to convert */
	char *s;

	for (s = str; *s; s++)
		*s = toLower(*s);
	return (str);
}


char *strupp(char str[]) {      /* INOUT - Native string to convert */
	char *s;

	for (s = str; *s; s++)
		*s = toUpper(*s);
	return (str);
}


/* The following work on ISO characters */

int isLowerCase(int c) {        /* IN - ISO character to test */
	static char lowChars[] = "abcdefghijklmnopqrstuvwxyz\340\341\342\343\344\345\346\347\351\352\353\354\355\356\357\360\361\362\363\364\365\366\370\371\372\373\374\375\376\377";
	return (c != '\0' && strchr(lowChars, c) != 0);
}


int isUpperCase(int c) {        /* IN - ISO character to test */
	static char upperChars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ\300\301\302\303\304\305\306\307\310\311\312\313\314\315\316\317\320\321\322\323\324\325\326\327\330\331\332\333\334\335\336\337";
	return (c != '\0' && strchr(upperChars, c) != 0);
}


int isISOLetter(int c) {        /* IN - ISO character to test */
	return (isLowerCase(c) || isUpperCase(c));
}


char toLowerCase(int c) {       /* IN - ISO character to convert */
	return (isUpperCase(c) ? c + ('a' - 'A') : c);
}


char toUpperCase(int c) {       /* IN - ISO character to convert */
	return (isLowerCase(c) ? c - ('a' - 'A') : c);
}


char *stringLower(char str[]) { /* INOUT - ISO string to convert */
	char *s;

	for (s = str; *s; s++)
		*s = toLowerCase(*s);
	return (str);
}


char *stringUpper(char str[]) { /* INOUT - ISO string to convert */
	char *s;

	for (s = str; *s; s++)
		*s = toUpperCase(*s);
	return (str);
}


/*----------------------------------------------------------------------
  toIso

  Converts the incoming string to ISO character set. The original is
  in the current character set which in the case of the compiler might
  be other than the native.

  */
void toIso(char copy[], /* OUT - Mapped  string */
           char original[], /* IN - string to convert */
           int charset) {   /* IN - the current character set */
	static unsigned char macMap[256]
	= {
		0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0A, 0x0E, 0x0F,
		0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
		0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
		0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
		0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
		0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F,
		0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
		0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F,
		0xC4, 0xC5, 0xC7, 0xC9, 0xD1, 0xD6, 0xDC, 0xE1, 0xE0, 0xE2, 0xE4, 0xE3, 0xE5, 0xE7, 0xE9, 0xE8,
		0xEA, 0xEB, 0xED, 0xEC, 0xEE, 0xEF, 0xF1, 0xF3, 0xF2, 0xF4, 0xF6, 0xF5, 0xFA, 0xF9, 0xFB, 0xFC,
		0xB9, 0xB0, 0xA2, 0xA3, 0xA7, 0xB7, 0xB6, 0xDF, 0xAE, 0xA9, 0xB2, 0xB4, 0xA8, 0xD7, 0xC6, 0xD8,
		0xA4, 0xB1, 0xCD, 0xCC, 0xA5, 0xB5, 0xF0, 0xCA, 0xDE, 0xFE, 0xA6, 0xAA, 0xBA, 0xD4, 0xE6, 0xF8,
		0xBF, 0xA1, 0xAC, 0xCE, 0xCF, 0xC8, 0xD0, 0xAB, 0xBB, 0xCB, 0xA0, 0xC0, 0xC3, 0xD5, 0xDD, 0xFD,
		0xAD, 0xAF, 0xDA, 0xD9, 0xB8, 0xB3, 0xF7, 0xC2, 0xFF, 0xBC, 0xBD, 0xBE, 0xC1, 0xD2, 0xD3, 0xDB,
		0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F,
		0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 0x9F
	};

	static unsigned char dosMap[256]
	= {
		0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0A, 0x0E, 0x0F,
		0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
		0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
		0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
		0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
		0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F,
		0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
		0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F,
		0xC7, 0xFC, 0xE9, 0xE2, 0xE4, 0xE0, 0xE5, 0xE7, 0xEA, 0xEB, 0xE8, 0xEF, 0xEE, 0xEC, 0xC4, 0xC5,
		0xC9, 0xE6, 0xC6, 0xF4, 0xF6, 0xF2, 0xFB, 0xF9, 0xFF, 0xD6, 0xDC, 0xA2, 0xA3, 0xA5, 0xDE, 0xA6,
		0xE1, 0xED, 0xF3, 0xFA, 0xF1, 0xD1, 0xAA, 0xBA, 0xBF, 0xC0, 0xC1, 0xBD, 0xBC, 0xCF, 0xAB, 0xBB,
		0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F,
		0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 0x9F,
		0xA1, 0xA7, 0xAD, 0xB3, 0xB8, 0xB9, 0xC3, 0xCE, 0xD2, 0xD3, 0xDB, 0xDD, 0xE3, 0xF5, 0xF8, 0xFD,
		0xA9, 0xDF, 0xC8, 0xB6, 0xCA, 0xA4, 0xB5, 0xAE, 0xD5, 0xD0, 0xD4, 0xF0, 0xD7, 0xD8, 0xCB, 0xC2,
		0xBE, 0xB1, 0xD9, 0xDA, 0xCD, 0xCC, 0xF7, 0xA8, 0xB0, 0xB7, 0xAF, 0xAC, 0xFE, 0xB2, 0xB4, 0xA0
	};
	unsigned char *o, *c;

	switch (charset) {
	case 0: /* ISO */
		if (copy != original)
			(void)strcpy(copy, original);
		break;
	case 1:           /* Mac */
		for (o = (unsigned char *)original, c = (unsigned char *)copy; *o; o++, c++)
			*c = macMap[*o];
		*c = '\0';
		break;

	case 2:           /* Dos */
		for (o = (unsigned char *)original, c = (unsigned char *)copy; *o; o++, c++)
			*c = dosMap[*o];
		*c = '\0';
		break;

	default:
		break;
	}
}

/*----------------------------------------------------------------------

  fromIso

  Converts a string from global Iso format to native. Only used in
  interpreter so character set is known at compile time.

  */
void fromIso(char copy[],       /* OUT - Mapped string */
             char original[]) { /* IN - string to convert */
	if (copy != original)
		(void)strcpy(copy, original);
}


/*----------------------------------------------------------------------
  toNative

  Converts the incoming string to the native character set from any of
  the others. The original is in the current character set which in
  the case of the compiler might be other than the native.

  */
void toNative(char copy[],  /* OUT - Mapped  string */
              char original[],  /* IN - string to convert */
              int charset) { /* IN - the current character set */
	toIso(copy, original, charset);
	if (NATIVECHARSET != 0)
		fromIso(copy, copy);
}

} // End of namespace Alan2
} // End of namespace Glk
