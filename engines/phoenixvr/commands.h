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

#ifndef PHOENIXVR_COMMANDS_H
#define PHOENIXVR_COMMANDS_H

#include "common/debug.h"
#include "common/stream.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "phoenixvr/phoenixvr.h"
#include "phoenixvr/script.h"

namespace PhoenixVR {

namespace {
constexpr int kMessengerInventorySlots = 12;
constexpr int kMessengerDisplayedInventorySlots = 8;
constexpr int kLouvreChestSize = 128;

struct LouvrePluginState {
	int objectChest[kLouvreChestSize];
	int monde4;
	uint32 rolloverBlockedUntil;
	bool initialized;
};

LouvrePluginState g_louvrePluginState;

void initLouvrePluginState() {
	if (g_louvrePluginState.initialized)
		return;

	for (int i = 0; i < kLouvreChestSize; ++i)
		g_louvrePluginState.objectChest[i] = 0;
	g_louvrePluginState.monde4 = 0;
	g_louvrePluginState.rolloverBlockedUntil = 0;
	g_louvrePluginState.initialized = true;
}

Common::String messengerInventorySlotName(int slot) {
	return Common::String::format("Pos%d", slot);
}

int messengerInventorySlot(int slot) {
	if (slot < 1 || slot > kMessengerInventorySlots)
		return 0;

	return g_engine->getVariable(messengerInventorySlotName(slot));
}

void setMessengerInventorySlot(int slot, int objectId) {
	if (slot >= 1 && slot <= kMessengerInventorySlots)
		g_engine->setVariable(messengerInventorySlotName(slot), objectId);
}

int messengerSpecialObjectSlot(int objectId) {
	switch (objectId) {
	case 100:
		return 10;
	case 200:
		return 11;
	case 300:
		return 9;
	case 400:
		return 12;
	default:
		return 0;
	}
}

Common::String louvreObjectImage(int objectId, int suffix) {
	return Common::String::format("Obj%02d%02d.bmp", objectId / 100, suffix);
}

void drawLouvreImage(const Common::String &image, int x, int y) {
	g_engine->drawArchiveImage(image, x, y);
}

void drawLouvreText(int textId, uint16 color = 1987) {
	static const Common::Rect kTextRect(62, 402, 354, 466);
	drawLouvreImage("EffaceText.bmp", kTextRect.left, kTextRect.top);
	g_engine->clearArchiveText(kTextRect);
	g_engine->drawArchiveText(textId, kTextRect, 14, true, color);
}

void clearLouvreText() {
	static const Common::Rect kTextRect(62, 402, 354, 466);
	drawLouvreImage("EffaceText.bmp", kTextRect.left, kTextRect.top);
	g_engine->clearArchiveText(kTextRect);
}

bool louvreTextBlocked() {
	return g_louvrePluginState.rolloverBlockedUntil > g_system->getMillis();
}

bool louvreObjectCanView(int objectId) {
	static const int kViewableObjects[] = {
		3000, 3100, 4200, 4300, 5500, 5900, 7600, 7700, 7800, 8500, 8600, 8800};

	for (int viewableObject : kViewableObjects)
		if (viewableObject == objectId)
			return true;
	return false;
}

int louvreCombineObjects(int selectedObjectId, int objectId) {
	struct CombineEntry {
		int left;
		int right;
		int result;
	};
	static const CombineEntry kCombineTable[] = {
		{500, 1200, 1300},
		{500, 600, 1400},
		{500, 700, 9600},
		{9600, 600, 1500},
		{1400, 700, 1500},
		{3000, 2900, 3100},
		{2300, 2200, 3500},
		{2100, 2200, 3500},
		{2100, 2300, 3500},
		{3500, 2100, 3600},
		{3500, 2200, 3600},
		{3500, 2300, 3600},
		{1800, 2600, 3700},
		{1800, 1700, 3700},
		{2600, 1700, 3700},
		{3700, 1800, 3800},
		{3700, 2600, 3800},
		{3700, 1700, 3800},
		{4200, 3400, 4300},
		{4400, 6100, 6200},
		{4400, 6000, 6300},
		{6200, 6000, 6400},
		{6300, 6100, 6400},
		{7700, 3400, 7800},
		{6900, 8200, 9800},
		{9300, 7000, 9900}};

	for (const CombineEntry &entry : kCombineTable) {
		if ((entry.left == selectedObjectId && entry.right == objectId) ||
			(entry.left == objectId && entry.right == selectedObjectId)) {
			if (entry.result == 3500) {
				g_engine->setVariable("RedPastryA", selectedObjectId);
				g_engine->setVariable("RedPastryB", objectId);
			} else if (entry.result == 3700) {
				g_engine->setVariable("YellowSeedA", selectedObjectId);
				g_engine->setVariable("YellowSeedB", objectId);
			}
			return entry.result;
		}
	}

	return 0;
}

int louvreSeparateObject(int objectId, int *parts) {
	struct SeparateEntry {
		int object;
		int parts[3];
	};
	static const SeparateEntry kSeparateTable[] = {
		{1300, {500, 1200, 0}},
		{1400, {500, 600, 0}},
		{1500, {1400, 700, 0}},
		{3100, {3000, 2900, 0}},
		{3600, {3500, 2100, 0}},
		{3800, {3700, 1800, 0}},
		{4300, {4200, 3400, 0}},
		{6200, {4400, 6100, 0}},
		{6300, {4400, 6000, 0}},
		{6400, {6200, 6000, 0}},
		{7800, {7700, 3400, 0}},
		{9600, {500, 700, 0}},
		{9800, {6900, 8200, 0}},
		{9900, {9300, 7000, 0}},
	};

	for (const SeparateEntry &entry : kSeparateTable) {
		if (entry.object == objectId) {
			Common::copy(entry.parts, entry.parts + ARRAYSIZE(entry.parts), parts);
			return entry.parts[2] != 0 ? 3 : 2;
		}
	}

	return 0;
}

bool louvreObjectCanCombine(int objectId) {
	static const int kCombinableObjects[] = {
		500, 600, 700, 1200, 1300, 1400, 1500, 1700, 1800, 2100, 2200, 2300, 2600, 2900,
		3000, 3100, 3400, 3500, 3600, 3700, 3800, 4200, 4300, 4400, 6000, 6100, 6200,
		6300, 6400, 6900, 7000, 7700, 7800, 8200, 9300, 9600, 9800, 9900};

	for (int combinableObject : kCombinableObjects)
		if (combinableObject == objectId)
			return true;
	return false;
}

bool louvreObjectCanSeparate(int objectId) {
	if (objectId == 3500 || objectId == 3700)
		return true;

	int parts[3] = {0, 0, 0};
	return louvreSeparateObject(objectId, parts) != 0;
}

int louvreObjectActionMask(int objectId) {
	if (objectId == 0)
		return 0;
	if (messengerSpecialObjectSlot(objectId) != 0)
		return 1;

	int mask = 1;
	if (louvreObjectCanView(objectId))
		mask |= 2;
	if (louvreObjectCanCombine(objectId))
		mask |= 4;
	if (louvreObjectCanSeparate(objectId))
		mask |= 8;
	return mask;
}

int louvreSelectedObjectActionMask() {
	int selection = g_engine->getVariable("Selection");
	if (selection > 100)
		selection -= 100;
	return louvreObjectActionMask(messengerInventorySlot(selection));
}

void drawLouvreActionButtons() {
	static const int kActionButtonX[] = {21, 22, 25, 24};
	static const int kActionButtonY[] = {145, 191, 270, 318};
	static const int kActionValues[] = {1, 2, 4, 8};

	int mask = louvreSelectedObjectActionMask();
	int currentAction = g_engine->getVariable("CurrentAction");
	for (uint i = 0; i < ARRAYSIZE(kActionValues); ++i) {
		int button = i + 1;
		if ((mask & kActionValues[i]) != 0)
			button += 10;
		if (currentAction == kActionValues[i])
			button += 20;
		drawLouvreImage(Common::String::format("Bout%04d.bmp", button), kActionButtonX[i], kActionButtonY[i]);
	}
}

void drawLouvreSelectionMarker(int slot, int imageNumber, bool selected) {
	static const Common::Point kSlotSelectedPos[kMessengerInventorySlots] = {
		Common::Point(389, 82),
		Common::Point(482, 83),
		Common::Point(574, 84),
		Common::Point(456, 170),
		Common::Point(556, 178),
		Common::Point(476, 258),
		Common::Point(567, 268),
		Common::Point(552, 352),
		Common::Point(8, 32),
		Common::Point(93, 32),
		Common::Point(182, 27),
		Common::Point(264, 28)};

	if (slot >= 1 && slot <= kMessengerInventorySlots && imageNumber > 0)
		drawLouvreImage(Common::String::format(selected ? "Sel%04d.bmp" : "DSel%04d.bmp", imageNumber), kSlotSelectedPos[slot - 1].x, kSlotSelectedPos[slot - 1].y);
}

void drawLouvreSelectedObject(int objectSlot) {
	static const int kSelectedObjectX = 103;
	static const int kSelectedObjectY = 123;

	initLouvrePluginState();
	if (objectSlot > 100)
		objectSlot -= 100;
	if (objectSlot < 1 || objectSlot > kMessengerInventorySlots)
		return;

	int objectId = messengerInventorySlot(objectSlot);
	if (objectId == 0)
		return;

	drawLouvreImage(louvreObjectImage(objectId, 0), kSelectedObjectX, kSelectedObjectY);
	clearLouvreText();
	drawLouvreActionButtons();
	drawLouvreSelectionMarker(objectSlot, objectSlot, true);
}

void clearLouvreSelection(bool clearObjectPreview = true) {
	int selection = g_engine->getVariable("Selection");
	int slot = selection > 100 ? selection - 100 : selection;

	g_engine->setVariable("Selection", 0);
	if (clearObjectPreview) {
		drawLouvreImage(louvreObjectImage(0, 0), 103, 123);
		clearLouvreText();
		drawLouvreActionButtons();
	}
	drawLouvreSelectionMarker(slot, selection, false);
}

void setLouvreSelectedSlot(int slot, int flags) {
	int previousSelection = g_engine->getVariable("Selection");
	int previousSlot = previousSelection > 100 ? previousSelection - 100 : previousSelection;
	int targetSlot = slot > 100 ? slot - 100 : slot;

	if (previousSelection == slot && (flags & 2) == 0) {
		clearLouvreSelection((flags & 1) == 0);
		return;
	}

	g_engine->setVariable("Selection", slot);
	drawLouvreSelectionMarker(previousSlot, previousSelection, false);
	drawLouvreSelectionMarker(targetSlot, slot, true);

	if ((slot < 100 || (flags & 2) != 0) && (flags & 5) == 0)
		drawLouvreSelectedObject(slot);
}

Common::String louvreNumberedImageName(const char *prefix, int value) {
	return Common::String::format("%s%04d", prefix, value);
}

int louvreChestLastUsedIndex() {
	for (int i = kLouvreChestSize - 1; i >= 0; --i)
		if (g_louvrePluginState.objectChest[i] != 0)
			return i + 1;
	return 0;
}

void drawLouvreChestScrollButtons() {
	static const Common::Point kUpButtonPos(191, 17);
	static const Common::Point kDownButtonPos(198, 375);

	int offset = g_engine->getVariable("CoffreOffset");
	int count = louvreChestLastUsedIndex();
	drawLouvreImage(offset <= 0 ? "CBou0001" : "CBou0011", kUpButtonPos.x, kUpButtonPos.y);
	drawLouvreImage(offset >= count - 1 || offset >= 124 ? "CBou0002" : "CBou0012", kDownButtonPos.x, kDownButtonPos.y);
}

void drawLouvreChestSlot(int slot) {
	static const Common::Point kChestImagePos[] = {
		Common::Point(80, 82), Common::Point(80, 145), Common::Point(80, 210), Common::Point(80, 273)};
	static const Common::Rect kChestTextRect[] = {
		Common::Rect(138, 80, 324, 134),
		Common::Rect(138, 145, 324, 199),
		Common::Rect(138, 208, 324, 262),
		Common::Rect(138, 272, 324, 326)};

	int index = g_engine->getVariable("CoffreOffset") + slot - 1;
	int objectId = (index >= 0 && index < kLouvreChestSize) ? g_louvrePluginState.objectChest[index] : 0;
	drawLouvreImage(louvreNumberedImageName("Cof", objectId + 1), kChestImagePos[slot - 1].x, kChestImagePos[slot - 1].y);
	drawLouvreImage(louvreNumberedImageName("CText", slot), kChestTextRect[slot - 1].left, kChestTextRect[slot - 1].top);
	g_engine->clearArchiveText(kChestTextRect[slot - 1]);
	if (objectId != 0)
		g_engine->drawArchiveText(objectId, kChestTextRect[slot - 1], 12, false, 1987);
}

void drawLouvreChest() {
	for (int slot = 1; slot < 5; ++slot)
		drawLouvreChestSlot(slot);
}

void drawLouvreChestSelection(int slot) {
	static const Common::Point kChestSelectionPos[] = {
		Common::Point(69, 70), Common::Point(70, 134), Common::Point(70, 199), Common::Point(70, 262), Common::Point(70, 326)};

	int current = g_engine->getVariable("CoffreSelect");
	if (current != 0 && current <= 4) {
		drawLouvreImage(louvreNumberedImageName("CDel", current), kChestSelectionPos[current - 1].x, kChestSelectionPos[current - 1].y);
		drawLouvreImage(louvreNumberedImageName("CDel", current + 1), kChestSelectionPos[current].x, kChestSelectionPos[current].y);
	}

	if (slot == 0 || current == slot) {
		g_engine->setVariable("CoffreSelect", 0);
		return;
	}

	drawLouvreImage(louvreNumberedImageName("Cel", slot), kChestSelectionPos[slot - 1].x, kChestSelectionPos[slot - 1].y);
	drawLouvreImage(louvreNumberedImageName("Cel", slot + 1), kChestSelectionPos[slot].x, kChestSelectionPos[slot].y);
	g_engine->setVariable("CoffreSelect", slot);
}

void drawLouvreInventorySlotObject(int slot, int objectId) {
	static const Common::Point kInventoryObjectPos[] = {
		Common::Point(381, 28),
		Common::Point(477, 28),
		Common::Point(565, 28),
		Common::Point(452, 116),
		Common::Point(549, 123),
		Common::Point(468, 204),
		Common::Point(558, 212),
		Common::Point(548, 300),
		Common::Point(18, 13),
		Common::Point(104, 20),
		Common::Point(193, 12),
		Common::Point(273, 15)};

	if (slot >= 1 && slot <= kMessengerInventorySlots && (slot <= kMessengerDisplayedInventorySlots || objectId != 0))
		drawLouvreImage(louvreNumberedImageName("Obj", objectId + 1), kInventoryObjectPos[slot - 1].x, kInventoryObjectPos[slot - 1].y);
}

void drawLouvreInventoryObjects(bool includeSpecialSlots) {
	for (int slot = 1; slot < (includeSpecialSlots ? 13 : 9); ++slot)
		drawLouvreInventorySlotObject(slot, messengerInventorySlot(slot));
}

void updateLouvreChestScroll(int direction) {
	int offset = g_engine->getVariable("CoffreOffset");
	int count = louvreChestLastUsedIndex();
	if (direction == 1)
		offset -= 3;
	else if (direction == 2)
		offset += 3;

	offset = CLIP(offset, 0, MIN(124, MAX(0, count - 1)));
	if (offset != g_engine->getVariable("CoffreOffset")) {
		g_engine->setVariable("CoffreSelect", 0);
		g_engine->setVariable("CoffreOffset", offset);
		drawLouvreChest();
	}
	drawLouvreChestScrollButtons();
}

void exchangeLouvreChestObject(int chestSlot, int inventorySlot) {
	if (chestSlot == 0)
		chestSlot = g_engine->getVariable("CoffreSelect");
	if (inventorySlot == 0)
		inventorySlot = g_engine->getVariable("Selection");
	if (inventorySlot > 100)
		inventorySlot -= 100;

	if (chestSlot < 1 || chestSlot > 4 || inventorySlot < 1 || inventorySlot > kMessengerInventorySlots)
		return;

	int chestIndex = g_engine->getVariable("CoffreOffset") + chestSlot - 1;
	if (chestIndex < 0 || chestIndex >= kLouvreChestSize)
		return;
	if (louvreChestLastUsedIndex() == 0)
		g_engine->setVariable("CoffreOffset", 0);

	int inventoryObject = messengerInventorySlot(inventorySlot);
	int chestObject = g_louvrePluginState.objectChest[chestIndex];
	g_louvrePluginState.objectChest[chestIndex] = inventoryObject;
	setMessengerInventorySlot(inventorySlot, chestObject);
	drawLouvreInventorySlotObject(inventorySlot, chestObject);
	drawLouvreChestSlot(chestSlot);
	drawLouvreChestSelection(0);
	g_engine->setVariable("Selection", 0);
	updateLouvreChestScroll(0);
}

struct MultiCD_Use_Install_Path : public Script::Command {
	Common::String path;

