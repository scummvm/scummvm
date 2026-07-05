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

#include "hollywood/scenes/playable/scene5030.h"

#include "hollywood/gameplay/game_state.h"
#include "hollywood/graphics.h"
#include "hollywood/hollywood.h"
#include "hollywood/resource.h"

namespace Hollywood {

const char *const kScene5030ArchiveName = "RESOURCE.E03";
const char *const kScene5030MusicArchiveName = "RESOURCE.M05";
const char *const kScene5030SoundArchiveName = "RESOURCE.S05";
const uint kScene5030InitialRequiredChunkCount = 5;
const uint kScene5030ArenaFirstChunk = 5;
const uint kScene5030ArenaLastChunk = 16;
const uint kScene5030StageIndex = 503;
const uint16 kScene5030FirstState = 0x13a6;
const uint16 kScene5030LastState = 0x13af;
const uint16 kScene5010ReturnState = 0x1393;
const uint16 kScene5030ViewportXOffset = 0x00c8;
const uint16 kScene5030ViewportMaxXOffset = 0x0108;
const uint kScene5030ActorBankTableEntry = 0x0000;
const uint kScene5030ActorPaletteTableEntry = 0x00cc;
const uint32 kScene5030SpeechCueDescriptorTableOffset = 0x1135;
const uint32 kScene5030FrameMillis = 75;
const uint kScene5030MineCartEntryDescriptorCount = 0x3e;
const byte kScene5030MineCartSoundFrame = 0x28;
const uint kScene5030Chunk8DescriptorCount = 0x1a;
const uint kScene5030Chunk9DescriptorCount = 0x17;
const uint kScene5030Chunk10DescriptorCount = 0x0d;
const byte kScene5030DeckOfCardsItem = 0x48;
const byte kScene5030UnderpantsItem = 0x53;
const byte kScene5030TakenSceneItemId = 6;
const byte kScene5030RenamedSmallRowA = 7;
const byte kScene5030RenamedSmallRowB = 8;
const byte kScene5030DocumentSmallRowA = 9;
const byte kScene5030DocumentSmallRowB = 10;
const byte kScene5030VanessaDialogueStageId = 0x5a;
const byte kScene5030VanessaPrimaryRow = 99;
const byte kScene5030GladysReplyToVanessaRow = 0x5f;
const byte kScene5030GladysDialogueStageId = 0x62;
const byte kScene5030GladysPrimaryRow = 0x60;
const byte kScene5030VanessaReplyToGladysRow = 0x61;
const uint kScene5030DialogueChoiceRecordCount = 10 * 10 * 7;
const byte kScene5030DialogueNoResponseFrame = 0xff;
const byte kScene5030DialogueTransitionEnd = 0;
const byte kScene5030DialogueTransitionDown = 1;
const byte kScene5030DialogueTransitionUp = 2;
const byte kScene5030DialogueTransitionStay = 3;
const byte kScene5030DialogueTransitionUpTwo = 4;
const uint16 kScene5030RonDialogueCenterX = 0x217;
const uint16 kScene5030RonDialogueTopY = 0x099;
const uint16 kScene5030RonTradeCenterX = 0x1fa;
const uint16 kScene5030RonTradeTopY = 0x0ae;
const uint16 kScene5030VanessaDialogueCenterX = 0x1b9;
const uint16 kScene5030VanessaDialogueTopY = 0x124;
const uint16 kScene5030GladysDialogueCenterX = 0x274;
const uint16 kScene5030GladysDialogueTopY = 0x11b;
const byte kScene5030GladysSpeechGroup = 0;
const byte kScene5030VanessaSpeechGroup = 1;
const byte kScene5030VanessaIdleFrame = 0x0f;
const byte kScene5030GladysIdleFrame = 0x15;
const uint kScene5030ScoutPlayingFrameCount = 8;

const byte kScene5030MineCartEntryDelayBuckets[] = {
	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 3, 3, 3, 3, 4,
	4, 4, 4, 5, 5, 5, 5, 6,
	6, 6, 6, 7, 7, 7, 7, 8,
	8, 9, 9, 10, 11, 12
};

const byte kScene5030ActorPathStepDeltaTable[] = {
	8, 1, 1, 4, 4, 3, 10, 1, 0, 0, 5, 4,
	4, 2, 11, 8, 8, 9, 8, 5, 14, 3, 2, 12,
	11, 11, 9, 7, 13, 8, 13, 13, 6, 8, 6, 14,
	5, 3, 3, 5, 0, 5, 5, 2, 0, 5, 2, 7,
	8, 13, 13, 6, 8, 6, 14, 11, 11, 9, 7, 13,
	8, 5, 14, 3, 2, 12, 4, 2, 11, 8, 8, 9
};

const byte kScene5030Chunk8FrameMap[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12,
	13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23,
	24, 25
};

const byte kScene5030Chunk9FrameMap[] = {
	0, 1, 2, 3, 4, 5, 2, 6,
	0, 7, 8, 9, 9, 8, 7, 9,
	10, 11, 12, 13, 14, 15, 16, 17,
	18, 19, 20, 21, 22, 0, 0, 0
};

const byte kScene5030Chunk10FrameMap[] = {
	0, 1, 2, 3, 4, 5, 2, 1,
	0, 1, 2, 3, 4, 6, 7, 7,
	6, 4, 3, 2, 1, 8, 9, 10,
	11, 12
};

PlayableSceneConfig scene5030Config() {
	PlayableSceneConfig config;
	config.resourceArchiveName = kScene5030ArchiveName;
	config.initialRequiredChunkCount = kScene5030InitialRequiredChunkCount;
	config.arenaFirstChunk = kScene5030ArenaFirstChunk;
	config.arenaLastChunk = kScene5030ArenaLastChunk;
	config.stageIndex = kScene5030StageIndex;
	config.debugName = "Scene 5030";
	config.viewportXOffset = kScene5030ViewportXOffset;
	config.viewportMinXOffset = kScene5030ViewportXOffset;
	config.viewportMaxXOffset = kScene5030ViewportMaxXOffset;
	config.inventoryOwnerIndex = 0;
	config.activeAudioChapterIndex = 5;
	config.actorBankTableEntry = kScene5030ActorBankTableEntry;
	config.actorPaletteTableEntry = kScene5030ActorPaletteTableEntry;
	config.inventoryActionTableExtraOffset = 0;
	config.inventoryRowsOffsetIndex = 0;
	config.speechCueDescriptorTableOffset = kScene5030SpeechCueDescriptorTableOffset;
	config.actorPathStepDeltaTable = kScene5030ActorPathStepDeltaTable;
	config.actorPathStepDeltaTableSize = ARRAYSIZE(kScene5030ActorPathStepDeltaTable);
	config.walkablePaletteMaxRegion = 20;
	config.musicArchiveName = kScene5030MusicArchiveName;
	config.soundBank0ArchiveName = kScene5030SoundArchiveName;
	config.loadActorDepthTables = true;
	config.useActorDepthTest = true;
	config.mainFlowFirstState = kScene5030FirstState;
	config.mainFlowLastState = kScene5030LastState;
	return config;
}

Scene5030::Scene5030(HollywoodEngine *vm) :
		PlayableScene(vm, scene5030Config(), "scene5030", 0x152, 0x16b, 2, 0xfd, 0xfb),
		_chunk8Channel(),
		_chunk9Channel(),
		_chunk10Channel(),
		_mineCartEntryLayer(),
		_chunk8Layer(),
		_chunk9Layer(),
		_chunk10Layer() {
	_mineCartEntryLayer.configure(5, kScene5030MineCartEntryDescriptorCount, nullptr, 0);
	_chunk8Layer.configure(8, kScene5030Chunk8DescriptorCount,
		kScene5030Chunk8FrameMap, ARRAYSIZE(kScene5030Chunk8FrameMap));
	_chunk9Layer.configure(9, kScene5030Chunk9DescriptorCount,
		kScene5030Chunk9FrameMap, ARRAYSIZE(kScene5030Chunk9FrameMap));
	_chunk10Layer.configure(10, kScene5030Chunk10DescriptorCount,
		kScene5030Chunk10FrameMap, ARRAYSIZE(kScene5030Chunk10FrameMap));
}

bool Scene5030::hasCustomPreviewState() const {
	return true;
}

void Scene5030::initializeCustomPreviewState() {
	initializeDefaultPreviewState();
	resetAnimationLayers();
	applySceneStateToHotspotsAndPatches(0xff);

	_activeActorWorldX = 0x152;
	_activeActorWorldY = 0x16b;
	_activeActorFacing = 2;
	_activeActorCel = 0;
	_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
}

bool Scene5030::hasCustomComposite() const {
	return true;
}

void Scene5030::drawCustomComposite(bool drawActiveActor, byte activeFacing, byte activeCel, int activeWorldX, int activeWorldY,
		bool drawSecondaryActor, byte secondaryFacing, byte secondaryFrame, int secondaryWorldX, int secondaryWorldY,
		byte actorDrawOrderMode) {
	(void)actorDrawOrderMode;

	copyBaseFramebufferToSceneFramebuffer();
	if (_mineCartEntryLayer.visible) {
		drawResourceSpriteLayer(_mineCartEntryLayer);
		drawResourceSpriteLayer(_chunk10Layer);
		drawResourceSpriteLayer(_chunk9Layer);
		drawResourceSpriteLayer(_chunk8Layer);
		drawActionOverlayLayer();
		return;
	}

	drawActiveAndSecondaryActorFrames(drawActiveActor, activeFacing, activeCel, activeWorldX, activeWorldY,
		drawSecondaryActor, secondaryFacing, secondaryFrame, secondaryWorldX, secondaryWorldY, -1);
	drawResourceSpriteLayer(_chunk9Layer);
	drawResourceSpriteLayer(_chunk10Layer);
	drawResourceSpriteLayer(_chunk8Layer);
	if (_sceneChunkTable.isValidChunk(6))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[6], _sceneFramebuffer);
	if (_sceneChunkTable.isValidChunk(7))
		drawResourceBlockList(_resourceArena, _resourceChunkOffsets[7], _sceneFramebuffer);
	drawActionOverlayLayer();
}

