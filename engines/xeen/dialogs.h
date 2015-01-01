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

#ifndef XEEN_DIALOGS_H
#define XEEN_DIALOGS_H

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

	void doScroll(bool drawFlag, bool doFade);

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

class CreditsScreen: public Dialog {
private:
	CreditsScreen(XeenEngine *vm) : Dialog(vm) {}

	void execute();
public:
	static void show(XeenEngine *vm);
};

} // End of namespace Xeen

#endif /* XEEN_DIALOGS_H */
