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
		_randomResourceIds[i] = stream->readSint32LE();

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
	bool doPlaySounds = false;

	if (_field_3C != 4)
		return;

	if (!isVisible()) {
		updateSoundItems();
		return;
	}

	// Examine flags
	if (flags & kBarrierFlag20) {
		if (_vm->getTick() - _tickCount >= Common::Rational(1000, _field_B4).toInt()) {
			_frameIndex =((_frameIndex + 1) % _frameCount);
			_tickCount = _vm->getTick();
			doPlaySounds = true;
		}
	} else if (flags & kBarrierFlag10) {

		bool isFirstFrame = (_frameIndex == 0);

		if (!_frameIndex) {
			if (_vm->getTick() - _tickCount >= 1000 * _tickCount2) {
				if (_vm->getRandom(_field_C0) == 1) {
					if (_randomResourceIds[0]) {
						_resourceId = getRandomResourceId();
						GraphicResource *gra = new GraphicResource(getScene()->getResourcePack(), _resourceId);
						_frameCount  = gra->getFrameCount();
						delete gra;
					}
					_frameIndex++;
				}
				_tickCount = _vm->getTick();
				doPlaySounds = true;
			}

			isFirstFrame = (_frameIndex == 0);
		}

		if (!isFirstFrame) {
			if (_vm->getTick() - _tickCount >= Common::Rational(1000, _field_B4).toInt()) {
				_frameIndex =((_frameIndex + 1) % _frameCount);
				_tickCount = _vm->getTick();
				doPlaySounds = true;
			}
		}
	} else if (BYTE1(flags) & kBarrierFlag8) {
		if (_vm->getTick() - _tickCount >= 1000 * _tickCount2) {
			if (_vm->getRandom(_field_C0) == 1)
				_frameIndex =((_frameIndex + 1) % _frameCount);

			_tickCount = _vm->getTick();
			doPlaySounds = true;
		}
	} else if (flags & kBarrierFlag8) {

		if (_vm->getTick() - _tickCount >= Common::Rational(1000, _field_B4).toInt()) {

			++_frameIndex;

			if (_frameIndex < _frameCount - 1) {
				if (_field_688 == 1) {
					GraphicResource *gra = new GraphicResource(getScene()->getResourcePack(), _resourceId);
					GraphicFrame *frame = gra->getFrame(_frameIndex);
					getScene()->setGlobalX(x + frame->x + (frame->getWidth() >> 1));
					getScene()->setGlobalY(y + frame->y + (frame->getHeight() >> 1));
					delete gra;
				}
			} else {
				flags &= ~kBarrierFlag8;
				if (_field_688 == 1) {
					getScene()->setGlobalX(-1);
					getScene()->setGlobalY(-1);
				}
			}
			_tickCount = _vm->getTick();
			doPlaySounds = true;
		}
	} else if (!(BYTE1(flags) & kBarrierFlag6)) {

		if ((flags & kBarrierFlag10000) && (_vm->getTick() - _tickCount >= Common::Rational(1000, _field_B4).toInt())) {

			++_frameIndex;

			if (_frameIndex <= 0) {
				flags &= ~kBarrierFlag10000;
				if (_field_688 == 1) {
					getScene()->setGlobalX(-1);
					getScene()->setGlobalY(-1);
				}
			} else if (_field_688 == 1) {
				GraphicResource *gra = new GraphicResource(getScene()->getResourcePack(), _resourceId);
				GraphicFrame *frame = gra->getFrame(_frameIndex);
				getScene()->setGlobalX(x + frame->x + (frame->getWidth() >> 1));
				getScene()->setGlobalY(y + frame->y + (frame->getHeight() >> 1));
				delete gra;
			}

			_tickCount = _vm->getTick();
			doPlaySounds = true;
		}
	} else if (_vm->getTick() - _tickCount >= Common::Rational(1000, _field_B4).toInt()) {
		if (BYTE1(flags) & kBarrierFlag2) {
			if (_frameIndex == _frameCount - 1) {
				_frameIndex--;
				flags = BYTE1(flags) & 0xFD | kBarrierFlag4;
			} else {
				_frameIndex++;
			}
		} else if (BYTE1(flags) & kBarrierFlag4) {
			if (_frameIndex) {
				_frameIndex--;
			} else {
				_frameIndex++;
				flags = BYTE1(flags) & 0xFB | kBarrierFlag2;
			}
		}
	}

	if (flags & kBarrierFlag40000) {
		if (_frameIndex == _frameCount - 1) {
			if (_field_B4 <= 15) {
				_field_B4 -= 2;

				if (_field_B4 < 0)
					_field_B4 = 0;
			} else {
				_field_B4 = 15;
			}

			if (!_field_B4)
				flags &= ~kBarrierFlag10E38;
		}
	}

	if (doPlaySounds)
		playSounds();

	getScene()->callSpecFunction(this, -1);
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
void Barrier::playSounds() {
	error("[Barrier::playSounds] not implemented!");
}

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

int32 Barrier::getRandomResourceId() {
	// Initialize random resource id array
	ResourceId shuffle[5];
	memset(&shuffle, 0, sizeof(shuffle));
	int32 count = 0;

	for (int32 i = 0; i < 5; i++) {
		if (_randomResourceIds[i]) {
			shuffle[count] = _randomResourceIds[i];
			count++;
		}
	}

	if (count == 0)
		error("[Barrier::getRandomId] Could not get a random resource Id");

	return shuffle[_vm->getRandom(count)];
}

bool Barrier::checkFlags() {
	return (flags & kBarrierFlagEnabled) && (flags & kBarrierFlag8 || flags & kBarrierFlag10000);
}

} // end of namespace Asylum
