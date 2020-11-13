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

#include "agds/agds.h"
#include "agds/animation.h"
#include "agds/character.h"
#include "agds/font.h"
#include "agds/opcode.h"
#include "agds/process.h"
#include "agds/region.h"
#include "agds/screen.h"
#include "agds/systemVariable.h"
#include "common/debug.h"
#include "common/savefile.h"
#include "common/system.h"
#include "graphics/transparent_surface.h"

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
	      " resources table with %u entries",
	      unk1, unk2, unk3, unk4, resCount);

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
	int value = pop();
	Common::String name = popString();
	debug("setIntegerSystemVariable: %s -> %d", name.c_str(), value);
	_engine->getSystemVariable(name)->setInteger(value);
}

void Process::getIntegerSystemVariable() {
	Common::String name = popString();
	int value = _engine->getSystemVariable(name)->getInteger();
	debug("getIntegerSystemVariable: %s -> %d", name.c_str(), value);
	push(value);
}

void Process::getRegionCenterX() {
	Common::String name = popString();
	RegionPtr reg = _engine->loadRegion(name);
	int value = reg->center.x;
	push(value);
	debug("getRegionCenterX %s -> %d", name.c_str(), value);
}

void Process::getRegionCenterY() {
	Common::String name = popString();
	RegionPtr reg = _engine->loadRegion(name);
	int value = reg->center.y;
	push(value);
	debug("getRegionCenterY %s -> %d", name.c_str(), value);
}

void Process::getObjectId() {
	const Common::String &name = _object->getName();
	//no rfind :(((
	Common::String::const_iterator dotpos = 0;
	for (Common::String::const_iterator i = name.begin(); i != name.end(); ++i)
		if (*i == '.')
			dotpos = i + 1;
	Common::String id(dotpos, name.end());
	int value = atoi(id.c_str());
	debug("getObjectId %s %d", name.c_str(), value);
	push(value);
}

void Process::loadPicture() {
	Common::String name = popText();
	int cacheId = _engine->loadFromCache(name);
	if (cacheId < 0) {
		cacheId = _engine->saveToCache(name, _engine->loadPicture(name));
	}
	debug("loadPicture %s -> %d", name.c_str(), cacheId);
	push(cacheId);
}

void Process::loadAnimation() {
	Common::String name = popText();
	debug("loadAnimation %s (phase: %s) %s", name.c_str(), _phaseVar.c_str(), _phaseVarControlled? "(phase-var)": "");
	Animation *animation = _engine->loadAnimation(name);
	if (animation) {
		setupAnimation(animation);
		_engine->getCurrentScreen()->add(animation);
	}
}

void Process::loadSample() {
	Common::String name = popText();
	debug("loadSample %s, phaseVar: %s", name.c_str(), _phaseVar.c_str());
	_engine->playSound(getName(), name, _phaseVar);
}

