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

	// 0x4b1: flipDelay, then two percentages - the share of the faces to deal out, and
	// the share of the board's pairs needed to win
	_flipDelay = stream.readUint32LE();
	int32 pairsPercent   = stream.readSint32LE(); // 0x4b5
	int32 requirePercent = stream.readSint32LE(); // 0x4b9

	_numPairs      = (uint32)(pairsPercent * _numTypes / 100);
	_requiredPairs = (uint32)(requirePercent * (_numTabs * _cardsPerTab / 2) / 100);

	// 0x4bd: cursor flag (ignored in ScummVM)
	stream.skip(1);

	// 0x4be: shuffle flag
	_shuffleGlobal = (stream.readByte() != 0);

	// 0x4bf: match sound; 0x4f0: no-match sound
	_matchSound.readNormal(stream);
	_noMatchSound.readNormal(stream);

	// 0x521: win scene + flag
	_winScene.readData(stream);

	stream.skip(1); // 0x53a: unknown

	// 0x53b: win sound
	_winSound.readNormal(stream);
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
	int32 pairsPercent   = stream.readSint32LE(); // 0x335 (percentage of the faces to deal out)
	int32 requirePercent = stream.readSint32LE(); // 0x339 (-1 = use the fixed count below)
	int32 requireCount   = stream.readSint32LE(); // 0x33d
	stream.skip(4);                         // 0x341 (unused)

	stream.skip(1);                         // 0x345 (cursor flag, ignored in ScummVM)
	_shuffleGlobal = (stream.readByte() != 0); // 0x346

	int32 pages      = stream.readSint32LE(); // 0x347
	int32 gridsWide  = stream.readSint32LE(); // 0x34b
	int32 gridsTall  = stream.readSint32LE(); // 0x34f
	int32 srcWide    = stream.readSint32LE(); // 0x353
	int32 srcTall    = stream.readSint32LE(); // 0x357
	_hasPageTabs = (stream.readByte() != 0); // 0x35b

	_numTabs     = CLIP<int>(pages, 1, kNumTabs);
	_cardsPerTab = CLIP<int>(gridsWide * gridsTall, 1, kCardsPerTab);
	_numTypes    = CLIP<int>(srcWide * srcTall, 1, kMaxTypes);

	// A single page can only ever pair within itself
	if (_numTabs == 1)
		_shuffleGlobal = false;

	// Both counts are percentages: the faces to deal out are a share of the available
	// faces, the pairs needed to win a share of every pair on the board.
	int halfBoard = _numTabs * _cardsPerTab / 2;
	_numPairs = (uint32)(pairsPercent * _numTypes / 100);
	_requiredPairs = (requirePercent == -1) ? (uint32)requireCount
	                                        : (uint32)(requirePercent * halfBoard / 100);

	// 27 fixed 0xb6-byte voice-clip blocks: [0] and [1] are the sounds for flipping the
	// first and second card of a pair, [17] starts the per-card match sounds (used here
	// as a single match sound; per-type audio is a TODO).
	_firstFlipSound.readNormal(stream);            // block 0 @ 0x35c
	stream.skip(0xb6 - 0x31);                      // advance to block 1 @ 0x412
	_secondFlipSound.readNormal(stream);           // block 1
	stream.skip(16 * 0xb6 - 0x31);                 // advance to block 17 @ 0xf72
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

// Picks the type ID for the next pair. Types that haven't been handed out yet are
// preferred; once all of them are used, a random type is reused, so the same face
// can end up on more than one pair.
int MemoryPuzzle::pickCardType() {
	bool allUsed = true;
	for (uint32 i = 0; i < _numPairs; ++i) {
		if (!_typeUsed[i]) {
			allUsed = false;
			break;
		}
	}

	if (allUsed)
		return g_nancy->_randomSource->getRandomNumber(_numPairs - 1);

	int type;
	do {
		type = g_nancy->_randomSource->getRandomNumber(_numPairs - 1);
	} while (_typeUsed[type]);

	_typeUsed[type] = true;
	return type;
}

// Shuffles type IDs into the card array, two cards at a time, until every card slot
// is filled. numPairs is clamped to [4, 36] and requiredPairs to [2, totalCards/2].
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

	for (int i = 0; i < kMaxTypes; ++i)
		_typeUsed[i] = false;

	if (!_shuffleGlobal) {
		// By-tab: pairs are always within the same tab.
		for (int tab = 0; tab < _numTabs; ++tab) {
			int base = tab * _cardsPerTab;
			for (int i = 0; i < _cardsPerTab; ++i) {
				if (_cards[base + i].typeId != -1)
					continue;

				int type = pickCardType();
				_cards[base + i].typeId = type;

				// Find a random unassigned slot in the same tab for the pair
				int partner;
				do {
					partner = g_nancy->_randomSource->getRandomNumber(_cardsPerTab - 1);
				} while (_cards[base + partner].typeId != -1);
				_cards[base + partner].typeId = type;
			}
		}
	} else {
		// Global: pairs may be in different tabs.
		for (int i = 0; i < totalCards; ++i) {
			if (_cards[i].typeId != -1)
				continue;

			int type = pickCardType();
			_cards[i].typeId = type;

			// Find a random unassigned slot anywhere
			int partner;
			do {
				partner = g_nancy->_randomSource->getRandomNumber(totalCards - 1);
			} while (_cards[partner].typeId != -1);
			_cards[partner].typeId = type;
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
		if (_firstFlipSound.name != "NO SOUND")
			g_nancy->_sound->loadSound(_firstFlipSound);
		if (_secondFlipSound.name != "NO SOUND")
			g_nancy->_sound->loadSound(_secondFlipSound);
		if (_matchSound.name != "NO SOUND")
			g_nancy->_sound->loadSound(_matchSound);
		if (_noMatchSound.name != "NO SOUND")
			g_nancy->_sound->loadSound(_noMatchSound);
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
		g_nancy->_sound->stopSound(_firstFlipSound);
		g_nancy->_sound->stopSound(_secondFlipSound);
		g_nancy->_sound->stopSound(_matchSound);
		g_nancy->_sound->stopSound(_noMatchSound);
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

	// Tab switching: _tabHotspots[currentTab][slot] where slot is the target tab
	for (int slot = 0; _hasPageTabs && slot < _numTabs; ++slot) {
		if (_tabHotspots[_currentTab][slot].contains(mouseVP)) {
			g_nancy->_cursor->setCursorType(CursorManager::kHotspot);
			if ((input.input & NancyInput::kLeftMouseButtonUp) && slot != _currentTab) {
				// Changing pages keeps any pending flip state, so a pair can be
				// matched across two different pages
				_currentTab = slot;
				g_nancy->_sound->playSound("BUOK");
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
		redrawCards();

		if (_firstFlip == -1) {
			// First card of a potential pair
			_firstFlip = idx;

			if (g_nancy->getGameType() >= kGameTypeNancy11) {
				if (_firstFlipSound.name != "NO SOUND")
					g_nancy->_sound->playSound(_firstFlipSound);
			} else {
				g_nancy->_sound->playSound("BUOK");
			}
		} else {
			// Second card. Up to Nancy 10 this one is silent, and the match or no-match
			// sound answers it instead.
			if (_secondFlipSound.name != "NO SOUND")
				g_nancy->_sound->playSound(_secondFlipSound);

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
				if (_noMatchSound.name != "NO SOUND")
					g_nancy->_sound->playSound(_noMatchSound);
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
