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

#include "ripper/wac/voice_lock_renderer.h"

#include "common/debug.h"
#include "common/endian.h"
#include "common/system.h"
#include "common/util.h"
#include "graphics/surface.h"

#include "ripper/cursor.h"
#include "ripper/detection.h"
#include "ripper/modal_dialog.h"
#include "ripper/resources.h"
#include "ripper/ripper.h"
#include "ripper/wac/database.h"
#include "ripper/wac/voice_lock_model.h"

namespace Ripper {

namespace {

static const uint kWacVoiceLockFileResource = 0xb1;
static const uint kWacVoiceLockEditorResource = 0xb2;
static const byte kWacDatabaseBackground = 4;
static const byte kWacVoiceLockSelectionColor = 255;
static const byte kWacVoiceLockWaveformColor = 255;
static const byte kWacVoiceLockMarkerColor = 196;
static const byte kWacVoiceLockPuzzleHelpColor = 254;

static int sampleAt(const WacVoiceLockPcm &pcm, uint sample) {
	const uint offset = sample * pcm.bytesPerSample;
	if (offset + pcm.bytesPerSample > pcm.data.size())
		return 0;
	if (pcm.bytesPerSample == 1)
		return ((int)pcm.data[offset] - 128) << 8;
	return (int16)READ_LE_UINT16(pcm.data.data() + offset);
}

static void clearWaveform(Graphics::Surface &screen,
		const Common::Rect &bounds) {
	if (bounds.isEmpty())
		return;
	for (int y = bounds.top; y < bounds.bottom; ++y)
		memset(screen.getBasePtr(bounds.left, y), kWacDatabaseBackground,
			bounds.width());
}

static void drawWaveformSpan(Graphics::Surface &screen,
		const WacVoiceLockPcm &pcm, const Common::Rect &bounds,
		uint byteOffset, uint byteCount) {
	if (bounds.isEmpty() || pcm.data.empty() || pcm.bytesPerSample == 0 ||
			byteOffset >= pcm.data.size())
		return;

	byteOffset -= byteOffset % pcm.bytesPerSample;
	byteCount = MIN<uint>(byteCount - byteCount % pcm.bytesPerSample,
		pcm.data.size() - byteOffset);
	if (byteCount == 0)
		return;
	const int center = (bounds.top + bounds.bottom) / 2;
	const uint firstSourceSample = byteOffset / pcm.bytesPerSample;
	const uint sampleCount = byteCount / pcm.bytesPerSample;
	const int plotWidth = bounds.width();
	const int amplitude = MAX<int>(1, bounds.height() / 2);
	for (int column = 0; column < plotWidth; ++column) {
		const uint firstSample = firstSourceSample +
			(uint64)column * sampleCount / plotWidth;
		const uint lastSample = firstSourceSample + MAX<uint>(
			(uint64)column * sampleCount / plotWidth + 1,
			(uint64)(column + 1) * sampleCount / plotWidth);
		int minimum = 0;
		int maximum = 0;
		for (uint sample = firstSample;
				sample < MIN<uint>(lastSample,
					firstSourceSample + sampleCount); ++sample) {
			const int value = sampleAt(pcm, sample);
			minimum = MIN(minimum, value);
			maximum = MAX(maximum, value);
		}
		const int top = CLIP<int>(center - maximum * amplitude / 32768,
			bounds.top, bounds.bottom - 1);
		const int bottom = CLIP<int>(center - minimum * amplitude / 32768,
			bounds.top, bounds.bottom - 1);
		for (int y = top; y <= bottom; ++y)
			*((byte *)screen.getBasePtr(bounds.left + column, y)) =
				kWacVoiceLockWaveformColor;
	}
}

static void xorRect(Graphics::Surface &screen,
		const Common::Rect &bounds, byte color) {
	const int left = CLIP<int>(bounds.left, 0, screen.w);
	const int top = CLIP<int>(bounds.top, 0, screen.h);
	const int right = CLIP<int>(bounds.right, 0, screen.w);
	const int bottom = CLIP<int>(bounds.bottom, 0, screen.h);
	if (left >= right || top >= bottom)
		return;
	for (int y = top; y < bottom; ++y) {
		byte *pixel = (byte *)screen.getBasePtr(left, y);
		for (int x = left; x < right; ++x)
			*pixel++ ^= color;
	}
}

static void captureRect(const Graphics::Surface &screen,
		const Common::Rect &bounds, Common::Array<byte> &pixels) {
	pixels.resize(bounds.width() * bounds.height());
	for (int row = 0; row < bounds.height(); ++row) {
		memcpy(pixels.data() + row * bounds.width(),
			screen.getBasePtr(bounds.left, bounds.top + row),
			bounds.width());
	}
}

static void capturePixels(const Graphics::Surface &screen,
		Common::Array<byte> &pixels, int width, int height,
		int sourceLeft, int sourceTop) {
	if (width <= 0 || height <= 0) {
		pixels.clear();
		return;
	}
	pixels.resize(width * height);
	memset(pixels.data(), 0, pixels.size());
	const int firstColumn = MAX<int>(0, -sourceLeft);
	const int firstRow = MAX<int>(0, -sourceTop);
	const int lastColumn = MIN<int>(width, screen.w - sourceLeft);
	const int lastRow = MIN<int>(height, screen.h - sourceTop);
	if (firstColumn >= lastColumn || firstRow >= lastRow)
		return;
	for (int row = firstRow; row < lastRow; ++row) {
		memcpy(pixels.data() + row * width + firstColumn,
			screen.getBasePtr(sourceLeft + firstColumn, sourceTop + row),
			lastColumn - firstColumn);
	}
}

static void blitPixels(Graphics::Surface &screen,
		const Common::Array<byte> &pixels, int sourceWidth, int sourceHeight,
		int destinationLeft, int destinationTop) {
	if (pixels.size() != (uint)(sourceWidth * sourceHeight))
		return;
	const int sourceLeft = MAX<int>(0, -destinationLeft);
	const int sourceTop = MAX<int>(0, -destinationTop);
	const int sourceRight = MIN<int>(sourceWidth,
		screen.w - destinationLeft);
	const int sourceBottom = MIN<int>(sourceHeight,
		screen.h - destinationTop);
	if (sourceLeft >= sourceRight || sourceTop >= sourceBottom)
		return;
	for (int sourceY = sourceTop; sourceY < sourceBottom; ++sourceY) {
		memcpy(screen.getBasePtr(destinationLeft + sourceLeft,
				destinationTop + sourceY),
			pixels.data() + sourceY * sourceWidth + sourceLeft,
			sourceRight - sourceLeft);
	}
}

static void drawWaveform(Graphics::Surface &screen,
		const WacVoiceLockPcm &pcm, const Common::Rect &bounds,
		const WacVoiceLockSelection *selection) {
	clearWaveform(screen, bounds);
	drawWaveformSpan(screen, pcm, bounds, 0, pcm.data.size());

	if (!selection)
		return;
	const int start = CLIP<int>(selection->start,
		bounds.left, bounds.right - 1);
	const int end = CLIP<int>(selection->end,
		start + 1, bounds.right);
	// RunWacVoiceLockPuzzleScene selects display write mode 3 at 0x25006
	// before filling the span with 0xff. Reapplying the old span at 0x25265
	// restores its pixels before the changed span is drawn, identifying the
	// operation as an XOR highlight rather than an opaque fill.
	xorRect(screen, Common::Rect(start, bounds.top, end, bounds.bottom),
		kWacDatabaseBackground ^ kWacVoiceLockSelectionColor);
}

static void drawEditorWaveform(Graphics::Surface &screen,
		const WacVoiceLockPcm &pcm, const Common::Rect &bounds,
		const Common::Array<WacVoiceLockEditorSegment> &segments) {
	// RunWacVoiceLockPuzzleScene at 0x25403 copies each selected source
	// rectangle to the next editor column. Preserve those pixel widths rather
	// than scaling the growing PCM assembly across the complete panel.
	clearWaveform(screen, bounds);
	int left = bounds.left;
	for (uint segmentIndex = 0; segmentIndex < segments.size() &&
			left < bounds.right; ++segmentIndex) {
		const WacVoiceLockEditorSegment &segment = segments[segmentIndex];
		const int width = MIN<int>(segment.width, bounds.right - left);
		if (width <= 0)
			continue;
		drawWaveformSpan(screen, pcm,
			Common::Rect(left, bounds.top, left + width, bounds.bottom),
			segment.offset, segment.size);
		left += width;
	}
}

} // End of anonymous namespace

WacVoiceLockRenderer::WacVoiceLockRenderer(WacDatabaseSession *database,
		const Common::Array<BitmapAssetFrame> &buttonAssets,
		const Common::Rect *buttonBounds,
		const Common::Rect &sourcePanel, const Common::Rect &editorPanel,
		const Common::Rect &sourceWaveform, const Common::Rect &editorWaveform,
		bool editorAvailable) :
		_database(database), _buttonAssets(buttonAssets),
		_sourcePanel(sourcePanel), _editorPanel(editorPanel),
		_sourceWaveform(sourceWaveform), _editorWaveform(editorWaveform),
		_editorAvailable(editorAvailable) {
	for (uint button = 0; button < 3; ++button)
		_buttonBounds[button] = buttonBounds[button];
}

bool WacVoiceLockRenderer::restoreSourceSelectionDragBacking(
		WacVoiceLockEditorState &state) const {
	if (state.sourceSelectionDragBackingPixels.empty())
		return true;
	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen || screen->format.bytesPerPixel != 1) {
		if (screen)
			g_system->unlockScreen();
		return false;
	}
	blitPixels(*screen, state.sourceSelectionDragBackingPixels,
		state.sourceSelectionDragBackingWidth,
		state.sourceSelectionDragBackingHeight,
		state.sourceSelectionDragBackingPosition.x,
		state.sourceSelectionDragBackingPosition.y);
	g_system->unlockScreen();
	debugC(3, kDebugWac,
		"Ripper: WAC voice-lock restored transient drag backing position=%d,%d size=%dx%d function=UpdateTransientPresentationOverlay@0x2a1ff",
		state.sourceSelectionDragBackingPosition.x,
		state.sourceSelectionDragBackingPosition.y,
		state.sourceSelectionDragBackingWidth,
		state.sourceSelectionDragBackingHeight);
	state.sourceSelectionDragBackingPixels.clear();
	state.sourceSelectionDragBackingWidth = 0;
	state.sourceSelectionDragBackingHeight = 0;
	return true;
}

