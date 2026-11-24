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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "dune/dune.h"
#include "dune/clock.h"
#include "dune/flight_navigation.h"
#include "dune/resources.h"
#include "dune/video.h"
#include "common/endian.h"
#include "common/events.h"
#include "common/system.h"

namespace Dune {

/** Native61 retains the decoded landscape, not a recomposed screenshot. */
void DuneEngine::saveFlightMovie(Common::WriteStream *stream) {
	const bool active = _flyingTo >= 0 && (_state[0x11c9] & 3) &&
		_flightMovie >= HNM_DFL2 && _flightMovie <= HNM_MNT4 && _approachMovie < 0;
	const bool inset = active && _inDesert && !_onWorld && !_onGlobe && !_onMixer && !_inCabin;
	stream->writeUint32BE(0x464c4d31); // FLM1, before FIN1/SKY1.
	stream->writeByte(active ? 1 | (_flightMovieFallback ? 2 : 0) | (_flightFilmClockSet ? 4 : 0) | (inset ? 8 : 0) : 0);
	if (active) {
		const uint32 now = isPaused() ? _flightStepPauseTime : _system->getMillis();
		const uint32 tick = isPaused() ? _flightFilmPauseTime : now / 5;
		stream->writeUint32LE(_flightFrame);
		stream->writeByte(_flightMovie);
		stream->writeUint32LE(_flightMovieFallback ? 0 : _video->_currentFrame);
		stream->writeUint32LE(_flightFilmClockSet ? tick - _flightFilmTime : 0);
		stream->writeUint32LE(_flightLoopLastFrame);
		stream->writeUint32LE(FlightStepClock::remaining(now, _nextFlightStep));
		if (!_flightMovieFallback)
			stream->write(_video->_frameBuffer, kScreenWidth * kSceneHeight);
		// 4F5C may update the shared map cache after the last 4B2B copy.
		// Retain the DISPLAYED rectangle separately; do not replace DS's
		// newer cache or show its next marker prematurely while loading.
		if (inset)
			for (uint y = 0; y < 64; y++)
				stream->write(_framebuffer1 + y * kScreenWidth + 200, 120);
	}
	stream->writeUint16LE(active ? 28 + (_flightMovieFallback ? 0 : kScreenWidth * kSceneHeight) + (inset ? 7680 : 0) : 7);
}

bool DuneEngine::loadFlightMovie(Common::SeekableReadStream *stream, int64 end, uint32 &elapsed, uint32 &stepRemaining,
		Common::Array<byte> &presentedInset) {
	const int64 start = stream->pos();
	if (end - start < 7 || stream->readUint32BE() != 0x464c4d31)
		return false;
	const byte flags = stream->readByte();
	if (!flags)
		return stream->readUint16LE() == 7 && !stream->err();
	if (flags != 5 && flags != 7 && flags != 13 && flags != 15)
		return false;
	const bool fallback = (flags & 2) != 0;
	const bool inset = (flags & 8) != 0;
	const uint size = 28 + (fallback ? 0 : kScreenWidth * kSceneHeight) + (inset ? 7680 : 0);
	if (end - start < size || _flyingTo < 0 || !(_state[0x11c9] & 3) ||
			(inset && (!_inDesert || _onWorld || _onGlobe || _onMixer || _inCabin)))
		return false;
	const uint32 animation = stream->readUint32LE();
	const byte movie = stream->readByte();
	const uint32 frame = stream->readUint32LE();
	elapsed = stream->readUint32LE();
	const uint32 last = stream->readUint32LE();
	stepRemaining = stream->readUint32LE();
	if (movie > HNM_MNT4 || stepRemaining > kFlightStepMs ||
			READ_LE_UINT16(_state.begin() + 0xdc00) != movie + 1 || (fallback && frame))
		return false;
	if (!fallback) {
		Resource reader = _archive.openMember(getResourceFilenameByIndex(INDEX_FIRST_HNM + movie));
		if (!reader)
			return false;
		_video->setReader(reader);
		_video->setInterlace(false);
		_video->start();
		_graphics.setFrameBuffer(_framebuffer1);
		if (!frame || frame > (uint)_video->_frameCount)
			return false;
		// decodeAVFrame seeks by its next-frame index. Restoring its retained
		// page avoids replaying old frames, palettes, audio or callbacks.
		if (stream->read(_video->_frameBuffer, kScreenWidth * kSceneHeight) != kScreenWidth * kSceneHeight)
			return false;
		_video->_currentFrame = frame;
		_video->_done = frame == (uint)_video->_frameCount;
	}
	if (inset) {
		presentedInset.resize(7680);
		if (stream->read(presentedInset.begin(), presentedInset.size()) != presentedInset.size())
			return false;
	}
	if (stream->readUint16LE() != size || stream->err() || stream->pos() - start != size)
		return false;
	_flightFrame = animation;
	_flightMovie = movie;
	_flightMovieFallback = fallback;
	_flightLoopLastFrame = last;
	_flightFilmClockSet = true;
	return true;
}

byte DuneEngine::flightFramePeriod() const {
	if (_flightMovie < 0)
		return 0;
	// C921/C93C: the descriptor begins with flags and a byte tick period.
	const uint table = 0x33a3 + 2 * (_flightMovie + 1);
	if (table + 2 > _state.size())
		return 0;
	const uint descriptor = READ_LE_UINT16(_state.begin() + table);
	return descriptor + 2 <= _state.size() ? _state[descriptor + 1] : 0;
}

bool DuneEngine::flightFrameDue(uint32 tick) const {
	return !_flightFilmClockSet || silentMovieFrameDue(_flightFilmTime, tick, flightFramePeriod());
}

void DuneEngine::updateFlightMovie() {
	if (!_inDesert || _flyingTo < 0 || _flightDiscoveryPrompt || _state[0x11ca] || _onWorld)
		return;
	// 4F09/CA60/CAD4 samples the film clock separately from D9D2's BX.
	if (flightFrameDue(_system->getMillis() / 5)) {
		// CA80 skips CE3B when the movie callback DC1E is FFFF, as it is
		// for flight. Terrain forecasting belongs to the 4FAD movement
		// tail, not every decoded frame: it can also change the bearing.
		if (_flightFilmClockSet)
			_flightFrame++;
		drawFlightMovieFrame(true);
		drawDesert();
	}
}

/** 4F0C: shared ordinary-poll and forward-button movement gate. */
void DuneEngine::pollFlightMovement() {
	if (!_inDesert || _flyingTo < 0 || _flightDiscoveryPrompt || _state[0x11ca] || _onWorld)
		return;
	updateFlightMovie(); // CA60 precedes the separate CE7A sample at 4F27.
	const uint32 now = _system->getMillis();
	if (!FlightStepClock::due(now, _nextFlightStep))
		return;
	_nextFlightStep = FlightStepClock::next(now);
	WRITE_LE_UINT16(_state.begin() + 0x4729, (uint16)(now / 5));
	// 4F3A moves before 4F49 tests the destination. Do not land on the
	// pre-movement cell, and retain the current input word for discoveries.
	stepThroughDesert(headingWay());
}

void DuneEngine::setFlightCourse() {
	if (_flyingTo >= 0)
		prepareFlightCourse(_flyingTo);
}

/** 407E: room positions use the current place's world coordinates. */
bool DuneEngine::playerWorldPosition(uint16 &longitude, int16 &latitude) const {
	if (_state.size() <= 0x114f)
		return false;
	if (_state[6] == 0x80) {
		const uint here = READ_LE_UINT16(_state.begin() + 0x114e);
		if (here < 0x100 || here + 6 > _state.size())
			return false;
		longitude = READ_LE_UINT16(_state.begin() + here + 2);
		latitude = READ_LE_INT16(_state.begin() + here + 4);
	} else {
		longitude = READ_LE_UINT16(_state.begin() + 4);
		latitude = (int8)_state[6];
	}
	return true;
}

void DuneEngine::prepareFlightCourse(uint destination) {
	if (_state.size() <= 0x11cd || destination >= _map.places().size())
		return;
	uint16 longitude;
	int16 latitude;
	if (!playerWorldPosition(longitude, latitude))
		return;
	const Place &to = _map.places()[destination];
	if (!_flightToPoint && !_flightSteered)
		WRITE_LE_UINT16(_state.begin() + 0x11c5, 0x100 + 28 * destination);
	flightBearing(longitude, latitude,
			_flightToPoint ? _flightTargetLongitude : to.longitude,
			_flightToPoint ? _flightTargetLatitude : to.latitude, _flightHeading);
	_state[0x11c7] = _flightHeading;
	_state[0x11c8] = (_flightToPoint || _flightSteered) ? 1 : 0;
	WRITE_LE_UINT16(_state.begin() + 0x11cc, 128);
	_nextFlightTurn = 0;
}

// The flight controls have the DOS 0x4000 held-button flag. A single press
// turns four bearing units; holding a side arrow repeats that same turn.
void DuneEngine::updateFlightSteering() {
	if (!_inDesert || _flyingTo < 0 || _onWorld || _onGlobe || _inCabin ||
			_onMixer || _menuKind != kMenuDesert || _flightDiscoveryPrompt ||
			!_pointerButtons.down()) {
		_nextFlightTurn = 0;
		return;
	}
	const int button = compassButtonAt(_system->getEventManager()->getMousePos());
	if (button != 1 && button != 3) {
		_nextFlightTurn = 0;
		return;
	}
	const uint32 now = _system->getMillis();
	if (!_nextFlightTurn)
		_nextFlightTurn = now + 100;
	else if ((int32)(now - _nextFlightTurn) >= 0)
		turnFlight(button == 1 ? -4 : 4);
}

/** 4E8E: forecast terrain without moving Paul or consuming his fraction. */
void DuneEngine::selectFlightTerrain() {
	if (_flyingTo < 0 || !(_state[0x11c9] & 3) || _approachMovie >= 0 || _state[0x11ca])
		return;
	const uint16 realLongitude = READ_LE_UINT16(_state.begin() + 4);
	const int16 realLatitude = (int8)_state[6];
	uint16 longitude = realLongitude;
	int16 latitude = realLatitude;
	byte fraction = _state[0x11cc], heading = _flightHeading;
	const bool manual = _state[0x11cb] || _state[0x11c8];
	const uint target = READ_LE_UINT16(_state.begin() + 0x11c5);
	byte sixth = 0, seventh = 0;
	for (uint step = 0; step < 7; step++) {
		// 51F5/5124 reads Paul's REAL point for each automatic bearing,
		// not the projected DX/BX which 5206 advances seven times.
		if (!manual && target >= 0x100 && target + 6 < _state.size())
			flightBearing(realLongitude, realLatitude,
					READ_LE_UINT16(_state.begin() + target + 2),
					(int16)READ_LE_UINT16(_state.begin() + target + 4), heading);
		advanceFlightPosition(longitude, latitude, heading, fraction,
				_map.step(ABS((int)latitude)), manual);
		if (step == 5)
			sixth = _map.groundAt(longitude, latitude);
		if (step == 6)
			seventh = _map.groundAt(longitude, latitude);
	}
	// 4EB4 restores 11CC, but NOT 11C7. Forecasting may refresh an
	// automatic bearing or deflect a manually steered course near a pole.
	_flightHeading = heading;
	_state[0x11c7] = heading;
	uint16 base = READ_LE_UINT16(_state.begin() + 0x487e);
	const uint16 wanted = flightTerrainMovie(sixth, seventh, base,
			READ_LE_UINT16(_state.begin() + 0xdc00));
	WRITE_LE_UINT16(_state.begin() + 0x487e, base);
	WRITE_LE_UINT16(_state.begin() + 0xdc02, wanted);
}

bool DuneEngine::prepareCarriedRoomRequests() {
	// 35E9: carried polls discard old request context and consume the
	// one-shot inhibit latch before either discoveries or hazards run.
	_state[0x1a] = _state[0x47a7] = _state[0x23] = 0;
	const byte inhibited = _state[0x47a6];
	_state[0x47a6] = 0;
	return !inhibited;
}

/** 2E52/35E9: scan both the initial carried scene and subsequent moves. */
bool DuneEngine::dispatchCarriedRoomRequests() {
	if (_flyingTo < 0 || !prepareCarriedRoomRequests())
		return false;
	const uint16 longitude = READ_LE_UINT16(_state.begin() + 4);
	const int16 latitude = (int8)_state[6];
	bool spotted = false;
	uint cell = 0;
	int sighted;
	while (READ_LE_UINT16(_state.begin() + 0x1152) != 0xffff &&
			(sighted = nextFlightSighting(cell)) >= 0) {
		// 40F9/425B reveals the place when spotted, not only on landing.
		_guidedFlightPlace = _flyingTo = sighted;
		byte bearing = _flightHeading;
		const Place &place = _map.places()[sighted];
		flightBearing(longitude, latitude, place.longitude, place.latitude, bearing);
		_state[0xe1] = (byte)(bearing - _flightHeading + 96) >= 96;
		WRITE_LE_UINT16(_state.begin() + 0x11f5, 206 + 2 * (_state[0xe1] != 0));
		WRITE_LE_UINT16(_state.begin() + 0x11f3, 72 + MIN(placeClass(place.kind), 3));
		_state[0x23] = 3;
		revealPlace(0x100 + 28 * sighted);
		// 416B/4944 changes the named course before hazards/the report.
		_flightToPoint = false;
		_flightSteered = false;
		_state[0x11cb] = 0;
		WRITE_LE_UINT16(_state.begin() + 0x11c5, 0x100 + 28 * sighted);
		setFlightCourse();
		spotted = true;
	}
	checkFlightHazards(); // 3606 follows the complete 40F9 course scan.
	if (_state[0x23] && beginCarriedReport())
		return true;
	if (spotted) {
		// The original controls already own the new course, even if no
		// report is eligible. Arrival was checked before, not after this scan.
		showDesertMenu(false);
		return true;
	}
	return false;
}

void DuneEngine::checkFlightHazards() {
	// 4182/5D36: only ornithopters can be shot down. A named safe or
	// guarded destination exempts the route; a broken-off course does not.
	if ((_state[0x11c9] & 3) != 1)
		return;
	const uint target = READ_LE_UINT16(_state.begin() + 0x11c5);
	const bool exempt = !_state[0x11cb] && target + 0xa < _state.size() &&
		(_state[target + 8] < 0x28 || (_state[target + 0xa] & 8));
	const bool dangerous = !exempt &&
		(_map.groundAt(READ_LE_UINT16(_state.begin() + 4), (int8)_state[6]) & 0x30) == 0x30;
	byte effect = 0;
	if (!dangerous) {
		_state[0x4726] = 0;
	} else {
		if (!_state[0x4726])
			_state[0x23] = 4;
		effect = 0x40;
		_state[0x4726] -= 0x20;
		if (!_state[0x4726])
			_state[0x46d9] = 2;
	}
	setFlightHazardEffect(effect);
}

void DuneEngine::clearFlightHazard() {
	// 41C5 clears the old warning and its countdown before fresh boarding.
	_state[0x4726] = 0;
	setFlightHazardEffect(0);
}

void DuneEngine::setFlightHazardEffect(byte effect) {
	// 41CC changes the selected menu's effect only for fast-forward.
	_state[0x21fd] = effect;
	if (READ_LE_UINT16(_state.begin() + 0x1f12) == 0x4ffb)
		_state[0x1f11] = effect;
	// The native menu owns a separate flag array. Keep the active SKIP
	// row synchronized when an ordinary movement callback enters or leaves
	// hostile terrain, without rebuilding the menu or resetting its page.
	if (_menuKind == kMenuDesert && !_menuParts.empty() && !_menuFlags.empty() &&
			_menuParts[0] == 0xff25) // kDesertSkip, the 21FC/4FFB command.
		_menuFlags[0] = (_menuFlags[0] & ~0x4000) | ((effect & 0x40) ? 0x4000 : 0);
}

// 40F9: B56C scans nine rows of nine terrain cells in stored order. A
// follower, the place's story gate, and the actual heading determine sightings;
// neither the departure's clue slot nor the clicked endpoint selects a place.
int DuneEngine::flightSighting() const {
	uint cell = 0;
	return nextFlightSighting(cell);
}

int DuneEngine::nextFlightSighting(uint &cell) const {
	if (_state.size() <= 0x11cd || _flyingTo < 0 || _flightDiscoveryPrompt ||
			!(_state[0x11c9] & 3) || !READ_LE_UINT16(_state.begin() + 0x10))
		return -1;
	const uint16 longitude = READ_LE_UINT16(_state.begin() + 4);
	const int latitude = (int8)_state[6];
	const int top = MAX(latitude - 4, -98);
	uint scanned = 0;
	for (int y = top; y < top + 9 && y <= 98; y++) {
		const int width = _map.width(ABS(y));
		if (!width)
			continue;
		const int column = ((uint32)longitude * width + 0x8000) >> 16;
		const int start = _map.groundOffset(longitude, y) - column;
		int x = column - 4;
		if (x < 0)
			x += width;
		for (int n = 0; n < 9; n++) {
			// 417A resumes the stored scan after revealing and turning toward
			// a sighting. Do not rescan earlier cells with the new heading.
			const bool pending = scanned++ >= cell;
			const int offset = start + x;
			if (pending && offset >= 0 && (uint)offset < _map.terrain().size() &&
					(_map.terrain()[offset] & 0x40)) {
				for (uint i = 0; i < _map.places().size(); i++) {
					const Place &place = _map.places()[i];
					if (_map.groundOffset(place.longitude, place.latitude) != offset)
						continue;
					byte bearing = _flightHeading;
					if (!place.isKnown() && storyStage() >= _state[0x10b + 28 * i] &&
							flightBearing(longitude, latitude, place.longitude, place.latitude, bearing) &&
							flightCanSee(_flightHeading, bearing)) {
						cell = scanned;
						return i;
					}
					break; // 409A uses the first record on this terrain cell.
				}
			}
			if (++x >= width)
				x -= width;
		}
	}
	cell = scanned;
	return -1;
}

} // namespace Dune
