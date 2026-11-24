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

#ifndef DUNE_MAP_H
#define DUNE_MAP_H

#include "common/array.h"

#include "dune/archive.h"
#include "dune/map_region.h"
#include "dune/map_renderer.h"

namespace Dune {

struct GlobeRenderState;

/**
 * Arrakis itself.  The terrain and latitude rows are kept in two files:
 *
 *   TABLAT.BIN  99 records of eight bytes, big endian, one to a latitude:
 *                 uint16 offset;     // where the row's bytes begin
 *                 uint16 halfWidth;  // half the row, 199 at the equator
 *                                    // down to 3 at the pole
 *                 uint16 step;       // how fast the row is walked when it
 *                                    // is drawn, in eighths of a pixel
 *                 uint16 unused;
 *
 *   MAP.HSQ     the rows themselves, a byte to a point: the near half of the
 *               world from the equator out to the pole, 25340 bytes of it,
 *               and then the far half the same way.
 *
 * GLOBDATA.HSQ supplies the globe view's scanline/radial lookup over these
 * rows. DNVGA 145h draws the resulting 129x110 oval.
 */
/**
 * A place on Arrakis: a sietch, a village, the palace.  The game keeps
 * seventy of them at DS:0x100, 28 bytes each, ended by 0xFF where a type
 * would be.  What is understood of a record:
 *
 *   byte   type;      // 1 to 12
 *   byte   id;
 *   uint16 longitude; // right round the world in 65536
 *   int16  latitude;  // degrees, north of the equator or south of it
 *   uint16 at;        // the point of the map it stands on, filled in as the
 *                     // game starts
 *   ...
 */
struct Place {
	byte   type;
	byte   id;
	uint16 longitude;
	int16  latitude;
	byte   kind;       // at +8, which sort of place it is: a sietch under
	                   // 0x20, the palace at 0x20, a village under 0x28, a
	                   // fort under 0x30 and a Harkonnen place above it
	byte   flags;      // 0x80 while the place is still to be found
	byte   people;     // how many are there, at +0x12
	byte   skills[6];  // what they are good at, at +0x14: the six tasks the
	                   // interface names, from spice mining to bulb growing

	bool isKnown() const { return (flags & 0x80) == 0; }
};

/**
 * A troop of Fremen.  Sixty seven of them follow the places in the game's
 * memory, from DS:0x8AA to DS:0xFBB, 27 bytes each and numbered from one -
 * which is the stretch the hourly round at 0x6C8F walks, a troop at a time.
 *
 * The byte at +3 is what the troop is doing: 0x80 while it has still to be
 * met, 0x40 and 0x20 for states of its own, and the low nibble the order it
 * has been given.  The round takes that nibble, doubles it, and calls what it
 * finds in the table at 0x6C26 - and the routine for order 0 is the one that
 * produces from the ground, which is why order 0 is spice mining.  The
 * command file names them from its 35th line on.
 */
struct Troop {
	byte id;
	byte slot;         // at +2, which of a place's sixteen standing points the
	                   // troop's figure is put on
	byte state;
	byte numbers[3];

	byte rate;         // at +0x15, what the troop is worth at its work
	byte skill;        // at +0x16, its high nibble counting towards what it
	                   // makes
	byte size;         // at +0x1A, in tens of men
	uint16 flags;      // at +0x10; bit 0x80 belongs to non-Fremen troops
	uint16 place;      // at +4, the place it is at
	uint16 longitude;  // at +6, used while bit 0x40 says it is travelling
	int16 latitude;    // at +8

	bool isKnown() const { return (state & 0x80) == 0; }
	bool isOurs() const { return isKnown() && (flags & 0x80) == 0; }
	byte order() const { return state & 0xf; }
};

class Map {
public:
	enum {
		kLatitudes = 99,
		kEquatorWidth = 398,
		kHalf = 25340
	};

	bool load(Archive &archive, const char *worldName = "MAP.HSQ");

	// The places, read out of the game's own memory.
	void readPlaces(const byte *state, uint stateSize);
	const Common::Array<Place> &places() const { return _places; }

	// A fresh world or a pre-terrain-save checkpoint needs reconstruction.
	// Ordinary place-cache refreshes must not erase ownership/vegetation.
	void resetTerrain(const byte *state, uint stateSize);
	void paintRegion(uint16 longitude, int latitude, uint radius, byte owner = 0,
			VegetationPlaceCallback onPlace = nullptr, void *context = nullptr);
	bool decayVegetation();
	void conquerEnemyTerritory();
	void liberateSpiceDeposits(const Map &deposits, const bool *released);
	const Common::Array<byte> &terrain() const { return _world; }
	uint16 vegetationSeed() const { return _vegetationSeed; }
	bool restoreTerrain(const Common::Array<byte> &world, uint16 seed);
	int32 groundOffset(uint16 longitude, int16 latitude, int along = 0) const;
	uint16 snapLongitude(uint16 longitude, int16 latitude) const;

	void readTroops(const byte *state, uint stateSize);
	// The executable's map start-up pass walks every place's troop chain and
	// writes that place and its coordinates into each troop record.
	void restoreTroopLocations(byte *state, uint stateSize);
	const Common::Array<Troop> &troops() const { return _troops; }

	/**
	 * The troops standing at a place.  A place keeps the number of its first
	 * troop in the byte at +9, and each troop the number of the next at +1,
	 * which is the chain the walks at 0x6603 and 0x661D follow.  A number of
	 * zero ends it, and a troop's record lies at 0x8AA + 27 * (n - 1).
	 */
	Common::Array<uint> troopsAt(uint place, const byte *state, uint stateSize) const;

