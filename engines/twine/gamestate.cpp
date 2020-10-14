/** @file gamestate.cpp
	@brief
	This file contains game state routines

	TwinEngine: a Little Big Adventure engine

	Copyright (C) 2013 The TwinEngine team
	Copyright (C) 2008-2013 Prequengine team
	Copyright (C) 2002-2007 The TwinEngine team

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "gamestate.h"
#include "scene.h"
#include "redraw.h"
#include "text.h"
#include "menu.h"
#include "renderer.h"
#include "grid.h"
#include "lbaengine.h"
#include "interface.h"
#include "animations.h"
#include "keyboard.h"
#include "resources.h"
#include "extra.h"
#include "sound.h"
#include "screens.h"
#include "music.h"
#include "filereader.h"
#include "menuoptions.h"
#include "collision.h"

#define SAVE_DIR "save/"

int32 magicLevelStrengthOfHit[] = {
	kNoBallStrenght,
	kYellowBallStrenght,
	kGreenBallStrenght,
	kRedBallStrenght,
	kFireBallStrength,
	0
};

/** Initialize engine 3D projections */
void initEngineProjections() { // reinitAll1
	setOrthoProjection(311, 240, 512);
	setBaseTranslation(0, 0, 0);
	setBaseRotation(0, 0, 0);
	setLightVector(alphaLight, betaLight, 0);
}

/** Initialize variables */
void initSceneVars() {
	int32 i;

	resetExtras();

	for (i = 0; i < OVERLAY_MAX_ENTRIES; i++) {
		overlayList[i].info0 = -1;
	}

	for (i = 0; i < NUM_SCENES_FLAGS; i++) {
		sceneFlags[i] = 0;
	}

	for (i = 0; i < NUM_GAME_FLAGS; i++) {
		gameFlags[i] = 0;
	}

	for (i = 0; i < NUM_INVENTORY_ITEMS; i++) {
		inventoryFlags[i] = 0;
	}

	sampleAmbiance[0] = -1;
	sampleAmbiance[1] = -1;
	sampleAmbiance[2] = -1;
	sampleAmbiance[3] = -1;

	sampleRepeat[0] = 0;
	sampleRepeat[1] = 0;
	sampleRepeat[2] = 0;
	sampleRepeat[3] = 0;

	sampleRound[0] = 0;
	sampleRound[1] = 0;
	sampleRound[2] = 0;
	sampleRound[3] = 0;

	for (i = 0; i < 150; i++) {
		holomapFlags[i] = 0;
	}

	sceneNumActors = 0;
	sceneNumZones  = 0;
	sceneNumTracks = 0;

	currentPositionInBodyPtrTab = 0;
}

void initHeroVars() { // reinitAll3
	resetActor(0); // reset Hero

	magicBallIdx = -1;

	inventoryNumLeafsBox = 2;
	inventoryNumLeafs    = 2;
	inventoryNumKashes   = 0;
	inventoryNumKeys     = 0;
	inventoryMagicPoints = 0;

	usingSabre = 0;

	sceneHero->body = 0;
	sceneHero->life = 50;
	sceneHero->talkColor = 4;
}

/** Initialize all engine variables */
void initEngineVars(int32 save) { // reinitAll
	resetClip();

	alphaLight = 896;
	betaLight = 950;
	initEngineProjections();
	initSceneVars();
	initHeroVars();

	newHeroX = 0x2000;
	newHeroY = 0x1800;
	newHeroZ = 0x2000;

	currentSceneIdx = -1;
	needChangeScene = 0;
	quitGame = -1;
	mecaPinguinIdx = -1;
	canShowCredits = 0;

	inventoryNumLeafs = 0;
	inventoryNumLeafsBox = 2;
	inventoryMagicPoints = 0;
	inventoryNumKashes = 0;
	inventoryNumKeys = 0;
	inventoryNumGas = 0;

	cropBottomScreen = 0;

	magicLevelIdx = 0;
	usingSabre = 0;

	gameChapter = 0;

	currentTextBank = 0;
	currentlyFollowedActor = 0;
	heroBehaviour = 0;
	previousHeroAngle = 0;
	previousHeroBehaviour = 0;

	if (save == -1) {
		loadGame();
		if (newHeroX == -1) {
			heroPositionType = kNoPosition;
		}
	}
}

