/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "twine/holomap_v2.h"
#include "twine/resources/hqr.h"
#include "twine/resources/resources.h"

namespace TwinE {

bool HolomapV2::setHoloPos(int32 locationIdx) {
	return false;
}

bool HolomapV2::loadLocations() {
	// _locations[MAX_OBJECTIF + 67].FlagHolo = 1; //	Desert Globe
	return HQR::getEntry((uint8 *)_locations, Resources::HQR_HOLOMAP_FILE, RESSHQR_ARROWBIN) != 0;
}

const char *HolomapV2::getLocationName(int index) const {
	if (index >= 0 && index < ARRAYSIZE(_locations)) {
		// TODO: return _locations[index].;
	}
	return "";
}

void HolomapV2::clrHoloPos(int32 locationIdx) {
}

void HolomapV2::drawHolomapTrajectory(int32 trajectoryIndex) {
}

void HolomapV2::initHoloDatas() {
}

void HolomapV2::holoMap() {
}

} // namespace TwinE
