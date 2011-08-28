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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef TSAGE_BLUEFORCE_UI_H
#define TSAGE_BLUEFORCE_UI_H

#include "common/scummsys.h"
#include "tsage/core.h"
#include "tsage/sound.h"

namespace TsAGE {

namespace BlueForce {

using namespace TsAGE;

class UIElement: public AltSceneObject {
public:
	int _field88;
	bool _enabled;
	int _frameNum;

	virtual Common::String getClassName() { return "UIElement"; }
	virtual void synchronize(Serializer &s);

	void setup(int visage, int stripNum, int frameNum, int posX, int posY, int priority);
	void setEnabled(bool flag);
};

// This class implements the Question mark button
class UIQuestion: public UIElement {
private:
	void showDescription(int lineNum);
public:
	virtual void process(Event &event);
	void setEnabled(bool flag);
};

// This class implements the score counter
class UIScore: public UIElement {
private:
	void showDescription(int lineNum);
public:
	UIElement _digit3, _digit2, _digit1, _digit0;

	virtual void postInit(SceneObjectList *OwnerList = NULL);
	virtual void draw();
	
	void updateScore();
};

class UIInventorySlot: public UIElement {
public:
	int _objIndex;

	UIInventorySlot();
	virtual Common::String getClassName() { return "UIInventorySlot"; }
	virtual void synchronize(Serializer &s);
	virtual void process(Event &event);
};

class UIInventoryScroll: public UIElement {
public:
	bool _isLeft;

	UIInventoryScroll();
	virtual Common::String getClassName() { return "UIInventoryScroll"; }
	virtual void synchronize(Serializer &s);
	virtual void process(Event &event);
};

class UICollection: public EventHandler {
protected:
	void erase();
public:
	Common::Point _position;
	Rect _bounds;
	bool _visible;
	bool _clearScreen;
	int _field4E;
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
	void updateInventory();
	void updateInvList();
public:
	UIElement _object1;
	UIQuestion _question;
	UIScore _score;
	UIInventorySlot _slot1, _slot2, _slot3, _slot4;
	UIInventoryScroll _scrollLeft, _scrollRight;
	ASound _sound;
	int _itemCount, _slotStart, _scoreValue;
	bool _active;
	Common::Array<int> _itemList;

	virtual void postInit(SceneObjectList *OwnerList = NULL) { error("Wrong init() called"); }
	virtual void process(Event &event);

	void setup(const Common::Point &pt);
};

} // End of namespace BlueForce

} // End of namespace TsAGE

#endif
