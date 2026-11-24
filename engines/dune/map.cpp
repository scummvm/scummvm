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

#include "common/textconsole.h"
#include "common/util.h"
#include "dune/detection.h"
#include "dune/map.h"
#include "dune/map_globe.h"

namespace Dune {

// Where the ramp of sixteen ground colours begins in the map screen's palette.
// The ramp the map screen paints the ground with.  A point of the world holds
// the ground in its low nibble, which runs from four for open sand to fifteen
// for bare rock, and ONMAP.HSQ sets the twelve colours those stand for from
// the twentieth of the palette on.
/*
 * What colour a point of the world is drawn in.  The low nibble of a point's
 * byte is its ground, running from four to fifteen and never one, two or
 * three, and ONMAP.HSQ sets twelve colours from the twentieth of the palette
 * for it - so the twelve fit the twelve exactly.
 *
 * Three hundred and ninety-two points of the world carry a nought there
 * instead, which is no ground at all; those fell outside the twelve and were
 * drawn in a colour the ramp never sets, which left black specks scattered
 * over the map and the little one watched from the air.
 */
static byte groundColour(byte of, byte ramp) {
	const byte ground = of & 0xf;

	return (byte)(ramp + (ground < 4 ? 4 : ground));
}

bool Map::load(Archive &archive, const char *worldName) {
	Resource table = archive.openMember("TABLAT.BIN");
	Resource world = archive.openMember(worldName);
	Resource driver = archive.openMember("DNVGA.HSQ");
	Resource globe = archive.openMember("GLOBDATA.HSQ");

	if (!table || !world) {
		warning("Dune: the world is missing its map %s", worldName);
		return false;
	}
	if (!driver || driver->size() < 0x2284 + (int)sizeof(_surveyEdges)) {
		warning("Dune: the world is missing its DNVGA polar edge asset");
		return false;
	}
	if (!globe || globe->size() != 16091) {
		warning("Dune: the world is missing its globe projection asset");
		return false;
	}

	_latitudes.clear();
	for (uint i = 0; i != kLatitudes; i++) {
		MapLatitude latitude;
		latitude.offset = table->readUint16BE();
		latitude.halfWidth = table->readUint16BE();
		latitude.step = table->readUint16BE();
		table->readUint16BE();

		_latitudes.push_back(latitude);
	}

	_world.resize(world->size());
	world->read(_world.begin(), _world.size());
	_baseWorld = _world;
	_globeGeometry.resize(globe->size());
	globe->read(_globeGeometry.begin(), _globeGeometry.size());
	// The driver is loaded at CS:0x100. Its polar edge asset is CS:0x8EF.
	driver->seek(0x7ef);
	driver->read(_strategicEdges, sizeof(_strategicEdges));
	driver->seek(0x2284);
	driver->read(_surveyEdges, sizeof(_surveyEdges));

	debugC(1, kDebugResource, "the world: %u latitudes, %u bytes", _latitudes.size(), _world.size());
	return true;
}

enum {
	kTroopsAt = 0x8aa,
	kTroopSize = 27,
	kTroops = 67,
	kPlacesAt = 0x100,
	kPlaceSize = 28,
	kEndOfPlaces = 0xff
};

void Map::readPlaces(const byte *state, uint stateSize) {
	_places.clear();

	for (uint at = kPlacesAt; at + kPlaceSize <= stateSize; at += kPlaceSize) {
		if (state[at] == kEndOfPlaces)
			break;

		Place place;
		place.type = state[at];
		place.id = state[at + 1];
		place.longitude = READ_LE_UINT16(state + at + 2);
		place.latitude = (int16)READ_LE_UINT16(state + at + 4);
		place.kind = state[at + 8];
		place.flags = state[at + 0xa];
		place.people = state[at + 0x12];
		for (uint i = 0; i != 6; i++)
			place.skills[i] = state[at + 0x14 + i];

		_places.push_back(place);
	}

	// 0x019E walks the places after MAP.HSQ has been unpacked and puts 0x40
	// in the terrain byte each one stands on.  The static map contains only
	// the terrain: without this start-up pass, walking back over even the
	// exact coordinates of the palace can never find it.  Clear the bit first
	// because this routine also runs after loading a saved game.
	for (uint i = 0; i != _world.size(); i++)
		_world[i] &= (byte)~0x40;

	for (uint i = 0; i != _places.size(); i++) {
		const int32 at = groundOffset(_places[i].longitude, _places[i].latitude);

		if (at >= 0)
			_world[(uint32)at] |= 0x40;
	}

	uint known = 0;
	for (uint i = 0; i != _places.size(); i++)
		known += _places[i].isKnown() ? 1 : 0;

	debugC(1, kDebugResource, "the world has %u places on it, %u of them known",
		   _places.size(), known);
}

void Map::resetTerrain(const byte *state, uint stateSize) {
	_world = _baseWorld;
	_vegetationSeed = 1;
	readPlaces(state, stateSize);
	// Older saves contain only the current footprint, not previous growth or
	// thinning. Reconstruct what they do retain; new saves keep the whole map.
	for (uint32 place = kPlacesAt; place + kPlaceSize <= stateSize; place += kPlaceSize) {
		if (state[place] == kEndOfPlaces)
			break;
		const byte radius = state[place + 0xb];
		if (!radius || radius > 12)
			continue;
		if (state[place + 0xa] & 1) {
			paintRegion(READ_LE_UINT16(state + place + 0xc), (int8)state[place + 0xe], radius);
			continue;
		}
		if (state[place + 8] >= 0x20 || (state[place + 0xa] & 0x88))
			continue;
		// Before discovery +0xB is a story gate. Revealing clears it; later
		// claims persist even after their troops have moved elsewhere.
		paintRegion(READ_LE_UINT16(state + place + 2),
				(int16)READ_LE_UINT16(state + place + 4), radius, 0x20);
	}
}

void Map::paintRegion(uint16 longitude, int latitude, uint radius, byte owner,
		VegetationPlaceCallback onPlace, void *context) {
	if (_latitudes.size() == kLatitudes)
		paintMapRegion(_world.begin(), _world.size(), _latitudes.begin(),
				longitude, latitude, radius, owner, onPlace, context, &_regionPaintState);
}

bool Map::decayVegetation() {
	bool changed;
	_vegetationSeed = decayMapVegetation(_world.begin(), _world.size(), _vegetationSeed, changed);
	return changed;
}

/** DOS 73B6: the palace falls; all remaining Harkonnen ground changes sides. */
void Map::conquerEnemyTerritory() {
	for (uint i = 0; i < _world.size(); i++) {
		if ((_world[i] & 0x30) == 0x30)
			_world[i] &= 0xef;
	}
}

bool Map::restoreTerrain(const Common::Array<byte> &world, uint16 seed) {
	if (world.size() != _baseWorld.size())
		return false;
	_world = world;
	_vegetationSeed = seed;
	return true;
}

/** DOS 55DD: MAP2's deposit IDs select matching enemy terrain in MAP. */
void Map::liberateSpiceDeposits(const Map &deposits, const bool *released) {
	liberateMapDeposits(_world.begin(), _world.size(), deposits._world.begin(), deposits._world.size(), released);
}

void Map::readTroops(const byte *state, uint stateSize) {
	_troops.clear();

	for (uint i = 0; i != kTroops; i++) {
		const uint at = kTroopsAt + i * kTroopSize;
		if (at + kTroopSize > stateSize)
			break;

		Troop troop;
		troop.id = state[at];
		troop.slot = state[at + 2];
		troop.state = state[at + 3];
		troop.place = READ_LE_UINT16(state + at + 4);
		troop.longitude = READ_LE_UINT16(state + at + 6);
		troop.latitude = (int16)READ_LE_UINT16(state + at + 8);
		troop.flags = READ_LE_UINT16(state + at + 0x10);
		troop.rate = state[at + 0x15];
		troop.skill = state[at + 0x16];
		troop.size = state[at + 0x1a];

		for (uint n = 0; n != 3; n++)
			troop.numbers[n] = state[at + 21 + n];

		_troops.push_back(troop);
	}

	uint known = 0;
	for (uint i = 0; i != _troops.size(); i++)
		known += _troops[i].isKnown() ? 1 : 0;

	debugC(1, kDebugResource, "%u troops, %u of them met", _troops.size(), known);
}

/**
 * Restores the part of the troop table which the original builds after it
 * has loaded MAP2.  0x101CB walks every place's linked list and 0x101E0 puts
 * the place record at troop +4, its longitude at +6 and its latitude at +8.
 * The byte at +0x12 also keeps the low nibble of the place type and which of
 * its three broad regions it belongs to.
 *
 * The initialized data in the executable precedes that start-up pass.
 */
void Map::restoreTroopLocations(byte *state, uint stateSize) {
	if (!state)
		return;
	uint restored = 0;

	for (uint place = kPlacesAt; place + kPlaceSize <= stateSize; place += kPlaceSize) {
		if (state[place] == kEndOfPlaces)
			break;

		byte number = state[place + 9];
		uint walked = 0;

		while (number != 0 && walked++ < kTroops) {
			const uint at = kTroopsAt + (number - 1) * kTroopSize;
			if (number > kTroops || at + kTroopSize > stateSize)
				break;

			WRITE_LE_UINT16(state + at + 4, (uint16)place);
			WRITE_LE_UINT16(state + at + 6, READ_LE_UINT16(state + place + 2));
			WRITE_LE_UINT16(state + at + 8, READ_LE_UINT16(state + place + 4));

			byte region = state[at + 0x12] & 0x70;
			const byte type = state[place];
			if ((type >= 4 && type <= 5) || type > 9)
				region ^= 0x80;
			state[at + 0x12] = (byte)(region | (type & 0x0f));
			restored++;

			number = state[at + 1];
		}
	}

	debugC(1, kDebugResource, "%u troop locations initialised", restored);
}

Common::Array<uint> Map::troopsAt(uint place, const byte *state, uint stateSize) const {
	Common::Array<uint> here;

	if (place + 0xa > stateSize)
		return here;

	byte number = state[place + 9];

	while (number != 0 && here.size() < kTroops) {
		const uint at = kTroopsAt + (number - 1) * kTroopSize;
		if (at + kTroopSize > stateSize)
			break;

		here.push_back(number - 1);
		number = state[at + 1];
	}

	return here;
}

uint Map::width(uint latitude) const {
	return latitude < _latitudes.size() ? 2u * _latitudes[latitude].halfWidth : 0;
}

uint Map::step(uint latitude) const {
	// DOS 00E7 builds DS:4880 from the latitude's circumference, rounding
	// 65536 / (2 * halfWidth). TABLAT's third word is instead a projection
	// scale: it shrinks poleward, while a world-coordinate travel step grows.
	return latitude < _latitudes.size() ? _latitudes[latitude].longitudeStep() : 256;
}

void Map::drawVegetation(uint16 longitude, int latitude, MapDecorationCallback draw, void *context) const {
	if (_latitudes.size() == kLatitudes)
		visitMapVegetation(_world.begin(), _world.size(), _latitudes.begin(), longitude, latitude, draw, context);
}

const byte *Map::row(uint latitude, bool farSide) const {
	if (latitude >= _latitudes.size())
		return nullptr;

	// The world is one strip from one pole to the other, and the offsets the
	// table keeps are counted from the equator out.  The rows above the
	// equator - which are the ones the map draws first - lie backwards from
	// kHalf, so that each of them ends where the next one down begins and the
	// equator comes last of all; the rows below it lie at kHalf and on, in the
	// order the table has them.  Reading both halves forwards laid the world
	// down a row at a time in the wrong place, which is what turned it into
	// streaks, and reading them the other way round turned the world upside
	// down: the Harkonnen ground the results globe paints blue then lay south
	// of the equator, where the recording has it north.
	const uint32 width = 2u * _latitudes[latitude].halfWidth;
	const uint32 from = _latitudes[latitude].offset;

	if (farSide && from + width > kHalf)
		return nullptr;

	const uint32 at = farSide ? kHalf - from - width : kHalf + from;

	return at + width <= _world.size() ? _world.begin() + at : nullptr;
}

void Map::drawFlat(byte *screen, int screenWidth, int screenHeight) const {
	if (_latitudes.empty())
		return;

	// The far side of the world above the equator, the near side below it,
	// each row laid on the middle of the screen.
	for (int y = 0; y != screenHeight; y++) {
		const int fromEquator = ABS(y - screenHeight / 2) * kLatitudes / (screenHeight / 2);
		if (fromEquator >= kLatitudes)
			continue;

		// The table counts latitudes from the equator out, so the middle of
		// the screen is its first row and the poles its last.
		const uint latitude = fromEquator;
		const byte *bytes = row(latitude, y < screenHeight / 2);
		if (!bytes)
			continue;

		const int rowWidth = MIN((int)width(latitude), screenWidth);
		byte *out = screen + y * screenWidth + (screenWidth - rowWidth) / 2;

		// The low nibble is the ground itself, which the map screen keeps a
		// ramp of sixteen colours for; the high nibble marks what is on it,
		// and is nothing over half the world.
		for (int x = 0; x != rowWidth; x++)
			out[x] = groundColour(bytes[x], kMapRamp);
	}
}

void Map::drawStretched(byte *screen, int screenWidth, int screenHeight, int scroll) const {
	if (_latitudes.empty())
		return;

	const int rows = 2 * kLatitudes;

	for (int y = 0; y != screenHeight; y++) {
		const int down = y * rows / screenHeight;

		// The far side of the world runs from its pole down to the equator,
		// and the near side from the equator on to its own pole.
		const bool farSide = down < kLatitudes;
		const uint latitude = farSide ? kLatitudes - 1 - down : down - kLatitudes;

		const byte *bytes = row(latitude, farSide);
		if (!bytes)
			continue;

		const uint stored = width(latitude);
		const uint step = _latitudes[latitude].step ? _latitudes[latitude].step : 256;

		byte *out = screen + y * screenWidth;

		for (int x = 0; x != screenWidth; x++) {
			const int around = (x + scroll) % kEquatorWidth;
			const uint at = MIN((uint)((around * step) >> 8), stored - 1);

			out[x] = groundColour(bytes[at], kMapRamp);
		}
	}
}

int32 Map::groundOffset(uint16 longitude, int16 latitude, int along) const {
	const uint rowIndex = ABS((int)latitude);
	if (rowIndex >= _latitudes.size())
		return -1;
	const MapLatitude &row = _latitudes[rowIndex];
	const uint stored = 2 * row.halfWidth;
	if (!stored)
		return -1;
	// DOS B58B: signed row offset about the equator, rounded longitude.
	// Neighbor scans are consecutive bytes, not clamped to the current row.
	const int start = (int)(_world.size() / 2) +
			(latitude < 0 ? -(int)row.offset : row.offset);
	const int at = start + (int)(((uint32)longitude * stored + 0x8000) >> 16) + along;
	return at >= 0 && (uint)at < _world.size() ? at : -1;
}

uint16 Map::snapLongitude(uint16 longitude, int16 latitude) const {
	const uint row = ABS((int)latitude);
	const uint stored = width(row);
	if (!stored)
		return longitude;
	const uint pixel = ((uint32)longitude * stored + 0x8000) >> 16;
	return (uint32)pixel * 65536u / stored; // DOS B5C5 after B58B.
}

byte Map::groundAt(uint16 longitude, int16 latitude, int along) const {
	const int32 at = groundOffset(longitude, latitude, along);

	return at >= 0 ? _world[(uint32)at] : 0;
}

uint32 Map::frequency(byte value) const {
	uint32 count = 0;

	for (uint i = 0; i != _world.size(); i++)
		count += _world[i] == value ? 1 : 0;

	return count;
}

uint32 Map::territoryPoints(byte owner) const {
	uint32 count = 0;
	owner &= 0x30;

	for (uint i = 0; i != _world.size(); i++)
		count += (_world[i] & 0x30) == owner ? 1 : 0;

	return count;
}

uint32 Map::controllablePoints() const {
	uint32 count = 0;

	for (uint i = 0; i != _world.size(); i++)
		count += (_world[i] & 0x0f) != 0 ? 1 : 0;

	return count;
}

/** DOS BFE3: results-view territory shares from the two ownership bits. */
void Map::resultsShares(uint16 &harkonnen, uint16 &atreides) const {
	enum { kMapBytes = 0xc5f9, kArea = kMapBytes - 0x188 + 1 };
	harkonnen = 0;
	atreides = 1;
	if (_world.size() < kMapBytes)
		return;

	uint32 harkPoints = 0, ourPoints = 0;
	for (uint i = 0; i < kMapBytes; i++) {
		const byte owner = _world[i] & 0x30;
		if (owner == 0x30)
			harkPoints++;
		else if (owner != 0)
			ourPoints++; // Both 0x10 and 0x20 belong to the Atreides total.
	}
	// BFE3 divides the count as a 16.16 fraction and multiplies the whole
	// percentage by 100. Its ADD AX,AX only carries the top bit of the low
	// word into DX, rounding the high word; it does not double the percentage.
	const uint32 harkScaled = ((harkPoints << 16) / kArea) * 100;
	const uint32 ourScaled = ((ourPoints << 16) / kArea) * 100;
	harkonnen = (uint16)((harkScaled >> 16) + ((harkScaled & 0x8000) != 0));
	atreides = (uint16)((ourScaled >> 16) + ((ourScaled & 0x8000) != 0) + 1);
}

int Map::placeAt(uint16 longitude, int16 latitude) const {
	const int32 here = groundOffset(longitude, latitude);

	if (here < 0 || (_world[(uint32)here] & 0x40) == 0)
		return -1;

	for (uint i = 0; i != _places.size(); i++) {
		if (groundOffset(_places[i].longitude, _places[i].latitude) == here)
			return (int)i;
	}

	return -1;
}

void Map::drawStrategic(byte *screen, int pitch, uint16 longitude, int latitude) const {
	if (_latitudes.size() == kLatitudes && !_world.empty())
		drawStrategicMap(screen, pitch, _world.begin(), _world.size(),
				_latitudes.begin(), _strategicEdges, longitude, latitude);
}

Common::Point Map::strategicPosition(uint16 centreLongitude, int centreLatitude,
		uint16 longitude, int latitude) const {
	if (_latitudes.size() != kLatitudes)
		return Common::Point(-32768, -32768);
	return strategicMapPosition(_latitudes.begin(), centreLongitude, centreLatitude, longitude, latitude);
}

void Map::drawTerrainInset(byte *screen, int pitch, uint16 longitude, int latitude,
		const Common::Rect &view, byte *scratch, uint scratchSize) const {
	if (_latitudes.size() == kLatitudes && !_world.empty())
		Dune::drawTerrainInset(screen, pitch, _world.begin(), _world.size(),
				_latitudes.begin(), _surveyEdges, longitude, latitude, view, scratch, scratchSize);
}

void Map::drawWindow(byte *screen, int screenWidth, int dstX, int dstY,
					 int across, int down_, int scrollX, int scrollY, int zoom,
					 byte ramp) const {
	if (_latitudes.empty() || zoom < 1)
		return;

	for (int y = 0; y != down_; y++) {
		const int down = scrollY + y / zoom;
		if (down < 0 || down >= kMapRows)
			continue;

		// The far half of the world runs from its pole down to the equator and
		// the near half from the equator on to its own pole.
		const bool farSide = down < kLatitudes;
		const uint latitude = farSide ? kLatitudes - 1 - down : down - kLatitudes;

		const byte *bytes = row(latitude, farSide);
		if (!bytes)
			continue;

		const uint stored = width(latitude);
		const uint step = _latitudes[latitude].step ? _latitudes[latitude].step : 256;

		byte *out = screen + (dstY + y) * screenWidth + dstX;

		for (int x = 0; x != across; x++) {
			const int around = ((scrollX + x / zoom) % kEquatorWidth + kEquatorWidth) % kEquatorWidth;
			const uint at = MIN((uint)((around * step) >> 8), stored - 1);

			out[x] = groundColour(bytes[at], ramp);
		}
	}
}

/**
 * Draws MAP2's full spice-density bytes. Unlike MAP.HSQ terrain, these are a
 * deposit identifiers from zero through seventy-six, not flags plus a
 * four-bit ground number. Folding them through groundColour() repeated the
 * palette every sixteen identifiers and turned neighbouring surveyed areas
 * into unrelated patches. The caller maps each deposit to its current survey
 * colour using the matching place record.
 */
void Map::drawSurvey(byte *screen, int pitch, uint16 longitude, int latitude,
		const Common::Rect &view, const byte *colours, bool troopMode, byte *scratch, uint scratchSize) const {
	if (_latitudes.size() == kLatitudes && !_world.empty())
		drawSurveyMap(screen, pitch, _world.begin(), _world.size(), _latitudes.begin(),
			_surveyEdges, colours, longitude, latitude, view, troopMode, scratch, scratchSize);
}

Common::Point Map::surveyPosition(uint16 centreLongitude, int centreLatitude,
		uint16 longitude, int latitude, const Common::Rect &view) const {
	if (_latitudes.size() != kLatitudes)
		return Common::Point(-32768, -32768);
	return surveyMapPosition(_latitudes.begin(), centreLongitude, centreLatitude, longitude, latitude, view);
}

/**
 * B977 calls DNVGA 145h through SS:3911. Its geometry stream comes from
 * GLOBDATA.HSQ, and BA15/B9E0 derive tilt and longitude tables from the
 * current globe controls. The driver writes a 128-wide oval in this rectangle.
 */
void Map::drawGlobe(byte *screen, int screenWidth, int left, int top,
					int wide, int high, int spin, int tilt, byte ramp,
					bool showTerritories) const {
	if (_latitudes.size() != kLatitudes || _globeGeometry.size() != 16091 ||
			wide != 129 || high != 110 || ramp != kMapRamp)
		return;
	drawDosGlobe(screen, screenWidth, left, top, _world.begin(), _world.size(),
			_latitudes.begin(), _globeGeometry.begin(), _globeGeometry.size(),
			spin, tilt, showTerritories);
}

bool Map::stepGlobe(byte *screen, int spin, int tilt, bool results, GlobeRenderState &progress) const {
	if (_latitudes.size() != kLatitudes || _globeGeometry.size() != 16091)
		return false;
	return drawDosGlobe(screen, 320, 96, 25, _world.begin(), _world.size(),
			_latitudes.begin(), _globeGeometry.begin(), _globeGeometry.size(), spin, tilt, results, &progress);
}

bool Map::validGlobeProgress(const GlobeRenderState &progress) const {
	return validGlobeRenderState(progress, _globeGeometry.begin(), _globeGeometry.size());
}

Common::Point Map::globeMarkerPosition(uint16 longitude, int latitude,
		int spin, int tilt) const {
	if (_latitudes.size() != kLatitudes || _globeGeometry.size() != 16091)
		return Common::Point(-1, -1);
	return projectDosGlobeMarker(_latitudes.begin(), _globeGeometry.begin(),
			_globeGeometry.size(), spin, tilt, longitude, latitude);
}

bool Map::globeWorldPosition(const Common::Point &point, int spin, int tilt,
		uint16 &longitude, int &latitude) const {
	if (_latitudes.size() != kLatitudes || _globeGeometry.size() != 16091)
		return false;
	return unprojectDosGlobePoint(_latitudes.begin(), _globeGeometry.begin(),
			_globeGeometry.size(), spin, tilt, point, longitude, latitude);
}

} // End of namespace Dune
