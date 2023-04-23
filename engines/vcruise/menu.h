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

#ifndef VCRUISE_MENU_H
#define VCRUISE_MENU_H

#include "common/array.h"
#include "common/ptr.h"

namespace Graphics {

struct Surface;
class ManagedSurface;

} // End of namespace Graphics

namespace Common {

struct Rect;

} // End of namespace Common

namespace VCruise {

struct OSEvent;
class MenuPage;
class Runtime;

class MenuInterface {
public:
	virtual ~MenuInterface();

	virtual void commitRect(const Common::Rect &rect) const = 0;
	virtual bool popOSEvent(OSEvent &evt) const = 0;
	virtual Graphics::Surface *getUIGraphic(uint index) const = 0;
	virtual Graphics::ManagedSurface *getMenuSurface() const = 0;
	virtual bool hasDefaultSave() const = 0;
	virtual Common::Point getMouseCoordinate() const = 0;
	virtual void restartGame() const = 0;
	virtual void goToCredits() const = 0;
	virtual void changeMenu(MenuPage *newPage) const = 0;
	virtual void quitGame() const = 0;
	virtual bool canSave() const = 0;
	virtual bool reloadFromCheckpoint() const = 0;
};

class MenuPage {
public:
	MenuPage();
	virtual ~MenuPage();

	void init(const MenuInterface *menuInterface);

	virtual void start();
	virtual bool run();

protected:
	const MenuInterface *_menuInterface;
};

MenuPage *createMenuReahMain();

} // End of namespace VCruise

#endif
