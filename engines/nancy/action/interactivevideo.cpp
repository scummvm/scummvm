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

#include "engines/nancy/nancy.h"
#include "engines/nancy/util.h"
#include "engines/nancy/resource.h"
#include "engines/nancy/video.h"
#include "engines/nancy/input.h"

#include "engines/nancy/action/interactivevideo.h"
#include "engines/nancy/action/secondarymovie.h"
#include "engines/nancy/state/scene.h"

namespace Nancy {
namespace Action {

void InteractiveVideo::readData(Common::SeekableReadStream &stream) {
	Common::Path ivFilename;
	readFilename(stream, ivFilename);

	stream.skip(2);

	_flags.resize(15);
	_cursors.resize(5);

	for (uint i = 0; i < 15; ++i) {
		_flags[i].label = stream.readSint16LE();
		_flags[i].flag = stream.readSint16LE();
	}

	for (uint i = 0; i < 5; ++ i) {
		_cursors[i] = stream.readSint16LE();
	}

	Common::SeekableReadStream *ivFile = SearchMan.createReadStreamForMember(ivFilename.append(".iv"));
	assert(ivFile);

	readFilename(*ivFile, _videoName);

	uint32 numFrames = ivFile->readUint32LE();
	_frames.resize(numFrames);
	for (uint i = 0; i < numFrames; ++i) {
		InteractiveFrame &frame = _frames[i];
		frame.frameID = ivFile->readUint16LE();
		uint16 numHotspots = ivFile->readUint16LE();
		frame.triggerOnNoHotspot = ivFile->readByte();
		frame.noHSFlagID = ivFile->readSint16LE();
		frame.noHSCursorID = ivFile->readSint16LE();

		frame.hotspots.resize(numHotspots);
		for (uint j = 0; j < numHotspots; ++j) {
			ivFile->skip(4);
			readRect(*ivFile, frame.hotspots[j].hotspot);
			frame.hotspots[j].flagID = ivFile->readSint16LE();
			frame.hotspots[j].cursorID = ivFile->readSint16LE();
		}
	}

	delete ivFile;
}

void InteractiveVideo::execute() {
	switch (_state) {
	case kBegin:
		_movieAR = NancySceneState.getActiveMovie();
		if (!_movieAR || _movieAR->_state == kRun) {
			_state = kRun;
		}

		break;
	case kRun:
		if (_movieAR->_state == kActionTrigger || _movieAR->_isFinished) {
			_state = kActionTrigger;
		}

		break;
	case kActionTrigger:
		finishExecution();
		break;
	}
}

void InteractiveVideo::handleInput(NancyInput &input) {
	if (_state != kRun) {
		return;
	}

	int curFrame = _movieAR->_decoder->getCurFrame();
	if (curFrame < 0) {
		return;
	}

	for (auto &frame : _frames) {
		if (frame.frameID == curFrame) {
			// Found data for the current video frame

			// First, look through the hotspots for the frame
			for (auto &hotspot : frame.hotspots) {
				if (NancySceneState.getViewport().convertViewportToScreen(hotspot.hotspot).contains(input.mousePos)) {
					// Mouse is in a hotspot, change cursor and set flag if clicked
					if (hotspot.cursorID >= 0 && _cursors[hotspot.cursorID] >= 0) {
						g_nancy->_cursor->setCursorType((CursorManager::CursorType)_cursors[hotspot.cursorID]);
					}

					if (input.input & NancyInput::kLeftMouseButtonUp) {
						NancySceneState.setEventFlag(_flags[hotspot.flagID]);
					}

					return;
				}
			}

			// Mouse is not in a hotspot for the frame, check if we have a default action
			if (frame.triggerOnNoHotspot) {
				if (frame.noHSCursorID >= 0 && _cursors[frame.noHSCursorID] >= 0) {
					g_nancy->_cursor->setCursorType((CursorManager::CursorType)_cursors[frame.noHSCursorID]);
				}

				if (input.input & NancyInput::kLeftMouseButtonUp) {
					NancySceneState.setEventFlag(_flags[frame.noHSFlagID]);
				}
			}

			return;
		}
	}
}

} // End of namespace Action
} // End of namespace Nancy
