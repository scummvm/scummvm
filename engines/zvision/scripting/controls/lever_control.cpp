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

#include "common/debug.h"
#include "common/file.h"
#include "common/scummsys.h"
#include "common/stream.h"
#include "common/system.h"
#include "common/tokenizer.h"
#include "graphics/surface.h"
#include "video/video_decoder.h"
#include "zvision/zvision.h"
#include "zvision/graphics/render_manager.h"
#include "zvision/graphics/cursors/cursor_manager.h"
#include "zvision/scripting/script_manager.h"
#include "zvision/scripting/controls/lever_control.h"

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
			if (sscanf(values.c_str(), "%24s", levFileName) == 1)
				parseLevFile(levFileName);
		} else if (param.matchString("cursor", true)) {
			char cursorName[25];
			if (sscanf(values.c_str(), "%24s", cursorName) == 1)
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

void LeverControl::parseLevFile(const Common::Path &fileName) {
	debugC(2, kDebugControl, "LeverControl::parseLevFile(%s)", fileName.toString().c_str());
	Common::File file;
	if (!file.open(fileName)) {
		warning("LEV file %s could could be opened", fileName.toString().c_str());
		return;
	}

	Common::String line;
	Common::String param;
	Common::String values;

	int id = 0;

	while (!file.eos()) {
		line = file.readLine();
		getLevParams(line, param, values);

		if (param.matchString("animation_id", true)) {
			sscanf(values.c_str(), "%d", &id);
			debugC(2, kDebugControl, "Lever animation ID: %d", id);
		} else if (param.matchString("filename", true)) {
			_animation = _engine->loadAnimation(Common::Path(values));
		} else if (param.matchString("skipcolor", true)) {
			// Not used
		} else if (param.matchString("anim_coords", true)) {
			int left, top, right, bottom;
			if (sscanf(values.c_str(), "%d %d %d %d", &left, &top, &right, &bottom) == 4) {
				_animationCoords.left = left;
				_animationCoords.top = top;
				_animationCoords.right = right;
				_animationCoords.bottom = bottom;
			}
		} else if (param.matchString("mirrored", true)) {
			uint mirrored;
			if (sscanf(values.c_str(), "%u", &mirrored) == 1)
				_mirrored = mirrored == 0 ? false : true;
		} else if (param.matchString("frames", true)) {
			if (sscanf(values.c_str(), "%u", &_frameCount) == 1)
				_frameInfo = new FrameInfo[_frameCount];
		} else if (param.matchString("elsewhere", true)) {
			// Not used
		} else if (param.matchString("out_of_control", true)) {
			// Not used
		} else if (param.matchString("start_pos", true)) {
			if (sscanf(values.c_str(), "%u", &_startFrame) == 1)
				_currentFrame = _startFrame;
		} else if (param.matchString("hotspot_deltas", true)) {
			uint x;
			uint y;
			if (sscanf(values.c_str(), "%u %u", &x, &y) == 2) {
				_hotspotDelta.x = x;
				_hotspotDelta.y = y;
			}
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
					if (sscanf(token.c_str(), "%u,%u", &toFrame, &angle) == 2)
						_frameInfo[frameNumber].paths.push_back(PathSegment(angle, toFrame));
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
	// WORKAROUND for a script bug in Zork: Nemesis, room tz2e (orrery)
	// Animation coordinates for left hand lever do not properly align with background image
	switch (id) {
	case 2926:
		_animationCoords.bottom -= 4;
		_animationCoords.right += 1;
		_animationCoords.left -= 1;
		_animationCoords.top += 1;
		break;
	default:
		break;
	}

	// Cycle through all unit direction vectors in path segments & determine step distance
	debugC(3, kDebugControl, "Setting step distances");
	for (uint frame=0; frame < _frameCount; frame++) {
		debugC(3, kDebugControl, "Frame %d", frame);
		for (auto &iter : _frameInfo[frame].paths) {
			uint destFrame = iter.toFrame;
			Common::Point deltaPos = _frameInfo[destFrame].hotspot.origin() - _frameInfo[frame].hotspot.origin();
			Math::Vector2d deltaPosVector((float)deltaPos.x, (float)deltaPos.y);
			iter.distance *= deltaPosVector.getMagnitude();
			debugC(3, kDebugControl, "\tdeltaPos = %d,%d, Distance %f", deltaPos.x, deltaPos.y, iter.distance);
		}
	}
	debugC(2, kDebugControl, "LeverControl::~parseLevFile()");
}

bool LeverControl::onMouseDown(const Common::Point &screenSpacePos, const Common::Point &backgroundImageSpacePos) {
	if (_engine->getScriptManager()->getStateFlag(_key) & Puzzle::DISABLED)
		return false;

	if (_frameInfo[_currentFrame].hotspot.contains(backgroundImageSpacePos)) {
		setVenus();
		_mouseIsCaptured = true;
		_gripOffset = backgroundImageSpacePos - _frameInfo[_currentFrame].hotspot.origin();
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
	_gripOffset = Common::Point(0,0);
	return false;
}

bool LeverControl::onMouseMove(const Common::Point &screenSpacePos, const Common::Point &backgroundImageSpacePos) {
	if (_engine->getScriptManager()->getStateFlag(_key) & Puzzle::DISABLED)
		return false;

	bool cursorWasChanged = false;

	if (_mouseIsCaptured) {
		uint nextFrame = _currentFrame;
		do {
			Common::Point gripOrigin = _frameInfo[_currentFrame].hotspot.origin() + _gripOffset;
			debugC(1, kDebugControl, "LeverControl::onMouseMove() screenPos = %d,%d, imagePos = %d,%d, gripOrigin = %d,%d", screenSpacePos.x, screenSpacePos.y, backgroundImageSpacePos.x, backgroundImageSpacePos.y, gripOrigin.x, gripOrigin.y);
			Common::Point deltaPos = backgroundImageSpacePos - gripOrigin;
			nextFrame = getNextFrame(deltaPos);
			if (nextFrame != _currentFrame) {
				_currentFrame = nextFrame;
				_engine->getScriptManager()->setStateValue(_key, _currentFrame);
			}
		} while (nextFrame != _currentFrame);

		if (_lastRenderedFrame != _currentFrame)
			renderFrame(_currentFrame);

		_engine->getCursorManager()->changeCursor(_cursor);
		cursorWasChanged = true;
	} else if (_frameInfo[_currentFrame].hotspot.contains(backgroundImageSpacePos)) {
		_engine->getCursorManager()->changeCursor(_cursor);
		cursorWasChanged = true;
	}
	return cursorWasChanged;
}

uint LeverControl::getNextFrame(Common::Point &deltaPos) {
	Math::Vector2d movement((float)deltaPos.x, (float)deltaPos.y);
	for (auto &iter : _frameInfo[_currentFrame].paths) {
		debugC(1, kDebugControl, "\tPossible step = %f,%f, angle = %d, distance %f", iter.direction.getX(), iter.direction.getY(), (uint)Math::rad2deg(iter.angle), iter.distance);
		if (movement.dotProduct(iter.direction) >= iter.distance/2) {
			return iter.toFrame;
		}
	}
	return _currentFrame;
}

bool LeverControl::process(uint32 deltaTimeInMillis) {
	if (_engine->getScriptManager()->getStateFlag(_key) & Puzzle::DISABLED)
		return false;

	if (_isReturning) {
		_accumulatedTime += deltaTimeInMillis;
		while (_accumulatedTime >= _returnFramePeriod) {
			_accumulatedTime -= _returnFramePeriod;
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

void LeverControl::renderFrame(uint frameNumber) {
	_lastRenderedFrame = frameNumber;
	if (frameNumber != 0 && frameNumber < _lastRenderedFrame && _mirrored)
		frameNumber = (_frameCount * 2) - frameNumber - 1;

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
