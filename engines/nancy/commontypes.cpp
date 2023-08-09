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
#include "engines/nancy/nancy.h"

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

	if (g_nancy->getGameType() >= kGameTypeNancy3) {
		stream.skip(12); // 3D sound listener position
	}
}

void SceneChangeWithFlag::readData(Common::SeekableReadStream &stream, bool longFormat) {
	_sceneChange.readData(stream, longFormat);
	stream.skip(2); // shouldStopRendering
	_flag.label = stream.readSint16LE();
	_flag.flag = stream.readByte();
}

void SceneChangeWithFlag::execute() {
	NancySceneState.changeScene(_sceneChange);
	NancySceneState.setEventFlag(_flag);
}

void HotspotDescription::readData(Common::SeekableReadStream &stream) {
	frameID = stream.readUint16LE();
	readRect(stream, coords);
}

void BitmapDescription::readData(Common::SeekableReadStream &stream, bool frameIsLong) {
	if (!frameIsLong) {
		frameID = stream.readUint16LE();
	} else {
		frameID = stream.readUint32LE();
	}

	if (g_nancy->getGameType() >= kGameTypeNancy3) {
		// Most likely transparency
		stream.skip(2);
	}

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

void SoundDescription::readNormal(Common::SeekableReadStream &stream) {
	Common::Serializer s(&stream, nullptr);
	s.setVersion(g_nancy->getGameType());

	readFilename(s, name);

	s.syncAsUint16LE(channelID);

	s.skip(2); // PLAY_SOUND_FROM_HD = 1, PLAY_SOUND_FROM_CDROM = 2
	s.skip(2); // PLAY_SOUND_AS_DIGI = 1, PLAY_SOUND_AS_STREAM = 2
	s.syncAsUint16LE(playCommands);

	s.skip(2, kGameTypeVampire, kGameTypeNancy2);

	s.syncAsUint32LE(numLoops);
	
	s.skip(2, kGameTypeVampire, kGameTypeNancy2);

	s.syncAsUint16LE(volume);
	s.skip(2); // Second volume, always (?) same as the first

	s.skip(4, kGameTypeVampire, kGameTypeNancy1); // Prior to nancy2 this field was used for something else
	s.syncAsUint32LE(samplesPerSec, kGameTypeNancy2, kGameTypeNancy2);
}

void SoundDescription::readDIGI(Common::SeekableReadStream &stream) {
	Common::Serializer s(&stream, nullptr);
	s.setVersion(g_nancy->getGameType());

	readFilename(s, name);

	s.syncAsUint16LE(channelID);

	s.skip(2); // PLAY_SOUND_FROM_HD = 1, PLAY_SOUND_FROM_CDROM = 2
	s.skip(2, kGameTypeVampire, kGameTypeVampire);
	s.syncAsUint16LE(playCommands, kGameTypeNancy1);

	s.syncAsUint32LE(numLoops);

	s.skip(2, kGameTypeVampire, kGameTypeNancy2);
	s.syncAsUint16LE(volume);
	s.skip(2); // Second volume, always (?) same as the first

	s.syncAsUint16LE(panAnchorFrame, kGameTypeVampire, kGameTypeNancy2);
	s.skip(2, kGameTypeVampire, kGameTypeNancy2);

	s.skip(0x61, kGameTypeNancy3);
}

void SoundDescription::readMenu(Common::SeekableReadStream &stream) {
	Common::Serializer s(&stream, nullptr);
	s.setVersion(g_nancy->getGameType());

	readFilename(s, name);

	s.syncAsUint16LE(channelID);

	s.skip(2); // PLAY_SOUND_FROM_HD = 1, PLAY_SOUND_FROM_CDROM = 2
	s.skip(2); // PLAY_SOUND_AS_DIGI = 1, PLAY_SOUND_AS_STREAM = 2

	s.skip(2, kGameTypeVampire, kGameTypeNancy2);

	s.syncAsUint32LE(numLoops);

	s.skip(2, kGameTypeVampire, kGameTypeNancy2);

	s.syncAsUint16LE(volume);
	s.skip(2); // Second volume, always (?) same as the first

	s.skip(4, kGameTypeVampire, kGameTypeNancy2);
}

void SoundDescription::readScene(Common::SeekableReadStream &stream) {
	Common::Serializer s(&stream, nullptr);
	s.setVersion(g_nancy->getGameType());

	readFilename(s, name);

	s.skip(2); // PLAY_SOUND_FROM_HD = 1, PLAY_SOUND_FROM_CDROM = 2
	s.skip(2); // PLAY_SOUND_AS_DIGI = 1, PLAY_SOUND_AS_STREAM = 2

	s.syncAsUint16LE(channelID);
	s.syncAsUint16LE(playCommands);

	s.skip(2, kGameTypeVampire, kGameTypeNancy2);

	s.syncAsUint32LE(numLoops);

	s.skip(2, kGameTypeVampire, kGameTypeNancy2);

	s.syncAsUint16LE(volume);
	s.skip(2); // Second volume, always (?) same as the first
	s.skip(2, kGameTypeVampire, kGameTypeNancy2);
	s.skip(4, kGameTypeVampire, kGameTypeNancy2); // Panning, always? at center
	s.syncAsUint32LE(samplesPerSec, kGameTypeVampire, kGameTypeNancy2);
}

void ConditionalDialogue::readData(Common::SeekableReadStream &stream) {
	textID = stream.readByte();
	sceneID = stream.readUint16LE();
	soundID = stream.readString();

	uint16 num = stream.readUint16LE();
	conditions.resize(num);
	for (uint16 i = 0; i < num; ++i) {
		conditions[i].type = stream.readByte();
		conditions[i].label = stream.readSint16LE();
		conditions[i].flag = stream.readByte();
	}
}

void GoodbyeSceneChange::readData(Common::SeekableReadStream &stream) {
	uint16 num = stream.readUint16LE();
	sceneIDs.resize(num);
	for (uint16 i = 0; i < num; ++i) {
		sceneIDs[i] = stream.readSint16LE();
	}

	num = stream.readUint16LE();
	conditions.resize(num);
	for (uint16 i = 0; i < num; ++i) {
		conditions[i].type = stream.readByte();
		conditions[i].label = stream.readSint16LE();
		conditions[i].flag = stream.readByte();
	}

	flagToSet.type = stream.readByte();
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
	soundIDs[0] = stream.readString();
	soundIDs[1] = stream.readString();
	soundIDs[2] = stream.readString();

	uint16 num = stream.readUint16LE();
	conditions.resize(num);
	for (uint16 i = 0; i < num; ++i) {
		conditions[i].type = stream.readByte();
		conditions[i].label = stream.readSint16LE();
		conditions[i].flag = stream.readByte();
	}
}

void SoundChannelInfo::readData(Common::SeekableReadStream &stream) {
	numChannels = stream.readByte();
	numSceneSpecificChannels = stream.readByte();

	uint16 num = stream.readUint16LE();
	speechChannels.resize(num);
	for (uint16 i = 0; i < num; ++i) {
		speechChannels[i] = stream.readByte();
	}

	num = stream.readUint16LE();
	musicChannels.resize(num);
	for (uint16 i = 0; i < num; ++i) {
		musicChannels[i] = stream.readByte();
	}

	num = stream.readUint16LE();
	sfxChannels.resize(num);
	for (uint16 i = 0; i < num; ++i) {
		sfxChannels[i] = stream.readByte();
	}
}

void StaticData::readData(Common::SeekableReadStream &stream, Common::Language language, uint32 endPos) {
	uint16 num;
	int languageID;

	while (stream.pos() < endPos) {
		uint32 nextSectionOffset = stream.readUint32LE();

		switch(stream.readUint32LE()) {
		case MKTAG('C', 'O', 'N', 'S') :
			// Game constants
			numItems = stream.readUint16LE();
			numEventFlags = stream.readUint16LE();

			num = stream.readUint16LE();
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

			break;
		case MKTAG('S', 'C', 'H', 'N') :
			// Sound channels data
			soundChannelInfo.readData(stream);

			break;
		case MKTAG('L', 'A', 'N', 'G') : 
			// Order of languages inside game data
			num = stream.readUint16LE();
			languageID = -1;
			for (uint16 i = 0; i < num; ++i) {
				if (stream.readByte() == language) {
					languageID = i;
				}
			}

			if (languageID == -1) {
				error("Language not present in nancy.dat");
			}

			break;
		case MKTAG('C', 'D', 'L', 'G') :
			// Conditional dialogue
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

			break;
		case MKTAG('G', 'D', 'B', 'Y') :
			// Goodbyes
			num = stream.readUint16LE();
			goodbyes.resize(num);
			for (uint16 i = 0; i < num; ++i) {
				goodbyes[i].readData(stream);
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

			break;
		case MKTAG('H', 'I', 'N', 'T') : {
			// Hints (nancy1 only)
			SceneChangeDescription sceneChange;
			sceneChange.readData(stream, false);

			num = stream.readUint16LE();
			hints.resize(num);
			for (uint16 i = 0; i < num; ++i) {
				uint16 num2 = stream.readUint16LE();
				hints[i].resize(num2);
				for (uint j = 0; j < num2; ++j) {
					hints[i][j].readData(stream);
					hints[i][j].sceneChange = sceneChange;
				}
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

			break;
		}
		case MKTAG('R', 'I', 'N', 'G') :
			// Ringing text (nancy1 and up)
			num = stream.readUint16LE();
			for (int i = 0; i < num; ++i) {
				if (i == languageID) {
					ringingText = stream.readString();
				} else {
					stream.readString();
				}
			}

			break;
		case MKTAG('E', 'F', 'L', 'G') :
			// Event flag names
			num = stream.readUint16LE();
			eventFlagNames.resize(num);
			for (uint16 i = 0; i < num; ++i) {
				eventFlagNames[i] = stream.readString();
			}

			break;
		default:
			stream.seek(nextSectionOffset);
		}
	}
	

	

	// Read the strings logic
	

	

	

	// Read the in-game strings, making sure to pick the correct language


	

	

	

	// Read debug strings
	
}

} // End of namespace Nancy
