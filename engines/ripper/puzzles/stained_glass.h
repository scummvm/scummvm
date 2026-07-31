/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of their respective copyright holders.
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

#ifndef RIPPER_PUZZLES_STAINED_GLASS_H
#define RIPPER_PUZZLES_STAINED_GLASS_H

#include "audio/mixer.h"
#include "common/array.h"
#include "common/random.h"
#include "common/rect.h"

#include "ripper/display.h"
#include "ripper/resources.h"

namespace Ripper {

class RipperEngine;

class StainedGlassPuzzle {
public:
	enum Result {
		kExited,
		kSolved,
		kLoadFailed
	};

	explicit StainedGlassPuzzle(RipperEngine *engine);

	Result run(uint completionFlag);

private:
	bool loadConfiguration();
	bool loadAssets();
	bool loadPcx(const Common::String &name, BitmapAssetFrame &frame);
	bool loadBitmap(const Common::String &name, BitmapAssetFrame &frame);
	bool buildPuzzleBacking();
	void drawFrame(byte *screen, uint pitch, const BitmapAssetFrame &frame,
		int x, int y) const;
	void renderOrder(const uint *order);
	void renderTransition(const uint *previousOrder, const uint *targetOrder,
		uint step);
	bool animateTo(const uint *targetOrder, const char *reason);
	bool showInitialPreview();
	bool showSolvedPreview();
	bool waitForPreview(uint32 duration);
	uint countSolved() const;
	bool isSolved() const;
	int findTile(const Common::Point &point) const;
	void updateCursor(const Common::Point &point);
	bool applyMove(uint selectedTile);
	bool playCue(uint cue, bool loop = false);
	bool playAmbient();
	void stopAudio();
	bool serviceKeyword(uint16 command);
	void applyPalette();
	void clearDisplay();

	RipperEngine *_engine;
	AssetLibrary _library;
	BitmapAssetFrame _background;
	BitmapAssetFrame _transitionMask;
	BitmapAssetFrame _tiles[16];
	BitmapAssetFrame _blankTile;
	IndexedDisplaySnapshot _incomingDisplay;
	Common::Array<byte> _puzzleBacking;
	Audio::SoundHandle _cueHandles[5];
	Audio::SoundHandle _ambientHandle;
	Common::RandomSource _random;
	int _moveTable[16][16];
	uint _initialOrder[16];
	uint _currentOrder[16];
	uint _savedOrder[16];
	int _hoveredTile;
	uint _keywordIndex;
};

} // End of namespace Ripper

#endif // RIPPER_PUZZLES_STAINED_GLASS_H
