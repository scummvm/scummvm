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
	int value = reg->centerX;
	push(value);
	debug("getRegionCenterX %s -> %d", name.c_str(), value);
}

void Process::getRegionCenterY() {
	Common::String name = popString();
	Region *reg = _engine->loadRegion(name);
	int value = reg->centerY;
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
	debug("loadPicture stub %s", name.c_str());
	push(100500); //dummy
}

void Process::loadAnimation() {
	Common::String name = popText();
	debug("loadAnimation %s", name.c_str());
}

void Process::loadSample() {
	Common::String name = popText();
	debug("loadSample %s", name.c_str());
	if (_phaseVar.empty()) {
		warning("playing sample %s without phase var", _phaseVar.c_str());
		return;
	}
	_engine->_soundManager.play(name, _phaseVar);
}

void Process::getSampleVolume() {
	Common::String name = popString();
	debug("getSampleVolume: stub");
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
	suspend(kExitCodeLoadScreenObject, name);
}

void Process::loadScreenRegion() {
	Common::String name = popString();
	debug("loadScreenRegion %s", name.c_str());
	_engine->loadObject(_engine->currentScreen()->getName())->setRegion(_engine->loadRegion(name));
}

void Process::cloneObject() {
	Common::String arg2 = popString();
	Common::String arg1 = popString();
	debug("cloneObject: %s %s", arg1.c_str(), arg2.c_str());
	suspend(kExitCodeLoadScreenObject, arg1, arg2);
}

void Process::removeScreenObject() {
	Common::String name = popString();
	debug("removeScreenObject: %s", name.c_str());
	Screen *screen = _engine->currentScreen();
	if (screen)
		screen->remove(name);
}

