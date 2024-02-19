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

#include "common/serializer.h"
#include "common/config-manager.h"
#include "common/func.h"

#include "engines/nancy/nancy.h"
#include "engines/nancy/iff.h"
#include "engines/nancy/input.h"
#include "engines/nancy/sound.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/cursor.h"
#include "engines/nancy/util.h"
#include "engines/nancy/resource.h"

#include "engines/nancy/state/scene.h"
#include "engines/nancy/state/map.h"

#include "engines/nancy/ui/button.h"
#include "engines/nancy/ui/ornaments.h"
#include "engines/nancy/ui/clock.h"

#include "engines/nancy/misc/lightning.h"
#include "engines/nancy/misc/specialeffect.h"

namespace Common {
DECLARE_SINGLETON(Nancy::State::Scene);
}

namespace Nancy {
namespace State {

void Scene::SceneSummary::read(Common::SeekableReadStream &stream) {
	char *buf = new char[0x32];
	int32 x = 0;
	int32 y = 0;
	int32 z = 0;

	stream.seek(0);
	Common::Serializer ser(&stream, nullptr);
	ser.setVersion(g_nancy->getGameType());

	ser.syncBytes((byte *)buf, 0x32);
	description = Common::String(buf);

	readFilename(stream, videoFile);

	// skip 2 unknown bytes
	ser.skip(2);
	videoFormat = stream.readUint16LE();

	// Load the palette data in The Vampire Diaries
	ser.skip(4, kGameTypeVampire, kGameTypeVampire);
	readFilenameArray(ser, palettes, 3, kGameTypeVampire, kGameTypeVampire);

	sound.readScene(stream);

	ser.syncAsUint16LE(panningType);
	ser.syncAsUint16LE(numberOfVideoFrames, kGameTypeVampire, kGameTypeNancy2);
	ser.syncAsUint16LE(degreesPerRotation);
	ser.syncAsUint16LE(totalViewAngle, kGameTypeVampire, kGameTypeNancy2);
	ser.syncAsUint32LE(x, kGameTypeNancy3);
	ser.syncAsUint32LE(y, kGameTypeNancy3);
	ser.syncAsUint32LE(z, kGameTypeNancy3);
	listenerPosition.set(x, y, z);
	ser.syncAsUint16LE(horizontalScrollDelta);
	ser.syncAsUint16LE(verticalScrollDelta);
	ser.syncAsUint16LE(horizontalEdgeSize);
	ser.syncAsUint16LE(verticalEdgeSize);
	ser.syncAsUint16LE((uint32 &)slowMoveTimeDelta);
	ser.syncAsUint16LE((uint32 &)fastMoveTimeDelta);
	ser.skip(1); // CD required for scene

	auto *bootSummary = GetEngineData(BSUM);
	assert(bootSummary);

	if (bootSummary->overrideMovementTimeDeltas) {
		slowMoveTimeDelta = bootSummary->slowMovementTimeDelta;
		fastMoveTimeDelta = bootSummary->fastMovementTimeDelta;
	}

	delete[] buf;
}

void Scene::SceneSummary::readTerse(Common::SeekableReadStream &stream) {
	char *buf = new char[0x32];
	stream.read(buf, 0x32);
	description = buf;
	readFilename(stream, videoFile);
	sound.readTerse(stream);
}

Scene::Scene() :
		_state (kInit),
		_lastHintCharacter(-1),
		_lastHintID(-1),
		_gameStateRequested(NancyState::kNone),
		_frame(),
		_viewport(),
		_textbox(),
		_inventoryBox(),
		_menuButton(nullptr),
		_helpButton(nullptr),
		_viewportOrnaments(nullptr),
		_textboxOrnaments(nullptr),
		_inventoryBoxOrnaments(nullptr),
		_clock(nullptr),
		_actionManager(),
		_difficulty(0),
		_activeMovie(nullptr),
		_activeConversation(nullptr),
		_lightning(nullptr),
		_destroyOnExit(false),
		_isRunningAd(false),
		_hotspotDebug(50) {}

Scene::~Scene() {
	delete _helpButton;
	delete _menuButton;
	delete _viewportOrnaments;
	delete _textboxOrnaments;
	delete _inventoryBoxOrnaments;
	delete _clock;
	delete _lightning;

	clearPuzzleData();
}

void Scene::process() {
	switch (_state) {
	case kInit:
		init();

		if (_state != kLoad) {
			break;
		}

		// fall through
	case kLoad:
		load();
		// fall through
	case kStartSound:
		_state = kRun;
		if (_sceneState.currentScene.continueSceneSound == kLoadSceneSound) {
			g_nancy->_sound->stopAndUnloadSceneSpecificSounds();
			g_nancy->_sound->loadSound(_sceneState.summary.sound);
			g_nancy->_sound->playSound(_sceneState.summary.sound);
		}
		// fall through
	case kRun:
		run();
		break;
	}
}

void Scene::onStateEnter(const NancyState::NancyState prevState) {
	if (_state != kInit) {
		registerGraphics();

		if (prevState != NancyState::kPause) {
			g_nancy->setTotalPlayTime((uint32)_timers.pushedPlayTime);
		}

		_actionManager.onPause(false);

		g_nancy->_graphics->redrawAll();

		if (getHeldItem() != -1) {
			g_nancy->_cursor->setCursorItemID(getHeldItem());
		}

		if (prevState == NancyState::kPause) {
			g_nancy->_sound->pauseAllSounds(false);
		} else {
			g_nancy->_sound->pauseSceneSpecificSounds(false);
		}

		g_nancy->_sound->stopSound("MSND");
	}

	g_nancy->_hasJustSaved = false;
}

bool Scene::onStateExit(const NancyState::NancyState nextState) {
	if (_state == kRun) {
		// Exiting the state outside the kRun state means we've encountered an error
		g_nancy->_graphics->screenshotScreen(_lastScreenshot);
	}

	if (nextState != NancyState::kPause) {
		_timers.pushedPlayTime = g_nancy->getTotalPlayTime();
	}

	_actionManager.onPause(true);

	if (nextState == NancyState::kPause) {
		g_nancy->_sound->pauseAllSounds(true);
	} else {
		g_nancy->_sound->pauseSceneSpecificSounds(true);
	}

	_gameStateRequested = NancyState::kNone;

	// Re-register the clock so the open/close animation can continue playing inside Map
	if (nextState == NancyState::kMap && g_nancy->getGameType() == kGameTypeVampire) {
		_clock->registerGraphics();
	}

	return _destroyOnExit;
}

void Scene::changeScene(const SceneChangeDescription &sceneDescription) {
	if (sceneDescription.sceneID == kNoScene || _state == kLoad) {
		return;
	}

	_sceneState.nextScene = sceneDescription;
	_state = kLoad;
}

void Scene::pushScene(int16 itemID) {
	if (itemID == -1) {
		_sceneState.pushedScene = _sceneState.currentScene;
		_sceneState.isScenePushed = true;
	} else {
		if (_sceneState.isInvScenePushed) {
			// Re-add current pushed item
			addItemToInventory(_sceneState.pushedInvItemID);
		} else {
			// Only set this when another item hasn't been pushed, otherwise
			// the player will never be able to exit
			_sceneState.pushedInvScene = _sceneState.currentScene;
		}

		_sceneState.isInvScenePushed = true;
		_sceneState.pushedInvItemID = itemID;
	}
}

void Scene::popScene(bool inventory) {
	if (!inventory || _sceneState.pushedInvItemID == -1) {
		_sceneState.pushedScene.continueSceneSound = true;
		changeScene(_sceneState.pushedScene);
		_sceneState.isScenePushed = false;
	} else {
		_sceneState.pushedInvScene.continueSceneSound = true;
		changeScene(_sceneState.pushedInvScene);
		_sceneState.isInvScenePushed = false;
		addItemToInventory(_sceneState.pushedInvItemID);
		_sceneState.pushedInvItemID = kEvNoEvent;
		_sceneState.pushedInvScene.sceneID = kNoScene;
	}
}

void Scene::setPlayerTime(Time time, byte relative) {
	if (relative == kRelativeClockBump) {
		// Relative, add the specified time to current playerTime
		_timers.playerTime += time;
	} else {
		// Absolute, maintain days but replace hours and minutes
		_timers.playerTime = _timers.playerTime.getDays() * 86400000 + time;
	}

	auto *bootSummary = GetEngineData(BSUM);
	assert(bootSummary);

	_timers.playerTimeNextMinute = g_nancy->getTotalPlayTime() + bootSummary->playerTimeMinuteLength;
}

byte Scene::getPlayerTOD() const {
	if (g_nancy->getGameType() <= kGameTypeNancy1) {
		if (_timers.playerTime.getHours() >= 7 && _timers.playerTime.getHours() < 18) {
			return kPlayerDay;
		} else if (_timers.playerTime.getHours() >= 19 || _timers.playerTime.getHours() < 6) {
			return kPlayerNight;
		} else {
			return kPlayerDuskDawn;
		}
	} else if (g_nancy->getGameType() <= kGameTypeNancy5) {
		// nancy2 and up removed dusk/dawn
		if (_timers.playerTime.getHours() >= 6 && _timers.playerTime.getHours() < 18) {
			return kPlayerDay;
		} else {
			return kPlayerNight;
		}
	} else {
		// nancy6 added the day start/end times (in minutes) to BSUM
		auto *bootSummary = GetEngineData(BSUM);
		assert(bootSummary);

		uint16 minutes = _timers.playerTime.getHours() * 60 + _timers.playerTime.getMinutes();

		if (minutes >= bootSummary->dayStartMinutes && minutes < bootSummary->dayEndMinutes) {
			return kPlayerDay;
		} else {
			return kPlayerNight;
		}
	}
}

void Scene::addItemToInventory(int16 id) {
	if (id == -1) {
		return;
	}

	if (_flags.items[id] == g_nancy->_false) {
		_flags.items[id] = g_nancy->_true;
		if (_flags.heldItem == id) {
			setHeldItem(-1);
		}

		g_nancy->_sound->playSound("BUOK");

		_inventoryBox.addItem(id);
	}
}

void Scene::removeItemFromInventory(int16 id, bool pickUp) {
	if (id == -1) {
		return;
	}

	if (_flags.items[id] == g_nancy->_true || getHeldItem() == id) {
		_flags.items[id] = g_nancy->_false;

		if (pickUp) {
			setHeldItem(id);
		} else if (getHeldItem() == id) {
			setHeldItem(-1);
		}

		g_nancy->_sound->playSound("BUOK");

		_inventoryBox.removeItem(id);
	}
}

void Scene::setHeldItem(int16 id) {
	_flags.heldItem = id; g_nancy->_cursor->setCursorItemID(id);
}

void Scene::setNoHeldItem() {
	if (getHeldItem() != -1) {
		addItemToInventory(getHeldItem());
	}
}

byte Scene::hasItem(int16 id) const {
	if (getHeldItem() == id) {
		return g_nancy->_true;
	} else {
		return _flags.items[id];
	}
}

void Scene::installInventorySoundOverride(byte command, const SoundDescription &sound, const Common::String &caption, uint16 itemID) {
	InventorySoundOverride newOverride;

	switch (command) {
	case kInvSoundOverrideCommandNoSound :
		// Make the sound silent
		newOverride.sound = sound;
		newOverride.sound.name = "NO SOUND";
		newOverride.caption = caption; // Assumes the caption will be empty
		_inventorySoundOverrides.setVal(itemID, newOverride);
		break;
	case kInvSoundOverrideCommandNewSound :
		newOverride.sound = sound;
		newOverride.caption = caption;
		_inventorySoundOverrides.setVal(itemID, newOverride);
		break;
	case kInvSoundOverrideCommandICant :
		// Make the sound the default "I can't use that here"
		newOverride.isDefault = true;
		_inventorySoundOverrides.setVal(itemID, newOverride);
		break;
	case kInvSoundOverrideCommandTurnOff :
		// Remove any previous override
		_inventorySoundOverrides.erase(itemID);
		break;
	default :
		return;
	}
}

void Scene::playItemCantSound(int16 itemID, bool notHoldingSound) {
	if (ConfMan.getBool("subtitles") && g_nancy->getGameType() >= kGameTypeNancy2) {
		_textbox.clear();
	}

	// Improvement: nancy2 never shows the caption text, even though it exists in the data; we show it
	auto *inventoryData = GetEngineData(INV);
	assert(inventoryData);

	if (itemID < 0) {
		if (inventoryData->cantSound.name.size()) {
			// Play default "can't" inside inventory data (if present)
			g_nancy->_sound->loadSound(inventoryData->cantSound);
			g_nancy->_sound->playSound(inventoryData->cantSound);

			if (ConfMan.getBool("subtitles")) {
				_textbox.addTextLine(inventoryData->cantText, inventoryData->captionAutoClearTime);
			}
		} else {
			// TVD and nancy1 contain no sound data in INV, and have no captions
			g_nancy->_sound->playSound("CANT");
		}
	} else if ((uint)itemID < _flags.items.size()) {
		if (_inventorySoundOverrides.contains(itemID)) {
			// We have an override installed
			InventorySoundOverride &override = _inventorySoundOverrides[itemID];
			if (!override.isDefault) {
				// Not set to the default sound, play the override
				g_nancy->_sound->loadSound(override.sound);
				g_nancy->_sound->playSound(override.sound);

				if (ConfMan.getBool("subtitles")) {
					_textbox.addTextLine(override.caption, inventoryData->captionAutoClearTime);
				}
				return;
			} else {
				// Play the default "I can't" sound
				const INV::ItemDescription item = inventoryData->itemDescriptions[itemID];

				if (notHoldingSound && item.cantSoundNotHolding.name.size()) {
					// This field only exists in nancy2
					g_nancy->_sound->loadSound(item.cantSoundNotHolding);
					g_nancy->_sound->playSound(item.cantSoundNotHolding);

					if (ConfMan.getBool("subtitles")) {
						_textbox.addTextLine(item.cantTextNotHolding, inventoryData->captionAutoClearTime);
					}
				} else if (inventoryData->cantSound.name.size()) {
					g_nancy->_sound->loadSound(inventoryData->cantSound);
					g_nancy->_sound->playSound(inventoryData->cantSound);

					if (ConfMan.getBool("subtitles")) {
						_textbox.addTextLine(inventoryData->cantText, inventoryData->captionAutoClearTime);
					}
				} else {
					// Should be unreachable
					g_nancy->_sound->playSound("CANT");
				}
			}
		}

		// No override installed
		const INV::ItemDescription item = inventoryData->itemDescriptions[itemID];

		if (item.cantSound.name.size()) {
			// The inventory data contains a custom "can't" sound for this item
			g_nancy->_sound->loadSound(item.cantSound);
			g_nancy->_sound->playSound(item.cantSound);

			if (ConfMan.getBool("subtitles")) {
				_textbox.addTextLine(item.cantText, inventoryData->captionAutoClearTime);
			}
		} else if (inventoryData->cantSound.name.size()) {
			// No custom sound, play default "can't" inside inventory data. Should (?) be unreachable
			g_nancy->_sound->loadSound(inventoryData->cantSound);
			g_nancy->_sound->playSound(inventoryData->cantSound);

			if (ConfMan.getBool("subtitles")) {
				_textbox.addTextLine(inventoryData->cantText, inventoryData->captionAutoClearTime);
			}
		} else {
			// TVD and nancy1 contain no sound data in INV, and have no captions
			g_nancy->_sound->playSound("CANT");
		}
	}
}

void Scene::setEventFlag(int16 label, byte flag) {
	if (label >= 1000) {
		// In nancy3 and onwards flags begin from 1000
		label -= 1000;
	}

	if (label > kEvNoEvent && (uint)label < g_nancy->getStaticData().numEventFlags) {
		_flags.eventFlags[label] = flag;
	}
}

void Scene::setEventFlag(FlagDescription eventFlag) {
	setEventFlag(eventFlag.label, eventFlag.flag);
}

bool Scene::getEventFlag(int16 label, byte flag) const {
	if (label >= 1000) {
		// In nancy3 and onwards flags begin from 1000
		label -= 1000;
	}

	if (label > kEvNoEvent && (uint)label < g_nancy->getStaticData().numEventFlags) {
		return _flags.eventFlags[label] == flag;
	} else {
		return false;
	}
}

bool Scene::getEventFlag(FlagDescription eventFlag) const {
	return getEventFlag(eventFlag.label, eventFlag.flag);
}

void Scene::setLogicCondition(int16 label, byte flag) {
	if (label > kEvNoEvent) {
		if (label >= 2000) {
			// In nancy3 and onwards logic conditions begin from 2000
			label -= 2000;
		}

		if (label > kEvNoEvent && (uint)label < 30) {
			_flags.logicConditions[label].flag = flag;
			_flags.logicConditions[label].timestamp = g_nancy->getTotalPlayTime();
		}
	}
}

bool Scene::getLogicCondition(int16 label, byte flag) const {
	if (label > kEvNoEvent) {
		return _flags.logicConditions[label].flag == flag;
	} else {
		return false;
	}
}

void Scene::clearLogicConditions() {
	for (auto &cond : _flags.logicConditions) {
		cond.flag = g_nancy->_false;
		cond.timestamp = 0;
	}
}

void Scene::useHint(uint16 characterID, uint16 hintID) {
	if (_lastHintID != hintID || _lastHintCharacter != characterID) {
		_hintsRemaining[_difficulty] += g_nancy->getStaticData().hints[characterID][hintID].hintWeight;
		_lastHintCharacter = characterID;
		_lastHintID = hintID;
	}
}

void Scene::registerGraphics() {
	_frame.registerGraphics();
	_viewport.registerGraphics();
	_textbox.registerGraphics();
	_inventoryBox.registerGraphics();
	_hotspotDebug.registerGraphics();

	if (_menuButton) {
		_menuButton->registerGraphics();
		_menuButton->setVisible(false);
	}

	if (_helpButton) {
		_helpButton->registerGraphics();
		_helpButton->setVisible(false);
	}

	if (_viewportOrnaments) {
		_viewportOrnaments->registerGraphics();
		_viewportOrnaments->setVisible(true);
	}

	if (_textboxOrnaments) {
		_textboxOrnaments->registerGraphics();
		_textboxOrnaments->setVisible(true);
	}

	if (_inventoryBoxOrnaments) {
		_inventoryBoxOrnaments->registerGraphics();
		_inventoryBoxOrnaments->setVisible(true);
	}

	if (_clock) {
		_clock->registerGraphics();
	}
}

void Scene::synchronize(Common::Serializer &ser) {
	ser.syncAsUint16LE(_sceneState.currentScene.sceneID);
	ser.syncAsUint16LE(_sceneState.currentScene.frameID);
	ser.syncAsUint16LE(_sceneState.currentScene.verticalOffset);

	if (g_nancy->getGameType() >= kGameTypeNancy3) {
		ser.syncAsUint16LE(_sceneState.currentScene.frontVectorFrameID);

		for (uint i = 0; i < 3; ++i) {
			ser.syncAsFloatLE(_sceneState.currentScene.listenerFrontVector.getData()[i]);
		}
	}

	if (ser.isLoading()) {
		_sceneState.currentScene.continueSceneSound = kLoadSceneSound;
		_sceneState.nextScene = _sceneState.currentScene;

		g_nancy->_sound->stopAllSounds();

		load(true);
	}

	ser.syncAsUint16LE(_sceneState.pushedScene.sceneID);
	ser.syncAsUint16LE(_sceneState.pushedScene.frameID);
	ser.syncAsUint16LE(_sceneState.pushedScene.verticalOffset);
	ser.syncAsByte(_sceneState.isScenePushed);

	// Inventory scene "stack" was introduced in nancy7
	if (g_nancy->getGameType() >= kGameTypeNancy7) {
		ser.syncAsUint16LE(_sceneState.pushedInvScene.sceneID);
		ser.syncAsUint16LE(_sceneState.pushedInvScene.frameID);
		ser.syncAsUint16LE(_sceneState.pushedInvScene.verticalOffset);
		ser.syncAsByte(_sceneState.isInvScenePushed);
		ser.syncAsUint16LE(_sceneState.pushedInvItemID);
	}

	// hardcoded number of logic conditions, check if there can ever be more/less
	for (uint i = 0; i < 30; ++i) {
		ser.syncAsUint32LE(_flags.logicConditions[i].flag);
	}

	for (uint i = 0; i < 30; ++i) {
		ser.syncAsUint32LE((uint32 &)_flags.logicConditions[i].timestamp);
	}

	auto &order = getInventoryBox()._order;
	uint prevSize = getInventoryBox()._order.size();
	getInventoryBox()._order.resize(g_nancy->getStaticData().numItems);

	if (ser.isSaving()) {
		for (uint i = prevSize; i < order.size(); ++i) {
			order[i] = -1;
		}
	}

	ser.syncArray(order.data(), g_nancy->getStaticData().numItems, Common::Serializer::Sint16LE);

	while (order.size() && order.back() == -1) {
		order.pop_back();
	}

	if (ser.isLoading()) {
		// Make sure the shades are open if we have items
		getInventoryBox().onReorder();
	}

	ser.syncArray(_flags.items.data(), g_nancy->getStaticData().numItems, Common::Serializer::Byte);
	ser.syncAsSint16LE(_flags.heldItem);
	g_nancy->_cursor->setCursorItemID(_flags.heldItem);

	if (g_nancy->getGameType() >= kGameTypeNancy7) {
		ser.syncArray(_flags.disabledItems.data(), g_nancy->getStaticData().numItems, Common::Serializer::Byte);
	}

	ser.syncAsUint32LE((uint32 &)_timers.lastTotalTime);
	ser.syncAsUint32LE((uint32 &)_timers.sceneTime);
	ser.syncAsUint32LE((uint32 &)_timers.playerTime);
	ser.syncAsUint32LE((uint32 &)_timers.pushedPlayTime);
	ser.syncAsUint32LE((uint32 &)_timers.timerTime);
	ser.syncAsByte(_timers.timerIsActive);
	ser.skip(1, 0, 2);

	g_nancy->setTotalPlayTime((uint32)_timers.lastTotalTime);

	ser.syncArray(_flags.eventFlags.data(), g_nancy->getStaticData().numEventFlags, Common::Serializer::Byte);

	// Clear generic flags
	for (uint16 id : g_nancy->getStaticData().genericEventFlags) {
		_flags.eventFlags[id] = g_nancy->_false;
	}

	// Skip empty sceneCount array
	ser.skip(2001 * 2, 0, 2);

	uint numSceneCounts = _flags.sceneCounts.size();
	ser.syncAsUint16LE(numSceneCounts);

	if (ser.isSaving()) {
		uint16 key;
		for (auto &entry : _flags.sceneCounts) {
			key = entry._key;
			ser.syncAsUint16LE(key);
			ser.syncAsUint16LE(entry._value);
		}
	} else {
		uint16 key = 0;
		uint16 val = 0;
		for (uint i = 0; i < numSceneCounts; ++i) {
			ser.syncAsUint16LE(key);
			ser.syncAsUint16LE(val);
			_flags.sceneCounts.setVal(key, val);
		}
	}

	ser.syncAsUint16LE(_difficulty);
	ser.syncArray<uint16>(_hintsRemaining.data(), _hintsRemaining.size(), Common::Serializer::Uint16LE);

	ser.syncAsSint16LE(_lastHintCharacter);
	ser.syncAsSint16LE(_lastHintID);

	// Sync game-specific puzzle data

	// Support for older savefiles
	if (ser.getVersion() < 3 && g_nancy->getGameType() <= kGameTypeNancy1) {
		PuzzleData *pd = getPuzzleData(SliderPuzzleData::getTag());
		if (pd) {
			pd->synchronize(ser);
		}

		return;
	}

	byte numPuzzleData = _puzzleData.size();
	ser.syncAsByte(numPuzzleData);

	if (ser.isSaving()) {
		for (auto &pd : _puzzleData) {
			uint32 tag = pd._key;
			ser.syncAsUint32LE(tag);
			pd._value->synchronize(ser);
		}
	} else {
		clearPuzzleData();

		uint32 tag = 0;
		for (uint i = 0; i < numPuzzleData; ++i) {
			ser.syncAsUint32LE(tag);
			PuzzleData *pd = getPuzzleData(tag);
			if (pd) {
				pd->synchronize(ser);
			}
		}
	}

	_isRunningAd = false;
	ConfMan.removeKey("restore_after_ad", Common::ConfigManager::kTransientDomain);

	g_nancy->_graphics->suppressNextDraw();
}

UI::Clock *Scene::getClock() {
	auto *clok = GetEngineData(CLOK);
	if (!clok || clok->clockIsDisabled || clok->clockIsDay) {
		return nullptr;
	} else {
		return (UI::Clock *)_clock;
	}
}

void Scene::init() {
	auto *bootSummary = GetEngineData(BSUM);
	auto *hintData = GetEngineData(HINT);
	assert(bootSummary);

	_flags.eventFlags.resize(g_nancy->getStaticData().numEventFlags, g_nancy->_false);

	_flags.sceneCounts.clear();

	_flags.items.resize(g_nancy->getStaticData().numItems, g_nancy->_false);
	_flags.disabledItems.resize(_flags.items.size(), 0);

	_timers.lastTotalTime = 0;
	_timers.playerTime = bootSummary->startTimeHours * 3600000;
	_timers.sceneTime = 0;
	_timers.timerTime = 0;
	_timers.timerIsActive = false;
	_timers.playerTimeNextMinute = 0;
	_timers.pushedPlayTime = 0;

	if (ConfMan.hasKey("load_ad", Common::ConfigManager::kTransientDomain)) {
		changeScene(bootSummary->adScene);
		ConfMan.removeKey("load_ad", Common::ConfigManager::kTransientDomain);
		_isRunningAd = true;
	} else {
		changeScene(bootSummary->firstScene);
	}

	if (hintData) {
		_hintsRemaining.clear();
		_hintsRemaining = hintData->numHints;
		_lastHintCharacter = _lastHintID = -1;
	}

	initStaticData();

	if (!_isRunningAd && ConfMan.hasKey("save_slot", Common::ConfigManager::kTransientDomain)) {
		// Load savefile directly from the launcher
		int saveSlot = ConfMan.getInt("save_slot", Common::ConfigManager::kTransientDomain);
		if (saveSlot >= 0 && saveSlot <= g_nancy->getMetaEngine()->getMaximumSaveSlot()) {
			g_nancy->loadGameState(saveSlot);
		}

		// Remove key so clicking on "New Game" in start menu doesn't just reload the save
		ConfMan.removeKey("save_slot", Common::ConfigManager::kTransientDomain);
	} else {
		// Normal boot, load default first scene
		_state = kLoad;
	}

	// Set relevant event flag when player has won the game at least once
	if (ConfMan.get("PlayerWonTheGame", ConfMan.getActiveDomainName()) == "AcedTheGame") {
		setEventFlag(g_nancy->getStaticData().wonGameFlagID, g_nancy->_true);
	}

	if (g_nancy->getGameType() == kGameTypeVampire) {
		_lightning = new Misc::Lightning();
	}

	Common::Rect vpPos = _viewport.getScreenPosition();
	_hotspotDebug._drawSurface.create(vpPos.width(), vpPos.height(), g_nancy->_graphics->getScreenPixelFormat());
	_hotspotDebug.moveTo(vpPos);
	_hotspotDebug.setTransparent(true);

	registerGraphics();
	g_nancy->_graphics->redrawAll();
}

void Scene::setActiveMovie(Action::PlaySecondaryMovie *activeMovie) {
	_activeMovie = activeMovie;
}

Action::PlaySecondaryMovie *Scene::getActiveMovie() {
	return _activeMovie;
}

void Scene::setActiveConversation(Action::ConversationSound *activeConversation) {
	_activeConversation = activeConversation;
}

Action::ConversationSound *Scene::getActiveConversation() {
	return _activeConversation;
}

void Scene::beginLightning(int16 distance, uint16 pulseTime, int16 rgbPercent) {
	if (_lightning) {
		_lightning->beginLightning(distance, pulseTime, rgbPercent);
	}
}

void Scene::specialEffect(byte type, uint16 fadeToBlackTime, uint16 frameTime) {
	_specialEffects.push(Misc::SpecialEffect(type, fadeToBlackTime, frameTime));
	_specialEffects.back().init();
}

void Scene::specialEffect(byte type, uint16 totalTime, uint16 fadeToBlackTime, Common::Rect rect) {
	_specialEffects.push(Misc::SpecialEffect(type, totalTime, fadeToBlackTime, rect));
	_specialEffects.back().init();
}

PuzzleData *Scene::getPuzzleData(const uint32 tag) {
	// Lazy initialization ensures both init() and synchronize() will not need
	// to care about which puzzles a specific game has

	if (_puzzleData.contains(tag)) {
		return _puzzleData[tag];
	} else {
		PuzzleData *newData = makePuzzleData(tag);
		if (newData) {
			_puzzleData.setVal(tag, newData);
		}

		return newData;
	}
}

void Scene::load(bool fromSaveFile) {
	if (_specialEffects.size()) {
		_specialEffects.front().onSceneChange();
	}

	clearSceneData();
	g_nancy->_graphics->suppressNextDraw();

	// Scene IDs are prefixed with S inside the cif tree; e.g 100 -> S100
	Common::Path sceneName(Common::String::format("S%u", _sceneState.nextScene.sceneID));
	IFF *sceneIFF = g_nancy->_resource->loadIFF(sceneName);

	if (!sceneIFF) {
		error("Faled to load IFF %s", sceneName.toString().c_str());
	}

	Common::SeekableReadStream *sceneSummaryChunk = sceneIFF->getChunkStream("SSUM");
	if (sceneSummaryChunk) {
		_sceneState.summary.read(*sceneSummaryChunk);
	} else {
		sceneSummaryChunk = sceneIFF->getChunkStream("TSUM");
		if (sceneSummaryChunk) {
			_sceneState.summary.readTerse(*sceneSummaryChunk);
		}
	}

	if (!sceneSummaryChunk) {
		error("Invalid IFF Chunk SSUM");
	}

	delete sceneSummaryChunk;

	debugC(0, kDebugScene, "Loading new scene %i: description \"%s\", frame %i, vertical scroll %i, %s",
				_sceneState.nextScene.sceneID,
				_sceneState.summary.description.c_str(),
				_sceneState.nextScene.frameID,
				_sceneState.nextScene.verticalOffset,
				_sceneState.currentScene.continueSceneSound == kContinueSceneSound ? "kContinueSceneSound" : "kLoadSceneSound");

	SceneChangeDescription lastScene = _sceneState.currentScene;
	_sceneState.currentScene = _sceneState.nextScene;

	// Make sure to discard invalid front vectors and reuse the last one
	if (_sceneState.currentScene.listenerFrontVector.isZero()) {
		_sceneState.currentScene.listenerFrontVector = lastScene.listenerFrontVector;
	}

	// Search for Action Records, maximum for a scene is 30
	Common::SeekableReadStream *actionRecordChunk = nullptr;

	uint numRecords = 0;
	while (actionRecordChunk = sceneIFF->getChunkStream("ACT", numRecords), actionRecordChunk != nullptr) {
		_actionManager.addNewActionRecord(*actionRecordChunk);
		delete actionRecordChunk;
		++numRecords;
	}

	if (_sceneState.currentScene.paletteID == -1) {
		_sceneState.currentScene.paletteID = 0;
	}

	_viewport.loadVideo(_sceneState.summary.videoFile,
						_sceneState.currentScene.frameID,
						_sceneState.currentScene.verticalOffset,
						_sceneState.summary.panningType,
						_sceneState.summary.videoFormat,
						_sceneState.summary.palettes.size() ? _sceneState.summary.palettes[(byte)_sceneState.currentScene.paletteID] : Common::Path());

	if (_viewport.getFrameCount() <= 1) {
		_viewport.disableEdges(kLeft | kRight);
	}

	if (_sceneState.summary.videoFormat == kSmallVideoFormat) {
		// TODO
	} else if (_sceneState.summary.videoFormat == kLargeVideoFormat) {
		// always start from the bottom
		_sceneState.currentScene.verticalOffset = _viewport.getMaxScroll();
	} else {
		error("Unrecognized Scene summary chunk video file format");
	}

	if (_sceneState.summary.videoFormat == kSmallVideoFormat) {
		// TODO
	} else if (_sceneState.summary.videoFormat == kLargeVideoFormat) {
		if (_viewport.getMaxScroll() == 0) {
			_viewport.disableEdges(kUp | kDown);
		}
	}

	for (auto &override : _inventorySoundOverrides) {
		g_nancy->_sound->stopSound(override._value.sound);
	}
	_inventorySoundOverrides.clear();

	_timers.sceneTime = 0;
	g_nancy->_sound->recalculateSoundEffects();

	// Increment the number of times we've visited this scene, unless we're
	// loading from a save
	if (!fromSaveFile) {
		_flags.sceneCounts.getOrCreateVal(_sceneState.currentScene.sceneID)++;
	}

	delete sceneIFF;
	_state = kStartSound;
}

void Scene::run() {
	if (_gameStateRequested != NancyState::kNone) {
		g_nancy->setState(_gameStateRequested);

		return;
	}

	Time currentPlayTime = g_nancy->getTotalPlayTime();
	Time deltaTime = currentPlayTime - _timers.lastTotalTime;
	_timers.lastTotalTime = currentPlayTime;

	if (_timers.timerIsActive) {
		_timers.timerTime += deltaTime;
	}

	_timers.sceneTime += deltaTime;

	// Calculate the in-game time (playerTime)
	if (currentPlayTime > _timers.playerTimeNextMinute) {
		auto *bootSummary = GetEngineData(BSUM);
		assert(bootSummary);

		_timers.playerTime += 60000; // Add a minute
		_timers.playerTimeNextMinute = currentPlayTime + bootSummary->playerTimeMinuteLength;
	}

	handleInput();

	if (g_nancy->getState() == NancyState::kMainMenu) {
		// Player pressed esc, do not process further
		return;
	}

	_actionManager.processActionRecords();

	if (_lightning) {
		_lightning->run();
	}

	// Do this after the first records are processed to fix the text in nancy3 intro
	if (_specialEffects.size()) {
		if (_specialEffects.front().isInitialized()) {
			if (_specialEffects.front().isDone()) {
				_specialEffects.pop();
				g_nancy->_graphics->redrawAll();
			}
		} else {
			_specialEffects.front().afterSceneChange();
		}
	}

	g_nancy->_sound->soundEffectMaintenance();

	if (_state == kLoad) {
		g_nancy->_graphics->suppressNextDraw();
	}
}

void Scene::handleInput() {
	NancyInput input = g_nancy->_input->getInput();

	// Warp the mouse below the inactive zone during dialogue scenes
	if (_activeConversation != nullptr) {
		const Common::Rect &inactiveZone = g_nancy->_cursor->getPrimaryVideoInactiveZone();

		if (g_nancy->getGameType() == kGameTypeVampire) {
			const Common::Point cursorHotspot = g_nancy->_cursor->getCurrentCursorHotspot();
			Common::Point adjustedMousePos = input.mousePos;
			adjustedMousePos.y -= cursorHotspot.y;

			if (inactiveZone.bottom > adjustedMousePos.y) {
				input.mousePos.y = inactiveZone.bottom + cursorHotspot.y;
				g_nancy->_cursor->warpCursor(input.mousePos);
			}
		} else {
			if (inactiveZone.bottom > input.mousePos.y) {
				input.mousePos.y = inactiveZone.bottom;
				g_nancy->_cursor->warpCursor(input.mousePos);
			}
		}
	} else if (!_activeMovie) {
		// Check if player has pressed esc
		if (input.input & NancyInput::kOpenMainMenu) {
			g_nancy->setState(NancyState::kMainMenu);
			return;
		}
	}

	// We handle the textbox and inventory box first because of their scrollbars, which
	// need to take highest priority
	_textbox.handleInput(input);
	_inventoryBox.handleInput(input);

	// Handle invisible map button
	// We do this before the viewport since TVD's map button overlaps the viewport's right hotspot
	for (uint16 id : g_nancy->getStaticData().mapAccessSceneIDs) {
		if ((int)_sceneState.currentScene.sceneID == id) {
			if (_mapHotspot.contains(input.mousePos)) {
				g_nancy->_cursor->setCursorType(g_nancy->getGameType() == kGameTypeVampire ? CursorManager::kHotspot : CursorManager::kHotspotArrow);

				if (input.input & NancyInput::kLeftMouseButtonUp) {
					requestStateChange(NancyState::kMap);

					if (g_nancy->getGameType() == kGameTypeVampire) {
						g_nancy->setMouseEnabled(false);
					}
				}

				input.eatMouseInput();
			}

			break;
		}
	}

	// Handle clock before viewport since it overlaps the left hotspot in TVD
	if (getClock()) {
		getClock()->handleInput(input);
	}

	_viewport.handleInput(input);

	_sceneState.currentScene.verticalOffset = _viewport.getCurVerticalScroll();

	if (_sceneState.currentScene.frameID != _viewport.getCurFrame()) {
		_sceneState.currentScene.frameID = _viewport.getCurFrame();
		g_nancy->_sound->recalculateSoundEffects();
	}

	_actionManager.handleInput(input);

	// Menu/help are disabled when a movie is active
	if (!_activeMovie) {
		if (_menuButton) {
			_menuButton->handleInput(input);

			if (_menuButton->_isClicked) {
				if (_buttonPressActivationTime == 0) {
					auto *bootSummary = GetEngineData(BSUM);
					assert(bootSummary);

					g_nancy->_sound->playSound("BUOK");
					_buttonPressActivationTime = g_system->getMillis() + bootSummary->buttonPressTimeDelay;
				} else if (g_system->getMillis() > _buttonPressActivationTime) {
					_menuButton->_isClicked = false;
					requestStateChange(NancyState::kMainMenu);
					_buttonPressActivationTime = 0;
				}
			}
		}

		if (_helpButton) {
			_helpButton->handleInput(input);

			if (_helpButton->_isClicked) {
				if (_buttonPressActivationTime == 0) {
					auto *bootSummary = GetEngineData(BSUM);
					assert(bootSummary);

					g_nancy->_sound->playSound("BUOK");
					_buttonPressActivationTime = g_system->getMillis() + bootSummary->buttonPressTimeDelay;
				} else if (g_system->getMillis() > _buttonPressActivationTime) {
					_helpButton->_isClicked = false;
					requestStateChange(NancyState::kHelp);
					_buttonPressActivationTime = 0;
				}
			}
		}
	}
}

void Scene::initStaticData() {
	auto *bootSummary = GetEngineData(BSUM);
	assert(bootSummary);

	const ImageChunk *fr0 = (const ImageChunk *)g_nancy->getEngineData("FR0");
	assert(fr0);

	auto *mapData = GetEngineData(MAP);

	_frame.init(fr0->imageName);
	_viewport.init();
	_textbox.init();
	_inventoryBox.init();

	// Init buttons
	if (g_nancy->getGameType() == kGameTypeVampire) {
		_mapHotspot = bootSummary->extraButtonHotspot;
	} else if (mapData) {
		_mapHotspot = mapData->buttonDest;
	}

	_menuButton = new UI::Button(5, g_nancy->_graphics->_object0, bootSummary->menuButtonSrc, bootSummary->menuButtonDest, bootSummary->menuButtonHighlightSrc);
	_helpButton = new UI::Button(5, g_nancy->_graphics->_object0, bootSummary->helpButtonSrc, bootSummary->helpButtonDest, bootSummary->helpButtonHighlightSrc);
	g_nancy->setMouseEnabled(true);

	// Init ornaments and clock (TVD only)
	if (g_nancy->getGameType() == kGameTypeVampire) {
		_viewportOrnaments = new UI::ViewportOrnaments(9);
		_viewportOrnaments->init();

		_textboxOrnaments = new UI::TextboxOrnaments(9);
		_textboxOrnaments->init();

		_inventoryBoxOrnaments = new UI::InventoryBoxOrnaments(9);
		_inventoryBoxOrnaments->init();

		_clock = new UI::Clock();
		_clock->init();
	}

	// Init just the clock (nancy2 and up; nancy1 has no clock, only a map button)
	if (g_nancy->getGameType() >= kGameTypeNancy2) {
		auto *clok = GetEngineData(CLOK);
		if (clok->clockIsDay) {
			// nancy5 uses a different "clock" that mostly just indicates the in-game day
			_clock = new UI::Nancy5Clock();
			_clock->init();
		} else if (!clok->clockIsDisabled) {
			_clock = new UI::Clock();
			_clock->init();
		} else {
			// In nancy7 the clock is entirely disabled
			_clock = nullptr;
		}
	}

	_state = kLoad;
}

void Scene::clearSceneData() {
	// Clear generic flags only
	for (uint16 id : g_nancy->getStaticData().genericEventFlags) {
		_flags.eventFlags[id] = g_nancy->_false;
	}

	clearLogicConditions();
	_actionManager.clearActionRecords();

	if (_lightning) {
		_lightning->endLightning();
	}

	if (_textbox.hasBeenDrawn()) {
		// Improvement: the dog portrait scenes in nancy7 queue a piece of text,
		// then immediately change the scene. This makes the text disappear instantly;
		// instead, we check if the textbox has been drawn, and don't clear it if it hasn't.
		// Hopefully this doesn't cause issues with earlier games.
		_textbox.clear();
	}

	_activeConversation = nullptr;
	_activeMovie = nullptr;
}

void Scene::clearPuzzleData() {
	for (auto &pd : _puzzleData) {
		delete pd._value;
	}

	_puzzleData.clear();
}

Scene::PlayFlags::LogicCondition::LogicCondition() : flag(g_nancy->_false) {}

} // End of namespace State
} // End of namespace Nancy
