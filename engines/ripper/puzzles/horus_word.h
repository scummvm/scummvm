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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef RIPPER_PUZZLES_HORUS_WORD_H
#define RIPPER_PUZZLES_HORUS_WORD_H

#include "audio/mixer.h"
#include "common/array.h"
#include "common/rect.h"

#include "ripper/display.h"
#include "ripper/resources.h"

namespace Ripper {

class RipperEngine;

class HorusWordPuzzle {
public:
	enum Result {
		kExited,
		kSolved,
		kLoadFailed
	};

	explicit HorusWordPuzzle(RipperEngine *engine);

	Result run(uint completionFlag);

private:
	bool loadPcx(const Common::String &name, BitmapAssetFrame &frame);
	bool loadBitmap(const Common::String &name, BitmapAssetFrame &frame);
	bool loadAssets();
	void applyPalette() const;
	void drawFrame(byte *screen, uint pitch, const BitmapAssetFrame &frame,
		int x, int y, int opacity = -1) const;
	byte blendPixel(byte source, byte destination, uint opacity) const;
	void render() const;
	int findLetter(const Common::Point &point) const;
	bool isLetterSelected(uint letter) const;
	void updateHover(const Common::Point &point);
	bool playCue(uint cue, uint volume = 100, bool loop = false);
	void stopAudio();
	void clearWord(const char *reason, bool playResetCue = true);
	void beginFade(const char *reason);
	void selectLetter(uint letter);
	bool stepAnimation(uint32 now);
	bool hasVisibleLetters() const;
	Common::String selectedWord() const;

	RipperEngine *_engine;
	AssetLibrary _library;
	BitmapAssetFrame _background;
	BitmapAssetFrame _mask;
	BitmapAssetFrame _buttons[26];
	BitmapAssetFrame _letters[26];
	Common::Array<byte> _shading;
	IndexedDisplaySnapshot _incomingDisplay;
	Audio::SoundHandle _audioHandles[3];
	Common::Array<int> _selectedLetters;
	Common::Array<int> _letterPhases;
	Common::Array<int> _letterTargets;
	int _hoveredLetter;
	uint32 _lastAnimationTick;
	uint32 _settleDeadline;
	bool _wordComplete;
	bool _wordSolved;
	bool _exitRequested;
};

} // End of namespace Ripper

#endif // RIPPER_PUZZLES_HORUS_WORD_H
