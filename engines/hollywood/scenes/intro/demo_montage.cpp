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
 *
 */

#include "hollywood/scenes/intro/demo_montage.h"
#include "common/events.h"
#include "hollywood/hollywood.h"
#include "hollywood/scenes/playable/playable_scene.h"
#include "hollywood/scenes/presentation_scene.h"
#include "hollywood/scenes/shared_frame_sequences.h"

namespace Hollywood {

const uint16 kDemoMontageScenes[] = {3090, 2080, 3100, 2060, 1080, 2010, 5030, 7010, 6073, 4070};

const byte kDemoBlindManFrames[] = {
	0, 1, 2, 3, 0, 4, 5, 6, 7, 8, 9, 10, 11, 12, 11, 10, 9, 8, 7, 6, 5, 4, 0
};
const byte kDemoPumpFrames[] = {24, 25, 26};
const byte kDemoCookFrames[] = {0, 1, 2, 3, 16, 4, 17, 5, 4, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 17, 18};
const byte kDemoSarcophagusFrames[] = {1, 0, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
const byte kDemoGatekeeperFrames[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 14,
	14, 14, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27
};
const byte kDemoPyramidKnockFrames[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0};
const byte kDemoVanessaFrames[] = {
	0, 1, 2, 3, 4, 5, 2, 6, 0, 7, 8, 9, 9, 8, 7, 9,
	10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22
};
const byte kDemoGladysFrames[] = {0, 1, 2, 3, 4, 5, 2, 1, 0, 1, 2, 3, 4, 6, 7, 7, 6, 4, 3, 2, 1, 8, 9, 10, 11, 12};
const byte kDemoDogFrames[] = {0, 1, 2, 3, 4, 5, 6, 5, 7, 8, 9, 10, 11, 12, 13, 21, 9, 8, 7, 0, 14, 15, 16, 17, 18, 19, 20};
const byte kDemoLabNpcFrames[] = {0, 5, 6, 7, 8, 9, 10, 11, 12, 7, 6, 5, 0};
const byte kDemoDraculaFrames[] = {1, 2, 3, 3, 2, 1, 3, 4, 5, 6, 11, 1, 7, 8, 9, 10, 12, 13, 14, 15, 16, 17, 18, 19, 1};

PlayableSceneConfig demoMontageConfig(uint16 sceneId) {
	PlayableSceneConfig config(sceneId / 10 * 10, SceneResourceLayout(5, 5, 39),
			SceneViewport(0, 0, 0), SceneActorPose(0, 0, 0));
	config.setActorResources(sceneId == 7010 ? 0xd0 : 0, sceneId == 7010 ? 0x108 : 0xcc);
	config.setTextResources(0, 0x1135);
	config.loadInventoryActionTables = false;
	config.loadActorDepthTables = sceneId != 7010;
	config.activeAudioChapterIndex = kSceneConfigNoAudioChapter;
	config.musicArchiveName = "RESOURCE.M09";
	config.soundBank0ArchiveName = "RESOURCE.S0D";
	config.walkablePaletteMaxRegion = 20;
	if (sceneId == 3090 || sceneId == 3100)
		config.setActorPathStepDeltas(kActorPathStepDeltaTableSet87);
	uint16 viewport = 0;
	switch (sceneId) {
	case 3090:
		viewport = 152;
		config.defaultActorPose = SceneActorPose(609, 284, 3);
		break;
	case 2080:
		viewport = 208;
		config.defaultActorPose = SceneActorPose(360, 323, 1);
		config.walkablePaletteMaxRegion = 1;
		break;
	case 3100:
		viewport = 40;
		config.defaultActorPose = SceneActorPose(437, 338, 1);
		break;
	case 2060:
		viewport = 104;
		config.defaultActorPose = SceneActorPose(338, 483, 0);
		config.stageIndex = 205;
		config.useActorDepthTest = true;
		config.walkablePaletteMaxRegion = 1;
		break;
	case 1080:
		viewport = 168;
		config.defaultActorPose = SceneActorPose(791, 435, 3);
		config.walkablePaletteMaxRegion = 6;
		break;
	case 2010:
		viewport = 232;
		config.defaultActorPose = SceneActorPose(868, 479, 5);
		config.useActorDepthTest = true;
		break;
	case 5030:
		viewport = 200;
		config.defaultActorPose = SceneActorPose(532, 354, 4);
		break;
	case 7010:
		config.defaultActorPose = SceneActorPose(363, 479, 5);
		break;
	case 6073:
		viewport = 104;
		config.stageIndex = 609;
		config.defaultActorPose = SceneActorPose(290, 336, 2);
		break;
	case 4070:
		viewport = 88;
		config.defaultActorPose = SceneActorPose(84, 460, 4);
		break;
	default:
		break;
	}
	// Montage framing is scripted; walking must not start a gameplay camera pan.
	config.viewportXOffset = config.viewportMinXOffset = config.viewportMaxXOffset = viewport;
	return config;
}

// Reuses the room renderer, actor paths and speech, but never enters the action loop.
class DemoMontageScene : public PlayableScene {
public:
	DemoMontageScene(HollywoodEngine *vm, uint16 sceneId);
	bool play();
	bool skipped() const { return _skipRequested; }

private:
	void setup();
	void runSequence();
	void curtain(bool opening);
	void layer(uint id, uint chunk, uint count, byte frame = 0,
			const byte *map = nullptr, uint mapSize = 0, bool foreground = false);
	void loop(uint id, uint32 millis, byte first, byte last);
	void frames(uint id, byte first, byte last, uint32 millis = 100);
	void primary(uint16 row, byte frame, uint layerId, byte talkBase,
			uint16 x, uint16 y, byte r, byte g, byte b);
	void secondary(uint16 row, byte frame);
	void walk(int x, int y, byte facing);
	void patch(uint chunk);
	void foreground(uint chunk);
	void setupMaze();
	void drawMazeLight(int x, int y);
	void applyFirePalette();
	bool pollEvents(bool allowSkip = true) override;
	bool animationPlaybackShouldStop() const override {
		return _skipRequested || PlayableScene::animationPlaybackShouldStop();
	}
	void advanceCustomGameplayLoop(uint32 delta) override;
	void advanceAmbientAudio(uint32 delta) override {}
	bool shouldApplyGameplayPanelObjectPalette() const override { return false; }
	void drawCustomBackgroundComposite(int x, int y) override;
	void prepareCustomComposite(bool drawActors, byte facing, int x, int y, byte order) override;
	byte paletteRegionAt(int x, int y) const override;
	bool customizeRouteSegment(byte current, byte next, const ActorPathBuildState &state,
			const ScenePoint &boundary, int &facing, bool &restore) override;
	bool customizeRouteFinal(byte current, byte target, const ActorPathBuildState &state,
			int x, int y, int &facing, bool &restore) override;
	void drawCustomActorForegroundComposite(int x, int y, byte order) override;
	void drawCustomForegroundComposite(int x, int y) override;
	void drawCustomComposite(bool drawActor, byte facing, byte cel, int x, int y,
			bool drawSpeaker, byte speakerFacing, byte speakerFrame, int speakerX, int speakerY, byte order) override;
	int alternatePaletteResourceChunkIndex() const override { return _sceneId == 6073 ? 14 : -1; }
	bool isAlternatePaletteResourceActive() const override { return _sceneId == 6073; }
	byte primarySpeechAnimationBaseFrame(byte group) const override { return _talkBase; }
	byte primarySpeechAnimationFrameCount(byte group) const override { return 5; }
	uint32 primarySpeechAnimationFrameMillis(byte group) const override { return 100; }
	void setPrimarySpeechAnimationFrame(byte group, byte frame) override;
	void primarySpeechAnimationRestored(byte group, byte frame) override;

	uint16 _sceneId;
	uint _talkLayer;
	byte _talkBase;
	int _deltaFrame;
	uint32 _paletteCycleMillis;
	uint32 _ambientMillis;
	uint32 _cookWorkMillis;
	bool _cookWorking;
	bool _propAnimationActive;
	bool _speakerPairActive[2];
	byte _speakerPairTicks[2];
	byte _fireShade;
	Common::Array<byte> _firePalette;
	Common::Array<byte> _litMaze;
};

DemoMontageScene::DemoMontageScene(HollywoodEngine *vm, uint16 sceneId) :
		PlayableScene(vm, demoMontageConfig(sceneId)),
		_sceneId(sceneId),
		_talkLayer(SceneLayerStack::kInvalidLayer),
		_talkBase(0),
		_deltaFrame(-1),
		_paletteCycleMillis(0),
		_ambientMillis(0),
		_cookWorkMillis(0),
		_cookWorking(false),
		_propAnimationActive(false),
		_speakerPairActive{false, false},
		_speakerPairTicks{0, 0},
		_fireShade(0) {
}

bool DemoMontageScene::play() {
	if (!load())
		return false;
	initializeDefaultPreviewState();
	setup();
	drawPlayableComposite();
	curtain(true);
	if (!animationPlaybackShouldStop())
		runSequence();
	stopRealtimeSpeech();
	clearAllSpeechOverlays();
	_speech.stop();
	if (!animationPlaybackShouldStop())
		curtain(false);
	_soundBank0.stop();
	stopAmbientSoundCues();
	_residentSoundEffects.stop();
	_vm->gameplayMusic()->setVolume(100);
	return true;
}

bool DemoMontageScene::pollEvents(bool allowSkip) {
	if (animationPlaybackShouldStop())
		return true;
	const bool interrupted = PlayableScene::pollEvents(allowSkip);
	// A presentation skip ends the showcase, not just the current gameplay line.
	_stepAdvanceRequested = false;
	return interrupted;
}

void DemoMontageScene::layer(uint id, uint chunk, uint count, byte frame,
		const byte *map, uint mapSize, bool foreground) {
	_sceneLayers.configureLayer(id, foreground ? kSceneAnimationInFrontOfActors : kSceneAnimationBehindActors,
			chunk, count, map, mapSize, true, frame);
}

void DemoMontageScene::loop(uint id, uint32 millis, byte first, byte last) {
	_realtimeAnimationTracks.addRange(id, millis, first, last, true);
}

void DemoMontageScene::frames(uint id, byte first, byte last, uint32 millis) {
	if (!animationPlaybackShouldStop())
		playAndPresentAnimationFrames(id, AnimationFrameRange(first, last, millis));
}

void DemoMontageScene::primary(uint16 row, byte frame, uint layerId, byte talkBase,
		uint16 x, uint16 y, byte r, byte g, byte b) {
	if (animationPlaybackShouldStop())
		return;
	_talkLayer = layerId;
	_talkBase = talkBase;
	_vm->gameplayMusic()->setVolume(50);
	beginPrimarySpeechLineWithAnimationGroup(row, frame, x, y, r, g, b, 0);
	_vm->gameplayMusic()->setVolume(100);
	_talkLayer = SceneLayerStack::kInvalidLayer;
}

void DemoMontageScene::secondary(uint16 row, byte frame) {
	if (animationPlaybackShouldStop())
		return;
	_vm->gameplayMusic()->setVolume(50);
	beginSecondarySpeechLine(row, frame);
	_vm->gameplayMusic()->setVolume(100);
}

void DemoMontageScene::walk(int x, int y, byte facing) {
	if (!animationPlaybackShouldStop() && !walkActiveActorTo(x, y, facing, 0, true))
		_skipRequested = true;
}

void DemoMontageScene::setPrimarySpeechAnimationFrame(byte group, byte frame) {
	if (_sceneLayers.hasLayer(_talkLayer))
		_sceneLayers.setLayerFrame(_talkLayer, frame);
}

void DemoMontageScene::primarySpeechAnimationRestored(byte group, byte frame) {
	setPrimarySpeechAnimationFrame(group, frame);
}

void DemoMontageScene::patch(uint chunk) {
	if (_sceneChunkTable.isValidChunk(chunk))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[chunk], _baseFramebuffer);
}

void DemoMontageScene::foreground(uint chunk) {
	if (_sceneChunkTable.isValidChunk(chunk))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[chunk], _sceneFramebuffer);
}

