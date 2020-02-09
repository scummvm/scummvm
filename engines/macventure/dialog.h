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

/*
 * Based on
 * WebVenture (c) 2010, Sean Kasun
 * https://github.com/mrkite/webventure, http://seancode.com/webventure/
 *
 * Used with explicit permission from the author
 */

#ifndef MACVENTURE_DIALOG_H
#define MACVENTURE_DIALOG_H

#include "graphics/macgui/macwindowmanager.h"

#include "macventure/macventure.h"
#include "macventure/prebuilt_dialogs.h"

namespace MacVenture {

using namespace Graphics::MacGUIConstants;
class Gui;
class DialogElement;

class Dialog {
public:
	Dialog(Gui *gui, Common::Point pos, uint width, uint height);
	Dialog(Gui *gui, PrebuiltDialogs prebuilt);

	~Dialog();

	bool processEvent(Common::Event event);
	void draw();
	void localize(Common::Point &point);
	void handleDialogAction(DialogElement *trigger, DialogAction action);

	const Graphics::Font &getFont();

	void addButton(Common::String title, DialogAction action, Common::Point position, uint width = 0, uint height = 0);
	void addText(Common::String content, Common::Point position);
	void addTextInput(Common::Point position, int width, int height);

	void setUserInput(Common::String content);

private:
	void addPrebuiltElement(const PrebuiltDialogElement &element);

	void calculateBoundsFromPrebuilt(const PrebuiltDialogBounds &bounds);

private:
	Gui *_gui;

	Common::String _userInput;
	Common::Array<DialogElement*> _elements;
	Common::Rect _bounds;
};

class DialogElement {
public:
	DialogElement(Dialog *dialog, Common::String title, DialogAction action, Common::Point position, uint width = 0, uint height = 0);
	virtual ~DialogElement() {}

	bool processEvent(Dialog *dialog, Common::Event event);
	void draw(MacVenture::Dialog *dialog, Graphics::ManagedSurface &target);
	const Common::String &getText();

private:
	virtual bool doProcessEvent(Dialog *dialog, Common::Event event) = 0;
	virtual void doDraw(MacVenture::Dialog *dialog, Graphics::ManagedSurface &target) = 0;
	virtual const Common::String &doGetText();

protected:
	Common::String _text;
	Common::Rect _bounds;
	DialogAction _action;
};

// Dialog elements
class DialogButton : public DialogElement {
public:
	DialogButton(Dialog *dialog, Common::String title, DialogAction action, Common::Point position, uint width = 0, uint height = 0);
	~DialogButton() override {}

private:
	bool doProcessEvent(Dialog *dialog, Common::Event event) override;
	void doDraw(MacVenture::Dialog *dialog, Graphics::ManagedSurface &target) override;
};

class DialogPlainText : public DialogElement {
public:
	DialogPlainText(Dialog *dialog, Common::String content, Common::Point position);
	~DialogPlainText() override;

private:
	bool doProcessEvent(Dialog *dialog, Common::Event event) override;
	void doDraw(MacVenture::Dialog *dialog, Graphics::ManagedSurface &target) override;
};

class DialogTextInput : public DialogElement {
public:
	DialogTextInput(Dialog *dialog, Common::Point position, uint width, uint height);
	~DialogTextInput() override;

private:
	bool doProcessEvent(Dialog *dialog, Common::Event event) override;
	void doDraw(MacVenture::Dialog *dialog, Graphics::ManagedSurface &target) override;
};

} // End of namespace MacVenture

#endif
