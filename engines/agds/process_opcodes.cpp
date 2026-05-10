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

#include "agds/agds.h"
#include "agds/animation.h"
#include "agds/character.h"
#include "agds/font.h"
#include "agds/opcode.h"
#include "agds/patch.h"
#include "agds/process.h"
#include "agds/region.h"
#include "agds/screen.h"
#include "agds/systemVariable.h"
#include "common/debug.h"
#include "common/savefile.h"
#include "common/system.h"
#include "graphics/managed_surface.h"

namespace AGDS {

void Process::dup() {
	push(top());
}

void Process::popNoResult() {
	pop();
}

void Process::push(bool value) {
	_stack.push(value);
}

void Process::push(int8 value) {
	_stack.push(value);
}

void Process::push(int16 value) {
	_stack.push(value);
}

void Process::push(int32 value) {
	_stack.push(value);
}

void Process::push2(int8 value) {
	_stack.push(value);
}

void Process::push2(int16 value) {
	_stack.push(value);
}

void Process::enter(uint16 magic, uint16 size) {
	if (magic != 0xdead || size != 0x0c)
		error("invalid enter() magic: 0x%04x or size: %u", magic, size);
	uint16 unk1 = next16();
	uint16 unk2 = next16();
	uint16 unk3 = next16();
	unsigned resOffset = next16();
	uint16 resCount = next16();
	uint16 unk4 = next16();
	debug("resource block %04x %04x %04x %04x,"
		  " resources table at %04x with %u entries",
		  unk1, unk2, unk3, unk4, resOffset, resCount);

	_object->readStringTable(resOffset, resCount);
}

void Process::setStringSystemVariable() {
	int16 valueIndex = pop();
	Common::String name = popString();

	if (valueIndex != -1) {
		Common::String value = getString(valueIndex);
		debug("setSystemVariable %s to %s", name.c_str(), value.c_str());
		_engine->getSystemVariable(name)->setString(value);
	} else {
		debug("resetSystemVariable %s", name.c_str());
		_engine->getSystemVariable(name)->reset();
	}
}

void Process::setIntegerSystemVariable() {
	int32 value = pop();
	Common::String name = popString();
	debug("setIntegerSystemVariable: %s -> %d", name.c_str(), value);
	_engine->getSystemVariable(name)->setInteger(value);
}

void Process::getIntegerSystemVariable() {
	Common::String name = popString();
	int32 value = _engine->getSystemVariable(name)->getInteger();
	debug("getIntegerSystemVariable: %s -> %d", name.c_str(), value);
	push(value);
}

void Process::getRegionCenterX() {
	Common::String name = popString();
	RegionPtr reg = _engine->loadRegion(name);
	int32 value = reg->center.x;
	push(value);
	debug("getRegionCenterX %s -> %d", name.c_str(), value);
}

void Process::getRegionCenterY() {
	Common::String name = popString();
	RegionPtr reg = _engine->loadRegion(name);
	int32 value = reg->center.y;
	push(value);
	debug("getRegionCenterY %s -> %d", name.c_str(), value);
}

void Process::getObjectId() {
	const Common::String &name = _object->getName();
	// no rfind :(((
	Common::String::const_iterator dotpos = 0;
	for (Common::String::const_iterator i = name.begin(); i != name.end(); ++i)
		if (*i == '.')
			dotpos = i + 1;
	Common::String id(dotpos, name.end());
	int32 value = atoi(id.c_str());
	debug("getObjectId %s %d", name.c_str(), value);
	push(value);
}

void Process::loadPicture() {
	Common::String name = popText();
	int32 cacheId = _engine->loadFromCache(name);
	if (cacheId < 0) {
		cacheId = _engine->saveToCache(name, _engine->loadPicture(name));
	}
	debug("loadPicture %s -> %d", name.c_str(), cacheId);
	push(cacheId);
}

void Process::loadAnimation() {
	Common::String name = popText();
	debug("loadAnimation %s (phase: %s) %s", name.c_str(), _phaseVar.c_str(), _phaseVarControlled ? "(phase-var)" : "");
	auto animation = _engine->loadAnimation(name);
	if (animation) {
		if (animation == _processAnimation) {
			warning("double adding animation, skipped.");
			return;
		}
		setupAnimation(animation);
		_processAnimation = animation;
		_engine->getCurrentScreen()->add(animation);
	}
	suspendIfPassive();
}

void Process::loadSample() {
	Common::String name = popString();
	debug("loadSample %s, phaseVar: %s, ambient: %d, volume: %d", name.c_str(), _phaseVar.c_str(), _sampleAmbient, _sampleVolume);
	bool playNow = _sampleAmbient || !_phaseVarControlled || _phaseVar.empty();
	int id = _engine->soundManager().play(getName(), name, _engine->loadText(name), _phaseVar, playNow, _sampleVolume, 0, -1, _sampleAmbient);
	if (_sampleAmbient)
		_engine->setAmbientSoundId(id);
	// original engine sets timer to 24 * bitrate / 44100 / 4
	if (playNow && _phaseVar.empty()) {
		deactivate();
	}
	suspendIfPassive();
}

void Process::getSampleVolume() {
	Common::String name = popString();
	debug("getSampleVolume: %s", name.c_str());
	auto sound = _engine->soundManager().findSampleByPhaseVar(name);
	if (sound) {
		debug("\treturning %d", sound->leftVolume());
		push((int32)sound->leftVolume());
	} else {
		warning("could not find sample %s", name.c_str());
		push((int32)-1);
	}
}

void Process::setSampleVolumeAndPan() {
	int pan = pop();
	int volume = pop();
	if (volume < 0)
		volume = 0;
	else if (volume > 100)
		volume = 100;
	if (pan < -100)
		pan = -100;
	if (pan > 100)
		pan = 100;
	Common::String name = popString();
	auto sound = _engine->soundManager().findSampleByPhaseVar(name);
	if (!sound) {
		warning("can't find sample with phase var %s", name.c_str());
		return;
	}

	debug("setSampleVolumeAndPan %s, volume: %d, pan: %d", name.c_str(), volume, pan);
	int l, r;
	if (pan < 0) {
		l = volume;
		r = volume * (100 + pan) / 100;
	} else {
		l = volume * (100 - pan) / 100;
		r = volume;
	}
	debug("\tleft: %d, right: %d", l, r);
	sound->volume = volume;
	sound->pan = pan;
}

void Process::addSampleToSoundGroup() {
	Common::String name = popText();
	int arg = pop();
	debug("addSampleToSoundGroup stub: %s sound group: %d", name.c_str(), arg);
}

void Process::restartSample() {
	Common::String name = popString();
	debug("restartSample %s", name.c_str());
	auto sound = _engine->soundManager().findSampleByPhaseVar(name);
	if (sound) {
		debug("sample found (%s:%s)", sound->resource.c_str(), sound->filename.c_str());
		int value = _engine->getGlobal(name);
		_engine->setGlobal(name, value | 2);
	} else {
		debug("sample not found");
	}
}

void Process::stopSample() {
	Common::String name = popString();
	debug("restartSample %s", name.c_str());
	auto sound = _engine->soundManager().findSampleByPhaseVar(name);
	if (sound) {
		debug("sample found (%s:%s)", sound->resource.c_str(), sound->filename.c_str());
		int value = _engine->getGlobal(name);
		_engine->setGlobal(name, value | 4);
	} else {
		debug("sample not found");
	}
}

void Process::loadScreenObject() {
	Common::String name = popString();
	debug("loadScreenObject: %s", name.c_str());
	suspend(kExitCodeLoadScreenObject, name);
}

void Process::loadScreenRegion() {
	Common::String name = popString();
	debug("loadScreenRegion %s", name.c_str());
	RegionPtr region = _engine->loadRegion(name);
	_engine->getCurrentScreen()->region(region);
	debug("region: %s", region->toString().c_str());
}

void Process::cloneObject() {
	Common::String arg2 = popString();
	Common::String arg1 = popString();
	debug("cloneObject: %s %s", arg1.c_str(), arg2.c_str());
	suspend(kExitCodeLoadScreenObjectAs, arg1, arg2);
}

void Process::removeScreenObject() {
	Common::String name = popString();
	removeScreenObject(name);
}

void Process::removeScreenObject(const Common::String &name) {
	debug("removeScreenObject: %s", name.c_str());
	auto screen = _engine->getCurrentScreen();
	if (!screen) {
		warning("no current screen");
		return;
	}

	auto object = screen->find(name);
	if (!object) {
		warning("removeScreenObject: object %s not found", name.c_str());
		return;
	}

	_object->lock();
	screen->remove(object);
	if (!object->locked()) {
		ProcessPtr process;
		do {
			process = _engine->findProcess(name);
			if (process) {
				process->removeProcessAnimation();
				process->done();
			}
		} while (process);
		_engine->soundManager().stopAllFrom(name);
	}
	_object->unlock();
}

void Process::loadFont() {
	Common::String name = popText();
	int id = pop();
	debug("loadFont %s %d", name.c_str(), id);
	_engine->loadFont(id, name, _tileWidth, _tileHeight);
}

void Process::loadMouse() {
	Common::String name = popText();
	debug("loadMouse %s", name.c_str());
	_engine->loadDefaultMouseCursor(name);
}

void Process::resetMousePointer() {
	debug("resetMousePointer");
	_engine->popCurrentInventoryObject();
}

void Process::getRandomNumber() {
	int max = pop();
	int32 value = _engine->getRandomNumber(max);
	debug("random %d -> %d", max, value);
	push(value);
}

void Process::setGlobal() {
	Common::String name = popString();
	int value = pop();
	debug("setGlobal %s %d", name.c_str(), value);
	_engine->setGlobal(name, value);
}

void Process::setGlobalWithTop() {
	Common::String name = popString();
	int value = top();
	debug("setGlobalWithTop %s %d", name.c_str(), value);
	_engine->setGlobal(name, value);
}

void Process::setPhaseVar() {
	Common::String name = popString();
	debug("setPhaseVar %s", name.c_str());
	_engine->setGlobal(name, 0);
	_phaseVar = name;
}

void Process::getGlobal(uint8 index) {
	auto name = getString(index);
	int32 value = _engine->getGlobal(name);
	debug("get global %u %s -> %d", index, name.c_str(), value);
	push(value);
}

void Process::hasGlobal() {
	Common::String name = popString();
	int32 result = _engine->hasGlobal(name) ? 1 : 0;
	debug("hasGlobal %s %d", name.c_str(), result);
	push(result);
}

void Process::postIncrementGlobal() {
	Common::String name = popString();
	int32 value = _engine->getGlobal(name);
	debug("post-increment global %s %d", name.c_str(), value);
	push(value);
	_engine->setGlobal(name, value + 1);
}

void Process::postDecrementGlobal() {
	Common::String name = popString();
	int32 value = _engine->getGlobal(name);
	debug("post-decrement global %s %d", name.c_str(), value);
	push(value);
	_engine->setGlobal(name, value - 1);
}

void Process::incrementGlobalByTop() {
	Common::String name = popString();
	int value = _engine->getGlobal(name);
	auto inc = top();
	debug("increment global %s %d by %d", name.c_str(), value, inc);
	_engine->setGlobal(name, value + inc);
}

void Process::decrementGlobalByTop() {
	Common::String name = popString();
	int value = _engine->getGlobal(name);
	auto dec = top();
	debug("decrement global %s %d by %d", name.c_str(), value, dec);
	_engine->setGlobal(name, value - dec);
}

void Process::multiplyGlobalByTop() {
	Common::String name = popString();
	int mul = top();
	int value = _engine->getGlobal(name);
	debug("multiply global %s %d by %d", name.c_str(), value, mul);
	_engine->setGlobal(name, value * mul);
}

void Process::divideGlobalByTop() {
	Common::String name = popString();
	int div = top();
	int value = _engine->getGlobal(name);
	debug("divide global %s %d by %d", name.c_str(), value, div);
	_engine->setGlobal(name, value / div);
}

void Process::modGlobalByTop() {
	Common::String name = popString();
	int div = top();
	int value = _engine->getGlobal(name);
	debug("mod global %s %d by %d", name.c_str(), value, div);
	_engine->setGlobal(name, value % div);
}

void Process::shlGlobalByTop() {
	Common::String name = popString();
	int bits = top();
	int value = _engine->getGlobal(name);
	debug("shift global left %s %d by %d", name.c_str(), value, bits);
	_engine->setGlobal(name, value << bits);
}

void Process::shrGlobalByTop() {
	Common::String name = popString();
	int bits = top();
	int value = _engine->getGlobal(name);
	debug("shift global right %s %d by %d", name.c_str(), value, bits);
	_engine->setGlobal(name, value >> bits);
}

void Process::andGlobalByTop() {
	Common::String name = popString();
	int arg = top();
	int value = _engine->getGlobal(name);
	debug("and global %s %d by %d", name.c_str(), value, arg);
	_engine->setGlobal(name, value & arg);
}

void Process::orGlobalByTop() {
	Common::String name = popString();
	int arg = top();
	int value = _engine->getGlobal(name);
	debug("or global %s %d by %d", name.c_str(), value, arg);
	_engine->setGlobal(name, value | arg);
}

void Process::xorGlobalByTop() {
	Common::String name = popString();
	int arg = top();
	int value = _engine->getGlobal(name);
	debug("xor global %s %d by %d", name.c_str(), value, arg);
	_engine->setGlobal(name, value ^ arg);
}

void Process::appendToSharedStorage() {
	Common::String value = popString();
	int32 index = _engine->appendToSharedStorage(value);
	// debug("appendToSharedStorage %s -> %d", value.c_str(), index);
	push(index);
}

void Process::appendNameToSharedStorage() {
	int32 index = _engine->appendToSharedStorage(_object->getName());
	push(index);
}

Common::String Process::getCloneVarName(const Common::String &arg1, const Common::String &arg2) {
	bool isNumeric = false;
	size_t prefixLength;
	{
		const char *begin = arg1.c_str();
		const char *ptr = begin + arg1.size() - 1;
		while (*ptr >= '0' && *ptr <= '9' && ptr > begin)
			--ptr;
		isNumeric = *ptr == '.';
		prefixLength = isNumeric ? ptr - begin : 0;
	}

	Common::String name;
	if (isNumeric) {
		// no substr :(((
		name = Common::String(arg1.c_str(), arg1.c_str() + prefixLength) +
			   "." + arg2 + Common::String(arg1.c_str() + prefixLength);
	} else {
		name = arg1 + "." + arg2;
	}
	return name;
}

void Process::getCloneVar() {
	Common::String arg2 = popString();
	Common::String arg1 = popString();
	debug("getCloneVar %s %s", arg1.c_str(), arg2.c_str());
	Common::String name = getCloneVarName(arg1, arg2);
	debug("global name for clone: %s", name.c_str());
	push((int32)_engine->getGlobal(name));
}

void Process::setCloneVar() {
	int32 arg3 = pop();
	Common::String arg2 = popString();
	Common::String arg1 = popString();
	debug("setCloneVar %s %s %d", arg1.c_str(), arg2.c_str(), arg3);
	Common::String name = getCloneVarName(arg1, arg2);
	debug("global name for clone: %s", name.c_str());
	_engine->setGlobal(name, arg3);
	push(arg3);
}

void Process::cloneName() {
	int arg2 = pop();
	Common::String arg1 = popString();
	Common::String name = Common::String::format("%s.%d", arg1.c_str(), arg2);
	debug("cloneName: %s %d -> %s", arg1.c_str(), arg2, name.c_str());
	push((int32)_engine->appendToSharedStorage(name));
}

void Process::disableUser() {
	debug("disableUser");
	_engine->enableUser(false);
}

void Process::userEnabled() {
	bool enabled = _engine->userEnabled();
	debug("userEnabled -> %d", enabled);
	push(enabled);
}

void Process::checkScreenPatch() {
	Common::String objectName = popString();
	Common::String screenName = popString();
	Common::String inventoryScr = _engine->getSystemVariable("inventory_scr")->getString();
	debug("checkScreenPatch: screen: %s, object: %s, inventory: %s",
		  screenName.empty() ? "none" : screenName.c_str(), objectName.c_str(), inventoryScr.empty() ? "none" : inventoryScr.c_str());

	Screen *screen = _engine->getCurrentScreen();
	if (!screen) {
		error("no current screen");
		return;
	}

	if (!screenName.empty() && screenName != screen->getName()) {
		if (objectName != _engine->getSystemVariable("inventory_scr")->getString()) {
			debug("checkScreenPatch for object %s %s", screenName.c_str(), objectName.c_str());
			auto patch = _engine->getPatch(screenName);
			push((int32)(patch ? patch->getFlag(objectName) : 0));
		} else {
			push(_engine->inventory().find(objectName) >= 0);
		}
	} else if (screen && screen->applyingPatch()) {
		debug("checkScreenPatch: attempt to change screen patch (%s) in patching process %s", screen->getName().c_str(), getName().c_str());
		push((int32)-1);
	} else {
		ObjectPtr object = screen->find(objectName);
		int32 value = object && object->alive();
		debug("checkScreenPatch: current screen object present: %d", value);
		push(value);
	}
}

void Process::loadMouseCursorFromObject() {
	Common::String name = popText();
	auto inventoryObject = _engine->currentInventoryObject();
	bool changeInventoryObject = (inventoryObject && inventoryObject->getName() == getName());
	debug("loadMouseCursorFromObject %s inventory: %d", !name.empty() ? name.c_str() : "<remove>", changeInventoryObject);
	auto cursor = !name.empty() ? _engine->loadMouseCursor(name) : nullptr;
	_object->setMouseCursor(cursor); // overlay cursor
}

void Process::attachInventoryObjectToMouse(bool keepGraphics) {
	Common::String name = popString();
	debug("attachInventoryObjectToMouse %s, keepGraphics: %d", name.c_str(), keepGraphics);
	auto object = _engine->getCurrentScreenObject(name);
	if (!object) {
		warning("cannot find object %s", name.c_str());
		return;
	}
	if (!keepGraphics) {
		object->setAnimation(nullptr);
		object->setPicture(nullptr);
	}
	_engine->currentInventoryObject(object);
}

void Process::returnCurrentInventoryObject() {
	auto object = _engine->popCurrentInventoryObject();
	if (!object) {
		warning("no current inventory object");
		return;
	}

	auto name = object->getName();
	debug("returnCurrentInventoryObject %s", name.c_str());
	auto screen = _engine->getCurrentScreen();
	if (screen) {
		_object->lock();
		object->lock();
		screen->remove(object);
		object->unlock();
		_object->unlock();
	}
	_engine->inventory().add(object);
	suspend();
}

void Process::attachInventoryObjectToMouse0() {
	attachInventoryObjectToMouse(false);
}

void Process::attachInventoryObjectToMouse1() {
	attachInventoryObjectToMouse(true);
}

void Process::fadeObject() {
	int value = pop();
	Common::String name = popString();
	debug("fadeObject %s %d", name.c_str(), value);
	ObjectPtr object = _engine->getCurrentScreenObject(name);
	if (object)
		object->setAlpha(value);
	else
		warning("fadeObject: object %s not found", name.c_str());
}

void Process::onObjectUse(uint16 size) {
	Common::String arg = popString();
	debug("use [handler] on %s -> 0x%04x", arg.c_str(), _ip);
	_object->setUseHandler(arg, _ip);
	_ip += size;
}

void Process::onObjectUserUse(uint16 size) {
	debug("user use [handler], %u instructions -> 0x%04x", size, _ip);
	_object->setUserUseHandler(_ip);
	_ip += size;
}

void Process::screenSetZNearFar() {
	int arg2 = pop();
	int arg1 = pop();
	debug("screenSetCharacterZNearFar: %d %d", arg1, arg2);
	_engine->getCurrentScreen()->setCharacterNearFar(arg1, arg2);
}

void Process::setDelay() {
	int delay = pop();
	debug("setDelay %d", delay);
	_animationDelay = delay;
}

void Process::setCycles() {
	int value = pop();
	debug("setCycles %d", value);
	_animationCycles = value;
}

void Process::setRandom() {
	int value = pop();
	debug("setRandom %d", value);
	_animationRandom = value;
}

void Process::setPeriodic() {
	int value = pop();
	debug("setPeriodic %d", value);
	_samplePeriodic = value;
}

void Process::stub102() {
	Common::String name = popString();
	debug("stub102: load picture? %s", name.c_str());
}

void Process::setPhaseVarControlledFlag() {
	debug("setPhaseVarControlledFlag");
	_phaseVarControlled = true;
}

void Process::resetState() {
	debug("process reset state");
	_phaseVar.clear();
	_animationCycles = 1;
	_animationLoop = false;
	_animationPosition = Common::Point();
	_phaseVarControlled = false;
	_animationZ = 0;
	_animationRandom = 0;
	_animationDelay = -1;
	_animationSpeed = 100;
	_samplePeriodic = false;
	_sampleAmbient = false;
	_sampleVolume = 100;

	_tileWidth = 16;
	_tileHeight = 16;
	_tileIndex = 0;
	_tileResource = -1;

	_filmSubtitlesResource = -1;
}

void Process::setAnimationZ() {
	int z = pop();
	debug("setAnimationZ %d", z);
	_animationZ = z;
}

void Process::setPanAndVolume() {
	int pan = 0;
	if (_version > 0) {
		pan = pop();
	}
	int volume = pop();
	debug("setPanAndVolume: pan %d volume %d", pan, volume);
}

void Process::setAnimationLoop() {
	debug("loopAnimation");
	_animationLoop = true;
}

void Process::setAnimationSpeed() {
	int value = pop();
	debug("setAnimationSpeed %d", value);
	if (value != 0)
		_animationSpeed = value;
}

void Process::compareScreenName() {
	auto name = popString();
	auto currentScreenName = _engine->getCurrentScreenName();
	debug("compareScreenName %s (currentScreen: %s)", name.c_str(), currentScreenName.c_str());
	push((int32)(name == currentScreenName ? 1 : 0));
}

void Process::objectPatchSetText() {
	Common::String resource = popString();
	Common::String objectName = popString();
	debug("objectPatchSetText: %s %s", objectName.c_str(), resource.c_str());
	auto text = _engine->loadText(resource);

	auto object = _engine->getCurrentScreenObject(objectName);
	if (object) {
		object->title(text);
	}
	auto patch = _engine->createObjectPatch(objectName);
	patch->text = resource;
}

void Process::objectPatchSetRegionName() {
	Common::String regionName = popString();
	Common::String objectName = popString();
	debug("objectPatchSetRegionName: %s %s", objectName.c_str(), regionName.c_str());
	auto object = _engine->getCurrentScreenObject(objectName);
	if (object) {
		RegionPtr region = _engine->loadRegion(regionName);
		debug("region: %s", region->toString().c_str());
		object->region(region);
	}
	auto patch = _engine->createObjectPatch(objectName);
	patch->region = regionName;
}

void Process::screenPatchSetRegionName() {
	Common::String regionName = popString();
	Common::String screenName = popString();
	debug("screenPatchSetRegionName: %s %s", screenName.c_str(), regionName.c_str());
	auto screen = _engine->getCurrentScreen();
	if (!screen) {
		warning("no screen");
		return;
	}
	if (screen->getName() == screenName) {
		auto object = screen->getObject();
		RegionPtr region = _engine->loadRegion(regionName);
		debug("region: %s", region->toString().c_str());
		object->region(region);
	}
	auto patch = _engine->createPatch(screenName);
	patch->screenRegionName = regionName;
}

void Process::screenObjectPatchIncRef() {
	Common::String objectName = popString();
	Common::String screenName = popString();
	debug("screenObjectPatchIncRef: %s %s", screenName.c_str(), objectName.c_str());
	if (!screenName.empty() && screenName != _engine->getCurrentScreenName()) {
		if (_engine->getCurrentScreen()->applyingPatch()) {
			warning("attempt to change screen patch (%s) in patching process (%s)", objectName.c_str(), screenName.c_str());
		} else {
			// fixme: add non-existent screen check?
			auto patch = _engine->createPatch(screenName);
			int refs = patch->incRef(objectName);
			debug("increment refcount for object %s, result: %d", objectName.c_str(), refs);
		}
	} else {
		debug("screenObjectPatchIncRef: current screen, loading object");
		suspend(kExitCodeLoadScreenObject, objectName);
	}
}

void Process::screenObjectPatchDecRef() {
	Common::String objectName = popString();
	Common::String screenName = popString();
	debug("screenObjectPatchDecRef %s %s", screenName.c_str(), objectName.c_str());
	if (!screenName.empty() && screenName != _engine->getCurrentScreenName()) {
		if (_engine->getCurrentScreen()->applyingPatch()) {
			warning("attempt to change screen patch (%s) in patching process (%s)", objectName.c_str(), screenName.c_str());
		} else {
			// fixme: add non-existent screen check?
			auto patch = _engine->createPatch(screenName);
			int refs = patch->decRef(objectName);
			debug("decrement refcount for object %s, result: %d", objectName.c_str(), refs);
		}
	} else {
		debug("screenObjectPatchDecRef: current screen, removing object");
		removeScreenObject(objectName);
	}
}

void Process::getPictureBaseX() {
	Common::String name = popString();
	debug("getPictureBaseX: %s", name.c_str());
	ObjectPtr object = _engine->getCurrentScreenObject(name);
	int32 x = object ? object->getOffset().x : 0;
	debug("\t%d", x);
	push(x);
}

void Process::getPictureBaseY() {
	Common::String name = popString();
	debug("getPictureBaseY: %s", name.c_str());
	ObjectPtr object = _engine->getCurrentScreenObject(name);
	int32 y = object ? object->getOffset().y : 0;
	debug("\t%d", y);
	push(y);
}

void Process::getObjectSurfaceX() {
	Common::String name = popString();
	debug("getObjectSurfaceX: %s", name.c_str());
	ObjectPtr object = _engine->getCurrentScreenObject(name);
	int32 x = object ? object->getPosition().x : 0;
	debug("\t%d", x);
	push(x);
}

void Process::getObjectSurfaceY() {
	Common::String name = popString();
	debug("getObjectSurfaceY: %s", name.c_str());
	ObjectPtr object = _engine->getCurrentScreenObject(name);
	int32 y = object ? object->getPosition().y : 0;
	debug("\t%d", y);
	push(y);
}

void Process::getSavedMouseX() {
	debug("saved mouse position x -> %d", _mousePosition.x);
	push(_mousePosition.x);
}

void Process::getSavedMouseY() {
	debug("saved mouse position y -> %d", _mousePosition.y);
	push(_mousePosition.y);
}

void Process::loadGame() {
	int saveSlot = pop();
	debug("loadGame %d", saveSlot);
	suspend(kExitCodeLoadGame, saveSlot);
}

void Process::saveGame() {
	int saveSlot = pop();
	debug("saveGame %d", saveSlot);
	if (_engine->saveGameState(saveSlot, "").getCode() != Common::kNoError) {
		warning("failed to save game");
	}
	suspend(kExitCodeSaveGame);
}

void Process::getSaveGameName() {
	int flag = pop();
	int saveSlot = pop();
	debug("getSaveGameName stub %d %d", saveSlot, flag);
	push((int32)1);
	suspendIfPassive();
}

void Process::loadSaveSlotNamePicture() {
	int saveSlot = pop();
	debug("loadSaveSlotNamePicture: %d", saveSlot);

	Common::String saveSlotName = _engine->getSaveStateName(saveSlot);
	Common::SaveFileManager *saveMan = _engine->getSaveFileManager();
	Common::ScopedPtr<Common::InSaveFile> save(saveMan->openForLoading(saveSlotName));

	debug("save state name: %s", saveSlotName.c_str());
	int fontId = _engine->getSystemVariable("edit_font")->getInteger();

	auto *font = _engine->getFont(fontId);
	int h = font->getFontHeight();
	static const int w = 400;

	Graphics::Surface *label = _engine->createSurface(w, h);
	uint32 color = _engine->pixelFormat().RGBToColor(255, 0, 255);
	label->fillRect(label->getRect(), color);
	font->drawString(label, save ? saveSlotName : "", 0, 0, w, color);
	Graphics::ManagedSurface *transparentLabel = _engine->convertToTransparent(label);
	_object->setPicture(transparentLabel);
	_object->generateRegion();
}

void Process::setObjectRegionOffset() {
	int dy = pop();
	int dx = pop();
	Common::String objectName = popString();
	debug("setObjectRegionOffset %s %d %d", objectName.c_str(), dx, dy);
	auto object = _engine->getCurrentScreenObject(objectName);
	if (object)
		object->regionOffset(Common::Point(dx, dy));
	else
		warning("setObjectRegionOffset: object %s not found", objectName.c_str());
}

void Process::setSampleVolume() {
	_sampleVolume = pop();
	debug("setSampleVolume: %d", _sampleVolume);
}

void Process::stub173() {
	debug("stub173: remove currentCursor");
}

void Process::stub174() {
	debug("stub174: set mouse relative mode");
}

void Process::objectIgnoreRegion() {
	int value = pop();
	Common::String objectName = popString();
	debug("objectIgnoreRegion: %s %d", objectName.c_str(), value);
	auto object = _engine->getCurrentScreenObject(objectName);
	if (object)
		object->ignoreRegion(value > 0);
	else
		warning("objectIgnoreRegion: object %s not found", objectName.c_str());
}

void Process::removeGapsFromInventory() {
	debug("removeGapsFromInventory");
	_engine->inventory().removeGaps();
}

void Process::setObjectScale() {
	int value = pop();
	debug("setObjectScale %d", value);
	_object->scale(value);
}

void Process::disableMouseAreas() {
	int value = pop();
	debug("disableMouseAreas %d", value);
	_engine->_mouseMap.disable(value > 0);
}

void Process::sampleAmbient() {
	debug("setAmbientSample");
	_sampleAmbient = true;
}

void Process::setRotation() {
	int rotate = pop();
	auto objectName = popString();
	debug("setRotation: object: %s %d", objectName.c_str(), rotate);
	ObjectPtr object = _engine->getCurrentScreenObject(objectName);
	if (object)
		object->rotate(object->rotation() + rotate);
	else
		warning("setRotation: object: %s not found in scene", objectName.c_str());
}

void Process::stub199() {
	int value = pop();
	debug("stub199: (free cached surface?) %d", value);
}

void Process::setTileIndex() {
	_tileIndex = pop();
	_tileResource = pop();
	debug("setTileIndex: index: %d, resource id: %d ", _tileIndex, _tileResource);
}

void Process::setThrowHandler(uint16 size) {
	debug("setThrowHandler %u instructions", size);
	_object->setThrowHandler(_ip);
	_ip += size;
}

void Process::setUseOnHandler(uint16 size) {
	debug("setUseOnHandler %u instructions", size);
	_object->setUseOnHandler(_ip);
	_ip += size;
}

void Process::modifyMouseArea() {
	int enabled = pop();
	int id = pop();
	debug("modifyMouseArea %d, %d", id, enabled);
	suspend(kExitCodeMouseAreaChange, id, enabled);
}

void Process::clearSoundGroup() {
	int id = pop();
	debug("clearSoundGroup stub: %d", id);
}

void Process::stub216() {
	int soundGroup = pop();
	int frame = pop();
	int id = pop();
	debug("stub216: setCharacterWalkSound? id: %d, frame: %d, soundGroup: %d", id, frame, soundGroup);
}

void Process::stub217() {
	int soundGroup = pop();
	int frame = pop();
	int id = pop();
	debug("stub217: animation? id: %d, frame: %d, soundGroup: %d", id, frame, soundGroup);
}

void Process::restartAnimation() {
	Common::String phaseVar = popString();
	debug("restartAnimation %s", phaseVar.c_str());
	if (phaseVar.empty()) {
		warning("no phaseVar");
		return;
	}
	auto animation = _engine->findAnimationByPhaseVar(phaseVar);
	if (animation) {
		if (_engine->getGlobal(phaseVar) == -1 && !animation->paused()) {
			debug("restartAnimation: rewind");
			animation->rewind();
		}
		animation->resume();
		animation->onScreen(true);
		auto phase = animation->phase();
		_engine->setGlobal(phaseVar, phase > 0 ? phase - 1 : 0);
	} else {
		warning("no animation with phase var %s found", phaseVar.c_str());
		_engine->setGlobal(phaseVar, -1);
	}
}

void Process::animationNextFrame() {
	Common::String phaseVar = popString();
	debug("animationNextFrame %s", phaseVar.c_str());
	if (phaseVar.empty()) {
		warning("no phaseVar");
		return;
	}
	auto animation = _engine->findAnimationByPhaseVar(phaseVar);
	if (animation) {
		auto value = _engine->getGlobal(phaseVar);
		if (value >= -1) {
			if (!animation->ended() || value == -1) {
				animation->decodeNextFrame();
				_engine->setGlobal(phaseVar, animation->phase() - 1);
			} else {
				_engine->setGlobal(phaseVar, -1);
			}
		}
	} else
		warning("no animation with phase var %s found", phaseVar.c_str());
}

void Process::signalAnimationEnd() {
	Common::String phaseVar = popString();
	debug("signalAnimationEnd %s", phaseVar.c_str());
	if (phaseVar.empty()) {
		warning("no phaseVar");
		return;
	}
	auto animation = _engine->findAnimationByPhaseVar(phaseVar);
	if (animation) {
		_engine->setGlobal(phaseVar, -2);
	} else {
		warning("no animation with phase var %s", phaseVar.c_str());
	}
}

void Process::setShadowIntensity() {
	int value = pop();
	debug("setShadowIntensity: %d", value);
	_engine->shadowIntensity(value);
}

void Process::pauseAnimation() {
	int arg = pop();
	Common::String phaseVar = popString();
	auto animation = _engine->findAnimationByPhaseVar(phaseVar);
	debug("pauseAnimation: phaseVar %s, arg %d", phaseVar.c_str(), arg);
	if (animation) {
		animation->pause();
		if (arg > 0) {
			// 1, 2 stop (2 with rewind)
			animation->onScreen(false);
			if (arg == 2) {
				animation->rewind();
				_engine->setGlobal(phaseVar, 0);
			}
		}
	}
}

void Process::setTileSize() {
	_tileHeight = pop();
	_tileWidth = pop();
	debug("setTileSize %d %d", _tileWidth, _tileHeight);
}

void Process::generateRegion() {
	Common::String name = popString();
	debug("generateRegion %s %d %d %d %d", name.c_str(), _animationPosition.x, _animationPosition.y, _tileWidth, _tileHeight);
	Common::Rect rect(_tileWidth, _tileHeight);
	rect.translate(_animationPosition.x, _animationPosition.y);
	RegionPtr region(new Region(rect));
	debug("result region: %s", region->toString().c_str());
	ObjectPtr object = _engine->getCurrentScreen()->find(name);
	if (object)
		_object->region(region);
	else
		warning("no object found");
}

void Process::setObjectTile() {
	Common::String name = popString();
	debug("setObjectTile: %s, tile: %d %dx%d, resource: %d", name.c_str(), _tileIndex, _tileWidth, _tileHeight, _tileResource);

	ObjectPtr object = _engine->getCurrentScreen()->find(name);
	if (!object) {
		warning("could not find object %s in screen", name.c_str());
		return;
	}

	if (_tileResource < 0) {
		int x = _animationPosition.x;
		int y = _animationPosition.y;
		object->srcRect(Common::Rect(x, y, x + _tileWidth, y + _tileHeight));
		return;
	}
	if (_tileHeight <= 0 || _tileWidth <= 0) {
		warning("invalid tile size");
		return;
	}
	Graphics::ManagedSurface *surface = _engine->loadFromCache(_tileResource);
	if (!surface) {
		warning("picture %d was not loaded", _tileResource);
		return;
	}

	int tw = surface->w / _tileWidth;
	int y = _tileIndex / tw;
	int x = _tileIndex % tw;
	debug("tile coordinate %dx%d", x, y);

	Graphics::ManagedSurface *tile = new Graphics::ManagedSurface();
	tile->create(_tileWidth, _tileHeight, surface->format);
	tile->surfacePtr()->applyColorKey(0xff, 0, 0xff);
	Common::Rect srcRect(_tileWidth, _tileHeight);
	srcRect.translate(x * _tileWidth, y * _tileHeight);
	surface->blendBlitTo(*tile, 0, 0, Graphics::FLIP_NONE, &srcRect);
	object->setPicture(tile);
}

void Process::setObjectText() {
	int arg3 = pop();
	Common::String text = popText();
	Common::String name = popString();
	debug("setObjectText %s \"%s\" %d", name.c_str(), text.c_str(), arg3);
	ObjectPtr object = _engine->getCurrentScreenObject(name);
	if (object)
		object->setText(text);
	else
		warning("setObjectText: object %s not found", name.c_str());
}

void Process::exitProcess() {
	debug("exitProcess");
	done();
	_exitCode = kExitCodeDestroy;
	_exited = true;
}

void Process::startNewGame() {
	suspend(kExitCodeNewGame);
}

void Process::clearScreen() {
	debug("clearScreen");
}

void Process::moveScreenObject() {
	int y = pop();
	int x = pop();
	Common::String name = popString();
	debug("moveScreenObject %s %d %d", name.c_str(), x, y);
	ObjectPtr object = _engine->getCurrentScreenObject(name);
	if (object)
		object->moveTo(Common::Point(x, y));
	else
		warning("moveScreenObject: object %s not found", name.c_str());
}

void Process::quit() {
	debug("quit");
	_engine->quitGame();
}

void Process::setDialogForNextFilm() {
	_filmSubtitlesResource = pop();
	debug("setDialogForNextFilm %d", _filmSubtitlesResource);
}

void Process::tell(bool npc, const Common::String &sound) {
	Common::String text = popText();
	Common::String region = popString();
	debug("%s '%s' '%s' '%s'", npc ? "npcSay" : "playerSay", region.c_str(), text.c_str(), sound.c_str());
	_engine->tell(*this, region, text, sound, npc);

	// close inventory here if close flag was set
	Common::String inventoryClose = _engine->getSystemVariable("inv_close")->getString();
	suspend(!inventoryClose.empty() ? kExitCodeCloseInventory : kExitCodeSuspend);
}

void Process::npcSay() {
	tell(true, popText());
}

void Process::npcSayNoSound() {
	tell(true);
}

void Process::playerSay122() {
	tell(false, popText());
}

void Process::playerSay120() {
	tell(false, Common::String());
}

void Process::playerSay125() {
	playerSay120(); // same case
}

void Process::stub241() {
	auto color = popColor();
	debug("stub241 #%08x", color);
}

void Process::stub276() {
	auto color = popColor();
	debug("stub276 #%08x", color);
}

void Process::stub275() {
	auto x = pop();
	debug("stub275: %d", x);
}

void Process::setCamera() {
	auto lens = pop();
	auto unk = pop();
	auto camera = pop();
	debug("setCamera %d %d %d", camera, unk, lens);
}

void Process::setFog() {
	auto unk2 = pop();
	auto unk1 = pop();
	debug("setFog %d %d: stub", unk1, unk2);
}

void Process::stub251() {
	auto arg3 = popString();
	auto arg2 = popString();
	auto arg1 = popString();
	debug("stub251 %s %s %s", arg1.c_str(), arg2.c_str(), arg3.c_str());
}

void Process::stub257() {
	auto arg2 = pop();
	auto arg1 = pop();
	debug("stub257 %d %d", arg1, arg2);
}

void Process::stub258() {
	auto arg = pop();
	debug("stub258 %d - rotation?", arg);
}

void Process::stub264() {
	auto color = popColor();
	debug("stub264: %08x", color);
}

void Process::stub265() {
	auto color = popColor();
	debug("stub265: %08x", color);
}

void Process::stub266() {
	auto color = popColor();
	debug("stub265: %08x", color);
}

void Process::stub267() {
	auto color = popColor();
	debug("stub267: %08x", color);
}

void Process::stub278() {
	auto arg = popString();
	debug("stub278 %s", arg.c_str());
}

void Process::stub279() {
	auto arg = popString();
	debug("stub279 %s", arg.c_str());
}

void Process::stub280() {
	auto arg2 = pop();
	auto arg1 = popString();
	debug("stub280 %s %d", arg1.c_str(), arg2);
}

void Process::stub284() {
	debug("stub284");
}

void Process::characterStub253() {
	auto arg3 = pop();
	auto arg2 = popString();
	auto arg1 = popString();
	debug("characterStub253: %s %s %d", arg1.c_str(), arg2.c_str(), arg3);
}

void Process::stub291() {
	auto arg = popString();
	debug("stub291: %s @ignore @ignore", arg.c_str());
}

void Process::stub292() {
	auto arg2 = popString();
	auto arg1 = popString();
	debug("stub292: %s %s", arg1.c_str(), arg2.c_str());
}

void Process::loadDialog() {
	Common::String arg3 = popString();
	Common::String arg2 = popString();
	Common::String arg1 = popString();
	debug("loadDialog %s %s %s", arg1.c_str(), arg2.c_str(), arg3.c_str());
	arg2 = _engine->loadText(arg2);
	arg3 = _engine->loadText(arg3);

	_engine->dialog().load(getName(), arg2, arg3);

	suspend(kExitCodeRunDialog, arg1);
}

void Process::setNPCTellNotifyVar() {
	Common::String name = popString();
	debug("setNPCTellNotifyVar %s", name.c_str());
	_engine->textLayout().setNPCNotifyVar(name);
	_engine->setGlobal(name, 0);
}

void Process::getObjectPictureWidth() {
	Common::String name = popString();
	debug("getObjectPictureWidth %s", name.c_str());
	ObjectPtr object = _engine->getCurrentScreenObject(name);
	if (object) {
		const auto *picture = object->getPicture();
		int32 value = picture ? picture->w : 0;
		debug("\t->%d", value);
		push(value);
	} else {
		warning("getObjectPictureWidth: object %s not found", name.c_str());
		push((int32)0);
	}
}

void Process::getObjectPictureHeight() {
	Common::String name = popString();
	debug("getObjectPictureHeight %s", name.c_str());
	ObjectPtr object = _engine->getCurrentScreenObject(name);
	if (object) {
		const auto *picture = object->getPicture();
		int32 value = picture ? picture->h : 0;
		debug("\t->%d", value);
		push(value);
	} else {
		warning("getObjectPictureHeight: object %s not found", name.c_str());
		push((int32)0);
	}
}

void Process::playFilm() {
	Common::String audio = popText();
	Common::String video = popText();
	Common::String subtitles = _engine->loadText(getString(_filmSubtitlesResource));

	debug("playFilm %s %s %s", video.c_str(), audio.c_str(), subtitles.c_str());
	_engine->playFilm(*this, video, audio, subtitles);
	suspendIfPassive();
}

void Process::inventoryClear() {
	debug("inventoryClear");
	_engine->inventory().clear();
}

void Process::inventoryAddObject() {
	Common::String name = popString();
	debug("inventoryAddObject %s", name.c_str());
	suspend(kExitCodeLoadInventoryObject, name);
}

void Process::inventoryRemoveObject() {
	Common::String name = popString();
	debug("inventoryRemoveObject %s", name.c_str());
	_object->lock();
	_engine->inventory().remove(name);
	_engine->getCurrentScreen()->remove(name);
	_object->unlock();
}

void Process::inventoryFindObjectByName() {
	Common::String name = popString();
	debug("inventoryFindObjectByName %s", name.c_str());
	int32 index = _engine->inventory().find(name);
	debug("\t->%d", index);
	push(index);
}

void Process::inventoryHasObjectByName() {
	Common::String name = popString();
	debug("inventoryHasObjectByName %s", name.c_str());
	bool hasObject = _engine->inventory().find(name) >= 0;
	debug("\t->%d", hasObject);
	push(hasObject);
}

void Process::inventoryHasObject() {
	int index = pop();
	bool hasObject = _engine->inventory().has(index);
	debug("inventoryHasObject %d -> %d", index, hasObject);
	push(hasObject);
}

void Process::getMaxInventorySize() {
	int32 size = _engine->inventory().maxSize();
	debug("getMaxInventorySize -> %d", size);
	push(size);
}

void Process::getInventoryFreeSpace() {
	int32 size = _engine->inventory().free();
	debug("getInventoryFreeSpace -> %d", size);
	push(size);
}

void Process::appendInventoryObjectNameToSharedSpace() {
	int index = pop();
	debug("appendInventoryObjectNameToSharedSpace %d", index);
	ObjectPtr object = _engine->inventory().get(index);
	push((int32)_engine->appendToSharedStorage(object ? object->getName() : Common::String()));
}

void Process::setNextScreen() {
	Common::String name = popString();
	debug("exitProcessSetNextScreen %s", name.c_str());
	suspend(kExitCodeSetNextScreen, name);
}

void Process::setNextScreenSaveOrLoad() {
	Common::String name = popString();
	debug("exitProcessSetNextScreenSaveOrLoad %s", name.c_str());
	suspend(kExitCodeSetNextScreenSaveOrLoad, name);
}

void Process::loadPreviousScreen() {
	debug("loadPreviousScreen");
	_engine->returnToPreviousScreen();
	suspend();
}

void Process::disableInventory() {
	debug("disableInventory");
	_engine->inventory().enable(false);
}

void Process::enableInventory() {
	debug("enableInventory");
	_engine->inventory().enable(true);
}

void Process::setObjectZ() {
	int z = pop();
	debug("setObjectZ %d", z);
	_object->z(z);
	_engine->getCurrentScreen()->update(_object);
}

void Process::setScreenBackground() {
	debug("setScreenBackground");
	_object->z(g_system->getHeight());
	_engine->getCurrentScreen()->update(_object);
	_engine->getCurrentScreen()->setBackground(_object.get());
}

void Process::loadTextFromObject() {
	Common::String text = popText();
	debug("loadTextFromObject %s", text.c_str());
	_object->title(text);
}

void Process::initialise(uint16 addr) {
	debug("initialise %04x", _ip);
	if (_object->allowInitialise()) {
		_engine->runProcess(_object, _ip);
	} else
		debug("initialise skipped (recovered object)");
	_ip += addr;
}

void Process::onKey(uint16 size) {
	Common::String key = popString();
	debug("onKey %s [handler], %u instructions, ip: 0x%04x", key.c_str(), size, _ip);
	_object->setKeyHandler(key, _ip);
	_ip += size;
}

void Process::onUse(uint16 size) {
	debug("lclick [handler], %u instructions, ip: 0x%04x", size, _ip);
	_object->setClickHandler(_ip);
	_ip += size;
}

void Process::onObjectC1(uint16 size) {
	debug("lclick [fallback for attached inventory object] [handler] stub, %u instructions, ip: 0x%04x", size, _ip);
	_object->setHandlerC1(_ip);
	_ip += size;
}

void Process::onLook(uint16 size) {
	debug("look [handler], %u instructions, ip: 0x%04x", size, _ip);
	_object->setExamineHandler(_ip);
	_ip += size;
}

void Process::onCharacterTrap(uint16 size) {
	auto regionName = popString();
	debug("setCharacterTrap %s [handler], %u instructions, ip: 0x%04x", regionName.c_str(), size, _ip);
	_object->setTrapHandler(_ip, _engine->loadRegion(regionName));
	_ip += size;
}

void Process::onObjectBD(uint16 size) {
	debug("onObject(+BD) [handler] stub, %u instructions, ip: 0x%04x", size, _ip);
	_object->setHandlerBD(_ip);
	_ip += size;
}

void Process::enableUser() {
	// screen loading block user interaction until this instruction
	debug("enableUser");
	_engine->enableUser(true);
}

void Process::addMouseArea() {
	Common::String onLeave = popString();
	Common::String onEnter = popString();
	Common::String name = popString();

	debug("addMouseArea (region: %s) %s %s", name.c_str(), onEnter.c_str(), onLeave.c_str());
	RegionPtr region = _engine->loadRegion(name);
	debug("region: %s", region->toString().c_str());

	int32 value = _engine->_mouseMap.add(MouseRegion(region, onEnter, onLeave));
	debug("\tmouse area id -> %d", value);
	push(value);
}

void Process::loadCharacter() {
	Common::String object = popString();
	Common::String filename = popString();
	Common::String name = popString();
	_engine->loadCharacter(name, filename, object);
}

void Process::associateCharacter() {
	Common::String object = popString();
	Common::String name = popString();
	debug("associateCharacter %s %s", name.c_str(), object.c_str());
	Character *character = _engine->getCharacter(name);
	if (character)
		character->associate(name);
	else
		warning("character %s could not be found", name.c_str());
}

void Process::enableCharacter() {
	Common::String name = popString();
	debug("enableCharacter %s", name.c_str());
	Character *character = _engine->getCharacter(name);
	if (character)
		character->enable();
	else
		warning("character %s could not be found", name.c_str());
}

void Process::moveCharacter(bool usermove) {
	int direction = pop();
	Common::String regionName = popString();
	Common::String id = popString();
	debug("moveCharacter %s %s, direction: %d, usermove: %d", id.c_str(), regionName.c_str(), direction, usermove);
	Character *character = _engine->getCharacter(id);
	if (character) {
		auto region = _engine->loadRegion(regionName);
		if (region) {
			if (character->moveTo(_object->getName(), region->center, direction)) {
				deactivate();
			}
		}
	} else
		warning("character %s could not be found", id.c_str());
	suspendIfPassive();
}

void Process::moveCharacterUserMove() {
	moveCharacter(true);
}

void Process::moveCharacterNoUserMove() {
	moveCharacter(false);
}

void Process::animateCharacter() {
	int direction = pop();
	Common::String name = popString();
	debug("animateCharacter: %s %d %d", name.c_str(), direction, _animationSpeed);

	Character *character = _engine->getCharacter(name);
	if (character) {
		character->animate(_animationPosition, direction, _animationSpeed);
	} else
		warning("character %s could not be found", name.c_str());
}

void Process::showCharacter() {
	Common::String name = popString();
	debug("showCharacter %s", name.c_str());
	Character *character = _engine->getCharacter(name);
	if (character) {
		character->visible(true);
		_engine->runObject(character->object());
	} else
		warning("character %s could not be found", name.c_str());
}

void Process::hideCharacter() {
	Common::String name = popString();
	debug("hideCharacter %s", name.c_str());
	Character *character = _engine->getCharacter(name);
	if (character) {
		character->visible(false);
	} else
		warning("character %s could not be found", name.c_str());
}

void Process::leaveCharacter(const Common::String &name, const Common::String &regionName, int dir) {
	debug("leaveCharacter %s %s %d", name.c_str(), regionName.c_str(), dir);
	Character *character = _engine->getCharacter(name);
	if (character) {
		RegionPtr region = _engine->loadRegion(regionName);
		debug("region: %s", region->toString().c_str());
		if (character->moveTo(getName(), region->center, dir))
			deactivate();
	} else
		warning("character %s could not be found", name.c_str());
	suspendIfPassive();
}

void Process::leaveCharacter() {
	Common::String regionName = popString();
	Common::String name = popString();
	leaveCharacter(name, regionName, -1);
}

void Process::leaveCharacterEx() {
	int dir = pop();
	Common::String regionName = popString();
	Common::String name = popString();
	leaveCharacter(name, regionName, dir);
}

void Process::setCharacter() {
	int dir = pop();
	Common::String regionName = popString();
	Common::String id = popString();
	debug("setCharacter %s %s %d", id.c_str(), regionName.c_str(), dir);
	auto character = _engine->getCharacter(id);
	if (character) {
		if (dir == -1)
			dir = character->direction();
		auto region = _engine->loadRegion(regionName);
		if (region) {
			debug("setting character position to %d,%d", region->center.x, region->center.y);
			character->position(region->center);
		} else
			warning("no region %s", regionName.c_str());
		character->direction(dir);
	} else
		warning("no character %s", id.c_str());
}

void Process::setCharacterDirection() {
	int dir = pop();
	Common::String name = popString();
	debug("setCharacterDirection %s %d", name.c_str(), dir);
	auto character = _engine->getCharacter(name);
	if (character) {
		character->direction(dir);
	} else
		warning("no character %s", name.c_str());
	suspendIfPassive();
}

void Process::pointCharacter() {
	Common::String regionName = popString();
	Common::String id = popString();
	debug("pointCharacter %s, region: %s", id.c_str(), regionName.c_str());
	Character *character = _engine->getCharacter(id);
	if (character) {
		auto region = _engine->loadRegion(regionName);
		if (region) {
			deactivate();
			// Point to is stub, and it reactivates process based on internal state.
			character->pointTo(getName(), region->center);
		}
	} else
		warning("character %s could not be found", id.c_str());
	suspendIfPassive();
}

void Process::getCharacterAnimationPhase() {
	Common::String name = popString();
	debug("getCharacterAnimationPhase: %s", name.c_str());
	Character *character = _engine->getCharacter(name);
	if (!character)
		warning("no character %s", name.c_str());
	int32 phase = character ? character->phase() : -1;
	debug("animation phase = %d", phase);
	push(phase);
}
void Process::getCharacterX() {
	Common::String name = popString();
	debug("getCharacterX: %s", name.c_str());
	Character *character = _engine->getCharacter(name);
	if (!character)
		warning("no character %s", name.c_str());
	int32 value = character ? character->position().x : -1;
	push(value);
}
void Process::getCharacterY() {
	Common::String name = popString();
	debug("getCharacterY: %s", name.c_str());
	Character *character = _engine->getCharacter(name);
	if (!character)
		warning("no character %s", name.c_str());
	int32 value = character ? character->position().y : -1;
	push(value);
}

void Process::stopCharacter() {
	int direction = pop();
	Common::String name = popString();
	debug("stopCharacter: %s, direction: %d", name.c_str(), direction);
	Character *character = _engine->getCharacter(name);
	if (character) {
		if (direction != -1) {
			character->direction(direction);
			character->notifyProcess(getName());
			deactivate();
		} else {
			character->stop();
		}
	} else
		warning("could not find character %s", name.c_str());
	suspendIfPassive();
}

void Process::fogOnCharacter() {
	int arg2 = pop();
	int arg1 = pop();
	Common::String name = popText();
	debug("fogOnCharacter %s z: [%d,%d]", name.c_str(), arg1, arg2);
	Character *character = _engine->currentCharacter();
	if (character)
		character->setFog(_engine->loadPicture(name), arg1, arg2);
}

void Process::setRain() {
	Common::String name = popString();
	debug("setRain stub: %s", name.c_str());
}

void Process::setRainDensity() {
	int change = pop();
	int density = pop();
	debug("setRainDensity stub: %d (change: %d)", density, change);
}

void Process::loadRegionFromObject() {
	Common::String name = popString();
	debug("loadRegionFromObject %s", name.c_str());
	RegionPtr region = _engine->loadRegion(name);
	debug("region: %s", region->toString().c_str());
	_object->region(region);
}

void Process::loadPictureFromObject() {
	Common::String name = popText();
	debug("loadPictureFromObject %s", name.c_str());
	_object->setPicture(_engine->loadPicture(name));
}

void Process::loadAnimationFromObject() {
	Common::String name = popText();
	debug("loadAnimationFromObject %s %s", name.c_str(), _phaseVarControlled ? "(phase-var)" : "");
	auto animation = _engine->loadAnimation(name);
	if (animation) {
		_animationCycles = 0;
		_animationLoop = true;
		_phaseVar.clear();
		_object->setAnimation(animation);
		setupAnimation(animation);
	}
}

void Process::setAnimationPosition() {
	int arg2 = pop();
	int arg1 = pop();
	debug("setAnimationPosition %d %d", arg1, arg2);
	_animationPosition.x = arg1;
	_animationPosition.y = arg2;
}

void Process::setTimer() {
	int value = pop();
	debug("setTimer %d", value);
	_timer = value;
	suspend();
}

void Process::stub231() {
	int arg2 = pop();
	int arg1 = pop();
	debug("stub231 %d %d", arg1, arg2);
}

void Process::objectFreePictureAndAnimation() {
	Common::String name = popString();
	debug("objectFreePictureAndAnimation %s", name.c_str());
	ObjectPtr object = _engine->getCurrentScreenObject(name);
	if (object) {
		object->setAnimation(nullptr);
		object->setPicture(nullptr);
	}
}

void Process::fadeScreen() {
	int fadeMusic = pop();
	int fadeSound = pop();
	int fadeScreen = pop();
	debug("fadeScreen screen: %d, sound: %d music: %d", fadeScreen, fadeSound, fadeMusic);
	_engine->curtain(getName(), fadeScreen, fadeSound, fadeMusic, true);
	deactivate();
	suspendIfPassive();
}

void Process::setCharacterNotifyVars() {
	Common::String arg2 = popString();
	Common::String arg1 = popString();

	debug("setCharacterNotifyVars, tell: %s, direction: %s", arg1.c_str(), arg2.c_str());
	auto character = _engine->currentCharacter();
	_engine->setGlobal(arg1, 0);
	_engine->setGlobal(arg2, character ? character->direction() : 0);
	_engine->textLayout().setCharNotifyVar(arg1);
	_engine->textLayout().setCharDirectionNotifyVar(arg2);
}

} // namespace AGDS