	MultiCD_Use_Install_Path(const Common::Array<Common::String> &args) : path(args[0]) {}
	void exec(Script::ExecutionContext &ctx) const override {
		debug("MultiCD_Use_Install_Path %s", path.c_str());
	}
};

struct MultiCD_Set_Transition_Script : public Script::Command {
	Common::String path;

	MultiCD_Set_Transition_Script(const Common::Array<Common::String> &args) : path(args[0]) {}
	void exec(Script::ExecutionContext &ctx) const override {
		debug("MultiCD_Set_Transition_Script %s", path.c_str());
	}
};

struct MultiCD_Set_Next_Script : public Script::Command {
	Common::String filename;

	MultiCD_Set_Next_Script(const Common::Array<Common::String> &args) : filename(args[0]) {}
	void exec(Script::ExecutionContext &ctx) const override {
		debug("MultiCD_Set_Next_Script %s", filename.c_str());
		g_engine->setNextScript(filename);
	}
};

struct MultiCD_If_Next_Script : public Script::Command {
	Common::String cd;
	Common::String var;

	MultiCD_If_Next_Script(const Common::Array<Common::String> &args) : cd(args[0]), var(args[1]) {}
	void exec(Script::ExecutionContext &ctx) const override {
		debug("MultiCD_If_Next_Script %s %s", cd.c_str(), var.c_str());
	}
};

struct LoadSave_Enter_Script : public Script::Command {
	Common::String reloading, notReloading;