void loadGame() {
	FileReader fr;
	uint8 data;
	int8* namePtr;

	if (!fropen2(&fr, SAVE_DIR "S9999.LBA", "rb")) {
		printf("Can't load S9999.LBA saved game!\n");
		return;
	}

	namePtr = savePlayerName;

	frread(&fr, &data, 1); // save game id

	do {
		frread(&fr, &data, 1); // get save player name characters
		*(namePtr++) = data;
	} while (data);

	frread(&fr, &data, 1); // number of game flags, always 0xFF
	frread(&fr, gameFlags, data);
	frread(&fr, &needChangeScene, 1); // scene index
	frread(&fr, &gameChapter, 1);

	frread(&fr, &heroBehaviour, 1);
	previousHeroBehaviour = heroBehaviour;
	frread(&fr, &sceneHero->life, 1);
	frread(&fr, &inventoryNumKashes, 2);
	frread(&fr, &magicLevelIdx, 1);
	frread(&fr, &inventoryMagicPoints, 1);
	frread(&fr, &inventoryNumLeafsBox, 1);
	frread(&fr, &newHeroX, 2);
	frread(&fr, &newHeroY, 2);
	frread(&fr, &newHeroZ, 2);
	frread(&fr, &sceneHero->angle, 2);
	previousHeroAngle = sceneHero->angle;
	frread(&fr, &sceneHero->body, 1);

	frread(&fr, &data, 1); // number of holomap locations, always 0x96
	frread(&fr, holomapFlags, data);

	frread(&fr, &inventoryNumGas, 1);

	frread(&fr, &data, 1); // number of used inventory items, always 0x1C
	frread(&fr, inventoryFlags, data);

	frread(&fr, &inventoryNumLeafs, 1);
	frread(&fr, &usingSabre, 1);

	frclose(&fr);

	currentSceneIdx = -1;
	heroPositionType = kReborn;
}

void saveGame() {
	FileReader fr;
	int8 data;

	if (!fropen2(&fr, SAVE_DIR "S9999.LBA", "wb+")) {
		printf("Can't save S9999.LBA saved game!\n");
		return;
	}

	data = 0x03;
	frwrite(&fr, &data, 1, 1);

	data = 0x00;
	frwrite(&fr, "TwinEngineSave", 15, 1);

	data = 0xFF; // number of game flags
	frwrite(&fr, &data, 1, 1);
	frwrite(&fr, gameFlags, 255, 1);

	frwrite(&fr, &currentSceneIdx, 1, 1);
	frwrite(&fr, &gameChapter, 1, 1);
	frwrite(&fr, &heroBehaviour, 1, 1);
	frwrite(&fr, &sceneHero->life, 1, 1);
	frwrite(&fr, &inventoryNumKashes, 2, 1);
	frwrite(&fr, &magicLevelIdx, 1, 1);
	frwrite(&fr, &inventoryMagicPoints, 1, 1);
	frwrite(&fr, &inventoryNumLeafsBox, 1, 1);
	frwrite(&fr, &newHeroX, 2, 1);
	frwrite(&fr, &newHeroY, 2, 1);
	frwrite(&fr, &newHeroZ, 2, 1);
	frwrite(&fr, &sceneHero->angle, 2, 1);
	frwrite(&fr, &sceneHero->body, 1, 1);

	data = 0x96; // number of holomap locations
	frwrite(&fr, &data, 1, 1);
	frwrite(&fr, holomapFlags, 150, 1);

	frwrite(&fr, &inventoryNumGas, 1, 1);

	data = 0x1C; // number of inventory items
	frwrite(&fr, &data, 1, 1);
	frwrite(&fr, inventoryFlags, 28, 1);

	frwrite(&fr, &inventoryNumLeafs, 1, 1);
	frwrite(&fr, &usingSabre, 1, 1);

	frclose(&fr);
}

