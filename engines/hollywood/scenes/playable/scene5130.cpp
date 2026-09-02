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

#include "common/debug.h"
#include "common/events.h"
#include "common/file.h"
#include "common/textconsole.h"
#include "common/util.h"

#include "hollywood/hollywood.h"
#include "hollywood/font.h"
#include "hollywood/game_strings.h"
#include "hollywood/gameplay/cursor.h"
#include "hollywood/gameplay/game_state.h"
#include "hollywood/graphics.h"
#include "hollywood/resource.h"
#include "hollywood/scenes/playable/scene5130.h"

namespace Hollywood {

const char *const kScene5130ArchiveName = "RESOURCE.E13";
const char *const kScene5130SoundArchiveName = "RESOURCE.S05";
const char *const kScene5130MusicArchiveName = "RESOURCE.M05";
const char *const kScene5130Resource000Name = "RESOURCE.000";
const uint kScene5130InitialRequiredChunkCount = 11;
const uint kScene5130ArenaFirstChunk = 4;
const uint kScene5130ArenaLastChunk = 10;
const uint kScene5130StageIndex = 512;
const uint32 kScene5130SpeechCueDescriptorTableOffset = 0x1135;
const uint kScene5130Resource000HeaderSize = 1;
const uint kScene5130ActorPaletteTableEntry = 0x00cc;
const uint kScene5130ActorPaletteOffset = 0x270;
const uint kScene5130ActorPaletteSize = 0x90;
const uint kScene5130FrameBufferSize = HollywoodEngine::kSceneBufferWidth * HollywoodEngine::kSceneBufferHeight;
const uint16 kScene5130ReturnState = 0x1401;
const byte kScene5130ActionChangeDrink = 1;
const byte kScene5130ActionOpenTap = 2;
const byte kScene5130ActionExit = 3;
const byte kScene5130InitialClipLastFrame = 0x39;
const byte kScene5130SuccessCocktailState = 2;
const byte kScene5130FailureCocktailState = 1;
const byte kScene5130SpeechColor = 0xfb;
const byte kScene5130CaptionColor = 0xfc;
const byte kScene5130CaptionSourceColor = 0xf2;
const byte kScene5130WhiteColor = 0xfd;
const byte kScene5130TapSoundCue = 0x24;
const byte kScene5130IntroSoundCue = 0x23;
const byte kScene5130MixLoopCue = 0x20;
const byte kScene5130PourCue = 0x21;
const uint32 kScene5130IntroFrameMillis = 40;
const uint32 kScene5130ChangeFrameMillis = 40;
const uint32 kScene5130DrinkStripFrameMillis = 20;
const uint32 kScene5130TapFrameMillis = 40;
const uint32 kScene5130MixFrameMillis = 10;
const uint32 kScene5130PourFrameMillis = 20;
const uint32 kScene5130LoopDelayMillis = 10;
const uint32 kScene5130PaletteFadeStepMillis = 10;
const uint32 kScene5130AmbientMusicCheckMillis = 250;
const byte kScene5130AmbientMusicFirstCue = 0x0b;
const byte kScene5130AmbientMusicCueCount = 5;
const byte kScene5130AmbientMusicProbability = 50;
const uint kScene5130DrinkStripX = 0x10a;
const uint kScene5130DrinkStripY = 0x76;
const uint kScene5130DrinkStripWidth = 0x68;
const uint kScene5130DrinkStripHeight = 0x10;
const uint kScene5130DrinkStripRows = 0x6180 / kScene5130DrinkStripWidth;
const uint kScene5130IntroLayer = 0;
const uint kScene5130TapLayer = 1;
const uint kScene5130ChangeLayer = 2;
const uint kScene5130LiquidLayer = 3;
const uint kScene5130MixLayer = 4;
const uint kScene5130PourLayer = 5;
const uint kScene5130IntroChunk = 4;
const uint kScene5130IntroDescriptorCount = 0x3a;
const uint kScene5130TapChunk = 5;
const uint kScene5130TapDescriptorCount = 0x10;
const uint kScene5130ChangeChunk = 6;
const uint kScene5130ChangeDescriptorCount = 0x0e;
const uint kScene5130LiquidChunk = 7;
const uint kScene5130LiquidDescriptorCount = 0x0e;
const uint kScene5130MixChunk = 8;
const uint kScene5130MixDescriptorCount = 0x13;
const uint kScene5130PourChunk = 9;
const uint kScene5130PourDescriptorCount = 0x1a;

const byte kScene5130TapFrameMap[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 9, 9, 9, 9, 9, 9,
	9, 9, 9, 9, 9, 9, 10, 11, 12, 13, 14, 15, 0
};

const byte kScene5130ChangeFrameMap[] = {
	13, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13
};

const byte kScene5130LiquidFrameMap[] = {
	5, 1, 2, 3, 4, 5, 6, 7, 8, 9, 9, 9, 9, 9, 9, 9,
	9, 9, 9, 9, 9, 9, 10, 11, 12, 13, 5
};

const byte kScene5130MixFrameMap[] = {
	18, 17, 16, 17, 18, 0, 1, 0, 18, 17, 16, 17, 18, 0, 1, 0,
	18, 17, 16, 17, 18, 0, 1, 0, 18, 17, 16, 17, 18, 0, 1, 0,
	18, 17, 16, 17, 18, 0, 1, 0, 18, 17, 16, 17, 18, 0, 1, 0,
	18, 17, 16, 17, 18, 0, 1, 0, 18, 17, 16, 17, 18, 0, 1, 0,
	18, 17, 16, 17, 18, 0, 1, 0, 18, 17, 16, 17, 18, 0, 1, 0,
	18, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
	16, 17, 18
};

const SceneLayerSpec kScene5130LayerSpecs[] = {
	{ kSceneAnimationScenePlaced, kScene5130IntroChunk,
		kScene5130IntroDescriptorCount, nullptr, 0, true, 0 },
	{ kSceneAnimationScenePlaced, kScene5130TapChunk,
		kScene5130TapDescriptorCount, kScene5130TapFrameMap,
		ARRAYSIZE(kScene5130TapFrameMap), true, 0 },
	{ kSceneAnimationScenePlaced, kScene5130ChangeChunk,
		kScene5130ChangeDescriptorCount, kScene5130ChangeFrameMap,
		ARRAYSIZE(kScene5130ChangeFrameMap), true, 0 },
	{ kSceneAnimationScenePlaced, kScene5130LiquidChunk,
		kScene5130LiquidDescriptorCount, kScene5130LiquidFrameMap,
		ARRAYSIZE(kScene5130LiquidFrameMap), true, 0 },
	{ kSceneAnimationScenePlaced, kScene5130MixChunk,
		kScene5130MixDescriptorCount, kScene5130MixFrameMap,
		ARRAYSIZE(kScene5130MixFrameMap), true, 0 },
	{ kSceneAnimationScenePlaced, kScene5130PourChunk,
		kScene5130PourDescriptorCount, nullptr, 0, false, 0 }
};

const int16 kScene5130DrinkPaletteTable[16][3] = {
	{ 18,  0,  0 },
	{ -1, -1, -1 },
	{ 30, 19,  6 },
	{ 63, 32, 53 },
	{ 57, 45,  7 },
	{ -1, -1, -1 },
	{ 59, 60, 51 },
	{ 63, 31,  0 },
	{ 57, 57,  0 },
	{ 48,  8,  0 },
	{ 31, 14, 14 },
	{ 31, 31, 31 },
	{ 26, 16,  0 },
	{ -1, -1, -1 },
	{ 57, 45,  7 },
	{ -1, -1, -1 }
};

Scene5130::Scene5130(HollywoodEngine *vm) :
		PresentationScene(vm, "scene 5130", kScene5130FrameBufferSize,
			kScene5130FrameBufferSize),
		_baseFramebuffer(),
		_fillRuns(),
		_paletteMask(),
		_textStore(),
		_speech(vm->getLanguage()),
		_soundBank0(),
		_random("hollywood_scene5130"),
		_selectedDrinks(),
		_selectedDrinkCount(0),
		_currentDrinkId(1),
		_drinkStripRow(0),
		_pendingActionId(0),
		_hoverActionId(0),
		_speechTextRecordId(0),
		_speechVoiceSampleId(0),
		_speechPartIndex(0),
		_speechPartCount(0),
		_speechRemainingMillis(0),
		_ambientMusicTimerMillis(0),
		_speechActive(false),
		_mixerActionsEnabled(false),
		_deferredExitRequested(false),
		_exitRequested(false) {
	_baseFramebuffer.resize(kScene5130FrameBufferSize);
	_soundBank0.setArchive(Common::Path(kScene5130SoundArchiveName));
}

Scene5130::~Scene5130() {
	stopAudio();
}

bool Scene5130::play() {
	_vm->setCanSave(false);

	GameplayState &state = _vm->gameState();
	state.activeAudioChapterIndex = 5;
	state.currentInventoryOwnerIndex = 0;

	if (!load())
		return false;
	_vm->gameplayMusic()->setArchive(Common::Path(kScene5130MusicArchiveName));

	runIntroAnimation();
	if (_deferredExitRequested)
		_exitRequested = true;
	if (!Engine::shouldQuit() && !_exitRequested)
		runMixerLoop();
	if (!Engine::shouldQuit() && !_vm->isSceneRestartRequested()) {
		state.mainFlowStateId = kScene5130ReturnState;
		fadePaletteToBlack();
	}

	_vm->cursor()->leaveInteractiveMode();
	stopAudio();
	return true;
}

bool Scene5130::load() {
	if (!_resources.loadChunkTable(kScene5130ArchiveName))
		return false;

	if (!_resources.validateRequiredChunks(kScene5130ArchiveName, _debugName,
			kScene5130InitialRequiredChunkCount))
		return false;

	if (!loadFixedChunk(0, _baseFramebuffer, kScene5130FrameBufferSize) ||
			!loadFixedChunk(1, _paletteCurrent, kPaletteSize) ||
			!loadVariableChunk(2, _fillRuns) ||
			!loadVariableChunk(3, _paletteMask)) {
		warning("Scene 5130 load failed: fixed E13 chunks");
		return false;
	}
	if (!loadInventoryOwnerPalette())
		return false;

	const uint captionSourceOffset = kScene5130CaptionSourceColor * 3;
	const uint captionOffset = kScene5130CaptionColor * 3;
	memcpy(_paletteCurrent.data() + captionOffset,
		_paletteCurrent.data() + captionSourceOffset, 3);
	for (uint component = 0; component < 3; ++component)
		_paletteCurrent[kScene5130WhiteColor * 3 + component] = 0x3f;

	memcpy(_sceneFramebuffer.data(), _baseFramebuffer.data(), _sceneFramebuffer.size());

	const uint32 arenaSize = _resources._chunkTable.sizes[4] + _resources._chunkTable.sizes[5] +
		_resources._chunkTable.sizes[6] + _resources._chunkTable.sizes[7] +
		_resources._chunkTable.sizes[8] + _resources._chunkTable.sizes[9] +
		_resources._chunkTable.sizes[10];
	_resources.allocateArena(arenaSize);
	for (uint chunk = kScene5130ArenaFirstChunk; chunk <= kScene5130ArenaLastChunk; ++chunk) {
		if (!loadArenaChunk(chunk)) {
			warning("Scene 5130 load failed: %s arena chunk %u", kScene5130ArchiveName, chunk);
			return false;
		}
	}

	_savedFramebuffer.clear();
	expandFillRunsToSavedFramebuffer();

	if (_paletteMask.size() < kSceneColorToItemMap + kScenePaletteMapPageSize) {
		warning("Scene 5130 load failed: %s chunk 3 is too short for action color map", kScene5130ArchiveName);
		return false;
	}

	if (!_textStore.load("RESOURCE.003", "Scene 5130", kScene5130StageIndex, 0,
			kScene5130SpeechCueDescriptorTableOffset)) {
		warning("Scene 5130 load failed: RESOURCE.003 stage %u text", kScene5130StageIndex);
		return false;
	}

	debugC(1, kDebugScene, "Scene 5130 loaded %s", kScene5130ArchiveName);
	return true;
}

bool Scene5130::loadInventoryOwnerPalette() {
	Common::File file;
	if (!file.open(Common::Path(kScene5130Resource000Name))) {
		warning("Failed to open %s actor palette for scene 5130", kScene5130Resource000Name);
		return false;
	}

	const uint32 tableEntryOffset = kScene5130Resource000HeaderSize + kScene5130ActorPaletteTableEntry;
	if (tableEntryOffset + 4 > (uint32)file.size()) {
		warning("Scene 5130 actor palette table entry is out of range");
		return false;
	}

	file.seek(tableEntryOffset);
	const uint32 paletteOffset = file.readUint32LE();
	if (file.err() || paletteOffset > (uint32)file.size() ||
			kScene5130ActorPaletteSize > (uint32)file.size() - paletteOffset ||
			kScene5130ActorPaletteOffset + kScene5130ActorPaletteSize > _paletteCurrent.size()) {
		warning("Scene 5130 actor palette is out of range");
		return false;
	}

	file.seek(paletteOffset);
	if (file.read(_paletteCurrent.data() + kScene5130ActorPaletteOffset,
			kScene5130ActorPaletteSize) != kScene5130ActorPaletteSize) {
		warning("Failed to read %s actor palette for scene 5130", kScene5130Resource000Name);
		return false;
	}

	return true;
}

void Scene5130::expandFillRunsToSavedFramebuffer() {
	uint destinationOffset = 0;
	uint sourceOffset = 0;
	byte *savedPixels = _savedFramebuffer.data();
	const uint savedSize = _savedFramebuffer.size();
	while (destinationOffset < savedSize && sourceOffset + 3 <= _fillRuns.size()) {
		const byte fill = _fillRuns[sourceOffset];
		const uint16 runLength = readUint16LE(_fillRuns, sourceOffset + 1);
		sourceOffset += 3;

		const uint count = MIN<uint>(runLength, savedSize - destinationOffset);
		if (count != 0) {
			memset(savedPixels + destinationOffset, fill, count);
			destinationOffset += count;
		}
	}
}

void Scene5130::runIntroAnimation() {
	GameplayState &state = _vm->gameState();
	state.scene5120CocktailState = 0;
	state.scene5120CocktailRed = 0;
	state.scene5120CocktailGreen = 0;
	state.scene5120CocktailBlue = 0;
	memset(_selectedDrinks, 0, sizeof(_selectedDrinks));
	_selectedDrinkCount = 0;
	_currentDrinkId = 1;
	_drinkStripRow = 0;
	_pendingActionId = 0;
	_hoverActionId = 0;
	_sceneLayers.configure(kScene5130LayerSpecs);

	setPaletteEntry6Bit(0xd0, 0, 0, 0);
	setPaletteEntry6Bit(0xd1, 0, 0, 0);
	_soundBank0.playSample(kScene5130IntroSoundCue, 50, true);
	drawFrame();
	if (fadePaletteFromBlack()) {
		_soundBank0.stop();
		return;
	}

	if (!waitAndRender(kScene5130IntroFrameMillis)) {
		AnimationFrameRange introRange(1, kScene5130InitialClipLastFrame,
			kScene5130IntroFrameMillis);
		introRange.noFinalFrameDelay();
		_animationPlayer.playAndPresent(_sceneLayers.layer(kScene5130IntroLayer), introRange);
	}

	_soundBank0.stop();
}

void Scene5130::runMixerLoop() {
	_vm->cursor()->enterInteractiveMode();
	_vm->cursor()->updatePosition(g_system->getEventManager()->getMousePos());
	_hoverActionId = actionAtCursor();

	while (!Engine::shouldQuit() && !_vm->isSceneRestartRequested() && !_exitRequested) {
		TimedPresentationLoop frameLoop(*this, kScene5130LoopDelayMillis,
			kScene5130LoopDelayMillis, false);
		_pendingActionId = 0;
		_mixerActionsEnabled = true;
		const bool frameReady = frameLoop.beginFrame();
		_mixerActionsEnabled = false;
		if (!frameReady)
			break;

		if (_pendingActionId != 0) {
			_vm->cursor()->leaveInteractiveMode();
			handleMixerAction(_pendingActionId);
			if (_deferredExitRequested)
				_exitRequested = true;
			if (!_exitRequested && !Engine::shouldQuit() && !_vm->isSceneRestartRequested())
				_vm->cursor()->enterInteractiveMode();
		}
		if (_exitRequested || Engine::shouldQuit() || _vm->isSceneRestartRequested())
			break;

		_hoverActionId = actionAtCursor();
		drawFrame();
		presentFrame();
		advanceRuntime(frameLoop.finishFrame());
	}

	_vm->cursor()->leaveInteractiveMode();
}

void Scene5130::handleMixerAction(byte actionId) {
	switch (actionId) {
	case kScene5130ActionChangeDrink:
		runChangeDrinkAction();
		break;
	case kScene5130ActionOpenTap:
		runOpenTapAction();
		break;
	case kScene5130ActionExit:
		_exitRequested = true;
		break;
	default:
		break;
	}
}

void Scene5130::runChangeDrinkAction() {
	_sceneLayers.resetLayer(kScene5130ChangeLayer, 0);
	_soundBank0.playSample(kScene5130TapSoundCue, 50);

	uint32 changeFrameMillis = 0;
	uint32 drinkStripMillis = 0;
	uint drinkStripSteps = 0;
	while ((_sceneLayers.layerFrame(kScene5130ChangeLayer) < ARRAYSIZE(kScene5130ChangeFrameMap) - 1 ||
			drinkStripSteps < kScene5130DrinkStripHeight) &&
			!Engine::shouldQuit() && !_exitRequested && !_vm->isSceneRestartRequested()) {
		if (waitAndRender(kScene5130LoopDelayMillis))
			break;

		changeFrameMillis += kScene5130LoopDelayMillis;
		drinkStripMillis += kScene5130LoopDelayMillis;
		bool changed = false;
		if (_sceneLayers.layerFrame(kScene5130ChangeLayer) < ARRAYSIZE(kScene5130ChangeFrameMap) - 1 &&
				changeFrameMillis >= kScene5130ChangeFrameMillis) {
			changeFrameMillis -= kScene5130ChangeFrameMillis;
			_sceneLayers.advanceLayerFrame(kScene5130ChangeLayer,
				ARRAYSIZE(kScene5130ChangeFrameMap) - 1);
			changed = true;
		}
		if (drinkStripSteps < kScene5130DrinkStripHeight &&
				drinkStripMillis >= kScene5130DrinkStripFrameMillis) {
			drinkStripMillis -= kScene5130DrinkStripFrameMillis;
			_drinkStripRow = (_drinkStripRow + kScene5130DrinkStripRows - 1) % kScene5130DrinkStripRows;
			++drinkStripSteps;
			changed = true;
		}
		if (changed) {
			drawFrame();
			presentFrame();
		}
	}
	if (Engine::shouldQuit() || _vm->isSceneRestartRequested())
		return;

	if (_currentDrinkId < 0x0f)
		++_currentDrinkId;
	else
		_currentDrinkId = 1;
}

void Scene5130::runOpenTapAction() {
	if (_selectedDrinkCount >= 3)
		return;

	startSpeechLine(4, _selectedDrinkCount);

	_soundBank0.playSample(kScene5130TapSoundCue, 50);
	_sceneLayers.resetLayer(kScene5130TapLayer, 0);
	_sceneLayers.resetLayer(kScene5130LiquidLayer, 0);
	while ((_sceneLayers.layerFrame(kScene5130TapLayer) < ARRAYSIZE(kScene5130TapFrameMap) - 1 ||
			_sceneLayers.layerFrame(kScene5130LiquidLayer) < ARRAYSIZE(kScene5130LiquidFrameMap) - 1) &&
			!Engine::shouldQuit() && !_exitRequested && !_vm->isSceneRestartRequested()) {
		if (waitAndRender(kScene5130TapFrameMillis))
			break;
		_sceneLayers.advanceLayerFrame(kScene5130TapLayer,
			ARRAYSIZE(kScene5130TapFrameMap) - 1);
		_sceneLayers.advanceLayerFrame(kScene5130LiquidLayer,
			ARRAYSIZE(kScene5130LiquidFrameMap) - 1);
		drawFrame();
		presentFrame();
	}
	if (Engine::shouldQuit() || _exitRequested || _vm->isSceneRestartRequested())
		return;

	_selectedDrinks[_selectedDrinkCount++] = _currentDrinkId;
	if (_selectedDrinkCount != 3)
		return;

	startSpeechLine(4, 3);
	const bool correctRecipe = selectedRecipeIsCorrect();
	runMixResultAction(correctRecipe);
	if (Engine::shouldQuit() || _exitRequested || _vm->isSceneRestartRequested())
		return;
	_vm->gameState().scene5120CocktailState = correctRecipe ?
		kScene5130SuccessCocktailState : kScene5130FailureCocktailState;
	_exitRequested = true;
}

void Scene5130::runMixResultAction(bool correctRecipe) {
	if (correctRecipe)
		applySuccessDrinkPalette();
	else
		applyFailureDrinkPalette();

	_sceneLayers.resetLayer(kScene5130MixLayer, 0);
	_soundBank0.playSample(kScene5130MixLoopCue, 50, true);
	AnimationFrameRange mixRange(0, ARRAYSIZE(kScene5130MixFrameMap) - 1,
		kScene5130MixFrameMillis);
	mixRange.noFinalFrameDelay();
	_animationPlayer.playAndPresent(_sceneLayers.layer(kScene5130MixLayer), mixRange);
	if (Engine::shouldQuit() || _exitRequested || _vm->isSceneRestartRequested())
		return;

	_soundBank0.playSample(kScene5130PourCue, 50);
	_sceneLayers.showLayerAtFrame(kScene5130PourLayer, 0);
	AnimationFrameRange pourRange(0, 0x19, kScene5130PourFrameMillis);
	pourRange.noFinalFrameDelay();
	_animationPlayer.playAndPresent(_sceneLayers.layer(kScene5130PourLayer), pourRange);
	if (Engine::shouldQuit() || _exitRequested || _vm->isSceneRestartRequested())
		return;

	waitForSpeechLine();
	if (Engine::shouldQuit() || _exitRequested || _vm->isSceneRestartRequested())
		return;

	if (correctRecipe) {
		beginSpeechLine(4, 6);
		beginSpeechLine(4, 7);
	} else {
		beginSpeechLine(4, 4);
		beginSpeechLine(4, 5);
	}
}

bool Scene5130::selectedRecipeIsCorrect() const {
	if (_vm->gameState().scene5110SalonTransformState != 2)
		return false;

	bool hasDrink0B = false;
	bool hasDrink0C = false;
	bool hasDrink0E = false;
	for (uint i = 0; i < ARRAYSIZE(_selectedDrinks); ++i) {
		hasDrink0B |= _selectedDrinks[i] == 0x0b;
		hasDrink0C |= _selectedDrinks[i] == 0x0c;
		hasDrink0E |= _selectedDrinks[i] == 0x0e;
	}

	return hasDrink0B && hasDrink0C && hasDrink0E;
}

void Scene5130::applySuccessDrinkPalette() {
	applyDrinkPalette(0x18, 0x30, 0x3c);
}

void Scene5130::applyFailureDrinkPalette() {
	uint total[3] = { 0, 0, 0 };
	uint validCount = 0;
	for (uint i = 0; i < ARRAYSIZE(_selectedDrinks); ++i) {
		const byte drinkId = _selectedDrinks[i];
		if (drinkId >= ARRAYSIZE(kScene5130DrinkPaletteTable) ||
				kScene5130DrinkPaletteTable[drinkId][0] < 0)
			continue;

		for (uint component = 0; component < 3; ++component)
			total[component] += kScene5130DrinkPaletteTable[drinkId][component];
		++validCount;
	}

	if (validCount == 0) {
		applyDrinkPalette(0x39, 0x39, 0x39);
		return;
	}

	const byte red = (byte)(total[0] / validCount);
	const byte green = (byte)(total[1] / validCount);
	const byte blue = (byte)(total[2] / validCount);
	applyDrinkPalette(red, green, blue);
}

void Scene5130::applyDrinkPalette(byte red, byte green, byte blue) {
	GameplayState &state = _vm->gameState();
	state.scene5120CocktailRed = red;
	state.scene5120CocktailGreen = green;
	state.scene5120CocktailBlue = blue;

	setPaletteEntry6Bit(0xd0, red, green, blue);
	setPaletteEntry6Bit(0xd1,
		red < 0x0d ? 0 : (byte)(red - 0x0c),
		green < 0x0d ? 0 : (byte)(green - 0x0c),
		blue < 0x0d ? 0 : (byte)(blue - 0x0c));
}

void Scene5130::drawFrame() {
	memcpy(_sceneFramebuffer.data(), _baseFramebuffer.data(), _sceneFramebuffer.size());
	drawLayerStack(kSceneAnimationScenePlaced);
	drawDrinkStrip();
}

void Scene5130::drawDrinkStrip() {
	if (!_resources._chunkTable.isValidChunk(10) || _resources._chunkOffsets[10] >= _resources._arena.size())
		return;

	const uint32 chunkOffset = _resources._chunkOffsets[10];
	byte *destinationPixels = _sceneFramebuffer.data();
	for (uint row = 0; row < kScene5130DrinkStripHeight; ++row) {
		const uint sourceRow = (_drinkStripRow + row) % kScene5130DrinkStripRows;
		const uint sourceOffset = chunkOffset + sourceRow * kScene5130DrinkStripWidth;
		if (sourceOffset + kScene5130DrinkStripWidth > _resources._arena.size())
			continue;

		const uint destinationOffset = (kScene5130DrinkStripY + row) * HollywoodEngine::kSceneBufferWidth +
			kScene5130DrinkStripX;
		if (destinationOffset + kScene5130DrinkStripWidth > _sceneFramebuffer.size())
			continue;

		memcpy(destinationPixels + destinationOffset, _resources._arena.data() + sourceOffset,
			kScene5130DrinkStripWidth);
	}
}

void Scene5130::drawFrameOverlays() {
	PresentationScene::drawFrameOverlays();
	drawCaption();
}

void Scene5130::drawCaption() {
	if (_hoverActionId == 0 || !_vm->font() || !_vm->font()->isLoaded() ||
			_subtitle.visible)
		return;

	const Common::String caption = captionForAction(_hoverActionId);
	if (caption.empty())
		return;

	HollywoodFont *font = _vm->font();
	font->setShadowColor(0);
	const int textWidth = font->getStringWidth(caption) + 2;
	const int x = MAX<int>(0, (HollywoodEngine::kScreenWidth - textWidth) / 2);
	font->drawString(_screen.surfacePtr(), caption, x, 456, textWidth,
		kScene5130CaptionColor, Graphics::kTextAlignLeft, 0, false, true);
}

void Scene5130::startSpeechLine(uint16 rowIndex, byte frameIndex) {
	stopSpeechLine();

	uint16 textRecordId = 0;
	byte continuationCount = 0;
	uint16 voiceSampleId = 0;
	if (!_textStore.getStageCue(rowIndex, frameIndex, textRecordId, continuationCount, voiceSampleId))
		return;

	_speechTextRecordId = textRecordId;
	_speechVoiceSampleId = voiceSampleId;
	_speechPartIndex = 0;
	_speechPartCount = MAX<byte>(1, continuationCount);
	startSpeechPart();
}

void Scene5130::startSpeechPart() {
	_speech.stop();
	clearSubtitle();
	_speechActive = false;

	while (_speechPartIndex < _speechPartCount) {
		const Common::String text = _textStore.largeTextRecord(_speechTextRecordId + _speechPartIndex);
		if (text.empty()) {
			++_speechPartIndex;
			continue;
		}

		setPaletteEntry6Bit(kScene5130SpeechColor, 0x3f, 0x3f, 0x3f);
		_subtitle.visible = _vm->subtitlesEnabled();
		_subtitle.colorIndex = kScene5130SpeechColor;
		wrapSpeechOverlayText(text, HollywoodEngine::kScreenWidth / 2, _subtitle.lines);
		layoutSpeechOverlay(_subtitle, _vm->font(), HollywoodEngine::kScreenWidth / 2, 0);

		const uint16 sampleId = _speechVoiceSampleId == 0 ? 0 : _speechVoiceSampleId + _speechPartIndex;
		const bool started = sampleId != 0 && _speech.playSample(sampleId, 100);
		_speechRemainingMillis = started ? MAX<uint32>(_speech.lastSampleDurationMillis(), 750) :
			MAX<uint32>(1200, _subtitle.lines.size() * 1100);
		_speechActive = true;
		return;
	}

	stopSpeechLine();
}

void Scene5130::advanceSpeech(uint32 millis) {
	while (_speechActive && millis >= _speechRemainingMillis) {
		millis -= _speechRemainingMillis;
		startNextSpeechPart();
	}
	if (_speechActive)
		_speechRemainingMillis -= millis;
}

void Scene5130::startNextSpeechPart() {
	++_speechPartIndex;
	startSpeechPart();
}

void Scene5130::waitForSpeechLine() {
	while (_speechActive && !Engine::shouldQuit() && !_exitRequested &&
			!_vm->isSceneRestartRequested()) {
		if (waitAndRender(kScene5130LoopDelayMillis))
			break;
	}
}

void Scene5130::stopSpeechLine() {
	_speech.stop();
	clearSubtitle();
	_speechTextRecordId = 0;
	_speechVoiceSampleId = 0;
	_speechPartIndex = 0;
	_speechPartCount = 0;
	_speechRemainingMillis = 0;
	_speechActive = false;
}

void Scene5130::beginSpeechLine(uint16 rowIndex, byte frameIndex) {
	startSpeechLine(rowIndex, frameIndex);
	waitForSpeechLine();
}

void Scene5130::advanceRuntime(uint32 millis) {
	_vm->cursor()->advance(millis);
	advanceSpeech(millis);
	updateAmbientMusic(millis);
}

void Scene5130::updateAmbientMusic(uint32 millis) {
	_ambientMusicTimerMillis += millis;
	while (_ambientMusicTimerMillis >= kScene5130AmbientMusicCheckMillis) {
		_ambientMusicTimerMillis -= kScene5130AmbientMusicCheckMillis;
		if (_vm->gameplayMusic()->isPlaying() ||
				_random.getRandomNumber(kScene5130AmbientMusicProbability - 1) != 0)
			continue;

		GameplayState &state = _vm->gameState();
		const byte previousCue = state.currentAmbientMusicCueId;
		byte nextCue = 0;
		do {
			nextCue = (byte)(kScene5130AmbientMusicFirstCue +
				_random.getRandomNumber(kScene5130AmbientMusicCueCount - 1));
		} while (nextCue == previousCue);
		state.currentAmbientMusicCueId = nextCue;
		_vm->gameplayMusic()->playMusicCue(nextCue, 100);
	}
}

bool Scene5130::fadePaletteFromBlack() {
	const Common::Array<byte> targetPalette = _paletteCurrent;
	memset(_paletteCurrent.data(), 0, _paletteCurrent.size());
	presentFrame();

	byte threshold = 0x3f;
	while (!Engine::shouldQuit() && !_exitRequested && !_vm->isSceneRestartRequested()) {
		for (uint i = 0; i < _paletteCurrent.size(); ++i) {
			if (targetPalette[i] >= threshold)
				_paletteCurrent[i] = MIN<byte>(targetPalette[i], _paletteCurrent[i] + 3);
		}
		presentFrame();
		if (threshold == 0)
			return false;

		threshold = threshold > 3 ? threshold - 3 : 0;
		if (waitForRuntime(kScene5130PaletteFadeStepMillis))
			return true;
	}

	return true;
}

bool Scene5130::fadePaletteToBlack() {
	const Common::Array<byte> sourcePalette = _paletteCurrent;
	byte threshold = 0;
	while (!Engine::shouldQuit() && !_vm->isSceneRestartRequested()) {
		for (uint i = 0; i < _paletteCurrent.size(); ++i) {
			if (sourcePalette[i] >= threshold)
				_paletteCurrent[i] = _paletteCurrent[i] >= 3 ?
					_paletteCurrent[i] - 3 : 0;
		}
		presentFrame();
		if (threshold >= 0x3f)
			return false;

		threshold = MIN<byte>(0x3f, threshold + 3);
		if (waitForRuntime(kScene5130PaletteFadeStepMillis))
			return true;
	}

	return true;
}

bool Scene5130::waitAndRender(uint32 millis) {
	TimedPresentationLoop loop(*this, millis, 10, false);
	while (!_exitRequested && loop.beginFrame()) {
		drawFrame();
		presentFrame();
		advanceRuntime(loop.finishFrame());
	}

	return Engine::shouldQuit() || _exitRequested || _vm->isSceneRestartRequested();
}

bool Scene5130::waitForRuntime(uint32 millis) {
	TimedPresentationLoop loop(*this, millis, 10, false);
	while (loop.beginFrame())
		advanceRuntime(loop.finishFrame());

	return Engine::shouldQuit() || _vm->isSceneRestartRequested();
}

bool Scene5130::pollEvents(bool allowSkip) {
	(void)allowSkip;
	Common::Event event;
	while (g_system->getEventManager()->pollEvent(event)) {
		switch (event.type) {
		case Common::EVENT_QUIT:
		case Common::EVENT_RETURN_TO_LAUNCHER:
			Engine::quitGame();
			return true;
		case Common::EVENT_MAINMENU:
			_vm->openMainMenuDialog();
			if (_vm->isSceneRestartRequested())
				return true;
			_displayPalette.markAllDirty();
			break;
		case Common::EVENT_MOUSEMOVE:
			_vm->cursor()->updatePosition(event.mouse);
			if (_mixerActionsEnabled)
				_hoverActionId = actionAtCursor();
			break;
		case Common::EVENT_LBUTTONDOWN:
			if (_speechActive)
				stopSpeechLine();
			else if (_mixerActionsEnabled)
				_pendingActionId = actionAtCursor();
			break;
		case Common::EVENT_RBUTTONDOWN:
			if (_speechActive) {
				stopSpeechLine();
			} else if (_mixerActionsEnabled) {
				_exitRequested = true;
				return true;
			} else {
				_deferredExitRequested = true;
			}
			break;
		case Common::EVENT_KEYDOWN:
			if (event.kbd.keycode == Common::KEYCODE_ESCAPE) {
				if (_speechActive) {
					stopSpeechLine();
				} else if (_mixerActionsEnabled) {
					_exitRequested = true;
					return true;
				} else {
					_deferredExitRequested = true;
				}
				break;
			}
			if (!event.kbdRepeat &&
					(event.kbd.keycode == Common::KEYCODE_RETURN ||
					 event.kbd.keycode == Common::KEYCODE_KP_ENTER ||
					 event.kbd.keycode == Common::KEYCODE_SPACE)) {
				if (_speechActive)
					startNextSpeechPart();
				else if (_mixerActionsEnabled)
					_pendingActionId = actionAtCursor();
			}
			break;
		default:
			break;
		}

		if (_vm->isSceneRestartRequested())
			return true;
	}

	return Engine::shouldQuit() || _vm->isSceneRestartRequested();
}

byte Scene5130::actionAtCursor() const {
	const uint16 x = _vm->cursor()->surfaceX();
	const uint16 y = _vm->cursor()->surfaceY();
	if (x >= HollywoodEngine::kScreenWidth || y >= HollywoodEngine::kScreenHeight)
		return 0;

	const uint offset = y * HollywoodEngine::kSceneBufferWidth + x;
	if (offset >= _savedFramebuffer.size())
		return 0;

	const byte pixel = _savedFramebuffer[offset];
	const uint mapOffset = kSceneColorToItemMap + pixel;
	if (mapOffset >= _paletteMask.size())
		return 0;

	const byte actionId = _paletteMask[mapOffset];
	if (actionId == kScene5130ActionChangeDrink ||
			actionId == kScene5130ActionOpenTap ||
			actionId == kScene5130ActionExit)
		return actionId;
	return 0;
}

Common::String Scene5130::captionForAction(byte actionId) const {
	const HollywoodGameStrings &strings = getGameStrings(_vm->getLanguage());
	switch (actionId) {
	case kScene5130ActionChangeDrink:
		return Common::String(strings.changeDrink);
	case kScene5130ActionOpenTap:
		return Common::String(strings.openTap);
	case kScene5130ActionExit:
		return Common::String(strings.stopMakingCocktails);
	default:
		return Common::String();
	}
}

void Scene5130::setPaletteEntry6Bit(byte colorIndex, byte red, byte green, byte blue) {
	const uint paletteOffset = colorIndex * 3;
	if (paletteOffset + 2 >= _paletteCurrent.size())
		return;

	_paletteCurrent[paletteOffset] = red;
	_paletteCurrent[paletteOffset + 1] = green;
	_paletteCurrent[paletteOffset + 2] = blue;
}

void Scene5130::stopAudio() {
	_soundBank0.stop();
	stopSpeechLine();
}

bool Scene5130::animationPlaybackShouldStop() const {
	return PresentationScene::animationPlaybackShouldStop() || _exitRequested ||
		_vm->isSceneRestartRequested();
}

void Scene5130::presentAnimationFrame() {
	drawFrame();
	presentFrame();
}

bool Scene5130::waitForAnimationFrame(uint32 millis, bool allowSkip) {
	(void)allowSkip;
	return waitAndRender(millis);
}

} // End of namespace Hollywood
