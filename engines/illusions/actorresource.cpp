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

#include "illusions/illusions.h"
#include "illusions/actorresource.h"
#include "illusions/dictionary.h"

namespace Illusions {

// ActorResourceLoader

void ActorResourceLoader::load(Resource *resource) {
	debug("ActorResourceLoader::load() Loading actor %08X from %s...", resource->_resId, resource->_filename.c_str());

	ActorResource *actorResource = new ActorResource();
	actorResource->load(resource->_data, resource->_dataSize);
	resource->_refId = actorResource;
	
	ActorItem *actorItem = _vm->_actorItems->allocActorItem();
	actorItem->_tag = resource->_tag;
	actorItem->_pauseCtr = 0;
	actorItem->_actRes = actorResource;
	
	for (uint i = 0; i < actorResource->_actorTypes.size(); ++i) {
		ActorType *actorType = &actorResource->_actorTypes[i];
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

	for (uint i = 0; i < actorResource->_sequences.size(); ++i) {
		Sequence *sequence = &actorResource->_sequences[i];
		_vm->_dict->addSequence(sequence->_sequenceId, sequence);
	}
	
}

void ActorResourceLoader::unload(Resource *resource) {
	debug("ActorResourceLoader::unload() Unloading actor %08X...", resource->_resId);
	// TODO Move to ActorItems
	ActorItem *actorItem = _vm->_actorItems->findActorByResource((ActorResource*)resource->_refId);
	if (actorItem->_pauseCtr <= 0) {
		for (uint i = 0; i < actorItem->_actRes->_actorTypes.size(); ++i)
			_vm->_dict->removeActorType(actorItem->_actRes->_actorTypes[i]._actorTypeId);
		for (uint i = 0; i < actorItem->_actRes->_sequences.size(); ++i)
			_vm->_dict->removeSequence(actorItem->_actRes->_sequences[i]._sequenceId);
	}
	_vm->_actorItems->freeActorItem(actorItem);
	delete actorItem->_actRes;
}

void ActorResourceLoader::buildFilename(Resource *resource) {
	resource->_filename = Common::String::format("%08X.act", resource->_resId);
}

bool ActorResourceLoader::isFlag(int flag) {
	return
		flag == kRlfLoadFile;
}

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

void Sequence::load(byte *dataStart, Common::SeekableReadStream &stream) {
	_sequenceId = stream.readUint32LE();
	_unk4 = stream.readUint32LE();
	uint32 sequenceCodeOffs = stream.readUint32LE();
	_sequenceCode = dataStart + sequenceCodeOffs;
	
	debug(5, "Sequence::load() _sequenceId: %08X; _unk4: %d; sequenceCodeOffs: %08X",
		_sequenceId, _unk4, sequenceCodeOffs);

	debug("_sequenceId: %08X", _sequenceId);
}

void ActorType::load(byte *dataStart, Common::SeekableReadStream &stream) {
	_actorTypeId = stream.readUint32LE();
	_surfInfo.load(stream);
	uint32 pointsConfigOffs = stream.readUint32LE();
	stream.readUint16LE(); // TODO namedPointsCount dw
	stream.skip(2); // Skip padding
	stream.readUint32LE(); // TODO namedPoints dd
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

	debug(5, "ActorType::load() _actorTypeId: %08X; _color(%d,%d,%d); _scale: %d; _priority: %d; _value1E: %d",
		_actorTypeId, _color.r, _color.g, _color.b, _scale, _priority, _value1E);
	debug(5, "ActorType::load() _pathWalkPointsIndex: %d; _scaleLayerIndex: %d; _pathWalkRectIndex: %d",
		_pathWalkPointsIndex, _scaleLayerIndex, _pathWalkRectIndex);
	debug(5, "ActorType::load() _priorityLayerIndex: %d; _regionLayerIndex: %d; _flags: %04X",
		_priorityLayerIndex, _regionLayerIndex,_flags);

	debug("_actorTypeId: %08X; dimensions: (%d, %d)", _actorTypeId, _surfInfo._dimensions._width, _surfInfo._dimensions._height);

}

// ActorResource

ActorResource::ActorResource() {
}

ActorResource::~ActorResource() {
}

void ActorResource::load(byte *data, uint32 dataSize) {
	Common::MemoryReadStream stream(data, dataSize, DisposeAfterUse::NO);

	_totalSize = stream.readUint32LE();

	// Load actor types	
	stream.seek(0x06);
	uint actorTypesCount = stream.readUint16LE();
	stream.seek(0x10);
	uint32 actorTypesOffs = stream.readUint32LE();
	stream.seek(actorTypesOffs);
	_actorTypes.reserve(actorTypesCount);
	for (uint i = 0; i < actorTypesCount; ++i) {
		ActorType actorType;
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

}

bool ActorResource::containsSequence(Sequence *sequence) {
	for (uint i = 0; i < _sequences.size(); ++i)
		if (sequence == &_sequences[i])
			return true;
	return false;
}

// ActorItem

ActorItem::ActorItem(IllusionsEngine *vm)
	: _vm(vm) {
}

ActorItem::~ActorItem() {
}

void ActorItem::pause() {
	++_pauseCtr;
	if (_pauseCtr == 1) {
		for (uint i = 0; i < _actRes->_actorTypes.size(); ++i)
			_vm->_dict->removeActorType(_actRes->_actorTypes[i]._actorTypeId);
		for (uint i = 0; i < _actRes->_sequences.size(); ++i)
			_vm->_dict->removeSequence(_actRes->_sequences[i]._sequenceId);
	}
}

void ActorItem::unpause() {
	--_pauseCtr;
	if (_pauseCtr == 0) {
		for (uint i = 0; i < _actRes->_actorTypes.size(); ++i) {
			ActorType *actorType = &_actRes->_actorTypes[i];
			_vm->_dict->addActorType(actorType->_actorTypeId, actorType);
		}
		for (uint i = 0; i < _actRes->_sequences.size(); ++i) {
			Sequence *sequence = &_actRes->_sequences[i];
			_vm->_dict->addSequence(sequence->_sequenceId, sequence);
		}
	}
}

// ActorItems

ActorItems::ActorItems(IllusionsEngine *vm)
	: _vm(vm) {
}

ActorItems::~ActorItems() {
}

ActorItem *ActorItems::allocActorItem() {
	ActorItem *actorItem = new ActorItem(_vm);
	_items.push_back(actorItem);
	return actorItem;
}

void ActorItems::freeActorItem(ActorItem *actorItem) {
	_items.remove(actorItem);
	delete actorItem;
}

void ActorItems::pauseByTag(uint32 tag) {
	for (ItemsIterator it = _items.begin(); it != _items.end(); ++it)
		if ((*it)->_tag == tag)
			(*it)->pause();
}

void ActorItems::unpauseByTag(uint32 tag) {
	for (ItemsIterator it = _items.begin(); it != _items.end(); ++it)
		if ((*it)->_tag == tag)
			(*it)->unpause();
}

FramesList *ActorItems::findSequenceFrames(Sequence *sequence) {
	for (ItemsIterator it = _items.begin(); it != _items.end(); ++it) {
		ActorItem *actorItem = *it;
		if (actorItem->_pauseCtr <= 0 && actorItem->_actRes->containsSequence(sequence))
			return &actorItem->_actRes->_frames;
	}
	return 0;
}

ActorItem *ActorItems::findActorByResource(ActorResource *actorResource) {
	for (ItemsIterator it = _items.begin(); it != _items.end(); ++it)
		if ((*it)->_actRes == actorResource)
			return (*it);
	return 0;
}

} // End of namespace Illusions
