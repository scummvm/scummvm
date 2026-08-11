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

/*
 * Based on
 * WebVenture (c) 2010, Sean Kasun
 * https://github.com/mrkite/webventure, http://seancode.com/webventure/
 *
 * Used with explicit permission from the author
 */

#include "common/system.h"

#include "graphics/macgui/mactext.h"

#include "macventure/dialog.h"
#include "macventure/gui.h"

namespace MacVenture {


Dialog::Dialog(Gui *gui, Common::MacResManager *resourceManager, uint16 resID) {
	_gui = gui;

	Common::SeekableReadStream *stream = resourceManager->getResource(MKTAG('D', 'L', 'O', 'G'), resID);

	_bounds.top = stream->readUint16BE();
	_bounds.left = stream->readUint16BE();
	_bounds.bottom = stream->readUint16BE();
	_bounds.right = stream->readUint16BE();

	stream->readUint16BE(); // def

	stream->readByte(); // vis?
	stream->readByte(); // padding
	stream->readByte(); // close
	stream->readByte(); // padding
	stream->readUint32BE(); // refcon

	uint16 itemList = stream->readUint16BE();
	byte titleLength = stream->readByte();
	char *str = new char[titleLength + 1];
	if (titleLength > 0) {
		stream->read(str, titleLength);
	}
	str[titleLength] = '\0';
	delete[] str;
	str = nullptr;
	delete stream;

	stream = resourceManager->getResource(MKTAG('D', 'I', 'T', 'L'), itemList);
	uint numItems = stream->readUint16BE() + 1;
	for (uint i = 0; i < numItems; i++) {
		stream->readUint32BE(); // reserved
		PrebuiltDialogElement element;
		element.action = kDANone;
		element.top = stream->readUint16BE();
		element.left = stream->readUint16BE();
		element.height = stream->readUint16BE() - element.top;
		element.width = stream->readUint16BE() - element.left;
		element.type = (PrebuiltElementType)stream->readByte();

		titleLength = stream->readByte();
		if (titleLength > 0) {
			str = new char[titleLength + 1];
			stream->read(str, titleLength);
			str[titleLength] = '\0';
			element.title = str;
		}
		if (titleLength & 1)
			stream->readByte(); // align
		switch (element.type & 0x7f) {
		case 4:
			element.type = kDEButton;
			addPrebuiltElement(element);
			break;
		case 8:
			element.type = kDEPlainText;
			addPrebuiltElement(element);
			break;
		case 0x10:
			element.type = kDETextInput;
			addPrebuiltElement(element);
			break;
		default:
			break;
		}
		if (str) {
			delete[] str;
			str = nullptr;
		}
	}
	delete stream;
}

Dialog::Dialog(Gui *gui, Common::Point pos, uint width, uint height) :
	_gui(gui), _bounds(Common::Rect(pos.x, pos.y, pos.x + width, pos.y + height)) {}

Dialog::Dialog(Gui *gui, PrebuiltDialogs prebuilt, const Common::String &title) {
	_gui = gui;
	const PrebuiltDialog &dialog = g_prebuiltDialogs[prebuilt];
	calculateBoundsFromPrebuilt(dialog.bounds);
	for (int i = 0; dialog.elements[i].type != kDEEnd; i++) {
		addPrebuiltElement(dialog.elements[i], title);
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
	case kDASaveAs:
		_gui->saveGame();
		_gui->closeDialog();
		break;
	case kDALoadGame:
		_gui->loadGame();
		_gui->closeDialog();
		break;
	case kDANewGame:
		_gui->newGame();
		_gui->closeDialog();
		break;
	case kDAQuit:
		_gui->quitGame();
		_gui->closeDialog();
		break;
	default:
		break;
	}
}

Graphics::MacWindowManager *Dialog::getMacWindowManager() const {
	return _gui->getMacWindowManager();
}

const Graphics::Font &Dialog::getFont() {
	return _gui->getCurrentFont();
}

bool Dialog::processEvent(Common::Event event) {
	for (Common::Array<DialogElement*>::iterator it = _elements.begin(); it != _elements.end(); it++) {
		if ((*it)->processEvent(this, event)) {
			return true;
		}
	}
	return false;
}

void Dialog::addButton(Common::String title, MacVenture::DialogAction action, Common::Point position, uint width, uint height) {
	_elements.push_back(new DialogButton(this, title, action, position, width, height));
}

void Dialog::addText(Common::String content, Common::Point position, int width, int height, Graphics::TextAlign alignment) {
	_elements.push_back(new DialogPlainText(this, content, position, width, height));
}

void Dialog::addTextInput(Common::Point position, int width, int height, Graphics::TextAlign alignment) {
	_elements.push_back(new DialogTextInput(this, _gui, position, width, height, alignment));
}

void Dialog::draw() {
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

DialogElement *Dialog::getElement(Common::String elementID) {
	for (auto &el : _elements) {
		if (el->getText() == elementID) {
			return el;
		}
	}
	return nullptr;
}

void Dialog::addPrebuiltElement(const MacVenture::PrebuiltDialogElement &element, const Common::String &title) {
	Common::Point position(element.left, element.top);
	switch(element.type) {
	case kDEButton:
		addButton(element.title, element.action, position, element.width, element.height);
		break;
	case kDEPlainText:
		if (title.size())
			addText(title, position, element.width, element.height);
		else
			addText(element.title, position, element.width, element.height);
		break;
	case kDETextInput:
		addTextInput(position, element.width, element.height);
		break;
	default:
		break;
	}
}

// Dialog Element

DialogElement::DialogElement(Dialog *dialog, Common::String title, DialogAction action, Common::Point position, uint width, uint height, Graphics::TextAlign alignment) :
	_text(title), _action(action) {
	if (width == 0) {
		width = dialog->getFont().getStringWidth(title);
	}
	if (height == 0) {
		height = dialog->getFont().getFontHeight();
	}
	_bounds = Common::Rect(position.x, position.y, position.x + width, position.y + height);

	Graphics::MacWindowManager *wm = dialog->getMacWindowManager();
	_macText = new Graphics::MacText(Common::U32String(_text), wm, &dialog->getFont(), kColorBlack, kColorWhite, width, alignment);
}

bool DialogElement::processEvent(MacVenture::Dialog *dialog, Common::Event event) {
	return doProcessEvent(dialog, event);
}

void DialogElement::draw(MacVenture::Dialog *dialog, Graphics::ManagedSurface &target) {
	doDraw(dialog, target);
}

const Common::String &DialogElement::getText() {
	return doGetText();
}

const Common::String &DialogElement::doGetText() {
	return _text;
}

// CONCRETE DIALOG ELEMENTS

DialogButton::DialogButton(Dialog *dialog, Common::String title, DialogAction action, Common::Point position, uint width, uint height):
	DialogElement(dialog, title, action, position, width, height, Graphics::kTextAlignCenter) {}

bool DialogButton::doProcessEvent(MacVenture::Dialog *dialog, Common::Event event) {
	Common::Point mouse = event.mouse;
	if (event.type == Common::EVENT_LBUTTONDOWN) {
		dialog->localize(mouse);
		if (_bounds.contains(mouse)) {
			debugC(2, kMVDebugGUI, "Click! Button: %s", _text.c_str());
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

DialogPlainText::DialogPlainText(Dialog *dialog, Common::String content, Common::Point position, int width, int height, Graphics::TextAlign alignment) :
	DialogElement(dialog, content, kDANone, position, width, height, alignment) { }

DialogPlainText::~DialogPlainText() {}

bool DialogPlainText::doProcessEvent(MacVenture::Dialog *dialog, Common::Event event) {
	return false;
}

void DialogPlainText::doDraw(MacVenture::Dialog *dialog, Graphics::ManagedSurface &target) {
	// Draw contents
	_macText->drawToPoint(&target, Common::Point(_bounds.left, _bounds.top));
}

static void cursorTimerHandler(void *refCon);

DialogTextInput::DialogTextInput(Dialog *dialog, Gui *gui, Common::Point position, uint width, uint height, Graphics::TextAlign alignment) :
	DialogElement(dialog, "", kDANone, position, width, height, alignment), _gui(gui) {
	_cursorPos = Common::Point(_bounds.left + 4, _bounds.top + _gui->getCurrentFont().getFontHeight() - 14);
	_cursorState = false;
	_cursorDirty = true;
	_cursorRect = Common::Rect(0, 0, 1, getCursorHeight());
	_cursorSurface = new Graphics::ManagedSurface(1, getCursorHeight());
	_cursorSurface->fillRect(_cursorRect, kColorBlack);

	g_system->getTimerManager()->installTimerProc(&cursorTimerHandler, 200000, this, "DialogTextWindowCursor");
}

DialogTextInput::~DialogTextInput() {
	delete _cursorSurface;
	g_system->getTimerManager()->removeTimerProc(&cursorTimerHandler);
}

bool DialogTextInput::doProcessEvent(Dialog *dialog, Common::Event event) {
	if (event.type == Common::EVENT_KEYDOWN) {
		switch (event.kbd.keycode) {
		case Common::KEYCODE_RETURN:
			dialog->handleDialogAction(this, kDASubmit);
			return true;
		case Common::KEYCODE_BACKSPACE:
			if (!_text.empty()) {
				_text.deleteLastChar();
				dialog->setUserInput(_text);
				updateCursorPos();
				return true;
			}
			break;
		default:
			if (event.kbd.ascii >= 0x20 && event.kbd.ascii <= 0x7f) {
				_text += (char)event.kbd.ascii;
				dialog->setUserInput(_text);
				updateCursorPos();
				return true;
			}
			break;
		}
	}
	return false;
}

void DialogTextInput::doDraw(MacVenture::Dialog *dialog, Graphics::ManagedSurface &target) {
	_cursorDirty = false;

	target.fillRect(_bounds, kColorWhite);
	target.frameRect(_bounds, kColorBlack);
	dialog->getFont().drawString(&target, _text, _bounds.left, _bounds.top, _bounds.width(), kColorBlack);

	if (_cursorState && _cursorPos.x < _bounds.right)
		target.blitFrom(*_cursorSurface, _cursorRect, _cursorPos);
}

int DialogTextInput::getCursorHeight() const {
	return _gui->getCurrentFont().getFontHeight();
}

void Dialog::calculateBoundsFromPrebuilt(const PrebuiltDialogBounds &bounds) {
	_bounds = Common::Rect(
		bounds.left,
		bounds.top,
		bounds.right,
		bounds.bottom);
}

static void cursorTimerHandler(void *refCon) {
	DialogTextInput *w = (DialogTextInput *)refCon;

	w->_cursorState = !w->_cursorState;
	w->_cursorDirty = true;
}

void DialogTextInput::updateCursorPos() {
	_cursorPos.x = _bounds.left + _gui->getCurrentFont().getStringWidth(_text); 
	_cursorPos.y = _bounds.top + _gui->getCurrentFont().getFontHeight() - getCursorHeight();
	_cursorPos.y += _text.empty() ? 3 : 0;
	_cursorDirty = true;
}

void DialogTextInput::undrawCursor() {
	_cursorState = false;
	_cursorDirty = true;
}

} // End of namespace MacVenture
