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

#ifndef RIVEN_STACKS_GSPIT_H
#define RIVEN_STACKS_GSPIT_H

#include "mohawk/riven_stacks/domespit.h"

namespace Mohawk {
namespace RivenStacks {

/**
 * Garden Island
 */
class GSpit : public DomeSpit {
public:
	GSpit(MohawkEngine_Riven *vm);

	// External commands - Pins
	void xgresetpins(const ArgumentArray &args);
	void xgrotatepins(const ArgumentArray &args);
	void xgpincontrols(const ArgumentArray &args);

	// External commands - Dome
	void xgisland25_opencard(const ArgumentArray &args);
	void xgisland25_resetsliders(const ArgumentArray &args);
	void xgisland25_slidermd(const ArgumentArray &args);
	void xgisland25_slidermw(const ArgumentArray &args);
	void xgscpbtn(const ArgumentArray &args);
	void xgisland1490_domecheck(const ArgumentArray &args);

	// External commands - Mapping
	void xgplateau3160_dopools(const ArgumentArray &args);

	// External commands - Scribe Taking the Tram
	void xgwt200_scribetime(const ArgumentArray &args);
	void xgwt900_scribe(const ArgumentArray &args);

	// External commands - Periscope/Prison Viewer
	void xgplaywhark(const ArgumentArray &args);
	void xgrviewer(const ArgumentArray &args);
	void xgwharksnd(const ArgumentArray &args);
	void xglview_prisonoff(const ArgumentArray &args);
	void xglview_villageoff(const ArgumentArray &args);
	void xglviewer(const ArgumentArray &args);
	void xglview_prisonon(const ArgumentArray &args);
	void xglview_villageon(const ArgumentArray &args);

	// Timer handlers
	void catherineViewerIdleTimer();

private:
	void lowerPins();
};

} // End of namespace RivenStacks
} // End of namespace Mohawk

#endif
