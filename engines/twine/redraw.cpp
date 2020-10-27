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

#include "twine/redraw.h"
#include "common/textconsole.h"
#include "graphics/surface.h"
#include "twine/actor.h"
#include "twine/animations.h"
#include "twine/collision.h"
#include "twine/debug_scene.h"
#include "twine/grid.h"
#include "twine/hqrdepack.h"
#include "twine/input.h"
#include "twine/interface.h"
#include "twine/menu.h"
#include "twine/movements.h"
#include "twine/renderer.h"
#include "twine/resources.h"
#include "twine/scene.h"
#include "twine/screens.h"
#include "twine/sound.h"
#include "twine/text.h"

namespace TwinE {

void Redraw::addRedrawCurrentArea(int32 left, int32 top, int32 right, int32 bottom) {
	int32 i = 0;

	const int32 area = (right - left) * (bottom - top);

	while (i < numOfRedrawBox) {
		int32 leftValue;
		if (currentRedrawList[i].left >= left)
			leftValue = left;
		else
			leftValue = currentRedrawList[i].left;

		int32 rightValue;
		if (currentRedrawList[i].right <= right)
			rightValue = right;
		else
			rightValue = currentRedrawList[i].right;

		int32 topValue;
		if (currentRedrawList[i].top >= top)
			topValue = top;
		else
			topValue = currentRedrawList[i].top;

		int32 bottomValue;
		if (currentRedrawList[i].bottom <= bottom)
			bottomValue = bottom;
		else
			bottomValue = currentRedrawList[i].bottom;

		if ((rightValue - leftValue) * (bottomValue - topValue) < ((currentRedrawList[i].bottom - currentRedrawList[i].top) * (currentRedrawList[i].right - currentRedrawList[i].left) + area)) {
			currentRedrawList[i].left = leftValue;
			currentRedrawList[i].top = topValue;
			currentRedrawList[i].right = rightValue;
			currentRedrawList[i].bottom = MIN<int32>(SCREEN_TEXTLIMIT_BOTTOM, bottomValue);

			assert(currentRedrawList[i].left <= currentRedrawList[i].right);
			assert(currentRedrawList[i].top <= currentRedrawList[i].bottom);
			return;
		}

		i++;
	}

	currentRedrawList[i].left = left;
	currentRedrawList[i].top = top;
	currentRedrawList[i].right = right;
	currentRedrawList[i].bottom = MIN<int32>(SCREEN_TEXTLIMIT_BOTTOM, bottom);

	assert(currentRedrawList[i].left <= currentRedrawList[i].right);
	assert(currentRedrawList[i].top <= currentRedrawList[i].bottom);

	numOfRedrawBox++;
}

void Redraw::addRedrawArea(int32 left, int32 top, int32 right, int32 bottom) {
	if (left < 0) {
		left = 0;
	}
	if (top < 0) {
		top = 0;
	}
	if (right >= SCREEN_WIDTH) {
		right = SCREEN_TEXTLIMIT_RIGHT;
	}
	if (bottom >= SCREEN_HEIGHT) {
		bottom = SCREEN_TEXTLIMIT_BOTTOM;
	}

	if (left > right || top > bottom) {
		return;
	}

	nextRedrawList[currNumOfRedrawBox].left = left;
	nextRedrawList[currNumOfRedrawBox].top = top;
	nextRedrawList[currNumOfRedrawBox].right = right;
	nextRedrawList[currNumOfRedrawBox].bottom = bottom;

	currNumOfRedrawBox++;

	addRedrawCurrentArea(left, top, right, bottom);
}

void Redraw::moveNextAreas() {
	numOfRedrawBox = 0;

	for (int32 i = 0; i < currNumOfRedrawBox; i++) {
		addRedrawCurrentArea(nextRedrawList[i].left, nextRedrawList[i].top, nextRedrawList[i].right, nextRedrawList[i].bottom);
	}
}

void Redraw::flipRedrawAreas() {
	for (int32 i = 0; i < numOfRedrawBox; i++) { // redraw areas on screen
		_engine->copyBlockPhys(currentRedrawList[i].left, currentRedrawList[i].top, currentRedrawList[i].right, currentRedrawList[i].bottom);
	}

	numOfRedrawBox = 0;

	for (int32 i = 0; i < currNumOfRedrawBox; i++) { //setup the redraw areas for next display
		addRedrawCurrentArea(nextRedrawList[i].left, nextRedrawList[i].top, nextRedrawList[i].right, nextRedrawList[i].bottom);
	}
}

void Redraw::blitBackgroundAreas() {
	const RedrawStruct *currentArea = currentRedrawList;

	for (int32 i = 0; i < numOfRedrawBox; i++) {
		_engine->_interface->blitBox(currentArea->left, currentArea->top, currentArea->right, currentArea->bottom, (const int8 *)_engine->workVideoBuffer.getPixels(), currentArea->left, currentArea->top, (int8 *)_engine->frontVideoBuffer.getPixels());
		currentArea++;
	}
}

void Redraw::sortDrawingList(DrawListStruct *list, int32 listSize) {
	for (int32 i = 0; i < listSize - 1; i++) {
		for (int32 j = 0; j < listSize - 1 - i; j++) {
			if (list[j + 1].posValue < list[j].posValue) {
				DrawListStruct tempStruct;
				memcpy(&tempStruct, &list[j + 1], sizeof(DrawListStruct));
				memcpy(&list[j + 1], &list[j], sizeof(DrawListStruct));
				memcpy(&list[j], &tempStruct, sizeof(DrawListStruct));
			}
		}
	}
}

void Redraw::addOverlay(int16 type, int16 info0, int16 x, int16 y, int16 info1, int16 posType, int16 lifeTime) {
	for (int32 i = 0; i < ARRAYSIZE(overlayList); i++) {
		OverlayListStruct *overlay = &overlayList[i];
		if (overlay->info0 == -1) {
			overlay->type = type;
			overlay->info0 = info0;
			overlay->x = x;
			overlay->y = y;
			overlay->info1 = info1;
			overlay->posType = posType;
			overlay->lifeTime = _engine->lbaTime + lifeTime * 50;
			break;
		}
	}
}

void Redraw::updateOverlayTypePosition(int16 X1, int16 Y1, int16 X2, int16 Y2) {
	const int16 newX = X2 - X1;
	const int16 newY = Y2 - Y1;

	for (int32 i = 0; i < ARRAYSIZE(overlayList); i++) {
		OverlayListStruct *overlay = &overlayList[i];
		if (overlay->type == koFollowActor) {
			overlay->x = newX;
			overlay->y = newY;
		}
	}
}

// TODO: convert to bool and check if this isn't always true...
void Redraw::redrawEngineActions(int32 bgRedraw) { // fullRedraw
	int16 tmp_projPosX;
	int16 tmp_projPosY;
	ActorStruct *actor;

	tmp_projPosX = _engine->_renderer->projPosXScreen;
	tmp_projPosY = _engine->_renderer->projPosYScreen;

	_engine->_interface->resetClip();

	if (bgRedraw) {
		_engine->freezeTime();
		if (_engine->_scene->needChangeScene != -1 && _engine->_scene->needChangeScene != -2)
			_engine->_screens->fadeOut(_engine->_screens->paletteRGBA);
		_engine->_screens->clearScreen();
		_engine->_grid->redrawGrid();
		updateOverlayTypePosition(tmp_projPosX, tmp_projPosY, _engine->_renderer->projPosXScreen, _engine->_renderer->projPosYScreen);
		_engine->_screens->copyScreen(_engine->frontVideoBuffer, _engine->workVideoBuffer);

		if (_engine->_scene->needChangeScene != -1 && _engine->_scene->needChangeScene != -2) {
			_engine->_screens->fadeIn(_engine->_screens->paletteRGBA);
			_engine->setPalette(_engine->_screens->paletteRGBA);
		}
	} else {
		blitBackgroundAreas();
	}

	// first loop

	int32 modelActorPos = 0;
	int32 drawListPos = 0;
	int32 spriteActorPos = 0x1000;
	int32 shadowActorPos = 0x0C00;

	// Process actors drawing list
	for (modelActorPos = 0; modelActorPos < _engine->_scene->sceneNumActors; modelActorPos++, spriteActorPos++, shadowActorPos++) {
		actor = _engine->_scene->getActor(modelActorPos);
		actor->dynamicFlags.bIsVisible = 0; // reset visible state

		if (_engine->_grid->useCellingGrid == -1 || actor->y <= (*(int16 *)(_engine->_grid->cellingGridIdx * 24 + (int8 *)_engine->_scene->sceneZones + 8))) {
			// no redraw required
			if (actor->staticFlags.bIsBackgrounded && bgRedraw == 0) {
				// get actor position on screen
				_engine->_renderer->projectPositionOnScreen(actor->x - _engine->_grid->cameraX, actor->y - _engine->_grid->cameraY, actor->z - _engine->_grid->cameraZ);

				// check if actor is visible on screen, otherwise don't display it
				if (_engine->_renderer->projPosX > -50 && _engine->_renderer->projPosX < 680 && _engine->_renderer->projPosY > -30 && _engine->_renderer->projPosY < 580) {
					actor->dynamicFlags.bIsVisible = 1;
				}
			} else {
				// if the actor isn't set as hidden
				if (actor->entity != -1 && !(actor->staticFlags.bIsHidden)) {
					// get actor position on screen
					_engine->_renderer->projectPositionOnScreen(actor->x - _engine->_grid->cameraX, actor->y - _engine->_grid->cameraY, actor->z - _engine->_grid->cameraZ);

					if ((actor->staticFlags.bUsesClipping && _engine->_renderer->projPosX > -112 && _engine->_renderer->projPosX < 752 && _engine->_renderer->projPosY > -50 && _engine->_renderer->projPosY < 651) ||
					    ((!actor->staticFlags.bUsesClipping) && _engine->_renderer->projPosX > -50 && _engine->_renderer->projPosX < 680 && _engine->_renderer->projPosY > -30 && _engine->_renderer->projPosY < 580)) {

						int32 tmpVal = actor->z + actor->x - _engine->_grid->cameraX - _engine->_grid->cameraZ;

						// if actor is above another actor
						if (actor->standOn != -1) {
							const ActorStruct *standOnActor = _engine->_scene->getActor(actor->standOn);
							tmpVal = standOnActor->x - _engine->_grid->cameraX + standOnActor->z - _engine->_grid->cameraZ + 2;
						}

						if (actor->staticFlags.bIsSpriteActor) {
							drawList[drawListPos].index = spriteActorPos; // > 0x1000
							if (actor->staticFlags.bUsesClipping) {
								tmpVal = actor->lastX - _engine->_grid->cameraX + actor->lastZ - _engine->_grid->cameraZ;
							}
						} else {
							drawList[drawListPos].index = modelActorPos;
						}

						drawList[drawListPos].posValue = tmpVal;

						drawListPos++;

						// if use shadows
						if (_engine->cfgfile.ShadowMode != 0 && !(actor->staticFlags.bDoesntCastShadow)) {
							if (actor->standOn != -1) {
								_engine->_actor->shadowX = actor->x;
								_engine->_actor->shadowY = actor->y - 1;
								_engine->_actor->shadowZ = actor->z;
							} else {
								_engine->_movements->getShadowPosition(actor->x, actor->y, actor->z);
							}

							tmpVal--;
							drawList[drawListPos].posValue = tmpVal; // save the shadow entry in the drawList
							drawList[drawListPos].index = 0xC00;     // shadowActorPos
							drawList[drawListPos].x = _engine->_actor->shadowX;
							drawList[drawListPos].y = _engine->_actor->shadowY;
							drawList[drawListPos].z = _engine->_actor->shadowZ;
							drawList[drawListPos].field_A = 2;
							drawListPos++;
						}
					}
				}
			}
		}
	}

	// second loop
	for (int32 i = 0; i < EXTRA_MAX_ENTRIES; i++) {
		ExtraListStruct *extra = &_engine->_extra->extraList[i];
		if (extra->info0 != -1) {
			if (extra->type & 0x400) {
				if (_engine->lbaTime - extra->lifeTime > 35) {
					extra->lifeTime = _engine->lbaTime;
					extra->type &= 0xFBFF;
					// FIXME make constant for sample index
					_engine->_sound->playSample(11, 4096, 1, extra->x, extra->y, extra->z);
				}
			} else {
				if ((extra->type & 1) || (extra->type & 0x40) || (extra->actorIdx + extra->lifeTime - 150 < _engine->lbaTime) || (!((_engine->lbaTime + extra->lifeTime) & 8))) {
					_engine->_renderer->projectPositionOnScreen(extra->x - _engine->_grid->cameraX, extra->y - _engine->_grid->cameraY, extra->z - _engine->_grid->cameraZ);

					if (_engine->_renderer->projPosX > -50 && _engine->_renderer->projPosX < 680 && _engine->_renderer->projPosY > -30 && _engine->_renderer->projPosY < 580) {
						drawList[drawListPos].posValue = extra->x - _engine->_grid->cameraX + extra->z - _engine->_grid->cameraZ;
						drawList[drawListPos].index = 0x1800 + i;
						drawListPos++;

						if (_engine->cfgfile.ShadowMode == 2 && !(extra->info0 & 0x8000)) {
							_engine->_movements->getShadowPosition(extra->x, extra->y, extra->z);

							drawList[drawListPos].posValue = extra->x - _engine->_grid->cameraX + extra->z - _engine->_grid->cameraZ - 1;
							drawList[drawListPos].index = 0xC00;
							drawList[drawListPos].x = _engine->_actor->shadowX;
							drawList[drawListPos].y = _engine->_actor->shadowY;
							drawList[drawListPos].z = _engine->_actor->shadowZ;
							drawList[drawListPos].field_A = 0;
							drawListPos++;
						}
					}
				}
			}
		}
	}

	sortDrawingList(drawList, drawListPos);

	currNumOfRedrawBox = 0;
	// if has something to draw
	if (drawListPos > 0) {
		int32 pos = 0;

		do {
			int32 actorIdx = drawList[pos].index & 0x3FF;
			ActorStruct *actor2 = _engine->_scene->getActor(actorIdx);
			uint32 flags = ((uint32)drawList[pos].index) & 0xFC00;

			// Drawing actors
			if (flags < 0xC00) {
				if (!flags) {
					_engine->_animations->setModelAnimation(actor2->animPosition, _engine->_animations->animTable[actor2->previousAnimIdx], _engine->_actor->bodyTable[actor2->entity], &actor2->animTimerData);

					if (!_engine->_renderer->renderIsoModel(actor2->x - _engine->_grid->cameraX, actor2->y - _engine->_grid->cameraY, actor2->z - _engine->_grid->cameraZ, 0, actor2->angle, 0, _engine->_actor->bodyTable[actor2->entity])) {
						if (renderLeft < 0) {
							renderLeft = SCREEN_TEXTLIMIT_LEFT;
						}

						if (renderTop < 0) {
							renderTop = SCREEN_TEXTLIMIT_TOP;
						}

						if (renderRight >= SCREEN_WIDTH) {
							renderRight = SCREEN_TEXTLIMIT_RIGHT;
						}

						if (renderBottom >= SCREEN_HEIGHT) {
							renderBottom = SCREEN_TEXTLIMIT_BOTTOM;
						}

						_engine->_interface->setClip(renderLeft, renderTop, renderRight, renderBottom);

						if (_engine->_interface->textWindowLeft <= _engine->_interface->textWindowRight && _engine->_interface->textWindowTop <= _engine->_interface->textWindowBottom) {
							actor2->dynamicFlags.bIsVisible = 1;

							const int32 tempX = (actor2->x + 0x100) >> 9;
							int32 tempY = actor2->y >> 8;
							if (actor2->brickShape & 0x7F) {
								tempY++;
							}

							const int32 tempZ = (actor2->z + 0x100) >> 9;

							_engine->_grid->drawOverModelActor(tempX, tempY, tempZ);

							if (_engine->_actor->cropBottomScreen) {
								renderBottom = _engine->_interface->textWindowBottom = _engine->_actor->cropBottomScreen + 10;
							}

							addRedrawArea(_engine->_interface->textWindowLeft, _engine->_interface->textWindowTop, renderRight, renderBottom);

							if (actor2->staticFlags.bIsBackgrounded && bgRedraw == 1) {
								_engine->_interface->blitBox(_engine->_interface->textWindowLeft, _engine->_interface->textWindowTop, renderRight, renderBottom, (const int8 *)_engine->frontVideoBuffer.getPixels(), _engine->_interface->textWindowLeft, _engine->_interface->textWindowTop, (int8 *)_engine->workVideoBuffer.getPixels());
							}
						}
					}
				}
			}
			// Drawing shadows
			else if (flags == 0xC00 && !_engine->_actor->cropBottomScreen) {
				DrawListStruct shadow = drawList[pos];

				// get actor position on screen
				_engine->_renderer->projectPositionOnScreen(shadow.x - _engine->_grid->cameraX, shadow.y - _engine->_grid->cameraY, shadow.z - _engine->_grid->cameraZ);

				int32 spriteWidth, spriteHeight;
				_engine->_grid->getSpriteSize(shadow.field_A, &spriteWidth, &spriteHeight, _engine->_scene->spriteShadowPtr);

				// calculate sprite size and position on screen
				renderLeft = _engine->_renderer->projPosX - (spriteWidth / 2);
				renderTop = _engine->_renderer->projPosY - (spriteHeight / 2);
				renderRight = _engine->_renderer->projPosX + (spriteWidth / 2);
				renderBottom = _engine->_renderer->projPosY + (spriteHeight / 2);

				_engine->_interface->setClip(renderLeft, renderTop, renderRight, renderBottom);

				if (_engine->_interface->textWindowLeft <= _engine->_interface->textWindowRight && _engine->_interface->textWindowTop <= _engine->_interface->textWindowBottom) {
					_engine->_grid->drawSprite(shadow.field_A, renderLeft, renderTop, _engine->_scene->spriteShadowPtr);
				}

				const int32 tmpX = (shadow.x + 0x100) >> 9;
				const int32 tmpY = shadow.y >> 8;
				const int32 tmpZ = (shadow.z + 0x100) >> 9;

				_engine->_grid->drawOverModelActor(tmpX, tmpY, tmpZ);

				addRedrawArea(_engine->_interface->textWindowLeft, _engine->_interface->textWindowTop, renderRight, renderBottom);

				// show clipping area
				//drawBox(_engine->_renderer->renderLeft, _engine->_renderer->renderTop, _engine->_renderer->renderRight, _engine->_renderer->renderBottom);
			}
			// Drawing unknown
			else if (flags < 0x1000) {
				// TODO reverse this part of the code
			}
			// Drawing sprite actors
			else if (flags == 0x1000) {
				int32 spriteWidth, spriteHeight;
				uint8 *spritePtr = _engine->_actor->spriteTable[actor2->entity];

				// get actor position on screen
				_engine->_renderer->projectPositionOnScreen(actor2->x - _engine->_grid->cameraX, actor2->y - _engine->_grid->cameraY, actor2->z - _engine->_grid->cameraZ);

				_engine->_grid->getSpriteSize(0, &spriteWidth, &spriteHeight, spritePtr);

				// calculate sprite position on screen
				renderLeft = _engine->_renderer->projPosX + *((int16 *)(_engine->_scene->spriteBoundingBoxPtr + (actor2->entity * 16)));
				renderTop = _engine->_renderer->projPosY + *((int16 *)(_engine->_scene->spriteBoundingBoxPtr + (actor2->entity * 16) + 2));
				renderRight = renderLeft + spriteWidth;
				renderBottom = renderTop + spriteHeight;

				if (actor2->staticFlags.bUsesClipping) {
					_engine->_interface->setClip(_engine->_renderer->projPosXScreen + actor2->info0, _engine->_renderer->projPosYScreen + actor2->info1, _engine->_renderer->projPosXScreen + actor2->info2, _engine->_renderer->projPosYScreen + actor2->info3);
				} else {
					_engine->_interface->setClip(renderLeft, renderTop, renderRight, renderBottom);
				}

				if (_engine->_interface->textWindowLeft <= _engine->_interface->textWindowRight && _engine->_interface->textWindowTop <= _engine->_interface->textWindowBottom) {
					_engine->_grid->drawSprite(0, renderLeft, renderTop, spritePtr);

					actor2->dynamicFlags.bIsVisible = 1;

					if (actor2->staticFlags.bUsesClipping) {
						_engine->_grid->drawOverSpriteActor((actor2->lastX + 0x100) >> 9, actor2->lastY >> 8, (actor2->lastZ + 0x100) >> 9);
					} else {
						const int32 tmpX = (actor2->x + actor2->boudingBox.x.topRight + 0x100) >> 9;
						int32 tmpY = actor2->y >> 8;
						if (actor2->brickShape & 0x7F) {
							tmpY++;
						}
						const int32 tmpZ = (actor2->z + actor2->boudingBox.z.topRight + 0x100) >> 9;

						_engine->_grid->drawOverSpriteActor(tmpX, tmpY, tmpZ);
					}

					addRedrawArea(_engine->_interface->textWindowLeft, _engine->_interface->textWindowTop, _engine->_interface->textWindowRight, _engine->_interface->textWindowBottom);

					if (actor2->staticFlags.bIsBackgrounded && bgRedraw == 1) {
						_engine->_interface->blitBox(_engine->_interface->textWindowLeft, _engine->_interface->textWindowTop, _engine->_interface->textWindowRight, _engine->_interface->textWindowBottom, (const int8 *)_engine->frontVideoBuffer.getPixels(), _engine->_interface->textWindowLeft, _engine->_interface->textWindowTop, (int8 *)_engine->workVideoBuffer.getPixels());
					}

					// show clipping area
					//drawBox(renderLeft, renderTop, renderRight, renderBottom);
				}
			}
			// Drawing extras
			else if (flags == 0x1800) {
				ExtraListStruct *extra = &_engine->_extra->extraList[actorIdx];

				_engine->_renderer->projectPositionOnScreen(extra->x - _engine->_grid->cameraX, extra->y - _engine->_grid->cameraY, extra->z - _engine->_grid->cameraZ);

				if (extra->info0 & 0x8000) {
					_engine->_extra->drawExtraSpecial(actorIdx, _engine->_renderer->projPosX, _engine->_renderer->projPosY);
				} else {
					int32 spriteWidth, spriteHeight;
					_engine->_grid->getSpriteSize(0, &spriteWidth, &spriteHeight, _engine->_actor->spriteTable[extra->info0]);

					// calculate sprite position on screen
					renderLeft = _engine->_renderer->projPosX + *(int16 *)(_engine->_scene->spriteBoundingBoxPtr + extra->info0 * 16);
					renderTop = _engine->_renderer->projPosY + *(int16 *)(_engine->_scene->spriteBoundingBoxPtr + extra->info0 * 16 + 2);
					renderRight = renderLeft + spriteWidth;
					renderBottom = renderTop + spriteHeight;

					_engine->_grid->drawSprite(0, renderLeft, renderTop, _engine->_actor->spriteTable[extra->info0]);
				}

				_engine->_interface->setClip(renderLeft, renderTop, renderRight, renderBottom);

				if (_engine->_interface->textWindowLeft <= _engine->_interface->textWindowRight && _engine->_interface->textWindowTop <= _engine->_interface->textWindowBottom) {
					const int32 tmpX = (drawList[pos].x + 0x100) >> 9;
					const int32 tmpY = drawList[pos].y >> 8;
					const int32 tmpZ = (drawList[pos].z + 0x100) >> 9;

					_engine->_grid->drawOverModelActor(tmpX, tmpY, tmpZ);
					addRedrawArea(_engine->_interface->textWindowLeft, _engine->_interface->textWindowTop, renderRight, renderBottom);

					// show clipping area
					//drawBox(renderLeft, renderTop, renderRight, renderBottom);
				}
			}

			_engine->_interface->resetClip();
			pos++;
		} while (pos < drawListPos);
	}

	if (_engine->cfgfile.Debug) {
		_engine->_debugScene->displayZones();
	}

	for (int32 i = 0; i < OVERLAY_MAX_ENTRIES; i++) {
		OverlayListStruct *overlay = &overlayList[i];
		if (overlay->info0 != -1) {
			// process position overlay
			switch (overlay->posType) {
			case koNormal:
				if (_engine->lbaTime >= overlay->lifeTime) {
					overlay->info0 = -1;
					continue;
				}
				break;
			case koFollowActor: {
				ActorStruct *actor2 = _engine->_scene->getActor(overlay->info1);

				_engine->_renderer->projectPositionOnScreen(actor2->x - _engine->_grid->cameraX, actor2->y + actor2->boudingBox.y.topRight - _engine->_grid->cameraY, actor2->z - _engine->_grid->cameraZ);

				overlay->x = _engine->_renderer->projPosX;
				overlay->y = _engine->_renderer->projPosY;

				if (_engine->lbaTime >= overlay->lifeTime) {
					overlay->info0 = -1;
					continue;
				}
				break;
			}
			}

			// process overlay type
			switch (overlay->type) {
			case koSprite: {
				int16 offsetX, offsetY;
				int32 spriteWidth, spriteHeight;
				uint8 *spritePtr = _engine->_actor->spriteTable[overlay->info0];

				_engine->_grid->getSpriteSize(0, &spriteWidth, &spriteHeight, spritePtr);

				offsetX = *((int16 *)(_engine->_scene->spriteBoundingBoxPtr + (overlay->info0 * 16)));
				offsetY = *((int16 *)(_engine->_scene->spriteBoundingBoxPtr + (overlay->info0 * 16) + 2));

				renderLeft = offsetX + overlay->x;
				renderTop = offsetY + overlay->y;
				renderRight = renderLeft + spriteWidth;
				renderBottom = renderTop + spriteHeight;

				_engine->_grid->drawSprite(0, renderLeft, renderTop, spritePtr);

				if (_engine->_interface->textWindowLeft <= _engine->_interface->textWindowRight && _engine->_interface->textWindowTop <= _engine->_interface->textWindowBottom) {
					addRedrawArea(_engine->_interface->textWindowLeft, _engine->_interface->textWindowTop, renderRight, renderBottom);
				}
				break;
			}
			case koNumber: {
				int32 textLength, textHeight;
				char text[10];
				snprintf(text, sizeof(text), "%d", overlay->info0);

				textLength = _engine->_text->getTextSize(text);
				textHeight = 48;

				renderLeft = overlay->x - (textLength / 2);
				renderTop = overlay->y - 24;
				renderRight = overlay->x + (textLength / 2);
				renderBottom = overlay->y + textHeight;

				_engine->_interface->setClip(renderLeft, renderTop, renderRight, renderBottom);

				_engine->_text->setFontColor(overlay->info1);

				_engine->_text->drawText(renderLeft, renderTop, text);

				if (_engine->_interface->textWindowLeft <= _engine->_interface->textWindowRight && _engine->_interface->textWindowTop <= _engine->_interface->textWindowBottom) {
					addRedrawArea(_engine->_interface->textWindowLeft, _engine->_interface->textWindowTop, renderRight, renderBottom);
				}
				break;
			}
			case koNumberRange: {
				int32 textLength, textHeight, range;
				char text[10];

				range = _engine->_collision->getAverageValue(overlay->info1, overlay->info0, 100, overlay->lifeTime - _engine->lbaTime - 50);

				sprintf(text, "%d", range);

				textLength = _engine->_text->getTextSize(text);
				textHeight = 48;

				renderLeft = overlay->x - (textLength / 2);
				renderTop = overlay->y - 24;
				renderRight = overlay->x + (textLength / 2);
				renderBottom = overlay->y + textHeight;

				_engine->_interface->setClip(renderLeft, renderTop, renderRight, renderBottom);

				_engine->_text->setFontColor(155);

				_engine->_text->drawText(renderLeft, renderTop, text);

				if (_engine->_interface->textWindowLeft <= _engine->_interface->textWindowRight && _engine->_interface->textWindowTop <= _engine->_interface->textWindowBottom) {
					addRedrawArea(_engine->_interface->textWindowLeft, _engine->_interface->textWindowTop, renderRight, renderBottom);
				}
				break;
			}
			case koInventoryItem: {
				int32 item = overlay->info0;

				_engine->_interface->drawSplittedBox(10, 10, 69, 69, 0);
				_engine->_interface->setClip(10, 10, 69, 69);

				_engine->_renderer->prepareIsoModel(_engine->_resources->inventoryTable[item]);
				_engine->_renderer->setCameraPosition(40, 40, 128, 200, 200);
				_engine->_renderer->setCameraAngle(0, 0, 0, 60, 0, 0, 16000);

				overlayRotation += 1; // overlayRotation += 8;

				_engine->_renderer->renderIsoModel(0, 0, 0, 0, overlayRotation, 0, _engine->_resources->inventoryTable[item]);
				_engine->_menu->drawBox(10, 10, 69, 69);
				addRedrawArea(10, 10, 69, 69);
				_engine->_gameState->initEngineProjections();
				break;
			}
			case koText: {
				char text[256];

				_engine->_text->getMenuText(overlay->info0, text, sizeof(text));

				int32 textLength = _engine->_text->getTextSize(text);
				int32 textHeight = 48;

				renderLeft = overlay->x - (textLength / 2);
				renderTop = overlay->y - 24;
				renderRight = overlay->x + (textLength / 2);
				renderBottom = overlay->y + textHeight;

				if (renderLeft < 0) {
					renderLeft = 0;
				}

				if (renderTop < 0) {
					renderTop = 0;
				}

				if (renderRight > SCREEN_TEXTLIMIT_RIGHT) {
					renderRight = SCREEN_TEXTLIMIT_RIGHT;
				}

				if (renderBottom > SCREEN_TEXTLIMIT_BOTTOM) {
					renderBottom = SCREEN_TEXTLIMIT_BOTTOM;
				}

				_engine->_interface->setClip(renderLeft, renderTop, renderRight, renderBottom);

				_engine->_text->setFontColor(_engine->_scene->getActor(overlay->info1)->talkColor);

				_engine->_text->drawText(renderLeft, renderTop, text);

				if (_engine->_interface->textWindowLeft <= _engine->_interface->textWindowRight && _engine->_interface->textWindowTop <= _engine->_interface->textWindowBottom) {
					addRedrawArea(_engine->_interface->textWindowLeft, _engine->_interface->textWindowTop, renderRight, renderBottom);
				}
				break;
			}
			}
		}
	}

	_engine->_interface->resetClip();

	// make celling grid fade
	// need to be here to fade after drawing all actors in scene
	if (_engine->_scene->needChangeScene == -2) {
		_engine->crossFade(_engine->frontVideoBuffer, _engine->_screens->paletteRGBA);
		_engine->_scene->needChangeScene = -1;
	}

	if (bgRedraw) {
		_engine->flip();
		moveNextAreas();
		_engine->unfreezeTime();
	} else {
		flipRedrawAreas();
	}

	if (_engine->_screens->lockPalette) {
		if (_engine->_screens->useAlternatePalette) {
			_engine->_screens->fadeToPal(_engine->_screens->paletteRGBA);
		} else {
			_engine->_screens->fadeToPal(_engine->_screens->mainPaletteRGBA);
		}
		_engine->_screens->lockPalette = false;
	}

	if (_engine->zoomScreen) {
		//zoomScreenScale();
	}
}

void Redraw::drawBubble(int32 actorIdx) {
	int32 spriteWidth, spriteHeight;
	uint8 *spritePtr;
	ActorStruct *actor = _engine->_scene->getActor(actorIdx);

	// get actor position on screen
	_engine->_renderer->projectPositionOnScreen(actor->x - _engine->_grid->cameraX, actor->y + actor->boudingBox.y.topRight - _engine->_grid->cameraY, actor->z - _engine->_grid->cameraZ);

	if (actorIdx != bubbleActor) {
		bubbleSpriteIndex = bubbleSpriteIndex ^ 1;
		bubbleActor = actorIdx;
	}

	spritePtr = _engine->_actor->spriteTable[bubbleSpriteIndex];
	_engine->_grid->getSpriteSize(0, &spriteWidth, &spriteHeight, spritePtr);

	// calculate sprite position on screen
	if (bubbleSpriteIndex == SPRITEHQR_DIAG_BUBBLE_RIGHT) {
		renderLeft = _engine->_renderer->projPosX + 10;
	} else {
		renderLeft = _engine->_renderer->projPosX - 10 - spriteWidth;
	}
	renderTop = _engine->_renderer->projPosY - 20;
	renderRight = spriteWidth + renderLeft - 1;
	renderBottom = spriteHeight + renderTop - 1;

	_engine->_interface->setClip(renderLeft, renderTop, renderRight, renderBottom);

	_engine->_grid->drawSprite(0, renderLeft, renderTop, spritePtr);
	if (_engine->_interface->textWindowLeft <= _engine->_interface->textWindowRight && _engine->_interface->textWindowTop <= _engine->_interface->textWindowBottom) {
		_engine->copyBlockPhys(renderLeft, renderTop, renderRight, renderBottom);
	}

	_engine->_interface->resetClip();
}

void Redraw::zoomScreenScale() {
	Graphics::ManagedSurface zoomWorkVideoBuffer;
	zoomWorkVideoBuffer.copyFrom(_engine->workVideoBuffer);

	// TODO: this is broken
	const uint8 *src = (const uint8 *)zoomWorkVideoBuffer.getPixels();
	uint8 *dest = (uint8 *)_engine->workVideoBuffer.getPixels();
	for (int h = 0; h < zoomWorkVideoBuffer.h; h++) {
		for (int w = 0; w < zoomWorkVideoBuffer.w; w++) {
			*dest++ = *src;
			*dest++ = *src++;
		}
		//memcpy(dest, dest - SCREEN_WIDTH, SCREEN_WIDTH);
		//dest += SCREEN_WIDTH;
	}
	_engine->_screens->copyScreen(_engine->workVideoBuffer, _engine->frontVideoBuffer);
	zoomWorkVideoBuffer.free();
}

} // namespace TwinE
