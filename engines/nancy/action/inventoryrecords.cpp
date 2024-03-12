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

#include "engines/nancy/nancy.h"
#include "engines/nancy/util.h"
#include "engines/nancy/resource.h"
#include "engines/nancy/sound.h"

#include "engines/nancy/action/inventoryrecords.h"

#include "engines/nancy/state/scene.h"

namespace Nancy {
namespace Action {

void AddInventoryNoHS::readData(Common::SeekableReadStream &stream) {
	_itemID = stream.readUint16LE();

	if (g_nancy->getGameType() >= kGameTypeNancy6) {
		_setCursor = stream.readUint16LE();
		_forceCursor = stream.readUint16LE();
	}
}

void AddInventoryNoHS::execute() {
	if (_setCursor) {
		if (NancySceneState.getHeldItem() != -1) {
			// Currently holding another item
			if (_forceCursor) {
				NancySceneState.addItemToInventory(NancySceneState.getHeldItem());
				if (NancySceneState.hasItem(_itemID) == g_nancy->_true) {
					NancySceneState.removeItemFromInventory(_itemID, true);
				} else {
					NancySceneState.setHeldItem(_itemID);
				}
			} else {
				NancySceneState.addItemToInventory(_itemID);
			}
		} else {
			if (NancySceneState.hasItem(_itemID) == g_nancy->_true) {
				NancySceneState.removeItemFromInventory(_itemID, true);
			} else {
				NancySceneState.setHeldItem(_itemID);
			}
		}
	} else {
		if (NancySceneState.hasItem(_itemID) == g_nancy->_false) {
			NancySceneState.addItemToInventory(_itemID);
		}
	}

	_isDone = true;
}

void RemoveInventoryNoHS::readData(Common::SeekableReadStream &stream) {
	_itemID = stream.readUint16LE();
}

void RemoveInventoryNoHS::execute() {
	if (NancySceneState.hasItem(_itemID) == g_nancy->_true) {
		NancySceneState.removeItemFromInventory(_itemID, false);
	}

	_isDone = true;
}

void ShowInventoryItem::init() {
	g_nancy->_resource->loadImage(_imageName, _fullSurface);

	_drawSurface.create(_fullSurface, _blitDescriptions[0].src);

	RenderObject::init();
}

void ShowInventoryItem::readData(Common::SeekableReadStream &stream) {
	GameType gameType = g_nancy->getGameType();
	_objectID = stream.readUint16LE();
	readFilename(stream, _imageName);

	uint16 numFrames = stream.readUint16LE();
	if (gameType >= kGameTypeNancy3) {
		stream.skip(2);
	}

	_blitDescriptions.resize(numFrames);
	for (uint i = 0; i < numFrames; ++i) {
		if (gameType <= kGameTypeNancy2) {
			_blitDescriptions[i].readData(stream);
		} else {
			_blitDescriptions[i].frameID = i;
			readRect(stream, _blitDescriptions[i].src);
			readRect(stream, _blitDescriptions[i].dest);
		}
	}
}

void ShowInventoryItem::execute() {
	switch (_state) {
	case kBegin:
		init();
		registerGraphics();
		_state = kRun;
		// fall through
	case kRun: {
		int newFrame = -1;

		for (uint i = 0; i < _blitDescriptions.size(); ++i) {
			if (_blitDescriptions[i].frameID == NancySceneState.getSceneInfo().frameID) {
				newFrame = i;
				break;
			}
		}

		if (newFrame != _drawnFrameID) {
			_drawnFrameID = newFrame;

			if (newFrame != -1) {
				_hasHotspot = true;
				_hotspot = _blitDescriptions[newFrame].dest;
				_drawSurface.create(_fullSurface, _blitDescriptions[newFrame].src);
				_screenPosition = _blitDescriptions[newFrame].dest;
				setVisible(true);
			} else {
				_hasHotspot = false;
				setVisible(false);
			}
		}

		break;
	}
	case kActionTrigger:
		g_nancy->_sound->playSound("BUOK");
		NancySceneState.addItemToInventory(_objectID);
		setVisible(false);
		_hasHotspot = false;
		finishExecution();
		break;
	}
}

void InventorySoundOverride::readData(Common::SeekableReadStream &stream) {
	_command = stream.readByte();
	_itemID = stream.readUint16LE();
	stream.skip(2);
	char buf[61];
	stream.read(buf, 60);
	buf[60] = '\0';
	_caption = buf;
	_sound.readNormal(stream);
}

void InventorySoundOverride::execute() {
	NancySceneState.installInventorySoundOverride(_command, _sound, _caption, _itemID);
	_isDone = true;
}

void EnableDisableInventory::readData(Common::SeekableReadStream &stream) {
	_itemID = stream.readUint16LE();
	bool disabled = stream.readUint16LE();
	bool playSound = stream.readUint16LE();

	if (disabled) {
		++_disabledState;
		if (playSound) {
			++_disabledState;
		}
	}
}

void EnableDisableInventory::execute() {
	NancySceneState.setItemDisabledState(_itemID, _disabledState);
	_isDone = true;
}

void PopInvViewPriorScene::readData(Common::SeekableReadStream &stream) {
	stream.skip(1);
}

void PopInvViewPriorScene::execute() {
	NancySceneState.popScene(true);
	_isDone = true;
}

void GoInvViewScene::readData(Common::SeekableReadStream &stream) {
	_itemID = stream.readUint16LE();
	_addToInventory = stream.readUint16LE();
}

void GoInvViewScene::execute() {
	auto *inv = GetEngineData(INV);
	assert(inv);

	const INV::ItemDescription &item = inv->itemDescriptions[_itemID];
	byte disabled = NancySceneState.getItemDisabledState(_itemID);

	if (!disabled && item.keepItem == kInvItemNewSceneView) {
		if (_addToInventory || NancySceneState.hasItem(_itemID)) {
			NancySceneState.pushScene(_itemID);
		} else {
			// Do not add the item to the inventory, only go to its scene
			NancySceneState.pushScene();
		}

		SceneChangeDescription sceneChange;
		sceneChange.sceneID = item.sceneID;
		sceneChange.continueSceneSound = item.sceneSoundFlag;
		NancySceneState.changeScene(sceneChange);
	}

	_isDone = true;
}

} // End of namespace Action
} // End of namespace Nancy
