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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/scummsys.h"

#include "zvision/scripting/controls/lever_control.h"

#include "zvision/zvision.h"
#include "zvision/scripting/script_manager.h"
#include "zvision/graphics/render_manager.h"
#include "zvision/graphics/cursors/cursor_manager.h"

#include "common/stream.h"
#include "common/file.h"
#include "common/tokenizer.h"
#include "common/system.h"
#include "graphics/surface.h"
#include "video/video_decoder.h"

namespace ZVision {

LeverControl::LeverControl(ZVision *engine, uint32 key, Common::SeekableReadStream &stream)
	: Control(engine, key, CONTROL_LEVER),
	  _frameInfo(0),
	  _frameCount(0),
	  _startFrame(0),
	  _currentFrame(0),
	  _lastRenderedFrame(0),
	  _mouseIsCaptured(false),
	  _isReturning(false),
	  _accumulatedTime(0),
	  _returnRoutesCurrentFrame(0),
	  _animation(NULL),
	  _cursor(CursorIndex_Active),
	  _mirrored(false) {

	// Loop until we find the closing brace
	Common::String line = stream.readLine();
	_engine->getScriptManager()->trimCommentsAndWhiteSpace(&line);

	Common::String param;
	Common::String values;
	getParams(line, param, values);

	while (!stream.eos() && !line.contains('}')) {
		if (param.matchString("descfile", true)) {
			char levFileName[25];
			sscanf(values.c_str(), "%24s", levFileName);

			parseLevFile(levFileName);
		} else if (param.matchString("cursor", true)) {
			char cursorName[25];
			sscanf(values.c_str(), "%24s", cursorName);

			_cursor = _engine->getCursorManager()->getCursorId(Common::String(cursorName));
		}

		line = stream.readLine();
		_engine->getScriptManager()->trimCommentsAndWhiteSpace(&line);
		getParams(line, param, values);
	}

	renderFrame(_currentFrame);
}

LeverControl::~LeverControl() {
	if (_animation)
		delete _animation;

	delete[] _frameInfo;
}

void LeverControl::parseLevFile(const Common::String &fileName) {
	Common::File file;
	if (!_engine->getSearchManager()->openFile(file, fileName)) {
		warning("LEV file %s could could be opened", fileName.c_str());
		return;
	}

	Common::String line;
	Common::String param;
	Common::String values;

	while (!file.eos()) {
		line = file.readLine();
		getLevParams(line, param, values);

		if (param.matchString("animation_id", true)) {
			// Not used
		} else if (param.matchString("filename", true)) {
			_animation = _engine->loadAnimation(values);
		} else if (param.matchString("skipcolor", true)) {
			// Not used
		} else if (param.matchString("anim_coords", true)) {
			int left, top, right, bottom;
			sscanf(values.c_str(), "%d %d %d %d", &left, &top, &right, &bottom);

			_animationCoords.left = left;
			_animationCoords.top = top;
			_animationCoords.right = right;
			_animationCoords.bottom = bottom;
		} else if (param.matchString("mirrored", true)) {
			uint mirrored;
			sscanf(values.c_str(), "%u", &mirrored);

			_mirrored = mirrored == 0 ? false : true;
		} else if (param.matchString("frames", true)) {
			sscanf(values.c_str(), "%u", &_frameCount);

			_frameInfo = new FrameInfo[_frameCount];
		} else if (param.matchString("elsewhere", true)) {
			// Not used
		} else if (param.matchString("out_of_control", true)) {
			// Not used
		} else if (param.matchString("start_pos", true)) {
			sscanf(values.c_str(), "%u", &_startFrame);
			_currentFrame = _startFrame;
		} else if (param.matchString("hotspot_deltas", true)) {
			uint x;
			uint y;
			sscanf(values.c_str(), "%u %u", &x, &y);

			_hotspotDelta.x = x;
			_hotspotDelta.y = y;
		} else if (param.matchString("venus_id", true)) {
			_venusId = atoi(values.c_str());
		} else {
			uint frameNumber;
			uint x, y;

			line.toLowercase();

			if (sscanf(line.c_str(), "%u:%u %u", &frameNumber, &x, &y) == 3) {
				_frameInfo[frameNumber].hotspot.left = x;
				_frameInfo[frameNumber].hotspot.top = y;
				_frameInfo[frameNumber].hotspot.right = x + _hotspotDelta.x;
				_frameInfo[frameNumber].hotspot.bottom = y + _hotspotDelta.y;
			}

			Common::StringTokenizer tokenizer(line, " ^=()~");
			tokenizer.nextToken();
			tokenizer.nextToken();

			Common::String token = tokenizer.nextToken();
			while (!tokenizer.empty()) {
				if (token == "d") {
					token = tokenizer.nextToken();

					uint angle;
					uint toFrame;
					sscanf(token.c_str(), "%u,%u", &toFrame, &angle);

					_frameInfo[frameNumber].directions.push_back(Direction(angle, toFrame));
				} else if (token.hasPrefix("p")) {
					// Format: P(<from> to <to>)
					tokenizer.nextToken();
					tokenizer.nextToken();
					token = tokenizer.nextToken();
					uint to = atoi(token.c_str());

					_frameInfo[frameNumber].returnRoute.push_back(to);
				}

				token = tokenizer.nextToken();
			}
		}

		// Don't read lines in this place because last will not be parsed.
	}
}

bool LeverControl::onMouseDown(const Common::Point &screenSpacePos, const Common::Point &backgroundImageSpacePos) {
	if (_engine->getScriptManager()->getStateFlag(_key) & Puzzle::DISABLED)
		return false;

	if (_frameInfo[_currentFrame].hotspot.contains(backgroundImageSpacePos)) {
		setVenus();
		_mouseIsCaptured = true;
		_lastMousePos = backgroundImageSpacePos;
	}
	return false;
}

bool LeverControl::onMouseUp(const Common::Point &screenSpacePos, const Common::Point &backgroundImageSpacePos) {
	if (_engine->getScriptManager()->getStateFlag(_key) & Puzzle::DISABLED)
		return false;

	if (_mouseIsCaptured) {
		_mouseIsCaptured = false;
		_engine->getScriptManager()->setStateValue(_key, _currentFrame);

		_isReturning = true;
		_returnRoutesCurrentProgress = _frameInfo[_currentFrame].returnRoute.begin();
		_returnRoutesCurrentFrame = _currentFrame;
	}
	return false;
}

bool LeverControl::onMouseMove(const Common::Point &screenSpacePos, const Common::Point &backgroundImageSpacePos) {
	if (_engine->getScriptManager()->getStateFlag(_key) & Puzzle::DISABLED)
		return false;

	bool cursorWasChanged = false;

	if (_mouseIsCaptured) {
		// Make sure the square distance between the last point and the current point is greater than 16
		// This is a heuristic. This determines how responsive the lever is to mouse movement.
		if (_lastMousePos.sqrDist(backgroundImageSpacePos) >= 16) {
			int angle = calculateVectorAngle(_lastMousePos, backgroundImageSpacePos);
			_lastMousePos = backgroundImageSpacePos;

			for (Common::List<Direction>::iterator iter = _frameInfo[_currentFrame].directions.begin(); iter != _frameInfo[_currentFrame].directions.end(); ++iter) {
				if (angle >= (int)iter->angle - ANGLE_DELTA && angle <= (int)iter->angle + ANGLE_DELTA) {
					_currentFrame = iter->toFrame;
					renderFrame(_currentFrame);
					_engine->getScriptManager()->setStateValue(_key, _currentFrame);
					break;
				}
			}
		}
		_engine->getCursorManager()->changeCursor(_cursor);
		cursorWasChanged = true;
	} else if (_frameInfo[_currentFrame].hotspot.contains(backgroundImageSpacePos)) {
		_engine->getCursorManager()->changeCursor(_cursor);
		cursorWasChanged = true;
	}

	return cursorWasChanged;
}

bool LeverControl::process(uint32 deltaTimeInMillis) {
	if (_engine->getScriptManager()->getStateFlag(_key) & Puzzle::DISABLED)
		return false;

	if (_isReturning) {
		_accumulatedTime += deltaTimeInMillis;
		while (_accumulatedTime >= ANIMATION_FRAME_TIME) {
			_accumulatedTime -= ANIMATION_FRAME_TIME;
			if (_returnRoutesCurrentFrame == *_returnRoutesCurrentProgress) {
				_returnRoutesCurrentProgress++;
			}
			if (_returnRoutesCurrentProgress == _frameInfo[_currentFrame].returnRoute.end()) {
				_isReturning = false;
				_currentFrame = _returnRoutesCurrentFrame;
				return false;
			}

			uint toFrame = *_returnRoutesCurrentProgress;
			if (_returnRoutesCurrentFrame < toFrame) {
				_returnRoutesCurrentFrame++;
			} else if (_returnRoutesCurrentFrame > toFrame) {
				_returnRoutesCurrentFrame--;
			}

			_engine->getScriptManager()->setStateValue(_key, _returnRoutesCurrentFrame);
			renderFrame(_returnRoutesCurrentFrame);
		}
	}

	return false;
}

int LeverControl::calculateVectorAngle(const Common::Point &pointOne, const Common::Point &pointTwo) {
	// Check for the easy angles first
	if (pointOne.x == pointTwo.x && pointOne.y == pointTwo.y)
		return -1; // This should never happen
	else if (pointOne.x == pointTwo.x) {
		if (pointTwo.y < pointOne.y)
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

		// Calculate the angle using arctan
		// Then convert to degrees. (180 / 3.14159 = 57.2958)
		int angle = int(atan((float)yDist / (float)abs(xDist)) * 57);

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
		//                         90                                             |
		//                         ^                                              |
		//                 *       |       *                                      |
		//                  \      |      /                                       |
		//                   \     |     /                                        |
		//                    \    |    /                                         |
		// Quadrant 1          \   |   /         Quadrant 0                       |
		//                      \  |  /                                           |
		//                       \ | /                                            |
		//                angle ( \|/ ) -angle                                    |
		// 180 <----------------------------------------> 0                       |
		//               -angle ( /|\ )  angle                                    |
		//                       / | \                                            |
		//                      /  |  \                                           |
		// Quadrant 3          /   |   \         Quadrant 2                       |
		//                    /    |    \                                         |
		//                   /     |     \                                        |
		//                  /      |      \                                       |
		//                 *       |       *                                      |
		//                         ^                                              |
		//                        270                                             |

		// Convert the local angles to unit circle angles
		switch (quadrant) {
		case 0:
			angle = -angle;
			break;
		case 1:
			angle = angle + 180;
			break;
		case 2:
			angle = 360 - angle;
			break;
		case 3:
			angle = 180 + angle;
			break;
		default:
			break;
		}

		return angle;
	}
}

void LeverControl::renderFrame(uint frameNumber) {
	if (frameNumber == 0) {
		_lastRenderedFrame = frameNumber;
	} else if (frameNumber < _lastRenderedFrame && _mirrored) {
		_lastRenderedFrame = frameNumber;
		frameNumber = (_frameCount * 2) - frameNumber - 1;
	} else {
		_lastRenderedFrame = frameNumber;
	}

	const Graphics::Surface *frameData;

	_animation->seekToFrame(frameNumber);
	frameData = _animation->decodeNextFrame();
	if (frameData)
		_engine->getRenderManager()->blitSurfaceToBkgScaled(*frameData, _animationCoords);
}

void LeverControl::getLevParams(const Common::String &inputStr, Common::String &parameter, Common::String &values) {
	const char *chrs = inputStr.c_str();
	uint lbr;

	for (lbr = 0; lbr < inputStr.size(); lbr++)
		if (chrs[lbr] == ':')
			break;

	if (lbr >= inputStr.size())
		return;

	uint rbr;

	for (rbr = lbr + 1; rbr < inputStr.size(); rbr++)
		if (chrs[rbr] == '~')
			break;

	if (rbr >= inputStr.size())
		return;

	parameter = Common::String(chrs, chrs + lbr);
	values = Common::String(chrs + lbr + 1, chrs + rbr);
}

} // End of namespace ZVision
