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

#include "common/events.h"
#include "common/system.h"
#include "common/util.h"
#include "common/config-manager.h"
#include "common/algorithm.h"
#include "common/rect.h"
#include "common/textconsole.h"
#include "common/translation.h"
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

namespace Common {
DECLARE_SINGLETON(GUI::GuiManager);
}

namespace GUI {

enum {
	kDoubleClickDelay = 500, // milliseconds
	kCursorAnimateDelay = 250,
	kTooltipDelay = 1250
};

// Constructor
GuiManager::GuiManager() : _redrawStatus(kRedrawDisabled), _stateIsSaved(false),
    _cursorAnimateCounter(0), _cursorAnimateTimer(0) {
	_theme = nullptr;
	_useStdCursor = false;

	_system = g_system;
	_lastScreenChangeID = _system->getScreenChangeID();
	_width = _system->getOverlayWidth();
	_height = _system->getOverlayHeight();

	_launched = false;

	_useRTL = false;

	_topDialogLeftPadding = 0;
	_topDialogRightPadding = 0;

	// Clear the cursor
	memset(_cursor, 0xFF, sizeof(_cursor));

#ifdef USE_TRANSLATION
	// Enable translation
	TransMan.setLanguage(ConfMan.get("gui_language").c_str());
	setLanguageRTL();
#endif // USE_TRANSLATION

#ifdef USE_TTS
	initTextToSpeech();
#endif // USE_TTS

	ConfMan.registerDefault("gui_theme", "scummremastered");
	Common::String themefile(ConfMan.get("gui_theme"));

	ConfMan.registerDefault("gui_renderer", ThemeEngine::findModeConfigName(ThemeEngine::_defaultRendererMode));
	ThemeEngine::GraphicsMode gfxMode = (ThemeEngine::GraphicsMode)ThemeEngine::findMode(ConfMan.get("gui_renderer"));

#ifdef __DS__
	// Searching for the theme file takes ~10 seconds on the DS.
	// Disable this search here because external themes are not supported.
	if (!loadNewTheme("builtin", gfxMode)) {
		// Loading the built-in theme failed as well. Bail out
		error("Failed to load any GUI theme, aborting");
	}
#else
	// Try to load the theme
	if (!loadNewTheme(themefile, gfxMode)) {
		// Loading the theme failed, try to load the built-in theme
		if (!loadNewTheme("builtin", gfxMode)) {
			// Loading the built-in theme failed as well. Bail out
			error("Failed to load any GUI theme, aborting");
		}
	}
#endif
}

GuiManager::~GuiManager() {
	delete _theme;
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
	guiMap->addAction(act);

	act = new Action(kStandardActionMoveDown, _("Down"));
	act->setKeyEvent(KEYCODE_DOWN);
	act->addDefaultInputMapping("JOY_DOWN");
	guiMap->addAction(act);

	act = new Action(kStandardActionMoveLeft, _("Left"));
	act->setKeyEvent(KEYCODE_LEFT);
	act->addDefaultInputMapping("JOY_LEFT");
	guiMap->addAction(act);

	act = new Action(kStandardActionMoveRight, _("Right"));
	act->setKeyEvent(KEYCODE_RIGHT);
	act->addDefaultInputMapping("JOY_RIGHT");
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

	if (!newTheme->init())
		return false;

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
	_redrawStatus = kRedrawFull;
	redraw();
	_system->updateScreen();

	return true;
}

void GuiManager::redrawFull() {
	_redrawStatus = kRedrawFull;
	redraw();
	_system->updateScreen();
}

void GuiManager::redraw() {
	ThemeEngine::ShadingStyle shading;

	if (_dialogStack.empty())
		return;

	shading = (ThemeEngine::ShadingStyle)xmlEval()->getVar("Dialog." + _dialogStack.top()->_name + ".Shading", 0);

	// Tanoku: Do not apply shading more than once when opening many dialogs
	// on top of each other. Screen ends up being too dark and it's a
	// performance hog.
	if (_redrawStatus == kRedrawOpenDialog && _dialogStack.size() > 3)
		shading = ThemeEngine::kShadingNone;

	switch (_redrawStatus) {
		case kRedrawCloseDialog:
		case kRedrawFull:
		case kRedrawTopDialog:
			_theme->clearAll();
			_theme->drawToBackbuffer();

			for (DialogStack::size_type i = 0; i < _dialogStack.size() - 1; i++) {
				_dialogStack[i]->drawDialog(kDrawLayerBackground);
				_dialogStack[i]->drawDialog(kDrawLayerForeground);
			}

			// fall through

		case kRedrawOpenDialog:
			// This case is an optimization to avoid redrawing the whole dialog
			// stack when opening a new dialog.

			_theme->drawToBackbuffer();

			if (_redrawStatus == kRedrawOpenDialog && _dialogStack.size() > 1) {
				Dialog *previousDialog = _dialogStack[_dialogStack.size() - 2];
				previousDialog->drawDialog(kDrawLayerForeground);
			}

			_theme->applyScreenShading(shading);
			_dialogStack.top()->drawDialog(kDrawLayerBackground);

			_theme->drawToScreen();
			_theme->copyBackBufferToScreen();

			_dialogStack.top()->drawDialog(kDrawLayerForeground);
			break;

		default:
			break;
	}

	// Redraw the widgets that are marked as dirty
	_theme->drawToScreen();
	_dialogStack.top()->drawWidgets();

	_theme->updateScreen();
	_redrawStatus = kRedrawDisabled;
}

Dialog *GuiManager::getTopDialog() const {
	if (_dialogStack.empty())
		return nullptr;
	return _dialogStack.top();
}

void GuiManager::addToTrash(GuiObject* object, Dialog* parent) {
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
	_guiObjectTrash.push_back(t);
}

void GuiManager::runLoop() {
	Dialog * const activeDialog = getTopDialog();
	bool didSaveState = false;

	if (activeDialog == nullptr)
		return;

#ifdef ENABLE_EVENTRECORDER
	// Suspend recording while GUI is shown
	g_eventRec.suspendRecording();
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

	while (!_dialogStack.empty() && activeDialog == getTopDialog() && !eventMan->shouldQuit()) {
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
			// an event immediately. For example, we might have an mouse move
			// event queued before an screen changed event. In some rare cases
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

		if (_lastMousePosition.time + kTooltipDelay < _system->getMillis(true)) {
			Widget *wdg = activeDialog->findWidget(_lastMousePosition.x, _lastMousePosition.y);
			if (wdg && wdg->hasTooltip() && !(wdg->getFlags() & WIDGET_PRESSED)) {
				Tooltip *tooltip = new Tooltip();
				tooltip->setup(activeDialog, wdg, _lastMousePosition.x, _lastMousePosition.y);
				tooltip->runModal();
				delete tooltip;
			}
		}

		redraw();

		// Delay until the allocated frame time is elapsed to match the target frame rate
		uint32 actualFrameDuration = _system->getMillis(true) - frameStartTime;
		if (actualFrameDuration < targetFrameDuration) {
			_system->delayMillis(targetFrameDuration - actualFrameDuration);
		}
		_system->updateScreen();
	}

	// WORKAROUND: When quitting we might not properly close the dialogs on
	// the dialog stack, thus we do this here to avoid any problems.
	// This is most noticable in bug #3481395 "LAUNCHER: Can't quit from unsupported game dialog".
	// It seems that Dialog::runModal never removes the dialog from the dialog
	// stack, thus if the dialog does not call Dialog::close to close itself
	// it will never be removed. Since we can have multiple run loops being
	// called we cannot rely on catching EVENT_QUIT in the event loop above,
	// since it would only catch it for the top run loop.
	if (eventMan->shouldQuit() && activeDialog == getTopDialog())
		getTopDialog()->close();

	if (didSaveState) {
		_theme->disable();
		restoreState();
		_useStdCursor = false;
	}

#ifdef ENABLE_EVENTRECORDER
	// Resume recording once GUI is shown
	g_eventRec.resumeRecording();
#endif
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
	int time = _system->getMillis(true);
	if (time > _cursorAnimateTimer + kCursorAnimateDelay) {
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
	_lastScreenChangeID = _system->getScreenChangeID();
	_width = _system->getOverlayWidth();
	_height = _system->getOverlayHeight();

	// reinit the whole theme
	_theme->refresh();

	// refresh all dialogs
	for (DialogStack::size_type i = 0; i < _dialogStack.size(); ++i) {
		_dialogStack[i]->reflowLayout();
	}
	// We need to redraw immediately. Otherwise
	// some other event may cause a widget to be
	// redrawn before redraw() has been called.
	_redrawStatus = kRedrawFull;
	redraw();
	_system->updateScreen();
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
		if (_lastClick.count && (time < _lastClick.time + kDoubleClickDelay)
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
	_redrawStatus = kRedrawTopDialog;
}

void GuiManager::giveFocusToDialog(Dialog *dialog) {
	int16 dialogX = _globalMousePosition.x - dialog->_x;
	int16 dialogY = _globalMousePosition.y - dialog->_y;
	dialog->receivedFocus(dialogX, dialogY);
	setLastMousePos(dialogX, dialogY);
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
	if (language.equals("he")) {		// GUI TODO: modify when we'll support other RTL languages, such as Arabic and Farsi
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

#ifdef USE_TTS
void GuiManager::initTextToSpeech() {
	Common::TextToSpeechManager *ttsMan = g_system->getTextToSpeechManager();
	if (ttsMan == nullptr)
		return;
#ifdef USE_TRANSLATION
	Common::String currentLanguage = TransMan.getCurrentLanguage();
	if (currentLanguage == "C")
		currentLanguage = "en";
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
#endif

} // End of namespace GUI
