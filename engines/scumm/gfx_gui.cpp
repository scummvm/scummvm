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
#include "scumm/scumm_v4.h"
#include "scumm/scumm_v8.h"
#include "scumm/gfx.h"
#include "scumm/dialogs.h"
#include "scumm/charset.h"
#include "scumm/string_v7.h"
#include "scumm/smush/smush_player.h"
#include "scumm/imuse_digi/dimuse_engine.h"

#include "graphics/cursorman.h"
#include "graphics/thumbnail.h"

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

	_messageBannerActive = true;

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

	// Backup the text surface...
	if (_game.version < 7 && !_mainMenuIsActive && _game.platform != Common::kPlatformFMTowns) {
		saveSurfacesPreGUI();
		if (_charset->_textScreenID == kMainVirtScreen && !(_game.version == 4 && _game.id == GID_LOOM))
			restoreCharsetBg();
	}

	// Pause shake effect
	_shakeTempSavedState = _shakeEnabled;
	setShake(0);

	// Pause the engine
	PauseToken pt = pauseEngine();

	// Gather the colors needed for the banner
	if (_game.version == 4) {
		normalFillColor = 7;
		normalTextColor = 0;
		topLineColor = 15;
		bottomLineColor = 8;
		leftLineColor = 15;
		rightLineColor = 8;
	} else {
		int palOffset = (_game.version == 8) ? 0 : 11;
		normalFillColor = getBannerColor(6 * bannerId + 15 + palOffset);
		normalTextColor = getBannerColor(6 * bannerId + 14 + palOffset);
		topLineColor = getBannerColor(6 * bannerId + 16 + palOffset);
		bottomLineColor = getBannerColor(6 * bannerId + 17 + palOffset);
		leftLineColor = getBannerColor(6 * bannerId + 18 + palOffset);
		rightLineColor = getBannerColor(6 * bannerId + 19 + palOffset);
	}

	// Backup the current charsetId, since we're going to switch
	// to charsetId == 1...
	int oldId = _charset->getCurID();
	_charset->setCurID(1);

	// Take all the necessary measurements for the box which
	// will contain the string...
	bool isCOMIDemo = (_game.id == GID_CMI && (_game.features & GF_DEMO) != 0);
	bannerMsgHeight = ((_game.id == GID_DIG || isCOMIDemo) ? getGUIStringHeight("ABC \x80\x78 \xb0\x78") : getGUIStringHeight(bannerMsg)) + 5;

	bannerMsgWidth = getGUIStringWidth(bannerMsg);
	if (bannerMsgWidth < 100)
		bannerMsgWidth = 100;

	roundedWidth = (((bannerMsgWidth + 15) & 0xFFF0) + 8) / 2;
	if (_game.version < 7) {
		roundedWidth = bannerMsgWidth / 2;
	}

	if (_game.version == 8) {
		startingPointX = _screenWidth / 2 - roundedWidth - 4;
		startingPointY = _screenHeight / 2 - 10;
		xPos = _screenWidth / 2 + roundedWidth + 3;
		yPos = 1 - bannerMsgHeight;
		bannerSaveYStart = startingPointY;
	} else if (_game.id == GID_MONKEY && _game.platform == Common::kPlatformFMTowns) {
		bannerMsgWidth = getGUIStringWidth(bannerMsg) / 2;
		startingPointX = ((160 - bannerMsgWidth) - 8) & 0xFFF8;
		startingPointY = (bannerMsgWidth + 168) | 0x7;
		xPos = startingPointX + 1; // Bogus value, since it is unused
		yPos = startingPointY + 1; // Bogus value, since it is unused
		bannerSaveYStart = 78;
	} else {
		startingPointX = 156 - roundedWidth;
		startingPointY = ((_game.version < 7) ? 80 : _screenHeight / 2 - 10);
		xPos = roundedWidth + 163 + ((_game.version < 7) ? 1 : 0);
		yPos = 1 - bannerMsgHeight; // For the normal font this will end up as -12, for CJK modes it will be appropriately adjusted.
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
							bannerMsg, true, true);

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

	// Restore the text surface...
	if (_game.version < 7 && !_mainMenuIsActive && _game.platform != Common::kPlatformFMTowns) {
		restoreSurfacesPostGUI();
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

	// Deactivate this, so it does not trigger as invalid click target inside
	// getInternalGUIControlFromCoordinates() (which can cause serious errors)
	_internalGUIControls[0].relativeCenterX = -1;

	_messageBannerActive = false;

	return ks;
}

