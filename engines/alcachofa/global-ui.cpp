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

#include "alcachofa/global-ui.h"
#include "alcachofa/menu.h"
#include "alcachofa/alcachofa.h"
#include "alcachofa/script.h"

using namespace Common;

namespace Alcachofa {

// originally the inventory only reacts to exactly top-left/bottom-right which is fine in
// fullscreen when you just slam the mouse cursor into the corner.
// In any other scenario this is cumbersome so I expand this area.
// And it is still pretty bad, especially in windowed mode so there is a key to open/close as well
static constexpr int16 kInventoryTriggerSize = 10;

Rect openInventoryTriggerBounds() {
	int16 size = kInventoryTriggerSize * 1024 / g_system->getWidth();
	return Rect(0, 0, size, size);
}

Rect closeInventoryTriggerBounds() {
	int16 size = kInventoryTriggerSize * 1024 / g_system->getWidth();
	return Rect(g_system->getWidth() - size, g_system->getHeight() - size, g_system->getWidth(), g_system->getHeight());
}

GlobalUI::GlobalUI() {
	auto &world = g_engine->world();
	_generalFont.reset(new Font(world.getGlobalAnimationName(GlobalAnimationKind::GeneralFont)));
	_dialogFont.reset(new Font(world.getGlobalAnimationName(GlobalAnimationKind::DialogFont)));
	_iconMortadelo.reset(new Animation(world.getGlobalAnimationName(GlobalAnimationKind::MortadeloIcon)));
	_iconFilemon.reset(new Animation(world.getGlobalAnimationName(GlobalAnimationKind::FilemonIcon)));
	_iconInventory.reset(new Animation(world.getGlobalAnimationName(GlobalAnimationKind::InventoryIcon)));

	_generalFont->load();
	_dialogFont->load();
	_iconMortadelo->load();
	_iconFilemon->load();
	_iconInventory->load();
}

void GlobalUI::startClosingInventory() {
	_isOpeningInventory = false;
	_isClosingInventory = true;
	_timeForInventory = g_engine->getMillis();
	updateClosingInventory(); // prevents the first frame of closing to not render the inventory overlay
}

void GlobalUI::updateClosingInventory() {
	static constexpr uint32 kDuration = 300;
	static constexpr float kSpeed = -10 / 3.0f / 1000.0f;

	uint32 deltaTime = g_engine->getMillis() - _timeForInventory;
	if (!_isClosingInventory || deltaTime >= kDuration)
		_isClosingInventory = false;
	else
		g_engine->world().inventory().drawAsOverlay((int32)(g_system->getHeight() * (deltaTime * kSpeed)));
}

bool GlobalUI::updateOpeningInventory() {
	static constexpr float kSpeed = 10 / 3.0f / 1000.0f;
	if (g_engine->menu().isOpen() || !g_engine->player().isGameLoaded())
		return false;

	const bool userWantsToOpenInventory =
		openInventoryTriggerBounds().contains(g_engine->input().mousePos2D()) ||
		g_engine->input().wasInventoryKeyPressed();

	if (_isOpeningInventory) {
		uint32 deltaTime = g_engine->getMillis() - _timeForInventory;
		if (deltaTime >= 1000) {
			_isOpeningInventory = false;
			g_engine->world().inventory().open();
		} else {
			deltaTime = MIN<uint32>(300, deltaTime);
			g_engine->world().inventory().drawAsOverlay((int32)(g_system->getHeight() * (deltaTime * kSpeed - 1)));
		}
		return true;
	} else if (userWantsToOpenInventory) {
		_isClosingInventory = false;
		_isOpeningInventory = true;
		_timeForInventory = g_engine->getMillis();
		g_engine->player().activeCharacter()->stopWalking();
		g_engine->world().inventory().updateItemsByActiveCharacter();
		return true;
	}
	return false;
}

Animation *GlobalUI::activeAnimation() const {
	return g_engine->player().activeCharacterKind() == MainCharacterKind::Mortadelo
		? _iconFilemon.get()
		: _iconMortadelo.get();
}

bool GlobalUI::isHoveringChangeButton() const {
	auto mousePos = g_engine->input().mousePos2D();
	auto anim = activeAnimation();
	auto offset = anim->totalFrameOffset(0);
	auto bounds = anim->frameBounds(0);

	const int minX = g_system->getWidth() + offset.x;
	const int maxY = bounds.height() + offset.y;
	return mousePos.x >= minX && mousePos.y <= maxY;
}

bool GlobalUI::updateChangingCharacter() {
	auto &player = g_engine->player();
	if (g_engine->menu().isOpen() ||
		!player.isGameLoaded() ||
		_isOpeningInventory)
		return false;
	_changeButton.frameI() = 0;

	if (!isHoveringChangeButton())
		return false;
	if (g_engine->input().wasMouseLeftPressed()) {
		player.pressedObject() = &_changeButton;
		return true;
	}
	if (player.pressedObject() != &_changeButton)
		return true;

	player.setActiveCharacter(player.inactiveCharacter()->kind());
	player.heldItem() = nullptr;
	g_engine->camera().setFollow(player.activeCharacter());
	g_engine->camera().restore(0);
	player.changeRoom(player.activeCharacter()->room()->name(), false);
	g_engine->game().onUserChangedCharacter();

	int32 characterJingle = g_engine->script().variable(
		player.activeCharacterKind() == MainCharacterKind::Mortadelo
		? "PistaMorta"
		: "PistaFile"
	);
	g_engine->sounds().startMusic(characterJingle);
	g_engine->sounds().queueMusic(player.currentRoom()->musicID());

	_changeButton.setAnimation(activeAnimation());
	_changeButton.start(false);
	return true;
}

void GlobalUI::drawChangingButton() {
	auto &player = g_engine->player();
	if (g_engine->menu().isOpen() ||
		!player.isGameLoaded() ||
		!player.semaphore().isReleased() ||
		_isOpeningInventory ||
		_isClosingInventory)
		return;

	auto anim = activeAnimation();
	if (!_changeButton.hasAnimation() || &_changeButton.animation() != anim) {
		_changeButton.setAnimation(anim);
		_changeButton.pause();
		_changeButton.lastTime() = 42 * (anim->frameCount() - 1) + 1;
	}

	_changeButton.topLeft() = { (int16)(g_system->getWidth() + 2), -2 };
	if (isHoveringChangeButton() &&
		g_engine->input().isMouseLeftDown() &&
		player.pressedObject() == &_changeButton) {
		_changeButton.topLeft().x -= 2;
		_changeButton.topLeft().y += 2;
	}

	_changeButton.order() = -9;
	_changeButton.update();
	g_engine->drawQueue().add<AnimationDrawRequest>(_changeButton, false, BlendMode::AdditiveAlpha);
}

struct CenterBottomTextTask final : public Task {
	CenterBottomTextTask(Process &process, int32 dialogId, uint32 durationMs)
		: Task(process)
		, _dialogId(dialogId)
		, _durationMs(durationMs) {}

