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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/stream.h"
#include "common/memstream.h"

#include "startrek/iwfile.h"
#include "startrek/resource.h"
#include "startrek/room.h"
#include "startrek/startrek.h"

namespace StarTrek {

void StarTrekEngine::initActors() {
	for (int i = 0; i < NUM_ACTORS; i++)
		_actorList[i] = Actor();

	for (int i = 0; i < MAX_BAN_FILES; i++) {
		delete _banFiles[i];
		_banFiles[i] = nullptr;
	}

	_kirkActor->animationString = "kstnd";
	_spockActor->animationString = "sstnd";
	_mccoyActor->animationString = "mstnd";
	_redshirtActor->animationString = "rstnd";
}

int StarTrekEngine::loadActorAnim(int actorIndex, const Common::String &animName, int16 x, int16 y, Fixed8 scale) {
	debugC(6, kDebugGraphics, "Load animation '%s' on actor %d", animName.c_str(), actorIndex);

	if (actorIndex == -1) {
		bool foundSlot = false;

		for (int i = 8; i < NUM_ACTORS; i++) {
			if (_actorList[i].spriteDrawn == 0) {
				actorIndex = i;
				foundSlot = true;
				break;
			}
		}

		if (!foundSlot) {
			error("All animations are in use");
		}
	}

	Actor *actor = &_actorList[actorIndex];

	if (actor->spriteDrawn) {
		releaseAnim(actor);
		drawActorToScreen(actor, animName, x, y, scale, false);
	} else {
		drawActorToScreen(actor, animName, x, y, scale, true);
	}

	actor->triggerActionWhenAnimFinished = false;
	actor->finishedAnimActionParam = 0;

	return actorIndex;
}

void StarTrekEngine::loadBanFile(const Common::String &name) {
	debugC(kDebugGeneral, 7, "Load BAN file: %s.ban", name.c_str());
	for (int i = 0; i < MAX_BAN_FILES; i++) {
		if (!_banFiles[i]) {
			_banFiles[i] = _resource->loadFile(name + ".ban");
			_banFileOffsets[i] = 0;
			return;
		}
	}

	warning("Couldn't load .BAN file \"%s.ban\"", name.c_str());
}

bool StarTrekEngine::actorWalkToPosition(int actorIndex, const Common::String &animFile, int16 srcX, int16 srcY, int16 destX, int16 destY) {
	debugC(6, "Obj %d: walk from (%d,%d) to (%d,%d)", actorIndex, srcX, srcY, destX, destY);

	Actor *actor = &_actorList[actorIndex];

	actor->triggerActionWhenAnimFinished = false;
	if (isPositionSolid(destX, destY))
		return false;

	if (actor->spriteDrawn)
		releaseAnim(actor);
	else
		_gfx->addSprite(&actor->sprite);

	actor->spriteDrawn = true;
	actor->animType = 1;
	actor->frameToStartNextAnim = _frameIndex + 1;
	actor->animationString2 = animFile;

	actor->dest.x = destX;
	actor->dest.y = destY;
	actor->field92 = 0;
	actor->triggerActionWhenAnimFinished = false;

	actor->iwDestPosition = -1;
	actor->iwSrcPosition = -1;

	if (directPathExists(srcX, srcY, destX, destY)) {
		chooseActorDirectionForWalking(actor, srcX, srcY, destX, destY);
		updateActorPositionWhileWalking(actor, (actor->granularPosX + 0.5).toInt(), (actor->granularPosY + 0.5).toInt());
		return true;
	} else {
		actor->iwSrcPosition = _iwFile->getClosestKeyPosition(srcX, srcY);
		actor->iwDestPosition = _iwFile->getClosestKeyPosition(destX, destY);

		if (actor->iwSrcPosition == -1 || actor->iwDestPosition == -1) {
			// No path exists; face south by default.
			actor->animationString2 += "S";
			actor->direction = 'S';

			updateActorPositionWhileWalking(actor, srcX, srcY);
			initStandAnim(actorIndex);

			return false;
		} else {
			Common::Point iwSrc = _iwFile->_keyPositions[actor->iwSrcPosition];
			chooseActorDirectionForWalking(actor, srcX, srcY, iwSrc.x, iwSrc.y);
			updateActorPositionWhileWalking(actor, (actor->granularPosX + 0.5).toInt(), (actor->granularPosY + 0.5).toInt());
			return true;
		}
	}
}

void StarTrekEngine::updateActorAnimations() {
	for (int i = 0; i < NUM_ACTORS; i++) {
		Actor *actor = &_actorList[i];
		if (!actor->spriteDrawn)
			continue;

		switch (actor->animType) {
		case 0: // Not walking?
		case 2:
			if (_frameIndex >= actor->frameToStartNextAnim) {
				int nextAnimIndex = getRandomWord() & 3;
				actor->animFile->seek(18 + nextAnimIndex + actor->animFrame * 22, SEEK_SET);
				byte nextAnimFrame = actor->animFile->readByte();

				if (actor->animFrame != nextAnimFrame) {
					if (nextAnimFrame == actor->numAnimFrames - 1) {
						actor->field62++;
						if (actor->triggerActionWhenAnimFinished) {
							addAction(ACTION_FINISHED_ANIMATION, actor->finishedAnimActionParam, 0, 0);
						}
					}
				}

				actor->animFrame = nextAnimFrame;
				if (actor->animFrame >= actor->numAnimFrames) {
					if (actor->animationString.empty())
						removeActorFromScreen(i);
					else
						initStandAnim(i);
				} else {
					Sprite *sprite = &actor->sprite;

					actor->animFile->seek(actor->animFrame * 22, SEEK_SET);
					char animFrameFilename[16];
					actor->animFile->read(animFrameFilename, 16);
					actor->bitmapFilename = animFrameFilename;
					actor->bitmapFilename.trim();
					if (actor->bitmapFilename.contains(' '))
						actor->bitmapFilename = actor->bitmapFilename.substr(0, actor->bitmapFilename.find(' '));

					sprite->setBitmap(loadAnimationFrame(actor->bitmapFilename, actor->scale));

					actor->animFile->seek(10 + actor->animFrame * 22, SEEK_SET);
					uint16 xOffset = actor->animFile->readUint16();
					uint16 yOffset = actor->animFile->readUint16();
					uint16 basePriority = actor->animFile->readUint16();
					uint16 frames = actor->animFile->readUint16();

					sprite->pos.x = xOffset + actor->pos.x;
					sprite->pos.y = yOffset + actor->pos.y;
					sprite->drawPriority = _gfx->getPriValue(0, yOffset + actor->pos.y) + basePriority;
					sprite->bitmapChanged = true;

					actor->frameToStartNextAnim = frames + _frameIndex;
				}
			}
			break;
		case 1: // Walking
			if (_frameIndex < actor->frameToStartNextAnim)
				break;
			if (i == 0) // Kirk only
				checkTouchedLoadingZone(actor->pos.x, actor->pos.y);
			if (actor->field90 != 0) {
				Sprite *sprite = &actor->sprite;
				int loops;
				if (getActorScaleAtPosition((actor->granularPosY + 0.5).toInt()) < 0.625)
					loops = 1;
				else
					loops = 2;
				for (int k = 0; k < loops; k++) {
					if (actor->field90 == 0)
						break;
					actor->field90--;
					Fixed16 newX = actor->granularPosX + actor->speedX;
					Fixed16 newY = actor->granularPosY + actor->speedY;
					if ((actor->field90 & 3) == 0) {
						delete sprite->bitmap;
						sprite->bitmap = nullptr;
						updateActorPositionWhileWalking(actor, (newX + 0.5).toInt(), (newY + 0.5).toInt());
						actor->field92++;
					}

					actor->granularPosX = newX;
					actor->granularPosY = newY;
					actor->frameToStartNextAnim = _frameIndex;
				}
			} else { // actor->field90 == 0
				if (actor->iwSrcPosition == -1) {
					if (actor->triggerActionWhenAnimFinished) {
						actor->triggerActionWhenAnimFinished = false;
						addAction(ACTION_FINISHED_WALKING, actor->finishedAnimActionParam & 0xff, 0, 0);
					}

					delete actor->sprite.bitmap;
					actor->sprite.bitmap = nullptr;
					updateActorPositionWhileWalking(actor, (actor->granularPosX + 0.5).toInt(), (actor->granularPosY + 0.5).toInt());
					initStandAnim(i);
				} else { // actor->iwSrcPosition != -1
					if (actor->iwSrcPosition == actor->iwDestPosition) {
						actor->animationString2.deleteLastChar();
						actor->iwDestPosition = -1;
						actor->iwSrcPosition = -1;
						chooseActorDirectionForWalking(actor, actor->pos.x, actor->pos.y, actor->dest.x, actor->dest.y);
					} else {
						int index = _iwFile->_iwEntries[actor->iwSrcPosition][actor->iwDestPosition];
						actor->iwSrcPosition = index;
						Common::Point dest = _iwFile->_keyPositions[actor->iwSrcPosition];
						actor->animationString2.deleteLastChar();
						chooseActorDirectionForWalking(actor, actor->pos.x, actor->pos.y, dest.x, dest.y);
					}
				}
			}
			break;
		default:
			error("Invalid anim type.");
			break;
		}
	}
}

void StarTrekEngine::renderBanBelowSprites() {
	if ((_frameIndex & 3) != 0)
		return;

	byte *screenPixels = _gfx->lockScreenPixels();
	byte *bgPixels = _gfx->getBackgroundPixels();

	for (int i = 0; i < MAX_BAN_FILES; i++) {
		if (!_banFiles[i])
			continue;

		// TODO: video modes other than VGA

		_banFiles[i]->seek(_banFileOffsets[i], SEEK_SET);
		uint16 offset = _banFiles[i]->readUint16();

		if (offset == 0xffff) {
			_banFileOffsets[i] = 0;
			_banFiles[i]->seek(0, SEEK_SET);
			offset = _banFiles[i]->readSint16();
		}

		int16 size = _banFiles[i]->readSint16();
		if (size != 0)
			renderBan(screenPixels, bgPixels, i);
	}

	_gfx->unlockScreenPixels();
}

void StarTrekEngine::renderBan(byte *screenPixels, byte *bgPixels, int banFileIndex) {
	Common::MemoryReadStreamEndian *banFile = _banFiles[banFileIndex];
	banFile->seek(_banFileOffsets[banFileIndex], SEEK_SET);

	uint16 offset = banFile->readUint16();
	int32 size = banFile->readUint16();

	byte *dest1 = screenPixels + offset;
	byte *dest2 = bgPixels + offset;

	// Skip 8 bytes (rectangle encompassing the area being drawn to)
	banFile->skip(8);

	while (--size >= 0) {
		assert(dest1 >= screenPixels && dest1 < screenPixels + SCREEN_WIDTH * SCREEN_HEIGHT);
		assert(dest2 >= bgPixels && dest2 < bgPixels + SCREEN_WIDTH * SCREEN_HEIGHT);

		int8 b = banFile->readByte();

		if (b == -128) {	// Add value to destination (usually jumping to next row)
			uint16 skip = banFile->readUint16();
			dest1 += skip;
			dest2 += skip;
		} else if (b < 0) { // Repeated byte
			byte c = banFile->readByte();
			if (c == 0) {
				dest1 += (-b) + 1;
				dest2 += (-b) + 1;
			} else {
				for (int j = 0; j < (-b) + 1; j++) {
					(*dest1++) = c;
					(*dest2++) = c;
				}
			}
		} else { // List of bytes
			b++;
			while (b-- != 0) {
				byte c = banFile->readByte();
				if (c == 0) {
					dest1++;
					dest2++;
				} else {
					*(dest1++) = c;
					*(dest2++) = c;
				}
			}
		}
	}
}

void StarTrekEngine::renderBanAboveSprites() {
	if ((_frameIndex & 3) != 0)
		return;

	for (int i = 0; i < MAX_BAN_FILES; i++) {
		if (!_banFiles[i])
			continue;

		_banFiles[i]->seek(_banFileOffsets[i], SEEK_SET);
		uint16 offset = _banFiles[i]->readUint16();

		if (offset == 0xffff) {
			_banFileOffsets[i] = 0;
			_banFiles[i]->seek(0, SEEK_SET);
			offset = _banFiles[i]->readSint16();
		}

		int16 size = _banFiles[i]->readSint16();
		if (size != 0) {
			Common::Rect rect;
			rect.left   = _banFiles[i]->readSint16();
			rect.top    = _banFiles[i]->readSint16();
			rect.right  = _banFiles[i]->readSint16() + 1;
			rect.bottom = _banFiles[i]->readSint16() + 1;

			// Draw all sprites in this rectangle to a custom surface, and only update the
			// specific pixels that were updated by the BAN file this frame.
			// Rationale behind this is that, since the background may not have been
			// redrawn, the transparent sprites (ie. textboxes) would further darken any
			// pixels behind them that haven't been updated this frame. So, we can't just
			// update everything in this rectangle.
			// FIXME: This copies the entire screen surface for temporary drawing, which
			// is somewhat wasteful. Original game had one more graphics layer it drew to
			// before the screen was updated...
			::Graphics::Surface surface;
			_gfx->drawAllSpritesInRectToSurface(rect, &surface);

			byte *destPixels = _gfx->lockScreenPixels();
			byte *src = (byte *)surface.getPixels() + offset;
			byte *dest = destPixels + offset;

			// This is similar to renderBan(), except it copies pixels from the surface
			// above instead of drawing directly to it. (Important since sprites may be
			// drawn on top.)
			while (--size >= 0) {
				assert(dest >= destPixels && dest < destPixels + SCREEN_WIDTH * SCREEN_HEIGHT);
				int8 b = _banFiles[i]->readByte();
				if (b == -128) {
					uint16 skip = _banFiles[i]->readUint16();
					dest += skip;
					src  += skip;
				} else if (b < 0) {
					byte c = _banFiles[i]->readByte();
					if (c == 0) {
						dest += (-b) + 1;
						src  += (-b) + 1;
					}
					else {
						for (int j = 0; j < (-b) + 1; j++)
							*(dest++) = *(src++);
					}
				} else {
					b++;
					while (b-- != 0) {
						byte c = _banFiles[i]->readByte();
						if (c == 0) {
							dest++;
							src++;
						} else
							*(dest++) = *(src++);
					}
				}
			}

			_gfx->unlockScreenPixels();
			surface.free();

			_banFileOffsets[i] = _banFiles[i]->pos();
		}
	}
}

void StarTrekEngine::removeActorFromScreen(int actorIndex) {
	Actor *actor = &_actorList[actorIndex];

	if (actor->spriteDrawn != 1)
		return;

	debugC(6, kDebugGraphics, "Stop drawing actor %d", actorIndex);

	Sprite *sprite = &actor->sprite;
	sprite->field16 = true;
	sprite->bitmapChanged = true;
	_gfx->drawAllSprites();
	_gfx->delSprite(sprite);
	releaseAnim(actor);
}

void StarTrekEngine::removeDrawnActorsFromScreen() {
	for (int i = 0; i < NUM_ACTORS; i++) {
		if (_actorList[i].spriteDrawn == 1) {
			removeActorFromScreen(i);
		}
	}

	for (int i = 0; i < MAX_BAN_FILES; i++) {
		delete _banFiles[i];
		_banFiles[i] = nullptr;
	}
}

void StarTrekEngine::drawActorToScreen(Actor *actor, const Common::String &_animName, int16 x, int16 y, Fixed8 scale, bool addSprite) {
	Common::String animFilename = _animName;
	if (_animName.hasPrefixIgnoreCase("stnd") /* && word_45d20 == -1 */) // TODO
		animFilename += 'j';

	actor->animFilename = _animName;
	actor->animType = 2;
	actor->animFile = SharedPtr<Common::MemoryReadStreamEndian>(_resource->loadFile(animFilename + ".anm"));
	actor->numAnimFrames = actor->animFile->size() / 22;
	actor->animFrame = 0;
	actor->pos.x = x;
	actor->pos.y = y;
	actor->field62 = 0;
	actor->scale = scale;

	actor->animFile->seek(16, SEEK_SET);
	actor->frameToStartNextAnim = actor->animFile->readUint16() + _frameIndex;

	char firstFrameFilename[11];
	actor->animFile->seek(0, SEEK_SET);
	actor->animFile->read(firstFrameFilename, 10);
	firstFrameFilename[10] = '\0';

	Sprite *sprite = &actor->sprite;
	if (addSprite)
		_gfx->addSprite(sprite);

	actor->bitmapFilename = firstFrameFilename;
	actor->bitmapFilename.trim();
	sprite->setBitmap(loadAnimationFrame(actor->bitmapFilename, scale));
	actor->scale = scale;
	actor->animFile->seek(10, SEEK_SET);

	uint16 xOffset = actor->animFile->readUint16();
	uint16 yOffset = actor->animFile->readUint16();
	uint16 basePriority = actor->animFile->readUint16();

	sprite->pos.x = xOffset + actor->pos.x;
	sprite->pos.y = yOffset + actor->pos.y;
	sprite->drawPriority = _gfx->getPriValue(0, yOffset + actor->pos.y) + basePriority;
	sprite->bitmapChanged = true;

	actor->spriteDrawn = 1;
}

void StarTrekEngine::releaseAnim(Actor *actor) {
	switch (actor->animType) {
	case 0:
	case 2:
		actor->animFile.reset();
		// Fall through
	case 1:
		delete actor->sprite.bitmap;
		actor->sprite.bitmap = nullptr;
		break;
	default:
		error("Invalid anim type");
		break;
	}

	actor->spriteDrawn = 0;
}

void StarTrekEngine::initStandAnim(int actorIndex) {
	Actor *actor = &_actorList[actorIndex];

	if (!actor->spriteDrawn)
		error("initStandAnim: dead anim");

	////////////////////
	// sub_239d2
	const char *directions = "nsew";

	if (actorIndex >= 0 && actorIndex <= 3) {
		int8 dir = _awayMission.crewDirectionsAfterWalk[actorIndex];
		if (dir != -1) {
			actor->direction = directions[dir];
			_awayMission.crewDirectionsAfterWalk[actorIndex] = -1;
		}
	}
	// end of sub_239d2
	////////////////////

	Common::String animName;
	if (actor->direction != 0)
		animName = actor->animationString + (char)actor->direction;
	else // Default to facing south
		animName = actor->animationString + 's';

	Fixed8 scale = getActorScaleAtPosition(actor->pos.y);
	loadActorAnim(actorIndex, animName, actor->pos.x, actor->pos.y, scale);
	actor->animType = 0;
}

void StarTrekEngine::updateActorPositionWhileWalking(Actor *actor, int16 x, int16 y) {
	actor->scale = getActorScaleAtPosition(y);
	Common::String animName = Common::String::format("%s%02d", actor->animationString2.c_str(), actor->field92 & 7);
	actor->sprite.setBitmap(loadAnimationFrame(animName, actor->scale));
	actor->bitmapFilename = animName;

	Sprite *sprite = &actor->sprite;
	sprite->drawPriority = _gfx->getPriValue(0, y);
	sprite->pos.x = x;
	sprite->pos.y = y;
	sprite->bitmapChanged = true;

	actor->frameToStartNextAnim = _frameIndex;
	actor->pos.x = x;
	actor->pos.y = y;
}

void StarTrekEngine::chooseActorDirectionForWalking(Actor *actor, int16 srcX, int16 srcY, int16 destX, int16 destY) {
	actor->granularPosX = srcX;
	actor->granularPosY = srcY;

	int16 distX = destX - srcX;
	int16 distY = destY - srcY;
	int16 absDistX = abs(distX);
	int16 absDistY = abs(distY);

	if (absDistX > absDistY) {
		char d;
		if (distX > 0)
			d = 'E';
		else
			d = 'W';

		// Append direction to animation string
		actor->animationString2 += d;

		actor->direction = d;
		actor->field90 = absDistX;

		if (distX != 0) {
			if (distX > 0)
				actor->speedX = 1.0;
			else
				actor->speedX = -1.0;

			actor->speedY = Fixed16(distY) / absDistX;
		}
	} else {
		char d;
		if (distY > 0)
			d = 'S';
		else
			d = 'N';

		// Append direction to animation string
		actor->animationString2 += d;

		actor->direction = d;
		actor->field90 = absDistY;

		if (distY != 0) {
			if (distY > 0)
				actor->speedY = 1.0;
			else
				actor->speedY = -1.0;

			actor->speedX = Fixed16(distX) / absDistY;
		}
	}
}

bool StarTrekEngine::directPathExists(int16 srcX, int16 srcY, int16 destX, int16 destY) {
	int32 distX = destX - srcX;
	int32 distY = destY - srcY;

	int32 absDistX = abs(distX);
	int32 absDistY = abs(distY);

	int32 distCounter;
	Fixed16 speedX, speedY;

	if (absDistX > absDistY) {
		distCounter = absDistX;

		if (distCounter == 0)
			return true;

		speedY = Fixed16(distY) / absDistX;

		if (distX > 0)
			speedX = 1.0;
		else
			speedX = -1.0;
	} else { // absDistX <= absDistY
		distCounter = absDistY;

		if (distCounter == 0)
			return true;

		speedX = Fixed16(distX) / absDistY;

		if (distY > 0)
			speedY = 1.0;
		else
			speedY = -1.0;
	}

	Fixed16 fixedX = srcX;
	Fixed16 fixedY = srcY;

	if (isPositionSolid((fixedX + 0.5).toInt(), (fixedY + 0.5).toInt()))
		return false;

	while (distCounter-- > 0) {
		fixedX += speedX;
		fixedY += speedY;

		if (isPositionSolid((fixedX + 0.5).toInt(), (fixedY + 0.5).toInt()))
			return false;
	}

	return true;
}

int StarTrekEngine::findObjectAt(int x, int y) {
	Sprite *sprite = _gfx->getSpriteAt(x, y);

	if (sprite != nullptr) {
		if (sprite == &_inventoryIconSprite)
			return OBJECT_INVENTORY_ICON;
		else if (sprite == &_itemIconSprite)
			return _awayMission.activeObject;

		for (int i = 0; i < NUM_ACTORS; i++) {
			Actor *actor = &_actorList[i];
			if (sprite == &actor->sprite)
				return i;
		}

		error("findObject: Clicked on an unknown sprite");
	}

	_objectHasWalkPosition = false;
	int actionBit = 1 << (_awayMission.activeAction - 1);
	int offset = _room->getFirstHotspot();

	while (offset != _room->getHotspotEnd()) {
		uint16 word = _room->readRdfWord(offset);
		if (word & 0x8000) {
			if ((word & actionBit) && _room->isPointInPolygon(offset + 6, x, y)) {
				int actorIndex = _room->readRdfWord(offset + 6);
				_objectHasWalkPosition = true;
				_objectWalkPosition.x = _room->readRdfWord(offset + 2);
				_objectWalkPosition.y = _room->readRdfWord(offset + 4);
				return actorIndex;
			}

			int numVertices = _room->readRdfWord(offset + 8);
			offset = offset + 10 + numVertices * 4;
		} else {
			if (_room->isPointInPolygon(offset, x, y)) {
				int actorIndex = _room->readRdfWord(offset);
				return actorIndex;
			}

			int numVertices = _room->readRdfWord(offset + 2);
			offset = offset + 4 + numVertices * 4;
		}
	}

	return -1;
}

Bitmap *StarTrekEngine::loadAnimationFrame(const Common::String &filename, Fixed8 scale) {
	Bitmap *bitmapToReturn = nullptr;
	bool isDemo = getFeatures() & GF_DEMO;

	char basename[5];
	strncpy(basename, filename.c_str() + 1, 4);
	basename[4] = '\0';

	char mcCoyChar = !isDemo ? 'm' : 'b';

	char c = filename[0];
	if ((strcmp(basename, "stnd") == 0 || strcmp(basename, "tele") == 0)
	        && (c == mcCoyChar || c == 's' || c == 'k' || c == 'r')) {
		if (c == mcCoyChar) {
			// Mccoy has the "base" animations for all crewmen
			bitmapToReturn = new Bitmap(_resource->loadBitmapFile(filename));
		} else {
			// All crewman other than mccoy copy the animation frames from mccoy, change
			// the colors of the uniforms, and load an "xor" file to redraw the face.

			// TODO: The ".$bm" extension is a "virtual file"? Caches the changes to the
			// file made here?
			// bitmapToReturn = new Bitmap(loadBitmapFile(filename + ".$bm"));

			if (bitmapToReturn == nullptr) {
				Common::String mccoyFilename = filename;
				mccoyFilename.setChar(mcCoyChar, 0);
				if (isDemo && mccoyFilename.hasPrefix("bstnds"))
					mccoyFilename.setChar('m', 0);
				Bitmap *bitmap = new Bitmap(_resource->loadBitmapFile(mccoyFilename));

				uint16 width = bitmap->width;
				uint16 height = bitmap->height;

				bitmapToReturn = new Bitmap(width, height);
				bitmapToReturn->xoffset = bitmap->xoffset;
				bitmapToReturn->yoffset = bitmap->yoffset;

				// Change uniform color
				int16 colorShift;
				switch (c) {
				case 'k': // Kirk
					colorShift = 8;
					break;
				case 'r': // Redshirt
					colorShift = -8;
					break;
				case 's': // Spock
					colorShift = 0;
					break;
				case 'm': // McCoy
				case 'b': // McCoy (demo)
					colorShift = 0;
					break;
				default:
					colorShift = 0;
					break;
				}

				if (colorShift == 0) {
					memcpy(bitmapToReturn->pixels, bitmap->pixels, width * height);
				} else {
					byte *src = bitmap->pixels;
					byte *dest = bitmapToReturn->pixels;
					byte baseUniformColor = 0xa8;

					for (int i = 0; i < width * height; i++) {
						byte b = *src++;
						if (b >= baseUniformColor && b < baseUniformColor + 8)
							*dest++ = b + colorShift;
						else
							*dest++ = b;
					}
				}

				// Redraw face with XOR file
				if (!isDemo) {
					Common::MemoryReadStreamEndian *xorFile = _resource->loadFile(filename + ".xor");
					xorFile->seek(0, SEEK_SET);
					uint16 xoffset = bitmap->xoffset - xorFile->readUint16();
					uint16 yoffset = bitmap->yoffset - xorFile->readUint16();
					uint16 xorWidth = xorFile->readUint16();
					uint16 xorHeight = xorFile->readUint16();

					byte *dest = bitmapToReturn->pixels + yoffset * bitmap->width + xoffset;

					for (int i = 0; i < xorHeight; i++) {
						for (int j = 0; j < xorWidth; j++)
							*dest++ ^= xorFile->readByte();
						dest += (bitmap->width - xorWidth);
					}

					delete xorFile;
				}

				delete bitmap;
			}
		}
	} else {
		// TODO: when loading a bitmap, it passes a different argument than is standard to
		// the "file loading with cache" function...
		bitmapToReturn = new Bitmap(_resource->loadBitmapFile(filename));
	}

	if (scale != 1.0) {
		bitmapToReturn = scaleBitmap(bitmapToReturn, scale);
	}

	return bitmapToReturn;
}


int StarTrekEngine::selectObjectForUseAction() {
	while (true) {
		if (!(_awayMission.crewDownBitset & (1 << OBJECT_KIRK)))
			showInventoryIcons(false);

		TrekEvent event;

		while (true) {
			if (!getNextEvent(&event))
				continue;

			if (event.type == TREKEVENT_TICK) {
				updateMouseBitmap();
				_gfx->drawAllSprites();
				_sound->checkLoopMusic();
			} else if (event.type == TREKEVENT_LBUTTONDOWN) {
				removeNextEvent();
				break;
			} else if (event.type == TREKEVENT_MOUSEMOVE) {
			} else if (event.type == TREKEVENT_RBUTTONDOWN) {
				// Allow this to be processed by main away mission loop
				break;
			} else if (event.type == TREKEVENT_KEYDOWN) {
				if (event.kbd.keycode == Common::KEYCODE_ESCAPE
				        || event.kbd.keycode == Common::KEYCODE_w
				        || event.kbd.keycode == Common::KEYCODE_t
				        || event.kbd.keycode == Common::KEYCODE_u
				        || event.kbd.keycode == Common::KEYCODE_g
				        || event.kbd.keycode == Common::KEYCODE_l
				        || event.kbd.keycode == Common::KEYCODE_SPACE
				        || event.kbd.keycode == Common::KEYCODE_F2) {
					// Allow these buttons to be processed by main away mission loop
					break;
				} else if (event.kbd.keycode == Common::KEYCODE_i) {
					removeNextEvent();
					break;
				} else if (event.kbd.keycode == Common::KEYCODE_RETURN || event.kbd.keycode == Common::KEYCODE_KP_ENTER || event.kbd.keycode == Common::KEYCODE_F1) {
					// Simulate left-click
					removeNextEvent();
					event.type = TREKEVENT_LBUTTONDOWN;
					break;
				}
			}

			removeNextEvent();
		}

		if (event.type == TREKEVENT_KEYDOWN && event.kbd.keycode == Common::KEYCODE_i) {
			hideInventoryIcons();
			int clickedObject = showInventoryMenu(50, 50, true);
			if (clickedObject == -1)
				continue;
			return clickedObject;
		} else if (event.type == TREKEVENT_LBUTTONDOWN) {
			int clickedObject = findObjectAt(_gfx->getMousePos());
			hideInventoryIcons();

			if (clickedObject == -1)
				continue;
			else if (isObjectUnusable(clickedObject, ACTION_USE))
				continue;
			else if (clickedObject == OBJECT_INVENTORY_ICON) {
				clickedObject = showInventoryMenu(50, 50, false);
				if (clickedObject == -1)
					continue;
				else
					return clickedObject;
			} else if (clickedObject <= OBJECT_REDSHIRT)
				return clickedObject;
			else if (isObjectUnusable(OBJECT_KIRK, ACTION_USE))
				continue;
			else if (_room->actionHasCode(ACTION_USE, OBJECT_KIRK, clickedObject, 0)
			         || _room->actionHasCode(ACTION_GET, clickedObject, 0, 0)
			         || _room->actionHasCode(ACTION_WALK, clickedObject, 0, 0)) {
				_awayMission.activeObject = OBJECT_KIRK;
				_awayMission.passiveObject = clickedObject;
				_awayMission.activeAction = ACTION_USE;
				clickedObject = OBJECT_KIRK;
				if (!walkActiveObjectToHotspot())
					addAction(_awayMission.activeAction, _awayMission.activeObject, _awayMission.passiveObject, 0);
				return clickedObject;
			} else
				continue;
		} else {
			hideInventoryIcons();
			return -1;
		}
	}
}

Common::String StarTrekEngine::getCrewmanAnimFilename(int actorIndex, const Common::String &basename) {
	bool isDemo = getFeatures() & GF_DEMO;
	const char *crewmanChars = !isDemo ? "ksmr" : "ksbr";	// Kirk, Spock, McCoy (Bones), RedShirt
	assert(actorIndex >= 0 && actorIndex < 4);
	return crewmanChars[actorIndex] + basename;
}

void StarTrekEngine::updateMouseBitmap() {
	const bool worksOnCrewmen[] = { // True if the action reacts with crewmen
		false, // ACTION_WALK
		true,  // ACTION_USE
		false, // ACTION_GET
		true,  // ACTION_LOOK
		true   // ACTION_TALK
	};
	const bool worksOnActors[] = { // True if the action reacts with other objects
		false, // ACTION_WALK
		true,  // ACTION_USE
		true,  // ACTION_GET
		true,  // ACTION_LOOK
		true   // ACTION_TALK
	};
	const bool worksOnHotspots[] = { // True if the action reacts with hotspots
		false, // ACTION_WALK
		true,  // ACTION_USE
		true,  // ACTION_GET
		true,  // ACTION_LOOK
		false  // ACTION_TALK
	};

	Common::Point mousePos = _gfx->getMousePos();
	int selected = findObjectAt(mousePos.x, mousePos.y);
	int action = _awayMission.activeAction;
	assert(action >= 1 && action <= 5);

	bool withRedOutline;

	if (selected >= 0 && selected <= 3 && worksOnCrewmen[action - 1])
		withRedOutline = true;
	else if (selected > 3 && selected < NUM_ACTORS && worksOnActors[action - 1])
		withRedOutline = true;
	else if (selected >= NUM_ACTORS && selected < HOTSPOTS_END && worksOnHotspots[action - 1])
		withRedOutline = true;
	else
		withRedOutline = false;

	chooseMouseBitmapForAction(action, withRedOutline);
}

bool StarTrekEngine::walkActiveObjectToHotspot() {
	if (!_objectHasWalkPosition)
		return false;

	int objectIndex;
	if (_awayMission.activeAction != ACTION_USE)
		objectIndex = OBJECT_KIRK;
	else if (_awayMission.activeObject <= OBJECT_REDSHIRT)
		objectIndex = _awayMission.activeObject;
	else if (_awayMission.activeObject >= ITEMS_START && _awayMission.activeObject <= ITEMS_END) { // FIXME: "<= ITEMS_END" doesn't make sense?
		if (_awayMission.activeObject == OBJECT_ISTRICOR)
			objectIndex = OBJECT_SPOCK;
		else if (_awayMission.activeObject == OBJECT_IMTRICOR)
			objectIndex = OBJECT_MCCOY;
		else
			objectIndex = OBJECT_KIRK;
	} else // This is the original error message...
		error("Jay didn't think about pmcheck");

	byte finishedAnimActionParam = false;
	bool walk = false;

	if (_awayMission.activeAction == ACTION_WALK)
		walk = true;
	else {
		// If this action has code defined for it in this room, buffer the action to be
		// done after the object finished walking there.
		Action action = {static_cast<int8>(_awayMission.activeAction), _awayMission.activeObject, 0, 0};
		if (_awayMission.activeAction == ACTION_USE)
			action.b2 = _awayMission.passiveObject;

		if (_room->actionHasCode(action)) {
			for (int i = 0; i < MAX_BUFFERED_WALK_ACTIONS; i++) {
				if (!_actionOnWalkCompletionInUse[i]) {
					finishedAnimActionParam = i + 0xe0;
					_actionOnWalkCompletionInUse[i] = true;
					_actionOnWalkCompletion[i] = action;
					walk = true;
					break;
				}
			}
		}
	}

	if (walk) {
		Actor *actor = &_actorList[objectIndex];
		Common::String anim = getCrewmanAnimFilename(objectIndex, "walk");
		actorWalkToPosition(objectIndex, anim, actor->pos.x, actor->pos.y, _objectWalkPosition.x, _objectWalkPosition.y);
		if (finishedAnimActionParam != 0) {
			actor->triggerActionWhenAnimFinished = true;
			actor->finishedAnimActionParam = finishedAnimActionParam;
		}
		_objectHasWalkPosition = false;
		return true;
	} else {
		_objectHasWalkPosition = false;
		return false;
	}
}

void StarTrekEngine::showInventoryIcons(bool showItem) {
	const char *crewmanFilenames[] = {
		"ikirk",
		"ispock",
		"imccoy",
		"iredshir"
	};

	Common::String itemFilename;

	if (showItem) {
		int i = _awayMission.activeObject;
		if (i >= OBJECT_KIRK && i <= OBJECT_REDSHIRT)
			itemFilename = crewmanFilenames[i];
		else {
			assert(i >= ITEMS_START && i < ITEMS_END);
			Item *item = &_itemList[i - ITEMS_START];
			itemFilename = item->name;
		}
	}

	if (itemFilename.empty())
		_inventoryIconSprite.pos.x = 10;
	else {
		_gfx->addSprite(&_itemIconSprite);
		_itemIconSprite.drawMode = 2;
		_itemIconSprite.pos.x = 10;
		_itemIconSprite.pos.y = 10;
		_itemIconSprite.drawPriority = 15;
		_itemIconSprite.drawPriority2 = 8;
		_itemIconSprite.setBitmap(_resource->loadBitmapFile(itemFilename));

		_inventoryIconSprite.pos.x = 46;
	}

	_gfx->addSprite(&_inventoryIconSprite);

	_inventoryIconSprite.pos.y = 10;
	_inventoryIconSprite.drawMode = 2;
	_inventoryIconSprite.drawPriority = 15;
	_inventoryIconSprite.drawPriority2 = 8;
	_inventoryIconSprite.setBitmap(_resource->loadBitmapFile("inv00"));
}

bool StarTrekEngine::isObjectUnusable(int object, int action) {
	if (action == ACTION_LOOK)
		return false;
	if (object == OBJECT_REDSHIRT && _awayMission.redshirtDead)
		return true;
	if (object >= OBJECT_KIRK && object <= OBJECT_REDSHIRT && (_awayMission.crewDownBitset & (1 << object)))
		return true;
	if (object == OBJECT_IMTRICOR && (_awayMission.crewDownBitset & (1 << OBJECT_MCCOY)))
		return true;
	if (object == OBJECT_ISTRICOR && (_awayMission.crewDownBitset & (1 << OBJECT_SPOCK)))
		return true;
	return false;
}

void StarTrekEngine::hideInventoryIcons() {
	// Clear these sprites from the screen
	if (_itemIconSprite.drawMode == 2)
		_itemIconSprite.dontDrawNextFrame();
	if (_inventoryIconSprite.drawMode == 2)
		_inventoryIconSprite.dontDrawNextFrame();

	_gfx->drawAllSprites();

	if (_itemIconSprite.drawMode == 2) {
		_gfx->delSprite(&_itemIconSprite);
		_itemIconSprite.drawMode = 0;
		delete _itemIconSprite.bitmap;
		_itemIconSprite.bitmap = nullptr;
	}

	if (_inventoryIconSprite.drawMode == 2) {
		_gfx->delSprite(&_inventoryIconSprite);
		_inventoryIconSprite.drawMode = 0;
		delete _inventoryIconSprite.bitmap;
		_inventoryIconSprite.bitmap = nullptr;
	}
}

void StarTrekEngine::updateCrewmanGetupTimers() {
	if (_awayMission.crewDownBitset == 0)
		return;
	for (int i = OBJECT_KIRK; i <= OBJECT_REDSHIRT; i++) {
		Actor *actor = &_actorList[i];

		if (!(_awayMission.crewDownBitset & (1 << i)))
			continue;

		_awayMission.crewGetupTimers[i]--;
		if (_awayMission.crewGetupTimers[i] <= 0) {
			Common::String anim = getCrewmanAnimFilename(i, "getu");
			int8 dir = _awayMission.crewDirectionsAfterWalk[i];
			char d;
			if (dir == -1) {
				d = actor->direction;
			} else {
				const char *dirs = "nsew";
				Fixed8 scale = getActorScaleAtPosition(actor->sprite.pos.y);
				d = dirs[dir];

				int16 xOffset = 0, yOffset = 0;
				if (d == 'n') {
					xOffset = -24;
					yOffset = -8;
				} else if (d == 'w') {
					xOffset = -35;
					yOffset = -12;
				}
				actor->sprite.pos.x += scale.multToInt(xOffset);
				actor->sprite.pos.y += scale.multToInt(yOffset);
			}

			anim += (char)d;
			loadActorAnimWithRoomScaling(i, anim, actor->sprite.pos.x, actor->sprite.pos.y);
			_awayMission.crewDownBitset &= ~(1 << i);
		}
	}
}

int StarTrekEngine::showInventoryMenu(int x, int y, bool restoreMouse) {
	const int ITEMS_PER_ROW = 5;

	Common::Point oldMousePos = _gfx->getMousePos();
	bool keyboardControlledMouse = _keyboardControlsMouse;
	_keyboardControlsMouse = false;

	int itemIndex = 0;
	int numItems = 0;

	char itemNames[NUM_OBJECTS][10];
	Common::Point itemPositions[NUM_OBJECTS];
	int16 itemIndices[NUM_OBJECTS];

	while (itemIndex < NUM_OBJECTS) {
		if (_itemList[itemIndex].have) {
			strcpy(itemNames[numItems], _itemList[itemIndex].name);

			int16 itemX = (numItems % ITEMS_PER_ROW) * 32 + x;
			int16 itemY = (numItems / ITEMS_PER_ROW) * 32 + y;
			itemPositions[numItems] = Common::Point(itemX, itemY);
			itemIndices[numItems] = _itemList[itemIndex].field2;

			numItems++;
		}
		itemIndex++;
	}

	Sprite itemSprites[NUM_OBJECTS];

	for (int i = 0; i < numItems; i++) {
		_gfx->addSprite(&itemSprites[i]);

		itemSprites[i].drawMode = 2;
		itemSprites[i].pos.x = itemPositions[i].x;
		itemSprites[i].pos.y = itemPositions[i].y;
		itemSprites[i].drawPriority = 15;
		itemSprites[i].drawPriority2 = 8;
		itemSprites[i].setBitmap(_resource->loadBitmapFile(itemNames[i]));
	}

	chooseMousePositionFromSprites(itemSprites, numItems, -1, 4);
	bool displayMenu = true;
	int lastItemIndex = -1;

	while (displayMenu) {
		_sound->checkLoopMusic();

		TrekEvent event;
		if (!getNextEvent(&event))
			continue;

		switch (event.type) {
		case TREKEVENT_TICK: {
			Common::Point mousePos = _gfx->getMousePos();
			itemIndex = getMenuButtonAt(itemSprites, numItems, mousePos.x, mousePos.y);
			if (itemIndex != lastItemIndex) {
				if (lastItemIndex != -1) {
					drawMenuButtonOutline(itemSprites[lastItemIndex].bitmap, 0);
					itemSprites[lastItemIndex].bitmapChanged = true;
				}
				if (itemIndex != -1) {
					drawMenuButtonOutline(itemSprites[itemIndex].bitmap, 15);
					itemSprites[itemIndex].bitmapChanged = true;
				}
				lastItemIndex = itemIndex;
			}
			_gfx->drawAllSprites();
			break;
		}

		case TREKEVENT_LBUTTONDOWN:
			displayMenu = false;
			break;

		case TREKEVENT_RBUTTONDOWN:
			displayMenu = false;
			lastItemIndex = -1;
			break;

		case TREKEVENT_KEYDOWN:
			switch (event.kbd.keycode) {
			case Common::KEYCODE_ESCAPE:
			case Common::KEYCODE_F2:
				displayMenu = false;
				lastItemIndex = -1;
				break;

			case Common::KEYCODE_RETURN:
			case Common::KEYCODE_KP_ENTER:
			case Common::KEYCODE_F1:
				displayMenu = false;
				break;

			case Common::KEYCODE_HOME:
			case Common::KEYCODE_KP7:
				chooseMousePositionFromSprites(itemSprites, numItems, lastItemIndex, 4);
				break;

			case Common::KEYCODE_UP:
			case Common::KEYCODE_KP8:
			case Common::KEYCODE_PAGEUP:
			case Common::KEYCODE_KP9:
				chooseMousePositionFromSprites(itemSprites, numItems, lastItemIndex, 2);
				break;

			case Common::KEYCODE_LEFT:
			case Common::KEYCODE_KP4:
				chooseMousePositionFromSprites(itemSprites, numItems, lastItemIndex, 1);
				break;

			case Common::KEYCODE_RIGHT:
			case Common::KEYCODE_KP6:
				chooseMousePositionFromSprites(itemSprites, numItems, lastItemIndex, 0);
				break;

			case Common::KEYCODE_END:
			case Common::KEYCODE_KP1:
				chooseMousePositionFromSprites(itemSprites, numItems, lastItemIndex, 5);
				break;

			case Common::KEYCODE_DOWN:
			case Common::KEYCODE_KP2:
			case Common::KEYCODE_PAGEDOWN:
			case Common::KEYCODE_KP3:
				chooseMousePositionFromSprites(itemSprites, numItems, lastItemIndex, 3);
				break;

			default:
				break;
			}
			break;

		default:
			break;
		}

		removeNextEvent();
	}

	_sound->playSoundEffectIndex(0x10);
	if (lastItemIndex >= 0)
		drawMenuButtonOutline(itemSprites[lastItemIndex].bitmap, 0);

	for (int i = 0; i < numItems; i++)
		itemSprites[i].dontDrawNextFrame();

	_gfx->drawAllSprites();

	for (int i = 0; i < numItems; i++) {
		delete itemSprites[i].bitmap;
		itemSprites[i].bitmap = nullptr;
		_gfx->delSprite(&itemSprites[i]);
	}

	if (lastItemIndex >= 0) {
		lastItemIndex = itemIndices[lastItemIndex];
	}

	if (restoreMouse)
		_gfx->warpMouse(oldMousePos.x, oldMousePos.y);

	_keyboardControlsMouse = keyboardControlledMouse;
	return lastItemIndex;
}

void StarTrekEngine::initStarfieldSprite(Sprite *sprite, Bitmap *bitmap, const Common::Rect &rect) {
	sprite->setXYAndPriority(rect.left, rect.top, 0);
	sprite->setBitmap(bitmap);
	bitmap->xoffset = 0;
	bitmap->yoffset = 0;
	bitmap->width = rect.width();
	bitmap->height = rect.height();
	_gfx->addSprite(sprite);
	sprite->drawMode = 1;
}

Bitmap *StarTrekEngine::scaleBitmap(Bitmap *bitmap, Fixed8 scale) {
	int scaledWidth  = scale.multToInt(bitmap->width);
	int scaledHeight = scale.multToInt(bitmap->height);
	int origWidth  = bitmap->width;
	int origHeight = bitmap->height;

	if (scaledWidth < 1)
		scaledWidth = 1;
	if (scaledHeight < 1)
		scaledHeight = 1;

	Bitmap *scaledBitmap = new Bitmap(scaledWidth, scaledHeight);
	scaledBitmap->xoffset = scale.multToInt(bitmap->xoffset);
	scaledBitmap->yoffset = scale.multToInt(bitmap->yoffset);

	// sub_344a5(scaledWidth, origWidth);

	origHeight--;
	scaledHeight--;

	byte *src = bitmap->pixels;
	byte *dest = scaledBitmap->pixels;

	if (scale <= 1.0) {
		int16 var2e = 0;
		uint16 var30 = scaledHeight << 1;
		uint16 var32 = (scaledHeight - origHeight) << 1;
		uint16 origRow = 0;

		while (origRow <= origHeight) {
			if (var2e < 0) {
				var2e += var30;
			} else {
				var2e += var32;
				scaleBitmapRow(src, dest, origWidth, scaledWidth);
				dest += scaledWidth;
			}

			src += bitmap->width;
			origRow++;
		}
	} else {
		int16 var2e = (origHeight << 1) - scaledHeight;
		uint16 var30 = origHeight << 1;
		uint16 var32 = (origHeight - scaledHeight) << 1;
		uint16 srcRowChanged = true;
		origWidth = bitmap->width;
		uint16 scaledRow = 0;
		byte *rowData = new byte[scaledWidth];

		while (scaledRow++ <= scaledHeight) {
			if (srcRowChanged) {
				scaleBitmapRow(src, rowData, origWidth, scaledWidth);
				srcRowChanged = false;
			}

			memcpy(dest, rowData, scaledWidth);
			dest += scaledWidth;

			if (var2e < 0) {
				var2e += var30;
			} else {
				var2e += var32;
				src += origWidth;
				srcRowChanged = true;
			}
		}

		delete[] rowData;
	}

	delete bitmap;

	return scaledBitmap;
}

void StarTrekEngine::scaleBitmapRow(byte *src, byte *dest, uint16 origWidth, uint16 scaledWidth) {
	if (origWidth >= scaledWidth) {
		int16 var2 = (scaledWidth << 1) - origWidth;
		uint16 var4 = scaledWidth << 1;
		uint16 var6 = (scaledWidth - origWidth) << 1;
		uint16 varE = 0;
		uint16 varA = 0;
		uint16 var8 = origWidth;
		uint16 di = 0;

		while (var8-- != 0) {
			if (var2 < 0) {
				var2 += var4;
			} else {
				var2 += var6;
				if (di != 0) {
					if (varE != 0) {
						*(dest - 1) = *src++;
						varE = 0;
						di--;
					}
					src += di;
					di = 0;
				}
				*dest++ = *src;
				varE = 1;
			}

			di++;
			varA++;
		}
	} else {
		int16 var2 = ((origWidth - 1) << 1) - (scaledWidth - 1);
		uint16 var4 = (origWidth - 1) << 1;
		uint16 var6 = ((origWidth - 1) - (scaledWidth - 1)) << 1;
		uint16 varA = 0;
		uint16 var8 = scaledWidth;
		uint16 di = 0;

		while (var8-- != 0) {
			if (di != 0) {
				src += di;
				di = 0;
			}
			*dest++ = *src;

			if (var2 < 0)
				var2 += var4;
			else {
				var2 += var6;
				di++;
			}

			varA++;
		}
	}
}

} // End of namespace StarTrek
