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
#include "scumm/scumm_v7.h"
#include "scumm/gfx.h"
#include "scumm/dialogs.h"
#include "scumm/charset.h"
#include "scumm/string_v7.h"
#include "scumm/smush/smush_player.h"

#include "graphics/cursorman.h"

namespace Scumm {

#ifdef ENABLE_SCUMM_7_8
void ScummEngine_v7::initBanners() {
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

Common::KeyState ScummEngine_v7::showBannerAndPause(int bannerId, int32 waitTime, const char *msg, ...) {
	char bannerMsg[512];
	char localizedMsg[512];
	char localizedY[512];
	char *ptrToBreak;
	int bannerMsgWidth, bannerMsgHeight, roundedWidth;
	int startingPointX, startingPointY;
	int xPos, yPos;
	int rightLineColor, leftLineColor, bottomLineColor, topLineColor;
	int primaryLineColor, primaryFillColor;
	InfoDialog d(this, 0);

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

	// Pause the engine
	PauseToken pt = pauseEngine();

	// Gather the colors needed for the banner
	primaryFillColor = getBannerColor(6 * bannerId + 15);
	primaryLineColor = getBannerColor(6 * bannerId + 14);
	topLineColor = getBannerColor(6 * bannerId + 16);
	bottomLineColor = getBannerColor(6 * bannerId + 17);
	leftLineColor = getBannerColor(6 * bannerId + 18);
	rightLineColor = getBannerColor(6 * bannerId + 19);

	// Backup the current charsetId, since we're going to switch
	// to charsetId == 1...
	int oldId = _charset->getCurID();
	_charset->setCurID(1);

	// Take all the necessary measurements for the box which
	// will contain the string...
	bool isCOMIDemo = (_game.id == GID_CMI && (_game.features & GF_DEMO) != 0);
	bannerMsgHeight = (isCOMIDemo ? _textV7->getStringHeight("ABC €x °x") : _textV7->getStringHeight(bannerMsg)) + 5;
	bannerMsgWidth = _textV7->getStringWidth(bannerMsg);
	if (bannerMsgWidth < 100)
		bannerMsgWidth = 100;

	roundedWidth = (((bannerMsgWidth + 15) & 0xFFF0) + 8) / 2;
	startingPointX = _screenWidth / 2 - roundedWidth - 4;
	startingPointY = _screenHeight / 2 - 10;
	xPos = _screenWidth / 2 + roundedWidth + 3;
	yPos = 1 - bannerMsgHeight;

	// Save the pixels which will be overwritten by the banner,
	// so that we can restore them later...
	if (!_bannerMem) {
		_bannerMemSize = bannerMsgHeight * (_screenWidth + 8);
		_bannerMem = (byte *)malloc(bannerMsgHeight * (_screenWidth + 8) * sizeof(byte));
		if (_bannerMem)
			memcpy(
				_bannerMem,
				&_virtscr[kMainVirtScreen].getPixels(0, _screenTop)[(_screenWidth + 8) * (_screenHeight / 2 - 10)],
				_bannerMemSize);
	}

	// Set up the GUI control, specifying all the related colors, the message and the position...
	setUpInternalGUIControl(0, primaryFillColor, primaryLineColor,
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

	// Fetch the localized confirmation letter and substitute it with the 'y' of 'yes'
	convertMessageToString((const byte *)d.getPlainEngineString(29), (byte *)localizedY, sizeof(localizedY));

	if (tolower(localizedY[0]) == ks.ascii || toupper((localizedY[0]) == ks.ascii))
		ks = Common::KEYCODE_y;

	return ks;
}

void ScummEngine_v7::clearBanner() {
	// Restore the GFX content which was under the banner,
	// and then mark that part of the screen as dirty.
	if (_bannerMem) {
		// Don't manually clear the banner if a SMUSH movie is playing,
		// as that will cause some rare small glitches. The SMUSH player
		// will take care of that for us automatically when updating the
		// screen for next frame.
		if (!isSmushActive()) {
			memcpy(
				&_virtscr[kMainVirtScreen].getPixels(0, _screenTop)[(_screenWidth + 8) * (_screenHeight / 2 - 10)],
				_bannerMem,
				_bannerMemSize);

			markRectAsDirty(_virtscr[kMainVirtScreen].number, 0, _screenWidth + 8, _screenTop, _screenHeight + _screenTop);
			ScummEngine::drawDirtyScreenParts();
			_system->updateScreen();
		}

		free(_bannerMem);
		_bannerMem = nullptr;
	}
}

void ScummEngine_v7::setBannerColors(int bannerId, byte r, byte g, byte b) {
	if (bannerId < 0 || bannerId > 50) {
		debug(1, "ScummEngine::setBannerColors(): invalid slot %d out of range (min %d, max %d)", bannerId, 0, 50);
		return;
	}

	_bannerColors[bannerId] = r | (b << 16) | (g << 8);
}

int ScummEngine_v7::getBannerColor(int bannerId) {
	byte r, g, b;
	byte *palette = isSmushActive() ? _splayer->getVideoPalette() : _currentPalette;
	r = (_bannerColors[bannerId] >> 0) & 0xFF;
	g = (_bannerColors[bannerId] >> 8) & 0xFF;
	b = (_bannerColors[bannerId] >> 16) & 0xFF;
	return getPaletteColorFromRGB(palette, r, g, b);
}

void ScummEngine_v7::setUpInternalGUIControl(int id, int primaryFillColor, int primaryLineColor,
											 int topLineColor, int bottomLineColor, int leftLineColor, int rightLineColor,
											 int secondaryLineColor, int secondaryFillColor,
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
	ctrl->primaryFillColor = primaryFillColor;
	ctrl->topLineColor = topLineColor;
	ctrl->bottomLineColor = bottomLineColor;
	ctrl->leftLineColor = leftLineColor;
	ctrl->rightLineColor = rightLineColor;
	ctrl->primaryLineColor = primaryLineColor;
	ctrl->secondaryLineColor = secondaryLineColor;
	ctrl->secondaryFillColor = secondaryFillColor;
}

void ScummEngine_v7::drawInternalGUIControl(int id, bool useSecondaryColor) {
	InternalGUIControl *ctrl;
	int relCentX, relCentY, textHeight;
	int x, y, textXPos, textYPos;
	int lineColor, fillColor;
	int boxSizeX, boxSizeY;

	bool centerFlag;
	char buttonString[512];

	ctrl = &_internalGUIControls[id];
	relCentX = ctrl->relativeCenterX;
	if (ctrl->relativeCenterX != -1) {
		relCentY = ctrl->relativeCenterY;
		x = ctrl->xPos;
		y = ctrl->yPos;

		boxSizeX = x - ctrl->relativeCenterX;
		boxSizeY = y - relCentY;

		fillColor = useSecondaryColor ? ctrl->secondaryFillColor : ctrl->primaryFillColor;

		// Draw the main box...
		drawBox(relCentX + 1, relCentY + 1, boxSizeX - 2, boxSizeY - 2, fillColor);

		// Draw the contour lines for the box; each of the lines is doubled to give a 3D effect.
		drawLine(relCentX + 1, relCentY, x - 1, relCentY, ctrl->topLineColor);
		drawLine(relCentX + 1, y, x - 1, y, ctrl->bottomLineColor);
		drawLine(relCentX, relCentY + 1, relCentX, y - 1, ctrl->leftLineColor);
		drawLine(x, relCentY + 1, x, y - 1, ctrl->rightLineColor);

		drawLine(relCentX + 1, relCentY + 1, x - 1, relCentY + 1, ctrl->topLineColor);
		drawLine(relCentX + 1, y - 1, x - 1, y - 1, ctrl->bottomLineColor);
		drawLine(relCentX + 1, relCentY + 1, relCentX + 1, y - 1, ctrl->leftLineColor);
		drawLine(x - 1, relCentY + 1, x - 1, y - 1, ctrl->rightLineColor);

		// Calculate the positioning for the text
		int oldId = _charset->getCurID();
		_charset->setCurID(1);
		textHeight = _textV7->getStringHeight(buttonString);
		centerFlag = ctrl->centerText;

		if (centerFlag)
			textXPos = relCentX + boxSizeX / 2;
		else
			textXPos = relCentX + 2;

		textYPos = relCentY + (boxSizeY - textHeight) / 2 + 1;

		// Finally, choose the color and draw the text message
		if (useSecondaryColor)
			lineColor = ctrl->secondaryLineColor;
		else
			lineColor = ctrl->primaryLineColor;

		if (ctrl->label)
			strcpy(buttonString, ctrl->label);
		else
			strcpy(buttonString, "null button");

		drawTextImmediately((const byte *)buttonString, textXPos, textYPos, lineColor, 1, (TextStyleFlags)centerFlag);

		// Restore the previous charset
		if (oldId)
			_charset->setCurID(oldId);
	}
}

int ScummEngine_v7::getInternalGUIControlFromCoordinates(int x, int y) {
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

void ScummEngine_v7::confirmExitDialog() {
	if (isUsingOriginalGUI()) {
		int boxWidth, strWidth;
		int ctrlId;
		char yesLabelPtr[512];
		char noLabelPtr[512];
		char msgLabelPtr[512];
		byte *curGrabbedCursor;
		int curCursorWidth, curCursorHeight, curCursorHotspotX, curCursorHotspotY, curCursorState;

		InfoDialog d(this, 0);

		// "Are you sure you want to quit?"
		convertMessageToString((const byte *)d.getPlainEngineString(22), (byte *)msgLabelPtr, sizeof(msgLabelPtr));
		// "Yes"
		convertMessageToString((const byte *)d.getPlainEngineString(23), (byte *)yesLabelPtr, sizeof(yesLabelPtr));
		// "No"
		convertMessageToString((const byte *)d.getPlainEngineString(24), (byte *)noLabelPtr, sizeof(noLabelPtr));

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

		boxWidth = ((_textV7->getStringWidth(msgLabelPtr) + 32) & 0xFFF0) + 8;
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
		if (_textV7->getStringWidth(noLabelPtr) <= _textV7->getStringWidth(yesLabelPtr)) {
			strWidth = _textV7->getStringWidth(yesLabelPtr);
		} else {
			strWidth = _textV7->getStringWidth(noLabelPtr);
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
	} else {
		ScummEngine::confirmExitDialog();
	}
}
#endif
}
