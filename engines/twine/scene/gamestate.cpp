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
	Common::fill(&gameFlags[0], &gameFlags[256], 0);
	Common::fill(&inventoryFlags[0], &inventoryFlags[NUM_INVENTORY_ITEMS], 0);
	Common::fill(&holomapFlags[0], &holomapFlags[NUM_LOCATIONS], 0);
	playerName[0] = '\0';
	Common::fill(&gameChoices[0], &gameChoices[10], 0);
}

void GameState::initEngineProjections() {
	_engine->_renderer->setOrthoProjection(311, 240, 512);
	_engine->_renderer->setBaseTranslation(0, 0, 0);
	_engine->_renderer->setBaseRotation(0, 0, 0);
	_engine->_renderer->setLightVector(_engine->_scene->alphaLight, _engine->_scene->betaLight, 0);
}

void GameState::initGameStateVars() {
	_engine->_extra->resetExtras();

	for (int32 i = 0; i < OVERLAY_MAX_ENTRIES; i++) {
		_engine->_redraw->overlayList[i].info0 = -1;
	}

	for (int32 i = 0; i < ARRAYSIZE(_engine->_scene->sceneFlags); i++) {
		_engine->_scene->sceneFlags[i] = 0;
	}

	for (int32 i = 0; i < ARRAYSIZE(gameFlags); i++) {
		gameFlags[i] = 0;
	}

	for (int32 i = 0; i < ARRAYSIZE(inventoryFlags); i++) {
		inventoryFlags[i] = 0;
	}

	_engine->_scene->initSceneVars();

	for (int32 i = 0; i < ARRAYSIZE(holomapFlags); i++) {
		holomapFlags[i] = 0;
	}

	_engine->_actor->clearBodyTable();
}

void GameState::initHeroVars() {
	_engine->_actor->resetActor(0); // reset Hero

	magicBallIdx = -1;

	inventoryNumLeafsBox = 2;
	inventoryNumLeafs = 2;
	inventoryNumKashes = 0;
	inventoryNumKeys = 0;
	inventoryMagicPoints = 0;

	usingSabre = false;

	_engine->_scene->sceneHero->body = 0;
	_engine->_scene->sceneHero->life = 50;
	_engine->_scene->sceneHero->talkColor = 4;
}

void GameState::initEngineVars() {
	_engine->_interface->resetClip();

	_engine->_scene->alphaLight = 896;
	_engine->_scene->betaLight = 950;
	initEngineProjections();
	initGameStateVars();
	initHeroVars();

	_engine->_scene->newHeroX = 0x2000;
	_engine->_scene->newHeroY = 0x1800;
	_engine->_scene->newHeroZ = 0x2000;

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

	if (file->readByte() != 0x03) {
		return false;
	}

	initEngineVars();

	int playerNameIdx = 0;
	do {
		const byte c = file->readByte();
		playerName[playerNameIdx++] = c;
		if (c == '\0') {
			break;
		}
		if (playerNameIdx >= ARRAYSIZE(playerName)) {
			warning("Failed to load savegame. Invalid playername.");
			return false;
		}
	} while (true);

	byte numGameFlags = file->readByte();
	if (numGameFlags != NUM_GAME_FLAGS) {
		warning("Failed to load gameflags. Expected %u, but got %u", NUM_GAME_FLAGS, numGameFlags);
		return false;
	}
	file->read(gameFlags, NUM_GAME_FLAGS);
	_engine->_scene->needChangeScene = file->readByte(); // scene index
	gameChapter = file->readByte();

	_engine->_actor->heroBehaviour = (HeroBehaviourType)file->readByte();
	_engine->_actor->previousHeroBehaviour = _engine->_actor->heroBehaviour;
	_engine->_scene->sceneHero->life = file->readByte();
	inventoryNumKashes = file->readSint16LE();
	magicLevelIdx = file->readByte();
	inventoryMagicPoints = file->readByte();
	inventoryNumLeafsBox = file->readByte();
	_engine->_scene->newHeroX = file->readSint16LE();
	_engine->_scene->newHeroY = file->readSint16LE();
	_engine->_scene->newHeroZ = file->readSint16LE();
	_engine->_scene->sceneHero->angle = ToAngle(file->readSint16LE());
	_engine->_actor->previousHeroAngle = _engine->_scene->sceneHero->angle;
	_engine->_scene->sceneHero->body = file->readByte();

	const byte numHolomapFlags = file->readByte(); // number of holomap locations
	if (numHolomapFlags != NUM_LOCATIONS) {
		warning("Failed to load holomapflags. Got %u, expected %i", numHolomapFlags, NUM_LOCATIONS);
		return false;
	}
	file->read(holomapFlags, NUM_LOCATIONS);

	inventoryNumGas = file->readByte();

	const byte numInventoryFlags = file->readByte(); // number of used inventory items, always 28
	if (numInventoryFlags != NUM_INVENTORY_ITEMS) {
		warning("Failed to load inventoryFlags. Got %u, expected %i", numInventoryFlags, NUM_INVENTORY_ITEMS);
		return false;
	}
	file->read(inventoryFlags, NUM_INVENTORY_ITEMS);

	inventoryNumLeafs = file->readByte();
	usingSabre = file->readByte();

	_engine->_scene->currentSceneIdx = -1;
	_engine->_scene->heroPositionType = ScenePositionType::kReborn;
	return true;
}

