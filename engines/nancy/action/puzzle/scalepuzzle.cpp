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

#include "engines/nancy/nancy.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/resource.h"
#include "engines/nancy/sound.h"
#include "engines/nancy/input.h"
#include "engines/nancy/cursor.h"
#include "engines/nancy/util.h"

#include "engines/nancy/state/scene.h"
#include "engines/nancy/action/puzzle/scalepuzzle.h"

namespace Nancy {
namespace Action {

// Reads a count-prefixed array of int32 rects (one of the original's sprite-container
// sub-objects).
static void readRects(Common::SeekableReadStream &stream, Common::Array<Common::Rect> &rects) {
	uint16 count = stream.readUint16LE();
	rects.resize(count);
	for (uint i = 0; i < count; ++i) {
		readRect(stream, rects[i]);
	}
}

void ScalePuzzle::readData(Common::SeekableReadStream &stream) {
	readFilename(stream, _imageName);			// 0x00
	// The puzzle carries its own cursors as raw Nancy13 cursor type ids.
	_hoverCursorType = stream.readUint16LE();	// 0x21
	_dragCursorType = stream.readUint16LE();	// 0x23

	// Applied when the puzzle comes out solved. Several scenes use 9999 (no scene) and
	// leave the transition to whatever watches the solve flag.
	_solveScene.sceneID = stream.readUint16LE();	// 0x25
	_solveScene.continueSceneSound = kContinueSceneSound;
	_solveFlag.label = stream.readSint16LE();		// 0x27
	_solveFlag.flag = stream.readByte();			// 0x29

	_latchSound.readData(stream);				// played on give-up

	// The figures to match this scene: a required coin count, the figure's number, its
	// open-latch sprite and destination rects, and the sound played when it lights.
	int16 numTargets = stream.readSint16LE();
	_targets.resize(numTargets);
	for (int16 i = 0; i < numTargets; ++i) {
		Target &t = _targets[i];
		t.coinCount = stream.readByte();
		t.number = stream.readSint32LE();
		readRect(stream, t.latchSrc);
		readRect(stream, t.latchDst);
		t.sound.readData(stream);
	}

	// The coin definitions: the inventory item id gating each coin, its value, and its sprite
	// in the overlay image.
	int16 numCoins = stream.readSint16LE();
	_coins.resize(numCoins);
	for (int16 i = 0; i < numCoins; ++i) {
		Coin &c = _coins[i];
		c.itemID = stream.readSint16LE();
		c.value = stream.readSint32LE();
		readRect(stream, c.src);
	}

	// The three slot groups, then a couple of loose sprite rects, the red-light atlas, the
	// indicator atlas and the indicator's on-screen rect.
	readRects(stream, _left.dests);		// container 0xb3 - left pan slots
	readRects(stream, _right.dests);	// container 0xbd - right pan slots
	readRects(stream, _tray.dests);		// container 0xc7 - source-tray slots
	readRect(stream, _coinBackSrc);		// 0xd1
	readRects(stream, _pileFrames);		// 0xe1
	readRect(stream, _altSrc);			// 0xef
	readRects(stream, _lightFrames);	// 0xff - red lights
	readRects(stream, _indicatorFrames);	// 0x85 - indicator digits (-N..+N)
	readRect(stream, _indicatorDest);	// 0x8f

	_pickupSound.readData(stream);		// 0x170
	_dropTraySound.readData(stream);	// 0x218
	_dropPanSound.readData(stream);		// 0x1c4

	// A count-prefixed array of fixed 23-byte hotspot records:
	// {rect, u16 cursorType, u16 sceneID, u16 frameID, byte}. The sample carries one - the
	// "give up / exit" hotspot, with the exit cursor type.
	int16 numZones = stream.readSint16LE();
	for (int16 i = 0; i < numZones; ++i) {
		Common::Rect r;
		readRect(stream, r);
		uint16 cursorType = stream.readUint16LE();
		uint16 sceneID = stream.readUint16LE();
		int16 flagLabel = stream.readSint16LE();
		byte flagValue = stream.readByte();

		if (i == 0) {
			_exitHotspot = r;
			_exitCursorType = cursorType;
			_exitScene.sceneID = sceneID;
			// The field after the scene id is a flag label (set on give-up), not a frame; the
			// exit always goes to the scene's first frame.
			_exitScene.frameID = 0;
			_exitFlag.label = flagLabel;
			_exitFlag.flag = flagValue;
		}
	}
}

void ScalePuzzle::init() {
	Common::Rect vpBounds = NancySceneState.getViewport().getBounds();
	_drawSurface.create(vpBounds.width(), vpBounds.height(), g_nancy->_graphics->getInputPixelFormat());
	_drawSurface.clear(g_nancy->_graphics->getTransColor());
	setTransparent(true);
	setVisible(true);
	moveTo(vpBounds);

	g_nancy->_resource->loadImage(_imageName, _image);
	_image.setTransparentColor(_drawSurface.getTransparentColor());

	// The indicator reads zero at the middle frame of its strip; the running total shifts it.
	_indicatorZeroFrame = _indicatorFrames.size() / 2;

	// Every slot starts empty.
	_left.coins.resize(_left.dests.size());
	_right.coins.resize(_right.dests.size());
	_tray.coins.resize(_tray.dests.size());
	for (uint i = 0; i < _left.coins.size(); ++i) {
		_left.coins[i] = kNoCoin;
	}
	for (uint i = 0; i < _right.coins.size(); ++i) {
		_right.coins[i] = kNoCoin;
	}
	for (uint i = 0; i < _tray.coins.size(); ++i) {
		_tray.coins[i] = kNoCoin;
	}

	// Place only the coins the player is carrying: the original filters the coin list by the
	// inventory "has item" state (coins are collected over the course of the game), so coins
	// the player does not own are never placed.
	Common::Array<int16> available;
	for (uint i = 0; i < _coins.size(); ++i) {
		if (NancySceneState.hasItem(_coins[i].itemID) == g_nancy->_true) {
			available.push_back((int16)i);
		}
	}

	Common::Array<uint> freeSlots;
	for (uint i = 0; i < _tray.dests.size(); ++i) {
		freeSlots.push_back(i);
	}
	for (uint i = 0; i < available.size() && !freeSlots.empty(); ++i) {
		uint k = g_nancy->_randomSource->getRandomNumber(freeSlots.size() - 1);
		_tray.coins[freeSlots[k]] = available[i];
		freeSlots.remove_at(k);
	}

	recomputeBalance();

	NancySceneState.setNoHeldItem();

	redraw();
	registerGraphics();
}

ScalePuzzle::SlotGroup &ScalePuzzle::group(SlotRegion region) {
	switch (region) {
	case kLeftPan:
		return _left;
	case kRightPan:
		return _right;
	default:
		return _tray;
	}
}

bool ScalePuzzle::slotAtCursor(const Common::Point &mousePos, bool wantEmpty, SlotRegion &outRegion, uint &outIndex) const {
	static const SlotRegion order[3] = { kSourceTray, kLeftPan, kRightPan };
	for (int g = 0; g < 3; ++g) {
		const SlotGroup &grp = (order[g] == kLeftPan) ? _left : (order[g] == kRightPan) ? _right : _tray;
		for (uint i = 0; i < grp.dests.size(); ++i) {
			bool empty = (grp.coins[i] == kNoCoin);
			if (empty != wantEmpty) {
				continue;
			}
			if (NancySceneState.getViewport().convertViewportToScreen(grp.dests[i]).contains(mousePos)) {
				outRegion = order[g];
				outIndex = i;
				return true;
			}
		}
	}
	return false;
}

void ScalePuzzle::recomputeBalance() {
	_tilt = 0;
	_placedCount = 0;

	// The indicator reads sum(right pan values) - sum(left pan values).
	for (uint i = 0; i < _left.coins.size(); ++i) {
		if (_left.coins[i] != kNoCoin) {
			_tilt -= _coins[_left.coins[i]].value;
			++_placedCount;
		}
	}
	for (uint i = 0; i < _right.coins.size(); ++i) {
		if (_right.coins[i] != kNoCoin) {
			_tilt += _coins[_right.coins[i]].value;
			++_placedCount;
		}
	}

	// Match the figures in order (mirrors FUN_0046b8b0). Walk the targets skipping the ones
	// already lit; act on the first target whose required coin count equals the number of
	// coins now on the pans. If the indicator magnitude matches its number it lights (and
	// plays its latch sound the first time); otherwise it, and every later target, is
	// cleared. A target that is not yet lit stops the walk, so they can only be lit in turn.
	for (uint i = 0; i < _targets.size(); ++i) {
		Target &t = _targets[i];
		if (_placedCount == t.coinCount) {
			if ((int)ABS(_tilt) == t.number) {
				if (!t.lit) {
					t.lit = true;
					_endSound = playSoundBlock(t.sound);
				}
			} else {
				for (uint j = i; j < _targets.size(); ++j) {
					_targets[j].lit = false;
				}
			}
			break;
		}
		if (!t.lit) {
			break;
		}
	}

	// Clearing the pans resets every light.
	if (_placedCount == 0) {
		for (uint i = 0; i < _targets.size(); ++i) {
			_targets[i].lit = false;
		}
	}

	// Solved once every figure of the scene is lit.
	_solved = !_targets.empty();
	for (uint i = 0; i < _targets.size(); ++i) {
		if (!_targets[i].lit) {
			_solved = false;
		}
	}
}

void ScalePuzzle::blitCentered(const Common::Rect &src, const Common::Rect &slot) {
	int x = slot.left + (slot.width() - src.width()) / 2;
	int y = slot.top + (slot.height() - src.height()) / 2;
	_drawSurface.blitFrom(_image, src, Common::Point(x, y));
}

void ScalePuzzle::redraw() {
	_drawSurface.clear(g_nancy->_graphics->getTransColor());

	// The coins resting in the tray and on the two pans, each centred in its slot.
	const SlotGroup *groups[3] = { &_tray, &_left, &_right };
	for (int g = 0; g < 3; ++g) {
		const SlotGroup &grp = *groups[g];
		for (uint i = 0; i < grp.dests.size(); ++i) {
			int16 coin = grp.coins[i];
			if (coin == kNoCoin) {
				continue;
			}
			blitCentered(_coins[coin].src, grp.dests[i]);
		}
	}

	// The numeric indicator, showing the running total.
	if (!_indicatorFrames.empty()) {
		int idx = CLIP<int>(_indicatorZeroFrame + _tilt, 0, (int)_indicatorFrames.size() - 1);
		_drawSurface.blitFrom(_image, _indicatorFrames[idx], Common::Point(_indicatorDest.left, _indicatorDest.top));
	}

	// The coin-count token, drawn at the position for the current number of coins on the pans.
	if (_placedCount > 0 && (uint)(_placedCount - 1) < _pileFrames.size() && !_coinBackSrc.isEmpty()) {
		const Common::Rect &pos = _pileFrames[_placedCount - 1];
		_drawSurface.blitFrom(_image, _coinBackSrc, Common::Point(pos.left, pos.top));
	}

	// Each matched figure: its red light (the shared light sprite drawn at the light slot for
	// its coin count) and its open-latch bar.
	for (uint i = 0; i < _targets.size(); ++i) {
		const Target &t = _targets[i];
		if (!t.lit) {
			continue;
		}
		int lightIdx = t.coinCount - 1;
		if (lightIdx >= 0 && (uint)lightIdx < _lightFrames.size() && !_altSrc.isEmpty()) {
			const Common::Rect &pos = _lightFrames[lightIdx];
			_drawSurface.blitFrom(_image, _altSrc, Common::Point(pos.left, pos.top));
		}
		_drawSurface.blitFrom(_image, t.latchSrc, Common::Point(t.latchDst.left, t.latchDst.top));
	}

	// The coin currently being carried, following the cursor.
	if (_carriedCoin != kNoCoin) {
		const Common::Rect &src = _coins[_carriedCoin].src;
		_drawSurface.blitFrom(_image, src, Common::Point(_dragPos.x - src.width() / 2, _dragPos.y - src.height() / 2));
	}

	_needsRedraw = true;
}

void ScalePuzzle::setDataCursor(uint16 cursorType) const {
	// The ids in the AR data are raw Nancy13 cursor types, which is exactly what the
	// "set from script" path expects.
	g_nancy->_cursor->setCursorType((CursorManager::CursorType)cursorType, true);
}

SoundDescription ScalePuzzle::playSoundBlock(const RandomSoundBlock &block) {
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

void ScalePuzzle::execute() {
	switch (_state) {
	case kBegin:
		init();
		_state = kRun;
		// fall through
	case kRun:
		if (_exitRequested) {
			_state = kActionTrigger;
			break;
		}

		if (_solved) {
			bool soundDone = _endSound.name.empty() || !g_nancy->_sound->isSoundPlaying(_endSound);
			if (soundDone) {
				_state = kActionTrigger;
			}
		}
		break;
	case kActionTrigger:
		if (_exitRequested) {
			NancySceneState.setEventFlag(_exitFlag);
			if (_exitScene.sceneID != kNoScene) {
				NancySceneState.changeScene(_exitScene);
			}
		} else {
			// Solved: play the latch sound, set the solve flag, change scene (9999 = stay).
			playSoundBlock(_latchSound);
			NancySceneState.setEventFlag(_solveFlag);
			if (_solveScene.sceneID != kNoScene) {
				NancySceneState.changeScene(_solveScene);
			}
		}

		finishExecution();
		break;
	}
}

void ScalePuzzle::handleInput(NancyInput &input) {
	if (_state != kRun || _solved) {
		return;
	}

	const bool click = (input.input & NancyInput::kLeftMouseButtonUp) != 0;

	// -- Carrying a coin: it follows the cursor; drop it on an empty slot. --
	if (_carriedCoin != kNoCoin) {
		setDataCursor(_dragCursorType);

		Common::Rect screenPt(input.mousePos.x, input.mousePos.y, input.mousePos.x + 1, input.mousePos.y + 1);
		Common::Rect vpPt = NancySceneState.getViewport().convertScreenToViewport(screenPt);
		_dragPos = Common::Point(vpPt.left, vpPt.top);
		redraw();

		if (click) {
			SlotRegion region;
			uint idx;
			if (slotAtCursor(input.mousePos, true, region, idx)) {
				group(region).coins[idx] = _carriedCoin;
				_carriedCoin = kNoCoin;
				playSoundBlock(region == kSourceTray ? _dropTraySound : _dropPanSound);
				recomputeBalance();
				redraw();
			}
		}

		input.eatMouseInput();
		return;
	}

	// -- Not carrying: pick up a coin, or leave via the exit hotspot. --
	SlotRegion region;
	uint idx;
	if (slotAtCursor(input.mousePos, false, region, idx)) {
		setDataCursor(_hoverCursorType);
		if (click) {
			SlotGroup &grp = group(region);
			_carriedCoin = grp.coins[idx];
			grp.coins[idx] = kNoCoin;

			Common::Rect screenPt(input.mousePos.x, input.mousePos.y, input.mousePos.x + 1, input.mousePos.y + 1);
			Common::Rect vpPt = NancySceneState.getViewport().convertScreenToViewport(screenPt);
			_dragPos = Common::Point(vpPt.left, vpPt.top);

			playSoundBlock(_pickupSound);
			recomputeBalance();
			redraw();
		}
		input.eatMouseInput();
		return;
	}

	if (!_exitHotspot.isEmpty() &&
			NancySceneState.getViewport().convertViewportToScreen(_exitHotspot).contains(input.mousePos)) {
		setDataCursor(_exitCursorType);
		if (click) {
			_exitRequested = true;
		}
	}
}

} // End of namespace Action
} // End of namespace Nancy
