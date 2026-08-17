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
#include "common/events.h"
#include "common/ptr.h"
#include "common/stream.h"
#include "common/system.h"
#include "common/util.h"

#include "ripper/cursor.h"
#include "ripper/detection.h"
#include "ripper/input.h"
#include "ripper/media.h"
#include "ripper/milestones.h"
#include "ripper/resources.h"
#include "ripper/ripper.h"
#include "ripper/script.h"
#include "ripper/wac/database.h"
#include "ripper/wac/wac.h"
#include "ripper/wac/voice_lock_model.h"
#include "ripper/wac/voice_lock_renderer.h"

namespace Ripper {

static const uint kWacDefaultCursor = 14;
static const uint kWacControlCursor = 16;
static const int kWacDatabaseLeft = 400;
static const int kWacDatabaseTop = 50;
static const int kWacDatabaseRight = 590;
static const int kWacDatabaseBottom = 332;
static const uint16 kWacDatabaseSelectionChanged = 0xfffe;
static const int kWacVoiceLockClientLeftInset = 5;
static const int kWacVoiceLockClientTopInset = 20;
static const int kWacVoiceLockClientRightInset = 20;
static const int kWacVoiceLockClientBottomInset = 6;
static const uint16 kNoAction = WacManager::kNoAction;
static const uint16 kExitAction = WacManager::kExitAction;

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
	// separately from the 340-by-116 outer control. The tertiary WAC template
	// metrics installed at 0x11b38 leave a 20-pixel heading, 5-pixel left
	// inset, 20-pixel right inset and 6-pixel bottom inset. The original
	// control record stores axes in the opposite order from Common::Rect;
	// DrawAudioDescriptorWaveform at 0x25b73 confirms that +0x0a/+0x0e are
	// the horizontal origin and width consumed by the waveform.
	const Common::Rect sourceWaveform(
		sourcePanel.left + kWacVoiceLockClientLeftInset,
		sourcePanel.top + kWacVoiceLockClientTopInset,
		sourcePanel.right - kWacVoiceLockClientRightInset,
		sourcePanel.bottom - kWacVoiceLockClientBottomInset);
	const Common::Rect editorWaveform(
		editorPanel.left + kWacVoiceLockClientLeftInset,
		editorPanel.top + kWacVoiceLockClientTopInset,
		editorPanel.right - kWacVoiceLockClientRightInset,
		editorPanel.bottom - kWacVoiceLockClientBottomInset);
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
	WacVoiceLockEditorState state;
	state.puzzleHelpEnabled = _database->engine()->isPuzzleHelpEnabled();
	// Keep local names aligned with RunWacVoiceLockPuzzleScene while placing
	// every mutable editor value in one state object shared with the renderer.
	Common::Array<WacVoiceLockSelection> &selections = state.selections;
	Common::Array<WacVoiceLockEditorSegment> &editorSegments =
		state.editorSegments;
	Common::Array<byte> &assembledAudio = state.assembledAudio;
	WacVoiceLockSelection &sourceSelection = state.sourceSelection;
	bool &quantized = state.quantized;
	bool &sourceSelectionActive = state.sourceSelectionActive;
	bool &selectingSource = state.selectingSource;
	bool &adjustingSelectionStart = state.adjustingSelectionStart;
	bool &draggingSourceSelection = state.draggingSourceSelection;
	Common::Array<byte> &sourceSelectionDragPixels =
		state.sourceSelectionDragPixels;
	int &sourceSelectionDragWidth = state.sourceSelectionDragWidth;
	int &sourceSelectionDragHeight = state.sourceSelectionDragHeight;
	Common::Point &sourceSelectionDragPosition =
		state.sourceSelectionDragPosition;
	int &sourceSelectionDragOffsetX = state.sourceSelectionDragOffsetX;
	int &sourceSelectionDragOffsetY = state.sourceSelectionDragOffsetY;
	bool &sourcePanelActive = state.sourcePanelActive;
	int &hoveredButton = state.hoveredButton;
	int &pressedButton = state.pressedButton;
	bool &validateAfterPlayback = state.validateAfterPlayback;
	bool &playbackProgressActive = state.playbackProgressActive;
	Common::Rect &playbackProgressBounds = state.playbackProgressBounds;
	uint32 &playbackDuration = state.playbackDuration;
	int &playbackProgressColumn = state.playbackProgressColumn;
	bool &puzzleHelpEnabled = state.puzzleHelpEnabled;
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

