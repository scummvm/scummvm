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

#ifndef SHERLOCK_SCALPEL_TSAGE_LOGO_H
#define SHERLOCK_SCALPEL_TSAGE_LOGO_H

#include "common/scummsys.h"
#include "common/array.h"
#include "common/file.h"
#include "common/list.h"
#include "common/str.h"
#include "common/str-array.h"
#include "common/util.h"
#include "graphics/surface.h"
#include "sherlock/scalpel/tsage/resources.h"
#include "sherlock/screen.h"

namespace Sherlock {
namespace Scalpel {

class ScalpelEngine;

namespace TsAGE {

class Logo {
private:
	ScalpelEngine *_vm;
	TLib _lib;
	Surface _surface;
	int _counter;
	byte _palette1[PALETTE_SIZE];
	byte _palette2[PALETTE_SIZE];
	byte _palette3[PALETTE_SIZE];

	Logo(ScalpelEngine *vm);

	void nextFrame();

	bool finished() const;

	/**
	 * Load the background for the scene
	 */
	void loadBackground();

	/**
	 * Fade from the current palette to a new one
	 */
	void fade(const byte palette[PALETTE_SIZE]);
public:
	static bool show(ScalpelEngine *vm);
};

} // end of namespace TsAGE
} // end of namespace Scalpel
} // end of namespace Sherlock

#endif
