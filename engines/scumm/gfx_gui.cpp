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

#include "scumm/scumm.h"
#include "scumm/scumm_v8.h"
#include "scumm/gfx.h"
#include "scumm/dialogs.h"
#include "scumm/charset.h"
#include "scumm/string_v7.h"
#include "scumm/smush/smush_player.h"

#include "graphics/cursorman.h"

namespace Scumm {

void ScummEngine::initBanners() {
	setPalColor(7, 0x5A, 0x5A, 0x5A);
	setPalColor(8, 0x46, 0x46, 0x46);
	setPalColor(15, 0x8C, 0x8C, 0x8C);
	updatePalette();

	setBannerColors(1, 0x00, 0x00, 0x00);
	setBannerColors(2, 0xA8, 0xA8, 0xA8);
	setBannerColors(3, 0x00, 0x00, 0x00);
	setBannerColors(4, 0xFC, 0xFC, 0x54);
	setBannerColors(5, 0x54, 0x54, 0xFC);
	setBannerColors(6, 0xA8, 0xA8, 0xA8);
	setBannerColors(7, 0x00, 0x00, 0x00);
	setBannerColors(8, 0xFC, 0xFC, 0x54);
	setBannerColors(9, 0x54, 0x54, 0xFC);
	setBannerColors(10, 0xFC, 0xFC, 0xFC);
	setBannerColors(11, 0x54, 0x54, 0x54);
	setBannerColors(12, 0xFC, 0xFC, 0xFC);
	setBannerColors(13, 0x54, 0x54, 0x54);
	setBannerColors(14, 0x00, 0x00, 0x00);
	setBannerColors(15, 0xA8, 0xA8, 0xA8);
	setBannerColors(16, 0xFC, 0xFC, 0xFC);
	setBannerColors(17, 0x54, 0x54, 0x54);
	setBannerColors(18, 0xFC, 0xFC, 0xFC);
	setBannerColors(19, 0x54, 0x54, 0x54);
	setBannerColors(20, 0xFC, 0x00, 0x00);
	setBannerColors(21, 0xA8, 0xA8, 0xA8);
	setBannerColors(22, 0xFC, 0xFC, 0xFC);
	setBannerColors(23, 0x54, 0x54, 0x54);
	setBannerColors(24, 0xFC, 0xFC, 0xFC);
	setBannerColors(25, 0x54, 0x54, 0x54);
	setBannerColors(26, 0x00, 0x00, 0x00);
	setBannerColors(27, 0xA8, 0xA8, 0xA8);
	setBannerColors(28, 0xFC, 0xFC, 0xFC);
	setBannerColors(29, 0x54, 0x54, 0x54);
	setBannerColors(30, 0xFC, 0xFC, 0xFC);
	setBannerColors(31, 0x54, 0x54, 0x54);
}

Common::KeyState ScummEngine::showBannerAndPause(int bannerId, int32 waitTime, const char *msg, ...) {
	char bannerMsg[512];
	char localizedMsg[512];
	char localizedY[512];
	char *ptrToBreak;
	int bannerMsgWidth, bannerMsgHeight, roundedWidth;
	int startingPointX, startingPointY;
	int bannerSaveYStart;
	int xPos, yPos;
	int rightLineColor, leftLineColor, bottomLineColor, topLineColor;
	int normalTextColor, normalFillColor;

	// Fetch the translated string for the message...
	convertMessageToString((const byte *)msg, (byte *)localizedMsg, sizeof(localizedMsg));
	ptrToBreak = strstr(localizedMsg, "\\n");
	if (!(_game.features & GF_DEMO) && ptrToBreak) { // Change the line break, if any...
		ptrToBreak[0] = '\n';
		ptrToBreak[1] = '\r';
	}

	// Format the string with the arguments...
	va_list va;
	va_start(va, msg);
	vsnprintf(bannerMsg, sizeof(bannerMsg), localizedMsg, va);
	va_end(va);

	// Fetch the localized confirmation letter and substitute it with the 'y' of 'yes'
	if (_game.version == 8) {
		convertMessageToString((const byte *)getGUIString(gsYesKey), (byte *)localizedY, sizeof(localizedY));
	}

	// Pause the engine
	PauseToken pt = pauseEngine();

	// Gather the colors needed for the banner
	int palOffset = (_game.version == 8) ? 0 : 11;
	normalFillColor = getBannerColor(6 * bannerId + 15 + palOffset);
	normalTextColor = getBannerColor(6 * bannerId + 14 + palOffset);
	topLineColor = getBannerColor(6 * bannerId + 16 + palOffset);
	bottomLineColor = getBannerColor(6 * bannerId + 17 + palOffset);
	leftLineColor = getBannerColor(6 * bannerId + 18 + palOffset);
	rightLineColor = getBannerColor(6 * bannerId + 19 + palOffset);

	// Backup the current charsetId, since we're going to switch
	// to charsetId == 1...
	int oldId = _charset->getCurID();
	_charset->setCurID(1);

	// Take all the necessary measurements for the box which
	// will contain the string...
	bool isCOMIDemo = (_game.id == GID_CMI && (_game.features & GF_DEMO) != 0);
	bannerMsgHeight = (isCOMIDemo ? _textV7->getStringHeight("ABC \x80\x78 \xb0\x78) : _textV7->getStringHeight(bannerMsg)) + 5;
	bannerMsgWidth = _textV7->getStringWidth(bannerMsg);
	if (bannerMsgWidth < 100)
		bannerMsgWidth = 100;

	roundedWidth = (((bannerMsgWidth + 15) & 0xFFF0) + 8) / 2;
	if (_game.version < 7) {
		roundedWidth = bannerMsgWidth / 2;
	}

	if (_game.version >= 7 && _game.id != GID_FT) {
		startingPointX = _screenWidth / 2 - roundedWidth - 4;
		startingPointY = _screenHeight / 2 - 10;
		xPos = _screenWidth / 2 + roundedWidth + 3;
		yPos = 1 - bannerMsgHeight;
		bannerSaveYStart = startingPointY;
	} else if (_game.id == GID_MONKEY && _game.platform == Common::kPlatformFMTowns) {
		bannerMsgWidth = getGUIStringWidth(bannerMsg) / 2;
		startingPointX = ((160 - bannerMsgWidth) - 8) & 0xFFF8;
		startingPointY = (bannerMsgWidth + 168) | 0x7;
		xPos = 1; // Bogus value, since it is unused
		yPos = 1; // Bogus value, since it is unused
		bannerSaveYStart = 78;
	} else {
		startingPointX = 156 - roundedWidth;
		startingPointY = ((_game.version < 7) ? 80 : _screenTop + 90);
		xPos = roundedWidth + 163 + ((_game.version < 7) ? 1 : 0);
		yPos = -12;
		bannerSaveYStart = startingPointY - ((_game.version < 7) ? 2 : 0);
	}

	// Save the pixels which will be overwritten by the banner,
	// so that we can restore them later...
	if (!_bannerMem) {
		int rowSize = _screenWidth + 8;
		_bannerMemSize = bannerMsgHeight * (_screenWidth + 8);
		_bannerMem = (byte *)malloc(_bannerMemSize * sizeof(byte));
		if (_bannerMem) {
			// FM-Towns games draw the banner on the text surface, so let's save that
#ifndef DISABLE_TOWNS_DUAL_LAYER_MODE
			if (_game.platform == Common::kPlatformFMTowns && !_textSurfBannerMem) {
				rowSize *= _textSurfaceMultiplier;
				bannerSaveYStart *= _textSurfaceMultiplier;
				_textSurfBannerMemSize = bannerMsgHeight * rowSize * _textSurfaceMultiplier;
				_textSurfBannerMem = (byte *)malloc(_textSurfBannerMemSize * sizeof(byte));
				if (_textSurfBannerMem) {
					memcpy(
						_textSurfBannerMem,
						&((byte *)_textSurface.getBasePtr(0, _screenTop * _textSurfaceMultiplier))[rowSize * bannerSaveYStart],
						_textSurfBannerMemSize);
				}

				// We're going to use these same values for saving the
				// virtual screen surface, so let's un-multiply them...
				rowSize /= _textSurfaceMultiplier;
				bannerSaveYStart /= _textSurfaceMultiplier;
			}
#endif

			memcpy(
				_bannerMem,
				&_virtscr[kMainVirtScreen].getPixels(0, _screenTop)[rowSize * bannerSaveYStart],
				_bannerMemSize);
		}
	}

	// Set up the GUI control, specifying all the related colors, the message and the position...
	setUpInternalGUIControl(0, normalFillColor, normalTextColor,
							topLineColor, bottomLineColor, leftLineColor, rightLineColor, 0, 0,
							startingPointX, startingPointY, xPos, yPos,
							bannerMsg, true);

	// Draw it!
	drawInternalGUIControl(0, 0);
	ScummEngine::drawDirtyScreenParts();

	// Wait until the engine receives a new Keyboard or Mouse input,
	// unless we have specified a positive waitTime: in that case, the banner
	// will stay on screen until an input has been received or until the time-out.
	Common::KeyState ks = Common::KEYCODE_INVALID;
	bool leftBtnPressed = false, rightBtnPressed = false;
	if (waitTime) {
		waitForBannerInput(waitTime, ks, leftBtnPressed, rightBtnPressed);
		clearBanner();
	}

	// Finally, resume the engine, clear the input state, and restore the charset.
	pt.clear();
	clearClickedStatus();
	if (oldId)
		_charset->setCurID(oldId);

	if (_game.version == 8) {
		if (tolower(localizedY[0]) == ks.ascii || toupper(localizedY[0]) == ks.ascii)
			ks = Common::KEYCODE_y;
	}

	return ks;
}

void ScummEngine::clearBanner() {
	// Restore the GFX content which was under the banner,
	// and then mark that part of the screen as dirty.
	if (_bannerMem) {
		int rowSize = _screenWidth + 8;
		// Don't manually clear the banner if a SMUSH movie is playing,
		// as that will cause some rare small glitches. The SMUSH player
		// will take care of that for us automatically when updating the
		// screen for next frame.
		if (!isSmushActive()) {
			int startingPointY;
			if (_game.version >= 7 && _game.id != GID_FT) {
				startingPointY = _screenHeight / 2 - 10;
			} else if (_game.id == GID_MONKEY && _game.platform == Common::kPlatformFMTowns) {
				startingPointY = 78;
			} else {
				startingPointY = ((_game.version < 7) ? 80 - 2 : _screenTop + 90);
			}

			// FM-Towns games draw the banners on the text surface, so restore both surfaces...
#ifndef DISABLE_TOWNS_DUAL_LAYER_MODE
			if (_game.platform == Common::kPlatformFMTowns && _textSurfBannerMem) {
				startingPointY *= _textSurfaceMultiplier;
				rowSize *= _textSurfaceMultiplier;
				memcpy(
					&((byte *)_textSurface.getBasePtr(0, _screenTop * _textSurfaceMultiplier))[rowSize * startingPointY],
					_textSurfBannerMem,
					_textSurfBannerMemSize);

				// We're going to use these same values for restoring the
				// virtual screen surface, so let's un-multiply them...
				rowSize /= _textSurfaceMultiplier;
				startingPointY /= _textSurfaceMultiplier;
			}
#endif
			memcpy(
				&_virtscr[kMainVirtScreen].getPixels(0, _screenTop)[rowSize * startingPointY],
				_bannerMem,
				_bannerMemSize);

			markRectAsDirty(_virtscr[kMainVirtScreen].number, 0, rowSize, _screenTop, _screenHeight + _screenTop);
			ScummEngine::drawDirtyScreenParts();
			_system->updateScreen();
		}

		free(_bannerMem);
		_bannerMem = nullptr;

		free(_textSurfBannerMem);
		_textSurfBannerMem = nullptr;
	}
}

void ScummEngine::setBannerColors(int bannerId, byte r, byte g, byte b) {
	if (bannerId < 0 || bannerId > 50) {
		debug(1, "ScummEngine::setBannerColors(): invalid slot %d out of range (min %d, max %d)", bannerId, 0, 50);
		return;
	}

	_bannerColors[bannerId] = r | (b << 16) | (g << 8);
}

void ScummEngine::setUpInternalGUIControl(int id, int normalFillColor, int normalTextColor,
										  int topLineColor, int bottomLineColor, int leftLineColor, int rightLineColor,
										  int highlightedTextColor, int highlightedFillColor,
										  int anchorPointX, int anchorPointY, int x, int y, char *label, bool centerFlag) {

	int effX, effY;
	InternalGUIControl *ctrl;

	effX = x;
	ctrl = &_internalGUIControls[id];
	if (x < 0)
		effX = anchorPointX - x;
	effY = y;
	if (y < 0)
		effY = anchorPointY - y;
	ctrl->relativeCenterX = anchorPointX;
	ctrl->relativeCenterY = anchorPointY;
	ctrl->xPos = effX;
	ctrl->yPos = effY;
	ctrl->label = label;
	ctrl->centerText = centerFlag;
	ctrl->normalFillColor = normalFillColor;
	ctrl->topLineColor = topLineColor;
	ctrl->bottomLineColor = bottomLineColor;
	ctrl->leftLineColor = leftLineColor;
	ctrl->rightLineColor = rightLineColor;
	ctrl->normalTextColor = normalTextColor;
	ctrl->highlightedTextColor = highlightedTextColor;
	ctrl->highlightedFillColor = highlightedFillColor;
}

void ScummEngine::drawInternalGUIControl(int id, bool highlightColor) {
	InternalGUIControl *ctrl;
	int relCentX, relCentY, textHeight;
	int x, y, textXPos, textYPos;
	int textColor, fillColor;
	int boxSizeX, boxSizeY;
	int offset = (_game.version >= 7 && _game.id != GID_FT) ? 2 : 1;

	bool centerFlag;
	char buttonString[512];

	ctrl = &_internalGUIControls[id];
	relCentX = ctrl->relativeCenterX;
	if (ctrl->relativeCenterX != -1) {
		relCentY = ctrl->relativeCenterY;
		x = ctrl->xPos;
		y = ctrl->yPos;

		boxSizeX = x - ((_game.version >= 7 && _game.id != GID_FT) ? ctrl->relativeCenterX : 0);
		boxSizeY = y - ((_game.version >= 7 && _game.id != GID_FT) ? relCentY : 0);

		fillColor = highlightColor ? ctrl->highlightedFillColor : ctrl->normalFillColor;

		if (_game.id == GID_MONKEY && _game.platform == Common::kPlatformFMTowns) {
			// MI1 for FMTowns does its own thing with hardcoded values here...
			drawBox(relCentX + 1, 81, relCentY - 1, 90, fillColor);

			drawLine(relCentX + 1, 80, relCentY - 1, 80, ctrl->topLineColor);
			drawLine(relCentX + 1, 91, relCentY - 1, 91, ctrl->bottomLineColor);
			drawLine(relCentX, 81, relCentX, 90, ctrl->leftLineColor);
			drawLine(relCentY, 81, relCentY, 90, ctrl->rightLineColor);
		} else {
			// Draw the main box...
			drawBox(relCentX + 1, relCentY + 1, boxSizeX - offset, boxSizeY - offset, fillColor);

			// Draw the contour lines for the box; each of the lines is doubled to give a 3D effect.
			drawLine(relCentX + 1, relCentY, x - 1, relCentY, ctrl->topLineColor);
			drawLine(relCentX + 1, y, x - 1, y, ctrl->bottomLineColor);
			drawLine(relCentX, relCentY + 1, relCentX, y - 1, ctrl->leftLineColor);
			drawLine(x, relCentY + 1, x, y - 1, ctrl->rightLineColor);

			drawLine(relCentX + 1, relCentY + 1, x - 1, relCentY + 1, ctrl->topLineColor);
			drawLine(relCentX + 1, y - 1, x - 1, y - 1, ctrl->bottomLineColor);
			drawLine(relCentX + 1, relCentY + 1, relCentX + 1, y - 1, ctrl->leftLineColor);
			drawLine(x - 1, relCentY + 1, x - 1, y - 1, ctrl->rightLineColor);
		}

		// Calculate the positioning for the text
		int oldId = _charset->getCurID();
		_charset->setCurID(1);

		centerFlag = ctrl->centerText;
		if (_game.id == GID_MONKEY && _game.platform == Common::kPlatformFMTowns) {
			// Again, MI1 for FMTowns hardcodes the values...
			textXPos = 160;
			textYPos = 82;
		} else {
			textHeight = getGUIStringHeight(buttonString);

			if (centerFlag)
				textXPos = relCentX + (x - ctrl->relativeCenterX) / 2;
			else
				textXPos = relCentX + 2;

			textYPos = relCentY + ((y - relCentY) - textHeight) / 2 + 1;
		}

		// Finally, choose the color and draw the text message
		if (highlightColor)
			textColor = ctrl->highlightedTextColor;
		else
			textColor = ctrl->normalTextColor;

		if (ctrl->label)
			strcpy(buttonString, ctrl->label);
		else
			strcpy(buttonString, "null button");

		drawGUIText(buttonString, textXPos, textYPos, textColor, centerFlag);

		// Restore the previous charset
		if (oldId)
			_charset->setCurID(oldId);
	}
}

int ScummEngine::getInternalGUIControlFromCoordinates(int x, int y) {
	int id = 0;
	while (_internalGUIControls[id].relativeCenterX == -1 ||
		   _internalGUIControls[id].relativeCenterX > x ||
		   _internalGUIControls[id].xPos < x ||
		   _internalGUIControls[id].relativeCenterY > y ||
		   _internalGUIControls[id].yPos < y) {

		id++;
		if (id >= ARRAYSIZE(_internalGUIControls))
			return -1;
	}
	return id;
}


#ifdef ENABLE_SCUMM_7_8
void ScummEngine_v7::queryQuit() {
	if (isUsingOriginalGUI()) {
		if (_game.version == 8 && !(_game.features & GF_DEMO)) {
			int boxWidth, strWidth;
			int ctrlId;
			char yesLabelPtr[512];
			char noLabelPtr[512];
			char msgLabelPtr[512];
			byte *curGrabbedCursor;
			int curCursorWidth, curCursorHeight, curCursorHotspotX, curCursorHotspotY, curCursorState;

			// Force the cursor to be ON...
			int8 oldCursorState = _cursor.state;
			_cursor.state = 1;
			CursorMan.showMouse(_cursor.state > 0);

			// "Are you sure you want to quit?"
			convertMessageToString((const byte *)getGUIString(gsQuitPrompt), (byte *)msgLabelPtr, sizeof(msgLabelPtr));
			// "Yes"
			convertMessageToString((const byte *)getGUIString(gsYes), (byte *)yesLabelPtr, sizeof(yesLabelPtr));
			// "No"
			convertMessageToString((const byte *)getGUIString(gsNo), (byte *)noLabelPtr, sizeof(noLabelPtr));

			// Pause the engine...
			PauseToken pt = pauseEngine();

			// Backup the current cursor graphics and parameters
			// and set up the internal v8 cursor...
			curGrabbedCursor = (byte *)malloc(sizeof(_grabbedCursor));
			memcpy(curGrabbedCursor, _grabbedCursor, sizeof(_grabbedCursor));
			curCursorState = isSmushActive() ? 0 : _cursor.state;
			curCursorWidth = _cursor.width;
			curCursorHeight = _cursor.height;
			curCursorHotspotX = _cursor.hotspotX;
			curCursorHotspotY = _cursor.hotspotY;
			setDefaultCursor();
			CursorMan.showMouse(true);

			// Backup the current charsetId, since we're going to switch
			// to charsetId == 1 and start taking measurements for the box...
			int oldId = _charset->getCurID();
			_charset->setCurID(1);

			boxWidth = ((getGUIStringWidth(msgLabelPtr) + 32) & 0xFFF0) + 8;
			if (boxWidth <= 400)
				boxWidth = 400;

			// Set up and draw the main box
			setUpInternalGUIControl(
				0,
				getBannerColor(33),
				getBannerColor(32),
				getBannerColor(34),
				getBannerColor(35),
				getBannerColor(36),
				getBannerColor(37),
				0,
				0,
				320 - boxWidth / 2,
				190,
				boxWidth / 2 + 319,
				-90,
				_emptyMsg,
				true);

			// Save the pixels which will be overwritten by the dialog,
			// so that we can restore them later. Note that the interpreter
			// doesn't do this, but we have to...
			if (!_bannerMem && !isSmushActive()) {
				_bannerMemSize = _screenWidth * _screenHeight;
				_bannerMem = (byte *)malloc(_bannerMemSize * sizeof(byte));
				if (_bannerMem)
					memcpy(
						_bannerMem,
						_virtscr[kMainVirtScreen].getPixels(0, _screenTop),
						_bannerMemSize);
			}

			drawInternalGUIControl(0, 0);

			// The text is drawn as a separate entity
			drawTextImmediately((const byte *)msgLabelPtr, 320, 200, getBannerColor(32), 1, (TextStyleFlags) true);

			// Now set up and draw the Yes and No buttons...
			if (getGUIStringWidth(noLabelPtr) <= getGUIStringWidth(yesLabelPtr)) {
				strWidth = getGUIStringWidth(yesLabelPtr);
			} else {
				strWidth = getGUIStringWidth(noLabelPtr);
			}

			if (strWidth <= 120)
				strWidth = 120;

			setUpInternalGUIControl(
				0,
				getBannerColor(45),
				getBannerColor(44),
				getBannerColor(46),
				getBannerColor(47),
				getBannerColor(48),
				getBannerColor(49),
				0,
				0,
				420 - (strWidth / 2),
				240, -strWidth,
				-30,
				noLabelPtr,
				true);

			drawInternalGUIControl(0, 0);

			setUpInternalGUIControl(
				0,
				getBannerColor(39),
				getBannerColor(38),
				getBannerColor(40),
				getBannerColor(41),
				getBannerColor(42),
				getBannerColor(43),
				0,
				0,
				220 - (strWidth / 2),
				240,
				-strWidth,
				-30,
				yesLabelPtr,
				true);

			drawInternalGUIControl(0, 0);

			// Done, draw everything to screen!
			ScummEngine::drawDirtyScreenParts();

			// Stay in the dialog while we keep pressing CTRL-C...
			Common::KeyState ks;
			bool leftBtnPressed = false, rightBtnPressed = false;
			do {
				clearClickedStatus();
				ks = Common::KEYCODE_INVALID;
				waitForBannerInput(-1, ks, leftBtnPressed, rightBtnPressed);
			} while (ks.keycode == Common::KEYCODE_LCTRL ||
					 ks.keycode == Common::KEYCODE_RCTRL ||
					 (ks.keycode == Common::KEYCODE_c && ks.hasFlags(Common::KBD_CTRL)));

			ctrlId = getInternalGUIControlFromCoordinates(_mouse.x, _mouse.y);
			if ((leftBtnPressed && ctrlId == 0) || (toupper(ks.ascii) == yesLabelPtr[0]))
				quitGame();

			// Restore the previous cursor...
			_cursor.state = curCursorState;
			CursorMan.showMouse(_cursor.state > 0);
			setCursorHotspot(curCursorHotspotX, curCursorHotspotY);
			setCursorFromBuffer(curGrabbedCursor, curCursorWidth, curCursorHeight, curCursorWidth);
			free(curGrabbedCursor);

			// The interpreter makes us wait 45 full frames;
			// let's wait half that time...
			waitForTimer(45 * 2);

			// Again, he interpreter does not explicitly restore the screen
			// after finishing displaying this query dialog, but we have to...
			if (_bannerMem && !isSmushActive()) {
				memcpy(
					_virtscr[kMainVirtScreen].getPixels(0, _screenTop),
					_bannerMem,
					_bannerMemSize);
				free(_bannerMem);
				_bannerMem = nullptr;

				markRectAsDirty(_virtscr[kMainVirtScreen].number, 0, _screenWidth + 8, _screenTop, _screenHeight + _screenTop);
				ScummEngine::drawDirtyScreenParts();
				_system->updateScreen();
			}

			// Finally, resume the engine, clear the input state, and restore the charset.
			pt.clear();
			clearClickedStatus();
			if (oldId)
				_charset->setCurID(oldId);

			// Restore the old cursor state...
			_cursor.state = oldCursorState;
			CursorMan.showMouse(_cursor.state > 0);
		} else {
			ScummEngine::queryQuit();
		}
	} else {
		ScummEngine::confirmExitDialog();
	}
}

const char *ScummEngine_v8::getGUIString(int stringId) {
	InfoDialog d(this, 0);
	int resStringId = -1;

	switch (stringId) {
	case gsPause:
		resStringId = 4;
		break;
	case gsQuitPrompt:
		resStringId = (_game.features & GF_DEMO) ? 30 : 22;
		break;
	case gsYes:
		resStringId = 23;
		break;
	case gsNo:
		resStringId = 24;
		break;
	case gsIMuseBuffer:
		resStringId = 25;
		break;
	case gsVoiceAndText:
		resStringId = 26;
		break;
	case gsTextDisplayOnly:
		resStringId = 27;
		break;
	case gsVoiceOnly:
		resStringId = 28;
		break;
	case gsYesKey:
		resStringId = 29;
		break;
	case gsTextSpeed:
		resStringId = 31;
		break;
	case gsMusicVolume:
		resStringId = 32;
		break;
	case gsVoiceVolume:
		resStringId = 33;
		break;
	case gsSfxVolume:
		resStringId = 34;
		break;
	case gsHeap:
		resStringId = 35;
		break;
	default:
		return "";
	}

	if (resStringId > 0)
		return d.getPlainEngineString(resStringId);
	else
		return "";
}

const char *ScummEngine_v7::getGUIString(int stringId) {
	InfoDialog d(this, 0);
	int resStringId = -1;

	switch (stringId) {
	case gsPause:
		resStringId = 4;
		break;
	case gsVersion:
		resStringId = 47;
		break;
	case gsRestart:
		resStringId = 5;
		break;
	case gsQuitPrompt:
		resStringId = 6;
		break;
	case gsSave:
		resStringId = 7;
		break;
	case gsLoad:
		resStringId = 8;
		break;
	case gsPlay:
		resStringId = 9;
		break;
	case gsCancel:
		resStringId = 10;
		break;
	case gsQuit:
		resStringId = 11;
		break;
	case gsOK:
		resStringId = 12;
		break;
	case gsMustName:
		resStringId = 14;
		break;
	case gsGameNotSaved:
		resStringId = 15;
		break;
	case gsGameNotLoaded:
		resStringId = 16;
		break;
	case gsSaving:
		resStringId = 17;
		break;
	case gsLoading:
		resStringId = 18;
		break;
	case gsNamePrompt:
		resStringId = 19;
		break;
	case gsSelectLoadPrompt:
		resStringId = 20;
		break;
	case gsSavePath:
		resStringId = 21;
		break;
	case gsTitle:
		resStringId = 22;
		break;
	case gsReplacePrompt:
		break;
	case gsYes:
		break;
	case gsNo:
		break;
	case gsVoiceOnly:
		resStringId = 50;
		break;
	case gsVoiceAndText:
		resStringId = 51;
		break;
	case gsTextDisplayOnly:
		resStringId = 52;
		break;
	case gsTextSpeed:
		resStringId = 53;
		break;
	case gsMusicVolume:
		resStringId = 54;
		break;
	case gsVoiceVolume:
		resStringId = 55;
		break;
	case gsSfxVolume:
		resStringId = 56;
		break;
	case gsHeap:
		resStringId = 57;
		break;
	case gsIMuseBuffer:
		resStringId = 58;
		break;
	default:
		return "";
	}

	if (resStringId > 0)
		return d.getPlainEngineString(resStringId);
	else
		return "";
}

int ScummEngine_v7::getGUIStringHeight(const char *str) {
	return _textV7->getStringHeight(str);
}

int ScummEngine_v7::getGUIStringWidth(const char *str) {
	return _textV7->getStringWidth(str);
}

void ScummEngine_v7::drawGUIText(const char *buttonString, int textXPos, int textYPos, int textColor, bool centerFlag) {
	drawTextImmediately((const byte *)buttonString, textXPos, textYPos, textColor, 1, (TextStyleFlags)centerFlag);
}

int ScummEngine_v7::getBannerColor(int bannerId) {
	if (_game.version == 8) {
		byte *palette = isSmushActive() ? _splayer->getVideoPalette() : _currentPalette;
		byte r, g, b;
		r = (_bannerColors[bannerId] >> 0) & 0xFF;
		g = (_bannerColors[bannerId] >> 8) & 0xFF;
		b = (_bannerColors[bannerId] >> 16) & 0xFF;
		return getPaletteColorFromRGB(palette, r, g, b);
	}

	int color = readArray(88, 0, bannerId);
	if (isSmushActive()) {
		color = (int)getPaletteColorFromRGB(_splayer->getVideoPalette(),
											_currentPalette[3 * color + 0],
											_currentPalette[3 * color + 1],
											_currentPalette[3 * color + 2]);
	}

	return color;
}
#endif

int ScummEngine_v6::getBannerColor(int bannerId) {
	return readArray(110, 0, bannerId);
}

int ScummEngine::getBannerColor(int bannerId) {
	byte *arrAddr = getResourceAddress(rtString, 21);
	return (int)arrAddr[bannerId];
}

void ScummEngine::queryQuit() {
	char msgLabelPtr[512];
	char localizedYesKey;

	convertMessageToString((const byte *)getGUIString(gsQuitPrompt), (byte *)msgLabelPtr, sizeof(msgLabelPtr));
	if (msgLabelPtr[0] != '\0') {
		localizedYesKey = msgLabelPtr[strnlen(msgLabelPtr, sizeof(msgLabelPtr)) - 1];
		msgLabelPtr[strnlen(msgLabelPtr, sizeof(msgLabelPtr)) - 1] = '\0';

		// "Are you sure you want to quit?  (Y-N)"
		Common::KeyState ks = showBannerAndPause(0, -1, msgLabelPtr);

		if (tolower(localizedYesKey) == ks.ascii || toupper(localizedYesKey) == ks.ascii) {
			quitGame();
		}
	}
}

void ScummEngine::queryRestart() {
	char msgLabelPtr[512];
	char localizedYesKey;

	convertMessageToString((const byte *)getGUIString(gsRestart), (byte *)msgLabelPtr, sizeof(msgLabelPtr));
	if (msgLabelPtr[0] != '\0') {
		localizedYesKey = msgLabelPtr[strnlen(msgLabelPtr, sizeof(msgLabelPtr)) - 1];
		msgLabelPtr[strnlen(msgLabelPtr, sizeof(msgLabelPtr)) - 1] = '\0';

		// "Are you sure you want to quit?  (Y-N)"
		Common::KeyState ks = showBannerAndPause(0, -1, msgLabelPtr);

		if (tolower(localizedYesKey) == ks.ascii || toupper(localizedYesKey) == ks.ascii) {
			restart();
		}
	}
}

int ScummEngine::getGUIStringHeight(const char *str) {
	return _charset->getFontHeight();
}

int ScummEngine::getGUIStringWidth(const char *str) {
	return _charset->getStringWidth(0, (const byte *)str);
}

void ScummEngine::drawGUIText(const char *buttonString, int textXPos, int textYPos, int textColor, bool centerFlag) {
	_string[4].ypos = textYPos;
	_string[4].xpos = textXPos;
	_string[4].right = _screenWidth - 1;
	_string[4].center = centerFlag;
	_string[4].color = textColor;
	_string[4].charset = 1;
	drawString(4, (const byte *)buttonString);
}

void ScummEngine::getSliderString(int stringId, int value, char *sliderString, int size) {
	char *ptrToChar;
	char tempStr[256];

	strcpy(tempStr, getGUIString(stringId));
	convertMessageToString((const byte *)tempStr, (byte *)sliderString, size);

	ptrToChar = strchr(sliderString, '=');

	if (!ptrToChar) {
		ptrToChar = strstr(sliderString, "xxx");
	}

	if (ptrToChar) {
		if (stringId == gsTextSpeed) {
			memset(ptrToChar, '\v', 10);
			ptrToChar[9 - value] = '\f';
		} else {
			memset(ptrToChar, '\v', 9);
			ptrToChar[value / 15] = '\f';
		}
	}
}

const char *ScummEngine_v6::getGUIString(int stringId) {
	InfoDialog d(this, 0);
	int resStringId = -1;

	switch (stringId) {
	case gsPause:
		resStringId = 4;
		break;
	case gsTextSpeed:
		break;
	case gsRestart:
		resStringId = 5;
		break;
	case gsQuitPrompt:
		resStringId = 6;
		break;
	case gsSave:
		resStringId = 7;
		break;
	case gsLoad:
		resStringId = 8;
		break;
	case gsPlay:
		resStringId = 9;
		break;
	case gsCancel:
		resStringId = 10;
		break;
	case gsQuit:
		resStringId = 11;
		break;
	case gsOK:
		resStringId = 12;
		break;
	case gsMustName:
		resStringId = 14;
		break;
	case gsGameNotSaved:
		resStringId = 15;
		break;
	case gsGameNotLoaded:
		resStringId = 16;
		break;
	case gsSaving:
		resStringId = 17;
		break;
	case gsLoading:
		resStringId = 18;
		break;
	case gsNamePrompt:
		resStringId = 19;
		break;
	case gsSelectLoadPrompt:
		resStringId = 20;
		break;
	case gsTitle:
		resStringId = 21;
		break;
	case gsReplacePrompt:
		break;
	case gsYes:
		break;
	case gsNo:
		break;
	case gsIMuseBuffer:
		break;
	case gsVoiceAndText:
		break;
	case gsTextDisplayOnly:
		break;
	case gsYesKey:
		break;
	case gsMusicVolume:
		break;
	case gsVoiceVolume:
		break;
	case gsSfxVolume:
		break;
	case gsHeap:
		return "Heap %dK";
	default:
		return "";
	}

	if (resStringId > 0)
		return d.getPlainEngineString(resStringId);
	else
		return "";
}

const char *ScummEngine::getGUIString(int stringId) {
	InfoDialog d(this, 0);
	int resStringId = -1;

	switch (stringId) {
	case gsPause:
		resStringId = 4;
		break;
	case gsVersion:
		break;
	case gsTextSpeed:
		break;
	case gsRestart:
		resStringId = 5;
		break;
	case gsQuitPrompt:
		resStringId = 6;
		break;
	case gsSave:
		resStringId = 7;
		break;
	case gsLoad:
		resStringId = 8;
		break;
	case gsPlay:
		resStringId = 9;
		break;
	case gsCancel:
		resStringId = 10;
		break;
	case gsQuit:
		resStringId = 11;
		break;
	case gsOK:
		resStringId = 12;
		break;
	case gsMustName:
		resStringId = 13;
		break;
	case gsGameNotSaved:
		resStringId = 14;
		break;
	case gsGameNotLoaded:
		resStringId = 15;
		break;
	case gsSaving:
		resStringId = 16;
		break;
	case gsLoading:
		resStringId = 17;
		break;
	case gsNamePrompt:
		resStringId = 18;
		break;
	case gsSelectLoadPrompt:
		resStringId = 19;
		break;
	case gsReplacePrompt:
		break;
	case gsYes:
		break;
	case gsNo:
		break;
	case gsVoiceAndText:
		break;
	case gsTextDisplayOnly:
		break;
	case gsMusicVolume:
		break;
	case gsVoiceVolume:
		break;
	case gsSfxVolume:
		break;
	case gsHeap:
		return "Heap %dK";
	case gsTitle:
		resStringId = 20;
		break;
	default:
		return "";
	}

	if (resStringId > 0)
		return d.getPlainEngineString(resStringId);
	else
		return "";
}

} // End of namespace Scumm
