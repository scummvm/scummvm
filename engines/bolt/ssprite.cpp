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

#include "bolt/bolt.h"

namespace Bolt {

void BoltEngine::setUpSSprite(SSprite *sprite, int16 frameCount, byte **frameData, int16 frameRate, int16 velocityX, int16 velocityY) {
	sprite->frameCount = frameCount;
	sprite->frameData = frameData;
	sprite->frameRate = frameRate;
	sprite->frameTimer = frameRate;
	sprite->velocityX = velocityX;
	sprite->velocityY = velocityY;

	// Read collision bounds from first frame's pic descriptor
	byte *firstFrame = sprite->frameData[0];
	sprite->collX = READ_UINT16(firstFrame + 6);
	sprite->collY = READ_UINT16(firstFrame + 8);
	sprite->collW = READ_UINT16(firstFrame + 0x0A);
	sprite->collH = READ_UINT16(firstFrame + 0x0C);

	sprite->xLimitHigh = 0;
	sprite->xLimitLow = 0;
	sprite->yLimitHigh = 0;
	sprite->yLimitLow = 0;
}

void BoltEngine::animateSSprite(SSprite *sprite, int16 page) {
	if (!(sprite->flags & 0x01))
		return;

	// Frozen: call update func or display and return
	if (sprite->flags & 0x80) {
		if (sprite->updateFunc) {
			sprite->updateFunc();
			return;
		}

		_xp->fillDisplay(0, stFront);
		byte *frame = sprite->frameData[sprite->currentFrame];
		displayPic(frame, sprite->x, sprite->y, page);
		return;
	}

	// Advance frame timer
	sprite->frameTimer--;
	if (sprite->frameTimer == 0) {
		sprite->frameTimer = sprite->frameRate;
		sprite->currentFrame++;
		if (sprite->currentFrame == sprite->frameCount)
			sprite->currentFrame = 0;
	}

	// Movement: path or velocity
	if (sprite->flags & 0x08) {
		// Path mode
		if (sprite->pathOffset >= sprite->pathLength) {
			// Path complete
			sprite->flags &= 0xFFF6; // clear alive + path bits
			if (sprite->pathCallback) {
				sprite->pathCallback(sprite);
			}
		} else {
			// Read next x,y from path data
			int16 idx = sprite->pathOffset;
			sprite->pathOffset++;
			int16 *pathWords = (int16 *)sprite->pathData;
			sprite->x = pathWords[idx];

			idx = sprite->pathOffset;
			sprite->pathOffset++;
			sprite->y = pathWords[idx];
		}
	} else {
		// Velocity mode
		int16 vx = sprite->velocityX;
		int16 vy = sprite->velocityY;

		sprite->x += vx;
		sprite->y += vy;

		sprite->velocityX += sprite->accelX;
		sprite->velocityY += sprite->accelY;
	}

	// X limit clamping
	if (sprite->flags & 0x20) {
		if (sprite->x < sprite->xLimitHigh) {
			sprite->x = sprite->xLimitHigh;
			sprite->accelX = 0;
			sprite->velocityX = 0;
		} else if (sprite->x > sprite->xLimitLow) {
			sprite->x = sprite->xLimitLow;
			sprite->accelX = 0;
			sprite->velocityX = 0;
		}
	}

	// Y limit clamping
	if (sprite->flags & 0x40) {
		if (sprite->y < sprite->yLimitHigh) {
			sprite->y = sprite->yLimitHigh;
			sprite->accelY = 0;
			sprite->velocityY = 0;
		} else if (sprite->y > sprite->yLimitLow) {
			sprite->y = sprite->yLimitLow;
			sprite->accelY = 0;
			sprite->velocityY = 0;
		}
	}

	// Call update func or display
	if (sprite->updateFunc) {
		sprite->updateFunc();
	} else {
		_xp->fillDisplay(0, stFront);
		byte *frame = sprite->frameData[sprite->currentFrame];
		displayPic(frame, sprite->x, sprite->y, page);
	}

	// Stop countdown
	if (sprite->flags & 0x04) {
		if (sprite->stopFrame != 0) {
			sprite->stopFrame--;
			return;
		}

		sprite->flags &= ~0x01; // not alive
		sprite->flags &= ~0x04; // no stop
		return;
	}

	// Start countdown
	if (sprite->flags & 0x02) {
		if (sprite->startFrame != 0) {
			sprite->startFrame--;
			return;
		}

		sprite->flags |= 0x01;  // alive
		sprite->flags &= ~0x02; // no longer starting
	}
}

void BoltEngine::displaySSprite(SSprite *sprite, int16 x, int16 y) {
	sprite->x = x;
	sprite->y = y;
	sprite->flags |= 0x01; // alive
}

void BoltEngine::eraseSSprite(SSprite *sprite) {
	sprite->flags &= ~0x01; // not alive
}

void BoltEngine::setSSpriteFrames(SSprite *sprite, int16 frameCount, byte **frameData, int16 frameRate) {
	sprite->frameCount = frameCount;
	sprite->frameData = frameData;
	sprite->currentFrame = 0;
	sprite->frameRate = frameRate;
}

void BoltEngine::setSSpriteDrag(SSprite *sprite, int16 dragX, int16 dragY) {
	sprite->dragX = dragX;
	sprite->dragY = dragY;
}

void BoltEngine::setSSpriteAccel(SSprite *sprite, int16 accelX, int16 accelY) {
	sprite->accelX = accelX;
	sprite->accelY = accelY;
}

void BoltEngine::reverseSSpriteAccel(SSprite *sprite) {
	sprite->accelX = -sprite->accelX;
	sprite->accelY = -sprite->accelY;
}

void BoltEngine::addSSpriteAccel(SSprite *sprite, int16 dx, int16 dy) {
	sprite->accelX += dx;
	sprite->accelY += dy;
}

void BoltEngine::setSSpriteVelocity(SSprite *sprite, int16 vx, int16 vy) {
	sprite->velocityX = vx;
	sprite->velocityY = vy;
}

void BoltEngine::reverseSSpriteVelocity(SSprite *sprite) {
	sprite->velocityX = -sprite->velocityX;
	sprite->velocityY = -sprite->velocityY;
}

void BoltEngine::setSSpriteStart(SSprite *sprite, int16 startFrame, int16 x, int16 y) {
	sprite->startFrame = startFrame;
	sprite->x = x;
	sprite->y = y;
	sprite->flags |= 0x02;  // started
	sprite->flags &= ~0x01; // not alive
	sprite->flags &= ~0x04; // no stop
}

void BoltEngine::setSSpriteStop(SSprite *sprite, int16 stopFrame) {
	sprite->stopFrame = stopFrame;
	sprite->flags |= 0x04; // hasStop
}

void BoltEngine::setSSpritePath(SSprite *sprite, byte *pathData, int16 pathCount, SSpritePathCallback callback) {
	sprite->pathLength = pathCount * 2;
	sprite->pathData = pathData;
	sprite->pathOffset = 0;
	sprite->pathCallback = callback;
	sprite->flags |= 0x09; // alive + hasPath
}

bool BoltEngine::inSSprite(SSprite *sprite, int16 x, int16 y) {
	_spriteCollTempX = sprite->collX;
	_spriteCollTempY = sprite->collY;
	_spriteCollTempW = sprite->collW;
	_spriteCollTempH = sprite->collH;

	_spriteScreenX = sprite->x + _spriteCollTempX;
	_spriteScreenY = sprite->y + _spriteCollTempY;

	if ((uint16)x < (uint16)_spriteScreenX)
		return false;

	if ((uint16)y < (uint16)_spriteScreenY)
		return false;

	if ((uint16)(_spriteScreenX + sprite->collW) < (uint16)x)
		return false;

	if ((uint16)(_spriteScreenY + sprite->collH) < (uint16)y)
		return false;

	return true;
}

bool BoltEngine::sSpriteCollide(SSprite *spriteA, SSprite *spriteB) {
	_spriteCollTempA[0] = spriteA->collX;
	_spriteCollTempA[1] = spriteA->collY;
	_spriteCollTempA[2] = spriteA->collW;
	_spriteCollTempA[3] = spriteA->collH;

	_spriteCollTempB[0] = spriteB->collX;
	_spriteCollTempB[1] = spriteB->collY;
	_spriteCollTempB[2] = spriteB->collW;
	_spriteCollTempB[3] = spriteB->collH;

	_spriteScreenAX = spriteA->x + _spriteCollTempA[0];
	_spriteScreenBX = spriteB->x + _spriteCollTempB[0];
	_spriteScreenAY = spriteA->y + _spriteCollTempA[1];
	_spriteScreenBY = spriteB->y + _spriteCollTempB[1];

	if ((uint16)(_spriteScreenAX + _spriteCollTempA[2]) < (uint16)_spriteScreenBX)
		return false;
	if ((uint16)(_spriteScreenAY + _spriteCollTempA[3]) < (uint16)_spriteScreenBY)
		return false;
	if ((uint16)(_spriteScreenBX + _spriteCollTempB[2]) < (uint16)_spriteScreenAX)
		return false;
	if ((uint16)(_spriteScreenBY + _spriteCollTempB[3]) < (uint16)_spriteScreenAY)
		return false;

	return true;
}

void BoltEngine::setSSpriteCollision(SSprite *sprite, int16 *bounds) {
	// bounds is a 4-word array: [offsetX, offsetY, width, height]
	byte *firstFrame = sprite->frameData[0];

	sprite->collX = READ_UINT16(firstFrame + 6) + bounds[0];
	sprite->collY = bounds[1] + READ_UINT16(firstFrame + 8);
	sprite->collW = bounds[2];
	sprite->collH = bounds[3];
}

bool BoltEngine::sSpriteAlive(SSprite *sprite) {
	return (sprite->flags & 0x01) != 0;
}

void BoltEngine::getSSpriteLoc(SSprite *sprite, Common::Point *out) {
	(*out).x = sprite->x;
	(*out).y = sprite->y;
}

void BoltEngine::getSSpriteAccel(SSprite *sprite, int16 *out) {
	out[0] = sprite->accelX;
	out[1] = sprite->accelY;
}

void BoltEngine::getSSpriteVelocity(SSprite *sprite, int16 *out) {
	out[0] = sprite->velocityX;
	out[1] = sprite->velocityY;
}

void BoltEngine::getSSpriteDrag(SSprite *sprite, int16 *out) {
	out[0] = sprite->dragX;
	out[1] = sprite->dragY;
}

void BoltEngine::setSSpriteXLimit(SSprite *sprite, int16 high, int16 low) {
	sprite->xLimitHigh = high;
	sprite->xLimitLow = low;
	sprite->flags |= 0x20;
}

void BoltEngine::setSSpriteYLimit(SSprite *sprite, int16 high, int16 low) {
	sprite->yLimitHigh = high;
	sprite->yLimitLow = low;
	sprite->flags |= 0x40;
}

void BoltEngine::setSSpriteInfo(SSprite *sprite, int16 info) {
	sprite->userInfo = info;
}

int16 BoltEngine::getSSpriteInfo(SSprite *sprite) {
	return sprite->userInfo;
}

void BoltEngine::freezeSSprite(SSprite *sprite) {
	sprite->flags |= 0x80;
}

void BoltEngine::unfreezeSSprite(SSprite *sprite) {
	sprite->flags &= 0x0F7F;
}

} // End of namespace Bolt