	CenterBottomTextTask(Process &process, Serializer &s)
		: Task(process) {
		CenterBottomTextTask::syncGame(s);
	}

	TaskReturn run() override {
		Font &font = g_engine->globalUI().dialogFont();
		const char *text = g_engine->world().getDialogLine(_dialogId);
		const Point pos(
			g_system->getWidth() / 2,
			g_system->getHeight() - 200
		);

		TASK_BEGIN;
		_startTime = g_engine->getMillis();
		while (g_engine->getMillis() - _startTime < _durationMs) {
			if (process().isActiveForPlayer()) {
				g_engine->drawQueue().add<TextDrawRequest>(
					font, text, pos, -1, true, kWhite, -kForegroundOrderCount + 1);
			}
			TASK_YIELD(1);
		}
		TASK_END;
	}

	void debugPrint() override {
		uint32 remaining = g_engine->getMillis() - _startTime <= _durationMs
			? _durationMs - (g_engine->getMillis() - _startTime)
			: 0;
		g_engine->console().debugPrintf("CenterBottomText (%d) with %ums remaining\n", _dialogId, remaining);
	}

	void syncGame(Serializer &s) override {
		Task::syncGame(s);
		s.syncAsSint32LE(_dialogId);
		s.syncAsUint32LE(_startTime);
		s.syncAsUint32LE(_durationMs);
	}

	const char *taskName() const override;

private:
	int32 _dialogId = 0;
	uint32 _startTime = 0, _durationMs = 0;
};
DECLARE_TASK(CenterBottomTextTask)

Task *showCenterBottomText(Process &process, int32 dialogId, uint32 durationMs) {
	return new CenterBottomTextTask(process, dialogId, durationMs);
}

void GlobalUI::drawScreenStates() {
	if (g_engine->menu().isOpen())
		return;

	auto &drawQueue = g_engine->drawQueue();
	if (_isPermanentFaded)
		drawQueue.add<FadeDrawRequest>(FadeType::ToBlack, 1.0f, -9);
	else
		g_engine->game().drawScreenStates();
}

void GlobalUI::syncGame(Serializer &s) {
	s.syncAsByte(_isPermanentFaded);
}

}
