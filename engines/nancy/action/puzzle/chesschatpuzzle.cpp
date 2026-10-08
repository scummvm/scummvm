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
#include "engines/nancy/input.h"
#include "engines/nancy/sound.h"
#include "engines/nancy/util.h"
#include "engines/nancy/misc/hypertext.h"

#include "engines/nancy/state/scene.h"
#include "engines/nancy/action/puzzle/chesschatpuzzle.h"

namespace Nancy {
namespace Action {

// The fonts of the chat itself aren't in the record
static const uint kChatFontID = 1;
static const uint kChatColorID = 1;

static const uint32 kCaretBlinkTime = 500;

// Upper limit for the height of the rendered chat
static const uint kMaxChatHeight = 2000;

class ChessChatRenderer : public Misc::HypertextParser {
public:
	// Returns the height of the text
	uint render(const Common::String &text, uint width, uint maxHeight, uint fontID, uint colorID, Graphics::ManagedSurface &image);
};

uint ChessChatRenderer::render(const Common::String &text, uint width, uint maxHeight, uint fontID, uint colorID, Graphics::ManagedSurface &image) {
	_fullSurface.create(width, maxHeight, g_nancy->_graphics->getInputPixelFormat());
	_fullSurface.clear(g_nancy->_graphics->getTransColor());
	_defaultTextColor = colorID;

	addTextLine(text);
	drawAllText(_fullSurface.getBounds(), 0, fontID, fontID);

	uint height = CLIP<uint>(_drawnTextHeight, 1, maxHeight);
	image.create(width, height, _fullSurface.format);
	image.blitFrom(_fullSurface, Common::Rect(width, height), Common::Point());
	image.setTransparentColor(g_nancy->_graphics->getTransColor());

	return height;
}

// The record names the AUTOTEXT entries of its texts
Common::String ChessChatPuzzle::readText(Common::SeekableReadStream &stream) {
	Common::String key;
	readFilename(stream, key);

	const CVTX *autotext = (const CVTX *)g_nancy->getEngineData("AUTOTEXT");
	assert(autotext);
	return autotext->texts.getValOrDefault(key);
}

void ChessChatPuzzle::readData(Common::SeekableReadStream &stream) {
	stream.skip(33);	// A name, empty in the game data
	_caret = stream.readByte();
	_maxTypedLength = stream.readUint32LE();

	_wrongKeySound.readData(stream);
	_sendSound.readData(stream);
	_replySound.readData(stream);

	readExclusiveRect(stream, _chatRect);
	readExclusiveRect(stream, _typingRect);
	_typingFontID = stream.readSint16LE();
	_typingColorID = stream.readSint16LE();
	_minReplyDelay = stream.readSint32LE();
	_maxReplyDelay = stream.readSint32LE();
	readFilename(stream, _waitCharacters);
	_waitCharactersPerSecond = stream.readSint32LE();
	_endFlag = stream.readSint16LE();

	_playerPrefix = readText(stream);
	_opponentPrefix = readText(stream);
	_greeting = readText(stream);

	_moveCharacters.resize(stream.readUint16LE());
	for (Common::String &characters : _moveCharacters) {
		characters = readText(stream);
	}

	_confusedReplies.resize(stream.readUint16LE());
	for (Common::String &reply : _confusedReplies) {
		reply = readText(stream);
	}

	_letterSound.readData(stream);

	_letterGrid.resize(stream.readUint16LE());
	for (Common::String &row : _letterGrid) {
		row = readText(stream);
	}

	_maxWordLength = stream.readUint32LE();

	// One more list of texts, which nothing was found to use
	stream.skip(stream.readUint16LE() * 33);

	_words.resize(stream.readUint16LE());
	for (Word &word : _words) {
		word.word = readText(stream);
		word.reply = readText(stream);
		word.flag = stream.readSint16LE();
	}
}

void ChessChatPuzzle::init() {
	initViewportSurface();

	_chat = _opponentPrefix + _greeting;
	_nextCaretTime = g_nancy->getTotalPlayTime() + kCaretBlinkTime;

	g_nancy->_input->setVKEnabled(true);
	redraw();
}

void ChessChatPuzzle::redraw() {
	_drawSurface.clear(_drawSurface.getTransparentColor());

	// The newest lines of the chat stay in view
	ChessChatRenderer renderer;
	Graphics::ManagedSurface chatImage;
	const int height = renderer.render(_chat, _chatRect.width(), kMaxChatHeight, kChatFontID, kChatColorID, chatImage);
	const int top = MAX<int>(height - _chatRect.height(), 0);
	_drawSurface.blitFrom(chatImage, Common::Rect(0, top, chatImage.w, MIN<int>(height, top + _chatRect.height())), _chatRect.origin());

	Common::String typing = _typed;
	if (_isWaitingForReply) {
		if (_shownWaitCharacter) {
			typing = Common::String(_shownWaitCharacter);
		}
	} else if (_isCaretShown) {
		typing += _caret;
	}

	if (!typing.empty()) {
		ChessChatRenderer typingRenderer;
		Graphics::ManagedSurface typingImage;
		typingRenderer.render(typing, _typingRect.width(), _typingRect.height(), _typingFontID, _typingColorID, typingImage);
		_drawSurface.blitFrom(typingImage, Common::Rect(typingImage.w, MIN<int>(typingImage.h, _typingRect.height())), _typingRect.origin());
	}

	_needsRedraw = true;
}

// The sound of a letter is named after it
void ChessChatPuzzle::sayLetter(char letter) {
	if (_letterSound.names.empty()) {
		return;
	}

	SoundDescription sound;
	sound.name = _letterSound.names[0] + letter;
	sound.channelID = _letterSound.channel;
	sound.numLoops = MAX<int32>(_letterSound.numLoops, 1);
	sound.volume = _letterSound.volume;

	g_nancy->_sound->loadSound(sound);
	g_nancy->_sound->playSound(sound);
	showSubtitle(resolveSubtitleText(sound.name));
}

Common::String ChessChatPuzzle::getReply(const Common::String &move) {
	const uint kPiece = 0;
	const uint kFile = 1;
	const uint kRank = 2;
	const uint kMoveLength = 3;

	bool isMove = move.size() == kMoveLength && _moveCharacters.size() >= kMoveLength;
	size_t file = 0;
	size_t rank = 0;

	if (isMove) {
		file = _moveCharacters[kFile].findFirstOf(move[kFile]);
		rank = _moveCharacters[kRank].findFirstOf(move[kRank]);
		isMove = _moveCharacters[kPiece].findFirstOf(move[kPiece]) != Common::String::npos &&
				file != Common::String::npos && rank != Common::String::npos &&
				rank < _letterGrid.size() && file < _letterGrid[rank].size();
	}

	if (isMove) {
		const char letter = _letterGrid[rank][file];
		_word += letter;

		if (_word.size() <= _maxWordLength) {
			sayLetter(letter);

			for (const Word &word : _words) {
				if (_word.equalsIgnoreCase(word.word)) {
					_pendingFlag = word.flag;
					return word.reply;
				}
			}

			// A move made up at random
			Common::String reply;
			for (const Common::String &characters : _moveCharacters) {
				if (!characters.empty()) {
					reply += characters[g_nancy->_randomSource->getRandomNumber(characters.size() - 1)];
				}
			}

			return reply;
		}

		_pendingFlag = _endFlag;
	}

	if (_confusedReplies.empty()) {
		return Common::String();
	}

	return _confusedReplies[g_nancy->_randomSource->getRandomNumber(_confusedReplies.size() - 1)];
}

void ChessChatPuzzle::sendMove() {
	_pendingReply = getReply(_typed);
	playSoundBlock(_sendSound);

	_chat += _playerPrefix + _typed;
	_typed.clear();

	_isWaitingForReply = true;
	_shownWaitCharacter = 0;
	_waitStartTime = g_nancy->getTotalPlayTime();
	_replyTime = _waitStartTime + _minReplyDelay;
	if (_maxReplyDelay > _minReplyDelay) {
		_replyTime += g_nancy->_randomSource->getRandomNumber(_maxReplyDelay - _minReplyDelay - 1);
	}

	redraw();
}

void ChessChatPuzzle::showReply() {
	_chat += _opponentPrefix + _pendingReply;
	playSoundBlock(_replySound);
	_isWaitingForReply = false;

	if (_pendingFlag != -1) {
		NancySceneState.setEventFlag(_pendingFlag, g_nancy->_true);
		_pendingFlag = -1;
	}

	redraw();
}

void ChessChatPuzzle::typeKey(const Common::KeyState &key) {
	if (key.keycode == Common::KEYCODE_BACKSPACE) {
		if (!_typed.empty()) {
			_typed.deleteLastChar();
		}
	} else if (key.keycode == Common::KEYCODE_RETURN || key.keycode == Common::KEYCODE_KP_ENTER) {
		sendMove();
		return;
	} else if (key.ascii < 0x80 && Common::isAlnum(key.ascii) && _typed.size() < _maxTypedLength) {
		_typed += (char)key.ascii;
	} else {
		playSoundBlock(_wrongKeySound);
		return;
	}

	redraw();
}

void ChessChatPuzzle::execute() {
	switch (_state) {
	case kBegin:
		init();
		registerGraphics();
		_state = kRun;
		// fall through
	case kRun: {
		const uint32 now = g_nancy->getTotalPlayTime();

		if (_isWaitingForReply) {
			if (now > _replyTime) {
				showReply();
			} else if (!_waitCharacters.empty()) {
				const uint frame = (now - _waitStartTime) * _waitCharactersPerSecond / 1000;
				const char character = _waitCharacters[frame % _waitCharacters.size()];
				if (character != _shownWaitCharacter) {
					_shownWaitCharacter = character;
					redraw();
				}
			}

			// The chat is over once its flag has been set
			if (NancySceneState.getEventFlag(_endFlag, g_nancy->_true)) {
				_isCaretShown = false;
				redraw();
				_state = kActionTrigger;
			}
		} else if (now >= _nextCaretTime) {
			_isCaretShown = !_isCaretShown;
			_nextCaretTime = now + kCaretBlinkTime;
			redraw();
		}

		break;
	}
	case kActionTrigger:
		g_nancy->_input->setVKEnabled(false);
		_isDone = true;
		break;
	}
}

void ChessChatPuzzle::handleInput(NancyInput &input) {
	if (_state != kRun || _isWaitingForReply) {
		return;
	}

	for (const Common::KeyState &key : input.otherKbdInput) {
		typeKey(key);

		if (_isWaitingForReply) {
			break;
		}
	}
}

} // End of namespace Action
} // End of namespace Nancy
