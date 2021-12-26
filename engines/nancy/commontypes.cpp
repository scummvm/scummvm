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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "engines/nancy/commontypes.h"
#include "engines/nancy/util.h"

#include "engines/nancy/state/scene.h"

namespace Nancy {

void SceneChangeDescription::readData(Common::SeekableReadStream &stream) {
	sceneID = stream.readUint16LE();
	frameID = stream.readUint16LE();
	verticalOffset = stream.readUint16LE();
	doNotStartSound = (bool)(stream.readUint16LE());
}

void HotspotDescription::readData(Common::SeekableReadStream &stream) {
	frameID = stream.readUint16LE();
	readRect(stream, coords);
}

void BitmapDescription::readData(Common::SeekableReadStream &stream) {
	frameID = stream.readUint16LE();
	readRect(stream, src);
	readRect(stream, dest);
}

void MultiEventFlagDescription::readData(Common::SeekableReadStream &stream) {
	for (uint i = 0; i < 10; ++i) {
		descs[i].label = stream.readSint16LE();
		descs[i].flag = (NancyFlag)stream.readUint16LE();
	}
}

void MultiEventFlagDescription::execute() {
	for (uint i = 0; i < 10; ++i) {
		NancySceneState.setEventFlag(descs[i]);
	}
}

void SecondaryVideoDescription::readData(Common::SeekableReadStream &stream) {
	frameID = stream.readUint16LE();
	readRect(stream, srcRect);
	readRect(stream, destRect);
	stream.skip(0x20);
}

void SoundDescription::read(Common::SeekableReadStream &stream, Type type) {
	readFilename(stream, name);

	if (type == SoundDescription::kScene) {
		stream.skip(4);
	}
	channelID = stream.readUint16LE();

	// 0xE is soundPlayFormat, but I have no idea what that does yet

	// The difference between these is a couple members found at the same position
	// whose purpose I don't understand, so for now just skip them
	switch (type) {
	case kNormal:
		stream.skip(8);
		break;
	case kMenu:
		stream.skip(6);
		break;
	case kScene:
		// fall through
	case kDIGI:
		stream.skip(4);
		break;
	}

	numLoops = stream.readUint16LE();
	if (stream.readUint16LE() != 0) { // loop indefinitely
		numLoops = 0;
	}
	stream.skip(2);
	volume = stream.readUint16LE();
	stream.skip(2);
	panAnchorFrame = stream.readUint16LE();
	stream.skip(2);
}

} // End of namespace Nancy