bool Scene5030::hasCustomEntrySequence() const {
	return true;
}

void Scene5030::runCustomEntrySequence() {
	_activeActorWorldX = 0x061;
	_activeActorWorldY = 0x19b;
	_activeActorFacing = 2;
	_activeActorCel = 0;
	_activeActorDrawOrderMode = paletteRegionAt(_activeActorWorldX, _activeActorWorldY);
	drawPlayableComposite();
	presentFrame();

	runMineCartEntryAnimation();
	runEntryPath(0x061, 0x19b, 2, 0x152, 0x16b);
	_activeActorFacing = 2;
	_activeActorCel = 0;

	GameplayState &state = _vm->gameState();
	if (!state.scene5030EntryLineSeen) {
		beginSecondarySpeechLine(0, 0);
		state.scene5030EntryLineSeen = true;
	}
}

bool Scene5030::prepareCustomGameplayLoop() {
	resetAnimationLayers();
	return true;
}

bool Scene5030::advanceCustomGameplayLoop(uint32 delta) {
	advanceLayer(_chunk8Channel, _chunk8Layer, ARRAYSIZE(kScene5030Chunk8FrameMap), delta);
	if (_primaryDialogueSpeechActive)
		advancePrimaryDialogueSpeechFrame(delta);
	if (!_primaryDialogueSpeechActive || _primaryDialogueSpeechGroup != kScene5030VanessaSpeechGroup)
		advanceLayer(_chunk9Channel, _chunk9Layer, kScene5030ScoutPlayingFrameCount, delta);
	if (!_primaryDialogueSpeechActive || _primaryDialogueSpeechGroup != kScene5030GladysSpeechGroup)
		advanceLayer(_chunk10Channel, _chunk10Layer, kScene5030ScoutPlayingFrameCount, delta);
	updateAmbientAudioAndMusicCues(delta);
	return true;
}