	LoadSave_Enter_Script(const Common::Array<Common::String> &args) : reloading(args[0]), notReloading(args[1]) {}
	void exec(Script::ExecutionContext &ctx) const override {
		debug("LoadSave_Enter_Script %s, %s", reloading.c_str(), notReloading.c_str());
		auto loading = g_engine->enterScript();
		g_engine->setVariable(reloading, loading);
		g_engine->setVariable(notReloading, !loading);
	}
};

struct Play_Movie : public Script::Command {
	Common::String filename;

	Play_Movie(const Common::Array<Common::String> &args) : filename(args[0]) {}
	void exec(Script::ExecutionContext &ctx) const override {
		g_engine->playMovie(filename);
	}
};

struct Play_AnimBloc : public Script::Command {
	Common::String name;
	Common::String dstVar;
	int dstVarValue;
	float speed; // ticks per second

	Play_AnimBloc(const Common::Array<Common::String> &args) : name(args[0]), dstVar(args[1]), dstVarValue(atoi(args[2].c_str())), speed(args.size() >= 4 ? atof(args[3].c_str()) : 25) {}
	void exec(Script::ExecutionContext &ctx) const override {
		debug("Play_AnimBloc %s %s %d, %g", name.c_str(), dstVar.c_str(), dstVarValue, speed);
		g_engine->playAnimation(name, dstVar, dstVarValue, speed);
	}
};

struct Stop_AnimBloc : public Script::Command {
	Common::String name;

	Stop_AnimBloc(const Common::Array<Common::String> &args) : name(args[0]) {}
	void exec(Script::ExecutionContext &ctx) const override {
		debug("Stop_AnimBloc %s", name.c_str());
		g_engine->stopAnimation(name);
	}
};

struct Play_AnimBloc_Number : public Script::Command {
	Common::String prefix, var;
	Common::String dstVar;
	int dstVarValue;
	float speed;

	Play_AnimBloc_Number(const Common::Array<Common::String> &args) : prefix(args[0]), var(args[1]),
																	  dstVar(args[2]), dstVarValue(atoi(args[3].c_str())),
																	  speed(args.size() >= 5 ? atof(args[4].c_str()) : 25) {}
	void exec(Script::ExecutionContext &ctx) const override {
		debug("Play_AnimBloc_Number %s %s %s %d, %g", prefix.c_str(), var.c_str(), dstVar.c_str(), dstVarValue, speed);
		int value = g_engine->getVariable(var);
		auto name = Common::String::format("%s%04d", prefix.c_str(), value);
		g_engine->playAnimation(name, dstVar, dstVarValue, speed);
	}
};

struct Until : public Script::Command {
	Common::String var;
	int value;

	Until(const Common::Array<Common::String> &args) : var(args[0]), value(atoi(args[1].c_str())) {}
	void exec(Script::ExecutionContext &ctx) const override {
		g_engine->until(var, value);
	}
};

struct While : public Script::Command {
	float seconds;

	While(const Common::Array<Common::String> &args) : seconds(atof(args[0].c_str())) {}
	void exec(Script::ExecutionContext &ctx) const override {
		g_engine->wait(seconds);
	}
};

struct StartTimer : public Script::Command {
	float seconds;
	bool showTimer;

	StartTimer(const Common::Array<Common::String> &args) : seconds(atof(args[0].c_str())), showTimer(args.size() < 2 || atoi(args[1].c_str()) != 0) {}
	void exec(Script::ExecutionContext &ctx) const override {
		debug("starttimer %g %d", seconds, showTimer);
		g_engine->startTimer(seconds, showTimer);
	}
};

struct PauseTimer : public Script::Command {
	int paused, deactivate;

	PauseTimer(const Common::Array<Common::String> &args) : paused(atoi(args[0].c_str())), deactivate(atoi(args[1].c_str())) {}
	void exec(Script::ExecutionContext &ctx) const override {
		debug("pausetimer %d %d", paused, deactivate);
		g_engine->pauseTimer(paused, deactivate);
	}
};

struct KillTimer : public Script::Command {
	KillTimer(const Common::Array<Common::String> &args) {}
	void exec(Script::ExecutionContext &ctx) const override {
		debug("killtimer");
		g_engine->killTimer();
	}
};

struct ChangeCurseur : public Script::Command {
	int cursor;
	ChangeCurseur(const Common::Array<Common::String> &args) : cursor(atoi(args[0].c_str())) {}
	void exec(Script::ExecutionContext &ctx) const override {
		debug("changecurseur %d", cursor);
		g_engine->scheduleTest(cursor);
	}
};

struct Add : public Script::Command {
	Common::String dstVar;
	Common::String arg0;
	Common::String arg1;

	Add(const Common::Array<Common::String> &args) : dstVar(args[0]), arg0(args[1]), arg1(args[2]) {}

	void exec(Script::ExecutionContext &ctx) const override {
		debug("add %s %s %s", dstVar.c_str(), arg0.c_str(), arg1.c_str());
		g_engine->setVariable(dstVar, valueOf(arg0) + valueOf(arg1));
	}
};

struct Sub : public Script::Command {
	Common::String dstVar;
	Common::String arg0;
	Common::String arg1;

	Sub(const Common::Array<Common::String> &args) : dstVar(args[0]), arg0(args[1]), arg1(args[2]) {}

	void exec(Script::ExecutionContext &ctx) const override {
		debug("sub %s %s %s", dstVar.c_str(), arg0.c_str(), arg1.c_str());
		g_engine->setVariable(dstVar, valueOf(arg0) - valueOf(arg1));
	}
};

struct CopyVar : public Script::Command {
	Common::String srcVar;
	Common::String dstVar;

	CopyVar(const Common::Array<Common::String> &args) : srcVar(args[0]), dstVar(args[1]) {}

	void exec(Script::ExecutionContext &ctx) const override {
		debug("copyvar %s %s", srcVar.c_str(), dstVar.c_str());
		g_engine->setVariable(dstVar, g_engine->getVariable(srcVar));
	}
};

struct Not : public Script::Command {
	Common::String var;

	Not(Common::String v) : var(Common::move(v)) {}

	void exec(Script::ExecutionContext &ctx) const override {
		debug("not %s", var.c_str());
		g_engine->setVariable(var, g_engine->getVariable(var) ? 0 : 1);
	}
};

struct GetMonde4 : public Script::Command {
	Common::String var;
	Common::String negativeVar;

