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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "ripper/inventory.h"

#include "common/debug.h"
#include "common/system.h"

#include "ripper/cursor.h"
#include "ripper/detection.h"
#include "ripper/dialogue.h"
#include "ripper/input.h"
#include "ripper/media.h"
#include "ripper/milestones.h"
#include "ripper/modal_dialog.h"
#include "ripper/resources.h"
#include "ripper/ripper.h"

namespace Ripper {

namespace {

static const uint kFirstUnlockFlag = 50;
static const uint kLastUnlockFlag = 69;
static const uint kConsumedFlagOffset = 50;
static const uint kLabelResourceBase = 200;
static const uint kSceneLabelResourceBase = 270;
static const uint kInventoryHelpResource = 0x1bb;
static const uint kInvalidInventoryResource = 0x4d;
static const uint kDefaultCursor = 14;
static const uint kChoiceCursor = 16;
static const uint16 kEscapeCommand = 0x1b;
static const uint16 kHelpCommand = 0x3b00;

} // End of anonymous namespace

Inventory::Inventory(RipperEngine *engine) : _engine(engine),
		_chooser(new DialogueChooser()) {
}

Inventory::~Inventory() {
	delete _chooser;
}

bool Inventory::initialize(ResourceManager &resources) {
	if (!resources.loadGameText(_gameText) ||
			_gameText.size() < kSceneLabelResourceBase +
				(kLastUnlockFlag - kFirstUnlockFlag) ||
			!_chooser->initialize(resources)) {
		warning("Ripper: inventory resources are incomplete");
		return false;
	}
	debugC(1, kDebugScene,
		"Ripper: initialized inventory unlockFlags=%u..%u consumedFlags=%u..%u labels=%u..%u useLabels=%u..%u",
		kFirstUnlockFlag, kLastUnlockFlag,
		kFirstUnlockFlag + kConsumedFlagOffset,
		kLastUnlockFlag + kConsumedFlagOffset,
		kLabelResourceBase, kLabelResourceBase + kLastUnlockFlag - kFirstUnlockFlag,
		kSceneLabelResourceBase,
		kSceneLabelResourceBase + kLastUnlockFlag - kFirstUnlockFlag);
	return true;
}

bool Inventory::isAvailable(uint unlockFlag) const {
	return unlockFlag >= kFirstUnlockFlag && unlockFlag <= kLastUnlockFlag &&
		_engine->getMilestones()->isSet(unlockFlag) &&
		!_engine->getMilestones()->isSet(unlockFlag + kConsumedFlagOffset);
}

bool Inventory::grant(uint unlockFlag, const char *source) {
	if (unlockFlag < kFirstUnlockFlag || unlockFlag > kLastUnlockFlag) {
		warning("Ripper: inventory unlock flag %u is outside %u..%u",
			unlockFlag, kFirstUnlockFlag, kLastUnlockFlag);
		return false;
	}
	// DispatchSceneEntryAction at 0x36892 action 3 sets the unlock bit and
	// clears its paired consumed bit before opening RunUnlockGatedSelectionMenu.
	if (!_engine->getMilestones()->set(unlockFlag, true, source) ||
			!_engine->getMilestones()->set(unlockFlag + kConsumedFlagOffset,
				false, source))
		return false;
	debugC(1, kDebugScene,
		"Ripper: granted inventory unlockFlag=%u item=%u consumedFlag=%u source=%s",
		unlockFlag, unlockFlag - kFirstUnlockFlag,
		unlockFlag + kConsumedFlagOffset, source ? source : "unknown");
	return true;
}

bool Inventory::rebuildChooser(int initialUnlockFlag) {
	_chooser->clearPending();
	uint available = 0;
	for (uint unlockFlag = kFirstUnlockFlag; unlockFlag <= kLastUnlockFlag; ++unlockFlag) {
		if (!isAvailable(unlockFlag))
			continue;
		const uint choiceId = unlockFlag - kFirstUnlockFlag;
		_chooser->appendChoice(_gameText[kLabelResourceBase + choiceId - 1], unlockFlag);
		++available;
	}
	if (available == 0) {
		debugC(2, kDebugScene, "Ripper: inventory has no available items");
		return false;
	}
	if (!_chooser->activateChoices("inventory"))
		return false;
	if (initialUnlockFlag >= 0)
		_chooser->selectResult(initialUnlockFlag, "inventory-initial-item");
	_chooser->draw(true);
	debugC(1, kDebugScene,
		"Ripper: entered inventory available=%u initialUnlockFlag=%d",
		available, initialUnlockFlag);
	return true;
}

void Inventory::closeChooser(const char *reason) {
	if (_chooser->isPending())
		_chooser->dismissForSceneTransition(reason);
	else
		_chooser->clearPending();
	_engine->getCursor()->update(0);
	_engine->getCursor()->setVisible(true);
	_engine->getInput()->discardMouseTransitions();
}

Inventory::ChoiceResult Inventory::executeChoice(uint unlockFlag,
		const Common::String &sceneLabel) {
	if (unlockFlag < kFirstUnlockFlag || unlockFlag > kLastUnlockFlag ||
			!isAvailable(unlockFlag))
		return kChoiceRejected;
	const uint choiceId = unlockFlag - kFirstUnlockFlag;
	const Common::String &requiredSceneLabel =
		_gameText[kSceneLabelResourceBase + choiceId - 1];
	// ExecuteUnlockSelectionChoice at 0x364be resolves the active frame's
	// labelOffset (+5 in the compiled 0x22-byte frame record) and compares it
	// case-insensitively with startup resource 270 + choice ID.
	if (!sceneLabel.equalsIgnoreCase(requiredSceneLabel)) {
		debugC(2, kDebugScene,
			"Ripper: rejected inventory item=%u unlockFlag=%u sceneLabel='%s' requiredLabel='%s'",
			choiceId, unlockFlag, sceneLabel.c_str(), requiredSceneLabel.c_str());
		_engine->getModalDialog()->run(kInvalidInventoryResource, true);
		return kChoiceRejected;
	}
	if ((choiceId == 5 &&
			(!_engine->getMilestones()->isSet(58) ||
			 _engine->getMilestones()->isSet(108))) ||
			(choiceId == 8 && _engine->getMilestones()->isSet(105))) {
		debugC(2, kDebugScene,
			"Ripper: rejected restricted inventory item=%u unlockFlag=%u",
			choiceId, unlockFlag);
		_engine->getModalDialog()->run(kInvalidInventoryResource, true);
		return kChoiceRejected;
	}

	if (choiceId == 1 || choiceId == 5 || choiceId == 8) {
		warning("Ripper: inventory item %u requires an unimplemented dedicated presentation", choiceId);
		return kChoiceFailed;
	}

	if (!_engine->getMilestones()->set(unlockFlag + kConsumedFlagOffset,
			true, "inventory-use"))
		return kChoiceFailed;
	if (choiceId == 0) {
		if (!_engine->getMilestones()->set(26, true, "inventory-scan-card") ||
				!_engine->getMedia()->play("verify.avi", true))
			return kChoiceFailed;
	} else if (choiceId == 6) {
		if (!_engine->getMedia()->play("q_cs_3.avi", true))
			return kChoiceFailed;
	} else if (choiceId == 7) {
		if (!_engine->getMedia()->play("p_shadow.avi", true) ||
				!_engine->getMilestones()->set(unlockFlag + kConsumedFlagOffset,
					false, "inventory-reusable-item"))
			return kChoiceFailed;
	}
	debugC(1, kDebugScene,
		"Ripper: used inventory item=%u unlockFlag=%u consumedFlag=%u sceneLabel='%s'",
		choiceId, unlockFlag, unlockFlag + kConsumedFlagOffset, sceneLabel.c_str());
	return kChoiceUsed;
}

Inventory::Result Inventory::run(const Common::String &sceneLabel,
		int initialUnlockFlag, uint *usedUnlockFlag) {
	if (usedUnlockFlag)
		*usedUnlockFlag = 0;
	_engine->getInput()->drainKeys();
	_engine->getInput()->discardMouseTransitions();
	_engine->getCursor()->update(kDefaultCursor);
	_engine->getCursor()->setVisible(true);
	int selectedUnlockFlag = initialUnlockFlag;
	if (!rebuildChooser(selectedUnlockFlag)) {
		closeChooser("inventory-empty");
		return kCancelled;
	}

	while (!_engine->shouldQuit()) {
		if (_engine->getInput()->pollEvents()) {
			_engine->quitGame();
			break;
		}
		bool selected = false;
		while (_engine->getInput()->hasPendingKey()) {
			const uint16 command = _engine->getInput()->consumeKey();
			if (command == kEscapeCommand) {
				closeChooser("inventory-escape");
				debugC(1, kDebugScene, "Ripper: exited inventory result=cancelled");
				return kCancelled;
			}
			if (command == kHelpCommand) {
				_engine->getModalDialog()->run(kInventoryHelpResource, true);
				_chooser->draw();
				continue;
			}
			uint result = 0;
			if (_chooser->serviceKeyboard(command, result)) {
				selectedUnlockFlag = result;
				selected = true;
				break;
			}
		}
		if (!selected) {
			const MouseState mouse = _engine->getInput()->publishMouseState();
			_chooser->updateHover(mouse.position);
			_engine->getCursor()->update(_chooser->contains(mouse.position) ?
				kChoiceCursor : kDefaultCursor);
			uint result = 0;
			if (_chooser->service(mouse, result)) {
				selectedUnlockFlag = result;
				selected = true;
			}
		}
		if (selected) {
			const ChoiceResult choiceResult = executeChoice(selectedUnlockFlag, sceneLabel);
			if (choiceResult == kChoiceUsed) {
				if (usedUnlockFlag)
					*usedUnlockFlag = selectedUnlockFlag;
				closeChooser("inventory-used");
				return kUsed;
			}
			if (choiceResult == kChoiceFailed) {
				closeChooser("inventory-failed");
				return kLoadFailed;
			}
			if (!rebuildChooser(selectedUnlockFlag)) {
				closeChooser("inventory-empty-after-selection");
				return kCancelled;
			}
		}
		_chooser->draw();
		g_system->updateScreen();
		g_system->delayMillis(10);
	}
	closeChooser("inventory-quit");
	return _engine->shouldQuit() ? kCancelled : kLoadFailed;
}

Inventory::Result Inventory::grantAndRun(uint unlockFlag,
		const Common::String &sceneLabel, const char *source) {
	if (!grant(unlockFlag, source))
		return kLoadFailed;
	return run(sceneLabel, unlockFlag);
}

} // End of namespace Ripper
