/***************************************************************************
 games.h Copyright (C) 2002 Solomon Peachy

 This program may be modified and copied freely according to the terms of
 the GNU general public license (GPL), as long as the above copyright
 notice and the licensing information contained herein are preserved.

 Please refer to www.gnu.org for licensing details.

 This work is provided AS IS, without warranty of any kind, expressed or
 implied, including but not limited to the warranties of merchantibility,
 noninfringement, and fitness for a specific purpose. The author will not
 be held liable for any damage caused by this work or derivatives of it.

 By using this source code, you agree to the licensing terms as stated
 above.

 Please contact the maintainer for bug reports or inquiries.

 Current Maintainer:

    Solomon Peachy [pizza@shaftnet.org]

***************************************************************************/

/* Game identification */

#ifndef _SCI_GAMES_H_
#define _SCI_GAMES_H_

#ifndef NEED_SCI_VERSIONS
#  error "You shouldn't be including this header file."
#endif

#include "sci/include/versions.h"

typedef struct _sci_game {
  int id; /* currently CRC of resource.001 */
  int res_version;
  sci_version_t version;
  const char *name;
} sci_game_t;

/* Interpreter versions for Amiga and Atari ST ports are tentative */
sci_game_t sci_games[] = {
	{ 0x5D451535, SCI_VERSION_AUTODETECT, SCI_VERSION(1,000,510), "Leisure Suit Larry 1 v1.0-mac"}, /* x.yyy.zzz */ /* Launcher says v2.0, game crashes on DoAvoider */
	{ 0x6C176EE0, SCI_VERSION_AUTODETECT, SCI_VERSION(1,000,577), "Leisure Suit Larry 1 v2.1"},
	{ 0x1C36E076, SCI_VERSION_AUTODETECT, SCI_VERSION(1,000,510), "Leisure Suit Larry 1 v1.000-es"}, /* 1.SQ4.057 */ /* Crashes on function 0x7b */

	{ 0xFEAB629D, SCI_VERSION_AUTODETECT, SCI_VERSION(0,000,343), "Leisure Suit Larry 2 v1.000.011-3.5"},
	{ 0x13DD3CD2, SCI_VERSION_AUTODETECT, SCI_VERSION(0,000,343), "Leisure Suit Larry 2 v1.000.011-5.25" },
	{ 0x1D0F3B31, SCI_VERSION_AUTODETECT, SCI_VERSION(0,000,572), "Leisure Suit Larry 2 v1.001.006-st"}, /* 1.000.159 */
	{ 0x40BEC726, SCI_VERSION_AUTODETECT, SCI_VERSION(0,000,409), "Leisure Suit Larry 2 v1.002.000-3.5"},
	{ 0x0C848403, SCI_VERSION_AUTODETECT, SCI_VERSION(0,000,409), "Leisure Suit Larry 2 v1.002.000-5.25" },
	{ 0x7E9CF339, SCI_VERSION_AUTODETECT, SCI_VERSION(0,000,572), "Leisure Suit Larry 2 v1.003-ami"}, /* x.yyy.zzz */

	{ 0x06D737B5, SCI_VERSION_AUTODETECT, SCI_VERSION(0,000,572), "Leisure Suit Larry 3 v1.003-3.5" },
	{ 0xE0A1C352, SCI_VERSION_AUTODETECT, SCI_VERSION(0,000,572), "Leisure Suit Larry 3 v1.003-5.25" },
	{ 0xC48FE83A, SCI_VERSION_AUTODETECT, SCI_VERSION(0,000,572), "Leisure Suit Larry 3 v1.021-3.5" },
	{ 0x484587DD, SCI_VERSION_AUTODETECT, SCI_VERSION(0,000,572), "Leisure Suit Larry 3 v1.021-5.25"},
/*	{ 0x????????, SCI_VERSION_AUTODETECT, SCI_VERSION(0,000,572), "Leisure Suit Larry 3 v1.021-st"},*/ /* 1.002.026 */
	{ 0x6348030A, SCI_VERSION_AUTODETECT, SCI_VERSION(0,000,572), "Leisure Suit Larry 3 v1.039-ami"}, /* 1.002.032 */

	{ 0x94EA377B, SCI_VERSION_AUTODETECT, SCI_VERSION(0,000,685), "CB1" },
	{ 0xFD9EE7BD, SCI_VERSION_AUTODETECT, SCI_VERSION(0,000,685), "Camelot" },
	{ 0x2829987F, SCI_VERSION_AUTODETECT, SCI_VERSION(0,000,685), "Camelot" },
	{ 0x980CEAD3, SCI_VERSION_AUTODETECT, SCI_VERSION(0,000,629), "Demo Quest" },
	{ 0x3DB972CA, SCI_VERSION_AUTODETECT, SCI_VERSION(0,000,572), "Hoyle 2" },
	{ 0xC0B37651, SCI_VERSION_AUTODETECT, SCI_VERSION(0,000,685), "Iceman" },
	{ 0xDABA6B8A, SCI_VERSION_AUTODETECT, SCI_VERSION(0,000,999), "KQ1 v1.000.051-3.5" }, /* S.old.010 */
	{ 0x270E37F3, SCI_VERSION_AUTODETECT, SCI_VERSION(0,000,274), "KQ4" },
	{ 0x685F1205, SCI_VERSION_AUTODETECT, SCI_VERSION(0,000,502), "KQ4" },
	{ 0xC14E3A2A, SCI_VERSION_AUTODETECT, SCI_VERSION(0,000,395), "PQ2" },
	{ 0x4BD66036, SCI_VERSION_AUTODETECT, SCI_VERSION(0,000,490), "PQ2" },
	{ 0x7132D6D8, SCI_VERSION_AUTODETECT, SCI_VERSION(0,000,629), "QfG1" },
	{ 0xF8F4913F, SCI_VERSION_AUTODETECT, SCI_VERSION(0,000,685), "SQ3" },
	{ 0x34FBC324, SCI_VERSION_AUTODETECT, SCI_VERSION(0,000,999), "SQ3/DE" }, /* S.old.114 */
	{ 0xE4A3234D, SCI_VERSION_AUTODETECT, SCI_VERSION(0,000,506), "Fun Seekers Guide v1.02"},
	{ 0x85AFE241, SCI_VERSION_AUTODETECT, SCI_VERSION(0,000,519), "Hoyle 1 v1.000.104"},
	{ 0xE0E070C3, SCI_VERSION_AUTODETECT, SCI_VERSION(0,000,572), "Hoyle 2 v1.000.011"},
	{ 0xD0B8794E, SCI_VERSION_AUTODETECT, SCI_VERSION(0,000,668), "Iceman v1.023"},
	{ 0x94EA377B, SCI_VERSION_AUTODETECT, SCI_VERSION(0,000,631), "The Colonel's Bequest v1.000.046"},
	{ 0x28543FDF, SCI_VERSION_AUTODETECT, SCI_VERSION(0,000,453), "Astro Chicken"},
	{ 0x31F46F7D, SCI_VERSION_AUTODETECT, SCI_VERSION(0,000,453), "Space Quest III v1.0V int"},
	{ 0xAA2C94B9, SCI_VERSION_AUTODETECT, SCI_VERSION(0,000,685), "Mixed-Up Mother Goose v1.011 Int.#8.2.90"},
	{ 0x3B15678B, SCI_VERSION_AUTODETECT, SCI_VERSION(0,000,631), "The Colonel's Bequest v1.000.046-3.5"},
	{ 0x0E042F46, SCI_VERSION_AUTODETECT, SCI_VERSION(0,000,530), "Hoyle 1 v1.000.113-3.5"},
	{ 0x1EACB959, SCI_VERSION_AUTODETECT, SCI_VERSION(0,000,566), "HQ v1.000-5.25"},
	{ 0x2BEAF5E7, SCI_VERSION_AUTODETECT, SCI_VERSION(0,000,566), "HQ v1.001-5.25"},
	{ 0x63626D3E, SCI_VERSION_AUTODETECT, SCI_VERSION(0,000,668), "Iceman v1.023-5.25"},
	{ 0xDA5E7B7D, SCI_VERSION_AUTODETECT, SCI_VERSION(0,000,409), "KQ4 v1.003.006-3.5"},
	{ 0x376A5472, SCI_VERSION_AUTODETECT, SCI_VERSION(0,000,502), "KQ4 v1.006.003-5.25"},
	{ 0x364B40B2, SCI_VERSION_AUTODETECT, SCI_VERSION(0,000,395), "PQ2 v1.001.000-5.25"},
	{ 0x664B4123, SCI_VERSION_AUTODETECT, SCI_VERSION(0,000,409), "PQ2 v1.001.006-3.5"},
	{ 0x379F4582, SCI_VERSION_AUTODETECT, SCI_VERSION(0,000,453), "SQ3 v1.0V-5.25"},
	{ 0x04B0B081, SCI_VERSION_AUTODETECT, SCI_VERSION(0,000,294), "xmascard v1.04"},

	{ 0x4447B28D, SCI_VERSION_AUTODETECT, SCI_VERSION(1,000,72),  "Trial v1.105"},

	{ 0xB1C2CCAE, SCI_VERSION_AUTODETECT, SCI_VERSION(1,000,200), "SQ4 v1.052"}, /* 1.000.753 */
	{ 0xAA6AF6A9, SCI_VERSION_AUTODETECT, SCI_VERSION(1,000,60),  "KQ5 v0.000.062"},
	{ 0x092C2C0D, 3,		      SCI_VERSION(1,000,172), "jones v1.000.060"}, /* x.yyy.zzz */

	{ 0xC415A485, SCI_VERSION_AUTODETECT, SCI_VERSION(1,000,172), "jones v1.000.060-cd"}, /* x.yyy.zzz */

	{ 0x89C595E3, SCI_VERSION_AUTODETECT, SCI_VERSION(1,000,510), "SQ1 v2.000"}, /* T.A00.081 */
	{ 0x09D4FC54, SCI_VERSION_AUTODETECT, SCI_VERSION(1,000,510), "LSL5 v1.000"}, /* T.A00.169 */
	{ 0xF3ED1D81, SCI_VERSION_AUTODETECT, SCI_VERSION(1,000,510), "PQ3 v1.00"}, /* T.A00.178 */
	{ 0x501B5E6B, SCI_VERSION_AUTODETECT, SCI_VERSION(1,000,510), "Brain v1.000"}, /* 1.000.044 */
	{ 0xB1B7279B, SCI_VERSION_AUTODETECT, SCI_VERSION(1,000,510), "Longbow v1.000"}, /* 1.000.168 */

	{ 0x82595EBE, SCI_VERSION_AUTODETECT, SCI_VERSION(0,000,453), "SQ3 v1.0V-ami"}, /* x.yyy.zzz */
	{ 0xF6080B61, SCI_VERSION_AUTODETECT, SCI_VERSION(0,000,530), "Hoyle 1 v1.000.139-ami"}, /* x.yyy.zzz */

	{ 0x8AFEA2D0, 2,		      SCI_VERSION(1,000,000), "KQ1 v1.000.054-ami"}, /* 1.003.007 */

	/* Undetermined Amiga versions: */
/*	{ 0x8AE5F854, ?, SCI_VERSION(), "ARTHUR" }, */
/*	{ 0x9FB7015B, ?, SCI_VERSION(), "CB1" }, */
/*	{ 0x560CEDD5, ?, SCI_VERSION(), "iceMan" }, */

	{ 0, 0, 0, NULL } /* terminator */
};

#endif /* _SCI_GAMES_H_ */