	GetMonde4(const Common::Array<Common::String> &args) : var(args[0]), negativeVar(args[1]) {}
	void exec(Script::ExecutionContext &ctx) const override {
		initLouvrePluginState();
		g_engine->setVariable(var, g_louvrePluginState.monde4);
		g_engine->setVariable(negativeVar, g_louvrePluginState.monde4 == 0);
	}
};

struct SetMonde4 : public Script::Command {
	int value;

	SetMonde4(const Common::Array<Common::String> &args) : value(atoi(args[0].c_str())) {}
	void exec(Script::ExecutionContext &ctx) const override {
		initLouvrePluginState();
		g_louvrePluginState.monde4 = value;
	}
};

struct AddObject : public Script::Command {
	int object;
	Common::String var;
	Common::String negativeVar;

	AddObject(const Common::Array<Common::String> &args) : object(atoi(args[0].c_str())), var(args[1]), negativeVar(args[2]) {}
	void exec(Script::ExecutionContext &ctx) const override {
		initLouvrePluginState();

		g_engine->setVariable(var, 1);
		g_engine->setVariable(negativeVar, 0);
		g_engine->setVariable("AddObjectOk", 0);

		int slot = messengerSpecialObjectSlot(object);
		if (slot != 0) {
			setMessengerInventorySlot(slot, object);
			g_engine->setVariable("Selection", slot);
			return;
		}

		for (slot = 1; slot < 9; ++slot) {
			if (messengerInventorySlot(slot) == 0) {
				setMessengerInventorySlot(slot, object);
				g_engine->setVariable("Selection", slot);
				return;
			}
		}

		g_engine->setVariable("Selection", 0);
		g_engine->setVariable("AddObjectOk", 1);
		g_engine->setVariable(var, 0);
		g_engine->setVariable(negativeVar, 1);
	}
};

struct AddCoffreObject : public Script::Command {
	int object;

	AddCoffreObject(const Common::Array<Common::String> &args) : object(atoi(args[0].c_str())) {}
	void exec(Script::ExecutionContext &ctx) const override {
		initLouvrePluginState();
		for (int i = 0; i < kLouvreChestSize; ++i) {
			if (g_louvrePluginState.objectChest[i] == 0) {
				g_louvrePluginState.objectChest[i] = object;
				return;
			}
		}
	}
};

struct IsPresent : public Script::Command {
	int object;
	Common::String var;
	Common::String negativeVar;

	IsPresent(const Common::Array<Common::String> &args) : object(atoi(args[0].c_str())), var(args[1]), negativeVar(args[2]) {}
	void exec(Script::ExecutionContext &ctx) const override {
		initLouvrePluginState();
		bool found = false;
		for (int i = 1; i <= kMessengerInventorySlots; ++i) {
			if (messengerInventorySlot(i) == object) {
				found = true;
				break;
			}
		}
		g_engine->setVariable(var, found);
		g_engine->setVariable(negativeVar, !found);
	}
};

struct RemoveObject : public Script::Command {
	Common::String object;

	RemoveObject(const Common::Array<Common::String> &args) : object(args[0]) {}
	void exec(Script::ExecutionContext &ctx) const override {
		initLouvrePluginState();
		int objectId = Script::Command::valueOf(object);
		for (int i = 1; i <= kMessengerInventorySlots; ++i)
			if (messengerInventorySlot(i) == objectId)
				setMessengerInventorySlot(i, 0);
	}
};

struct Cmp : public Script::Command {
	Common::String var;
	Common::String negativeVar;
	Common::String arg0;
	Common::String op;
	Common::String arg1;

	Cmp(const Common::Array<Common::String> &args) : var(args[0]), negativeVar(args[1]), arg0(args[2]), op(args[3]) {
		if (args.size() == 5) {
			arg1 = args[4];
		} else {
			uint opLength = (op.size() > 1 && (op[1] == '=' || op[1] == '>')) ? 2 : 1;
			arg1 = op.substr(opLength);
			op = op.substr(0, opLength);
		}
	}

	void exec(Script::ExecutionContext &ctx) const override {
		debug("cmp %s %s %s %s %s", var.c_str(), negativeVar.c_str(), arg0.c_str(), op.c_str(), arg1.c_str());
		bool r;
		auto value0 = valueOf(arg0);
		auto value1 = valueOf(arg1);
		if (op == "==") {
			r = value0 == value1;
		} else if (op == "!=" || op == "<>") {
			r = value0 != value1;
		} else if (op == "<") {
			r = value0 < value1;
		} else if (op == "<=") {
			r = value0 <= value1;
		} else if (op == ">") {
			r = value0 > value1;
		} else if (op == ">=") {
			r = value0 >= value1;
		} else {
			error("invalid cmp op %s", op.c_str());
		}
		g_engine->setVariable(var, r);
		g_engine->setVariable(negativeVar, !r);
	}
};

struct Select : public Script::Command {
	int value;
	Common::String arg0;
	Common::String arg1;

	Select(const Common::Array<Common::String> &args) : value(atoi(args[0].c_str())), arg0(args[1]), arg1(args[2]) {}

	void exec(Script::ExecutionContext &ctx) const override {
		initLouvrePluginState();
		g_engine->setVariable(arg1, 0);

		int objectId = messengerInventorySlot(value);
		if (objectId == 0)
			return;

		if (g_engine->getVariable("CurrentAction") == 4) {
			g_engine->setVariable(arg0, 1);
			g_engine->setVariable(arg1, 0);

			int selectedSlot = g_engine->getVariable("Selection") - 100;
			int selectedObjectId = messengerInventorySlot(selectedSlot);
			int combinedObjectId = louvreCombineObjects(selectedObjectId, objectId);
			if (combinedObjectId != 0) {
				g_engine->setVariable(arg1, 1);
				g_engine->setVariable("CurrentAction", 0);
				setMessengerInventorySlot(selectedSlot, combinedObjectId);
				setMessengerInventorySlot(value, 0);
				drawLouvreInventorySlotObject(selectedSlot, combinedObjectId);
				drawLouvreInventorySlotObject(value, 0);
				drawLouvreActionButtons();
				setLouvreSelectedSlot(selectedSlot, 0);
			}
			return;
		}

		setLouvreSelectedSlot(value, 0);
	}
};

struct DoAction : public Script::Command {
	int value;
	Common::String arg;

	DoAction(const Common::Array<Common::String> &args) : value(atoi(args[0].c_str())), arg(args[1]) {}

	void exec(Script::ExecutionContext &ctx) const override {
		initLouvrePluginState();
		g_engine->setVariable(arg, 0);

		int selection = g_engine->getVariable("Selection");
		if (selection > 100)
			selection -= 100;
		if (selection == 0)
			return;

		int objectId = messengerInventorySlot(selection);
		int actionMask = louvreSelectedObjectActionMask();

		int currentAction = g_engine->getVariable("CurrentAction");
		if (!((currentAction == value || currentAction == 0) && (actionMask & value) != 0))
			return;

		if (currentAction != 0) {
			setLouvreSelectedSlot(selection, 4);
			g_engine->setVariable("CurrentAction", 0);
			drawLouvreActionButtons();
			return;
		}

		g_engine->setVariable(arg, 1);
		switch (value) {
		case 1:
			g_engine->setVariable("Inventaire", objectId);
			break;
		case 2:
			if (louvreObjectCanView(objectId))
				g_engine->setVariable(arg, objectId);
			break;
		case 4:
			g_engine->setVariable("CurrentAction", value);
			setLouvreSelectedSlot(selection + 100, 0);
			break;
		case 8:
			g_engine->setVariable("CurrentAction", value);
			drawLouvreActionButtons();
			break;
		default:
			break;
		}
	}
};

struct IsHere : public Script::Command {
	IsHere(const Common::Array<Common::String> &args) {}
	void exec(Script::ExecutionContext &ctx) const override {
		int comefrom = g_engine->getVariable("comefrom");
		if (comefrom > 10000000)
			comefrom -= 10000000;

		Common::String value = Common::String::format("%d", comefrom);
		if (value.size() >= 3) {
			Common::String name = Common::String::format("P0%cS%c%c", value[0], value[1], value[2]);
			g_engine->playAnimation(name, "y", 20, 20);
		}
	}
};

struct InitCoffre : public Script::Command {
	InitCoffre(const Common::Array<Common::String> &args) {}
	void exec(Script::ExecutionContext &ctx) const override {
		initLouvrePluginState();
		int compacted[kLouvreChestSize];
		Common::fill(compacted, compacted + kLouvreChestSize, 0);

		int dst = 0;
		for (int i = 0; i < kLouvreChestSize; ++i)
			if (g_louvrePluginState.objectChest[i] != 0)
				compacted[dst++] = g_louvrePluginState.objectChest[i];

		Common::copy(compacted, compacted + kLouvreChestSize, g_louvrePluginState.objectChest);
	}
};

struct LoadCoffre : public Script::Command {
	LoadCoffre(const Common::Array<Common::String> &args) {}
	void exec(Script::ExecutionContext &ctx) const override {
		initLouvrePluginState();
		for (int i = 0; i < kLouvreChestSize; ++i)
			g_louvrePluginState.objectChest[i] = g_engine->getVariable(Common::String::format("coffre%d", i + 1));
	}
};

struct SaveCoffre : public Script::Command {
	SaveCoffre(const Common::Array<Common::String> &args) {}
	void exec(Script::ExecutionContext &ctx) const override {
		initLouvrePluginState();
		for (int i = 0; i < kLouvreChestSize; ++i)
			g_engine->setVariable(Common::String::format("coffre%d", i + 1), g_louvrePluginState.objectChest[i]);
	}
};

struct ExeDemo : public Script::Command {
	Common::String var;

