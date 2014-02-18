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
#include "zvision/cursors/cursor_manager.h"
#include "zvision/animation/rlf_animation.h"
#include "zvision/video/zork_avi_decoder.h"
#include "zvision/utility/utility.h"

#include "common/stream.h"
#include "common/file.h"
#include "common/tokenizer.h"
#include "common/system.h"

#include "graphics/surface.h"


namespace ZVision {

LeverControl::LeverControl(ZVision *engine, uint32 key, Common::SeekableReadStream &stream)
		: Control(engine, key),
		  _frameInfo(0),
		  _frameCount(0),
		  _startFrame(0),
		  _currentFrame(0),
		  _lastRenderedFrame(0),
		  _mouseIsCaptured(false),
		  _isReturning(false),
		  _accumulatedTime(0),
		  _returnRoutesCurrentFrame(0) {

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
	
	delete[] _frameInfo;
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
				_fileType = AVI;
			} else if (animationFileName.hasSuffix(".rlf")) {
				_animation.rlf = new RlfAnimation(animationFileName, false);
				_fileType = RLF;
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

			Common::StringTokenizer tokenizer(line, " ^=()");
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

		line = file.readLine();
	}
}

void LeverControl::onMouseDown(const Common::Point &screenSpacePos, const Common::Point &backgroundImageSpacePos) {
	if (!_enabled) {
		return;
	}
	
	if (_frameInfo[_currentFrame].hotspot.contains(backgroundImageSpacePos)) {
		_mouseIsCaptured = true;
		_lastMousePos = backgroundImageSpacePos;
	}
}

void LeverControl::onMouseUp(const Common::Point &screenSpacePos, const Common::Point &backgroundImageSpacePos) {
	if (!_enabled) {
		return;
	}
	
	if (_mouseIsCaptured) {
		_mouseIsCaptured = false;
		_engine->getScriptManager()->setStateValue(_key, _currentFrame);

		_isReturning = true;
		_returnRoutesCurrentProgress = _frameInfo[_currentFrame].returnRoute.begin();
		_returnRoutesCurrentFrame = _currentFrame;
	}
}

bool LeverControl::onMouseMove(const Common::Point &screenSpacePos, const Common::Point &backgroundImageSpacePos) {
	if (!_enabled) {
		return false;
	}
	
	bool cursorWasChanged = false;

	if (_mouseIsCaptured) {
		// Make sure the square distance between the last point and the current point is greater than 64
		// This is a heuristic. This determines how responsive the lever is to mouse movement.
		// TODO: Fiddle with the heuristic to get a good lever responsiveness 'feel'
		if (_lastMousePos.sqrDist(backgroundImageSpacePos) >= 64) {
			int angle = calculateVectorAngle(_lastMousePos, backgroundImageSpacePos);
			_lastMousePos = backgroundImageSpacePos;

			for (Common::List<Direction>::iterator iter = _frameInfo[_currentFrame].directions.begin(); iter != _frameInfo[_currentFrame].directions.end(); ++iter) {
				if (angle >= (int)iter->angle - ANGLE_DELTA && angle <= (int)iter->angle + ANGLE_DELTA) {
					_currentFrame = iter->toFrame;
					renderFrame(_currentFrame);
					break;
				}
			}
		}
	} else if (_frameInfo[_currentFrame].hotspot.contains(backgroundImageSpacePos)) {
		_engine->getCursorManager()->changeCursor(_cursorName);
		cursorWasChanged = true;
	}

	return cursorWasChanged;
}

bool LeverControl::process(uint32 deltaTimeInMillis) {
	if (!_enabled) {
		return false;
	}

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
		int angle = int(atan((float)yDist / (float)xDist) * 57);

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
			angle = 180 + angle;
			break;
		case 1:
			// Do nothing
			break;
		case 2:
			angle = 180 + angle;
			break;
		case 3:
			angle = 360 + angle;
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

	const uint16 *frameData = 0;
	int x = _animationCoords.left;
	int y = _animationCoords.top;
	int width = 0;
	int height = 0;

	if (_fileType == RLF) {
		// getFrameData() will automatically optimize to getNextFrame() / getPreviousFrame() if it can
		frameData = (const uint16 *)_animation.rlf->getFrameData(frameNumber)->getPixels();
		width = _animation.rlf->width(); // Use the animation width instead of _animationCoords.width()
		height = _animation.rlf->height(); // Use the animation height instead of _animationCoords.height()			
	} else if (_fileType == AVI) {
		_animation.avi->seekToFrame(frameNumber);
		const Graphics::Surface *surface = _animation.avi->decodeNextFrame();
		frameData = (const uint16 *)surface->getPixels();
		width = surface->w;
		height = surface->h;
	}

	_engine->getRenderManager()->copyRectToWorkingWindow(frameData, x, y, width, width, height);
}

} // End of namespace ZVision