bool WacVoiceLockRenderer::drawPresentation(
		const WacVoiceLockPcm &sourcePcm,
		WacVoiceLockEditorState &state) const {
	// UpdateTransientPresentationOverlay at 0x2a1ff restores the saved display
	// rectangle before capturing and drawing at the new position.
	if (!restoreSourceSelectionDragBacking(state))
		return false;
	if (!_database->engine()->getModalDialog()->drawRetainedTitlePanelText(
			_database->resourceString(kWacVoiceLockFileResource),
			_sourcePanel, ModalDialogManager::kWacPresentation, false))
		return false;
	if (_editorAvailable &&
			!_database->engine()->getModalDialog()->drawRetainedTitlePanelText(
				_database->resourceString(kWacVoiceLockEditorResource),
				_editorPanel, ModalDialogManager::kWacPresentation, false))
		return false;

	WacVoiceLockSelection displayedSelection;
	const WacVoiceLockSelection *displayedSelectionPtr = nullptr;
	if (state.sourceSelectionActive || state.selectingSource) {
		displayedSelection = WacVoiceLockSelection(
			MIN(state.sourceSelection.start, state.sourceSelection.end),
			MAX(state.sourceSelection.start, state.sourceSelection.end));
		displayedSelectionPtr = &displayedSelection;
	}
	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen || screen->format.bytesPerPixel != 1) {
		if (screen)
			g_system->unlockScreen();
		return false;
	}
	drawWaveform(*screen, sourcePcm, _sourceWaveform, displayedSelectionPtr);
	if (state.draggingSourceSelection && state.sourceSelectionActive &&
			state.sourceSelectionDragPixels.empty()) {
		const Common::Rect capturedSelection(state.sourceSelection.start,
			_sourceWaveform.top, state.sourceSelection.end,
			_sourceWaveform.bottom);
		state.sourceSelectionDragWidth = capturedSelection.width();
		state.sourceSelectionDragHeight = capturedSelection.height();
		captureRect(*screen, capturedSelection,
			state.sourceSelectionDragPixels);
		debugC(3, kDebugWac,
			"Ripper: WAC voice-lock captured XOR-highlighted drag pixels range=%d..%d size=%dx%d opcode=0x18",
			state.sourceSelection.start, state.sourceSelection.end,
			state.sourceSelectionDragWidth, state.sourceSelectionDragHeight);
	}
	if (state.puzzleHelpEnabled && state.quantized) {
		for (uint range = 0; range < kWacVoiceLockSelectionCount; ++range) {
			const WacVoiceLockSelection solution = wacVoiceLockSolution(range);
			const int nominalStart = CLIP<int>(solution.start,
				_sourceWaveform.left, _sourceWaveform.right - 1);
			const int nominalEnd = CLIP<int>(solution.end,
				nominalStart + 1, _sourceWaveform.right - 1);
			const int acceptedStartMinimum = CLIP<int>(
				nominalStart - (int)kWacVoiceLockSelectionTolerance,
				_sourceWaveform.left, _sourceWaveform.right - 1);
			const int acceptedStartMaximum = CLIP<int>(
				nominalStart + (int)kWacVoiceLockSelectionTolerance,
				_sourceWaveform.left, _sourceWaveform.right - 1);
			const int acceptedEndMinimum = CLIP<int>(
				nominalEnd - (int)kWacVoiceLockSelectionTolerance,
				_sourceWaveform.left, _sourceWaveform.right - 1);
			const int acceptedEndMaximum = CLIP<int>(
				nominalEnd + (int)kWacVoiceLockSelectionTolerance,
				_sourceWaveform.left, _sourceWaveform.right - 1);
			const int lineY = _sourceWaveform.bottom - 3 -
				(range % 2) * 4;
			for (int x = nominalStart; x <= nominalEnd; ++x)
				*((byte *)screen->getBasePtr(x, lineY)) =
					kWacVoiceLockPuzzleHelpColor;
			for (int y = lineY - 1; y <= lineY + 1; ++y) {
				for (int x = acceptedStartMinimum;
						x <= acceptedStartMaximum; ++x)
					*((byte *)screen->getBasePtr(x, y)) =
						kWacVoiceLockPuzzleHelpColor;
				for (int x = acceptedEndMinimum;
						x <= acceptedEndMaximum; ++x)
					*((byte *)screen->getBasePtr(x, y)) =
						kWacVoiceLockPuzzleHelpColor;
			}
			for (int y = lineY - 4; y <= lineY + 2; ++y) {
				*((byte *)screen->getBasePtr(nominalStart, y)) =
					kWacVoiceLockPuzzleHelpColor;
				*((byte *)screen->getBasePtr(nominalEnd, y)) =
					kWacVoiceLockPuzzleHelpColor;
			}
			const Common::String label =
				Common::String::format("%u", range + 1);
			const int labelLeft = nominalStart +
				(nominalEnd - nominalStart -
					_database->measureText(label)) / 2;
			_database->drawText((byte *)screen->getPixels(),
				screen->pitch, labelLeft, lineY - 12, label,
				kWacVoiceLockPuzzleHelpColor);
		}
	}
	if (_editorAvailable) {
		WacVoiceLockPcm assembledPcm;
		assembledPcm.data = state.assembledAudio;
		assembledPcm.sampleRate = sourcePcm.sampleRate;
		assembledPcm.flags = sourcePcm.flags;
		assembledPcm.bytesPerSample = sourcePcm.bytesPerSample;
		drawEditorWaveform(*screen, assembledPcm, _editorWaveform,
			state.editorSegments);
	}
	if (state.playbackProgressActive && state.playbackProgressColumn >= 0 &&
			!state.playbackProgressBounds.isEmpty()) {
		const int x = state.playbackProgressBounds.left +
			MIN<int>(state.playbackProgressColumn,
				state.playbackProgressBounds.width() - 1);
		for (int y = state.playbackProgressBounds.top;
				y < state.playbackProgressBounds.bottom; ++y)
			*((byte *)screen->getBasePtr(x, y)) =
				kWacVoiceLockMarkerColor;
	}
	g_system->unlockScreen();

	const uint buttonCount = _editorAvailable ? 3 : 1;
	for (uint button = 0; button < buttonCount; ++button) {
		const uint frame = button * 2 +
			((int)button == state.pressedButton ? 1 : 0);
		_database->drawBitmap(_buttonAssets[frame],
			_buttonBounds[button].left, _buttonBounds[button].top);
	}
	if (state.draggingSourceSelection && state.sourceSelectionActive) {
		screen = g_system->lockScreen();
		if (!screen || screen->format.bytesPerPixel != 1) {
			if (screen)
				g_system->unlockScreen();
			return false;
		}
		const int destinationLeft = state.sourceSelectionDragPosition.x -
			state.sourceSelectionDragOffsetX;
		const int destinationTop = state.sourceSelectionDragPosition.y -
			state.sourceSelectionDragOffsetY;
		state.sourceSelectionDragBackingWidth =
			state.sourceSelectionDragWidth;
		state.sourceSelectionDragBackingHeight =
			state.sourceSelectionDragHeight;
		state.sourceSelectionDragBackingPosition =
			Common::Point(destinationLeft, destinationTop);
		capturePixels(*screen, state.sourceSelectionDragBackingPixels,
			state.sourceSelectionDragBackingWidth,
			state.sourceSelectionDragBackingHeight,
			destinationLeft, destinationTop);
		blitPixels(*screen, state.sourceSelectionDragPixels,
			state.sourceSelectionDragWidth, state.sourceSelectionDragHeight,
			destinationLeft, destinationTop);
		g_system->unlockScreen();
		debugC(3, kDebugWac,
			"Ripper: WAC voice-lock captured transient drag backing position=%d,%d size=%dx%d function=UpdateTransientPresentationOverlay@0x2a1ff",
			destinationLeft, destinationTop,
			state.sourceSelectionDragBackingWidth,
			state.sourceSelectionDragBackingHeight);
	}
	_database->engine()->getCursor()->refresh();
	g_system->updateScreen();
	return true;
}

} // End of namespace Ripper