bool Scene5030::dispatchCustomSceneAction(uint16 handlerId) {
	switch (handlerId) {
	case 301: // Ir a salida/vagoneta (go to exit/mine cart): return to mine switches.
		runExitToMineSwitches();
		return true;
	case 302: // Coger poste deteriorado (take rotten pole): Ron says it is useless.
		beginSecondarySpeechLine(1, 0);
		return true;
	case 303: // Mirar pala (look at shovel): it is in bad shape.
		beginSecondarySpeechLine(2, 0);
		return true;
	case 304: // Coger tienda de campana (take tent): the musicians stop Ron.
		beginSecondarySpeechLine(3, 0);
		return true;
	case 305: // Mirar tienda de campana (look at tent).
		beginSecondarySpeechLine(4, 0);
		return true;
	case 306: // Coger baraja de cartas (take deck of cards): grants item 0x48 after the underpants exchange.
		runDeckOfCardsAction();
		return true;
	case 307: // Mirar baraja de cartas (look at deck of cards).
		beginSecondarySpeechLine(6, 0);
		return true;
	case 308: // Hablar con chica de la guitarra/Vanessa (talk to guitar girl/Vanessa): opens her dialogue tree.
		runVanessaConversation();
		return true;
	case 309: // Mirar chica de la guitarra/Vanessa (look at guitar girl/Vanessa).
		beginSecondarySpeechLine(7, 0);
		return true;
	case 310: // Mirar chica de las maracas/Gladys (look at maracas girl/Gladys).
		beginSecondarySpeechLine(8, 0);
		return true;
	case 311: // Usar calzoncillo con Vanessa/Gladys (use underpants with Vanessa/Gladys): exchange for the deck.
		runSpecialInventorySequence();
		return true;
	case 312: // Hablar con chica de las maracas/Gladys (talk to maracas girl/Gladys): opens her dialogue tree.
		runGladysConversation();
		return true;
	default:
		return false;
	}
}

