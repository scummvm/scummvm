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

#include "twine/renderer/redraw.h"
#include "common/memstream.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "common/util.h"
#include "graphics/surface.h"
#include "twine/audio/sound.h"
#include "twine/debugger/debug_scene.h"
#include "twine/input.h"
#include "twine/menu/interface.h"
#include "twine/menu/menu.h"
#include "twine/parser/sprite.h"
#include "twine/renderer/renderer.h"
#include "twine/renderer/screens.h"
#include "twine/resources/hqr.h"
#include "twine/resources/resources.h"
#include "twine/scene/actor.h"
#include "twine/scene/animations.h"
#include "twine/scene/collision.h"
#include "twine/scene/grid.h"
#include "twine/scene/movements.h"
#include "twine/scene/scene.h"
#include "twine/text.h"

namespace TwinE {

void Redraw::addRedrawCurrentArea(const Common::Rect &redrawArea) {
	const int32 area = (redrawArea.right - redrawArea.left) * (redrawArea.bottom - redrawArea.top);

	for (int32 i = 0; i < numOfRedrawBox; ++i) {
		Common::Rect &rect = _currentRedrawList[i];
		const int32 leftValue = MIN<int32>(redrawArea.left, rect.left);
		const int32 rightValue = MAX<int32>(redrawArea.right, rect.right);
		const int32 topValue = MIN<int32>(redrawArea.top, rect.top);
		const int32 bottomValue = MAX<int32>(redrawArea.bottom, rect.bottom);

		const int32 areaValue = (rightValue - leftValue) * (bottomValue - topValue);
		const int32 areaValueDiff = ((rect.right - rect.left) * (rect.bottom - rect.top) + area);
		if (areaValue < areaValueDiff) {
			rect.left = leftValue;
			rect.top = topValue;
			rect.right = rightValue;
			rect.bottom = MIN<int32>((_engine->height() - 1), bottomValue);

			assert(rect.left <= rect.right);
			assert(rect.top <= rect.bottom);
			return;
		}
	}

	Common::Rect &rect = _currentRedrawList[numOfRedrawBox];
	rect.left = redrawArea.left;
	rect.top = redrawArea.top;
	rect.right = redrawArea.right;
	rect.bottom = MIN<int32>((_engine->height() - 1), redrawArea.bottom);

	assert(rect.left <= rect.right);
	assert(rect.top <= rect.bottom);

	numOfRedrawBox++;
}

void Redraw::addRedrawArea(const Common::Rect &rect) {
	if (!rect.isValidRect()) {
		return;
	}
	addRedrawArea(rect.left, rect.top, rect.right, rect.bottom);
}

void Redraw::addRedrawArea(int32 left, int32 top, int32 right, int32 bottom) {
	if (left < 0) {
		left = 0;
	}
	if (top < 0) {
		top = 0;
	}
	if (right >= _engine->width()) {
		right = (_engine->width() - 1);
	}
	if (bottom >= _engine->height()) {
		bottom = (_engine->height() - 1);
	}

	if (left > right || top > bottom) {
		return;
	}

	Common::Rect &rect = _nextRedrawList[currNumOfRedrawBox];
	rect.left = left;
	rect.top = top;
	rect.right = right;
	rect.bottom = bottom;

	currNumOfRedrawBox++;

	addRedrawCurrentArea(rect);
}

void Redraw::moveNextAreas() {
	numOfRedrawBox = 0;

	for (int32 i = 0; i < currNumOfRedrawBox; i++) {
		addRedrawCurrentArea(_nextRedrawList[i]);
	}
}

void Redraw::flipRedrawAreas() {
	for (int32 i = 0; i < numOfRedrawBox; i++) { // redraw areas on screen
		_engine->copyBlockPhys(_currentRedrawList[i].left, _currentRedrawList[i].top, _currentRedrawList[i].right, _currentRedrawList[i].bottom);
	}

	numOfRedrawBox = 0;

	for (int32 i = 0; i < currNumOfRedrawBox; i++) { //setup the redraw areas for next display
		addRedrawCurrentArea(_nextRedrawList[i]);
	}
}

void Redraw::blitBackgroundAreas() {
	for (int32 i = 0; i < numOfRedrawBox; i++) {
		_engine->_interface->blitBox(_currentRedrawList[i], _engine->workVideoBuffer, _engine->frontVideoBuffer);
	}
}

void Redraw::sortDrawingList(DrawListStruct *list, int32 listSize) {
	if (listSize == 1) {
		return;
	}
	for (int32 i = 0; i < listSize - 1; i++) {
		for (int32 j = 0; j < listSize - 1 - i; j++) {
			if (list[j + 1].posValue < list[j].posValue) {
				SWAP(list[j + 1], list[j]);
			}
		}
	}
}

void Redraw::addOverlay(OverlayType type, int16 info0, int16 x, int16 y, int16 info1, OverlayPosType posType, int16 lifeTime) {
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

void Redraw::updateOverlayTypePosition(int16 x1, int16 y1, int16 x2, int16 y2) {
	const int16 newX = x2 - x1;
	const int16 newY = y2 - y1;

	for (int32 i = 0; i < ARRAYSIZE(overlayList); i++) {
		OverlayListStruct *overlay = &overlayList[i];
		if (overlay->posType == OverlayPosType::koFollowActor) {
			overlay->x = newX;
			overlay->y = newY;
		}
	}
}

int32 Redraw::fillActorDrawingList(bool bgRedraw) {
	int32 drawListPos = 0;
	for (int32 modelActorPos = 0; modelActorPos < _engine->_scene->sceneNumActors; modelActorPos++) {
		ActorStruct *actor = _engine->_scene->getActor(modelActorPos);
		actor->dynamicFlags.bIsVisible = 0; // reset visible state

		if (_engine->_grid->useCellingGrid != -1 && actor->pos.y > _engine->_scene->sceneZones[_engine->_grid->cellingGridIdx].maxs.y) {
			continue;
		}
		// no redraw required
		if (actor->staticFlags.bIsBackgrounded && !bgRedraw) {
			// get actor position on screen
			_engine->_renderer->projectPositionOnScreen(actor->pos - _engine->_grid->camera);

			// check if actor is visible on screen, otherwise don't display it
			if (_engine->_renderer->projPos.x > -50 && _engine->_renderer->projPos.x < _engine->width() + 40 && _engine->_renderer->projPos.y > -30 && _engine->_renderer->projPos.y < _engine->height() + 100) {
				actor->dynamicFlags.bIsVisible = 1;
			}
			continue;
		}
		// if the actor isn't set as hidden
		if (actor->entity == -1 || actor->staticFlags.bIsHidden) {
			continue;
		}
		// get actor position on screen
		_engine->_renderer->projectPositionOnScreen(actor->pos - _engine->_grid->camera);

		if ((actor->staticFlags.bUsesClipping && _engine->_renderer->projPos.x > -112 && _engine->_renderer->projPos.x < _engine->width() + 112 && _engine->_renderer->projPos.y > -50 && _engine->_renderer->projPos.y < _engine->height() + 171) ||
		    ((!actor->staticFlags.bUsesClipping) && _engine->_renderer->projPos.x > -50 && _engine->_renderer->projPos.x < _engine->width() + 40 && _engine->_renderer->projPos.y > -30 && _engine->_renderer->projPos.y < _engine->height() + 100)) {

			int32 tmpVal = actor->pos.z + actor->pos.x - _engine->_grid->camera.x - _engine->_grid->camera.z;

			// if actor is above another actor
			if (actor->standOn != -1) {
				const ActorStruct *standOnActor = _engine->_scene->getActor(actor->standOn);
				tmpVal = standOnActor->pos.x - _engine->_grid->camera.x + standOnActor->pos.z - _engine->_grid->camera.z + 2;
			}

			if (actor->staticFlags.bIsSpriteActor) {
				drawList[drawListPos].type = DrawListType::DrawActorSprites;
				drawList[drawListPos].actorIdx = modelActorPos;
				if (actor->staticFlags.bUsesClipping) {
					tmpVal = actor->lastPos.x - _engine->_grid->camera.x + actor->lastPos.z - _engine->_grid->camera.z;
				}
			} else {
				drawList[drawListPos].type = 0;
				drawList[drawListPos].actorIdx = modelActorPos;
			}

			drawList[drawListPos].posValue = tmpVal;

			drawListPos++;

			// if use shadows
			if (_engine->cfgfile.ShadowMode != 0 && !(actor->staticFlags.bDoesntCastShadow)) {
				if (actor->standOn != -1) {
					_engine->_actor->shadowCoord.x = actor->pos.x;
					_engine->_actor->shadowCoord.y = actor->pos.y - 1;
					_engine->_actor->shadowCoord.z = actor->pos.z;
				} else {
					_engine->_movements->getShadowPosition(actor->pos.x, actor->pos.y, actor->pos.z);
				}

				drawList[drawListPos].posValue = tmpVal - 1; // save the shadow entry in the drawList
				drawList[drawListPos].type = DrawListType::DrawShadows;
				drawList[drawListPos].actorIdx = 0;
				drawList[drawListPos].x = _engine->_actor->shadowCoord.x;
				drawList[drawListPos].y = _engine->_actor->shadowCoord.y;
				drawList[drawListPos].z = _engine->_actor->shadowCoord.z;
				drawList[drawListPos].offset = 2;
				drawListPos++;
			}
			if (inSceneryView && modelActorPos == _engine->_scene->currentlyFollowedActor) {
				_sceneryViewX = _engine->_renderer->projPos.x;
				_sceneryViewY = _engine->_renderer->projPos.y;
			}
		}
	}
	return drawListPos;
}

int32 Redraw::fillExtraDrawingList(int32 drawListPos) {
	for (int32 i = 0; i < EXTRA_MAX_ENTRIES; i++) {
		ExtraListStruct *extra = &_engine->_extra->extraList[i];
		if (extra->info0 == -1) {
			continue;
		}
		if (extra->type & ExtraType::TIME_IN) {
			if (_engine->lbaTime - extra->spawnTime > 35) {
				extra->spawnTime = _engine->lbaTime;
				extra->type &= ~ExtraType::TIME_IN;
				_engine->_sound->playSample(Samples::ItemPopup, 1, extra->pos);
			}
			continue;
		}
		if ((extra->type & ExtraType::TIME_OUT) || (extra->type & ExtraType::FLASH) || (extra->payload.lifeTime + extra->spawnTime - 150 < _engine->lbaTime) || (!((_engine->lbaTime + extra->spawnTime) & 8))) {
			_engine->_renderer->projectPositionOnScreen(extra->pos - _engine->_grid->camera);

			if (_engine->_renderer->projPos.x > -50 && _engine->_renderer->projPos.x < _engine->width() + 40 && _engine->_renderer->projPos.y > -30 && _engine->_renderer->projPos.y < _engine->height() + 100) {
				const int16 tmpVal = extra->pos.x - _engine->_grid->camera.x + extra->pos.z - _engine->_grid->camera.z;
				drawList[drawListPos].posValue = tmpVal;
				drawList[drawListPos].actorIdx = i;
				drawList[drawListPos].type = DrawListType::DrawExtras;
				drawListPos++;

				if (_engine->cfgfile.ShadowMode == 2 && !(extra->info0 & EXTRA_SPECIAL_MASK)) {
					_engine->_movements->getShadowPosition(extra->pos.x, extra->pos.y, extra->pos.z);

					drawList[drawListPos].posValue = tmpVal - 1;
					drawList[drawListPos].actorIdx = 0;
					drawList[drawListPos].type = DrawListType::DrawShadows;
					drawList[drawListPos].x = _engine->_actor->shadowCoord.x;
					drawList[drawListPos].y = _engine->_actor->shadowCoord.y;
					drawList[drawListPos].z = _engine->_actor->shadowCoord.z;
					drawList[drawListPos].offset = 0;
					drawListPos++;
				}
			}
		}
	}
	return drawListPos;
}

void Redraw::processDrawListShadows(const DrawListStruct &drawCmd) {
	// get actor position on screen
	_engine->_renderer->projectPositionOnScreen(drawCmd.x - _engine->_grid->camera.x, drawCmd.y - _engine->_grid->camera.y, drawCmd.z - _engine->_grid->camera.z);

	int32 spriteWidth, spriteHeight;
	_engine->_grid->getSpriteSize(drawCmd.offset, &spriteWidth, &spriteHeight, _engine->_resources->spriteShadowPtr);

	// calculate sprite size and position on screen
	renderRect.left = _engine->_renderer->projPos.x - (spriteWidth / 2);
	renderRect.top = _engine->_renderer->projPos.y - (spriteHeight / 2);
	renderRect.right = _engine->_renderer->projPos.x + (spriteWidth / 2);
	renderRect.bottom = _engine->_renderer->projPos.y + (spriteHeight / 2);

	_engine->_interface->setClip(renderRect);

	_engine->_grid->drawSprite(drawCmd.offset, renderRect.left, renderRect.top, _engine->_resources->spriteShadowPtr);

	const int32 tmpX = (drawCmd.x + BRICK_HEIGHT) / BRICK_SIZE;
	const int32 tmpY = drawCmd.y / BRICK_HEIGHT;
	const int32 tmpZ = (drawCmd.z + BRICK_HEIGHT) / BRICK_SIZE;

	_engine->_grid->drawOverModelActor(tmpX, tmpY, tmpZ);

	addRedrawArea(_engine->_interface->textWindow);

	// show clipping area
	//drawBox(_engine->_renderer->renderRect.left, _engine->_renderer->renderRect.top, _engine->_renderer->renderRect.right, _engine->_renderer->renderRect.bottom);
}

void Redraw::processDrawListActors(const DrawListStruct &drawCmd, bool bgRedraw) {
	const int32 actorIdx = drawCmd.actorIdx;
	ActorStruct *actor = _engine->_scene->getActor(actorIdx);
	if (actor->previousAnimIdx >= 0) {
		const AnimData &animData = _engine->_resources->animData[actor->previousAnimIdx];
		_engine->_animations->setModelAnimation(actor->animPosition, animData, _engine->_resources->bodyData[actor->entity], &actor->animTimerData);
	}

	const int32 x = actor->pos.x - _engine->_grid->camera.x;
	const int32 y = actor->pos.y - _engine->_grid->camera.y;
	const int32 z = actor->pos.z - _engine->_grid->camera.z;
	if (!_engine->_renderer->renderIsoModel(x, y, z, ANGLE_0, actor->angle, ANGLE_0, _engine->_resources->bodyData[actor->entity])) {
		return;
	}

	if (renderRect.left < 0) {
		renderRect.left = 0;
	}

	if (renderRect.top < 0) {
		renderRect.top = 0;
	}

	if (renderRect.right >= _engine->width()) {
		renderRect.right = (_engine->width() - 1);
	}

	if (renderRect.bottom >= _engine->height()) {
		renderRect.bottom = (_engine->height() - 1);
	}

	_engine->_interface->setClip(renderRect);

	if (_engine->_interface->textWindow.left <= _engine->_interface->textWindow.right && _engine->_interface->textWindow.top <= _engine->_interface->textWindow.bottom) {
		actor->dynamicFlags.bIsVisible = 1;

		const int32 tempX = (actor->pos.x + BRICK_HEIGHT) / BRICK_SIZE;
		int32 tempY = actor->pos.y / BRICK_HEIGHT;
		const int32 tempZ = (actor->pos.z + BRICK_HEIGHT) / BRICK_SIZE;
		if (actor->brickShape() != ShapeType::kNone) {
			tempY++;
		}

		_engine->_grid->drawOverModelActor(tempX, tempY, tempZ);

		if (_engine->_actor->cropBottomScreen) {
			renderRect.bottom = _engine->_interface->textWindow.bottom = _engine->_actor->cropBottomScreen + 10;
		}

		addRedrawArea(_engine->_interface->textWindow);

		if (actor->staticFlags.bIsBackgrounded && bgRedraw) {
			_engine->_interface->blitBox(_engine->_interface->textWindow, _engine->frontVideoBuffer, _engine->workVideoBuffer);
		}

		_engine->_debugScene->drawClip(renderRect);
	}
}

void Redraw::processDrawListActorSprites(const DrawListStruct &drawCmd, bool bgRedraw) {
	int32 actorIdx = drawCmd.actorIdx;
	ActorStruct *actor = _engine->_scene->getActor(actorIdx);
	const SpriteData &spriteData = _engine->_resources->spriteData[actor->entity];
	// TODO: using the raw pointer and not the SpriteData surface here is a workaround for issue https://bugs.scummvm.org/ticket/12024
	const uint8 *spritePtr = _engine->_resources->spriteTable[actor->entity];

	// get actor position on screen
	_engine->_renderer->projectPositionOnScreen(actor->pos - _engine->_grid->camera);

	const int32 spriteWidth = spriteData.surface().w;
	const int32 spriteHeight = spriteData.surface().h;

	// calculate sprite position on screen
	const SpriteDim *dim = _engine->_resources->spriteBoundingBox.dim(actor->entity);
	renderRect.left = _engine->_renderer->projPos.x + dim->x;
	renderRect.top = _engine->_renderer->projPos.y + dim->y;
	renderRect.right = renderRect.left + spriteWidth;
	renderRect.bottom = renderRect.top + spriteHeight;

	if (actor->staticFlags.bUsesClipping) {
		const Common::Rect rect(_engine->_renderer->projPosScreen.x + actor->cropLeft, _engine->_renderer->projPosScreen.y + actor->cropTop, _engine->_renderer->projPosScreen.x + actor->cropRight, _engine->_renderer->projPosScreen.y + actor->cropBottom);
		_engine->_interface->setClip(rect);
	} else {
		_engine->_interface->setClip(renderRect);
	}

	if (_engine->_interface->textWindow.left <= _engine->_interface->textWindow.right && _engine->_interface->textWindow.top <= _engine->_interface->textWindow.bottom) {
		_engine->_grid->drawSprite(0, renderRect.left, renderRect.top, spritePtr);

		actor->dynamicFlags.bIsVisible = 1;

		if (actor->staticFlags.bUsesClipping) {
			const int32 tmpX = (actor->lastPos.x + BRICK_HEIGHT) / BRICK_SIZE;
			const int32 tmpY = actor->lastPos.y / BRICK_HEIGHT;
			const int32 tmpZ = (actor->lastPos.z + BRICK_HEIGHT) / BRICK_SIZE;
			_engine->_grid->drawOverSpriteActor(tmpX, tmpY, tmpZ);
		} else {
			const int32 tmpX = (actor->pos.x + actor->boudingBox.maxs.x + BRICK_HEIGHT) / BRICK_SIZE;
			int32 tmpY = actor->pos.y / BRICK_HEIGHT;
			const int32 tmpZ = (actor->pos.z + actor->boudingBox.maxs.z + BRICK_HEIGHT) / BRICK_SIZE;
			if (actor->brickShape() != ShapeType::kNone) {
				tmpY++;
			}

			_engine->_grid->drawOverSpriteActor(tmpX, tmpY, tmpZ);
		}

		addRedrawArea(_engine->_interface->textWindow);

		if (actor->staticFlags.bIsBackgrounded && bgRedraw) {
			_engine->_interface->blitBox(_engine->_interface->textWindow, _engine->frontVideoBuffer, _engine->workVideoBuffer);
		}

		// show clipping area
		//_engine->_debugScene->drawClip(renderRect);
	}
}

void Redraw::processDrawListExtras(const DrawListStruct &drawCmd) {
	int32 extraIdx = drawCmd.actorIdx;
	ExtraListStruct *extra = &_engine->_extra->extraList[extraIdx];

	_engine->_renderer->projectPositionOnScreen(extra->pos - _engine->_grid->camera);

	if (extra->info0 & EXTRA_SPECIAL_MASK) {
		_engine->_extra->drawExtraSpecial(extraIdx, _engine->_renderer->projPos.x, _engine->_renderer->projPos.y);
	} else {
		const SpriteData &spritePtr = _engine->_resources->spriteData[extra->info0];
		const int32 spriteWidth = spritePtr.surface().w;
		const int32 spriteHeight = spritePtr.surface().h;

		// calculate sprite position on screen
		const SpriteDim *dim = _engine->_resources->spriteBoundingBox.dim(extra->info0);
		renderRect.left = _engine->_renderer->projPos.x + dim->x;
		renderRect.top = _engine->_renderer->projPos.y + dim->y;
		renderRect.right = renderRect.left + spriteWidth;
		renderRect.bottom = renderRect.top + spriteHeight;

		_engine->_grid->drawSprite(renderRect.left, renderRect.top, spritePtr);
	}

	_engine->_interface->setClip(renderRect);

	if (_engine->_interface->textWindow.left <= _engine->_interface->textWindow.right && _engine->_interface->textWindow.top <= _engine->_interface->textWindow.bottom) {
		const int32 tmpX = (drawCmd.x + BRICK_HEIGHT) / BRICK_SIZE;
		const int32 tmpY = drawCmd.y / BRICK_HEIGHT;
		const int32 tmpZ = (drawCmd.z + BRICK_HEIGHT) / BRICK_SIZE;

		_engine->_grid->drawOverModelActor(tmpX, tmpY, tmpZ);
		addRedrawArea(_engine->_interface->textWindow);

		// show clipping area
		//drawBox(renderRect);
	}
}

void Redraw::processDrawList(int32 drawListPos, bool bgRedraw) {
	for (int32 pos = 0; pos < drawListPos; ++pos) {
		const DrawListStruct &drawCmd = drawList[pos];
		const uint32 flags = drawCmd.type;
		// Drawing actors
		if (flags < DrawListType::DrawShadows) {
			if (flags == 0) {
				processDrawListActors(drawCmd, bgRedraw);
			}
		}
		// Drawing shadows
		else if (flags == DrawListType::DrawShadows && !_engine->_actor->cropBottomScreen) {
			processDrawListShadows(drawCmd);
		}
		// Drawing unknown
		else if (flags < DrawListType::DrawActorSprites) {
			// TODO reverse this part of the code
			warning("Not yet reversed part of the rendering code");
		}
		// Drawing sprite actors, doors and entities
		else if (flags == DrawListType::DrawActorSprites) {
			processDrawListActorSprites(drawCmd, bgRedraw);
		}
		// Drawing extras
		else if (flags == DrawListType::DrawExtras) {
			processDrawListExtras(drawCmd);
		}

		_engine->_interface->resetClip();
	}
}

void Redraw::renderOverlays() {
	for (int32 i = 0; i < OVERLAY_MAX_ENTRIES; i++) {
		OverlayListStruct *overlay = &overlayList[i];
		if (overlay->info0 != -1) {
			// process position overlay
			switch (overlay->posType) {
			case OverlayPosType::koNormal:
				if (_engine->lbaTime >= overlay->lifeTime) {
					overlay->info0 = -1;
					continue;
				}
				break;
			case OverlayPosType::koFollowActor: {
				ActorStruct *actor2 = _engine->_scene->getActor(overlay->info1);

				_engine->_renderer->projectPositionOnScreen(actor2->pos.x - _engine->_grid->camera.x, actor2->pos.y + actor2->boudingBox.maxs.y - _engine->_grid->camera.y, actor2->pos.z - _engine->_grid->camera.z);

				overlay->x = _engine->_renderer->projPos.x;
				overlay->y = _engine->_renderer->projPos.y;

				if (_engine->lbaTime >= overlay->lifeTime) {
					overlay->info0 = -1;
					continue;
				}
				break;
			}
			}

			// process overlay type
			switch (overlay->type) {
			case OverlayType::koSprite: {
				const SpriteData &spritePtr = _engine->_resources->spriteData[overlay->info0];
				const int32 spriteWidth = spritePtr.surface().w;
				const int32 spriteHeight = spritePtr.surface().h;

				const SpriteDim *dim = _engine->_resources->spriteBoundingBox.dim(overlay->info0);
				renderRect.left = dim->x + overlay->x;
				renderRect.top = dim->y + overlay->y;
				renderRect.right = renderRect.left + spriteWidth;
				renderRect.bottom = renderRect.top + spriteHeight;

				_engine->_grid->drawSprite(renderRect.left, renderRect.top, spritePtr);

				addRedrawArea(_engine->_interface->textWindow);
				break;
			}
			case OverlayType::koNumber: {
				char text[10];
				snprintf(text, sizeof(text), "%d", overlay->info0);

				const int32 textLength = _engine->_text->getTextSize(text);
				const int32 textHeight = 48;

				renderRect.left = overlay->x - (textLength / 2);
				renderRect.top = overlay->y - 24;
				renderRect.right = overlay->x + (textLength / 2);
				renderRect.bottom = overlay->y + textHeight;

				_engine->_interface->setClip(renderRect);

				_engine->_text->setFontColor(overlay->info1);

				_engine->_text->drawText(renderRect.left, renderRect.top, text);

				addRedrawArea(_engine->_interface->textWindow);
				break;
			}
			case OverlayType::koNumberRange: {
				const int32 range = _engine->_collision->getAverageValue(overlay->info1, overlay->info0, 100, overlay->lifeTime - _engine->lbaTime - 50);

				char text[10];
				sprintf(text, "%d", range);

				const int32 textLength = _engine->_text->getTextSize(text);
				const int32 textHeight = 48;

				renderRect.left = overlay->x - (textLength / 2);
				renderRect.top = overlay->y - 24;
				renderRect.right = overlay->x + (textLength / 2);
				renderRect.bottom = overlay->y + textHeight;

				_engine->_interface->setClip(renderRect);

				_engine->_text->setFontColor(COLOR_GOLD);

				_engine->_text->drawText(renderRect.left, renderRect.top, text);

				addRedrawArea(_engine->_interface->textWindow);
				break;
			}
			case OverlayType::koInventoryItem: {
				const int32 item = overlay->info0;
				const Common::Rect rect(10, 10, 69, 69);

				_engine->_interface->drawFilledRect(rect, COLOR_BLACK);
				_engine->_interface->setClip(rect);

				const BodyData &bodyPtr = _engine->_resources->inventoryTable[item];
				_overlayRotation += 1; // overlayRotation += 8;
				_engine->_renderer->renderInventoryItem(40, 40, bodyPtr, _overlayRotation, 16000);
				_engine->_menu->drawBox(rect);
				addRedrawArea(rect);
				_engine->_gameState->initEngineProjections();
				break;
			}
			case OverlayType::koText: {
				char text[256];
				_engine->_text->getMenuText((TextId)overlay->info0, text, sizeof(text));

				const int32 textLength = _engine->_text->getTextSize(text);
				const int32 textHeight = 48;

				renderRect.left = overlay->x - (textLength / 2);
				renderRect.top = overlay->y - 24;
				renderRect.right = overlay->x + (textLength / 2);
				renderRect.bottom = overlay->y + textHeight;

				if (renderRect.left < 0) {
					renderRect.left = 0;
				}

				if (renderRect.top < 0) {
					renderRect.top = 0;
				}

				if (renderRect.right > (_engine->width() - 1)) {
					renderRect.right = (_engine->width() - 1);
				}

				if (renderRect.bottom > (_engine->height() - 1)) {
					renderRect.bottom = (_engine->height() - 1);
				}

				_engine->_interface->setClip(renderRect);

				_engine->_text->setFontColor(_engine->_scene->getActor(overlay->info1)->talkColor);

				_engine->_text->drawText(renderRect.left, renderRect.top, text);

				addRedrawArea(_engine->_interface->textWindow);
				break;
			}
			}
		}
	}
}

void Redraw::redrawEngineActions(bool bgRedraw) {
	int32 tmp_projPosX = _engine->_renderer->projPosScreen.x;
	int32 tmp_projPosY = _engine->_renderer->projPosScreen.y;

	_engine->_interface->resetClip();

	if (bgRedraw) {
		_engine->freezeTime();
		if (_engine->_scene->needChangeScene != -1 && _engine->_scene->needChangeScene != -2) {
			_engine->_screens->fadeOut(_engine->_screens->paletteRGBA);
		}
		_engine->_screens->clearScreen();
		_engine->_grid->redrawGrid();
		updateOverlayTypePosition(tmp_projPosX, tmp_projPosY, _engine->_renderer->projPosScreen.x, _engine->_renderer->projPosScreen.y);
		_engine->_screens->copyScreen(_engine->frontVideoBuffer, _engine->workVideoBuffer);

		if (_engine->_scene->needChangeScene != -1 && _engine->_scene->needChangeScene != -2) {
			_engine->_screens->fadeIn(_engine->_screens->paletteRGBA);
			_engine->setPalette(_engine->_screens->paletteRGBA);
			_engine->flip();
		}
	} else {
		blitBackgroundAreas();
	}

	int32 drawListPos = fillActorDrawingList(bgRedraw);
	drawListPos = fillExtraDrawingList(drawListPos);
	sortDrawingList(drawList, drawListPos);

	currNumOfRedrawBox = 0;
	processDrawList(drawListPos, bgRedraw);

	if (_engine->cfgfile.Debug) {
		_engine->_debugScene->renderDebugView();
	}

	renderOverlays();

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

	if (inSceneryView) {
		zoomScreenScale();
	}
}

void Redraw::drawBubble(int32 actorIdx) {
	ActorStruct *actor = _engine->_scene->getActor(actorIdx);

	// get actor position on screen
	_engine->_renderer->projectPositionOnScreen(actor->pos.x - _engine->_grid->camera.x, actor->pos.y + actor->boudingBox.maxs.y - _engine->_grid->camera.y, actor->pos.z - _engine->_grid->camera.z);

	if (actorIdx != bubbleActor) {
		bubbleSpriteIndex = bubbleSpriteIndex ^ 1;
		bubbleActor = actorIdx;
	}

	const SpriteData &spritePtr = _engine->_resources->spriteData[bubbleSpriteIndex];
	const int32 spriteWidth = spritePtr.surface().w;
	const int32 spriteHeight = spritePtr.surface().h;

	// calculate sprite position on screen
	if (bubbleSpriteIndex == SPRITEHQR_DIAG_BUBBLE_RIGHT) {
		renderRect.left = _engine->_renderer->projPos.x + 10;
	} else {
		renderRect.left = _engine->_renderer->projPos.x - 10 - spriteWidth;
	}
	renderRect.top = _engine->_renderer->projPos.y - 20;
	renderRect.right = spriteWidth + renderRect.left - 1;
	renderRect.bottom = spriteHeight + renderRect.top - 1;

	_engine->_interface->setClip(renderRect);

	if (_engine->_grid->drawSprite(renderRect.left, renderRect.top, spritePtr)) {
		_engine->copyBlockPhys(renderRect, true);
	}

	_engine->_interface->resetClip();
}

void Redraw::zoomScreenScale() {
	Graphics::ManagedSurface zoomWorkVideoBuffer(_engine->workVideoBuffer);
	const int maxW = zoomWorkVideoBuffer.w;
	const int maxH = zoomWorkVideoBuffer.h;
	const int left = CLIP<int>(_sceneryViewX - maxW / 4, 0, maxW / 2);
	const int top = CLIP<int>(_sceneryViewY - maxH / 4, 0, maxH / 2);
	const Common::Rect srcRect(left, top, left + maxW / 2, top + maxH / 2);
	const Common::Rect& destRect = zoomWorkVideoBuffer.getBounds();
	zoomWorkVideoBuffer.transBlitFrom(_engine->frontVideoBuffer, srcRect, destRect);
	g_system->copyRectToScreen(zoomWorkVideoBuffer.getPixels(), zoomWorkVideoBuffer.pitch, 0, 0, zoomWorkVideoBuffer.w, zoomWorkVideoBuffer.h);
	g_system->updateScreen();
}

} // namespace TwinE
