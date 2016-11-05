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
	void xgresetpins(uint16 argc, uint16 *argv);
	void xgrotatepins(uint16 argc, uint16 *argv);
	void xgpincontrols(uint16 argc, uint16 *argv);

	// External commands - Dome
	void xgisland25_opencard(uint16 argc, uint16 *argv);
	void xgisland25_resetsliders(uint16 argc, uint16 *argv);
	void xgisland25_slidermd(uint16 argc, uint16 *argv);
	void xgisland25_slidermw(uint16 argc, uint16 *argv);
	void xgscpbtn(uint16 argc, uint16 *argv);
	void xgisland1490_domecheck(uint16 argc, uint16 *argv);

	// External commands - Mapping
	void xgplateau3160_dopools(uint16 argc, uint16 *argv);

	// External commands - Scribe Taking the Tram
	void xgwt200_scribetime(uint16 argc, uint16 *argv);
	void xgwt900_scribe(uint16 argc, uint16 *argv);

	// External commands - Periscope/Prison Viewer
	void xgplaywhark(uint16 argc, uint16 *argv);
	void xgrviewer(uint16 argc, uint16 *argv);
	void xgwharksnd(uint16 argc, uint16 *argv);
	void xglview_prisonoff(uint16 argc, uint16 *argv);
	void xglview_villageoff(uint16 argc, uint16 *argv);
	void xglviewer(uint16 argc, uint16 *argv);
	void xglview_prisonon(uint16 argc, uint16 *argv);
	void xglview_villageon(uint16 argc, uint16 *argv);

	// Timer handlers
	void catherineViewerIdleTimer();

private:
	void lowerPins();
};

} // End of namespace RivenStacks
} // End of namespace Mohawk

#endif
