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
#include "twine/scene/extra.h"
#include "twine/scene/grid.h"
#include "twine/scene/movements.h"
#include "twine/scene/scene.h"
#include "twine/shared.h"
#include "twine/text.h"

namespace TwinE {

Redraw::Redraw(TwinEEngine *engine) : _engine(engine), _bubbleSpriteIndex(SPRITEHQR_DIAG_BUBBLE_LEFT) {}

void Redraw::addRedrawCurrentArea(const Common::Rect &redrawArea) {
	const int32 area = (redrawArea.right - redrawArea.left) * (redrawArea.bottom - redrawArea.top);

	for (int32 i = 0; i < _numOfRedrawBox; ++i) {
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

	Common::Rect &rect = _currentRedrawList[_numOfRedrawBox];
	rect.left = redrawArea.left;
	rect.top = redrawArea.top;
	rect.right = redrawArea.right;
	rect.bottom = MIN<int32>((_engine->height() - 1), redrawArea.bottom);

	assert(rect.left <= rect.right);
	assert(rect.top <= rect.bottom);

	_numOfRedrawBox++;
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

	Common::Rect &rect = _nextRedrawList[_currNumOfRedrawBox];
	rect.left = left;
	rect.top = top;
	rect.right = right;
	rect.bottom = bottom;

	_currNumOfRedrawBox++;

	addRedrawCurrentArea(rect);
}

void Redraw::moveNextAreas() {
	_numOfRedrawBox = 0;

	for (int32 i = 0; i < _currNumOfRedrawBox; i++) {
		addRedrawCurrentArea(_nextRedrawList[i]);
	}
}

void Redraw::flipRedrawAreas() {
	for (int32 i = 0; i < _numOfRedrawBox; i++) { // redraw areas on screen
		_engine->copyBlockPhys(_currentRedrawList[i]);
	}

	moveNextAreas();
}

void Redraw::blitBackgroundAreas() {
	for (int32 i = 0; i < _numOfRedrawBox; i++) {
		_engine->blitWorkToFront(_currentRedrawList[i]);
	}
}

void Redraw::sortDrawingList(DrawListStruct *list, int32 listSize) const { // SmallSort
	DrawListStruct* pNext;
	DrawListStruct* pSmallest;
	int32 n;

	for (listSize--; listSize > 0; listSize--) {
		pSmallest = list;
		pNext = list + 1;

		for (n = listSize; n > 0; n--) {
			if (pNext->z < pSmallest->z) {
				pSmallest = pNext;
			}
			pNext++;
		}

		if (pSmallest != list) {
			DrawListStruct tmp = *pSmallest;
			*pSmallest = *list;
			*list = tmp;
		}

		list++;
	}
}

void Redraw::posObjIncrust(OverlayListStruct *ptrdisp, int32 num) {
	// in case we have several 3D objects rotating at the same time!
	int32 x = 10;
	OverlayType type = ptrdisp->type;

	if (type == OverlayType::koInventory || type == OverlayType::koInventoryItem) {
		for (int32 n = 0; n < ARRAYSIZE(overlayList); n++) {
			OverlayListStruct *overlay = &overlayList[n];
			if (n != num && overlay->num != -1) {
				if (overlay->type == OverlayType::koInventory || overlay->type == OverlayType::koInventoryItem) {
					x += 70;
				}
			}
		}

		ptrdisp->y = 10;
		ptrdisp->x = (int16)x;
	}
}

int32 Redraw::addOverlay(OverlayType type, int16 info0, int16 x, int16 y, int16 info1, OverlayPosType posType, int16 lifeTime) { // InitIncrustDisp
	for (int32 i = 0; i < ARRAYSIZE(overlayList); i++) {
		OverlayListStruct *overlay = &overlayList[i];
		if (_engine->isLBA1()) {
			if (overlay->num == -1) {
				overlay->type = type;
				overlay->num = info0;
				overlay->x = x;
				overlay->y = y;
				overlay->info = info1;
				overlay->move = posType;
				overlay->timerEnd = _engine->timerRef + _engine->toSeconds(lifeTime);
				return i;
			}
		} else {
			if (overlay->num == -1 || (overlay->num == info0 && overlay->type == type)) {
				if (overlay->num == -1 || overlay->type != type) {
					overlay->x = x;
					overlay->y = y;
				}
				if ((OverlayType)((uint8)type) == OverlayType::koNumberRange) {
					// ATTENTION: Big Trickery: counters are always displayed
					// at y=20, this allows using the Y to store the
					// current value of the counter (see FlagAnimWhoSpeak)
					overlay->y = info0;
				}
				overlay->type = type;
				overlay->num = info0;
				overlay->info = info1;
				overlay->move = posType;
				overlay->timerEnd = _engine->timerRef + _engine->toSeconds(lifeTime);
				posObjIncrust(overlay, i);
				return i;
			}
		}
	}
	return -1;
}

void Redraw::updateOverlayTypePosition(int16 x1, int16 y1, int16 x2, int16 y2) {
	const int16 newX = x2 - x1;
	const int16 newY = y2 - y1;

	for (int32 i = 0; i < ARRAYSIZE(overlayList); i++) {
		OverlayListStruct *overlay = &overlayList[i];
		if (overlay->move == OverlayPosType::koFollowActor) {
			overlay->x = newX;
			overlay->y = newY;
		}
	}
}

int32 Redraw::fillActorDrawingList(DrawListStruct *drawList, bool flagflip) {
	int32 drawListPos = 0;
	for (int32 n = 0; n < _engine->_scene->_nbObjets; n++) {
		ActorStruct *actor = _engine->_scene->getActor(n);
		actor->_workFlags.bWasDrawn = 0; // reset visible state
		actor->_workFlags.bIsTargetable = 0;

		if (_engine->_grid->_useCellingGrid != -1 && actor->_posObj.y > _engine->_scene->_sceneZones[_engine->_grid->_cellingGridIdx].maxs.y) {
			continue;
		}
		// no redraw required
		if (actor->_staticFlags.bIsBackgrounded && !flagflip) {
			// get actor position on screen
			const IVec3 &projPos = _engine->_renderer->projectPoint(actor->posObj() - _engine->_grid->_worldCube);
			// check if actor is visible on screen, otherwise don't display it
			if (projPos.x > VIEW_X0 && projPos.x < VIEW_X1(_engine) && projPos.y > VIEW_Y0 && projPos.y < VIEW_Y1(_engine)) {
				actor->_workFlags.bWasDrawn = 1;
			}
			continue;
		}
		// if the actor isn't set as hidden
		if (actor->_body == -1 || actor->_staticFlags.bIsInvisible) {
			continue;
		}
		// get actor position on screen
		const IVec3 &projPos = _engine->_renderer->projectPoint(actor->posObj() - _engine->_grid->_worldCube);

		if ((actor->_staticFlags.bSpriteClip && projPos.x > -112 && projPos.x < _engine->width() + 112 && projPos.y > VIEW_X0 && projPos.y < _engine->height() + 171) ||
		    ((!actor->_staticFlags.bSpriteClip) && projPos.x > VIEW_X0 && projPos.x < VIEW_X1(_engine) && projPos.y > VIEW_Y0 && projPos.y < VIEW_Y1(_engine))) {

			int32 ztri = actor->_posObj.x - _engine->_grid->_worldCube.x + actor->_posObj.z - _engine->_grid->_worldCube.z;

			// if actor is above another actor
			if (actor->_carryBy != -1) {
				const ActorStruct *standOnActor = _engine->_scene->getActor(actor->_carryBy);
				ztri = standOnActor->_posObj.x - _engine->_grid->_worldCube.x + standOnActor->_posObj.z - _engine->_grid->_worldCube.z + 2;
			}

			if (actor->_staticFlags.bSprite3D) {
				drawList[drawListPos].type = DrawListType::DrawActorSprites;
				drawList[drawListPos].numObj = n;
				if (actor->_staticFlags.bSpriteClip) {
					ztri = actor->_animStep.x - _engine->_grid->_worldCube.x + actor->_animStep.z - _engine->_grid->_worldCube.z;
				}
			} else {
				drawList[drawListPos].type = DrawListType::DrawObject3D;
				drawList[drawListPos].numObj = n;
			}

			drawList[drawListPos].z = ztri;

			drawListPos++;

			// if use shadows
			if (_engine->_cfgfile.ShadowMode != 0 && !(actor->_staticFlags.bNoShadow)) {
				if (actor->_carryBy != -1) {
					drawList[drawListPos].xw = actor->_posObj.x;
					drawList[drawListPos].yw = actor->_posObj.y - 1;
					drawList[drawListPos].zw = actor->_posObj.z;
				} else {
					const IVec3 shadowCoord = _engine->_movements->getShadow(actor->posObj());
					drawList[drawListPos].xw = shadowCoord.x;
					drawList[drawListPos].yw = shadowCoord.y;
					drawList[drawListPos].zw = shadowCoord.z;
				}

				drawList[drawListPos].z = ztri - 1; // save the shadow entry in the _drawList
				drawList[drawListPos].type = DrawListType::DrawShadows;
				drawList[drawListPos].numObj = 0;
				drawList[drawListPos].num = 1;
				drawListPos++;
			}
			if (_flagMCGA && n == _engine->_scene->_currentlyFollowedActor) {
				_sceneryViewX = projPos.x;
				_sceneryViewY = projPos.y;
			}
		}
	}
	return drawListPos;
}

int32 Redraw::fillExtraDrawingList(DrawListStruct *drawList, int32 drawListPos) { // part of AffScene
	for (int32 i = 0; i < EXTRA_MAX_ENTRIES; i++) {
		ExtraListStruct *extra = &_engine->_extra->_extraList[i];
		if (extra->sprite == -1) {
			continue;
		}
		if (extra->type & ExtraType::TIME_IN) {
			if (_engine->timerRef - extra->spawnTime > 35) {
				extra->spawnTime = _engine->timerRef;
				extra->type &= ~ExtraType::TIME_IN;
				_engine->_sound->playSample(Samples::ItemPopup, 1, extra->pos);
			}
			continue;
		}
		if ((extra->type & ExtraType::TIME_OUT) && (extra->type & ExtraType::FLASH)) {
			if (_engine->timerRef >= extra->spawnTime + extra->payload.lifeTime - _engine->toSeconds(3)) {
				if ((_engine->timerRef + extra->spawnTime) & 8) {
					continue;
				}
			}
		}
		const IVec3 &projPos = _engine->_renderer->projectPoint(extra->pos - _engine->_grid->_worldCube);

		if (projPos.x > VIEW_X0 && projPos.x < VIEW_X1(_engine) && projPos.y > VIEW_Y0 && projPos.y < VIEW_Y1(_engine)) {
			const int16 zVal = extra->pos.x - _engine->_grid->_worldCube.x + extra->pos.z - _engine->_grid->_worldCube.z;
			drawList[drawListPos].z = zVal;
			drawList[drawListPos].numObj = i;
			drawList[drawListPos].type = DrawListType::DrawExtras;
			drawListPos++;

			if (_engine->_cfgfile.ShadowMode == 2 && !(extra->sprite & EXTRA_SPECIAL_MASK)) {
				const IVec3 &shadowCoord = _engine->_movements->getShadow(extra->pos);

				drawList[drawListPos].z = zVal - 1;
				drawList[drawListPos].numObj = 0;
				drawList[drawListPos].type = DrawListType::DrawShadows;
				drawList[drawListPos].xw = shadowCoord.x;
				drawList[drawListPos].yw = shadowCoord.y;
				drawList[drawListPos].zw = shadowCoord.z;
				drawList[drawListPos].num = 0;
				drawListPos++;
			}
		}
	}
	return drawListPos;
}

void Redraw::processDrawListShadows(const DrawListStruct &drawCmd) {
	// get actor position on screen
	const IVec3 &projPos = _engine->_renderer->projectPoint(drawCmd.xw - _engine->_grid->_worldCube.x, drawCmd.yw - _engine->_grid->_worldCube.y, drawCmd.zw - _engine->_grid->_worldCube.z);

	int32 spriteWidth = _engine->_resources->_spriteShadowPtr.surface(drawCmd.num).w;
	int32 spriteHeight = _engine->_resources->_spriteShadowPtr.surface(drawCmd.num).h;

	// calculate sprite size and position on screen
	Common::Rect renderRect;
	renderRect.left = projPos.x - (spriteWidth / 2);
	renderRect.top = projPos.y - (spriteHeight / 2);
	renderRect.right = projPos.x + (spriteWidth / 2);
	renderRect.bottom = projPos.y + (spriteHeight / 2);

	if (_engine->_interface->setClip(renderRect)) {
		_engine->_grid->drawSprite(renderRect.left, renderRect.top, _engine->_resources->_spriteShadowPtr, drawCmd.num);

		const int32 tmpX = (drawCmd.xw + SIZE_BRICK_Y) / SIZE_BRICK_XZ;
		const int32 tmpY = drawCmd.yw / SIZE_BRICK_Y;
		const int32 tmpZ = (drawCmd.zw + SIZE_BRICK_Y) / SIZE_BRICK_XZ;

		_engine->_grid->drawOverBrick(tmpX, tmpY, tmpZ);

		addRedrawArea(_engine->_interface->_clip);

		_engine->_debugScene->drawClip(renderRect);
	}
	_engine->_interface->unsetClip();
}

void Redraw::processDrawListActors(const DrawListStruct &drawCmd, bool bgRedraw) {
	const int32 actorIdx = drawCmd.numObj;
	ActorStruct *actor = _engine->_scene->getActor(actorIdx);
	if (actor->_anim >= 0) {
		const AnimData &animData = _engine->_resources->_animData[actor->_anim];
		_engine->_animations->doSetInterAnimObjet(actor->_frame, animData, _engine->_resources->_bodyData[actor->_body], &actor->_animTimerData);
	}

	const IVec3 &delta = actor->posObj() - _engine->_grid->_worldCube;
	Common::Rect renderRect;

	if (actorIdx == OWN_ACTOR_SCENE_INDEX) {
		if (_engine->_actor->_cropBottomScreen) {
			_engine->_interface->_clip.bottom = _engine->_actor->_cropBottomScreen;
		}
	}

	if (!_engine->_renderer->affObjetIso(delta.x, delta.y, delta.z, LBAAngles::ANGLE_0, actor->_beta, LBAAngles::ANGLE_0, _engine->_resources->_bodyData[actor->_body], renderRect)) {
		return;
	}

	if (_engine->_interface->setClip(renderRect)) {
		actor->_workFlags.bWasDrawn = 1;

		const int32 tempX = (actor->_posObj.x + DEMI_BRICK_XZ) / SIZE_BRICK_XZ;
		int32 tempY = actor->_posObj.y / SIZE_BRICK_Y;
		const int32 tempZ = (actor->_posObj.z + DEMI_BRICK_XZ) / SIZE_BRICK_XZ;
		if (actor->brickShape() != ShapeType::kNone) {
			tempY++;
		}

		_engine->_grid->drawOverBrick(tempX, tempY, tempZ);

		addRedrawArea(_engine->_interface->_clip);

		if (actor->_staticFlags.bIsBackgrounded && bgRedraw) {
			_engine->blitFrontToWork(_engine->_interface->_clip);
		}

		_engine->_debugScene->drawClip(_engine->_interface->_clip);
	}
	_engine->_interface->unsetClip();
}

void Redraw::processDrawListActorSprites(const DrawListStruct &drawCmd, bool bgRedraw) {
	int32 actorIdx = drawCmd.numObj;
	ActorStruct *actor = _engine->_scene->getActor(actorIdx);
	const SpriteData &spriteData = _engine->_resources->_spriteData[actor->_body];
	// TODO: using the raw pointer and not the SpriteData surface here is a workaround for issue https://bugs.scummvm.org/ticket/12024
	const uint8 *spritePtr = _engine->_resources->_spriteTable[actor->_body];

	// get actor position on screen
	const IVec3 &projPos = _engine->_renderer->projectPoint(actor->posObj() - _engine->_grid->_worldCube);

	const int32 dx = spriteData.surface().w;
	const int32 dy = spriteData.surface().h;

	// calculate sprite position on screen
	const SpriteDim *dim = _engine->_resources->_spriteBoundingBox.dim(actor->_body);
	Common::Rect renderRect;
	renderRect.left = projPos.x + dim->x;
	renderRect.top = projPos.y + dim->y;
	renderRect.right = renderRect.left + dx;
	renderRect.bottom = renderRect.top + dy;

	bool validClip;
	if (actor->_staticFlags.bSpriteClip) {
		const Common::Rect rect(_projPosScreen.x + actor->_cropLeft, _projPosScreen.y + actor->_cropTop, _projPosScreen.x + actor->_cropRight, _projPosScreen.y + actor->_cropBottom);
		validClip = _engine->_interface->setClip(rect);
	} else {
		validClip = _engine->_interface->setClip(renderRect);
	}

	if (validClip) {
		_engine->_grid->drawSprite(0, renderRect.left, renderRect.top, spritePtr);

		actor->_workFlags.bWasDrawn = 1;

		if (actor->_staticFlags.bSpriteClip) {
			const int32 xm = (actor->_animStep.x + DEMI_BRICK_XZ) / SIZE_BRICK_XZ;
			const int32 ym = actor->_animStep.y / SIZE_BRICK_Y;
			const int32 zm = (actor->_animStep.z + DEMI_BRICK_XZ) / SIZE_BRICK_XZ;
			_engine->_grid->drawOverBrick3(xm, ym, zm);
		} else {
			const int32 xm = (actor->_posObj.x + actor->_boundingBox.maxs.x + DEMI_BRICK_XZ) / SIZE_BRICK_XZ;
			int32 ym = actor->_posObj.y / SIZE_BRICK_Y;
			const int32 zm = (actor->_posObj.z + actor->_boundingBox.maxs.z + DEMI_BRICK_XZ) / SIZE_BRICK_XZ;
			if (actor->brickShape() != ShapeType::kNone) {
				ym++;
			}

			_engine->_grid->drawOverBrick3(xm, ym, zm);
		}

		addRedrawArea(_engine->_interface->_clip);

		if (actor->_staticFlags.bIsBackgrounded && bgRedraw) {
			_engine->blitFrontToWork(_engine->_interface->_clip);
		}

		_engine->_debugScene->drawClip(renderRect);
		_engine->_interface->unsetClip();
	}
}

void Redraw::processDrawListExtras(const DrawListStruct &drawCmd) {
	int32 extraIdx = drawCmd.numObj;
	ExtraListStruct *extra = &_engine->_extra->_extraList[extraIdx];

	const IVec3 &projPos = _engine->_renderer->projectPoint(extra->pos - _engine->_grid->_worldCube);

	Common::Rect renderRect;
	if (extra->sprite & EXTRA_SPECIAL_MASK) {
		_engine->_extra->affSpecial(extraIdx, projPos.x, projPos.y, renderRect);
	} else {
		const SpriteData &spritePtr = _engine->_resources->_spriteData[extra->sprite];
		const int32 dx = spritePtr.surface().w;
		const int32 dy = spritePtr.surface().h;

		// calculate sprite position on screen
		const SpriteDim *dim = _engine->_resources->_spriteBoundingBox.dim(extra->sprite);
		renderRect.left = projPos.x + dim->x;
		renderRect.top = projPos.y + dim->y;
		renderRect.right = renderRect.left + dx;
		renderRect.bottom = renderRect.top + dy;

		_engine->_grid->drawSprite(renderRect.left, renderRect.top, spritePtr);
	}

	if (_engine->_interface->setClip(renderRect)) {
		const int32 xm = (extra->pos.x + DEMI_BRICK_XZ) / SIZE_BRICK_XZ;
		const int32 ym = extra->pos.y / SIZE_BRICK_Y;
		const int32 zm = (extra->pos.z + DEMI_BRICK_XZ) / SIZE_BRICK_XZ;

		_engine->_grid->drawOverBrick(xm, ym, zm);
		addRedrawArea(_engine->_interface->_clip);

		// show clipping area
		//drawRectBorders(renderRect);
		_engine->_interface->unsetClip();
	}
}

void Redraw::correctZLevels(DrawListStruct *listTri, int32 drawListPos) {
	ActorStruct *ptrobj = _engine->_scene->getActor(OWN_ACTOR_SCENE_INDEX);
	if (ptrobj->_staticFlags.bIsInvisible || ptrobj->_body == -1) {
		return;
	}

	IVec3 tmin = ptrobj->posObj() + ptrobj->_boundingBox.mins;
	IVec3 tmax = ptrobj->posObj() + ptrobj->_boundingBox.maxs;
	int32 twinsenpos = -1;
	int32 twinsenz = -1;
	for (int32 pos = 0; pos < drawListPos; ++pos) {
		DrawListStruct &drawCmd = listTri[pos];
		if (drawCmd.type == DrawListType::DrawObject3D && drawCmd.numObj == OWN_ACTOR_SCENE_INDEX) {
			twinsenpos = pos;
			twinsenz = drawCmd.z;
			break;
		}
	}

	if (twinsenpos == -1) {
		return;
	}

	for (int32 n = 0; n < drawListPos; ++n) {
		DrawListStruct &ptrtri = listTri[n];
		uint32 typeobj = ptrtri.type;
		int32 numobj = ptrtri.numObj;
		ptrobj = _engine->_scene->getActor(numobj);
		switch (typeobj) {
		default:
			break;
		case DrawListType::DrawActorSprites:
			if (ptrobj->_staticFlags.bSpriteClip) {
				IVec3 pmin = ptrobj->_animStep + ptrobj->_boundingBox.mins;
				IVec3 pmax = ptrobj->_animStep + ptrobj->_boundingBox.maxs;

				if (pmax.x > tmin.x && pmin.x < tmax.x) {
					if (pmax.z <= tmin.z) {
						// twinsen after
						if (twinsenz < ptrtri.z) {
							// correct the error
							listTri[twinsenpos].z = ptrtri.z;
							listTri[twinsenpos].numObj = ptrtri.numObj;
							listTri[twinsenpos].type = ptrtri.type;

							ptrtri.numObj = OWN_ACTOR_SCENE_INDEX;
							ptrtri.type = DrawListType::DrawObject3D;
							ptrtri.z = (int16)twinsenz;

							twinsenpos = n;
							numobj = -1;
							break;
						}
					}
					if (pmin.z >= tmax.z) {
						// twinsen before
						if (twinsenz > ptrtri.z) {
							// correct the error
							listTri[twinsenpos].z = ptrtri.z;
							listTri[twinsenpos].numObj = ptrtri.numObj;
							listTri[twinsenpos].type = ptrtri.type;

							ptrtri.numObj = OWN_ACTOR_SCENE_INDEX;
							ptrtri.type = DrawListType::DrawObject3D;
							ptrtri.z = (int16)twinsenz;

							twinsenpos = n;
							numobj = -1;
							break;
						}
					}
					break;
				}

				if (pmax.z > tmin.z && pmin.z < tmax.z) {
					if (pmax.x <= tmin.x) {
						// twinsen after
						if (twinsenz < ptrtri.z) {
							// correct the error
							listTri[twinsenpos].z = ptrtri.z;
							listTri[twinsenpos].numObj = ptrtri.numObj;
							listTri[twinsenpos].type = ptrtri.type;

							ptrtri.numObj = OWN_ACTOR_SCENE_INDEX;
							ptrtri.type = DrawListType::DrawObject3D;
							ptrtri.z = (int16)twinsenz;

							twinsenpos = n;
							numobj = -1;
							break;
						}
					} else {
						// twinsen before
						if (twinsenz > ptrtri.z) {
							// correct the error
							listTri[twinsenpos].z = ptrtri.z;
							listTri[twinsenpos].numObj = ptrtri.numObj;
							listTri[twinsenpos].type = ptrtri.type;

							ptrtri.numObj = OWN_ACTOR_SCENE_INDEX;
							ptrtri.type = DrawListType::DrawObject3D;
							ptrtri.z = (int16)twinsenz;

							twinsenpos = n;
							numobj = -1;
							break;
						}
					}
				}
			}
			break;
		}
		if (numobj == -1) {
			break;
		}
	}
}

void Redraw::processDrawList(DrawListStruct *drawList, int32 drawListPos, bool bgRedraw) {
	bool shadowtwinsen = false;
	for (int32 pos = 0; pos < drawListPos; ++pos) {
		const DrawListStruct &drawCmd = drawList[pos];
		const uint32 flags = drawCmd.type;
		if (flags == DrawListType::DrawObject3D) {
			// this is correcting a bug that came with correctZLevels() - original sources
			if (_engine->_cfgfile.ShadowMode != 0 && drawCmd.numObj == OWN_ACTOR_SCENE_INDEX && !shadowtwinsen) {
				for (int32 i = pos; i < drawListPos; i++) {
					if (drawList[i].numObj == OWN_ACTOR_SCENE_INDEX && drawList[i].type == DrawListType::DrawShadows) {
						shadowtwinsen = true;
						processDrawListShadows(drawList[i]);
						drawList[i].type = -1; // invalidate shadow entry
						break;
					}
				}
			}
			processDrawListActors(drawCmd, bgRedraw);
		} else if (flags == DrawListType::DrawShadows && !_engine->_actor->_cropBottomScreen) {
			if (drawCmd.numObj == OWN_ACTOR_SCENE_INDEX) {
				shadowtwinsen = true;
			}
			processDrawListShadows(drawCmd);
		} else if (flags == DrawListType::DrawActorSprites) {
			processDrawListActorSprites(drawCmd, bgRedraw);
		} else if (flags == DrawListType::DrawExtras) {
			processDrawListExtras(drawCmd);
		}

		_engine->_interface->unsetClip();
	}
}

void Redraw::renderOverlays() {
	for (int32 i = 0; i < OVERLAY_MAX_ENTRIES; i++) {
		OverlayListStruct *overlay = &overlayList[i];
		if (overlay->num == -1) {
			continue;
		}
		// process position overlay
		switch (overlay->move) {
		case OverlayPosType::koNormal: // wait number of seconds and die
			if (_engine->timerRef >= overlay->timerEnd) {
				overlay->num = -1;
				continue;
			}
			break;
		case OverlayPosType::koFollowActor: { // follow obj coordinates for number of seconds and die
			ActorStruct *actor2 = _engine->_scene->getActor(overlay->info);

			const IVec3 &projPos = _engine->_renderer->projectPoint(actor2->_posObj.x - _engine->_grid->_worldCube.x, actor2->_posObj.y + actor2->_boundingBox.maxs.y - _engine->_grid->_worldCube.y, actor2->_posObj.z - _engine->_grid->_worldCube.z);

			overlay->x = projPos.x;
			overlay->y = projPos.y;

			if (_engine->timerRef >= overlay->timerEnd) {
				overlay->num = -1;
				continue;
			}
			break;
		}
		}

		// process overlay type
		switch (overlay->type) {
		case OverlayType::koSprite: {
			const SpriteData &spritePtr = _engine->_resources->_spriteData[overlay->num];
			const int32 spriteWidth = spritePtr.surface().w;
			const int32 spriteHeight = spritePtr.surface().h;

			const SpriteDim *dim = _engine->_resources->_spriteBoundingBox.dim(overlay->num);
			Common::Rect renderRect;
			renderRect.left = dim->x + overlay->x;
			renderRect.top = dim->y + overlay->y;
			renderRect.right = renderRect.left + spriteWidth;
			renderRect.bottom = renderRect.top + spriteHeight;

			_engine->_grid->drawSprite(renderRect.left, renderRect.top, spritePtr);

			addRedrawArea(_engine->_interface->_clip);
			break;
		}
		case OverlayType::koNumber: {
			char text[10];
			snprintf(text, sizeof(text), "%d", overlay->num);

			const int32 textLength = _engine->_text->getTextSize(text);
			const int32 textHeight = 48;

			Common::Rect renderRect;
			renderRect.left = overlay->x - (textLength / 2);
			renderRect.top = overlay->y - 24;
			renderRect.right = overlay->x + (textLength / 2);
			renderRect.bottom = overlay->y + textHeight;

			_engine->_interface->setClip(renderRect);

			_engine->_text->setFontColor(overlay->info);

			_engine->_text->drawText(renderRect.left, renderRect.top, text);

			addRedrawArea(_engine->_interface->_clip);

			_engine->_interface->unsetClip();
			break;
		}
		case OverlayType::koNumberRange: {
			const int32 range = _engine->_collision->boundRuleThree(overlay->info, overlay->num, 100, overlay->timerEnd - _engine->timerRef - _engine->toSeconds(1));

			char text[10];
			Common::sprintf_s(text, "%d", range);

			const int32 textLength = _engine->_text->getTextSize(text);
			const int32 textHeight = 48;

			Common::Rect renderRect;
			renderRect.left = overlay->x - (textLength / 2);
			renderRect.top = overlay->y - 24;
			renderRect.right = overlay->x + (textLength / 2);
			renderRect.bottom = overlay->y + textHeight;

			_engine->_interface->setClip(renderRect);

			_engine->_text->setFontColor(COLOR_GOLD);

			_engine->_text->drawText(renderRect.left, renderRect.top, text);

			addRedrawArea(_engine->_interface->_clip);
			_engine->_interface->unsetClip();
			break;
		}
		case OverlayType::koInventoryItem: {
			const int32 item = overlay->num;
			const Common::Rect rect(10, 10, 79, 79);

			_engine->_interface->drawFilledRect(rect, COLOR_BLACK);
			_engine->_interface->setClip(rect);

			const BodyData &bodyPtr = _engine->_resources->_inventoryTable[item];
			_overlayRotation += 1; // overlayRotation += 8;
			_engine->_renderer->draw3dObject(40, 40, bodyPtr, _overlayRotation, 16000);
			_engine->_menu->drawRectBorders(rect);
			addRedrawArea(rect);
			_engine->_gameState->init3DGame();
			_engine->_interface->unsetClip();
			break;
		}
		case OverlayType::koText: {
			char text[256];
			_engine->_text->getMenuText((TextId)overlay->num, text, sizeof(text));

			const int32 textLength = _engine->_text->getTextSize(text);
			const int32 textHeight = 48;

			Common::Rect renderRect;
			renderRect.left = overlay->x - (textLength / 2);
			renderRect.top = overlay->y - (textHeight / 2);
			renderRect.right = overlay->x + (textLength / 2);
			renderRect.bottom = overlay->y + textHeight;

			renderRect.clip(_engine->rect());

			_engine->_interface->setClip(renderRect);

			_engine->_text->setFontColor(_engine->_scene->getActor(overlay->info)->_talkColor);

			_engine->_text->drawText(renderRect.left, renderRect.top, text);

			addRedrawArea(_engine->_interface->_clip);
			_engine->_interface->unsetClip();
			break;
		}
		case OverlayType::koSysText:
		case OverlayType::koFlash:
		case OverlayType::koRain:
		case OverlayType::koInventory:
			// TODO lba2
		case OverlayType::koMax:
			break;
		}
	}
}

void Redraw::setRenderText(const Common::String &text) {
	_text = text;
	if (_text.empty()) {
		_textDisappearTime = -1;
	} else {
		_textDisappearTime = _engine->timerRef + _engine->toSeconds(1);
	}
}

void Redraw::renderText() {
	if (_textDisappearTime <= _engine->timerRef) {
		return;
	}
	if (_text.empty()) {
		return;
	}
	_engine->_text->setFontColor(COLOR_WHITE);

	const int padding = 10;
	const int x = padding;
	const int height = _engine->_text->lineHeight;
	const int y = _engine->height() - height - padding;
	const int width = _engine->_text->getTextSize(_text.c_str());
	_engine->_text->drawText(x, y, _text.c_str(), true);
	_engine->copyBlockPhys(x, y, x + width, y + height);
	const Common::Rect redraw(x, y, x + width, y + height);
	addRedrawArea(redraw);
}

void Redraw::redrawEngineActions(bool bgRedraw) { // AffScene
	int32 tmp_projPosX = _projPosScreen.x;
	int32 tmp_projPosY = _projPosScreen.y;

	_engine->_interface->unsetClip();

	if (bgRedraw) {
		_engine->freezeTime(false);
		if (_engine->_scene->_needChangeScene != SCENE_CEILING_GRID_FADE_1 && _engine->_scene->_needChangeScene != SCENE_CEILING_GRID_FADE_2) {
			_engine->_screens->fadeOut(_engine->_screens->_paletteRGBA);
		}
		_engine->_screens->clearScreen();

		_engine->_grid->redrawGrid();
		const IVec3 &projPos = _engine->_renderer->projectPoint(-_engine->_grid->_worldCube);
		_projPosScreen.x = projPos.x;
		_projPosScreen.y = projPos.y;

		updateOverlayTypePosition(tmp_projPosX, tmp_projPosY, _projPosScreen.x, _projPosScreen.y);
		_engine->saveFrontBuffer();

		if (_engine->_scene->_needChangeScene != SCENE_CEILING_GRID_FADE_1 && _engine->_scene->_needChangeScene != SCENE_CEILING_GRID_FADE_2) {
			_engine->_screens->fadeIn(_engine->_screens->_paletteRGBA);
		}
	} else {
		blitBackgroundAreas();
	}

	DrawListStruct drawList[NUM_MAX_ACTORS + EXTRA_MAX_ENTRIES]; // ListTri[MAX_OBJECTS + MAX_EXTRAS]
	int32 drawListPos = fillActorDrawingList(drawList, bgRedraw);
	drawListPos = fillExtraDrawingList(drawList, drawListPos);

	_currNumOfRedrawBox = 0;
	sortDrawingList(drawList, drawListPos);
	correctZLevels(drawList, drawListPos);
	processDrawList(drawList, drawListPos, bgRedraw);

	if (_engine->_cfgfile.Debug) {
		_engine->_debugScene->renderDebugView();
	}

	renderOverlays();
	renderText();

	_engine->_interface->unsetClip();

	// need to be here to fade after drawing all actors in scene
	if (_engine->_scene->_needChangeScene == SCENE_CEILING_GRID_FADE_2) {
		_engine->_scene->_needChangeScene = SCENE_CEILING_GRID_FADE_1;
	}

	if (bgRedraw) {
		moveNextAreas();
		_engine->unfreezeTime();
	} else {
		flipRedrawAreas();
	}

	if (_engine->_screens->_fadePalette) {
		if (_engine->_screens->_useAlternatePalette) {
			_engine->_screens->fadeToPal(_engine->_screens->_paletteRGBA);
		} else {
			_engine->_screens->fadeToPal(_engine->_screens->_mainPaletteRGBA);
		}
		_engine->_screens->_fadePalette = false;
	}
}

void Redraw::drawBubble(int32 actorIdx) {
	ActorStruct *actor = _engine->_scene->getActor(actorIdx);

	// get actor position on screen
	const IVec3 &projPos = _engine->_renderer->projectPoint(actor->_posObj.x - _engine->_grid->_worldCube.x, actor->_posObj.y + actor->_boundingBox.maxs.y - _engine->_grid->_worldCube.y, actor->_posObj.z - _engine->_grid->_worldCube.z);

	if (actorIdx != _bubbleActor) {
		_bubbleSpriteIndex = _bubbleSpriteIndex ^ 1;
		_bubbleActor = actorIdx;
	}

	const SpriteData &spritePtr = _engine->_resources->_spriteData[_bubbleSpriteIndex];
	const int32 spriteWidth = spritePtr.surface().w;
	const int32 spriteHeight = spritePtr.surface().h;

	// calculate sprite position on screen
	Common::Rect renderRect;
	if (_bubbleSpriteIndex == SPRITEHQR_DIAG_BUBBLE_RIGHT) {
		renderRect.left = projPos.x + 10;
	} else {
		renderRect.left = projPos.x - 10 - spriteWidth;
	}
	renderRect.top = projPos.y - 20;
	renderRect.right = spriteWidth + renderRect.left - 1;
	renderRect.bottom = spriteHeight + renderRect.top - 1;

	if (_engine->_interface->setClip(renderRect)) {
		_engine->_grid->drawSprite(renderRect.left, renderRect.top, spritePtr);
		_engine->_interface->unsetClip();
	}
}

} // namespace TwinE
