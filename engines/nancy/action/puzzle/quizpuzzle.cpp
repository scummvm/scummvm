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
#include "engines/nancy/cursor.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/sound.h"
#include "engines/nancy/input.h"
#include "engines/nancy/util.h"

#include "engines/nancy/state/scene.h"
#include "engines/nancy/action/puzzle/quizpuzzle.h"
#include "engines/nancy/puzzledata.h"

namespace Nancy {
namespace Action {

QuizPuzzle::~QuizPuzzle() {
	g_nancy->_input->setVKEnabled(false);
	g_nancy->_sound->stopSound(_correctSound);
	g_nancy->_sound->stopSound(_wrongSound);
	g_nancy->_sound->stopSound(_doneSound);
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

	g_nancy->_input->setVKEnabled(true);
	RenderActionRecord::init();
}

Common::String QuizPuzzle::readSubtitle(Common::SeekableReadStream &stream) {
	const CVTX *autotext = (const CVTX *)g_nancy->getEngineData("AUTOTEXT");
	assert(autotext);

	Common::String result;
	char textBuf[30];

	stream.read(textBuf, 30);
	textBuf[29] = '\0';
	result = textBuf;

	if (!result.empty() && autotext->texts.contains(result))
		result = autotext->texts[result];

	return result;
}

void QuizPuzzle::showSubtitle(const Common::String &text) {
	if (!text.empty()) {
		NancySceneState.getTextbox().clear();
		NancySceneState.getTextbox().addTextLine(text);
	}
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
void QuizPuzzle::readDataOld(Common::SeekableReadStream &stream) {
	_fontID            = stream.readUint16LE();
	_cursorBlinkInterval = stream.readUint16LE();
	_numBoxes          = stream.readUint16LE();

	for (int i = 0; i < 5; ++i) {
		readRect(stream, _boxRects[i]);
	}

	char answerBuf[20];
	for (int i = 0; i < 5; ++i) {
		for (int j = 0; j < 3; ++j) {
			stream.read(answerBuf, 20);
			answerBuf[19] = '\0';
			_answers[i][j] = answerBuf;
		}
		_answerFlags[i] = stream.readSint16LE();
	}

	char textBuf[30];

	_correctSound.readNormal(stream);
	_correctText = readSubtitle(stream);

	_wrongSound.readNormal(stream);
	_wrongText = readSubtitle(stream);

	_solveScene.readData(stream);
	_doneSound.readNormal(stream);
	_doneText = readSubtitle(stream);

	_cancelScene.readData(stream);
}

// ---- Nancy 9+ data format ----
// Header (0xB2 bytes):
// 0x00  2   fontID
// 0x02  2   blinkInterval
// 0x04  1   cursor char
// 0x05  20  allowed chars (null-terminated)
// 0x19  25  solveScene
// 0x32  49  doneSound (readNormal)
// 0x63  30  done subtitle (skip)
// 0x81  25  cancelScene
// 0x9A  16  unknown (skip)
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
void QuizPuzzle::readDataNew(Common::SeekableReadStream &stream) {
	const CVTX *autotext = (const CVTX *)g_nancy->getEngineData("AUTOTEXT");
	assert(autotext);

	_fontID            = stream.readUint16LE();
	_cursorBlinkInterval = stream.readUint16LE();
	_cursorChar        = stream.readByte();

	char allowedBuf[20];
	stream.read(allowedBuf, 20);
	allowedBuf[19] = '\0';
	_allowedChars = allowedBuf;

	_solveScene.readData(stream);
	_doneSound.readNormal(stream);
	_doneText = readSubtitle(stream);

	_cancelScene.readData(stream);
	stream.skip(16); // unknown

	_correctSoundChannel = stream.readUint16LE();
	_wrongSoundChannel   = stream.readUint16LE();
	_skipEmptyOnEnter    = (stream.readByte() != 0);
	_autoCheck           = (stream.readByte() == 0); // 0 = auto-check ON
	_numBoxes            = stream.readUint16LE();

	if (_numBoxes > (uint16)kMaxBoxes) {
		warning("QuizPuzzle: numBoxes %d exceeds maximum %d, clamping", _numBoxes, kMaxBoxes);
		_numBoxes = kMaxBoxes;
	}

	char soundNameBuf[33];
	char answerBuf[20];
	for (int i = 0; i < _numBoxes; ++i) {
		readRect(stream, _boxRects[i]);

		for (int j = 0; j < 3; ++j) {
			stream.read(answerBuf, 20);
			answerBuf[19] = '\0';
			_answers[i][j] = answerBuf;
		}

		_answerFlags[i] = stream.readSint16LE();

		stream.read(soundNameBuf, 33);
		soundNameBuf[32] = '\0';
		_boxCorrectSoundName[i] = soundNameBuf;
		_boxCorrectSoundVolume[i] = stream.readUint16LE();
		_boxCorrectText[i] = readSubtitle(stream);

		stream.read(soundNameBuf, 33);
		soundNameBuf[32] = '\0';
		_boxWrongSoundName[i] = soundNameBuf;
		_boxWrongSoundVolume[i] = stream.readUint16LE();
		_boxWrongText[i] = readSubtitle(stream);

		// Precompute max answer length for auto-check mode
		_boxMaxLen[i] = 0;
		for (int j = 0; j < 3; ++j) {
			uint16 len = (uint16)_answers[i][j].size();
			if (len > _boxMaxLen[i])
				_boxMaxLen[i] = len;
		}
	}
}

void QuizPuzzle::readData(Common::SeekableReadStream &stream) {
	if (g_nancy->getGameType() == kGameTypeNancy8)
		readDataOld(stream);
	else
		readDataNew(stream);
}

// ---- Nancy 8 state machine ----
void QuizPuzzle::executeOld() {
	switch (_internalState) {
	case kTyping: {
		if (_hasNewKey) {
			_hasNewKey = false;

			Common::String &text = _typedText[_currentBox];
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
				if (text.size() < 16)
					text += _pendingChar;
				drawText();
				_pendingChar = 0;
			}
		}

		// Cursor blink: toggle trailing '-'
		Time now = g_nancy->getTotalPlayTime();
		if (now >= _nextBlinkTime) {
			_nextBlinkTime = now + _cursorBlinkInterval;
			Common::String &text = _typedText[_currentBox];
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
		_typedText[_currentBox].clear();
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
		if (_doneSound.name == "NO SOUND") {
			_internalState = kFinish;
		} else {
			g_nancy->_sound->loadSound(_doneSound);
			g_nancy->_sound->playSound(_doneSound);
			showSubtitle(_doneText);
			_internalState = kWaitDone;
		}
		break;
	}

	case kWaitDone:
		if (!g_nancy->_sound->isSoundPlaying(_doneSound)) {
			g_nancy->_sound->stopSound(_doneSound);
			_internalState = kFinish;
		}
		break;

	case kFinish:
		_state = kActionTrigger;
		break;
	}
}

// ---- Nancy 9+ state machine ----
void QuizPuzzle::executeNew() {
	switch (_internalState) {
	case kTyping: {
		if (_hasNewKey) {
			_hasNewKey = false;

			Common::String &text = _typedText[_currentBox];
			bool hadCursor = !text.empty() && text.lastChar() == _cursorChar;
			if (hadCursor)
				text.deleteLastChar();

			if (_pendingBackspace) {
				if (!text.empty())
					text.deleteLastChar();
				drawText();
				_pendingBackspace = false;
			} else if (_pendingReturn) {
				if (text.empty()) {
					if (hadCursor) text += _cursorChar;
					if (!_skipEmptyOnEnter)
						advanceToNextBox();
				} else {
					if (hadCursor) text += _cursorChar;
					_internalState = kCheckAnswer;
				}
				_pendingReturn = false;
			} else if (_pendingChar != 0) {
				bool charAllowed = _allowedChars.empty();
				if (!charAllowed) {
					for (uint ci = 0; ci < _allowedChars.size(); ++ci) {
						if (_allowedChars[ci] == _pendingChar) {
							charAllowed = true;
							break;
						}
					}
				}
				if (charAllowed && text.size() < 16) {
					text += _pendingChar;

					if (_autoCheck) {
						bool correct = checkAnswerForCurrentBox();
						if (correct) {
							_internalState = kStartCorrect;
						} else if ((uint16)text.size() >= _boxMaxLen[_currentBox]) {
							_internalState = kStartWrong;
						}
					}
				}
				drawText();
				_pendingChar = 0;
			}
		}

		// Cursor blink (only while still typing)
		if (_internalState == kTyping && !checkAllSolved()) {
			Time now = g_nancy->getTotalPlayTime();
			if (now >= _nextBlinkTime) {
				_nextBlinkTime = now + _cursorBlinkInterval;
				Common::String &text = _typedText[_currentBox];
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
		_activeBoxSound.name         = _boxCorrectSoundName[_currentBox];
		_activeBoxSound.channelID    = _correctSoundChannel;
		_activeBoxSound.volume       = _boxCorrectSoundVolume[_currentBox];
		_activeBoxSound.playCommands = 1;
		_activeBoxSound.numLoops     = 1;

		if (_activeBoxSound.name == "NO SOUND") {
			advanceToNextBox();
			_solved = checkAllSolved();
			_internalState = _solved ? kStartDone : kTyping;
		} else {
			g_nancy->_sound->loadSound(_activeBoxSound);
			g_nancy->_sound->playSound(_activeBoxSound);
			showSubtitle(_boxCorrectText[_currentBox]);
			advanceToNextBox();
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
		_typedText[_currentBox].clear();
		drawText();

		_activeBoxSound.name         = _boxWrongSoundName[_currentBox];
		_activeBoxSound.channelID    = _wrongSoundChannel;
		_activeBoxSound.volume       = _boxWrongSoundVolume[_currentBox];
		_activeBoxSound.playCommands = 1;
		_activeBoxSound.numLoops     = 1;

		if (_activeBoxSound.name == "NO SOUND") {
			_internalState = kTyping;
		} else {
			g_nancy->_sound->loadSound(_activeBoxSound);
			g_nancy->_sound->playSound(_activeBoxSound);
			showSubtitle(_boxWrongText[_currentBox]);
			_internalState = kWaitWrong;
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
		if (_doneSound.name == "NO SOUND") {
			_internalState = kFinish;
		} else {
			g_nancy->_sound->loadSound(_doneSound);
			g_nancy->_sound->playSound(_doneSound);
			showSubtitle(_doneText);
			_internalState = kWaitDone;
		}
		break;
	}

	case kWaitDone:
		if (!g_nancy->_sound->isSoundPlaying(_doneSound)) {
			g_nancy->_sound->stopSound(_doneSound);
			_internalState = kFinish;
		}
		break;

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
		_nextBlinkTime = g_nancy->getTotalPlayTime() + _cursorBlinkInterval;
		if (g_nancy->getGameType() == kGameTypeNancy8) {
			g_nancy->_sound->loadSound(_correctSound);
			g_nancy->_sound->loadSound(_wrongSound);
		}
		g_nancy->_sound->loadSound(_doneSound);

		// Restore previously correct boxes from persistent puzzle data
		QuizPuzzleData *qpd = (QuizPuzzleData *)NancySceneState.getPuzzleData(QuizPuzzleData::getTag());
		if (qpd) {
			uint16 key = _solveScene._sceneChange.sceneID;
			if (qpd->boxCorrect.contains(key)) {
				const auto &bc = qpd->boxCorrect[key];
				const auto &tt = qpd->typedText[key];
				for (uint i = 0; i < _numBoxes && i < bc.size(); ++i) {
					if (bc[i]) {
						_boxCorrect[i] = true;
						_typedText[i] = tt[i];
					}
				}
			}
		}
		// Start cursor on first unsolved box
		for (uint i = 0; i < _numBoxes; ++i) {
			if (!_boxCorrect[i]) {
				_currentBox = i;
				break;
			}
		}
		drawText();

		_state = kRun;
	}
		// fall through
	case kRun:
		if (g_nancy->getGameType() == kGameTypeNancy8)
			executeOld();
		else
			executeNew();
		break;

	case kActionTrigger:
		if (_cancelled)
			_cancelScene.execute();
		else if (_solved)
			_solveScene.execute();

		g_nancy->_input->setVKEnabled(false);
		finishExecution();
		break;
	}
}

void QuizPuzzle::handleInput(NancyInput &input) {
	if (_internalState != kTyping)
		return;

	char cursorChar = (g_nancy->getGameType() == kGameTypeNancy8) ? '-' : _cursorChar;

	// Hover over an unsolved text box: show the hotspot cursor and, on click,
	// move the typing focus to that box.
	for (uint i = 0; i < _numBoxes; ++i) {
		if (_boxCorrect[i])
			continue;
		Common::Rect screenRect = NancySceneState.getViewport().convertViewportToScreen(_boxRects[i]);
		if (!screenRect.contains(input.mousePos))
			continue;

		g_nancy->_cursor->setCursorType(CursorManager::kHotspot);

		if (input.input & NancyInput::kLeftMouseButtonUp) {
			if (i != _currentBox) {
				Common::String &oldText = _typedText[_currentBox];
				if (!oldText.empty() && oldText.lastChar() == cursorChar)
					oldText.deleteLastChar();
				_currentBox = i;
				_nextBlinkTime = 0;
				drawText();
			}
		}
		break;
	}

	for (auto &key : input.otherKbdInput) {
		if (key.keycode == Common::KEYCODE_BACKSPACE) {
			_pendingBackspace = true;
			_hasNewKey = true;
		} else if (key.keycode == Common::KEYCODE_TAB) {
			// Tab: advance to next unsolved box without submitting
			Common::String &oldText = _typedText[_currentBox];
			if (!oldText.empty() && oldText.lastChar() == cursorChar)
				oldText.deleteLastChar();
			advanceToNextBox();
			_nextBlinkTime = 0;
			drawText();
		} else if (key.keycode == Common::KEYCODE_RETURN ||
		           key.keycode == Common::KEYCODE_KP_ENTER) {
			_pendingReturn = true;
			_hasNewKey = true;
		} else if (key.ascii != 0 && key.ascii != (byte)cursorChar) {
			bool accept = false;
			if (g_nancy->getGameType() == kGameTypeNancy8) {
				accept = Common::isAlnum(key.ascii) || key.ascii == '.' || key.ascii == '-' ||
				         key.ascii == '\'' || Common::isSpace(key.ascii);
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
	g_nancy->_input->setVKEnabled(!paused);
	RenderActionRecord::onPause(paused);
}

void QuizPuzzle::drawText() {
	_drawSurface.clear(g_nancy->_graphics->getTransColor());

	const Graphics::Font *font = g_nancy->_graphics->getFont(_fontID);
	if (!font)
		return;

	for (uint i = 0; i < _numBoxes; ++i) {
		const Common::String &text = _typedText[i];
		if (text.empty())
			continue;

		Common::Rect bounds = _boxRects[i];
		bounds = NancySceneState.getViewport().convertViewportToScreen(bounds);
		bounds = convertToLocal(bounds);

		int y = bounds.bottom - font->getFontHeight();
		font->drawString(&_drawSurface, text, bounds.left - 1, y, bounds.width(), 0);
	}

	_needsRedraw = true;
}

void QuizPuzzle::advanceToNextBox() {
	for (uint n = 0; n < _numBoxes; ++n) {
		_currentBox = (_currentBox + 1) % _numBoxes;
		if (!_boxCorrect[_currentBox])
			return;
	}
}

bool QuizPuzzle::checkAllSolved() const {
	for (uint i = 0; i < _numBoxes; ++i) {
		if (!_boxCorrect[i])
			return false;
	}
	return true;
}

bool QuizPuzzle::checkAnswerForCurrentBox() {
	Common::String input = _typedText[_currentBox];
	char cursorChar = (g_nancy->getGameType() == kGameTypeNancy8) ? '-' : _cursorChar;
	if (!input.empty() && input.lastChar() == cursorChar)
		input.deleteLastChar();

	bool correct = false;
	for (int j = 0; j < 3; ++j) {
		if (!_answers[_currentBox][j].empty() &&
		        input.equalsIgnoreCase(_answers[_currentBox][j])) {
			correct = true;
			break;
		}
	}

	if (correct) {
		// Strip cursor from actual buffer (matches original game state-1 behaviour)
		if (!_typedText[_currentBox].empty() && _typedText[_currentBox].lastChar() == cursorChar)
			_typedText[_currentBox].deleteLastChar();
		_boxCorrect[_currentBox] = true;
		if (_answerFlags[_currentBox] != -1)
			NancySceneState.setEventFlag(_answerFlags[_currentBox], g_nancy->_true);

		// Persist so the answered box survives scene re-entry
		QuizPuzzleData *qpd = (QuizPuzzleData *)NancySceneState.getPuzzleData(QuizPuzzleData::getTag());
		if (qpd) {
			uint16 key = _solveScene._sceneChange.sceneID;
			auto &bc = qpd->boxCorrect[key];
			auto &tt = qpd->typedText[key];
			if (bc.size() < _numBoxes) {
				bc.resize(_numBoxes, false);
				tt.resize(_numBoxes);
			}
			bc[_currentBox] = true;
			tt[_currentBox] = _typedText[_currentBox];
		}
	}
	return correct;
}

} // End of namespace Action
} // End of namespace Nancy