Common::KeyState ScummEngine::showOldStyleBannerAndPause(const char *msg, int color, int32 waitTime) {
	// LOOM VGA uses the new style GUI
	if (_game.version == 4 && _game.id == GID_LOOM) {
		return showBannerAndPause(0, waitTime, msg);
	}

	char bannerMsg[512];
	int bannerMsgWidth, bannerMsgHeight;
	int startingPointY;
	int bannerSaveYStart;

	_messageBannerActive = true;

	// Fetch the translated string for the message...
	convertMessageToString((const byte *)msg, (byte *)bannerMsg, sizeof(bannerMsg));

	// Backup the text surface...
	if (!_mainMenuIsActive) {
		saveSurfacesPreGUI();
		if (_charset->_textScreenID == kMainVirtScreen)
			restoreCharsetBg();
	}

	// Pause shake effect
	_shakeTempSavedState = _shakeEnabled;
	setShake(0);

	// Pause the engine
	PauseToken pt = pauseEngine();

	// Backup the current charsetId, since we're going to switch
	// to charsetId == 1...
	int oldId = _charset->getCurID();
	_charset->setCurID(1);

	// Take all the necessary measurements for the box which
	// will contain the string...
	bannerMsgHeight = getGUIStringHeight(bannerMsg) + 3;
	bannerMsgWidth = getGUIStringWidth(bannerMsg);
	if (bannerMsgWidth < 100)
		bannerMsgWidth = 100;

	startingPointY = 80;
	bannerSaveYStart = startingPointY - 2;


	// Save the pixels which will be overwritten by the banner,
	// so that we can restore them later...
	if (!_bannerMem) {
		int rowSize = _screenWidth + 8;
		_bannerMemSize = (bannerMsgHeight + 2) * (_screenWidth + 8);
		_bannerMem = (byte *)malloc(_bannerMemSize * sizeof(byte));
		if (_bannerMem) {
			memcpy(
				_bannerMem,
				&_virtscr[kMainVirtScreen].getPixels(0, _screenTop)[rowSize * bannerSaveYStart],
				_bannerMemSize);
		}
	}

	// Draw the GUI control
	drawBox(0, startingPointY, _screenWidth - 1, startingPointY + bannerMsgHeight, 0);
	drawBox(0, startingPointY, _screenWidth - 1, startingPointY, color);
	drawBox(0, startingPointY + bannerMsgHeight, _screenWidth - 1, startingPointY + bannerMsgHeight, color);
	drawGUIText(bannerMsg, 0, _screenWidth / 2, startingPointY + 2, color, true);
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

	// Restore the text surface...
	if (!_mainMenuIsActive) {
		restoreSurfacesPostGUI();
	}

	// Finally, resume the engine, clear the input state, and restore the charset.
	pt.clear();
	clearClickedStatus();
	if (oldId)
		_charset->setCurID(oldId);

	// Deactivate this, so it does not trigger as invalid click target inside
	// getInternalGUIControlFromCoordinates() (which can cause serious errors)
	_internalGUIControls[0].relativeCenterX = -1;

	_messageBannerActive = false;

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
			if (_game.version == 8) {
				startingPointY = _screenHeight / 2 - 10;
			} else if (_game.id == GID_MONKEY && _game.platform == Common::kPlatformFMTowns) {
				startingPointY = 78;
			} else {
				startingPointY = ((_game.version < 7) ? 80 - 2 : _screenHeight / 2 - 10);
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

	// Restore shake effect
	setShake(_shakeTempSavedState);
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
										  int anchorPointX, int anchorPointY, int x, int y, const char *label, bool centerFlag, bool doubleLinesFlag) {

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
	ctrl->doubleLinesFlag = doubleLinesFlag;
}

void ScummEngine::drawInternalGUIControl(int id, bool highlightColor) {
	InternalGUIControl *ctrl;
	int relCentX, relCentY, textHeight;
	int x, y, textXPos, textYPos;
	int textColor, fillColor;
	int boxSizeX, boxSizeY;
	int offset = (_game.version == 8 || _game.id == GID_DIG) ? 2 : 1;
	int topComp = (_game.version < 8) ? _screenTop : 0;

	bool centerFlag;
	char buttonString[512];

	ctrl = &_internalGUIControls[id];
	relCentX = ctrl->relativeCenterX;
	if (ctrl->relativeCenterX != -1) {
		relCentY = ctrl->relativeCenterY;
		x = ctrl->xPos;
		y = ctrl->yPos;

		boxSizeX = x - ((_game.version == 8) ? ctrl->relativeCenterX : 0);
		boxSizeY = y - ((_game.version == 8) ? relCentY : 0);

		fillColor = highlightColor ? ctrl->highlightedFillColor : ctrl->normalFillColor;

		if (_game.id == GID_MONKEY && _game.platform == Common::kPlatformFMTowns) {
			// MI1 for FMTowns does its own thing with hardcoded values here...
			drawBox(relCentX + 1, 81, relCentY - 1, 90, fillColor);

			drawLine(relCentX + 1, 80, relCentY - 1, 80, ctrl->topLineColor);
			drawLine(relCentX + 1, 91, relCentY - 1, 91, ctrl->bottomLineColor);
			drawLine(relCentX, 81, relCentX, 90, ctrl->leftLineColor);
			drawLine(relCentY, 81, relCentY, 90, ctrl->rightLineColor);
		} else {
			if (ctrl->doubleLinesFlag) {
				// Draw the main box...
				drawBox(relCentX + 1, relCentY + 1 + topComp, boxSizeX - offset, boxSizeY - offset + topComp, fillColor);

				if (_game.version == 4 && _game.id != GID_LOOM) {
					// This is for MI1 v4 (EGA and VGA floppy versions) and the Passport to Adventure demo;
					// these games use a very early version of this GUI system, with only
					// one line color; we will use the topLineColor to mimick that.

					// Draw the contour lines...
					drawBox(relCentX + 2, relCentY, x - 2, relCentY, ctrl->topLineColor);
					drawBox(relCentX + 2, y, x - 2, y, ctrl->topLineColor);
					drawBox(relCentX, relCentY + 2, relCentX, y - 2, ctrl->topLineColor);
					drawBox(x, relCentY + 2, x, y - 2, ctrl->topLineColor);

					// Draw single pixels for the button roundness effect...
					drawBox(relCentX + 1, relCentY + 1, relCentX + 1, relCentY + 1, ctrl->topLineColor);
					drawBox(x - 1, relCentY + 1, x - 1, relCentY + 1, ctrl->topLineColor);
					drawBox(relCentX + 1, y - 1, relCentX + 1, y - 1, ctrl->topLineColor);
					drawBox(x - 1, y - 1, x - 1, y - 1, ctrl->topLineColor);
				} else {
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
			} else {
				drawBox(relCentX, relCentY + topComp, x, y + topComp, (highlightColor ? ctrl->highlightedFillColor : ctrl->normalFillColor));
				if (_game.version == 4 && _game.id != GID_LOOM) {
					drawBox(relCentX, relCentY, x, relCentY, ctrl->topLineColor);
					drawBox(relCentX, y, x, y, ctrl->bottomLineColor);
					drawBox(relCentX, relCentY, relCentX, y, ctrl->leftLineColor);
					drawBox(x, relCentY, x, y, ctrl->rightLineColor);
				} else {
					drawLine(relCentX, relCentY, x, relCentY, ctrl->topLineColor);
					drawLine(relCentX, y, x, y, ctrl->bottomLineColor);
					drawLine(relCentX, relCentY, relCentX, y, ctrl->leftLineColor);
					drawLine(x, relCentY, x, y, ctrl->rightLineColor);
				}
			}
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
			textHeight = getGUIStringHeight(ctrl->label.c_str());

			if (centerFlag)
				textXPos = relCentX + (x - ctrl->relativeCenterX) / 2;
			else
				textXPos = relCentX + 2;

			if (_game.version == 8 || _game.id == GID_DIG)
				textYPos = relCentY + (y - relCentY - textHeight) / 2 + 1;
			else
				textYPos = relCentY + (y - 8 - relCentY + 2) / 2;
		}

		// Finally, choose the color and draw the text message
		if (highlightColor)
			textColor = ctrl->highlightedTextColor;
		else
			textColor = ctrl->normalTextColor;

		Common::strlcpy(buttonString, ctrl->label.c_str(), sizeof(buttonString));

		if ((id >= GUI_CTRL_FIRST_SG && id <= GUI_CTRL_LAST_SG) &&
			_mainMenuSavegameLabel == id && _menuPage == GUI_PAGE_SAVE) {
			Common::strlcat(buttonString, "_", sizeof(buttonString));
		}

		if ((id == GUI_CTRL_ARROW_UP_BUTTON || id == GUI_CTRL_ARROW_DOWN_BUTTON) && _useCJKMode && _game.id == GID_DIG) {
			// Instead of printing the "normal" arrow glyphs, the DIG CJK interpreter draws slightly larger arrows
			// using the drawLine function. Instead of making a row of 14 drawLine calls here (like the original),
			// I have implemented it like this...
			static const int8 drwOffsets[2][29] = {
				{ 0, 13, -5, 18, -5, 18, -3, 18, -3, 18, -3, 33, 0, 13, 5, 18, 5, 18, 3, 18, 3, 18, 3, 33, -3, 33, 3, 33, -1 },
				{ 0, 33, -5, 28, -5, 28, -3, 28, -3, 28, -3, 13, 0, 33, 5, 28, 5, 28, 3, 28, 3, 28, 3, 13, -3, 13, 3, 13, -1 }
			};

			for (const int8 *s = drwOffsets[id - GUI_CTRL_ARROW_UP_BUTTON]; *s != -1; s += 4)
				drawLine(textXPos + s[0], relCentY + s[1], textXPos + s[2], relCentY + s[3], textColor);

		} else {
			int tmpRight = _string[5].right;
			_string[5].right = _screenWidth - 1;

			// The original CJK DIG interpreter limits the clipping to the save slots. Other elements
			// seem to (theoretically) be allowed to draw text wherever they want...
			bool isSaveSlot = (id >= GUI_CTRL_FIRST_SG && id <= GUI_CTRL_LAST_SG);
			Common::Rect clipRect(relCentX, relCentY, x, y);
			drawGUIText(buttonString, isSaveSlot ? &clipRect : 0, textXPos, textYPos, textColor, centerFlag);
			_string[5].right = tmpRight;
		}

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
void ScummEngine_v7::queryQuit(bool returnToLauncher) {
	if (isUsingOriginalGUI()) {
		if (_game.version == 8 && !(_game.features & GF_DEMO)) {
			int boxWidth, strWidth;
			int ctrlId;
			char yesLabelPtr[512];
			char noLabelPtr[512];
			char msgLabelPtr[512];
			byte *curGrabbedCursor;
			int curCursorWidth, curCursorHeight, curCursorHotspotX, curCursorHotspotY, curCursorState;

			_messageBannerActive = true;
			_comiQuitMenuIsOpen = true;

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
				true,
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
			drawTextImmediately((const byte *)msgLabelPtr, &_defaultTextClipRect, 320, 200, getBannerColor(32), 1, (TextStyleFlags) true);

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
				true,
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
				true,
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
			if ((leftBtnPressed && ctrlId == 0) || (toupper(ks.ascii) == yesLabelPtr[0])) {
				_quitByGUIPrompt = true;
				if (returnToLauncher) {
					Common::Event event;
					event.type = Common::EVENT_RETURN_TO_LAUNCHER;
					getEventManager()->pushEvent(event);
				} else {
					quitGame();
				};
			}

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

			_comiQuitMenuIsOpen = false;
			_messageBannerActive = false;
		} else {
			ScummEngine::queryQuit(returnToLauncher);
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
	case gsRestart:
		resStringId = 5;
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
	case gsTextSpeedSlider:
		resStringId = 31;
		break;
	case gsMusicVolumeSlider:
		resStringId = 32;
		break;
	case gsVoiceVolumeSlider:
		resStringId = 33;
		break;
	case gsSfxVolumeSlider:
		resStringId = 34;
		break;
	case gsHeap:
		resStringId = 35;
		break;
	default:
		return _emptyMsg;
	}

	if (resStringId > 0)
		return d.getPlainEngineString(resStringId);
	else
		return _emptyMsg;
}

const char *ScummEngine_v7::getGUIString(int stringId) {
	InfoDialog d(this, 0);
	int resStringId = -1;

	switch (stringId) {
	case gsPause:
		resStringId = 4;
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
	case gsYes:
		break;
	case gsNo:
		break;
	case gsMusic:
		resStringId = 41;
		break;
	case gsVoice:
		resStringId = 42;
		break;
	case gsSfx:
		resStringId = 43;
		break;
	case gsDisabled:
		resStringId = 44;
		break;
	case gsTextSpeed:
		resStringId = 45;
		break;
	case gsDisplayText:
		resStringId = 46;
		break;
	case gsVersion:
		resStringId = 47;
		break;
	case gsSpooledMusic:
		resStringId = 48;
		break;
	case gsReplacePrompt:
		resStringId = 49;
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
	case gsTextSpeedSlider:
		resStringId = 53;
		break;
	case gsMusicVolumeSlider:
		resStringId = 54;
		break;
	case gsVoiceVolumeSlider:
		resStringId = 55;
		break;
	case gsSfxVolumeSlider:
		resStringId = 56;
		break;
	case gsHeap:
		resStringId = 57;
		break;
	case gsIMuseBuffer:
		resStringId = 58;
		break;
	default:
		return _emptyMsg;
	}

	const char *res =  (resStringId > 0) ? d.getPlainEngineString(resStringId) : _emptyMsg;

	if (_game.id == GID_DIG) {
		convertMessageToString((const byte*)res, _guiStringTransBuff, 512);
		res = (const char*)_guiStringTransBuff;
	}

	return res;
}

int ScummEngine_v7::getGUIStringHeight(const char *str) {
	return _textV7->getStringHeight(str);
}

int ScummEngine_v7::getGUIStringWidth(const char *str) {
	return _textV7->getStringWidth(str);
}

void ScummEngine_v7::drawGUIText(const char *buttonString, Common::Rect *clipRect, int textXPos, int textYPos, int textColor, bool centerFlag) {
	drawTextImmediately((const byte *)buttonString, clipRect, textXPos, textYPos, textColor, 1, (TextStyleFlags)centerFlag);
}

int ScummEngine_v7::getMusicVolume() {
	return CLIP<int>(ConfMan.getInt("music_volume") / 2, 0, 127);
}

int ScummEngine_v7::getSpeechVolume() {
	return CLIP<int>(ConfMan.getInt("speech_volume") / 2, 0, 127);
}

int ScummEngine_v7::getSFXVolume() {
	return CLIP<int>(ConfMan.getInt("sfx_volume") / 2, 0, 127);
}

void ScummEngine_v7::setMusicVolume(int volume) {
	_imuseDigital->diMUSESetMusicGroupVol(CLIP<int>(volume, 0, 127));
	ScummEngine::setMusicVolume(CLIP<int>(volume, 0, 127));
}

void ScummEngine_v7::setSpeechVolume(int volume) {
	ScummEngine::setSpeechVolume(CLIP<int>(volume, 0, 127));
	_imuseDigital->diMUSESetVoiceGroupVol(CLIP<int>(volume, 0, 127));
}

void ScummEngine_v7::setSFXVolume(int volume) {
	ScummEngine::setSFXVolume(CLIP<int>(volume, 0, 127));
	_imuseDigital->diMUSESetSFXGroupVol(CLIP<int>(volume, 0, 127));
}

void ScummEngine_v7::toggleVoiceMode() {
	ScummEngine::toggleVoiceMode();
	if (VAR_VOICE_MODE != 0xFF) {
		_splayer->setChanFlag(0, VAR(VAR_VOICE_MODE) != 0);
		_splayer->setChanFlag(2, VAR(VAR_VOICE_MODE) != 2);
	}
}

void ScummEngine_v7::handleLoadDuringSmush() {
	// Notify the SMUSH player that we want to load a game...
	_saveLoadFlag = 2;
	_saveLoadSlot = _mainMenuSavegameLabel - 1 + _curDisplayedSaveSlotPage * 9;

	// Force screen to black to avoid glitches...
	VirtScreen *vs = &_virtscr[kMainVirtScreen];
	memset(vs->getPixels(0, 0), 0, vs->pitch * vs->h);
	vs->setDirtyRange(0, vs->h);
	ScummEngine::drawDirtyScreenParts();

	// The original at this point does this, but we automatically
	// handle the corresponding operations automatically in our
	// SMUSH player...
	//
	//_splayer->release();
	//_splayer->resetAudioTracks();
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

int ScummEngine_v4::getBannerColor(int bannerId) {
	const byte *palette = (_renderMode == Common::kRenderCGA) ? _GUIPaletteCGA : _GUIPalette;

	return (int)palette[bannerId - 6];
}

int ScummEngine_v6::getBannerColor(int bannerId) {
	return readArray(110, 0, bannerId);
}

int ScummEngine::getBannerColor(int bannerId) {
	byte *arrAddr = getResourceAddress(rtString, 21);
	return (int)arrAddr[bannerId];
}

void ScummEngine::saveSurfacesPreGUI() {
	// Why are we saving three surfaces? Bear with me:
	// texts in v6 and below are drawn sometimes onto the main surface, while
	// other times they are drawn onto the text surface.
	//
	// Now, the original interpreters just killed every text after the
	// menu is closed; this includes dialog texts, plain printed strings and
	// possibly more.
	//
	// We can do better than that! Let's save both the main and the text surfaces
	// so that we can restore them later when we're done with the menu.
	//
	// Some edge cases can happen though: i.e. during the SAMNMAX credits there's
	// not enough space for the menu to be drawn completely on the main surface,
	// so the last line is being drawn on the verb surface; to address this, we
	// save and restore that too.

	if (_game.version < 4 || _game.version > 6)
		return;

	_tempTextSurface = (byte *)malloc(_textSurface.pitch * _textSurface.h * sizeof(byte));
	_tempMainSurface = (byte *)malloc(_virtscr[kMainVirtScreen].w * _virtscr[kMainVirtScreen].h * sizeof(byte));
	_tempVerbSurface = (byte *)malloc(_virtscr[kVerbVirtScreen].w * _virtscr[kVerbVirtScreen].h * sizeof(byte));

	if (_tempMainSurface) {
		for (int y = 0; y < _virtscr[kMainVirtScreen].h; y++) {
			memcpy(&_tempMainSurface[y * _virtscr[kMainVirtScreen].w],
				_virtscr[kMainVirtScreen].getBasePtr(_virtscr[kMainVirtScreen].xstart, y),
				_virtscr[kMainVirtScreen].w);
		}
	}

	if (_tempVerbSurface) {
		memcpy(_tempVerbSurface,
			_virtscr[kVerbVirtScreen].getBasePtr(0, 0),
			_virtscr[kVerbVirtScreen].pitch * _virtscr[kVerbVirtScreen].h);
	}

	if (_tempTextSurface) {
		memcpy(_tempTextSurface, _textSurface.getBasePtr(0, 0), _textSurface.pitch * _textSurface.h);

		// For each v4-v6 game (except for LOOM VGA which does its own thing), we take the text surface
		// and stamp it on top of the main screen: this is done to ensure that the GUI is drawn on top
		// of possible subtitle texts instead of having the latters being deleted or being drawn on top
		// of the GUI...
		if (!(_game.version == 4 && _game.id == GID_LOOM)) {
			for (int y = 0; y < _screenHeight; y++) {
				for (int x = 0; x < _screenWidth; x++) {
					// Only draw non transparent pixels
					if (_tempTextSurface[x + y * _screenWidth] != 0xFD) {
						_virtscr[kMainVirtScreen].setPixel(_virtscr[kMainVirtScreen].xstart + x, y, _tempTextSurface[x + y * _screenWidth]);
					}
				}
			}
		}
	}
}

void ScummEngine::restoreSurfacesPostGUI() {

	if (_game.version < 4 || _game.version > 6)
		return;

	if (_tempTextSurface) {
		memcpy(_textSurface.getBasePtr(0, 0), _tempTextSurface, _textSurface.pitch * _textSurface.h);

		// Signal the restoreCharsetBg() function that there's text
		// on the text surface, so it gets deleted the next time another
		// text is displayed...
		if (_game.version != 4 || _game.id != GID_LOOM)
			_postGUICharMask = true;

		free(_tempTextSurface);
		_tempTextSurface = nullptr;
	}

	if (_tempMainSurface) {
		for (int y = 0; y < _virtscr[kMainVirtScreen].h; y++) {
			memcpy(_virtscr[kMainVirtScreen].getBasePtr(_virtscr[kMainVirtScreen].xstart, y),
				&_tempMainSurface[y * _virtscr[kMainVirtScreen].w],
				_virtscr[kMainVirtScreen].w);
		}

		free(_tempMainSurface);
		_tempMainSurface = nullptr;

		_virtscr[kMainVirtScreen].setDirtyRange(0, _virtscr[kMainVirtScreen].h);
	}

	if (_tempVerbSurface) {
		memcpy(_virtscr[kVerbVirtScreen].getBasePtr(0, 0),
			_tempVerbSurface,
			_virtscr[kVerbVirtScreen].pitch * _virtscr[kVerbVirtScreen].h);

		free(_tempVerbSurface);
		_tempVerbSurface = nullptr;

		_virtscr[kVerbVirtScreen].setDirtyRange(0, _virtscr[kVerbVirtScreen].h);
	}
}

void ScummEngine::toggleVoiceMode() {
	if (VAR_VOICE_MODE != 0xFF) {
		VAR(VAR_VOICE_MODE) = (VAR(VAR_VOICE_MODE) != 1) ? 1 : 0;
		ConfMan.setInt("original_gui_text_status", VAR(VAR_VOICE_MODE));
		ConfMan.setBool("speech_mute", VAR(VAR_VOICE_MODE) == 2);
		ConfMan.setBool("subtitles", VAR(VAR_VOICE_MODE) > 0);
		syncSoundSettings();
		ConfMan.flushToDisk();
	}
}

void ScummEngine::setMusicVolume(int volume) {
	volume = CLIP<int>(volume, 0, 127);
	if (_game.version < 7)
		_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, volume * 2);
	ConfMan.setInt("music_volume", volume * 2);
	ConfMan.flushToDisk();
}

void ScummEngine::setSpeechVolume(int volume) {
	volume = CLIP<int>(volume, 0, 127);
	if (_game.version < 7)
		_mixer->setVolumeForSoundType(Audio::Mixer::kSpeechSoundType, volume * 2);
	ConfMan.setInt("speech_volume", volume * 2);
	ConfMan.flushToDisk();
}

void ScummEngine::setSFXVolume(int volume) {
	volume = CLIP<int>(volume, 0, 127);
	if (_game.version < 7)
		_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, volume * 2);
	ConfMan.setInt("sfx_volume", volume * 2);
	ConfMan.flushToDisk();
}

int ScummEngine::getMusicVolume() {
	return CLIP<int>(_mixer->getVolumeForSoundType(Audio::Mixer::kMusicSoundType) / 2, 0, 127);
}

int ScummEngine::getSpeechVolume() {
	return CLIP<int>(_mixer->getVolumeForSoundType(Audio::Mixer::kSpeechSoundType) / 2, 0, 127);
}

int ScummEngine::getSFXVolume() {
	return CLIP<int>(_mixer->getVolumeForSoundType(Audio::Mixer::kSFXSoundType) / 2, 0, 127);
}

void ScummEngine::queryQuit(bool returnToLauncher) {
	char msgLabelPtr[512];
	char localizedYesKey;

	convertMessageToString((const byte *)getGUIString(gsQuitPrompt), (byte *)msgLabelPtr, sizeof(msgLabelPtr));
	if (msgLabelPtr[0] != '\0') {
		localizedYesKey = msgLabelPtr[strnlen(msgLabelPtr, sizeof(msgLabelPtr)) - 1];
		msgLabelPtr[strnlen(msgLabelPtr, sizeof(msgLabelPtr)) - 1] = '\0';

		_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, true);

		// "Are you sure you want to quit?  (Y/N)"
		Common::KeyState ks;
		if (_game.version > 4)
			ks = showBannerAndPause(0, -1, msgLabelPtr);
		else
			ks = showOldStyleBannerAndPause(msgLabelPtr, 12, -1);

		_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, false);

		if (tolower(localizedYesKey) == ks.ascii || toupper(localizedYesKey) == ks.ascii ||
			(ks.keycode == Common::KEYCODE_c && ks.hasFlags(Common::KBD_CTRL)) ||
			(ks.keycode == Common::KEYCODE_x && ks.hasFlags(Common::KBD_ALT))) {
			_quitByGUIPrompt = true;
			if (returnToLauncher) {
				Common::Event event;
				event.type = Common::EVENT_RETURN_TO_LAUNCHER;
				getEventManager()->pushEvent(event);
			} else {
				quitGame();
			}
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

		_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, true);

		// "Are you sure you want to restart?  (Y/N)"
		Common::KeyState ks;
		if (_game.version > 4)
			ks = showBannerAndPause(0, -1, msgLabelPtr);
		else
			ks = showOldStyleBannerAndPause(msgLabelPtr, 12, -1);

		_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, false);

		if ((tolower(localizedYesKey) == ks.ascii || toupper(localizedYesKey) == ks.ascii) ||
			(_game.version == 8 && ks.keycode == Common::KEYCODE_y)) {

			if (_game.version < 5)
				restoreCharsetBg();

			restart();
		}
	}
}

// This function is actually not a thing in the original, it's here to
// make things a little bit more modern and avoid making the menu feel
// artificially slow.
bool ScummEngine::shouldHighlightLabelAndWait(int clickedControl) {
	return ((clickedControl >= GUI_CTRL_SAVE_BUTTON && clickedControl <= GUI_CTRL_PATH_BUTTON) ||
			(clickedControl == GUI_CTRL_DISPLAY_TEXT_CHECKBOX ||
			 clickedControl == GUI_CTRL_SPOOLED_MUSIC_CHECKBOX));
}

void ScummEngine::fillSavegameLabels() {
	bool availSaves[100];
	listSavegames(availSaves, ARRAYSIZE(availSaves));
	Common::String name;
	int curSaveSlot;
	bool isLoomVga = (_game.id == GID_LOOM && _game.version == 4);
	_savegameNames.clear();

	for (int i = 0; i < 9; i++) {
		curSaveSlot = i + (isLoomVga ? _firstSaveStateOfList : _curDisplayedSaveSlotPage * 9);
		if (_game.version > 4 || (_game.version == 4 && _game.id == GID_LOOM)) {
			if (availSaves[curSaveSlot]) {
				if (getSavegameName(curSaveSlot, name)) {
					_savegameNames.push_back(Common::String::format("%2d. %s", curSaveSlot + 1, name.c_str()));
				} else {
					// The original printed "WARNING... old savegame", but we do support old savegames :-)
					_savegameNames.push_back(Common::String::format("%2d. WARNING: wrong save version", curSaveSlot + 1));
				}
			} else {
				_savegameNames.push_back(Common::String::format("%2d. ", curSaveSlot + 1));
			}
		} else {
			if (availSaves[curSaveSlot]) {
				if (getSavegameName(curSaveSlot, name)) {
					_savegameNames.push_back(Common::String::format("%s", name.c_str()));
				} else {
					// The original printed "WARNING... old savegame", but we do support old savegames :-)
					_savegameNames.push_back(Common::String::format("%s", "WARNING: wrong save version"));
				}
			} else {
				_savegameNames.push_back(Common::String());
			}
		}
	}
}

bool ScummEngine::canWriteGame(int slotId) {
	bool saveList[100];
	char msgLabelPtr[512];
	char localizedYesKey;

	if (_game.version < 7)
		return true;

	listSavegames(saveList, ARRAYSIZE(saveList));
	if (saveList[slotId - 1]) {
		convertMessageToString((const byte *)getGUIString(gsReplacePrompt), (byte *)msgLabelPtr, sizeof(msgLabelPtr));

		// Fallback to a hardcoded string
		if (msgLabelPtr[0] == '\0') {
			Common::strlcpy(msgLabelPtr, "Do you want to replace this saved game?  (Y/N)Y", sizeof(msgLabelPtr));
		}

		localizedYesKey = msgLabelPtr[strnlen(msgLabelPtr, sizeof(msgLabelPtr)) - 1];
		msgLabelPtr[strnlen(msgLabelPtr, sizeof(msgLabelPtr)) - 1] = '\0';

		_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, true);

		// "Do you want to replace this saved game?  (Y/N)"
		Common::KeyState ks = showBannerAndPause(0, -1, msgLabelPtr);

		_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, false);

		return (tolower(localizedYesKey) == ks.ascii || toupper(localizedYesKey) == ks.ascii);
	}

	return true;
}

