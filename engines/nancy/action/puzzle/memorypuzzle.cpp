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
#include "engines/nancy/graphics.h"
#include "engines/nancy/resource.h"
#include "engines/nancy/sound.h"
#include "engines/nancy/input.h"
#include "engines/nancy/util.h"

#include "engines/nancy/state/scene.h"
#include "engines/nancy/action/puzzle/memorypuzzle.h"

namespace Nancy {
namespace Action {

// Binary layout (offsets from stream start, all LE):
//   0x000  33  image filename
//   0x021 576  36 x face src rect (int32 l/t/r/b x 4) [types 0..35]
//   0x261  48  3 x tab indicator src rect (one per tab)
//   0x291 384  24 x card screen rect (viewport-relative, int32 x 4)
//   0x411  16  tab rect (screen rect where the active-tab graphic is drawn)
//   0x421 144  3 x 3 x 16 tab hotspot rects [currentTab][targetTab]
//   0x4b1   4  flipDelay (uint32, milliseconds)
//   0x4b5   4  numPairs (uint32; clamped [4..36] in init)
//   0x4b9   4  requiredPairs (uint32; clamped [2..36] in init)
//   0x4bd   1  cursor flag (byte; skip in ScummVM)
//   0x4be   1  shuffle flag (0 = pairs stay within tab, nonzero = global)
//   0x4bf  49  match sound (played when a matching pair is found)
//   0x4f0  49  card flip sound (played when flipping a card face-up)
//   0x521  25  win SceneChangeWithFlag
//   0x53a   1  unknown (skip)
//   0x53b  49  win sound
//   0x56c  16  exit hotspot rect
//   [total: 0x57c = 1404 bytes]
void MemoryPuzzle::readData(Common::SeekableReadStream &stream) {
	// 0x000: image filename (33 bytes)
	readFilename(stream, _imageName);

	// 0x021: 36 face src rects (for types 0..35)
	for (int i = 0; i < kMaxTypes; ++i)
		readRect(stream, _faceSrcRects[i]);

	// 0x261: tab indicator src rects (one per tab; drawn over the active tab button)
	for (int tab = 0; tab < kNumTabs; ++tab)
		readRect(stream, _tabSrcRects[tab]);

	// 0x291: 24 card screen-position rects (viewport-relative)
	for (int i = 0; i < kCardsPerTab; ++i)
		readRect(stream, _cardRects[i]);

	// 0x411: tab rect (screen destination for the active tab indicator)
	readRect(stream, _tabRect);

	// 0x421: tab hotspot rects - 3 tabs x 3 slots x 16 bytes
	for (int tab = 0; tab < kNumTabs; ++tab)
		for (int slot = 0; slot < 3; ++slot)
			readRect(stream, _tabHotspots[tab][slot]);

	// 0x4b1: flipDelay (uint32), numPairs (uint32), requiredPairs (uint32)
	_flipDelay     = stream.readUint32LE();
	_numPairs      = stream.readUint32LE();
	_requiredPairs = stream.readUint32LE();

	// 0x4bd: cursor flag (ignored in ScummVM)
	stream.skip(1);

	// 0x4be: shuffle flag
	_shuffleGlobal = (stream.readByte() != 0);

	// 0x4bf: match sound; 0x4f0: card flip sound
	_matchSound.readNormal(stream);
	_cardFlipSound.readNormal(stream);

	// 0x521: win scene + flag
	_winScene.readData(stream);

	stream.skip(1); // 0x53a: unknown

	// 0x53b: win sound
	_winSound.readNormal(stream);

	// 0x56c: exit hotspot
	readRect(stream, _exitHotspot);
}

// Shuffles type IDs (0..numPairs-1) into the 72-card array so that every type
// appears exactly twice. numPairs is clamped to [4, 36]; cards beyond numPairs
// remain typeId -1 (unassigned, unselectable). requiredPairs is clamped to [2, totalCards/2].
void MemoryPuzzle::initCards() {
	_numPairs = CLIP<uint32>(_numPairs, 4, (uint32)kMaxTypes);

	const int totalCards = kNumTabs * kCardsPerTab;
	const uint32 maxRequire = (uint32)(totalCards / 2);
	_requiredPairs = CLIP<uint32>(_requiredPairs, 2, maxRequire);

	// Init all cards
	for (int i = 0; i < totalCards; ++i) {
		_cards[i].typeId     = -1;
		_cards[i].flipState  = 0;
		_cards[i].matchState = 0;
	}
	_matchedPairs    = 0;
	_firstFlip       = -1;
	_secondFlip      = -1;
	_flipTimerActive = false;

	int nextType = 0;

	if (!_shuffleGlobal) {
		// By-tab: pairs are always within the same tab.
		for (int tab = 0; tab < kNumTabs; ++tab) {
			int base = tab * kCardsPerTab;
			for (int i = 0; i < kCardsPerTab; ++i) {
				if (_cards[base + i].typeId != -1)
					continue;
				if (static_cast<uint32>(nextType) >= _numPairs)
					break; // all types used up for this tab

				_cards[base + i].typeId = nextType;

				// Find a random unassigned slot in the same tab for the pair
				int partner;
				do {
					partner = g_nancy->_randomSource->getRandomNumber(kCardsPerTab - 1);
				} while (_cards[base + partner].typeId != -1);
				_cards[base + partner].typeId = nextType;

				++nextType;
			}
		}
	} else {
		// Global: pairs may be in different tabs.
		for (int i = 0; i < totalCards; ++i) {
			if (_cards[i].typeId != -1)
				continue;
			if (static_cast<uint32>(nextType) >= _numPairs)
				break;

			_cards[i].typeId = nextType;

			// Find a random unassigned slot anywhere
			int partner;
			do {
				partner = g_nancy->_randomSource->getRandomNumber(totalCards - 1);
			} while (_cards[partner].typeId != -1);
			_cards[partner].typeId = nextType;

			++nextType;
		}
	}
}

void MemoryPuzzle::init() {
	Common::Rect vpBounds = NancySceneState.getViewport().getBounds();
	_drawSurface.create(vpBounds.width(), vpBounds.height(),
		g_nancy->_graphics->getInputPixelFormat());
	_drawSurface.clear(g_nancy->_graphics->getTransColor());
	setTransparent(true);
	setVisible(true);
	moveTo(vpBounds);

	g_nancy->_resource->loadImage(_imageName, _image);
	_image.setTransparentColor(_drawSurface.getTransparentColor());

	_currentTab = 0;
	initCards();
	redrawCards();
}

void MemoryPuzzle::execute() {
	switch (_state) {
	case kBegin:
		init();
		registerGraphics();
		if (_cardFlipSound.name != "NO SOUND")
			g_nancy->_sound->loadSound(_cardFlipSound);
		if (_matchSound.name != "NO SOUND")
			g_nancy->_sound->loadSound(_matchSound);
		_state = kRun;
		// fall through

	case kRun:
		switch (_solveSubState) {
		case kPlaying:
			// Flip-back timer: hide non-matching cards when timer expires
			if (_flipTimerActive && g_system->getMillis() >= _flipTimerEnd)
				flipBackCards();
			checkIfSolved();
			if (_isSolved)
				_solveSubState = kPlayWinSound;
			break;

		case kPlayWinSound:
			if (_winSound.name != "NO SOUND") {
				g_nancy->_sound->loadSound(_winSound);
				g_nancy->_sound->playSound(_winSound);
				_solveSubState = kWaitWinSound;
			} else {
				_state = kActionTrigger;
			}
			break;

		case kWaitWinSound:
			if (!g_nancy->_sound->isSoundPlaying(_winSound)) {
				g_nancy->_sound->stopSound(_winSound);
				_state = kActionTrigger;
			}
			break;
		}
		break;

	case kActionTrigger:
		g_nancy->_sound->stopSound(_cardFlipSound);
		g_nancy->_sound->stopSound(_matchSound);
		g_nancy->_sound->stopSound(_winSound);
		_winScene.execute();
		finishExecution();
		break;
	}
}

void MemoryPuzzle::handleInput(NancyInput &input) {
	if (_state != kRun || _solveSubState != kPlaying || _isSolved)
		return;

	Common::Rect vpScreen = NancySceneState.getViewport().getScreenPosition();
	Common::Point mouseVP = input.mousePos - Common::Point(vpScreen.left, vpScreen.top);

	// Exit hotspot
	if (!_exitHotspot.isEmpty() && _exitHotspot.contains(mouseVP)) {
		g_nancy->_cursor->setCursorType(g_nancy->_cursor->_puzzleExitCursor);
		if (input.input & NancyInput::kLeftMouseButtonUp)
			_state = kActionTrigger;
		return;
	}

	// Tab switching: _tabHotspots[currentTab][slot] where slot is the target tab
	for (int slot = 0; slot < kNumTabs; ++slot) {
		if (_tabHotspots[_currentTab][slot].contains(mouseVP)) {
			g_nancy->_cursor->setCursorType(CursorManager::kHotspot);
			if ((input.input & NancyInput::kLeftMouseButtonUp) && slot != _currentTab) {
				// Cancel any pending flip-back timer and flip both cards back
				if (_flipTimerActive)
					flipBackCards();
				// Also flip back any lone first-flip card
				if (_firstFlip != -1) {
					_cards[_firstFlip].flipState = 0;
					_firstFlip = -1;
				}
				_currentTab = slot;
				redrawCards();
			}
			return;
		}
	}

	// Card clicks are blocked while the flip-back timer is running
	if (_flipTimerActive)
		return;

	if (!(input.input & NancyInput::kLeftMouseButtonUp))
		return;

	int base = _currentTab * kCardsPerTab;
	for (int i = 0; i < kCardsPerTab; ++i) {
		if (!_cardRects[i].contains(mouseVP))
			continue;

		int idx = base + i;
		CardState &card = _cards[idx];

		// Unassigned or already matched or face-up: ignore
		if (card.typeId == -1 || card.matchState != 0 || card.flipState != 0)
			return;

		// Flip this card face-up
		card.flipState = 1;
		if (_cardFlipSound.name != "NO SOUND")
			g_nancy->_sound->playSound(_cardFlipSound);
		redrawCards();

		if (_firstFlip == -1) {
			// First card of a potential pair
			_firstFlip = idx;
		} else {
			// Second card flipped: check for match
			CardState &first = _cards[_firstFlip];
			if (first.typeId == card.typeId && first.typeId != -1) {
				// Match! Mark both as matched
				first.matchState = 1;
				card.matchState  = 1;
				first.flipState  = 0;
				card.flipState   = 0;
				++_matchedPairs;
				_firstFlip = -1;
				if (_matchSound.name != "NO SOUND")
					g_nancy->_sound->playSound(_matchSound);
				redrawCards();
			} else {
				// No match: start flip-back timer
				_secondFlip      = idx;
				_flipTimerActive = true;
				_flipTimerEnd    = g_system->getMillis() + _flipDelay;
			}
		}
		return;
	}
}

void MemoryPuzzle::checkIfSolved() {
	if (static_cast<uint32>(_matchedPairs) >= _requiredPairs)
		_isSolved = true;
}

void MemoryPuzzle::flipBackCards() {
	if (_firstFlip != -1) {
		_cards[_firstFlip].flipState = 0;
		_firstFlip = -1;
	}
	if (_secondFlip != -1) {
		_cards[_secondFlip].flipState = 0;
		_secondFlip = -1;
	}
	_flipTimerActive = false;
	_flipTimerEnd    = 0;
	redrawCards();
}

void MemoryPuzzle::redrawCards() {
	_drawSurface.clear(_drawSurface.getTransparentColor());

	// Draw the active tab indicator over the corresponding tab button.
	// The scene background shows inactive tab visuals; the overlay only marks the active one.
	if (_currentTab < kNumTabs && !_tabSrcRects[_currentTab].isEmpty())
		_drawSurface.blitFrom(_image, _tabSrcRects[_currentTab],
			Common::Point(_tabRect.left, _tabRect.top));

	// Draw face-up and matched cards. Face-down cards are left transparent so
	// the scene background (which carries the card-back visual) shows through.
	int base = _currentTab * kCardsPerTab;
	for (int i = 0; i < kCardsPerTab; ++i) {
		int idx = base + i;
		const CardState &card = _cards[idx];
		const Common::Rect &dest = _cardRects[i];

		if (card.matchState != 0 || card.flipState != 0) {
			int t = card.typeId;
			if (t >= 0 && t < kMaxTypes && !_faceSrcRects[t].isEmpty())
				_drawSurface.blitFrom(_image, _faceSrcRects[t],
					Common::Point(dest.left, dest.top));
		}
	}
	_needsRedraw = true;
}

} // End of namespace Action
} // End of namespace Nancy
