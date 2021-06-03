/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "twine/scene/gamestate.h"
#include "common/file.h"
#include "common/rect.h"
#include "common/str.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "common/util.h"
#include "twine/audio/music.h"
#include "twine/audio/sound.h"
#include "twine/input.h"
#include "twine/menu/interface.h"
#include "twine/menu/menu.h"
#include "twine/menu/menuoptions.h"
#include "twine/renderer/redraw.h"
#include "twine/renderer/renderer.h"
#include "twine/renderer/screens.h"
#include "twine/resources/resources.h"
#include "twine/scene/actor.h"
#include "twine/scene/animations.h"
#include "twine/scene/collision.h"
#include "twine/scene/extra.h"
#include "twine/scene/grid.h"
#include "twine/scene/scene.h"
#include "twine/shared.h"
#include "twine/text.h"
#include "twine/twine.h"

namespace TwinE {

GameState::GameState(TwinEEngine *engine) : _engine(engine) {
	clearGameFlags();
	Common::fill(&inventoryFlags[0], &inventoryFlags[NUM_INVENTORY_ITEMS], 0);
	Common::fill(&holomapFlags[0], &holomapFlags[NUM_LOCATIONS], 0);
	Common::fill(&gameChoices[0], &gameChoices[10], TextId::kNone);
}

void GameState::initEngineProjections() {
	_engine->_renderer->setOrthoProjection(311, 240, 512);
	_engine->_renderer->setBaseTranslation(0, 0, 0);
	_engine->_renderer->setBaseRotation(ANGLE_0, ANGLE_0, ANGLE_0);
	_engine->_renderer->setLightVector(_engine->_scene->alphaLight, _engine->_scene->betaLight, ANGLE_0);
}

void GameState::initGameStateVars() {
	debug(2, "Init game state variables");
	_engine->_extra->resetExtras();

	for (int32 i = 0; i < OVERLAY_MAX_ENTRIES; i++) {
		_engine->_redraw->overlayList[i].info0 = -1;
	}

	for (int32 i = 0; i < ARRAYSIZE(_engine->_scene->sceneFlags); i++) {
		_engine->_scene->sceneFlags[i] = 0;
	}

	clearGameFlags();
	Common::fill(&inventoryFlags[0], &inventoryFlags[NUM_INVENTORY_ITEMS], 0);

	_engine->_scene->initSceneVars();

	Common::fill(&holomapFlags[0], &holomapFlags[NUM_LOCATIONS], 0);
}

void GameState::initHeroVars() {
	_engine->_actor->resetActor(OWN_ACTOR_SCENE_INDEX); // reset Hero

	magicBallIdx = -1;

	inventoryNumLeafsBox = 2;
	inventoryNumLeafs = 2;
	inventoryNumKashes = 0;
	inventoryNumKeys = 0;
	inventoryMagicPoints = 0;

	usingSabre = false;

	_engine->_scene->sceneHero->body = BodyType::btNormal;
	_engine->_scene->sceneHero->setLife(kActorMaxLife);
	_engine->_scene->sceneHero->talkColor = COLOR_BRIGHT_BLUE;
}

void GameState::initEngineVars() {
	debug(2, "Init engine variables");
	_engine->_interface->resetClip();

	_engine->_scene->alphaLight = ANGLE_315;
	_engine->_scene->betaLight = ANGLE_334;
	initEngineProjections();
	initGameStateVars();
	initHeroVars();

	_engine->_scene->newHeroPos.x = 0x2000;
	_engine->_scene->newHeroPos.y = 0x1800;
	_engine->_scene->newHeroPos.z = 0x2000;

	_engine->_scene->currentSceneIdx = -1;
	_engine->_scene->needChangeScene = LBA1SceneId::Citadel_Island_Prison;
	_engine->quitGame = -1;
	_engine->_scene->mecaPinguinIdx = -1;
	_engine->_menuOptions->canShowCredits = false;

	inventoryNumLeafs = 0;
	inventoryNumLeafsBox = 2;
	inventoryMagicPoints = 0;
	inventoryNumKashes = 0;
	inventoryNumKeys = 0;
	inventoryNumGas = 0;

	_engine->_actor->cropBottomScreen = 0;

	magicLevelIdx = 0;
	usingSabre = false;

	gameChapter = 0;

	_engine->_scene->sceneTextBank = TextBankId::Options_and_menus;
	_engine->_scene->currentlyFollowedActor = OWN_ACTOR_SCENE_INDEX;
	_engine->_actor->heroBehaviour = HeroBehaviourType::kNormal;
	_engine->_actor->previousHeroAngle = 0;
	_engine->_actor->previousHeroBehaviour = HeroBehaviourType::kNormal;
}

// http://lbafileinfo.kazekr.net/index.php?title=LBA1:Savegame
bool GameState::loadGame(Common::SeekableReadStream *file) {
	if (file == nullptr) {
		return false;
	}

	debug(2, "Load game");
	const byte saveFileVersion = file->readByte();
	// 4 is dotemu enhanced version of lba1
	if (saveFileVersion != 3 && saveFileVersion != 4) {
		warning("Could not load savegame - wrong magic byte");
		return false;
	}

	initEngineVars();

	int playerNameIdx = 0;
	do {
		const byte c = file->readByte();
		_engine->_menuOptions->saveGameName[playerNameIdx++] = c;
		if (c == '\0') {
			break;
		}
		if (playerNameIdx >= ARRAYSIZE(_engine->_menuOptions->saveGameName)) {
			warning("Failed to load savegame. Invalid playername.");
			return false;
		}
	} while (true);

	byte numGameFlags = file->readByte();
	if (numGameFlags != NUM_GAME_FLAGS) {
		warning("Failed to load gameflags. Expected %u, but got %u", NUM_GAME_FLAGS, numGameFlags);
		return false;
	}
	for (uint8 i = 0; i < numGameFlags; ++i) {
		setGameFlag(i, file->readByte());
	}
	_engine->_scene->needChangeScene = file->readByte(); // scene index
	gameChapter = file->readByte();

	_engine->_actor->heroBehaviour = (HeroBehaviourType)file->readByte();
	_engine->_actor->previousHeroBehaviour = _engine->_actor->heroBehaviour;
	_engine->_scene->sceneHero->setLife(file->readByte());
	setKashes(file->readSint16LE());
	magicLevelIdx = file->readByte();
	setMagicPoints(file->readByte());
	setLeafBoxes(file->readByte());
	_engine->_scene->newHeroPos.x = file->readSint16LE();
	_engine->_scene->newHeroPos.y = file->readSint16LE();
	_engine->_scene->newHeroPos.z = file->readSint16LE();
	_engine->_scene->sceneHero->angle = ToAngle(file->readSint16LE());
	_engine->_actor->previousHeroAngle = _engine->_scene->sceneHero->angle;
	_engine->_scene->sceneHero->body = (BodyType)file->readByte();

	const byte numHolomapFlags = file->readByte(); // number of holomap locations
	if (numHolomapFlags != NUM_LOCATIONS) {
		warning("Failed to load holomapflags. Got %u, expected %i", numHolomapFlags, NUM_LOCATIONS);
		return false;
	}
	file->read(holomapFlags, NUM_LOCATIONS);

	setGas(file->readByte());

	const byte numInventoryFlags = file->readByte(); // number of used inventory items, always 28
	if (numInventoryFlags != NUM_INVENTORY_ITEMS) {
		warning("Failed to load inventoryFlags. Got %u, expected %i", numInventoryFlags, NUM_INVENTORY_ITEMS);
		return false;
	}
	file->read(inventoryFlags, NUM_INVENTORY_ITEMS);

	setLeafs(file->readByte());
	usingSabre = file->readByte();

	if (saveFileVersion == 4) {
		// the time the game was played
		file->readUint32LE();
		file->readUint32LE();
	}

	_engine->_scene->currentSceneIdx = -1;
	_engine->_scene->heroPositionType = ScenePositionType::kReborn;
	return true;
}

bool GameState::saveGame(Common::WriteStream *file) {
	debug(2, "Save game");
	if (_engine->_menuOptions->saveGameName[0] == '\0') {
		Common::strlcpy(_engine->_menuOptions->saveGameName, "TwinEngineSave", sizeof(_engine->_menuOptions->saveGameName));
	}

	int32 sceneIdx = _engine->_scene->currentSceneIdx;
	if (sceneIdx == Polar_Island_end_scene || sceneIdx == Citadel_Island_end_sequence_1 || sceneIdx == Citadel_Island_end_sequence_2 || sceneIdx == Credits_List_Sequence) {
		/* inventoryMagicPoints = 0x50 */
		/* herobehaviour = 0 */
		/* newheropos.x = 0xffff */
		sceneIdx = Polar_Island_Final_Battle;
	}

	file->writeByte(0x03);
	file->writeString(_engine->_menuOptions->saveGameName);
	file->writeByte('\0');
	file->writeByte(NUM_GAME_FLAGS);
	for (uint8 i = 0; i < NUM_GAME_FLAGS; ++i) {
		file->writeByte(hasGameFlag(i));
	}
	file->writeByte(sceneIdx);
	file->writeByte(gameChapter);
	file->writeByte((byte)_engine->_actor->heroBehaviour);
	file->writeByte(_engine->_scene->sceneHero->life);
	file->writeSint16LE(inventoryNumKashes);
	file->writeByte(magicLevelIdx);
	file->writeByte(inventoryMagicPoints);
	file->writeByte(inventoryNumLeafsBox);
	// we don't save the whole scene state - so we have to make sure that the hero is
	// respawned at the start of the scene - and not at its current position
	file->writeSint16LE(_engine->_scene->newHeroPos.x);
	file->writeSint16LE(_engine->_scene->newHeroPos.y);
	file->writeSint16LE(_engine->_scene->newHeroPos.z);
	file->writeSint16LE(FromAngle(_engine->_scene->sceneHero->angle));
	file->writeByte((uint8)_engine->_scene->sceneHero->body);

	// number of holomap locations
	file->writeByte(NUM_LOCATIONS);
	file->write(holomapFlags, NUM_LOCATIONS);

	file->writeByte(inventoryNumGas);

	// number of inventory items
	file->writeByte(NUM_INVENTORY_ITEMS);
	file->write(inventoryFlags, NUM_INVENTORY_ITEMS);

	file->writeByte(inventoryNumLeafs);
	// TODO: writeUInt16LE in disassembly
	file->writeByte(usingSabre ? 1 : 0);
	file->writeByte(0);

	return true;
}

void GameState::setGameFlag(uint8 index, uint8 value) {
	debug(2, "Set gameStateFlags[%u]=%u", index, value);
	_gameStateFlags[index] = value;

	// all 4 slap videos
	if ((index == GAMEFLAG_VIDEO_BAFFE || index == GAMEFLAG_VIDEO_BAFFE2 || index == GAMEFLAG_VIDEO_BAFFE3 || index == GAMEFLAG_VIDEO_BAFFE5) &&
		_gameStateFlags[GAMEFLAG_VIDEO_BAFFE] != 0 && _gameStateFlags[GAMEFLAG_VIDEO_BAFFE2] != 0 && _gameStateFlags[GAMEFLAG_VIDEO_BAFFE3] != 0 && _gameStateFlags[GAMEFLAG_VIDEO_BAFFE5] != 0) {
		_engine->unlockAchievement("LBA_ACH_012");
	}
	// second video of ferry trip
	if (index == GAMEFLAG_VIDEO_BATEAU2) {
		_engine->unlockAchievement("LBA_ACH_010");
	}
	if (index == (uint8)InventoryItems::kiUseSabre) {
		_engine->unlockAchievement("LBA_ACH_002");
	}
	if (index == (uint8)InventoryItems::kBottleOfSyrup) {
		_engine->unlockAchievement("LBA_ACH_007");
	}
}

void GameState::processFoundItem(InventoryItems item) {
	ScopedEngineFreeze freeze(_engine);
	_engine->_grid->centerOnActor(_engine->_scene->sceneHero);

	_engine->exitSceneryView();
	// Hide hero in scene
	_engine->_scene->sceneHero->staticFlags.bIsHidden = 1;
	_engine->_redraw->redrawEngineActions(true);
	_engine->_scene->sceneHero->staticFlags.bIsHidden = 0;

	_engine->_screens->copyScreen(_engine->frontVideoBuffer, _engine->workVideoBuffer);

	const int32 itemCameraX = _engine->_grid->newCamera.x * BRICK_SIZE;
	const int32 itemCameraY = _engine->_grid->newCamera.y * BRICK_HEIGHT;
	const int32 itemCameraZ = _engine->_grid->newCamera.z * BRICK_SIZE;

	BodyData &bodyData = _engine->_resources->bodyData[_engine->_scene->sceneHero->entity];
	const int32 bodyX = _engine->_scene->sceneHero->pos.x - itemCameraX;
	const int32 bodyY = _engine->_scene->sceneHero->pos.y - itemCameraY;
	const int32 bodyZ = _engine->_scene->sceneHero->pos.z - itemCameraZ;
	_engine->_renderer->renderIsoModel(bodyX, bodyY, bodyZ, ANGLE_0, ANGLE_45, ANGLE_0, bodyData);
	_engine->_interface->setClip(_engine->_redraw->renderRect);

	const int32 itemX = (_engine->_scene->sceneHero->pos.x + BRICK_HEIGHT) / BRICK_SIZE;
	int32 itemY = _engine->_scene->sceneHero->pos.y / BRICK_HEIGHT;
	if (_engine->_scene->sceneHero->brickShape() != ShapeType::kNone) {
		itemY++;
	}
	const int32 itemZ = (_engine->_scene->sceneHero->pos.z + BRICK_HEIGHT) / BRICK_SIZE;

	_engine->_grid->drawOverModelActor(itemX, itemY, itemZ);

	_engine->_renderer->projectPositionOnScreen(bodyX, bodyY, bodyZ);
	_engine->_renderer->projPos.y -= 150;

	const int32 boxTopLeftX = _engine->_renderer->projPos.x - 65;
	const int32 boxTopLeftY = _engine->_renderer->projPos.y - 65;
	const int32 boxBottomRightX = _engine->_renderer->projPos.x + 65;
	const int32 boxBottomRightY = _engine->_renderer->projPos.y + 65;
	const Common::Rect boxRect(boxTopLeftX, boxTopLeftY, boxBottomRightX, boxBottomRightY);
	_engine->_sound->playSample(Samples::BigItemFound);

	// process vox play
	_engine->_music->stopMusic();
	_engine->_text->initTextBank(TextBankId::Inventory_Intro_and_Holomap);

	_engine->_interface->resetClip();
	_engine->_text->initItemFoundText(item);
	_engine->_text->initDialogueBox();

	ProgressiveTextState textState = ProgressiveTextState::ContinueRunning;

	_engine->_text->initVoxToPlayTextId((TextId)item);

	const int32 bodyAnimIdx = _engine->_animations->getBodyAnimIndex(AnimationTypes::kFoundItem);
	const AnimData &currentAnimData = _engine->_resources->animData[bodyAnimIdx];

	AnimTimerDataStruct tmpAnimTimer = _engine->_scene->sceneHero->animTimerData;

	_engine->_animations->stockAnimation(bodyData, &_engine->_scene->sceneHero->animTimerData);

	uint currentAnimState = 0;

	_engine->_redraw->numOfRedrawBox = 0;

	ScopedKeyMap uiKeyMap(_engine, uiKeyMapId);
	for (;;) {
		FrameMarker frame(_engine, 66);
		_engine->_interface->resetClip();
		_engine->_redraw->currNumOfRedrawBox = 0;
		_engine->_redraw->blitBackgroundAreas();
		_engine->_interface->drawTransparentBox(boxRect, 4);

		_engine->_interface->setClip(boxRect);

		_engine->_menu->itemAngle[item] += ANGLE_2;

		_engine->_renderer->renderInventoryItem(_engine->_renderer->projPos.x, _engine->_renderer->projPos.y, _engine->_resources->inventoryTable[item], _engine->_menu->itemAngle[item], 10000);

		_engine->_menu->drawBox(boxRect);
		_engine->_redraw->addRedrawArea(boxRect);
		_engine->_interface->resetClip();
		initEngineProjections();

		if (_engine->_animations->setModelAnimation(currentAnimState, currentAnimData, bodyData, &_engine->_scene->sceneHero->animTimerData)) {
			currentAnimState++; // keyframe
			if (currentAnimState >= currentAnimData.getNumKeyframes()) {
				currentAnimState = currentAnimData.getLoopFrame();
			}
		}

		_engine->_renderer->renderIsoModel(bodyX, bodyY, bodyZ, ANGLE_0, ANGLE_45, ANGLE_0, bodyData);
		_engine->_interface->setClip(_engine->_redraw->renderRect);
		_engine->_grid->drawOverModelActor(itemX, itemY, itemZ);
		_engine->_redraw->addRedrawArea(_engine->_redraw->renderRect);

		if (textState == ProgressiveTextState::ContinueRunning) {
			_engine->_interface->resetClip();
			textState = _engine->_text->updateProgressiveText();
		} else {
			_engine->_text->fadeInRemainingChars();
		}

		_engine->_redraw->flipRedrawAreas();

		_engine->readKeys();
		if (_engine->_input->toggleAbortAction()) {
			_engine->_text->stopVox(_engine->_text->currDialTextEntry);
			break;
		}

		if (_engine->_input->toggleActionIfActive(TwinEActionType::UINextPage)) {
			if (textState == ProgressiveTextState::End) {
				_engine->_text->stopVox(_engine->_text->currDialTextEntry);
				break;
			}
			if (textState == ProgressiveTextState::NextPage) {
				textState = ProgressiveTextState::ContinueRunning;
			}
		}

		_engine->_text->playVoxSimple(_engine->_text->currDialTextEntry);

		_engine->lbaTime++;
	}

	while (_engine->_text->playVoxSimple(_engine->_text->currDialTextEntry)) {
		FrameMarker frame(_engine);
		_engine->readKeys();
		if (_engine->shouldQuit() || _engine->_input->toggleAbortAction()) {
			break;
		}
	}

	initEngineProjections();
	_engine->_text->initSceneTextBank();
	_engine->_text->stopVox(_engine->_text->currDialTextEntry);

	_engine->_scene->sceneHero->animTimerData = tmpAnimTimer;
}

void GameState::processGameChoices(TextId choiceIdx) {
	_engine->_screens->copyScreen(_engine->frontVideoBuffer, _engine->workVideoBuffer);

	_gameChoicesSettings.reset();
	_gameChoicesSettings.setTextBankId((TextBankId)((int)_engine->_scene->sceneTextBank + (int)TextBankId::Citadel_Island));

	// filled via script
	for (int32 i = 0; i < numChoices; i++) {
		_gameChoicesSettings.addButton(gameChoices[i], 0);
	}

	_engine->_text->drawAskQuestion(choiceIdx);

	_engine->_menu->processMenu(&_gameChoicesSettings, false);
	const int16 activeButton = _gameChoicesSettings.getActiveButton();
	choiceAnswer = gameChoices[activeButton];

	// get right VOX entry index
	if (_engine->_text->initVoxToPlayTextId(choiceAnswer)) {
		while (_engine->_text->playVoxSimple(_engine->_text->currDialTextEntry)) {
			FrameMarker frame(_engine);
			if (_engine->shouldQuit()) {
				break;
			}
		}
		_engine->_text->stopVox(_engine->_text->currDialTextEntry);

		_engine->_text->hasHiddenVox = false;
		_engine->_text->voxHiddenIndex = 0;
	}
}

void GameState::processGameoverAnimation() {
	const int32 tmpLbaTime = _engine->lbaTime;

	_engine->exitSceneryView();
	// workaround to fix hero redraw after drowning
	_engine->_scene->sceneHero->staticFlags.bIsHidden = 1;
	_engine->_redraw->redrawEngineActions(true);
	_engine->_scene->sceneHero->staticFlags.bIsHidden = 0;

	// TODO: inSceneryView
	_engine->setPalette(_engine->_screens->paletteRGBA);
	_engine->_screens->copyScreen(_engine->frontVideoBuffer, _engine->workVideoBuffer);
	BodyData gameOverPtr;
	if (!gameOverPtr.loadFromHQR(Resources::HQR_RESS_FILE, RESSHQR_GAMEOVERMDL)) {
		return;
	}

	_engine->_sound->stopSamples();
	_engine->_music->stopMidiMusic(); // stop fade music
	_engine->_renderer->setCameraPosition(_engine->width() / 2, _engine->height() / 2, 128, 200, 200);
	int32 startLbaTime = _engine->lbaTime;
	const Common::Rect &rect = _engine->centerOnScreen(_engine->width() / 2, _engine->height() / 2);
	_engine->_interface->setClip(rect);

	while (!_engine->_input->toggleAbortAction() && (_engine->lbaTime - startLbaTime) <= 500) {
		FrameMarker frame(_engine, 66);
		_engine->readKeys();
		if (_engine->shouldQuit()) {
			return;
		}

		const int32 avg = _engine->_collision->getAverageValue(40000, 3200, 500, _engine->lbaTime - startLbaTime);
		const int32 cdot = _engine->_screens->crossDot(1, 1024, 100, (_engine->lbaTime - startLbaTime) % 100);

		_engine->_interface->blitBox(rect, _engine->workVideoBuffer, _engine->frontVideoBuffer);
		_engine->_renderer->setCameraAngle(0, 0, 0, 0, -cdot, 0, avg);
		_engine->_renderer->renderIsoModel(0, 0, 0, ANGLE_0, ANGLE_0, ANGLE_0, gameOverPtr);
		_engine->copyBlockPhys(rect);

		_engine->lbaTime++;
	}

	_engine->_sound->playSample(Samples::Explode);
	_engine->_interface->blitBox(rect, _engine->workVideoBuffer, _engine->frontVideoBuffer);
	_engine->_renderer->setCameraAngle(0, 0, 0, 0, 0, 0, 3200);
	_engine->_renderer->renderIsoModel(0, 0, 0, ANGLE_0, ANGLE_0, ANGLE_0, gameOverPtr);
	_engine->copyBlockPhys(rect);

	_engine->delaySkip(2000);

	_engine->_interface->resetClip();
	_engine->_screens->copyScreen(_engine->workVideoBuffer, _engine->frontVideoBuffer);
	initEngineProjections();

	_engine->lbaTime = tmpLbaTime;
}

void GameState::giveUp() {
	_engine->_sound->stopSamples();
	// TODO: is an autosave desired on giving up? I don't think so. What did the original game do here?
	//_engine->autoSave();
	initGameStateVars();
	_engine->_scene->stopRunningGame();
}

int16 GameState::setGas(int16 value) {
	inventoryNumGas = CLIP<int16>(value, 0, 100);
	return inventoryNumGas;
}

void GameState::addGas(int16 value) {
	setGas(inventoryNumGas + value);
}

int16 GameState::setKashes(int16 value) {
	inventoryNumKashes = CLIP<int16>(value, 0, 999);
	if (_engine->_gameState->inventoryNumKashes >= 500) {
		_engine->unlockAchievement("LBA_ACH_011");
	}
	return inventoryNumKashes;
}

int16 GameState::setKeys(int16 value) {
	inventoryNumKeys = MAX<int16>(0, value);
	return inventoryNumKeys;
}

void GameState::addKeys(int16 val) {
	setKeys(inventoryNumKeys + val);
}

void GameState::addKashes(int16 val) {
	setKashes(inventoryNumKashes + val);
}

int16 GameState::setMagicPoints(int16 val) {
	inventoryMagicPoints = val;
	if (inventoryMagicPoints > magicLevelIdx * 20) {
		inventoryMagicPoints = magicLevelIdx * 20;
	} else if (inventoryMagicPoints < 0) {
		inventoryMagicPoints = 0;
	}
	return inventoryMagicPoints;
}

int16 GameState::setMaxMagicPoints() {
	inventoryMagicPoints = magicLevelIdx * 20;
	return inventoryMagicPoints;
}

void GameState::addMagicPoints(int16 val) {
	setMagicPoints(inventoryMagicPoints + val);
}

int16 GameState::setLeafs(int16 val) {
	inventoryNumLeafs = val;
	if (inventoryNumLeafs > inventoryNumLeafsBox) {
		inventoryNumLeafs = inventoryNumLeafsBox;
	}
	return inventoryNumLeafs;
}

void GameState::addLeafs(int16 val) {
	setLeafs(inventoryNumLeafs + val);
}

int16 GameState::setLeafBoxes(int16 val) {
	inventoryNumLeafsBox = val;
	if (inventoryNumLeafsBox > 10) {
		inventoryNumLeafsBox = 10;
	}
	if (inventoryNumLeafsBox == 5) {
		_engine->unlockAchievement("LBA_ACH_003");
	}
	return inventoryNumLeafsBox;
}

void GameState::addLeafBoxes(int16 val) {
	setLeafBoxes(inventoryNumLeafsBox + val);
}

} // namespace TwinE
