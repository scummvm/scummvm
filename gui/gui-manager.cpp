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

#include "common/events.h"
#include "common/translation.h"
#include "common/zip-set.h"
#include "gui/EventRecorder.h"

#include "backends/keymapper/action.h"
#include "backends/keymapper/keymap.h"
#include "backends/keymapper/keymapper.h"
#include "backends/keymapper/standard-actions.h"

#include "gui/gui-manager.h"
#include "gui/dialog.h"
#include "gui/ThemeEngine.h"
#include "gui/ThemeEval.h"
#include "gui/Tooltip.h"
#include "gui/widget.h"

#include "graphics/cursorman.h"
#include "graphics/macgui/macwindowmanager.h"

namespace Common {
DECLARE_SINGLETON(GUI::GuiManager);
}

namespace GUI {

enum {
	kDoubleClickDelay = 500, // milliseconds
	kCursorAnimateDelay = 250,
	kTooltipDelay = 1250,
	kTooltipSameWidgetDelay = 7000
};

// Constructor
GuiManager::GuiManager() : CommandSender(nullptr), _redrawStatus(kRedrawDisabled), _stateIsSaved(false),
	_cursorAnimateCounter(0), _cursorAnimateTimer(0) {
	_theme = nullptr;
	_useStdCursor = false;

	_system = g_system;
	_lastScreenChangeID = _system->getScreenChangeID();

	computeScaleFactor();

	_launched = false;

	_useRTL = false;

	_iconsSetChanged = false;

	_topDialogLeftPadding = 0;
	_topDialogRightPadding = 0;

	_displayTopDialogOnly = false;

	// Clear the cursor
	memset(_cursor, 0xFF, sizeof(_cursor));

#ifdef USE_TRANSLATION
	// Enable translation
	TransMan.setLanguage(ConfMan.get("gui_language").c_str());
	setLanguageRTL();
#endif // USE_TRANSLATION

	initTextToSpeech();
	initIconsSet();
	_iconsSetChanged = false;

	ConfMan.registerDefault("gui_theme", "scummremastered");
	Common::String themefile(ConfMan.get("gui_theme"));

	ConfMan.registerDefault("gui_renderer", ThemeEngine::findModeConfigName(ThemeEngine::_defaultRendererMode));
	ThemeEngine::GraphicsMode gfxMode = (ThemeEngine::GraphicsMode)ThemeEngine::findMode(ConfMan.get("gui_renderer"));

	// Try to load the theme
	if (!loadNewTheme(themefile, gfxMode)) {
		// Loading the theme failed, try to load the built-in theme
		if (!loadNewTheme("builtin", gfxMode)) {
			// Loading the built-in theme failed as well. Bail out
			error("Failed to load any GUI theme, aborting");
		}
	}
}

GuiManager::~GuiManager() {
	delete _theme;
	delete _wm;
}

void GuiManager::initIconsSet() {
	Common::StackLock lock(_iconsMutex);

	_iconsSet.clear();
#ifdef EMSCRIPTEN
	Common::Path iconsPath = ConfMan.getPath("iconspath");
	_iconsSet = Common::SearchSet();
	_iconsSet.addDirectory("gui-icons/", iconsPath, 0, 3, false);
	_iconsSetChanged = true;
#else
	_iconsSetChanged = Common::generateZipSet(_iconsSet, "gui-icons.dat", "gui-icons*.dat");
#endif
}

void GuiManager::computeScaleFactor() {
	uint16 w = g_system->getOverlayWidth();
	uint16 h = g_system->getOverlayHeight();

	_scaleFactor = g_system->getHiDPIScreenFactor();
	if (ConfMan.hasKey("gui_scale"))
		_scaleFactor *= ConfMan.getInt("gui_scale") / 100.f;

	_baseHeight = (int16)((float)h / _scaleFactor);
	_baseWidth = (int16)((float)w / _scaleFactor);

	// Never go below 320x200. Our GUI layout is not designed to go below that.
	// On the DS, this causes issues at 256x192 due to the use of non-scalable
	// BDF fonts.
#ifndef __DS__
	if (_baseHeight < 200) {
		_baseHeight = 200;
		_scaleFactor = (float)h / (float)_baseHeight;
		_baseWidth = (int16)((float)w / _scaleFactor);
	}
	if (_baseWidth < 320) {
		_baseWidth = 320;
		_scaleFactor = (float)w / (float)_baseWidth;
		_baseHeight = (int16)((float)h / _scaleFactor);
	}
#endif

	if (_theme)
		_theme->setBaseResolution(_baseWidth, _baseHeight, _scaleFactor);

	debug(1, "Setting %d x %d -> %d x %d -- %g", w, h, _baseWidth, _baseHeight, _scaleFactor);
}

Common::Keymap *GuiManager::getKeymap() const {
	using namespace Common;

	Keymap *guiMap = new Keymap(Keymap::kKeymapTypeGui, kGuiKeymapName, _("GUI"));

	Action *act;

	act = new Action(Common::kStandardActionInteract, _("Interact"));
	act->addDefaultInputMapping("JOY_A");
	act->setLeftClickEvent();
	guiMap->addAction(act);

	act = new Action("CLOS", _("Close"));
	act->addDefaultInputMapping("ESCAPE");
	act->addDefaultInputMapping("JOY_Y");
	act->setKeyEvent(KeyState(KEYCODE_ESCAPE, ASCII_ESCAPE, 0));
	guiMap->addAction(act);

	act = new Action(kStandardActionMoveUp, _("Up"));
	act->setKeyEvent(KEYCODE_UP);
	act->addDefaultInputMapping("JOY_UP");
	act->allowKbdRepeats();
	guiMap->addAction(act);

	act = new Action(kStandardActionMoveDown, _("Down"));
	act->setKeyEvent(KEYCODE_DOWN);
	act->addDefaultInputMapping("JOY_DOWN");
	act->allowKbdRepeats();
	guiMap->addAction(act);

	act = new Action(kStandardActionMoveLeft, _("Left"));
	act->setKeyEvent(KEYCODE_LEFT);
	act->addDefaultInputMapping("JOY_LEFT");
	act->allowKbdRepeats();
	guiMap->addAction(act);

	act = new Action(kStandardActionMoveRight, _("Right"));
	act->setKeyEvent(KEYCODE_RIGHT);
	act->addDefaultInputMapping("JOY_RIGHT");
	act->allowKbdRepeats();
	guiMap->addAction(act);

	act = new Action("BACKSPACE", _("Backspace"));
	act->setKeyEvent(KEYCODE_BACKSPACE);
	act->addDefaultInputMapping("BACKSPACE");
	act->allowKbdRepeats();
	guiMap->addAction(act);

	act = new Action("DEL", _("Delete Character"));
	act->setKeyEvent(KEYCODE_DELETE);
	act->addDefaultInputMapping("DELETE");
	act->allowKbdRepeats();
	guiMap->addAction(act);

	act = new Action("END", _("Go to end of line"));
#ifdef MACOSX
	act->setCustomEngineActionEvent(kActionEnd);
	act->addDefaultInputMapping("C+e");
#else
	act->setKeyEvent(KEYCODE_END);
	act->addDefaultInputMapping("END");
#endif
	guiMap->addAction(act);

	act = new Action("SHIFT_END", _("Select to end of line"));
#ifdef MACOSX
	act->setCustomEngineActionEvent(kActionShiftEnd);
	act->addDefaultInputMapping("C+S+e");
#else
	act->setKeyEvent(KeyState(KEYCODE_END, (uint16)KEYCODE_END, KBD_SHIFT));
	act->addDefaultInputMapping("S+END");
#endif
	guiMap->addAction(act);

	act = new Action("SHIFT_HOME", _("Select to start of line"));
#ifdef MACOSX
	act->setCustomEngineActionEvent(kActionShiftHome);
	act->addDefaultInputMapping("C+S+a");
#else
	act->setKeyEvent(KeyState(KEYCODE_HOME, (uint16)KEYCODE_HOME, KBD_SHIFT));
	act->addDefaultInputMapping("S+HOME");
#endif
	guiMap->addAction(act);

	act = new Action("HOME", _("Go to start of line"));
#ifdef MACOSX
	act->setCustomEngineActionEvent(kActionHome);
	act->addDefaultInputMapping("C+a");
#else
	act->setKeyEvent(KEYCODE_HOME);
	act->addDefaultInputMapping("HOME");
#endif
	guiMap->addAction(act);

#ifdef MACOSX
	act = new Action(kStandardActionCut, _("Cut"));
	act->setCustomEngineActionEvent(kActionCut);
	act->addDefaultInputMapping("M+x");
	guiMap->addAction(act);

	act = new Action(kStandardActionPaste, _("Paste"));
	act->setCustomEngineActionEvent(kActionPaste);
	act->addDefaultInputMapping("M+v");
	guiMap->addAction(act);

	act = new Action(kStandardActionCopy, _("Copy"));
	act->setCustomEngineActionEvent(kActionCopy);
	act->addDefaultInputMapping("M+c");
	guiMap->addAction(act);
#else
	act = new Action(kStandardActionCut, _("Cut"));
	act->setCustomEngineActionEvent(kActionCut);
	act->addDefaultInputMapping("C+x");
	guiMap->addAction(act);

	act = new Action(kStandardActionPaste, _("Paste"));
	act->setCustomEngineActionEvent(kActionPaste);
	act->addDefaultInputMapping("C+v");
	guiMap->addAction(act);

	act = new Action(kStandardActionCopy, _("Copy"));
	act->setCustomEngineActionEvent(kActionCopy);
	act->addDefaultInputMapping("C+c");
	guiMap->addAction(act);
#endif

	act = new Action(kStandardActionEE, _("???"));
	act->setKeyEvent(KEYCODE_v);
	guiMap->addAction(act);

	return guiMap;
}

void GuiManager::initKeymap() {
	using namespace Common;

	Keymapper *mapper = _system->getEventManager()->getKeymapper();

	// Do not try to recreate same keymap over again
	if (mapper->getKeymap(kGuiKeymapName) != 0)
		return;

	Keymap *guiMap = getKeymap();
	mapper->addGlobalKeymap(guiMap);
}

void GuiManager::enableKeymap(bool enabled) {
	Common::Keymapper *keymapper = _system->getEventManager()->getKeymapper();
	keymapper->setEnabledKeymapType(enabled ? Common::Keymap::kKeymapTypeGui : Common::Keymap::kKeymapTypeGame);
}

bool GuiManager::loadNewTheme(Common::String id, ThemeEngine::GraphicsMode gfx, bool forced) {
	// If we are asked to reload the currently active theme, just do nothing
	// FIXME: Actually, why? It might be desirable at times to force a theme reload...
	if (!forced)
		if (_theme && id == _theme->getThemeId() && gfx == _theme->getGraphicsMode())
			return true;

	ThemeEngine *newTheme = nullptr;

	if (gfx == ThemeEngine::kGfxDisabled)
		gfx = ThemeEngine::_defaultRendererMode;

	// Try to load the new theme
	newTheme = new ThemeEngine(id, gfx);
	assert(newTheme);
	newTheme->setBaseResolution(_baseWidth, _baseHeight, _scaleFactor);

	if (!newTheme->init()) {
		delete newTheme;
		return false;
	}

	//
	// Disable and delete the old theme
	//
	if (_theme)
		_theme->disable();
	delete _theme;

	if (_useStdCursor) {
		CursorMan.popCursorPalette();
		CursorMan.popCursor();
	}

	//
	// Enable the new theme
	//
	_theme = newTheme;
	_useStdCursor = !_theme->ownCursor();

	// If _stateIsSaved is set, we know that a Theme is already initialized,
	// thus we initialize the new theme properly
	if (_stateIsSaved) {
		_theme->enable();

		if (_useStdCursor)
			setupCursor();
	}

	// refresh all dialogs
	for (DialogStack::size_type i = 0; i < _dialogStack.size(); ++i)
		_dialogStack[i]->reflowLayout();

	// We need to redraw immediately. Otherwise
	// some other event may cause a widget to be
	// redrawn before redraw() has been called.
	redrawFull();

	return true;
}

void GuiManager::redrawFull() {
	_redrawStatus = kRedrawFull;
	redraw();
	_system->updateScreen();
}

void GuiManager::displayTopDialogOnly(bool mode) {
	if (mode == _displayTopDialogOnly)
		return;

	_displayTopDialogOnly = mode;

	redrawFull();
}

void GuiManager::redrawInternalTopDialogOnly() {
	// This is the simple case where only one dialog (the top one) is drawn on screen
	switch (_redrawStatus) {
		case kRedrawCloseDialog:
		case kRedrawFull:
		case kRedrawOpenDialog:
			// Clear everything
			_theme->clearAll();

			// fall through

		case kRedrawTopDialog:
			// Draw top dialog background on backbuffer
			_theme->drawToBackbuffer();
			_dialogStack.top()->drawDialog(kDrawLayerBackground);

			// Copy just drawn background to screen and draw foreground
			_theme->drawToScreen();
			_theme->copyBackBufferToScreen();

			_dialogStack.top()->drawDialog(kDrawLayerForeground);
			break;

		default:
			// Redraw only the widgets that are marked as dirty on screen
			_theme->drawToScreen();
			_dialogStack.top()->drawWidgets();
			break;
	}
}

void GuiManager::redrawInternal() {
	ThemeEngine::ShadingStyle shading;

	shading = (ThemeEngine::ShadingStyle)xmlEval()->getVar("Dialog." + _dialogStack.top()->_name + ".Shading", 0);

	switch (_redrawStatus) {
		case kRedrawCloseDialog:
		case kRedrawFull:
			// Clear everything
			_theme->clearAll();

			// Draw background and foreground of the whole dialog stack except top one on the backbuffer
			_theme->drawToBackbuffer();
			for (DialogStack::size_type i = 0; i < _dialogStack.size() - 1; i++) {
				_dialogStack[i]->drawDialog(kDrawLayerBackground);
				_dialogStack[i]->drawDialog(kDrawLayerForeground);
			}

			// fall through

		case kRedrawOpenDialog:
		case kRedrawTopDialog:
			// This case is an optimization to avoid redrawing the whole dialog
			// stack when opening a new dialog or redrawing the current one.

			_theme->drawToBackbuffer();
			if (_redrawStatus == kRedrawOpenDialog && _dialogStack.size() > 1) {
				// When opening a new dialog, merge the foreground of the last top dialog
				// inside the backbuffer
				// New top dialog foreground will be drawn on screen
				Dialog *previousDialog = _dialogStack[_dialogStack.size() - 2];
				previousDialog->drawDialog(kDrawLayerForeground);
			}

			// Do not shade when only redrawing the top dialog: shading has already been applied
			// Do not shade more than once when opening many dialogs on top of each other.
			// Shading being already applied previously, screen darkens
			if ((_redrawStatus != kRedrawTopDialog) &&
				((_redrawStatus != kRedrawOpenDialog) || (_dialogStack.size() <= 2))) {
				_theme->applyScreenShading(shading);
			}

			// Finally, draw the top dialog background
			_dialogStack.top()->drawDialog(kDrawLayerBackground);

			// copy everything to screen and render the top dialog foreground
			_theme->drawToScreen();
			_theme->copyBackBufferToScreen();

			_dialogStack.top()->drawDialog(kDrawLayerForeground);
			break;

		default:
			// Redraw only the widgets that are marked as dirty on screen
			_theme->drawToScreen();
			_dialogStack.top()->drawWidgets();
			break;
	}
}

void GuiManager::redraw() {
	if (_dialogStack.empty())
		return;

	// Reset any custom RTL paddings set by stacked dialogs when we go back to the top
	if (useRTL() && _dialogStack.size() == 1) {
		setDialogPaddings(0, 0);
	}

	if (_displayTopDialogOnly) {
		redrawInternalTopDialogOnly();
	} else {
		redrawInternal();
	}

	_theme->updateScreen();
	_redrawStatus = kRedrawDisabled;
}

Dialog *GuiManager::getTopDialog() const {
	if (_dialogStack.empty())
		return nullptr;
	return _dialogStack.top();
}

void GuiManager::addToTrash(GuiObject* object, Dialog *parent) {
	debug(7, "Adding Gui Object %p to trash", (void *)object);
	GuiObjectTrashItem t;
	t.object = object;
	t.parent = nullptr;
	// If a dialog was provided, check it is in the dialog stack
	if (parent != nullptr) {
		for (uint i = 0 ; i < _dialogStack.size() ; ++i) {
			if (_dialogStack[i] == parent) {
				t.parent = parent;
				break;
			}
		}
	}

	for (auto it = _guiObjectTrash.begin(); it != _guiObjectTrash.end(); ++it) {
		if (it->object == object) {
			debug(6, "The object %p was already scheduled for deletion, skipping", (void *)(*it).object);
			return;
		}
	}

	_guiObjectTrash.push_back(t);
}

void GuiManager::runLoop() {
	Dialog * const activeDialog = getTopDialog();
	bool didSaveState = false;

	if (activeDialog == nullptr)
		return;

#ifdef ENABLE_EVENTRECORDER
	// Suspend recording while GUI is shown
	g_eventRec.acquireRecording();
#endif

	if (!_stateIsSaved) {
		saveState();
		_theme->enable();
		didSaveState = true;

		_useStdCursor = !_theme->ownCursor();
		if (_useStdCursor)
			setupCursor();

//		_theme->refresh();

		_redrawStatus = kRedrawFull;
		redraw();
	}

	Common::EventManager *eventMan = _system->getEventManager();
	const uint32 targetFrameDuration = 1000 / 60;

	while (!_dialogStack.empty() && activeDialog == getTopDialog() && !eventMan->shouldQuit() && (!g_engine || !eventMan->shouldReturnToLauncher())) {
		uint32 frameStartTime = _system->getMillis(true);

		// Don't "tickle" the dialog until the theme has had a chance
		// to re-allocate buffers in case of a scaler change.

		activeDialog->handleTickle();

		if (_useStdCursor)
			animateCursor();

		Common::Event event;

		while (eventMan->pollEvent(event)) {
			// We will need to check whether the screen changed while polling
			// for an event here. While we do send EVENT_SCREEN_CHANGED
			// whenever this happens we still cannot be sure that we get such
			// an event immediately. For example, we might have a mouse move
			// event queued before a screen changed event. In some rare cases
			// this would make the GUI redraw (with the code a few lines
			// below) when it is not yet updated for new overlay dimensions.
			// As a result ScummVM would crash because it tries to copy data
			// outside the actual overlay screen.
			if (event.type != Common::EVENT_SCREEN_CHANGED) {
				checkScreenChange();
			}

			// The top dialog can change during the event loop. In that case, flush all the
			// dialog-related events since they were probably generated while the old dialog
			// was still visible, and therefore not intended for the new one.
			//
			// This hopefully fixes strange behavior/crashes with pop-up widgets. (Most easily
			// triggered in 3x mode or when running ScummVM under Valgrind.)
			if (activeDialog != getTopDialog() && event.type != Common::EVENT_SCREEN_CHANGED) {
				processEvent(event, getTopDialog());
				continue;
			}

			processEvent(event, activeDialog);
		}

		// If iconsSet was modified, notify dialogs so that they can be  updated if needed
		_iconsMutex.lock();
		bool iconsChanged = _iconsSetChanged;
		_iconsSetChanged = false;
		_iconsMutex.unlock();
		if (iconsChanged) {
			for (DialogStack::size_type i = 0; i < _dialogStack.size(); ++i) {
				setTarget(_dialogStack[i]);
				sendCommand(kIconsSetLoadedCmd, 0);
			}
		}

		// Delete GuiObject that have been added to the trash for a delayed deletion
		Common::List<GuiObjectTrashItem>::iterator it = _guiObjectTrash.begin();
		while (it != _guiObjectTrash.end()) {
			if ((*it).parent == nullptr || (*it).parent == activeDialog) {
				debug(7, "Delayed deletion of Gui Object %p", (void *)(*it).object);
				delete (*it).object;
				it = _guiObjectTrash.erase(it);
			} else
				++it;
		}

		// Handle tooltip for the widget under the mouse cursor.
		// 1. Only try to show a tooltip if the mouse cursor was actually moved
		//    and sufficient time (kTooltipDelay) passed since mouse cursor rested in-place.
		//    Note, Dialog objects acquiring or losing focus lead to a _lastMousePosition update,
		//    which may lead to a change of its time and x,y coordinate values.
		//    See: GuiManager::giveFocusToDialog()
		//    We avoid updating _lastMousePosition when giving focus to the Tooltip object
		//    by having the Tooltip objects set a false value for their (inherited) member
		//    var _mouseUpdatedOnFocus (in Tooltip::setup()).
		//    However, when the tooltip loses focus, _lastMousePosition will be updated.
		//    If the mouse had stayed in the same position in the meantime,
		//    then at the time of the tooltip losing focus
		//    the _lastMousePosition.time will be new, but the x,y cordinates
		//    will be the same as the stored ones in _lastTooltipShown.
		// 2. If the mouse was moved but ended on the same (tooltip enabled) widget,
		//    then delay showing the tooltip based on the value of kTooltipSameWidgetDelay.
		uint32 systemMillisNowForTooltipCheck = _system->getMillis(true);
		if ((_lastTooltipShown.x != _lastMousePosition.x || _lastTooltipShown.y != _lastMousePosition.y)
		    && systemMillisNowForTooltipCheck - _lastMousePosition.time > (uint32)kTooltipDelay
		    && !activeDialog->isDragging()) {
			Widget *wdg = activeDialog->findWidget(_lastMousePosition.x, _lastMousePosition.y);
			if (wdg && (wdg->hasTooltip() || (wdg->getFlags() & WIDGET_DYN_TOOLTIP)) && !(wdg->getFlags() & WIDGET_PRESSED)
			    && (_lastTooltipShown.wdg != wdg || systemMillisNowForTooltipCheck - _lastTooltipShown.time > (uint32)kTooltipSameWidgetDelay)) {
				_lastTooltipShown.time = systemMillisNowForTooltipCheck;
				_lastTooltipShown.wdg  = wdg;
				_lastTooltipShown.x = _lastMousePosition.x;
				_lastTooltipShown.y = _lastMousePosition.y;
				if (wdg->getType() != kEditTextWidget || activeDialog->getFocusWidget() != wdg) {
					if (wdg->getFlags() & WIDGET_DYN_TOOLTIP)
						wdg->handleTooltipUpdate(_lastMousePosition.x + activeDialog->_x - wdg->getAbsX(), _lastMousePosition.y + activeDialog->_y - wdg->getAbsY());

					if (wdg->hasTooltip()) {
						Tooltip *tooltip = new Tooltip();
						tooltip->setup(activeDialog, wdg, _lastMousePosition.x, _lastMousePosition.y);
						tooltip->runModal();
						delete tooltip;
					}
				}
			}
		}

		redraw();

		// Delay until the allocated frame time is elapsed to match the target frame rate.
		// In case we have vsync enabled, we should rely on vsync to do take care about frame times.
		// With vsync enabled, we currently have to force a frame time of 1ms since otherwise
		// CPU usage will skyrocket on one thread as soon as no updateScreen(); calls happening.
		if (g_system->getFeatureState(OSystem::kFeatureVSync)) {
			_system->delayMillis(1);
		} else {
			uint32 actualFrameDuration = _system->getMillis(true) - frameStartTime;
			if (actualFrameDuration < targetFrameDuration) {
				_system->delayMillis(targetFrameDuration - actualFrameDuration);
			}
		}
		_system->updateScreen();
	}

	// WORKAROUND: When quitting we might not properly close the dialogs on
	// the dialog stack, thus we do this here to avoid any problems.
	// This is most noticeable in bug #5954 "LAUNCHER: Can't quit from unsupported game dialog".
	// It seems that Dialog::runModal never removes the dialog from the dialog
	// stack, thus if the dialog does not call Dialog::close to close itself
	// it will never be removed. Since we can have multiple run loops being
	// called we cannot rely on catching EVENT_QUIT in the event loop above,
	// since it would only catch it for the top run loop.
	if ((eventMan->shouldQuit() || (g_engine && eventMan->shouldReturnToLauncher())) && activeDialog == getTopDialog())
		getTopDialog()->close();

	if (didSaveState) {
		_theme->disable();
		restoreState();
		_useStdCursor = false;
	}

#ifdef ENABLE_EVENTRECORDER
	// Resume recording once GUI is shown
	g_eventRec.releaseRecording();
#endif
}

void GuiManager::exitLoop() {
	while (!_dialogStack.empty())
		getTopDialog()->close();
}

#pragma mark -

void GuiManager::saveState() {
	initKeymap();
	enableKeymap(true);

	// Backup old cursor
	_lastClick.x = _lastClick.y = 0;
	_lastClick.time = 0;
	_lastClick.count = 0;

	_stateIsSaved = true;
}

void GuiManager::restoreState() {
	enableKeymap(false);

	if (_useStdCursor) {
		CursorMan.popCursor();
		CursorMan.popCursorPalette();
	}

	_system->updateScreen();

	_stateIsSaved = false;
}

void GuiManager::openDialog(Dialog *dialog) {
	giveFocusToDialog(dialog);

	if (!_dialogStack.empty())
		getTopDialog()->lostFocus();

	_dialogStack.push(dialog);
	// We were already ready to redraw a new dialog
	// Redraw fully to ensure a proper draw of the whole stack
	if (_redrawStatus == kRedrawOpenDialog)
		_redrawStatus = kRedrawFull;
	if (_redrawStatus != kRedrawFull)
		_redrawStatus = kRedrawOpenDialog;

	// We reflow the dialog just before opening it. If the screen changed
	// since the last time we looked, also refresh the loaded theme,
	// and reflow all other open dialogs, too.
	if (!checkScreenChange())
		dialog->reflowLayout();
}

void GuiManager::closeTopDialog() {
	// Don't do anything if no dialog is open
	if (_dialogStack.empty())
		return;

	// Remove the dialog from the stack
	_dialogStack.pop()->lostFocus();

	if (!_dialogStack.empty()) {
		Dialog *dialog = getTopDialog();
		giveFocusToDialog(dialog);
	}

	if (_redrawStatus != kRedrawFull)
		_redrawStatus = kRedrawCloseDialog;

	redraw();
}

void GuiManager::setupCursor() {
	const byte palette[] = {
		255, 255, 255,
		255, 255, 255,
		171, 171, 171,
		 87,  87,  87
	};

	CursorMan.pushCursorPalette(palette, 0, 4);
	CursorMan.pushCursor(nullptr, 0, 0, 0, 0, 0);
	CursorMan.showMouse(true);
}

// Draw the mouse cursor (animated). This is pretty much the same as in old
// SCUMM games, but the code no longer resembles what we have in cursor.cpp
// very much. We could plug in a different cursor here if we like to.

void GuiManager::animateCursor() {
	uint32 time = _system->getMillis(true);
	if (time - _cursorAnimateTimer > (uint32)kCursorAnimateDelay) {
		for (int i = 0; i < 15; i++) {
			if ((i < 6) || (i > 8)) {
				_cursor[16 * 7 + i] = _cursorAnimateCounter;
				_cursor[16 * i + 7] = _cursorAnimateCounter;
			}
		}

		CursorMan.replaceCursor(_cursor, 16, 16, 7, 7, 255);

		_cursorAnimateTimer = time;
		_cursorAnimateCounter = (_cursorAnimateCounter + 1) % 4;
	}
}

bool GuiManager::checkScreenChange() {
	int tmpScreenChangeID = _system->getScreenChangeID();
	if (_lastScreenChangeID != tmpScreenChangeID) {
		screenChange();
		return true;
	}
	return false;
}

void GuiManager::screenChange() {
#ifdef ENABLE_EVENTRECORDER
	// Suspend recording while GUI is redrawn.
	// We need this in addition to the lock in runLoop, as EVENT_SCREEN_CHANGED can
	// be fired by in-game GUI components (such as the event recorder itself)
	g_eventRec.acquireRecording();
#endif

	_lastScreenChangeID = _system->getScreenChangeID();

	computeScaleFactor();

	// reinit the whole theme
	_theme->refresh();

	// refresh all dialogs
	for (DialogStack::size_type i = 0; i < _dialogStack.size(); ++i) {
		_dialogStack[i]->reflowLayout();
	}
	// We need to redraw immediately. Otherwise
	// some other event may cause a widget to be
	// redrawn before redraw() has been called.
	redrawFull();

#ifdef ENABLE_EVENTRECORDER
	// Resume recording once GUI has redrawn
	g_eventRec.releaseRecording();
#endif
}

void GuiManager::processEvent(const Common::Event &event, Dialog *const activeDialog) {
	if (activeDialog == nullptr)
		return;
	int button;
	uint32 time;
	Common::Point mouse(event.mouse.x - activeDialog->_x, event.mouse.y - activeDialog->_y);
	if (g_gui.useRTL()) {
		mouse.x = g_system->getOverlayWidth() - event.mouse.x - activeDialog->_x + g_gui.getOverlayOffset();
	}
	switch (event.type) {
	case Common::EVENT_KEYDOWN:
		activeDialog->handleKeyDown(event.kbd);
		break;
	case Common::EVENT_KEYUP:
		activeDialog->handleKeyUp(event.kbd);
		break;
	case Common::EVENT_MOUSEMOVE:
		if (g_gui.useRTL()) {
			_globalMousePosition.x = g_system->getOverlayWidth() - event.mouse.x + g_gui.getOverlayOffset();
		} else {
			_globalMousePosition.x = event.mouse.x;
		}
		_globalMousePosition.y = event.mouse.y;
		activeDialog->handleMouseMoved(mouse.x, mouse.y, 0);

		if (mouse.x != _lastMousePosition.x || mouse.y != _lastMousePosition.y) {
			setLastMousePos(mouse.x, mouse.y);
		}

		break;
		// We don't distinguish between mousebuttons (for now at least)
	case Common::EVENT_LBUTTONDOWN:
	case Common::EVENT_RBUTTONDOWN:
		button = (event.type == Common::EVENT_LBUTTONDOWN ? 1 : 2);
		time = _system->getMillis(true);
		if (_lastClick.count && (time - _lastClick.time < (uint32)kDoubleClickDelay)
		    && ABS(_lastClick.x - event.mouse.x) < 3
		    && ABS(_lastClick.y - event.mouse.y) < 3) {
			_lastClick.count++;
		} else {
			_lastClick.x = event.mouse.x;
			_lastClick.y = event.mouse.y;
			_lastClick.count = 1;
		}
		_lastClick.time = time;
		activeDialog->handleMouseDown(mouse.x, mouse.y, button, _lastClick.count);
		break;
	case Common::EVENT_LBUTTONUP:
	case Common::EVENT_RBUTTONUP:
		button = (event.type == Common::EVENT_LBUTTONUP ? 1 : 2);
		activeDialog->handleMouseUp(mouse.x, mouse.y, button, _lastClick.count);
		break;
	case Common::EVENT_WHEELUP:
		activeDialog->handleMouseWheel(mouse.x, mouse.y, -1);
		break;
	case Common::EVENT_WHEELDOWN:
		activeDialog->handleMouseWheel(mouse.x, mouse.y, 1);
		break;
	case Common::EVENT_SCREEN_CHANGED:
		screenChange();
		break;
	default:
		activeDialog->handleOtherEvent(event);
		break;
	}
}

void GuiManager::scheduleTopDialogRedraw() {
	// Open/Close dialog redraws have higher priority, otherwise they may not be processed at all
	// Full redraw also has higher priority
	if (_redrawStatus != kRedrawOpenDialog &&
		_redrawStatus != kRedrawCloseDialog &&
		_redrawStatus != kRedrawFull)
		_redrawStatus = kRedrawTopDialog;
}

void GuiManager::scheduleFullRedraw() {
	_redrawStatus = kRedrawFull;
}

void GuiManager::giveFocusToDialog(Dialog *dialog) {
	int16 dialogX = _globalMousePosition.x - dialog->_x;
	int16 dialogY = _globalMousePosition.y - dialog->_y;
	dialog->receivedFocus(dialogX, dialogY);
	if (dialog->isMouseUpdatedOnFocus()) {
		setLastMousePos(dialogX, dialogY);
	}
}

void GuiManager::setLastMousePos(int16 x, int16 y) {
	_lastMousePosition.x = x;
	_lastMousePosition.y = y;
	_lastMousePosition.time = _system->getMillis(true);
}

void GuiManager::setLanguageRTL() {
	if (ConfMan.hasKey("guiRTL")) {		// Put guiRTL = yes to your scummvm.ini to force RTL GUI
		_useRTL = ConfMan.getBool("guiRTL");
		return;
	}
#ifdef USE_TRANSLATION
	Common::String language = TransMan.getCurrentLanguage();
	if (language.equals("he") || language.equals("ar")) {		// GUI TODO: modify when we'll support other RTL languages, such as Arabic and Farsi
		_useRTL = true;
		return;
	}
#endif // USE_TRANSLATION

	_useRTL = false;
}

void GuiManager::setDialogPaddings(int l, int r) {
	_topDialogLeftPadding = l;
	_topDialogRightPadding = r;
}

void GuiManager::initTextToSpeech() {
	Common::TextToSpeechManager *ttsMan = g_system->getTextToSpeechManager();
	if (ttsMan == nullptr)
		return;
	ttsMan->enable(ConfMan.hasKey("tts_enabled", "scummvm") ? ConfMan.getBool("tts_enabled", "scummvm") : false);
#ifdef USE_TRANSLATION
	Common::String currentLanguage = TransMan.getCurrentLanguage();
	ttsMan->setLanguage(currentLanguage);
#endif
	int volume = (ConfMan.getInt("speech_volume", "scummvm") * 100) / 256;
	if (ConfMan.hasKey("mute", "scummvm") && ConfMan.getBool("mute", "scummvm"))
		volume = 0;
	ttsMan->setVolume(volume);

	unsigned voice;
	if(ConfMan.hasKey("tts_voice")) {
		voice = ConfMan.getInt("tts_voice", "scummvm");
		if (voice >= ttsMan->getVoicesArray().size())
			voice = ttsMan->getDefaultVoice();
	} else
		voice = ttsMan->getDefaultVoice();
	ttsMan->setVoice(voice);
}

Graphics::MacWindowManager *GuiManager::getWM() {
	if (_wm)
		return _wm;

	if (ConfMan.hasKey("extrapath")) {
		Common::FSNode dir(ConfMan.getPath("extrapath"));
		SearchMan.addDirectory(dir);
	}

	uint32 wmMode = Graphics::kWMModeNoDesktop | Graphics::kWMMode32bpp | Graphics::kWMModeNoCursorOverride;

	_wm = new Graphics::MacWindowManager(wmMode);

	return _wm;
}

} // End of namespace GUI