void Process::loadFont() {
	Common::String name = popText();
	int id = pop();
	debug("loadFont %s %d stub", name.c_str(), id);
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

void Process::resetPhaseVar() {
	Common::String name = popString();
	_engine->setGlobal(name, 0);
	_phaseVar = name;
	debug("resetPhaseVar %s", name.c_str());
}

void Process::getGlobal(unsigned index) {
	const Common::String & name = _object->getString(index).string;
	int value = _engine->getGlobal(name);
	debug("get global %s -> %d", name.c_str(), value);
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

void Process::setCloneVar() {
	int arg3 = pop();
	Common::String arg2 = popString();
	Common::String arg1 = popString();
	debug("setCloneVar %s %s %d", arg1.c_str(), arg2.c_str(), arg3);
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
	debug("global name for clone: %s", name.c_str());
	_engine->setGlobal(name, arg3);
	push(arg3);
}

void Process::disableUser() {
	debug("disableUser");
	_engine->enableUser(false);
}

void Process::changeScreenPatch() {
	Common::String res2 = popString();
	Common::String res1 = popString();
	//change screen patch
	debug("changeScreenPatch: '%s' '%s'", res1.c_str(), res2.c_str());
	push(0);
}

void Process::loadMouseCursorFromObject() {
	Common::String name = popText();
	debug("loadMouseCursorFromObject %s", name.c_str());
	_object->setMouseCursor(_engine->loadAnimation(name)); //overlay cursor
}

void Process::fadeObject() {
	int value = pop();
	Common::String name = popString();
	debug("fadeObject %s %d", name.c_str(), value);
	_engine->loadObject(name)->setAlpha(value);
}

void Process::stub63(unsigned size) {
	Common::String arg = popString();
	debug("stub63: [handler] %u instructions, arg: %s", size, arg.c_str());
	_ip += size;
}

void Process::stub74() {
	int arg2 = pop();
	int arg1 = pop();
	debug("stub74: %d %d", arg1, arg2);
}

void Process::stub82() {
	Common::String arg2 = popString();
	Common::String arg1 = popString();
	debug("stub82: %s %s", arg1.c_str(), arg2.c_str());
}

void Process::stub119() {
	debug("stub119");
}

void Process::stub128() {
	debug("processCleanupStub128");
	_phaseVar.clear();
}

void Process::stub129() {
	int value = pop();
	debug("stub129 %d", value);
}

void Process::stub130() {
	int value = pop();
	debug("stub130 %d", value);
}
void Process::stub133() {
	int pan = pop();
	int volume = pop();
	debug("stub133: pan? %d volume? %d", pan, volume);
}

void Process::stub134() {
	int arg2 = pop();
	int arg1 = pop();
	debug("stub134, font related %d %d", arg1, arg2);
}

void Process::stub136() {
	debug("stub136 sets value of stub130 to 1000000000");
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
	push(154);
}

void Process::stub155() {
	Common::String name = popString();
	debug("stub154(getSomeY): %s", name.c_str());
	push(155);
}

void Process::stub166() {
	int arg2 = pop();
	int arg1 = pop();
	debug("stub166 %d %d", arg1, arg2);
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

void Process::stub176() {
	debug("stub176");
}

void Process::stub192() {
	int value = pop();
	Common::String name = popString();
	debug("stub192: %s: set some object flag to %d", name.c_str(), value);
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
	debug("stub194");
}

void Process::stub199() {
	int value = pop();
	debug("stub199: %d", value);
}

void Process::stub200() {
	int value = pop();
	debug("stub200: %d", value);
}

void Process::stub202(unsigned size) {
	debug("stub203, [handler] %u instructions", size);
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

void Process::stub221() {
	Common::String phaseVar = popString();
	debug("stub221: animation related, phaseVar %s", phaseVar.c_str());
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

void Process::setFontGlyphSize() {
	_glyphHeight = pop();
	_glyphWidth = pop();
	debug("setFontGlyphSize %d %d", _glyphWidth, _glyphHeight);
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
	Common::String arg2 = popText();
	Common::String arg1 = popString();
	debug("setObjectText %s \"%s\" %d", arg1.c_str(), arg2.c_str(), arg3);
	//_engine->loadObject(arg1);
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
	Common::String arg1 = popString();
	debug("moveScreenObject %s %d %d", arg1.c_str(), arg2, arg3);
	Object *object = _engine->loadObject(arg1);
	object->move(Common::Point(arg2, arg3));
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
	Common::String arg3 = popString();
	Common::String arg2 = popString();
	Common::String arg1 = popString();
	debug("playerSay %s %s %s", arg1.c_str(), arg2.c_str(), arg3.c_str());
	//close inventory here if close flag was set
}
void Process::runDialog() {
	Common::String arg3 = popString();
	Common::String arg2 = popString();
	Common::String arg1 = popString();
	debug("runDialog %s %s %s", arg1.c_str(), arg2.c_str(), arg3.c_str());
	arg2 = _engine->loadText(arg2);
	arg3 = _engine->loadText(arg3);
	debug("definition:\n%s", arg3.c_str());
	debug("dialog:\n%s", arg2.c_str());
	suspend(kExitCodeRunDialog, arg1);
}


void Process::getObjectPictureWidth() {
	Common::String name = popString();
	debug("getObjectPictureWidth %s", name.c_str());
	Object *object = _engine->loadObject(name);
	const Graphics::Surface *picture = object->getPicture();
	int value = picture? picture->w: 0;
	debug("\t->%d", value);
	push(value);
}

void Process::getObjectPictureHeight() {
	Common::String name = popString();
	debug("getObjectPictureHeight %s", name.c_str());
	Object *object = _engine->loadObject(name);
	const Graphics::Surface *picture = object->getPicture();
	int value = picture? picture->h: 0;
	debug("\t->%d", value);
	push(value);
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
	suspend(kExitCodeLoadInventoryObject, name);
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
	Object *object = _engine->inventory().get(index);
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
}

void Process::enableInventory() {
	debug("enableInventory");
}

void Process::setScreenHeight() {
	int height = pop();
	debug("setScreenHeight %d", height);
}

void Process::updateScreenHeightToDisplay() {
	debug("updateScreenHeightToDisplay");
}

void Process::loadTextFromObject() {
	Common::String text = popText();
	debug("loadTextFromObject %s", text.c_str());
}

void Process::call(uint16 addr) {
	debug("call %04x", addr);
	//original engine just create new process, save exit code in screen object
	//and on stack, then just ignore return code, fixme?
	Process callee(_engine, _object, _ip + addr);
	ProcessExitCode code = callee.execute();
	debug("call returned %d", code);
	suspend();
}

void Process::onKey(unsigned size) {
	Common::String key = popString();
	debug("onKey %s [handler], %u instructions", key.c_str(), size);
	_ip += size;
}

void Process::onUse(unsigned size) {
	debug("lclick [handler], %u instructions", size);
	_object->setClickHandler(_ip);
	_ip += size;
}

void Process::onLook(unsigned size) {
	debug("look? [handler], %u instructions", size);
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
	Common::String arg3 = popString();
	Common::String arg2 = popString();
	Common::String arg1 = popString();

	debug("addMouseArea (region: %s) %s %s", arg1.c_str(), arg2.c_str(), arg3.c_str());
	Region *region = _engine->loadRegion(arg1);

	int value = _engine->_mouseMap.add(MouseRegion(region, arg2, arg3));
	debug("\tmouse area id -> %d", value);
	push(value);
}

void Process::loadCharacter() {
	Common::String arg3 = popString();
	Common::String arg2 = popString();
	Common::String arg1 = popString();
	debug("loadCharacter %s %s %s", arg1.c_str(), arg2.c_str(), arg3.c_str());
}

void Process::enableCharacter() {
	Common::String name = popString();
	debug("enableCharacter %s", name.c_str());
}

void Process::showCharacter() {
	Common::String name = popString();
	debug("showCharacter %s", name.c_str());
}

void Process::leaveCharacter() {
	Common::String arg2 = popString();
	Common::String arg1 = popString();
	debug("leaveCharacter %s %s", arg1.c_str(), arg2.c_str());
	_engine->loadRegion(arg2);
}

void Process::fogOnCharacter() {
	int arg2 = pop();
	int arg1 = pop();
	Common::String name = popString();
	debug("fogOnCharacter %s %d %d", name.c_str(), arg1, arg2);
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
}

void Process::setTimer() {
	int value = pop();
	debug("setTimer %d", value);
	_engine->setTimer(value);
	suspend();
}

void Process::stub235() {
	int arg3 = pop();
	int arg2 = pop();
	int arg1 = pop();
	debug("stub235 (fadeScreen?) %d %d %d", arg1, arg2, arg3);
	suspend();
}



//fixme: add trace here
#define OP(NAME, METHOD) \
	case NAME: METHOD (); break

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
			OP_U	(kCallImm16, call);
			OP_U	(kObjectRegisterLookHandler, onLook);
			OP_U	(kObjectRegisterUseHandler, onUse);
			OP_U	(kStub63, stub63);
			OP_U	(kScreenRegisterHandlerBD, onScreenBD);
			OP		(kLoadMouseCursorFromObject, loadMouseCursorFromObject);
			OP		(kLoadRegionFromObject, loadRegionFromObject);
			OP		(kLoadPictureFromObject, loadPictureFromObject);
			OP		(kLoadAnimationFromObject, loadAnimationFromObject);
			OP		(kShowCharacter, showCharacter);
			OP		(kEnableCharacter, enableCharacter);
			OP		(kLeaveCharacter, leaveCharacter);
			OP		(kDisableUser, disableUser);
			OP		(kEnableUser, enableUser);
			OP		(kUpdatePhaseVarOr2, updatePhaseVarOr2);
			OP		(kUpdatePhaseVarOr4, updatePhaseVarOr4);
			OP		(kClearScreen, clearScreen);
			OP		(kInventoryClear, inventoryClear);
			OP		(kLoadMouse, loadMouse);
			OP		(kInventoryAddObject, inventoryAddObject);
			OP		(kSetNextScreenSaveInHistory, setNextScreenSaveInHistory);
			OP		(kStub82, stub82);
			OP		(kLoadCharacter, loadCharacter);
			OP		(kSetScreenHeight, setScreenHeight);
			OP		(kUpdateScreenHeightToDisplay, updateScreenHeightToDisplay);
			OP		(kLoadTextFromObject, loadTextFromObject);
			OP		(kStub74, stub74);
			OP		(kScreenLoadObject, loadScreenObject);
			OP		(kScreenLoadRegion, loadScreenRegion);
			OP		(kScreenCloneObject, cloneObject);
			OP		(kSetNextScreen, setNextScreen);
			OP		(kScreenRemoveObject, removeScreenObject);
			OP		(kLoadAnimation, loadAnimation);
			OP		(kLoadSample, loadSample);
			OP		(kStub119, stub119);
			OP		(kPlayerSay, playerSay);
			OP		(kNPCSay, npcSay);
			OP		(kSetTimer, setTimer);
			OP		(kProcessCleanupStub128, stub128);
			OP		(kStub129, stub129);
			OP		(kStub130, stub130);
			OP		(kStub133, stub133);
			OP		(kStub134, stub134);
			OP		(kResetPhaseVar, resetPhaseVar);
			OP		(kStub136, stub136);
			OP		(kScreenChangeScreenPatch, changeScreenPatch);
			OP		(kGetFreeInventorySpace, getInventoryFreeSpace);
			OP		(kSetStringSystemVariable, setStringSystemVariable);
			OP		(kSetSystemIntegerVariable, setIntegerSystemVariable);
			OP		(kGetRegionCenterX, getRegionCenterX);
			OP		(kGetRegionCenterY, getRegionCenterY);
			OP		(kGetIntegerSystemVariable, getIntegerSystemVariable);
			OP		(kGetRandomNumber, getRandomNumber);
			OP		(kAppendToSharedStorage, appendToSharedStorage);
			OP		(kAppendNameToSharedStorage, appendNameToSharedStorage);
			OP		(kStub176, stub176);
			OP		(kSetCloneVar, setCloneVar);
			OP		(kStub152, stub152);
			OP		(kStub153, stub153);
			OP		(kStub154, stub154);
			OP		(kStub155, stub155);
			OP		(kStub166, stub166);
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
			OP		(kSetGlyphSize, setFontGlyphSize);
			OP		(kGenerateRegion, generateRegion);
			OP		(kGetMaxInventorySize, getMaxInventorySize);
			OP		(kAppendInventoryObjectNameToSharedSpace, appendInventoryObjectNameToSharedSpace);
			OP		(kStub184, stub184);
			OP		(kSetObjectText, setObjectText);
			OP		(kStub190, stub190);
			OP		(kStub191, disableMouseAreas);
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
			OP		(kStub221, stub221);
			OP		(kStub223, stub223);
			OP		(kStub225, stub225);
			OP		(kFadeObject, fadeObject);
			OP		(kLoadFont, loadFont);
			OP_U	(kStub202ScreenHandler, stub202);
			OP		(kPlayFilm, playFilm);
			OP		(kAddMouseArea, addMouseArea);
			OP		(kFogOnCharacter, fogOnCharacter);
			OP		(kStub200, stub200);
			OP		(kModifyMouseArea, modifyMouseArea);
			OP_U	(kStub209, stub209);
			OP_U	(kOnKey, onKey);
			OP		(kGetSampleVolume, getSampleVolume);
			OP		(kStub235, stub235);
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
