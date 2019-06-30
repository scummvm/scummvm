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

#include "common/stream.h"

#include "cryomni3d/wam_parser.h"
#include "cryomni3d/omni3d.h"

namespace CryOmni3D {

void WAMParser::loadStream(Common::ReadStream &stream) {
	char str[16];

	_places.clear();

	// These are unused and unknown values
	(void) stream.readByte();
	(void) stream.readByte();
	(void) stream.read(str, 16);
	(void) stream.readUint32LE();

	uint nPlaces = stream.readByte();
	//debug("nPlaces = %u", nPlaces);
	for (uint i = 0; i < nPlaces; i++) {
		Place place;
		uint nWarps = stream.readByte();
		//debug("nWarps = %u", nWarps);
		for (uint k = 0; k < 8; k++) {
			stream.read(str, 16);
			//debug("Warp: %.16s", str);
			if (nWarps > 0) {
				place.warps.push_back(str);
				nWarps--;
			}
		}
		place.placeId = stream.readUint32LE();
		// Normally placeId should be unique but it's not always the case
		// In original game the last place is considered but we try to be efficient and stop at the first place in findPlaceById
		// Let's be a little less efficient at startup by removing duplicates
		Place *oldPlace = findPlaceById_(place.placeId);
		if (oldPlace) {
			debug("Found duplicate place %u at %u, removing it", place.placeId,
			      (uint)(oldPlace - _places.begin()));
			_places.erase(oldPlace);
		}
		//debug("nPlaceId = %u", place.placeId);
		(void) stream.readUint32LE();
		uint nTransitions = stream.readByte();
		//debug("nTransitions = %u", nTransitions);
		uint nZones = stream.readByte();
		//debug("nZones = %u", nZones);
		for (uint j = 0; j < nTransitions; j++) {
			Transition trans;
			(void) stream.readUint32LE();
			uint nAnimations = stream.readByte();
			for (uint k = 0; k < 8; k++) {
				stream.read(str, 16);
				if (nAnimations > 0) {
					trans.animations.push_back(str);
					nAnimations--;
				}
			}
			(void) stream.readUint32LE();
			trans.dstId = stream.readUint32LE();
			// Unused byte
			(void) stream.readByte();
			trans.srcAlpha = stream.readDoubleLE();
			trans.srcBeta = stream.readDoubleLE();
			trans.dstAlpha = stream.readDoubleLE();
			trans.dstBeta = stream.readDoubleLE();
			place.transitions.push_back(trans);
		}
		for (uint j = 0; j < nZones; j++) {
			Zone zone;
			zone.zoneId = stream.readSint32LE();
			zone.rct.left = stream.readSint32LE();
			zone.rct.top = stream.readSint32LE();
			zone.rct.setWidth(stream.readSint32LE());
			zone.rct.setHeight(stream.readSint32LE());
			zone.action = stream.readSint32LE();
			place.zones.push_back(zone);
		}
		_places.push_back(place);
	}
}

const Place *WAMParser::findPlaceById(uint placeId) const {
	for (Common::Array<Place>::const_iterator it = _places.begin(); it != _places.end(); it++) {
		if (it->placeId == placeId) {
			return it;
		}
	}
	return nullptr;
}

Place *WAMParser::findPlaceById_(uint placeId) {
	for (Common::Array<Place>::iterator it = _places.begin(); it != _places.end(); it++) {
		if (it->placeId == placeId) {
			return it;
		}
	}
	return nullptr;
}

void Place::setupWarpConstraints(Omni3DManager &omni3d) const {
	int16 iAlphaMin = -32768, iAlphaMax = 32767;
	bool alphaConstraint = false;

	omni3d.clearConstraints();
	for (Common::Array<Zone>::const_iterator it = zones.begin(); it != zones.end(); it++) {
		if (it->action == 100000) {
			int16 aMin = it->rct.left;
			if (aMin < 0) {
				aMin += 2048;
			}
			int16 aMax = aMin + it->rct.width();
			if (aMax > 2048) {
				aMax -= 2048;
			}
			// debug("x1=%d x2=%d", aMin, aMax);
			if (aMax < aMin) {
				int16 tmp = aMax;
				aMax = aMin;
				aMin = tmp;
			}
			if (alphaConstraint) {
				if (aMin < iAlphaMax && aMax > iAlphaMax) {
					iAlphaMax = aMax;
				}
				if (aMin < iAlphaMin && aMax > iAlphaMin) {
					iAlphaMin = aMin;
				}
			} else {
				iAlphaMin = aMin;
				iAlphaMax = aMax;
				alphaConstraint = true;
			}
		} else if (it->action == 200000) {
			double betaMin = ((int)it->rct.bottom - (768 / 2)) / 768. * M_PI;
			omni3d.setBetaMinConstraint(betaMin);
		} else if (it->action == 300000) {
			double betaMax = ((int)it->rct.top - (768 / 2)) / 768. * M_PI;
			omni3d.setBetaMaxConstraint(betaMax);
		}
	}
	if (alphaConstraint) {
		double alphaMin = (1 - iAlphaMin / 2048.) * M_PI * 2.;
		alphaMin += 75. / 180. * M_PI_2;
		if (alphaMin < 0.) {
			alphaMin += M_PI * 2.;
		} else if (alphaMin > M_PI * 2.) {
			alphaMin -= M_PI * 2.;
		}
		double alphaMax = (1 - iAlphaMax / 2048.) * M_PI * 2.;
		alphaMax -= 75. / 180. * M_PI_2;
		if (alphaMax < 0.) {
			alphaMax += M_PI * 2.;
		} else if (alphaMax > M_PI * 2.) {
			alphaMax -= M_PI * 2.;
		}
		omni3d.setAlphaConstraints(alphaMin, alphaMax);
	}
}

uint Place::hitTest(const Common::Point &point) const {
	for (Common::Array<Zone>::const_iterator it = zones.begin(); it != zones.end(); it++) {
		if (it->action) {
			if (it->rct.contains(point)) {
				return it->action;
			}
			if (it->rct.left < 0) {
				Common::Rect rct = it->rct;
				rct.translate(2048, 0);
				if (rct.contains(point)) {
					return it->action;
				}
			} else if (it->rct.right > 2048) {
				Common::Rect rct = it->rct;
				rct.translate(-2048, 0);
				if (rct.contains(point)) {
					return it->action;
				}
			}
		}
	}
	return 0;
}

const Transition *Place::findTransition(uint nextPlaceId) const {
	for (Common::Array<Transition>::const_iterator it = transitions.begin(); it != transitions.end();
	        it++) {
		if (it->dstId == nextPlaceId) {
			return it;
		}
	}
	return nullptr;
}

} // End of namespace CryOmni3D
