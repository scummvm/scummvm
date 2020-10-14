/** @file lbaengine.cpp
	@brief
	This file contains the main game engine routines

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

#include "lbaengine.h"
#include "main.h"
#include "sdlengine.h"
#include "screens.h"
#include "grid.h"
#include "debug.grid.h"
#include "scene.h"
#include "menu.h"
#include "interface.h"
#include "text.h"
#include "redraw.h"
#include "hqrdepack.h"
#include "resources.h"
#include "renderer.h"
#include "animations.h"
#include "movements.h"
#include "keyboard.h"
#include "gamestate.h"
#include "sound.h"
#include "script.life.h"
#include "script.move.h"
#include "extra.h"
#include "menuoptions.h"
#include "collision.h"

#ifdef GAMEMOD
#include "debug.h"
#endif

int32 isTimeFreezed = 0;
int32 saveFreezedTime = 0;

enum InventoryItems {
	kiHolomap = 0,
	kiMagicBall = 1,
	kiUseSabre = 2,
	kiBookOfBu = 5,
	kiProtoPack = 12,
	kiPinguin = 14,
	kiBonusList = 26,
	kiCloverLeaf = 27
};

void freezeTime() {
	if (!isTimeFreezed)
		saveFreezedTime = lbaTime;
	isTimeFreezed++;
}

void unfreezeTime() {
	--isTimeFreezed;
	if (isTimeFreezed == 0)
		lbaTime = saveFreezedTime;
}

void processActorSamplePosition(int32 actorIdx) {
	int32 channelIdx;
	ActorStruct *actor = &sceneActors[actorIdx];
	channelIdx = getActorChannel(actorIdx);
	setSamplePosition(channelIdx, actor->X, actor->Y, actor->Z);
}

/** Game engine main loop
	@return true if we want to show credit sequence */
