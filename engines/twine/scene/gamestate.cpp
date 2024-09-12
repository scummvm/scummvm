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

#define SIZE_FOUND_OBJ 130

namespace TwinE {

GameState::GameState(TwinEEngine *engine) : _engine(engine) {
	clearGameFlags();
	Common::fill(&_inventoryFlags[0], &_inventoryFlags[NUM_INVENTORY_ITEMS], 0);
	Common::fill(&_holomapFlags[0], &_holomapFlags[MAX_HOLO_POS_2], 0);
	Common::fill(&_gameChoices[0], &_gameChoices[10], TextId::kNone);
}

void GameState::init3DGame() {
	_engine->_renderer->setIsoProjection(_engine->width() / 2 - 8 - 1, _engine->height() / 2, SIZE_BRICK_XZ);
	_engine->_renderer->setPosCamera(0, 0, 0);
	_engine->_renderer->setAngleCamera(LBAAngles::ANGLE_0, LBAAngles::ANGLE_0, LBAAngles::ANGLE_0);
	_engine->_renderer->setLightVector(_engine->_scene->_alphaLight, _engine->_scene->_betaLight, LBAAngles::ANGLE_0);
}

void GameState::initGameStateVars() {
	debug(2, "Init game state variables");
	_engine->_extra->resetExtras();

	for (int32 i = 0; i < OVERLAY_MAX_ENTRIES; i++) {
		_engine->_redraw->overlayList[i].num = -1;
	}

	for (int32 i = 0; i < ARRAYSIZE(_engine->_scene->_listFlagCube); i++) {
		_engine->_scene->_listFlagCube[i] = 0;
	}

	clearGameFlags();
	Common::fill(&_inventoryFlags[0], &_inventoryFlags[NUM_INVENTORY_ITEMS], 0);

	_engine->_scene->initSceneVars();

	Common::fill(&_holomapFlags[0], &_holomapFlags[MAX_HOLO_POS_2], 0);
}

void GameState::initHeroVars() {
	_engine->_actor->initObject(OWN_ACTOR_SCENE_INDEX); // reset Hero

	_magicBall = -1;

	_inventoryNumLeafsBox = 2;
	_inventoryNumLeafs = 2;
	_goldPieces = 0;
	_inventoryNumKeys = 0;
	_magicPoint = 0;

	_usingSabre = false;

	_engine->_scene->_sceneHero->_genBody = BodyType::btNormal;
	_engine->_scene->_sceneHero->setLife(_engine->getMaxLife());
	_engine->_scene->_sceneHero->_talkColor = COLOR_BRIGHT_BLUE;
}

void GameState::initEngineVars() {
	debug(2, "Init engine variables");
	_engine->_interface->unsetClip();

	_engine->_scene->_alphaLight = LBAAngles::ANGLE_315;
	_engine->_scene->_betaLight = LBAAngles::ANGLE_334;
	init3DGame();
	initGameStateVars();
	initHeroVars();

	_engine->_scene->_newHeroPos.x = 16 * SIZE_BRICK_XZ;
	_engine->_scene->_newHeroPos.y = 24 * SIZE_BRICK_Y;
	_engine->_scene->_newHeroPos.z = 16 * SIZE_BRICK_XZ;

	_engine->_scene->_currentSceneIdx = SCENE_CEILING_GRID_FADE_1;
	_engine->_scene->_needChangeScene = LBA1SceneId::Citadel_Island_Prison;
	_engine->_sceneLoopState = SceneLoopState::Continue;
	_engine->_scene->_mecaPenguinIdx = -1;
	_engine->_menuOptions->flagCredits = false;

	_inventoryNumLeafs = 0;
	_inventoryNumLeafsBox = 2;
	_magicPoint = 0;
	_goldPieces = 0;
	_inventoryNumKeys = 0;
	_inventoryNumGas = 0;

	_engine->_actor->_cropBottomScreen = 0;

	_magicLevelIdx = 0;
	_usingSabre = false;

	setChapter(0);

	_engine->_scene->_sceneTextBank = TextBankId::Options_and_menus;
	_engine->_scene->_currentlyFollowedActor = OWN_ACTOR_SCENE_INDEX;
	_engine->_actor->_heroBehaviour = HeroBehaviourType::kNormal;
	_engine->_actor->_previousHeroAngle = 0;
	_engine->_actor->_previousHeroBehaviour = HeroBehaviourType::kNormal;
}

// https://web.archive.org/web/*/http://lbafileinfo.kazekr.net/index.php?title=LBA1:Savegame
bool GameState::loadGame(Common::SeekableReadStream *file) {
	if (file == nullptr) {
		return false;
	}

	if (!_engine->isLBA1()) {
		warning("Loading not implemented for lba2");
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
		_engine->_menuOptions->_saveGameName[playerNameIdx++] = c;
		if (c == '\0') {
			break;
		}
		if (playerNameIdx >= ARRAYSIZE(_engine->_menuOptions->_saveGameName)) {
			warning("Failed to load savegame. Invalid playername.");
			return false;
		}
	} while (true);

	byte numGameFlags = file->readByte();
	if (numGameFlags != NUM_GAME_FLAGS_LBA1) {
		warning("Failed to load gameflags. Expected %u, but got %u", NUM_GAME_FLAGS_LBA1, numGameFlags);
		return false;
	}
	for (uint8 i = 0; i < numGameFlags; ++i) {
		setGameFlag(i, file->readByte());
	}
	_engine->_scene->_needChangeScene = file->readByte(); // scene index
	setChapter(file->readByte());

	_engine->_actor->_heroBehaviour = (HeroBehaviourType)file->readByte();
	_engine->_actor->_previousHeroBehaviour = _engine->_actor->_heroBehaviour;
	_engine->_scene->_sceneHero->setLife(file->readByte());
	setKashes(file->readSint16LE());
	_magicLevelIdx = file->readByte();
	setMagicPoints(file->readByte());
	setLeafBoxes(file->readByte());
	_engine->_scene->_newHeroPos.x = file->readSint16LE();
	_engine->_scene->_newHeroPos.y = file->readSint16LE();
	_engine->_scene->_newHeroPos.z = file->readSint16LE();
	_engine->_scene->_sceneHero->_beta = ToAngle(file->readSint16LE());
	_engine->_actor->_previousHeroAngle = _engine->_scene->_sceneHero->_beta;
	_engine->_scene->_sceneHero->_genBody = (BodyType)file->readByte();

	const byte numHolomapFlags = file->readByte(); // number of holomap locations
	if (numHolomapFlags != _engine->numHoloPos()) {
		warning("Failed to load holomapflags. Got %u, expected %i", numHolomapFlags, _engine->numHoloPos());
		return false;
	}
	file->read(_holomapFlags, _engine->numHoloPos());

	setGas(file->readByte());

	const byte numInventoryFlags = file->readByte(); // number of used inventory items, always 28
	if (numInventoryFlags != NUM_INVENTORY_ITEMS) {
		warning("Failed to load inventoryFlags. Got %u, expected %i", numInventoryFlags, NUM_INVENTORY_ITEMS);
		return false;
	}
	file->read(_inventoryFlags, NUM_INVENTORY_ITEMS);

	setLeafs(file->readByte());
	_usingSabre = file->readByte();

	if (saveFileVersion == 4) {
		// the time the game was played
		file->readUint32LE();
		file->readUint32LE();
	}

	_engine->_scene->_currentSceneIdx = SCENE_CEILING_GRID_FADE_1;
	_engine->_scene->_heroPositionType = ScenePositionType::kReborn;
	return true;
}

bool GameState::saveGame(Common::WriteStream *file) {
	debug(2, "Save game");
	if (!_engine->isLBA1()) {
		warning("Saving not implemented for lba2");
		return false;
	}
	if (_engine->_menuOptions->_saveGameName[0] == '\0') {
		Common::strlcpy(_engine->_menuOptions->_saveGameName, "TwinEngineSave", sizeof(_engine->_menuOptions->_saveGameName));
	}

	int32 sceneIdx = _engine->_scene->_currentSceneIdx;
	if (sceneIdx == Polar_Island_end_scene || sceneIdx == Citadel_Island_end_sequence_1 || sceneIdx == Citadel_Island_end_sequence_2 || sceneIdx == Credits_List_Sequence) {
		/* inventoryMagicPoints = 0x50 */
		/* herobehaviour = 0 */
		/* newheropos.x = 0xffff */
		sceneIdx = Polar_Island_Final_Battle;
	}

	file->writeByte(0x03);
	file->writeString(_engine->_menuOptions->_saveGameName);
	file->writeByte('\0');
	file->writeByte(NUM_GAME_FLAGS_LBA1);
	for (uint8 i = 0; i < NUM_GAME_FLAGS_LBA1; ++i) {
		file->writeByte((uint8)hasGameFlag(i));
	}
	file->writeByte(sceneIdx);
	file->writeByte(getChapter());
	file->writeByte((byte)_engine->_actor->_heroBehaviour);
	file->writeByte(_engine->_scene->_sceneHero->_lifePoint);
	file->writeSint16LE(_goldPieces);
	file->writeByte(_magicLevelIdx);
	file->writeByte(_magicPoint);
	file->writeByte(_inventoryNumLeafsBox);
	// we don't save the whole scene state - so we have to make sure that the hero is
	// respawned at the start of the scene - and not at its current position
	file->writeSint16LE(_engine->_scene->_newHeroPos.x);
	file->writeSint16LE(_engine->_scene->_newHeroPos.y);
	file->writeSint16LE(_engine->_scene->_newHeroPos.z);
	file->writeSint16LE(FromAngle(_engine->_scene->_sceneHero->_beta));
	file->writeByte((uint8)_engine->_scene->_sceneHero->_genBody);

	// number of holomap locations
	file->writeByte(_engine->numHoloPos());
	file->write(_holomapFlags, _engine->numHoloPos());

	file->writeByte(_inventoryNumGas);

	// number of inventory items
	file->writeByte(NUM_INVENTORY_ITEMS);
	file->write(_inventoryFlags, NUM_INVENTORY_ITEMS);

	file->writeByte(_inventoryNumLeafs);
	file->writeByte(_usingSabre ? 1 : 0);
	file->writeByte(0);

	return true;
}

void GameState::setChapter(int16 chapter) {
	if (_engine->isLBA1()) {
		_gameChapter = chapter;
		return;
	}
	setGameFlag(253, chapter);
}

int16 GameState::getChapter() const {
	if (_engine->isLBA1()) {
		return _gameChapter;
	}
	return _listFlagGame[253];
}

void GameState::setGameFlag(uint8 index, int16 value) {
	if (_listFlagGame[index] == value) {
		return;
	}
	debug(2, "Set gameStateFlags[%u]=%u", index, value);
	_listFlagGame[index] = value;
	if (!value) {
		return;
	}

	if ((index == GAMEFLAG_VIDEO_BAFFE || index == GAMEFLAG_VIDEO_BAFFE2 || index == GAMEFLAG_VIDEO_BAFFE3 || index == GAMEFLAG_VIDEO_BAFFE5) &&
		_listFlagGame[GAMEFLAG_VIDEO_BAFFE] != 0 && _listFlagGame[GAMEFLAG_VIDEO_BAFFE2] != 0 && _listFlagGame[GAMEFLAG_VIDEO_BAFFE3] != 0 && _listFlagGame[GAMEFLAG_VIDEO_BAFFE5] != 0) {
		// all 4 slap videos
		_engine->unlockAchievement("LBA_ACH_012");
	} else if (index == GAMEFLAG_VIDEO_BATEAU2) {
		// second video of ferry trip
		_engine->unlockAchievement("LBA_ACH_010");
	} else if (index == (uint8)InventoryItems::kiUseSabre) {
		_engine->unlockAchievement("LBA_ACH_002");
	} else if (index == (uint8)InventoryItems::kBottleOfSyrup) {
		_engine->unlockAchievement("LBA_ACH_007");
	}
}

void GameState::doFoundObj(InventoryItems item) {
	ScopedEngineFreeze freeze(_engine);
	_engine->_grid->centerOnActor(_engine->_scene->_sceneHero);

	_engine->extInitSvga();
	// Hide hero in scene
	_engine->_scene->_sceneHero->_staticFlags.bIsInvisible = 1;
	_engine->_redraw->redrawEngineActions(true);
	_engine->_scene->_sceneHero->_staticFlags.bIsInvisible = 0;

	_engine->saveFrontBuffer();

	IVec3 itemCamera;
	itemCamera.x = _engine->_grid->_newCamera.x * SIZE_BRICK_XZ;
	itemCamera.y = _engine->_grid->_newCamera.y * SIZE_BRICK_Y;
	itemCamera.z = _engine->_grid->_newCamera.z * SIZE_BRICK_XZ;

	BodyData &bodyData = _engine->_resources->_bodyData[_engine->_scene->_sceneHero->_body];
	const IVec3 bodyPos = _engine->_scene->_sceneHero->_posObj - itemCamera;
	Common::Rect modelRect;
	_engine->_renderer->renderIsoModel(bodyPos, LBAAngles::ANGLE_0, LBAAngles::ANGLE_45, LBAAngles::ANGLE_0, bodyData, modelRect);
	_engine->_interface->setClip(modelRect);

	const int32 itemX = (_engine->_scene->_sceneHero->_posObj.x + SIZE_BRICK_Y) / SIZE_BRICK_XZ;
	int32 itemY = _engine->_scene->_sceneHero->_posObj.y / SIZE_BRICK_Y;
	if (_engine->_scene->_sceneHero->brickShape() != ShapeType::kNone) {
		itemY++;
	}
	const int32 itemZ = (_engine->_scene->_sceneHero->_posObj.z + SIZE_BRICK_Y) / SIZE_BRICK_XZ;

	_engine->_grid->drawOverBrick(itemX, itemY, itemZ);

	IVec3 projPos = _engine->_renderer->projectPoint(bodyPos);
	projPos.y -= 150;

	const int32 boxTopLeftX = projPos.x - (SIZE_FOUND_OBJ / 2);
	const int32 boxTopLeftY = projPos.y - (SIZE_FOUND_OBJ / 2);
	const int32 boxBottomRightX = projPos.x + (SIZE_FOUND_OBJ / 2);
	const int32 boxBottomRightY = projPos.y + (SIZE_FOUND_OBJ / 2);
	const Common::Rect boxRect(boxTopLeftX, boxTopLeftY, boxBottomRightX, boxBottomRightY);
	_engine->_sound->playSample(Samples::BigItemFound);

	// process vox play
	_engine->_music->stopMusic();
	_engine->_text->initDial(TextBankId::Inventory_Intro_and_Holomap);

	_engine->_interface->unsetClip();
	_engine->_text->initItemFoundText(item);
	_engine->_text->initDialogueBox();

	ProgressiveTextState textState = ProgressiveTextState::ContinueRunning;

	_engine->_text->initVoxToPlayTextId((TextId)item);

	const int32 bodyAnimIdx = _engine->_animations->searchAnim(AnimationTypes::kFoundItem);
	const AnimData &currentAnimData = _engine->_resources->_animData[bodyAnimIdx];

	AnimTimerDataStruct tmpAnimTimer = _engine->_scene->_sceneHero->_animTimerData;

	_engine->_animations->stockInterAnim(bodyData, &_engine->_scene->_sceneHero->_animTimerData);

	uint currentAnimState = 0;

	_engine->_redraw->_numOfRedrawBox = 0;

	ScopedKeyMap uiKeyMap(_engine, uiKeyMapId);
	int16 itemAngle = LBAAngles::ANGLE_0;
	for (;;) {
		FrameMarker frame(_engine, 66);
		_engine->_interface->unsetClip();
		_engine->_redraw->_currNumOfRedrawBox = 0;
		_engine->_redraw->blitBackgroundAreas();
		_engine->_interface->drawTransparentBox(boxRect, 4);

		_engine->_interface->setClip(boxRect);

		itemAngle += LBAAngles::ANGLE_2;

		_engine->_renderer->draw3dObject(projPos.x, projPos.y, _engine->_resources->_inventoryTable[item], itemAngle, 10000);

		_engine->_menu->drawRectBorders(boxRect);
		_engine->_redraw->addRedrawArea(boxRect);
		_engine->_interface->unsetClip();
		init3DGame();

		if (_engine->_animations->doSetInterAnimObjet(currentAnimState, currentAnimData, bodyData, &_engine->_scene->_sceneHero->_animTimerData)) {
			currentAnimState++; // keyframe
			if (currentAnimState >= currentAnimData.getNumKeyframes()) {
				currentAnimState = currentAnimData.getLoopFrame();
			}
		}

		_engine->_renderer->renderIsoModel(bodyPos, LBAAngles::ANGLE_0, LBAAngles::ANGLE_45, LBAAngles::ANGLE_0, bodyData, modelRect);
		_engine->_interface->setClip(modelRect);
		_engine->_grid->drawOverBrick(itemX, itemY, itemZ);
		_engine->_redraw->addRedrawArea(modelRect);

		if (textState == ProgressiveTextState::ContinueRunning) {
			_engine->_interface->unsetClip();
			textState = _engine->_text->updateProgressiveText();
		} else {
			_engine->_text->fadeInRemainingChars();
		}

		_engine->_redraw->flipRedrawAreas();

		_engine->readKeys();
		if (_engine->_input->toggleAbortAction()) {
			_engine->_text->stopVox(_engine->_text->_currDialTextEntry);
			break;
		}

		if (_engine->_input->toggleActionIfActive(TwinEActionType::UINextPage)) {
			if (textState == ProgressiveTextState::End) {
				_engine->_text->stopVox(_engine->_text->_currDialTextEntry);
				break;
			}
			if (textState == ProgressiveTextState::NextPage) {
				textState = ProgressiveTextState::ContinueRunning;
			}
		}

		_engine->_text->playVoxSimple(_engine->_text->_currDialTextEntry);

		// advance the timer to play animations
		_engine->timerRef++;
		debugC(3, kDebugLevels::kDebugTime, "FoundObj time: %i", _engine->timerRef);
	}

	while (_engine->_text->playVoxSimple(_engine->_text->_currDialTextEntry)) {
		FrameMarker frame(_engine);
		_engine->readKeys();
		if (_engine->shouldQuit() || _engine->_input->toggleAbortAction()) {
			break;
		}
	}

	init3DGame();
	_engine->_text->initSceneTextBank();
	_engine->_text->stopVox(_engine->_text->_currDialTextEntry);

	_engine->_scene->_sceneHero->_animTimerData = tmpAnimTimer;
	_engine->_interface->unsetClip();
}

void GameState::processGameChoices(TextId choiceIdx) {
	_engine->saveFrontBuffer();

	_gameChoicesSettings.reset();
	_gameChoicesSettings.setTextBankId((TextBankId)((int)_engine->_scene->_sceneTextBank + (int)TextBankId::Citadel_Island));

	// filled via script
	for (int32 i = 0; i < _numChoices; i++) {
		_gameChoicesSettings.addButton(_gameChoices[i], 0);
	}

	_engine->_text->drawAskQuestion(choiceIdx);

	_engine->_menu->doGameMenu(&_gameChoicesSettings);
	const int16 activeButton = _gameChoicesSettings.getActiveButton();
	_choiceAnswer = _gameChoices[activeButton];

	// get right VOX entry index
	if (_engine->_text->initVoxToPlayTextId(_choiceAnswer)) {
		while (_engine->_text->playVoxSimple(_engine->_text->_currDialTextEntry)) {
			FrameMarker frame(_engine);
			if (_engine->shouldQuit()) {
				break;
			}
		}
		_engine->_text->stopVox(_engine->_text->_currDialTextEntry);

		_engine->_text->_hasHiddenVox = false;
		_engine->_text->_voxHiddenIndex = 0;
	}
}

void GameState::processGameoverAnimation() {
	const int32 tmpLbaTime = _engine->timerRef;

	_engine->testRestoreModeSVGA(false);
	// workaround to fix hero redraw after drowning
	_engine->_scene->_sceneHero->_staticFlags.bIsInvisible = 1;
	_engine->_redraw->redrawEngineActions(true);
	_engine->_scene->_sceneHero->_staticFlags.bIsInvisible = 0;

	// TODO: inSceneryView
	_engine->setPalette(_engine->_screens->_paletteRGBA);
	_engine->saveFrontBuffer();
	BodyData gameOverPtr;
	if (!gameOverPtr.loadFromHQR(Resources::HQR_RESS_FILE, RESSHQR_GAMEOVERMDL, _engine->isLBA1())) {
		return;
	}

	_engine->_sound->stopSamples();
	_engine->_music->stopMusicMidi(); // stop fade music
	_engine->_renderer->setProjection(_engine->width() / 2, _engine->height() / 2, 128, 200, 200);
	int32 startLbaTime = _engine->timerRef;
	const Common::Rect &rect = _engine->centerOnScreen(_engine->width() / 2, _engine->height() / 2);
	_engine->_interface->setClip(rect);

	int32 zoom = 50000;
	Common::Rect dummy;
	while (!_engine->_input->toggleAbortAction() && (_engine->timerRef - startLbaTime) <= _engine->toSeconds(10)) {
		FrameMarker frame(_engine, 66);
		_engine->readKeys();
		if (_engine->shouldQuit()) {
			return;
		}

		zoom = _engine->_collision->boundRuleThree(40000, 3200, _engine->toSeconds(10), _engine->timerRef - startLbaTime);
		const int32 angle = _engine->_screens->lerp(1, LBAAngles::ANGLE_360, _engine->toSeconds(2), (_engine->timerRef - startLbaTime) % _engine->toSeconds(2));

		_engine->blitWorkToFront(rect);
		_engine->_renderer->setFollowCamera(0, 0, 0, 0, -angle, 0, zoom);
		_engine->_renderer->affObjetIso(0, 0, 0, LBAAngles::ANGLE_0, LBAAngles::ANGLE_0, LBAAngles::ANGLE_0, gameOverPtr, dummy);

		_engine->timerRef++;
		debugC(3, kDebugLevels::kDebugTime, "GameOver time: %i", _engine->timerRef);
	}

	_engine->_sound->playSample(Samples::Explode);
	_engine->blitWorkToFront(rect);
	_engine->_renderer->setFollowCamera(0, 0, 0, 0, 0, 0, zoom);
	_engine->_renderer->affObjetIso(0, 0, 0, LBAAngles::ANGLE_0, LBAAngles::ANGLE_0, LBAAngles::ANGLE_0, gameOverPtr, dummy);

	_engine->delaySkip(2000);

	_engine->_interface->unsetClip();
	_engine->restoreFrontBuffer();
	init3DGame();

	_engine->timerRef = tmpLbaTime;
}

void GameState::giveUp() {
	_engine->_sound->stopSamples();
	// TODO: is an autosave desired on giving up? I don't think so. What did the original game do here?
	//_engine->autoSave();
	initGameStateVars();
	_engine->_scene->stopRunningGame();
}

int16 GameState::setGas(int16 value) {
	_inventoryNumGas = CLIP<int16>(value, 0, 100);
	return _inventoryNumGas;
}

void GameState::addGas(int16 value) {
	setGas(_inventoryNumGas + value);
}

// late game items from lba1 classic new game +
void GameState::handleLateGameItems() {
	if (!_endGameItems) {
		return;
	}
	debug("Give end game items");
	_endGameItems = false;
	_magicLevelIdx = 4;
	setMaxMagicPoints();
	giveItem(InventoryItems::kiUseSabre);
	giveItem(InventoryItems::kiProtoPack);
	giveItem(InventoryItems::kiHolomap);
	giveItem(InventoryItems::kiTunic);
	giveItem(InventoryItems::kiMagicBall);
	giveItem(InventoryItems::kSendellsMedallion);
	giveItem(InventoryItems::kiPenguin);
	giveItem(InventoryItems::kGasItem);
	giveItem(InventoryItems::kiCloverLeaf);
	addGas(10);
}

int16 GameState::setKashes(int16 value) {
	_goldPieces = CLIP<int16>(value, 0, 999);
	if (_engine->_gameState->_goldPieces >= 500) {
		_engine->unlockAchievement("LBA_ACH_011");
	}
	return _goldPieces;
}

int16 GameState::setZlitos(int16 value) {
	_zlitosPieces = CLIP<int16>(value, 0, 999);
	return _zlitosPieces;
}

int16 GameState::setKeys(int16 value) {
	_inventoryNumKeys = MAX<int16>(0, value);
	return _inventoryNumKeys;
}

void GameState::addKeys(int16 val) {
	setKeys(_inventoryNumKeys + val);
}

void GameState::addKashes(int16 val) {
	setKashes(_goldPieces + val);
}

int16 GameState::setMagicPoints(int16 val) {
	_magicPoint = val;
	if (_magicPoint > _magicLevelIdx * 20) {
		_magicPoint = _magicLevelIdx * 20;
	} else if (_magicPoint < 0) {
		_magicPoint = 0;
	}
	return _magicPoint;
}

int16 GameState::setMaxMagicPoints() {
	_magicPoint = _magicLevelIdx * 20;
	return _magicPoint;
}

void GameState::addMagicPoints(int16 val) {
	setMagicPoints(_magicPoint + val);
}

int16 GameState::setLeafs(int16 val) {
	_inventoryNumLeafs = val;
	if (_inventoryNumLeafs > _inventoryNumLeafsBox) {
		_inventoryNumLeafs = _inventoryNumLeafsBox;
	}
	return _inventoryNumLeafs;
}

void GameState::addLeafs(int16 val) {
	setLeafs(_inventoryNumLeafs + val);
}

int16 GameState::setLeafBoxes(int16 val) {
	_inventoryNumLeafsBox = val;
	if (_inventoryNumLeafsBox > 10) {
		_inventoryNumLeafsBox = 10;
	}
	if (_inventoryNumLeafsBox == 5) {
		_engine->unlockAchievement("LBA_ACH_003");
	}
	return _inventoryNumLeafsBox;
}

void GameState::addLeafBoxes(int16 val) {
	setLeafBoxes(_inventoryNumLeafsBox + val);
}

void GameState::clearGameFlags() {
	debug(2, "Clear all gameStateFlags");
	Common::fill(&_listFlagGame[0], &_listFlagGame[NUM_GAME_FLAGS], 0);
}

int16 GameState::hasGameFlag(uint8 index) const {
	debug(6, "Query gameStateFlags[%u]=%u", index, _listFlagGame[index]);
	return _listFlagGame[index];
}

} // namespace TwinE
