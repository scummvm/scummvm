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

#ifndef CRYOMNI3D_WAM_PARSER_H
#define CRYOMNI3D_WAM_PARSER_H

#include "common/array.h"
#include "common/rect.h"
#include "common/str.h"

namespace Common {
class ReadStream;
}

namespace CryOmni3D {

class Omni3DManager;

struct Zone {
	uint zoneId;
	uint action;
	Common::Rect rct;
};

struct Transition {
	uint dstId;
	double srcAlpha;
	double srcBeta;
	double dstAlpha;
	double dstBeta;
	Common::Array<Common::String> animations;
	uint getNumAnimations() const { return animations.size(); }
};

struct Place {
	uint placeId;
	Common::Array<Common::String> warps;
	Common::Array<Transition> transitions;
	Common::Array<Zone> zones;

	uint getNumStates() const { return warps.size(); }
	uint getNumTransitions() const { return transitions.size(); }
	void setupWarpConstraints(Omni3DManager &omni3d) const;
	uint hitTest(const Common::Point &point) const;
	const Transition *findTransition(uint nextPlaceId) const;
};

class WAMParser {
public:
	void loadStream(Common::ReadStream &stream);
	const Place *findPlaceById(uint placeId) const;

private:
	// For duplicate finding
	// We use a different name because else it gets chosen before the const one and fails because it's private
	Place *findPlaceById_(uint placeId);
	Common::Array<Place> _places;
};

} // End of namespace CryOmni3D

#endif