void Process::getSampleVolume() {
	Common::String name = popString();
	debug("getSampleVolume: %s", name.c_str());
	auto sound = _engine->soundManager().findSampleByPhaseVar(name);
	if (sound) {
		debug("\treturning %d", sound->leftVolume);
		push(sound->leftVolume);
	} else {
		warning("could not find sample %s", name.c_str());
		push(-1);
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
	sound->leftVolume = l;
	sound->rightVolume = r;
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
		debug("sample found (%s)", sound->name.c_str());
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
		debug("sample found (%s)", sound->name.c_str());
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
	debug("removeScreenObject: %s", name.c_str());
	Screen *screen = _engine->getCurrentScreen();
	if (screen) {
		if (!screen->remove(name))
			warning("removeScreenObject: object %s not found", name.c_str());
	}
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
	_engine->currentInventoryObject(ObjectPtr());
}

void Process::getRandomNumber() {
	int max = pop();
	int value = max > 0 ? _engine->_random.getRandomNumber(max - 1) : 0;
	debug("random %d -> %d", max, value);
	push(value);
}

void Process::setGlobal() {
	Common::String name = popString();
	int value = pop();
	_engine->setGlobal(name, value);
}

void Process::setPhaseVar() {
	Common::String name = popString();
	_engine->setGlobal(name, 0);
	_phaseVar = name;
	debug("setPhaseVar %s", name.c_str());
}

void Process::getGlobal(uint8 index) {
	const Common::String &name = _object->getString(index).string;
	int value = _engine->getGlobal(name);
	debug("get global %u %s -> %d", index, name.c_str(), value);
	push(value);
}

void Process::hasGlobal() {
	Common::String name = popString();
	int result = _engine->hasGlobal(name) ? 1 : 0;
	debug("hasGlobal %s %d", name.c_str(), result);
	push(result);
}

void Process::postIncrementGlobal() {
	Common::String name = popString();
	int value = _engine->getGlobal(name);
	debug("post-increment global %s %d", name.c_str(), value);
	push(value);
	_engine->setGlobal(name, value + 1);
}

void Process::postDecrementGlobal() {
	Common::String name = popString();
	int value = _engine->getGlobal(name);
	debug("post-decrement global %s %d", name.c_str(), value);
	push(value);
	_engine->setGlobal(name, value - 1);
}

void Process::incrementGlobal(int inc) {
	Common::String name = popString();
	int value = _engine->getGlobal(name);
	debug("increment global %s %d by %d", name.c_str(), value, inc);
	_engine->setGlobal(name, value + inc);
}

void Process::decrementGlobal(int dec) {
	Common::String name = popString();
	int value = _engine->getGlobal(name);
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
	int index = _engine->appendToSharedStorage(value);
	//debug("appendToSharedStorage %s -> %d", value.c_str(), index);
	push(index);
}

void Process::appendNameToSharedStorage() {
	int index = _engine->appendToSharedStorage(_object->getName());
	debug("appendNameToSharedStorage %s -> %d", _object->getName().c_str(), index);
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
		//no substr :(((
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
	push(_engine->getGlobal(name));
}

void Process::setCloneVar() {
	int arg3 = pop();
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
	push(_engine->appendToSharedStorage(name));
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

	if (!screenName.empty()) {
		debug("checkScreenPatch: stub for screen != -1, returning 0");
		//check that patch exist
		push(0);
		return;
	} else {
		ObjectPtr object = screen->find(objectName);
		int value = object && object->inScene();
		debug("checkScreenPatch: current screen object present: %d", value);
		push(value);
	}
}

void Process::loadMouseCursorFromObject() {
	Common::String name = popText();
	debug("loadMouseCursorFromObject %s", name.c_str());
	Animation *cursor = _engine->loadMouseCursor(name);
	_object->setMouseCursor(cursor); //overlay cursor
}

void Process::attachInventoryObjectToMouse() {
	Common::String name = popString();
	debug("attachInventoryObjectToMouse %s", name.c_str());
	auto object = _engine->getCurrentScreenObject(name);
	if (object)
		_engine->currentInventoryObject(object);
	else
		warning("cannot find object %s", name.c_str());
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
	debug("register use object handler %s -> 0x%04x", arg.c_str(), _ip);
	_object->setUseHandler(arg, _ip);
	_ip += size;
}

void Process::onObjectUserUse(uint16 size) {
	debug("register user use handler -> 0x%04x", _ip);
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

void Process::stub82() {
	Common::String arg2 = popString();
	Common::String arg1 = popString();
	debug("stub82: %s %s", arg1.c_str(), arg2.c_str());
}

void Process::stub83() {
	Common::String arg2 = popString();
	Common::String arg1 = popString();
	debug("stub83: %s %s", arg1.c_str(), arg2.c_str());
}

void Process::stub102() {
	Common::String name = popString();
	debug("stub102: load picture? %s", name.c_str());
}

void Process::setAnimationPaused() {
	debug("setAnimationPaused");
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

	_tileWidth = 16;
	_tileHeight = 16;
	_tileIndex = 0;
	_tileResource = 0;

	_filmSubtitlesResource = -1;
}

void Process::setAnimationZ() {
	int z = pop();
	debug("setAnimationZ %d", z);
	_animationZ = z;
}

void Process::setPanAndVolume() {
	int pan = pop();
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

void Process::screenSaveScreenPatch() {
	Common::String objectName = popString();
	Common::String screenName = popString();
	debug("saveScreenPatch stub %s %s", screenName.c_str(), objectName.c_str());
	suspend(kExitCodeLoadScreenObject, objectName);
}

void Process::screenRemoveObjectSavePatch() {
	Common::String objectName = popString();
	Common::String screenName = popString();
	if (screenName != _engine->getCurrentScreenName()) {
		debug("screenRemoveObjectSavePatch semi-stub %s %s", screenName.c_str(), objectName.c_str());
	} else {
		Screen *screen = _engine->getCurrentScreen();
		if (screen) {
			if (!screen->remove(objectName))
				warning("screenRemoveObjectSavePatch: object %s not found", objectName.c_str());
		}
	}
}

void Process::getPictureBaseX() {
	Common::String name = popString();
	debug("getPictureBaseX: %s", name.c_str());
	ObjectPtr object = _engine->getCurrentScreenObject(name);
	int x = object? object->getOffset().x: 0;
	debug("\t%d", x);
	push(x);
}

void Process::getPictureBaseY() {
	Common::String name = popString();
	debug("getPictureBaseY: %s", name.c_str());
	ObjectPtr object = _engine->getCurrentScreenObject(name);
	int y = object? object->getOffset().y: 0;
	debug("\t%d", y);
	push(y);
}

void Process::getObjectSurfaceX() {
	Common::String name = popString();
	debug("getObjectSurfaceX: %s", name.c_str());
	ObjectPtr object = _engine->getCurrentScreenObject(name);
	int x = object? object->getPosition().x: 0;
	debug("\t%d", x);
	push(x);
}

void Process::getObjectSurfaceY() {
	Common::String name = popString();
	debug("getObjectSurfaceY: %s", name.c_str());
	ObjectPtr object = _engine->getCurrentScreenObject(name);
	int y = object? object->getPosition().y: 0;
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
	suspend(kExitCodeLoadSaveGame, saveSlot);
}

void Process::loadSaveSlotNamePicture() {
	int saveSlot = pop();
	debug("loadSaveSlotNamePicture: %d", saveSlot);

	Common::String saveSlotName = _engine->getSaveStateName(saveSlot);
	Common::SaveFileManager * saveMan = _engine->getSaveFileManager();
	Common::InSaveFile * save = saveMan->openForLoading(saveSlotName);
	if (!save) {
		debug("no save in slot %d", saveSlot);
		return;
	}

	debug("save state name: %s", saveSlotName.c_str());
	int fontId = _engine->getSystemVariable("edit_font")->getInteger();

	debug("font id = %d", fontId);
	Font * font = _engine->getFont(fontId);
	int h = font->getFontHeight();
	static const int w = 400;

	Graphics::Surface * label = _engine->createSurface(w, h);
	uint32 color = _engine->pixelFormat().RGBToColor(255, 0, 255);
	label->fillRect(label->getRect(), color);
	font->drawString(label, saveSlotName, 0, 0, w, color);
	Graphics::TransparentSurface *transparentLabel = _engine->convertToTransparent(label);
	_object->setPicture(transparentLabel);
	_object->generateRegion();

	delete save;
}

void Process::stub166() {
	int arg3 = pop();
	int arg2 = pop();
	Common::String arg1 = popString();
	debug("stub166 %s %d %d", arg1.c_str(), arg2, arg3);
}

void Process::stub172() {
	int value = pop();
	debug("stub172: setMusicVolume? %d", value);
}

void Process::stub173() {
	debug("stub173: remove currentCursor");
}

void Process::stub174() {
	debug("stub174: set mouse relative mode");
}

void Process::stub192() {
	int value = pop();
	Common::String name = popString();
	debug("stub192: %s: set some object flag to %d", name.c_str(), value);
}

void Process::stub193() {
	debug("stub193: removing inventory object 0?");
}

void Process::setObjectScale() {
	int value = pop();
	_object->scale(value);
	debug("setObjectScale stub %d", value);
}

void Process::disableMouseAreas() {
	int value = pop();
	debug("disableMouseAreas %d", value);
	_engine->_mouseMap.disable(_engine, value > 0);
}

void Process::stub194() {
	debug("stub194: mute");
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

void Process::stub201(uint16 size) {
	debug("stub201, [handler] %u instructions", size);
	_ip += size;
}

void Process::stub202(uint16 size) {
	debug("stub202, [handler] %u instructions", size);
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
	Animation *animation = _engine->findAnimationByPhaseVar(phaseVar);
	if (animation) {
		if (_engine->getGlobal(phaseVar) == -1 && !animation->paused()) {
			animation->rewind();
		}
		animation->resume();
		animation->decodeNextFrameIfNoFrame(*_engine);
		_engine->setGlobal(phaseVar, animation->phase() - 1);
	} else {
		warning("no animation with phase var %s found", phaseVar.c_str());
		_engine->setGlobal(phaseVar, -1);
	}
}

void Process::stub223() {
	int value = pop();
	debug("stub223: %d", value);
}

void Process::pauseAnimation() {
	int arg = pop();
	Common::String phaseVar = popString();
	Animation *animation = _engine->findAnimationByPhaseVar(phaseVar);
	debug("pauseAnimation: phaseVar %s, arg %d", phaseVar.c_str(), arg);
	if (animation) {
		animation->pause();
		if (arg > 0) {
			//1, 2 stop (2 with rewind)
			animation->freeFrame();
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

	if (_tileResource <= 0) {
		warning("invalid resource id, skipping");
		return;
	}
	if (_tileHeight <= 0 || _tileWidth <= 0) {
		warning("invalid tile size");
		return;
	}
	Graphics::TransparentSurface *surface = _engine->loadFromCache(_tileResource);
	if (!surface) {
		warning("picture %d was not loaded", _tileResource);
		return;
	}

	int tw = surface->w / _tileWidth;
	int y = _tileIndex / tw;
	int x = _tileIndex % tw;
	debug("tile coordinate %dx%d", x, y);

	Graphics::TransparentSurface *tile = new Graphics::TransparentSurface();
	tile->create(_tileWidth, _tileHeight, surface->format);
	tile->applyColorKey(0xff, 0, 0xff);
	Common::Rect srcRect(_tileWidth, _tileHeight);
	srcRect.translate(x * _tileWidth, y * _tileHeight);
	surface->blit(*tile, 0, 0, Graphics::FLIP_NONE, &srcRect);
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
}

void Process::exitProcessCreatePatch() {
	suspend(kExitCodeCreatePatchLoadResources);
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
	if (object) {
		RegionPtr region = object->region();
		if (region)
			debug("object region %s", region->toString().c_str());
		object->moveTo(Common::Point(x, y));
	} else
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
	debug("%s '%s' '%s' '%s'", npc? "npcSay": "playerSay", region.c_str(), text.c_str(), sound.c_str());
	_engine->tell(*this, region, text, sound, npc);

	//close inventory here if close flag was set
	Common::String inventoryClose = _engine->getSystemVariable("inv_close")->getString();
	suspend(!inventoryClose.empty()? kExitCodeCloseInventory: kExitCodeSuspend);
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
	playerSay120(); //same case
}

void Process::loadDialog() {
	Common::String arg3 = popString();
	Common::String arg2 = popString();
	Common::String arg1 = popString();
	debug("loadDialog %s %s %s", arg1.c_str(), arg2.c_str(), arg3.c_str());
	arg2 = _engine->loadText(arg2);
	arg3 = _engine->loadText(arg3);

	_engine->dialog().load(arg2, arg3);

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
		const Graphics::Surface *picture = object->getPicture();
		int value = picture ? picture->w : 0;
		debug("\t->%d", value);
		push(value);
	} else {
		warning("getObjectPictureWidth: object %s not found", name.c_str());
		push(0);
	}
}

void Process::getObjectPictureHeight() {
	Common::String name = popString();
	debug("getObjectPictureHeight %s", name.c_str());
	ObjectPtr object = _engine->getCurrentScreenObject(name);
	if (object) {
		const Graphics::Surface *picture = object->getPicture();
		int value = picture ? picture->h : 0;
		debug("\t->%d", value);
		push(value);
	} else {
		warning("getObjectPictureHeight: object %s not found", name.c_str());
		push(0);
	}
}

void Process::playFilm() {
	Common::String audio = popText();
	Common::String video = popText();
	Common::String subtitles = _engine->loadText(getString(_filmSubtitlesResource));

	debug("playFilm %s %s %s", video.c_str(), audio.c_str(), subtitles.c_str());
	suspend();
	_engine->playFilm(*this, video, audio, subtitles);
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
	_engine->inventory().remove(name);
}

void Process::inventoryFindObjectByName() {
	Common::String name = popString();
	debug("inventoryFindObjectByName %s", name.c_str());
	int index = _engine->inventory().find(name);
	debug("\t->%d", index);
	push(index);
}

void Process::inventoryHasObject() {
	int index = pop();
	debug("inventoryHasObject %d", index);
	bool hasObject = _engine->inventory().get(index);
	debug("\t->%d", hasObject);
	push(hasObject);
}

void Process::getMaxInventorySize() {
	int size = _engine->inventory().maxSize();
	debug("getMaxInventorySize -> %d", size);
	push(size);
}

void Process::getInventoryFreeSpace() {
	int size = _engine->inventory().free();
	debug("getInventoryFreeSpace -> %d", size);
	push(size);
}

void Process::appendInventoryObjectNameToSharedSpace() {
	int index = pop();
	debug("appendInventoryObjectNameToSharedSpace %d", index);
	ObjectPtr object = _engine->inventory().get(index);
	push(_engine->appendToSharedStorage(object ? object->getName() : Common::String()));
}

void Process::setNextScreen() {
	Common::String name = popString();
	debug("exitProcessSetNextScreen %s", name.c_str());
	suspend(kExitCodeSetNextScreen, name);
}

void Process::setNextScreenSaveInHistory() {
	Common::String name = popString();
	debug("setNextScreenSaveInHistory %s", name.c_str());
	suspend(kExitCodeSetNextScreenSaveInHistory, name);
}

void Process::loadPreviousScreen() {
	debug("loadPreviousScreen");
	suspend(kExitCodeLoadPreviousScreenObject);
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

void Process::updateScreenHeightToDisplay() {
	debug("updateObjectZtoDisplayHeight");
	_object->z(g_system->getHeight());
	_engine->getCurrentScreen()->update(_object);
}

void Process::loadTextFromObject() {
	Common::String text = popText();
	debug("loadTextFromObject %s", text.c_str());
	_object->title(text);
}

void Process::call(uint16 addr) {
	debug("call %04x", addr);
	_engine->runProcess(_object, _ip + addr);
}

void Process::onKey(uint16 size) {
	Common::String key = popString();
	debug("onKey %s [handler], %u instructions", key.c_str(), size);
	_object->setKeyHandler(key, _ip);
	_ip += size;
}

void Process::onUse(uint16 size) {
	debug("lclick [handler], %u instructions", size);
	_object->setClickHandler(_ip);
	_ip += size;
}

void Process::onObjectC1(uint16 size) {
	debug("unknown (0xc1) [handler], %u instructions", size);
	_ip += size;
}

void Process::onLook(uint16 size) {
	debug("look? [handler], %u instructions", size);
	_object->setExamineHandler(_ip);
	_ip += size;
}

void Process::onObjectB9(uint16 size) {
	debug("onObject(+B9) [handler], %u instructions", size);
	_ip += size;
}

void Process::onObjectBD(uint16 size) {
	debug("onObject(+BD) [handler], %u instructions", size);
	_ip += size;
}

void Process::enableUser() {
	//screen loading block user interaction until this instruction
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

	int value = _engine->_mouseMap.add(MouseRegion(region, onEnter, onLeave));
	debug("\tmouse area id -> %d", value);
	push(value);
}

void Process::loadCharacter() {
	Common::String object = popString();
	Common::String chrFilename = popText();
	Common::String id = popString();
	_engine->loadCharacter(id, chrFilename, object);
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
			character->moveTo(region->center, direction);
		}
	} else
		warning("character %s could not be found", id.c_str());
	if (_status == kStatusPassive)
		suspend();
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
	if (character)
		character->animate(_animationPosition, direction, _animationSpeed);
	else
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

void Process::leaveCharacter() {
	Common::String arg2 = popString();
	Common::String arg1 = popString();
	debug("leaveCharacter %s %s", arg1.c_str(), arg2.c_str());
	RegionPtr region = _engine->loadRegion(arg2);
	debug("region: %s", region->toString().c_str());
}
void Process::leaveCharacterEx() {
	int arg3 = pop();
	Common::String arg2 = popString();
	Common::String arg1 = popString();
	debug("leaveCharacterEx %s %s %d", arg1.c_str(), arg2.c_str(), arg3);
}

void Process::setCharacter() {
	int dir = pop();
	Common::String regionName = popString();
	Common::String id = popString();
	debug("setCharacter %s %s %d", id.c_str(), regionName.c_str(), dir);
	auto character = _engine->getCharacter(id);
	if (character) {
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
	debug("setCharacterDirection stub %s %d", name.c_str(), dir);
}

void Process::pointCharacter() {
	Common::String arg2 = popString();
	Common::String arg1 = popString();
	debug("pointCharacter stub %s %s", arg1.c_str(), arg2.c_str());
	suspend();
}

void Process::getCharacterAnimationPhase() {
	Common::String name = popString();
	debug("getCharacterAnimationPhase: %s", name.c_str());
	Character *character = _engine->getCharacter(name);
	if (!character)
		warning("no character %s", name.c_str());
	int phase = character ? character->getPhase() : -1;
	debug("animation phase = %d", phase);
	push(phase);
}
void Process::getCharacterX() {
	Common::String name = popString();
	debug("getCharacterX: %s", name.c_str());
	Character *character = _engine->getCharacter(name);
	if (!character)
		warning("no character %s", name.c_str());
	int value = character? character->position().x: -1;
	push(value);
}
void Process::getCharacterY() {
	Common::String name = popString();
	debug("getCharacterY: %s", name.c_str());
	Character *character = _engine->getCharacter(name);
	if (!character)
		warning("no character %s", name.c_str());
	int value = character? character->position().y: -1;
	push(value);
}

void Process::stopCharacter() {
	int arg = pop();
	Common::String name = popString();
	debug("stopCharacter: stub %s %d", name.c_str(), arg);
	Character *character = _engine->getCharacter(name);
	if (character)
		character->stop();
	else
		warning("could not find character %s", name.c_str());
}

void Process::fogOnCharacter() {
	int arg2 = pop();
	int arg1 = pop();
	Common::String name = popText();
	debug("fogOnCharacter %s %d %d", name.c_str(), arg1, arg2);
}

void Process::setRain() {
	Common::String name = popString();
	debug("setRain stub: %s", name.c_str());
}

void Process::setRainDensity() {
	int density = pop();
	debug("setRainDensity stub: %d", density);

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
	debug("loadAnimationFromObject %s %s", name.c_str(), _phaseVarControlled? "(phase-var)": "");
	Animation *animation = _engine->loadAnimation(name);
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

void Process::stub233() {
	Common::String name = popString();
	debug("stub233 %s unload picture?", name.c_str());
	ObjectPtr object = _engine->getCurrentScreenObject(name);
	if (object) {
		object->setPicture(NULL);
	}
}

void Process::stub235() {
	int fadeMusic = pop();
	int fadeSound = pop();
	int fadeScreen = pop();
	debug("stub235 (fadeScreen) screen: %d, sound: %d music: %d", fadeScreen, fadeSound, fadeMusic);
	_engine->getSystemVariable("screen_curtain")->setInteger(fadeScreen);
	_engine->getSystemVariable("sound_curtain")->setInteger(fadeSound);
	_engine->getSystemVariable("music_curtain")->setInteger(fadeMusic);
}

void Process::setCharacterNotifyVars() {
	Common::String arg2 = popString();
	Common::String arg1 = popString();

	debug("setCharacterNotifyVars, tell: %s, direction: %s", arg1.c_str(), arg2.c_str());
	auto character = _engine->currentCharacter();
	_engine->setGlobal(arg1, 0);
	_engine->setGlobal(arg2, character? character->direction(): 0);
	_engine->textLayout().setCharNotifyVar(arg1);
	_engine->textLayout().setCharDirectionNotifyVar(arg2);
}

//fixme: add trace here
#define AGDS_OP(NAME, METHOD) \
	case NAME:           \
		METHOD();        \
		break;

#define AGDS_OP_C(NAME, METHOD) \
	case NAME: {           \
		int8 arg = next(); \
		METHOD(arg);       \
	} break;

#define AGDS_OP_B(NAME, METHOD)  \
	case NAME: {            \
		uint8 arg = next(); \
		METHOD(arg);        \
	} break;

#define AGDS_OP_W(NAME, METHOD)    \
	case NAME: {              \
		int16 arg = next16(); \
		METHOD(arg);          \
	} break;

#define AGDS_OP_U(NAME, METHOD)     \
	case NAME: {               \
		uint16 arg = next16(); \
		METHOD(arg);           \
	} break;

#define AGDS_OP_UU(NAME, METHOD)     \
	case NAME: {                \
		uint16 arg1 = next16(); \
		uint16 arg2 = next16(); \
		METHOD(arg1, arg2);     \
	} break;

#define AGDS_OP_UD(NAME, METHOD)           \
	case NAME: {                     \
		uint16 arg1 = next16();      \
		uint32 arg2 = next16();      \
		METHOD(static_cast<int32>(arg1 | (arg2 << 16))); \
	} break;


ProcessExitCode Process::resume() {
	_exitCode = kExitCodeDestroy;
	if (_timer) {
		--_timer;
		return kExitCodeSuspend;
	}

	const Object::CodeType &code = _object->getCode();
	while (active() && _ip < code.size()) {
		if (_timer) {
			return kExitCodeSuspend;
		}
		_lastIp = _ip;
		uint8 op = next();
		//debug("CODE %04x: %u", _lastIp, (uint)op);
		switch (op) {
			AGDS_OPCODE_LIST(
				AGDS_OP,
				AGDS_OP_C, AGDS_OP_B,
				AGDS_OP_W, AGDS_OP_U,
				AGDS_OP_UD, AGDS_OP_UU
			)
		default:
			error("%s: %08x: unknown opcode 0x%02x (%u)", _object->getName().c_str(), _ip - 1, (unsigned)op, (unsigned)op);
			fail();
			break;
		}
	}

	if (active()) {
		debug("code ended, exiting...");
	}

	return _exitCode;
}

#define AGDS_DIS(NAME, METHOD) \
	case NAME:           \
		source += Common::String::format("%s\n", #NAME); \
		break;

#define AGDS_DIS_C(NAME, METHOD) \
	case NAME: {           \
		int8 arg = code[ip++]; \
		source += Common::String::format("%s %d\n", #NAME, (int)arg); \
	} break;

#define AGDS_DIS_B(NAME, METHOD)  \
	case NAME: {            \
		uint8 arg = code[ip++]; \
		source += Common::String::format("%s %u\n", #NAME, (uint)arg); \
	} break;

#define AGDS_DIS_W(NAME, METHOD)    \
	case NAME: {              \
		int16 arg = code[ip++]; arg |= ((int16)code[ip++]) << 8; \
		source += Common::String::format("%s %d\n", #NAME, (int)arg); \
	} break;

#define AGDS_DIS_U(NAME, METHOD)     \
	case NAME: {               \
		uint16 arg = code[ip++]; arg |= ((uint16)code[ip++]) << 8; \
		source += Common::String::format("%s %u\n", #NAME, (uint)arg); \
	} break;

#define AGDS_DIS_UU(NAME, METHOD)     \
	case NAME: {                \
		uint16 arg1 = code[ip++]; arg1 |= ((uint16)code[ip++]) << 8; \
		uint16 arg2 = code[ip++]; arg2 |= ((uint16)code[ip++]) << 8; \
		source += Common::String::format("%s %u %u\n", #NAME, (uint)arg1, (uint)arg2); \
	} break;

#define AGDS_DIS_UD(NAME, METHOD)           \
	case NAME: {                     \
		uint16 arg1 = code[ip++]; arg1 |= ((uint16)code[ip++]) << 8; \
		uint16 arg2 = code[ip++]; arg2 |= ((uint16)code[ip++]) << 8; \
		source += Common::String::format("%s %u\n", #NAME, (uint)(arg1 | (arg2 << 16))); \
	} break;

Common::String Process::disassemble(ObjectPtr object) {
	Common::String source = Common::String::format("Object %s disassembly:\n", object->getName().c_str());

	const auto &code = object->getCode();
	uint ip = 0;
	while (ip < code.size()) {
		uint8 op = code[ip++];
		source += Common::String::format("%04x: %02x: ", ip - 1, op);
		switch (op) {
			AGDS_OPCODE_LIST(
				AGDS_DIS,
				AGDS_DIS_C, AGDS_DIS_B,
				AGDS_DIS_W, AGDS_DIS_U,
				AGDS_DIS_UD, AGDS_DIS_UU
			)
		default:
			source += Common::String::format("unknown opcode 0x%02x (%u)\n", (unsigned)op, (unsigned)op);
			break;
		}
	}
	source += Common::String::format("Object %s disassembly end\n", object->getName().c_str());
	return source;
}

} // namespace AGDS