bool Scene5030::applyCustomSceneStateToHotspotsAndPatches(byte selector) {
	if (_paletteMaskOriginal.empty())
		return true;

	restoreBaseFramebufferFromOriginal();
	memcpy(_paletteMask.data(), _paletteMaskOriginal.data(), _paletteMask.size());
	memcpy(_fullPaletteRegionMask.data(), _paletteMaskOriginal.data(), _fullPaletteRegionMask.size());

	const GameplayState &state = _vm->gameState();
	if ((selector == 1 || selector == 0xff) && (state.scene5030DeckOfCardsState >= 2 || hasInventoryItem(kScene5030DeckOfCardsItem))) {
		if (_sceneChunkTable.isValidChunk(12))
			drawResourceBlockList(_resourceArena, _resourceChunkOffsets[12], _baseFramebuffer);
		clearSceneItemFromColorMap(kScene5030TakenSceneItemId);
	}
	if ((selector == 3 || selector == 0xff) && state.scene5030MusiciansNamed) {
		copyStageSmallRow(kScene5030RenamedSmallRowA, kScene5030DocumentSmallRowA);
		copyStageSmallRow(kScene5030RenamedSmallRowB, kScene5030DocumentSmallRowB);
	}

	rebuildWalkablePaletteMask();
	_hotspots.load(_paletteMask, _metadata, _stage003SmallRows);
	return true;
}

AmbientAudioProfile Scene5030::ambientAudioProfile() const {
	return createRandomAmbientAudioProfile(0x0d, 8, 75, 25, 0x10, 1, 100, 50);
}