void DemoMontageScene::curtain(bool opening) {
	Graphics::ManagedSurface frame;
	frame.copyFrom(_sceneFramebuffer);
	for (int step = 0; step <= 12 && !animationPlaybackShouldStop(); ++step) {
		_sceneFramebuffer.copyFrom(frame);
		const int halfHeight = opening ? step * 20 : 240 - step * 20;
		const int inset = 240 - halfHeight;
		_sceneFramebuffer.fillRect(Common::Rect(0, 0, 1024, inset), 0);
		_sceneFramebuffer.fillRect(Common::Rect(0, 480 - inset, 1024, 480), 0);
		_sceneFramebuffer.fillRect(Common::Rect(_viewportXOffset, 0, _viewportXOffset + inset, 480), 0);
		_sceneFramebuffer.fillRect(Common::Rect(_viewportXOffset + 640 - inset, 0, _viewportXOffset + 640, 480), 0);
		presentFrame();
		if (waitDeltaClipFrameMillis(20))
			break;
	}
}

void DemoMontageScene::advanceCustomGameplayLoop(uint32 delta) {
	if (_sceneId == 3100) {
		_paletteCycleMillis += delta;
		while (_paletteCycleMillis >= 300) {
			_paletteCycleMillis -= 300;
			byte last[3];
			memcpy(last, _paletteCurrent.data() + 159 * 3, 3);
			memmove(_paletteCurrent.data() + 145 * 3, _paletteCurrent.data() + 144 * 3, 15 * 3);
			memcpy(_paletteCurrent.data() + 144 * 3, last, 3);
		}
	}
	if (_cookWorking) {
		_cookWorkMillis += delta;
		while (_cookWorkMillis >= 75) {
			_cookWorkMillis -= 75;
			const byte frame = _sceneLayers.layerFrame(0);
			if (frame == 9)
				_additionalAmbientSoundBank0Slots[0].playSample(40 + _random.getRandomNumber(2), 15);
			if (frame == 18)
				_additionalAmbientSoundBank0Slots[0].stop();
			_sceneLayers.setLayerFrame(0, frame == 18 ? 9 : frame + 1);
		}
	}
	_ambientMillis += delta;
	while (_ambientMillis >= 100) {
		_ambientMillis -= 100;
		switch (_sceneId) {
		case 2080:
		case 5030:
		case 4070:
			_fireShade = _random.getRandomNumber(2);
			break;
		case 1080:
		case 3100: {
			if (_sceneId == 3100 && !_sceneLayers.layerVisible(1))
				break;
			const byte lastFrame = _sceneId == 1080 ? 8 : 15;
			if (_propAnimationActive) {
				const byte frame = _sceneLayers.layerFrame(1);
				if (frame < lastFrame)
					_sceneLayers.setLayerFrame(1, frame + 1);
				else
					_propAnimationActive = false;
			} else if (_random.getRandomNumber(_sceneId == 1080 ? 29 : 14) == 0) {
				_sceneLayers.setLayerFrame(1, 0);
				_propAnimationActive = true;
			}
			break;
		}
		case 7010:
			for (uint pair = 0; pair < 2; ++pair) {
				if (_speakerPairTicks[pair] == 0) {
					const uint nextSpeaker = pair + (_speakerPairActive[pair] ? 2 : 0);
					_sceneLayers.setLayerFrame(nextSpeaker, nextSpeaker * 4);
					_speakerPairActive[pair] = !_speakerPairActive[pair];
					_speakerPairTicks[pair] = 10 + _random.getRandomNumber(24);
				} else {
					const uint active = pair + (_speakerPairActive[pair] ? 0 : 2);
					_sceneLayers.setLayerFrame(active, active * 4 + _random.getRandomNumber(3));
					--_speakerPairTicks[pair];
				}
			}
			break;
		default:
			break;
		}
	}
}

