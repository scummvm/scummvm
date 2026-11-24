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

#ifndef DUNE_INTRO_H
#define DUNE_INTRO_H

#include "common/scummsys.h"

namespace Dune {

class DuneEngine;

/**
 * The opening of the game: the two publisher logos, the title, and Irulan
 * reading the first lines of the book over the desert.
 *
 * The original drives this from a table of scenes, each naming a routine to
 * load with, a routine to play with, a fade to come in on and a count of
 * ticks of the intro music to hold for.  The table is kept in the engine as
 * kOriginalScript for the parts still to be written; what runs here plays the
 * movies of the opening in order and lets any of them be skipped.
 */
class Intro {
public:
	explicit Intro(DuneEngine *vm) : _vm(vm) {}

	// Returns false if the player asked to leave the intro behind.
	bool run();

private:
	DuneEngine *_vm;
};

} // End of namespace Dune

#endif