void Scene5030::resetAnimationLayers() {
	_chunk8Channel.reset(0, kScene5030FrameMillis);
	_chunk9Channel.reset(0, kScene5030FrameMillis);
	_chunk10Channel.reset(0, kScene5030FrameMillis);
	_chunk8Layer.visible = true;
	_chunk9Layer.visible = true;
	_chunk10Layer.visible = true;
	_mineCartEntryLayer.visible = false;
	_chunk8Layer.reset(0);
	_chunk9Layer.reset(0);
	_chunk10Layer.reset(0);
}

void Scene5030::advanceLayer(TimedAnimationChannel &channel, ResourceSpriteLayer &layer, uint frameCount, uint32 delta) {
	const uint consumedFrames = channel.consumeFrames(delta);
	for (uint i = 0; i < consumedFrames; ++i) {
		byte nextFrame = (byte)(layer.frameIndex + 1);
		if (nextFrame >= frameCount)
			nextFrame = 0;
		layer.setFrame(nextFrame);
	}
}

void Scene5030::runMineCartEntryAnimation() {
	if (!_sceneChunkTable.isValidChunk(5))
		return;

	const bool previousHideActiveActor = _hideActiveActor;
	_hideActiveActor = true;
	_mineCartEntryLayer.visible = true;
	_mineCartEntryLayer.reset(0);

	for (uint frame = 0; frame < ARRAYSIZE(kScene5030MineCartEntryDelayBuckets) && !Engine::shouldQuit(); ++frame) {
		_mineCartEntryLayer.setFrame((byte)frame);
		if (frame == kScene5030MineCartSoundFrame)
			_soundBank0.playSample(0x16, 100);

		const byte delayBucket = kScene5030MineCartEntryDelayBuckets[frame];
		const uint32 frameMillis = 200 / MAX<uint32>(1, 13 - delayBucket);
		if (waitSceneMillis(frameMillis))
			break;
	}

	_mineCartEntryLayer.visible = false;
	_hideActiveActor = previousHideActiveActor;
	drawPlayableComposite();
	presentFrame();
}

byte Scene5030::primarySpeechAnimationBaseFrame(byte animationGroup) const {
	switch (animationGroup) {
	case kScene5030VanessaSpeechGroup:
		return kScene5030VanessaIdleFrame;
	case kScene5030GladysSpeechGroup:
		return kScene5030GladysIdleFrame;
	default:
		return 0;
	}
}

void Scene5030::setPrimarySpeechAnimationFrame(byte animationGroup, byte frameIndex) {
	switch (animationGroup) {
	case kScene5030VanessaSpeechGroup:
		_chunk9Layer.setFrame(frameIndex);
		break;
	case kScene5030GladysSpeechGroup:
		_chunk10Layer.setFrame(frameIndex);
		break;
	default:
		break;
	}
}

void Scene5030::runExitToMineSwitches() {
	walkActiveActorTo(0x061, 0x19b, 0xff, 0, false);
	_soundBank0.playSample(0x15, 100);
	_vm->gameState().mainFlowStateId = kScene5010ReturnState;
}

void Scene5030::runDeckOfCardsAction() {
	GameplayState &state = _vm->gameState();
	if (state.scene5030DeckOfCardsState >= 2 || hasInventoryItem(kScene5030DeckOfCardsItem)) {
		beginSecondarySpeechLine(6, 0);
		return;
	}

	if (state.scene5030DeckOfCardsState == 0) {
		beginVanessaSpeechLine(5, 0);
		beginGladysSpeechLine(5, 1);
		return;
	}

	grantDeckOfCards();
}

