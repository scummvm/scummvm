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

#include "common/scummsys.h"
#include "mads/game.h"
#include "mads/mads.h"
#include "mads/menu_views.h"
#include "mads/resources.h"
#include "mads/scene.h"
#include "mads/screen.h"

namespace MADS {

MenuView::MenuView(MADSEngine *vm) : FullScreenDialog(vm) {
	_breakFlag = false;
	_redrawFlag = true;
	_palFlag = false;
}

void MenuView::show() {
 	Scene &scene = _vm->_game->_scene;
	EventsManager &events = *_vm->_events;
	_vm->_screenFade = SCREEN_FADE_FAST;

	scene._spriteSlots.reset(true);
	display();

	events.setEventTarget(this);
	events.hideCursor();

	while (!_breakFlag && !_vm->shouldQuit()) {
		if (_redrawFlag) {
			scene._kernelMessages.update();

			_vm->_game->_scene.drawElements(_vm->_game->_fx, _vm->_game->_fx);
			_redrawFlag = false;
		}

		_vm->_events->waitForNextFrame();
		_vm->_game->_fx = kTransitionNone;
		doFrame();
	}

	events.setEventTarget(nullptr);
	_vm->_sound->stop();
}

void MenuView::display() {
	_vm->_palette->resetGamePalette(4, 8);

	FullScreenDialog::display();
}

bool MenuView::onEvent(Common::Event &event) {
	if (event.type == Common::EVENT_KEYDOWN || event.type == Common::EVENT_LBUTTONDOWN) {
		_breakFlag = true;
		_vm->_dialogs->_pendingDialog = DIALOG_MAIN_MENU;
		return true;
	}

	return false;
}

Common::String MenuView::getResourceName() {
	Common::String s(_filename);
	s.toLowercase();
	while (s.contains('.'))
		s.deleteLastChar();

	return s;
}

/*------------------------------------------------------------------------*/

char TextView::_resourceName[100];
#define TEXTVIEW_LINE_SPACING 2
#define TEXT_ANIMATION_DELAY 100
#define TV_NUM_FADE_STEPS 40
#define TV_FADE_DELAY_MILLI 50

void TextView::execute(MADSEngine *vm, const Common::String &resName) {
	assert(resName.size() < 100);
	Common::strlcpy(_resourceName, resName.c_str(), sizeof(_resourceName));
	vm->_dialogs->_pendingDialog = DIALOG_TEXTVIEW;
}

TextView::TextView(MADSEngine *vm) : MenuView(vm) {
	_animating = false;
	_panSpeed = 0;
	_spareScreen = nullptr;
	_scrollCount = 0;
	_lineY = -1;
	_scrollTimeout = 0;
	_panCountdown = 0;
	_translationX = 0;
	_screenId = -1;

	_font = _vm->_font->getFont(FONT_CONVERSATION);
	_vm->_palette->resetGamePalette(4, 0);

	load();
}

TextView::~TextView() {
	// Turn off palette cycling as well as any playing sound
	Scene &scene = _vm->_game->_scene;
	scene._cyclingActive = false;
	_vm->_sound->stop();
}

void TextView::load() {
	Common::String scriptName(_resourceName);
	scriptName += ".txr";

	_filename = scriptName;
	if (!_script.open(scriptName))
		error("Could not open resource %s", _resourceName);

	processLines();
}

void TextView::processLines() {
	if (_script.eos())
		error("Attempted to read past end of response file");

	while (!_script.eos()) {
		// Read in the next line
		_script.readLine(_currentLine, 79);
		char *p = _currentLine + strlen(_currentLine) - 1;
		if (*p == '\n')
			*p = '\0';

		// Commented out line, so go loop for another
		if (_currentLine[0] == '#')
			continue;

		// Process the line
		char *cStart = strchr(_currentLine, '[');
		if (cStart) {
			while (cStart) {
				// Loop for possible multiple commands on one line
				char *cEnd = strchr(_currentLine, ']');
				if (!cEnd)
					error("Unterminated command '%s' in response file", _currentLine);

				*cEnd = '\0';
				processCommand();

				// Copy rest of line (if any) to start of buffer
				Common::strlcpy(_currentLine, cEnd + 1, sizeof(_currentLine));

				cStart = strchr(_currentLine, '[');
			}

			if (_currentLine[0]) {
				processText();
				break;
			}

		} else {
			processText();
			break;
		}
	}
}

void TextView::processCommand() {
	Scene &scene = _vm->_game->_scene;
	Common::String scriptLine(_currentLine + 1);
	scriptLine.toUppercase();
	const char *paramP;
	const char *commandStr = scriptLine.c_str();

	if (!strncmp(commandStr, "BACKGROUND", 10)) {
		// Set the background
		paramP = commandStr + 10;
		resetPalette();
		int screenId = getParameter(&paramP);

		SceneInfo *sceneInfo = SceneInfo::init(_vm);
		sceneInfo->load(screenId, 0, "", 0, scene._depthSurface, scene._backgroundSurface);
		scene._spriteSlots.fullRefresh();
		_redrawFlag = true;

	} else if (!strncmp(commandStr, "GO", 2)) {
		_animating = true;

	} else if (!strncmp(commandStr, "PAN", 3)) {
		// Set panning values
		paramP = commandStr + 3;
		int panX = getParameter(&paramP);
		int panY = getParameter(&paramP);
		int panSpeed = getParameter(&paramP);

		if ((panX != 0) || (panY != 0)) {
			_pan = Common::Point(panX, panY);
			_panSpeed = panSpeed;
		}

	} else if (!strncmp(commandStr, "DRIVER", 6)) {
		// Set the driver to use
		paramP = commandStr + 7;

		if (!strncmp(paramP, "#SOUND.00", 9)) {
			int driverNum = paramP[9] - '0';
			_vm->_sound->init(driverNum);
		}
	} else if (!strncmp(commandStr, "SOUND", 5)) {
		// Set sound number
		paramP = commandStr + 5;
		int soundId = getParameter(&paramP);
		_vm->_sound->command(soundId);

	} else if (!strncmp(commandStr, "COLOR", 5) && ((commandStr[5] == '0') ||
			(commandStr[5] == '1'))) {
		// Set the text colors
		int index = commandStr[5] - '0';
		paramP = commandStr + 6;

		byte r = getParameter(&paramP);
		byte g = getParameter(&paramP);
		byte b = getParameter(&paramP);

		_vm->_palette->setEntry(5 + index, r, g, b);

	} else if (!strncmp(commandStr, "SPARE", 5)) {
		// Sets a secondary background number that can be later switched in with a PAGE command
		paramP = commandStr + 6;
		int spareIndex = commandStr[5] - '0';
		assert(spareIndex < 4);
		int screenId = getParameter(&paramP);

		// Load the spare background
		SceneInfo *sceneInfo = SceneInfo::init(_vm);
		sceneInfo->_width = MADS_SCREEN_WIDTH;
		sceneInfo->_height = MADS_SCENE_HEIGHT;
		_spareScreens[spareIndex].create(MADS_SCREEN_WIDTH, MADS_SCENE_HEIGHT);

		sceneInfo->loadMadsV1Background(screenId, "", SCENEFLAG_TRANSLATE,
			_spareScreens[spareIndex]);
		delete sceneInfo;

	} else if (!strncmp(commandStr, "PAGE", 4)) {
		// Signals to change to a previous specified secondary background
		paramP = commandStr + 4;
		int spareIndex = getParameter(&paramP);

		// Only allow background switches if one isn't currently in progress
		if (!_spareScreen && _spareScreens[spareIndex].getPixels() != nullptr) {
			_spareScreen = &_spareScreens[spareIndex];
			_translationX = 0;
		}

	} else {
		error("Unknown response command: '%s'", commandStr);
	}
}

int TextView::getParameter(const char **paramP) {
	if ((**paramP != '=') && (**paramP != ','))
		return 0;

	int result = 0;
	++*paramP;
	while ((**paramP >= '0') && (**paramP <= '9')) {
		result = result * 10 + (**paramP - '0');
		++*paramP;
	}

	return result;
}

void TextView::processText() {
	int xStart;

	if (!strcmp(_currentLine, "***")) {
		// Special signifier for end of script
		_scrollCount = _font->getHeight() * 13;
		_lineY = -1;
		return;
	}

	_lineY = 0;

	// Lines are always centered, except if line contains a '@', in which case the
	// '@' marks the position that must be horizontally centered
	char *centerP = strchr(_currentLine, '@');
	if (centerP) {
		*centerP = '\0';
		xStart = (MADS_SCREEN_WIDTH / 2) - _font->getWidth(_currentLine);

		// Delete the @ character and shift back the remainder of the string
		char *p = centerP + 1;
		if (*p == ' ') ++p;
		strcpy(centerP, p);

	} else {
		int lineWidth = _font->getWidth(_currentLine);
		xStart = (MADS_SCREEN_WIDTH - lineWidth) / 2;
	}

	// Add the new line to the list of pending lines
	TextLine tl;
	tl._pos = Common::Point(xStart, MADS_SCENE_HEIGHT);
	tl._line = _currentLine;
	tl._textDisplayIndex = -1;
	_textLines.push_back(tl);
}

void TextView::display() {
	FullScreenDialog::display();
}

void TextView::resetPalette() {
	_vm->_palette->resetGamePalette(8, 8);
	_vm->_palette->setEntry(5, 0, 63, 63);
	_vm->_palette->setEntry(6, 0, 45, 45);
}

void TextView::doFrame() {
	Scene &scene = _vm->_game->_scene;
	if (!_animating)
		return;

	// Only update state if wait period has expired
	uint32 currTime = g_system->getMillis();

	// If a screen transition is in progress and it's time for another column, handle it
	if (_spareScreen) {
		const byte *srcP = (const byte *)_spareScreen->getBasePtr(_translationX, 0);
		byte *bgP = (byte *)scene._backgroundSurface.getBasePtr(_translationX, 0);

		Graphics::Surface dest = _vm->_screen->getSubArea(Common::Rect(_translationX, 0, _translationX + 1, 0));
		byte *screenP = (byte *)dest.getBasePtr(0, 0);

		for (int y = 0; y < MADS_SCENE_HEIGHT; ++y, srcP += MADS_SCREEN_WIDTH,
			bgP += MADS_SCREEN_WIDTH, screenP += MADS_SCREEN_WIDTH) {
			*bgP = *srcP;
			*screenP = *srcP;
		}

		// Keep moving the column to copy to the right
		if (++_translationX == MADS_SCREEN_WIDTH) {
			// Surface transition is complete
			_spareScreen = nullptr;
		}
	}

	// Make sure it's time for an update
	if (currTime < _scrollTimeout)
		return;
	_scrollTimeout = g_system->getMillis() + TEXT_ANIMATION_DELAY;
	_redrawFlag = true;

	// If any panning values are set, pan the background surface
	if ((_pan.x != 0) || (_pan.y != 0)) {
		if (_panCountdown > 0) {
			--_panCountdown;
			return;
		}

		// Handle horizontal panning
		if (_pan.x != 0) {
			byte *lineTemp = new byte[_pan.x];
			for (int y = 0; y < MADS_SCENE_HEIGHT; ++y) {
				byte *pixelsP = (byte *)scene._backgroundSurface.getBasePtr(0, y);

				// Copy the first X pixels into temp buffer, move the rest of the line
				// to the start of the line, and then move temp buffer pixels to end of line
				Common::copy(pixelsP, pixelsP + _pan.x, lineTemp);
				Common::copy(pixelsP + _pan.x, pixelsP + MADS_SCREEN_WIDTH, pixelsP);
				Common::copy(lineTemp, lineTemp + _pan.x, pixelsP + MADS_SCREEN_WIDTH - _pan.x);
			}

			delete[] lineTemp;
		}

		// Handle vertical panning
		if (_pan.y != 0) {
			// Store the bottom Y lines into a temp buffer, move the rest of the lines down,
			// and then copy the stored lines back to the top of the screen
			byte *linesTemp = new byte[_pan.y * MADS_SCREEN_WIDTH];
			byte *pixelsP = (byte *)scene._backgroundSurface.getBasePtr(0, MADS_SCENE_HEIGHT - _pan.y);
			Common::copy(pixelsP, pixelsP + MADS_SCREEN_WIDTH * _pan.y, linesTemp);

			for (int y = MADS_SCENE_HEIGHT - 1; y >= _pan.y; --y) {
				byte *destP = (byte *)scene._backgroundSurface.getBasePtr(0, y);
				byte *srcP = (byte *)scene._backgroundSurface.getBasePtr(0, y - _pan.y);
				Common::copy(srcP, srcP + MADS_SCREEN_WIDTH, destP);
			}

			Common::copy(linesTemp, linesTemp + _pan.y * MADS_SCREEN_WIDTH,
				(byte *)scene._backgroundSurface.getPixels());
			delete[] linesTemp;
		}

		// Flag for a full screen refresh
		scene._spriteSlots.fullRefresh();
	}

	// Scroll all active text lines up
	for (int i = _textLines.size() - 1; i >= 0; --i) {
		TextLine &tl = _textLines[i];
		if (tl._textDisplayIndex != -1)
			// Expire the text line that's already on-screen
			scene._textDisplay.expire(tl._textDisplayIndex);

		tl._pos.y--;
		if (tl._pos.y + _font->getHeight() < 0) {
			_textLines.remove_at(i);
		} else {
			tl._textDisplayIndex = scene._textDisplay.add(tl._pos.x, tl._pos.y,
				0x605, -1, tl._line, _font);
		}
	}

	if (_scrollCount > 0) {
		// Handling final scrolling of text off of screen
		if (--_scrollCount == 0) {
			scriptDone();
			return;
		}
	} else {
		// Handling a text row
		if (++_lineY == (_font->getHeight() + TEXTVIEW_LINE_SPACING))
			processLines();
	}
}

void TextView::scriptDone() {
	_breakFlag = true;
	_vm->_dialogs->_pendingDialog = DIALOG_MAIN_MENU;
}

/*------------------------------------------------------------------------*/

char AnimationView::_resourceName[100];

void AnimationView::execute(MADSEngine *vm, const Common::String &resName) {
	assert(resName.size() < 100);
	Common::strlcpy(_resourceName, resName.c_str(), sizeof(_resourceName));
	vm->_dialogs->_pendingDialog = DIALOG_ANIMVIEW;
}

AnimationView::AnimationView(MADSEngine *vm) : MenuView(vm) {
	_redrawFlag = false;

	_soundDriverLoaded = false;
	_previousUpdate = 0;
	_screenId = -1;
	_resetPalette = false;
	_resyncMode = NEVER;
	_v1 = 0;
	_v2 = -1;
	_resourceIndex = -1;
	_currentAnimation = nullptr;
	_sfx = 0;
	_soundFlag = _bgLoadFlag = true;
	_showWhiteBars = true;
	_manualFrameNumber = 0;
	_manualSpriteSet = nullptr;
	_manualStartFrame = _manualEndFrame = 0;
	_manualFrame2 = 0;
	_animFrameNumber = 0;
	_nextCyclingActive = false;
	_sceneInfo = SceneInfo::init(_vm);
	_scrollFrameCtr = 0;

	load();
}

AnimationView::~AnimationView() {
	// Turn off palette cycling as well as any playing sound
	Scene &scene = _vm->_game->_scene;
	scene._cyclingActive = false;
	_vm->_sound->stop();
	_vm->_audio->stop();

	// Delete data
	delete _currentAnimation;
	delete _sceneInfo;
}

void AnimationView::load() {
	Common::String resName(_resourceName);
	if (!resName.hasSuffix("."))
		resName += ".res";

	_filename = resName;
	if (!_script.open(resName))
		error("Could not open resource %s", resName.c_str());

	processLines();
}

void AnimationView::display() {
	Scene &scene = _vm->_game->_scene;
	_vm->_palette->initPalette();
	Common::fill(&_vm->_palette->_cyclingPalette[0], &_vm->_palette->_cyclingPalette[PALETTE_SIZE], 0);

	_vm->_palette->resetGamePalette(1, 8);
	scene._spriteSlots.reset();
	scene._paletteCycles.clear();

	MenuView::display();
}

bool AnimationView::onEvent(Common::Event &event) {
	// Wait for the Escape key or a mouse press
	if (((event.type == Common::EVENT_KEYDOWN) && (event.kbd.keycode == Common::KEYCODE_ESCAPE)) ||
			(event.type == Common::EVENT_LBUTTONUP)) {
		scriptDone();
		return true;
	}

	return false;
}

void AnimationView::doFrame() {
	Scene &scene = _vm->_game->_scene;

	if (_resourceIndex == -1 || _currentAnimation->freeFlag()) {
		if (++_resourceIndex == (int)_resources.size()) {
			scriptDone();
		} else {
			scene._frameStartTime = 0;
			scene._spriteSlots.clear();
			loadNextResource();
		}
	} else if (_currentAnimation->getCurrentFrame() == 1) {
		scene._cyclingActive = _nextCyclingActive;
	}

	if (_currentAnimation && (++_scrollFrameCtr >= _currentAnimation->_header._scrollTicks)) {
		_scrollFrameCtr = 0;
		scroll();
	}

	if (_currentAnimation) {
		++scene._frameStartTime;
		_currentAnimation->update();
		_redrawFlag = true;

		if (_currentAnimation->freeFlag())
			// We don't want the sprites removed after the last animation frame
			scene._spriteSlots.clear();
	}
}

void AnimationView::loadNextResource() {
	Scene &scene = _vm->_game->_scene;
	Palette &palette = *_vm->_palette;
	Screen &screen = *_vm->_screen;
	ResourceEntry &resEntry = _resources[_resourceIndex];
	Common::Array<PaletteCycle> paletteCycles;

	if (resEntry._bgFlag)
		palette.resetGamePalette(1, 8);

	palette._mainPalette[253 * 3] = palette._mainPalette[253 * 3 + 1]
		= palette._mainPalette[253 * 3 + 2] = 0xb4;
	palette.setPalette(&palette._mainPalette[253 * 3], 253, 1);

	// Free any previous messages
	scene._kernelMessages.reset();

	// Handle the bars at the top/bottom
	if (resEntry._showWhiteBars) {
		// For animations the screen has been clipped to the middle 156 rows.
		// So although it's slightly messy, temporarily reset clip bounds
		// so we can redraw the white lines
		Common::Rect clipBounds = screen.getClipBounds();
		screen.resetClipBounds();

		screen.hLine(0, 20, MADS_SCREEN_WIDTH, 253);
		screen.hLine(0, 179, MADS_SCREEN_WIDTH, 253);

		screen.setClipBounds(clipBounds);
	}

	// Load the new animation
	delete _currentAnimation;
	_currentAnimation = Animation::init(_vm, &scene);
	int flags = ANIMFLAG_ANIMVIEW | (resEntry._bgFlag ? ANIMFLAG_LOAD_BACKGROUND : 0);
	_currentAnimation->load(scene._backgroundSurface, scene._depthSurface,
		resEntry._resourceName, flags, &paletteCycles, _sceneInfo);

	// Signal for a screen refresh
	scene._spriteSlots.fullRefresh();

	// If a sound driver has been specified, then load the correct one
	if (!_currentAnimation->_header._soundName.empty()) {
		const char *chP = strchr(_currentAnimation->_header._soundName.c_str(), '.');
		assert(chP);

		// Handle both Rex naming (xxx.009) and naming in later games (e.g. xxx.ph9)
		int driverNum = atoi(chP + 3);
		// HACK for Dragon
		if (_currentAnimation->_header._soundName == "#SOUND.DRG")
			driverNum = 9;
		_vm->_sound->init(driverNum);
	}

	// Handle any manual setup
	if (_currentAnimation->_header._manualFlag) {
		_manualFrameNumber = _currentAnimation->_header._spritesIndex;
		_manualSpriteSet = _currentAnimation->getSpriteSet(_manualFrameNumber);
	}

	// Set the sound data for the animation
	_vm->_sound->setEnabled(resEntry._soundFlag);

	Common::String dsrName = _currentAnimation->_header._dsrName;
	if (!dsrName.empty())
		_vm->_audio->setSoundGroup(dsrName);

	// Start the new animation
	_currentAnimation->startAnimation(0);

	// Handle the palette and cycling palette
	scene._cyclingActive = false;
	Common::copy(&palette._mainPalette[0], &palette._mainPalette[PALETTE_SIZE],
		&palette._cyclingPalette[0]);

	_vm->_game->_fx = (ScreenTransition)resEntry._fx;
	_nextCyclingActive = paletteCycles.size() > 0;
	if (!_vm->_game->_fx) {
		palette.setFullPalette(palette._mainPalette);
	}

	scene.initPaletteAnimation(paletteCycles, _nextCyclingActive && !_vm->_game->_fx);
}

void AnimationView::scroll() {
	Scene &scene = _vm->_game->_scene;
	Common::Point pt = _currentAnimation->_header._scrollPosition;

	if (pt.x != 0) {
		scene._backgroundSurface.scrollX(pt.x);
		scene._spriteSlots.fullRefresh();
	}

	if (pt.y != 0) {
		scene._backgroundSurface.scrollY(pt.y);
		scene._spriteSlots.fullRefresh();
	}
}

void AnimationView::scriptDone() {
	_breakFlag = true;
	_vm->_dialogs->_pendingDialog = DIALOG_MAIN_MENU;
}

void AnimationView::processLines() {
	if (_script.eos()) {
		// end of script, end animation
		scriptDone();
		return;
	}

	while (!_script.eos()) {
		// Get in next line
		_currentLine.clear();
		char c;
		while (!_script.eos() && (c = _script.readByte()) != '\n') {
			if (c != '\r' && c != '\0')
				_currentLine += c;
		}

		// Check for comment line
		if (_currentLine.hasPrefix("#"))
			continue;

		// Process the line
		while (!_currentLine.empty()) {
			if (_currentLine.hasPrefix("-")) {
				_currentLine.deleteChar(0);

				processCommand();
			} else {
				// Get resource name
				Common::String resName;
				while (!_currentLine.empty() && (c = _currentLine[0]) != ' ') {
					_currentLine.deleteChar(0);
					resName += c;
				}

				// Add resource into list along with any set state information
				_resources.push_back(ResourceEntry(resName, _sfx, _soundFlag,
					_bgLoadFlag, _showWhiteBars));

				// Fx resets between resource entries
				_sfx = 0;
			}

			// Skip any spaces
			while (_currentLine.hasPrefix(" "))
				_currentLine.deleteChar(0);
		}
	}
}

void AnimationView::processCommand() {
	// Get the command character
	char commandChar = toupper(_currentLine[0]);
	_currentLine.deleteChar(0);

	// Handle the command
	switch (commandChar) {
	case 'B':
		_soundFlag = !_soundFlag;
		break;
	case 'H':
		// -h[:ex]  Disable EMS / XMS high memory support
		if (_currentLine.hasPrefix(":"))
			_currentLine.deleteChar(0);
		while (_currentLine.hasPrefix("e") || _currentLine.hasPrefix("x"))
			_currentLine.deleteChar(0);
		break;
	case 'O':
		// -o:xxx  Specify opening special effect
		assert(_currentLine[0] == ':');
		_currentLine.deleteChar(0);
		_sfx = getParameter();
		break;
	case 'P':
		// Switch to CONCAT mode, which is ignored anyway
		break;
	case 'R': {
		// Resynch timer (always, beginning, never)
		assert(_currentLine[0] == ':');
		_currentLine.deleteChar(0);

		char v = toupper(_currentLine[0]);
		_currentLine.deleteChar(0);
		if (v == 'N')
			_resyncMode = NEVER;
		else if (v == 'A')
			_resyncMode = ALWAYS;
		else if (v == 'B')
			_resyncMode = BEGINNING;
		else
			error("Unknown parameter");
		break;
	}
	case 'W':
		// Switch white bars being visible
		_showWhiteBars = !_showWhiteBars;
		break;
	case 'X':
		// Exit after animation finishes. Ignore
		break;
	case 'D':
		// Unimplemented and ignored in the original. Ignore as well
		break;
	case 'Y':
		// Reset palette on startup
		_resetPalette = true;
		break;
	default:
		error("Unknown command char: '%c'", commandChar);
	}
}

int AnimationView::getParameter() {
	int result = 0;

	while (!_currentLine.empty()) {
		char c = _currentLine[0];

		if (c >= '0' && c <= '9') {
			_currentLine.deleteChar(0);
			result = result * 10 + (c - '0');
		} else {
			break;
		}
	}

	return result;
}

void AnimationView::checkResource(const Common::String &resourceName) {
	//bool hasSuffix = false;

}

int AnimationView::scanResourceIndex(const Common::String &resourceName) {
	int foundIndex = -1;

	if (_v1) {
		const char *chP = strchr(resourceName.c_str(), '\\');
		if (!chP) {
			chP = strchr(resourceName.c_str(), '*');
		}

		Common::String resName = chP ? Common::String(chP + 1) : resourceName;

		if (_v2 != 3) {
			assert(_resIndex.size() == 0);
		}

		// Scan for the resource name
		for (uint resIndex = 0; resIndex < _resIndex.size(); ++resIndex) {
			ResIndexEntry &resEntry = _resIndex[resIndex];
			if (resEntry._resourceName.compareToIgnoreCase(resourceName)) {
				foundIndex = resIndex;
				break;
			}
		}
	}

	if (foundIndex >= 0) {
		// TODO
	}
	return -1;
}

} // End of namespace MADS