bool ScummEngine::userWriteLabelRoutine(Common::KeyState &ks, bool &leftMsClicked, bool &rightMsClicked) {
	bool hasLoadedState = false;
	int firstChar = (_game.version == 4 && _game.id != GID_LOOM) ? 0 : 4;
	bool opResult = true;
	_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, true);

	while (!shouldQuit()) {
		waitForTimer(1);

		waitForBannerInput(-1, ks, leftMsClicked, rightMsClicked);
		rightMsClicked = false;
		if (ks.keycode == Common::KEYCODE_RETURN) {
			clearClickedStatus();
			opResult = executeMainMenuOperation(GUI_CTRL_OK_BUTTON, -1, -1, hasLoadedState);

			_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, false);
			return opResult;
		} else if (leftMsClicked) {
			clearClickedStatus();
			break;
		}

		// Handle special key presses
		int curLen = _savegameNames[_mainMenuSavegameLabel - 1].size();
		if (ks.keycode == Common::KEYCODE_BACKSPACE) {
			 // Prevent the user from deleting the header (" 1. ")
			if (curLen > firstChar) {
				_savegameNames[_mainMenuSavegameLabel - 1].deleteLastChar();
				_internalGUIControls[_mainMenuSavegameLabel].label = _savegameNames[_mainMenuSavegameLabel - 1];
				drawInternalGUIControl(_mainMenuSavegameLabel, 1);
				ScummEngine::drawDirtyScreenParts();
				_system->updateScreen();
			}
		} else if (ks.ascii >= 32 && ks.ascii <= 122) { // Handle characters
			if (curLen < 39) {
				_savegameNames[_mainMenuSavegameLabel - 1] += (char)ks.ascii;
				_internalGUIControls[_mainMenuSavegameLabel].label = _savegameNames[_mainMenuSavegameLabel - 1];
				drawInternalGUIControl(_mainMenuSavegameLabel, 1);
				ScummEngine::drawDirtyScreenParts();
				_system->updateScreen();
			}
		}

		clearClickedStatus();
	}

	_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, false);
	return false;
}

void ScummEngine::saveCursorPreMenu() {
	// Force the cursor to be ON...
	_oldCursorState = _cursor.state;
	_cursor.state = 1;
	CursorMan.showMouse(_cursor.state > 0);

	if (_game.version > 6) {
		// Backup the current cursor graphics and parameters
		// and set up the main menu cursor...
		_curGrabbedCursor = (byte *)malloc(sizeof(_grabbedCursor));
		if (_curGrabbedCursor) {
			memcpy(_curGrabbedCursor, _grabbedCursor, sizeof(_grabbedCursor));
			_curCursorState = isSmushActive() ? 0 : _cursor.state;
			_curCursorWidth = _cursor.width;
			_curCursorHeight = _cursor.height;
			_curCursorHotspotX = _cursor.hotspotX;
			_curCursorHotspotY = _cursor.hotspotY;
			setDefaultCursor();
		}
	} else if (_game.version == 6) {
		// V6 handles cursor substitution via scripts, but it handles
		// setting dimensions and hotspot here; since manually changing
		// cursor parameters at this stage glitches the cursor itself,
		// let's call this function without saving anything unlike above...
		setDefaultCursor();
	}

	CursorMan.showMouse(true);
}

void ScummEngine::restoreCursorPostMenu() {
	if (_game.version > 6 && _curGrabbedCursor) {
		// Restore the previous cursor...
		_cursor.state = _curCursorState;
		CursorMan.showMouse(_cursor.state > 0);
		if (_enableEGADithering) {
			_curCursorHotspotX >>= 1;
			_curCursorHotspotY >>= 1;
		}
		setCursorHotspot(_curCursorHotspotX, _curCursorHotspotY);
		setCursorFromBuffer(_curGrabbedCursor, _curCursorWidth, _curCursorHeight, _curCursorWidth, true);
		free(_curGrabbedCursor);
		_curGrabbedCursor = nullptr;
	} else if (_game.version == 6) {
		setCursorHotspot(_curCursorHotspotX, _curCursorHotspotY);
		_cursor.width = _curCursorWidth;
		_cursor.height = _curCursorHeight;
	}

	// Restore the old cursor state...
	_cursor.state = _oldCursorState;
}