void DemoMontageScene::applyFirePalette() {
	if (_firePalette.empty())
		return;
	const uint colorCount = _sceneId == 2080 ? 176 : 160;
	for (uint i = 0; i < colorCount * 3; ++i)
		_paletteCurrent[i] = MAX<int>(0, _firePalette[i] - _fireShade);
	for (uint i = 208 * 3; i < 233 * 3; ++i)
		_paletteCurrent[i] = MAX<int>(0, _firePalette[i] - _fireShade);
}

void DemoMontageScene::drawCustomBackgroundComposite(int x, int y) {
	if (_sceneId != 2060 || _litMaze.empty())
		return;
	const Common::Rect rect(MAX(0, x - 74), MAX(0, y - 74), MIN(1024, x + 75), MIN(480, y + 75));
	for (int row = rect.top; row < rect.bottom; ++row)
		memcpy(_sceneFramebuffer.getBasePtr(rect.left, row), _litMaze.data() + row * 1024 + rect.left, rect.width());
}

void DemoMontageScene::drawCustomActorForegroundComposite(int x, int y, byte order) {
	switch (_sceneId) {
	case 2060:
		drawMazeLight(x, y);
		break;
	case 3090:
		foreground(y < 331 ? 18 : 17);
		break;
	case 3100:
		if (!_sceneLayers.layerVisible(2))
			foreground(5);
		break;
	case 1080:
		if (y < 308 && x < 455)
			foreground(6);
		if (y < 307 && x > 573)
			foreground(7);
		foreground(5);
		break;
	case 5030:
		foreground(6);
		foreground(7);
		break;
	case 7010:
		if (order == 1) {
			foreground(6);
			foreground(7);
		} else {
			foreground(5);
		}
		break;
	case 4070:
		if (y < 390)
			foreground(14);
		foreground(5);
		break;
	default:
		break;
	}
}

