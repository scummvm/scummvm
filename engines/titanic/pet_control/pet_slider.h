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

#ifndef TITANIC_PET_SLIDER_H
#define TITANIC_PET_SLIDER_H

#include "titanic/support/rect.h"
#include "titanic/support/string.h"
#include "titanic/core/game_object.h"

namespace Titanic {

class CPetControl;

class CPetSlider {
private:
	int _field4;
	int _field8;
	Rect _bounds;
	int _field1C;
	int _field20;
	int _field24;
	int _field28;
	int _field2C;
	int _field30;
	int _field34;
	Rect _bounds2;
public:
	CPetSlider();

	/**
	 * Setup the background
	 */
	virtual void setupBackground(const CString &name, CPetControl *petControl) {}

	/**
	 * Setup the thumb
	 */
	virtual void setupThumb(const CString &name, CPetControl *petControl) {}

	/**
	 * Setup the background
	 */
	virtual void setupBackground(const CString &name, CTreeItem *treeItem) {}

	/**
	 * Setup the thumb
	 */
	virtual void setupThumb(const CString &name, CTreeItem *treeItem) {}

	/**
	 * Reset the slider
	 */
	virtual void reset(const CString &name) {}
	
	virtual void proc5() {}
	virtual void proc6() {}
	
	/**
	 * Reset the bounds of the slider
	 */
	virtual void initBounds(Rect *rect);
	
	virtual void proc8();
	virtual void proc9();
	virtual void proc10();
	virtual void proc11();
	virtual void proc12();
	virtual void proc13();
	virtual void proc14();
	virtual void proc15();
	virtual void proc16();
	virtual void proc17();
	virtual void proc18();

	/**
	 * Returns true if the passed point falls within the slider's bounds
	 */
	bool contains(const Point &pt) const { return _bounds.contains(pt); }
};

class CPetSoundSlider : public CPetSlider {
public:
	CGameObject *_background;
	CGameObject *_thumb;
public:
	CPetSoundSlider() : CPetSlider(), _background(nullptr),
		_thumb(0) {}

	/**
	 * Setup the background
	 */
	virtual void setupBackground(const CString &name, CPetControl *petControl);

	/**
	 * Setup the thumb
	 */
	virtual void setupThumb(const CString &name, CPetControl *petControl);
};

} // End of namespace Titanic

#endif /* TITANIC_PET_SLIDER_H */