	ExeDemo(const Common::Array<Common::String> &args) : var(args[0]) {}
	void exec(Script::ExecutionContext &ctx) const override {
		debug("ExeDemo %s", var.c_str());
		g_engine->setVariable(var, 1);
	}
};

struct AfficheImage : public Script::Command {
	Common::String image;
	int x;
	int y;

	AfficheImage(const Common::Array<Common::String> &args) : image(args[0]), x(atoi(args[1].c_str())), y(atoi(args[2].c_str())) {}
	void exec(Script::ExecutionContext &ctx) const override {
		g_engine->showImageOverlay(image, x, y);
	}
};

struct StopAffiche : public Script::Command {
	StopAffiche(const Common::Array<Common::String> &args) {}
	void exec(Script::ExecutionContext &ctx) const override {
		g_engine->stopImageOverlay();
	}
};

struct UpdateStage : public Script::Command {
	UpdateStage(const Common::Array<Common::String> &args) {}
	void exec(Script::ExecutionContext &ctx) const override {
		g_engine->updateStage();
	}
};

struct StartCible : public Script::Command {
	Common::String name;
	int periodSeconds;
	Common::Array<int> bounds;

	StartCible(const Common::Array<Common::String> &args) : name(args[0]), periodSeconds(atoi(args[1].c_str())) {
		for (uint i = 2; i < args.size(); ++i)
			bounds.push_back(atoi(args[i].c_str()));
	}
	void exec(Script::ExecutionContext &ctx) const override {
		g_engine->startCible(name, periodSeconds, bounds);
	}
};

struct StopCible : public Script::Command {
	StopCible(const Common::Array<Common::String> &args) {}
	void exec(Script::ExecutionContext &ctx) const override {
		g_engine->stopCible();
	}
};

struct TestCible : public Script::Command {
	Common::String xVar;
	Common::String yVar;

	TestCible(const Common::Array<Common::String> &args) : xVar(args[0]), yVar(args[1]) {}
	void exec(Script::ExecutionContext &ctx) const override {
		g_engine->testCible(xVar, yVar);
	}
};

struct AfficheCoffre : public Script::Command {
	AfficheCoffre(const Common::Array<Common::String> &args) {}
	void exec(Script::ExecutionContext &ctx) const override {
		initLouvrePluginState();
		drawLouvreChest();
	}
};

struct AfficheSelection : public Script::Command {
	AfficheSelection(const Common::Array<Common::String> &args) {}
	void exec(Script::ExecutionContext &ctx) const override {
		initLouvrePluginState();
		int selection = g_engine->getVariable("Selection");
		if (selection > 100)
			selection -= 100;
		if (selection != 0)
			drawLouvreSelectedObject(selection);
	}
};

struct AffichePorteF : public Script::Command {
	int value;
	AffichePorteF(const Common::Array<Common::String> &args) : value(atoi(args[0].c_str())) {}
	void exec(Script::ExecutionContext &ctx) const override {
		initLouvrePluginState();
		drawLouvreInventoryObjects(value != 0);
	}
};

struct SelectPorteF : public Script::Command {
	int value;
	Common::String var;
	SelectPorteF(const Common::Array<Common::String> &args) : value(atoi(args[0].c_str())), var(args[1]) {}
	void exec(Script::ExecutionContext &ctx) const override {
		initLouvrePluginState();
		g_engine->setVariable(var, 0);
		if (g_engine->getVariable("CoffreSelect") != 0) {
			exchangeLouvreChestObject(0, value);
		} else if (messengerInventorySlot(value) != 0) {
			setLouvreSelectedSlot(value, 1);
		}
		g_engine->setVariable(var, g_engine->getVariable("Selection") != 0);
	}
};

struct SelectCoffre : public Script::Command {
	int value;
	Common::String var;
	SelectCoffre(const Common::Array<Common::String> &args) : value(atoi(args[0].c_str())), var(args[1]) {}
	void exec(Script::ExecutionContext &ctx) const override {
		initLouvrePluginState();
		g_engine->setVariable(var, 0);
		if (g_engine->getVariable("Selection") != 0) {
			exchangeLouvreChestObject(value, 0);
		} else {
			int index = g_engine->getVariable("CoffreOffset") + value - 1;
			if (index >= 0 && index < kLouvreChestSize && g_louvrePluginState.objectChest[index] != 0)
				drawLouvreChestSelection(value);
		}
		g_engine->setVariable(var, g_engine->getVariable("CoffreSelect") != 0);
	}
};

struct LoadSave_Init_Slots : public Script::Command {
	int slots;

	LoadSave_Init_Slots(const Common::Array<Common::String> &args) : slots(atoi(args[0].c_str())) {}
	void exec(Script::ExecutionContext &ctx) const override {
		debug("LoadSave_Init_Slots %d", slots);
	}
};

struct LoadSave_Draw_Slot : public Script::Command {
	int slot;
	int face;
	int x;
	int y;

	LoadSave_Draw_Slot(const Common::Array<Common::String> &args) : slot(atoi(args[0].c_str())),
																	face(atoi(args[1].c_str())),
																	x(atoi(args[2].c_str())),
																	y(atoi(args[3].c_str())) {}
	void exec(Script::ExecutionContext &ctx) const override {
		debug("LoadSave_Draw_Slot %d %d %d %d", slot, face, x, y);
		g_engine->drawSlot(slot, face, x, y);
	}
};

struct LoadSave_Test_Slot : public Script::Command {
	int slot;
	Common::String show;
	Common::String hide;

	LoadSave_Test_Slot(const Common::Array<Common::String> &args) : slot(atoi(args[0].c_str())), show(args[1]), hide(args[2]) {}
	void exec(Script::ExecutionContext &ctx) const override {
		bool exists = g_engine->testSaveSlot(slot);
		debug("LoadSave_Test_Slot %d %s %s -> %d", slot, show.c_str(), hide.c_str(), exists);
		g_engine->setVariable(show, exists);
		g_engine->setVariable(hide, !exists);
	}
};

struct Set : public Script::Command {
	Common::String var;
	Common::String value;
	Set(const Common::Array<Common::String> &args) {
		switch (args.size()) {
		case 2:
			var = args[0];
			value = args[1];
			break;
		case 1: {
			auto pos = args[0].rfind('=');
			if (pos == Common::String::npos)
				pos = args[0].size();
			var = args[0].substr(0, pos);
			value = args[0].c_str() + pos + 1;
		} break;
		default:
			error("invalid plugin set signature");
		}
	}
	void exec(Script::ExecutionContext &ctx) const override {
		g_engine->setVariable(var, valueOf(value));
	}
};

struct Set_Global_Pan : public Script::Command {
	Common::String arg;
	Set_Global_Pan(const Common::Array<Common::String> &args) : arg(args[0]) {}
	void exec(Script::ExecutionContext &ctx) const override {
		auto value = valueOf(arg);
		debug("set_global_pan %s -> %d", arg.c_str(), value);
		g_engine->setGlobalPan(value);
	}
};

struct Set_Global_Volume : public Script::Command {
	Common::String arg;
	Set_Global_Volume(const Common::Array<Common::String> &args) : arg(args[0]) {}
	void exec(Script::ExecutionContext &ctx) const override {
		auto value = valueOf(arg);
		debug("set_global_volume %s -> %d", arg.c_str(), value);
		g_engine->setGlobalVolume(value);
	}
};

struct Op : public Script::Command {
	Common::String var;
	Common::String negativeVar;
	Common::String arg;
	Op(const Common::Array<Common::String> &args) : var(args[0]), negativeVar(args[1]), arg(args[2]) {}
	void exec(Script::ExecutionContext &ctx) const override {
		int value = g_engine->getVariable(arg);
		g_engine->setVariable(var, value);
		g_engine->setVariable(negativeVar, !value);
	}
};

// Amerzone-specific preload plugin
struct Preload : public Script::Command {
	Common::String arg;
	Preload(const Common::Array<Common::String> &args) : arg(args[0]) {}
	void exec(Script::ExecutionContext &ctx) const override {
		debug("preload %s", arg.c_str());
		if (!arg.empty() && Common::isDigit(arg[0])) {
			g_engine->loadGameState(atoi(arg.c_str()));
		} else {
			auto loaded = g_engine->enterScript();
			debug("preload: loaded: %d", loaded);
			g_engine->setVariable(arg, loaded);
			if (loaded) {
				ctx.running = false;
			}
		}
	}
};

// Amerzone-specific loadsave plugin
struct LoadSave : public Script::Command {
	Common::Array<Common::String> args;
	LoadSave(const Common::Array<Common::String> &args_) : args(args_) {}

