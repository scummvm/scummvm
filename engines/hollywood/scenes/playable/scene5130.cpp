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

#include "hollywood/scenes/playable/scene5130.h"

#include "common/debug.h"
#include "common/events.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "common/util.h"
#include "graphics/pixelformat.h"

#include "hollywood/font.h"
#include "hollywood/gameplay/cursor.h"
#include "hollywood/gameplay/game_state.h"
#include "hollywood/graphics.h"
#include "hollywood/hollywood.h"
#include "hollywood/resource.h"

namespace Hollywood {

const char *const kScene5130ArchiveName = "RESOURCE.E13";
const char *const kScene5130SoundArchiveName = "RESOURCE.S05";
const uint kScene5130InitialRequiredChunkCount = 11;
const uint kScene5130ArenaFirstChunk = 4;
const uint kScene5130ArenaLastChunk = 10;
const uint kScene5130StageIndex = 512;
const uint32 kScene5130SpeechCueDescriptorTableOffset = 0x1135;
const uint kScene5130FrameBufferSize = HollywoodEngine::kSceneBufferWidth * HollywoodEngine::kSceneBufferHeight;
const uint16 kScene5130ReturnState = 0x1401;
const byte kScene5130ActionChangeDrink = 1;
const byte kScene5130ActionOpenTap = 2;
const byte kScene5130ActionExit = 3;
const byte kScene5130InitialClipLastFrame = 0x39;
const byte kScene5130SuccessCocktailState = 2;
const byte kScene5130FailureCocktailState = 1;
const byte kScene5130TextColor = 0xfb;
const byte kScene5130TapSoundCue = 0x24;
const byte kScene5130IntroSoundCue = 0x23;
const byte kScene5130MixLoopCue = 0x20;
const byte kScene5130PourCue = 0x21;
const uint32 kScene5130IntroFrameMillis = 40;
const uint32 kScene5130ChangeFrameMillis = 20;
const uint32 kScene5130TapFrameMillis = 40;
const uint32 kScene5130MixFrameMillis = 10;
const uint32 kScene5130PourFrameMillis = 20;
const uint32 kScene5130LoopDelayMillis = 10;
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
		_vm(vm),
		_resources(),
		_surface(),
		_textStore(),
		_speechOverlay(),
		_speech(),
		_soundBank0(),
		_animationLayers(),
		_selectedDrinks(),
		_selectedDrinkCount(0),
		_currentDrinkId(1),
		_introFrame(0),
		_tapFrame(0),
		_changeFrame(0),
		_liquidFrame(0),
		_mixFrame(0),
		_pourFrame(0),
		_drinkStripRow(0),
		_hoverActionId(0),
		_pourVisible(false),
		_exitRequested(false) {
	_surface.paletteResource.resize(kPaletteSize);
	_surface.paletteCurrent.resize(kPaletteSize);
	_surface.initializeFramebuffers();
	_surface.screen.create(HollywoodEngine::kScreenWidth, HollywoodEngine::kScreenHeight,
		Graphics::PixelFormat::createFormatCLUT8());
	_speechOverlay.visible = false;
	_speechOverlay.colorIndex = kScene5130TextColor;
	_speechOverlay.centerX = HollywoodEngine::kScreenWidth / 2;
	_speechOverlay.topY = 1;
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

	runIntroAnimation();
	if (!Engine::shouldQuit() && !_exitRequested)
		runMixerLoop();

	_vm->cursor()->leaveInteractiveMode();
	stopAudio();
	return true;
}

bool Scene5130::load() {
	if (!_resources.loadChunkTable(kScene5130ArchiveName)) {
		warning("Failed to read %s header", kScene5130ArchiveName);
		return false;
	}

	if (!_resources.validateRequiredChunks(kScene5130ArchiveName, "scene 5130", kScene5130InitialRequiredChunkCount))
		return false;

	if (!_resources.loadFixedChunk("scene 5130 framebuffer", 0, _surface.baseFramebuffer, kScene5130FrameBufferSize) ||
			!_resources.loadFixedChunk("scene 5130 palette", 1, _surface.paletteResource, kPaletteSize) ||
			!_resources.loadVariableChunk(2, _surface.fillRuns) ||
			!_resources.loadVariableChunk(3, _surface.paletteMask)) {
		warning("Scene 5130 load failed: fixed E13 chunks");
		return false;
	}

	_surface.baseFramebufferOriginal.copyFrom(_surface.baseFramebuffer);
	memcpy(_surface.framebufferPixels(_surface.sceneFramebuffer),
		_surface.framebufferPixels(_surface.baseFramebuffer), _surface.framebufferByteCount());
	memcpy(_surface.paletteCurrent.data(), _surface.paletteResource.data(), _surface.paletteCurrent.size());

	const uint32 arenaSize = _resources.chunkTable.sizes[4] + _resources.chunkTable.sizes[5] +
		_resources.chunkTable.sizes[6] + _resources.chunkTable.sizes[7] +
		_resources.chunkTable.sizes[8] + _resources.chunkTable.sizes[9] +
		_resources.chunkTable.sizes[10];
	_resources.allocateArena(arenaSize);
	for (uint chunk = kScene5130ArenaFirstChunk; chunk <= kScene5130ArenaLastChunk; ++chunk) {
		if (!_resources.loadArenaChunk("scene 5130", chunk)) {
			warning("Scene 5130 load failed: %s arena chunk %u", kScene5130ArchiveName, chunk);
			return false;
		}
	}

	memset(_surface.framebufferPixels(_surface.savedFramebuffer), 0, _surface.framebufferByteCount());
	expandFillRunsToSavedFramebuffer();

	if (_surface.paletteMask.size() < kSceneColorToItemMap + kScenePaletteMapPageSize) {
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

void Scene5130::expandFillRunsToSavedFramebuffer() {
	uint destinationOffset = 0;
	uint sourceOffset = 0;
	byte *savedPixels = _surface.framebufferPixels(_surface.savedFramebuffer);
	const uint savedSize = _surface.framebufferByteCount();
	while (destinationOffset < savedSize && sourceOffset + 3 <= _surface.fillRuns.size()) {
		const byte fill = _surface.fillRuns[sourceOffset];
		const uint16 runLength = readUint16LE(_surface.fillRuns, sourceOffset + 1);
		sourceOffset += 3;

		const uint count = MIN<uint>(runLength, savedSize - destinationOffset);
		if (count != 0) {
			memset(savedPixels + destinationOffset, fill, count);
			destinationOffset += count;
		}
	}
}

void Scene5130::runIntroAnimation() {
	_selectedDrinkCount = 0;
	_currentDrinkId = 1;
	_introFrame = 0;
	_tapFrame = 0;
	_changeFrame = 0;
	_liquidFrame = 0;
	_mixFrame = 0;
	_pourFrame = 0;
	_drinkStripRow = 0;
	_pourVisible = false;
	_hoverActionId = 0;
	resetAnimationLayers();

	setPaletteEntry6Bit(0xd0, 0, 0, 0);
	setPaletteEntry6Bit(0xd1, 0, 0, 0);
	_soundBank0.playSample(kScene5130IntroSoundCue, 50, true);

	for (uint frame = 0; frame <= kScene5130InitialClipLastFrame && !Engine::shouldQuit() && !_exitRequested; ++frame) {
		_introFrame = (byte)frame;
		drawFrame();
		presentFrame();
		waitAndRender(kScene5130IntroFrameMillis);
	}

	_soundBank0.stop();
	_introFrame = kScene5130InitialClipLastFrame;
}

void Scene5130::runMixerLoop() {
	_vm->cursor()->enterInteractiveMode();
	_vm->cursor()->updatePosition(g_system->getEventManager()->getMousePos());

	while (!Engine::shouldQuit() && !_exitRequested) {
		byte selectedAction = 0;
		if (pollEvents(selectedAction))
			break;

		if (selectedAction != 0) {
			_vm->cursor()->leaveInteractiveMode();
			handleMixerAction(selectedAction);
			if (!_exitRequested && !Engine::shouldQuit())
				_vm->cursor()->enterInteractiveMode();
		}

		const byte hoverAction = actionAtCursor();
		if (hoverAction != _hoverActionId)
			_hoverActionId = hoverAction;

		_vm->cursor()->advance(kScene5130LoopDelayMillis);
		drawFrame();
		presentFrame();
		g_system->delayMillis(kScene5130LoopDelayMillis);
	}

	_vm->gameState().mainFlowStateId = kScene5130ReturnState;
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
	_soundBank0.playSample(kScene5130TapSoundCue, 50);

	for (uint step = 0; step < 16 && !Engine::shouldQuit() && !_exitRequested; ++step) {
		if (step < ARRAYSIZE(kScene5130ChangeFrameMap))
			_changeFrame = (byte)step;
		_drinkStripRow = (_drinkStripRow + 1) % kScene5130DrinkStripRows;
		drawFrame();
		presentFrame();
		waitAndRender(kScene5130ChangeFrameMillis);
	}

	if (_currentDrinkId < 0x0f)
		++_currentDrinkId;
	else
		_currentDrinkId = 1;
}

void Scene5130::runOpenTapAction() {
	if (_selectedDrinkCount >= 3)
		return;

	beginSpeechLine(4, _selectedDrinkCount);

	_soundBank0.playSample(kScene5130TapSoundCue, 50);
	const uint frameCount = MAX<uint>(ARRAYSIZE(kScene5130TapFrameMap), ARRAYSIZE(kScene5130LiquidFrameMap));
	for (uint frame = 0; frame < frameCount && !Engine::shouldQuit() && !_exitRequested; ++frame) {
		_tapFrame = (byte)MIN<uint>(frame, ARRAYSIZE(kScene5130TapFrameMap) - 1);
		_liquidFrame = (byte)MIN<uint>(frame, ARRAYSIZE(kScene5130LiquidFrameMap) - 1);
		drawFrame();
		presentFrame();
		waitAndRender(kScene5130TapFrameMillis);
	}

	_selectedDrinks[_selectedDrinkCount++] = _currentDrinkId;
	if (_selectedDrinkCount != 3)
		return;

	beginSpeechLine(4, 3);
	const bool correctRecipe = selectedRecipeIsCorrect();
	runMixResultAction(correctRecipe);
	_vm->gameState().scene5120CocktailState = correctRecipe ?
		kScene5130SuccessCocktailState : kScene5130FailureCocktailState;
	_exitRequested = true;
}

void Scene5130::runMixResultAction(bool correctRecipe) {
	if (correctRecipe)
		applySuccessDrinkPalette();
	else
		applyFailureDrinkPalette();

	_soundBank0.playSample(kScene5130MixLoopCue, 50, true);
	for (uint frame = 0; frame < ARRAYSIZE(kScene5130MixFrameMap) && !Engine::shouldQuit() && !_exitRequested; ++frame) {
		_mixFrame = (byte)frame;
		drawFrame();
		presentFrame();
		waitAndRender(kScene5130MixFrameMillis);
	}

	_soundBank0.playSample(kScene5130PourCue, 50);
	_pourVisible = true;
	for (uint frame = 0; frame <= 0x19 && !Engine::shouldQuit() && !_exitRequested; ++frame) {
		_pourFrame = (byte)frame;
		drawFrame();
		presentFrame();
		waitAndRender(kScene5130PourFrameMillis);
	}
	_pourVisible = false;

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
	setPaletteEntry6Bit(0xd0, 0x18, 0x30, 0x3c);
	setPaletteEntry6Bit(0xd1, 0x0c, 0x24, 0x30);
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
		setPaletteEntry6Bit(0xd0, 0x39, 0x39, 0x39);
		setPaletteEntry6Bit(0xd1, 0x2d, 0x2d, 0x2d);
		return;
	}

	const byte red = (byte)(total[0] / validCount);
	const byte green = (byte)(total[1] / validCount);
	const byte blue = (byte)(total[2] / validCount);
	setPaletteEntry6Bit(0xd0, red, green, blue);
	setPaletteEntry6Bit(0xd1,
		red < 0x0d ? 0 : (byte)(red - 0x0c),
		green < 0x0d ? 0 : (byte)(green - 0x0c),
		blue < 0x0d ? 0 : (byte)(blue - 0x0c));
}

void Scene5130::resetAnimationLayers() {
	_animationLayers.clear();
	_animationLayers.configureLayer(kScene5130IntroLayer, kScene5130IntroChunk,
		kScene5130IntroDescriptorCount, nullptr, 0);
	_animationLayers.configureLayer(kScene5130TapLayer, kScene5130TapChunk,
		kScene5130TapDescriptorCount, kScene5130TapFrameMap, ARRAYSIZE(kScene5130TapFrameMap));
	_animationLayers.configureLayer(kScene5130ChangeLayer, kScene5130ChangeChunk,
		kScene5130ChangeDescriptorCount, kScene5130ChangeFrameMap, ARRAYSIZE(kScene5130ChangeFrameMap));
	_animationLayers.configureLayer(kScene5130LiquidLayer, kScene5130LiquidChunk,
		kScene5130LiquidDescriptorCount, kScene5130LiquidFrameMap, ARRAYSIZE(kScene5130LiquidFrameMap));
	_animationLayers.configureLayer(kScene5130MixLayer, kScene5130MixChunk,
		kScene5130MixDescriptorCount, kScene5130MixFrameMap, ARRAYSIZE(kScene5130MixFrameMap));
	_animationLayers.configureLayer(kScene5130PourLayer, kScene5130PourChunk,
		kScene5130PourDescriptorCount, nullptr, 0, false);
}

void Scene5130::updateAnimationLayerFrames() {
	_animationLayers.setLayerFramePreservingVisibility(kScene5130IntroLayer, _introFrame);
	_animationLayers.setLayerFramePreservingVisibility(kScene5130TapLayer, _tapFrame);
	_animationLayers.setLayerFramePreservingVisibility(kScene5130ChangeLayer, _changeFrame);
	_animationLayers.setLayerFramePreservingVisibility(kScene5130LiquidLayer, _liquidFrame);
	_animationLayers.setLayerFramePreservingVisibility(kScene5130MixLayer, _mixFrame);
	_animationLayers.setLayerVisible(kScene5130PourLayer, _pourVisible);
	_animationLayers.setLayerFramePreservingVisibility(kScene5130PourLayer, _pourFrame);
}

void Scene5130::drawFrame() {
	_surface.copyBaseFramebufferToSceneFramebuffer();
	updateAnimationLayerFrames();
	drawTransientLayers(_animationLayers);
	drawDrinkStrip();
}

void Scene5130::drawTransientLayers(const TransientLayerCompositor &compositor) {
	for (uint i = 0; i < compositor.layerCount(); ++i)
		drawSpriteLayer(compositor.layer(i));
}

void Scene5130::drawSpriteLayer(const ResourceSpriteLayer &layer) {
	if (!layer.visible || layer.chunkIndex >= HollywoodEngine::kResourceChunkCount ||
			!_resources.chunkTable.isValidChunk(layer.chunkIndex) ||
			layer.descriptorIndex() >= layer.descriptorCount)
		return;

	drawStripSpriteFrame(_resources.arena, _resources.chunkOffsets[layer.chunkIndex], 0, layer.descriptorCount,
		layer.descriptorIndex(), _surface.sceneFramebuffer);
}

void Scene5130::drawDrinkStrip() {
	if (!_resources.chunkTable.isValidChunk(10) || _resources.chunkOffsets[10] >= _resources.arena.size())
		return;

	const uint32 chunkOffset = _resources.chunkOffsets[10];
	byte *destinationPixels = _surface.framebufferPixels(_surface.sceneFramebuffer);
	for (uint row = 0; row < kScene5130DrinkStripHeight; ++row) {
		const uint sourceRow = (_drinkStripRow + row) % kScene5130DrinkStripRows;
		const uint sourceOffset = chunkOffset + sourceRow * kScene5130DrinkStripWidth;
		if (sourceOffset + kScene5130DrinkStripWidth > _resources.arena.size())
			continue;

		const uint destinationOffset = (kScene5130DrinkStripY + row) * HollywoodEngine::kSceneBufferWidth +
			kScene5130DrinkStripX;
		if (destinationOffset + kScene5130DrinkStripWidth > _surface.framebufferByteCount())
			continue;

		memcpy(destinationPixels + destinationOffset, _resources.arena.data() + sourceOffset,
			kScene5130DrinkStripWidth);
	}
}

void Scene5130::presentFrame() {
	_surface.displayPalette.uploadFrom6Bit(_surface.paletteCurrent);
	_surface.screen.copyRectToSurface(_surface.sceneFramebuffer.rawSurface(), 0, 0,
		Common::Rect(0, 0, HollywoodEngine::kScreenWidth, HollywoodEngine::kScreenHeight));

	drawSpeechOverlay();
	drawCaption();

	g_system->copyRectToScreen(_surface.screen.getPixels(), _surface.screen.pitch,
		0, 0, _surface.screen.w, _surface.screen.h);
	g_system->updateScreen();
}

void Scene5130::drawSpeechOverlay() {
	if (!_speechOverlay.visible || !_vm->font() || !_vm->font()->isLoaded())
		return;

	HollywoodFont *font = _vm->font();
	font->setShadowColor(0);
	Graphics::Surface *screenSurface = _surface.screen.surfacePtr();

	for (uint lineIndex = 0; lineIndex < _speechOverlay.lines.size(); ++lineIndex) {
		const Common::String &line = _speechOverlay.lines[lineIndex];
		const int lineWidth = speechTextWidth(line);
		const int x = (int)_speechOverlay.centerX - (lineWidth >> 1);
		const int y = (int)_speechOverlay.topY + lineIndex * 20;
		font->drawString(screenSurface, line, x, y, lineWidth, _speechOverlay.colorIndex,
			Graphics::kTextAlignLeft, 0, false, true);
	}
}

void Scene5130::drawCaption() {
	if (_hoverActionId == 0 || !_vm->font() || !_vm->font()->isLoaded() ||
			_speechOverlay.visible)
		return;

	const Common::String caption = captionForAction(_hoverActionId);
	if (caption.empty())
		return;

	HollywoodFont *font = _vm->font();
	font->setShadowColor(0);
	const int textWidth = font->getStringWidth(caption) + 2;
	const int x = MAX<int>(0, (HollywoodEngine::kScreenWidth - textWidth) / 2);
	font->drawString(_surface.screen.surfacePtr(), caption, x, 456, textWidth,
		kScene5130TextColor, Graphics::kTextAlignLeft, 0, false, true);
}

void Scene5130::beginSpeechLine(uint16 rowIndex, byte frameIndex) {
	uint16 textRecordId = 0;
	byte continuationCount = 0;
	uint16 voiceSampleId = 0;
	if (!_textStore.getStageCue(rowIndex, frameIndex, textRecordId, continuationCount, voiceSampleId))
		return;

	const byte lineCount = MAX<byte>(1, continuationCount);
	for (byte part = 0; part < lineCount && !Engine::shouldQuit() && !_exitRequested; ++part) {
		const Common::String text = _textStore.largeTextRecord(textRecordId + part);
		if (text.empty())
			continue;

		_speechOverlay.visible = _vm->subtitlesEnabled();
		_speechOverlay.colorIndex = kScene5130TextColor;
		wrapSpeechText(text, HollywoodEngine::kScreenWidth / 2, _speechOverlay.lines);
		calculateSpeechOverlayBounds(_speechOverlay, HollywoodEngine::kScreenWidth / 2, 0);

		const uint16 sampleId = voiceSampleId == 0 ? 0 : voiceSampleId + part;
		const bool started = sampleId != 0 && _speech.playSample(sampleId, 100);
		const uint32 duration = started ? MAX<uint32>(_speech.lastSampleDurationMillis(), 750) :
			MAX<uint32>(1200, _speechOverlay.lines.size() * 1100);
		waitAndRender(duration);
		_speech.stop();
		_speechOverlay.visible = false;
		_speechOverlay.lines.clear();
	}
}

void Scene5130::wrapSpeechText(const Common::String &text, uint16 centerX, Common::Array<Common::String> &lines) const {
	lines.clear();
	if (text.empty())
		return;

	uint maxChars = 0x32;
	const int anchorX = centerX;
	if (anchorX < 0xa0)
		maxChars = (anchorX * 0x32) / 0xa0;
	else if (HollywoodEngine::kScreenWidth - anchorX < 0xa0)
		maxChars = ((HollywoodEngine::kScreenWidth - anchorX) * 0x32) / 0xa0;
	maxChars = MAX<uint>(maxChars, 0x18);

	const uint lineShrink = maxChars < 0x2a ? (maxChars > 0x20 ? 2 : 1) : 3;
	const char *source = text.c_str();
	const uint textLength = text.size();
	uint cursor = 0;
	while (cursor < textLength && lines.size() < 10) {
		uint end = textLength;
		if (cursor + maxChars < textLength) {
			end = cursor + maxChars;
			while (end > cursor && (byte)source[end] != 0x20 && source[end] != 0)
				--end;
			while (end > cursor && (byte)source[end - 1] == 0x20)
				--end;
			if (end == cursor)
				end = MIN<uint>(textLength, cursor + maxChars);
		}

		lines.push_back(Common::String(source + cursor, end - cursor));

		cursor = end;
		while (cursor < textLength && (byte)source[cursor] == 0x20)
			++cursor;

		maxChars = maxChars > lineShrink ? maxChars - lineShrink : 1;
	}
}

void Scene5130::calculateSpeechOverlayBounds(SpeechOverlay &overlay, int centerX, int topY) const {
	const uint textWidth = speechOverlayTextWidth(overlay);
	int adjustedCenterX = centerX;
	if (((adjustedCenterX - (int)(textWidth >> 1)) - 1 + (int)textWidth) > 0x27e)
		adjustedCenterX = (textWidth & 1) == 0 ? 0x27e - (textWidth >> 1) : 0x27d - (textWidth >> 1);
	if (adjustedCenterX - (int)(textWidth >> 1) < 1)
		adjustedCenterX = (textWidth >> 1) + 1;

	int adjustedTopY = topY - (int)overlay.lines.size() * 20;
	if (adjustedTopY < 1)
		adjustedTopY = 1;

	overlay.centerX = (uint16)adjustedCenterX;
	overlay.topY = (uint16)adjustedTopY;
}

uint Scene5130::speechTextWidth(const Common::String &text) const {
	if (!_vm->font() || !_vm->font()->isLoaded())
		return 0;

	return _vm->font()->getStringWidth(text) + 2;
}

uint Scene5130::speechOverlayTextWidth(const SpeechOverlay &overlay) const {
	uint textWidth = 0;
	for (uint i = 0; i < overlay.lines.size(); ++i)
		textWidth = MAX<uint>(textWidth, speechTextWidth(overlay.lines[i]));

	return textWidth;
}

bool Scene5130::waitAndRender(uint32 millis) {
	uint32 remaining = millis;
	while (remaining != 0 && !Engine::shouldQuit() && !_exitRequested) {
		byte selectedAction = 0;
		if (pollEvents(selectedAction))
			return true;

		const uint32 slice = MIN<uint32>(remaining, 10);
		_vm->cursor()->advance(slice);
		drawFrame();
		presentFrame();
		g_system->delayMillis(slice);
		remaining -= slice;
	}

	return Engine::shouldQuit() || _exitRequested;
}

bool Scene5130::pollEvents(byte &selectedAction) {
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
			_surface.displayPalette.markAllDirty();
			break;
		case Common::EVENT_MOUSEMOVE:
			_vm->cursor()->updatePosition(event.mouse);
			break;
		case Common::EVENT_LBUTTONDOWN:
			selectedAction = actionAtCursor();
			break;
		case Common::EVENT_RBUTTONDOWN:
			_exitRequested = true;
			return true;
		case Common::EVENT_KEYDOWN:
			if (event.kbd.keycode == Common::KEYCODE_ESCAPE) {
				_exitRequested = true;
				return true;
			}
			if (event.kbd.keycode == Common::KEYCODE_RETURN ||
					event.kbd.keycode == Common::KEYCODE_KP_ENTER ||
					event.kbd.keycode == Common::KEYCODE_SPACE)
				selectedAction = actionAtCursor();
			break;
		default:
			break;
		}

		if (_vm->isSceneRestartRequested())
			return true;
	}

