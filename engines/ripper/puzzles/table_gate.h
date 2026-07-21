/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this program.
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

#ifndef RIPPER_PUZZLES_TABLE_GATE_H
#define RIPPER_PUZZLES_TABLE_GATE_H

#include "audio/mixer.h"
#include "common/array.h"
#include "common/random.h"
#include "common/rect.h"

#include "ripper/display.h"
#include "ripper/resources.h"

namespace Ripper {

class RipperEngine;

class TableGatePuzzle {
public:
	enum Result {
		kExited,
		kSolved,
		kLoadFailed
	};

	explicit TableGatePuzzle(RipperEngine *engine);

	Result run(uint completionFlag);

private:
	struct LineState {
		int x;
		int y;
		int targetX;
		int targetY;
		int deltaX;
		int deltaY;
		int stepX;
		int stepY;
		int error;
		uint stepsPerAdvance;

		LineState() : x(0), y(0), targetX(0), targetY(0), deltaX(0),
			deltaY(0), stepX(0), stepY(0), error(0), stepsPerAdvance(1) {}
	};

	bool captureBackground();
	void restoreBackground() const;
	bool loadConfiguration();
	bool loadAssets();
	bool loadFrame(const Common::String &name, BitmapAssetFrame &frame);
	void render();
	void drawFrame(byte *screen, uint pitch, const BitmapAssetFrame &frame,
		int x, int y) const;
	void updateCursor(const Common::Point &point);
	int findLever(const Common::Point &point) const;
	bool isExitRegion(const Common::Point &point) const;
	void activateLever(uint lever);
	void startMarkerRun();
	int nextPathNode(uint node) const;
	void initializeMarkerSegment();
	bool advanceMarkerLine();
	Result serviceMarker(uint completionFlag);
	void resetAfterFailedRun();
	void playCue(uint cue);
	void stopAudio();

	RipperEngine *_engine;
	BitmapAssetFrame _markerFrame;
	Common::Array<BitmapAssetFrame> _launchFrames;
	Common::Array<BitmapAssetFrame> _gateFrames;
	Common::Array<BitmapAssetFrame> _activationFrames;
	Common::Array<BitmapAssetFrame> _leverFrames;
	IndexedDisplaySnapshot _backgroundDisplay;
	Audio::SoundHandle _audioHandles[6];
	Common::RandomSource _random;
	LineState _markerLine;
	bool _gateStates[13];
	bool _leverEnabled[5];
	int _leverGates[5][5];
	bool _allowMultipleLeverUse;
	bool _markerActive;
	bool _segmentAudioPending;
	uint _leverActivationsBeforeStart;
	uint _leverActivationCount;
	uint _visibleActivationCount;
	uint _currentPathNode;
	uint _launchFrame;
	uint _speedRampFrameCounter;
	uint32 _lastMarkerAdvanceMillis;
	uint32 _segmentStartMillis;
	int _pressedLever;
	int _hoveredLever;
};

} // End of namespace Ripper

#endif // RIPPER_PUZZLES_TABLE_GATE_H
