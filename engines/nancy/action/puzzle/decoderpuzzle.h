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

#ifndef NANCY_ACTION_DECODERPUZZLE_H
#define NANCY_ACTION_DECODERPUZZLE_H

#include "engines/nancy/commontypes.h"
#include "engines/nancy/movieplayer.h"
#include "engines/nancy/action/actionrecord.h"

namespace Nancy {

struct DecoderData;

namespace Action {

// Keyboard decoding puzzle, new in Nancy14 (AR 182). Keystrokes collect into a
// pending sequence; when it matches the typed half of a substitution, the other
// half is appended to the decoded line. A sequence that is still a prefix of some
// substitution keeps collecting; one that matches nothing - as does Enter, or
// overflowing the line - wipes both buffers and plays the carriage-return
// animation. Solved once the line equals one of the target words.
//
// Each scene using it holds two records, one with the substitution table and one
// with none (every keystroke emits a random letter); they share the decoded line
// through the puzzle data.
class DecoderPuzzle : public RenderActionRecord {
public:
	DecoderPuzzle() : RenderActionRecord(7) {}
	virtual ~DecoderPuzzle() {}

	void init() override;
	void onPause(bool paused) override;

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;
	void handleInput(NancyInput &input) override;

	bool isViewportRelative() const override { return true; }

protected:
	Common::String getRecordTypeName() const override { return "DecoderPuzzle"; }

	// One entry of the substitution table: typing `keys` emits `output`.
	struct Substitution {
		Common::String keys;
		Common::String output;
	};

	static bool isAcceptedKey(byte key);

	DecoderData *getPuzzleData() const;
	void playSoundBlock(const RandomSoundBlock &block);
	bool isSoundBlockPlaying(const RandomSoundBlock &block) const;

	// Consumes the pending sequence. True if it produced output; sets noMatch
	// when the sequence cannot lead to any substitution.
	bool decodePending(bool &noMatch);
	void checkSolved();
	void beginReset();
	void redraw();

	// -- File data --
	byte _saveOutput = 0;		// 0x3d
	byte _restoreOutput = 0;	// 0x3e
	uint16 _fontID = 0;			// 0x3f
	int32 _textX = 0;			// 0x41
	int32 _textY = 0;			// 0x45, text baseline

	Common::Array<Common::String> _words;			// target words, uppercased
	Common::Array<Substitution> _substitutions;		// empty = emit random letters
	int16 _maxLength = 0;							// 0x5d

	RandomSoundBlock _typeSound;		// 0x6f, keystroke
	RandomSoundBlock _decodeSound;		// 0xc5, substitution resolved

	Common::Path _resetMovieName;		// 0x11b, carriage-return animation
	Common::Rect _resetMovieRect;		// 0x11f
	RandomSoundBlock _resetSound;		// 0x12f, plays while it runs

	SceneChangeDescription _solveScene;	// 0x1db
	FlagDescription _solveFlag;
	RandomSoundBlock _solveSound;		// 0x185, plays before the scene change

	// Give-up hotspot, from the count-prefixed 23-byte trailer
	Common::Rect _exitHotspot;
	uint16 _exitCursorType = 0;
	SceneChangeDescription _exitScene;
	FlagDescription _exitFlag;

	// -- Runtime state --
	MoviePlayer _resetMovie;

	Common::String _output;		// the decoded line
	Common::String _pending;	// keys typed since the last substitution
	byte _pendingKey = 0;
	bool _hasPendingKey = false;
	bool _decodeSoundPending = false;
	bool _resetting = false;
	bool _solved = false;
	bool _exitRequested = false;
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_DECODERPUZZLE_H
