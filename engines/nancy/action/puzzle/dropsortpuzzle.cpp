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

#include "common/random.h"
#include "common/util.h"

#include "graphics/font.h"

#include "engines/nancy/nancy.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/resource.h"
#include "engines/nancy/sound.h"
#include "engines/nancy/input.h"
#include "engines/nancy/cursor.h"
#include "engines/nancy/util.h"

#include "engines/nancy/state/scene.h"
#include "engines/nancy/action/puzzle/dropsortpuzzle.h"

namespace Nancy {
namespace Action {

// Reads a count-prefixed array of int32 rects.
static void readRects(Common::SeekableReadStream &stream, Common::Array<Common::Rect> &rects) {
	uint16 count = stream.readUint16LE();
	rects.resize(count);
	for (uint i = 0; i < count; ++i) {
		readRect(stream, rects[i]);
	}
}

void DropSortPuzzle::readData(Common::SeekableReadStream &stream) {
	readFilename(stream, _imageName);			// 0x00
	readFilename(stream, _hoseMovieName);		// 0x21
	readRect(stream, _hoseRect);				// 0x42
	_dispenseFrame = stream.readUint32LE();		// 0x52
	readFilename(stream, _conveyorMovieName);	// 0x56
	readRect(stream, _conveyorRect);			// 0x77
	_totalItems = stream.readUint32LE();		// 0x87

	// Belt path (left/right ends) and speed.
	readRect(stream, _beltLeft);
	readRect(stream, _beltRight);
	_beltSpeed = stream.readSint32LE();

	_dispenseInterval = stream.readSint32LE();
	_hoverCursorType = stream.readUint16LE();	// raw Nancy13 cursor type ids
	_dragCursorType = stream.readUint16LE();

	readRects(stream, _itemSrcRects);			// candy sprites in the overlay image
	_unk1ad = stream.readSint32LE();

	uint16 numBins = stream.readUint16LE();
	_bins.resize(numBins);
	for (uint i = 0; i < numBins; ++i) {
		Bin &b = _bins[i];
		readRect(stream, b.region);
		b.enabled = stream.readByte();
		uint16 numAccepted = stream.readUint16LE();
		b.accepted.resize(numAccepted);
		for (uint j = 0; j < numAccepted; ++j) {
			b.accepted[j] = stream.readSint16LE();
		}
	}

	_showCounter = stream.readByte();
	_counterX = stream.readSint32LE();		// viewport position of the "Taffy to sort" number
	_counterY = stream.readSint32LE();
	_atlasId = stream.readUint16LE();

	readRects(stream, _strikeSrcRects);			// strike marker sprites in the overlay image
	readRects(stream, _strikeDestRects);		// where the strike markers are drawn

	_pickupSound.readData(stream);
	_dropSound.readData(stream);
	_hornSound.readData(stream);

	// Win scene + flag. frameID 0xffff means "no specific frame" (target may be a video) - keep 0.
	_winScene.sceneID = stream.readUint16LE();
	uint16 winFrame = stream.readUint16LE();
	_winScene.frameID = (winFrame == 0xffff) ? 0 : winFrame;
	_winScene.continueSceneSound = kContinueSceneSound;
	_winFlag.label = stream.readSint16LE();
	_winFlag.flag = stream.readByte();

	_winSound.readData(stream);

	_loseScene.sceneID = stream.readUint16LE();
	uint16 loseFrame = stream.readUint16LE();
	_loseScene.frameID = (loseFrame == 0xffff) ? 0 : loseFrame;
	_loseScene.continueSceneSound = kContinueSceneSound;
	_loseFlag.label = stream.readSint16LE();
	_loseFlag.flag = stream.readByte();

	_loseSound.readData(stream);

	// Count-prefixed 23-byte hotspot records; the first is the "give up / exit" hotspot.
	int16 numZones = stream.readSint16LE();
	for (int16 i = 0; i < numZones; ++i) {
		Common::Rect r;
		readRect(stream, r);
		uint16 cursorType = stream.readUint16LE();
		uint16 sceneID = stream.readUint16LE();
		int16 exitFlagLabel = stream.readSint16LE();
		byte exitFlagValue = stream.readByte();

		if (i == 0) {
			_exitHotspot = r;
			_exitCursorType = cursorType;
			_exitScene.sceneID = sceneID;
			// The field after the scene id is a flag label (set on give-up), not a frame.
			_exitScene.frameID = 0;
			_exitFlag.label = exitFlagLabel;
			_exitFlag.flag = exitFlagValue;
		}
	}
}

void DropSortPuzzle::init() {
	Common::Rect vpBounds = NancySceneState.getViewport().getBounds();
	_drawSurface.create(vpBounds.width(), vpBounds.height(), g_nancy->_graphics->getInputPixelFormat());
	_drawSurface.clear(g_nancy->_graphics->getTransColor());
	setTransparent(true);
	setVisible(true);
	moveTo(vpBounds);

	g_nancy->_resource->loadImage(_imageName, _image);
	_image.setTransparentColor(_drawSurface.getTransparentColor());

	// Both animations loop for the whole puzzle.
	if (_conveyorMovie.loadFile(_conveyorMovieName)) {
		_conveyorMovie.playRange(0, _conveyorMovie.getFrameCount() - 1);
	}
	if (_hoseMovie.loadFile(_hoseMovieName)) {
		_hoseMovie.playRange(0, _hoseMovie.getFrameCount() - 1);
	}

	// Time to cross the belt, from its length and speed (guarded against bad data).
	int dist = ABS(_beltRight.left - _beltLeft.left);
	int speed = _beltSpeed > 0 ? _beltSpeed : 60;
	_travelDuration = dist > 0 ? (uint32)((int64)dist * 1000 / speed) : 8000;
	if (_travelDuration < 1000) {
		_travelDuration = 8000;
	}

	_lastDispenseTime = g_nancy->getTotalPlayTime();

	// The reject bin sits at the belt's end, so candies that fall off land in it.
	Common::Point fallPoint = beltPosition(1.0f);
	for (uint i = 0; i < _bins.size(); ++i) {
		if (_bins[i].region.contains(fallPoint)) {
			_rejectBin = (int)i;
			break;
		}
	}

	NancySceneState.setNoHeldItem();

	redraw();
	registerGraphics();
}

Common::Point DropSortPuzzle::beltPosition(float progress) const {
	int16 x = (int16)(_beltLeft.left + (_beltRight.left - _beltLeft.left) * progress);
	int16 y = (int16)((_beltLeft.top + _beltLeft.bottom) / 2);
	return Common::Point(x, y);
}

Common::Rect DropSortPuzzle::itemDestAt(const Common::Point &pos, int16 type) const {
	int w = _itemSrcRects[type].width();
	int h = _itemSrcRects[type].height();
	return Common::Rect((int16)(pos.x - w / 2), (int16)(pos.y - h / 2),
		(int16)(pos.x - w / 2 + w), (int16)(pos.y - h / 2 + h));
}

int DropSortPuzzle::itemAtCursor(const Common::Point &mousePos) const {
	uint32 now = g_nancy->getTotalPlayTime();
	// Oldest (front-most) candy first, so overlapping sprites pick the one nearest the bins.
	for (uint i = 0; i < _items.size(); ++i) {
		float progress = (float)(now - _items[i].spawnTime) / (float)_travelDuration;
		Common::Rect dest = itemDestAt(beltPosition(progress), _items[i].type);
		if (NancySceneState.getViewport().convertViewportToScreen(dest).contains(mousePos)) {
			return (int)i;
		}
	}
	return -1;
}

int DropSortPuzzle::binAtCursor(const Common::Point &mousePos) const {
	for (uint i = 0; i < _bins.size(); ++i) {
		if (NancySceneState.getViewport().convertViewportToScreen(_bins[i].region).contains(mousePos)) {
			return (int)i;
		}
	}
	return -1;
}

Common::Point DropSortPuzzle::cursorToViewport(const Common::Point &mousePos) const {
	Common::Rect screenPt(mousePos.x, mousePos.y, mousePos.x + 1, mousePos.y + 1);
	Common::Rect vpPt = NancySceneState.getViewport().convertScreenToViewport(screenPt);
	return Common::Point(vpPt.left, vpPt.top);
}

void DropSortPuzzle::applyDrop(int binIndex, int16 type) {
	if (binIndex < 0 || binIndex >= (int)_bins.size()) {
		return;
	}

	const Bin &b = _bins[binIndex];
	bool accepted = false;
	for (uint i = 0; i < b.accepted.size(); ++i) {
		if (b.accepted[i] == type) {
			accepted = true;
			break;
		}
	}

	if (accepted) {
		playSoundBlock(_dropSound);
	} else if (b.enabled) {
		// Wrong bin: a mistake.
		++_strikes;
		playSoundBlock(_hornSound);
	}
}

void DropSortPuzzle::redraw() {
	_drawSurface.clear(g_nancy->_graphics->getTransColor());

	uint32 now = g_nancy->getTotalPlayTime();

	// The conveyor belt, behind everything.
	if (_conveyorMovie.isVideoLoaded()) {
		_conveyorMovie.drawFrame(_drawSurface, Common::Point(_conveyorRect.left, _conveyorRect.top));
	}

	// The candies riding the belt, drawn on top of the belt.
	for (uint i = 0; i < _items.size(); ++i) {
		float progress = (float)(now - _items[i].spawnTime) / (float)_travelDuration;
		Common::Rect dest = itemDestAt(beltPosition(progress), _items[i].type);
		_drawSurface.blitFrom(_image, _itemSrcRects[_items[i].type], Common::Point(dest.left, dest.top));
	}

	// The hose, over the candies: a fresh candy stays hidden under it until the belt carries it out.
	if (_hoseMovie.isVideoLoaded()) {
		_hoseMovie.drawFrame(_drawSurface, Common::Point(_hoseRect.left, _hoseRect.top));
	}

	// The strike markers earned so far.
	for (int i = 0; i < _strikes && i < (int)_strikeDestRects.size() && i < (int)_strikeSrcRects.size(); ++i) {
		_drawSurface.blitFrom(_image, _strikeSrcRects[i],
			Common::Point(_strikeDestRects[i].left, _strikeDestRects[i].top));
	}

	// The candy currently being carried, following the cursor.
	if (_carriedType != kNoItem) {
		const Common::Rect &src = _itemSrcRects[_carriedType];
		int w = src.width();
		int h = src.height();
		_drawSurface.blitFrom(_image, src, Common::Point(_dragPos.x - w / 2, _dragPos.y - h / 2));
	}

	drawCounter();

	_needsRedraw = true;
}

// The "Taffy to sort" counter: candies still to be dispensed, drawn at the AR-data position
// with the AR-data font id.
void DropSortPuzzle::drawCounter() {
	if (!_showCounter) {
		return;
	}

	const Graphics::Font *font = g_nancy->_graphics->getFont(_atlasId);
	if (!font) {
		font = g_nancy->_graphics->getFont(0);
	}
	if (!font) {
		return;
	}

	int remaining = (int)_totalItems - (int)_numDispensed;
	if (remaining < 0) {
		remaining = 0;
	}

	Common::String str = Common::String::format("%d", remaining);
	int w = font->getStringWidth(str);
	if (w <= 0) {
		w = 24;
	}

	font->drawString(&_drawSurface, str, _counterX, _counterY, w + 4, 0);
}

SoundDescription DropSortPuzzle::playSoundBlock(const RandomSoundBlock &block) {
	SoundDescription desc;
	if (block.names.empty()) {
		return desc;
	}

	uint idx = block.names.size() == 1 ? 0 : g_nancy->_randomSource->getRandomNumber(block.names.size() - 1);
	const Common::String &name = block.names[idx];
	if (name.empty() || name == "NO SOUND") {
		return desc;
	}

	desc.name = name;
	desc.channelID = block.channel;
	desc.numLoops = block.numLoops > 0 ? block.numLoops : 1;
	desc.volume = block.volume;

	g_nancy->_sound->loadSound(desc);
	g_nancy->_sound->playSound(desc);
	return desc;
}

void DropSortPuzzle::execute() {
	switch (_state) {
	case kBegin:
		init();
		_state = kRun;
		// fall through
	case kRun: {
		uint32 now = g_nancy->getTotalPlayTime();

		if (_exitRequested) {
			_state = kActionTrigger;
			break;
		}

		if (!_ended) {
			// Keep both animations looping.
			if (_conveyorMovie.isVideoLoaded() && !_conveyorMovie.isRangePlaying()) {
				_conveyorMovie.playRange(0, _conveyorMovie.getFrameCount() - 1);
			}
			if (_hoseMovie.isVideoLoaded() && !_hoseMovie.isRangePlaying()) {
				_hoseMovie.playRange(0, _hoseMovie.getFrameCount() - 1);
			}
			bool moviesUpdated = false;
			if (_conveyorMovie.isVideoLoaded() && _conveyorMovie.update()) {
				moviesUpdated = true;
			}
			if (_hoseMovie.isVideoLoaded() && _hoseMovie.update()) {
				moviesUpdated = true;
			}

			// Dispense on the data's interval (ramps with the difficulty variant).
			int interval = _dispenseInterval > 0 ? _dispenseInterval : 2000;
			if (now - _lastDispenseTime >= (uint32)interval &&
					_numDispensed < _totalItems && !_itemSrcRects.empty()) {
				BeltItem item;
				item.type = (int16)g_nancy->_randomSource->getRandomNumber(_itemSrcRects.size() - 1);
				item.spawnTime = now;
				_items.push_back(item);
				++_numDispensed;
				_lastDispenseTime = now;
			}

			// Candies reaching the belt's end fall into the reject bin (a mistake unless it
			// accepts them).
			for (uint i = 0; i < _items.size();) {
				if (now - _items[i].spawnTime >= _travelDuration) {
					applyDrop(_rejectBin, _items[i].type);
					_items.remove_at(i);
				} else {
					++i;
				}
			}

			// The belt and candies move every frame, so keep the overlay in sync.
			if (moviesUpdated || !_items.empty() || _carriedType != kNoItem) {
				redraw();
			}

			// Lose on three strikes; win once all candies are dispensed and the belt is empty.
			if (_strikes >= (int)_strikeDestRects.size() && !_strikeDestRects.empty()) {
				_ended = true;
				_lost = true;
				_endSound = playSoundBlock(_loseSound);
				_endTime = now;
			} else if (_numDispensed >= _totalItems && _items.empty() && _carriedType == kNoItem) {
				_ended = true;
				_solved = true;
				_endSound = playSoundBlock(_winSound);
				_endTime = now;
			}
		}

		if (_ended) {
			bool soundDone = _endSound.name.empty() || !g_nancy->_sound->isSoundPlaying(_endSound);
			if (soundDone && now - _endTime > 500) {
				_state = kActionTrigger;
			}
		}

		break;
	}
	case kActionTrigger:
		if (_exitRequested) {
			NancySceneState.setEventFlag(_exitFlag);
			if (_exitScene.sceneID != kNoScene) {
				NancySceneState.changeScene(_exitScene);
			}
		} else if (_solved) {
			NancySceneState.setEventFlag(_winFlag);
			if (_winScene.sceneID != kNoScene) {
				NancySceneState.changeScene(_winScene);
			}
		} else {
			NancySceneState.setEventFlag(_loseFlag);
			if (_loseScene.sceneID != kNoScene) {
				NancySceneState.changeScene(_loseScene);
			}
		}

		finishExecution();
		break;
	}
}

void DropSortPuzzle::handleInput(NancyInput &input) {
	if (_state != kRun || _ended) {
		return;
	}

	const bool click = (input.input & NancyInput::kLeftMouseButtonUp) != 0;

	// -- Carrying a candy: it follows the cursor; drop it into a bin. --
	if (_carriedType != kNoItem) {
		// Raw Nancy13 cursor type ids from the AR data, applied via the "set from script" path.
		g_nancy->_cursor->setCursorType((CursorManager::CursorType)_dragCursorType, true);
		_dragPos = cursorToViewport(input.mousePos);
		redraw();

		if (click) {
			int bin = binAtCursor(input.mousePos);
			if (bin >= 0) {
				applyDrop(bin, _carriedType);
				_carriedType = kNoItem;
				redraw();
			}
		}

		input.eatMouseInput();
		return;
	}

	// -- Not carrying: pick a candy off the belt, or leave via the exit hotspot. --
	int item = itemAtCursor(input.mousePos);
	if (item >= 0) {
		g_nancy->_cursor->setCursorType((CursorManager::CursorType)_hoverCursorType, true);
		if (click) {
			_carriedType = _items[item].type;
			_items.remove_at(item);
			_dragPos = cursorToViewport(input.mousePos);
			playSoundBlock(_pickupSound);
			redraw();
		}
		input.eatMouseInput();
		return;
	}

	if (!_exitHotspot.isEmpty() &&
			NancySceneState.getViewport().convertViewportToScreen(_exitHotspot).contains(input.mousePos)) {
		g_nancy->_cursor->setCursorType((CursorManager::CursorType)_exitCursorType, true);
		if (click) {
			_exitRequested = true;
		}
	}
}

} // End of namespace Action
} // End of namespace Nancy