void Scene5030::runVanessaConversation() {
	Common::Array<DialogueChoiceRecord> records;
	initializeVanessaDialogueRecords(records);

	GameplayState &state = _vm->gameState();
	const bool firstConversation = !state.scene5030MusiciansNamed;
	beginRonDialogueLine(kScene5030VanessaDialogueStageId, firstConversation ? 0 : 1);
	beginVanessaSpeechLine(kScene5030VanessaPrimaryRow, firstConversation ? 0 : 1);
	if (firstConversation) {
		state.scene5030MusiciansNamed = true;
		applySceneStateToHotspotsAndPatches(3);
	}

	byte depthIndex = 0;
	byte nodeIndex = 0;
	while (!Engine::shouldQuit() && !_vm->isSceneRestartRequested()) {
		DialogueMenu menu(_vm, this);
		const byte selectedChoice = menu.choose(kScene5030VanessaDialogueStageId,
			records, depthIndex, nodeIndex);
		if (selectedChoice == DialogueMenu::kCancelledChoice) {
			beginRonDialogueLine(kScene5030VanessaDialogueStageId, 6);
			beginVanessaSpeechLine(kScene5030VanessaPrimaryRow, 6);
			beginGladysSpeechLine(kScene5030GladysReplyToVanessaRow, 4);
			return;
		}

		const uint recordIndex = ((uint)depthIndex * 10 + nodeIndex) * 7 + selectedChoice;
		if (recordIndex >= records.size())
			return;

		DialogueChoiceRecord &record = records[recordIndex];
		beginRonDialogueLine(kScene5030VanessaDialogueStageId, record.playerTextRowId);
		if (record.responseFrameIndex != kScene5030DialogueNoResponseFrame) {
			beginVanessaSpeechLine(kScene5030VanessaPrimaryRow, record.responseFrameIndex);
			beginGladysSpeechLine(kScene5030GladysReplyToVanessaRow, record.reserved);
		}

		if (record.disableAfterUse != 0) {
			record.enabled = 0;
			record.selectable = 0;
		}
		if (applyDialogueTransition(record, depthIndex, nodeIndex))
			return;
	}
}

void Scene5030::runGladysConversation() {
	Common::Array<DialogueChoiceRecord> records;
	initializeGladysDialogueRecords(records);

	GameplayState &state = _vm->gameState();
	const bool firstConversation = !state.scene5030MusiciansNamed;
	beginRonDialogueLine(kScene5030GladysDialogueStageId, firstConversation ? 0 : 1);
	beginGladysSpeechLine(kScene5030GladysPrimaryRow, firstConversation ? 0 : 1);
	if (firstConversation) {
		state.scene5030MusiciansNamed = true;
		applySceneStateToHotspotsAndPatches(3);
	}

	byte depthIndex = 0;
	byte nodeIndex = 0;
	while (!Engine::shouldQuit() && !_vm->isSceneRestartRequested()) {
		DialogueMenu menu(_vm, this);
		const byte selectedChoice = menu.choose(kScene5030GladysDialogueStageId,
			records, depthIndex, nodeIndex);
		if (selectedChoice == DialogueMenu::kCancelledChoice) {
			beginRonDialogueLine(kScene5030GladysDialogueStageId, 6);
			beginGladysSpeechLine(kScene5030GladysPrimaryRow, 6);
			beginVanessaSpeechLine(kScene5030VanessaReplyToGladysRow, 4);
			return;
		}

		const uint recordIndex = ((uint)depthIndex * 10 + nodeIndex) * 7 + selectedChoice;
		if (recordIndex >= records.size())
			return;

		DialogueChoiceRecord &record = records[recordIndex];
		beginRonDialogueLine(kScene5030GladysDialogueStageId, record.playerTextRowId);
		if (record.responseFrameIndex != kScene5030DialogueNoResponseFrame) {
			beginGladysSpeechLine(kScene5030GladysPrimaryRow, record.responseFrameIndex);
			beginVanessaSpeechLine(kScene5030VanessaReplyToGladysRow, record.reserved);
		}

		if (record.disableAfterUse != 0) {
			record.enabled = 0;
			record.selectable = 0;
		}
		if (applyDialogueTransition(record, depthIndex, nodeIndex))
			return;
	}
}

