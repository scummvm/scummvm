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
#include "common/memstream.h"

#include "engines/nancy/nancy.h"
#include "engines/nancy/resource.h"
#include "engines/nancy/iff.h"
#include "engines/nancy/input.h"
#include "engines/nancy/cursor.h"
#include "engines/nancy/util.h"

#include "engines/nancy/ui/ndui/manager.h"
#include "engines/nancy/ui/ndui/panel.h"
#include "engines/nancy/ui/ndui/control.h"

namespace Nancy {
namespace UI {
namespace NDUI {

// Panels are drawn over everything else, in the order they were created
static const uint16 kFirstPanelZ = 1000;

// The cursor shown while the mouse is over the UI
static const uint16 kUICursor = 14;

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

void Manager::registerGraphics() {
	for (Panel *panel : _panels) {
		panel->registerGraphics();
	}
}

void Manager::takeAction(const Action &action) {
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

Manager::Request Manager::takeRequest() {
	Request request = _request;
	_request = Request();
	return request;
}

void Manager::createObject(const Common::String &name, const Common::String &templateName, const Common::String &parentName) {
	Panel *parent = findPanel(parentName);
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

Panel *Manager::findPanel(const Common::String &name) const {
	for (Panel *panel : _panels) {
		if (panel->getName().equalsIgnoreCase(name)) {
			return panel;
		}
	}

	return nullptr;
}

void Manager::handleInput(NancyInput &input) {
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

	if (isOverUI) {
		g_nancy->_cursor->setCursorType((CursorManager::CursorType)kUICursor, true, isOverControl);
		input.eatMouseInput();
	}
}

const Graphics::ManagedSurface *Manager::getTexture(const Common::String &name) {
	Graphics::ManagedSurface *texture = _textures.getValOrDefault(name, nullptr);
	if (!texture) {
		texture = new Graphics::ManagedSurface();
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
