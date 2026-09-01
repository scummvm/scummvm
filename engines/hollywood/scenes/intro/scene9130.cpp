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

#include "hollywood/scenes/intro/scene9130.h"

#include "common/debug.h"

#include "hollywood/gameplay/game_state.h"
#include "hollywood/graphics.h"
#include "hollywood/hollywood.h"
#include "hollywood/scenes/resource_delta_clip_player.h"

namespace Hollywood {

const char *const kScene9130ArchiveName = "RESOURCE.I13";
const char *const kScene9130TextArchiveName = "RESOURCE.003";
const uint kScene9130StageIndex = 912;
const uint16 kScene9130NextState = 0x23d2;
const uint kScene9130ClipChunk = 5;
const uint kScene9130ClipFrameCount = 0xf3;
const uint kScene9130ClipFrameIntervalMillis = 120;
const uint kScene9130SpeechRow = 5;

struct Scene9130SpeechStyle {
	uint16 centerX;
	uint16 topY;
	byte colorIndex;
	byte red;
	byte green;
	byte blue;
};

const Scene9130SpeechStyle kScene9130SpeechStyles[] = {
	{ 0x108, 0xda, 0xfb, 0x3f, 0x3f, 0x3f },
	{ 0x16d, 0xda, 0xfb, 0x3f, 0x28, 0x32 },
	{ 0x16d, 0xda, 0xfb, 0x3f, 0x28, 0x32 },
	{ 0x108, 0xda, 0xfb, 0x3f, 0x3f, 0x3f },
	{ 0x16d, 0xda, 0xfb, 0x3f, 0x28, 0x32 }
};

Scene9130::Scene9130(HollywoodEngine *vm) :
		PresentationScene(vm, "Scene 9130"),
		_music(vm->introMusic()),
		_speech(vm->getLanguage()),
		_text(),
		_paletteResource(),
		_activeTextRecordId(0),
		_activeVoiceSampleId(0),
		_activeContinuationCount(0),
		_activeContinuationIndex(0),
		_activeSpeechStyleIndex(0),
		_activeSpeechCue(false),
		_nextSpeechFrameIndex(0),
		_clipFrameIndex(0) {
	_paletteResource.resize(kPaletteSize);
}

bool Scene9130::play() {
	if (!load())
		return false;

	memset(_sceneFramebuffer.data(), 0, _sceneFramebuffer.size());
	memcpy(_sceneFramebuffer.data(), _savedFramebuffer.data(), _sceneFramebuffer.size());
	drawClipFrame(0);
	presentFrame();

	memcpy(_savedFramebuffer.data(), _sceneFramebuffer.data(), _savedFramebuffer.size());
	memset(_sceneFramebuffer.data(), 0, _sceneFramebuffer.size());
	presentFrame();

	memcpy(_paletteCurrent.data(), _paletteResource.data(), _paletteCurrent.size());
	_music->setVolume(100);
	revealSavedFramebufferWithCurtain();

	if (!_skipRequested && !Engine::shouldQuit())
		runClipAndDialogue();

	clearSubtitle();
	clearSceneFramebufferWithCurtain();
	memset(_paletteCurrent.data(), 0, _paletteCurrent.size());
	presentFrame();
	_speech.stop();

	if (!Engine::shouldQuit())
		_vm->gameState().mainFlowStateId = kScene9130NextState;

	return true;
}

bool Scene9130::load() {
	if (!_resources.loadChunkTable(kScene9130ArchiveName))
		return false;

	for (uint i = 0; i <= kScene9130ClipChunk; ++i) {
		if (!_resources.validateChunk(kScene9130ArchiveName, _debugName, i))
			return false;
	}

	if (!loadFixedChunk(0, _savedFramebuffer, kFrameBufferSize) ||
			!loadFixedChunk(1, _paletteResource, kPaletteSize))
		return false;

	_resources.allocateArena(_resources.chunkTable.sizes[kScene9130ClipChunk]);
	if (!loadArenaChunk(kScene9130ClipChunk))
		return false;

	if (!_text.loadStage(kScene9130TextArchiveName, _debugName, kScene9130StageIndex))
		return false;

	memset(_paletteCurrent.data(), 0, _paletteCurrent.size());
	return true;
}

void Scene9130::runClipAndDialogue() {
	_nextSpeechFrameIndex = 0;
	_clipFrameIndex = 1;
	if (waitForAnimationFrame(kScene9130ClipFrameIntervalMillis, true))
		return;

	AnimationFrameRange range(1, kScene9130ClipFrameCount - 1,
		kScene9130ClipFrameIntervalMillis);
	range.noFinalFrameDelay();
	_animationPlayer.playAndPresent(_clipFrameIndex, range);

	while (_speech.isPlaying() && !_skipRequested && !Engine::shouldQuit()) {
		presentFrame();
		if (delay(10))
			return;
	}
}

void Scene9130::presentAnimationFrame() {
	if ((_clipFrameIndex - 1) % 12 == 0)
		_speech.setVolume(85);
	drawClipFrame(_clipFrameIndex);
	presentFrame();
}

bool Scene9130::waitForAnimationFrame(uint32 millis, bool allowSkip) {
	TimedPresentationLoop loop(*this, millis, 5, allowSkip);
	while (loop.beginFrame()) {
		maybeStartNextSpeechLine();
		loop.finishFrame();
	}

	return animationPlaybackShouldStop();
}

void Scene9130::drawClipFrame(byte frameIndex) {
	ResourceDeltaClipPlayer::drawFrame(_resources.arena, _resources.chunkOffsets[kScene9130ClipChunk],
		_resources.chunkTable.sizes[kScene9130ClipChunk], kScene9130ClipFrameCount,
		frameIndex, _sceneFramebuffer.data(), _sceneFramebuffer.size());
}

void Scene9130::maybeStartNextSpeechLine() {
	if (_speech.isPlaying())
		return;

	if (_activeSpeechCue) {
		if (_activeContinuationIndex < _activeContinuationCount) {
			startCurrentSpeechSegment();
			return;
		}
		_activeSpeechCue = false;
		clearSubtitle();
	}

	if (_nextSpeechFrameIndex >= ARRAYSIZE(kScene9130SpeechStyles))
		return;

	if (prepareSpeechLine(_nextSpeechFrameIndex)) {
		++_nextSpeechFrameIndex;
		startCurrentSpeechSegment();
	} else {
		_nextSpeechFrameIndex = ARRAYSIZE(kScene9130SpeechStyles);
	}
}

bool Scene9130::prepareSpeechLine(byte frameIndex) {
	uint16 textRecordId = 0;
	byte continuationCount = 0;
	uint16 voiceSampleId = 0;
	if (!_text.getStageCue(kScene9130SpeechRow, frameIndex, textRecordId, continuationCount, voiceSampleId))
		return false;

	_activeTextRecordId = textRecordId;
	_activeVoiceSampleId = voiceSampleId;
	_activeContinuationCount = MAX<byte>(1, continuationCount);
	_activeContinuationIndex = 0;
	_activeSpeechStyleIndex = frameIndex;
	_activeSpeechCue = true;
	return true;
}

bool Scene9130::startCurrentSpeechSegment() {
	if (!_activeSpeechCue || _activeContinuationIndex >= _activeContinuationCount)
		return false;

	const byte frameIndex = _activeSpeechStyleIndex;
	const Scene9130SpeechStyle &style = kScene9130SpeechStyles[frameIndex];
	_paletteCurrent[style.colorIndex * 3] = style.red;
	_paletteCurrent[style.colorIndex * 3 + 1] = style.green;
	_paletteCurrent[style.colorIndex * 3 + 2] = style.blue;

	const Common::String text = _text.largeTextRecord(_activeTextRecordId + _activeContinuationIndex);
	showPositionedSubtitle(text, style.colorIndex, style.centerX, style.topY,
		kSpeechOverlayFixedEdgeWrap);

	const uint16 sampleId = _activeVoiceSampleId == 0 ? 0 : _activeVoiceSampleId + _activeContinuationIndex;
	++_activeContinuationIndex;
	_speech.stop();
	if (sampleId != 0)
		_speech.playSample(sampleId, 100);
	return true;
}

void Scene9130::stopAudio() {
	_speech.stop();
	_music->stop();
}

} // End of namespace Hollywood
