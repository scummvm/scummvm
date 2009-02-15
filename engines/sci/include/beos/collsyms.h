/* collsyms.h -- collating symbol names and their corresponding characters
		 (in ascii) as given by POSIX.2 in table 2.8. */

/* Copyright (C) 1997 Free Software Foundation, Inc.

   This file is part of GNU Bash, the Bourne Again SHell.
   
   Bash is free software; you can redistribute it and/or modify it under
   the terms of the GNU General Public License as published by the Free
   Software Foundation; either version 2, or (at your option) any later
   version.
              
   Bash is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
   for more details.
                         
   You should have received a copy of the GNU General Public License along
   with Bash; see the file COPYING.  If not, write to the Free Software
   Foundation, 59 Temple Place, Suite 330, Boston, MA 02111 USA. */

#ifndef _COLLSYMS_H_
#  define _COLLSYSMS_H_

/* The upper-case letters, lower-case letters, and digits are omitted from
   this table.  The digits are not included in the table in the POSIX.2
   spec.  The upper and lower case letters are translated by the code
   in fnmatch.c:collsym(). */

typedef struct _collsym {
  char *name;
  char code;
} COLLSYM;

static COLLSYM posix_collsyms[] =
{
  "NUL",	'\0',
  "SOH",	'\001',
  "STX",	'\002',
  "ETX",	'\003',
  "EOT",	'\004',
  "ENQ",	'\005',
  "ACK",	'\006',
#ifdef __STDC__
  "alert",	'\a',
#else
  "alert",	'\007',
#endif
  "backspace",	'\b',
  "tab",	'\t',
  "newline",	'\n',
  "vertical-tab",	'\v',
  "form-feed",	'\f',
  "carriage-return",	'\r',
  "SO",		'\016',
  "SI",		'\017',
  "DLE",	'\020',
  "DC1",	'\021',
  "DC2",	'\022',
  "DC3",	'\023',
  "DC4",	'\024',
  "NAK",	'\025',
  "SYN",	'\026',
  "ETB",	'\027',
  "CAN",	'\030',
  "EM",		'\031',
  "SUB",	'\032',
  "ESC",	'\033',
  "IS4",	'\034',
  "IS3",	'\035',
  "IS2",	'\036',
  "IS1",	'\037',
  "space",		' ',
  "exclamation-mark",	'!',
  "quotation-mark",	'"',
  "number-sign",	'#',
  "dollar-sign",	'$',
  "percent-sign",	'%',
  "ampersand",		'&',
  "apostrophe",		'\'',
  "left-parenthesis",	'(',
  "right-parenthesis",	')',
  "asterisk",	'*',
  "plus-sign",	'+',
  "comma",	',',
  "hyphen",	'-',
  "minus",	'-',		/* extension from POSIX.2 */
  "dash",	'-',		/* extension from POSIX.2 */
  "period",	'.',
  "slash",	'/',
  "solidus",	'/',		/* extension from POSIX.2 */
  "zero",	'0',
  "one",	'1',
  "two",	'2',
  "three",	'3',
  "four",	'4',
  "five",	'5',
  "six",	'6',
  "seven",	'7',
  "eight",	'8',
  "nine",	'9',
  "colon",	':',
  "semicolon",	';',
  "less-than-sign",	'<',
  "equals-sign",	'=',
  "greater-than-sign",	'>',
  "question-mark",	'?',
  "commercial-at",	'@',
  /* upper-case letters omitted */
  "left-square-bracket",'[',
  "backslash",		'\\',
  "reverse-solidus",	'\\',
  "right-square-bracket",	']',
  "circumflex",		'^',
  "circumflex-accent",	'^',		/* extension from POSIX.2 */
  "underscore",		'_',
  "grave-accent",		'`',
  /* lower-case letters omitted */
  "left-brace",		'{',		/* extension from POSIX.2 */
  "left-curly-bracket",	'{',
  "vertical-line",	'|',
  "right-brace",		'}',	/* extension from POSIX.2 */
  "right-curly-bracket",	'}',
  "tilde",		'~',
  "DEL",	'\177',
  0,	0,
};

#endif
