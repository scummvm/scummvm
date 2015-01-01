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
	SpriteResource *_sprites;
	char _c;
	bool _draw;

	DialogButton(const Common::Rect &bounds, char c, SpriteResource *sprites, bool draw) :
		_bounds(bounds), _c(c), _sprites(sprites), _draw(draw) {}

	DialogButton() : _c('\0'), _sprites(nullptr), _draw(false) {}
};

class Dialog {
private:
	Common::Stack< Common::Array<DialogButton> > _savedButtons;
protected:
	XeenEngine *_vm;
	Common::Array<DialogButton> _buttons;
	char _key;

	virtual void doScroll(bool drawFlag, bool doFade);

	void checkEvents();
public:
	Dialog(XeenEngine *vm): _vm(vm), _key('\0') {}

	void saveButtons();

	void clearButtons();

	void restoreButtons();

	void addButton(const Common::Rect &bounds, char c, SpriteResource *sprites, bool draw);
};

class SettingsBaseDialog : public Dialog {
protected:
	virtual void showContents(SpriteResource &title1, bool mode);
public:
	SettingsBaseDialog(XeenEngine *vm) : Dialog(vm) {}
};
class OptionsMenu : public SettingsBaseDialog {
private:
	void execute();
protected:
	OptionsMenu(XeenEngine *vm) : SettingsBaseDialog(vm) {}
protected:
	virtual void startup(Common::String &title1, Common::String &title2) = 0;

	virtual void setBackground(bool doFade) {}

	virtual void showTitles1(SpriteResource &sprites);

	virtual void showTitles2();

	virtual void setupButtons(SpriteResource *buttons);

	virtual void openWindow() {}
public:
	static void show(XeenEngine *vm);
};

class CloudsOptionsMenu : public OptionsMenu {
protected:
	virtual void startup(Common::String &title1, Common::String &title2);
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
private:
	int _bgFrame;
protected:
	virtual void startup(Common::String &title1, Common::String &title2);

	virtual void setBackground(bool doFade);

	virtual void showTitles2() {}

	virtual void setupButtons(SpriteResource *buttons);

	virtual void openWindow();

	virtual void showContents(SpriteResource &title1, bool mode);
public:
	WorldOptionsMenu(XeenEngine *vm) : DarkSideOptionsMenu(vm), _bgFrame(0) {}
};

} // End of namespace Xeen

#endif /* XEEN_MENUS_H */
