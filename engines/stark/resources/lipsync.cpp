/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#include "engines/stark/resources/lipsync.h"

#include "engines/stark/formats/xrc.h"

#include "engines/stark/resources/anim.h"
#include "engines/stark/resources/item.h"
#include "engines/stark/resources/textureset.h"

#include "engines/stark/services/global.h"
#include "engines/stark/services/services.h"
#include "engines/stark/services/stateprovider.h"

#include "engines/stark/visual/visual.h"
#include "engines/stark/visual/actor.h"

namespace Stark {
namespace Resources {

LipSync::~LipSync() {
}

LipSync::LipSync(Object *parent, byte subType, uint16 index, const Common::String &name) :
		Object(parent, subType, index, name),
		_item(nullptr),
		_sceneItem(nullptr),
		_faceTexture(nullptr),
		_visual(nullptr),
		_enabled(false),
        _checkForNewVisual(false),
        _positionMs(0) {
	_type = TYPE;
}

void LipSync::readData(Formats::XRCReadStream *stream) {
	uint32 shapeCount = stream->readUint32LE();
	for (uint i = 0; i < shapeCount; i++) {
		uint32 shape = stream->readUint32LE();
		_shapes.push_back(shape);

		// The original does not use that data
		stream->skip(4);
	}

	// The original does not use that data
	uint32 unkCount = stream->readUint32LE();
	stream->skip(unkCount);
}

void LipSync::printData() {
	Object::printData();

	Common::String phrase;
	for (uint i = 0; i < _shapes.size(); i++) {
		phrase += _shapes[i];
	}

	debug("shapes: %s", phrase.c_str());
}

void LipSync::setItem(ItemVisual *item, bool playTalkAnim) {
	_item = item;
	_checkForNewVisual = !playTalkAnim;

	if (_item->getSubType() != Item::kItemModel) {
		return;
	}

	_sceneItem = Object::cast<ModelItem>(item);
	_faceTexture = _sceneItem->findTextureSet(TextureSet::kTextureFace);

	if (!_faceTexture) {
		return;
	}

	Anim *anim = _item->getAnim();
	_visual = nullptr;

	if (!anim || anim->getSubType() != Anim::kAnimSkeleton) {
		return;
	}

	AnimSkeleton *animSkeleton = Object::cast<AnimSkeleton>(anim);
	_visual = animSkeleton->getVisual()->get<VisualActor>();

	if (!_visual) {
		return;
	}

	_visual->setTextureFacial(_faceTexture->getTexture());
	_enabled = true;
	_positionMs = 0;
}

void LipSync::reset() {
	_enabled = false;
	_visual = nullptr;
	_positionMs = 0;
	_item = nullptr;
	_sceneItem = nullptr;
	_checkForNewVisual = false;
	_faceTexture = nullptr;
}

void LipSync::onGameLoop() {
	Object::onGameLoop();

	if (!_enabled) {
		return;
	}

	if (_checkForNewVisual && _sceneItem && _faceTexture) {
		Anim *anim = _sceneItem->getAnim();
		if (anim && anim->getSubType() == Anim::kAnimSkeleton) {
			AnimSkeleton *animSkeleton = Object::cast<AnimSkeleton>(anim);
			_visual = animSkeleton->getVisual()->get<VisualActor>();

			if (_visual) {
				_visual->setTextureFacial(_faceTexture->getTexture());
			}
		}
	}

	if (_visual) {
		uint32 shapeIndex = (_positionMs + 100) / 100;
		if (shapeIndex < _shapes.size()) {
			_visual->setNewFace(_shapes[shapeIndex]);
		} else {
			reset();
		}

		_positionMs += StarkGlobal->getMillisecondsPerGameloop();
	}

	if (_enabled && !_visual) {
		reset();
	}
}

void LipSync::saveLoadCurrent(ResourceSerializer *serializer) {
	serializer->syncAsUint32LE(_enabled);
	if (_enabled) {
		serializer->syncAsResourceReference(&_item);
		serializer->syncAsUint32LE(_positionMs);

		if (serializer->isLoading()) {
			setItem(_item, false);
		}
	}
}

} // End of namespace Resources
} // End of namespace Stark
