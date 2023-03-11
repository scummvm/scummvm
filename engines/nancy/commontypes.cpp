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

void SceneChangeDescription::readData(Common::SeekableReadStream &stream, bool longFormat) {
	sceneID = stream.readUint16LE();
	frameID = stream.readUint16LE();
	verticalOffset = stream.readUint16LE();
	if (longFormat) {
		paletteID = stream.readByte();
		stream.skip(2);
	}
	continueSceneSound = stream.readUint16LE();
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
		descs[i].flag = stream.readUint16LE();
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

void ConditionalDialogue::readData(Common::SeekableReadStream &stream) {
	textID = stream.readByte();
	sceneID = stream.readUint16LE();
	soundID = stream.readString();

	uint16 num = stream.readUint16LE();
	flagConditions.resize(num);
	for (uint16 i = 0; i < num; ++i) {
		flagConditions[i].label = stream.readSint16LE();
		flagConditions[i].flag = stream.readByte();
	}

	num = stream.readUint16LE();
	inventoryConditions.resize(num);
	for (uint16 i = 0; i < num; ++i) {
		inventoryConditions[i].label = stream.readSint16LE();
		inventoryConditions[i].flag = stream.readByte();
	}
}

void GoodbyeSceneChange::readData(Common::SeekableReadStream &stream) {
	uint16 num = stream.readUint16LE();
	sceneIDs.resize(num);
	for (uint16 i = 0; i < num; ++i) {
		sceneIDs[i] = stream.readSint16LE();
	}

	num = stream.readUint16LE();
	flagConditions.resize(num);
	for (uint16 i = 0; i < num; ++i) {
		flagConditions[i].label = stream.readSint16LE();
		flagConditions[i].flag = stream.readByte();
	}

	flagToSet.label = stream.readSint16LE();
	flagToSet.flag = stream.readByte();
}

void Goodbye::readData(Common::SeekableReadStream &stream) {
	soundID = stream.readString();

	uint16 num = stream.readUint16LE();
	sceneChanges.resize(num);
	for (uint16 i = 0; i < num; ++i) {
		sceneChanges[i].readData(stream);
	}
}

void Hint::readData(Common::SeekableReadStream &stream) {
	textID = stream.readByte();
	hintWeight = stream.readSint16LE();
	sceneChange.readData(stream);
	soundIDs[0] = stream.readString();
	soundIDs[1] = stream.readString();
	soundIDs[2] = stream.readString();

	uint16 num = stream.readUint16LE();
	flagConditions.resize(num);
	for (uint16 i = 0; i < num; ++i) {
		flagConditions[i].label = stream.readSint16LE();
		flagConditions[i].flag = stream.readByte();
	}

	num = stream.readUint16LE();
	inventoryConditions.resize(num);
	for (uint16 i = 0; i < num; ++i) {
		inventoryConditions[i].label = stream.readSint16LE();
		inventoryConditions[i].flag = stream.readByte();
	}
}

void StaticData::readData(Common::SeekableReadStream &stream, Common::Language language) {
	numItems = stream.readUint16LE();
	numEventFlags = stream.readUint16LE();

	uint16 num = stream.readUint16LE();
	mapAccessSceneIDs.resize(num);
	for (uint16 i = 0; i < num; ++i) {
		mapAccessSceneIDs[i] = stream.readUint16LE();
	}

	num = stream.readUint16LE();
	genericEventFlags.resize(num);
	for (uint16 i = 0; i < num; ++i) {
		genericEventFlags[i] = stream.readUint16LE();
	}

	numNonItemCursors = stream.readUint16LE();
	numCurtainAnimationFrames = stream.readUint16LE();
	logoEndAfter = stream.readUint32LE();

	// Check for language
	num = stream.readUint16LE();
	int languageID = -1;
	for (uint16 i = 0; i < num; ++i) {
		if (stream.readByte() == language) {
			languageID = i;
		}
	}

	if (languageID == -1) {
		error("Language not present in nancy.dat");
	}

	// Read the strings logic
	num = stream.readUint16LE();
	conditionalDialogue.resize(num);
	for (uint16 i = 0; i < num; ++i) {
		uint16 num2 = stream.readUint16LE();
		conditionalDialogue[i].resize(num2);
		for (uint j = 0; j < num2; ++j) {
			conditionalDialogue[i][j].readData(stream);
		}
	}

	num = stream.readUint16LE();
	goodbyes.resize(num);
	for (uint16 i = 0; i < num; ++i) {
		goodbyes[i].readData(stream);
	}

	num = stream.readUint16LE();
	hints.resize(num);
	for (uint16 i = 0; i < num; ++i) {
		uint16 num2 = stream.readUint16LE();
		hints[i].resize(num2);
		for (uint j = 0; j < num2; ++j) {
			hints[i][j].readData(stream);
		}
	}

	// Read the in-game strings, making sure to pick the correct language
	num = stream.readUint16LE();
	if (num > 0) {
		uint32 endOffset = stream.readUint32LE();
		stream.skip(languageID * 4);
		stream.seek(stream.readUint32LE());
		num = stream.readUint16LE();
		conditionalDialogueTexts.resize(num);
		for (uint16 i = 0; i < num; ++i) {
			conditionalDialogueTexts[i] = stream.readString();
		}

		stream.seek(endOffset);
	}

	num = stream.readUint16LE();
	if (num > 0) {
		uint32 endOffset = stream.readUint32LE();
		stream.skip(languageID * 4);
		stream.seek(stream.readUint32LE());
		num = stream.readUint16LE();
		goodbyeTexts.resize(num);
		for (uint16 i = 0; i < num; ++i) {
			goodbyeTexts[i] = stream.readString();
		}

		stream.seek(endOffset);
	}

	num = stream.readUint16LE();
	if (num > 0) {
		uint32 endOffset = stream.readUint32LE();
		stream.skip(languageID * 4);
		stream.seek(stream.readUint32LE());
		num = stream.readUint16LE();
		hintTexts.resize(num);
		for (uint16 i = 0; i < num; ++i) {
			hintTexts[i] = stream.readString();
		}

		stream.seek(endOffset);
	}

	num = stream.readUint16LE();
	for (int i = 0; i < num; ++i) {
		if (i == languageID) {
			ringingText = stream.readString();
		} else {
			stream.readString();
		}
	}

	// Read debug strings
	num = stream.readUint16LE();
	itemNames.resize(num);
	for (uint16 i = 0; i < num; ++i) {
		itemNames[i] = stream.readString();
	}

	num = stream.readUint16LE();
	eventFlagNames.resize(num);
	for (uint16 i = 0; i < num; ++i) {
		eventFlagNames[i] = stream.readString();
	}
}

} // End of namespace Nancy