void ScummEngine::showMainMenu() {
	char saveScreenTitle[512];
	int args[NUM_SCRIPT_LOCAL];
	bool leftMsClicked = false, rightMsClicked = false;
	int clickedControl = -1;
	int heldControl = -1;
	int curMouseX, curMouseY;
	bool hasLoadedState = false;

	Common::KeyState ks;

	memset(args, 0, sizeof(args));

	// Generate the thumbnail, in case the game is saved
	Graphics::createThumbnail(_savegameThumbnail);

	// Pause the engine
	PauseToken pt = pauseEngine();

	// Run the entrance savescreen script, if available
	if (VAR_SAVELOAD_SCRIPT != 0xFF)
		runScript(VAR(VAR_SAVELOAD_SCRIPT), 0, 0, nullptr);

	_saveSound = 1;
	_shakeTempSavedState = _shakeEnabled;
	setShake(0);

	if (_game.version < 7) {
		// Below version 7, we draw texts on a separate surface which is then composited
		// on top of the main one during ScummEngine::drawDirtyScreenParts().
		// This results in texts overlapping on top of the menu; let's simulate the end result
		// of the original by copying the text surface over the main one just before showing
		// the menu...
		saveSurfacesPreGUI();

		// V6 games should call for stopTalk() instead, but that's a bit too drastic;
		// this ensures that we can at least hear the speech after the menu is closed.
		if (_charset->_textScreenID == kMainVirtScreen && !(_game.version == 4 && _game.id == GID_LOOM))
			restoreCharsetBg();
	}

	_menuPage = GUI_PAGE_MAIN;
	setUpMainMenuControls();
	drawMainMenuControls();

	if (_game.platform == Common::kPlatformAmiga) {
		convertMessageToString((const byte *)getGUIString(gsInsertSaveDisk), (byte *)saveScreenTitle, sizeof(saveScreenTitle));
		drawMainMenuTitle(saveScreenTitle);
	} else if (_game.version > 4 && _game.id != GID_MONKEY2 && _game.id != GID_MONKEY) {
		convertMessageToString((const byte *)getGUIString(gsTitle), (byte *)saveScreenTitle, sizeof(saveScreenTitle));
		drawMainMenuTitle(saveScreenTitle);
	}

	updateMainMenuControls();

	// Save the current cursor state...
	saveCursorPreMenu();

	// Notify that the menu is now active
	_mainMenuIsActive = true;

	// Clear keypresses and mouse presses
	clearClickedStatus();

	// Menu loop
	while (!shouldQuit()) {
		// Update the screen and the cursor while we're in the loop
		waitForTimer(1);

		if (_menuPage == GUI_PAGE_SAVE && _mainMenuSavegameLabel > 0) {
			if (userWriteLabelRoutine(ks, leftMsClicked, rightMsClicked))
				break;
		} else {
			// Wait for any mouse button presses...
			waitForBannerInput(-1, ks, leftMsClicked, rightMsClicked, _menuPage != GUI_PAGE_MAIN);
		}

		if (leftMsClicked || rightMsClicked || _mouseWheelFlag) {
			curMouseX = _mouse.x;
			curMouseY = _mouse.y;
			clickedControl = getInternalGUIControlFromCoordinates(curMouseX, curMouseY);

			// Mouse wheel movement with cursor over the list box. If there are actual button
			// clicks, we ignore the wheel.
			if (!(leftMsClicked || rightMsClicked)) {
				// Unfortunately, there can be space (vertically) between the save slot controls,
				// so the mouse wheel will not catch if the cursor happens to be right at that pixel.
				// Thus, we also have to check for the GUI_CTRL_OUTER_BOX and the see if the cursor
				// is within the save list bounds.
				if (clickedControl == GUI_CTRL_OUTER_BOX && _internalGUIControls[GUI_CTRL_FIRST_SG].relativeCenterX != -1 &&
					_internalGUIControls[GUI_CTRL_LAST_SG].relativeCenterX != -1) {
						Common::Rect saveList(_internalGUIControls[GUI_CTRL_FIRST_SG].relativeCenterX,
							_internalGUIControls[GUI_CTRL_FIRST_SG].relativeCenterY,
							_internalGUIControls[GUI_CTRL_LAST_SG].xPos + 1,
							_internalGUIControls[GUI_CTRL_LAST_SG].yPos + 1);
						// Doesn't matter which slot we assign, we just want to trigger the next if-clause.
						clickedControl = saveList.contains(curMouseX, curMouseY) ? GUI_CTRL_FIRST_SG : -1;
				}

				if (clickedControl >= GUI_CTRL_FIRST_SG && clickedControl <= GUI_CTRL_LAST_SG) {
					if (_mouseWheelFlag == Common::EVENT_WHEELUP)
						clickedControl = (_internalGUIControls[GUI_CTRL_ARROW_UP_BUTTON].relativeCenterX != -1) ? GUI_CTRL_ARROW_UP_BUTTON : -1;
					else if (_mouseWheelFlag == Common::EVENT_WHEELDOWN)
						clickedControl = (_internalGUIControls[GUI_CTRL_ARROW_DOWN_BUTTON].relativeCenterX != -1) ? GUI_CTRL_ARROW_DOWN_BUTTON : -1;
				} else {
					clickedControl = -1;
				}
			}

			// Allow the user to drag the volume and text speed sliders horizontally;
			// this feature isn't available in the original interpreters (v6-v7),
			// but it makes the sliders a lot more comfortable to use, so there shouldn't be
			// any harm in this.
			while ((_game.version >= 6) && (_menuPage == GUI_PAGE_MAIN) && (_leftBtnPressed & 1)) {
				// Allow the engine to fetch new mouse states...
				waitForTimer(1);

				// Register one and only one control for the duration of this loop...
				if (heldControl == -1)
					heldControl = getInternalGUIControlFromCoordinates(curMouseX, curMouseY);

				// Choose among the available sliders, register the new mouse coordinates,
				// and execute the control operation...
				switch (heldControl) {
				case GUI_CTRL_MUSIC_SLIDER:
				case GUI_CTRL_SPEECH_SLIDER:
				case GUI_CTRL_SFX_SLIDER:
				case GUI_CTRL_TEXT_SPEED_SLIDER:
					curMouseX = _mouse.x;
					curMouseY = _mouse.y;
					executeMainMenuOperation(heldControl, curMouseX, curMouseY, hasLoadedState);
					break;
				default:
					break;
				}
			}

			heldControl = -1;

			clearClickedStatus();
			leftMsClicked = false;
			rightMsClicked = false;

			if (clickedControl != -1) {
				if (clickedControl < GUI_CTRL_FIRST_SG || clickedControl > GUI_CTRL_LAST_SG) {
					// Avoid highlighting the main container boxes :-)
					if (clickedControl != GUI_CTRL_OUTER_BOX && clickedControl != GUI_CTRL_INNER_BOX) {
						// Highlight the control
						drawInternalGUIControl(clickedControl, 1);
						ScummEngine::drawDirtyScreenParts();

						// Wait a little bit (the original waited 120 quarter frames, which feels like molasses).
						// We only perform this artificial wait for buttons; the original also did this for
						// sliders but it feels artificially slow, and we don't want that here :-)
						if (shouldHighlightLabelAndWait(clickedControl))
							waitForTimer(60);

						// Dehighlight it
						drawInternalGUIControl(clickedControl, 0);

						// Execute the operation pertaining the clicked control
						if (executeMainMenuOperation(clickedControl, curMouseX, curMouseY, hasLoadedState))
							break;
					}
				} else {
					int tmp = _mainMenuSavegameLabel;
					_mainMenuSavegameLabel = clickedControl;

					drawInternalGUIControl(tmp, 0);

					// For v4 we activated the first slot by default; deactivate it...
					if (_game.version == 4 && _game.id != GID_LOOM)
						drawInternalGUIControl(GUI_CTRL_FIRST_SG, 0);

					drawInternalGUIControl(_mainMenuSavegameLabel, 1);

					ScummEngine::drawDirtyScreenParts();
					_system->updateScreen();

					if (_menuPage == GUI_PAGE_LOAD && (_game.version != 4 || _game.id == GID_LOOM)) {
						if (executeMainMenuOperation(GUI_CTRL_OK_BUTTON, curMouseX, curMouseY, hasLoadedState))
							break;
					}
				}
			}
		}

		if (shouldQuit() && !_quitByGUIPrompt) {
			clearClickedStatus();
			if (executeMainMenuOperation(GUI_CTRL_QUIT_BUTTON, 0, 0, hasLoadedState) || _quitByGUIPrompt)
				break;
		}
	}

	_mainMenuIsActive = false;

	if (_game.version > 6)
		_completeScreenRedraw = true;

	// Restore the old cursor state only if we're not loading a game...
	if (_saveScriptParam != GAME_PROPER_LOAD && _saveLoadFlag != 2) {
		restoreCursorPostMenu();
	} else if (_saveLoadFlag == 2) {
		_cursor.state = 0;
	}

	// Run the exit savescreen script, if available
	if (_saveScriptParam != 0 || _game.version == 6) {
		args[0] = _saveScriptParam;
		if (VAR_SAVELOAD_SCRIPT2 != 0xFF) {
			runScript(VAR(VAR_SAVELOAD_SCRIPT2), 0, 0, args);
			_saveScriptParam = 0;
		}
	}

	// A little bit of hackery: since we handle the main loop a little bit
	// differently (basically we start from a different position, but the order
	// remains the same), we call CHARSET_1() here to refresh the dialog texts
	// immediately and avoid getting a frame in which their color is wrong...
	if (_game.version == 7)
		CHARSET_1();

	if (_game.version < 7 && !hasLoadedState) {
		restoreSurfacesPostGUI();

		// Restore shake effect
		setShake(_shakeTempSavedState);
	} else {
		free(_tempTextSurface);
		_tempTextSurface = nullptr;
		free(_tempMainSurface);
		_tempMainSurface = nullptr;
		free(_tempVerbSurface);
		_tempVerbSurface = nullptr;
		free(_curGrabbedCursor);
		_curGrabbedCursor = nullptr;
	}

	_savegameThumbnail.free();

	// Resume the engine
	pt.clear();
	clearClickedStatus();
}

