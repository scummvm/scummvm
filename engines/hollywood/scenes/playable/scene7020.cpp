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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "hollywood/scenes/playable/scene7020.h"

#include "common/file.h"
#include "common/system.h"

#include "hollywood/gameplay/game_state.h"
#include "hollywood/graphics.h"
#include "hollywood/hollywood.h"

namespace Hollywood {

const uint16 kScene7020ExitState9101 = 0x238d;
const uint16 kScene7020ViewportXOffset = 0;
const uint16 kScene7020Chunk6DescriptorCount = 0x21;
const uint16 kScene7020Chunk7DescriptorCount = 10;
const uint kScene7020Chunk6Layer = 0;
const uint kScene7020Chunk7Layer = 0;
const uint32 kScene7020FrameMillis = 75;
const uint32 kScene7020PrimarySpeechFrameMillis = 125;
const uint32 kScene7020OpeningWaitMillis = 2000;
const char *const kScene7020StageArchiveName = "RESOURCE.003";
const uint32 kScene7020Owner0SpeechCueTableOffset = 0x1135;
const int kScene7020SueStartX = 0x50;
const int kScene7020SueStartY = 0x108;
const int kScene7020SueTargetX = 0x13f;
const int kScene7020SueTargetY = 0x153;
const byte kScene7020SueStartFacing = 2;
const byte kScene7020SueFirstTurnFacing = 4;
const byte kScene7020SueDialogueFacing = 1;
const byte kScene7020InvalidPrimarySpeechAnimationGroup = 0xff;
const byte kScene7020Chunk6FrameMap[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
	15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 25, 24, 23, 18,
	26, 27, 28, 29, 30, 31, 18, 32, 18
};
const byte kScene7020Chunk7RevealFrameMap[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 7
};

static PlayableSceneConfig scene7020Config() {
	PlayableSceneConfig config(7020,
		SceneResourceLayout(8, 5, 7),
		SceneViewport(kScene7020ViewportXOffset),
		SceneActorPose(kScene7020SueTargetX, kScene7020SueTargetY, kScene7020SueDialogueFacing));
	config.loadInventoryActionTables = false;
	config.useActorDepthTest = true;
	return config;
}

Scene7020::Scene7020(HollywoodEngine *vm) :
		PlayableScene(vm, scene7020Config()),
		_primaryPoseMode(0),
		_drawChunk7OverlayInsteadOfActor(false),
		_chunk7TimerAccumulator(0),
		_primaryTimerAccumulator(0),
		_backTransientLayers(),
		_actorReplacementLayers() {
}

bool Scene7020::play() {
	_vm->gameplayMusic()->stop();
	if (!load() || !loadOwner0StaticSpeechTables())
		return false;

	initializePreviewState();
	drawPreviewComposite();
	presentFrame();
	runScriptedSequence();

	_speech.stop();
	_soundBank0.stop();
	_ambientSoundBank0.stop();
	if (!Engine::shouldQuit() && !_vm->isSceneRestartRequested())
		_vm->gameState().mainFlowStateId = kScene7020ExitState9101;
	return true;
}

void Scene7020::initializeCustomPreviewState() {
	_primaryPoseMode = 0;
	resetTransientOverlayLayers();
	_drawChunk7OverlayInsteadOfActor = false;
	_chunk7TimerAccumulator = 0;
	_primaryTimerAccumulator = 0;
	setActiveActorPose(kScene7020SueStartX, kScene7020SueStartY, kScene7020SueStartFacing);
	_secondaryActorFrame = 0;
	clearAllSpeechOverlays();
}

void Scene7020::drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) {
	copyBaseFramebufferToSceneFramebuffer();

	drawTransientLayers(_backTransientLayers);

	if (_actorReplacementLayers.visible()) {
		drawTransientLayers(_actorReplacementLayers);
	} else {
		drawActiveAndSecondaryActorFrames(drawActiveActor, activeFacing, activeCel, activeWorldX, activeWorldY,
			drawSecondaryActor, secondaryFacing, secondaryFrame, secondaryWorldX, secondaryWorldY, -1);
	}

	if (actorDrawOrderMode != 2)
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[5], _sceneFramebuffer);
}