void Scene5030::runSpecialInventorySequence() {
	GameplayState &state = _vm->gameState();
	if (!state.scene5030MusiciansNamed) {
		beginStaticSecondarySpeechLine(0xda, 0);
		return;
	}

	if (hasInventoryItem(kScene5030UnderpantsItem))
		removeInventoryItem(kScene5030UnderpantsItem);

	beginRonDialogueLine(9, 0);
	beginVanessaSpeechLine(9, 1);
	beginGladysSpeechLine(9, 2);
	beginPrimarySpeechLine(9, 3, kScene5030RonTradeCenterX, kScene5030RonTradeTopY,
		0x3f, 0x3f, 0x3f);
	beginVanessaSpeechLine(9, 4);
	beginGladysSpeechLine(9, 5);
	beginPrimarySpeechLine(9, 6, kScene5030RonTradeCenterX, kScene5030RonTradeTopY,
		0x3f, 0x3f, 0x3f);
	beginVanessaSpeechLine(9, 7);
	beginGladysSpeechLine(9, 8);
	beginPrimarySpeechLine(9, 9, kScene5030RonTradeCenterX, kScene5030RonTradeTopY,
		0x3f, 0x3f, 0x3f);
	state.scene5030DeckOfCardsState = 1;
	grantDeckOfCards();
	beginVanessaSpeechLine(9, 10);
	beginGladysSpeechLine(9, 11);
}

void Scene5030::grantDeckOfCards() {
	GameplayState &state = _vm->gameState();
	if (!hasInventoryItem(kScene5030DeckOfCardsItem))
		addInventoryItem(kScene5030DeckOfCardsItem);
	_soundBank0.playSample(1, 100);
	state.scene5030DeckOfCardsState = 2;
	applySceneStateToHotspotsAndPatches(1);
}

void Scene5030::initializeVanessaDialogueRecords(Common::Array<DialogueChoiceRecord> &records) const {
	records.clear();
	records.resize(kScene5030DialogueChoiceRecordCount);

	// DAT_00507e68: Vanessa root choices and the nested werewolf branch.
	setDialogueRecord(records, 0, 0, kScene5030DialogueTransitionDown, 2, 2, 1, 0);
	setDialogueRecord(records, 1, 0, kScene5030DialogueTransitionStay, 3, 3, 1, 1);
	setDialogueRecord(records, 2, 0, kScene5030DialogueTransitionStay, 4, 4, 1, 2);
	setDialogueRecord(records, 3, 0, kScene5030DialogueTransitionStay, 5, 5, 1, 3);
	setDialogueRecord(records, 4, 0, kScene5030DialogueTransitionEnd, 6, 6, 0, 4);
	setDialogueRecord(records, 70, 0, kScene5030DialogueTransitionStay, 7, 7, 1, 5);
	setDialogueRecord(records, 71, 0, kScene5030DialogueTransitionStay, 8, 8, 1, 6);
	setDialogueRecord(records, 72, 0, kScene5030DialogueTransitionStay, 9, 9, 1, 7);
	setDialogueRecord(records, 73, 0, kScene5030DialogueTransitionUp, 10, 10, 0, 8);
}

void Scene5030::initializeGladysDialogueRecords(Common::Array<DialogueChoiceRecord> &records) const {
	records.clear();
	records.resize(kScene5030DialogueChoiceRecordCount);

	// DAT_00506b40: Gladys root choices and the nested werewolf branch.
	setDialogueRecord(records, 0, 0, kScene5030DialogueTransitionDown, 2, 2, 1, 0);
	setDialogueRecord(records, 1, 0, kScene5030DialogueTransitionStay, 3, 3, 1, 1);
	setDialogueRecord(records, 2, 0, kScene5030DialogueTransitionStay, 4, 4, 1, 2);
	setDialogueRecord(records, 3, 0, kScene5030DialogueTransitionStay, 5, 5, 1, 3);
	setDialogueRecord(records, 4, 0, kScene5030DialogueTransitionEnd, 6, 6, 0, 4);
	setDialogueRecord(records, 70, 0, kScene5030DialogueTransitionStay, 7, 7, 1, 5);
	setDialogueRecord(records, 71, 0, kScene5030DialogueTransitionStay, 8, 8, 1, 6);
	setDialogueRecord(records, 72, 0, kScene5030DialogueTransitionUp, 9, 9, 0, 7);
}

