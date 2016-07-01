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

#include "common/file.h"
#include "image/bmp.h"

#include "macventure/macventure.h"
#include "macventure/gui.h"

// For the dragged object
#include "common/system.h"

namespace MacVenture {

enum MenuAction;

enum {
	kCursorWidth = 2, // HACK Arbitrary width to test
	kCursorHeight = 2
};

enum {
	kExitButtonWidth = 10, // HACK Arbitrary width to test
	kExitButtonHeight = 10
};

enum {
	kMenuHighLevel = -1,
	kMenuAbout = 0,
	kMenuFile = 1,
	kMenuEdit = 2,
	kMenuSpecial = 3
};

static const Graphics::MenuData menuSubItems[] = {
	{ kMenuHighLevel,	"File",				0, 0, false },
	{ kMenuHighLevel,	"Edit",				0, 0, false },
	{ kMenuHighLevel,	"Special",			0, 0, false },
	{ kMenuHighLevel,	"Font",				0, 0, false },
	{ kMenuHighLevel,	"FontSize",			0, 0, false },

	//{ kMenuAbout,		"About",			kMenuActionAbout, 0, true},

	{ kMenuFile,		"New",				kMenuActionNew, 0, true },
	{ kMenuFile,		NULL,				0, 0, false },
	{ kMenuFile,		"Open...",			kMenuActionOpen, 0, true },
	{ kMenuFile,		"Save",				kMenuActionSave, 0, true },
	{ kMenuFile,		"Save as...",		kMenuActionSaveAs, 0, true },
	{ kMenuFile,		NULL,				0, 0, false },
	{ kMenuFile,		"Quit",				kMenuActionQuit, 0, true },

	{ kMenuEdit,		"Undo",				kMenuActionUndo, 'Z', true },
	{ kMenuEdit,		NULL,				0, 0, false },
	{ kMenuEdit,		"Cut",				kMenuActionCut, 'K', true },
	{ kMenuEdit,		"Copy",				kMenuActionCopy, 'C', true },
	{ kMenuEdit,		"Paste",			kMenuActionPaste, 'V', true },
	{ kMenuEdit,		"Clear",			kMenuActionClear, 'B', true },

	{ kMenuSpecial,		"Clean Up",			kMenuActionCleanUp, 0, true },
	{ kMenuSpecial,		"Mess Up",			kMenuActionMessUp, 0, true },

	{ 0,				NULL,				0, 0, false }
};

bool commandsWindowCallback(Graphics::WindowClick, Common::Event &event, void *gui);
bool mainGameWindowCallback(Graphics::WindowClick, Common::Event &event, void *gui);
bool outConsoleWindowCallback(Graphics::WindowClick, Common::Event &event, void *gui);
bool selfWindowCallback(Graphics::WindowClick, Common::Event &event, void *gui);
bool exitsWindowCallback(Graphics::WindowClick, Common::Event &event, void *gui);
bool diplomaWindowCallback(Graphics::WindowClick, Common::Event &event, void *gui);
bool inventoryWindowCallback(Graphics::WindowClick, Common::Event &event, void *gui);

void menuCommandsCallback(int action, Common::String &text, void *data);

Gui::Gui(MacVentureEngine *engine, Common::MacResManager *resman) {
	_engine = engine;
	_resourceManager = resman;
	_windowData = nullptr;
	_controlData = nullptr;
	_draggedObj.id = 0;
	_draggedObj.pos = Common::Point(0, 0);

	initGUI();
}

Gui::~Gui() {

	if (_windowData)
		delete _windowData;

	if (_controlData)
		delete _controlData;

	if (_exitsData)
		delete _exitsData;

	Common::HashMap<ObjID, ImageAsset*>::const_iterator it = _assets.begin();
	for (; it != _assets.end(); it++) {
		delete it->_value;
	}
}

void Gui::initGUI() {
	_screen.create(kScreenWidth, kScreenHeight, Graphics::PixelFormat::createFormatCLUT8());
	_wm.setScreen(&_screen);

	// Menu
	_menu = _wm.addMenu();
	if (!loadMenus())
		error("Could not load menus");
	_menu->setCommandsCallback(menuCommandsCallback, this);
	_menu->calcDimensions();

	loadGraphics();

	if (!loadWindows())
		error("Could not load windows");

	initWindows();

	assignObjReferences();

	if (!loadControls())
		error("Could not load controls");

	draw();

}

void Gui::draw() {

	// Will be performance-improved after the milestone
	_wm.setFullRefresh(true);

	drawWindows();

	_wm.draw();

	drawDraggedObject();

	//drawWindowTitle(kMainGameWindow, _mainGameWindow->getSurface());
}

void Gui::drawMenu() {
	_menu->draw(&_screen);
}

void Gui::drawTitle() {
	warning("drawTitle hasn't been tested yet");
}

void Gui::clearControls() {
	if (!_controlData)
		return;

	Common::Array<CommandButton>::iterator it = _controlData->begin();
	for (; it != _controlData->end(); ++it) {
		it->unselect();
	}
}

const WindowData& Gui::getWindowData(WindowReference reference) {
	return findWindowData(reference);
}

const Graphics::Font& Gui::getCurrentFont() {
	return *_wm.getFont("Chicago-12", Graphics::FontManager::kBigGUIFont);
}

void Gui::bringToFront(WindowReference winID) {
	// FIXME: There has to be a better way to do this, maybe with the _wm
	switch (winID) {
	case MacVenture::kCommandsWindow:
		_controlsWindow->setActive(true);
		break;
	case MacVenture::kMainGameWindow:
		_mainGameWindow->setActive(true);
		break;
	case MacVenture::kOutConsoleWindow:
		_outConsoleWindow->setActive(true);
		break;
	case MacVenture::kSelfWindow:
		_selfWindow->setActive(true);
		break;
	case MacVenture::kExitsWindow:
		_exitsWindow->setActive(true);
		break;
	case MacVenture::kDiplomaWindow:
		_diplomaWindow->setActive(true);
		break;
	default:
		break;
	}
}

void Gui::setWindowTitle(WindowReference winID, Common::String string) {
	findWindowData(winID).title = string;
	findWindowData(winID).titleLength = string.size();
}

void Gui::updateWindowInfo(WindowReference ref, ObjID objID, const Common::Array<ObjID> &children) {
	WindowData &data = findWindowData(ref);
	data.children.clear();
	data.objRef = objID;
	uint32 originx = 0x7fff;
	uint32 originy = 0x7fff;
	for (uint i = 0; i < children.size(); i++) {
		if (children[i] != 1) {
			ObjID child = children[i];
			if (ref != kMainGameWindow) {
				Common::Point childPos = _engine->getObjPosition(child);
				originx = originx > (uint)childPos.x ? (uint)childPos.x : originx;
				originy = originy > (uint)childPos.y ? (uint)childPos.y : originy;
			}
			data.children.push_back(DrawableObject(child, kBlitBIC));
		}
	}
	if (originx != 0x7fff) data.bounds.left = originx;
	if (originy != 0x7fff) data.bounds.top = originy;
	if (ref != kMainGameWindow) data.updateScroll = true;
}

void Gui::addChild(WindowReference target, ObjID child) {
	findWindowData(target).children.push_back(DrawableObject(child, kBlitBIC));
}

void Gui::removeChild(WindowReference target, ObjID child) {
	WindowData data = findWindowData(target);
	uint index = 0;
	for (;index < data.children.size(); index++) {
		if (data.children[index].obj == child) break;
	}

	if (index < data.children.size())
		data.children.remove_at(index);
}

void Gui::initWindows() {

	// Game Controls Window
	_controlsWindow = _wm.addWindow(false, false, false);
	_controlsWindow->setDimensions(getWindowData(kCommandsWindow).bounds);
	_controlsWindow->setActive(false);
	_controlsWindow->setCallback(commandsWindowCallback, this);
	loadBorder(_controlsWindow, "border_command.bmp", false);
	loadBorder(_controlsWindow, "border_command.bmp", true);

	// Main Game Window
	_mainGameWindow = _wm.addWindow(false, false, false);
	_mainGameWindow->setDimensions(getWindowData(kMainGameWindow).bounds);
	_mainGameWindow->setActive(false);
	_mainGameWindow->setCallback(mainGameWindowCallback, this);
	loadBorder(_mainGameWindow, "border_no_scroll_inac.bmp", false);
	loadBorder(_mainGameWindow, "border_no_scroll_act.bmp", true);

	// In-game Output Console
	_outConsoleWindow = _wm.addWindow(false, true, true);
	_outConsoleWindow->setDimensions(Common::Rect(20, 20, 120, 120));
	_outConsoleWindow->setActive(false);
	_outConsoleWindow->setCallback(outConsoleWindowCallback, this);
	loadBorder(_outConsoleWindow, "border_left_scroll_inac.bmp", false);
	loadBorder(_outConsoleWindow, "border_left_scroll_inac.bmp", true);

	// Self Window
	_selfWindow = _wm.addWindow(false, true, false);
	_selfWindow->setDimensions(getWindowData(kSelfWindow).bounds);
	_selfWindow->setActive(false);
	_selfWindow->setCallback(selfWindowCallback, this);
	loadBorder(_selfWindow, "border_no_scroll_inac.bmp", false);
	loadBorder(_selfWindow, "border_no_scroll_inac.bmp", true);

	// Exits Window
	_exitsWindow = _wm.addWindow(false, false, false);
	_exitsWindow->setDimensions(getWindowData(kExitsWindow).bounds);
	_exitsWindow->setActive(false);
	_exitsWindow->setCallback(exitsWindowCallback, this);
	loadBorder(_exitsWindow, "border_no_scroll_inac.bmp", false);
	loadBorder(_exitsWindow, "border_no_scroll_act.bmp", true);


}

void Gui::assignObjReferences() {

	findWindowData(kSelfWindow).objRef = 0;

}

WindowReference Gui::createInventoryWindow(ObjID objRef) {
	Graphics::MacWindow *newWindow = _wm.addWindow(true, true, true);
	WindowData newData;
	GlobalSettings settings = _engine->getGlobalSettings();
	newData.refcon = (WindowReference)ABS(_inventoryWindows.size()); // This is a HACK

	if (_windowData->back().refcon < 0x80) { // There is already another inventory window
		newData.bounds = _windowData->back().bounds; // Inventory windows are always last
		newData.bounds.translate(newData.bounds.left + settings.invOffsetX, newData.bounds.top + settings.invOffsetY);
	}
	else {
		newData.bounds = Common::Rect(
			settings.invLeft,
			settings.invTop,
			settings.invLeft + settings.invWidth,
			settings.invTop + settings.invHeight);
	}
	newData.type = kZoomDoc;
	newData.hasCloseBox = true;
	newData.visible = true;
	newData.objRef = objRef;
	_windowData->push_back(newData);

	newWindow->setDimensions(newData.bounds);
	newWindow->setCallback(inventoryWindowCallback, this);
	loadBorder(newWindow, "border_no_scroll_inac.bmp", false);
	loadBorder(newWindow, "border_no_scroll_act.bmp", true);
	_inventoryWindows.push_back(newWindow);

	debug("Create new inventory window. Reference: %d", newData.refcon);
	return newData.refcon;
}

void Gui::loadBorder(Graphics::MacWindow * target, Common::String filename, bool active) {
	Common::File borderfile;

	if (!borderfile.open(filename)) {
		debug(1, "Cannot open border file");
		return;
	}

	Image::BitmapDecoder bmpDecoder;
	Common::SeekableReadStream *stream = borderfile.readStream(borderfile.size());
	Graphics::Surface source;
	Graphics::TransparentSurface *surface = new Graphics::TransparentSurface();

	if (stream) {
		debug(4, "Loading %s border from %s", (active ? "active" : "inactive"), filename.c_str());
		target->loadBorder(*stream, active);

		delete stream;
	}
	borderfile.close();
}

void Gui::loadGraphics() {
	_graphics = new Container(_engine->getFilePath(kGraphicPathID).c_str());
}

bool Gui::loadMenus() {

	// We assume that, if there are static menus, we don't need dynamic ones
	if (menuSubItems) {
		_menu->addStaticMenus(menuSubItems);
		return true;
	}

	Common::MacResIDArray resArray;
	Common::SeekableReadStream *res;
	Common::MacResIDArray::const_iterator iter;

	if ((resArray = _resourceManager->getResIDArray(MKTAG('M', 'E', 'N', 'U'))).size() == 0)
		return false;

	_menu->addMenuSubItem(0, "Abb", kMenuActionAbout, 0, 'A', true);

	int i = 1;
	for (iter = resArray.begin(); iter != resArray.end(); ++iter) {
		res = _resourceManager->getResource(MKTAG('M', 'E', 'N', 'U'), *iter);
		bool enabled;
		uint16 key;
		uint16 style;
		uint8 titleLength;
		char* title;

		/* Skip menuID, width, height, resourceID, placeholder */
		for (int skip = 0; skip < 5; skip++) { res->readUint16BE(); }
		enabled = res->readUint32BE();
		titleLength = res->readByte();
		title = new char[titleLength + 1];
		res->read(title, titleLength);
		title[titleLength] = '\0';

		if (titleLength > 1) {
			_menu->addMenuItem(title);

			// Read submenu items
			while (titleLength = res->readByte()) {
				title = new char[titleLength + 1];
				res->read(title, titleLength);
				title[titleLength] = '\0';
				// Skip icon
				res->readUint16BE();
				// Read key
				key = res->readUint16BE();
				// Skip mark
				res->readUint16BE();
				// Read style
				style = res->readUint16BE();
				_menu->addMenuSubItem(i, title, 0, style, key, false);
			}
		}

		i++;
	}

	return true;
}

bool Gui::loadWindows() {
	Common::MacResIDArray resArray;
	Common::SeekableReadStream *res;
	Common::MacResIDArray::const_iterator iter;

	_windowData = new Common::List<WindowData>();

	if ((resArray = _resourceManager->getResIDArray(MKTAG('W', 'I', 'N', 'D'))).size() == 0)
		return false;

	uint32 id = kCommandsWindow;
	for (iter = resArray.begin(); iter != resArray.end(); ++iter) {
		res = _resourceManager->getResource(MKTAG('W', 'I', 'N', 'D'), *iter);
		WindowData data;
		uint16 top, left, bottom, right;
		top = res->readUint16BE();
		left = res->readUint16BE();
		bottom = res->readUint16BE();
		right = res->readUint16BE();
		data.type = (MVWindowType)res->readUint16BE();
		data.bounds = Common::Rect(
			left - borderBounds(data.type).leftOffset,
			top - borderBounds(data.type).topOffset,
			right + borderBounds(data.type).rightOffset * 2,
			bottom + borderBounds(data.type).bottomOffset * 2);
		data.visible = res->readUint16BE();
		data.hasCloseBox = res->readUint16BE();
		data.refcon = (WindowReference)id; id++;
		res->readUint32BE(); // Skip the true id. For some reason it's reading 0
		data.titleLength = res->readByte();
		if (data.titleLength) {
			char* newTitle = new char[data.titleLength + 1];
			res->read(newTitle, data.titleLength);
			newTitle[data.titleLength] = '\0';
			data.title = Common::String(newTitle);
		}

		debug(4, "Window loaded: %s", data.title);

		_windowData->push_back(data);
	}

	return true;
}

bool Gui::loadControls() {
	Common::MacResIDArray resArray;
	Common::SeekableReadStream *res;
	Common::MacResIDArray::const_iterator iter;

	_controlData = new Common::Array<CommandButton>();
	_exitsData = new Common::Array<CommandButton>();

	if ((resArray = _resourceManager->getResIDArray(MKTAG('C', 'N', 'T', 'L'))).size() == 0)
		return false;

	uint16 commandsBorder = borderBounds(kPlainDBox).topOffset;
	uint32 id = kControlExitBox;
	for (iter = resArray.begin(); iter != resArray.end(); ++iter) {
		res = _resourceManager->getResource(MKTAG('C', 'N', 'T', 'L'), *iter);
		ControlData data;
		uint16 top, left, bottom, right;
		top = res->readUint16BE();
		left = res->readUint16BE();
		bottom = res->readUint16BE();
		right = res->readUint16BE();
		data.scrollValue = res->readUint16BE();
		data.visible = res->readByte();
		res->readByte(); // Unused
		data.scrollMax = res->readUint16BE();
		data.scrollMin = res->readUint16BE();
		data.cdef = res->readUint16BE();
		data.refcon = (ControlReference)id; id++;
		res->readUint32BE();
		data.titleLength = res->readByte();
		if (data.titleLength) {
			data.title = new char[data.titleLength + 1];
			res->read(data.title, data.titleLength);
			data.title[data.titleLength] = '\0';
		}
		if (data.refcon != kControlExitBox)
			data.border = commandsBorder;

		Common::Rect bounds(left, top, right, bottom); // For some reason, if I remove this it segfaults
		data.bounds = Common::Rect(left + data.border, top + data.border, right + data.border, bottom + data.border);

		i++;
	}

	return true;
}

void Gui::drawWindows() {

	drawCommandsWindow();
	drawMainGameWindow();
	drawSelfWindow();
	drawInventories();
	drawExitsWindow();

}

void Gui::drawCommandsWindow() {
	if (_engine->needsClickToContinue()) {
		Graphics::ManagedSurface *srf = _controlsWindow->getSurface();
		WindowData data = getWindowData(kCommandsWindow);
		uint16 border = borderBounds(data.type).topOffset;
		srf->fillRect(Common::Rect(border * 2, border * 2, srf->w - (border * 3), srf->h - (border * 3)), kColorWhite);
		getCurrentFont().drawString(
			srf,
			_engine->getCommandsPausedString(),
			0,
			(srf->h / 2) - getCurrentFont().getFontHeight(),
			data.bounds.right - data.bounds.left,
			kColorBlack,
			Graphics::kTextAlignCenter);
	} else {
		Common::Array<CommandButton>::const_iterator it = _controlData->begin();
		for (; it != _controlData->end(); ++it) {
			CommandButton button = *it;
			if (button.getData().refcon != kControlExitBox)
				button.draw(*_controlsWindow->getSurface());
		}
	}
}

void Gui::drawMainGameWindow() {
	const WindowData &data = getWindowData(kMainGameWindow);
	BorderBounds border = borderBounds(data.type);
	ObjID objRef = data.objRef;

	_mainGameWindow->setDirty(true);

	if (data.objRef > 0 && data.objRef < 2000) {
		if (!_assets.contains(objRef)) {
			_assets[objRef] = new ImageAsset(objRef, _graphics);
		}

		_assets[objRef]->blitInto(
			_mainGameWindow->getSurface(),
			border.leftOffset,
			border.topOffset,
			kBlitDirect);

	}
	drawObjectsInWindow(kMainGameWindow, _mainGameWindow->getSurface());

	findWindow(kMainGameWindow)->setDirty(true);
}

void Gui::drawSelfWindow() {
	drawObjectsInWindow(kSelfWindow, _selfWindow->getSurface());
	if (_engine->isObjSelected(1)) invertWindowColors(kSelfWindow);
	findWindow(kSelfWindow)->setDirty(true);
}

void Gui::drawInventories() {
	Common::List<WindowData>::const_iterator it = _windowData->begin();
	while (it != _windowData->end() && (*it).refcon >= 0x80) {
		it++;
	}
	Graphics::ManagedSurface *srf;
	while (it != _windowData->end()) {
		srf = _inventoryWindows[(*it).refcon]->getSurface();
		BorderBounds border = borderBounds((*it).type);
		srf->fillRect(Common::Rect(
			border.leftOffset,
			border.topOffset,
			srf->w + border.rightOffset,
			srf->h + border.bottomOffset), kColorWhite);
		drawObjectsInWindow((*it).refcon, _inventoryWindows[(*it).refcon]->getSurface());
		it++;

		findWindow((*it).refcon)->setDirty(true);
	}

}

void Gui::drawExitsWindow() {

	Graphics::ManagedSurface *srf = _exitsWindow->getSurface();
	BorderBounds border = borderBounds(getWindowData(kExitsWindow).type);

	srf->fillRect(Common::Rect(
		border.leftOffset,
		border.topOffset,
		srf->w + border.rightOffset,
		srf->h + border.bottomOffset), kColorWhite);

	Common::Array<CommandButton>::const_iterator it = _exitsData->begin();
	for (; it != _exitsData->end(); ++it) {
		CommandButton button = *it;
		button.draw(*_exitsWindow->getSurface());
	}

	findWindow(kExitsWindow)->setDirty(true);

	// To be deleted
	//g_system->copyRectToScreen(srf->getPixels(), srf->pitch, 0, 0, srf->w, srf->h);
	//g_system->updateScreen();
}

void Gui::drawObjectsInWindow(WindowReference target, Graphics::ManagedSurface * surface) {
	WindowData &data = findWindowData(target);
	BorderBounds border = borderBounds(data.type);
	Common::Point pos;
	ObjID child;
	BlitMode mode;

	if (data.children.size() == 0) return;

	for (uint i = 0; i < data.children.size(); i++) {
		child = data.children[i].obj;
		mode = (BlitMode)data.children[i].mode;
		pos = _engine->getObjPosition(child);
		pos += Common::Point(border.leftOffset, border.topOffset);

		if (child < 600) { // Small HACK until I figre out where the last garbage child in main game window comes from
			if (!_assets.contains(child)) {
				_assets[child] = new ImageAsset(child, _graphics);
			}

			_assets[child]->blitInto(
				surface,
				pos.x,
				pos.y,
				mode);

			if (_engine->isObjSelected(child))
				_assets[child]->blitInto(
					surface, pos.x, pos.y, kBlitXOR);

			// For test
			surface->frameRect(Common::Rect(
				pos.x,
				pos.y,
				pos.x + _assets[child]->getWidth(),
				pos.y + _assets[child]->getHeight()), kColorGreen);
		}

	}

}

void Gui::drawWindowTitle(WindowReference target, Graphics::ManagedSurface * surface) {
	WindowData &data = findWindowData(target);
	BorderBounds border = borderBounds(data.type);

	uint left = 10;//getCurrentFont().getStringWidth(data.title) - 10;
	uint right = 30;//getCurrentFont().getStringWidth(data.title) + 10;

	surface->fillRect(Common::Rect(left, 0, right, border.topOffset - 1), kColorGray);
	getCurrentFont().drawString(
		surface,
		data.title,
		0,
		right,
		right - left,
		kColorBlack,
		Graphics::kTextAlignCenter);
}

void Gui::drawDraggedObject() {
	if (_draggedObj.id != 0) {
		if (!_assets.contains(_draggedObj.id))
			_assets[_draggedObj.id] = new ImageAsset(_draggedObj.id, _graphics);

		ImageAsset *asset = _assets[_draggedObj.id];

		_draggedSurface.create(asset->getWidth(), asset->getHeight(), _screen.format);
		_screen.copyRectToSurface(_draggedSurface, _draggedObj.pos.x, _draggedObj.pos.y,
			Common::Rect(asset->getWidth() - 1, asset->getHeight() - 1));

		asset->blitInto(&_draggedSurface, 0, 0, kBlitBIC);

		g_system->copyRectToScreen(
			_draggedSurface.getPixels(),
			_draggedSurface.pitch,
			_draggedObj.pos.x,
			_draggedObj.pos.y,
			_draggedSurface.w,
			_draggedSurface.h);

	}
}


void Gui::updateWindow(WindowReference winID, bool containerOpen) {
	if (winID == kSelfWindow || containerOpen) {
		WindowData &data = findWindowData(winID);
		if (winID == kCommandsWindow) {
			Common::Array<CommandButton>::iterator it = _controlData->begin();
			for (; it != _controlData->end(); ++it) {
				it->unselect();
			}
		}
		Common::Array<DrawableObject> &children = data.children;
		for (uint i = 0; i < children.size(); i++) {
			uint flag = 0;
			ObjID child = children[i].obj;
			BlitMode mode = kBlitDirect;
			bool off = !_engine->isObjVisible(child);
			if (flag || !off || !_engine->isObjClickable(child)) {
				mode = kBlitBIC;
				if (off || flag) {
					mode = kBlitXOR;
				}
				else if (_engine->isObjSelected(child)) {
					mode = kBlitOR;
				}
				children[i] = DrawableObject(child, mode);
			}
			else {
				children[i] = DrawableObject(child, kBlitXOR);
			}
		}
		if (winID == kMainGameWindow) {
			drawMainGameWindow();
		}
		else {
			Graphics::MacWindow *winRef = findWindow(winID);
			winRef->getSurface()->fillRect(data.bounds, kColorGray);
		}
		if (data.type == kZoomDoc && data.updateScroll) {
			warning("Unimplemented: update scroll");
		}
	}
}

void Gui::unselectExits() {
	Common::Array<CommandButton>::const_iterator it = _exitsData->begin();
	for (; it != _exitsData->end(); ++it) {
		CommandButton button = *it;
		button.unselect();
	}
}

void Gui::updateExit(ObjID obj) {
	if (!_engine->isObjExit(obj)) return;

	BorderBounds border = borderBounds(getWindowData(kExitsWindow).type);

	int ctl = -1;
	int i = 0;
	Common::Array<CommandButton>::const_iterator it = _exitsData->begin();
	for (;it != _exitsData->end(); it++) {
		if (it->getData().refcon == obj)
			ctl = i;
		else
			i++;
	}

	if (ctl != -1)
		_exitsData->remove_at(ctl);

	if (!_engine->isHiddenExit(obj) &&
		_engine->getParent(obj) == _engine->getParent(1))
	{
		ControlData data;
		data.titleLength = 0;
		data.refcon = obj;
		Common::Point pos = _engine->getObjExitPosition(obj);
		pos.x = border.leftOffset;
		pos.y = border.topOffset;
		data.bounds = Common::Rect(pos.x, pos.y, pos.x + kExitButtonWidth, pos.y + kExitButtonHeight);
		data.visible = true;

		_exitsData->push_back(CommandButton(data, this));
	}
}


Common::Point Gui::getWindowSurfacePos(WindowReference reference) {
	const WindowData &data = getWindowData(reference);
	BorderBounds border = borderBounds(data.type);
	return Common::Point(data.bounds.left + border.leftOffset, data.bounds.top + border.topOffset);
}

WindowData & Gui::findWindowData(WindowReference reference) {
	assert(_windowData);

	Common::List<WindowData>::iterator iter = _windowData->begin();
	while (iter->refcon != reference && iter != _windowData->end()) {
		iter++;
	}

	if (iter->refcon == reference)
		return *iter;

	error("Could not locate the desired window data");
}

Graphics::MacWindow * Gui::findWindow(WindowReference reference) {
	if (reference < 0x80) { // It's an inventory window
		return _inventoryWindows[reference];
	}
	switch (reference) {
	case MacVenture::kNoWindow:
		return nullptr;
	case MacVenture::kCommandsWindow:
		return _controlsWindow;
	case MacVenture::kMainGameWindow:
		return _mainGameWindow;
	case MacVenture::kOutConsoleWindow:
		return _outConsoleWindow;
	case MacVenture::kSelfWindow:
		return _selfWindow;
	case MacVenture::kExitsWindow:
		return _exitsWindow;
	case MacVenture::kDiplomaWindow:
		return _diplomaWindow;
	}
	return nullptr;
}

void Gui::checkSelect(ObjID obj, const Common::Event &event, const Common::Rect & clickRect, WindowReference ref) {
	if (_engine->isObjVisible(obj) &&
		_engine->isObjClickable(obj) &&
		isRectInsideObject(clickRect, obj))
	{
		selectDraggable(obj, ref, event.mouse);
		_engine->handleObjectSelect(obj, (WindowReference)ref, event, false);
	}
}

bool Gui::isRectInsideObject(Common::Rect target, ObjID obj) {
	if (_assets.contains(obj) &&
		//_engine->isObjClickable(obj) &&
		_engine->isObjVisible(obj))
	{
		Common::Rect bounds = _engine->getObjBounds(obj);
		Common::Rect intersection = bounds.findIntersectingRect(target);
		// We translate it to the image's coord system
		intersection = Common::Rect(
			intersection.left - bounds.left,
			intersection.top - bounds.top,
			intersection.left - bounds.left + intersection.width(),
			intersection.top - bounds.top + intersection.height());


		if (_assets[obj]->isRectInside(intersection)) {
			return true;
		}
	}
	return false;
}

void Gui::selectDraggable(ObjID child, WindowReference origin, Common::Point startPos) {
	if (_engine->isObjClickable(child)) {
		_draggedObj.id = child;
		_draggedObj.mouseOffset = (_engine->getObjPosition(child) + getWindowSurfacePos(origin)) - startPos;
		_draggedObj.pos = startPos + _draggedObj.mouseOffset;
	}
}

void Gui::handleDragRelease(Common::Point pos) {
	_draggedObj.id = 0;
	_engine->updateDelta(pos);
	_engine->selectControl(kControlOperate);
	_engine->activateCommand(kControlOperate);
	_engine->refreshReady();
	_engine->preparedToRun();
}

Common::Rect Gui::calculateClickRect(Common::Point clickPos, Common::Rect windowBounds) {
	int left = clickPos.x - windowBounds.left;
	int top = clickPos.y - windowBounds.top;
	return Common::Rect(left - kCursorWidth, top - kCursorHeight, left + kCursorWidth, top + kCursorHeight);
}


/* HANDLERS */
void Gui::handleMenuAction(MenuAction action) {
	switch (action)	{
	case MacVenture::kMenuActionAbout:
		debug("MacVenture Menu Action: About");
		break;
	case MacVenture::kMenuActionNew:
		debug("MacVenture Menu Action: New");
		break;
	case MacVenture::kMenuActionOpen:
		debug("MacVenture Menu Action: Open");
		break;
	case MacVenture::kMenuActionSave:
		debug("MacVenture Menu Action: Save");
		break;
	case MacVenture::kMenuActionSaveAs:
		debug("MacVenture Menu Action: Save As");
		break;
	case MacVenture::kMenuActionQuit:
		debug("MacVenture Menu Action: Quit");
		break;
	case MacVenture::kMenuActionUndo:
		debug("MacVenture Menu Action: Undo");
		break;
	case MacVenture::kMenuActionCut:
		debug("MacVenture Menu Action: Cut");
		break;
	case MacVenture::kMenuActionCopy:
		debug("MacVenture Menu Action: Copy");
		break;
	case MacVenture::kMenuActionPaste:
		debug("MacVenture Menu Action: Paste");
		break;
	case MacVenture::kMenuActionClear:
		debug("MacVenture Menu Action: Clear");
		break;
	case MacVenture::kMenuActionCleanUp:
		debug("MacVenture Menu Action: Clean Up");
		break;
	case MacVenture::kMenuActionMessUp:
		debug("MacVenture Menu Action: Mess Up");
		break;
	case MacVenture::kMenuActionCommand:
		debug("MacVenture Menu Action: GENERIC");
		break;
	default:
		break;
	}
}

/* CALLBACKS */

bool commandsWindowCallback(Graphics::WindowClick click, Common::Event &event, void *gui) {
	Gui *g = (Gui*)gui;
	return g->processCommandEvents(click, event);
}

bool mainGameWindowCallback(Graphics::WindowClick click, Common::Event &event, void *gui) {
	Gui *g = (Gui*)gui;
	return g->processMainGameEvents(click, event);
}

bool outConsoleWindowCallback(Graphics::WindowClick click, Common::Event &event, void *gui) {
	Gui *g = (Gui*)gui;
	return g->processOutConsoleEvents(click, event);
}

bool selfWindowCallback(Graphics::WindowClick click, Common::Event &event, void *gui) {
	Gui *g = (Gui*)gui;

	return g->processSelfEvents(click, event);
}

bool exitsWindowCallback(Graphics::WindowClick click, Common::Event &event, void *gui) {
	Gui *g = (Gui*)gui;

	return g->processExitsEvents(click, event);
}

bool diplomaWindowCallback(Graphics::WindowClick click, Common::Event &event, void *gui) {
	Gui *g = (Gui*)gui;

	return g->processDiplomaEvents(click, event);
}

bool inventoryWindowCallback(Graphics::WindowClick click, Common::Event &event, void *gui) {
	Gui *g = (Gui*)gui;

	return g->processInventoryEvents(click, event);
}

void menuCommandsCallback(int action, Common::String &text, void *data) {
	Gui *g = (Gui *)data;

	g->handleMenuAction((MenuAction)action);
}


void Gui::invertWindowColors(WindowReference winID) {
	Graphics::ManagedSurface *srf = findWindow(winID)->getSurface();
	for (uint y = 0; y < srf->h; y++) {
		for (uint x = 0; x < srf->w; x++) {
			byte p = *(byte *)srf->getBasePtr(x, y);
			*(byte *)srf->getBasePtr(x, y) =
				(p == kColorWhite) ? kColorBlack : kColorGray;
		}
	}
}

bool Gui::tryCloseWindow(WindowReference winID) {
	WindowData data = findWindowData(winID);
	if (winID < 0x80) { // Inventory window
		warning("Window closing not implemented");
	} else {
		warning("Window closing not implemented");
	}
	return true;
}

uint Gui::getObjWidth(ObjID obj) {
	if (!_assets.contains(obj)) return 0;
	return _assets[obj]->getWidth();
}

uint Gui::getObjHeight(ObjID obj) {
	if (!_assets.contains(obj)) return 0;
	return _assets[obj]->getHeight();
}

bool Gui::processEvent(Common::Event &event) {
	bool processed = false;
	if (event.type == Common::EVENT_MOUSEMOVE) {
		if (_draggedObj.id != 0) {
			_draggedObj.pos = event.mouse + _draggedObj.mouseOffset;
		}
		processed = true;

		// TEST
		Common::Rect mr = calculateClickRect(event.mouse, _screen.getBounds());
		_screen.fillRect(mr, kColorGreen);
		g_system->copyRectToScreen(_screen.getPixels(), _screen.pitch, 0, 0, _screen.w, _screen.h);
		g_system->updateScreen();
	}
	else if (event.type == Common::EVENT_LBUTTONUP) {
		if (_draggedObj.id != 0) {
			handleDragRelease(event.mouse);
		}
		processed = true;
	}

	processed |= _wm.processEvent(event);
	return (processed);
}

bool Gui::processCommandEvents(WindowClick click, Common::Event &event) {
	if (event.type == Common::EVENT_LBUTTONUP) {
		if (_engine->needsClickToContinue()) {
			_engine->activateCommand(kControlClickToContinue);
			return true;
		}

		Common::Point position(
			event.mouse.x - _controlsWindow->getDimensions().left,
			event.mouse.y - _controlsWindow->getDimensions().top);

		CommandButton data;
		if (!_controlData)
			return false;

		Common::Array<CommandButton>::iterator it = _controlData->begin();
		for (; it != _controlData->end(); ++it) {
			if (it->isInsideBounds(position)) {
				it->select();
				data = *it;
			}
			else {
				it->unselect();
			}
		}


		_engine->selectControl((ControlReference)data.getData().refcon);
		_engine->activateCommand((ControlReference)data.getData().refcon);
		_engine->refreshReady();
		_engine->preparedToRun();
	}
	return false;
}

bool MacVenture::Gui::processMainGameEvents(WindowClick click, Common::Event & event) {
	if (_engine->needsClickToContinue())
		return true;

	if (click == kBorderInner && event.type == Common::EVENT_LBUTTONDOWN) {
		WindowData &data = findWindowData(kMainGameWindow);
		ObjID child;
		Common::Point pos;
		// Click rect to local coordinates. We assume the click is inside the window ^
		Common::Rect clickRect = calculateClickRect(event.mouse, _mainGameWindow->getDimensions());
		for (Common::Array<DrawableObject>::const_iterator it = data.children.begin(); it != data.children.end(); it++) {
			child = (*it).obj;
			checkSelect(child, event, clickRect, kMainGameWindow);
		}
	}
	return false;
}
bool MacVenture::Gui::processOutConsoleEvents(WindowClick click, Common::Event & event) {
	if (_engine->needsClickToContinue())
		return true;

	return getWindowData(kOutConsoleWindow).visible;
}

bool MacVenture::Gui::processSelfEvents(WindowClick click, Common::Event & event) {
	if (_engine->needsClickToContinue())
		return true;

	if (event.type == Common::EVENT_LBUTTONUP) {
		_engine->handleObjectSelect(1, kSelfWindow, event, false);
	}
	return true;
}

bool MacVenture::Gui::processExitsEvents(WindowClick click, Common::Event & event) {
	if (event.type == Common::EVENT_LBUTTONUP) {
		if (_engine->needsClickToContinue()) {
			return true;
		}

		Common::Point position(
			event.mouse.x - _exitsWindow->getDimensions().left,
			event.mouse.y - _exitsWindow->getDimensions().top);

		CommandButton data;
		if (!_exitsData)
			return false;

		Common::Array<CommandButton>::iterator it = _exitsData->begin();
		for (; it != _exitsData->end(); ++it) {
			if (it->isInsideBounds(position)) {
				it->select();
				data = *it;
			}
			else {
				it->unselect();
			}
		}

		_engine->handleObjectSelect(data.getData().refcon, kExitsWindow, event, false);
	}
	return getWindowData(kExitsWindow).visible;
}

bool MacVenture::Gui::processDiplomaEvents(WindowClick click, Common::Event & event) {
	if (_engine->needsClickToContinue())
		return true;

	return getWindowData(kDiplomaWindow).visible;
}

bool Gui::processInventoryEvents(WindowClick click, Common::Event & event) {
	if (_engine->needsClickToContinue())
		return true;

	if (click == kBorderInner && event.type == Common::EVENT_LBUTTONDOWN) {

		// Find the appropriate window
		uint ref = 0;
		uint i;
		for (uint i = 0; i < _inventoryWindows.size(); i++) {
			if (_inventoryWindows[i]->hasAllFocus()) { // HACK
				ref = i;
			}
		}

		WindowData &data = findWindowData((WindowReference) ref);
		ObjID child;
		Common::Point pos;
		// Click rect to local coordinates. We assume the click is inside the window ^
		Common::Rect clickRect = calculateClickRect(event.mouse, _inventoryWindows[i]->getDimensions());
		for (Common::Array<DrawableObject>::const_iterator it = data.children.begin(); it != data.children.end(); it++) {
			child = (*it).obj;
			checkSelect(child, event, clickRect, (WindowReference)ref);
		}
	}
	return true;
}

/* Ugly switches */

BorderBounds Gui::borderBounds(MVWindowType type) {
	switch (type) {
	case MacVenture::kDocument:
		break;
	case MacVenture::kDBox:
		break;
	case MacVenture::kPlainDBox:
		return BorderBounds(6, 6, 6, 6);
	case MacVenture::kAltBox:
		return BorderBounds(8, 9, 11, 10);
		break;
	case MacVenture::kNoGrowDoc:
		return BorderBounds(1, 17, 1, 1);
	case MacVenture::kMovableDBox:
		break;
	case MacVenture::kZoomDoc:
		return BorderBounds(1, 19, 16, 1);
	case MacVenture::kZoomNoGrow:
		break;
	case MacVenture::kRDoc16:
		break;
	case MacVenture::kRDoc4:
		return BorderBounds(1, 19, 1, 1);
	case MacVenture::kRDoc6:
		break;
	case MacVenture::kRDoc10:
		break;
	default:
		break;
	}

	return BorderBounds(0, 0, 0, 0);
}

} // End of namespace MacVenture
