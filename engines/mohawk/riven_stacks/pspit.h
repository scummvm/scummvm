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

#ifndef RIVEN_STACKS_PSPIT_H
#define RIVEN_STACKS_PSPIT_H

#include "mohawk/riven_stacks/domespit.h"

namespace Mohawk {
namespace RivenStacks {

/**
 * Prison Island
 */
class PSpit : public DomeSpit {
public:
	PSpit(MohawkEngine_Riven *vm);

	// RivenStack API
	void installCardTimer() override;

	// External commands - Prison Elevator
	void xpisland990_elevcombo(const ArgumentArray &args);	// Param1: button

	// External commands - Dome
	void xpscpbtn(const ArgumentArray &args);
	void xpisland290_domecheck(const ArgumentArray &args);
	void xpisland25_opencard(const ArgumentArray &args);
	void xpisland25_resetsliders(const ArgumentArray &args);
	void xpisland25_slidermd(const ArgumentArray &args);
	void xpisland25_slidermw(const ArgumentArray &args);

	// Timer callbacks
	void catherineIdleTimer();
};

} // End of namespace RivenStacks
} // End of namespace Mohawk

#endif
