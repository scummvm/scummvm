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

#include "ripper/wac/voice_lock.h"

#include "audio/decoders/raw.h"
#include "audio/mixer.h"
#include "common/debug.h"
#include "common/endian.h"
#include "common/events.h"
#include "common/ptr.h"
#include "common/stream.h"
#include "common/system.h"
#include "common/util.h"
#include "graphics/surface.h"

#include "ripper/cursor.h"
#include "ripper/detection.h"
#include "ripper/input.h"
#include "ripper/media.h"
#include "ripper/milestones.h"
#include "ripper/modal_dialog.h"
#include "ripper/resources.h"
#include "ripper/ripper.h"
#include "ripper/script.h"
#include "ripper/wac/database.h"
#include "ripper/wac/wac.h"

namespace Ripper {

static const uint kWacDefaultCursor = 14;
static const uint kWacControlCursor = 16;
static const int kWacDatabaseLeft = 400;
static const int kWacDatabaseTop = 50;
static const int kWacDatabaseRight = 590;
static const int kWacDatabaseBottom = 332;
static const byte kWacDatabaseBackground = 4;
static const uint16 kWacDatabaseSelectionChanged = 0xfffe;
static const uint kWacVoiceLockFileResource = 0xb1;
static const uint kWacVoiceLockEditorResource = 0xb2;
static const uint kWacVoiceLockSelectionCount = 5;
static const uint kWacVoiceLockSelectionTolerance = 3;
static const byte kWacVoiceLockWaveformColor = 255;
static const byte kWacVoiceLockMarkerColor = 196;
static const uint16 kNoAction = WacManager::kNoAction;
static const uint16 kExitAction = WacManager::kExitAction;

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

static bool loadWacVoiceLockPcm(ResourceManager *resources,
		const Common::String &path, WacVoiceLockPcm &pcm) {
	Common::ScopedPtr<Common::SeekableReadStream> stream(
		resources->createReadStreamForPath(path));
	if (!stream || stream->size() < 12)
		return false;

	const uint32 riff = stream->readUint32BE();
	stream->readUint32LE();
	const uint32 wave = stream->readUint32BE();
	if (riff != MKTAG('R', 'I', 'F', 'F') ||
			wave != MKTAG('W', 'A', 'V', 'E'))
		return false;

	uint16 format = 0;
	uint16 channels = 0;
	uint16 bitsPerSample = 0;
	uint sampleRate = 0;
	Common::Array<byte> data;
	while (stream->pos() + 8 <= stream->size()) {
		const uint32 tag = stream->readUint32BE();
		const uint32 size = stream->readUint32LE();
		const int64 chunkStart = stream->pos();
		if (chunkStart + size > stream->size())
			return false;
		if (tag == MKTAG('f', 'm', 't', ' ') && size >= 16) {
			format = stream->readUint16LE();
			channels = stream->readUint16LE();
			sampleRate = stream->readUint32LE();
			stream->skip(6);
			bitsPerSample = stream->readUint16LE();
		} else if (tag == MKTAG('d', 'a', 't', 'a')) {
			data.resize(size);
			if (size != 0 && stream->read(data.data(), size) != size)
				return false;
		}
		stream->seek(chunkStart + size + (size & 1));
	}

	if (format != 1 || channels != 1 || sampleRate == 0 ||
			(bitsPerSample != 8 && bitsPerSample != 16) || data.empty())
		return false;
	pcm.data = Common::move(data);
	pcm.sampleRate = sampleRate;
	pcm.bytesPerSample = bitsPerSample / 8;
	pcm.flags = bitsPerSample == 8 ? Audio::FLAG_UNSIGNED :
		Audio::FLAG_16BITS | Audio::FLAG_LITTLE_ENDIAN;
	return true;
}

static int wacVoiceLockSample(const WacVoiceLockPcm &pcm, uint sample) {
	const uint offset = sample * pcm.bytesPerSample;
	if (offset + pcm.bytesPerSample > pcm.data.size())
		return 0;
	if (pcm.bytesPerSample == 1)
		return ((int)pcm.data[offset] - 128) << 8;
	return (int16)READ_LE_UINT16(pcm.data.data() + offset);
}

static void drawWacVoiceLockWaveform(Graphics::Surface &screen,
		const WacVoiceLockPcm &pcm, const Common::Rect &bounds,
		const WacVoiceLockSelection *selection) {
	if (bounds.isEmpty())
		return;
	for (int y = bounds.top; y < bounds.bottom; ++y)
		memset(screen.getBasePtr(bounds.left, y), kWacDatabaseBackground,
			bounds.width());
	if (pcm.data.empty() || pcm.bytesPerSample == 0)
		return;

	const int center = (bounds.top + bounds.bottom) / 2;
	const uint sampleCount = pcm.data.size() / pcm.bytesPerSample;
	const int plotWidth = bounds.width();
	const int amplitude = MAX<int>(1, bounds.height() / 2);
	for (int column = 0; column < plotWidth; ++column) {
		const uint firstSample = (uint64)column * sampleCount / plotWidth;
		const uint lastSample = MAX<uint>(firstSample + 1,
			(uint64)(column + 1) * sampleCount / plotWidth);
		int minimum = 0;
		int maximum = 0;
		for (uint sample = firstSample;
				sample < MIN<uint>(lastSample, sampleCount); ++sample) {
			const int value = wacVoiceLockSample(pcm, sample);
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

	if (!selection)
		return;
	const int start = CLIP<int>(selection->start,
		bounds.left, bounds.right - 1);
	const int end = CLIP<int>(selection->end,
		bounds.left, bounds.right - 1);
	for (int y = bounds.top; y < bounds.bottom; ++y) {
		*((byte *)screen.getBasePtr(start, y)) =
			kWacVoiceLockMarkerColor;
		*((byte *)screen.getBasePtr(end, y)) =
			kWacVoiceLockMarkerColor;
	}
}

static uint32 getWacVoiceLockDuration(uint byteCount,
		const WacVoiceLockPcm &pcm) {
	if (pcm.sampleRate == 0 || pcm.bytesPerSample == 0)
		return 0;
	return MAX<uint32>(1, (uint64)byteCount * 1000 /
		(pcm.sampleRate * pcm.bytesPerSample));
}

static void appendWacVoiceLockSelectionAudio(
		const WacVoiceLockPcm &source,
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

static bool validateWacVoiceLockSelections(
		const Common::Array<WacVoiceLockSelection> &selections) {
	static const WacVoiceLockSelection solution[kWacVoiceLockSelectionCount] = {
		WacVoiceLockSelection(240, 252),
		WacVoiceLockSelection(70, 82),
		WacVoiceLockSelection(87, 99),
		WacVoiceLockSelection(171, 184),
		WacVoiceLockSelection(190, 199)
	};
	if (selections.size() != kWacVoiceLockSelectionCount)
		return false;

	bool matched[kWacVoiceLockSelectionCount] = { false, false, false, false, false };
	for (uint selection = 0; selection < selections.size(); ++selection) {
		bool found = false;
		for (uint candidate = 0; candidate < kWacVoiceLockSelectionCount;
				++candidate) {
			if (matched[candidate])
				continue;
			if (ABS(selections[selection].start - solution[candidate].start) <=
					(int)kWacVoiceLockSelectionTolerance &&
					ABS(selections[selection].end - solution[candidate].end) <=
					(int)kWacVoiceLockSelectionTolerance) {
				matched[candidate] = true;
				found = true;
				break;
			}
		}
		if (!found)
			return false;
	}
	return true;
}

WacVoiceLockPuzzle::WacVoiceLockPuzzle(WacDatabaseSession *database) :
		_database(database) {
}

uint16 WacVoiceLockPuzzle::run(byte entryIndex,
		const Common::String &entryLabel) {
	// RunWacVoiceLockPuzzleScene at 0x24ba4 creates a 340-by-116 source
	// waveform panel at physical 50,50. Flag 0x54 adds the equally sized
	// editor panel at 50,176 and Play/Clear/Quantize controls at y=293.
	ResourceManager *resources = _database->engine()->getResources();
	const Common::Rect sourcePanel(50, 50, 390, 166);
	const Common::Rect editorPanel(50, 176, 390, 292);
	// CreateWrappedTextPanelControl at 0x58fb6 stores the client rectangle
	// separately from the 340-by-116 outer control. The title-only WAC layout
	// leaves a 20-pixel heading, 5-pixel side insets and a 6-pixel bottom inset.
	const Common::Rect sourceWaveform(55, 70, 385, 160);
	const Common::Rect editorWaveform(55, 196, 385, 286);
	Common::Array<BitmapAssetFrame> buttonAssets;
	WacVoiceLockPcm sourcePcm;
	if (!resources->loadInterfaceBitmapSet("wacwav", buttonAssets) ||
			buttonAssets.size() < 6 ||
			!loadWacVoiceLockPcm(resources, "voxlok.wav", sourcePcm)) {
		warning("Ripper: could not load WAC voice-lock assets buttons=%u sourceBytes=%u",
			buttonAssets.size(), sourcePcm.data.size());
		return kNoAction;
	}

	const bool editorAvailable = _database->engine()->getMilestones()->isSet(
		kMilestoneWacAudioEditorAvailable);
	Common::Array<WacVoiceLockSelection> selections;
	Common::Array<byte> assembledAudio;
	WacVoiceLockSelection sourceSelection;
	bool quantized = false;
	bool sourceSelectionActive = false;
	bool selectingSource = false;
	bool adjustingSelectionStart = false;
	bool draggingSourceSelection = false;
	bool sourcePanelActive = true;
	int hoveredButton = -1;
	int pressedButton = -1;
	bool validateAfterPlayback = false;
	bool playbackProgressActive = false;
	Common::Rect playbackProgressBounds;
	uint32 playbackDuration = 0;
	int playbackProgressColumn = -1;
	const uint savedCursor =
		_database->engine()->getCursor()->getSelectionIndex();
	Audio::SoundHandle audioHandle;

	Common::Rect buttonBounds[3];
	int buttonLeft = sourcePanel.left;
	const int buttonTop = editorAvailable ? editorPanel.bottom + 1 :
		sourcePanel.bottom + 1;
	for (uint button = 0; button < 3; ++button) {
		const BitmapAssetFrame &bitmap = buttonAssets[button * 2];
		buttonBounds[button] = Common::Rect(buttonLeft, buttonTop,
			buttonLeft + bitmap.width, buttonTop + bitmap.height);
		buttonLeft += bitmap.width;
	}

	auto drawPresentation = [&]() -> bool {
		// Playback and selection redraws replace both title controls and their
		// client waveforms as one presentation. Do not expose the empty framed
		// controls before the waveform pixels and buttons have been restored.
		if (!_database->engine()->getModalDialog()->drawRetainedTitlePanelText(
				_database->resourceString(kWacVoiceLockFileResource),
				sourcePanel, ModalDialogManager::kWacPresentation, false))
			return false;
		if (editorAvailable) {
			if (!_database->engine()->getModalDialog()->drawRetainedTitlePanelText(
					_database->resourceString(kWacVoiceLockEditorResource),
					editorPanel, ModalDialogManager::kWacPresentation, false))
				return false;
		}

		WacVoiceLockSelection displayedSelection;
		const WacVoiceLockSelection *displayedSelectionPtr = nullptr;
		if (sourceSelectionActive || selectingSource) {
			displayedSelection = WacVoiceLockSelection(
				MIN(sourceSelection.start, sourceSelection.end),
				MAX(sourceSelection.start, sourceSelection.end));
			displayedSelectionPtr = &displayedSelection;
		}
		Graphics::Surface *screen = g_system->lockScreen();
		if (!screen || screen->format.bytesPerPixel != 1) {
			if (screen)
				g_system->unlockScreen();
			return false;
		}
		drawWacVoiceLockWaveform(*screen, sourcePcm,
			sourceWaveform, displayedSelectionPtr);
		if (editorAvailable) {
			WacVoiceLockPcm assembledPcm;
			assembledPcm.data = assembledAudio;
			assembledPcm.sampleRate = sourcePcm.sampleRate;
			assembledPcm.flags = sourcePcm.flags;
			assembledPcm.bytesPerSample = sourcePcm.bytesPerSample;
			drawWacVoiceLockWaveform(*screen, assembledPcm,
				editorWaveform, nullptr);
		}
		if (playbackProgressActive && playbackProgressColumn >= 0 &&
				!playbackProgressBounds.isEmpty()) {
			const int x = playbackProgressBounds.left +
				MIN<int>(playbackProgressColumn,
					playbackProgressBounds.width() - 1);
			for (int y = playbackProgressBounds.top;
					y < playbackProgressBounds.bottom; ++y)
				*((byte *)screen->getBasePtr(x, y)) =
					kWacVoiceLockMarkerColor;
		}
		g_system->unlockScreen();

		const uint buttonCount = editorAvailable ? 3 : 1;
		for (uint button = 0; button < buttonCount; ++button) {
			const uint frame = button * 2 +
				((int)button == pressedButton ? 1 : 0);
			_database->drawBitmap(buttonAssets[frame], buttonBounds[button].left,
				buttonBounds[button].top);
		}
		_database->engine()->getCursor()->refresh();
		g_system->updateScreen();
		return true;
	};

	if (!drawPresentation())
		return kNoAction;
	_database->engine()->getInput()->discardMouseTransitions();
	_database->engine()->getCursor()->setSelectionIndex(kWacDefaultCursor);
	_database->engine()->getCursor()->dispatchSelectionIndexChange(
		kWacDefaultCursor);
	debugC(1, kDebugWac,
		"Ripper: entered WAC voice-lock puzzle entry=%u label='%s' function=RunWacVoiceLockPuzzleScene@0x24ba4 editor=%d flag=0x%x sourcePanel=%d,%d,%d,%d editorPanel=%d,%d,%d,%d",
		entryIndex, entryLabel.c_str(), editorAvailable,
		kMilestoneWacAudioEditorAvailable,
		sourcePanel.left, sourcePanel.top, sourcePanel.width(), sourcePanel.height(),
		editorPanel.left, editorPanel.top, editorPanel.width(), editorPanel.height());
	debugC(2, kDebugWac,
		"Ripper: loaded WAC voice-lock source='voxlok.wav' bytes=%u rate=%u flags=0x%x buttons=%u solutionTable=0x215d1 tolerance=%u",
		sourcePcm.data.size(), sourcePcm.sampleRate, sourcePcm.flags,
		buttonAssets.size(), kWacVoiceLockSelectionTolerance);

	uint16 result = kNoAction;
	bool redraw = false;
	bool solved = false;
	while (!_database->engine()->shouldQuit()) {
		MouseState mouse;
		const uint16 command = _database->serviceDatabaseMediaInput(entryIndex,
			nullptr, 0, 0, &mouse, true);
		if (command == kWacDatabaseSelectionChanged || command == kExitAction ||
				command == 0x1b) {
			result = command;
			break;
		}
		if (command == MediaSequenceCallback::kContinueRefreshPalette)
			redraw = true;

		int nextHoveredButton = -1;
		const uint buttonCount = editorAvailable ? 3 : 1;
		for (uint button = 0; button < buttonCount; ++button) {
			if (buttonBounds[button].contains(mouse.position)) {
				nextHoveredButton = button;
				break;
			}
		}
		if (nextHoveredButton != hoveredButton) {
			debugC(3, kDebugWac,
				"Ripper: WAC voice-lock button hover=%d previous=%d point=%d,%d",
				nextHoveredButton, hoveredButton, mouse.position.x,
				mouse.position.y);
			hoveredButton = nextHoveredButton;
			redraw = true;
		}

		const bool sourceWaveformHover = editorAvailable &&
			sourceWaveform.contains(mouse.position);
		const bool editorWaveformHover = editorAvailable &&
			editorWaveform.contains(mouse.position);
		const bool waveformHover =
			sourceWaveformHover || editorWaveformHover;
		const Common::Rect databaseBounds(kWacDatabaseLeft, kWacDatabaseTop,
			kWacDatabaseRight, kWacDatabaseBottom);
		_database->engine()->getCursor()->update(
			databaseBounds.contains(mouse.position) ||
				_database->persistentControlHovered() ||
				waveformHover || hoveredButton >= 0 ?
				kWacControlCursor : kWacDefaultCursor);
		if ((mouse.pressed & kMouseButtonLeft) != 0) {
			pressedButton = hoveredButton;
			if (pressedButton >= 0)
				redraw = true;
			if (sourceWaveformHover) {
				sourcePanelActive = true;
				const int x = CLIP<int>(mouse.position.x,
					sourceWaveform.left, sourceWaveform.right - 1);
				const bool shiftHeld =
					(g_system->getEventManager()->getModifierState() &
						Common::KBD_SHIFT) != 0;
				const int selectionLeft = MIN(sourceSelection.start,
					sourceSelection.end);
				const int selectionRight = MAX(sourceSelection.start,
					sourceSelection.end);
				// RunWacVoiceLockPuzzleScene at 0x2505c checks the BIOS
				// shift state when adjusting the highlighted endpoints. A
				// plain press within the retained highlight starts the
				// transient drag that can be dropped onto the editor.
				if (sourceSelectionActive && !shiftHeld &&
						x >= selectionLeft && x <= selectionRight) {
					draggingSourceSelection = true;
					debugC(2, kDebugWac,
						"Ripper: WAC voice-lock began highlighted selection drag range=%d..%d point=%d,%d",
						selectionLeft, selectionRight,
						mouse.position.x, mouse.position.y);
				} else {
					selectingSource = true;
					if (sourceSelectionActive && shiftHeld) {
						adjustingSelectionStart =
							ABS(x - sourceSelection.start) <=
							ABS(x - sourceSelection.end);
						if (adjustingSelectionStart)
							sourceSelection.start = x;
						else
							sourceSelection.end = x;
					} else {
						sourceSelection.start = x;
						sourceSelection.end = x;
						adjustingSelectionStart = false;
					}
					debugC(3, kDebugWac,
						"Ripper: WAC voice-lock source highlight began x=%d adjustStart=%d shift=%d point=%d,%d",
						x, adjustingSelectionStart, shiftHeld,
						mouse.position.x, mouse.position.y);
				}
				pressedButton = -1;
				redraw = true;
			} else if (editorWaveformHover) {
				sourcePanelActive = false;
				pressedButton = -1;
				debugC(2, kDebugWac,
					"Ripper: WAC voice-lock activated editor waveform bytes=%u point=%d,%d",
					assembledAudio.size(), mouse.position.x, mouse.position.y);
			}
		}
		if (selectingSource && (mouse.buttons & kMouseButtonLeft) != 0) {
			const int nextSelectionEnd = CLIP<int>(mouse.position.x,
				sourceWaveform.left, sourceWaveform.right - 1);
			int &movingEndpoint = adjustingSelectionStart ?
				sourceSelection.start : sourceSelection.end;
			if (nextSelectionEnd != movingEndpoint) {
				movingEndpoint = nextSelectionEnd;
				redraw = true;
			}
		}

		if (selectingSource && (mouse.released & kMouseButtonLeft) != 0) {
			int &movingEndpoint = adjustingSelectionStart ?
				sourceSelection.start : sourceSelection.end;
			movingEndpoint = CLIP<int>(mouse.position.x,
				sourceWaveform.left, sourceWaveform.right - 1);
			selectingSource = false;
			sourceSelection = WacVoiceLockSelection(
				MIN(sourceSelection.start, sourceSelection.end),
				MAX(sourceSelection.start, sourceSelection.end));
			sourceSelectionActive =
				sourceSelection.end - sourceSelection.start >= 2;
			debugC(sourceSelectionActive ? 2 : 3, kDebugWac,
				"Ripper: WAC voice-lock source highlight ended range=%d..%d active=%d",
				sourceSelection.start, sourceSelection.end,
				sourceSelectionActive);
			redraw = true;
		} else if (draggingSourceSelection &&
				(mouse.released & kMouseButtonLeft) != 0) {
			draggingSourceSelection = false;
			// The retail transient overlay is destroyed at 0x252d6 and its
			// audio span is appended only after the editor-control bounds
			// check at 0x25307 succeeds.
			if (sourceSelectionActive &&
					editorPanel.contains(mouse.position)) {
				selections.push_back(sourceSelection);
				appendWacVoiceLockSelectionAudio(sourcePcm,
					sourceSelection, sourceWaveform, assembledAudio);
				sourcePanelActive = false;
				debugC(2, kDebugWac,
					"Ripper: WAC voice-lock dropped selection=%u range=%d..%d into editor assembledBytes=%u quantized=%d point=%d,%d",
					selections.size() - 1, sourceSelection.start,
					sourceSelection.end, assembledAudio.size(), quantized,
					mouse.position.x, mouse.position.y);
			} else {
				debugC(3, kDebugWac,
					"Ripper: WAC voice-lock cancelled highlighted selection drag range=%d..%d point=%d,%d",
					sourceSelection.start, sourceSelection.end,
					mouse.position.x, mouse.position.y);
			}
			redraw = true;
		} else if (!selectingSource && !draggingSourceSelection &&
				(mouse.released & kMouseButtonLeft) != 0 &&
				pressedButton >= 0 && pressedButton == hoveredButton) {
			switch (pressedButton) {
			case 0: {
				bool started = false;
				uint playbackBytes = sourcePcm.data.size();
				playbackProgressBounds = sourceWaveform;
				if (editorAvailable && !sourcePanelActive) {
					if (!assembledAudio.empty())
						started =
							_database->engine()->getMedia()->playRawSoundEffect(
								assembledAudio, sourcePcm.sampleRate,
								sourcePcm.flags, audioHandle);
					playbackBytes = assembledAudio.size();
					playbackProgressBounds = editorWaveform;
					debugC(2, kDebugWac,
						"Ripper: WAC voice-lock played active editor audio bytes=%u selections=%u started=%d",
						assembledAudio.size(), selections.size(), started);
				} else if (sourceSelectionActive) {
					Common::Array<byte> selectedAudio;
					appendWacVoiceLockSelectionAudio(sourcePcm,
						sourceSelection, sourceWaveform, selectedAudio);
					started =
						_database->engine()->getMedia()->playRawSoundEffect(
							selectedAudio, sourcePcm.sampleRate,
							sourcePcm.flags, audioHandle);
					playbackBytes = selectedAudio.size();
					playbackProgressBounds = Common::Rect(
						sourceSelection.start, sourceWaveform.top,
						sourceSelection.end + 1, sourceWaveform.bottom);
					debugC(2, kDebugWac,
						"Ripper: WAC voice-lock played highlighted source range=%d..%d bytes=%u started=%d",
						sourceSelection.start, sourceSelection.end,
						selectedAudio.size(), started);
				} else {
					started = _database->engine()->getMedia()->playSoundEffect(
						quantized ? "voxlok1.wav" : "voxlok.wav",
						audioHandle);
					debugC(2, kDebugWac,
						"Ripper: WAC voice-lock played source audio quantized=%d started=%d",
						quantized, started);
				}
				validateAfterPlayback = started;
				playbackDuration = getWacVoiceLockDuration(playbackBytes,
					sourcePcm);
				playbackProgressActive = started && playbackDuration != 0;
				playbackProgressColumn =
					playbackProgressActive ? 0 : -1;
				redraw = true;
				break;
			}
			case 1:
				_database->engine()->getMedia()->stopSoundEffect(audioHandle);
				validateAfterPlayback = false;
				playbackProgressActive = false;
				playbackProgressColumn = -1;
				selections.clear();
				assembledAudio.clear();
				sourcePanelActive = false;
				debugC(2, kDebugWac,
					"Ripper: WAC voice-lock cleared assembled audio");
				redraw = true;
				break;
			case 2: {
				playbackProgressActive = false;
				playbackProgressColumn = -1;
				WacVoiceLockPcm quantizedPcm;
				if (loadWacVoiceLockPcm(resources, "voxlok1.wav",
						quantizedPcm)) {
					sourcePcm = Common::move(quantizedPcm);
					quantized = true;
					sourceSelectionActive = false;
					selectingSource = false;
					draggingSourceSelection = false;
					sourcePanelActive = true;
					validateAfterPlayback = false;
					_database->engine()->getMedia()->playSoundEffect(
						"wacjrnl.wav", audioHandle);
					debugC(2, kDebugWac,
						"Ripper: WAC voice-lock quantized source='voxlok1.wav' bytes=%u rate=%u retainedDrops=%u assembledBytes=%u",
						sourcePcm.data.size(), sourcePcm.sampleRate,
						selections.size(), assembledAudio.size());
					redraw = true;
				} else {
					warning("Ripper: could not load WAC voice-lock quantized source 'voxlok1.wav'");
				}
				break;
			}
			default:
				break;
			}
			pressedButton = -1;
			redraw = true;
		} else if ((mouse.released & kMouseButtonLeft) != 0) {
			pressedButton = -1;
			redraw = true;
		}

		const bool playbackActive =
			_database->engine()->getMedia()->isSoundEffectActive(audioHandle);
		if (playbackProgressActive && playbackActive) {
			const uint32 elapsed =
				_database->engine()->getMedia()->getSoundEffectElapsedTime(
					audioHandle);
			const int nextProgressColumn = MIN<int>(
				playbackProgressBounds.width() - 1,
				(uint64)elapsed * playbackProgressBounds.width() /
					playbackDuration);
			if (nextProgressColumn != playbackProgressColumn) {
				playbackProgressColumn = nextProgressColumn;
				redraw = true;
				debugC(3, kDebugWac,
					"Ripper: WAC voice-lock playback progress elapsedMs=%u durationMs=%u column=%d",
					elapsed, playbackDuration, playbackProgressColumn);
			}
		}

		if (validateAfterPlayback && !playbackActive) {
			validateAfterPlayback = false;
			playbackProgressActive = false;
			playbackProgressColumn = -1;
			redraw = true;
			Common::String sceneLabel;
			ScriptManager *scripts = _database->engine()->getScripts();
			const uint activeFrame = scripts->getActiveFrame();
			if (activeFrame < scripts->ba0().getFrames().size())
				sceneLabel = scripts->ba0().getString(
					scripts->ba0().getFrames()[activeFrame].labelOffset);
			const bool rangesMatch =
				validateWacVoiceLockSelections(selections);
			debugC(2, kDebugWac,
				"Ripper: WAC voice-lock validation selections=%u rangesMatch=%d scene='%s' expectedScene='eez1'",
				selections.size(), rangesMatch, sceneLabel.c_str());
			if (editorAvailable && quantized && rangesMatch &&
					sceneLabel.equalsIgnoreCase("eez1")) {
				solved = true;
				break;
			}
		}

		if (redraw) {
			if (!drawPresentation())
				break;
			redraw = false;
		}
		g_system->updateScreen();
		g_system->delayMillis(10);
	}

	_database->engine()->getMedia()->stopSoundEffect(audioHandle);
	if (solved) {
		_database->engine()->getCursor()->setVisible(false);
		const bool played =
			_database->engine()->getMedia()->play("accesed.avi", true);
		_database->engine()->getMilestones()->set(
			kMilestoneSecretAnimalLabOpen, true, "wac-voice-lock");
		result = kExitAction;
		debugC(1, kDebugWac,
			"Ripper: solved WAC voice-lock puzzle media='accesed.avi' played=%d milestone=%u selections=%u",
			played, kMilestoneSecretAnimalLabOpen, selections.size());
	}

	_database->clearDatabaseMediaViewport();
	_database->drawDatabase();
	_database->engine()->getCursor()->setSelectionIndex(savedCursor);
	_database->engine()->getCursor()->dispatchSelectionIndexChange(savedCursor);
	_database->engine()->getInput()->discardMouseTransitions();
	debugC(1, kDebugWac,
		"Ripper: left WAC voice-lock puzzle entry=%u result=0x%x editor=%d quantized=%d selections=%u solved=%d",
		entryIndex, result, editorAvailable, quantized,
		selections.size(), solved);
	if (result == kWacDatabaseSelectionChanged &&
			_database->_databaseSelection < _database->_databaseEntries.size())
		return _database->dispatchDatabaseEntry(
			_database->_databaseEntries[_database->_databaseSelection]);
	return result == kExitAction ? kExitAction : kNoAction;
}

} // End of namespace Ripper
