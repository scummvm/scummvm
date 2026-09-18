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
 */

#include "ripper/scenes/ebz2s_scene.h"

#include "common/debug.h"
#include "common/system.h"

#include "ripper/cursor.h"
#include "ripper/detection.h"
#include "ripper/input.h"
#include "ripper/inventory.h"
#include "ripper/media.h"
#include "ripper/milestones.h"
#include "ripper/modal_dialog.h"
#include "ripper/ripper.h"
#include "ripper/toolbar.h"

namespace Ripper {

namespace {

static const char *const kBackgroundMedia = "ebz2s.pcx";
static const char *const kSceneLabel = "EB_HEAD";
static const uint kDefaultCursor = 14;
static const uint kToolbarMask = 0x84;
static const uint kInventoryAction = 2;
static const uint kHelpAction = 7;
static const uint kHelpResource = 400;
static const uint kBugUnlockFlag = 55;
static const uint kEyeballUnlockFlag = 58;
static const uint kBugConsumedFlag = 105;
static const uint kEyeballConsumedFlag = 108;
static const uint16 kHelpCommand = 0x3b00;

} // End of anonymous namespace

Ebz2sScene::Ebz2sScene(RipperEngine *engine) : Scene(engine) {
}

void Ebz2sScene::resetSelection() {
	_engine->getCursor()->setSelectionIndex(0);
	_engine->getCursor()->dispatchSelectionIndexChange(0);
}

bool Ebz2sScene::runInventory(int grantFlag) {
	const Inventory::Result result = grantFlag >= 0 ?
		_engine->getInventory()->grantAndRun(grantFlag, kSceneLabel,
			"ebz2s-scene-action-14") :
		_engine->getInventory()->run(kSceneLabel);
	debugC(result == Inventory::kLoadFailed ? 1 : 2, kDebugScene,
		"Ripper: EBZ2S inventory completed result=%d grantFlag=%d bugConsumed=%d eyeballConsumed=%d",
		result, grantFlag,
		_engine->getMilestones()->isSet(kBugConsumedFlag),
		_engine->getMilestones()->isSet(kEyeballConsumedFlag));
	resetSelection();
	return result != Inventory::kLoadFailed;
}

Ebz2sScene::Result Ebz2sScene::run(uint sceneArgument) {
	(void)sceneArgument;
	prepare("ebz2s-entry", kDefaultCursor, true);
	Result result = kExited;
	if (!_engine->getMedia()->displayScenePcx(kBackgroundMedia)) {
		result = kLoadFailed;
	} else {
		Milestones *milestones = _engine->getMilestones();
		int grantFlag = -1;
		if (!milestones->isSet(kEyeballUnlockFlag) ||
				milestones->isSet(kEyeballConsumedFlag))
			grantFlag = kEyeballUnlockFlag;
		else if (milestones->isSet(kBugConsumedFlag))
			grantFlag = kBugUnlockFlag;

		debugC(1, kDebugScene,
			"Ripper: entered EBZ2S action menu function=RunEbz2sUnlockGatedActionMenuScene@0x36783 "
			"toolbarMask=0x%03x grantFlag=%d bug=%d,%d eyeball=%d,%d",
			kToolbarMask, grantFlag,
			milestones->isSet(kBugUnlockFlag),
			milestones->isSet(kBugConsumedFlag),
			milestones->isSet(kEyeballUnlockFlag),
			milestones->isSet(kEyeballConsumedFlag));

		if (grantFlag >= 0 && !runInventory(grantFlag)) {
			result = kLoadFailed;
		} else {
			while (!_engine->shouldQuit() &&
					!milestones->isSet(kBugConsumedFlag) &&
					!milestones->isSet(kEyeballConsumedFlag)) {
				if (_engine->getInput()->pollEvents()) {
					_engine->quitGame();
					break;
				}
				while (_engine->getInput()->hasPendingKey()) {
					if (_engine->getInput()->consumeKey() == kHelpCommand &&
							!_engine->getModalDialog()->run(kHelpResource, true)) {
						result = kLoadFailed;
						break;
					}
				}
				if (result == kLoadFailed)
					break;

				const MouseState mouse = _engine->getInput()->publishMouseState();
				int toolbarAction = -1;
				if (_engine->getToolbar()->service(mouse, kToolbarMask,
						&toolbarAction)) {
					_engine->getCursor()->update(kDefaultCursor);
					_engine->getCursor()->setVisible(true);
					if (toolbarAction == kInventoryAction &&
							!runInventory(-1)) {
						result = kLoadFailed;
						break;
					}
					if (toolbarAction == kHelpAction &&
							!_engine->getModalDialog()->run(kHelpResource, true)) {
						result = kLoadFailed;
						break;
					}
					if (toolbarAction >= 0)
						resetSelection();
				} else {
					_engine->getCursor()->update(kDefaultCursor);
					_engine->getCursor()->setVisible(true);
				}
				_engine->getCursor()->refresh();
				presentScreen();
				g_system->delayMillis(10);
			}
			if (result != kLoadFailed &&
					(milestones->isSet(kBugConsumedFlag) ||
					 milestones->isSet(kEyeballConsumedFlag)))
				result = kSolved;
		}
	}

	resetSelection();
	finish("ebz2s-exit", 0, false);
	debugC(result == kLoadFailed ? 1 : 2, kDebugScene,
		"Ripper: left EBZ2S action menu result=%d bugConsumed=%d eyeballConsumed=%d quit=%d",
		result,
		_engine->getMilestones()->isSet(kBugConsumedFlag),
		_engine->getMilestones()->isSet(kEyeballConsumedFlag),
		_engine->shouldQuit());
	return result;
}

} // End of namespace Ripper
