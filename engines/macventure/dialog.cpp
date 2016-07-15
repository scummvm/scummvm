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
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.

* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*
*/

#include "common/system.h"

#include "macventure/dialog.h"

namespace MacVenture {

// Prebuilt dialogs

enum {
  // HACK
  kMaxPrebuiltDialogElements = 10
};

struct PrebuiltDialog {
  Common::Rect bounds;
  PrebuiltDialogElement elements[kMaxPrebuiltDialogElements];
};

PrebuiltDialog prebuiltDialogs[kPrebuiltDialogCount] = {

  {/* kSaveAsDialog */
    Common::Rect(0, 146, 456, 254),
    {
      {kDEButton, "YES", kDANone, Common::Point(24, 68), 120, 22},
      {kDEButton, "NO", kDACloseDialog, Common::Point(168, 68), 120, 22},
      {kDEButton, "CANCEL", kDACloseDialog, Common::Point(312, 68), 120, 22},
      {kDEPlainText, "Save As...", kDANone, Common::Point(100, 10), 340, 38},
      {kDETextInput, "", kDANone, Common::Point(100, 30), 340, 20},
      {kDEEnd, "", kDANone, Common::Point(0, 0), 0, 0}
    }
  },

  { /* kSpeakDialog */
    Common::Rect(20, 92, 400, 200),
    {
      {kDEButton, "OK", kDASubmit, Common::Point(10, 70), 50, 20},
      {kDEButton, "CANCEL", kDACloseDialog, Common::Point(96, 70), 50, 20},
      {kDEPlainText, "What would you like to say?", kDANone, Common::Point(10, 10), 400, 20},
      {kDETextInput, "", kDANone, Common::Point(10, 25), 350, 40},
      {kDEEnd, "", kDANone, Common::Point(0, 0), 0, 0}
    }
  }

};

Dialog::Dialog(Gui *gui, Common::Point pos, uint width, uint height) :
  _gui(gui), _bounds(Common::Rect(pos.x, pos.y, pos.x + width, pos.y + height)) {}

Dialog::Dialog(Gui *gui, PrebuiltDialogs prebuilt) {
  _gui = gui;
  const PrebuiltDialog &dialog = prebuiltDialogs[prebuilt];
  _bounds = dialog.bounds;
  for (int i = 0; dialog.elements[i].type != kDEEnd; i++) {
    addPrebuiltElement(dialog.elements[i]);
  }
}

Dialog::~Dialog() {
  for (Common::Array<DialogElement*>::iterator it = _elements.begin(); it != _elements.end(); it++) {
    delete *it;
  }
}

void Dialog::handleDialogAction(DialogElement *trigger, DialogAction action) {
  switch(action) {
  case kDACloseDialog:
    _gui->closeDialog();
    break;
  case kDASubmit:
    _gui->setTextInput(_userInput);
    _gui->closeDialog();
    break;
  }
}

const Graphics::Font& Dialog::getFont() {
  return _gui->getCurrentFont();
}

bool Dialog::processEvent(Common::Event event) {
  for (Common::Array<DialogElement*>::iterator it = _elements.begin(); it != _elements.end(); it++) {
    if ((*it)->processEvent(this, event)) return true;
  }
}

void Dialog::addButton(Common::String title, MacVenture::DialogAction action, Common::Point position, uint width, uint height)  {
  _elements.push_back(new DialogButton(this, title, action, position, width, height));
}

void Dialog::addText(Common::String content, Common::Point position)  {
  _elements.push_back(new DialogPlainText(this, content, position));
}

void Dialog::addTextInput(Common::Point position, int width, int height) {
  _elements.push_back(new DialogTextInput(this, position, width, height));
}

void Dialog::draw()  {
  Graphics::ManagedSurface compose;
  // Compose the surface
  compose.create(_bounds.width(), _bounds.height());
  Common::Rect base(0, 0, _bounds.width(), _bounds.height());
  compose.fillRect(base, kColorWhite);
  compose.frameRect(base, kColorBlack);
  for (Common::Array<DialogElement*>::iterator it = _elements.begin(); it != _elements.end(); it++) {
    (*it)->draw(this, compose);
  }

  g_system->copyRectToScreen(compose.getPixels(), compose.pitch,
    _bounds.left, _bounds.top, _bounds.width(), _bounds.height());
}

void Dialog::localize(Common::Point &point) {
  point.x -= _bounds.left;
  point.y -= _bounds.top;
}

void Dialog::setUserInput(Common::String content) {
  _userInput = content;
}

void Dialog::addPrebuiltElement(const MacVenture::PrebuiltDialogElement &element) {
  switch(element.type) {
    case kDEButton:
      addButton(element.title, element.action, element.position, element.width, element.height);
      break;
    case kDEPlainText:
      addText(element.title, element.position);
      break;
    case kDETextInput:
      addTextInput(element.position, element.width, element.height);
      break;
  }
}

// Dialog Element

DialogElement::DialogElement(Dialog *dialog, Common::String title, DialogAction action, Common::Point position, uint width, uint height) :
  _text(title), _action(action) {
    if (width == 0) {
      width = dialog->getFont().getStringWidth(title);
    }
    if (height == 0) {
      height = dialog->getFont().getFontHeight();
    }
    _bounds = Common::Rect(position.x, position.y, position.x + width, position.y + height);
  }

bool DialogElement::processEvent(MacVenture::Dialog *dialog, Common::Event event)  {
  // HACK for test, please delete me
  Common::Point mouse = event.mouse;
  dialog->localize(mouse);
  if (_bounds.contains(mouse)) debug(1, "Hovering over: %s", _text.c_str());

  return doProcessEvent(dialog, event);
}

void DialogElement::draw(MacVenture::Dialog *dialog, Graphics::ManagedSurface &target) {
  doDraw(dialog, target);
}

const Common::String& DialogElement::getText() {
  return doGetText();
}

const Common::String& DialogElement::doGetText() {
  return _text;
}

// CONCRETE DIALOG ELEMENTS

DialogButton::DialogButton(Dialog *dialog, Common::String title, DialogAction action, Common::Point position, uint width, uint height):
  DialogElement(dialog, title, action, position, width, height) {}

bool DialogButton::doProcessEvent(MacVenture::Dialog *dialog, Common::Event event) {
  Common::Point mouse = event.mouse;
  if (event.type == Common::EVENT_LBUTTONDOWN) {
    dialog->localize(mouse);
    if (_bounds.contains(mouse)) {
      debug(1, "Click! Button: %s", _text.c_str());
      dialog->handleDialogAction(this, _action);
      return true;
    }
  }
  return false;
}

void DialogButton::doDraw(MacVenture::Dialog *dialog, Graphics::ManagedSurface &target) {
  target.fillRect(_bounds, kColorWhite);
  target.frameRect(_bounds, kColorBlack);
  // Draw title
  dialog->getFont().drawString(
    &target, _text, _bounds.left, _bounds.top, _bounds.width(), kColorBlack, Graphics::kTextAlignCenter);
}

DialogPlainText::DialogPlainText(Dialog *dialog, Common::String content, Common::Point position) :
  DialogElement(dialog, content, kDANone, position, 0, 0) { }

DialogPlainText::~DialogPlainText() {}

bool DialogPlainText::doProcessEvent(MacVenture::Dialog *dialog, Common::Event event) {
  return false;
}

void DialogPlainText::doDraw(MacVenture::Dialog *dialog, Graphics::ManagedSurface &target) {
  // Draw contents
  dialog->getFont().drawString(
    &target, _text, _bounds.left, _bounds.top, _bounds.width(), kColorBlack, Graphics::kTextAlignCenter);

}

DialogTextInput::DialogTextInput(Dialog *dialog, Common::Point position, uint width, uint height) :
  DialogElement(dialog, "", kDANone, position, width, height) {}
DialogTextInput::~DialogTextInput() {}

bool DialogTextInput::doProcessEvent(Dialog *dialog, Common::Event event) {
  if (event.type == Common::EVENT_KEYDOWN) {
    switch (event.kbd.keycode) {
			case Common::KEYCODE_BACKSPACE:
				if (!_text.empty()) {
					_text.deleteLastChar();
          dialog->setUserInput(_text);
          return true;
				}
				break;

			default:
				if (event.kbd.ascii >= 0x20 && event.kbd.ascii <= 0x7f) {
					_text += (char)event.kbd.ascii;
          dialog->setUserInput(_text);
          return true;
				}

				break;
    }
  }
  return false;
}
void DialogTextInput::doDraw(MacVenture::Dialog *dialog, Graphics::ManagedSurface &target) {
  target.fillRect(_bounds, kColorWhite);
  target.frameRect(_bounds, kColorBlack);
  dialog->getFont().drawString(
    &target, _text, _bounds.left, _bounds.top, _bounds.width(), kColorBlack);
}



} // End of namespace MacVenture
