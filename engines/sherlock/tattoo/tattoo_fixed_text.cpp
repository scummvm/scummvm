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

#include "sherlock/tattoo/tattoo_fixed_text.h"
#include "sherlock/sherlock.h"

namespace Sherlock {

namespace Tattoo {

static const char *const FIXED_TEXT_ENGLISH[] = {
	"Money",
	"Card",
	"Tobacco",
	"Timetable",
	"Summons",
	"Foolscap",
	"Damp Paper",
	"Bull's Eye",

	"Money",
	"Card",
	"Tobacco",
	"Timetable",
	"Summons",
	"Foolscap",
	"Foolscap",
	"Bull's Eye Lantern",

	"Open",
	"Look",
	"Talk",
	"Use",
	"Journal",
	"Inventory",
	"Options",
	"Solve",
	"with",
	"No effect...",
	"This person has nothing to say at the moment"
};

TattooFixedText::TattooFixedText(SherlockEngine *vm) : FixedText(vm) {
}

const char *TattooFixedText::getText(int fixedTextId) {
	return FIXED_TEXT_ENGLISH[fixedTextId];
}

const Common::String TattooFixedText::getActionMessage(FixedTextActionId actionId, int messageIndex) {
	return Common::String();
}


} // End of namespace Tattoo

} // End of namespace Sherlock
