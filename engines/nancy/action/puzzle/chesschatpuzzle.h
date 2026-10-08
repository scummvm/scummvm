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

#ifndef NANCY_ACTION_CHESSCHATPUZZLE_H
#define NANCY_ACTION_CHESSCHATPUZZLE_H

#include "common/keyboard.h"
#include "common/str-array.h"

#include "engines/nancy/commontypes.h"
#include "engines/nancy/action/puzzlerecord.h"

namespace Nancy {
namespace Action {

// Nancy18 chess game played over a chat. The player types moves (a piece, a file
// and a rank), and gets one back after a moment. The square of every move stands
// for a letter, which gets read out loud; the letters make up a word, and the
// words the record knows are answered with a line and an event flag in place of
// a move. Anything that isn't a move, or a word that got too long, ends the chat.
class ChessChatPuzzle : public PuzzleRecord {
public:
	ChessChatPuzzle() : PuzzleRecord(7) {}
	virtual ~ChessChatPuzzle() {}

	void init() override;

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;
	void handleInput(NancyInput &input) override;

	bool isViewportRelative() const override { return true; }

protected:
	Common::String getRecordTypeName() const override { return "ChessChatPuzzle"; }

	struct Word {
		Common::String word;
		Common::String reply;
		int16 flag = -1;
	};

	static Common::String readText(Common::SeekableReadStream &stream);

	void typeKey(const Common::KeyState &key);
	void sendMove();
	Common::String getReply(const Common::String &move);
	void sayLetter(char letter);
	void showReply();
	void redraw();

	// -- File data --
	char _caret = '_';
	uint32 _maxTypedLength = 0;
	RandomSoundBlock _wrongKeySound;
	RandomSoundBlock _sendSound;
	RandomSoundBlock _replySound;
	Common::Rect _chatRect;
	Common::Rect _typingRect;
	int16 _typingFontID = 0;
	int16 _typingColorID = 0;
	int32 _minReplyDelay = 0;	// in milliseconds
	int32 _maxReplyDelay = 0;
	Common::String _waitCharacters;	// shown one after the other while waiting for the reply
	int32 _waitCharactersPerSecond = 0;
	int16 _endFlag = -1;
	Common::String _playerPrefix;
	Common::String _opponentPrefix;
	Common::String _greeting;
	Common::StringArray _moveCharacters;	// what the characters of a move can be
	Common::StringArray _confusedReplies;
	RandomSoundBlock _letterSound;			// its name gets the letter added
	Common::StringArray _letterGrid;		// one row of letters per rank
	uint32 _maxWordLength = 0;
	Common::Array<Word> _words;

	// -- Runtime --
	Common::String _chat;
	Common::String _typed;
	Common::String _word;
	Common::String _pendingReply;
	int16 _pendingFlag = -1;
	bool _isWaitingForReply = false;
	uint32 _replyTime = 0;
	uint32 _waitStartTime = 0;
	bool _isCaretShown = true;
	uint32 _nextCaretTime = 0;
	char _shownWaitCharacter = 0;
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_CHESSCHATPUZZLE_H
