/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/scummsys.h"

#include "common/stream.h"
#include "common/file.h"
#include "common/tokenizer.h"
#include "common/system.h"

#include "zvision/lever_control.h"
#include "zvision/zvision.h"
#include "zvision/script_manager.h"
#include "zvision/cursor_manager.h"
#include "zvision/rlf_animation.h"
#include "zvision/zork_avi_decoder.h"
#include "zvision/utility.h"

namespace ZVision {

LeverControl::LeverControl(ZVision *engine, uint32 key, Common::SeekableReadStream &stream)
		: Control(engine, key),
		  _frameInfo(0),
		  _frameCount(0),
		  _startFrame(0),
		  _currentFrame(0),
		  _mouseIsCaptured(false),
		  _isReturning(false) {

	// Loop until we find the closing brace
	Common::String line = stream.readLine();
	trimCommentsAndWhiteSpace(&line);

	while (!stream.eos() && !line.contains('}')) {
		if (line.matchString("*descfile*", true)) {
			char levFileName[25];
			sscanf(line.c_str(), "%*[^(](%25[^)])", levFileName);

			parseLevFile(levFileName);
		} else if (line.matchString("*cursor*", true)) {
			char cursorName[25];
			sscanf(line.c_str(), "%*[^(](%25[^)])", cursorName);

			_cursorName = Common::String(cursorName);
		}

		line = stream.readLine();
		trimCommentsAndWhiteSpace(&line);
	}

	renderFrame(_currentFrame);
}

LeverControl::~LeverControl() {
	if (_fileType == AVI) {
		delete _animation.avi;
	} else if (_fileType == RLF) {
		delete _animation.rlf;
	}
	if (_frameInfo != 0) {
		delete[] _frameInfo;
	}
}

void LeverControl::parseLevFile(const Common::String &fileName) {
	Common::File file;
	if (!file.open(fileName)) {
		warning("LEV file %s could could be opened", fileName.c_str());
		return;
	}

	Common::String line = file.readLine();

	while (!file.eos()) {
		if (line.matchString("*animation_id*", true)) {
			// Not used
		} else if (line.matchString("*filename*", true)) {
			char fileNameBuffer[25];
			sscanf(line.c_str(), "%*[^:]:%25[^~]~", fileNameBuffer);

			Common::String animationFileName(fileNameBuffer);

			if (animationFileName.hasSuffix(".avi")) {
				_animation.avi = new ZorkAVIDecoder();
				_animation.avi->loadFile(animationFileName);
			} else if (animationFileName.hasSuffix(".rlf")) {
				_animation.rlf = new RlfAnimation(animationFileName, false);
			}
		} else if (line.matchString("*skipcolor*", true)) {
			// Not used
		} else if (line.matchString("*anim_coords*", true)) {
			int left, top, right, bottom;
			sscanf(line.c_str(), "%*[^:]:%d %d %d %d~", &left, &top, &right, &bottom);

			_animationCoords.left = left;
			_animationCoords.top = top;
			_animationCoords.right = right;
			_animationCoords.bottom = bottom;
		} else if (line.matchString("*mirrored*", true)) {
			uint mirrored;
			sscanf(line.c_str(), "%*[^:]:%u~", &mirrored);

			_mirrored = mirrored == 0 ? false : true;
		} else if (line.matchString("*frames*", true)) {
			sscanf(line.c_str(), "%*[^:]:%u~", &_frameCount);

			_frameInfo = new FrameInfo[_frameCount];
		} else if (line.matchString("*elsewhere*", true)) {
			// Not used
		} else if (line.matchString("*out_of_control*", true)) {
			// Not used
		} else if (line.matchString("*start_pos*", true)) {
			sscanf(line.c_str(), "%*[^:]:%u~", &_startFrame);
			_currentFrame = _startFrame;
		} else if (line.matchString("*hotspot_deltas*", true)) {
			uint x;
			uint y;
			sscanf(line.c_str(), "%*[^:]:%u %u~", &x, &y);

			_hotspotDelta.x = x;
			_hotspotDelta.y = y;
		} else {
			uint frameNumber;
			uint x, y;

			if (sscanf(line.c_str(), "%u:%u %u", &frameNumber, &x, &y) == 3) {
				_frameInfo[frameNumber].hotspot.left = x;
				_frameInfo[frameNumber].hotspot.top = y;
				_frameInfo[frameNumber].hotspot.right = x + _hotspotDelta.x;
				_frameInfo[frameNumber].hotspot.bottom = y + _hotspotDelta.y;
			}

			Common::StringTokenizer tokenizer(line, " ^=");
			tokenizer.nextToken();
			tokenizer.nextToken();

			Common::String token = tokenizer.nextToken();
			while (!tokenizer.empty()) {
				if (token == "D") {
					token = tokenizer.nextToken();

					uint angle;
					uint toFrame;
					sscanf(token.c_str(), "%u,%u", &toFrame, &angle);

					_frameInfo[frameNumber].directions.push_back(Direction(angle, toFrame));
				} else if (token.hasPrefix("P")) {
					uint to;
					sscanf(token.c_str(), "P(%*u to %u)", &to);

					_frameInfo[frameNumber].returnRoute.push_back(to);
				}

				token = tokenizer.nextToken();
			}
		}

		line = file.readLine();
	}
}

void LeverControl::onMouseDown(const Common::Point &screenSpacePos, const Common::Point &backgroundImageSpacePos) {
	if (_frameInfo[_currentFrame].hotspot.contains(backgroundImageSpacePos)) {
		_mouseIsCaptured = true;
		_lastMousePos = backgroundImageSpacePos;
	}
}

void LeverControl::onMouseUp(const Common::Point &screenSpacePos, const Common::Point &backgroundImageSpacePos) {
	_mouseIsCaptured = false;
	_engine->getScriptManager()->setStateValue(_key, _currentFrame);

	if (_frameInfo[_currentFrame].hotspot.contains(backgroundImageSpacePos)) {

	}

	// TODO: Animation reversal back to origin
}

bool LeverControl::onMouseMove(const Common::Point &screenSpacePos, const Common::Point &backgroundImageSpacePos) {
	bool cursorWasChanged = false;

	if (_mouseIsCaptured) {
		// Make sure the square distance between the last point and the current point is greater than 64
		// This is a heuristic. This determines how responsive the lever is to mouse movement.
		// TODO: Fiddle with the heuristic to get a good lever responsiveness 'feel'
		if (_lastMousePos.sqrDist(backgroundImageSpacePos) >= 64) {
			int angle = calculateVectorAngle(_lastMousePos, backgroundImageSpacePos);
			_lastMousePos = backgroundImageSpacePos;

			for (Common::List<Direction>::iterator iter = _frameInfo[_currentFrame].directions.begin(); iter != _frameInfo[_currentFrame].directions.end(); iter++) {
				if (angle >= (int)(*iter).angle - ANGLE_DELTA && angle <= (int)(*iter).angle + ANGLE_DELTA) {
					_currentFrame = (*iter).toFrame;
					renderFrame(_currentFrame);
					break;
				}
			}
		}
	} else if (_frameInfo[_currentFrame].hotspot.contains(backgroundImageSpacePos)) {
		_engine->getCursorManager()->changeCursor(_cursorName);
	}

	return cursorWasChanged;
}

bool LeverControl::process(uint32 deltaTimeInMillis) {
	// TODO: Implement reversal over time

	return false;
}

int LeverControl::calculateVectorAngle(const Common::Point &pointOne, const Common::Point &pointTwo) {
	// Check for the easy angles first
	if (pointOne.x == pointTwo.x && pointOne.y == pointTwo.y)
		return -1; // This should never happen
	else if (pointOne.x == pointTwo.x) {
		if (pointTwo.y > pointOne.y)
			return 90;
		else
			return 270;
	} else if (pointOne.y == pointTwo.y) {
		if (pointTwo.x > pointOne.x)
			return 0;
		else
			return 180;
	} else {
		// Calculate the angle with trig
		int16 xDist = pointTwo.x - pointOne.x;
		int16 yDist = pointTwo.y - pointOne.y;

		int angle = int(atan((float)yDist / (float)xDist));

		// Convert to degrees. (180 / 3.14159 = 57.2958)
		angle *= 57;

		// Calculate what quadrant pointTwo is in
		uint quadrant = ((yDist > 0 ? 1 : 0) << 1) | (xDist < 0 ? 1 : 0);

		// Explanation of quadrants:
		//
		// yDist > 0  | xDist < 0 | Quadrant number
		//     0      |     0     |   0
		//     0      |     1     |   1
		//     1      |     0     |   2
		//     1      |     1     |   3
		//
		// Note: I know this doesn't line up with traditional mathematical quadrants
		// but doing it this way allows you can use a switch and is a bit cleaner IMO.
		//
		// The graph below shows the 4 quadrants pointTwo can end up in as well
		// as what the angle as calculated above refers to.
		// Note: The calculated angle in quadrants 0 and 3 is negative
		// due to arctan(-x) = -theta
		//
		// Origin => (pointOne.x, pointOne.y)
		//   *    => (pointTwo.x, pointTwo.y)
		//
		//                         90
		//                         ^
		//                 *       |       *
		//                  \      |      /
		//                   \     |     /
		//                    \    |    /
		// Quadrant 3          \   |   /         Quadrant 2
		//                      \  |  /
		//                       \ | /
		//               -angle ( \|/ )  angle
		// 180 <----------------------------------------> 0
		//                angle ( /|\ ) -angle
		//                       / | \
		//                      /  |  \
		// Quadrant 1          /   |   \         Quadrant 0
		//                    /    |    \
		//                   /     |     \
		//                  /      |      \
		//                 *       |       *
		//                         ^
		//                        270

		// Convert the local angles to unit circle angles
		switch (quadrant) {
		case 0:
			angle = 360 + angle;
			break;
		case 1:
			angle = 180 + angle;
			break;
		case 2:
			angle = 180 + angle;
			break;
		case 3:
			// Do nothing
			break;
		}

		return angle;
	}
}

void LeverControl::renderFrame(uint frameNumber) {
	const uint16 *frameData;
	int pitch;
	int x;
	int y;
	int width;
	int height;

	if (_fileType == RLF) {
		// getFrameData() will automatically optimize to getNextFrame() / getPreviousFrame() if it can
		frameData = _animation.rlf->getFrameData(_currentFrame);
		pitch = _animation.rlf->width() * sizeof(uint16);
		x = _animationCoords.left;
		y = _animationCoords.right;
		width = _animation.rlf->width(); // Use the animation width instead of _animationCoords.width()
		height = _animation.rlf->height(); // Use the animation height instead of _animationCoords.height()			
	} else if (_fileType == AVI) {
		// Cry because AVI seeking isn't implemented (yet)
	}

	_engine->_system->copyRectToScreen(frameData, pitch, x + _engine->_workingWindow.left, y + _engine->_workingWindow.top, width, height);
}

} // End of namespace ZVision
