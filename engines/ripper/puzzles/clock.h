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
 */

#ifndef RIPPER_PUZZLES_CLOCK_H
#define RIPPER_PUZZLES_CLOCK_H

#include "audio/mixer.h"
#include "common/array.h"
#include "common/rect.h"

#include "ripper/display.h"
#include "ripper/resources.h"

namespace Ripper {

class RipperEngine;

class ClockPuzzle {
public:
	enum Result {
		kExited,
		kSolved,
		kLoadFailed
	};

	explicit ClockPuzzle(RipperEngine *engine);

	Result run(uint completionFlag);

private:
	bool captureBackground();
	void restoreBackground() const;
	bool loadAssets();
	bool loadFrames(const char *pattern, uint count,
		Common::Array<BitmapAssetFrame> &frames);
	bool loadFrame(const Common::String &name, BitmapAssetFrame &frame);
	void drawBitmap(byte *screen, uint pitch, const BitmapAssetFrame &frame,
		int x, int y) const;
	void render() const;
	void playCue(uint cue);
	void stopAudio();
	bool playOverlay(uint index);
	uint16 serviceKeyboard();
	int findControl(const Common::Point &point) const;
	bool isExitRegion(const Common::Point &point) const;
	int dialIndexAt(const Common::Point &point) const;
	void updateCursor(const Common::Point &point);
	void applyStepper(uint16 command);
	bool isSolved() const;
	void loadPersistentState();
	void storePersistentState() const;

	RipperEngine *_engine;
	Common::Array<BitmapAssetFrame> _hourFrames;
	Common::Array<BitmapAssetFrame> _minuteFrames;
	Common::Array<BitmapAssetFrame> _armyFrames;
	Common::Array<BitmapAssetFrame> _egyptFrames;
	BitmapAssetFrame _clockFace;
	BitmapAssetFrame _armyBackdrop;
	BitmapAssetFrame _dialHitMask;
	BitmapAssetFrame _markerButtons[2];
	IndexedDisplaySnapshot _backgroundDisplay;
	Audio::SoundHandle _audioHandles[6];
	int _dialIndices[2];
	int _firstClockDigits[4];
	int _secondClockDigits[4];
	int _hoveredControl;
	int _dragDial;
	int _lastDragIndex;
	bool _halfDay;
};

} // End of namespace Ripper

#endif // RIPPER_PUZZLES_CLOCK_H