void processFoundItem(int32 item) {
	int32 itemCameraX, itemCameraY, itemCameraZ; // objectXYZ
	int32 itemX, itemY, itemZ; // object2XYZ
	int32 boxTopLeftX, boxTopLeftY, boxBottomRightX, boxBottomRightY;
	int32 textState, quitItem, currentAnimState;
	uint8 *currentAnim;
	AnimTimerDataStruct tmpAnimTimer;

	newCameraX = (sceneHero->X + 0x100) >> 9;
	newCameraY = (sceneHero->Y + 0x100) >> 8;
	newCameraZ = (sceneHero->Z + 0x100) >> 9;

	// Hide hero in scene
	sceneHero->staticFlags.bIsHidden = 1;
	redrawEngineActions(1);
	sceneHero->staticFlags.bIsHidden = 0;

	copyScreen(frontVideoBuffer, workVideoBuffer);

	itemCameraX = newCameraX << 9;
	itemCameraY = newCameraY << 8;
	itemCameraZ = newCameraZ << 9;

	renderIsoModel(sceneHero->X - itemCameraX, sceneHero->Y - itemCameraY, sceneHero->Z - itemCameraZ, 0, 0x80, 0, bodyTable[sceneHero->entity]);
	setClip(renderLeft, renderTop, renderRight, renderBottom);

	itemX = (sceneHero->X + 0x100) >> 9;
	itemY = sceneHero->Y >> 8;
	if (sceneHero->brickShape & 0x7F) {
		itemY++;
	}
	itemZ = (sceneHero->Z + 0x100) >> 9;

	drawOverModelActor(itemX, itemY, itemZ);
	flip();

	projectPositionOnScreen(sceneHero->X - itemCameraX, sceneHero->Y - itemCameraY, sceneHero->Z - itemCameraZ);
	projPosY -= 150;

	boxTopLeftX = projPosX - 65;
	boxTopLeftY = projPosY - 65;

	boxBottomRightX = projPosX + 65;
	boxBottomRightY = projPosY + 65;

	playSample(41, 0x1000, 1, 0x80, 0x80, 0x80, -1);

	// process vox play
	{
		int32 tmpLanguageCDId;
		stopMusic();
		tmpLanguageCDId = cfgfile.LanguageCDId;
		//cfgfile.LanguageCDId = 0; // comented so we can init vox bank
		initTextBank(2);
		cfgfile.LanguageCDId = tmpLanguageCDId;
	}

	resetClip();
	initText(item);
	initDialogueBox();

	textState = 1;
	quitItem = 0;

	if (cfgfile.LanguageCDId) {
		initVoxToPlay(item);
	}

	currentAnim = animTable[getBodyAnimIndex(kFoundItem, 0)];

	tmpAnimTimer = sceneHero->animTimerData;

	animBuffer2 += stockAnimation(animBuffer2, bodyTable[sceneHero->entity], &sceneHero->animTimerData);
	if (animBuffer1 + 4488 < animBuffer2) {
		animBuffer2 = animBuffer1;
	}

	currentAnimState = 0;

	prepareIsoModel(inventoryTable[item]);
	numOfRedrawBox = 0;

	while (!quitItem) {
		resetClip();
		currNumOfRedrawBox = 0;
		blitBackgroundAreas();
		drawTransparentBox(boxTopLeftX, boxTopLeftY, boxBottomRightX, boxBottomRightY, 4);

		setClip(boxTopLeftX, boxTopLeftY, boxBottomRightX, boxBottomRightY);

		itemAngle[item] += 8;

		renderInventoryItem(projPosX, projPosY, inventoryTable[item], itemAngle[item], 10000);

		drawBox(boxTopLeftX, boxTopLeftY, boxBottomRightX, boxBottomRightY);
		addRedrawArea(boxTopLeftX, boxTopLeftY, boxBottomRightX, boxBottomRightY);
		resetClip();
		initEngineProjections();

		if (setModelAnimation(currentAnimState, currentAnim, bodyTable[sceneHero->entity], &sceneHero->animTimerData)) {
			currentAnimState++; // keyframe
			if (currentAnimState >= getNumKeyframes(currentAnim)) {
				currentAnimState = getStartKeyframe(currentAnim);
			}
		}

		renderIsoModel(sceneHero->X - itemCameraX, sceneHero->Y - itemCameraY, sceneHero->Z - itemCameraZ, 0, 0x80, 0, bodyTable[sceneHero->entity]);
		setClip(renderLeft, renderTop, renderRight, renderBottom);
		drawOverModelActor(itemX, itemY, itemZ);
		addRedrawArea(renderLeft, renderTop, renderRight, renderBottom);

		if (textState) {
			resetClip();
			textState = printText10();
		}

		if (textState == 0 || textState == 2) {
			sdldelay(15);
		}

		flipRedrawAreas();

		readKeys();
		if (skippedKey) {
			if (!textState) {
				quitItem = 1;
			}

			if (textState == 2) {
				textState = 1;
			}
		}

		lbaTime++;
	}

	while (playVoxSimple(currDialTextEntry)) {
		readKeys();
		if (skipIntro == 1) {
			break;
		}
		delaySkip(1);
	}

	initEngineProjections();
	initTextBank(currentTextBank + 3);

	/*do {
		readKeys();
		delaySkip(1);
	} while (!skipIntro);*/

	if (cfgfile.LanguageCDId && isSamplePlaying(currDialTextEntry)) {
		stopVox(currDialTextEntry);
	}

	sceneHero->animTimerData = tmpAnimTimer;
}

