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

#ifndef CRYOMNI3D_ATLANTIS_WAM_PARSER_H
#define CRYOMNI3D_ATLANTIS_WAM_PARSER_H

#include "common/array.h"
#include "common/rect.h"
#include "common/str.h"

namespace Common {
class SeekableReadStream;
}

namespace CryOmni3DEngine {
class Omni3DManager;
}

namespace CryOmni3D {
namespace Atlantis {

// A clickable zone on the panoramic view.  Coordinates are in panorama space
// (x: 0-2047, y: 0-767). action is the place ID to transition to.
struct AtlantisZone {
	int16 x, y, w, h;
	int16 action;
};

// A transition between two places.  Plays a UBB/HNM video then loads dstPlaceId.
struct AtlantisTransition {
	Common::String videoName;  // e.g. "at001013.UBB"
	uint16 dstPlaceId;
	uint8 flags;               // 0x01 = normal transition
	float srcAlpha, srcBeta;   // camera angle when leaving source
	float dstAlpha, dstBeta;   // camera angle when entering destination
};

// A sub-WAM reference: cross-level transition pointing to another WAM file.
struct AtlantisSubWAM {
	uint16 srcPlaceId;
	uint16 dstPlaceId;
	Common::String wamName;    // e.g. "atlan2.wam"
};

// An NPC entry from the WAM tail: identifies a character active in this WAM area.
struct AtlantisNPCEntry {
	uint32 persoId;            // character ID referenced by CON scripts as ClicPerso
	Common::String spriteFile; // canonical SPW file (others found via SPWATL*.TXT)
};

struct AtlantisPlace {
	uint16 placeId;
	Common::String cycloHnm;   // panoramic view HNM file, e.g. "atl16001.hnm"
	Common::Array<AtlantisTransition> transitions;
	Common::Array<AtlantisZone> zones;

	// Returns the transition to dstPlaceId, or nullptr.
	const AtlantisTransition *findTransition(uint16 dstPlaceId) const;

	// Returns the action for the point (panorama coordinates), or 0.
	int16 hitTest(int16 x, int16 y) const;
};

class AtlantisWAMParser {
public:
	void loadStream(Common::SeekableReadStream &stream);
	void clear();

	const AtlantisPlace *findPlaceById(uint16 placeId) const;
	const Common::Array<AtlantisSubWAM> &getSubWAMs() const { return _subWAMs; }
	const Common::Array<AtlantisPlace> &getPlaces() const { return _places; }
	const Common::Array<AtlantisNPCEntry> &getNPCEntries() const { return _npcEntries; }

	// Sprite and view list filenames embedded in the WAM.
	Common::String spwListFile;   // e.g. "spwatl1.txt"
	Common::String spfListFile;   // e.g. "spfatl1.txt"
	Common::String centVueFile;   // e.g. "atl1cent.vue"

private:
	Common::Array<AtlantisPlace> _places;
	Common::Array<AtlantisSubWAM> _subWAMs;
	Common::Array<AtlantisNPCEntry> _npcEntries;
};

} // namespace Atlantis
} // namespace CryOmni3D

#endif // CRYOMNI3D_ATLANTIS_WAM_PARSER_H