bool ScummEngine::executeMainMenuOperation(int op, int mouseX, int mouseY, bool &hasLoadedState) {
	char saveScreenTitle[512];
	Common::String formattedString;
	int curSlot;
	bool isLoomVga = (_game.id == GID_LOOM && _game.version == 4);
	size_t labelSkip = (_game.version == 4 && _game.id != GID_LOOM) ? 0 : 4;
	bool rtlRequest = getEventManager()->shouldReturnToLauncher();

	switch (op) {
	case GUI_CTRL_SAVE_BUTTON:
		_mainMenuSavegameLabel = 0;
		fillSavegameLabels();
		_menuPage = GUI_PAGE_SAVE;
		setUpMainMenuControls();
		drawMainMenuControls();
		ScummEngine::drawDirtyScreenParts();
		break;
	case GUI_CTRL_LOAD_BUTTON:
		_mainMenuSavegameLabel = 0;
		fillSavegameLabels();
		_menuPage = GUI_PAGE_LOAD;
		setUpMainMenuControls();
		drawMainMenuControls();
		ScummEngine::drawDirtyScreenParts();
		break;
	case GUI_CTRL_PLAY_BUTTON:
		return true;
	case GUI_CTRL_QUIT_BUTTON:
		getEventManager()->resetQuit();
		getEventManager()->resetReturnToLauncher();
		queryQuit(rtlRequest);
		if (_game.version == 7)
			return true;
		break;
	case GUI_CTRL_OK_BUTTON:
		if (_menuPage == GUI_PAGE_SAVE) {
			// We check for an empty label since v4 might generate that...
			if (_mainMenuSavegameLabel > 0 && !_savegameNames[_mainMenuSavegameLabel - 1].substr(labelSkip).empty()) {
				convertMessageToString((const byte *)getGUIString(gsSaving), (byte *)saveScreenTitle, sizeof(saveScreenTitle));
				formattedString = Common::String::format(saveScreenTitle, _savegameNames[_mainMenuSavegameLabel - 1].substr(labelSkip).c_str());
				drawMainMenuTitle(formattedString.c_str());
				ScummEngine::drawDirtyScreenParts();
				_system->updateScreen();

				waitForTimer(60);

				Common::String dummyString;
				_saveLoadDescription = _savegameNames[_mainMenuSavegameLabel - 1].substr(labelSkip);

				curSlot = _mainMenuSavegameLabel + (isLoomVga ? _firstSaveStateOfList : _curDisplayedSaveSlotPage * 9);
				if (canWriteGame(curSlot)) {
					restoreCursorPostMenu();
					if (saveState(curSlot - 1, false, dummyString)) {
						saveCursorPreMenu();
						_saveScriptParam = GAME_PROPER_SAVE;
						drawMainMenuControls();
						return true;
					}
				} else {
					convertMessageToString((const byte *)getGUIString(gsGameNotSaved), (byte *)saveScreenTitle, sizeof(saveScreenTitle));
					if (_game.id == GID_DIG) {
						showBannerAndPause(1, -1, saveScreenTitle);
						drawMainMenuControls();
					} else {
						drawMainMenuTitle(saveScreenTitle);
					}

					ScummEngine::drawDirtyScreenParts();
				}
			} else {
				convertMessageToString((const byte *)getGUIString(gsMustName), (byte *)saveScreenTitle, sizeof(saveScreenTitle));
				drawMainMenuTitle(saveScreenTitle);
				ScummEngine::drawDirtyScreenParts();
			}

			// The original, after doing the above, immediately redraws the menus,
			// effectively overwriting the title we've just changed.
			// In a DOS machine one could effectively see the changed title for a
			// fraction of time, before reverting to the normale one.
			// We, on the other hand, wouldn't be able to see changed title at all,
			// so let's comment the following instruction :-)
			//
			//drawMainMenuControls();

		} else if (_menuPage == GUI_PAGE_LOAD) {
			if (_mainMenuSavegameLabel > 0) {
				convertMessageToString((const byte *)getGUIString(gsLoading), (byte *)saveScreenTitle, sizeof(saveScreenTitle));
				formattedString = Common::String::format(saveScreenTitle, _savegameNames[_mainMenuSavegameLabel - 1].substr(labelSkip).c_str());

				if (_savegameNames[_mainMenuSavegameLabel - 1].size() == labelSkip) {
					drawMainMenuControls();
					ScummEngine::drawDirtyScreenParts();
					break;
				}

				drawMainMenuTitle(formattedString.c_str());
				ScummEngine::drawDirtyScreenParts();
				_system->updateScreen();

				waitForTimer(60);

				if (isSmushActive()) {
					handleLoadDuringSmush();
					return true;
				}

				if ((_game.version != 4 || _game.id != GID_LOOM) && _game.version < 7) {
					_postGUICharMask = true;
				}

				curSlot = _mainMenuSavegameLabel + (isLoomVga ? _firstSaveStateOfList : _curDisplayedSaveSlotPage * 9);
				if (loadState(curSlot - 1, false)) {
					hasLoadedState = true;

					if (!_spooledMusicIsToBeEnabled)
						_imuseDigital->diMUSEDisableSpooledMusic();

					setSkipVideo(0);
					_saveScriptParam = GAME_PROPER_LOAD;
					return true;
				} else {
					convertMessageToString((const byte *)getGUIString(gsGameNotLoaded), (byte *)saveScreenTitle, sizeof(saveScreenTitle));
					if (_game.id == GID_DIG) {
						showBannerAndPause(1, -1, saveScreenTitle);
						drawMainMenuControls();
					} else {
						drawMainMenuTitle(saveScreenTitle);
					}

					ScummEngine::drawDirtyScreenParts();
				}

				// See the comment for the Save control
				//
				//drawMainMenuControls();
			}
		}

		break;
	case GUI_CTRL_CANCEL_BUTTON:
		_menuPage = GUI_PAGE_MAIN;
		setUpMainMenuControls();
		drawMainMenuControls();

		if (_game.platform == Common::kPlatformAmiga) {
			convertMessageToString((const byte *)getGUIString(gsInsertSaveDisk), (byte *)saveScreenTitle, sizeof(saveScreenTitle));
			drawMainMenuTitle(saveScreenTitle);
		} else if (_game.version > 4 && _game.id != GID_MONKEY2 && _game.id != GID_MONKEY) {
			convertMessageToString((const byte *)getGUIString(gsTitle), (byte *)saveScreenTitle, sizeof(saveScreenTitle));
			drawMainMenuTitle(saveScreenTitle);
		}

		updateMainMenuControls();
		ScummEngine::drawDirtyScreenParts();
		break;
	case GUI_CTRL_ARROW_UP_BUTTON:
	case GUI_CTRL_ARROW_DOWN_BUTTON:
		if (_menuPage != GUI_PAGE_MAIN) {
			if (!isLoomVga) {
				if (op == GUI_CTRL_ARROW_UP_BUTTON) {
					_curDisplayedSaveSlotPage--;
				} else {
					_curDisplayedSaveSlotPage++;
				}

				_curDisplayedSaveSlotPage = CLIP<int>(_curDisplayedSaveSlotPage, 0, 10);
			} else {
				// LOOM VGA uses its own system to scroll the savegame list, based
				// on high or low the arrow button is pressed...
				if (op == GUI_CTRL_ARROW_UP_BUTTON) {
					_firstSaveStateOfList -= ((_virtscr[kMainVirtScreen].h / 2) - mouseY + _virtscr[kMainVirtScreen].topline + 54) / 14;
				} else {
					_firstSaveStateOfList += (mouseY - (_virtscr[kMainVirtScreen].h / 2) - _virtscr[kMainVirtScreen].topline + 45) / 14;
				}

				_firstSaveStateOfList = CLIP<int>(_firstSaveStateOfList, 0, 90);
			}

			_mainMenuSavegameLabel = 0;
			fillSavegameLabels();

			// Update the control labels with the newly changed savegame names
			for (int i = GUI_CTRL_FIRST_SG; i <= GUI_CTRL_LAST_SG; i++)
				_internalGUIControls[i].label = _savegameNames[i - 1];

			drawMainMenuControls();
			ScummEngine::drawDirtyScreenParts();
		} else {
			drawInternalGUIControl(GUI_CTRL_ARROW_UP_BUTTON, 0);
			drawInternalGUIControl(GUI_CTRL_ARROW_DOWN_BUTTON, 0);
			ScummEngine::drawDirtyScreenParts();
		}

		break;
	case GUI_CTRL_PATH_BUTTON:
		// This apparently should't do anything
		break;
	case GUI_CTRL_MUSIC_SLIDER:
		setMusicVolume(((mouseX - (_game.version == 7 ? 111 : 105)) << 7) / 87);
		updateMainMenuControls();
		ScummEngine::drawDirtyScreenParts();
		break;
	case GUI_CTRL_SPEECH_SLIDER:
		setSpeechVolume(((mouseX - (_game.version == 7 ? 111 : 105)) << 7) / 87);
		updateMainMenuControls();
		ScummEngine::drawDirtyScreenParts();
		break;
	case GUI_CTRL_SFX_SLIDER:
		setSFXVolume(((mouseX - (_game.version == 7 ? 111 : 105)) << 7) / 87);
		updateMainMenuControls();
		ScummEngine::drawDirtyScreenParts();
		break;
	case GUI_CTRL_TEXT_SPEED_SLIDER:
		_defaultTextSpeed = CLIP<int>((mouseX - (_game.version == 7 ? 108 : 102)) / 9, 0, 9);
		ConfMan.setInt("original_gui_text_speed", _defaultTextSpeed);
		setTalkSpeed(_defaultTextSpeed);
		syncSoundSettings();
		ConfMan.flushToDisk();
		updateMainMenuControls();
		ScummEngine::drawDirtyScreenParts();
		break;
	case GUI_CTRL_DISPLAY_TEXT_CHECKBOX:
		toggleVoiceMode();
		updateMainMenuControls();
		ScummEngine::drawDirtyScreenParts();
		break;
	case GUI_CTRL_SPOOLED_MUSIC_CHECKBOX:
		_spooledMusicIsToBeEnabled ^= 1;

		// Just for safety, this should never be nullptr...
		if (_imuseDigital) {
			if (_spooledMusicIsToBeEnabled) {
				_imuseDigital->diMUSEEnableSpooledMusic();
			} else {
				_imuseDigital->diMUSEDisableSpooledMusic();
			}
		}
		updateMainMenuControls();
		ScummEngine::drawDirtyScreenParts();
		break;
	default:
		break;
	}

	return false;
}

void ScummEngine_v4::setUpMainMenuControls() {
	if (_game.id == GID_LOOM && _game.version == 4) {
		ScummEngine::setUpMainMenuControls();
		return;
	}

	int yConstant = _virtscr[kMainVirtScreen].topline + (_virtscr[kMainVirtScreen].h / 2);

	for (int i = 0; i < ARRAYSIZE(_internalGUIControls); i++) {
		_internalGUIControls[i].relativeCenterX = -1;
	}

	// Outer box
	setUpInternalGUIControl(GUI_CTRL_OUTER_BOX,
		getBannerColor(6),
		getBannerColor(6),
		getBannerColor(7),
		getBannerColor(7),
		getBannerColor(7),
		getBannerColor(7),
		getBannerColor(7),
		getBannerColor(6),
		20,
		yConstant - 58,
		300,
		yConstant + 58,
		_emptyMsg, 0, 0);

	if (_menuPage == GUI_PAGE_MAIN) {
		// Save button
		setUpInternalGUIControl(GUI_CTRL_SAVE_BUTTON,
			getBannerColor(10),
			getBannerColor(11),
			getBannerColor(9),
			getBannerColor(9),
			getBannerColor(9),
			getBannerColor(9),
			getBannerColor(12),
			getBannerColor(13),
			242,
			yConstant - 25,
			-50,
			-12,
			getGUIString(gsSave), 1, 1);

		// Load button
		setUpInternalGUIControl(GUI_CTRL_LOAD_BUTTON,
			getBannerColor(10),
			getBannerColor(11),
			getBannerColor(9),
			getBannerColor(9),
			getBannerColor(9),
			getBannerColor(9),
			getBannerColor(12),
			getBannerColor(13),
			242,
			yConstant - 11,
			-50,
			-12,
			getGUIString(gsLoad), 1, 1);

		// Play button
		setUpInternalGUIControl(GUI_CTRL_PLAY_BUTTON,
			getBannerColor(10),
			getBannerColor(11),
			getBannerColor(9),
			getBannerColor(9),
			getBannerColor(9),
			getBannerColor(9),
			getBannerColor(12),
			getBannerColor(13),
			242,
			yConstant + 3,
			-50,
			-12,
			getGUIString(gsPlay), 1, 1);

		// Quit button
		setUpInternalGUIControl(GUI_CTRL_QUIT_BUTTON,
			getBannerColor(10),
			getBannerColor(11),
			getBannerColor(9),
			getBannerColor(9),
			getBannerColor(9),
			getBannerColor(9),
			getBannerColor(12),
			getBannerColor(13),
			242,
			yConstant + 17,
			-50,
			-12,
			getGUIString(gsQuit), 1, 1);
	}

	if (_menuPage == GUI_PAGE_SAVE || _menuPage == GUI_PAGE_LOAD) {
		// OK button
		setUpInternalGUIControl(GUI_CTRL_OK_BUTTON,
			getBannerColor(10),
			getBannerColor(11),
			getBannerColor(9),
			getBannerColor(9),
			getBannerColor(9),
			getBannerColor(9),
			getBannerColor(12),
			getBannerColor(13),
			242,
			yConstant - 11,
			-50,
			-12,
			getGUIString(gsOK), 1, 1);

		// Cancel button
		setUpInternalGUIControl(GUI_CTRL_CANCEL_BUTTON,
			getBannerColor(10),
			getBannerColor(11),
			getBannerColor(9),
			getBannerColor(9),
			getBannerColor(9),
			getBannerColor(9),
			getBannerColor(12),
			getBannerColor(13),
			242,
			yConstant + 3,
			-50,
			-12,
			getGUIString(gsCancel), 1, 1);

		// Savegame names
		for (int i = GUI_CTRL_FIRST_SG, j = 11; i <= GUI_CTRL_LAST_SG; i++, j += 11) {
			setUpInternalGUIControl(i,
				getBannerColor(15),
				getBannerColor(16),
				getBannerColor(14),
				getBannerColor(14),
				getBannerColor(14),
				getBannerColor(14),
				getBannerColor(17),
				getBannerColor(18),
				28,
				yConstant - 56 + j,
				-206,
				-9,
				_savegameNames[i - 1].c_str(), 0, 0);
		}
	}
}

