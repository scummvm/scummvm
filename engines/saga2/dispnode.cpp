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
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#include "saga2/saga2.h"
#include "saga2/blitters.h"
#include "saga2/detection.h"
#include "saga2/spelshow.h"
#include "saga2/player.h"
#include "saga2/sensor.h"
#include "saga2/mouseimg.h"

namespace Saga2 {

uint8 bubbleColorTable[] = { 1, 0, 0, 0 };

DisplayNode                     *DisplayNodeList::head;

bool                            centerActorIndicatorEnabled;

/* ===================================================================== *
   Imports
 * ===================================================================== */

extern WorldMapData *mapList;

extern StaticPoint16 fineScroll;

extern SpriteSet    *objectSprites,        // object sprites
                    *spellSprites;        // spell effect sprites

ActorAppearance     *tempAppearance;        // test structure

/* ===================================================================== *
   Test spell crap
 * ===================================================================== */

bool InCombatPauseKludge();
//void updateSpellPos( int32 delTime );

//-----------------------------------------------------------------------
//	build the list of stuff to draw (like guns)

uint8 identityColors[256] = {
	0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
	16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
	32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
	48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
	64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
	80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,
	96,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
	112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127,
	128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143,
	144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159,
	160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175,
	176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191,
	192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207,
	208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223,
	224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239,
	240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255
};

//-----------------------------------------------------------------------
//	build the list of stuff to draw (like guns)

void buildDisplayList() {
	g_vm->_mainDisplayList->buildObjects(true);
	g_vm->_activeSpells->buildList();
}

//-----------------------------------------------------------------------
//	Update all objects which have no motion task

void updateObjectAppearances(int32 deltaTime) {
	g_vm->_mainDisplayList->updateOStates(deltaTime);
#ifdef WEWANTSPELLSTOSTOPINCOMBAT
	if (!InCombatPauseKludge())
#endif
		g_vm->_activeSpells->updateStates(deltaTime);
}

//-----------------------------------------------------------------------
//	Draw all sprites on the display list

void drawDisplayList() {
	g_vm->_mainDisplayList->draw();
}

void  DisplayNodeList::init(uint16 s) {
	for (int i = 0; i < s; i++) {
		displayList[i].efx = nullptr;
		displayList[i].nextDisplayed = nullptr;
		displayList[i].object = nullptr;
		displayList[i].type = nodeTypeObject;
	}
}
//-----------------------------------------------------------------------
// DisplayNode stuff

DisplayNode::DisplayNode() {
	nextDisplayed = nullptr;
	sortDepth = 0;
	object = nullptr;
	flags = 0;                  // various flags
	type = nodeTypeObject;
	efx = nullptr;
}

TilePoint DisplayNode::SpellPos() {
	if (efx)
		return efx->current;
	return Nowhere;
}


inline void DisplayNode::updateEffect(const int32 deltaTime) {
	if (efx)   efx->updateEffect(deltaTime);
}

//-----------------------------------------------------------------------
//	Update router

void DisplayNodeList::updateOStates(const int32 deltaTime) {
	if (count)
		for (uint16 i = 0; i < count; i++)
			displayList[i].updateObject(deltaTime);
}

void DisplayNodeList::updateEStates(const int32 deltaTime) {
	if (count)
		for (uint16 i = 0; i < count; i++)
			displayList[i].updateEffect(deltaTime);
}

//-----------------------------------------------------------------------
//	Draw router

void DisplayNodeList::draw() {
	DisplayNode     *dn;
	SpriteSet       *objectSet,
	                *spellSet;

	objectSet = objectSprites;
	if (objectSet == nullptr)
		error("Object sprites have been dumped!\n");

	if (g_vm->getGameId() == GID_FTA2) {
		spellSet = spellSprites;
		if (spellSet == nullptr)
			error("Spell sprites have been dumped!\n");
	}

	for (dn = DisplayNodeList::head; dn; dn = dn->nextDisplayed) {
		if (dn->type == nodeTypeEffect)
			dn->drawEffect();
		else
			dn->drawObject();
	}
}

//-----------------------------------------------------------------------
//	This routine searches through the map and finds the 64
//	objects or actors which are closest to the center view point.

void DisplayNodeList::buildObjects(bool fromScratch) {
	GameObject      *sortList[maxDisplayed + 1];
	int16           distList[maxDisplayed + 1];
	int16           sortCount = 0;
	int16           i;
	int16           viewSize = kTileRectHeight;

	//  Distance at which characters should be loaded.
	int16           loadDist = viewSize + viewSize / 2;

	//  Run through list generated from previous incarnation,
	//  and put to bed all actors which are too far away from the
	//  view region.

	for (i = 0; i < count; i++) {
		DisplayNode *dn = &displayList[i];
		GameObject  *obj = dn->object;
		TilePoint   objLoc = obj->getLocation();
		int16       dist;

		//  Compute distance from object to screen center.
		dist =      ABS(viewCenter.u - objLoc.u)
		            +   ABS(viewCenter.v - objLoc.v);

		//  Determine if the object is beyond the screen threshold
		if ((dist >= loadDist
		        ||  obj->IDParent() != currentWorld->thisID())) {
			//  Mark this object as being off-screen
			obj->setOnScreen(false);

			//  If it's an actor
			if (isActor(obj)) {
				Actor       *a = (Actor *)obj;

				//  Release the actor appearance if loaded
				if (a->_appearance != nullptr) {
					ReleaseActorAppearance(a->_appearance);
					a->_appearance = nullptr;
				}
			}
		}
	}

	if (currentWorld == nullptr) return;

	DispRegionObjectIterator    iter(currentWorld, viewCenter, loadDist);
	GameObject *obj = nullptr;
	ObjectID id;
	int16 dist = 0;
	Actor *centerActor = getCenterActor();

	if (fromScratch)
		//  Reset the list...
		DisplayNodeList::head = nullptr;

	for (id = iter.first(&obj, &dist);
	        id != Nothing;
	        id = iter.next(&obj, &dist)) {
		//  Of object is anywhere near screen center,
		//  then insert object into array, sorted by
		//  distance.

		//  Also, don't add object to display list if it's
		//  invisible.
		if (!(obj->isInvisible())) {
			//  Special processing for actors to "wake up"
			if (isActor(id)) {
				Actor   *a = (Actor *)obj;

				//  If actor is newly entered to the arena
				//  (appearance == NULL), then load the
				//  actor's appearance.
				if (a->_appearance == nullptr) {
					a->_appearance =
					    LoadActorAppearance(a->_appearanceID, sprStandBank);
				}
			}

			//  An insertion sort which has been clamped
			//  to a limited number of items.
			for (i = sortCount; i > 0;) {
				if (dist >= distList[i - 1]) break;
				i--;
				distList[i + 1] = distList[i];
				sortList[i + 1] = sortList[i];
			}

			if (i < maxDisplayed) {
				distList[i] = dist;
				sortList[i] = obj;

				if (sortCount < maxDisplayed) sortCount++;
			}
		}
	}

	//  Build display nodes for each of the objects.

	count = sortCount;

	for (i = 0; i < sortCount; i++) {
		DisplayNode *dn = &displayList[i];
		GameObject  *ob = sortList[i];
		DisplayNode **search;
		TilePoint oLoc = ob->getLocation();
		dn->nextDisplayed = nullptr;
		dn->object = ob;

		dn->type = nodeTypeObject;

		dn->flags = 0;
		if (centerActorIndicatorEnabled
		        &&  isActor(dn->object)
		        && ((Actor *)dn->object) == centerActor)
			dn->flags |= DisplayNode::displayIndicator;

		//  Various test data
//		dn->spriteFrame = 0;

		//  Convert object coordinates to screen coords
		TileToScreenCoords(oLoc, dn->screenCoords);

		//  REM: At this point we could reject some more off-screen
		//  objects.

		//  Set the sort depth for this object
		dn->sortDepth = dn->screenCoords.y + oLoc.z / 2;

		//  Find where we belong on the sorted list
		for (search = &DisplayNodeList::head;
		        *search;
		        search = &(*search)->nextDisplayed) {
			if ((*search)->sortDepth >= dn->sortDepth) break;
		}

		//  Insert into the sorted list
		dn->nextDisplayed = *search;
		*search = dn;
	}
}

//-----------------------------------------------------------------------
//	Update normal objects

void DisplayNode::updateObject(const int32 deltaTime) {
	GameObject  *obj = object;

	if (obj->isMoving()) return;

	if (isActor(obj)) {
		Actor   *a = (Actor *)obj;

		a->updateAppearance(deltaTime);
	}
}

//-----------------------------------------------------------------------
//	Draw sprites for normal objects
void DisplayNode::drawObject() {
	const int maxSpriteWidth = (g_vm->getGameId() == GID_FTA2) ? 32 : 320;
	const int maxSpriteHeight = (g_vm->getGameId() == GID_FTA2) ? 120 : 320;
	const int maxSpriteBaseLine = (g_vm->getGameId() == GID_FTA2) ? 16 : 50;

	ColorTable      mainColors,             // colors for object
	                leftColors,             // colors for left-hand object
	                rightColors;            // colors for right-hand object
	SpriteComponent scList[3],
	                *sc;
	int16           bodyIndex,              // drawing order of body
	                leftIndex,              // drawing order of left
	                rightIndex,             // drawing order of right
	                partCount;              // number of sprite parts
	bool            ghostIt = false;
	GameObject  *obj = object;
	ProtoObj    *proto = obj->proto();
	Point16     drawPos;
	SpriteSet   *ss;
	Sprite      *bodySprite;
	ActorAppearance *aa = nullptr;
	SpriteSet   *sprPtr = nullptr;

	TilePoint   objCoords = obj->getLocation(),
	            tCoords,
	            mCoords;
	MetaTile    *mt;
	RipTable    *rt;

	tCoords.u = (objCoords.u >> kTileUVShift) & kPlatMask;
	tCoords.v = (objCoords.v >> kTileUVShift) & kPlatMask;
	mCoords.u = objCoords.u >> (kTileUVShift + kPlatShift);
	mCoords.v = objCoords.v >> (kTileUVShift + kPlatShift);
	mCoords.z = 0;

	//  Do not display objects that are on a ripped roof
	if ((mt = mapList[g_vm->_currentMapNum].lookupMeta(mCoords)) != nullptr) {
		if ((rt = mt->ripTable(g_vm->_currentMapNum)) != nullptr) {
			if (objCoords.z >= rt->zTable[tCoords.u][tCoords.v]) {
				//  Disable hit-test on the object's box
				hitBox.width = -1;
				hitBox.height = -1;

				obj->setOnScreen(false);
				obj->setObscured(false);
				return;
			}
		}
	}

	TileToScreenCoords(objCoords, screenCoords);

	drawPos.x = screenCoords.x + fineScroll.x;
	drawPos.y = screenCoords.y + fineScroll.y;

	//  If it's an object, then the drawing is fairly straight
	//  forward.
	if (isObject(obj)) {
		ObjectSpriteInfo    sprInfo;

		//  Reject any sprites which fall off the edge of the screen.
		if (drawPos.x < -32
		        || drawPos.x > kTileRectX + kTileRectWidth + 32
		        || drawPos.y < -32
		        || drawPos.y > kTileRectY + kTileRectHeight + 100) {
			//  Disable hit-test on the object's box
			hitBox.width = -1;
			hitBox.height = -1;

			//  Mark as being off screen
			obj->setOnScreen(false);
			obj->setObscured(false);
			return;
		}

		if (!obj->isOnScreen()) {
			SenseInfo   info;

			obj->setOnScreen(true);

			if (getCenterActor()->canSenseSpecificObject(info, maxSenseRange, obj->thisID()))
				obj->setSightedByCenter(true);
			else {
				obj->setSightedByCenter(false);
				obj->setObscured(false);
			}

			obj->_data.sightCtr = 5;
		} else {
			if (--obj->_data.sightCtr == 0) {
				SenseInfo   info;

				if (getCenterActor()->canSenseSpecificObject(info, maxSenseRange, obj->thisID()))
					obj->setSightedByCenter(true);
				else {
					obj->setSightedByCenter(false);
					obj->setObscured(false);
				}

				obj->_data.sightCtr = 5;
			}
		}

		//  Figure out which sprite to show
		sprInfo = proto->getSprite(obj, ProtoObj::objOnGround);

		//  Build the color translation table for the object
		obj->getColorTranslation(mainColors);

		//  Fill in the SpriteComponent structure
		sc = &scList[0];
		sc->sp = sprInfo.sp;
		sc->offset.x = scList->offset.y = 0;
		sc->colorTable = mainColors;

		sc->flipped = sprInfo.flipped;

		partCount = 1;
		bodyIndex = 0;

	} else {
		Actor           *a = (Actor *)obj;
		ActorAnimation  *anim;
		ActorPose       *pose;
		int16           poseFlags;

		if (!a->isDead() && objCoords.z < -proto->height - 8) {
			//  The actor is under water so display the bubbles sprite
			drawPos.y += objCoords.z;
			objCoords.z = 0;

			//  Disable hit-test on the object's box
			hitBox.width = -1;
			hitBox.height = -1;

			//  Reject any sprites which fall off the edge of the screen.
			if (drawPos.x < -maxSpriteWidth
			        || drawPos.x > kTileRectX + kTileRectWidth + maxSpriteWidth
			        || drawPos.y < -maxSpriteBaseLine
			        || drawPos.y > kTileRectY + kTileRectHeight + maxSpriteHeight) {
				//  Mark as being off screen
				a->setOnScreen(false);
				a->setObscured(false);
				return;
			}

			buildColorTable(
			    mainColors,
			    bubbleColorTable,
			    ARRAYSIZE(bubbleColorTable));

			if (a->_kludgeCount < 0 || ++a->_kludgeCount >= kBubbleSpriteCount)
				a->_kludgeCount = 0;

			sc = &scList[0];
			sc->sp = spellSprites->sprite(
			             kBaseBubbleSpriteIndex + a->_kludgeCount);
			sc->offset.x = scList->offset.y = 0;
			sc->colorTable = mainColors;
			sc->flipped = false;

			partCount = 1;
			bodyIndex = 0;
		} else {
			//  Reject any sprites which fall off the edge of the screen.
			if (drawPos.x < -maxSpriteWidth
			        || drawPos.x > kTileRectX + kTileRectWidth + maxSpriteWidth
			        || drawPos.y < -maxSpriteBaseLine
			        || drawPos.y > kTileRectY + kTileRectHeight + maxSpriteHeight) {
				//  Disable hit-test on the object's box
				hitBox.width = -1;
				hitBox.height = -1;

				//  Mark as being off screen
				a->setOnScreen(false);
				a->setObscured(false);
				return;
			}

			if (a->hasEffect(actorInvisible)) {
				if (!isPlayerActor(a)
				        &&  !(getCenterActor()->hasEffect(actorSeeInvis))) {
					hitBox.width = -1;
					hitBox.height = -1;
					return;
				}
				ghostIt = true;
			}

			if (!a->isOnScreen()) {
				SenseInfo   info;

				a->setOnScreen(true);

				if (getCenterActor()->canSenseSpecificActor(info, maxSenseRange, a))
					a->setSightedByCenter(true);
				else {
					a->setSightedByCenter(false);
					a->setObscured(false);
				}

				a->_data.sightCtr = 5;
			} else {
				if (--a->_data.sightCtr == 0) {
					SenseInfo   info;

					if (getCenterActor()->canSenseSpecificActor(info, maxSenseRange, a))
						a->setSightedByCenter(true);
					else {
						a->setSightedByCenter(false);
						a->setObscured(false);
					}

					a->_data.sightCtr = 5;
				}
			}

			aa = a->_appearance;

			if (aa == nullptr)
				return;

			//  Fetch the animation series, and determine which
			//  pose in the series is the current one.
			anim = aa->animation(a->_currentAnimation);
			pose = aa->pose(anim, a->_currentFacing, a->_currentPose);

			if (anim == nullptr)
				return;

			assert(anim->start[0] <  10000);
			assert(anim->start[1] <  10000);
			assert(anim->start[2] <  10000);

			assert(pose->rightObjectOffset.x < 1000);
			assert(pose->rightObjectOffset.x > -1000);
			assert(pose->rightObjectOffset.y < 1000);
			assert(pose->rightObjectOffset.y > -1000);

			assert(pose->leftObjectOffset.x < 1000);
			assert(pose->leftObjectOffset.x > -1000);
			assert(pose->leftObjectOffset.y < 1000);
			assert(pose->leftObjectOffset.y > -1000);

			//          washHandle( aa->spriteBanks[pose->actorFrameBank] );

			//  If the new sprite is loaded, then we can go
			//  ahead and show it. If it's not, then we can
			//  pause for a frame or two until it is loaded.
			//  However, if the previous frame isn't loaded
			//  either, then we need to go ahead and force
			//  the new frame to finish loaded (handled by
			//  lockResource())
			if (aa->isBankLoaded(pose->actorFrameBank)
			        || !aa->isBankLoaded(a->_poseInfo.actorFrameBank)) {
				ActorPose   pTemp = *pose;

				//  Initiate a load of the sprite bank needed.
				/*  if (!RHandleLoading(
				            (RHANDLE)(aa->spriteBanks[pose->actorFrameBank]) ))
				    {
				        aa->loadSpriteBanks( (1<<pose->actorFrameBank) );
				    } */

				aa->requestBank(pose->actorFrameBank);

				//  Indicate that animation is OK.
				a->_animationFlags &= ~animateNotLoaded;

				//  Set up which bank and frame to use.
				a->_poseInfo = pTemp;
			} else {
				//  Indicate that animation isn't loaded
				a->_animationFlags |= animateNotLoaded;

				//  Initiate a load of the sprite bank needed.
				/*  if (!RHandleLoading(
				            (RHANDLE)(aa->spriteBanks[pose->actorFrameBank]) ))
				    {
				        aa->loadSpriteBanks( (1<<pose->actorFrameBank) );
				    }
				    */
				aa->requestBank(pose->actorFrameBank);
			}

			//  For actors, start by assuming that the actor has
			//  nothing in either hand.

			bodyIndex = 0;
			rightIndex = leftIndex = -2;
			partCount = 1;
			poseFlags = a->_poseInfo.flags;

			a->getColorTranslation(mainColors);

			//  Do various tests to see what the actor is
			//  carrying in each hand, and what drawing
			//  order should be used for these objects.

			if (a->_leftHandObject != Nothing) {
				partCount++;

				if (poseFlags & ActorPose::leftObjectInFront) {
					leftIndex = 1;
				} else {
					leftIndex = 0;
					bodyIndex = 1;
				}
			}

			if (a->_rightHandObject != Nothing) {
				partCount++;

				if (poseFlags & ActorPose::rightObjectInFront) {
					if (leftIndex == 1
					        &&  poseFlags & ActorPose::leftOverRight) {
						leftIndex = 2;
						rightIndex = 1;
					} else {
						rightIndex = partCount - 1;
					}
				} else {
					if (leftIndex == 0
					        &&  poseFlags & ActorPose::leftOverRight) {
						rightIndex = 0;
						leftIndex = 1;
						bodyIndex = 2;
					} else {
						rightIndex = 0;
						bodyIndex++;
						if (leftIndex != -2) leftIndex++;
					}
				}
			}


			//  REM: Locking bug...

			//          ss = (SpriteSet *)RLockHandle( aa->sprites );
			sprPtr = aa->spriteBanks[a->_poseInfo.actorFrameBank];
			ss = sprPtr;
			if (ss == nullptr)
				return;

			//  Fill in the SpriteComponent structure for body
			sc = &scList[bodyIndex];
			assert(a->_poseInfo.actorFrameIndex < ss->count);
			sc->sp = ss->sprite(a->_poseInfo.actorFrameIndex);
			sc->offset.x = sc->offset.y = 0;
			//  Color remapping info
			sc->colorTable = mainColors;
			//          sc->colorTable = aa->schemeList ? mainColors : identityColors;
			sc->flipped = (poseFlags & ActorPose::actorFlipped);

			assert(sc->sp != nullptr);
			assert(sc->sp->size.x > 0);
			assert(sc->sp->size.y > 0);
			assert(sc->sp->size.x < 255);
			assert(sc->sp->size.y < 255);

			//  If we were carrying something in the left hand,
			//  then fill in the component structure for it.
			if (leftIndex >= 0) {
				GameObject *ob = GameObject::objectAddress(a->_leftHandObject);
				ProtoObj *prot = ob->proto();

				ob->getColorTranslation(leftColors);

				sc = &scList[leftIndex];
				sc->sp =    prot->getOrientedSprite(
				                ob,
				                a->_poseInfo.leftObjectIndex);
				assert(sc->sp != nullptr);
				sc->offset = a->_poseInfo.leftObjectOffset;
				assert(sc->offset.x < 1000);
				assert(sc->offset.x > -1000);
				assert(sc->offset.y < 1000);
				assert(sc->offset.y > -1000);
				sc->colorTable = leftColors;
				sc->flipped = (poseFlags & ActorPose::leftObjectFlipped);
			}

			//  If we were carrying something in the right hand,
			//  then fill in the component structure for it.
			if (rightIndex >= 0) {
				GameObject *ob = GameObject::objectAddress(a->_rightHandObject);
				ProtoObj *prot = ob->proto();

				ob->getColorTranslation(rightColors);

				sc = &scList[rightIndex];
				sc->sp =    prot->getOrientedSprite(
				                ob,
				                a->_poseInfo.rightObjectIndex);
				assert(sc->sp != nullptr);
				assert(sc->sp->size.x > 0);
				assert(sc->sp->size.y > 0);
				assert(sc->sp->size.x < 255);
				assert(sc->sp->size.y < 255);
				sc->offset = a->_poseInfo.rightObjectOffset;
				assert(sc->offset.x < 1000);
				assert(sc->offset.x > -1000);
				assert(sc->offset.y < 1000);
				assert(sc->offset.y > -1000);
				sc->colorTable = rightColors;
				sc->flipped = (poseFlags & ActorPose::rightObjectFlipped);
			}
		}
	}

	if (!ghostIt && obj->isGhosted())
		ghostIt = true;

	int16       effectFlags = 0;
	bool        obscured;

	if (ghostIt) effectFlags |= sprFXGhosted;

	if (obj->isSightedByCenter() && objRoofRipped(obj))
		effectFlags |= sprFXGhostIfObscured;

	effectFlags |= sprFXTerrainMask;

	DrawCompositeMaskedSprite(
	    g_vm->_backPort,
	    scList,
	    partCount,
	    drawPos,
	    objCoords,
	    effectFlags,
	    &obscured);

	if (effectFlags & sprFXGhostIfObscured)
		obj->setObscured(obscured);

	//  Record the extent box that the sprite was drawn
	//  at, in order to facilitate mouse picking functions
	//  later on in the event loop.
	bodySprite = scList[bodyIndex].sp;
	hitBox.x =      drawPos.x
	                + (scList[bodyIndex].flipped
	                   ?   -bodySprite->size.x - bodySprite->offset.x
	                   :   bodySprite->offset.x)
	                -   fineScroll.x;
	hitBox.y = drawPos.y + bodySprite->offset.y - fineScroll.y;
	hitBox.width = bodySprite->size.x;
	hitBox.height = bodySprite->size.y;

	if (flags & displayIndicator) {
		Point16     indicatorCoords;
		gPixelMap   &indicator = *mouseCursors[kMouseCenterActorIndicatorImage];

		indicatorCoords.x = hitBox.x + fineScroll.x + (hitBox.width - indicator.size.x) / 2;
		indicatorCoords.y = hitBox.y + fineScroll.y - indicator.size.y - 2;

		TBlit(g_vm->_backPort.map, &indicator, indicatorCoords.x, indicatorCoords.y);
	}
}

//-----------------------------------------------------------------------
//	Do mouse hit-test on objects

ObjectID pickObject(const StaticPoint32 &mouse, StaticTilePoint &objPos) {
	DisplayNode     *dn;
	ObjectID        result = Nothing;
	int32           dist = maxint32;
	SpriteSet       *objectSet;

	objectSet = objectSprites;
	if (objectSet == nullptr)
		error("Object sprites have been dumped!");

	for (dn = DisplayNodeList::head; dn; dn = dn->nextDisplayed) {
		if (dn->type == nodeTypeObject) {
			GameObject  *obj = dn->object;

			if (obj->parent() == currentWorld && dn->hitBox.ptInside(mouse.x, mouse.y)) {
				TilePoint   loc = obj->getLocation();
				int32       newDist = loc.u + loc.v;

				if (newDist < dist) {
					Point16     testPoint;
					SpriteSet   *ss;
					Sprite      *spr;
					ActorAppearance *aa = nullptr;
					SpriteSet   *sprPtr = nullptr;
					bool        flipped = true;

					testPoint.x = mouse.x - dn->hitBox.x;
					testPoint.y = mouse.y - dn->hitBox.y;

					//  If it's an object, then the test is fairly straight
					//  forward.
					if (isObject(obj)) {
						ObjectSpriteInfo    sprInfo;

						sprInfo = obj->proto()->getSprite(obj, ProtoObj::objOnGround);
						spr = sprInfo.sp;
						flipped = sprInfo.flipped;
					} else {
						Actor   *a = (Actor *)obj;

						aa = a->_appearance;

						if (aa == nullptr) continue;

						sprPtr = aa->spriteBanks[a->_poseInfo.actorFrameBank];
						ss = sprPtr;
						if (ss == nullptr)
							continue;

						spr = ss->sprite(a->_poseInfo.actorFrameIndex);
						flipped =
						    (a->_poseInfo.flags & ActorPose::actorFlipped) ? 1 : 0;
					}

					if (GetSpritePixel(spr, flipped, testPoint)) {
						dist = newDist;
						result = obj->thisID();
						objPos.set(loc.u, loc.v, loc.z);
						objPos.z += MAX(-spr->offset.y - testPoint.y, 0);
					} else if (result == Nothing) { //  If no object found yet
						Point16     testPoint2;
						int16       minX, maxX;

						//  Try checking a wider area for mouse hit

						testPoint2.y = testPoint.y;
						minX = MAX(0, testPoint.x - 6);
						maxX = MIN(dn->hitBox.width - 1, testPoint.x + 6);

						//  scan a horizontal strip of the character for a hit.
						//  If we find a hit, go ahead and set result anyway
						//  If we later find a real hit, then it will overwrite
						//  the results of this one.
						for (testPoint2.x = minX; testPoint2.x <= maxX; testPoint2.x++) {
							if (GetSpritePixel(spr, flipped, testPoint2)) {
								result = obj->thisID();
								objPos.set(loc.u, loc.v, loc.z);
								objPos.z += MAX(-spr->offset.y - testPoint.y, 0);
								break;
							}
						}
					}
				}
			}
		}
	}

	return result;
}

//-----------------------------------------------------------------------
//	Adds spell effects into the dispplay list
//
//  NOTE : all spell effects are currently placed behind any real stuff
//         they can also easily be placed in front

void DisplayNodeList::buildEffects(bool) {
	if (count) {
		for (int i = 0; i < count; i++) {
			DisplayNode *dn = DisplayNodeList::head;

			if (displayList[i].efx->isHidden() || displayList[i].efx->isDead())
				continue;
			// make sure it knows it's not a real object
			displayList[i].type = nodeTypeEffect;

			displayList[i].sortDepth = displayList[i].efx->screenCoords.y + displayList[i].efx->current.z / 2;
			if (dn) {
				int32 sd = displayList[i].sortDepth;
				while (dn->nextDisplayed && dn->nextDisplayed->sortDepth <= sd)
					dn = dn->nextDisplayed;
			}

			if (dn == DisplayNodeList::head) {
				displayList[i].nextDisplayed = DisplayNodeList::head;
				DisplayNodeList::head = &displayList[i];
			} else {
				displayList[i].nextDisplayed = dn->nextDisplayed;
				dn->nextDisplayed = &displayList[i];
			}

		}
	}
}

bool DisplayNodeList::dissipated() {
	if (count) {
		for (int i = 0; i < count; i++) {
			if (displayList[i].efx && !displayList[i].efx->isDead())
				return false;
		}
	}
	return true;
}

//-----------------------------------------------------------------------
//	Draw sprites for spell effects
//
//  NOTE : all spell effects currently use the center actor for their
//         sprites.

void DisplayNode::drawEffect() {
	if (efx)   efx->drawEffect();
}

void Effectron::drawEffect() {
	ColorTable      eColors;                // colors for object
	bool obscured = false;
	Point16         drawPos;
	TilePoint       objCoords = SpellPos();
	SpriteComponent scList[3],
	                *sc;

	if (isHidden() || isDead())
		return;

	drawPos.x = screenCoords.x + fineScroll.x;
	drawPos.y = screenCoords.y + fineScroll.y;

	//  Reject any sprites which fall off the edge of the screen.
	if (drawPos.x < -32
	        || drawPos.x > kTileRectX + kTileRectWidth + 32
	        || drawPos.y < -32
	        || drawPos.y > kTileRectY + kTileRectHeight + 100) {
		//  Disable hit-test on the object's box
		hitBox.width = -1;
		hitBox.height = -1;
		return;
	}

	TileToScreenCoords(objCoords, screenCoords);

	sc = &scList[0];
	//sc->sp = (*spellSprites)->sprite( spriteID() );
	sc->sp = spellSprites->sprite(spriteID());   //tempSpellSpriteIDs[rand()%39] );
	sc->offset.x = scList->offset.y = 0;

	(*g_vm->_sdpList)[parent->spell]->
	getColorTranslation(eColors, this);

	sc->colorTable = eColors;
	sc->flipped = false;

	obscured = (visiblePixelsInSprite(sc->sp,
	                                  sc->flipped,
	                                  sc->colorTable,
	                                  drawPos,
	                                  current,
	                                  0) <= 5);

	DrawCompositeMaskedSprite(
	    g_vm->_backPort,
	    scList,
	    1,
	    drawPos,
	    objCoords,
	    ((obscured) &&    //objectFlags & GameObject::objectObscured ) &&
	     0
	     ? sprFXGhosted : sprFXTerrainMask));

}

/* ===================================================================== *
   Misc. functions
 * ===================================================================== */

//-----------------------------------------------------------------------
//	Enable or disable the center actor indicator

void setCenterActorIndicator(bool enabled) {
	centerActorIndicatorEnabled = enabled;
}

} // end of namespace Saga2