int32 runGameEngine() { // mainLoopInteration
	int32 a;
	readKeys();

	if (needChangeScene > -1) {
		changeScene();
	}

	previousLoopPressedKey = loopPressedKey;
	key  = pressedKey;
	loopPressedKey = skippedKey;
	loopCurrentKey = skipIntro;

#ifdef GAMEMOD
	processDebug(loopCurrentKey);
#endif

	if(canShowCredits != 0) {
		// TODO: if current music playing != 8, than play_track(8);
		if (skipIntro != 0) {
			return 0;
		}
		if (pressedKey != 0) {
			return 0;
		}
		if (skippedKey != 0) {
			return 0;
		}
	} else {
		// Process give up menu - Press ESC
		if (skipIntro == 1 && sceneHero->life > 0 && sceneHero->entity != -1 && !sceneHero->staticFlags.bIsHidden) {
			freezeTime();
			if (giveupMenu()) {
				unfreezeTime();
				redrawEngineActions(1);
				freezeTime();
				saveGame(); // auto save game
				quitGame = 0;
				cfgfile.Quit = 0;
				unfreezeTime();
				return 0;
			} else {
				unfreezeTime();
				redrawEngineActions(1);
			}
		}

		// Process options menu - Press F6
		if (loopCurrentKey == 0x40) {
			int tmpLangCD = cfgfile.LanguageCDId;
			freezeTime();
			pauseSamples();
			OptionsMenuSettings[5] = 15;
			cfgfile.LanguageCDId = 0;
			initTextBank(0);
			optionsMenu();
			cfgfile.LanguageCDId = tmpLangCD;
			initTextBank(currentTextBank + 3);
			//TODO: play music
			resumeSamples();
			unfreezeTime();
			redrawEngineActions(1);
		}

		// inventory menu
		loopInventoryItem = -1;
		if (loopCurrentKey == 0x36 && sceneHero->entity != -1 && sceneHero->controlMode == kManual) {
			freezeTime();
			processInventoryMenu();

			switch (loopInventoryItem) {
			case kiHolomap:
				printf("Use Inventory [kiHolomap] not implemented!\n");
				break;
			case kiMagicBall:
				if (usingSabre == 1) {
					initModelActor(0, 0);
				}
				usingSabre = 0;
				break;
			case kiUseSabre:
				if (sceneHero->body != GAMEFLAG_HAS_SABRE) {
					if (heroBehaviour == kProtoPack) {
						setBehaviour(kNormal);
					}
					initModelActor(GAMEFLAG_HAS_SABRE, 0);
					initAnim(24, 1, 0, 0);

					usingSabre = 1;
				}
				break;
			case kiBookOfBu: {
				int32 tmpFlagDisplayText;

				fadeToBlack(paletteRGBA);
				loadImage(RESSHQR_INTROSCREEN1IMG, 1);
				initTextBank(2);
				newGameVar4 = 0;
				textClipFull();
				setFontCrossColor(15);
				tmpFlagDisplayText = cfgfile.FlagDisplayText;
				cfgfile.FlagDisplayText = 1;
				drawTextFullscreen(161);
				cfgfile.FlagDisplayText = tmpFlagDisplayText;
				textClipSmall();
				newGameVar4 = 1;
				initTextBank(currentTextBank + 3);
				fadeToBlack(paletteRGBACustom);
				clearScreen();
				flip();
				setPalette(paletteRGBA);
				lockPalette = 1;
			}
				break;
			case kiProtoPack:
				if (gameFlags[GAMEFLAG_BOOKOFBU]) {
					sceneHero->body = 0;
				} else {
					sceneHero->body = 1;
				}

				if (heroBehaviour == kProtoPack) {
					setBehaviour(kNormal);
				} else {
					setBehaviour(kProtoPack);
				}
				break;
			case kiPinguin: {
				ActorStruct *pinguin = &sceneActors[mecaPinguinIdx];

				pinguin->X = destX + sceneHero->X;
				pinguin->Y = sceneHero->Y;
				pinguin->Z = destZ + sceneHero->Z;
				pinguin->angle = sceneHero->angle;

				rotateActor(0, 800, pinguin->angle);

				if (!checkCollisionWithActors(mecaPinguinIdx)) {
					pinguin->life = 50;
					pinguin->body = -1;
					initModelActor(0, mecaPinguinIdx);
					pinguin->dynamicFlags.bIsDead = 0; // &= 0xDF
                    pinguin->brickShape = 0;
					moveActor(pinguin->angle, pinguin->angle, pinguin->speed, &pinguin->move);
					gameFlags[GAMEFLAG_MECA_PINGUIN] = 0; // byte_50D89 = 0;
                    pinguin->info0 = lbaTime + 1500;
				}
			}
				break;
			case kiBonusList: {
				int32 tmpLanguageCDIdx;
				tmpLanguageCDIdx = cfgfile.LanguageCDId;
				unfreezeTime();
				redrawEngineActions(1);
				freezeTime();
				cfgfile.LanguageCDId = 0;
				initTextBank(2);
				textClipFull();
				setFontCrossColor(15);
				drawTextFullscreen(162);
				textClipSmall();
				cfgfile.LanguageCDId = tmpLanguageCDIdx;
				initTextBank(currentTextBank + 3);
			}
				break;
			case kiCloverLeaf:
				if (sceneHero->life < 50) {
					if (inventoryNumLeafs > 0) {
						sceneHero->life = 50;
						inventoryMagicPoints = magicLevelIdx * 20;
						inventoryNumLeafs--;
						addOverlay(koInventoryItem, 27, 0, 0, 0, koNormal, 3);
					}
				}
				break;
			}


			unfreezeTime();
			redrawEngineActions(1);
		}

		// Process behaviour menu - Press CTRL and F1..F4 Keys
		if ((loopCurrentKey == 0x1D || loopCurrentKey == 0x3B || loopCurrentKey == 0x3C || loopCurrentKey == 0x3D || loopCurrentKey == 0x3E) && sceneHero->entity != -1 && sceneHero->controlMode == kManual) {
			if (loopCurrentKey != 0x1D) {
				heroBehaviour = loopCurrentKey - 0x3B;
			}
			freezeTime();
			processBehaviourMenu();
			unfreezeTime();
			redrawEngineActions(1);
		}

		// use Proto-Pack
		if (loopCurrentKey == 0x24 && gameFlags[GAMEFLAG_PROTOPACK] == 1) {
			if (gameFlags[GAMEFLAG_BOOKOFBU]) {
				sceneHero->body = 0;
			} else {
				sceneHero->body = 1;
			}

			if (heroBehaviour == kProtoPack) {
				setBehaviour(kNormal);
			} else {
				setBehaviour(kProtoPack);
			}
		}

		// Press Enter to Recenter Screen
		if ((loopPressedKey & 2) && !disableScreenRecenter) {
			newCameraX = sceneActors[currentlyFollowedActor].X >> 9;
			newCameraY = sceneActors[currentlyFollowedActor].Y >> 8;
			newCameraZ = sceneActors[currentlyFollowedActor].Z >> 9;
			reqBgRedraw = 1;
		}

		// TODO: draw holomap

		// Process Pause - Press P
		if (loopCurrentKey == 0x19) {
			freezeTime();
			setFontColor(15);
			drawText(5, 446, (int8*)"Pause"); // no key for pause in Text Bank
			copyBlockPhys(5, 446, 100, 479);
			do {
				readKeys();
				SDL_Delay(10);
			} while (skipIntro != 0x19 && !pressedKey);
			unfreezeTime();
			redrawEngineActions(1);
		}
	}

	loopActorStep = getRealValue(&loopMovePtr);
	if (!loopActorStep) {
		loopActorStep = 1;
	}

	setActorAngle(0, -256, 5, &loopMovePtr);
	disableScreenRecenter = 0;

	processEnvironmentSound();

	// Reset HitBy state
	for (a = 0; a < sceneNumActors; a++)
    {
        sceneActors[a].hitBy = -1;
    }

	processExtras();

	for (a = 0; a < sceneNumActors; a++) {
		ActorStruct *actor = &sceneActors[a];

		if (!actor->dynamicFlags.bIsDead) {
			if (actor->life == 0) {
				if (a == 0) { // if its hero who died
					initAnim(kLandDeath, 4, 0, 0);
					actor->controlMode = 0;
				} else {
					playSample(37, Rnd(2000) + 3096, 1, actor->X, actor->Y, actor->Z, a);

					if (a == mecaPinguinIdx) {
						addExtraExplode(actor->X, actor->Y, actor->Z);
					}
				}

				if (actor->bonusParameter & 0x1F0 && !(actor->bonusParameter & 1)) {
					processActorExtraBonus(a);
				}
			}

			processActorMovements(a);

			actor->collisionX = actor->X;
			actor->collisionY = actor->Y;
			actor->collisionZ = actor->Z;

			if (actor->positionInMoveScript != -1) {
				processMoveScript(a);
			}

			processActorAnimations(a);

			if (actor->staticFlags.bIsZonable) {
				processActorZones(a);
			}

			if (actor->positionInLifeScript != -1) {
				processLifeScript(a);
			}

			processActorSamplePosition(a);

			if (quitGame != -1) {
				return quitGame;
			}

			if (actor->staticFlags.bCanDrown) {
				int32 brickSound;
				brickSound = getBrickSoundType(actor->X, actor->Y - 1, actor->Z);
				actor->brickSound = brickSound;

				if ((brickSound & 0xF0) == 0xF0) {
					if ((brickSound & 0xF) == 1) {
						if (a) { // all other actors
							int32 rnd = Rnd(2000) + 3096;
							playSample(0x25, rnd, 1, actor->X, actor->Y, actor->Z, a);
							if (actor->bonusParameter & 0x1F0) {
								if (!(actor->bonusParameter & 1)) {
									processActorExtraBonus(a);
								}
								actor->life = 0;
							}
						} else { // if Hero
							if (heroBehaviour != 4 || (brickSound & 0x0F) != actor->anim) {
								if (!cropBottomScreen)
								{
									initAnim(kDrawn, 4, 0, 0);
									projectPositionOnScreen(actor->X - cameraX, actor->Y - cameraY, actor->Z - cameraZ);
									cropBottomScreen = projPosY;
								}
								projectPositionOnScreen(actor->X - cameraX, actor->Y - cameraY, actor->Z - cameraZ);
								actor->controlMode = 0;
								actor->life = -1;
								cropBottomScreen = projPosY;
								actor->staticFlags.bCanDrown |= 0x10;
							}
						}
					}
				}
			}

			if (actor->life <= 0) {
				if (!a) { // if its Hero
					if (actor->dynamicFlags.bAnimEnded) {
						if (inventoryNumLeafs > 0) { // use clover leaf automaticaly
							sceneHero->X = newHeroX;
							sceneHero->Y = newHeroY;
							sceneHero->Z = newHeroZ;

							needChangeScene = currentSceneIdx;
							inventoryMagicPoints = magicLevelIdx * 20;

							newCameraX = (sceneHero->X >> 9);
							newCameraY = (sceneHero->Y >> 8);
							newCameraZ = (sceneHero->Z >> 9);

							heroPositionType = kReborn;

							sceneHero->life = 50;
							reqBgRedraw = 1;
							lockPalette = 1;
							inventoryNumLeafs--;
							cropBottomScreen = 0;
						} else { // game over
							inventoryNumLeafsBox = 2;
							inventoryNumLeafs = 1;
							inventoryMagicPoints = magicLevelIdx * 20;
							heroBehaviour = previousHeroBehaviour;
							actor->angle  = previousHeroAngle;
							actor->life = 50;

							if (previousSceneIdx != currentSceneIdx) {
								newHeroX = -1;
								newHeroY = -1;
								newHeroZ = -1;
								currentSceneIdx = previousSceneIdx;
							}

							saveGame();
							processGameoverAnimation();
							quitGame = 0;
							return 0;
						}
					}
				} else {
					processActorCarrier(a);
					actor->dynamicFlags.bIsDead = 1;
					actor->entity = -1;
					actor->zone = -1;
				}
			}

			if (needChangeScene != -1) {
				return 0;
			}
		}
	}

	// recenter screen automatically
	if (!disableScreenRecenter && !useFreeCamera) {
		ActorStruct *actor = &sceneActors[currentlyFollowedActor];
		projectPositionOnScreen(actor->X - (newCameraX << 9),
								   actor->Y - (newCameraY << 8),
								   actor->Z - (newCameraZ << 9));
		if (projPosX < 80 || projPosX > 539 || projPosY < 80 || projPosY > 429) {
			newCameraX = ((actor->X + 0x100) >> 9) + (((actor->X + 0x100) >> 9) - newCameraX) / 2;
			newCameraY = actor->Y >> 8;
			newCameraZ = ((actor->Z + 0x100) >> 9) + (((actor->Z + 0x100) >> 9) - newCameraZ) / 2;

			if(newCameraX >= 64) {
				newCameraX = 63;
			}

			if(newCameraZ >= 64) {
				newCameraZ = 63;
			}

			reqBgRedraw = 1;
		}
	}

	redrawEngineActions(reqBgRedraw);

	// workaround to fix hero redraw after drowning
	if(cropBottomScreen && reqBgRedraw == 1) {
		sceneHero->staticFlags.bIsHidden = 1;
		redrawEngineActions(1);
		sceneHero->staticFlags.bIsHidden = 0;
	}

	needChangeScene = -1;
	reqBgRedraw = 0;

	return 0;
}

/** Game engine main loop
	@return true if we want to show credit sequence */
int32 gameEngineLoop() { // mainLoop
	uint32 start;

	reqBgRedraw = 1;
	lockPalette = 1;
	setActorAngle(0, -256, 5, &loopMovePtr);

	while (quitGame == -1) {
		start = SDL_GetTicks();

		while (SDL_GetTicks() < start + cfgfile.Fps) {
			if (runGameEngine())
				return 1;
			SDL_Delay(1);
		}
		lbaTime++;
	}
	return 0;
}