void processGameChoices(int32 choiceIdx) {
	int32 i;
	copyScreen(frontVideoBuffer, workVideoBuffer);

	gameChoicesSettings[0] = 0;	// Current loaded button (button number)
	gameChoicesSettings[1] = numChoices; // Num of buttons
	gameChoicesSettings[2] = 0; // Buttons box height
	gameChoicesSettings[3] = currentTextBank + 3;

	if (numChoices > 0) {
		for(i = 0; i < numChoices; i++) {
			gameChoicesSettings[i * 2 + 4] = 0;
			gameChoicesSettings[i * 2 + 5] = gameChoices[i];
		}
	}

	drawAskQuestion(choiceIdx);

	processMenu(gameChoicesSettings);
	choiceAnswer = gameChoices[gameChoicesSettings[0]];

	// get right VOX entry index
	if (cfgfile.LanguageCDId) {
		initVoxToPlay(choiceAnswer);
		while(playVoxSimple(currDialTextEntry));
		stopVox(currDialTextEntry);

		hasHiddenVox = 0;
		voxHiddenIndex = 0;
	}
}

void processGameoverAnimation() { // makeGameOver
	int32 tmpLbaTime, startLbaTime;
	uint8 *gameOverPtr;

	tmpLbaTime = lbaTime;

	// workaround to fix hero redraw after drowning
	sceneHero->staticFlags.bIsHidden = 1;
	redrawEngineActions(1);
	sceneHero->staticFlags.bIsHidden = 0;

	// TODO: drawInGameTransBox
	setPalette(paletteRGBA);
	copyScreen(frontVideoBuffer, workVideoBuffer);
	gameOverPtr = malloc(hqrEntrySize(HQR_RESS_FILE, RESSHQR_GAMEOVERMDL));
	hqrGetEntry(gameOverPtr, HQR_RESS_FILE, RESSHQR_GAMEOVERMDL);

	if (gameOverPtr) {
		int32 avg, cdot;

		prepareIsoModel(gameOverPtr);
		stopSamples();
		stopMidiMusic(); // stop fade music
		setCameraPosition(320, 240, 128, 200, 200);
		startLbaTime = lbaTime;
		setClip(120, 120, 519, 359);

		while(skipIntro != 1 && (lbaTime - startLbaTime) <= 0x1F4) {
			readKeys();

			avg = getAverageValue(40000, 3200, 500, lbaTime - startLbaTime);
			cdot = crossDot(1, 1024, 100, (lbaTime - startLbaTime) % 0x64);
			blitBox(120, 120, 519, 359, (int8*) workVideoBuffer, 120, 120, (int8*) frontVideoBuffer);
			setCameraAngle(0, 0, 0, 0, -cdot, 0, avg);
			renderIsoModel(0, 0, 0, 0, 0, 0, gameOverPtr);
			copyBlockPhys(120, 120, 519, 359);

			lbaTime++;
			sdldelay(15);
		}

		playSample(37, Rnd(2000) + 3096, 1, 0x80, 0x80, 0x80, -1);
		blitBox(120, 120, 519, 359, (int8*) workVideoBuffer, 120, 120, (int8*) frontVideoBuffer);
		setCameraAngle(0, 0, 0, 0, 0, 0, 3200);
		renderIsoModel(0, 0, 0, 0, 0, 0, gameOverPtr);
		copyBlockPhys(120, 120, 519, 359);

		delaySkip(2000);

		resetClip();
		free(gameOverPtr);
		copyScreen(workVideoBuffer, frontVideoBuffer);
		flip();
		initEngineProjections();

		lbaTime = tmpLbaTime;
	}
}
