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

#include "common/random.h"

#include "engines/nancy/nancy.h"
#include "engines/nancy/sound.h"
#include "engines/nancy/util.h"

#include "engines/nancy/action/soundrecords.h"

#include "engines/nancy/state/scene.h"

namespace Nancy {
namespace Action {

void PlayDigiSound::readData(Common::SeekableReadStream &stream) {
	_sound.readDIGI(stream);

	if (g_nancy->getGameType() >= kGameTypeNancy3) {
		_soundEffect = new SoundEffectDescription;
		_soundEffect->readData(stream);

		if (g_nancy->getGameType() >= kGameTypeNancy6) {
			_changeSceneImmediately = stream.readByte();
		}
	}

	_sceneChange.readData(stream, g_nancy->getGameType() == kGameTypeVampire);

	_flag.label = stream.readSint16LE();
	_flag.flag = stream.readByte();
	stream.skip(2); // VIDEO_STOP_RENDERING, VIDEO_CONTINUE_RENDERING
}

void PlayDigiSound::execute() {
	switch (_state) {
	case kBegin:
		g_nancy->_sound->loadSound(_sound, &_soundEffect);
		g_nancy->_sound->playSound(_sound);

		if (_changeSceneImmediately) {
			NancySceneState.changeScene(_sceneChange);
			finishExecution();
			break;
		}

		_state = kRun;
		break;
	case kRun:
		if (!g_nancy->_sound->isSoundPlaying(_sound)) {
			_state = kActionTrigger;
		}

		break;
	case kActionTrigger:
		NancySceneState.changeScene(_sceneChange);
		NancySceneState.setEventFlag(_flag);

		g_nancy->_sound->stopSound(_sound);

		finishExecution();
		break;
	}
}

Common::String PlayDigiSound::getRecordTypeName() const {
	return g_nancy->getGameType() <= kGameTypeNancy2 ? "PlayDigiSoundAndDie" : "PlayDigiSound";
}

void PlayDigiSoundCC::readData(Common::SeekableReadStream &stream) {
	PlayDigiSound::readData(stream);

	uint16 textSize = stream.readUint16LE();
	if (textSize) {
		char *strBuf = new char[textSize];
		stream.read(strBuf, textSize);
		assembleTextLine(strBuf, _ccText, textSize);
		delete[] strBuf;
	}
}

void PlayDigiSoundCC::execute() {
	if (_state == kBegin) {
		NancySceneState.getTextbox().clear();
		NancySceneState.getTextbox().addTextLine(_ccText);
	}
	PlayDigiSound::execute();
}

void PlaySoundPanFrameAnchorAndDie::readData(Common::SeekableReadStream &stream) {
	_sound.readDIGI(stream);
	stream.skip(2);
	_sound.isPanning = true;
}

void PlaySoundPanFrameAnchorAndDie::execute() {
	g_nancy->_sound->loadSound(_sound);
	g_nancy->_sound->playSound(_sound);
	_isDone = true;
}

void PlaySoundMultiHS::readData(Common::SeekableReadStream &stream) {
	_sound.readNormal(stream);

	if (g_nancy->getGameType() != kGameTypeVampire) {
		_sceneChange.readData(stream);
		_flag.label = stream.readSint16LE();
		_flag.flag = stream.readByte();
		stream.skip(2);
	} else {
		_flag.label = kEvNoEvent;
		_sceneChange.sceneID = 9999;
	}

	uint16 numHotspots = stream.readUint16LE();

	_hotspots.reserve(numHotspots);
	for (uint i = 0; i < numHotspots; ++i) {
		_hotspots.push_back(HotspotDescription());
		_hotspots.back().frameID = stream.readUint16LE();
		readRect(stream, _hotspots.back().coords);
	}
}

void PlaySoundMultiHS::execute() {
	switch (_state) {
	case kBegin:
		_state = kRun;
		// fall through
	case kRun: {
		_hasHotspot = false;
		uint currentFrame = NancySceneState.getSceneInfo().frameID;

		for (uint i = 0; i < _hotspots.size(); ++i) {
			if (_hotspots[i].frameID == currentFrame) {
				_hotspot = _hotspots[i].coords;
				_hasHotspot = true;
				break;
			}
		}

		break;
	}
	case kActionTrigger:
		g_nancy->_sound->loadSound(_sound);
		g_nancy->_sound->playSound(_sound);
		NancySceneState.changeScene(_sceneChange);
		NancySceneState.setEventFlag(_flag);
		finishExecution();
		break;
	}
}

void StopSound::readData(Common::SeekableReadStream &stream) {
	_channelID = stream.readUint16LE();
	_sceneChange.readData(stream);
}

void StopSound::execute() {
	g_nancy->_sound->stopSound(_channelID);
	_sceneChange.execute();
}

void PlayRandomSound::readData(Common::SeekableReadStream &stream) {
	uint16 numSounds = stream.readUint16LE();
	readFilenameArray(stream, _soundNames, numSounds - 1);

	PlayDigiSound::readData(stream);
	_soundNames.push_back(_sound.name);
}

void PlayRandomSound::execute() {
	if (_state == kBegin) {
		_sound.name = _soundNames[g_nancy->_randomSource->getRandomNumber(_soundNames.size() - 1)];
	}

	PlayDigiSound::execute();
}

void TableIndexPlaySound::readData(Common::SeekableReadStream &stream) {
	_tableIndex = stream.readUint16LE();
	PlayDigiSound::readData(stream); // Data does NOT contain captions, so we call the PlayDigiSound version
}

void TableIndexPlaySound::execute() {
	TableData *playerTable = (TableData *)NancySceneState.getPuzzleData(TableData::getTag());
	assert(playerTable);
	const TABL *tabl = (const TABL *)g_nancy->getEngineData("TABL");
	assert(tabl);

	if (_lastIndexVal != playerTable->currentIDs[_tableIndex - 1]) {
		g_nancy->_sound->stopSound(_sound);
		NancySceneState.getTextbox().clear();
		_lastIndexVal = playerTable->currentIDs[_tableIndex - 1];
		_sound.name = Common::String::format("%s%u", tabl->soundBaseName.c_str(), playerTable->currentIDs[_tableIndex - 1]);
		_ccText = tabl->strings[playerTable->currentIDs[_tableIndex - 1] - 1];
	}

	PlayDigiSoundCC::execute();
}

} // End of namespace Action
} // End of namespace Nancy