bool Scene7020::advanceCustomGameplayLoop(uint32 delta) {
	if (!_ambientSoundBank0.isPlaying())
		_ambientSoundBank0.playSampleLooping(0x0c, 100);

	if (_primaryDialogueSpeechActive) {
		_primaryTimerAccumulator += delta;
		while (_primaryTimerAccumulator >= kScene7020PrimarySpeechFrameMillis) {
			const byte baseFrame = primarySpeechAnimationBaseFrame(_primaryDialogueSpeechGroup);
			byte nextFrame = _primaryDialogueSpeechLastFrame;
			for (uint attempt = 0; attempt < 8 && nextFrame == _primaryDialogueSpeechLastFrame; ++attempt)
				nextFrame = (byte)(baseFrame + _random.getRandomNumber(4));
			if (nextFrame == _primaryDialogueSpeechLastFrame)
				nextFrame = nextFrame >= baseFrame + 4 ? baseFrame : (byte)(nextFrame + 1);
			_primaryDialogueSpeechLastFrame = nextFrame;
			setPrimarySpeechAnimationFrame(_primaryDialogueSpeechGroup, nextFrame);
			_primaryTimerAccumulator -= kScene7020PrimarySpeechFrameMillis;
		}
	} else {
		_primaryTimerAccumulator = 0;
	}

	if (_drawChunk7OverlayInsteadOfActor) {
		_chunk7TimerAccumulator += delta;
		while (_chunk7TimerAccumulator >= kScene7020FrameMillis) {
			setChunk7Frame(chunk7Frame() == 9 ? 7 : chunk7Frame() + 1);
			_chunk7TimerAccumulator -= kScene7020FrameMillis;
		}
	} else {
		_chunk7TimerAccumulator = 0;
	}

	return true;
}

byte Scene7020::primarySpeechAnimationBaseFrame(byte animationGroup) const {
	(void)animationGroup;
	if (_primaryPoseMode == 1)
		return 0x13;
	if (_primaryPoseMode == 2)
		return 0x20;
	return _backTransientLayers.layerFrame(kScene7020Chunk6Layer);
}

void Scene7020::setPrimarySpeechAnimationFrame(byte animationGroup, byte frameIndex) {
	(void)animationGroup;
	if (_primaryPoseMode == 1 || _primaryPoseMode == 2)
		setChunk6Frame(MIN<byte>(frameIndex, ARRAYSIZE(kScene7020Chunk6FrameMap) - 1));
}

