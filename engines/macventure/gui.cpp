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

//Test
#include "common/system.h"

namespace MacVenture {

enum MenuAction;

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
	initGUI();
}

Gui::~Gui() {

	if (_windowData)
		delete _windowData;

	if (_controlData)
		delete _controlData;

	Common::HashMap<ObjID, ImageAsset*>::const_iterator it = _assets.begin();
	for (; it != _assets.end(); it++) {
		delete it->_value;
	}
}

void Gui::draw() {

	drawWindows();

	drawTitle();

	_wm.draw();
}

void Gui::drawMenu() {
	_menu->draw(&_screen);
}

void Gui::drawTitle() {
	warning("drawTitle hasn't been tested yet");
}

void Gui::drawExit(ObjID id) {
	warning("Unimplemented method: drawExit");
}

bool Gui::processEvent(Common::Event &event) {
	bool processed = false;
	if (event.type == Common::EVENT_LBUTTONDOWN) {
		debug("Click on the ui");
	}
	return (processed || _wm.processEvent(event));
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
	uint32 originx = 0x7fff;
	uint32 originy = 0x7fff;
	for (uint i = 0; i < children.size(); i++) {
		if (children[i] != 1) {
			ObjID child = children[i];
			if (ref != kMainGameWindow) {
				Common::Point childPos = _engine->getObjPosition(child);
				originx = originx > childPos.x ? childPos.x : originx;
				originy = originy > childPos.y ? childPos.y : originy;
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

	data.children.remove_at(index);
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

	if (!loadControls())
		error("Could not load controls");


	draw();

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
	loadBorder(_mainGameWindow, "border_command.bmp", false);
	loadBorder(_mainGameWindow, "border_command.bmp", true);

	// In-game Output Console
	_outConsoleWindow = _wm.addWindow(false, true, true);
	_outConsoleWindow->setDimensions(Common::Rect(20, 20, 120, 120));
	_outConsoleWindow->setActive(false);
	_outConsoleWindow->setCallback(outConsoleWindowCallback, this);
	loadBorder(_outConsoleWindow, "border_command.bmp", false);

	// Self Window
	_selfWindow = _wm.addWindow(false, true, true);
	_selfWindow->setDimensions(getWindowData(kSelfWindow).bounds);
	_selfWindow->setActive(false);
	_selfWindow->setCallback(selfWindowCallback, this);
	loadBorder(_selfWindow, "border_self_inac.bmp", false);
	loadBorder(_selfWindow, "border_self_act.bmp", true);
	findWindowData(kSelfWindow).children.push_back(DrawableObject(1, kBlitDirect));

	// Exits Window
	_exitsWindow = _wm.addWindow(false, true, true);
	_exitsWindow->setDimensions(getWindowData(kExitsWindow).bounds);
	_exitsWindow->setActive(false);
	_exitsWindow->setCallback(exitsWindowCallback, this);
	loadBorder(_exitsWindow, "border_title_inac.bmp", false);
	loadBorder(_exitsWindow, "border_title_inac.bmp", true);

	// Diploma Window (we can go without it for now)
	/*
	_diplomaWindow = _wm.addWindow(false, true, true);
	_diplomaWindow->setDimensions(getWindowData(kDiplomaWindow).bounds);
	_diplomaWindow->setActive(false);
	_diplomaWindow->setCallback(diplomaWindowCallback, this);
	loadBorder(_diplomaWindow, "border_command.bmp", false);
	// Render invisible for now
	_diplomaWindow->getSurface()->fillRect(_diplomaWindow->getSurface()->getBounds(), kColorGreen2);
	*/

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
		debug(4, "Loading %s border from %s", (active ? "active" : "inactive"), filename);
		bmpDecoder.loadStream(*stream);
		source = *(bmpDecoder.getSurface());

		source.convertToInPlace(surface->getSupportedPixelFormat(), bmpDecoder.getPalette());
		surface->create(source.w, source.h, source.format);
		surface->copyFrom(source);
		surface->applyColorKey(255, 0, 255, false);

		target->setBorder(*surface, active);

		borderfile.close();

		delete stream;
	}
}

void Gui::loadGraphics() {
	_graphics = new Container("Shadowgate II/Shadow Graphic");
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

	_controlData = new Common::List<CommandButton>();

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

}

void Gui::drawCommandsWindow() {
	if (_engine->isPaused()) {
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
		Common::List<CommandButton>::const_iterator it = _controlData->begin();
		for (; it != _controlData->end(); ++it) {
			CommandButton button = *it;
			if (button.getData().refcon != kControlExitBox)
				button.draw(*_controlsWindow->getSurface());
		}
	}
}

void Gui::drawMainGameWindow() {
	Graphics::ManagedSurface *srf = _mainGameWindow->getSurface();
	BorderBounds border = borderBounds(getWindowData(kMainGameWindow).type);

	if (!_assets.contains(3)) {
		_assets[3] = new ImageAsset(3, _graphics);
	}

	_assets[3]->blitInto(
		_mainGameWindow->getSurface(),
		border.leftOffset * 2,
		border.topOffset * 2,
		kBlitDirect);

	drawObjectsInWindow(kMainGameWindow, _mainGameWindow->getSurface());

	// To be deleted
	_wm.draw();

	g_system->updateScreen();

}

void Gui::drawSelfWindow() {
	Graphics::ManagedSurface *srf = _selfWindow->getSurface();
	BorderBounds border = borderBounds(getWindowData(kSelfWindow).type);
}

void Gui::drawObjectsInWindow(WindowReference target, Graphics::ManagedSurface * surface) {
	WindowData &data = findWindowData(kMainGameWindow);
	BorderBounds border = borderBounds(data.type);
	Common::Point pos;
	ObjID child;
	BlitMode mode;
	for (Common::Array<DrawableObject>::const_iterator it = data.children.begin(); it != data.children.end(); it++) {
		child = (*it).obj;
		mode = (BlitMode)(*it).mode;
		pos = _engine->getObjPosition(child);
		if (!_assets.contains(child)) {
			_assets[child] = new ImageAsset(child, _graphics);
		}
		_assets[child]->blitInto(
			surface,
			border.leftOffset * 2 + pos.x,
			border.topOffset * 2 + pos.y,
			mode);

		// To be deleted
		_wm.draw();

		g_system->updateScreen();

	}
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

void Gui::updateWindow(WindowReference winID, bool containerOpen) {
	if (winID > 0x90 || winID == kNoWindow) return;
	if (winID == kSelfWindow || containerOpen) {
		if (winID == kMainGameWindow) {
			drawMainGameWindow();
		} else {
			warning("Unimplemented: fill window with background");
		}
		WindowData &data = findWindowData(winID);
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
				} else if (_engine->isObjSelected(child)) {
					mode = kBlitOR;
				}
				children[i] = DrawableObject(child, mode);
			}
		}
		if (data.type == kZoomDoc && data.updateScroll) {
			warning("Unimplemented: update scroll");
		}
	}
}