	return Engine::shouldQuit();
}

byte Scene5130::actionAtCursor() const {
	const uint16 x = _vm->cursor()->surfaceX();
	const uint16 y = _vm->cursor()->surfaceY();
	if (x >= HollywoodEngine::kScreenWidth || y >= HollywoodEngine::kScreenHeight)
		return 0;

	const uint offset = y * HollywoodEngine::kSceneBufferWidth + x;
	if (offset >= _surface.framebufferByteCount())
		return 0;

	const byte pixel = _surface.savedFramebufferPixelAt(offset);
	const uint mapOffset = kSceneColorToItemMap + pixel;
	if (mapOffset >= _surface.paletteMask.size())
		return 0;

	const byte actionId = _surface.paletteMask[mapOffset];
	if (actionId == kScene5130ActionChangeDrink ||
			actionId == kScene5130ActionOpenTap ||
			actionId == kScene5130ActionExit)
		return actionId;
	return 0;
}

Common::String Scene5130::captionForAction(byte actionId) const {
	switch (actionId) {
	case kScene5130ActionChangeDrink:
		return Common::String(" Cambiar bebida");
	case kScene5130ActionOpenTap:
		return Common::String(" Abrir grifo");
	case kScene5130ActionExit:
		return Common::String(" Dejar de hacer cocteles");
	default:
		return Common::String();
	}
}

void Scene5130::setPaletteEntry6Bit(byte colorIndex, byte red, byte green, byte blue) {
	_surface.setPaletteEntry6Bit(colorIndex, red, green, blue);
}

void Scene5130::stopAudio() {
	_soundBank0.stop();
	_speech.stop();
}

} // End of namespace Hollywood
