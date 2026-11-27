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

#ifndef NGI_GUIMANAGER_H
#define NGI_GUIMANAGER_H

#include "common/array.h"
#include "common/events.h"
#include "common/hash-str.h"
#include "common/rect.h"
#include "common/stablemap.h"

namespace Graphics { class ManagedSurface; }

namespace NGI {

class Scene;
class GameProject;
class ObjectIdList;
class PictureObject;

class GuiElement {
public:
	GuiElement(Scene &scene, const Common::StringMap &attrs, const ObjectIdList &ids);
	virtual ~GuiElement() {}
	virtual void load(GameProject &project);
	virtual void unload() { _picture = nullptr; }
	virtual bool update(uint32 elapsed) { return false; }
	virtual void onKeyDown(Common::KeyCode key) {}
	virtual void onKeyUp(Common::KeyCode key) {}
	virtual void onChar(uint16 ch) {}
	virtual bool onMouseRKeyDown(Common::Point pos) { return false; }
	virtual bool onMouseRKeyUp(Common::Point pos) { return false; }
	virtual void render(Graphics::ManagedSurface &target) const = 0;
	virtual bool hit(Common::Point pos) const { return false; }
	virtual bool onMouseMove(Common::Point pos) { return false; }
	virtual bool onMouseLKeyDown(Common::Point pos) { return false; }
	virtual bool onMouseLKeyUp(Common::Point pos) { return false; }

	virtual void activate() { _active = true; }
	void showAndActivate() { _visible = true; activate(); }
	void hideAndDeactivate() { _visible = _active = _selected = false; }
	void setPos(Common::Point pos) { _pos = pos; }

	Common::String _id;
	Common::Point _pos, _size;
	int _z, _cursor;
	bool _active, _visible, _selected = false, _pressed = false;

protected:
	Scene &_scene;
	bool _scrolling, _hasX, _hasY;
	int _sourceScene, _pictureId;
	PictureObject *_picture = nullptr;
	Common::Point screenPos() const;
};

class GuiManager {
public:
	explicit GuiManager(Scene &scene) : _scene(scene) {}
	~GuiManager();
	void createElement(const Common::String &type, const Common::StringMap &attrs, const ObjectIdList &ids);
	// Takes ownership until removeElement() or destruction.
	void addGuiElement(GuiElement *element);
	void removeElement(GuiElement *element);
	GuiElement *getElement(const Common::String &id) const;
	void load(GameProject &project);
	void render(Graphics::ManagedSurface &target);
	bool onMouseMove(Common::Point pos);
	bool onMouseLKeyDown(Common::Point pos);
	bool onMouseLKeyUp(Common::Point pos);
	bool onMouseRKeyDown(Common::Point pos);
	bool onMouseRKeyUp(Common::Point pos);
	void onKeyDown(Common::KeyCode key);
	void onKeyUp(Common::KeyCode key);
	void onChar(uint16 ch);
	bool update(uint32 elapsed);
	void unload();
	bool hit(Common::Point pos) const;
	int getCursorType(Common::Point pos) const;
	bool isCursorObjectRender(Common::Point pos) const;
	void deselectAll();
	void unpressAll();
	void onHide();
private:
	Scene &_scene;
	Common::Array<GuiElement *> _renderables;
	Common::StableMap<Common::String, GuiElement *> _elements;
	typedef bool (GuiElement::*MouseHandler)(Common::Point);
	bool dispatchMouse(MouseHandler handler, Common::Point pos);
	void sortRenderables();
};

} // End of namespace NGI
#endif