bool GameState::saveGame(Common::WriteStream *file) {
	if (playerName[0] == '\0') {
		Common::strlcpy(playerName, "TwinEngineSave", sizeof(playerName));
	}

	file->writeByte(0x03);
	file->writeString(playerName);
	file->writeByte('\0');
	file->writeByte(NUM_GAME_FLAGS);
	file->write(gameFlags, NUM_GAME_FLAGS);
	file->writeByte(_engine->_scene->currentSceneIdx);
	file->writeByte(gameChapter);
	file->writeByte((byte)_engine->_actor->heroBehaviour);
	file->writeByte(_engine->_scene->sceneHero->life);
	file->writeSint16LE(inventoryNumKashes);
	file->writeByte(magicLevelIdx);
	file->writeByte(inventoryMagicPoints);
	file->writeByte(inventoryNumLeafsBox);
	// we don't save the whole scene state - so we have to make sure that the hero is
	// respawned at the start of the scene - and not at its current position
	file->writeSint16LE(_engine->_scene->newHeroX);
	file->writeSint16LE(_engine->_scene->newHeroY);
	file->writeSint16LE(_engine->_scene->newHeroZ);
	file->writeSint16LE(FromAngle(_engine->_scene->sceneHero->angle));
	file->writeByte(_engine->_scene->sceneHero->body);

	// number of holomap locations
	file->writeByte(NUM_LOCATIONS);
	file->write(holomapFlags, NUM_LOCATIONS);

	file->writeByte(inventoryNumGas);

	// number of inventory items
	file->writeByte(NUM_INVENTORY_ITEMS);
	file->write(inventoryFlags, NUM_INVENTORY_ITEMS);

	file->writeByte(inventoryNumLeafs);
	file->writeByte(usingSabre ? 1 : 0);
	file->writeByte(0);

	return true;
}

