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

#include "ngi/guimanager.h"
#include "ngi/ngi.h"
#include "ngi/scene.h"

namespace NGI {

static int resourceId(const Common::StringMap &attrs, const char *key, const ObjectIdList &ids) {
	uint32 id;
	return ids.findId(attrs.getValOrDefault(key), id) ? static_cast<int16>(id) : 0;
}

static bool guiFlag(const Common::StringMap &attrs, const char *key, bool fallback) {
	const Common::String value = attrs.getValOrDefault(key, fallback ? "1" : "0");
	return value == "1" || value == "true" || value == "yes";
}

GuiElement::GuiElement(Scene &scene, const Common::StringMap &attrs, const ObjectIdList &ids) :
		_scene(scene) {
	_id = attrs.getValOrDefault("id", "error");
	_pos = Common::Point(atoi(attrs.getValOrDefault("x").c_str()), atoi(attrs.getValOrDefault("y").c_str()));
	_hasX = attrs.contains("x");
	_hasY = attrs.contains("y");
	_cursor = atoi(attrs.getValOrDefault("cursor", "1").c_str());
	_z = atoi(attrs.getValOrDefault("z").c_str());
	_active = guiFlag(attrs, "active", true);
	_visible = guiFlag(attrs, "visible", false);
	_scrolling = guiFlag(attrs, "scrolling", false);
	_sourceScene = attrs.contains("scene") ? resourceId(attrs, "scene", ids) : scene._sceneId;
	_pictureId = resourceId(attrs, "texture", ids);
}

void GuiElement::load(GameProject &project) {
	Scene *source = project.findScene(_sourceScene);
	_picture = source && _pictureId ? source->findPictureObjectById(_pictureId, 0) : nullptr;
	if (_picture) {
		if (!_hasX)
			_pos.x = _picture->_x;
		if (!_hasY)
			_pos.y = _picture->_y;
		_size = _picture->getFullSize();
	}
}

Common::Point GuiElement::screenPos() const {
	return _scrolling ? _pos - Common::Point(_scene._x, _scene._y) : _pos;
}

class GuiElementStub : public GuiElement {
public:
	using GuiElement::GuiElement;
	void load(GameProject &project) override {}
	void render(Graphics::ManagedSurface &target) const override {}
};

class GuiPicture : public GuiElement {
public:
	using GuiElement::GuiElement;
	void render(Graphics::ManagedSurface &target) const override {
		if (_visible && _picture)
			_picture->_picture->blitTo(target, screenPos());
	}
};

class GuiButton : public GuiElement {
public:
	GuiButton(Scene &scene, const Common::StringMap &attrs, const ObjectIdList &ids);
	void load(GameProject &project) override;
	void unload() override;
	void activate() override;
	void render(Graphics::ManagedSurface &target) const override;
	bool hit(Common::Point pos) const override;
	bool onMouseMove(Common::Point pos) override;
	bool onMouseLKeyDown(Common::Point pos) override;
	bool onMouseLKeyUp(Common::Point pos) override;

private:
	enum State { kDisabled, kPressed, kSelected, kNumStates };
	int _pictureIds[kNumStates];
	PictureObject *_pictures[kNumStates] = {};
	Common::Point _offsets[kNumStates];
	bool _useOffsets;
	int _pressSound, _inactiveSound, _selectSound;
};

GuiButton::GuiButton(Scene &scene, const Common::StringMap &attrs, const ObjectIdList &ids) :
		GuiElement(scene, attrs, ids) {
	_pictureIds[kDisabled] = resourceId(attrs, "tex_disable", ids);
	_pictureIds[kPressed] = resourceId(attrs, "tex_pressed", ids);
	_pictureIds[kSelected] = resourceId(attrs, "tex_selected", ids);
	_useOffsets = attrs.getValOrDefault("offset") != "0";
	_pressSound = resourceId(attrs, "press", ids);
	_inactiveSound = resourceId(attrs, "deactive_press", ids);
	_selectSound = resourceId(attrs, "select", ids);
}

void GuiButton::load(GameProject &project) {
	GuiElement::load(project);
	Scene *source = project.findScene(_sourceScene);
	PictureObject *first = _picture;
	for (int i = 0; source && i < kNumStates; ++i) {
		if (_pictureIds[i])
			_pictures[i] = source->findPictureObjectById(_pictureIds[i], 0);
		if (!first)
			first = _pictures[i];
	}
	if (first) {
		if (!_hasX)
			_pos.x = first->_x;
		if (!_hasY)
			_pos.y = first->_y;
		_size = first->getFullSize();
	}
	for (int i = kPressed; _useOffsets && i < kNumStates; ++i) {
		if (_pictures[i])
			_offsets[i] = Common::Point(_pictures[i]->_x, _pictures[i]->_y) - _pos;
	}
}

void GuiButton::unload() {
	GuiElement::unload();
	for (PictureObject *&picture : _pictures)
		picture = nullptr;
}

void GuiButton::activate() {
	GuiElement::activate();
	onMouseMove(g_system->getEventManager()->getMousePos());
}

void GuiButton::render(Graphics::ManagedSurface &target) const {
	if (!_visible)
		return;
	int state = !_active ? kDisabled : _pressed ? kPressed : _selected ? kSelected : kNumStates;
	if (state != kNumStates && _pictures[state])
		_pictures[state]->_picture->blitTo(target, screenPos() + _offsets[state]);
	else if (_picture)
		_picture->_picture->blitTo(target, screenPos());
}

bool GuiButton::hit(Common::Point pos) const {
	pos -= screenPos();
	return _visible && pos.x > 0 && pos.y > 0 && pos.x < _size.x && pos.y < _size.y;
}

bool GuiButton::onMouseMove(Common::Point pos) {
	bool inside = hit(pos);
	if (inside != _selected) {
		if (inside && _active && _selectSound)
			g_nmi->playSound(_selectSound, 0);
		const char *event = inside ? (_active ? "select" : "select_unactive") : (_active ? "deselect" : "deselect_unactive");
		_scene.onGuiEvent(_id, event);
		_selected = inside;
	}
	return inside && _active;
}

bool GuiButton::onMouseLKeyDown(Common::Point pos) {
	if (!_visible || !_selected)
		return false;
	int sound = _active ? _pressSound : _inactiveSound;
	if (sound)
		g_nmi->playSound(sound, 0);
	const bool handled = _active;
	_scene.onGuiEvent(_id, handled ? "press" : "press_unactive");
	if (handled)
		_pressed = true;
	return handled;
}

bool GuiButton::onMouseLKeyUp(Common::Point pos) {
	bool handled = _visible && _selected && _active;
	if (_visible && _selected) {
		if (_active)
			_selected = false;
		_scene.onGuiEvent(_id, handled ? "unpress" : "unpress_unactive");
	}
	_pressed = false;
	return handled;
}

GuiManager::~GuiManager() {
	for (GuiElement *element : _renderables)
		delete element;
}

void GuiManager::createElement(const Common::String &type, const Common::StringMap &attrs, const ObjectIdList &ids) {
	if (type == "BUTTON")
		addGuiElement(new GuiButton(_scene, attrs, ids));
	else if (type == "PICTURE")
		addGuiElement(new GuiPicture(_scene, attrs, ids));
	else if (type == "TEXT" || type == "TEXTBUTTON" || type == "SLIDER" ||
			type == "CHECKBOX" || type == "PROGRESSBAR") {
		warning("STUB: %s", type.c_str());
		addGuiElement(new GuiElementStub(_scene, attrs, ids));
	} else
		debugC(2, kDebugXML, "GUI element %s (%s) not implemented", type.c_str(), attrs.getValOrDefault("id").c_str());
}

void GuiManager::addGuiElement(GuiElement *element) {
	if (_elements.find(element->_id) == _elements.end())
		_elements[element->_id] = element;
	_renderables.push_back(element);
}

GuiElement *GuiManager::getElement(const Common::String &id) const {
	auto it = _elements.find(id);
	return it == _elements.end() ? nullptr : it->second;
}

void GuiManager::removeElement(GuiElement *element) {
	_elements.erase(element->_id);
	for (uint i = 0; i < _renderables.size(); ++i) {
		if (_renderables[i] == element) {
			_renderables.remove_at(i);
			break;
		}
	}
}

void GuiManager::load(GameProject &project) {
	for (const auto &entry : _elements)
		entry.second->load(project);
	sortRenderables();
}

void GuiManager::sortRenderables() {
	// Equal z values retain their XML order.
	for (uint i = 1; i < _renderables.size(); ++i) {
		for (uint j = i; j && _renderables[j - 1]->_z < _renderables[j]->_z; --j)
			SWAP(_renderables[j - 1], _renderables[j]);
	}
}

void GuiManager::render(Graphics::ManagedSurface &target) {
	sortRenderables();
	for (GuiElement *element : _renderables)
		element->render(target);
}

bool GuiManager::onMouseMove(Common::Point pos) {
	for (uint i = _renderables.size(); i > 0; --i) {
		if (_renderables[i - 1]->onMouseMove(pos)) {
			while (--i)
				_renderables[i - 1]->_selected = false;
			return true;
		}
	}
	return false;
}

bool GuiManager::dispatchMouse(MouseHandler handler, Common::Point pos) {
	for (uint i = _renderables.size(); i > 0; --i) {
		if ((_renderables[i - 1]->*handler)(pos))
			return true;
	}
	return false;
}

bool GuiManager::onMouseLKeyDown(Common::Point pos) {
	return dispatchMouse(&GuiElement::onMouseLKeyDown, pos);
}

bool GuiManager::onMouseLKeyUp(Common::Point pos) {
	return dispatchMouse(&GuiElement::onMouseLKeyUp, pos);
}

bool GuiManager::onMouseRKeyDown(Common::Point pos) {
	return dispatchMouse(&GuiElement::onMouseRKeyDown, pos);
}

bool GuiManager::onMouseRKeyUp(Common::Point pos) {
	return dispatchMouse(&GuiElement::onMouseRKeyUp, pos);
}

void GuiManager::onKeyDown(Common::KeyCode key) {
	for (const auto &entry : _elements)
		entry.second->onKeyDown(key);
}

void GuiManager::onKeyUp(Common::KeyCode key) {
	for (const auto &entry : _elements)
		entry.second->onKeyUp(key);
}

void GuiManager::onChar(uint16 ch) {
	for (const auto &entry : _elements)
		entry.second->onChar(ch);
}

void GuiManager::unload() {
	for (const auto &entry : _elements)
		entry.second->unload();
}

bool GuiManager::update(uint32 elapsed) {
	bool changed = false;
	for (const auto &entry : _elements)
		changed |= entry.second->update(elapsed);
	return changed;
}

bool GuiManager::hit(Common::Point pos) const {
	for (const auto &entry : _elements) {
		if (entry.second->hit(pos))
			return true;
	}
	return false;
}

int GuiManager::getCursorType(Common::Point pos) const {
	int cursor = -1;
	for (const auto &entry : _elements) {
		const GuiElement &e = *entry.second;
		if (e._visible && e._selected && e.hit(pos))
			cursor = MAX(cursor, e._cursor);
	}
	return cursor;
}

bool GuiManager::isCursorObjectRender(Common::Point pos) const {
	for (const auto &entry : _elements) {
		const GuiElement &e = *entry.second;
		if (e._visible && e._selected && e.hit(pos))
			return false;
	}
	return true;
}

void GuiManager::deselectAll() {
	for (const auto &entry : _elements)
		entry.second->_selected = false;
}

void GuiManager::unpressAll() {
	for (const auto &entry : _elements)
		entry.second->_pressed = false;
}

void GuiManager::onHide() {
	deselectAll();
	unpressAll();
}

} // End of namespace NGI
