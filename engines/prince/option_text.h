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
	"Gjlevfnm",
	"Jcvjnhtnm",
	"Dpznm ",
	"Bcgjkmp.",
	"Jnrhsnm/""\x83""bnm ",
	"Pfrhsnm/Nzyenm  ",
	"Ujdjhbnm  "
};

} // End of namespace Prince
