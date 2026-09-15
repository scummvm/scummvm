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

#ifndef RIPPER_PUZZLES_CIRCUIT_CHIP_H
#define RIPPER_PUZZLES_CIRCUIT_CHIP_H

#include "audio/mixer.h"
#include "common/array.h"
#include "common/rect.h"

#include "ripper/display.h"
#include "ripper/puzzles/puzzle.h"
#include "ripper/resources.h"

namespace Ripper {

class RipperEngine;

class CircuitChipPuzzle : public Puzzle {
public:
	explicit CircuitChipPuzzle(RipperEngine *engine);

	Result run(uint completionFlag) override;

private:
	bool loadAssets();
	bool loadPcx(const Common::String &path, BitmapAssetFrame &frame);
	bool captureManualBacking();
	void drawFrame(byte *screen, uint pitch, const BitmapAssetFrame &frame,
		int x, int y) const;
	void render();
	void captureMeter();
	void animateMeter(uint previousCount, uint correctCount);
	bool advanceIdleMeter(uint32 now);
	bool handleManualClick(const Common::Point &point);
	int findSourceSlot(const Common::Point &point) const;
	int findTargetSlot(const Common::Point &point) const;
	bool handleClick(const Common::Point &point);
	void pickSourceChip(uint slot);
	void pickTargetChip(uint slot);
	void placeSourceChip(uint slot);
	void placeTargetChip(uint slot);
	uint countCorrectGroups(bool logGroups) const;
	bool complete(uint completionFlag);
	bool handleKeyword(uint16 command, uint completionFlag, Result &result);
	bool updateCursor(const Common::Point &point);
	void stopAudio();
	Common::String placementString() const;

	BitmapAssetFrame _background;
	Common::Array<BitmapAssetFrame> _chips;
	Common::Array<BitmapAssetFrame> _idleMeterFrames;
	IndexedDisplaySnapshot _incomingDisplay;
	Common::Array<byte> _meterPixels;
	Audio::SoundHandle _audioHandles[6];
	Common::Point _dragPoint;
	int _sourceChips[16];
	int _targetChips[16];
	int _draggedChip;
	int _hoveredControl;
	uint _correctGroupCount;
	uint _idleMeterFrame;
	uint _keywordProgress;
	uint32 _lastMeterTick;
	uint _manualFirstVisible;
	uint _manualMaximumFirstVisible;
	uint _manualVisibleRows;
	int _manualHoveredControl;
	bool _manualAvailable;
};

} // End of namespace Ripper

#endif // RIPPER_PUZZLES_CIRCUIT_CHIP_H
