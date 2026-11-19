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

#include "hollywood/runaway/saveload.h"
#include "hollywood/runaway/scene_data.h"

namespace Hollywood {

void RunawaySaveState::sync(Common::Serializer &s) {
	s.syncAsUint16LE(scene);
	s.syncAsUint16LE(x);
	s.syncAsUint16LE(y);
	s.syncAsUint16LE(camera);
	s.syncAsByte(facing);
	s.syncAsByte(lidOpen);
	s.syncAsByte(unplugged);
	s.syncAsByte(windowClean);
	s.syncAsByte(coolerMoved);
	s.syncAsByte(coolerInspected);
	s.syncAsByte(peeked);
	s.syncAsByte(ice);
	s.syncAsByte(hatch);
	s.syncBytes(pickedUp, sizeof(pickedUp));
	s.syncBytes(examined, sizeof(examined));
	s.syncAsByte(itemCount);
	s.syncBytes(items, sizeof(items));
	s.syncAsByte(selected);
	s.syncAsByte(cloth);
	s.syncAsByte(inventoryOpen);
	s.syncAsByte(verb);
	s.syncAsUint32LE(randomSeed);
	s.syncAsUint32LE(playTime);
	s.syncAsByte(game, 2);
	s.syncAsUint16LE(cameraY, 2);
	s.syncAsByte(upperFloor, 2);
	s.syncAsByte(books, 2);
	s.syncAsByte(recorder, 2);
	s.syncAsByte(moneyIdentified, 2);
	s.syncAsByte(safeOpen, 2);
	s.syncAsByte(varnishTaken, 2);
	s.syncAsByte(talcTaken, 2);
	s.syncAsByte(coffeeTaken, 2);
	s.syncAsByte(keyCount, 2);
	s.syncAsByte(hospital.glassTaken, 3);
	s.syncAsByte(hospital.pillsTaken, 3);
	s.syncAsByte(hospital.sheetTaken, 3);
	s.syncAsByte(hospital.bagTaken, 3);
	s.syncAsByte(hospital.exitMapRead, 3);
	s.syncAsByte(hospital.markerTaken, 4);
	s.syncAsByte(hospital.alcohol, 4);
	s.syncAsByte(water, 4);
	s.syncAsByte(hospital.window, 5);
	s.syncAsByte(hospital.chartReplaced, 5);
	s.syncAsByte(hospital.chartMoved, 5);
	s.syncAsByte(hospital.decoy, 5);
	s.syncAsByte(hospital.attacked, 5);
	s.syncAsByte(hospital.medicineRead, 5);
	s.syncAsByte(hospital.storeroomVisited, 5);
	s.syncAsByte(hospital.syringesSeen, 5);
	s.syncAsByte(hospital.sprayTaken, 5);
	s.syncAsByte(hospital.book, 5);
	s.syncAsByte(hospital.blankChart, 5);
	s.syncAsByte(hospital.headTaken, 5);
	s.syncAsByte(hospital.pillowsTaken, 5);
	s.syncAsByte(markerInk, 5);
	s.syncAsByte(syringeFilled, 5);
	s.syncAsByte(chartWritten, 5);
	s.syncAsByte(headWig, 5);
}

byte RunawaySaveState::chapter() const {
	switch (scene / 1000) {
	case 1: case 2: return 1;
	case 4: return 2;
	case 5: return 3;
	case 6: return 4;
	case 7: return 5;
	case 8: case 9: return 6;
	default: return 0;
	}
}

bool RunawaySaveState::valid() const {
	if (game > kRunawayFullGame || !chapter() || x >= 32768 || y >= 32768 ||
			camera >= 32768 || cameraY >= 32768 || facing >= 8 ||
			lidOpen > 1 || unplugged > 1 || windowClean > 1 || coolerMoved > 1 ||
			coolerInspected > 1 || peeked > 1 || ice > 2 || hatch > 3 ||
			itemCount > sizeof(items) || selected > sizeof(items) || cloth > 2 || inventoryOpen > 1 || verb > 2)
		return false;
	if (game == kRunawayCabinDemo && (scene != 5030 || x >= 1376 || y >= 600 || camera > 352 || cameraY))
		return false;
	if (upperFloor > 1 || books > 2 || recorder > 2 || moneyIdentified > 1 || safeOpen > 1 ||
			varnishTaken > 1 || talcTaken > 1 || coffeeTaken > 1 || keyCount > 4)
		return false;
	if (game == kRunawayMuseumDemo) {
		const RunawayRoom *room = findRunawayRoom(scene);
		if (!room || scene < 4000 || x >= room->width || y >= room->height || camera > room->width - 1024 ||
				cameraY > room->height - 600)
			return false;
	}
	if (hospital.glassTaken > 1 || hospital.pillsTaken > 1 || hospital.sheetTaken > 1 ||
			hospital.bagTaken > 1 || hospital.exitMapRead > 1 || hospital.markerTaken > 1 || hospital.alcohol > 2 || water > 2)
		return false;
	if (hospital.window > 2 || hospital.chartReplaced > 1 || hospital.chartMoved > 1 ||
			hospital.decoy > 1 || hospital.attacked > 1 || hospital.medicineRead > 1 ||
			hospital.storeroomVisited > 1 || hospital.syringesSeen > 1 || hospital.sprayTaken > 1 ||
			hospital.book > 2 || hospital.blankChart > 2 || hospital.headTaken > 1 || hospital.pillowsTaken > 1 ||
			markerInk > 3 || syringeFilled > 1 || chartWritten > 1 || headWig > 1)
		return false;
	if (game == kRunawayFullGame) {
		const RunawayRoom *room = findRunawayRoom(scene);
		if (!room || (room->number != kHospitalBedroom && room->number != kHospitalStoreroom && room->number != kHospitalBathroom) ||
				x >= room->width || y >= room->height || cameraY ||
				camera < (room->number == kHospitalBedroom ? 156 : 0) || camera > room->width - 1024)
			return false;
	}
	if ((coolerMoved && ice != 2) || ((hatch != 0) != (coolerMoved != 0)))
		return false;
	for (uint i = 0; i < sizeof(pickedUp); ++i) {
		if (pickedUp[i] > 1 || examined[i] > 1)
			return false;
	}
	bool owned[27] = {};
	for (uint i = 0; i < itemCount; ++i) {
		if (!items[i] || items[i] >= sizeof(owned) || owned[items[i]] ||
				(game != kRunawayCabinDemo && items[i] > 19))
			return false;
		owned[items[i]] = true;
	}
	return !selected || owned[selected];
}

bool RunawaySaveState::read(Common::SeekableReadStream &stream) {
	*this = RunawaySaveState();
	if (stream.readUint32BE() != MKTAG('R', 'U', 'N', 'A'))
		return false;
	const uint16 version = stream.readUint16LE();
	if (version < 1 || version > 5)
		return false;
	Common::Serializer s(&stream, nullptr);
	s.setVersion(version);
	sync(s);
	return !s.err() && !stream.eos() && valid();
}

bool RunawaySaveState::write(Common::WriteStream &stream) {
	if (!valid())
		return false;
	stream.writeUint32BE(MKTAG('R', 'U', 'N', 'A'));
	stream.writeUint16LE(5);
	Common::Serializer s(nullptr, &stream);
	s.setVersion(5);
	sync(s);
	return !s.err();
}

} // End of namespace Hollywood
