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

#include "engines/nancy/nancy.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/cursor.h"
#include "engines/nancy/input.h"
#include "engines/nancy/puzzledata.h"
#include "engines/nancy/sound.h"
#include "engines/nancy/util.h"

#include "engines/nancy/state/scene.h"
#include "engines/nancy/action/puzzle/decoderpuzzle.h"

namespace Nancy {
namespace Action {

static const uint kWordSize				= 100;	// stride of a target word
static const uint kSubstitutionSize		= 4;	// stride of either half of a substitution

static const byte kEnterKey				= '\r';
static const uint kNumRandomLetters		= 25;	// never picks 'Z'

// Backspace, return and the punctuation the original accepts - a narrower set
// than Common::isPunct()
static const byte kAcceptedSymbols[] = {
	'\b', '\r', ' ', '.', ',', '?', '!', '/', '$', '(', ')', '&', ':', ';', '+', '%', '=', '-'
};

// Accented characters accepted alongside them
static const byte kAcceptedHighKeys[] = {
	0x80, 0x9c, 0xa1, 0xbf, 0xc0, 0xc4, 0xc7, 0xc9, 0xd1, 0xd6, 0xdc, 0xdf,
	0xe0, 0xe1, 0xe2, 0xe4, 0xe7, 0xe8, 0xe9, 0xea, 0xeb, 0xed, 0xee, 0xef,
	0xf1, 0xf3, 0xf4, 0xf6, 0xf9, 0xfa, 0xfb, 0xfc
};

// Reads a NUL-padded string of exactly size bytes
static Common::String readFixedString(Common::SeekableReadStream &stream, uint size) {
	char *buf = new char[size + 1];
	stream.read(buf, size);
	buf[size] = '\0';
	Common::String ret(buf);
	delete[] buf;
	return ret;
}

static bool isInKeySet(byte key, const byte *set, uint size) {
	for (uint i = 0; i < size; ++i) {
		if (key == set[i]) {
			return true;
		}
	}

	return false;
}

bool DecoderPuzzle::isAcceptedKey(byte key) {
	// Common::isAlnum() and friends are ASCII-only, so the rest is listed out
	return Common::isAlnum(key) ||
		isInKeySet(key, kAcceptedSymbols, ARRAYSIZE(kAcceptedSymbols)) ||
		isInKeySet(key, kAcceptedHighKeys, ARRAYSIZE(kAcceptedHighKeys));
}

DecoderData *DecoderPuzzle::getPuzzleData() const {
	DecoderData *data = (DecoderData *)NancySceneState.getPuzzleData(DecoderData::getTag());
	assert(data);
	return data;
}

void DecoderPuzzle::readData(Common::SeekableReadStream &stream) {
	_saveOutput = stream.readByte();		// 0x3d
	_restoreOutput = stream.readByte();		// 0x3e
	_fontID = stream.readUint16LE();		// 0x3f
	_textX = stream.readSint32LE();			// 0x41
	_textY = stream.readSint32LE();			// 0x45
	_maxLength = stream.readSint16LE();		// 0x5d

	// Target words and the typed half of every substitution are uppercased on
	// load, making both comparisons case-insensitive
	int16 numWords = stream.readSint16LE();
	for (int16 i = 0; i < numWords; ++i) {
		Common::String word = readFixedString(stream, kWordSize);
		word.toUppercase();
		_words.push_back(word);
	}

	// A count of halves, not of pairs
	int16 numHalves = stream.readSint16LE();
	_substitutions.resize(numHalves / 2);
	for (uint i = 0; i < _substitutions.size(); ++i) {
		Substitution &sub = _substitutions[i];
		sub.keys = readFixedString(stream, kSubstitutionSize);
		sub.keys.toUppercase();
		sub.output = readFixedString(stream, kSubstitutionSize);
	}

	_typeSound.readData(stream);		// 0x6f
	_decodeSound.readData(stream);		// 0xc5

	readFilename(stream, _resetMovieName);	// 0x11b
	readRect(stream, _resetMovieRect);		// 0x11f

	_resetSound.readData(stream);		// 0x12f

	_solveScene.sceneID = stream.readUint16LE();	// 0x1db
	_solveScene.frameID = stream.readUint16LE();
	_solveScene.continueSceneSound = kContinueSceneSound;
	_solveFlag.label = stream.readSint16LE();
	_solveFlag.flag = stream.readByte();

	_solveSound.readData(stream);		// 0x185

	readExitHotspot(stream, _exitHotspot, _exitCursorType, _exitScene, _exitFlag);
	_exitScene.continueSceneSound = kContinueSceneSound;
}

void DecoderPuzzle::init() {
	Common::Rect vpBounds = NancySceneState.getViewport().getBounds();
	_drawSurface.create(vpBounds.width(), vpBounds.height(),
		g_nancy->_graphics->getInputPixelFormat());
	_drawSurface.clear(g_nancy->_graphics->getTransColor());
	setTransparent(true);
	setVisible(true);
	moveTo(vpBounds);

	// The decoded line is shared between a scene's records
	DecoderData *data = getPuzzleData();
	uint16 sceneID = NancySceneState.getSceneInfo().sceneID;
	if (_restoreOutput && data->sceneID == sceneID) {
		_output = data->text;
	} else {
		_output.clear();
	}

	if (_saveOutput) {
		data->sceneID = sceneID;
	}

	_pending.clear();
	_hasPendingKey = false;
	_decodeSoundPending = false;
	_resetting = false;
	_solved = false;

	if (!_resetMovieName.empty()) {
		_resetMovie.loadFile(_resetMovieName);
	}

	redraw();
}

void DecoderPuzzle::onPause(bool paused) {
	g_nancy->_input->setVKEnabled(!paused);
	RenderActionRecord::onPause(paused);
}

void DecoderPuzzle::playSoundBlock(const RandomSoundBlock &block) {
	if (block.names.empty()) {
		return;
	}

	uint idx = block.names.size() == 1 ? 0 : g_nancy->_randomSource->getRandomNumber(block.names.size() - 1);
	const Common::String &name = block.names[idx];
	if (name.empty() || name == "NO SOUND") {
		return;
	}

	SoundDescription desc;
	desc.name = name;
	desc.channelID = block.channel;
	desc.numLoops = block.numLoops > 0 ? block.numLoops : 1;
	desc.volume = block.volume;

	g_nancy->_sound->loadSound(desc);
	g_nancy->_sound->playSound(desc);
}

bool DecoderPuzzle::isSoundBlockPlaying(const RandomSoundBlock &block) const {
	return !block.names.empty() && g_nancy->_sound->isSoundPlaying((uint16)block.channel);
}

bool DecoderPuzzle::decodePending(bool &noMatch) {
	noMatch = false;

	// With no substitution table every keystroke produces a random letter
	if (_substitutions.empty()) {
		_output += (char)('A' + g_nancy->_randomSource->getRandomNumber(kNumRandomLetters - 1));
		_pending.clear();
		return true;
	}

	for (uint i = 0; i < _substitutions.size(); ++i) {
		if (_substitutions[i].keys == _pending) {
			_output += _substitutions[i].output;
			_pending.clear();
			return true;
		}
	}

	// Still the start of some substitution: keep collecting keys
	for (uint i = 0; i < _substitutions.size(); ++i) {
		if (_substitutions[i].keys.hasPrefix(_pending)) {
			return false;
		}
	}

	noMatch = true;
	_pending.clear();
	return false;
}

void DecoderPuzzle::checkSolved() {
	for (uint i = 0; i < _words.size(); ++i) {
		if (_words[i] == _output) {
			_solved = true;
			return;
		}
	}
}

void DecoderPuzzle::beginReset() {
	_output.clear();
	_pending.clear();

	if (_resetMovie.isVideoLoaded() && _resetMovie.getFrameCount() > 0) {
		_resetMovie.playRange(0, _resetMovie.getFrameCount() - 1);
		_resetting = true;
	}

	redraw();
}

void DecoderPuzzle::redraw() {
	_drawSurface.clear(g_nancy->_graphics->getTransColor());

	if (_resetting) {
		// The animation covers the line while it plays
		_resetMovie.drawFrame(_drawSurface, Common::Point(_resetMovieRect.left, _resetMovieRect.top));
	} else if (!_output.empty()) {
		const Graphics::Font *font = g_nancy->_graphics->getFont(_fontID);
		if (font) {
			// The stored y is the text baseline
			font->drawString(&_drawSurface, _output, _textX, _textY - (int)font->getFontHeight(),
				_drawSurface.w - _textX, 0);
		}
	}

	_needsRedraw = true;
}

void DecoderPuzzle::execute() {
	switch (_state) {
	case kBegin:
		init();
		registerGraphics();
		_state = kRun;
		// fall through
	case kRun:
		// Both sounds share a channel, so hold the decode one back
		if (_decodeSoundPending && !isSoundBlockPlaying(_typeSound)) {
			playSoundBlock(_decodeSound);
			_decodeSoundPending = false;
		}

		if (_solved) {
			_resetMovie.close();
			_resetting = false;
			playSoundBlock(_solveSound);
			_state = kActionTrigger;
			break;
		}

		if (_resetting) {
			if (!isSoundBlockPlaying(_resetSound)) {
				playSoundBlock(_resetSound);
			}

			if (_resetMovie.update()) {
				redraw();
			}

			if (!_resetMovie.isRangePlaying()) {
				_resetting = false;
				redraw();
			}
		} else if (_hasPendingKey) {
			_hasPendingKey = false;

			bool reset = (int)_output.size() > _maxLength || _pendingKey == kEnterKey;
			if (!reset) {
				bool noMatch = false;
				if (decodePending(noMatch)) {
					if (isSoundBlockPlaying(_typeSound)) {
						_decodeSoundPending = true;
					} else {
						playSoundBlock(_decodeSound);
					}

					checkSolved();
					redraw();
				}

				reset = noMatch;
			}

			if (reset) {
				beginReset();
			}
		}

		if (_saveOutput) {
			getPuzzleData()->text = _output;
		}

		break;
	case kActionTrigger:
		// The solve voiceover gets to finish first
		if (!_exitRequested && isSoundBlockPlaying(_solveSound)) {
			break;
		}

		if (_exitRequested) {
			NancySceneState.setEventFlag(_exitFlag);
			NancySceneState.changeScene(_exitScene);
		} else {
			NancySceneState.setEventFlag(_solveFlag);
			NancySceneState.changeScene(_solveScene);
		}

		finishExecution();
		break;
	}
}

void DecoderPuzzle::handleInput(NancyInput &input) {
	if (_state != kRun || _solved || _resetting || _hasPendingKey) {
		return;
	}

	if (!_exitHotspot.isEmpty() &&
			NancySceneState.getViewport().convertViewportToScreen(_exitHotspot).contains(input.mousePos)) {
		g_nancy->_cursor->setCursorType((CursorManager::CursorType)_exitCursorType, true);

		if (input.input & NancyInput::kLeftMouseButtonUp) {
			_exitRequested = true;
			_state = kActionTrigger;
			return;
		}
	}

	for (uint i = 0; i < input.otherKbdInput.size(); ++i) {
		byte key = (byte)input.otherKbdInput[i].ascii;

		if (!isAcceptedKey(key)) {
			continue;
		}

		// The original gets a key code, already uppercase for letters. Accented
		// characters are left alone, as they are in the loaded substitutions.
		if (Common::isLower(key)) {
			key = toupper(key);
		}

		playSoundBlock(_typeSound);
		_pending += (char)key;
		_pendingKey = key;
		_hasPendingKey = true;
		break;
	}
}

} // End of namespace Action
} // End of namespace Nancy