	static bool testSlot(int slot) {
		auto status = g_engine->testSaveSlot(slot);
		if (!status)
			return false;

		g_engine->drawSaveCard(slot);
		return true;
	}

	void exec(Script::ExecutionContext &ctx) const override {
		uint n = args.size();
		if (n == 3) {
			auto slot = atoi(args[0].c_str());
			auto &var = args[1];
			auto &negativeVar = args[2];
			debug("loadsave %d %s %s", slot, var.c_str(), negativeVar.c_str());
			int status = 0;
			// Amerzone script checks those:
			// 99 -> true, 98 -> false: continue
			// 99 -> true, 98 -> true: initial main menu
			// 99 -> false, 98 -> true: new game
			// false, false -> black screen
			bool restarted = g_engine->wasRestarted();
			bool loaded = g_engine->wasLoaded();
			debug("engine status, loaded: %d, restarted: %d", loaded, restarted);
			if (slot == 99) {
				// special save idx - continue game
				status = !restarted;
			} else if (slot == 98) {
				// special save idx - new game started
				status = !loaded;
			} else if (slot >= 1 && slot <= 8) {
				status = testSlot(slot);
			} else {
				warning("LoadSave slot %d", slot);
			}
			g_engine->setVariable(var, status);
			g_engine->setVariable(negativeVar, !status);
		} else if (n == 2) {
			auto &srcVar = args[0];
			auto &dstVar = args[1];
			debug("loadsave %s %s", srcVar.c_str(), dstVar.c_str());
			if (!dstVar.empty() && Common::isDigit(dstVar[0])) {
				uint level = atoi(dstVar.c_str());
				uint currentLevel = g_engine->currentAmerzoneLevel();
				if (currentLevel != 0) {
					g_engine->setVariable(srcVar, currentLevel == level);
					return;
				}
			}

			auto value = g_engine->getVariable(srcVar);
			g_engine->setVariable(srcVar, 0);
			g_engine->setVariable(dstVar, value);
			if (!value) {
				for (int slot = 1; slot <= 8; ++slot)
					testSlot(slot);
			}
		} else {
			warning("LoadSave, %u args", n);
			for (uint i = 0; i < n; ++i)
				warning("LoadSave %u: %s", i, args[i].c_str());
		}
	}
};

// Amerzone-specific save plugin
struct Save : public Script::Command {
	int slot;
	Save(const Common::Array<Common::String> &args) : slot(atoi(args[0].c_str())) {}
	void exec(Script::ExecutionContext &ctx) const override {
		debug("Save %d", slot);
		g_engine->setContextLabel("Amerzone");
		g_engine->captureContext();
		auto err = g_engine->saveGameState(slot, {});
		if (err.getCode() != Common::ErrorCode::kNoError)
			error("saving state failed %d", slot);
	}
};

struct LoadSave_Capture_Context : public Script::Command {
	LoadSave_Capture_Context(const Common::Array<Common::String> &args) {}
	void exec(Script::ExecutionContext &ctx) const override {
		debug("LoadSave_Capture_Context");
		g_engine->captureContext();
	}
};

struct LoadSave_Context_Restored : public Script::Command {
	Common::String progress;
	Common::String done;

	LoadSave_Context_Restored(const Common::Array<Common::String> &args) : progress(args[0]), done(args[1]) {}
	void exec(Script::ExecutionContext &ctx) const override {
		int value = g_engine->getVariable(progress);
		debug("LoadSave_Context_Restored: %s -> %d -> %s", progress.c_str(), value, done.c_str());
		g_engine->setVariable(progress, 0);
		g_engine->setVariable(done, value);
	}
};

struct LoadSave_Load : public Script::Command {
	int slot;

	LoadSave_Load(const Common::Array<Common::String> &args) : slot(atoi(args[0].c_str())) {}
	void exec(Script::ExecutionContext &ctx) const override {
		debug("LoadSave_Load %d", slot);
		auto err = g_engine->loadGameState(slot);
		if (err.getCode() != Common::ErrorCode::kNoError)
			error("loading state failed %d", slot);
	}
};

struct LoadSave_Save : public Script::Command {
	int slot;

	LoadSave_Save(const Common::Array<Common::String> &args) : slot(atoi(args[0].c_str())) {}
	void exec(Script::ExecutionContext &ctx) const override {
		debug("LoadSave_Save %d", slot);
		auto err = g_engine->saveGameState(slot, {});
		if (err.getCode() != Common::ErrorCode::kNoError)
			error("saving state failed %d", slot);
	}
};

struct LoadSave_Set_Context_Label : public Script::Command {
	Common::String label;