void ScummEngine::setUpMainMenuControls() {
	int yConstant;
	bool isLoomVGA = (_game.id == GID_LOOM && _game.version == 4);

	yConstant = _virtscr[kMainVirtScreen].topline + (_virtscr[kMainVirtScreen].h / 2);

	for (int i = 0; i < ARRAYSIZE(_internalGUIControls); i++) {
		_internalGUIControls[i].relativeCenterX = -1;
	}

	// Outer box
	setUpInternalGUIControl(GUI_CTRL_OUTER_BOX,
		isLoomVGA ? 7  : getBannerColor(4),
		isLoomVGA ? 0  : getBannerColor(2),
		isLoomVGA ? 15 : getBannerColor(13),
		isLoomVGA ? 8  : getBannerColor(14),
		isLoomVGA ? 15 : getBannerColor(15),
		isLoomVGA ? 8  : getBannerColor(16),
		isLoomVGA ? 14 : getBannerColor(6),
		isLoomVGA ? 1  : getBannerColor(4),
		20,
		yConstant - 60,
		300,
		((yConstant + 60) < 0 ? -120 : yConstant + 60),
		_emptyMsg, 1, 1);

	// Inner box
	setUpInternalGUIControl(GUI_CTRL_INNER_BOX,
		isLoomVGA ? 7  : getBannerColor(4),
		isLoomVGA ? 0  : getBannerColor(5),
		isLoomVGA ? 8  : getBannerColor(18),
		isLoomVGA ? 15 : getBannerColor(17),
		isLoomVGA ? 8  : getBannerColor(20),
		isLoomVGA ? 15 : getBannerColor(19),
		isLoomVGA ? 14 : getBannerColor(6),
		isLoomVGA ? 1  : getBannerColor(7),
		26,
		yConstant - 47,
		212 - (isLoomVGA ? 10 : 0),
		yConstant - 47 + 102,
		_emptyMsg, 1, 1);

	if (_menuPage == GUI_PAGE_MAIN) {
		// Save button
		setUpInternalGUIControl(GUI_CTRL_SAVE_BUTTON,
			isLoomVGA ? 7  : getBannerColor(4),
			isLoomVGA ? 0  : getBannerColor(5),
			isLoomVGA ? 15 : getBannerColor(17),
			isLoomVGA ? 8  : getBannerColor(18),
			isLoomVGA ? 15 : getBannerColor(19),
			isLoomVGA ? 8  : getBannerColor(20),
			isLoomVGA ? 14 : getBannerColor(6),
			isLoomVGA ? 1  : getBannerColor(7),
			242 - (isLoomVGA ? 10 : 0),
			yConstant - 23,
			292,
			yConstant - 23 + 12,
			getGUIString(gsSave), 1, 1);

		// Load button
		setUpInternalGUIControl(GUI_CTRL_LOAD_BUTTON,
			isLoomVGA ? 7  : getBannerColor(4),
			isLoomVGA ? 0  : getBannerColor(5),
			isLoomVGA ? 15 : getBannerColor(17),
			isLoomVGA ? 8  : getBannerColor(18),
			isLoomVGA ? 15 : getBannerColor(19),
			isLoomVGA ? 8  : getBannerColor(20),
			isLoomVGA ? 14 : getBannerColor(6),
			isLoomVGA ? 1  : getBannerColor(7),
			242 - (isLoomVGA ? 10 : 0),
			yConstant - 8,
			292,
			yConstant - 8 + 12,
			getGUIString(gsLoad), 1, 1);

		// Play button
		setUpInternalGUIControl(GUI_CTRL_PLAY_BUTTON,
			isLoomVGA ? 7  : getBannerColor(4),
			isLoomVGA ? 0  : getBannerColor(5),
			isLoomVGA ? 15 : getBannerColor(17),
			isLoomVGA ? 8  : getBannerColor(18),
			isLoomVGA ? 15 : getBannerColor(19),
			isLoomVGA ? 8  : getBannerColor(20),
			isLoomVGA ? 14 : getBannerColor(6),
			isLoomVGA ? 1  : getBannerColor(7),
			242 - (isLoomVGA ? 10 : 0),
			yConstant + 7,
			292,
			yConstant + 19,
			getGUIString(gsPlay), 1, 1);

		// Quit button
		setUpInternalGUIControl(GUI_CTRL_QUIT_BUTTON,
			isLoomVGA ? 7  : getBannerColor(4),
			isLoomVGA ? 0  : getBannerColor(5),
			isLoomVGA ? 15 : getBannerColor(17),
			isLoomVGA ? 8  : getBannerColor(18),
			isLoomVGA ? 15 : getBannerColor(19),
			isLoomVGA ? 8  : getBannerColor(20),
			isLoomVGA ? 14 : getBannerColor(6),
			isLoomVGA ? 1  : getBannerColor(7),
			242 - (isLoomVGA ? 10 : 0),
			yConstant + 22,
			292,
			yConstant + 34,
			getGUIString(gsQuit), 1, 1);
	}

	if (_menuPage != GUI_PAGE_MAIN || !(_game.id == GID_MONKEY2 || _game.id == GID_MONKEY)) {
		// Arrow up button
		setUpInternalGUIControl(GUI_CTRL_ARROW_UP_BUTTON,
			isLoomVGA ? 7  : getBannerColor(9),
			isLoomVGA ? 0  : getBannerColor(10),
			isLoomVGA ? 15 : getBannerColor(17),
			isLoomVGA ? 8  : getBannerColor(18),
			isLoomVGA ? 15 : getBannerColor(19),
			isLoomVGA ? 8  : getBannerColor(20),
			isLoomVGA ? 14 : getBannerColor(11),
			isLoomVGA ? 1  : getBannerColor(12),
			216 - (isLoomVGA ? 10 : 0),
			yConstant - 43,
			232 - (isLoomVGA ? 10 : 0),
			yConstant - 43 + 47,
			_arrowUp, 1, 1);

		// Arrow down button
		setUpInternalGUIControl(GUI_CTRL_ARROW_DOWN_BUTTON,
			isLoomVGA ? 7  : getBannerColor(9),
			isLoomVGA ? 0  : getBannerColor(10),
			isLoomVGA ? 15 : getBannerColor(17),
			isLoomVGA ? 8  : getBannerColor(18),
			isLoomVGA ? 15 : getBannerColor(19),
			isLoomVGA ? 8  : getBannerColor(20),
			isLoomVGA ? 14 : getBannerColor(11),
			isLoomVGA ? 1  : getBannerColor(12),
			216 - (isLoomVGA ? 10 : 0),
			yConstant + 7,
			232 - (isLoomVGA ? 10 : 0),
			yConstant + 52,
			_arrowDown, 1, 1);
	}

	if (_menuPage == GUI_PAGE_SAVE || _menuPage == GUI_PAGE_LOAD) {
		if (_menuPage == GUI_PAGE_SAVE) {
			// OK button
			setUpInternalGUIControl(GUI_CTRL_OK_BUTTON,
				isLoomVGA ? 7  : getBannerColor(4),
				isLoomVGA ? 0  : getBannerColor(5),
				isLoomVGA ? 15 : getBannerColor(17),
				isLoomVGA ? 8  : getBannerColor(18),
				isLoomVGA ? 15 : getBannerColor(19),
				isLoomVGA ? 8  : getBannerColor(20),
				isLoomVGA ? 14 : getBannerColor(6),
				isLoomVGA ? 1  : getBannerColor(7),
				242 - (isLoomVGA ? 10 : 0),
				yConstant - 8,
				292,
				yConstant - 8 + 12,
				getGUIString(gsOK), 1, 1);
		}

		// Cancel button
		setUpInternalGUIControl(GUI_CTRL_CANCEL_BUTTON,
			isLoomVGA ? 7  : getBannerColor(4),
			isLoomVGA ? 0  : getBannerColor(5),
			isLoomVGA ? 15 : getBannerColor(17),
			isLoomVGA ? 8  : getBannerColor(18),
			isLoomVGA ? 15 : getBannerColor(19),
			isLoomVGA ? 8  : getBannerColor(20),
			isLoomVGA ? 14 : getBannerColor(6),
			isLoomVGA ? 1  : getBannerColor(7),
			242 - (isLoomVGA ? 10 : 0),
			(_menuPage == GUI_PAGE_LOAD ? yConstant - 1 : yConstant + 7),
			292,
			(_menuPage == GUI_PAGE_LOAD ? yConstant - 1 : yConstant + 7) + 12,
			getGUIString(gsCancel), 1, 1);

		// Savegame names
		for (int i = GUI_CTRL_FIRST_SG, j = 0; i <= GUI_CTRL_LAST_SG; i++, j += 11) {
			setUpInternalGUIControl(i,
				isLoomVGA ? 7  : getBannerColor(9),
				isLoomVGA ? 0  : getBannerColor(10),
				isLoomVGA ? 7  : getBannerColor(4),
				isLoomVGA ? 7  : getBannerColor(4),
				isLoomVGA ? 7  : getBannerColor(4),
				isLoomVGA ? 7  : getBannerColor(4),
				isLoomVGA ? 14 : getBannerColor(11),
				isLoomVGA ? 1  : getBannerColor(12),
				28,
				yConstant - 45 + j,
				210 - (isLoomVGA ? 10 : 0),
				-9,
				_savegameNames[i - 1].c_str(), 0, 0);
		}
	}
}

