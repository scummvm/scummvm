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
#include "common/system.h"

#include "engines/nancy/nancy.h"
#include "engines/nancy/cursor.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/input.h"
#include "engines/nancy/resource.h"
#include "engines/nancy/sound.h"
#include "engines/nancy/util.h"
#include "engines/nancy/puzzledata.h"

#include "engines/nancy/action/puzzle/hangmanpuzzle.h"

#include "engines/nancy/state/scene.h"

namespace Nancy {
namespace Action {

void HangmanPuzzle::readData(Common::SeekableReadStream &stream) {
	readFilename(stream, _puzzleImageName);		// 0x3d
	readFilename(stream, _lettersImageName);	// 0x41
	_hoverCursorType = stream.readUint16LE();	// 0x45

	int16 numWords = stream.readSint16LE();
	_words.resize(numWords);
	for (int16 i = 0; i < numWords; ++i) {
		readFilename(stream, _words[i]);
	}

	int16 numHangPieces = stream.readSint16LE();
	readRectArray(stream, _hangPieceRects, numHangPieces);	// 0x57
	readRect(stream, _boardRect);							// 0x67

	int16 numSlots = stream.readSint16LE();
	readRectArray(stream, _letterSlotRects, numSlots);		// 0xae
	int16 numGuessed = stream.readSint16LE();
	readRectArray(stream, _guessedRowRects, numGuessed);	// 0xbe

	int16 numLetters = stream.readSint16LE();
	_letters.resize(numLetters);							// 0x77
	for (int16 i = 0; i < numLetters; ++i) {
		LetterTile &tile = _letters[i];
		tile.letter = (char)stream.readByte();
		readRect(stream, tile.idleRect);
		readRect(stream, tile.hoverRect);
		readRect(stream, tile.glyphRect);
		readFilename(stream, tile.name);
	}

	_letterSoundChannel = stream.readSint16LE();	// 0xce
	_letterSoundLoops = stream.readSint32LE();		// 0xd0
	_letterSoundVolume = stream.readSint16LE();		// 0xd4

	_correctSound.readData(stream);	// 0x12c
	_wrongSound.readData(stream);	// 0x182
	_revealSound.readData(stream);	// 0x1d8

	readFilename(stream, _targetSequence);	// 0x236

	SceneOutcome *outcomes[] = { &_sequenceScene, &_winScene, &_loseScene };
	for (SceneOutcome *outcome : outcomes) {
		outcome->sceneID = stream.readSint16LE();
		outcome->frameID = stream.readSint16LE();
		outcome->flag.label = stream.readSint16LE();
		outcome->flag.flag = stream.readByte();
		outcome->sound.readData(stream);
	}

	// Trailing count-prefixed array of 23-byte give-up hotspots
	// {Rect, uint16 cursorType, uint16 sceneID, int16 flagLabel, byte flagValue}.
	// The exit always jumps to the scene's first frame.
	readExitHotspot(stream);
}

HangmanData *HangmanPuzzle::getPuzzleData() const {
	return (HangmanData *)NancySceneState.getPuzzleData(HangmanData::getTag());
}

void HangmanPuzzle::pickWord() {
	HangmanData *data = getPuzzleData();

	// Words not yet used this cycle; once all are used, start over.
	Common::Array<uint> available;
	for (uint i = 0; i < _words.size(); ++i) {
		bool used = false;
		if (data) {
			for (const Common::String &w : data->usedWords) {
				if (w == _words[i]) {
					used = true;
					break;
				}
			}
		}
		if (!used) {
			available.push_back(i);
		}
	}

	if (available.empty()) {
		if (data) {
			data->usedWords.clear();
		}
		for (uint i = 0; i < _words.size(); ++i) {
			available.push_back(i);
		}
	}

	if (available.empty()) {
		return;
	}

	uint pick = available[g_nancy->_randomSource->getRandomNumber(available.size() - 1)];
	_word = _words[pick];
	if (data) {
		data->usedWords.push_back(_word);
	}
}

void HangmanPuzzle::init() {
	Common::Rect vpBounds = NancySceneState.getViewport().getBounds();
	_drawSurface.create(vpBounds.width(), vpBounds.height(),
		g_nancy->_graphics->getInputPixelFormat());
	_drawSurface.clear(g_nancy->_graphics->getTransColor());
	setTransparent(true);
	setVisible(true);
	moveTo(vpBounds);

	g_nancy->_resource->loadImage(_puzzleImageName, _puzzleImage);
	g_nancy->_resource->loadImage(_lettersImageName, _lettersImage);
	_puzzleImage.setTransparentColor(_drawSurface.getTransparentColor());
	_lettersImage.setTransparentColor(_drawSurface.getTransparentColor());

	pickWord();
	_guessed.clear();
	_revealed.clear();
	_revealed.resize(_word.size(), false);
	_wrongCount = 0;
	_hoverTile = -1;
	_lost = false;
	_sequenceComplete = false;
	_pendingFeedback = nullptr;
	_outcome = nullptr;
	_outcomeSoundStarted = false;

	redraw();
}

int HangmanPuzzle::tileAtCursor(const Common::Point &mousePos) const {
	for (uint i = 0; i < _letters.size(); ++i) {
		if (_letters[i].idleRect.isEmpty()) {
			continue;
		}
		if (NancySceneState.getViewport().convertViewportToScreen(_letters[i].idleRect).contains(mousePos)) {
			return (int)i;
		}
	}
	return -1;
}

Common::Rect HangmanPuzzle::glyphForLetter(char letter) const {
	for (const LetterTile &tile : _letters) {
		if (tile.letter == letter) {
			return tile.glyphRect;
		}
	}
	return Common::Rect();
}

void HangmanPuzzle::redraw() {
	_drawSurface.clear(g_nancy->_graphics->getTransColor());

	// The puzzle images are sprite sheets: the source rect indexes a sprite in
	// the sheet, the destination is a separate on-screen position.

	// A guessed ("used") tile stamps its used-mark sprite (2nd tile rect, from
	// the puzzle sheet) onto the tile's alphabet position (1st tile rect).
	for (const LetterTile &tile : _letters) {
		if (tile.used) {
			_drawSurface.blitFrom(_puzzleImage, tile.hoverRect, Common::Point(tile.idleRect.left, tile.idleRect.top));
		}
	}

	// Guessed-letters row: each played letter's glyph (from the letters sheet)
	// at its slot in the row.
	for (uint i = 0; i < _guessed.size() && i < _guessedRowRects.size(); ++i) {
		_drawSurface.blitFrom(_lettersImage, glyphForLetter(_guessed[i]),
			Common::Point(_guessedRowRects[i].left, _guessedRowRects[i].top));
	}

	// Revealed word letters in their blanks; after losing, the whole word.
	for (uint i = 0; i < _revealed.size() && i < _letterSlotRects.size(); ++i) {
		if (_revealed[i] || _lost) {
			_drawSurface.blitFrom(_lettersImage, glyphForLetter(_word[i]),
				Common::Point(_letterSlotRects[i].left, _letterSlotRects[i].top));
		}
	}

	// The hang figure: the current cumulative stage sprite (from the sheet's
	// stage strip) drawn at the board position.
	if (_wrongCount > 0 && _wrongCount <= (int)_hangPieceRects.size()) {
		_drawSurface.blitFrom(_puzzleImage, _hangPieceRects[_wrongCount - 1],
			Common::Point(_boardRect.left, _boardRect.top));
	}

	_needsRedraw = true;
}

void HangmanPuzzle::commitGuess(uint tileIndex) {
	LetterTile &tile = _letters[tileIndex];
	if (tile.used) {
		return;
	}
	tile.used = true;

	RandomSoundBlock letterSound;
	letterSound.names.push_back(tile.name);
	letterSound.channel = _letterSoundChannel;
	letterSound.numLoops = _letterSoundLoops;
	letterSound.volume = _letterSoundVolume;
	playSoundBlock(letterSound);

	char c = tile.letter;
	_guessed.push_back(c);

	bool correct = false;
	for (uint i = 0; i < _word.size(); ++i) {
		if (_word[i] == c) {
			_revealed[i] = true;
			correct = true;
		}
	}

	if (!correct) {
		++_wrongCount;
	}

	// The correct/wrong reaction follows the letter a second later
	_pendingFeedback = correct ? &_correctSound : &_wrongSound;
	_feedbackTime = g_system->getMillis() + 1000;

	redraw();
}

void HangmanPuzzle::updateFeedback() {
	if (_pendingFeedback && g_system->getMillis() >= _feedbackTime) {
		playSoundBlock(*_pendingFeedback);
		_pendingFeedback = nullptr;
	} else if (!_targetSequence.empty() && _guessed.size() == _targetSequence.size()) {
		_sequenceComplete = true;
	}
}

void HangmanPuzzle::checkOutcome() {
	bool sequenceMatched = _sequenceComplete;
	for (uint i = 0; sequenceMatched && i < _guessed.size(); ++i) {
		if (i >= _targetSequence.size() || _guessed[i] != _targetSequence[i]) {
			sequenceMatched = false;
		}
	}

	bool allRevealed = !_revealed.empty();
	for (uint i = 0; i < _revealed.size(); ++i) {
		if (!_revealed[i]) {
			allRevealed = false;
			break;
		}
	}

	if (_wrongCount >= (int)_hangPieceRects.size() && !_lost) {
		_lost = true;
		playSoundBlock(_revealSound);
		_revealEndTime = g_system->getMillis() + 2000;
		redraw();
	}

	// A completed word beats the target sequence, which beats a completed hang figure
	if (allRevealed) {
		_outcome = &_winScene;
	} else if (sequenceMatched) {
		_outcome = &_sequenceScene;
	} else if (_lost) {
		_outcome = &_loseScene;
	}

	if (_outcome) {
		_state = kActionTrigger;
	}
}

void HangmanPuzzle::handleInput(NancyInput &input) {
	if (_state != kRun) {
		return;
	}

	// Give-up hotspot: leave the puzzle.
	if (hoverExitHotspot(input)) {
		if (input.input & NancyInput::kLeftMouseButtonUp) {
			_exitRequested = true;
		}
		input.eatMouseInput();
		return;
	}

	int tile = tileAtCursor(input.mousePos);
	if (tile >= 0 && !_letters[tile].used) {
		g_nancy->_cursor->setCursorType((CursorManager::CursorType)_hoverCursorType, true);
		if (input.input & NancyInput::kLeftMouseButtonUp) {
			commitGuess((uint)tile);
		}
		input.eatMouseInput();
	}
}

void HangmanPuzzle::execute() {
	switch (_state) {
	case kBegin:
		init();
		registerGraphics();
		_state = kRun;
		break;
	case kRun:
		if (_exitRequested) {
			_exitScene.execute();
			break;
		}
		updateFeedback();
		checkOutcome();
		break;
	case kActionTrigger:
		if (_lost && g_system->getMillis() < _revealEndTime) {
			break;
		}

		// The outcome's line plays out before the scene changes
		if (!_outcomeSoundStarted) {
			playSoundBlock(_outcome->sound);
			_outcomeSoundStarted = true;
			break;
		}
		if (!_outcome->sound.names.empty() && g_nancy->_sound->isSoundPlaying((uint16)_outcome->sound.channel)) {
			break;
		}

		{
			SceneChangeDescription desc;
			desc.sceneID = _outcome->sceneID;
			desc.frameID = _outcome->frameID;
			NancySceneState.changeScene(desc);
			NancySceneState.setEventFlag(_outcome->flag);
		}
		finishExecution();
		break;
	}
}

} // End of namespace Action
} // End of namespace Nancy
