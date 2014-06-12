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

// Mazovia coding (with U2 negation)

const char invOptionsTextPL[5][18] = {
	{ 'O', 'b', 'e', 'j', 'r', 'z', 'y', 'j', '\0' }, 
	{ 'U', -89, 'y', 'j', '\0' },
	{ 'O', 't', 'w', -94, 'r', 'z', '/', 'P', 'c', 'h', 'n', 'i', 'j', '\0' },
	{ 'Z', 'a', 'm', 'k', 'n', 'i', 'j', '/', 'P', 'o', 'c', 'i', -122, 'g', 'n', 'i', 'j', '\0' },
	{ 'D', 'a', 'j', '\0' }
};

// TODO
const char *invOptionsTextDE[] = {
	"Anschauen",
	"Benutzen",
	"÷ffnen/Stoﬂen",
	"Schlieﬂen/Ziehen",
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
	{ 'O', 'b', 'e', 'j', 'r', 'z', 'y', 'j', '\0' },
	{ 'Z', 'a', 'b', 'i', 'e', 'r', 'z', '\0' },
	{ 'U', -89, 'y', 'j' },
	{ 'O', 't', 'w', -94, 'r', 'z', '/', 'P', 'c', 'h', 'n', 'i', 'j', '\0' },
	{ 'Z', 'a', 'm', 'k', 'n', 'i', 'j', '/', 'P', 'o', 'c', 'i', -122, 'g', 'n', 'i', 'j', '\0' },
	{ 'P', 'o', 'r', 'o', 'z', 'm', 'a', 'w', 'i', 'a', 'j', '\0' }
};

// TODO
const char *optionsTextDE[] = {
	"Hingehen",
	"Anschauen",
	"Wegnehmen",
	"Benutzen",
	"÷ffnen/Stoﬂen",
	"Schlieﬂen/Ziehen",
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