void ScummEngine_v6::setUpMainMenuControls() {
	int yConstantV6;

	// V7 auxiliary constants
	int cid = _charset->getCurID();
	_charset->setCurID(1);
	int lh = getGUIStringHeight("ABC \x80\x78 \xb0\x78");
	_charset->setCurID(cid);
	int yCntr = _screenHeight / 2;
	int calculatedHeight = (110 + lh) / 2;
	int yOffset = _useCJKMode ? 0 : 1;

	// V6 auxiliary constant
	yConstantV6 = _virtscr[kMainVirtScreen].topline + (_virtscr[kMainVirtScreen].h / 2);

	for (int i = 0; i < ARRAYSIZE(_internalGUIControls); i++) {
		_internalGUIControls[i].relativeCenterX = -1;
	}

	// Outer box
	setUpInternalGUIControl(GUI_CTRL_OUTER_BOX,
		getBannerColor(4),
		getBannerColor(2),
		getBannerColor(13),
		getBannerColor(14),
		getBannerColor(15),
		getBannerColor(16),
		getBannerColor(6),
		getBannerColor(4),
		(_game.version == 7 ? 16 : 20),
		(_game.version == 7 ? yCntr - calculatedHeight - yOffset : yConstantV6 - 60),
		(_game.version == 7 ? 303 : 300),
		(_game.version == 7 ? yCntr + calculatedHeight + yOffset : yConstantV6 + 60),
		_emptyMsg, 1, 1);

	// Inner box
	setUpInternalGUIControl(GUI_CTRL_INNER_BOX,
		getBannerColor(4),
		getBannerColor(5),
		getBannerColor(18),
		getBannerColor(17),
		getBannerColor(20),
		getBannerColor(19),
		getBannerColor(6),
		getBannerColor(7),
		(_game.version == 7 ? 22 : 26),
		(_game.version == 7 ? yCntr - calculatedHeight + lh + 4 : yConstantV6 - 47),
		(_game.version == 7 ? -183 : -176),
		-102,
		_emptyMsg, 1, 1);

	if (_menuPage == GUI_PAGE_MAIN) {
		if (_game.id == GID_FT) {
			// Spooled music checkbox
			setUpInternalGUIControl(GUI_CTRL_SPOOLED_MUSIC_CHECKBOX,
				getBannerColor(9),
				getBannerColor(10),
				getBannerColor(18),
				getBannerColor(17),
				getBannerColor(20),
				getBannerColor(19),
				getBannerColor(11),
				getBannerColor(12),
				108,
				57,
				-12,
				-12,
				_uncheckedBox, 1, 1);

			// Music volume slider
			setUpInternalGUIControl(GUI_CTRL_MUSIC_SLIDER,
				getBannerColor(9),
				getBannerColor(10),
				getBannerColor(18),
				getBannerColor(17),
				getBannerColor(20),
				getBannerColor(19),
				getBannerColor(10),
				getBannerColor(12),
				108,
				71,
				-90,
				-12,
				_uncheckedBox, 1, 1);

			// Speech volume slider
			setUpInternalGUIControl(GUI_CTRL_SPEECH_SLIDER,
				getBannerColor(9),
				getBannerColor(10),
				getBannerColor(18),
				getBannerColor(17),
				getBannerColor(20),
				getBannerColor(19),
				getBannerColor(10),
				getBannerColor(12),
				108,
				85,
				-90,
				-12,
				_uncheckedBox, 1, 1);

			// SFX volume slider
			setUpInternalGUIControl(GUI_CTRL_SFX_SLIDER,
				getBannerColor(9),
				getBannerColor(10),
				getBannerColor(18),
				getBannerColor(17),
				getBannerColor(20),
				getBannerColor(19),
				getBannerColor(10),
				getBannerColor(12),
				108,
				99,
				-90,
				-12,
				_uncheckedBox, 1, 1);
		} else if (_game.variant && strcmp(_game.variant, "Floppy")) {
			// Music volume slider
			setUpInternalGUIControl(GUI_CTRL_MUSIC_SLIDER,
				getBannerColor(9),
				getBannerColor(10),
				getBannerColor(18),
				getBannerColor(17),
				getBannerColor(20),
				getBannerColor(19),
				getBannerColor(10),
				getBannerColor(12),
				(_game.version == 7 ? 108 : 102),
				(_game.version == 7 ? yCntr - calculatedHeight - yOffset + 25 : yConstantV6 - 39),
				-90,
				-12,
				_uncheckedBox, 1, 1);

			// Speech volume slider
			setUpInternalGUIControl(GUI_CTRL_SPEECH_SLIDER,
				getBannerColor(9),
				getBannerColor(10),
				getBannerColor(18),
				getBannerColor(17),
				getBannerColor(20),
				getBannerColor(19),
				getBannerColor(10),
				getBannerColor(12),
				(_game.version == 7 ? 108 : 102),
				(_game.version == 7 ? yCntr - calculatedHeight - yOffset + 43 : yConstantV6 - 25),
				-90,
				-12,
				_uncheckedBox, 1, 1);

			// SFX volume slider
			setUpInternalGUIControl(GUI_CTRL_SFX_SLIDER,
				getBannerColor(9),
				getBannerColor(10),
				getBannerColor(18),
				getBannerColor(17),
				getBannerColor(20),
				getBannerColor(19),
				getBannerColor(10),
				getBannerColor(12),
				(_game.version == 7 ? 108 : 102),
				(_game.version == 7 ? yCntr - calculatedHeight - yOffset + 61 : yConstantV6 - 11),
				-90,
				-12,
				_uncheckedBox, 1, 1);
		}

		if (_game.variant && strcmp(_game.variant, "Floppy")) {
			// Display text checkbox
			setUpInternalGUIControl(GUI_CTRL_DISPLAY_TEXT_CHECKBOX,
				getBannerColor(9),
				getBannerColor(10),
				getBannerColor(18),
				getBannerColor(17),
				getBannerColor(20),
				getBannerColor(19),
				getBannerColor(11),
				getBannerColor(12),
				(_game.version == 7 ? 108 : 102),
				(_game.version == 7 ? yCntr - calculatedHeight - yOffset + 85 : yConstantV6 + 17),
				-12,
				-12,
				_uncheckedBox, 1, 1);

			// Text speed slider
			setUpInternalGUIControl(GUI_CTRL_TEXT_SPEED_SLIDER,
				getBannerColor(9),
				getBannerColor(10),
				getBannerColor(18),
				getBannerColor(17),
				getBannerColor(20),
				getBannerColor(19),
				getBannerColor(10),
				getBannerColor(12),
				(_game.version == 7 ? 108 : 102),
				(_game.version == 7 ? yCntr - calculatedHeight - yOffset + 99 : yConstantV6 + 31),
				-90,
				-(lh + 4),
				_uncheckedBox, 1, 1);
		}

		// Save button
		setUpInternalGUIControl(GUI_CTRL_SAVE_BUTTON,
			getBannerColor(4),
			getBannerColor(5),
			getBannerColor(17),
			getBannerColor(18),
			getBannerColor(19),
			getBannerColor(20),
			getBannerColor(6),
			getBannerColor(7),
			(_game.version == 7 ? 235 : 232),
			(_game.version == 7 ? yCntr - calculatedHeight + yOffset * 6 + 30 : yConstantV6 - 23),
			-60,
			-(lh + 4),
			getGUIString(gsSave), 1, 1);

		// Load button
		setUpInternalGUIControl(GUI_CTRL_LOAD_BUTTON,
			getBannerColor(4),
			getBannerColor(5),
			getBannerColor(17),
			getBannerColor(18),
			getBannerColor(19),
			getBannerColor(20),
			getBannerColor(6),
			getBannerColor(7),
			(_game.version == 7 ? 235 : 232),
			(_game.version == 7 ? yCntr - calculatedHeight + lh  + yOffset * 6 + 37 : yConstantV6 - 8),
			-60,
			-(lh + 4),
			getGUIString(gsLoad), 1, 1);

		// Play button
		setUpInternalGUIControl(GUI_CTRL_PLAY_BUTTON,
			getBannerColor(4),
			getBannerColor(5),
			getBannerColor(17),
			getBannerColor(18),
			getBannerColor(19),
			getBannerColor(20),
			getBannerColor(6),
			getBannerColor(7),
			(_game.version == 7 ? 235 : 232),
			(_game.version == 7 ? yCntr - calculatedHeight + lh * 2  + yOffset * 6 + 44 : yConstantV6 + 7),
			-60,
			-(lh + 4),
			getGUIString(gsPlay), 1, 1);

		// Quit button
		setUpInternalGUIControl(GUI_CTRL_QUIT_BUTTON,
			getBannerColor(4),
			getBannerColor(5),
			getBannerColor(17),
			getBannerColor(18),
			getBannerColor(19),
			getBannerColor(20),
			getBannerColor(6),
			getBannerColor(7),
			(_game.version == 7 ? 235 : 232),
			(_game.version == 7 ? yCntr - calculatedHeight + lh * 3  + yOffset * 6 + 51 : yConstantV6 + 22),
			-60,
			-(lh + 4),
			getGUIString(gsQuit), 1, 1);
	}

	if (_game.version == 6 || _menuPage != GUI_PAGE_MAIN) {
		// Arrow up button
		setUpInternalGUIControl(GUI_CTRL_ARROW_UP_BUTTON,
			getBannerColor(9),
			getBannerColor(10),
			getBannerColor(17),
			getBannerColor(18),
			getBannerColor(19),
			getBannerColor(20),
			getBannerColor(11),
			getBannerColor(12),
			(_game.version == 7 ? 209 : 206),
			(_game.version == 7 ? yCntr - calculatedHeight + lh + 8: yConstantV6 - 43),
			-16,
			-47,
			_arrowUp, 1, 1);

		// Arrow down button
		setUpInternalGUIControl(GUI_CTRL_ARROW_DOWN_BUTTON,
			getBannerColor(9),
			getBannerColor(10),
			getBannerColor(17),
			getBannerColor(18),
			getBannerColor(19),
			getBannerColor(20),
			getBannerColor(11),
			getBannerColor(12),
			(_game.version == 7 ? 209 : 206),
			(_game.version == 7 ? yCntr - calculatedHeight + lh + 58 : yConstantV6 + 7),
			-16,
			-45,
			_arrowDown, 1, 1);
	}

	if (_menuPage == GUI_PAGE_SAVE || _menuPage == GUI_PAGE_LOAD) {
		if (_menuPage == GUI_PAGE_SAVE) {
			// OK button
			setUpInternalGUIControl(GUI_CTRL_OK_BUTTON,
				getBannerColor(4),
				getBannerColor(5),
				getBannerColor(17),
				getBannerColor(18),
				getBannerColor(19),
				getBannerColor(20),
				getBannerColor(6),
				getBannerColor(7),
				(_game.version == 7 ? 235 : 232),
				(_game.version == 7 ? yCntr - calculatedHeight + lh + yOffset * 6 + 37: yConstantV6 - 8),
				-60,
				-(lh + 4),
				getGUIString(gsOK), 1, 1);
		}

		// Cancel button
		int cancelButtonAnchorY;
		if (_menuPage == GUI_PAGE_LOAD) {
			if (_game.version == 7) {
				cancelButtonAnchorY = yCntr - calculatedHeight + (lh + 7) / 2 + lh + yOffset * 6 + 37;
			} else {
				cancelButtonAnchorY = yConstantV6 - 1;
			}
		} else {
			if (_game.version == 7) {
				cancelButtonAnchorY = yCntr - calculatedHeight + lh * 2 + yOffset * 6 + 44;
			} else {
				cancelButtonAnchorY = yConstantV6 + 7;
			}
		}
		setUpInternalGUIControl(GUI_CTRL_CANCEL_BUTTON,
			getBannerColor(4),
			getBannerColor(5),
			getBannerColor(17),
			getBannerColor(18),
			getBannerColor(19),
			getBannerColor(20),
			getBannerColor(6),
			getBannerColor(7),
			(_game.version == 7 ? 235 : 232),
			cancelButtonAnchorY,
			-60,
			-(lh + 4),
			getGUIString(gsCancel), 1, 1);

		// Savegame names
		for (int i = GUI_CTRL_FIRST_SG, j = 11; i <= GUI_CTRL_LAST_SG; i++, j += 11) {
			setUpInternalGUIControl(i,
				getBannerColor(9),
				getBannerColor(10),
				getBannerColor(4),
				getBannerColor(4),
				getBannerColor(4),
				getBannerColor(4),
				getBannerColor(11),
				getBannerColor(12),
				(_game.version == 7 ? 24 : 28),
				(_game.version == 7 ? yCntr - calculatedHeight + j + lh - 5 : yConstantV6 + j - 56),
				(_game.version == 7 ? -179 : -172),
				-9,
				_savegameNames[i - 1].c_str(), 0, 0);
		}
	}
}

void ScummEngine::drawMainMenuControls() {
	char namePrompt[256];
	char loadPrompt[256];
	char insertDisk[256];

	// Outer box
	drawInternalGUIControl(GUI_CTRL_OUTER_BOX, 0);

	if (_menuPage == GUI_PAGE_MAIN) {
		drawInternalGUIControl(GUI_CTRL_SAVE_BUTTON, 0); // Save button
		drawInternalGUIControl(GUI_CTRL_LOAD_BUTTON, 0); // Load button
		drawInternalGUIControl(GUI_CTRL_PLAY_BUTTON, 0); // Play button
		drawInternalGUIControl(GUI_CTRL_QUIT_BUTTON, 0); // Quit button

		if (_game.version > 4 &&
			_game.id != GID_MONKEY2 && _game.id != GID_MONKEY &&
			_game.platform != Common::kPlatformAmiga)
			drawInternalGUIControl(GUI_CTRL_INNER_BOX, 0); // Inner box

		if ((_game.version == 5 &&
			(_game.id != GID_MONKEY2 && _game.id != GID_MONKEY && _game.platform != Common::kPlatformAmiga)) ||
			_game.version == 6) {
			drawInternalGUIControl(GUI_CTRL_ARROW_UP_BUTTON, 0);   // Arrow up button
			drawInternalGUIControl(GUI_CTRL_ARROW_DOWN_BUTTON, 0); // Arrow down button
		}

		if ((VAR_FIXEDDISK != 0xFF && VAR(VAR_FIXEDDISK) == 0) || _game.platform == Common::kPlatformAmiga) {
			convertMessageToString((const byte *)getGUIString(gsInsertSaveDisk), (byte *)insertDisk, sizeof(insertDisk));
			drawMainMenuTitle(insertDisk);
		}
	}

	if (_menuPage == GUI_PAGE_SAVE || _menuPage == GUI_PAGE_LOAD) {
		if (_game.version > 4 || (_game.version == 4 && _game.id == GID_LOOM))
			drawInternalGUIControl(GUI_CTRL_INNER_BOX, 0);     // Inner box

		drawInternalGUIControl(GUI_CTRL_PATH_BUTTON, 0);   // Path button
		drawInternalGUIControl(GUI_CTRL_OK_BUTTON, 0);     // Ok button
		drawInternalGUIControl(GUI_CTRL_CANCEL_BUTTON, 0); // Cancel button

		// Savegame names
		for (int i = GUI_CTRL_FIRST_SG; i <= GUI_CTRL_LAST_SG; i++) {
			if ((_game.version == 4 && _game.id != GID_LOOM) && _mainMenuSavegameLabel == 0 && i == 1)
				drawInternalGUIControl(i, 1);
			else
				drawInternalGUIControl(i, 0);
		}

		if (_game.version > 4 || (_game.version == 4 && _game.id == GID_LOOM)) {
			drawInternalGUIControl(GUI_CTRL_ARROW_UP_BUTTON, 0);   // Arrow up button
			drawInternalGUIControl(GUI_CTRL_ARROW_DOWN_BUTTON, 0); // Arrow down button
		}

		if (_menuPage == GUI_PAGE_SAVE) {
			convertMessageToString((const byte *)getGUIString(gsNamePrompt), (byte *)namePrompt, sizeof(namePrompt));
			drawMainMenuTitle(namePrompt);
		} else if (_menuPage == GUI_PAGE_LOAD) {
			convertMessageToString((const byte *)getGUIString(gsSelectLoadPrompt), (byte *)loadPrompt, sizeof(loadPrompt));
			drawMainMenuTitle(loadPrompt);
		}
	}

	if (_mainMenuSavegameLabel)
		drawInternalGUIControl(_mainMenuSavegameLabel, 1);

	ScummEngine::drawDirtyScreenParts();
	_system->updateScreen();
}

