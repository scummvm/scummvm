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
#include "engines/nancy/graphics.h"
#include "engines/nancy/resource.h"
#include "engines/nancy/sound.h"
#include "engines/nancy/input.h"
#include "engines/nancy/util.h"

#include "engines/nancy/state/scene.h"
#include "engines/nancy/action/puzzle/matchpuzzle.h"

#include "graphics/font.h"

#include "common/system.h"
#include "common/random.h"

namespace Nancy {
namespace Action {

void MatchPuzzle::readData(Common::SeekableReadStream &stream) {
	// data+0x00..0x20  main sprite sheet name
	readFilename(stream, _overlayName);
	// data+0x21..0x41  score-panel background name
	readFilename(stream, _flagPointBgName);

	_rows         = stream.readSint16LE();  // data+0x42
	_cols         = stream.readSint16LE();  // data+0x44
	_numFlagTypes = stream.readSint16LE();  // data+0x46

	readRect(stream, _shuffleButtonSrcRect);   // data+0x48..0x57 (source rect in sprite sheet)

	_flagSrcRects.resize(26);

	for (int i = 0; i < 26; ++i)
		readRect(stream, _flagSrcRects[i]); // data+0x58..0x1F7 (source rects in sprite sheet)

	// data+0x1F8..0x237 — 64 bytes unused (all zeros)
	stream.skip(0x40);

	// data+0x238  script execution / flag-info display enable (byte)
	_execScript = stream.readByte() != 0;
	stream.skip(1);                         // data+0x239 padding
	_scriptID   = stream.readSint16LE();    // data+0x23A script ID

	// data+0x23C  score-panel font ID (uint16)
	/*_scorePanelFontID = */stream.readUint16LE();

	// data+0x23E..0x25E  33-byte label string drawn in score panel
	readFilename(stream, _displayLabelString);

	// data+0x25F..0x5B8 — 26 per-flag-type names (33 bytes each)
	readFilenameArray(stream, _flagSoundNames, 26);

	// data+0x5B9..0x63C — 132 bytes (4 × 33-byte strings, e.g. score-panel labels)
	stream.skip(132);

	// data+0x63D  score-panel display enable (byte)
	_showScoreDisplay = stream.readByte() != 0;
	_timeLimitSecs  = stream.readSint16LE();          // data+0x63E
	_scoreTarget    = stream.readSint32LE();          // data+0x640
	_scorePerFlag   = stream.readSint16LE();          // data+0x644

	readRect(stream, _matchedFlagSrcRect);            // data+0x646..0x655 matched/highlight src rect

	_timeBonusFor3  = stream.readSint16LE();          // data+0x656 (seconds)
	_scoreBonusFor4 = stream.readSint16LE();          // data+0x658
	_timeBonusFor4  = stream.readSint16LE();          // data+0x65A (seconds)
	_scoreBonusFor5 = stream.readSint16LE();          // data+0x65C
	_timeBonusFor5  = stream.readSint16LE();          // data+0x65E (seconds)
	_gridOffX       = stream.readSint16LE();          // data+0x660
	_gridOffY       = stream.readSint16LE();          // data+0x662
	_rowSpacing     = stream.readSint16LE();          // data+0x664
	_colSpacing     = stream.readSint16LE();          // data+0x666

	// data+0x668..0x6E7 — 8 × 16-byte viewport-local destination rects
	readRect(stream, _labelStringRect);       // data+0x668 — label string display rect
	readRect(stream, _shuffleButtonDestRect); // data+0x678 — shuffle button on-screen rect (hotspot)
	readRect(stream, _goalValueRect);         // data+0x688 — goal value text rect
	readRect(stream, _scoreValueRect);        // data+0x698 — score value text rect
	readRect(stream, _timerValueRect);        // data+0x6A8 — timer text rect
	readRect(stream, _flagNameRect);          // data+0x6B8 — matched flag name text rect
	readRect(stream, _flagImageRect);         // data+0x6C8 — matched flag image rect
	readRect(stream, _hsDisplayRect);         // data+0x6D8 — high-score display positions

	_scoreDisplayDelay = stream.readSint16LE();       // data+0x6E8

	_slotWinSound.readNormal(stream);           // data+0x6EA..0x71A  match animation sound
	_shuffleSound.readNormal(stream);           // data+0x71B..0x74B
	_cardPlaceSound.readNormal(stream);         // data+0x74C..0x77C

	_solveSceneChange.readData(stream);         // data+0x77D..0x795  win scene
	stream.skip(2);                             // data+0x796..0x797 pre-result delay (unused)

	_matchSuccessSound.readNormal(stream);      // data+0x798..0x7C8  win/time-up sound

	_exitSceneChange.readData(stream);          // data+0x7C9..0x7E1  quit/exit scene

	readRect(stream, _exitHotspot);             // data+0x7E2..0x7F1 exit hotspot
}

void MatchPuzzle::init() {
	Common::Rect vpBounds = NancySceneState.getViewport().getBounds();

	_drawSurface.create(vpBounds.width(), vpBounds.height(),
	                    g_nancy->_graphics->getInputPixelFormat());
	_drawSurface.clear(g_nancy->_graphics->getTransColor());
	setTransparent(true);
	setVisible(true);
	moveTo(vpBounds);

	g_nancy->_resource->loadImage(_overlayName, _image);
	_image.setTransparentColor(_drawSurface.getTransparentColor());

	if (!_flagPointBgName.empty()) {
		g_nancy->_resource->loadImage(_flagPointBgName, _scorePanelImage);
		_scorePanelImage.setTransparentColor(_drawSurface.getTransparentColor());
	}

	// Build grid — compute dest rects; cells will be filled by shuffleGrid()
	_grid.resize(_cols);
	for (int col = 0; col < _cols; ++col) {
		_grid[col].resize(_rows);
		for (int row = 0; row < _rows; ++row)
			computeDestRect(col, row);
	}

	// Initialise display strings
	_goalStr  = Common::String::format("%d", _scoreTarget);
	_scoreStr = Common::String::format("%d", (int32)0);
	if (_timeLimitSecs > 0)
		_timerStr = Common::String::format("%2dm %2ds", _timeLimitSecs / 60, _timeLimitSecs % 60);

	shuffleGrid(true);
	redrawAllCells();
}

void MatchPuzzle::execute() {
	switch (_state) {
	case kBegin:
		init();
		registerGraphics();

		if (_slotWinSound.name != "NO SOUND")
			g_nancy->_sound->loadSound(_slotWinSound);
		if (_shuffleSound.name != "NO SOUND")
			g_nancy->_sound->loadSound(_shuffleSound);
		if (_cardPlaceSound.name != "NO SOUND")
			g_nancy->_sound->loadSound(_cardPlaceSound);

		_score        = 0;
		_wonGame      = false;
		_hasPiece1 = _hasPiece2 = false;
		_hasSelection = false;
		_gameSubState = kPlaying;
		_showFlagName = false;
		_prevTimerSecs = -1;

		if (_timeLimitSecs > 0)
			_timerDeadline = g_system->getMillis() + (uint32)_timeLimitSecs * 1000;

		_state = kRun;
		// fall through

	case kRun:
		switch (_gameSubState) {

		case kPlaying: { // Playing — update timer display, process pending clicks, check win/lose
			uint32 now = g_system->getMillis();

			// Update countdown timer display once per second
			if (_timeLimitSecs > 0) {
				int32 remainMs = (int32)(_timerDeadline - now);
				if (remainMs < 0) remainMs = 0;
				int secs = remainMs / 1000;
				if (secs != _prevTimerSecs) {
					_prevTimerSecs = secs;
					_timerStr = Common::String::format("%2dm %2ds", secs / 60, secs % 60);
					redrawAllCells();
				}
			}

			// Process piece 1 first
			if (_hasPiece1) {
				_hasPiece1 = false;
				checkForMatch(_piece1Col, _piece1Row);
				if (_hasVMatch || _hasHMatch) {
					_hasSelection = false;
					if (_execScript && (uint16)_matchedFlagType < _flagSoundNames.size()) {
						_flagNameStr  = _flagSoundNames[_matchedFlagType];
						_showFlagName = true;
					}
					_scoreStr = Common::String::format("%d", _score);
					if (_slotWinSound.name != "NO SOUND")
						g_nancy->_sound->playSound(_slotWinSound);
					_stateTimer   = now + 800;
					_gameSubState = kMatchAnim;
					redrawAllCells();
				}
				break;
			}

			// Process piece 2 on the next frame after piece 1 is cleared
			if (_hasPiece2) {
				_hasPiece2 = false;
				checkForMatch(_piece2Col, _piece2Row);
				if (_hasVMatch || _hasHMatch) {
					_hasSelection = false;
					if (_execScript && (uint16)_matchedFlagType < _flagSoundNames.size()) {
						_flagNameStr  = _flagSoundNames[_matchedFlagType];
						_showFlagName = true;
					}
					_scoreStr = Common::String::format("%d", _score);
					if (_slotWinSound.name != "NO SOUND")
						g_nancy->_sound->playSound(_slotWinSound);
					_stateTimer   = now + 800;
					_gameSubState = kMatchAnim;
					redrawAllCells();
				}
				break;
			}

			// Neither pending: check win/lose conditions
			bool timerExpired  = (_timeLimitSecs > 0) && ((int32)(_timerDeadline - now) < 500);
			bool reachedTarget = (_score >= _scoreTarget);

			if (timerExpired || reachedTarget) {
				_wonGame      = reachedTarget;
				_timerStr     = _wonGame ? "WIN!!!" : "TIME!";
				_hasSelection = false;
				_gameSubState = kStartEndSeq;
				redrawAllCells();
			}
			break;
		}

		case kStartEndSeq: {
			if (_matchSuccessSound.name != "NO SOUND") {
				g_nancy->_sound->loadSound(_matchSuccessSound);
				g_nancy->_sound->playSound(_matchSuccessSound);
			}
			_stateTimer   = g_system->getMillis() + (uint32)_scoreDisplayDelay * 1000;
			_gameSubState = kWaitDelay;
			break;
		}

		case kMatchAnim: { // Match animation: wait for 800ms timer and sound to finish
			uint32 now = g_system->getMillis();
			bool timerDone = (now >= _stateTimer);
			bool soundDone = !g_nancy->_sound->isSoundPlaying(_slotWinSound);

			if (timerDone && soundDone) {
				// Reshuffle only the cells that were part of the match
				for (int c = 0; c < _cols; ++c)
					for (int r = 0; r < _rows; ++r)
						if (_grid[c][r].matched)
							shuffleGrid(false, c, r);

				_hasVMatch = _hasHMatch = false;
				_showFlagName = false;
				redrawAllCells();
				_gameSubState = kPlaying;
			}
			break;
		}

		case kShuffleDelay: { // Shuffle-button delay
			if (g_system->getMillis() >= _shuffleTimer) {
				shuffleGrid(true);
				redrawAllCells();
				_gameSubState = kPlaying;
			}
			break;
		}

		case kWaitSound: { // Wait for win/time-up sound to finish, then enter score display
			if (!g_nancy->_sound->isSoundPlaying(_matchSuccessSound)) {
				_stateTimer   = g_system->getMillis() + (uint32)_scoreDisplayDelay * 1000;
				_gameSubState = kScoreDisplay;
				redrawAllCells(); // show the final-score / high-score screen
			}
			break;
		}

		case kWaitDelay: { // Wait for delay timer, then go to sound-wait
			if (g_system->getMillis() >= _stateTimer)
				_gameSubState = kWaitSound;
			break;
		}

		case kScoreDisplay: { // Score display — show until timer expires, then insert score and exit/reset
			if (g_system->getMillis() < _stateTimer)
				break;

			// Insert current score into the top-5 high score list (descending)
			int32 toInsert = _score;
			for (int i = 0; i < 5; ++i) {
				if (_highScores[i] < toInsert)
					SWAP(_highScores[i], toInsert);
			}

			if (_wonGame) {
				_state = kActionTrigger;
			} else {
				// Time ran out — fresh round
				_score         = 0;
				_scoreStr      = Common::String::format("%d", (int32)0);
				_hasPiece1     = _hasPiece2 = false;
				_hasSelection  = false;
				_showFlagName  = false;
				_prevTimerSecs = -1;
				if (_timeLimitSecs > 0)
					_timerDeadline = g_system->getMillis() + (uint32)_timeLimitSecs * 1000;
				shuffleGrid(true);
				_gameSubState  = kPlaying;
				redrawAllCells();
			}
			break;
		}

		default:
			break;
		}
		break;

	case kActionTrigger:
		g_nancy->_sound->stopSound(_slotWinSound);
		g_nancy->_sound->stopSound(_shuffleSound);
		g_nancy->_sound->stopSound(_cardPlaceSound);
		g_nancy->_sound->stopSound(_matchSuccessSound);

		if (_wonGame)
			_solveSceneChange.execute();
		else
			_exitSceneChange.execute();

		finishExecution();
		break;
	}
}

void MatchPuzzle::handleInput(NancyInput &input) {
	if (_state != kRun)
		return;
	// Convert mouse to viewport-local coordinates.
	Common::Rect vpPos = NancySceneState.getViewport().getScreenPosition();
	Common::Point localMouse = input.mousePos;
	localMouse -= Common::Point(vpPos.left, vpPos.top);

	if (!_exitHotspot.isEmpty() && _exitHotspot.contains(localMouse)) {
		g_nancy->_cursor->setCursorType(CursorManager::kMoveBackward);
		if (input.input & NancyInput::kLeftMouseButtonUp)
			_state = kActionTrigger;
	}

	if (_gameSubState != kPlaying)
		return;

	// Shuffle button — use the on-screen destination rect for hit-testing
	if (_shuffleButtonDestRect.contains(localMouse)) {
		g_nancy->_cursor->setCursorType(CursorManager::kHotspot);
		if (input.input & NancyInput::kLeftMouseButtonUp) {
			_hasSelection = false;
			if (_shuffleSound.name != "NO SOUND")
				g_nancy->_sound->playSound(_shuffleSound);
			_shuffleTimer = g_system->getMillis() + 500;
			_gameSubState = kShuffleDelay;
		}
		return;
	}

	// Grid cell click
	for (int col = 0; col < _cols; ++col) {
		for (int row = 0; row < _rows; ++row) {
			if (!_grid[col][row].visible)
				continue;
			if (_grid[col][row].destRect.contains(localMouse)) {
				g_nancy->_cursor->setCursorType(CursorManager::kHotspot);
				if (input.input & NancyInput::kLeftMouseButtonUp) {
					if (!_hasSelection) {
						// First click: remember this cell as the selection
						_selCol = col;
						_selRow = row;
						_hasSelection = true;
						if (_cardPlaceSound.name != "NO SOUND")
							g_nancy->_sound->playSound(_cardPlaceSound);
						redrawAllCells();
					} else if (col == _selCol && row == _selRow) {
						// Clicked same cell again: deselect
						_hasSelection = false;
						redrawAllCells();
					} else {
						// Second click: swap the two flags and queue both for match-check
						SWAP(_grid[_selCol][_selRow].flagType,
						     _grid[col][row].flagType);

						_piece1Col = _selCol;
						_piece1Row = _selRow;
						_piece2Col = col;
						_piece2Row = row;
						_hasPiece1 = true;
						_hasPiece2 = true;
						_hasSelection = false;

						if (_cardPlaceSound.name != "NO SOUND")
							g_nancy->_sound->playSound(_cardPlaceSound);
						redrawAllCells();
					}
				}
				return;
			}
		}
	}
}

void MatchPuzzle::shuffleGrid(bool allCells, int targetCol, int targetRow) {
	// Valid flag indices are 0 .. (_numFlagTypes - 2) inclusive
	int numTypes = (_numFlagTypes > 1) ? (_numFlagTypes - 1) : 1;

	for (int row = 0; row < _rows; ++row) {
		for (int col = 0; col < _cols; ++col) {
			if (!allCells && (col != targetCol || row != targetRow))
				continue;

			// Pick a random type that doesn't match its above or left neighbour
			int16 chosen = 0;
			for (int attempt = 0; attempt < 100; ++attempt) {
				chosen = (int16)(g_nancy->_randomSource->getRandomNumber(numTypes - 1));
				bool sameAbove = (row > 0) && (chosen == _grid[col][row - 1].flagType);
				bool sameLeft  = (col > 0) && (chosen == _grid[col - 1][row].flagType);
				if (!sameAbove && !sameLeft)
					break;
			}

			_grid[col][row].flagType = chosen;
			_grid[col][row].visible  = true;
			_grid[col][row].matched  = false;
		}
	}
}

void MatchPuzzle::checkForMatch(int col, int row) {
	_hasVMatch = _hasHMatch = false;

	if (col < 0 || col >= _cols || row < 0 || row >= _rows)
		return;
	if (!_grid[col][row].visible)
		return;

	int16 type = _grid[col][row].flagType;
	_matchedFlagType = type;

	// --- Vertical run (fixed column, walk along rows) ---
	int rStart = row, rEnd = row;
	while (rStart > 0        && _grid[col][rStart - 1].flagType == type) --rStart;
	while (rEnd   < _rows - 1 && _grid[col][rEnd   + 1].flagType == type) ++rEnd;
	_matchRowStart = rStart;
	_matchRowEnd   = rEnd;

	// --- Horizontal run (fixed row, walk along cols) ---
	int cStart = col, cEnd = col;
	while (cStart > 0        && _grid[cStart - 1][row].flagType == type) --cStart;
	while (cEnd   < _cols - 1 && _grid[cEnd   + 1][row].flagType == type) ++cEnd;
	_matchColStart = cStart;
	_matchColEnd   = cEnd;

	int vLen = rEnd - rStart; // 2 → 3-match, 3 → 4-match, 4 → 5-match
	int hLen = cEnd - cStart;

	// --- Score and mark: vertical match (>=3 flags) ---
	if (vLen >= 2) {
		_hasVMatch = true;
		for (int r = rStart; r <= rEnd; ++r) {
			_grid[col][r].matched = true;
			_score += _scorePerFlag;
		}
		if (vLen == 2)
			_timerDeadline += (uint32)_timeBonusFor3 * 1000;
		else if (vLen == 3) {
			_score         += _scoreBonusFor4;
			_timerDeadline += (uint32)_timeBonusFor4 * 1000;
		} else if (vLen >= 4) {
			_score         += _scoreBonusFor5;
			_timerDeadline += (uint32)_timeBonusFor5 * 1000;
		}
	}

	// --- Score and mark: horizontal match (>=3 flags) ---
	if (hLen >= 2) {
		_hasHMatch = true;
		for (int c = cStart; c <= cEnd; ++c) {
			_grid[c][row].matched = true;
			_score += _scorePerFlag;
		}
		if (hLen == 2)
			_timerDeadline += (uint32)_timeBonusFor3 * 1000;
		else if (hLen == 3) {
			_score         += _scoreBonusFor4;
			_timerDeadline += (uint32)_timeBonusFor4 * 1000;
		} else if (hLen >= 4) {
			_score         += _scoreBonusFor5;
			_timerDeadline += (uint32)_timeBonusFor5 * 1000;
		}
	}

	if (_score > _scoreTarget)
		_score = _scoreTarget;
}

void MatchPuzzle::computeDestRect(int col, int row) {
	if (_flagSrcRects.empty())
		return;

	// Cell size taken from the first flag rect (all flags are the same size)
	int cellW = _flagSrcRects[0].width() - 1;
	int cellH = _flagSrcRects[0].height() - 1;

	// Column position: extra spacing per col + cell width
	int left = col * (_colSpacing + cellW) + _gridOffX;
	// Row position: extra spacing per row + cell height
	int top  = row * (_rowSpacing + cellH) + _gridOffY;

	_grid[col][row].destRect = Common::Rect(left, top, left + cellW, top + cellH);
}

// ---- rendering helpers ------------------------------------------------------

void MatchPuzzle::drawCell(int col, int row) {
	const GridCell &cell = _grid[col][row];
	if (!cell.visible)
		return;

	int type = cell.flagType;
	if (type < 0 || type >= (int)_flagSrcRects.size())
		return;

	// Draw matched cells with the highlight source rect ("50" graphic); others with their normal rect
	const Common::Rect &srcRect = cell.matched ? _matchedFlagSrcRect : _flagSrcRects[type];
	_drawSurface.blitFrom(_image, srcRect,
	                      Common::Point(cell.destRect.left, cell.destRect.top));
	_needsRedraw = true;
}

void MatchPuzzle::eraseCell(int col, int row) {
	_drawSurface.fillRect(_grid[col][row].destRect,
	                      _drawSurface.getTransparentColor());
	_needsRedraw = true;
}

void MatchPuzzle::drawScorePanel() {
	// ---- State 6: final score / high-score screen ----
	if (_gameSubState == kScoreDisplay) {
		if (!_scorePanelImage.empty())
			_drawSurface.blitFrom(_scorePanelImage, Common::Point(0, 0));

		if (_showScoreDisplay) {
			const Graphics::Font *font = g_nancy->_graphics->getFont(_scriptID);
			if (!font) font = g_nancy->_graphics->getFont(0);
			if (font) {
				const int fh = font->getFontHeight();
				const int lineSpacing = fh + 12;

				// Final score
				Common::Rect rect = _hsDisplayRect;
				rect.moveTo(196, 37);
				rect = NancySceneState.getViewport().convertViewportToScreen(rect);
				rect = convertToLocal(rect);
				
				int scoreX = rect.left + 60;
				int hsX = rect.right - 1;
				int scoreY = (rect.bottom - 1) - 16 + lineSpacing;
				font->drawString(&_drawSurface, _scoreStr, scoreX, scoreY, 100, 0);

				// High-score list: entries start one lineSpacing below the final score
				int hsY = scoreY + lineSpacing;
				for (int i = 0; i < 5; ++i) {
					Common::String hs = Common::String::format("%d", _highScores[i]);
					font->drawString(&_drawSurface, hs, hsX, hsY, 80, 0);
					hsY += lineSpacing;
				}
			}
		}
		return; // don't draw the normal gameplay overlay
	}

	// ---- Normal gameplay ----

	// Draw the shuffle button sprite
	if (!_shuffleButtonSrcRect.isEmpty() && !_shuffleButtonDestRect.isEmpty())
		_drawSurface.blitFrom(_image, _shuffleButtonSrcRect,
		                      Common::Point(_shuffleButtonDestRect.left, _shuffleButtonDestRect.top));

	if (!_showScoreDisplay)
		return;

	// The score-panel font is determined by _scriptID, instead of _scorePanelFontID.
	const Graphics::Font *font = g_nancy->_graphics->getFont(_scriptID);
	if (!font)
		font = g_nancy->_graphics->getFont(0);

	// Helper: vertically centre font within rect (font may be taller than rect)
	const int fh = font->getFontHeight();
	auto textY = [&](const Common::Rect &r) {
		return r.top + (r.height() - fh) / 2 - 1;
	};

	// Static label string (empty in practice — labels are baked into the background image)
	if (!_displayLabelString.empty() && !_labelStringRect.isEmpty())
		font->drawString(&_drawSurface, _displayLabelString,
		                 _labelStringRect.left, textY(_labelStringRect),
		                 _labelStringRect.width(), 0);

	// Goal value (fixed for the lifetime of the puzzle)
	if (!_goalValueRect.isEmpty())
		font->drawString(&_drawSurface, _goalStr,
		                 _goalValueRect.left, textY(_goalValueRect),
		                 _goalValueRect.width(), 0);

	// Current score value
	if (!_scoreValueRect.isEmpty())
		font->drawString(&_drawSurface, _scoreStr,
		                 _scoreValueRect.left, textY(_scoreValueRect),
		                 _scoreValueRect.width(), 0);

	// Countdown timer
	if (_timeLimitSecs > 0 && !_timerValueRect.isEmpty())
		font->drawString(&_drawSurface, _timerStr,
		                 _timerValueRect.left, textY(_timerValueRect),
		                 _timerValueRect.width(), 0);

	// Matched flag info — shown briefly after a match (gated by _execScript)
	if (_execScript && _showFlagName) {
		if (!_flagNameStr.empty() && !_flagNameRect.isEmpty())
			font->drawString(&_drawSurface, _flagNameStr,
			                 _flagNameRect.left, textY(_flagNameRect),
			                 _flagNameRect.width(), 0);

		int16 ft = _matchedFlagType;
		if (ft >= 0 && ft < (int16)_flagSrcRects.size() && !_flagImageRect.isEmpty())
			_drawSurface.blitFrom(_image, _flagSrcRects[ft],
			                      Common::Point(_flagImageRect.left, _flagImageRect.top));
	}
}

void MatchPuzzle::redrawAllCells() {
	_drawSurface.clear(_drawSurface.getTransparentColor());
	drawScorePanel();
	// During state 6 the score-screen covers everything; skip cell drawing
	if (_gameSubState != kScoreDisplay) {
		for (int col = 0; col < _cols; ++col)
			for (int row = 0; row < _rows; ++row)
				drawCell(col, row);
	}
	_needsRedraw = true;
}

} // End of namespace Action
} // End of namespace Nancy
