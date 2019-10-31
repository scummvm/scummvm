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

#include "agds/process.h"
#include "agds/agds.h"
#include "agds/animation.h"
#include "agds/character.h"
#include "agds/opcode.h"
#include "agds/region.h"
#include "agds/screen.h"
#include "agds/systemVariable.h"
#include "graphics/transparent_surface.h"
#include "common/debug.h"

namespace AGDS {

void Process::enter(uint16 magic, uint16 size) {
	if (magic != 0xdead || size != 0x0c)
		error("invalid enter() magic: 0x%04x or size: %u", magic, size);
	uint16 unk1			= next16();
	uint16 unk2			= next16();
	uint16 unk3			= next16();
	unsigned resOffset	= next16();
	uint16 resCount		= next16();
	uint16 unk4			= next16();
	debug("resource block %04x %04x %04x %04x,"
		" resources table with %u entries", unk1, unk2, unk3, unk4, resCount);

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
	Region *reg = _engine->loadRegion(name);
	int value = reg->center.x;
	push(value);
	debug("getRegionCenterX %s -> %d", name.c_str(), value);
}

void Process::getRegionCenterY() {
	Common::String name = popString();
	Region *reg = _engine->loadRegion(name);
	int value = reg->center.y;
	push(value);
	debug("getRegionCenterY %s -> %d", name.c_str(), value);
}

void Process::getObjectId() {
	const Common::String &name = _object->getName();
	//no rfind :(((
	Common::String::const_iterator dotpos = 0;
	for(Common::String::const_iterator i = name.begin(); i != name.end(); ++i)
		if (*i == '.')
			dotpos = i + 1;
	Common::String id(dotpos, name.end());
	int value = atoi(id.c_str());
	debug("getObjectId %s %d", name.c_str(), value);
	push(value);
}

void Process::loadPicture() {
	Common::String name = popText();
	int value = _engine->saveToCache(_engine->loadPicture(name));
	debug("loadPicture %s -> %d", name.c_str(), value);
	push(value);
}

void Process::loadAnimation() {
	Common::String name = popText();
	debug("loadAnimation %s", name.c_str());
	Animation *animation = _engine->loadAnimation(name);
	if (animation) {
		animation->position(_animationPosition);
		animation->phaseVar(_phaseVar);
		animation->loop(_animationLoop);
		animation->cycles(_animationCycles);
		if (_animationPaused)
			animation->stop();
		else
			animation->play();
		_engine->getCurrentScreen()->add(animation);
	}
}

void Process::loadSample() {
	Common::String name = popText();
	debug("loadSample %s", name.c_str());
	if (_phaseVar.empty()) {
		warning("playing sample %s without phase var", _phaseVar.c_str());
		return;
	}
	_engine->playSound(name, _phaseVar);
}

void Process::getSampleVolume() {
	Common::String name = popString();
	debug("getSampleVolume: stub %s", name.c_str());
	push(100);
}

void Process::setSampleVolumeAndPan() {
	int pan = pop();
	int volume = pop();
	Common::String name = popString();
	debug("setSampleVolumeAndPan %s %d %d", name.c_str(), volume, pan);
}

void Process::playSound() {
	Common::String name = popText();
	int arg = pop();
	debug("playSound %s %d", name.c_str(), arg);
	_engine->playSound(name, _phaseVar);
}

void Process::updatePhaseVarOr2() {
	Common::String name = popString();
	debug("updatePhaseVarOr2 stub %s", name.c_str());
	int value = _engine->getGlobal(name);
	_engine->setGlobal(name, value | 2);
}

void Process::updatePhaseVarOr4() {
	Common::String name = popString();
	debug("updatePhaseVarOr4 stub %s", name.c_str());
	int value = _engine->getGlobal(name);
	_engine->setGlobal(name, value | 4);
}

void Process::loadScreenObject() {
	Common::String name = popString();
	debug("loadScreenObject: %s", name.c_str());
	Screen * screen = _engine->getCurrentScreen();
	if (!screen->find(name)) {
		screen->add(_engine->loadObject(name));
	} else{
		warning("loadScreenObject: object %s already loaded", name.c_str());
	}
	suspend(kExitCodeLoadScreenObject, name);
}

void Process::loadScreenRegion() {
	Common::String name = popString();
	debug("loadScreenRegion %s", name.c_str());
	Screen * screen = _engine->getCurrentScreen();
	if (screen)
		screen->getObject()->setRegion(_engine->loadRegion(name));
	else
		warning("no current screen");
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

void Process::getRandomNumber() {
	int max = pop();
	int value = max > 0? _engine->_random.getRandomNumber(max - 1): 0;
	debug("random %d -> %d", max, value);
	push(value);
}

void Process::setGlobal() {
	Common::String name = popString();
	int value = pop();
	debug("setting global %s -> %d", name.c_str(), value);
	_engine->setGlobal(name, value);
}

void Process::setPhaseVar() {
	Common::String name = popString();
	_engine->setGlobal(name, 0);
	_phaseVar = name;
	debug("setPhaseVar %s", name.c_str());
}

void Process::getGlobal(unsigned index) {
	const Common::String & name = _object->getString(index).string;
	int value = _engine->getGlobal(name);
	debug("get global %u %s -> %d", index, name.c_str(), value);
	push(value);
}

void Process::hasGlobal() {
	Common::String name = popString();
	int result = _engine->hasGlobal(name)? 1: 0;
	debug("hasGlobal %s %d", name.c_str(), result);
	push(result);
}

void Process::postIncrementGlobal() {
	Common::String name = popString();
	int value = _engine->getGlobal(name);
	debug("increment global %s %d", name.c_str(), value);
	push(value);
	_engine->setGlobal(name, value + 1);
}

void Process::postDecrementGlobal() {
	Common::String name = popString();
	int value = _engine->getGlobal(name);
	debug("decrement global %s %d", name.c_str(), value);
	push(value);
	_engine->setGlobal(name, value - 1);
}

void Process::incrementGlobal(int inc) {
	Common::String name = popString();
	int value = _engine->getGlobal(name);
	debug("increment global %s %d", name.c_str(), value);
	_engine->setGlobal(name, value + inc);
}

void Process::decrementGlobal(int dec) {
	Common::String name = popString();
	int value = _engine->getGlobal(name);
	debug("decrement global %s %d", name.c_str(), value);
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
	debug("appendToSharedStorage %s -> %d", value.c_str(), index);
	push(index);
}

void Process::appendNameToSharedStorage() {
	int index = _engine->appendToSharedStorage(_object->getName());
	debug("appendNameToSharedStorage %s -> %d", _object->getName().c_str(), index);
	push(index);
}

Common::String Process::getCloneVarName(const Common::String & arg1, const Common::String & arg2) {
	bool isNumeric = false;
	size_t prefixLength;
	{
		const char *begin = arg1.c_str();
		const char *ptr = begin + arg1.size() - 1;
		while(*ptr >= '0' && *ptr <= '9' && ptr > begin)
			--ptr;
		isNumeric = *ptr == '.';
		prefixLength = isNumeric? ptr - begin: 0;
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

void Process::changeScreenPatch() {
	Common::String objectName = popString();
	Common::String screenName = popString();
	Common::String inventoryScr = _engine->getSystemVariable("inventory_scr")->getString();
	debug("changeScreenPatch: screen: %s, object: %s, inventory: %s",
		screenName.empty()? "none": screenName.c_str(), objectName.c_str(), inventoryScr.empty()? "none": inventoryScr.c_str());

	Screen * screen = _engine->getCurrentScreen();
	if (!screen) {
		error("no current screen");
		return;
	}

	if (!screenName.empty()) {
		debug("stub, returning 0");
		//check that patch exist
		push(0);
		return;
	} else {
		//change screen patch (load and return 1)
		ObjectPtr object = screen->find(objectName);
		int value = object && object->isActive();
		push(value);
	}
}

void Process::loadMouseCursorFromObject() {
	Common::String name = popText();
	debug("loadMouseCursorFromObject %s", name.c_str());
	Animation *cursor = _engine->loadMouseCursor(name);
	_object->setMouseCursor(cursor); //overlay cursor
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

void Process::onObjectUse(unsigned size) {
	Common::String arg = popString();
	debug("register use object handler %s -> %u", arg.c_str(), _ip);
	_object->setUseHandler(arg, _ip);
	_ip += size;
}

void Process::setScreenHeight() {
	int arg2 = pop();
	int arg1 = pop();
	debug("setScreenHeight: %d %d", arg1, arg2);
}

void Process::setDelay() {
	int delay = pop();
	debug("setDelay stub %d", delay);
}

void Process::setCycles() {
	int value = pop();
	debug("setCycles %d", value);
	_animationCycles = value;
}

void Process::setRandom() {
	int value = pop();
	debug("setRandom stub %d", value);
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
	_animationPaused = true;
}

void Process::resetState() {
	debug("process reset state");
	_phaseVar.clear();
	_animationCycles = 1;
	_animationLoop = false;
	_animationPosition = Common::Point();
	_animationPaused = false;

	_tileWidth = 16;
	_tileHeight = 16;
	_tileIndex = 0;
	_tileResource = 0;
}

void Process::stub129() {
	int value = pop();
	debug("stub129 %d animation duration?", value);
}

void Process::stub133() {
	int pan = pop();
	int volume = pop();
	debug("stub133: pan? %d volume? %d", pan, volume);
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

void Process::stub138() {
	Common::String arg2 = popString();
	Common::String arg1 = popString();
	debug("stub138 %s %s", arg1.c_str(), arg2.c_str());
	suspend(kExitCodeLoadScreenObject, arg2);
}

void Process::stub152() {
	Common::String name = popString();
	debug("stub152(getSomeX): %s", name.c_str());
	push(152);
}

void Process::stub153() {
	Common::String name = popString();
	debug("stub153:(getSomeY): %s", name.c_str());
	push(153);
}

void Process::stub154() {
	Common::String name = popString();
	debug("stub154(getSomeX): %s", name.c_str());
	ObjectPtr object = _engine->getCurrentScreenObject(name);
	push(0);
}

void Process::stub155() {
	Common::String name = popString();
	debug("stub155(getSomeY): %s", name.c_str());
	ObjectPtr object = _engine->getCurrentScreenObject(name);
	push(0);
}

void Process::stub160() {
	int arg = pop();
	debug("stub160: %d", arg);
}

void Process::stub166() {
	int arg3 = pop();
	int arg2 = pop();
	Common::String arg1 = popString();
	debug("stub166 %s %d %d", arg1.c_str(), arg2, arg3);
}

void Process::stub172() {
	int value = pop();
	debug("stub172: %d", value);
}

void Process::stub173() {
	debug("stub173: delAnimations?");
}

void Process::stub174() {
	debug("stub174: mouse pointer mode 1?");
}

void Process::stub192() {
	int value = pop();
	Common::String name = popString();
	debug("stub192: %s: set some object flag to %d", name.c_str(), value);
}

void Process::stub193() {
	debug("stub193: removing inventory object 0?");
}

void Process::stub190() {
	int value = pop();
	debug("stub190 %d", value);
}

void Process::disableMouseAreas() {
	int value = pop();
	debug("disableMouseAreas %d", value);
	_engine->_mouseMap.disable(value > 0);
}

void Process::stub194() {
	debug("stub194: mute?");
}

void Process::stub199() {
	int value = pop();
	debug("stub199: %d", value);
}

void Process::setTileIndex() {
	int index = pop();
	int resource = pop();
	debug("setTileIndex: index: %d, resource id: %d ", index, resource);
}

void Process::stub201(unsigned size) {
	debug("stub201, [handler] %u instructions", size);
	_ip += size;
}

void Process::stub202(unsigned size) {
	debug("stub202, [handler] %u instructions", size);
	_ip += size;
}

void Process::stub209(unsigned size) {
	debug("stub209, [handler] %u instructions", size);
	_ip += size;
}

void Process::modifyMouseArea() {
	int enabled = pop();
	int id = pop();
	debug("modifyMouseArea %d, %d", id, enabled);
	suspend(kExitCodeMouseAreaChange, id, enabled);
}

void Process::stub215() {
	int id = pop();
	debug("stub215: sound group %d", id);
}

void Process::stub216() {
	int soundGroup = pop();
	int frame = pop();
	int id = pop();
	debug("stub216: animation? id: %d, frame: %d, soundGroup: %d", id, frame, soundGroup);
}

void Process::stub217() {
	int soundGroup = pop();
	int frame = pop();
	int id = pop();
	debug("stub217: animation? id: %d, frame: %d, soundGroup: %d", id, frame, soundGroup);
}

void Process::playAnimationWithPhaseVar() {
	Common::String phaseVar = popString();
	debug("playAnimationWithPhaseVar %s", phaseVar.c_str());
	Animation * animation = _engine->findAnimationByPhaseVar(phaseVar);
	if (animation) {
		animation->phaseVar(phaseVar);
		animation->play();
		_engine->setGlobal(phaseVar, 0);
	}
	else
		warning("no animation with phase var %s found", phaseVar.c_str());
}

void Process::stub223() {
	int value = pop();
	debug("stub223: %d", value);
}

void Process::stub225() {
	int arg = pop();
	Common::String phaseVar = popString();
	debug("stub225: animation related, phaseVar %s, arg %d", phaseVar.c_str(), arg);
}

void Process::setTileSize() {
	_tileHeight = pop();
	_tileWidth = pop();
	debug("setTileSize %d %d", _tileWidth, _tileHeight);
}

void Process::generateRegion() {
	Common::String name = popString();
	debug("generateRegion %s", name.c_str());
}

void Process::stub184() {
	Common::String name = popString();
	debug("stub184: %s", name.c_str());
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
	_status = kStatusDone;
	_exitCode = kExitCodeDestroy;
}

void Process::exitProcessCreatePatch() {
	SystemVariable * initVar = _engine->getSystemVariable("init_resources");
	if (!initVar)
		error("no init_resources declared");
	Common::String init = initVar->getString();

	SystemVariable * doneVar = _engine->getSystemVariable("done_resources");
	if (!doneVar)
		error("no done_resources declared");
	Common::String done = doneVar->getString();
	debug("exitProcessCreatePatch stub, resource objects: %s %s", done.c_str(), init.c_str());
	suspend(kExitCodeCreatePatchLoadResources, done, init);
}

void Process::clearScreen() {
	debug("clearScreen");
}

void Process::moveScreenObject() {
	int arg3 = pop();
	int arg2 = pop();
	Common::String name = popString();
	debug("moveScreenObject %s %d %d", name.c_str(), arg2, arg3);
	ObjectPtr object = _engine->getCurrentScreenObject(name);
	if (object)
		object->move(Common::Point(arg2, arg3));
	else
		warning("moveScreenObject: object %s not found", name.c_str());
}

void Process::quit() {
	debug("quit");
	_engine->quitGame();
}

void Process::setDialogForNextFilm() {
	int value = pop();
	debug("setDialogForNextFilm %d", value);
}

void Process::npcSay() {
	debug("npcSay -> playerSay");
	playerSay();
}

void Process::playerSay() {
	Common::String sound = popText();
	Common::String arg2 = popText();
	Common::String arg1 = popString();
	debug("playerSay '%s' '%s' '%s'", arg1.c_str(), arg2.c_str(), sound.c_str());
	if (!sound.empty())
		_engine->playSound(sound, _phaseVar);
	//close inventory here if close flag was set
}

void Process::runDialog() {
	Common::String arg3 = popString();
	Common::String arg2 = popString();
	Common::String arg1 = popString();
	debug("runDialog %s %s %s", arg1.c_str(), arg2.c_str(), arg3.c_str());
	arg2 = _engine->loadText(arg2);
	arg3 = _engine->loadText(arg3);

	_engine->runDialog(arg2, arg3);

	suspend(kExitCodeRunDialog, arg1);
}


void Process::getObjectPictureWidth() {
	Common::String name = popString();
	debug("getObjectPictureWidth %s", name.c_str());
	ObjectPtr object = _engine->getCurrentScreenObject(name);
	if (object) {
		const Graphics::Surface *picture = object->getPicture();
		int value = picture? picture->w: 0;
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
		int value = picture? picture->h: 0;
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

	debug("playFilm %s %s", video.c_str(), audio.c_str());
	_engine->playFilm(video, audio);
	suspend();
}

void Process::inventoryClear() {
	debug("inventoryClear");
	_engine->inventory().clear();
}

void Process::inventoryAddObject() {
	Common::String name = popString();
	debug("inventoryAddObject %s", name.c_str());
	_engine->inventory().add(_engine->loadObject(name));
	suspend(kExitCodeLoadInventoryObject, name);
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
	push(_engine->appendToSharedStorage(object? object->getName(): Common::String()));
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

void Process::setObjectHeight() {
	int height = pop();
	debug("setObjectHeight %d", height);
}

void Process::updateScreenHeightToDisplay() {
	debug("updateScreenHeightToDisplay");
}

void Process::loadTextFromObject() {
	Common::String text = popText();
	debug("loadTextFromObject %s", text.c_str());
	_object->setText(text);
}

void Process::call(uint16 addr) {
	debug("call %04x", addr);
	//original engine just create new process, save exit code in screen object
	//and on stack, then just ignore return code, fixme?
	_waitForCall = true;
	_engine->runProcess(_object, _ip + addr, this);
	suspend();
}

void Process::onKey(unsigned size) {
	Common::String key = popString();
	debug("onKey %s [handler], %u instructions", key.c_str(), size);
	_object->setKeyHandler(key, _ip);
	_ip += size;
}

void Process::onUse(unsigned size) {
	debug("lclick [handler], %u instructions", size);
	_object->setClickHandler(_ip);
	_ip += size;
}

void Process::onObjectC1(unsigned size) {
	debug("unknown (0xc1) [handler], %u instructions", size);
	_ip += size;
}

void Process::onLook(unsigned size) {
	debug("look? [handler], %u instructions", size);
	_object->setExamineHandler(_ip);
	_ip += size;
}

void Process::onScreenBD(unsigned size) {
	debug("onScreen(+BD) [handler], %u instructions", size);
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
	Region *region = _engine->loadRegion(name);

	int value = _engine->_mouseMap.add(MouseRegion(region, onEnter, onLeave));
	debug("\tmouse area id -> %d", value);
	push(value);
}

void Process::loadCharacter() {
	Common::String object = popString();
	Common::String name = popText();
	Common::String id = popString();
	debug("loadCharacter %s %s %s", id.c_str(), name.c_str(), object.c_str());
	_engine->loadCharacter(id, name, object);
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
	int arg3 = pop();
	Common::String arg2 = popString();
	Common::String arg1 = popString();
	debug("moveCharacter %s %s %d, usermove: %d", arg1.c_str(), arg2.c_str(), arg3, usermove);
	if (_status == kStatusPassive)
		suspend();
}

void Process::animateCharacter() {
	int arg2 = pop();
	Common::String name = popString();
	debug("animateCharacter: %s %d", name.c_str(), arg2);

	Character *character = _engine->getCharacter(name);
	if (character)
		character->animate(arg2 * 100 / _animationSpeed);
	else
		warning("character %s could not be found", name.c_str());
}

void Process::showCharacter() {
	Common::String name = popString();
	debug("showCharacter %s", name.c_str());
	Character *character = _engine->getCharacter(name);
	if (character)
		_engine->runObject(character->object());
	else
		warning("character %s could not be found", name.c_str());
}

void Process::leaveCharacter() {
	Common::String arg2 = popString();
	Common::String arg1 = popString();
	debug("leaveCharacter %s %s", arg1.c_str(), arg2.c_str());
	_engine->loadRegion(arg2);
}

void Process::setCharacter() {
	Common::String object = popString();
	Common::String region = popString();
	debug("setCharacter %s %s", region.c_str(), object.c_str());
}

void Process::pointCharacter() {
	Common::String arg2 = popString();
	Common::String arg1 = popString();
	debug("pointCharacter %s %s", arg1.c_str(), arg2.c_str());
	suspend();
}

void Process::getCharacterAnimationPhase() {
	Common::String name = popString();
	debug("getCharacterAnimationPhase: %s", name.c_str());
	Character *character = _engine->getCharacter(name);
	int phase = character? character->getPhase(): -1;
	debug("animation phase = %d", phase);
	push(phase);
}

void Process::stopCharacter() {
	int arg = pop();
	Common::String name = popString();
	debug("stopCharacter: stub %s %d", name.c_str(), arg);
	Character * character = _engine->getCharacter(name);
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
	debug("fixme: some script commands call enableUser again");
	enableUser();
}

void Process::loadRegionFromObject() {
	Common::String name = popString();
	debug("loadRegionFromObject %s", name.c_str());
	_object->setRegion(_engine->loadRegion(name));
}


void Process::loadPictureFromObject() {
	Common::String name = popText();
	debug("loadPictureFromObject %s", name.c_str());
	_object->setPicture(_engine->loadPicture(name));
}

void Process::loadAnimationFromObject() {
	Common::String name = popText();
	debug("loadAnimationFromObject %s", name.c_str());
	if (!_phaseVar.empty()) {
		_engine->setGlobal(_phaseVar, -2);
	}
	Animation * animation = _engine->loadAnimation(name);
	if (animation) {
		animation->phaseVar(_phaseVar);
		animation->loop(_animationLoop);
		animation->cycles(_animationCycles);
		if (_animationPaused)
			animation->stop();
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
	if (object)
		object->setPicture(NULL);
}

void Process::stub235() {
	int arg3 = pop();
	int arg2 = pop();
	int arg1 = pop();
	debug("stub235 (fadeScreen?) %d %d %d", arg1, arg2, arg3);
	enableUser();
	if (_status == kStatusPassive)
		suspend();
}

void Process::stub244() {
	Common::String arg2 = popString();
	Common::String arg1 = popString();

	debug("stub244 [text tell status?] %s %s", arg1.c_str(), arg2.c_str());
	_engine->setGlobal(arg1, 1);
	_engine->setGlobal(arg2, 1);
	_engine->getSystemVariable("dialog_var")->setInteger(1);
}


//fixme: add trace here
#define OP(NAME, METHOD) \
	case NAME: METHOD (); break

#define OP_I(NAME, METHOD, IMM) \
	case NAME: { METHOD (IMM); } break

#define OP_C(NAME, METHOD) \
	case NAME: { int8 arg = next(); METHOD (arg); } break

#define OP_B(NAME, METHOD) \
	case NAME: { uint8 arg = next(); METHOD (arg); } break

#define OP_W(NAME, METHOD) \
	case NAME: { int16 arg = next16(); METHOD (arg); } break

#define OP_U(NAME, METHOD) \
	case NAME: { uint16 arg = next16(); METHOD (arg); } break

#define OP_UU(NAME, METHOD) \
	case NAME: { uint16 arg1 = next16(); uint16 arg2 = next16(); METHOD (arg1, arg2); } break

#define OP_D(NAME, METHOD) \
	case NAME: { uint16 arg1 = next16(); uint32 arg2 = next16(); METHOD (arg1 | (arg2 << 16)); } break

ProcessExitCode Process::execute() {
	if (_waitForCall) {
		_exitCode = kExitCodeSuspend;
		return _exitCode;
	}
	_exitCode = kExitCodeDestroy;

	const Object::CodeType &code = _object->getCode();
	while(_status == kStatusActive && _ip < code.size()) {
		_lastIp = _ip;
		uint8 op = next();
		switch(op) {
			OP_UU	(kEnter, enter);
			OP_W	(kJumpZImm16, jumpz);
			OP_W	(kJumpImm16, jump);
			OP		(kPop, pop);
			OP		(kDup, dup);
			OP		(kExitProcess, exitProcess);
			OP		(kSuspendProcess, suspend);
			OP_D	(kPushImm32, push);
			OP_C	(kPushImm8, push);
			OP_W	(kPushImm16, push);
			OP_C	(kPushImm8_2, push);
			OP_W	(kPushImm16_2, push);
			OP_B	(kGetGlobalImm8, getGlobal);
			OP		(kPostIncrementGlobal, postIncrementGlobal);
			OP		(kPostDecrementGlobal, postDecrementGlobal);
			OP		(kIncrementGlobalByTop, incrementGlobalByTop);
			OP		(kDecrementGlobalByTop, decrementGlobalByTop);
			OP		(kMultiplyGlobalByTop, multiplyGlobalByTop);
			OP		(kDivideGlobalByTop, divideGlobalByTop);
			OP		(kModGlobalByTop, modGlobalByTop);
			OP		(kShlGlobalByTop, shlGlobalByTop);
			OP		(kShrGlobalByTop, shrGlobalByTop);
			OP		(kAndGlobalByTop, andGlobalByTop);
			OP		(kOrGlobalByTop, orGlobalByTop);
			OP		(kXorGlobalByTop, xorGlobalByTop);
			OP		(kEquals, equals);
			OP		(kNotEquals, notEquals);
			OP		(kGreater, greater);
			OP		(kLess, less);
			OP		(kGreaterOrEquals, greaterOrEquals);
			OP		(kLessOrEquals, lessOrEquals);
			OP		(kAdd, add);
			OP		(kSub, sub);
			OP		(kMul, mul);
			OP		(kDiv, div);
			OP		(kSetGlobal, setGlobal);
			OP		(kBoolOr, boolOr);
			OP		(kBoolAnd, boolAnd);
			OP		(kAnd, bitAnd);
			OP		(kOr, bitOr);
			OP		(kXor, bitXor);
			OP		(kNot, bitNot);
			OP		(kBoolNot, boolNot);
			OP		(kNegate, negate);
			OP_U	(kCallImm16, call);
			OP_U	(kObjectRegisterLookHandler, onLook);
			OP_U	(kObjectRegisterUseHandler, onUse);
			OP_U	(kObjectRegisterHandlerC1, onObjectC1);
			OP_U	(kScreenRegisterHandlerBD, onScreenBD);
			OP		(kLoadMouseCursorFromObject, loadMouseCursorFromObject);
			OP		(kLoadRegionFromObject, loadRegionFromObject);
			OP		(kLoadPictureFromObject, loadPictureFromObject);
			OP		(kLoadAnimationFromObject, loadAnimationFromObject);
			OP		(kShowCharacter, showCharacter);
			OP		(kEnableCharacter, enableCharacter);
			OP_I	(kMoveCharacterUserMove, moveCharacter, true);
			OP		(kLeaveCharacter, leaveCharacter);
			OP		(kSetCharacter, setCharacter);
			OP		(kPointCharacter, pointCharacter);
			OP		(kDisableUser, disableUser);
			OP		(kEnableUser, enableUser);
			OP		(kUpdatePhaseVarOr2, updatePhaseVarOr2);
			OP		(kUpdatePhaseVarOr4, updatePhaseVarOr4);
			OP		(kStub102, stub102);
			OP		(kClearScreen, clearScreen);
			OP		(kInventoryClear, inventoryClear);
			OP		(kLoadMouse, loadMouse);
			OP		(kInventoryAddObject, inventoryAddObject);
			OP		(kSetNextScreenSaveInHistory, setNextScreenSaveInHistory);
			OP_U	(kObjectRegisterUseObjectHandler, onObjectUse);
			OP		(kStub82, stub82);
			OP		(kStub83, stub83);
			OP		(kAnimateCharacter, animateCharacter);
			OP		(kLoadCharacter, loadCharacter);
			OP		(kSetObjectHeight, setObjectHeight);
			OP		(kUpdateScreenHeightToDisplay, updateScreenHeightToDisplay);
			OP		(kLoadTextFromObject, loadTextFromObject);
			OP		(kScreenSetHeight, setScreenHeight);
			OP		(kScreenLoadObject, loadScreenObject);
			OP		(kScreenLoadRegion, loadScreenRegion);
			OP		(kScreenCloneObject, cloneObject);
			OP		(kSetNextScreen, setNextScreen);
			OP		(kScreenRemoveObject, removeScreenObject);
			OP		(kLoadAnimation, loadAnimation);
			OP		(kLoadSample, loadSample);
			OP		(kSetAnimationPaused, setAnimationPaused);
			OP		(kPlayerSay, playerSay);
			OP		(kNPCSay, npcSay);
			OP		(kSetTimer, setTimer);
			OP		(kProcessResetState, resetState);
			OP		(kStub129, stub129);
			OP		(kSetCycles, setCycles);
			OP		(kSetRandom, setRandom);
			OP		(kStub133, stub133);
			OP		(kSetAnimationPosition, setAnimationPosition);
			OP		(kSetPhaseVar, setPhaseVar);
			OP		(kSetAnimationLoop, setAnimationLoop);
			OP		(kSetAnimationSpeed, setAnimationSpeed);
			OP		(kStub138, stub138);
			OP		(kScreenChangeScreenPatch, changeScreenPatch);
			OP		(kGetFreeInventorySpace, getInventoryFreeSpace);
			OP		(kSetStringSystemVariable, setStringSystemVariable);
			OP		(kSetSystemIntegerVariable, setIntegerSystemVariable);
			OP		(kGetRegionCenterX, getRegionCenterX);
			OP		(kGetRegionCenterY, getRegionCenterY);
			OP		(kGetCharacterAnimationPhase, getCharacterAnimationPhase);
			OP		(kGetIntegerSystemVariable, getIntegerSystemVariable);
			OP		(kGetRandomNumber, getRandomNumber);
			OP		(kAppendToSharedStorage, appendToSharedStorage);
			OP		(kAppendNameToSharedStorage, appendNameToSharedStorage);
			OP		(kCloneName, cloneName);
			OP		(kGetCloneVar, getCloneVar);
			OP		(kSetCloneVar, setCloneVar);
			OP		(kStub152, stub152);
			OP		(kStub153, stub153);
			OP		(kStub154, stub154);
			OP		(kStub155, stub155);
			OP		(kStub160, stub160);
			OP		(kStub166, stub166);
			OP		(kSetDelay, setDelay);
			OP		(kStub172, stub172);
			OP		(kStub173, stub173);
			OP		(kStub174, stub174);
			OP		(kStub192, stub192);
			OP		(kQuit, quit);
			OP		(kExitProcessCreatePatch, exitProcessCreatePatch);
			OP		(kDisableInventory, disableInventory);
			OP		(kEnableInventory, enableInventory);
			OP		(kLoadPreviousScreen, loadPreviousScreen);
			OP		(kMoveScreenObject, moveScreenObject);
			OP		(kGetObjectId, getObjectId);
			OP		(kSetTileSize, setTileSize);
			OP		(kGenerateRegion, generateRegion);
			OP		(kGetMaxInventorySize, getMaxInventorySize);
			OP		(kAppendInventoryObjectNameToSharedSpace, appendInventoryObjectNameToSharedSpace);
			OP		(kStub184, stub184);
			OP		(kInventoryHasObject, inventoryHasObject);
			OP		(kSetObjectText, setObjectText);
			OP		(kStub190, stub190);
			OP		(kStub191, disableMouseAreas);
			OP		(kStub193, stub193);
			OP		(kStub194, stub194);
			OP		(kGetObjectPictureWidth, getObjectPictureWidth);
			OP		(kGetObjectPictureHeight, getObjectPictureHeight);
			OP		(kLoadPicture, loadPicture);
			OP		(kStub199, stub199);
			OP		(kSetSampleVolumeAndPan, setSampleVolumeAndPan);
			OP		(kPlaySound, playSound);
			OP		(kStub215, stub215);
			OP		(kStub216, stub216);
			OP		(kStub217, stub217);
			OP		(kStopCharacter, stopCharacter);
			OP		(kPlayAnimationWithPhaseVar, playAnimationWithPhaseVar);
			OP		(kStub223, stub223);
			OP		(kStub225, stub225);
			OP		(kFadeObject, fadeObject);
			OP		(kLoadFont, loadFont);
			OP_U	(kStub201Handler, stub201);
			OP_U	(kStub202ScreenHandler, stub202);
			OP		(kPlayFilm, playFilm);
			OP		(kAddMouseArea, addMouseArea);
			OP		(kFogOnCharacter, fogOnCharacter);
			OP		(kSetTileIndex, setTileIndex);
			OP		(kModifyMouseArea, modifyMouseArea);
			OP_U	(kStub209, stub209);
			OP_I	(kMoveCharacterNoUserMove, moveCharacter, false);
			OP_U	(kOnKey, onKey);
			OP		(kGetSampleVolume, getSampleVolume);
			OP		(kStub231, stub231);
			OP		(kStub233, stub233);
			OP		(kStub235, stub235);
			OP		(kUserEnabled, userEnabled);
			OP		(kStub244, stub244);
			OP		(kInventoryFindObjectByName, inventoryFindObjectByName);
			OP		(kRunDialog, runDialog);
			OP		(kHasGlobal, hasGlobal);
			OP		(kSetDialogForNextFilm, setDialogForNextFilm);
		default:
			error("%s: %08x: unknown opcode 0x%02x (%u)", _object->getName().c_str(), _ip - 1, (unsigned)op, (unsigned)op);
			_status = kStatusError;
			break;
		}
	}

	if (_status == kStatusActive) {
		debug("code ended, exiting...");
	}

	return _exitCode;
}

}
