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

	_field_74     = stream->readSint32LE();
	_field_78     = stream->readSint32LE();
	_field_7C     = stream->readSint32LE();
	_field_80     = stream->readSint32LE();
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


void Barrier::destroy() {
	flags &= kBarrierFlagDestroyed;
}

void Barrier::destroyAndRemoveFromQueue() {
	destroy();
	flags|= kBarrierFlag20000;

	getScreen()->deleteGraphicFromQueue(_resourceId);
}

/////////////////////////////////////////////////////////////////////////
// Visibility
//////////////////////////////////////////////////////////////////////////
bool Barrier::isOnScreen() {
	Common::Rect screenRect  = Common::Rect(getWorld()->xLeft, getWorld()->yTop, getWorld()->xLeft + 640, getWorld()->yTop + 480);
	Common::Rect barrierRect = Common::Rect(_boundingRect);

	barrierRect.translate(x, y);

	return isVisible() && (flags & 1) && screenRect.intersects(barrierRect);
}

bool Barrier::isVisible() {
	if ((flags & 0xFF) & 1) {
		for (int32 f = 0; f < 10; f++) {
			bool   isSet = false;
			GameFlag flag  = _gameFlags[f];

			if (flag <= 0)
				isSet = _vm->isGameFlagNotSet(flag);
			else
				isSet = _vm->isGameFlagSet(flag);

			if (!isSet)
				return false;
		}
		return true;
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////
// Update
//////////////////////////////////////////////////////////////////////////
void Barrier::draw(Actor *actor, Common::Point &pt) {
	bool actInBar   = _boundingRect.contains(*actor->getBoundingRect());
	bool intersects = false;

	// TODO verify that my funky LOBYTE macro actually
	// works the way I assume it should :P
	if (!actInBar) {
		if (LOBYTE(flags) & 0x20)
			if (!(LOBYTE(flags) & 0x80))
				// XXX not sure if this will work, as it's
				// supposed to set 0x40 to the lobyte...
				flags |= 0x40;
		return;
	}

	if (flags & 2) {
		// TODO refactor
		if (_field_74 || _field_78 ||
			_field_7C || _field_80)
			intersects = (pt.y > _field_78 + (_field_80 - _field_78) * (pt.x - _field_74) / (_field_7C - _field_74)) == 0;
		else
			intersects = true;
	} else {
		if (flags & 0x40) {
			PolyDefinitions *poly = &getScene()->polygons()->entries[_polygonIndex];
			if (pt.x > 0 && pt.y > 0 && poly->numPoints > 0)
				intersects = poly->contains(pt.x, pt.y);
			else
				;//warning ("[drawActorsAndBarriers] trying to find intersection of uninitialized point");
		}
		// XXX the original has an else case here that
		// assigns intersects the value of the
		// flags & 2 check, which doesn't make any sense since
		// that path would never have been taken if code
		// execution had made it's way here.
	}
	if (LOBYTE(flags) & 0x80 || intersects) {
		if (LOBYTE(flags) & 0x20)
			// XXX not sure if this will work, as it's
			// supposed to set this value on the lobyte...
			flags &= 0xBF | 0x80;
		else
			// XXX another lobyte assignment...
			flags |= 0x40;
		// TODO label jump up a few lines here. Investigate...
	}
	if (flags & 4) {
		if (intersects) {
			if(actor->flags & 2)
				;//warning ("[drawActorsAndBarriers] Assigning mask to masked character [%s]", _name);
			else {
				// TODO there's a call to sub_40ac10 that does
				// a point calculation, but the result doesn't appear to
				// ever be used, and the object passed in as a parameter
				// isn't updated
				actor->setBarrierIndex(getWorld()->getBarrierIndexById(_id));
				actor->flags |= 2;
			}
		}
	} else {
		if (intersects) {
			// XXX assuming the following:
			// "if ( *(int *)((char *)&scene.characters[0].priority + v18) < *(v12_barrierPtr + 35) )"
			// is the same as what I'm comparing :P
			if (actor->getPriority() < _priority) {
				actor->setField934(1);
				actor->setPriority(_priority + 3);
				// TODO there's a block of code here that seems
				// to loop through the CharacterUpdateItems and do some
				// priority adjustment. Since I'm not using CharacterUpdateItems as of yet,
				// I'm not sure what to do here
				// The loop seems to occur if:
				// (a) there are still character items to process
				// (b) sceneNumber != 2 && actor->field_944 != 1
			}
		} else {
			if (actor->getPriority() > _priority || actor->getPriority() == 1) {
				actor->setField934(1);
				actor->setPriority(_priority - 1);
				// TODO another character update loop
				// This time it looks like there's another
				// intersection test, and more updates
				// to field_934 and field_944, then
				// priority updates
			}
		}
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
			updateSoundItems(_vm->sound());
			stopSound();
		}
	}
	// TODO: get sound functions according with scene
}

void Barrier::setNextFrame(int32 targetFlags) {
	int32 newFlag = targetFlags | 1 | flags;
	flags |= targetFlags | 1;

	if (newFlag & 0x10000)
		_frameIndex = _frameCount - 1;
	else
		_frameIndex = 0;
}

/////////////////////////////////////////////////////////////////////////
// Misc
/////////////////////////////////////////////////////////////////////////
void Barrier::updateSoundItems(Sound *snd) {
	for (int32 i = 0; i < 16; i++) {
		SoundItem *item = &_soundItems[i];
		if (snd->isPlaying(item->resourceId)) {
			if (item->field_4) {
				snd->stopSound(item->resourceId);
				item->resourceId   = 0;
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
		if (_soundItems[i].resourceId)
			getSound()->stopSound(_soundItems[i].resourceId);
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
	return (flags & 1) && (flags & 8 || flags & 0x10000);
}

bool Barrier::checkGameFlags() {
	if (LOBYTE(flags) & 1) {
		for (int32 i = 0; i < 10; i++) {
			if (_vm->isGameFlagSet(_gameFlags[i]))
				return true;
		}

		return true;
	}

	return false;
}

} // end of namespace Asylum