void ScummEngine::updateMainMenuControls() {
	if ((_game.variant && !strcmp(_game.variant, "Floppy")) || _game.version < 6)
		return;

	int cid = _charset->getCurID();
	_charset->setCurID(1);
	int lh = getGUIStringHeight("ABC \x80\x78 \xb0\x78");
	_charset->setCurID(cid);

	char msg[256];

	// V7 auxiliary constants
	int yCntr = _screenHeight / 2;
	int calculatedHeight = (110 + lh) / 2;
	int textColor = getBannerColor(2);
	int yOffset = _useCJKMode ? 0 : 1;

	// V6 auxiliary constant
	int yConstantV6 = _virtscr[kMainVirtScreen].topline + (_virtscr[kMainVirtScreen].h / 2);

	strncpy(_mainMenuMusicSlider, "\v\v\v\v\v\v\v\v\v\v\v\v\v\v\v\v", sizeof(_mainMenuMusicSlider));
	strncpy(_mainMenuSpeechSlider, "\v\v\v\v\v\v\v\v\v\v\v\v\v\v\v\v", sizeof(_mainMenuSpeechSlider));
	strncpy(_mainMenuSfxSlider, "\v\v\v\v\v\v\v\v\v\v\v\v\v\v\v\v", sizeof(_mainMenuSfxSlider));
	strncpy(_mainMenuTextSpeedSlider, "\v\v\v\v\v\v\v\v\v\v\v\v\v\v\v\v", sizeof(_mainMenuTextSpeedSlider));

	_mainMenuMusicSlider[getMusicVolume() / 8] = '\f';
	_mainMenuSpeechSlider[getSpeechVolume() / 8] = '\f';
	_mainMenuSfxSlider[getSFXVolume() / 8] = '\f';

	if (VAR_CHARINC != 0xFF)
		_mainMenuTextSpeedSlider[15 - (15 * VAR(VAR_CHARINC) / 9)] = '\f';

	_internalGUIControls[GUI_CTRL_MUSIC_SLIDER].label      = _mainMenuMusicSlider;
	_internalGUIControls[GUI_CTRL_SPEECH_SLIDER].label     = _mainMenuSpeechSlider;
	_internalGUIControls[GUI_CTRL_SFX_SLIDER].label        = _mainMenuSfxSlider;
	_internalGUIControls[GUI_CTRL_TEXT_SPEED_SLIDER].label = _mainMenuTextSpeedSlider;

	if (_sound->isAudioDisabled()) {
		_internalGUIControls[GUI_CTRL_MUSIC_SLIDER].label  = getGUIString(gsDisabled);
		_internalGUIControls[GUI_CTRL_SPEECH_SLIDER].label = getGUIString(gsDisabled);
		_internalGUIControls[GUI_CTRL_SFX_SLIDER].label    = getGUIString(gsDisabled);
	}

	if (_spooledMusicIsToBeEnabled) {
		_internalGUIControls[GUI_CTRL_SPOOLED_MUSIC_CHECKBOX].label = _checkedBox;
	} else {
		_internalGUIControls[GUI_CTRL_SPOOLED_MUSIC_CHECKBOX].label = _uncheckedBox;
	}

	_internalGUIControls[GUI_CTRL_DISPLAY_TEXT_CHECKBOX].label = _checkedBox;

	if (VAR_VOICE_MODE != 0xFF && VAR(VAR_VOICE_MODE) == 0) {
		_internalGUIControls[GUI_CTRL_DISPLAY_TEXT_CHECKBOX].label = _uncheckedBox;
		_internalGUIControls[GUI_CTRL_TEXT_SPEED_SLIDER].label = getGUIString(gsDisabled);
	}

	drawInternalGUIControl(GUI_CTRL_MUSIC_SLIDER,  0); // Music slider
	drawInternalGUIControl(GUI_CTRL_SPEECH_SLIDER, 0); // Speech slider
	drawInternalGUIControl(GUI_CTRL_SFX_SLIDER,    0); // SFX slider

	if (_game.id == GID_FT)
		drawInternalGUIControl(GUI_CTRL_SPOOLED_MUSIC_CHECKBOX, 0); // Spooled music checkbox

	drawInternalGUIControl(GUI_CTRL_DISPLAY_TEXT_CHECKBOX, 0); // Display text checkbox
	drawInternalGUIControl(GUI_CTRL_TEXT_SPEED_SLIDER,     0); // Text speed slider

	if (_game.version == 7) {
		// Full Throttle has the "Spooled Music" checkbox,
		// not rendered in the other games, so adjust that...
		if (_game.id == GID_FT) {
			convertMessageToString((const byte *)getGUIString(gsSpooledMusic), (byte *)msg, sizeof(msg));
			drawGUIText(msg, 0, 29, yCntr - calculatedHeight - yOffset + 19, textColor, false);

			convertMessageToString((const byte *)getGUIString(gsMusic), (byte *)msg, sizeof(msg));
			drawGUIText(msg, 0, 29, yCntr - calculatedHeight - yOffset + 33, textColor, false);

			convertMessageToString((const byte *)getGUIString(gsVoice), (byte *)msg, sizeof(msg));
			drawGUIText(msg, 0, 29, yCntr - calculatedHeight - yOffset + 47, textColor, false);
		} else {
			convertMessageToString((const byte *)getGUIString(gsMusic), (byte *)msg, sizeof(msg));
			drawGUIText(msg, 0, 29, yCntr - calculatedHeight - yOffset + 25, textColor, false);

			convertMessageToString((const byte *)getGUIString(gsVoice), (byte *)msg, sizeof(msg));
			drawGUIText(msg, 0, 29, yCntr - calculatedHeight - yOffset + 43, textColor, false);
		}

		convertMessageToString((const byte *)getGUIString(gsSfx), (byte *)msg, sizeof(msg));
		drawGUIText(msg, 0, 29, yCntr - calculatedHeight - yOffset + 61, textColor, false);

		convertMessageToString((const byte *)getGUIString(gsDisplayText), (byte *)msg, sizeof(msg));
		drawGUIText(msg, 0, 29, yCntr - calculatedHeight - yOffset + 88, textColor, false);

		convertMessageToString((const byte *)getGUIString(gsTextSpeed), (byte *)msg, sizeof(msg));
		drawGUIText(msg, 0, 29, yCntr - calculatedHeight - yOffset + 102, textColor, false);

		drawLine(23, yCntr - calculatedHeight - yOffset + 77, 204, yCntr - calculatedHeight - yOffset + 77, getBannerColor(17));
		drawLine(23, yCntr - calculatedHeight - yOffset + 78, 204, yCntr - calculatedHeight - yOffset + 78, getBannerColor(4));
		drawLine(23, yCntr - calculatedHeight - yOffset + 79, 204, yCntr - calculatedHeight - yOffset + 79, getBannerColor(4));
		drawLine(23, yCntr - calculatedHeight - yOffset + 80, 204, yCntr - calculatedHeight - yOffset + 80, getBannerColor(18));

		// The following line is from the Aaron Giles' interpreter of FT, based on the first DOS version;
		// for some reason it doesn't get displayed in the DOS version, and it also overflows
		// onto the internal panel lines, so let's just not draw it...
		// drawLine(24, yCntr - calculatedHeight + 81, 204, yCntr - calculatedHeight + 81, getBannerColor(4));
	} else {
		convertMessageToString((const byte *)getGUIString(gsMusic), (byte *)msg, sizeof(msg));
		drawGUIText(msg, 0, 33, yConstantV6 - 36, textColor, false);

		convertMessageToString((const byte *)getGUIString(gsVoice), (byte *)msg, sizeof(msg));
		drawGUIText(msg, 0, 33, yConstantV6 - 22, textColor, false);

		convertMessageToString((const byte *)getGUIString(gsSfx), (byte *)msg, sizeof(msg));
		drawGUIText(msg, 0, 33, yConstantV6 - 8, textColor, false);

		convertMessageToString((const byte *)getGUIString(gsDisplayText), (byte *)msg, sizeof(msg));
		drawGUIText(msg, 0, 33, yConstantV6 + 19, textColor, false);

		convertMessageToString((const byte *)getGUIString(gsTextSpeed), (byte *)msg, sizeof(msg));
		drawGUIText(msg, 0, 33, yConstantV6 + 34, textColor, false);

		drawLine(27, yConstantV6 + 8,  201, yConstantV6 + 8,  getBannerColor(17));
		drawLine(27, yConstantV6 + 9,  201, yConstantV6 + 9,  getBannerColor(4));
		drawLine(27, yConstantV6 + 10, 201, yConstantV6 + 10, getBannerColor(4));
		drawLine(27, yConstantV6 + 11, 201, yConstantV6 + 11, getBannerColor(18));
	}

	ScummEngine::drawDirtyScreenParts();
	_system->updateScreen();
}

void ScummEngine::drawMainMenuTitle(const char *title) {
	int boxColor, stringColor;
	int yConstantV6 = _virtscr[kMainVirtScreen].topline + (_virtscr[kMainVirtScreen].h / 2);

	if (_game.version == 4) {
		if (_game.id == GID_LOOM) {
			boxColor = 7;
			stringColor = 0;
		} else {
			boxColor = getBannerColor(6);
			stringColor = getBannerColor(8);
		}
	} else {
		boxColor = getBannerColor(4);
		stringColor = getBannerColor(2);
	}

	if (_game.id == GID_DIG) {
		int cid = _charset->getCurID();
		_charset->setCurID(1);
		int lh = getGUIStringHeight("ABC \x80\x78 \xb0\x78");
		_charset->setCurID(cid);
		int yCntr = _screenHeight / 2;
		int calculatedHeight = (110 + lh) / 2;
		int yOffset = _useCJKMode ? 0 : 1;

		drawBox(18,
			yCntr - calculatedHeight + _screenTop - yOffset + 4,
			301,
			yCntr - calculatedHeight + _screenTop - yOffset + 3 + lh,
			boxColor);

		drawGUIText(title, 0,
			159,
			yCntr - calculatedHeight - yOffset + 4,
			stringColor,
			true);
	} else if (_game.version == 7) {
		drawBox(18, _screenTop + 44, 301, _screenTop + 52, boxColor);
		drawGUIText(title, 0, 159, 44, stringColor, true);
	} else if (_game.version == 4) {
		if (_game.id == GID_LOOM) {
			drawBox(22, yConstantV6 - 57, 298, yConstantV6 - 49, boxColor);
			drawGUIText(title, 0, 160, yConstantV6 - 57, stringColor, true);
		} else {
			drawBox(21, yConstantV6 - 55, 299, yConstantV6 - 47, boxColor);
			drawGUIText(title, 0, 160, yConstantV6 - 55, stringColor, true);
		}
	} else {
		drawBox(22, yConstantV6 - 56, 298, yConstantV6 - 48, boxColor);
		drawGUIText(title, 0, 160, yConstantV6 - 56, stringColor, true);
	}

	ScummEngine::drawDirtyScreenParts();
	_system->updateScreen();
}

int ScummEngine::getGUIStringHeight(const char *str) {
	return _charset->getFontHeight();
}

int ScummEngine::getGUIStringWidth(const char *str) {
	return _charset->getStringWidth(0, (const byte *)str);
}

void ScummEngine::drawGUIText(const char *buttonString, Common::Rect *clipRect, int textXPos, int textYPos, int textColor, bool centerFlag) {
	int tmpRight = _string[5].right;

	_string[5].xpos = textXPos;
	_string[5].ypos = textYPos;
	_string[5].right = clipRect ? clipRect->right : _screenWidth - 1;
	_string[5].center = centerFlag;
	_string[5].color = textColor;
	_string[5].charset = 1;

	drawString(5, (const byte *)buttonString);
	_string[5].right = tmpRight;
}

void ScummEngine::getSliderString(int stringId, int value, char *sliderString, int size) {
	char *ptrToChar;
	char tempStr[256];

	Common::strlcpy(tempStr, getGUIString(stringId), sizeof(tempStr));
	convertMessageToString((const byte *)tempStr, (byte *)sliderString, size);

	ptrToChar = strchr(sliderString, '=');

	if (!ptrToChar) {
		ptrToChar = strstr(sliderString, "xxx");
	}

	if (ptrToChar) {
		if (stringId == gsTextSpeedSlider) {
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
	case gsInsertSaveDisk:
		resStringId = 13;
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
	case gsTextSpeed:
		resStringId = 22;
		break;
	case gsDisplayText:
		resStringId = 23;
		break;
	case gsMusic:
		resStringId = 24;
		break;
	case gsVoice:
		resStringId = 25;
		break;
	case gsSfx:
		resStringId = 26;
		break;
	case gsDisabled:
		resStringId = 27;
		break;
	case gsVoiceOnly:
		resStringId = 28;
		break;
	case gsVoiceAndText:
		resStringId = 29;
		break;
	case gsTextDisplayOnly:
		resStringId = 30;
		break;
	case gsTextSpeedSlider:
		resStringId = 31;
		break;
	case gsMusicVolumeSlider:
		resStringId = 32;
		break;
	case gsVoiceVolumeSlider:
		resStringId = 33;
		break;
	case gsSfxVolumeSlider:
		resStringId = 34;
		break;
	case gsHeap:
		resStringId = 35;
		break;
	default:
		return _emptyMsg;
	}

	if (resStringId > 0)
		return d.getPlainEngineString(resStringId);
	else
		return _emptyMsg;
}

const char *ScummEngine::getGUIString(int stringId) {
	InfoDialog d(this, 0);
	int resStringId = -1;

	switch (stringId) {
	case gsPause:
		resStringId = 4;
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
	case gsInsertSaveDisk:
		resStringId = 13;
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
	case gsVoiceOnly:
		resStringId = 22;
		break;
	case gsVoiceAndText:
		resStringId = 23;
		break;
	case gsTextDisplayOnly:
		resStringId = 24;
		break;
	case gsTextSpeedSlider:
		resStringId = 25;
		break;
	case gsMusicVolumeSlider:
		resStringId = 26;
		break;
	case gsHeap:
		resStringId = 28;
		break;
	default:
		return _emptyMsg;
	}

	if (resStringId > 0)
		return d.getPlainEngineString(resStringId);
	else
		return _emptyMsg;
}

} // End of namespace Scumm