WindowReference Gui::createInventoryWindow() {
	Graphics::MacWindow *newWindow = _wm.addWindow(true, true, true);
	WindowData newData;
	GlobalSettings settings = _engine->getGlobalSettings();
	newData.refcon = (WindowReference)ABS(_inventoryWindows.size()); // This is a hack

	if (_windowData->back().refcon < 0x80) { // There is already another inventory window
		newData.bounds = _windowData->back().bounds; // Inventory windows are always last
		newData.bounds.translate(newData.bounds.left + settings.invOffsetX, newData.bounds.top + settings.invOffsetY);
	} else {
		newData.bounds = Common::Rect(
			settings.invLeft,
			settings.invTop,
			settings.invLeft + settings.invWidth,
			settings.invTop + settings.invHeight);
	}
	newData.type = kZoomDoc;
	newData.hasCloseBox = true;
	newData.visible = true;
	_windowData->push_back(newData);

	newWindow->setDimensions(newData.bounds);
	newWindow->setCallback(inventoryWindowCallback, this);
	loadBorder(newWindow, "border_self_inac.bmp", false);
	loadBorder(newWindow, "border_self_act.bmp", true);
	_inventoryWindows.push_back(newWindow);

	debug(3, "Create new inventory window. Reference: %d", newData.refcon);
	return newData.refcon;
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


bool Gui::processCommandEvents(WindowClick click, Common::Event &event) {
	if (event.type == Common::EVENT_LBUTTONUP) {
		if (_engine->isPaused())
			return true;

		Common::Point position(
			event.mouse.x - _controlsWindow->getDimensions().left,
			event.mouse.y - _controlsWindow->getDimensions().top);

		CommandButton data;
		Common::List<CommandButton>::const_iterator it = _controlData->begin();
		for (; it != _controlData->end(); ++it) {
			if (it->isInsideBounds(position)) {
				debug(4, "Command active: %s", it->getData().title);
				data = *it;
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
	return getWindowData(kMainGameWindow).visible;
}
bool MacVenture::Gui::processOutConsoleEvents(WindowClick click, Common::Event & event) {
	return getWindowData(kOutConsoleWindow).visible;
}

bool MacVenture::Gui::processSelfEvents(WindowClick click, Common::Event & event) {
	return getWindowData(kSelfWindow).visible;
}

bool MacVenture::Gui::processExitsEvents(WindowClick click, Common::Event & event) {
	return getWindowData(kExitsWindow).visible;
}

bool MacVenture::Gui::processDiplomaEvents(WindowClick click, Common::Event & event) {
	return getWindowData(kDiplomaWindow).visible;
}

bool Gui::processInventoryEvents(WindowClick click, Common::Event & event) {
	return false;
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
		//return BorderBounds(8, 9, 11, 10); // For now, I'll stick to the original bmp, it's gorgeous
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
