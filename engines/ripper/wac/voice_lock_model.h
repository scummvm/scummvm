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

#ifndef RIPPER_WAC_VOICE_LOCK_MODEL_H
#define RIPPER_WAC_VOICE_LOCK_MODEL_H

#include "common/array.h"
#include "common/rect.h"
#include "common/str.h"

namespace Ripper {

enum {
	kWacVoiceLockSelectionCount = 5,
	kWacVoiceLockSelectionTolerance = 3
};

struct WacVoiceLockPcm {
	Common::Array<byte> data;
	uint sampleRate;
	byte flags;
	uint bytesPerSample;

	WacVoiceLockPcm() : sampleRate(0), flags(0), bytesPerSample(0) {}
};

struct WacVoiceLockSelection {
	int start;
	int end;

	WacVoiceLockSelection() : start(0), end(0) {}
	WacVoiceLockSelection(int start_, int end_) : start(start_), end(end_) {}
};

struct WacVoiceLockEditorSegment {
	uint offset;
	uint size;
	uint width;

	WacVoiceLockEditorSegment(uint offset_, uint size_, uint width_) :
			offset(offset_), size(size_), width(width_) {}
};

struct WacVoiceLockEditorState {
	Common::Array<WacVoiceLockSelection> selections;
	Common::Array<WacVoiceLockEditorSegment> editorSegments;
	Common::Array<byte> assembledAudio;
	WacVoiceLockSelection sourceSelection;
	bool quantized;
	bool sourceSelectionActive;
	bool selectingSource;
	bool adjustingSelectionStart;
	bool draggingSourceSelection;
	Common::Array<byte> sourceSelectionDragPixels;
	int sourceSelectionDragWidth;
	int sourceSelectionDragHeight;
	Common::Point sourceSelectionDragPosition;
	int sourceSelectionDragOffsetX;
	int sourceSelectionDragOffsetY;
	Common::Array<byte> sourceSelectionDragBackingPixels;
	int sourceSelectionDragBackingWidth;
	int sourceSelectionDragBackingHeight;
	Common::Point sourceSelectionDragBackingPosition;
	bool sourcePanelActive;
	int hoveredButton;
	int pressedButton;
	bool validateAfterPlayback;
	bool playbackProgressActive;
	Common::Rect playbackProgressBounds;
	uint32 playbackDuration;
	int playbackProgressColumn;
	bool puzzleHelpEnabled;

	WacVoiceLockEditorState();

	void clearEditor();
	void resetSourceDrag();
};

WacVoiceLockSelection wacVoiceLockSolution(uint index);
bool validateWacVoiceLockSelections(
	const Common::Array<WacVoiceLockSelection> &selections,
	Common::String &diagnostics);
void appendWacVoiceLockSelectionAudio(const WacVoiceLockPcm &source,
	const WacVoiceLockSelection &selection, const Common::Rect &waveform,
	Common::Array<byte> &assembled);
int getWacVoiceLockEditorWaveformWidth(
	const Common::Array<WacVoiceLockEditorSegment> &segments,
	const Common::Rect &bounds);
uint32 getWacVoiceLockDuration(uint byteCount, const WacVoiceLockPcm &pcm);

} // End of namespace Ripper

#endif // RIPPER_WAC_VOICE_LOCK_MODEL_H
