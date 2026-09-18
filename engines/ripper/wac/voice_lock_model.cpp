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

#include "ripper/wac/voice_lock_model.h"

#include "common/util.h"

namespace Ripper {

namespace {

static const int kSolution[kWacVoiceLockSelectionCount][2] = {
	{ 240, 252 },
	{ 70, 82 },
	{ 87, 99 },
	{ 171, 184 },
	{ 190, 199 }
};

} // End of anonymous namespace

WacVoiceLockEditorState::WacVoiceLockEditorState() :
		quantized(false), sourceSelectionActive(false), selectingSource(false),
		adjustingSelectionStart(false), draggingSourceSelection(false),
		sourceSelectionDragWidth(0), sourceSelectionDragHeight(0),
		sourceSelectionDragOffsetX(0), sourceSelectionDragOffsetY(0),
		sourceSelectionDragBackingWidth(0), sourceSelectionDragBackingHeight(0),
		sourcePanelActive(true), hoveredButton(-1), pressedButton(-1),
		validateAfterPlayback(false), playbackProgressActive(false),
		playbackDuration(0), playbackProgressColumn(-1),
		puzzleHelpEnabled(false) {
}

void WacVoiceLockEditorState::clearEditor() {
	validateAfterPlayback = false;
	playbackProgressActive = false;
	playbackProgressColumn = -1;
	selections.clear();
	editorSegments.clear();
	assembledAudio.clear();
	sourcePanelActive = false;
}

void WacVoiceLockEditorState::resetSourceDrag() {
	draggingSourceSelection = false;
	sourceSelectionDragPixels.clear();
	sourceSelectionDragWidth = 0;
	sourceSelectionDragHeight = 0;
}

WacVoiceLockSelection wacVoiceLockSolution(uint index) {
	assert(index < kWacVoiceLockSelectionCount);
	return WacVoiceLockSelection(kSolution[index][0], kSolution[index][1]);
}

bool validateWacVoiceLockSelections(
		const Common::Array<WacVoiceLockSelection> &selections,
		Common::String &diagnostics) {
	diagnostics.clear();
	if (selections.size() != kWacVoiceLockSelectionCount) {
		diagnostics = Common::String::format("count=%u expected=%u",
			selections.size(), kWacVoiceLockSelectionCount);
		return false;
	}

	bool allMatched = true;
	for (uint selection = 0; selection < selections.size(); ++selection) {
		bool found = false;
		uint diagnosticCandidate = 0;
		int diagnosticStartDelta =
			selections[selection].start - kSolution[0][0];
		int diagnosticEndDelta =
			selections[selection].end - kSolution[0][1];
		int closestDistance = MAX(ABS(diagnosticStartDelta),
			ABS(diagnosticEndDelta));
		for (uint candidate = 0; candidate < kWacVoiceLockSelectionCount;
				++candidate) {
			const int startDelta =
				selections[selection].start - kSolution[candidate][0];
			const int endDelta =
				selections[selection].end - kSolution[candidate][1];
			const int distance = MAX(ABS(startDelta), ABS(endDelta));
			if (distance < closestDistance) {
				diagnosticCandidate = candidate;
				diagnosticStartDelta = startDelta;
				diagnosticEndDelta = endDelta;
				closestDistance = distance;
			}
			if (ABS(startDelta) <= (int)kWacVoiceLockSelectionTolerance &&
					ABS(endDelta) <=
						(int)kWacVoiceLockSelectionTolerance) {
				diagnosticCandidate = candidate;
				diagnosticStartDelta = startDelta;
				diagnosticEndDelta = endDelta;
				found = true;
				break;
			}
		}
		if (!diagnostics.empty())
			diagnostics += " ";
		diagnostics += Common::String::format(
			"#%u=%d..%d target%u=%d..%d delta=%+d,%+d match=%d",
			selection, selections[selection].start, selections[selection].end,
			diagnosticCandidate, kSolution[diagnosticCandidate][0],
			kSolution[diagnosticCandidate][1], diagnosticStartDelta,
			diagnosticEndDelta, found);
		allMatched &= found;
	}
	return allMatched;
}

void appendWacVoiceLockSelectionAudio(const WacVoiceLockPcm &source,
		const WacVoiceLockSelection &selection,
		const Common::Rect &waveform, Common::Array<byte> &assembled) {
	if (source.data.empty() || source.bytesPerSample == 0)
		return;
	const int startX = CLIP<int>(MIN(selection.start, selection.end),
		waveform.left, waveform.right - 1);
	const int endX = CLIP<int>(MAX(selection.start, selection.end),
		waveform.left + 1, waveform.right);
	uint start = (uint64)(startX - waveform.left) * source.data.size() /
		waveform.width();
	uint end = (uint64)(endX - waveform.left) * source.data.size() /
		waveform.width();
	start -= start % source.bytesPerSample;
	end -= end % source.bytesPerSample;
	end = MIN<uint>(end, source.data.size());
	if (end <= start)
		return;
	const uint oldSize = assembled.size();
	assembled.resize(oldSize + end - start);
	memcpy(assembled.data() + oldSize, source.data.data() + start, end - start);
}

int getWacVoiceLockEditorWaveformWidth(
		const Common::Array<WacVoiceLockEditorSegment> &segments,
		const Common::Rect &bounds) {
	uint width = 0;
	for (uint segment = 0; segment < segments.size(); ++segment)
		width += segments[segment].width;
	return MIN<int>(width, bounds.width());
}

uint32 getWacVoiceLockDuration(uint byteCount,
		const WacVoiceLockPcm &pcm) {
	if (pcm.sampleRate == 0 || pcm.bytesPerSample == 0)
		return 0;
	return MAX<uint32>(1, (uint64)byteCount * 1000 /
		(pcm.sampleRate * pcm.bytesPerSample));
}

} // End of namespace Ripper
