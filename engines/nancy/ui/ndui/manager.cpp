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

#include "common/debug.h"
#include "common/events.h"
#include "common/memstream.h"

#include "engines/nancy/nancy.h"
#include "engines/nancy/resource.h"
#include "engines/nancy/iff.h"
#include "engines/nancy/input.h"
#include "engines/nancy/cursor.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/util.h"

#include "engines/nancy/ui/ndui/manager.h"
#include "engines/nancy/ui/ndui/panel.h"
#include "engines/nancy/ui/ndui/control.h"
#include "engines/nancy/ui/ndui/button.h"
#include "engines/nancy/ui/ndui/editbox.h"

namespace Nancy {
namespace UI {
namespace NDUI {

// Panels are drawn over everything else, in the order they were created
static const uint16 kFirstPanelZ = 60000;

// The cursor shown while the mouse is over the UI; Nancy19 has it by name
static const uint16 kUICursor = 14;
static const char *const kUICursorName = "Menu";

Manager::~Manager() {
	for (Panel *panel : _panels) {
		delete panel;
	}

	for (auto &texture : _textures) {
		delete texture._value;
	}
}

void Manager::load(const Common::String &name) {
	IFF *iff = g_nancy->_resource->loadIFF(Common::Path(name));
	if (!iff) {
		error("Failed to load UI file %s", name.c_str());
	}

	for (const Common::String &include : iff->getIncludes()) {
		load(include);
	}

	Common::SeekableReadStream *chunk = nullptr;
	for (uint i = 0; chunk = iff->getChunkStream("NDUI", i), chunk != nullptr; ++i) {
		byte kind = chunk->readByte();
		if (kind == kObjectTemplate) {
			// Kept as data, and read again for every copy. The name comes after
			// the version, the control type and the control's own version and state.
			Common::Array<byte> data(chunk->size() - chunk->pos());
			chunk->read(data.data(), data.size());

			Common::String templateName;
			Common::MemoryReadStream nameStream(data.data(), data.size());
			nameStream.skip(33 + 4 + 33 + 4);
			readFilename(nameStream, templateName);
			_templates.setVal(templateName, data);
		} else {
			Panel *panel = new Panel(this, (uint16)(kFirstPanelZ + _panels.size()), kind);
			panel->read(*chunk);
			_panels.push_back(panel);
		}

		delete chunk;
	}

	delete iff;
}

Panel *Manager::addPanel(byte kind) {
	Panel *panel = new Panel(this, (uint16)(kFirstPanelZ + _panels.size()), kind);
	_panels.push_back(panel);
	return panel;
}

void Manager::clearPanels() {
	for (Panel *panel : _panels) {
		delete panel;
	}

	_panels.clear();
	_hoveredControl = nullptr;
	_hoveredTooltip = nullptr;
	_tooltipControl = nullptr;
	_tooltipWindow.setVisible(false);
}

void Manager::registerGraphics() {
	for (Panel *panel : _panels) {
		panel->registerGraphics();
	}

	_tooltipWindow.setNativeResolution(true);
	_tooltipWindow.setVisible(false);
	_tooltipWindow.registerGraphics();
}

void Manager::setHoveredTooltip(const Control *control, Tooltip *tooltip) {
	_hoveredControl = control;
	_hoveredTooltip = tooltip;
}

void Manager::updateTooltip(const Common::Point &mousePos) {
	const uint32 now = g_nancy->getTotalPlayTime();

	if (_hoveredControl != _tooltipControl) {
		// Another control starts over
		_tooltipControl = _hoveredControl;
		_tooltipHoverStart = now;
		_isTooltipDismissed = false;
	}

	bool show = _tooltipControl && !_isTooltipDismissed && !_hoveredTooltip->isPrevented() &&
		_hoveredTooltip->isTimeToShow((now - _tooltipHoverStart) / 1000.0f);

	if (show && _tooltipWindow.isVisible()) {
		// The mouse getting to the tooltip takes it down for good
		if (_tooltipWindow.getScreenPositionRaw().contains(mousePos)) {
			_isTooltipDismissed = true;
			show = false;
		}
	} else if (show) {
		_hoveredTooltip->render(_tooltipWindow._drawSurface, _tooltipControl->getTooltip());

		// Above the mouse, and inside of the screen
		const int width = _tooltipWindow._drawSurface.w;
		const int height = _tooltipWindow._drawSurface.h;
		const Common::Rect screen = g_nancy->_graphics->getScreen()->getBounds();
		int x = MAX<int>(MIN<int>(mousePos.x + 1, screen.right - width - 7), 5);
		int y = MAX<int>(MIN<int>(mousePos.y - height - 6, screen.bottom - height - 7), 5);

		_tooltipWindow.moveTo(Common::Rect(x, y, x + width, y + height));
		_tooltipWindow.setNeedsRedraw(true);
	}

	_tooltipWindow.setVisible(show);
}

void Manager::takeAction(const Action &action) {
	if (action.target.hasPrefixIgnoreCase("Engine_")) {
		takeEngineAction(action);
		return;
	}

	for (Panel *panel : _panels) {
		if (panel->takeAction(action)) {
			return;
		}
	}

	debugC(1, kDebugEngine, "Unhandled NDUI action %d on \"%s\" from \"%s\"", action.type, action.target.c_str(), action.sender.c_str());
}

void Manager::takeAction(int32 type, const Common::String &target, double value) {
	Action action;
	action.type = type;
	action.target = target;
	action.hasValue = true;
	action.value = value;
	takeAction(action);
}

void Manager::takeAction(int32 type, const Common::String &target, const Common::String &text) {
	Action action;
	action.type = type;
	action.target = target;
	action.hasValue = true;
	action.text = text;
	takeAction(action);
}

void Manager::saveState(Common::Array<UIStateData::Change> &changes) const {
	changes.clear();
	for (const Panel *panel : _panels) {
		panel->saveState(changes);
	}
}

void Manager::restoreState(const Common::Array<UIStateData::Change> &changes) {
	for (const UIStateData::Change &change : changes) {
		// Older saved games have the inventory buttons too, which may not match
		// the items: those are shown from the inventory itself
		if (isItemButton(change.name)) {
			continue;
		}

		takeAction(change.action, change.name, 0.0);
	}

	for (Panel *panel : _panels) {
		panel->finishBlending();
	}
}

Manager::Request Manager::takeRequest() {
	Request request = _request;
	_request = Request();
	return request;
}

void Manager::createObject(const Common::String &name, const Common::String &templateName, const Common::String &parentName) {
	Panel *parent = findPanel(parentName);

	// Nancy19 has the script of the panel make the control
	if (parent && g_nancy->getGameType() >= kGameTypeNancy19) {
		if (!parent->createObject(name, templateName)) {
			warning("Couldn't create UI object %s from %s in %s", name.c_str(), templateName.c_str(), parentName.c_str());
		}

		return;
	}

	if (!parent || !_templates.contains(templateName)) {
		warning("Couldn't create UI object %s from %s in %s", name.c_str(), templateName.c_str(), parentName.c_str());
		return;
	}

	const Common::Array<byte> &data = _templates[templateName];
	Common::MemoryReadStream stream(data.data(), data.size());
	stream.skip(33); // version
	int32 type = stream.readSint32LE();

	Control *control = parent->addNewControl(type, stream);
	control->setName(name);
	parent->arrangeControls();
}

void Manager::destroyObject(const Common::String &name) {
	for (Panel *panel : _panels) {
		if (panel->removeControl(name)) {
			return;
		}
	}
}

bool Manager::isItemButton(const Common::String &name) const {
	for (const Panel *panel : _panels) {
		if (panel->isItemButton(name)) {
			return true;
		}
	}

	return false;
}

Panel *Manager::findPanel(const Common::String &name) const {
	for (Panel *panel : _panels) {
		if (panel->getName().equalsIgnoreCase(name)) {
			return panel;
		}
	}

	return nullptr;
}

InvButton *Manager::findItemButton(int16 itemID) const {
	for (Panel *panel : _panels) {
		InvButton *button = panel->findItemButton(itemID);
		if (button) {
			return button;
		}
	}

	return nullptr;
}

// The keymapper turns the arrow keys into movement, so they never arrive as keys
struct ArrowKey {
	uint16 input;
	Common::KeyCode key;
};

static const ArrowKey kArrowKeys[] = {
	{ NancyInput::kMoveLeft, Common::KEYCODE_LEFT },
	{ NancyInput::kMoveRight, Common::KEYCODE_RIGHT },
	{ NancyInput::kMoveUp, Common::KEYCODE_UP },
	{ NancyInput::kMoveDown, Common::KEYCODE_DOWN }
};

void Manager::handleInput(NancyInput &input) {
	const uint16 heldArrowKeys = _heldArrowKeys;
	_heldArrowKeys = input.input;

	// The dialog on top gets what gets typed
	for (int i = _panels.size() - 1; i >= 0; --i) {
		EditBox *editBox = _panels[i]->getKeyboardFocus();
		if (editBox) {
			for (const Common::KeyState &key : input.otherKbdInput) {
				editBox->handleKey(key);
			}

			// The arrow keys move the caret, not the player
			for (const ArrowKey &arrowKey : kArrowKeys) {
				if ((input.input & arrowKey.input) && !(heldArrowKeys & arrowKey.input)) {
					editBox->handleKey(Common::KeyState(arrowKey.key, 0, g_nancy->getEventManager()->getModifierState()));
				}
			}

			input.otherKbdInput.clear();
			input.input &= ~(NancyInput::kMoveUp | NancyInput::kMoveDown | NancyInput::kMoveLeft |
				NancyInput::kMoveRight | NancyInput::kMoveFastModifier);
			break;
		}
	}

	_hoveredControl = nullptr;

	// The panels drawn on top get the mouse first
	bool isOverUI = false;
	bool isOverControl = false;
	for (int i = _panels.size() - 1; i >= 0; --i) {
		bool isOverPanelControl = false;
		if (_panels[i]->handleInput(input, !isOverUI, isOverPanelControl)) {
			isOverUI = true;
			isOverControl |= isOverPanelControl;
		}
	}

	updateTooltip(input.screenMousePos);

	if (isOverUI) {
		int cursor = kUICursor;
		if (g_nancy->getGameType() >= kGameTypeNancy19) {
			cursor = MAX(g_nancy->_cursor->getCursorTypeByName(kUICursorName), 0);
		}

		g_nancy->_cursor->setCursorType((CursorManager::CursorType)cursor, true, isOverControl);
		input.eatMouseInput();
	}
}

const Graphics::ManagedSurface *Manager::getTexture(const Common::String &name) {
	Graphics::ManagedSurface *texture = _textures.getValOrDefault(name, nullptr);
	if (!texture) {
		texture = new Graphics::ManagedSurface();

		// The textures the engine makes are empty until it does
		if (name.hasPrefixIgnoreCase("Engine_")) {
			_textures.setVal(name, texture);
			return nullptr;
		}

		if (!g_nancy->_resource->loadImage(Common::Path(name), *texture)) {
			warning("Couldn't load UI texture %s", name.c_str());
		}

		_textures.setVal(name, texture);
	}

	return texture->empty() ? nullptr : texture;
}

} // End of namespace NDUI
} // End of namespace UI
} // End of namespace Nancy
