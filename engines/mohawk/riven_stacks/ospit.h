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

#ifndef RIVEN_STACKS_OSPIT_H
#define RIVEN_STACKS_OSPIT_H

#include "mohawk/riven_stack.h"

namespace Mohawk {
namespace RivenStacks {

/**
 * 233rd Age / Gehn's Office
 */
class OSpit : public RivenStack {
public:
	OSpit(MohawkEngine_Riven *vm);

	// External commands - Death!
	void xorollcredittime(const ArgumentArray &args);

	// External commands - Trap Book Puzzle
	void xbookclick(const ArgumentArray &args); // Four params -- movie_sref, start_time, end_time, u0

	// External commands - Blank Linking Book
	void xooffice30_closebook(const ArgumentArray &args);

	// External commands - Gehn's Journal
	void xobedroom5_closedrawer(const ArgumentArray &args);
	void xogehnopenbook(const ArgumentArray &args);
	void xogehnbookprevpage(const ArgumentArray &args);
	void xogehnbooknextpage(const ArgumentArray &args);

	// External commands - Elevator Combination
	void xgwatch(const ArgumentArray &args);

};

} // End of namespace RivenStacks
} // End of namespace Mohawk

#endif
