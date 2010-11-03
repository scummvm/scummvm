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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "asylum/resources/barrier.h"

#include "asylum/resources/actor.h"

#include "asylum/views/scene.h"

namespace Asylum {

Barrier::Barrier(AsylumEngine *engine) : _vm(engine) {
}

Barrier::~Barrier() {
}

/////////////////////////////////////////////////////////////////////////
// Loading & destroying
/////////////////////////////////////////////////////////////////////////
void Barrier::load(Common::SeekableReadStream *stream) {
	_id	  = stream->readSint32LE();
	_resourceId = stream->readSint32LE();
	x	  = stream->readSint32LE();
	y	  = stream->readSint32LE();

	_boundingRect.left	= stream->readSint32LE() & 0xFFFF;
	_boundingRect.top	= stream->readSint32LE() & 0xFFFF;
	_boundingRect.right	= stream->readSint32LE() & 0xFFFF;
	_boundingRect.bottom = stream->readSint32LE() & 0xFFFF;

	_field_20   = stream->readSint32LE();
	_frameIndex = stream->readSint32LE();
	_frameCount = stream->readSint32LE();
	_field_2C   = stream->readSint32LE();
	_field_30   = stream->readSint32LE();
	_field_34   = stream->readSint32LE();
	flags       = stream->readSint32LE();
	_field_3C   = stream->readSint32LE();

	stream->read(_name, sizeof(_name));

	_rect.left    = stream->readSint32LE();
	_rect.top     = stream->readSint32LE();
	_rect.right   = stream->readSint32LE();
	_rect.bottom  = stream->readSint32LE();
	_polygonIndex = stream->readSint32LE();
	actionType    = stream->readSint32LE();

	for (int i = 0; i < 10; i++)
		_gameFlags[i] = (GameFlag)stream->readSint32LE();

	_field_B4	  = stream->readSint32LE();
	_tickCount	  = stream->readSint32LE();
	_tickCount2	  = stream->readSint32LE();
	_field_C0	  = stream->readSint32LE();
	_priority	  = stream->readSint32LE();
	_actionListIdx = stream->readSint32LE();

	for (int i = 0; i < 16; i++) {
		_soundItems[i].resourceId	  = stream->readSint32LE();
		_soundItems[i].field_4 = stream->readSint32LE();
		_soundItems[i].field_8 = stream->readSint32LE();
		_soundItems[i].field_C = stream->readSint32LE();

	}

	for (int i = 0; i < 50; i++) {
		_frameSoundItems[i].resourceId	= stream->readSint32LE();
		_frameSoundItems[i].frameIdx = stream->readSint32LE();
		_frameSoundItems[i].index	= stream->readSint32LE();
		_frameSoundItems[i].field_C	= stream->readSint32LE();
		_frameSoundItems[i].field_10 = stream->readSint32LE();
		_frameSoundItems[i].field_14 = stream->readSint32LE();
	}

	_field_67C = stream->readSint32LE();
	_soundX	  = stream->readSint32LE();
	_soundY	  = stream->readSint32LE();
	_field_688 = stream->readSint32LE();

	for (int i = 0; i < 5; i++)
		_field_68C[i] = stream->readSint32LE();

	_soundResourceId = stream->readSint32LE();
	_field_6A4       = stream->readSint32LE();
}


void Barrier::disable() {
	flags &= ~kBarrierFlagEnabled;
}

void Barrier::disableAndRemoveFromQueue() {
	disable();

	flags |= kBarrierFlag20000;

	getScreen()->deleteGraphicFromQueue(_resourceId);
}

/////////////////////////////////////////////////////////////////////////
// Visibility
//////////////////////////////////////////////////////////////////////////
bool Barrier::isOnScreen() {
	Common::Rect screenRect  = Common::Rect(getWorld()->xLeft, getWorld()->yTop, getWorld()->xLeft + 640, getWorld()->yTop + 480);
	Common::Rect barrierRect = Common::Rect(_boundingRect);

	barrierRect.translate(x, y);

	return isVisible() && (flags & kBarrierFlagEnabled) && screenRect.intersects(barrierRect);
}

bool Barrier::isVisible() {
	if (flags & kBarrierFlagEnabled) {

		// Check each game flag
		for (int32 i = 0; i < 10; i++) {
			GameFlag flag = _gameFlags[i];
			bool ok = false;

			if (flag <= 0)
				ok = _vm->isGameFlagNotSet((GameFlag)-flag);
			else
				ok = _vm->isGameFlagSet(flag);

			if (!ok)
				return false;
		}

		// All flags were ok, we are done!
		return true;
	}

	return false;
}

/////////////////////////////////////////////////////////////////////////
// Update
//////////////////////////////////////////////////////////////////////////
void Barrier::draw() {
	if (LOBYTE(flags) & kBarrierFlag4)
		return;

	if (BYTE1(flags) & kBarrierFlag40)
		return;

	if (!isOnScreen())
		return;

	// Draw the barrier
	Common::Point point;
	getScene()->adjustCoordinates(x, y, &point);

	if (_field_67C <= 0 || _field_67C >= 4 || Config.performance <= 1)
		getScreen()->addGraphicToQueue(_resourceId, _frameIndex, x, y, (flags >> 11) & kBarrierFlag2, _field_67C - 3, _priority);
	else {
		// TODO: Do Cross Fade
		//getScreen()->addGraphicToQueue(_resourceId, _frameIndex, x, y, getWorld()->backgroundImage, getWorld()->xLeft, getWorld()->yTop, 0, 0, _field_67C - 1);
		error("[Barrier::draw] Crossfade not implemented!");
	}
}



void Barrier::update() {
	bool canPlaySound = false;
	if (_field_3C == 4) {
		if (isVisible()) {
			int32 flag = flags;
			if (flag & 0x20) {
				if (_vm->getTick() - _tickCount >= 0x3E8 / _field_B4) {
					_frameIndex =((_frameIndex + 1) % _frameCount);
					_tickCount = _vm->getTick();
					canPlaySound       = true;
				}
			} else if (flag & 0x10) {
				uint32 frameIdx  = _frameIndex;
				int equalZero = frameIdx == 0;
				if (!frameIdx) {
					if (_vm->getTick() - _tickCount >= 1000 * _tickCount2) {
						if (_vm->getRandom(_field_C0) == 1) {
							if (_field_68C[0]) {
								// TODO: fix this, and find a better way to get frame count
								// Sometimes we get wrong random resource id

								_resourceId = getRandomId();
								GraphicResource *gra = new GraphicResource(getScene()->getResourcePack(), _resourceId);
								_frameCount  = gra->getFrameCount();
								delete gra;
							}
							_frameIndex++;
						}
						_tickCount = _vm->getTick();
						canPlaySound       = true;
					}
					frameIdx  = _frameIndex;
					equalZero = frameIdx == 0;
				}

				if (!equalZero) {
					if (_vm->getTick() - _tickCount >= 0x3E8 / _field_B4) {
						_frameIndex  = (_frameIndex + 1) % _frameCount;
						_tickCount = _vm->getTick();
						canPlaySound = true;
					}
				}
			} else if (flag & 8) {
				if (_vm->getTick() - _tickCount >= 0x3E8 / _field_B4) {
					uint32 frameIdx = _frameIndex + 1;
					if (frameIdx < _frameCount - 1) {
						if (_field_688 == 1) {
							// TODO: get global x, y positions
						}
					} else {
						flags &= ~kBarrierFlag8;
						if (_field_688 == 1) {
							// TODO: reset global x, y positions
						}
					}
					_frameIndex = frameIdx;
				}
			} else if ((flag & 0xFF) & 8) { // check this
				if (_vm->getTick() - _tickCount >= 1000 * _tickCount2) {
					if (_vm->getRandom(_field_C0) == 1) { // TODO: THIS ISNT WORKING
						_frameIndex  = (_frameIndex + 1) % _frameCount;
						_tickCount = _vm->getTick();
						canPlaySound = true;
					}
				}
			} else if (!((flag & 0xFFFF) & 6)) {
				if (_vm->getTick() - _tickCount >= 0x3E8 / _field_B4 && (flag & 0x10000)) {
					uint32 frameIdx = _frameIndex - 1;
					if (frameIdx <= 0) {
						flags &= ~kBarrierFlag10000;
						if (_field_688 == 1) {
							// TODO: reset global x, y positions
						}
						_tickCount = _vm->getTick();
						canPlaySound = true;
					}
					if (_field_688 == 1) {
						// TODO: get global x, y positions
					}
					_frameIndex = frameIdx;
				} else if (_vm->getTick() - _tickCount >= 0x3E8 / _field_B4) {
					if ((flag & 0xFF) & 2) {
						if (_frameIndex == _frameCount - 1) {
							_frameIndex--;
							flags = ((flag & 0xFF) & 0xFD) | 4;
						} else {
							_frameIndex++;
						}
					} else if ((flag & 0xFF) & 4) {
						if (_frameIndex) {
							_frameIndex--;
						} else {
							_frameIndex++;
							flags = ((flag & 0xFF) & 0xFB) | 2;
						}
					}
				}
			}

			flag = flags;
			flag &= 0x40000;
			if (flag != 0) {
				if (_frameIndex == _frameCount - 1) {
					if (_field_B4 <= 15) {
						_field_B4 -= 2;
						if (_field_B4 < 0)
							_field_B4 = 0;
					} else {
						_field_B4 = 15;
					}
					if (!_field_B4)
						flags &= 0xFFFEF1C7;
				}
			}
		}

		if (canPlaySound) {
			updateSoundItems();
			stopSound();
		}
	}
	// TODO: get sound functions according with scene
}

void Barrier::setNextFrame(int32 targetFlags) {
	flags |= targetFlags | kBarrierFlagEnabled;

	if (flags & kBarrierFlag10000)
		_frameIndex = _frameCount - 1;
	else
		_frameIndex = 0;
}

/////////////////////////////////////////////////////////////////////////
// Misc
/////////////////////////////////////////////////////////////////////////
void Barrier::updateSoundItems() {
	for (int32 i = 0; i < ARRAYSIZE(_soundItems); i++) {

		SoundItem *item = &_soundItems[i];

		if (getSound()->isPlaying(item->resourceId)) {
			if (item->field_4) {
				getSound()->stopSound(item->resourceId);
				item->resourceId = kResourceNone;
				item->field_4 = 0;
			}
		}
	}

	stopSound();
}

void Barrier::stopSound() {
	if (getSound()->isPlaying(_soundResourceId))
		getSound()->stopSound(_soundResourceId);
}

void Barrier::stopAllSounds() {
	for (int i = 0; i < ARRAYSIZE(_soundItems); i++)
		if (_soundItems[i].resourceId) {
			getSound()->stopSound(_soundItems[i].resourceId);
			_soundItems[i].resourceId = kResourceNone;
		}
}

int32 Barrier::getRandomId() {
	int32 numRes = 0;
	ResourceId randomResourceIds[5];
	memset(&randomResourceIds, 0, sizeof(randomResourceIds));
	for (int32 i = 0; i < 5; i++) {
		if (_field_68C[i]) {
			randomResourceIds[numRes] = _field_68C[i];
			numRes++;
		}
	}

	if (numRes > 0)
		return randomResourceIds[rand() % numRes];

	return _resourceId;
}

bool Barrier::checkFlags() {
	return (flags & kBarrierFlagEnabled) && (flags & kBarrierFlag8 || flags & kBarrierFlag10000);
}

} // end of namespace Asylum