	WacVoiceLockRenderer renderer(_database, buttonAssets, buttonBounds,
		sourcePanel, editorPanel, sourceWaveform, editorWaveform,
		editorAvailable);

	if (!renderer.drawPresentation(sourcePcm, state))
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
		"Ripper: loaded WAC voice-lock source='voxlok.wav' bytes=%u rate=%u flags=0x%x buttons=%u sourceClient=%d,%d,%dx%d insets=%d,%d,%d,%d solutionTable=0x215d1 tolerance=%u",
		sourcePcm.data.size(), sourcePcm.sampleRate, sourcePcm.flags,
		buttonAssets.size(), sourceWaveform.left, sourceWaveform.top,
		sourceWaveform.width(), sourceWaveform.height(),
		kWacVoiceLockClientLeftInset, kWacVoiceLockClientTopInset,
		kWacVoiceLockClientRightInset, kWacVoiceLockClientBottomInset,
		kWacVoiceLockSelectionTolerance);

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
		const bool nextPuzzleHelpEnabled =
			_database->engine()->isPuzzleHelpEnabled();
		if (nextPuzzleHelpEnabled != puzzleHelpEnabled) {
			puzzleHelpEnabled = nextPuzzleHelpEnabled;
			debugC(2, kDebugWac,
				"Ripper: WAC voice-lock puzzle-help overlay enabled=%d visible=%d quantized=%d ranges=%u endpointTolerance=%u command=PUZZLE_HELP",
				puzzleHelpEnabled, puzzleHelpEnabled && quantized, quantized,
				kWacVoiceLockSelectionCount,
				kWacVoiceLockSelectionTolerance);
			redraw = true;
		}

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
					sourceSelectionDragPixels.clear();
					sourceSelectionDragWidth = 0;
					sourceSelectionDragHeight = 0;
					sourceSelectionDragPosition = mouse.position;
					sourceSelectionDragOffsetX = x - selectionLeft;
					sourceSelectionDragOffsetY =
						mouse.position.y - sourceWaveform.top;
					debugC(2, kDebugWac,
						"Ripper: WAC voice-lock began highlighted selection drag range=%d..%d offset=%d,%d point=%d,%d",
						selectionLeft, selectionRight,
						sourceSelectionDragOffsetX,
						sourceSelectionDragOffsetY,
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
		if (draggingSourceSelection &&
				(mouse.buttons & kMouseButtonLeft) != 0 &&
				sourceSelectionDragPosition != mouse.position) {
			sourceSelectionDragPosition = mouse.position;
			redraw = true;
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
			state.resetSourceDrag();
			// The retail transient overlay is destroyed at 0x252d6 and its
			// audio span is appended only after the editor-control bounds
			// check at 0x25307 succeeds.
			if (sourceSelectionActive &&
					editorPanel.contains(mouse.position)) {
				const uint assembledOffset = assembledAudio.size();
				selections.push_back(sourceSelection);
				appendWacVoiceLockSelectionAudio(sourcePcm,
					sourceSelection, sourceWaveform, assembledAudio);
				const uint appendedSize =
					assembledAudio.size() - assembledOffset;
				if (appendedSize != 0) {
					editorSegments.push_back(WacVoiceLockEditorSegment(
						assembledOffset, appendedSize,
						sourceSelection.end - sourceSelection.start));
				}
				sourcePanelActive = false;
				debugC(2, kDebugWac,
					"Ripper: WAC voice-lock dropped selection=%u range=%d..%d into editor segmentWidth=%d assembledBytes=%u quantized=%d point=%d,%d",
					selections.size() - 1, sourceSelection.start,
					sourceSelection.end,
					sourceSelection.end - sourceSelection.start,
					assembledAudio.size(), quantized,
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
					const int editorWidth =
						getWacVoiceLockEditorWaveformWidth(
							editorSegments, editorWaveform);
					playbackProgressBounds = Common::Rect(
						editorWaveform.left, editorWaveform.top,
						editorWaveform.left + editorWidth,
						editorWaveform.bottom);
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
						sourceSelection.end, sourceWaveform.bottom);
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
				break;
			}
		case 1:
			_database->engine()->getMedia()->stopSoundEffect(audioHandle);
			state.clearEditor();
				debugC(2, kDebugWac,
					"Ripper: WAC voice-lock cleared assembled audio");
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
					state.resetSourceDrag();
					sourcePanelActive = true;
					validateAfterPlayback = false;
					_database->engine()->getMedia()->playSoundEffect(
						"wacjrnl.wav", audioHandle);
					debugC(2, kDebugWac,
						"Ripper: WAC voice-lock quantized source='voxlok1.wav' bytes=%u rate=%u retainedDrops=%u assembledBytes=%u puzzleHelpVisible=%d endpointTolerance=%u",
						sourcePcm.data.size(), sourcePcm.sampleRate,
						selections.size(), assembledAudio.size(),
						puzzleHelpEnabled,
						kWacVoiceLockSelectionTolerance);
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
			Common::String rangeDiagnostics;
			const bool rangesMatch = validateWacVoiceLockSelections(
				selections, rangeDiagnostics);
			const bool sceneMatches =
				sceneLabel.equalsIgnoreCase("eez1");
			Common::String selectionSummary;
			for (uint selection = 0; selection < selections.size();
					++selection) {
				if (!selectionSummary.empty())
					selectionSummary += ",";
				selectionSummary += Common::String::format("%d..%d",
					selections[selection].start, selections[selection].end);
			}
			debugC(2, kDebugWac,
				"Ripper: WAC voice-lock validation gates editor=%d quantized=%d ranges=%d sceneMatch=%d scene='%s' expectedScene='eez1' selections=%u spans=[%s] tolerance=%u diagnostics=[%s]",
				editorAvailable, quantized, rangesMatch, sceneMatches,
				sceneLabel.c_str(), selections.size(),
				selectionSummary.c_str(), kWacVoiceLockSelectionTolerance,
				rangeDiagnostics.c_str());
			if (editorAvailable && quantized && rangesMatch &&
					sceneMatches) {
				debugC(1, kDebugWac,
					"Ripper: WAC voice-lock solve gate passed editor=1 quantized=1 ranges=1 sceneMatch=1 scene='%s' selections=%u spans=[%s]",
					sceneLabel.c_str(), selections.size(),
					selectionSummary.c_str());
				solved = true;
				break;
			}
		}

		if (redraw) {
			if (!renderer.drawPresentation(sourcePcm, state))
				break;
			redraw = false;
		}
		presentScreen();
		g_system->delayMillis(10);
	}

	_database->engine()->getMedia()->stopSoundEffect(audioHandle);
	if (!renderer.restoreSourceSelectionDragBacking(state))
		warning("Ripper: could not restore WAC voice-lock transient drag backing");
	if (solved) {
		_database->engine()->getCursor()->setVisible(false);
		// RunWacVoiceLockPuzzleScene at 0x24ba4 dispatches display command
		// 0x14 to ClearGenericVideoLogicalPage at 0x45ed8 immediately before
		// ACCESED.AVI. The movie owns only the middle 640x300 viewport, so clear
		// the retained 640x400 page first instead of leaving WAC pixels in the
		// uncovered top and bottom bands.
		g_system->fillScreen(0);
		presentScreen();
		debugC(2, kDebugWac,
			"Ripper: cleared WAC voice-lock completion display before media='accesed.avi' bounds=0,0,640,400 command=0x14 function=ClearGenericVideoLogicalPage@0x45ed8");
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
