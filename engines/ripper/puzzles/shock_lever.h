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

#ifndef RIPPER_PUZZLES_SHOCK_LEVER_H
#define RIPPER_PUZZLES_SHOCK_LEVER_H

#include "audio/mixer.h"
#include "common/array.h"

#include "ripper/display.h"
#include "ripper/media.h"
#include "ripper/puzzles/puzzle.h"
#include "ripper/resources.h"

namespace Ripper {

class RipperEngine;

class ShockLeverPuzzle : public Puzzle, public MediaSequenceCallback {
public:
	explicit ShockLeverPuzzle(RipperEngine *engine);

	Result run(uint completionFlag) override;
	uint16 service(uint frame) override;
	bool ownsInput() const override { return true; }

private:
	enum HoverControl {
		kHoverNone,
		kHoverLever,
		kHoverSubmit,
		kHoverExit
	};

	bool loadAssets();
	bool loadBitmap(AssetLibrary &library, const Common::String &name,
		BitmapAssetFrame &frame);
	bool loadLever(uint lever);
	bool drawBackground();
	void drawOverlays();
	void drawPuzzleHelpOverlay(byte *screen, uint pitch) const;
	void restoreLeverBackings(byte *screen, uint pitch) const;
	void restorePuzzleHelpBackings(byte *screen, uint pitch) const;
	Common::Point puzzleHelpLabelPosition(uint lever) const;
	void drawFrame(byte *screen, uint pitch, const BitmapAssetFrame &frame,
		int x, int y, bool transparent) const;
	int findLever(const Common::Point &point) const;
	HoverControl findControl(const Common::Point &point, int &lever) const;
	void updateCursor(const Common::Point &point);
	void updateDraggedLever(const Common::Point &point);
	bool playCue(uint cue, bool loop = false, uint volumePercent = 100);
	void stopAudio();
	bool waitForCue(uint cue);
	bool playOutcome(uint correctCount);
	uint countCorrect(bool logState) const;
	uint16 serviceKeyboard();

	AssetLibrary _library;
	BitmapAssetFrame _background;
	BitmapAssetFrame _submit;
	BitmapFontAsset _puzzleHelpFont;
	Common::Array<BitmapAssetFrame> _leverFrames[3];
	IndexedDisplaySnapshot _incomingDisplay;
	Audio::SoundHandle _audioHandles[5];
	uint _leverFrame[3];
	int _draggedLever;
	int _hoveredLever;
	HoverControl _hoveredControl;
	uint _keywordIndex;
	uint32 _lastLeverStepMillis;
	bool _puzzleHelpFontLoadAttempted;
	bool _puzzleHelpEnabled;
	bool _solved;
	bool _completionRecorded;
};

} // End of namespace Ripper

#endif // RIPPER_PUZZLES_SHOCK_LEVER_H
