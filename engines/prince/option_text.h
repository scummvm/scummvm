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

namespace Prince {

// PL - Mazovia coding (with U2 negation for special letters)
// DE - Still problem with special letters

const char invOptionsTextPL[5][18] = {
	"Obejrzyj",
	{ 'U', -89, 'y', 'j', '\0' },
	{ 'O', 't', 'w', -94, 'r', 'z', '/', 'P', 'c', 'h', 'n', 'i', 'j', '\0' },
	{ 'Z', 'a', 'm', 'k', 'n', 'i', 'j', '/', 'P', 'o', 'c', 'i', -122, 'g', 'n', 'i', 'j', '\0' },
	"Daj"
};

// TODO
const char invOptionsTextDE[5][17] = {
	"Anschauen",
	"Benutzen",
	{ 'Ö', 'f', 'f', 'n', 'e', 'n', '/', 'S', 't', 'o', 'ß', 'e', 'n', '\0' },
	{ 'S', 'c', 'h', 'l', 'i', 'e', 'ß', 'e', 'n', '/', 'Z', 'i', 'e', 'h', 'e', 'n', '\0' },
	"Geben"
};

const char *invOptionsTextEN[] = {
	"Examine",
	"Use",
	"Open/Push",
	"Close/Pull",
	"Give"
};

const char optionsTextPL[7][18] = {
	{ 'P', 'o', 'd', 'e', 'j', 'd', -90, '\0' },
	"Obejrzyj",
	"Zabierz",
	{ 'U', -89, 'y', 'j' },
	{ 'O', 't', 'w', -94, 'r', 'z', '/', 'P', 'c', 'h', 'n', 'i', 'j', '\0' },
	{ 'Z', 'a', 'm', 'k', 'n', 'i', 'j', '/', 'P', 'o', 'c', 'i', -122, 'g', 'n', 'i', 'j', '\0' },
	"Porozmawiaj"
};

// TODO
const char optionsTextDE[7][17] = {
	"Hingehen",
	"Anschauen",
	"Wegnehmen",
	"Benutzen",
	{ 'Ö', 'f', 'f', 'n', 'e', 'n', '/', 'S', 't', 'o', 'ß', 'e', 'n', '\0' },
	{ 'S', 'c', 'h', 'l', 'i', 'e', 'ß', 'e', 'n', '/', 'Z', 'i', 'e', 'h', 'e', 'n', '\0' },
	"Ansprechen"
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

}

