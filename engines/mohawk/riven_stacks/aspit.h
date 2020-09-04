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

	enum PatchedExternalCommandNameId {
		kExternalSaveGame    = 20,
		kExternalRestoreGame = 21,
		kExternalResume      = 22,
		kExternalOptions     = 23,
		kExternalQuit        = 24,
		kExternalNewGame     = 25

	};

	// External commands - Main Menu
	void xastartupbtnhide(const ArgumentArray &args);
	void xasetupcomplete(const ArgumentArray &args);

	// External commands - Atrus' Journal
	void xaatrusopenbook(const ArgumentArray &args);
	void xaatrusbookback(const ArgumentArray &args);
	void xaatrusbookprevpage(const ArgumentArray &args);
	void xaatrusbooknextpage(const ArgumentArray &args);

	// External commands - Catherine's Journal
	void xacathopenbook(const ArgumentArray &args);
	void xacathbookback(const ArgumentArray &args);
	void xacathbookprevpage(const ArgumentArray &args);
	void xacathbooknextpage(const ArgumentArray &args);

	// External commands - Trap Book
	void xtrapbookback(const ArgumentArray &args);
	void xatrapbookclose(const ArgumentArray &args);
	void xatrapbookopen(const ArgumentArray &args);

	// External commands - DVD-specific
	void xarestoregame(const ArgumentArray &args);

	// External commands - ScummVM 25th anniversary specific
	void xaSaveGame(const ArgumentArray &args);
	void xaResumeGame(const ArgumentArray &args);
	void xaOptions(const ArgumentArray &args);
	void xaNewGame(const ArgumentArray &args);

	// External commands - Demo-specific
	void xadisablemenureturn(const ArgumentArray &args);
	void xaenablemenureturn(const ArgumentArray &args);
	void xalaunchbrowser(const ArgumentArray &args);
	void xadisablemenuintro(const ArgumentArray &args);
	void xaenablemenuintro(const ArgumentArray &args);
	void xademoquit(const ArgumentArray &args);
	void xaexittomain(const ArgumentArray &args);

private:
	void cathBookDrawTelescopeCombination();

	void cathBookDrawPage(uint32 page);

	bool showConfirmationDialog(const Common::U32String &message, const Common::U32String &confirmButton, const Common::U32String &cancelButton);
};

} // End of namespace RivenStacks
} // End of namespace Mohawk

#endif
