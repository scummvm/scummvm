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

void MemoryPuzzle::readData(Common::SeekableReadStream &stream) {
	if (g_nancy->getGameType() >= kGameTypeNancy11) {
		readDataNancy11(stream);
		return;
	}

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

// Nancy 11 reworked the layout: fewer (12) face rects, a configurable grid/page count,
// per-card-type voice clips in 27 fixed-size (0xb6) blocks, and two outcome scenes.
void MemoryPuzzle::readDataNancy11(Common::SeekableReadStream &stream) {
	readFilename(stream, _imageName); // 0x000

	for (int i = 0; i < 12; ++i)            // 0x021 face src rects
		readRect(stream, _faceSrcRects[i]);
	for (int i = 0; i < kNumTabs; ++i)      // 0x0e1 page-tab indicator src rects
		readRect(stream, _tabSrcRects[i]);
	for (int i = 0; i < kCardsPerTab; ++i)  // 0x111 card position / back src rects
		readRect(stream, _cardRects[i]);

	readRect(stream, _tabRect);             // 0x291 tab indicator dest

	for (int tab = 0; tab < kNumTabs; ++tab) // 0x2a1 tab hotspots
		for (int slot = 0; slot < 3; ++slot)
			readRect(stream, _tabHotspots[tab][slot]);

	_flipDelay = stream.readUint32LE();     // 0x331
	stream.skip(4);                         // 0x335 (second timing value, unused)
	int32 requirePercent = stream.readSint32LE(); // 0x339 (-1 = use the fixed count below)
	int32 requireCount   = stream.readSint32LE(); // 0x33d
	stream.skip(4);                         // 0x341 (unused)

	_shuffleGlobal = (stream.readByte() != 0); // 0x345
	stream.skip(1);                         // 0x346 (flag, unused)

	int32 pages      = stream.readSint32LE(); // 0x347
	int32 gridsWide  = stream.readSint32LE(); // 0x34b
	int32 gridsTall  = stream.readSint32LE(); // 0x34f
	int32 srcWide    = stream.readSint32LE(); // 0x353
	int32 srcTall    = stream.readSint32LE(); // 0x357
	stream.skip(1);                         // 0x35b (tabs flag)

	_numTabs     = CLIP<int>(pages, 1, kNumTabs);
	_cardsPerTab = CLIP<int>(gridsWide * gridsTall, 1, kCardsPerTab);
	_numTypes    = CLIP<int>(srcWide * srcTall, 1, kMaxTypes);
	_numPairs    = _numTypes;
	_requiredPairs = (requirePercent < 0) ? (uint32)requireCount
	                                       : (uint32)(_numPairs * requirePercent / 100);

	// 27 fixed 0xb6-byte voice-clip blocks: [0] is the card-flip sound, [17] starts the
	// per-card match sounds (used here as a single match sound; per-type audio is a TODO).
	_cardFlipSound.readNormal(stream);             // block 0 @ 0x35c
	stream.skip(17 * 0xb6 - 0x31);                 // advance to block 17 @ 0xf72
	_matchSound.readNormal(stream);                // block 17
	stream.skip((27 - 17) * 0xb6 - 0x31);          // advance to the scenes @ 0x168e
	// Nancy 11 has no win sound; _winSound keeps its default "NO SOUND".

	// Solve scene (0x168e), then an alternate-outcome scene (0x16a8, unused). The event flags
	// store a 16-bit value rather than a simple on/off.
	_winScene._sceneChange.readData(stream);
	_winScene._sceneChange.continueSceneSound = stream.readUint16LE();
	_winScene._flag.label = stream.readSint16LE();
	_winScene._flag.flag = stream.readSint16LE() ? g_nancy->_true : g_nancy->_false;
	stream.skip(g_nancy->getGameType() >= kGameTypeNancy12 ? 24 : 26);	// alternate scene
}

// Shuffles type IDs (0..numPairs-1) into the 72-card array so that every type
// appears exactly twice. numPairs is clamped to [4, 36]; cards beyond numPairs
// remain typeId -1 (unassigned, unselectable). requiredPairs is clamped to [2, totalCards/2].
void MemoryPuzzle::initCards() {
	_numPairs = CLIP<uint32>(_numPairs, 4, (uint32)_numTypes);

	const int totalCards = _numTabs * _cardsPerTab;
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
		for (int tab = 0; tab < _numTabs; ++tab) {
			int base = tab * _cardsPerTab;
			for (int i = 0; i < _cardsPerTab; ++i) {
				if (_cards[base + i].typeId != -1)
					continue;
				if (static_cast<uint32>(nextType) >= _numPairs)
					break; // all types used up for this tab

				_cards[base + i].typeId = nextType;

				// Find a random unassigned slot in the same tab for the pair
				int partner;
				do {
					partner = g_nancy->_randomSource->getRandomNumber(_cardsPerTab - 1);
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
	for (int slot = 0; slot < _numTabs; ++slot) {
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
				if (_cardFlipSound.name != "NO SOUND")
					g_nancy->_sound->playSound(_cardFlipSound);
				redrawCards();
			}
			return;
		}
	}

	// Card clicks are blocked while the flip-back timer is running
	if (_flipTimerActive)
		return;

	int base = _currentTab * _cardsPerTab;
	for (int i = 0; i < _cardsPerTab; ++i) {
		if (!_cardRects[i].contains(mouseVP))
			continue;

		int idx = base + i;
		CardState &card = _cards[idx];

		// Unassigned or already matched or face-up: ignore
		if (card.typeId == -1 || card.matchState != 0 || card.flipState != 0)
			return;

		g_nancy->_cursor->setCursorType(CursorManager::kHotspot);

		if (!(input.input & NancyInput::kLeftMouseButtonUp))
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
	if (_currentTab < _numTabs && !_tabSrcRects[_currentTab].isEmpty())
		_drawSurface.blitFrom(_image, _tabSrcRects[_currentTab],
			Common::Point(_tabRect.left, _tabRect.top));

	// Draw face-up and matched cards. Face-down cards are left transparent so the scene
	// background (which carries the card-back visual) shows through.
	int base = _currentTab * _cardsPerTab;
	for (int i = 0; i < _cardsPerTab; ++i) {
		int idx = base + i;
		const CardState &card = _cards[idx];
		const Common::Rect &dest = _cardRects[i];

		if (card.matchState != 0 || card.flipState != 0) {
			int t = card.typeId;
			if (t >= 0 && t < _numTypes && !_faceSrcRects[t].isEmpty())
				_drawSurface.blitFrom(_image, _faceSrcRects[t],
					Common::Point(dest.left, dest.top));
		}
	}
	_needsRedraw = true;
}

} // End of namespace Action
} // End of namespace Nancy
