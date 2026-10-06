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
#include "engines/nancy/cursor.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/sound.h"
#include "engines/nancy/input.h"
#include "engines/nancy/util.h"

#include "engines/nancy/state/scene.h"
#include "engines/nancy/action/actionmanager.h"
#include "engines/nancy/action/puzzle/quizpuzzle.h"
#include "engines/nancy/puzzledata.h"

namespace Nancy {
namespace Action {

QuizPuzzle::~QuizPuzzle() {
	g_nancy->_input->setVKEnabled(false);
	g_nancy->_sound->stopSound(_correctSound);
	g_nancy->_sound->stopSound(_wrongSound);
	g_nancy->_sound->stopSound(_solveSound);
	g_nancy->_sound->stopSound(_activeBoxSound);
}

void QuizPuzzle::init() {
	Common::Rect screenClip = NancySceneState.getViewport().getBounds();
	const uint16 sceneId = NancySceneState.getSceneInfo().sceneID;
	if (g_nancy->getGameType() == kGameTypeNancy9 && (sceneId == 6441 || sceneId == 6443))
		screenClip.right += 20; // WORKAROUND for chess puzzle in Nancy 9: the rightmost answer box is partially off-screen
	_screenPosition = screenClip;
	_drawSurface.create(screenClip.width(), screenClip.height(), g_nancy->_graphics->getInputPixelFormat());
	_drawSurface.clear(g_nancy->_graphics->getTransColor());
	setTransparent(true);

	g_nancy->_input->setVKEnabled(!_isDisplayOnly);
	PuzzleRecord::init();
}

// ---- Nancy 8 data format ----
// Offset  Size  Field
// 0x000   2     fontID
// 0x002   2     blinkInterval
// 0x004   2     numBoxes
// 0x006   80    5 box rects (4×sint32 each)
// 0x056   310   5×(3×20-byte answers + int16 flag)
// 0x18C   49    correctSound (readNormal)
// 0x1BD   30    correct subtitle (skip)
// 0x1DB   49    wrongSound (readNormal)
// 0x20C   30    wrong subtitle (skip)
// 0x22A   25    solveScene
// 0x243   49    doneSound (readNormal)
// 0x274   30    done subtitle (skip)
// 0x292   25    cancelScene
// 0x2AB   16    unused; the record is a fixed 699 bytes
void QuizPuzzle::readDataNancy8(Common::SeekableReadStream &stream) {
	_fontID            = stream.readUint16LE();
	_cursorBlinkInterval = stream.readUint16LE();
	uint16 numBoxes    = stream.readUint16LE();

	_boxes.resize(5);

	for (uint i = 0; i < _boxes.size(); ++i) {
		readRect(stream, _boxes[i].rect);
	}

	char answerBuf[20];
	for (uint i = 0; i < _boxes.size(); ++i) {
		Box &box = _boxes[i];
		for (int j = 0; j < 3; ++j) {
			stream.read(answerBuf, 20);
			answerBuf[19] = '\0';
			if (answerBuf[0]) {
				box.answers.push_back(answerBuf);
			}
		}
		box.correctFlag = stream.readSint16LE();
	}

	_correctSound.readNormal(stream);
	_correctText = readSubtitleText(stream);

	_wrongSound.readNormal(stream);
	_wrongText = readSubtitleText(stream);

	_solveScene.readData(stream);
	_solveSound.readNormal(stream);
	_doneText = readSubtitleText(stream);

	_exitScene.readData(stream);

	// The record is a fixed 699 bytes and nothing reads this tail. Leaving it
	// unread would make the action record loader mistake it for a dependency.
	stream.skip(16);

	if (numBoxes < _boxes.size()) {
		_boxes.resize(numBoxes);
	}
}

// ---- Nancy 9 - Nancy 14 data format ----
// Header (0xB2 bytes):
// 0x00  2   fontID
// 0x02  2   blinkInterval
// 0x04  1   cursor char
// 0x05  20  allowed chars (null-terminated)
// 0x19  25  solveScene
// 0x32  49  doneSound (readNormal)
// 0x63  30  done subtitle (skip)
// 0x81  25  cancelScene
// 0x9A  16  exitHotspot rect (4×sint32)
// 0xAA  2   correctSoundChannel
// 0xAC  2   wrongSoundChannel
// 0xAE  1   skipEmptyOnEnter flag
// 0xAF  1   autoCheck flag (0 = ON)
// 0xB0  2   numBoxes
// Per-box (0xD0 bytes):
// +0x00  16  box rect (4×sint32)
// +0x10  60  3×20-byte answers
// +0x4C  2   event flag (int16)
// +0x4E  33  correct sound name
// +0x6F  2   correct sound volume
// +0x71  30  correct subtitle (skip)
// +0x8F  33  wrong sound name
// +0xB0  2   wrong sound volume
// +0xB2  30  wrong subtitle (skip)
void QuizPuzzle::readDataNancy9(Common::SeekableReadStream &stream) {
	_fontID            = stream.readUint16LE();
	_cursorBlinkInterval = stream.readUint16LE();
	_cursorChar        = stream.readByte();

	char allowedBuf[20];
	stream.read(allowedBuf, 20);
	allowedBuf[19] = '\0';
	_allowedChars = allowedBuf;

	_solveScene.readData(stream);
	_solveSound.readNormal(stream);
	_doneText = readSubtitleText(stream);

	_exitScene.readData(stream);
	readRect(stream, _exitHotspot);

	uint16 correctSoundChannel = stream.readUint16LE();
	uint16 wrongSoundChannel   = stream.readUint16LE();
	_skipEmptyOnEnter    = (stream.readByte() != 0);
	_autoCheck           = (stream.readByte() == 0); // 0 = auto-check ON
	uint16 numBoxes      = stream.readUint16LE();

	if (numBoxes > kMaxBoxes) {
		warning("QuizPuzzle: numBoxes %d exceeds maximum %d, clamping", numBoxes, (int)kMaxBoxes);
		numBoxes = kMaxBoxes;
	}

	_boxes.resize(numBoxes);

	char answerBuf[20];
	for (uint i = 0; i < _boxes.size(); ++i) {
		Box &box = _boxes[i];
		readRect(stream, box.rect);

		for (int j = 0; j < 3; ++j) {
			stream.read(answerBuf, 20);
			answerBuf[19] = '\0';
			if (answerBuf[0]) {
				box.answers.push_back(answerBuf);
				box.maxAnswerLength = MAX<uint16>(box.maxAnswerLength, box.answers.back().size());
			}
		}

		box.correctFlag = stream.readSint16LE();

		// The per-box sounds name a single file and take their channel from the
		// record-wide channels read above, so they fold into a RandomSoundBlock.
		box.correctSound.names.resize(1);
		readFilename(stream, box.correctSound.names[0]);
		box.correctSound.channel = correctSoundChannel;
		box.correctSound.numLoops = 1;
		box.correctSound.volume = stream.readUint16LE();
		box.correctText = readSubtitleText(stream);

		box.wrongSound.names.resize(1);
		readFilename(stream, box.wrongSound.names[0]);
		box.wrongSound.channel = wrongSoundChannel;
		box.wrongSound.numLoops = 1;
		box.wrongSound.volume = stream.readUint16LE();
		box.wrongText = readSubtitleText(stream);
	}
}

// ---- Nancy 15 data format ----
// Header (99 bytes, copied into the record in one go):
// 0x00  2   fontID
// 0x02  2   blinkInterval
// 0x04  1   cursor char (0 = display-only record)
// 0x05  1   unused
// 0x06  2   restore flag (int16)
// 0x08  1   0 = move on to the next box after a wrong answer
// 0x09  1   0 = check the answer after every keystroke
// 0x0A  1   non-zero = box cycling also stops on solved boxes
//           (with no cursor char, 0x08/0x09 are a source scene id instead)
// 0x0B  33  AUTOTEXT key for the "right answer" caption
// 0x2C  33  AUTOTEXT key for the "wrong answer" caption
// 0x4D  20  allowed chars (null-terminated)
// 0x61  2   unused
// Then: invalid-key sound block, solve scene (terse) + solve sound block,
// the give-up hotspot, the flag-setting hotspot array, and the boxes.
void QuizPuzzle::readDataNancy15(Common::SeekableReadStream &stream) {
	_fontID              = stream.readUint16LE();
	_cursorBlinkInterval = stream.readUint16LE();
	_cursorChar          = stream.readByte();
	stream.skip(1);
	_restoreFlag         = stream.readSint16LE();

	byte flagA = stream.readByte();
	byte flagB = stream.readByte();
	_cycleSolvedBoxes = (stream.readByte() != 0);

	_isDisplayOnly = (_cursorChar == 0);
	if (_isDisplayOnly) {
		_displaySourceScene = flagA | (flagB << 8);
	} else {
		_keepBoxOnWrongAnswer = (flagA != 0);
		_autoCheck = (flagB == 0); // 0 = auto-check ON
	}

	Common::String captionKey;
	readFilename(stream, captionKey);
	_recordCorrectText = resolveSubtitleText(captionKey, Common::String());
	readFilename(stream, captionKey);
	_recordWrongText = resolveSubtitleText(captionKey, Common::String());

	char allowedBuf[20];
	stream.read(allowedBuf, 20);
	allowedBuf[19] = '\0';
	_allowedChars = allowedBuf;
	stream.skip(2);

	_invalidKeySound.readData(stream);

	_solveScene._sceneChange.sceneID = stream.readUint16LE();
	_solveScene._sceneChange.frameID = stream.readUint16LE();
	_solveScene._flag.label = stream.readSint16LE();
	_solveScene._flag.flag = stream.readByte();
	_doneSoundBlock.readData(stream);

	// The give-up hotspot is stored inline here, not as one of the count-prefixed
	// records the other puzzles use, and its flag is always set to true.
	readRect(stream, _exitHotspot);
	_exitCursorType = stream.readUint16LE();
	_exitCursorFromData = true;
	_exitScene._sceneChange.sceneID = stream.readUint16LE();
	_exitScene._flag.label = stream.readSint16LE();
	_exitScene._flag.flag = g_nancy->_true;

	readExitHotspots(stream, _hotspots);

	// The box count is stored doubled
	uint16 numBoxes = stream.readUint16LE() / 2;

	if (numBoxes > kMaxBoxes) {
		warning("QuizPuzzle: numBoxes %d exceeds maximum %d, clamping", numBoxes, (int)kMaxBoxes);
		numBoxes = kMaxBoxes;
	}

	_boxes.resize(numBoxes);

	for (uint i = 0; i < _boxes.size(); ++i) {
		Box &box = _boxes[i];
		readRect(stream, box.rect);

		uint16 numAnswers = stream.readUint16LE();
		box.answers.resize(numAnswers);
		for (uint j = 0; j < numAnswers; ++j) {
			readFilename(stream, box.answers[j]);
			box.maxAnswerLength = MAX<uint16>(box.maxAnswerLength, box.answers[j].size());
		}

		box.correctFlag = stream.readSint16LE();
		box.nonEmptyFlag = stream.readSint16LE();

		box.correctSound.readData(stream);
		box.wrongSound.readData(stream);
	}
}

void QuizPuzzle::readData(Common::SeekableReadStream &stream) {
	if (g_nancy->getGameType() == kGameTypeNancy8) {
		readDataNancy8(stream);
	} else if (g_nancy->getGameType() >= kGameTypeNancy15) {
		readDataNancy15(stream);
	} else {
		readDataNancy9(stream);
	}
}

char QuizPuzzle::getCursorChar() const {
	return g_nancy->getGameType() == kGameTypeNancy8 ? '-' : _cursorChar;
}

uint QuizPuzzle::getMaxTypedLength() const {
	return g_nancy->getGameType() >= kGameTypeNancy15 ? kMaxTypedLengthNancy15 : kMaxTypedLength;
}

bool QuizPuzzle::acceptsChar(char c) const {
	if (_allowedChars.empty()) {
		return true;
	}

	for (uint i = 0; i < _allowedChars.size(); ++i) {
		if (_allowedChars[i] == c) {
			return true;
		}
	}

	return false;
}

// From Nancy 9 to Nancy 14 the original measures the text already in the box
// and refuses the keystroke unless it still leaves a margin at the right edge.
bool QuizPuzzle::fitsInBox(const Box &box, const Common::String &text) const {
	const Graphics::Font *font = g_nancy->_graphics->getFont(_fontID);
	if (!font) {
		return true;
	}

	return font->getStringWidth(text) < box.rect.width() - kBoxTextMargin;
}

bool QuizPuzzle::playBoxSoundBlock(const RandomSoundBlock &block) {
	if (block.names.empty()) {
		return false;
	}

	uint idx = block.names.size() == 1 ? 0 : g_nancy->_randomSource->getRandomNumber(block.names.size() - 1);
	const Common::String &name = block.names[idx];
	if (name.empty() || name == "NO SOUND") {
		return false;
	}

	_activeBoxSound.name = name;
	_activeBoxSound.channelID = block.channel;
	_activeBoxSound.numLoops = block.numLoops > 0 ? block.numLoops : 1;
	_activeBoxSound.volume = block.volume;
	_activeBoxSound.playCommands = 1;

	g_nancy->_sound->loadSound(_activeBoxSound);
	g_nancy->_sound->playSound(_activeBoxSound);
	return true;
}

void QuizPuzzle::showBoxSubtitle(const Common::String &perBoxText, const Common::String &perRecordText) {
	// Nancy 15 moved the captions out of the boxes and into the record
	showSubtitle(g_nancy->getGameType() >= kGameTypeNancy15 ? perRecordText : perBoxText);
}

// ---- Nancy 8 state machine ----
void QuizPuzzle::executeNancy8() {
	Box &box = _boxes[_currentBox];

	switch (_internalState) {
	case kTyping: {
		if (_hasNewKey) {
			_hasNewKey = false;

			Common::String &text = box.typedText;
			bool hadCursor = !text.empty() && text.lastChar() == '-';
			if (hadCursor)
				text.deleteLastChar();

			if (_pendingBackspace) {
				if (!text.empty())
					text.deleteLastChar();
				drawText();
				_pendingBackspace = false;
			} else if (_pendingReturn) {
				if (text.empty()) {
					if (hadCursor) text += '-';
					advanceToNextBox();
				} else {
					if (hadCursor) text += '-';
					_internalState = kCheckAnswer;
				}
				_pendingReturn = false;
			} else if (_pendingChar != 0) {
				if (text.size() < kMaxTypedLength)
					text += _pendingChar;
				drawText();
				_pendingChar = 0;
			}
		}

		// Cursor blink: toggle trailing '-'
		Time now = g_nancy->getTotalPlayTime();
		if (now >= _nextBlinkTime) {
			_nextBlinkTime = now + _cursorBlinkInterval;
			Common::String &text = _boxes[_currentBox].typedText;
			if (!text.empty() && text.lastChar() == '-')
				text.deleteLastChar();
			else
				text += '-';
			drawText();
		}
		break;
	}

	case kCheckAnswer: {
		bool correct = checkAnswerForCurrentBox();
		if (correct) {
			advanceToNextBox();
			_internalState = kStartCorrect;
		} else {
			_internalState = kStartWrong;
		}
		break;
	}

	case kStartCorrect: {
		if (_correctSound.name == "NO SOUND") {
			_solved = checkAllSolved();
			_internalState = _solved ? kStartDone : kTyping;
		} else {
			g_nancy->_sound->loadSound(_correctSound);
			g_nancy->_sound->playSound(_correctSound);
			showSubtitle(_correctText);
			_internalState = kWaitCorrect;
		}
		_nextBlinkTime = 0;
		drawText();
		break;
	}

	case kWaitCorrect:
		if (!g_nancy->_sound->isSoundPlaying(_correctSound)) {
			g_nancy->_sound->stopSound(_correctSound);
			_solved = checkAllSolved();
			_internalState = _solved ? kStartDone : kTyping;
			_nextBlinkTime = 0;
			drawText();
		}
		break;

	case kStartWrong: {
		box.typedText.clear();
		drawText();

		if (_wrongSound.name == "NO SOUND") {
			_internalState = kTyping;
		} else {
			g_nancy->_sound->loadSound(_wrongSound);
			g_nancy->_sound->playSound(_wrongSound);
			showSubtitle(_wrongText);
			_internalState = kWaitWrong;
		}
		break;
	}

	case kWaitWrong:
		if (!g_nancy->_sound->isSoundPlaying(_wrongSound)) {
			g_nancy->_sound->stopSound(_wrongSound);
			_internalState = kTyping;
			_nextBlinkTime = 0;
		}
		break;

	case kStartDone: {
		if (!hasSolveSound()) {
			_internalState = kFinish;
		} else {
			playSolveSound();
			showSubtitle(_doneText);
			_internalState = kWaitDone;
		}
		break;
	}

	case kWaitDone:
		if (!isSolveSoundPlaying()) {
			g_nancy->_sound->stopSound(_solveSound);
			_internalState = kFinish;
		}
		break;

	case kFinish:
		_state = kActionTrigger;
		break;
	}
}

// ---- Nancy 9+ state machine ----
void QuizPuzzle::executeNancy9() {
	const bool isNancy15 = g_nancy->getGameType() >= kGameTypeNancy15;
	Box &box = _boxes[_currentBox];

	// A Nancy 15 record without a give-up hotspot completes as soon as every box
	// is right, and keeps its solve flag in step with that. With a give-up
	// hotspot, that hotspot doubles as the submit button instead.
	if (isNancy15 && _internalState == kTyping && _exitHotspot.isEmpty()) {
		bool allSolved = checkAllSolved();
		if (allSolved != _solved) {
			_solved = allSolved;
			if (_solveScene._flag.label != -1) {
				NancySceneState.setEventFlag(_solveScene._flag.label,
					allSolved ? _solveScene._flag.flag : g_nancy->_false);
			}
		}

		// A record whose solve scene is the one it already runs in never
		// finishes: it only sets its flag, so its hotspots stay clickable.
		if (_solved && _solveScene._sceneChange.sceneID != NancySceneState.getSceneInfo().sceneID) {
			_internalState = kStartDone;
		}
	}

	switch (_internalState) {
	case kTyping: {
		if (_hasNewKey) {
			_hasNewKey = false;

			Common::String &text = box.typedText;
			bool hadCursor = !text.empty() && text.lastChar() == _cursorChar;
			if (hadCursor)
				text.deleteLastChar();

			if (_pendingBackspace) {
				if (!text.empty())
					text.deleteLastChar();

				// Nancy 15 re-checks the shortened answer, and clears the flag
				// of a box that is no longer right
				if (isNancy15) {
					saveAnswer(_currentBox);
					if (_autoCheck && checkAnswerForCurrentBox()) {
						_internalState = kStartCorrect;
					}
				}

				drawText();
				_pendingBackspace = false;
			} else if (_pendingReturn) {
				if (hadCursor) text += _cursorChar;

				if (isNancy15 || !text.empty()) {
					// Nancy 15 checks the box whatever it holds, empty included
					_internalState = kCheckAnswer;
				} else if (!_skipEmptyOnEnter) {
					advanceToNextBox();
				}
				_pendingReturn = false;
			} else if (_pendingChar != 0) {
				if (!acceptsChar(_pendingChar)) {
					// Nancy 15 buzzes at characters the record does not allow
					playBoxSoundBlock(_invalidKeySound);
				} else if (text.size() < getMaxTypedLength() &&
						(isNancy15 || fitsInBox(box, text))) {
					text += _pendingChar;

					if (isNancy15) {
						saveAnswer(_currentBox);
					}

					if (_autoCheck) {
						if (checkAnswerForCurrentBox()) {
							_internalState = kStartCorrect;
						} else if (!isNancy15 && text.size() >= box.maxAnswerLength) {
							_internalState = kStartWrong;
						}
					}
				}
				drawText();
				_pendingChar = 0;
			}

			// Nancy 15 re-scores the box the cursor is still in after every key,
			// so a box that is filled in and then left with the mouse - by
			// clicking the submit hotspot, say - has already been graded
			if (isNancy15 && _internalState == kTyping) {
				checkAnswerForCurrentBox(true);
			}
		}

		// Cursor blink (only while still typing)
		if (_internalState == kTyping && !checkAllSolved()) {
			Time now = g_nancy->getTotalPlayTime();
			if (now >= _nextBlinkTime) {
				_nextBlinkTime = now + _cursorBlinkInterval;
				Common::String &text = _boxes[_currentBox].typedText;
				if (!text.empty() && text.lastChar() == _cursorChar)
					text.deleteLastChar();
				else
					text += _cursorChar;
				drawText();
			}
		}
		break;
	}

	case kCheckAnswer: {
		bool correct = checkAnswerForCurrentBox();
		_internalState = correct ? kStartCorrect : kStartWrong;
		break;
	}

	case kStartCorrect: {
		bool playing = playBoxSoundBlock(box.correctSound);
		if (playing) {
			showBoxSubtitle(box.correctText, _recordCorrectText);
		}

		advanceToNextBox();

		// Nancy 15 does not wait for the feedback sound before carrying on
		if (isNancy15) {
			_internalState = kTyping;
		} else if (!playing) {
			_solved = checkAllSolved();
			_internalState = _solved ? kStartDone : kTyping;
		} else {
			_internalState = kWaitCorrect;
		}

		_nextBlinkTime = 0;
		drawText();
		break;
	}

	case kWaitCorrect:
		if (!g_nancy->_sound->isSoundPlaying(_activeBoxSound)) {
			g_nancy->_sound->stopSound(_activeBoxSound);
			_solved = checkAllSolved();
			_internalState = _solved ? kStartDone : kTyping;
			_nextBlinkTime = 0;
			drawText();
		}
		break;

	case kStartWrong: {
		// Nancy 15 leaves the wrong answer in place for the player to correct
		if (!isNancy15) {
			box.typedText.clear();
		}
		drawText();

		bool playing = playBoxSoundBlock(box.wrongSound);
		if (playing) {
			showBoxSubtitle(box.wrongText, _recordWrongText);
		}

		if (isNancy15) {
			if (!_keepBoxOnWrongAnswer) {
				advanceToNextBox();
				drawText();
			}
			_internalState = kTyping;
			_nextBlinkTime = 0;
		} else {
			_internalState = playing ? kWaitWrong : kTyping;
		}
		break;
	}

	case kWaitWrong:
		if (!g_nancy->_sound->isSoundPlaying(_activeBoxSound)) {
			g_nancy->_sound->stopSound(_activeBoxSound);
			_internalState = kTyping;
			_nextBlinkTime = 0;
		}
		break;

	case kStartDone: {
		if (isNancy15) {
			_internalState = playBoxSoundBlock(_doneSoundBlock) ? kWaitDone : kFinish;
		} else if (!hasSolveSound()) {
			_internalState = kFinish;
		} else {
			playSolveSound();
			showSubtitle(_doneText);
			_internalState = kWaitDone;
		}
		break;
	}

	case kWaitDone: {
		SoundDescription &sound = isNancy15 ? _activeBoxSound : _solveSound;
		if (!g_nancy->_sound->isSoundPlaying(sound)) {
			g_nancy->_sound->stopSound(sound);
			_internalState = kFinish;
		}
		break;
	}

	case kFinish:
		_state = kActionTrigger;
		break;
	}
}

void QuizPuzzle::execute() {
	switch (_state) {
	case kBegin: {
		init();
		registerGraphics();
		NancySceneState.setNoHeldItem();
		_nextBlinkTime = g_nancy->getTotalPlayTime() + _cursorBlinkInterval;
		if (g_nancy->getGameType() == kGameTypeNancy8) {
			g_nancy->_sound->loadSound(_correctSound);
			g_nancy->_sound->loadSound(_wrongSound);
		}

		if (g_nancy->getGameType() < kGameTypeNancy15) {
			g_nancy->_sound->loadSound(_solveSound);
		}

		loadSavedAnswers();

		// Start the cursor on the first unsolved box
		for (uint i = 0; i < _boxes.size(); ++i) {
			if (!_boxes[i].correct) {
				_currentBox = i;
				break;
			}
		}
		drawText();

		_state = kRun;
	}
		// fall through
	case kRun:
		if (_boxes.empty() || _isDisplayOnly) {
			// Nothing to type into; the record only draws
			break;
		}

		if (g_nancy->getGameType() == kGameTypeNancy8) {
			executeNancy8();
		} else {
			executeNancy9();
		}
		break;

	case kActionTrigger:
		if (_cancelled) {
			_exitScene.execute();
		} else if (_solved) {
			_solveScene.execute();
		}

		g_nancy->_input->setVKEnabled(false);
		finishExecution();
		break;
	}
}

void QuizPuzzle::handleInput(NancyInput &input) {
	if (_internalState != kTyping || _isDisplayOnly || _boxes.empty()) {
		return;
	}

	const char cursorChar = getCursorChar();
	const bool isNancy15 = g_nancy->getGameType() >= kGameTypeNancy15;
	bool mouseOverHotspot = false;

	// Nancy 9+: give-up hotspot. Clicking it cancels the puzzle.
	if (g_nancy->getGameType() != kGameTypeNancy8) {
		if (hoverExitHotspot(input)) {
			if (input.input & NancyInput::kLeftMouseButtonUp) {
				// From Nancy 15 the give-up hotspot submits the quiz: with every
				// box answered it leads to the solve scene instead
				if (isNancy15 && checkAllSolved()) {
					_solved = true;
					_internalState = kStartDone;
				} else {
					_cancelled = true;
					_state = kActionTrigger;
				}
				return;
			}

			mouseOverHotspot = true;
		}
	}

	// Nancy 15: regions around the boxes that only set an event flag when clicked
	if (!mouseOverHotspot) {
		for (uint i = 0; i < _hotspots.size(); ++i) {
			const ExitHotspot &hotspot = _hotspots[i];
			if (hotspot.hotspot.isEmpty() ||
					!NancySceneState.getViewport().convertViewportToScreen(hotspot.hotspot).contains(input.mousePos)) {
				continue;
			}

			if (hotspot.cursorType != 0) {
				g_nancy->_cursor->setCursorType((CursorManager::CursorType)hotspot.cursorType, true);
			} else {
				g_nancy->_cursor->setCursorType(CursorManager::kHotspot);
			}

			if (input.input & NancyInput::kLeftMouseButtonUp) {
				NancySceneState.setEventFlag(hotspot.flag);
				if (hotspot.scene.sceneID != kNoScene) {
					NancySceneState.changeScene(hotspot.scene);
					return;
				}
			}

			mouseOverHotspot = true;
			break;
		}
	}

	// Hover over a text box: show the hotspot cursor and, on click, move the
	// typing focus to that box. From Nancy 15 an answered box can be edited again.
	for (uint i = 0; i < _boxes.size() && !mouseOverHotspot; ++i) {
		if (_boxes[i].correct && !isNancy15)
			continue;
		Common::Rect screenRect = NancySceneState.getViewport().convertViewportToScreen(_boxes[i].rect);
		if (!screenRect.contains(input.mousePos))
			continue;

		g_nancy->_cursor->setCursorType(CursorManager::kHotspot);

		if (input.input & NancyInput::kLeftMouseButtonUp) {
			if (i != _currentBox) {
				Common::String &oldText = _boxes[_currentBox].typedText;
				if (!oldText.empty() && oldText.lastChar() == cursorChar)
					oldText.deleteLastChar();

				if (isNancy15) {
					// Nancy 15 scores the box being left, so a form filled in by
					// clicking from box to box is graded like a tabbed one
					checkAnswerForCurrentBox(true);
				}

				_currentBox = i;
				_nextBlinkTime = 0;
				drawText();
			}
		}
		break;
	}

	// The keyboard is read whatever the mouse happens to be over
	for (auto &key : input.otherKbdInput) {
		if (key.keycode == Common::KEYCODE_BACKSPACE) {
			_pendingBackspace = true;
			_hasNewKey = true;
		} else if (key.keycode == Common::KEYCODE_TAB) {
			// Nancy 15 submits the box on Tab, exactly as it does on Enter. The
			// earlier games have no Tab handling at all, so it does nothing there.
			if (isNancy15) {
				_pendingReturn = true;
				_hasNewKey = true;
			}
		} else if (key.keycode == Common::KEYCODE_RETURN ||
		           key.keycode == Common::KEYCODE_KP_ENTER) {
			_pendingReturn = true;
			_hasNewKey = true;
		} else if (key.ascii != 0 && key.ascii != (byte)cursorChar) {
			bool accept = false;
			if (g_nancy->getGameType() == kGameTypeNancy8) {
				// Nancy 8 takes letters, digits and the space bar, nothing else
				accept = Common::isAlnum(key.ascii) || key.ascii == ' ';
			} else {
				accept = key.ascii >= 0x20 && key.ascii < 0x7f;
			}
			if (accept) {
				_pendingChar = key.ascii;
				_hasNewKey = true;
			}
		}
	}
}

void QuizPuzzle::onPause(bool paused) {
	g_nancy->_input->setVKEnabled(!paused && !_isDisplayOnly);
	PuzzleRecord::onPause(paused);
}

void QuizPuzzle::drawText() {
	_drawSurface.clear(g_nancy->_graphics->getTransColor());

	const Graphics::Font *font = g_nancy->_graphics->getFont(_fontID);
	if (!font) {
		warning("QuizPuzzle: font %u is missing, no answer text will be drawn", _fontID);
		return;
	}

	// Nancy 15 draws a box's text into its rect widened by the cursor character
	const int extraWidth = g_nancy->getGameType() >= kGameTypeNancy15 ? 8 : 0;

	for (uint i = 0; i < _boxes.size(); ++i) {
		const Common::String &text = _boxes[i].typedText;
		if (text.empty())
			continue;

		Common::Rect bounds = _boxes[i].rect;
		bounds = NancySceneState.getViewport().convertViewportToScreen(bounds);
		bounds = convertToLocal(bounds);

		int y = bounds.bottom - font->getFontHeight();
		font->drawString(&_drawSurface, text, bounds.left - 1, y, bounds.width() + extraWidth, 0);
	}

	_needsRedraw = true;
}

void QuizPuzzle::advanceToNextBox() {
	for (uint n = 0; n < _boxes.size(); ++n) {
		_currentBox = (_currentBox + 1) % _boxes.size();
		if (_cycleSolvedBoxes || !_boxes[_currentBox].correct)
			return;
	}
}

bool QuizPuzzle::checkAllSolved() const {
	for (uint i = 0; i < _boxes.size(); ++i) {
		if (!_boxes[i].correct)
			return false;
	}

	// A record with no boxes counts as solved, as it does in the original
	return true;
}

bool QuizPuzzle::checkAnswerForCurrentBox(bool silent) {
	Box &box = _boxes[_currentBox];
	const char cursorChar = getCursorChar();

	Common::String input = box.typedText;
	if (!input.empty() && input.lastChar() == cursorChar)
		input.deleteLastChar();

	bool correct = false;
	for (uint j = 0; j < box.answers.size(); ++j) {
		if (!box.answers[j].empty() && input.equalsIgnoreCase(box.answers[j])) {
			correct = true;
			break;
		}
	}

	if (correct) {
		// Strip the cursor from the actual buffer, as the original does
		if (!box.typedText.empty() && box.typedText.lastChar() == cursorChar)
			box.typedText.deleteLastChar();
		box.correct = true;

		// A silent check only scores the box; its flag is set by the Enter/Tab path
		if (!silent && box.correctFlag != -1)
			NancySceneState.setEventFlag(box.correctFlag, g_nancy->_true);

		saveAnswer(_currentBox);
	} else if (g_nancy->getGameType() >= kGameTypeNancy15) {
		// Nancy 15 clears the flag of a box that is no longer right
		box.correct = false;
		if (box.correctFlag != -1)
			NancySceneState.setEventFlag(box.correctFlag, g_nancy->_false);
	}

	if (box.nonEmptyFlag != -1) {
		NancySceneState.setEventFlag(box.nonEmptyFlag, box.typedText.empty() ? g_nancy->_false : g_nancy->_true);
	}

	return correct;
}

uint32 QuizPuzzle::getSaveKey() {
	if (g_nancy->getGameType() < kGameTypeNancy15) {
		return _solveScene._sceneChange.sceneID;
	}

	// Nancy 15 keys its save slots by scene and record index, so that the several
	// quiz records a scene may carry each keep their own answers.
	const Common::Array<ActionRecord *> &records = NancySceneState.getActionManager().getActionRecords();
	uint index = 0;
	for (uint i = 0; i < records.size(); ++i) {
		if (records[i] == this) {
			index = i;
			break;
		}
	}

	return ((uint32)NancySceneState.getSceneInfo().sceneID << 16) | index;
}

void QuizPuzzle::loadSavedAnswers() {
	QuizPuzzleData *data = (QuizPuzzleData *)NancySceneState.getPuzzleData(QuizPuzzleData::getTag());
	if (!data) {
		return;
	}

	uint32 key = getSaveKey();

	if (_isDisplayOnly) {
		// A display-only record shows the answers typed into the quiz of another
		// scene, whichever of that scene's records they came from
		key = 0;
		for (auto &entry : data->typedText) {
			if ((entry._key >> 16) == _displaySourceScene) {
				key = entry._key;
				break;
			}
		}
	} else if (g_nancy->getGameType() >= kGameTypeNancy15 &&
			(_restoreFlag == -1 || !NancySceneState.getEventFlag(_restoreFlag, g_nancy->_true))) {
		// Without the restore flag set the puzzle starts over
		data->boxCorrect.erase(key);
		data->typedText.erase(key);
		return;
	}

	if (!data->typedText.contains(key)) {
		return;
	}

	const Common::Array<Common::String> &saved = data->typedText[key];
	for (uint i = 0; i < _boxes.size() && i < saved.size(); ++i) {
		_boxes[i].typedText = saved[i];
	}

	if (_isDisplayOnly) {
		// Another quiz's answers are only shown, never scored
		return;
	}

	// The original re-checks every restored answer instead of trusting the slot
	uint currentBox = _currentBox;
	for (uint i = 0; i < _boxes.size(); ++i) {
		_currentBox = i;
		checkAnswerForCurrentBox(true);
	}
	_currentBox = currentBox;
}

void QuizPuzzle::saveAnswer(uint boxIndex) {
	if (_isDisplayOnly) {
		return;
	}

	QuizPuzzleData *data = (QuizPuzzleData *)NancySceneState.getPuzzleData(QuizPuzzleData::getTag());
	if (!data) {
		return;
	}

	uint32 key = getSaveKey();
	Common::Array<bool> &boxCorrect = data->boxCorrect[key];
	Common::Array<Common::String> &typedText = data->typedText[key];

	if (boxCorrect.size() < _boxes.size()) {
		boxCorrect.resize(_boxes.size(), false);
		typedText.resize(_boxes.size());
	}

	boxCorrect[boxIndex] = _boxes[boxIndex].correct;
	typedText[boxIndex] = _boxes[boxIndex].typedText;
}

} // End of namespace Action
} // End of namespace Nancy
