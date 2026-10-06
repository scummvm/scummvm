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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/random.h"

#include "engines/nancy/nancy.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/resource.h"
#include "engines/nancy/sound.h"
#include "engines/nancy/input.h"
#include "engines/nancy/cursor.h"
#include "engines/nancy/util.h"

#include "engines/nancy/state/scene.h"
#include "engines/nancy/action/puzzle/weightsortpuzzle.h"

namespace Nancy {
namespace Action {

static void readRectList16(Common::SeekableReadStream &stream, Common::Array<Common::Rect> &dst) {
	uint16 num = stream.readUint16LE();
	dst.resize(num);
	for (uint16 i = 0; i < num; ++i) {
		readRect(stream, dst[i]);
	}
}

static void readIntList16(Common::SeekableReadStream &stream, Common::Array<int32> &dst) {
	uint16 num = stream.readUint16LE();
	dst.resize(num);
	for (uint16 i = 0; i < num; ++i) {
		dst[i] = stream.readSint32LE();
	}
}

void WeightSortPuzzle::readContainer(Common::SeekableReadStream &stream, Container &dst) {
	dst.showTotalWeight = stream.readByte();
	dst.countsTowardOutcome = (stream.readSint16LE() != 0);
	dst.pickUpMode = (byte)stream.readSint16LE();
	dst.dropInPlace = (stream.readSint16LE() != 0);

	for (uint i = 0; i < kNumOutcomes; ++i) {
		dst.ranges[i].maxExclusive = stream.readSint32LE();
		dst.ranges[i].minExclusive = stream.readSint32LE();
	}

	readRect(stream, dst.area);
	readRect(stream, dst.shakeHotspot);

	readFilename(stream, dst.readoutImageName);
	readRectList16(stream, dst.digitSrcs);
	readRectList16(stream, dst.digitDests);

	// The accepted kinds arrive as a list of bit sets, folded into one mask.
	dst.acceptedKinds = 0;
	int16 numKinds = stream.readSint16LE();
	for (int16 i = 0; i < numKinds; ++i) {
		dst.acceptedKinds |= stream.readSint32LE();
	}

	dst.pickUpSound.readData(stream);
	dst.putDownSound.readData(stream);

	readIntList16(stream, dst.initialContents);
	readIntList16(stream, dst.requiredContents);
}

void WeightSortPuzzle::readData(Common::SeekableReadStream &stream) {
	_firstNeedsAllContainers = stream.readByte();
	_secondNeedsAllContainers = stream.readByte();
	_firstNeedsEmptyCursor = stream.readByte();
	_returnOnMiss = stream.readByte();

	_hoverCursorType = stream.readUint16LE();
	_dragCursorType = stream.readUint16LE();
	_rejectCursorType = stream.readUint16LE();

	stream.skip(8); // object-slide animation speed and step

	int16 numTypes = stream.readSint16LE();
	_objectTypes.resize(numTypes > 0 ? numTypes : 0);
	for (uint i = 0; i < _objectTypes.size(); ++i) {
		ObjectType &type = _objectTypes[i];
		type.kind = stream.readSint32LE();
		type.weight = stream.readSint32LE();
		readFilename(stream, type.imageName);

		int16 numSrcs = stream.readSint16LE();
		type.srcs.resize(numSrcs > 0 ? numSrcs : 0);
		for (uint j = 0; j < type.srcs.size(); ++j) {
			readRect(stream, type.srcs[j]);
		}
	}

	int16 numContainers = stream.readSint16LE();
	_containers.resize(numContainers > 0 ? numContainers : 0);
	for (uint i = 0; i < _containers.size(); ++i) {
		readContainer(stream, _containers[i]);
	}

	_rejectSound.readData(stream);

	for (uint i = 0; i < kNumOutcomes; ++i) {
		Outcome &outcome = _outcomes[i];
		outcome.scene.sceneID = stream.readUint16LE();
		outcome.scene.frameID = stream.readUint16LE();
		outcome.scene.continueSceneSound = kContinueSceneSound;
		outcome.flag.label = stream.readSint16LE();
		outcome.flag.flag = stream.readByte();
		outcome.sound.readData(stream);
	}

	readExitHotspot(stream);
	_exitScene._sceneChange.continueSceneSound = kContinueSceneSound;
}

// Drops the object at a random spot fully inside its container.
void WeightSortPuzzle::scatter(Object &object) {
	const Common::Rect &area = _containers[object.container].area;
	int w = object.src.width();
	int h = object.src.height();

	int spanX = MAX(0, area.width() - w);
	int spanY = MAX(0, area.height() - h);
	int x = area.left + (spanX ? (int)g_nancy->_randomSource->getRandomNumber(spanX) : 0);
	int y = area.top + (spanY ? (int)g_nancy->_randomSource->getRandomNumber(spanY) : 0);

	object.dest = Common::Rect((int16)x, (int16)y, (int16)(x + w), (int16)(y + h));
}

void WeightSortPuzzle::scatterContainer(uint container) {
	for (uint i = 0; i < _objects.size(); ++i) {
		if (_objects[i].container == (int)container) {
			scatter(_objects[i]);
		}
	}
}

void WeightSortPuzzle::init() {
	initViewportSurface();

	_typeImages.resize(_objectTypes.size());
	for (uint i = 0; i < _objectTypes.size(); ++i) {
		if (!_objectTypes[i].imageName.empty()) {
			g_nancy->_resource->loadImage(_objectTypes[i].imageName, _typeImages[i]);
			_typeImages[i].setTransparentColor(_drawSurface.getTransparentColor());
		}
	}

	_readoutImages.resize(_containers.size());
	for (uint i = 0; i < _containers.size(); ++i) {
		if (!_containers[i].readoutImageName.empty()) {
			g_nancy->_resource->loadImage(_containers[i].readoutImageName, _readoutImages[i]);
			_readoutImages[i].setTransparentColor(_drawSurface.getTransparentColor());
		}
	}

	_objects.clear();
	for (uint i = 0; i < _containers.size(); ++i) {
		const Common::Array<int32> &contents = _containers[i].initialContents;
		for (uint j = 0; j < contents.size(); ++j) {
			int32 typeID = contents[j];
			if (typeID < 0 || (uint)typeID >= _objectTypes.size()) {
				continue;
			}

			const ObjectType &type = _objectTypes[typeID];
			if (type.srcs.empty()) {
				continue;
			}

			Object object;
			object.type = typeID;
			object.container = i;
			object.src = type.srcs[g_nancy->_randomSource->getRandomNumber(type.srcs.size() - 1)];
			scatter(object);
			_objects.push_back(object);
		}
	}

	_carriedObjectID = -1;
	_carriedFrom = -1;
	_outcome = -1;
	_exitRequested = false;

	NancySceneState.setNoHeldItem();

	redraw();
	registerGraphics();
	_carriedObject.registerGraphics();
}

int WeightSortPuzzle::objectAtCursor(const Common::Point &mousePos) const {
	// Later objects are drawn on top of earlier ones, so they are picked up first.
	for (int i = (int)_objects.size() - 1; i >= 0; --i) {
		if (_objects[i].container == -1) {
			continue;
		}
		if (NancySceneState.getViewport().convertViewportToScreen(_objects[i].dest).contains(mousePos)) {
			return i;
		}
	}
	return -1;
}

int WeightSortPuzzle::containerAtCursor(const Common::Point &mousePos) const {
	for (uint i = 0; i < _containers.size(); ++i) {
		if (NancySceneState.getViewport().convertViewportToScreen(_containers[i].area).contains(mousePos)) {
			return i;
		}
	}
	return -1;
}

int WeightSortPuzzle::shakeHotspotAtCursor(const Common::Point &mousePos) const {
	for (uint i = 0; i < _containers.size(); ++i) {
		if (_containers[i].shakeHotspot.isEmpty()) {
			continue;
		}
		if (NancySceneState.getViewport().convertViewportToScreen(_containers[i].shakeHotspot).contains(mousePos)) {
			return i;
		}
	}
	return -1;
}

int WeightSortPuzzle::countListed(const Common::Array<int32> &list, int32 type) {
	int count = 0;
	for (uint i = 0; i < list.size(); ++i) {
		if (list[i] == type) {
			++count;
		}
	}
	return count;
}

int WeightSortPuzzle::countIn(uint container, int32 type) const {
	int count = 0;
	for (uint i = 0; i < _objects.size(); ++i) {
		if (_objects[i].container == (int)container && (int32)_objects[i].type == type) {
			++count;
		}
	}
	return count;
}

bool WeightSortPuzzle::canPickUp(const Object &object) const {
	const Container &container = _containers[object.container];

	switch (container.pickUpMode) {
	case kPickUpOriginal:
		return !object.placedByHand;
	case kPickUpSpare:
		return countIn(object.container, object.type) >
			countListed(container.requiredContents, object.type);
	default:
		return true;
	}
}

int32 WeightSortPuzzle::readoutValue(uint container) const {
	int32 value = 0;
	for (uint i = 0; i < _objects.size(); ++i) {
		if (_objects[i].container != (int)container) {
			continue;
		}
		value += _containers[container].showTotalWeight ? _objectTypes[_objects[i].type].weight : 1;
	}
	return value;
}

bool WeightSortPuzzle::containerPasses(uint container, uint outcome) const {
	const Container &data = _containers[container];
	int32 total = readoutValue(container);

	if (total >= data.ranges[outcome].maxExclusive || total <= data.ranges[outcome].minExclusive) {
		return false;
	}

	// Only the first outcome asks for the container's required contents.
	if (outcome == 0) {
		for (uint i = 0; i < data.requiredContents.size(); ++i) {
			int32 type = data.requiredContents[i];
			if (countIn(container, type) < countListed(data.requiredContents, type)) {
				return false;
			}
		}
	}

	return true;
}

int WeightSortPuzzle::reachedOutcome() const {
	uint numCounted = 0;
	uint numPassing[kNumOutcomes] = {};

	for (uint i = 0; i < _containers.size(); ++i) {
		if (!_containers[i].countsTowardOutcome) {
			continue;
		}

		++numCounted;
		for (uint j = 0; j < kNumOutcomes; ++j) {
			if (containerPasses(i, j)) {
				++numPassing[j];
			}
		}
	}

	if (numCounted == 0) {
		return -1;
	}

	// The second outcome is tested first and takes the board if it passes.
	if (_secondNeedsAllContainers ? (numPassing[1] == numCounted) : (numPassing[1] > 0)) {
		return 1;
	}

	bool cursorOK = !_firstNeedsEmptyCursor || _carriedObjectID == -1;
	if (cursorOK && (_firstNeedsAllContainers ? (numPassing[0] == numCounted) : (numPassing[0] > 0))) {
		return 0;
	}

	return -1;
}

void WeightSortPuzzle::carryObject(int object, NancyInput &input) {
	_carriedObjectID = object;

	if (object >= 0 && !_typeImages[_objects[object].type].empty()) {
		_carriedObject._drawSurface.create(_typeImages[_objects[object].type], _objects[object].src);
		_carriedObject.setTransparent(true);
		_carriedObject.setVisible(true);
		_carriedObject.pickUp();
		_carriedObject.handleInput(input);
	} else {
		_carriedObject.setVisible(false);
		_carriedObject.putDown();
	}
}

void WeightSortPuzzle::redraw() {
	_drawSurface.clear(g_nancy->_graphics->getTransColor());

	for (uint i = 0; i < _objects.size(); ++i) {
		const Object &object = _objects[i];
		if (object.container == -1 || _typeImages[object.type].empty()) {
			continue;
		}
		_drawSurface.blitFrom(_typeImages[object.type], object.src,
			Common::Point(object.dest.left, object.dest.top));
	}

	// Every readout slot holds a digit, so shorter numbers come out zero-padded.
	for (uint i = 0; i < _containers.size(); ++i) {
		const Container &container = _containers[i];
		if (container.digitDests.empty() || container.digitSrcs.size() < 10 || _readoutImages[i].empty()) {
			continue;
		}

		int32 value = readoutValue(i);
		for (int slot = (int)container.digitDests.size() - 1; slot >= 0; --slot) {
			_drawSurface.blitFrom(_readoutImages[i], container.digitSrcs[value % 10],
				Common::Point(container.digitDests[slot].left, container.digitDests[slot].top));
			value /= 10;
		}
	}

	_needsRedraw = true;
}

void WeightSortPuzzle::execute() {
	switch (_state) {
	case kBegin:
		init();
		_state = kRun;
		// fall through
	case kRun:
		if (_exitRequested) {
			_state = kActionTrigger;
			break;
		}

		if (_outcome != -1) {
			if (_endSound.name.empty() || !g_nancy->_sound->isSoundPlaying(_endSound)) {
				_state = kActionTrigger;
			}
		}

		break;
	case kActionTrigger:
		if (_exitRequested) {
			_exitScene.execute();
		} else {
			NancySceneState.setEventFlag(_outcomes[_outcome].flag);
			NancySceneState.changeScene(_outcomes[_outcome].scene);
		}

		finishExecution();
		break;
	}
}

void WeightSortPuzzle::handleInput(NancyInput &input) {
	if (_state != kRun || _outcome != -1 || _exitRequested) {
		return;
	}

	const bool click = (input.input & NancyInput::kLeftMouseButtonUp) != 0;

	// Carrying an object: drop it into a container that takes its kind.
	if (_carriedObjectID >= 0) {
		Object &carried = _objects[_carriedObjectID];
		int target = containerAtCursor(input.mousePos);
		bool accepted = (target >= 0) &&
			(_containers[target].acceptedKinds & _objectTypes[carried.type].kind) != 0;

		setDataCursor((target >= 0 && !accepted) ? _rejectCursorType : _dragCursorType);
		_carriedObject.handleInput(input);

		if (click) {
			if (accepted) {
				carried.container = target;
				carried.placedByHand = true;
				carried.dest.moveTo(input.mousePos.x - carried.dest.width() / 2,
					input.mousePos.y - carried.dest.height() / 2);
				carried.dest.clip(_containers[target].area);
				carried.dest.setWidth(carried.src.width());
				carried.dest.setHeight(carried.src.height());

				playSoundBlock(_containers[target].putDownSound);
				if (!_containers[target].dropInPlace) {
					scatterContainer(target);
				}
			} else {
				if (target >= 0) {
					playSoundBlock(_rejectSound);
				}

				// Nothing took it, so it goes back where it came from.
				carried.container = _carriedFrom;
				if (target >= 0 || _returnOnMiss) {
					scatter(carried);
				}
			}

			carryObject(-1, input);
			redraw();

			_outcome = reachedOutcome();
			if (_outcome != -1) {
				_endSound = playSoundBlock(_outcomes[_outcome].sound);
			}
		}

		input.eatMouseInput();
		return;
	}

	// Not carrying: pick an object up, or shake a container to re-scatter it.
	int object = objectAtCursor(input.mousePos);
	if (object >= 0 && canPickUp(_objects[object])) {
		setDataCursor(_hoverCursorType);
		if (click) {
			_carriedFrom = _objects[object].container;
			playSoundBlock(_containers[_carriedFrom].pickUpSound);
			_objects[object].container = -1;
			carryObject(object, input);
			redraw();
		}
		input.eatMouseInput();
		return;
	}

	int shaken = shakeHotspotAtCursor(input.mousePos);
	if (shaken >= 0) {
		setDataCursor(_hoverCursorType);
		if (click) {
			scatterContainer(shaken);
			redraw();
		}
		input.eatMouseInput();
		return;
	}

	if (hoverExitHotspot(input)) {
		if (click) {
			_exitRequested = true;
		}
	}
}

} // End of namespace Action
} // End of namespace Nancy