void DemoMontageScene::drawCustomForegroundComposite(int x, int y) {
	applyFirePalette();
}

void DemoMontageScene::drawCustomComposite(bool drawActor, byte facing, byte cel, int x, int y,
		bool drawSpeaker, byte speakerFacing, byte speakerFrame, int speakerX, int speakerY, byte order) {
	if (_sceneId == 2080) {
		copyBaseFramebufferToSceneFramebuffer();
		if (_deltaFrame >= 0) {
			foreground(12);
			drawSceneLayer(0);
			drawSceneLayer(3);
			for (int frame = 0; frame <= _deltaFrame; ++frame)
				drawClipFrameDelta(13, 20, frame);
		} else {
			const bool replacement = _actionOverlayPlayer.isVisible();
			if (replacement)
				foreground(x < 589 ? 11 : 12);
			drawSceneLayer(1);
			drawSceneLayer(2);
			drawSceneLayer(0);
			if (replacement) {
				updateActorPaletteForWorldPoint(x, y);
				drawActionOverlayLayer();
			} else {
				drawActiveAndSecondaryActorFrames(drawActor, facing, cel, x, y, drawSpeaker,
						speakerFacing, speakerFrame, speakerX, speakerY, -1);
				if (!_sceneLayers.layerVisible(2))
					foreground(x < 589 ? 11 : 12);
			}
		}
		applyFirePalette();
		return;
	}
	if (_sceneId != 5030) {
		PlayableScene::drawCustomComposite(drawActor, facing, cel, x, y, drawSpeaker,
				speakerFacing, speakerFrame, speakerX, speakerY, order);
		return;
	}
	copyBaseFramebufferToSceneFramebuffer();
	const bool replacement = _sceneLayers.layerVisible(3);
	if (replacement) {
		updateActorPaletteForWorldPoint(x, y);
		drawSceneLayer(3);
	} else {
		drawActiveAndSecondaryActorFrames(drawActor, facing, cel, x, y, drawSpeaker,
				speakerFacing, speakerFrame, speakerX, speakerY, -1);
	}
	drawSceneLayer(2);
	drawSceneLayer(1);
	drawSceneLayer(0);
	if (!replacement) {
		foreground(6);
		foreground(7);
	}
	applyFirePalette();
}

byte DemoMontageScene::paletteRegionAt(int x, int y) const {
	if (_sceneId == 2060 && y == 483)
		return x == 338 ? 6 : 7;
	if (_sceneId == 2010 && x == 304 && y == 261)
		return 1;
	return PlayableScene::paletteRegionAt(x, y);
}

bool DemoMontageScene::customizeRouteSegment(byte current, byte next, const ActorPathBuildState &state,
		const ScenePoint &boundary, int &facing, bool &restore) {
	if (_sceneId != 2010)
		return false;
	uint destination = 0;
	uint source = 0;
	if (current == 5 && next == 4) {
		destination = 12;
		facing = 1;
	} else if ((current == 6 && next == 4) || (current == 3 && next == 2)) {
		destination = 60;
		facing = 5;
	} else if (current == 3 && next == 4) {
		destination = 24;
		source = 36;
		facing = 2;
	} else {
		return false;
	}
	for (uint i = 0; i < 12; ++i)
		_actorPathStepDeltas[destination + i] = kActorPathStepDeltaTableSetB4[source + i];
	restore = true;
	return true;
}

bool DemoMontageScene::customizeRouteFinal(byte current, byte target, const ActorPathBuildState &state,
		int x, int y, int &facing, bool &restore) {
	if (_sceneId != 2010 || (current != 5 && current != 6))
		return false;
	facing = current == 5 ? 4 : 2;
	for (uint i = 0; i < 12; ++i)
		_actorPathStepDeltas[facing * 12 + i] = kActorPathStepDeltaTableSetB4[36 + i];
	restore = true;
	return true;
}