bool Scene7020::loadOwner0StaticSpeechTables() {
	Common::File file;
	if (!file.open(Common::Path(kScene7020StageArchiveName))) {
		warning("Failed to open %s owner 0 text", kScene7020StageArchiveName);
		return false;
	}

	if (kScene7020Owner0SpeechCueTableOffset + kSpeechCueDescriptorTableSize + 3 > (uint32)file.size()) {
		warning("%s owner 0 speech cue table is out of range", kScene7020StageArchiveName);
		return false;
	}

	_owner0SpeechCueDescriptors.resize(kSpeechCueDescriptorTableSize);
	file.seek(kScene7020Owner0SpeechCueTableOffset);
	if (file.read(_owner0SpeechCueDescriptors.data(), _owner0SpeechCueDescriptors.size()) !=
			_owner0SpeechCueDescriptors.size()) {
		warning("Failed to read %s owner 0 speech cue table", kScene7020StageArchiveName);
		return false;
	}

	const byte smallRowCount = file.readByte();
	const uint16 largeRowCount = file.readUint16LE();
	const uint32 rowsOffsetEntry = kStage003DecodeKeySize;
	if (rowsOffsetEntry + 4 > (uint32)file.size()) {
		warning("%s owner 0 text row offset is out of range", kScene7020StageArchiveName);
		return false;
	}

	file.seek(rowsOffsetEntry);
	const uint32 rowsOffset = file.readUint32LE();
	const uint32 smallRowBytes = (uint32)smallRowCount * kStage003SmallRowSize;
	const uint32 largeRowBytes = (uint32)largeRowCount * kStage003LargeRowSize;
	if (rowsOffset == 0 || rowsOffset + smallRowBytes + largeRowBytes > (uint32)file.size()) {
		warning("%s owner 0 text rows are out of range", kScene7020StageArchiveName);
		return false;
	}

	_owner0SmallRows.resize((uint32)(smallRowCount + 1) * kStage003SmallRowSize);
	memset(_owner0SmallRows.data(), 0, _owner0SmallRows.size());
	_owner0LargeRows.resize((uint32)(largeRowCount + 1) * kStage003LargeRowSize);
	memset(_owner0LargeRows.data(), 0, _owner0LargeRows.size());

	file.seek(rowsOffset);
	if (file.read(_owner0SmallRows.data() + kStage003SmallRowSize, smallRowBytes) != smallRowBytes ||
			file.read(_owner0LargeRows.data() + kStage003LargeRowSize, largeRowBytes) != largeRowBytes) {
		warning("Failed to read %s owner 0 text rows", kScene7020StageArchiveName);
		return false;
	}

	for (uint row = 1; row <= smallRowCount; ++row) {
		for (uint column = 0; column < kStage003SmallRowSize; ++column)
			_owner0SmallRows[row * kStage003SmallRowSize + column] -= _stage003DecodeKey[column];
	}
	for (uint row = 1; row <= largeRowCount; ++row) {
		for (uint column = 0; column < kStage003LargeRowSize; ++column)
			_owner0LargeRows[row * kStage003LargeRowSize + column] -= _stage003DecodeKey[column];
	}

	return true;
}

bool Scene7020::getOwner0StaticSpeechCue(uint16 rowIndex, byte frameIndex, uint16 &textRecordId,
		byte &continuationCount, uint16 &voiceSampleId) const {
	const uint offset = ((uint)frameIndex + (uint)rowIndex * 10) * 5;
	if (offset + 5 > _owner0SpeechCueDescriptors.size())
		return false;

	textRecordId = readUint16LE(_owner0SpeechCueDescriptors, offset);
	continuationCount = _owner0SpeechCueDescriptors[offset + 2];
	voiceSampleId = readUint16LE(_owner0SpeechCueDescriptors, offset + 3);
	return textRecordId != 0;
}

Common::String Scene7020::getOwner0LargeTextRecord(uint16 recordId) const {
	if (recordId >= kStage003LargeRowBaseIndex)
		return getResource003LargeTextRecord(recordId);

	const uint offset = (uint)recordId * kStage003LargeRowSize;
	if (recordId == 0 || offset >= _owner0LargeRows.size())
		return Common::String();

	const byte *row = _owner0LargeRows.data() + offset;
	uint length = 0;
	while (length < kStage003LargeRowSize && row[length] != 0)
		++length;

	return Common::String((const char *)row, length);
}