	LoadSave_Set_Context_Label(const Common::Array<Common::String> &args) : label(args[0]) {}
	void exec(Script::ExecutionContext &ctx) const override {
		g_engine->setContextLabel(label);
	}
};

struct Discocier : public Script::Command {
	Common::String var;
	Discocier(const Common::Array<Common::String> &args) : var(args[0]) {}
	void exec(Script::ExecutionContext &ctx) const override {
		initLouvrePluginState();
		g_engine->setVariable(var, 0);

		int selection = g_engine->getVariable("Selection");
		int slot = selection > 100 ? selection - 100 : selection;
		int objectId = messengerInventorySlot(slot);

		int parts[3] = {0, 0, 0};
		int partCount = 0;
		if (objectId == 3500) {
			partCount = 2;
			parts[0] = g_engine->getVariable("RedPastryA");
			parts[1] = g_engine->getVariable("RedPastryB");
		} else if (objectId == 3700) {
			partCount = 2;
			parts[0] = g_engine->getVariable("YellowSeedA");
			parts[1] = g_engine->getVariable("YellowSeedB");
		} else {
			partCount = louvreSeparateObject(objectId, parts);
		}

		g_engine->setVariable("CurrentAction", 0);
		int emptySlots = 0;
		for (int i = 1; i < 9; ++i)
			if (messengerInventorySlot(i) == 0)
				++emptySlots;

		if (partCount > emptySlots + 1) {
			drawLouvreActionButtons();
			drawLouvreText(28, 63687);
			g_louvrePluginState.rolloverBlockedUntil = g_system->getMillis() + 5000;
			return;
		}

		if (partCount == 0)
			return;

		setMessengerInventorySlot(slot, 0);
		for (int i = partCount - 1; i >= 0; --i) {
			for (int dstSlot = 1; dstSlot < 9; ++dstSlot) {
				if (messengerInventorySlot(dstSlot) == 0) {
					setMessengerInventorySlot(dstSlot, parts[i]);
					break;
				}
			}
		}

		setLouvreSelectedSlot(g_engine->getVariable("Selection"), 2);
		drawLouvreInventoryObjects(true);
		g_engine->setVariable(var, 1);
	}
};

struct Reset : public Script::Command {
	Reset(const Common::Array<Common::String> &args) {}
	void exec(Script::ExecutionContext &ctx) const override {
		initLouvrePluginState();
		for (int i = 0; i < kLouvreChestSize; ++i)
			g_louvrePluginState.objectChest[i] = 0;
		for (int i = 1; i <= kMessengerInventorySlots; ++i)
			setMessengerInventorySlot(i, 0);
	}
};

struct Restart : public Script::Command {
	Restart(const Common::Array<Common::String> &args) {}
	void exec(Script::ExecutionContext &ctx) const override {
		g_engine->restart();
	}
};

struct MemoryRelease : public Script::Command {
	MemoryRelease(const Common::Array<Common::String> &args) {}
	void exec(Script::ExecutionContext &ctx) const override {
		debug("MemoryRelease");
	}
};

struct DrawTextSelection : public Script::Command {
	DrawTextSelection(const Common::Array<Common::String> &args) {}
	void exec(Script::ExecutionContext &ctx) const override {
		initLouvrePluginState();
		if (louvreTextBlocked())
			return;

		static const Common::Rect kTextRect(62, 402, 354, 466);
		clearLouvreText();

		int selection = g_engine->getVariable("Selection");
		if (selection > 100)
			selection -= 100;
		if (selection != 0) {
			int objectId = messengerInventorySlot(selection);
			if (objectId != 0)
				g_engine->drawArchiveText(objectId, kTextRect, 14, true, 1987);
		}
	}
};

struct CarteDestination : public Script::Command {
	Common::String varX;
	Common::String varY;
	CarteDestination(const Common::Array<Common::String> &args) : varX(args[0]), varY(args[1]) {}
	void exec(Script::ExecutionContext &ctx) const override {
		int comefrom = g_engine->getVariable("comefrom");
		if (comefrom > 10000000)
			comefrom -= 10000000;

		Common::String value = Common::String::format("%d", comefrom);
		if (value.size() >= 1)
			g_engine->setVariable(varX, value[0] - '0');
		if (value.size() >= 3)
			g_engine->setVariable(varY, atoi(value.substr(1, 2).c_str()));
	}
};

struct Scroll : public Script::Command {
	int value;
	Scroll(const Common::Array<Common::String> &args) : value(atoi(args[0].c_str())) {}
	void exec(Script::ExecutionContext &ctx) const override {
		initLouvrePluginState();
		updateLouvreChestScroll(value);
	}
};

struct Waves : public Script::Command {
	Waves(const Common::Array<Common::String> &args) {}
	void exec(Script::ExecutionContext &ctx) const override {
		g_engine->showWaves();
	}
};

struct Rollover : public Script::Command {
	int textId;
	RolloverType type;

	Rollover(const Common::Array<Common::String> &args, RolloverType t = RolloverType::Default) : textId(atoi(args[0].c_str())), type(t) {}
	void exec(Script::ExecutionContext &ctx) const override {
		g_engine->rollover(textId, type);
	}
};

struct RolloverMalette : public Rollover {
	RolloverMalette(const Common::Array<Common::String> &args) : Rollover(args, RolloverType::Malette) {}
};

struct RolloverSecretaire : public Rollover {
	RolloverSecretaire(const Common::Array<Common::String> &args) : Rollover(args, RolloverType::Secretaire) {}
};

struct PorteFRollover : public Script::Command {
	int arg;