void DemoMontageScene::prepareCustomComposite(bool drawActors, byte facing, int x, int y, byte order) {
	if (_sceneId != 2060 || _drawActorDepthYThresholds.size() < 2)
		return;
	_drawActorDepthYThresholds = _actorDepthYThresholds;
	bool inFront = false;
	switch (order) {
	case 1:
		inFront = true;
		break;
	case 2:
		inFront = y > 89;
		break;
	case 3:
		inFront = y > 88;
		break;
	case 4:
		inFront = x > 217;
		break;
	case 5:
		inFront = x < 625;
		break;
	case 6:
		inFront = y < 391;
		break;
	case 7:
		inFront = y < 392;
		break;
	default:
		return;
	}
	_drawActorDepthYThresholds[1] = inFront ? 0 : 0xffff;
}

void DemoMontageScene::setupMaze() {
	const byte passages[] = {11, 6, 13, 8, 9, 16};
	for (uint i = 0; i < ARRAYSIZE(passages); ++i)
		patch(passages[i]);
	_litMaze.resize(kSceneBufferByteCount);
	memcpy(_litMaze.data(), _baseFramebuffer.getPixels(), _litMaze.size());
	byte *pixels = (byte *)_baseFramebuffer.getPixels();
	for (uint i = 0; i < _litMaze.size(); ++i) {
		for (uint step = 0; step < 4; ++step)
			pixels[i] = _presentationPaletteRemapTable[pixels[i]];
	}

	const byte pathSteps[] = {8, 8, 7, 5, 10, 6, 10, 10, 4, 6, 4, 10};
	for (uint i = 0; i < _actorPathStepDeltas.size(); ++i)
		_actorPathStepDeltas[i] = pathSteps[i % ARRAYSIZE(pathSteps)];
	for (uint facing = 0; facing < 6; ++facing) {
		const uint chunk = 25 + facing;
		const uint size = MIN<uint>(_sceneChunkTable.sizes[chunk], kActiveActorFacingRunStride);
		memset(_activeActorRunStreams.data() + facing * kActiveActorFacingRunStride, 0, kActiveActorFacingRunStride);
		memcpy(_activeActorRunStreams.data() + facing * kActiveActorFacingRunStride,
				_resourceArena.data() + _resourceChunkOffsets[chunk], size);
	}
	const byte *descriptors = _resourceArena.data() + _resourceChunkOffsets[31];
	const uint count = MIN<uint>(_activeActorDescriptors.size(), _sceneChunkTable.sizes[31] / kActiveActorDescriptorSize);
	for (uint i = 0; i < count; ++i) {
		const byte *descriptor = descriptors + i * kActiveActorDescriptorSize;
		_activeActorDescriptors[i].runStreamOffset = READ_LE_UINT32(descriptor);
		_activeActorDescriptors[i].opaqueRunCount = READ_LE_UINT32(descriptor + 4);
		_activeActorDescriptors[i].paletteRunCount = READ_LE_UINT32(descriptor + 8);
		_activeActorDescriptors[i].anchorX = (int16)READ_LE_UINT16(descriptor + 12);
		_activeActorDescriptors[i].anchorY = (int16)READ_LE_UINT16(descriptor + 16);
		_activeActorDescriptors[i].width = READ_LE_UINT16(descriptor + 20);
		_activeActorDescriptors[i].height = READ_LE_UINT16(descriptor + 24);
	}
}

void DemoMontageScene::drawMazeLight(int x, int y) {
	if (_litMaze.empty() || _sceneChunkTable.sizes[17] < 149 * 149)
		return;
	const byte *mask = _resourceArena.data() + _resourceChunkOffsets[17];
	for (int my = 0; my < 149; ++my) {
		const int py = y - 74 + my;
		if (py < 0 || py >= 480)
			continue;
		for (int mx = 0; mx < 149; ++mx) {
			const int px = x - 74 + mx;
			if (px < 0 || px >= 1024)
				continue;
			const byte depth = _colorToActorDepthClassMap[*(const byte *)_savedFramebuffer.getBasePtr(px, py)];
			const bool behind = depth < _drawActorDepthYThresholds.size() && _drawActorDepthYThresholds[depth] < y;
			const byte shade = mask[my * 149 + mx];
			if ((shade == 0 && behind) || (shade > 3 && shade != 208))
				continue;
			const uint steps = shade <= 3 && behind ? shade : 4;
			byte color = _litMaze[py * 1024 + px];
			for (uint step = 0; step < steps; ++step)
				color = _presentationPaletteRemapTable[color];
			*(byte *)_sceneFramebuffer.getBasePtr(px, py) = color;
		}
	}
}