void Scene7020::runOwner0SpeechCue(SpeechOverlay &overlay, uint16 textRecordId, byte continuationCount,
		uint16 voiceSampleId, uint16 centerX, uint16 topY, byte colorIndex, bool useRequestedTop,
		bool animatePrimaryDialogue) {
	const byte lineCount = MAX<byte>(1, continuationCount);
	for (byte part = 0; part < lineCount && !Engine::shouldQuit(); ++part) {
		const Common::String text = getOwner0LargeTextRecord(textRecordId + part);
		if (text.empty())
			continue;

		overlay.visible = true;
		overlay.colorIndex = colorIndex;
		wrapActorSpeechText(text, centerX, overlay.lines);
		calculateSpeechOverlayBounds(overlay, centerX, topY, useRequestedTop, _activeActorWorldY);

		const uint16 sampleId = voiceSampleId == 0 ? 0 : voiceSampleId + part;
		const bool started = sampleId != 0 && _speech.playSample(sampleId, 100);
		const uint32 duration = started ? MAX<uint32>(_speech.lastSampleDurationMillis(), 750) :
			MAX<uint32>(1200, overlay.lines.size() * 1100);

		if (animatePrimaryDialogue) {
			_primaryDialogueSpeechActive = true;
			_primaryDialogueSpeechGroup = 0;
			_primaryDialogueSpeechLastFrame = primarySpeechAnimationBaseFrame(0);
			_primaryDialogueSpeechTimerAccumulator = 0;
			setPrimarySpeechAnimationFrame(0, _primaryDialogueSpeechLastFrame);
		}

		const bool interrupted = waitForSpeechOrDelay(duration, false);
		if (_primaryDialogueSpeechActive) {
			setPrimarySpeechAnimationFrame(_primaryDialogueSpeechGroup,
				primarySpeechAnimationBaseFrame(_primaryDialogueSpeechGroup));
			_primaryDialogueSpeechActive = false;
			_primaryDialogueSpeechGroup = kScene7020InvalidPrimarySpeechAnimationGroup;
			_primaryDialogueSpeechTimerAccumulator = 0;
			_primaryDialogueSpeechLastFrame = 7;
		}
		_speech.stop();
		overlay.visible = false;
		overlay.lines.clear();
		if (interrupted)
			break;
	}
}

void Scene7020::beginOwner0SecondarySpeechLine(uint16 rowIndex, byte frameIndex) {
	uint16 textRecordId = 0;
	byte continuationCount = 0;
	uint16 voiceSampleId = 0;
	if (!getOwner0StaticSpeechCue(rowIndex, frameIndex, textRecordId, continuationCount, voiceSampleId))
		return;

	runOwner0SpeechCue(_speechOverlay, textRecordId, continuationCount, voiceSampleId,
		_activeActorWorldX, 0, 0xfd, false, false);
}

void Scene7020::beginOwner0PrimarySpeechLine(uint16 rowIndex, byte frameIndex, uint16 centerX, uint16 topY,
		byte red, byte green, byte blue) {
	const uint paletteOffset = 0xfb * 3;
	if (_paletteCurrent.size() > paletteOffset + 2) {
		_paletteCurrent[paletteOffset] = red;
		_paletteCurrent[paletteOffset + 1] = green;
		_paletteCurrent[paletteOffset + 2] = blue;
	}

	uint16 textRecordId = 0;
	byte continuationCount = 0;
	uint16 voiceSampleId = 0;
	if (!getOwner0StaticSpeechCue(rowIndex, frameIndex, textRecordId, continuationCount, voiceSampleId))
		return;

	runOwner0SpeechCue(_primarySpeechOverlay, textRecordId, continuationCount, voiceSampleId,
		centerX, topY, 0xfb, true, true);
}

void Scene7020::runScriptedSequence() {
	setChunk6Visible(false);
	_primaryPoseMode = 0;
	_drawChunk7OverlayInsteadOfActor = false;
	setChunk7Visible(false);
	runOpeningSueEntryAndIdleWaits();
	if (Engine::shouldQuit())
		return;

	beginSecondarySpeechLine(1, 0);
	_activeActorFacing = kScene7020SueDialogueFacing;
	beginPrimarySpeechLine(2, 0, 0x27f, 0x78, 0x20, 0, 0x3f);
	runChunk6FrameRange(0, 0x13, 1);
	beginPrimarySpeechLine(2, 1, 0x195, 100, 0x20, 0, 0x3f);
	beginSecondarySpeechLine(1, 1);
	runChunk6FrameRange(0x17, 0x1b, 2);
	beginPrimarySpeechLine(2, 2, 0x18b, 100, 0x20, 0, 0x3f);
	runChunk6FrameRange(0x1b, 0x1f, 1);
	beginSecondarySpeechLine(1, 2);
	runChunk7RevealFramesThenHold();
	runChunk6FrameRange(0x17, 0x1b, 2);
	beginPrimarySpeechLine(2, 3, 0x18b, 100, 0x20, 0, 0x3f);
	blackOutScenePalette();

	beginOwner0SecondarySpeechLine(0x51, 1);
	_primaryPoseMode = 3;
	beginOwner0PrimarySpeechLine(0x51, 2, 0, 0xa0, 0x3f, 0, 0);
	beginOwner0SecondarySpeechLine(0x51, 3);
	_primaryPoseMode = 2;
	beginOwner0PrimarySpeechLine(0x51, 4, 0x18b, 100, 0x20, 0, 0x3f);
}

