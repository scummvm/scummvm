/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef TSAGE_USER_INTERFACE_H
#define TSAGE_USER_INTERFACE_H

#include "common/scummsys.h"
#include "tsage/core.h"
#include "tsage/graphics.h"
#include "tsage/sound.h"

namespace TsAGE {

class StripProxy: public EventHandler {
public:
	void process(Event &event) override;
};

class UIElement: public BackgroundSceneObject {
public:
	bool _enabled;
	int _frameNum;

	Common::String getClassName() override { return "UIElement"; }
	void synchronize(Serializer &s) override;

	void setup(int visage, int stripNum, int frameNum, int posX, int posY, int priority);
	void setEnabled(bool flag);
};

// This class implements the Question mark button
class UIQuestion: public UIElement {
private:
	void showDescription(CursorType item);
	void showItem(int resNum, int rlbNum, int frameNum);
public:
	void process(Event &event) override;
	void setEnabled(bool flag);
};

// This class implements the score counter
class UIScore: public UIElement {
private:
	void showDescription(int lineNum);
public:
	UIElement _digit3, _digit2, _digit1, _digit0;

	void postInit(SceneObjectList *OwnerList = NULL) override;
	void draw() override;

	void updateScore();
};

class UIInventorySlot: public UIElement {
public:
	int _objIndex;
	InvObject *_object;

	UIInventorySlot();
	Common::String getClassName() override { return "UIInventorySlot"; }
	void synchronize(Serializer &s) override;
	void process(Event &event) override;
};

class UIInventoryScroll: public UIElement {
private:
	void toggle(bool pressed);
public:
	bool _isLeft;

	UIInventoryScroll();
	Common::String getClassName() override { return "UIInventoryScroll"; }
	void synchronize(Serializer &s) override;
	void process(Event &event) override;
};

class UICollection: public EventHandler {
private:
	void r2rDrawFrame();
protected:
	void erase();
public:
	Common::Point _position;
	Rect _bounds;
	bool _visible;
	bool _clearScreen;
	bool _cursorChanged;
	Common::Array<UIElement *> _objList;

	UICollection();
	void setup(const Common::Point &pt);
	void hide();
	void show();
	void resetClear();
	void draw();
};

class UIElements: public UICollection {
private:
	void add(UIElement *obj);
	void updateInvList();
public:
	UIElement _background;
	UIQuestion _question;
	UIScore _score;
	UIInventorySlot _slot1, _slot2, _slot3, _slot4;
	UIInventoryScroll _scrollLeft, _scrollRight;
	ASound _sound;
	int _slotStart, _scoreValue;
	bool _active;
	Common::Array<int> _itemList;
	Visage _cursorVisage;
	UIElement _character;

	UIElements();
	Common::String getClassName() override { return "UIElements"; }
	void synchronize(Serializer &s) override;
	void postInit(SceneObjectList *OwnerList = NULL) override { error("Wrong init() called"); }
	void process(Event &event) override;

	void setup(const Common::Point &pt);
	void updateInventory(int objectNumber = 0);
	void addScore(int amount);
	void scrollInventory(bool isLeft);

	static void loadNotifierProc(bool postFlag);
};

} // End of namespace TsAGE

#endif