void DemoMontageScene::setup() {
	if (_sceneId == 2080 || _sceneId == 5030 || _sceneId == 4070)
		_firePalette = _paletteCurrent;
	switch (_sceneId) {
	case 3090:
		layer(0, 9, 26);
		loop(0, 100, 0, 25);
		layer(1, 12, 38, 0, kDemoPumpFrames, ARRAYSIZE(kDemoPumpFrames));
		layer(2, 11, 36, 7, kDemoBlindManFrames, ARRAYSIZE(kDemoBlindManFrames));
		patch(6);
		_ambientSoundBank0.playSampleLooping(14);
		break;
	case 2080:
		layer(0, 10, 26);
		loop(0, 100, 0, 25);
		layer(1, 5, 16, 11, kDemoSarcophagusFrames, ARRAYSIZE(kDemoSarcophagusFrames));
		break;
	case 3100:
		layer(0, 6, 22, 6);
		layer(1, 12, 16, 15);
		_sceneLayers.setLayerVisible(1, false);
		patch(10);
		_ambientSoundBank0.playSampleLooping(38, 5);
		break;
	case 2060:
		setupMaze();
		break;
	case 1080:
		for (uint i = 0; i < _walkablePaletteMask.size(); ++i) {
			if (_walkablePaletteMask[i] == 2 || _walkablePaletteMask[i] == 4 || _walkablePaletteMask[i] == 6)
				_walkablePaletteMask[i] = 0;
			if (_fullPaletteRegionMask[i] == 6)
				_fullPaletteRegionMask[i] = 0;
		}
		layer(0, 8, 19, 9, kDemoCookFrames, ARRAYSIZE(kDemoCookFrames));
		_cookWorking = true;
		layer(1, 11, 5, 0, kFiveFramePingPongFrames, kFiveFramePingPongFrameCount, true);
		break;
	case 2010:
		for (uint i = 0; i < _walkablePaletteMask.size(); ++i) {
			if (_walkablePaletteMask[i] != 1 && _walkablePaletteMask[i] != 4)
				_walkablePaletteMask[i] = 0;
		}
		layer(0, 7, 28, 0, kDemoGatekeeperFrames, ARRAYSIZE(kDemoGatekeeperFrames));
		_sceneLayers.setLayerVisible(0, false);
		break;
	case 5030:
		layer(0, 8, 26);
		loop(0, 100, 0, 25);
		layer(1, 9, 23, 15, kDemoVanessaFrames, ARRAYSIZE(kDemoVanessaFrames));
		layer(2, 10, 13, 21, kDemoGladysFrames, ARRAYSIZE(kDemoGladysFrames));
		_ambientSoundBank0.playSampleLooping(14);
		_additionalAmbientSoundBank0Slots[0].playSampleLooping(39, 50);
		break;
	case 7010:
		for (uint i = 0; i < 4; ++i)
			layer(i, 10, 16, i * 4);
		layer(4, 8, 22, 0, kDemoDogFrames, ARRAYSIZE(kDemoDogFrames), true);
		layer(5, 9, 2, 0, nullptr, 0, true);
		break;
	case 6073:
		layer(0, 13, 5, 0, nullptr, 0, true);
		layer(1, 12, 20, 0, kDualPoseSpeakerFrames, kDualPoseSpeakerFrameCount, true);
		layer(2, 11, 13, 0, kDemoLabNpcFrames, ARRAYSIZE(kDemoLabNpcFrames), true);
		break;
	case 4070:
		patch(7);
		layer(0, 11, 2);
		_realtimeAnimationTracks.addRandom(0, 100, 0, 1, false);
		layer(1, 12, 26);
		loop(1, 100, 0, 25);
		layer(2, 9, 20, 11, kDemoDraculaFrames, ARRAYSIZE(kDemoDraculaFrames));
		_sceneLayers.setLayerVisible(2, false);
		_ambientSoundBank0.playSampleLooping(14);
		break;
	default:
		break;
	}
}