void Scene7020::runOpeningSueEntryAndIdleWaits() {
	runEntryPath(kScene7020SueStartX, kScene7020SueStartY, kScene7020SueStartFacing,
		kScene7020SueTargetX, kScene7020SueTargetY);
	waitSceneMillis(kScene7020OpeningWaitMillis);
	walkActiveActorTo(kScene7020SueTargetX, kScene7020SueTargetY, kScene7020SueFirstTurnFacing, 0);
	waitSceneMillis(kScene7020OpeningWaitMillis);
}

void Scene7020::runChunk6FrameRange(byte firstFrame, byte lastFrame, byte finalPoseMode) {
	setChunk6Visible(true);
	if (_backTransientLayers.hasLayer(kScene7020Chunk6Layer)) {
		playAnimationFrames(_backTransientLayers.layer(kScene7020Chunk6Layer),
			AnimationFrameRange(firstFrame, lastFrame, kScene7020FrameMillis));
	}
	_primaryPoseMode = finalPoseMode;
}

void Scene7020::runChunk7RevealFramesThenHold() {
	setChunk7Visible(true);
	if (_actorReplacementLayers.hasLayer(kScene7020Chunk7Layer))
		playAnimationFrames(_actorReplacementLayers.layer(kScene7020Chunk7Layer),
			AnimationFrameRange(kScene7020Chunk7RevealFrameMap, kScene7020FrameMillis));
	_drawChunk7OverlayInsteadOfActor = true;
	setChunk7Frame(8);
}

void Scene7020::blackOutScenePalette() {
	for (uint i = 0; i <= 0xf9 && i * 3 + 2 < _paletteCurrent.size(); ++i) {
		_paletteCurrent[i * 3] = 0;
		_paletteCurrent[i * 3 + 1] = 0;
		_paletteCurrent[i * 3 + 2] = 0;
	}
	drawPlayableComposite();
	presentFrame();
}

void Scene7020::resetTransientOverlayLayers() {
	_backTransientLayers.clear();
	_backTransientLayers.configureLayer(kScene7020Chunk6Layer, 6, kScene7020Chunk6DescriptorCount,
		kScene7020Chunk6FrameMap, ARRAYSIZE(kScene7020Chunk6FrameMap), false);
	_actorReplacementLayers.clear();
	_actorReplacementLayers.configureLayer(kScene7020Chunk7Layer, 7, kScene7020Chunk7DescriptorCount,
		nullptr, 0, false);
}

void Scene7020::setChunk6Visible(bool visible) {
	_backTransientLayers.setLayerVisible(kScene7020Chunk6Layer, visible);
}

void Scene7020::setChunk6Frame(byte frameMapIndex) {
	if (_backTransientLayers.hasLayer(kScene7020Chunk6Layer))
		_backTransientLayers.setLayerFramePreservingVisibility(kScene7020Chunk6Layer, frameMapIndex);
}

void Scene7020::setChunk7Visible(bool visible) {
	_actorReplacementLayers.setLayerVisible(kScene7020Chunk7Layer, visible);
}

void Scene7020::setChunk7Frame(byte frameIndex) {
	if (_actorReplacementLayers.hasLayer(kScene7020Chunk7Layer))
		_actorReplacementLayers.setLayerFramePreservingVisibility(kScene7020Chunk7Layer, frameIndex);
}

byte Scene7020::chunk7Frame() const {
	if (!_actorReplacementLayers.hasLayer(kScene7020Chunk7Layer))
		return 0;
	return _actorReplacementLayers.layer(kScene7020Chunk7Layer).frameIndex;
}

} // End of namespace Hollywood