	PorteFRollover(const Common::Array<Common::String> &args) : arg(atoi(args[0].c_str())) {}
	void exec(Script::ExecutionContext &ctx) const override {
		initLouvrePluginState();
		if (louvreTextBlocked())
			return;

		if (arg == 28) {
			drawLouvreText(arg, 63687);
			g_louvrePluginState.rolloverBlockedUntil = g_system->getMillis() + 5000;
		} else {
			drawLouvreText(arg);
		}
	}
};

struct SaveVariable : public Script::Command {
	SaveVariable(const Common::Array<Common::String> &args) {}
	void exec(Script::ExecutionContext &ctx) const override {
		g_engine->saveVariables();
	}
};

struct LoadVariable : public Script::Command {
	LoadVariable(const Common::Array<Common::String> &args) {}
	void exec(Script::ExecutionContext &ctx) const override {
		g_engine->loadVariables();
	}
};

struct Init2 : public Script::Command {
	Common::String var;
	Init2(const Common::Array<Common::String> &args) : var(args[0]) {}
	void exec(Script::ExecutionContext &ctx) const override {
		g_engine->setVariable(var, 0);
	}
};

struct Init : public Script::Command {
	int value;
	Common::String var;
	Init(const Common::Array<Common::String> &args) : value(atoi(args[0].c_str())), var(args[1]) {}
	void exec(Script::ExecutionContext &ctx) const override {
		initLouvrePluginState();
		g_engine->setVariable(var, value == 2 ? 1 : 0);
	}
};

struct End : public Script::Command {
	End(const Common::Array<Common::String> &args) {}
	void exec(Script::ExecutionContext &ctx) const override {
		debug("plugin end (quit)");
		g_engine->quitGame();
	}
};

#define PLUGIN_LIST(E)               \
	E(Add)                           \
	E(AddCoffreObject)               \
	E(AddObject)                     \
	E(AfficheCoffre)                 \
	E(AfficheImage)                  \
	E(AffichePorteF)                 \
	E(AfficheSelection)              \
	E(CarteDestination)              \
	E(ChangeCurseur)                 \
	E(CopyVar)                       \
	E(Cmp)                           \
	E(Discocier)                     \
	E(DrawTextSelection)             \
	E(End)                           \
	E(ExeDemo)                       \
	E(GetMonde4)                     \
	E(SetMonde4)                     \
	E(Init)                          \
	E(Init2)                         \
	E(IsPresent)                     \
	E(KillTimer)                     \
	E(InitCoffre)                    \
	E(IsHere)                        \
	E(LoadCoffre)                    \
	E(SaveCoffre)                    \
	E(SelectPorteF)                  \
	E(SelectCoffre)                  \
	E(Op)                            \
	E(Set)                           \
	E(Preload)                       \
	E(LoadSave)                      \
	E(Save)                          \
	E(LoadSave_Capture_Context)      \
	E(LoadSave_Context_Restored)     \
	E(LoadSave_Enter_Script)         \
	E(LoadSave_Init_Slots)           \
	E(LoadSave_Load)                 \
	E(LoadSave_Save)                 \
	E(LoadSave_Set_Context_Label)    \
	E(LoadSave_Draw_Slot)            \
	E(LoadSave_Test_Slot)            \
	E(LoadVariable)                  \
	E(MemoryRelease)                 \
	E(MultiCD_Set_Transition_Script) \
	E(MultiCD_If_Next_Script)        \
	E(MultiCD_Use_Install_Path)      \
	E(MultiCD_Set_Next_Script)       \
	E(PauseTimer)                    \
	E(Play_AnimBloc)                 \
	E(Play_AnimBloc_Number)          \
	E(Play_Movie)                    \
	E(Reset)                         \
	E(Restart)                       \
	E(RemoveObject)                  \
	E(Rollover)                      \
	E(RolloverMalette)               \
	E(RolloverSecretaire)            \
	E(PorteFRollover)                \
	E(SaveVariable)                  \
	E(Select)                        \
	E(Set_Global_Pan)                \
	E(Set_Global_Volume)             \
	E(Scroll)                        \
	E(StartCible)                    \
	E(StopAffiche)                   \
	E(Stop_AnimBloc)                 \
	E(StopCible)                     \
	E(TestCible)                     \
	E(DoAction)                      \
	E(StartTimer)                    \
	E(Sub)                           \
	E(Until)                         \
	E(UpdateStage)                   \
	E(While)                         \
	E(Waves)                         \
	/* */

#define ADD_PLUGIN(NAME)             \
	if (cmd.equalsIgnoreCase(#NAME)) \
		return Script::CommandPtr(new NAME(args));

Script::CommandPtr createCommand(const Common::String &cmd, const Common::Array<Common::String> &args, int lineno) {
	PLUGIN_LIST(ADD_PLUGIN)
	error("unhandled plugin command %s at line %d", cmd.c_str(), lineno);
}

struct IfAnd : public Script::Conditional {
	IfAnd(Common::Array<Common::String> args) : Script::Conditional(Common::move(args)) {}
	void exec(Script::ExecutionContext &ctx) const override {
		bool result = true;
		for (auto &var : vars) {
			if (var.empty() || !g_engine->hasVariable(var))
				continue;
			auto value = g_engine->getVariable(var);
			debug("ifand, %s: %d", var.c_str(), value);
			if (!value)
				result = false;
		}
		if (!result) {
			debug("ifand: not executing conditional block");
			return;
		}
		debug("ifand: executing conditional block");
		target->exec(ctx);
	}
};

struct IfOr : public Script::Conditional {
	IfOr(Common::Array<Common::String> args) : Script::Conditional(Common::move(args)) {}
	void exec(Script::ExecutionContext &ctx) const override {
		bool result = false;
		for (auto &var : vars) {
			if (var.empty() || !g_engine->hasVariable(var))
				continue;
			auto value = g_engine->getVariable(var);
			debug("ifor, %s: %d", var.c_str(), value);
			if (value)
				result = true;
		}
		if (!result)
			return;
		debug("ifor: executing conditional block");
		target->exec(ctx);
	}
};

struct SetVar : public Script::Command {
	Common::String name;
	int value;

	SetVar(Common::String n, int v) : name(Common::move(n)), value(v) {}
	void exec(Script::ExecutionContext &ctx) const override {
		g_engine->setVariable(name, value);
	}
};

struct GoSub : public Script::Command {
	Common::String label;
	GoSub(const Common::String &l) : label(l) {}
	void exec(Script::ExecutionContext &ctx) const override {
		debug("gosub %s", label.c_str());
		auto *scope = ctx.rootScope ? ctx.rootScope : ctx.scope;
		assert(scope);
		auto *labelPtr = scope->findLabel(label);
		if (!labelPtr) {
			warning("gosub: label %s not found", label.c_str());
			return;
		}
		Script::ExecutionContext sub = {};
		sub.subroutine = true;
		sub.rootScope = scope;
		scope->exec(sub, labelPtr->offset);
	}
};

struct EndScript : public Script::Command {
	EndScript() {}
	void exec(Script::ExecutionContext &ctx) const override {
		debug("end");
		ctx.running = false;
		g_engine->end();
	}
};

struct Return : public Script::Command {
	Return() {}
	void exec(Script::ExecutionContext &ctx) const override {
		ctx.running = false;
		if (ctx.subroutine) {
			debug("return to caller");
		} else {
			debug("return to previous warp");
			g_engine->returnToWarp();
		}
	}
};

struct SetCursorDefault : public Script::Command {
	int idx;
	Common::String fname;
	SetCursorDefault(int i, Common::String f) : idx(i), fname(Common::move(f)) {}
	void exec(Script::ExecutionContext &ctx) const override {
		if (idx >= 0)
			g_engine->setCursorDefault(idx, fname);
	}
};

struct SetCursor : public Script::Command {
	Common::String fname;
	Common::String wname;
	int idx;
	SetCursor(Common::String f, Common::String w, int i) : fname(Common::move(f)), wname(Common::move(w)), idx(i) {}
	void exec(Script::ExecutionContext &ctx) const override {
		g_engine->setCursor(fname, wname, idx);
	}
};

struct HideCursor : public Script::Command {
	Common::String warp;
	int idx;
	HideCursor(Common::String w, int i) : warp(Common::move(w)), idx(i) {}
	void exec(Script::ExecutionContext &ctx) const override {
		g_engine->hideCursor(warp, idx);
	}
};

struct ResetLockKey : public Script::Command {
	void exec(Script::ExecutionContext &ctx) const override {
		g_engine->resetLockKey();
	}
};

struct LockKey : public Script::Command {
	int idx;
	Common::String warp;

	LockKey(int i, Common::String w) : idx(i), warp(Common::move(w)) {}

	void exec(Script::ExecutionContext &ctx) const override {
		debug("lock key %d: %s", idx, warp.c_str());
		g_engine->lockKey(idx, warp);
	}
};

struct SetZoom : public Script::Command {
	float fov;
	SetZoom(float f) : fov(f) {}

	void exec(Script::ExecutionContext &ctx) const override {
		g_engine->setZoom(fov);
	}
};

struct AngleXMax : public Script::Command {
	float xMax;
	AngleXMax(float x) : xMax(x) {}

	void exec(Script::ExecutionContext &ctx) const override {
		debug("anglexmax %g", xMax);
		g_engine->setXMax(xMax);
	}
};

struct AngleYMax : public Script::Command {
	float yMin, yMax;
	AngleYMax(float min, float max) : yMin(min), yMax(max) {}

	void exec(Script::ExecutionContext &ctx) const override {
		if (yMin != yMax) {
			debug("angleymax %g %g", yMin, yMax);
			g_engine->setYMax(yMin, yMax);
		} else {
			debug("angleymax: reset");
			g_engine->resetYMax();
		}
	}
};

struct SetAngle : public Script::Command {
	float x, y;
	SetAngle(float x_, float y_) : x(x_), y(y_) {}

	void exec(Script::ExecutionContext &ctx) const override {
		debug("setangle %g %g", x, y);
		g_engine->setAngle(x, y);
	}
};

struct SetNord : public Script::Command {
	float angle;
	SetNord(float a) : angle(a) {}

	void exec(Script::ExecutionContext &ctx) const override {
		debug("setnord %g", angle);
		g_engine->setNord(angle);
	}
};

struct InterpolAngle : public Script::Command {
	float x, y, speed, zoom;

	InterpolAngle(float x_, float y_, float s, float z) : x(x_), y(y_), speed(s), zoom(z) {}

	void exec(Script::ExecutionContext &ctx) const override {
		g_engine->interpolateAngle(x, y, speed, zoom);
	}
};

struct GoToWarp : public Script::Command {
	Common::String warp;
	GoToWarp(Common::String w) : warp(Common::move(w)) {}

	void exec(Script::ExecutionContext &ctx) const override {
		// terminate script after successful warp.
		// continue if not (for instance, goto to the same location)
		if (g_engine->goToWarp(warp))
			ctx.running = false;
	}
};

struct PlaySound : public Script::Command {
	Common::String sound;
	int volume;
	int loops;
	Audio::Mixer::SoundType type;

	PlaySound(Common::String s, int v, int l, Audio::Mixer::SoundType t = Audio::Mixer::kSFXSoundType) : sound(Common::move(s)), volume(v), loops(l), type(t) {}

	void exec(Script::ExecutionContext &ctx) const override {
		g_engine->playSound(sound, type, volume, loops);
	}
};

struct PlayRandomSound : public PlaySound {
	int unk;

	PlayRandomSound(Common::String s, int v, int u, int l) : PlaySound(Common::move(s), v, l), unk(u) {}

	void exec(Script::ExecutionContext &ctx) const override {
		debug("PlayRandomSound %s %d %d %d", sound.c_str(), volume, unk, loops);
		PlaySound::exec(ctx);
	}
};

struct PlayMusique : public PlaySound {
	PlayMusique(Common::String s, int v) : PlaySound(Common::move(s), v, -1, Audio::Mixer::kMusicSoundType) {}
};

struct StopSound : public Script::Command {
	Common::String sound;

	StopSound(Common::String s) : sound(Common::move(s)) {}

	void exec(Script::ExecutionContext &ctx) const override {
		g_engine->stopSound(sound);
	}
};

struct PlaySound3D : public Script::Command {
	Common::String sound;
	int volume;
	float angle;
	int loops;

	PlaySound3D(Common::String s, int v, float a, int l) : sound(Common::move(s)), volume(v), angle(a), loops(l) {}

	void exec(Script::ExecutionContext &ctx) const override {
		g_engine->playSound(sound, Audio::Mixer::kSFXSoundType, volume, loops, true, angle);
	}
};

struct StopSound3D : public Script::Command {
	Common::String sound;

	StopSound3D(Common::String s) : sound(Common::move(s)) {}

	void exec(Script::ExecutionContext &ctx) const override {
		g_engine->stopSound(sound);
	}
};

struct Fade : public Script::Command {
	int start, stop, speed;

	Fade(int a0, int a1, int a2) : start(a0), stop(a1), speed(a2) {}
	void exec(Script::ExecutionContext &ctx) const override {
		g_engine->fade(start, stop, speed);
	}
};

struct Transfade : public Script::Command {
	int speed;

	Transfade(int a0) : speed(a0) {}
	void exec(Script::ExecutionContext &ctx) const override {
		g_engine->transFade(speed);
	}
};

} // namespace
} // namespace PhoenixVR

#endif