void DemoMontageScene::runSequence() {
	switch (_sceneId) {
	case 3090:
		walk(437, 367, 5);
		for (byte frame = 8; frame <= 22 && !animationPlaybackShouldStop(); ++frame) {
			_sceneLayers.setLayerFrame(2, frame);
			if (frame >= 13 && frame <= 15)
				_sceneLayers.setLayerFrame(1, (frame - 12) % 3);
			if (waitSceneMillis(100))
				break;
		}
		_sceneLayers.setLayerFrame(2, 0);
		primary(99, 1, 2, 0, 340, 172, 44, 63, 19);
		secondary(98, 3);
		primary(99, 4, 2, 0, 340, 172, 44, 63, 19);
		break;
	case 2080: {
		primary(99, 12, 1, 11, 425, 105, 32, 48, 63);
		_sceneLayers.setLayerVisible(1, false);
		layer(2, 6, 33);
		for (byte frame = 0; frame < 33 && !animationPlaybackShouldStop(); ++frame) {
			_sceneLayers.setLayerFrame(2, frame);
			if (frame == 6 || frame == 12 || frame == 19 || frame == 24 || frame == 30)
				playResidentSoundEffect(10);
			if (waitSceneMillis(75))
				break;
		}
		_sceneLayers.setLayerVisible(2, false);
		walk(360, 323, 5);
		waitSceneMillis(1000);
		walk(383, 305, 5);
		const byte exitFrames[] = {0, 1, 2, 3, 4, 5, 4, 5, 4, 3, 2, 1, 0};
		if (!animationPlaybackShouldStop())
			runActorReplacement(ActionOverlaySpec(7, 6, exitFrames, ARRAYSIZE(exitFrames), 100).soundAt(6, 34));
		if (animationPlaybackShouldStop())
			break;
		_hideActiveActor = true;
		layer(3, 7, 6);
		_soundBank0.playSample(35);
		for (_deltaFrame = 0; _deltaFrame < 20 && !animationPlaybackShouldStop(); ++_deltaFrame) {
			if (waitSceneMillis(100))
				break;
		}
		_deltaFrame = 19;
		break;
	}
	case 3100: {
		primary(99, 6, 0, 0, 511, 182, 63, 32, 63);
		secondary(98, 11);
		const byte ronFrames[] = {0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
		const byte cabinFrames[] = {17, 17, 18, 19, 20, 21};
		layer(2, 8, 11, 0, ronFrames, ARRAYSIZE(ronFrames));
		_sceneLayers.setLayerStratum(2, kSceneAnimationActorReplacement);
		uint32 elapsed = 0;
		byte previousRonFrame = 0;
		byte previousCabinFrame = 0;
		while (elapsed <= 1100 && !animationPlaybackShouldStop()) {
			const byte ronFrame = MIN<uint32>(elapsed / 75, 11);
			const byte cabinFrame = elapsed < 450 ? 0 : MIN<uint32>((elapsed - 450) / 100, 5);
			_sceneLayers.setLayerFrame(2, ronFrame);
			if (elapsed >= 450)
				_sceneLayers.setLayerFrame(0, cabinFrames[cabinFrame]);
			if (ronFrame == 5 && previousRonFrame != 5)
				_soundBank0.playSample(37);
			if (cabinFrame == 5 && previousCabinFrame != 5)
				_soundBank0.playSample(36);
			previousRonFrame = ronFrame;
			previousCabinFrame = cabinFrame;
			if (waitSceneMillis(25))
				break;
			elapsed += 25;
		}
		_sceneLayers.setLayerVisible(2, false);
		_sceneLayers.setLayerVisible(0, false);
		_sceneLayers.setLayerVisible(1, true);
		patch(9);
		secondary(98, 12);
		break;
	}
	case 2060:
		walk(338, 364, 0xff);
		walk(739, 240, 0xff);
		break;
	case 1080:
		secondary(0, 0);
		walk(498, 326, 1);
		_cookWorking = false;
		frames(0, _sceneLayers.layerFrame(0), 18, 75);
		_additionalAmbientSoundBank0Slots[0].stop();
		_sceneLayers.setLayerFrame(0, 0);
		primary(99, 0, 0, 0, 558, 132, 13, 50, 58);
		break;
	case 2010:
		walk(540, 294, 5);
		layer(1, 5, 10, 0, kDemoPyramidKnockFrames, ARRAYSIZE(kDemoPyramidKnockFrames));
		_sceneLayers.setLayerStratum(1, kSceneAnimationActorReplacement);
		for (byte frame = 0; frame < 19 && !animationPlaybackShouldStop(); ++frame) {
			_sceneLayers.setLayerFrame(1, frame);
			if (frame == 9)
				_soundBank0.playSample(30);
			if (waitSceneMillis(50))
				break;
		}
		_sceneLayers.setLayerVisible(1, false);
		while (_soundBank0.isPlaying() && !animationPlaybackShouldStop()) {
			if (waitSceneMillis(10))
				break;
		}
		_soundBank0.playSample(31);
		layer(1, 6, 16);
		frames(1, 0, 15);
		while (_soundBank0.isPlaying() && !animationPlaybackShouldStop()) {
			if (waitSceneMillis(10))
				break;
		}
		patch(8);
		_sceneLayers.setLayerVisible(1, false);
		_sceneLayers.setLayerVisible(0, true);
		frames(0, 0, 21);
		primary(3, 0, 0, 21, 450, 120, 0, 63, 63);
		secondary(3, 1);
		primary(3, 4, 0, 21, 450, 120, 0, 63, 63);
		frames(0, 26, 31);
		walk(304, 261, 0xff);
		break;
	case 5030: {
		primary(96, 4, 2, 21, 628, 283, 63, 32, 0);
		if (animationPlaybackShouldStop())
			break;
		layer(3, 13, 11, 5);
		_sceneLayers.setLayerStratum(3, kSceneAnimationActorReplacement);
		_talkLayer = 1;
		_talkBase = 15;
		_vm->gameplayMusic()->setVolume(50);
		startRealtimePrimarySpeechLine(97, 2, 441, 292, 0, 32, 63, 0, 0);
		const byte turnFrames[] = {5, 5, 0};
		for (uint i = 0; i < ARRAYSIZE(turnFrames) && !animationPlaybackShouldStop(); ++i) {
			_sceneLayers.setLayerFrame(3, turnFrames[i]);
			if (waitSceneMillis(75))
				break;
		}
		waitForRealtimeSpeech();
		_vm->gameplayMusic()->setVolume(100);
		_talkLayer = SceneLayerStack::kInvalidLayer;
		break;
	}
	case 7010:
		frames(4, 7, 11);
		primary(8, 0, 4, 10, 250, 310, 51, 34, 57);
		primary(8, 3, 4, 10, 250, 310, 51, 34, 57);
		frames(4, 15, 19);
		_sceneLayers.setLayerFrame(4, 0);
		walk(363, 479, 3);
		secondary(7, 2);
		break;
	case 6073:
		frames(1, 4, 8, 75);
		primary(15, 0, 1, 8, 380, 168, 32, 50, 0);
		frames(1, 12, 16, 75);
		_sceneLayers.setLayerFrame(1, 17);
		loop(1, 125, 17, 24);
		frames(2, 1, 4, 75);
		primary(15, 1, 2, 4, 456, 150, 40, 22, 11);
		frames(2, 9, 12, 75);
		_sceneLayers.setLayerFrame(2, 0);
		break;
	case 4070:
		layer(3, 8, 31);
		_soundBank0.playSample(11);
		for (byte frame = 0; frame < 31 && !animationPlaybackShouldStop(); ++frame) {
			_sceneLayers.setLayerFrame(3, frame);
			if (frame == 17)
				_soundBank0.playSample(12);
			if (frame == 21 || frame == 24)
				playResidentSoundEffect(4);
			if (frame == 28)
				_soundBank0.playSample(13);
			if (waitSceneMillis(100))
				break;
		}
		_sceneLayers.setLayerVisible(3, false);
		_sceneLayers.setLayerVisible(2, true);
		primary(18, 0, 2, 11, 455, 167, 10, 25, 63);
		frames(2, 0, 3);
		primary(18, 1, 2, 6, 412, 159, 10, 25, 63);
		frames(2, 3, 0);
		primary(99, 3, 2, 11, 455, 167, 10, 25, 63);
		break;
	default:
		break;
	}
}

class DemoEndCard : public PresentationScene {
public:
	DemoEndCard(HollywoodEngine *vm) :
			PresentationScene(vm, "demo end card"),
			_random("demo_end_card") {
		_sound.setArchive(Common::Path("RESOURCE.S0D"));
	}
	bool play();

private:
	bool pollEvents(bool allowSkip = true) override;
	void stopAudio() override { _sound.stop(); }
	void fadeIn(uint firstColor, uint lastColor, const Common::Array<byte> &palette);
	SoundBank0Player _sound;
	Common::RandomSource _random;
};

bool DemoEndCard::pollEvents(bool allowSkip) {
	Common::Event event;
	while (g_system->getEventManager()->pollEvent(event)) {
		if (event.type == Common::EVENT_QUIT || event.type == Common::EVENT_RETURN_TO_LAUNCHER) {
			Engine::quitGame();
			return true;
		}
		if (allowSkip && ((event.type == Common::EVENT_KEYDOWN && !event.kbdRepeat) ||
								 event.type == Common::EVENT_LBUTTONDOWN || event.type == Common::EVENT_RBUTTONDOWN)) {
			_skipRequested = true;
			return true;
		}
	}
	return false;
}

void DemoEndCard::fadeIn(uint firstColor, uint lastColor, const Common::Array<byte> &palette) {
	for (int threshold = 63; threshold > 0 && !_skipRequested && !Engine::shouldQuit(); --threshold) {
		for (uint i = firstColor * 3; i < (lastColor + 1) * 3; ++i) {
			if (palette[i] >= threshold)
				++_paletteCurrent[i];
		}
		presentFrame();
		if (delay(20))
			break;
	}
}

bool DemoEndCard::play() {
	Common::Array<byte> palette;
	palette.resize(kPaletteSize);
	if (!_resources.loadChunkTable("RESOURCE.I00") ||
			!loadFixedChunk(8, _sceneFramebuffer, kSceneBufferByteCount) ||
			!loadFixedChunk(9, palette, kPaletteSize))
		return false;
	memset(_paletteCurrent.data(), 0, _paletteCurrent.size());
	fadeIn(1, 199, palette);
	fadeIn(200, 255, palette);
	byte remainingSteps = 3;
	byte brightness = 3;
	bool rising = false;
	uint32 afterMusicMillis = 0;
	_sound.playSample(43, 50);
	while (!_skipRequested && !Engine::shouldQuit()) {
		if ((rising && brightness == 7) || (!rising && brightness == 0))
			remainingSteps = 0;
		if (remainingSteps == 0) {
			remainingSteps = _random.getRandomNumber(7);
			rising = !rising;
			_sound.playSample(43, 50);
		} else {
			for (uint i = 213 * 3; i < 256 * 3; ++i)
				_paletteCurrent[i] = CLIP<int>(_paletteCurrent[i] + (rising ? 1 : -1), 0, 63);
			brightness += rising ? 1 : -1;
			--remainingSteps;
		}
		presentFrame();
		if (delay(40))
			break;
		if (!_vm->gameplayMusic()->isPlaying()) {
			afterMusicMillis += 40;
			if (afterMusicMillis >= 20000)
				break;
		}
	}
	_sound.stop();
	if (_skipRequested) {
		memset(_paletteCurrent.data(), 0, _paletteCurrent.size());
		presentFrame();
		return true;
	}
	for (uint step = 0; step < 63 && !Engine::shouldQuit(); ++step) {
		for (uint i = 0; i < _paletteCurrent.size(); ++i) {
			if (_paletteCurrent[i])
				--_paletteCurrent[i];
		}
		presentFrame();
		if (delay(20, false))
			break;
	}
	return true;
}

bool playSpanishDemoEnding(HollywoodEngine *vm, bool endCardOnly) {
	if (!vm->isDemo() || vm->getLanguage() != Common::ES_ESP)
		return false;
	const GameplayState previousState = vm->gameState();
	const Common::Path previousMusicArchive = vm->gameplayMusic()->archiveName();
	const bool previousCanSave = vm->canSave();
	const bool cursorVisible = vm->cursor()->isInteractiveMode();
	vm->setCanSave(false);
	vm->cursor()->leaveInteractiveMode();
	// The showcase may render actors, but must not expose a saveable gameplay state.
	vm->gameState().mainFlowStateId = 0;
	vm->gameplayMusic()->stop();
	vm->gameplayMusic()->setArchive(Common::Path("RESOURCE.M09"));
	// Direct end-card playback needs the music normally carried over from the montage.
	vm->gameplayMusic()->playMusicCue(12);
	bool result = true;
	if (!endCardOnly) {
		for (uint i = 0; i < ARRAYSIZE(kDemoMontageScenes) && !Engine::shouldQuit(); ++i) {
			DemoMontageScene scene(vm, kDemoMontageScenes[i]);
			if (!scene.play()) {
				result = false;
				break;
			}
			if (scene.skipped() || vm->isSceneRestartRequested())
				break;
		}
	}
	if (result && !Engine::shouldQuit() && !vm->isSceneRestartRequested()) {
		DemoEndCard endCard(vm);
		result = endCard.play();
	}
	vm->gameplayMusic()->stop();
	vm->gameplayMusic()->setArchive(previousMusicArchive);
	if (!vm->isSceneRestartRequested())
		vm->gameState() = previousState;
	vm->setCanSave(previousCanSave);
	if (cursorVisible)
		vm->cursor()->enterInteractiveMode();
	return result;
}

} // End of namespace Hollywood
