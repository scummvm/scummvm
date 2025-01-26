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

#include "mediastation/assets/hotspot.h"
#include "mediastation/mediastation.h"

namespace MediaStation {

Hotspot::Hotspot(AssetHeader *header) : Asset(header) {
	if (header->_startup == kAssetStartupActive) {
		_isActive = true;
	}
}

bool Hotspot::isInside(const Common::Point &pointToCheck) {
	// No sense checking the polygon if we're not even in the bbox.
	if (!_header->_boundingBox->contains(pointToCheck)) {
		return false;
	}

	// We're in the bbox, but there might not be a polygon to check.
	if (_header->_mouseActiveArea.empty()) {
		return true;
	}

	// Polygon intersection code adapted from HADESCH engine, might need more
	// refinement once more testing is possible.
	Common::Point point = pointToCheck - Common::Point(_header->_boundingBox->left, _header->_boundingBox->top);
	int rcross = 0; // Number of right-side overlaps

	// Each edge is checked whether it cuts the outgoing stream from the point
	Common::Array<Common::Point *> _polygon = _header->_mouseActiveArea;
	for (unsigned i = 0; i < _polygon.size(); i++) {
		const Common::Point &edgeStart = *_polygon[i];
		const Common::Point &edgeEnd = *_polygon[(i + 1) % _polygon.size()];

		// A vertex is a point? Then it lies on one edge of the polygon
		if (point == edgeStart)
			return true;

		if ((edgeStart.y > point.y) != (edgeEnd.y > point.y)) {
			int term1 = (edgeStart.x - point.x) * (edgeEnd.y - point.y) - (edgeEnd.x - point.x) * (edgeStart.y - point.y);
			int term2 = (edgeEnd.y - point.y) - (edgeStart.y - edgeEnd.y);
			if ((term1 > 0) == (term2 >= 0))
				rcross++;
		}
	}

	// The point is strictly inside the polygon if and only if the number of overlaps is odd
	return ((rcross % 2) == 1);
}

Operand Hotspot::callMethod(BuiltInMethod methodId, Common::Array<Operand> &args) {
	switch (methodId) {
	case kMouseActivateMethod: {
		assert(args.empty());
		_isActive = true;
		g_engine->addPlayingAsset(this);
		return Operand();
	}

	case kMouseDeactivateMethod: {
		assert(args.empty());
		_isActive = false;
		return Operand();
	}

	default: {
		error("Hotspot::callMethod(): Got unimplemented method ID %d", methodId);
	}
	}
}

} // End of namespace MediaStation