void Scene5030::setDialogueRecord(Common::Array<DialogueChoiceRecord> &records, uint index,
		byte nextNodeIndex, byte transitionMode, byte playerTextRowId,
		byte responseFrameIndex, byte disableAfterUse, byte otherScoutFrameIndex) const {
	if (index >= records.size())
		return;

	DialogueChoiceRecord &record = records[index];
	record.enabled = 1;
	record.nextNodeIndex = nextNodeIndex;
	record.transitionMode = transitionMode;
	record.playerTextRowId = playerTextRowId;
	record.responseFrameIndex = responseFrameIndex;
	record.disableAfterUse = disableAfterUse;
	record.reserved = otherScoutFrameIndex;
	record.selectable = 1;
}

bool Scene5030::applyDialogueTransition(const DialogueChoiceRecord &record, byte &depthIndex, byte &nodeIndex) const {
	const byte previousDepth = depthIndex;
	switch (record.transitionMode) {
	case kScene5030DialogueTransitionEnd:
		return true;
	case kScene5030DialogueTransitionDown:
		nodeIndex = record.nextNodeIndex;
		depthIndex = previousDepth + 1;
		break;
	case kScene5030DialogueTransitionUp:
		nodeIndex = record.nextNodeIndex;
		depthIndex = previousDepth == 0 ? 0 : (byte)(previousDepth - 1);
		break;
	case kScene5030DialogueTransitionUpTwo:
		nodeIndex = record.nextNodeIndex;
		depthIndex = previousDepth > 1 ? (byte)(previousDepth - 2) : 0;
		break;
	case kScene5030DialogueTransitionStay:
	default:
		break;
	}

	return false;
}

void Scene5030::beginRonDialogueLine(uint16 rowIndex, byte frameIndex) {
	beginPrimarySpeechLine(rowIndex, frameIndex, kScene5030RonDialogueCenterX, kScene5030RonDialogueTopY,
		0x3f, 0x3f, 0x3f);
}

void Scene5030::beginVanessaSpeechLine(uint16 rowIndex, byte frameIndex) {
	beginPrimarySpeechLineWithAnimationGroup(rowIndex, frameIndex, kScene5030VanessaDialogueCenterX,
		kScene5030VanessaDialogueTopY, 0, 0x20, 0x3f, kScene5030VanessaSpeechGroup);
}

void Scene5030::beginGladysSpeechLine(uint16 rowIndex, byte frameIndex) {
	beginPrimarySpeechLineWithAnimationGroup(rowIndex, frameIndex, kScene5030GladysDialogueCenterX,
		kScene5030GladysDialogueTopY, 0x3f, 0x20, 0, kScene5030GladysSpeechGroup);
}

void Scene5030::copyStageSmallRow(byte destinationRow, byte sourceRow) {
	const uint destinationOffset = destinationRow * kStage003SmallRowSize;
	const uint sourceOffset = sourceRow * kStage003SmallRowSize;
	if (destinationOffset + kStage003SmallRowSize > _stage003SmallRows.size() ||
			sourceOffset + kStage003SmallRowSize > _stage003SmallRows.size())
		return;

	memcpy(_stage003SmallRows.data() + destinationOffset,
		_stage003SmallRows.data() + sourceOffset, kStage003SmallRowSize);
}

void Scene5030::clearSceneItemFromColorMap(byte itemId) {
	if (_paletteMask.size() < kSceneColorToItemMap + kScenePaletteMapPageSize)
		return;

	for (uint color = 0; color < kScenePaletteMapPageSize; ++color) {
		if (_paletteMask[kSceneColorToItemMap + color] == itemId)
			_paletteMask[kSceneColorToItemMap + color] = 0;
	}
}

} // End of namespace Hollywood
