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

namespace Prince {

// PL - Mazovia coding
const char invOptionsTextPL[5][18] = {
	"Obejrzyj",
	"U\xa7""yj",
	"Otw\xa2""rz/Pchnij",
	"Zamknij/Poci\x86""gnij",
	"Daj"
};

const char optionsTextPL[7][18] = {
	"Podejd\xa6",
	"Obejrzyj",
	"Zabierz",
	"U\xa7""yj",
	"Otw\xa2""rz/Pchnij",
	"Zamknij/Poci\x86""gnij",
	"Porozmawiaj"
};

// DE - Other font then for PL + ISO 8859-2 or Windows-1250
// + special letter values changing
// Normal value:  196,  214,  220,  223,  228,  246,  252
// Prince change: 131,  132,  133,  127,  128,  129,  130
char invOptionsTextDE[5][17] = {
	"Anschauen",
	"Benutzen",
	"\x84""ffnen/Sto\x7f""en",
	"Schlie\x7f""en/Ziehen",
	"Geben"
};

const char optionsTextDE[7][17] = {
	"Hingehen",
	"Anschauen",
	"Wegnehmen",
	"Benutzen",
	"\x84""ffnen/Sto\x7f""en",
	"Schlie\x7f""en/Ziehen",
	"Ansprechen"
};

// EN
const char *invOptionsTextEN[] = {
	"Examine",
	"Use",
	"Open/Push",
	"Close/Pull",
	"Give"
};

const char *optionsTextEN[] = {
	"Walk to",
	"Examine",
	"Pick up",
	"Use",
	"Open/Push",
	"Close/Pull",
	"Talk to"
};

// RU
const char *invOptionsTextRU[] = {
	"Cvjnhtnm",
	"Bcgjkmp.",
	"Jnrhsnm/""\x83""bnm ",
	"Pfrhsnm/Nzyenm  ",
	"Lfnm "
};

const char *optionsTextRU[] = {
	"Gjljqnb",
	"Jcvjnhtnm",
	"Dpznm ",
	"Bcgjkmp.",
	"Jnrhsnm/""\x83""bnm ",
	"Pfrhsnm/Nzyenm  ",
	"Ujdjhbnm  "
};

// RU localization from "Russian Project"
const char *invOptionsTextRU2[] = {
	"n""\x91""\x8c""\x8e""\x92""\x90""\x85""\x92""\x9c",
	"h""\x91""\x8f""\x8e""\x8b""\x9c""\x87"".",
	"n""\x92""\x8a""\x90""\x9b""\x92""\x9c""/r""\x8e""\x8b""\x8a""\x80""\x92""\x9c",
	"g""\x80""\x8a""\x90""\x9b""\x92""\x9c""/r""\x9f""\x8d""\x93""\x92""\x9c",
	"d""\x80""\x92""\x9c"
};

const char *optionsTextRU2[] = {
	"o""\x8e""\x84""\x8e""\x89""\x92""\x88",
	"n""\x91""\x8c""\x8e""\x92""\x90""\x85""\x92""\x9c",
	"b""\x87""\x9f""\x92""\x9c",
	"h""\x91""\x8f""\x8e""\x8b""\x9c""\x87"".",
	"n""\x92""\x8a""\x90""\x9b""\x92""\x9c""/r""\x8e""\x8b""\x8a""\x80""\x92""\x9c",
	"g""\x80""\x8a""\x90""\x9b""\x92""\x9c""/r""\x9f""\x8d""\x93""\x92""\x9c",
	"c""\x8e""\x82""\x8e""\x90""\x88""\x92""\x9c"
};

} // End of namespace Prince
