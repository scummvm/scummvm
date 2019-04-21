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

#include "illusions/actor.h"
#include "illusions/illusions.h"
#include "illusions/resources/actorresource.h"
#include "illusions/dictionary.h"

namespace Illusions {

// ActorResourceLoader

void ActorResourceLoader::load(Resource *resource) {
	resource->_instance = _vm->_actorInstances->createActorInstance(resource);
}

bool ActorResourceLoader::isFlag(int flag) {
	return
		flag == kRlfLoadFile;
}

// Frame

void Frame::load(byte *dataStart, Common::SeekableReadStream &stream) {
	_flags = stream.readUint16LE();
	stream.skip(2); // Skip padding
	uint32 pointsConfigOffs = stream.readUint32LE();
	_surfInfo.load(stream);
	uint32 compressedPixelsOffs = stream.readUint32LE();
	_compressedPixels = dataStart + compressedPixelsOffs;
	_pointsConfig = dataStart + pointsConfigOffs;
	debug(5, "Frame::load() compressedPixelsOffs: %08X",
		compressedPixelsOffs);
}

// Sequence

void Sequence::load(byte *dataStart, Common::SeekableReadStream &stream) {
	_sequenceId = stream.readUint32LE();
	_unk4 = stream.readUint32LE();
	uint32 sequenceCodeOffs = stream.readUint32LE();
	_sequenceCode = dataStart + sequenceCodeOffs;
	debug(5, "Sequence::load() _sequenceId: %08X; _unk4: %d; sequenceCodeOffs: %08X",
		_sequenceId, _unk4, sequenceCodeOffs);
}

// ActorType

void ActorType::load(byte *dataStart, Common::SeekableReadStream &stream) {
	_actorTypeId = stream.readUint32LE();
	_surfInfo.load(stream);
	uint32 pointsConfigOffs = stream.readUint32LE();
	uint namedPointsCount = stream.readUint16LE();
	stream.skip(2); // Skip padding
	uint32 namedPointsOffs = stream.readUint32LE();
	_color.r = stream.readByte();
	_color.g = stream.readByte();
	_color.b = stream.readByte();
	stream.readByte(); // Skip padding
	_scale = stream.readByte();
	_priority = stream.readByte();
	_value1E = stream.readUint16LE();
	_pathWalkPointsIndex = stream.readUint16LE();
	_scaleLayerIndex = stream.readUint16LE();
	_pathWalkRectIndex = stream.readUint16LE();
	_priorityLayerIndex = stream.readUint16LE();
	_regionLayerIndex = stream.readUint16LE();
	_flags = stream.readUint16LE();
	_pointsConfig = dataStart + pointsConfigOffs;
	stream.seek(namedPointsOffs);
	_namedPoints.load(namedPointsCount, stream);
	debug(5, "ActorType::load() _actorTypeId: %08X; _color(%d,%d,%d); _scale: %d; _priority: %d; _value1E: %d",
		_actorTypeId, _color.r, _color.g, _color.b, _scale, _priority, _value1E);
	debug(5, "ActorType::load() _pathWalkPointsIndex: %d; _scaleLayerIndex: %d; _pathWalkRectIndex: %d",
		_pathWalkPointsIndex, _scaleLayerIndex, _pathWalkRectIndex);
	debug(5, "ActorType::load() _priorityLayerIndex: %d; _regionLayerIndex: %d; _flags: %04X",
		_priorityLayerIndex, _regionLayerIndex,_flags);
}

// ActorResource

ActorResource::ActorResource() {
}

ActorResource::~ActorResource() {
}

void ActorResource::load(Resource *resource) {
	byte *data = resource->_data;
	uint32 dataSize = resource->_dataSize;
	Common::MemoryReadStream stream(data, dataSize, DisposeAfterUse::NO);

	_totalSize = stream.readUint32LE();

	// Load actor types
	stream.seek(0x06);
	uint actorTypesCount = stream.readUint16LE();
	stream.seek(0x10);
	uint32 actorTypesOffs = stream.readUint32LE();
	_actorTypes.reserve(actorTypesCount);
	for (uint i = 0; i < actorTypesCount; ++i) {
		ActorType actorType;
		stream.seek(actorTypesOffs + i * 0x2C);
		actorType.load(data, stream);
		_actorTypes.push_back(actorType);
	}

	// Load sequences
	stream.seek(0x08);
	uint sequencesCount = stream.readUint16LE();
	stream.seek(0x14);
	uint32 sequencesOffs = stream.readUint32LE();
	stream.seek(sequencesOffs);
	_sequences.reserve(sequencesCount);
	for (uint i = 0; i < sequencesCount; ++i) {
		Sequence sequence;
		sequence.load(data, stream);
		_sequences.push_back(sequence);
	}

	// Load frames
	stream.seek(0x0A);
	uint framesCount = stream.readUint16LE();
	stream.seek(0x18);
	uint32 framesOffs = stream.readUint32LE();
	stream.seek(framesOffs);
	_frames.reserve(framesCount);
	for (uint i = 0; i < framesCount; ++i) {
		Frame frame;
		frame.load(data, stream);
		_frames.push_back(frame);
	}

	// Load named points
	if (resource->_gameId == kGameIdBBDOU) {
		// The count isn't stored explicitly so calculate it
		uint namedPointsCount = (actorTypesOffs - 0x20) / 8;
		stream.seek(0x20);
		_namedPoints.load(namedPointsCount, stream);
	}

	debug(1, "ActorResource(%08X) framesCount: %d", resource->_resId, framesCount);
}

bool ActorResource::containsSequence(Sequence *sequence) {
	for (uint i = 0; i < _sequences.size(); ++i) {
		if (sequence == &_sequences[i])
			return true;
	}
	return false;
}

bool ActorResource::findNamedPoint(uint32 namedPointId, Common::Point &pt) {
	return _namedPoints.findNamedPoint(namedPointId, pt);
}

// ActorInstance

ActorInstance::ActorInstance(IllusionsEngine *vm)
	: _vm(vm) {
}

void ActorInstance::load(Resource *resource) {
	_actorResource = new ActorResource();
	_actorResource->load(resource);
	_sceneId = resource->_sceneId;
	_pauseCtr = 0;
	initActorTypes(resource->_gameId);
}

void ActorInstance::unload() {
	if (_pauseCtr <= 0)
		unregisterResources();
	_vm->_actorInstances->removeActorInstance(this);
	delete _actorResource;
}

void ActorInstance::pause() {
	++_pauseCtr;
	if (_pauseCtr == 1)
		unregisterResources();
}

void ActorInstance::unpause() {
	--_pauseCtr;
	if (_pauseCtr == 0)
		registerResources();
}

void ActorInstance::initActorTypes(int gameId) {
	for (uint i = 0; i < _actorResource->_actorTypes.size(); ++i) {
		ActorType *actorType = &_actorResource->_actorTypes[i];
		ActorType *actorType2 = _vm->_dict->findActorType(actorType->_actorTypeId);
		if (actorType2) {
			actorType->_surfInfo._dimensions._width = MAX(actorType->_surfInfo._dimensions._width,
				actorType2->_surfInfo._dimensions._width);
			actorType->_surfInfo._dimensions._height = MAX(actorType->_surfInfo._dimensions._height,
				actorType2->_surfInfo._dimensions._height);
			if (actorType->_color.r == 255 && actorType->_color.g == 255 && actorType->_color.b == 255)
				actorType->_color = actorType2->_color;
			if (actorType->_value1E == 0)
				actorType->_value1E = actorType2->_value1E;
		}
		_vm->_dict->addActorType(actorType->_actorTypeId, actorType);
	}
	for (uint i = 0; i < _actorResource->_sequences.size(); ++i) {
		Sequence *sequence = &_actorResource->_sequences[i];
		_vm->_dict->addSequence(sequence->_sequenceId, sequence);
		if (gameId == kGameIdDuckman && sequence->_sequenceId == 0x60101) {
			// TODO check that this is the correct location for this logic.
			_vm->_controls->placeActor(0x50023, Common::Point(0,0), sequence->_sequenceId, 0x400d7, 0);
		}
	}
}

void ActorInstance::registerResources() {
	for (uint i = 0; i < _actorResource->_actorTypes.size(); ++i) {
		ActorType *actorType = &_actorResource->_actorTypes[i];
		_vm->_dict->addActorType(actorType->_actorTypeId, actorType);
	}
	for (uint i = 0; i < _actorResource->_sequences.size(); ++i) {
		Sequence *sequence = &_actorResource->_sequences[i];
		_vm->_dict->addSequence(sequence->_sequenceId, sequence);
	}
}

void ActorInstance::unregisterResources() {
	for (uint i = 0; i < _actorResource->_actorTypes.size(); ++i) {
		_vm->_dict->removeActorType(_actorResource->_actorTypes[i]._actorTypeId);
	}
	for (uint i = 0; i < _actorResource->_sequences.size(); ++i) {
		_vm->_dict->removeSequence(_actorResource->_sequences[i]._sequenceId);
	}
}

// ActorInstanceList

ActorInstanceList::ActorInstanceList(IllusionsEngine *vm)
	: _vm(vm) {
}

ActorInstanceList::~ActorInstanceList() {
}

ActorInstance *ActorInstanceList::createActorInstance(Resource *resource) {
	ActorInstance *actorInstance = new ActorInstance(_vm);
	actorInstance->load(resource);
	_items.push_back(actorInstance);
	return actorInstance;
}

void ActorInstanceList::removeActorInstance(ActorInstance *actorInstance) {
	_items.remove(actorInstance);
}

void ActorInstanceList::pauseBySceneId(uint32 sceneId) {
	for (ItemsIterator it = _items.begin(); it != _items.end(); ++it) {
		if ((*it)->_sceneId == sceneId)
			(*it)->pause();
	}
}

void ActorInstanceList::unpauseBySceneId(uint32 sceneId) {
	for (ItemsIterator it = _items.begin(); it != _items.end(); ++it) {
		if ((*it)->_sceneId == sceneId)
			(*it)->unpause();
	}
}

FramesList *ActorInstanceList::findSequenceFrames(Sequence *sequence) {
	for (ItemsIterator it = _items.begin(); it != _items.end(); ++it) {
		ActorInstance *actorInstance = *it;
		if (actorInstance->_pauseCtr <= 0 && actorInstance->_actorResource->containsSequence(sequence))
			return &actorInstance->_actorResource->_frames;
	}
	return 0;
}

ActorInstance *ActorInstanceList::findActorByResource(ActorResource *actorResource) {
	for (ItemsIterator it = _items.begin(); it != _items.end(); ++it) {
		if ((*it)->_actorResource == actorResource)
			return (*it);
	}
	return 0;
}

bool ActorInstanceList::findNamedPoint(uint32 namedPointId, Common::Point &pt) {
	for (ItemsIterator it = _items.begin(); it != _items.end(); ++it) {
		ActorInstance *actorInstance = *it;
		if (actorInstance->_pauseCtr == 0 && actorInstance->_actorResource->findNamedPoint(namedPointId, pt))
			return true;
	}
	return false;
}

} // End of namespace Illusions