void GameState::processFoundItem(int32 item) {
	_engine->_grid->newCameraX = (_engine->_scene->sceneHero->x + BRICK_HEIGHT) / BRICK_SIZE;
	_engine->_grid->newCameraY = (_engine->_scene->sceneHero->y + BRICK_HEIGHT) / BRICK_HEIGHT;
	_engine->_grid->newCameraZ = (_engine->_scene->sceneHero->z + BRICK_HEIGHT) / BRICK_SIZE;

	// Hide hero in scene
	_engine->_scene->sceneHero->staticFlags.bIsHidden = 1;
	_engine->_redraw->redrawEngineActions(true);
	_engine->_scene->sceneHero->staticFlags.bIsHidden = 0;

	_engine->_screens->copyScreen(_engine->frontVideoBuffer, _engine->workVideoBuffer);

	const int32 itemCameraX = _engine->_grid->newCameraX * BRICK_SIZE;
	const int32 itemCameraY = _engine->_grid->newCameraY * BRICK_HEIGHT;
	const int32 itemCameraZ = _engine->_grid->newCameraZ * BRICK_SIZE;

	_engine->_renderer->renderIsoModel(_engine->_scene->sceneHero->x - itemCameraX, _engine->_scene->sceneHero->y - itemCameraY, _engine->_scene->sceneHero->z - itemCameraZ, 0, 0x80, 0, _engine->_actor->bodyTable[_engine->_scene->sceneHero->entity]);
	_engine->_interface->setClip(_engine->_redraw->renderRect);

	const int32 itemX = (_engine->_scene->sceneHero->x + BRICK_HEIGHT) / BRICK_SIZE;
	int32 itemY = _engine->_scene->sceneHero->y / BRICK_HEIGHT;
	if (_engine->_scene->sceneHero->brickShape() != ShapeType::kNone) {
		itemY++;
	}
	const int32 itemZ = (_engine->_scene->sceneHero->z + BRICK_HEIGHT) / BRICK_SIZE;

	_engine->_grid->drawOverModelActor(itemX, itemY, itemZ);
	_engine->flip();

	_engine->_renderer->projectPositionOnScreen(_engine->_scene->sceneHero->x - itemCameraX, _engine->_scene->sceneHero->y - itemCameraY, _engine->_scene->sceneHero->z - itemCameraZ);
	_engine->_renderer->projPosY -= 150;

	const int32 boxTopLeftX = _engine->_renderer->projPosX - 65;
	const int32 boxTopLeftY = _engine->_renderer->projPosY - 65;
	const int32 boxBottomRightX = _engine->_renderer->projPosX + 65;
	const int32 boxBottomRightY = _engine->_renderer->projPosY + 65;
	const Common::Rect boxRect(boxTopLeftX, boxTopLeftY, boxBottomRightX, boxBottomRightY);
	_engine->_sound->playSample(Samples::BigItemFound);

	// process vox play
	_engine->_music->stopMusic();
	_engine->_text->initTextBank(TextBankId::Inventory_Intro_and_Holomap);

	_engine->_interface->resetClip();
	_engine->_text->initItemFoundText(item);
	_engine->_text->initDialogueBox();

	ProgressiveTextState textState = ProgressiveTextState::ContinueRunning;

	_engine->_text->initVoxToPlay(item);

	const int32 bodyAnimIdx = _engine->_animations->getBodyAnimIndex(AnimationTypes::kFoundItem);
	uint8 *currentAnim = _engine->_resources->animTable[bodyAnimIdx];

	AnimTimerDataStruct tmpAnimTimer = _engine->_scene->sceneHero->animTimerData;

	_engine->_animations->stockAnimation(_engine->_actor->bodyTable[_engine->_scene->sceneHero->entity], &_engine->_scene->sceneHero->animTimerData);

	int32 currentAnimState = 0;

	Renderer::prepareIsoModel(_engine->_resources->inventoryTable[item]);
	_engine->_redraw->numOfRedrawBox = 0;

	ScopedKeyMap uiKeyMap(_engine, uiKeyMapId);
	for (;;) {
		ScopedFPS fps(66);
		_engine->_interface->resetClip();
		_engine->_redraw->currNumOfRedrawBox = 0;
		_engine->_redraw->blitBackgroundAreas();
		_engine->_interface->drawTransparentBox(boxRect, 4);

		_engine->_interface->setClip(boxRect);

		_engine->_menu->itemAngle[item] += 8;

		_engine->_renderer->renderInventoryItem(_engine->_renderer->projPosX, _engine->_renderer->projPosY, _engine->_resources->inventoryTable[item], _engine->_menu->itemAngle[item], 10000);

		_engine->_menu->drawBox(boxRect);
		_engine->_redraw->addRedrawArea(boxRect);
		_engine->_interface->resetClip();
		initEngineProjections();

		if (_engine->_animations->setModelAnimation(currentAnimState, currentAnim, _engine->_actor->bodyTable[_engine->_scene->sceneHero->entity], &_engine->_scene->sceneHero->animTimerData)) {
			currentAnimState++; // keyframe
			if (currentAnimState >= _engine->_animations->getNumKeyframes(currentAnim)) {
				currentAnimState = _engine->_animations->getStartKeyframe(currentAnim);
			}
		}

		_engine->_renderer->renderIsoModel(_engine->_scene->sceneHero->x - itemCameraX, _engine->_scene->sceneHero->y - itemCameraY, _engine->_scene->sceneHero->z - itemCameraZ, ANGLE_0, ANGLE_45, ANGLE_0, _engine->_actor->bodyTable[_engine->_scene->sceneHero->entity]);
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
		ScopedFPS scopedFps;
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

void GameState::processGameChoices(int32 choiceIdx) {
	_engine->_screens->copyScreen(_engine->frontVideoBuffer, _engine->workVideoBuffer);

	gameChoicesSettings.reset();
	gameChoicesSettings.setTextBankId(_engine->_scene->sceneTextBank + TextBankId::Citadel_Island);

	// filled via script
	for (int32 i = 0; i < numChoices; i++) {
		gameChoicesSettings.addButton(gameChoices[i], 0);
	}

	_engine->_text->drawAskQuestion(choiceIdx);

	_engine->_menu->processMenu(&gameChoicesSettings);
	const int16 activeButton = gameChoicesSettings.getActiveButton();
	choiceAnswer = gameChoices[activeButton];

	// get right VOX entry index
	if (_engine->_text->initVoxToPlay(choiceAnswer)) {
		while (_engine->_text->playVoxSimple(_engine->_text->currDialTextEntry)) {
			ScopedFPS scopedFps;
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
	int32 tmpLbaTime = _engine->lbaTime;

	// workaround to fix hero redraw after drowning
	_engine->_scene->sceneHero->staticFlags.bIsHidden = 1;
	_engine->_redraw->redrawEngineActions(true);
	_engine->_scene->sceneHero->staticFlags.bIsHidden = 0;

	// TODO: drawInGameTransBox
	_engine->setPalette(_engine->_screens->paletteRGBA);
	_engine->_screens->copyScreen(_engine->frontVideoBuffer, _engine->workVideoBuffer);
	uint8 *gameOverPtr = nullptr;
	if (HQR::getAllocEntry(&gameOverPtr, Resources::HQR_RESS_FILE, RESSHQR_GAMEOVERMDL) == 0) {
		return;
	}

	const int32 left = 120;
	const int32 top = 120;
	const int32 right = 519;
	const int32 bottom = 359;
	const Common::Rect rect(left, top, right, bottom);
	Renderer::prepareIsoModel(gameOverPtr);
	_engine->_sound->stopSamples();
	_engine->_music->stopMidiMusic(); // stop fade music
	_engine->_renderer->setCameraPosition(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 128, 200, 200);
	int32 startLbaTime = _engine->lbaTime;
	_engine->_interface->setClip(rect);

	while (!_engine->_input->toggleAbortAction() && (_engine->lbaTime - startLbaTime) <= 500) {
		ScopedFPS scopedFps(66);
		_engine->readKeys();
		if (_engine->shouldQuit()) {
			free(gameOverPtr);
			return;
		}

		const int32 avg = _engine->_collision->getAverageValue(40000, 3200, 500, _engine->lbaTime - startLbaTime);
		const int32 cdot = _engine->_screens->crossDot(1, 1024, 100, (_engine->lbaTime - startLbaTime) % 100);

		_engine->_interface->blitBox(rect, _engine->workVideoBuffer, _engine->frontVideoBuffer);
		_engine->_renderer->setCameraAngle(0, 0, 0, 0, -cdot, 0, avg);
		_engine->_renderer->renderIsoModel(0, 0, 0, 0, 0, 0, gameOverPtr);
		_engine->copyBlockPhys(rect);

		_engine->lbaTime++;
	}

	_engine->_sound->playSample(Samples::Explode);
	_engine->_interface->blitBox(rect, _engine->workVideoBuffer, _engine->frontVideoBuffer);
	_engine->_renderer->setCameraAngle(0, 0, 0, 0, 0, 0, 3200);
	_engine->_renderer->renderIsoModel(0, 0, 0, 0, 0, 0, gameOverPtr);
	_engine->copyBlockPhys(rect);

	_engine->delaySkip(2000);

	_engine->_interface->resetClip();
	free(gameOverPtr);
	_engine->_screens->copyScreen(_engine->workVideoBuffer, _engine->frontVideoBuffer);
	_engine->flip();
	initEngineProjections();

	_engine->lbaTime = tmpLbaTime;
}

void GameState::giveUp() {
	_engine->_sound->stopSamples();
	initGameStateVars();
	_engine->_scene->stopRunningGame();
}

} // namespace TwinE
