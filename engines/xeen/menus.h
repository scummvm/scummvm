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

#ifndef XEEN_MENUS_H
#define XEEN_MENUS_H

#include "common/array.h"
#include "common/stack.h"
#include "common/rect.h"
#include "xeen/xeen.h"

namespace Xeen {

class DialogButton {
public:
	Common::Rect _bounds;
	int _d;
	int _i;
};

class Dialog {
private:
	Common::Array<DialogButton> _buttons;
	Common::Stack< Common::Array<DialogButton> > _savedButtons;
protected:
	XeenEngine *_vm;

	virtual void doScroll(bool drawFlag, bool doFade) = 0;
public:
	Dialog(XeenEngine *vm): _vm(vm) {}

	void saveButtons();

	void clearButtons();

	void restoreButtons();
};

class OptionsMenu: public Dialog {
private:
	void execute();
protected:
	OptionsMenu(XeenEngine *vm) : Dialog(vm) {}
protected:
	virtual void startup(Common::String &title1, Common::String &title2);

	virtual void setBackground() {}

	virtual void showTitles1(const Common::String &title);

	virtual void showTitles2();
public:
	static void show(XeenEngine *vm);
};

class CloudsOptionsMenu : public OptionsMenu {
protected:
	virtual void startup(Common::String &title1, Common::String &title2);

	virtual void doScroll(bool drawFlag, bool doFade);
public:
	CloudsOptionsMenu(XeenEngine *vm) : OptionsMenu(vm) {}
};

class DarkSideOptionsMenu : public OptionsMenu {
protected:
	virtual void startup(Common::String &title1, Common::String &title2);

	virtual void doScroll(bool drawFlag, bool doFade);
public:
	DarkSideOptionsMenu(XeenEngine *vm) : OptionsMenu(vm) {}
};

class WorldOptionsMenu : public DarkSideOptionsMenu {
protected:
	virtual void startup(Common::String &title1, Common::String &title2);

	virtual void setBackground();

	virtual void showTitles2() {}
public:
	WorldOptionsMenu(XeenEngine *vm) : DarkSideOptionsMenu(vm) {}
};

} // End of namespace Xeen

#endif /* XEEN_MENUS_H */