	// The width of a latitude, and its bytes.  A latitude is counted from the
	// equator out; the far side of the world is the same latitudes again.
	uint width(uint latitude) const;
	// The amount by which one movement substep changes longitude at this
	// latitude: DOS 00E7's rounded reciprocal of TABLAT's circumference,
	// used by the troop-travel routine at 0x8604 and contact range at 0x7C63.
	uint step(uint latitude) const;
	void drawVegetation(uint16 longitude, int latitude, MapDecorationCallback draw, void *context) const;
	const byte *row(uint latitude, bool farSide) const;

	/**
	 * The byte of the world a place's own longitude and latitude fall on: the
	 * low nibble is the ground and 0x40 in the high one says a place stands
	 * there, which is how walking out into the desert finds the next place.
	 * Answers nought where the point is off the world.
	 */
	// `along` steps that many consecutive stored bytes, which is how
	// 0x385E looks at the points either side of the player.
	byte groundAt(uint16 longitude, int16 latitude, int along = 0) const;

	// How many points of this map carry one exact byte.  The MAP2 start-up
	// pass uses this distribution to give each place its depletion divisor.
	uint32 frequency(byte value) const;
	// Territory lives in bits 4 and 5 of the terrain byte. Exact-bit counts
	// are useful elsewhere; DOS BFE3's results share instead treats both
	// 0x10 and 0x20 as Atreides and 0x30 as Harkonnen ownership.
	uint32 territoryPoints(byte owner) const;
	uint32 controllablePoints() const;
	void resultsShares(uint16 &harkonnen, uint16 &atreides) const;

	// The place whose marked terrain point contains these coordinates, or -1.
	// Places occupy a point of MAP.HSQ, not one otherwise invisible sub-pixel
	// longitude within that point.
	int placeAt(uint16 longitude, int16 latitude) const;

	// Lays the world out flat, the way it is stored: the poles at the top and
	// bottom and the equator across the middle.
	void drawFlat(byte *screen, int width, int height) const;

	/**
	 * Draws the world as the map screen shows it: every latitude stretched to
	 * the same width, which is what the step in its record is for - it is how
	 * far along the stored row to move for each pixel drawn, in 256ths.  The
	 * whole world is 398 pixels round, so it is scrolled past the screen.
	 */
	void drawStretched(byte *screen, int width, int height, int scroll) const;
	void drawStrategic(byte *screen, int pitch, uint16 longitude, int latitude) const;
	Common::Point strategicPosition(uint16 centreLongitude, int centreLatitude,
			uint16 longitude, int latitude) const;

	/**
	 * A window on the world, drawn at so many screen pixels to the point,
	 * which is what the map screen shows: the original does not put the whole
	 * globe up but a piece of it around where the player is, coarse enough
	 * that a point of the world covers more than one pixel.  The world runs
	 * 398 points round and 198 down, the far half above the equator and the
	 * near half below it, and it wraps round the sides.
	 */
	// Where the twelve colours ONMAP.HSQ sets for the ground begin, less the
	// four the ground's own numbering starts at.
	enum { kMapRamp = 16, kSkyRamp = 80 };

	void drawWindow(byte *screen, int screenWidth, int dstX, int dstY,
					int across, int down, int scrollX, int scrollY, int zoom,
					byte ramp = kMapRamp) const;
	void drawTerrainInset(byte *screen, int pitch, uint16 longitude, int latitude,
					const Common::Rect &view, byte *scratch = nullptr, uint scratchSize = 0) const;
	void drawSurvey(byte *screen, int pitch, uint16 longitude, int latitude,
		const Common::Rect &view, const byte *colours, bool troopMode = false,
		byte *scratch = nullptr, uint scratchSize = 0) const;
	Common::Point surveyPosition(uint16 centreLongitude, int centreLatitude,
			uint16 longitude, int latitude, const Common::Rect &view) const;
	// Approximate the centre of pre-25 saves' obsolete flat crop, without
	// resampling or changing any place/troop coordinates.

	// The world as a ball, which is what the planet in the panel opens on.
	void drawGlobe(byte *screen, int screenWidth, int left, int top,
				   int wide, int high, int spin, int tilt, byte ramp = kMapRamp,
				   bool showTerritories = false) const;
	Common::Point globeMarkerPosition(uint16 longitude, int latitude,
			int spin, int tilt) const;
	bool stepGlobe(byte *screen, int spin, int tilt, bool results, GlobeRenderState &progress) const;
	bool validGlobeProgress(const GlobeRenderState &progress) const;
	bool globeWorldPosition(const Common::Point &point, int spin, int tilt,
			uint16 &longitude, int &latitude) const;

	enum {
		// The whole world laid out as the map screen counts it: the far pole
		// at nought, the equator at the middle and the near pole at the foot.
		kMapRows = 2 * kLatitudes,
		kEquator = kLatitudes
	};

private:
	Common::Array<Place> _places;
	Common::Array<Troop> _troops;
	Common::Array<MapLatitude> _latitudes;
	byte _strategicEdges[120] = {};
	byte _surveyEdges[18] = {};
	Common::Array<byte> _globeGeometry;
	Common::Array<byte> _baseWorld;
	Common::Array<byte> _world;
	MapRegionPaintState _regionPaintState;
	uint16 _vegetationSeed = 1; // DOS CS:65B4, separate from the gameplay RNG.
};

} // End of namespace Dune

#endif
