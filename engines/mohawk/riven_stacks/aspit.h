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

#ifndef RIVEN_STACKS_ASPIT_H
#define RIVEN_STACKS_ASPIT_H

#include "mohawk/riven_stack.h"

namespace Mohawk {
namespace RivenStacks {

/**
 * Main Menu, Books, Setup
 */
class ASpit : public RivenStack {
public:
	ASpit(MohawkEngine_Riven *vm);

	// External commands - Main Menu
	void xastartupbtnhide(uint16 argc, uint16 *argv);
	void xasetupcomplete(uint16 argc, uint16 *argv);

	// External commands - Atrus' Journal
	void xaatrusopenbook(uint16 argc, uint16 *argv);
	void xaatrusbookback(uint16 argc, uint16 *argv);
	void xaatrusbookprevpage(uint16 argc, uint16 *argv);
	void xaatrusbooknextpage(uint16 argc, uint16 *argv);

	// External commands - Catherine's Journal
	void xacathopenbook(uint16 argc, uint16 *argv);
	void xacathbookback(uint16 argc, uint16 *argv);
	void xacathbookprevpage(uint16 argc, uint16 *argv);
	void xacathbooknextpage(uint16 argc, uint16 *argv);

	// External commands - Trap Book
	void xtrapbookback(uint16 argc, uint16 *argv);
	void xatrapbookclose(uint16 argc, uint16 *argv);
	void xatrapbookopen(uint16 argc, uint16 *argv);

	// External commands - DVD-specific
	void xarestoregame(uint16 argc, uint16 *argv);

	// External commands - Demo-specific
	void xadisablemenureturn(uint16 argc, uint16 *argv);
	void xaenablemenureturn(uint16 argc, uint16 *argv);
	void xalaunchbrowser(uint16 argc, uint16 *argv);
	void xadisablemenuintro(uint16 argc, uint16 *argv);
	void xaenablemenuintro(uint16 argc, uint16 *argv);
	void xademoquit(uint16 argc, uint16 *argv);
	void xaexittomain(uint16 argc, uint16 *argv);

private:
	void cathBookDrawTelescopeCombination();

	void cathBookDrawPage(uint32 page);
};

} // End of namespace RivenStacks
} // End of namespace Mohawk

#endif
