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
#include "audio/softsynth/pcspk.h"
#include "common/config-manager.h"
#include "common/events.h"
#include "graphics/scaler.h"

#include "igor/igor.h"
#include "igor/floppy_menu.h"

namespace Igor {

// The menu is drawn in a 234x115 panel whose top-left corner is at (43, 15) on the 320x200 screen.
static const int kMenuX = 43;
static const int kMenuY = 15;
static const int kMenuWidth = 234;
static const int kMenuHeight = 115;
static const int kMenuSize = 320 * kMenuHeight;
static const int kButtonSpacing = 46;
static const int kButtonWidth = 44;
static const int kCaptionY = 18 - kMenuY;
static const int kPageCenterX = 159;
static const int kPageRowHeight = 22;
static const int kFadeSteps = 21;
static const int kLastFadedColor = 245;

enum {
	kColorTitle = 0xF6,
	kColorFrameLight = 0xF7,
	kColorFill = 0xF8,
	kColorFrameDark = 0xF9,
	kColorPageCaption = 0xFB,
	kColorText = 0xFF
};

static const struct {
	int y;
	int h;
} buttonsBarCoords[] = {
	{ 3, 1 },
	{ 1, 1 },
	{ 2, 2 },
	{ 4, 1 },
	{ 5, 10 },
	{ 6, 1 },
	{ 2, 2 },
	{ 7, 1 },
	{ 8, 93 },
	{ 9, 1 },
	{ 2, 2 },
	{ 3, 1 }
};

static void drawButtonsBar(uint8 *dst, const uint8 *src) {
	int y0 = kMenuY;
	for (int i = 1; i <= 11; ++i) {
		int y2 = y0 + buttonsBarCoords[i].h - 1;
		if (y0 > y2) {
			continue;
		}
		for (int y1 = y0; y1 <= y2; ++y1) {
			memcpy(dst + (y1 - kMenuY) * 320 + kMenuX, src + (buttonsBarCoords[i].y - 1) * kMenuWidth, kMenuWidth);
			++y0;
		}
	}
}


static void redrawButton(uint8 *dst, int button, bool highlight) {
	const int colorDiff = highlight ? 4 : -4;
	for (int y = 0; y <= 11; ++y) {
		uint8 *p = dst + (y + 3) * 320 + button * kButtonSpacing + kMenuX + 3;
		for (int x = 0; x < kButtonWidth; ++x) {
			p[x] += colorDiff;
		}
	}
}

enum {
	kPageSave = 0,
	kPageLoad,
	kPageQuit,
	kPageCtrl,
	kPagePlay
};

enum OptionsMenuText {
	kOptionsSave = 0,
	kOptionsLoad,
	kOptionsQuit,
	kOptionsCtrl,
	kOptionsPlay,
	kOptionsSoundOff,
	kOptionsSoundOn,
	kOptionsSoundUnavailable,
	kOptionsSpeechOnly,
	kOptionsSpeechAndText,
	kOptionsTextOnly,
	kOptionsSpeechUnavailable,
	kOptionsMusicOff,
	kOptionsMusicOn,
	kOptionsMusicUnavailable,
	kOptionsSpeedVerySlow,
	kOptionsSpeedSlow,
	kOptionsSpeedNormal,
	kOptionsSpeedFast,
	kOptionsSpeedVeryFast,
	kOptionsSpeedSpeech,
	kOptionsSpeedOff,
	kOptionsCtrlTitle,
	kOptionsQuitTitle,
	kOptionsQuitQuestion,
	kOptionsQuitKeys,
	kOptionsSaveTitle,
	kOptionsNoSavedGames,
	kOptionsLoadTitle,
	kOptionsUnavailable,
	kOptionsTextCount
};

struct Page {
	int captions[2];
	int xPos[2];
	void (IgorEngine::*paintProc)();
	bool (IgorEngine::*handleKeyDownProc)(int key);
};

static const Page pages[] = {
	{
		{ kOptionsSave, kOptionsSaveTitle },
		{ 67, 159 },
		0, // todo: native save/load screens
		0
	},
	{
		{ kOptionsLoad, kOptionsLoadTitle },
		{ 113, 159 },
		0,
		0
	},
	{
		{ kOptionsQuit, kOptionsQuitTitle },
		{ 159, 159 },
		&IgorEngine::handleOptionsMenu_paintQuit,
		&IgorEngine::handleOptionsMenu_handleKeyDownQuit
	},
	{
		{ kOptionsCtrl, kOptionsCtrlTitle },
		{ 205, 159 },
		&IgorEngine::handleOptionsMenu_paintCtrl,
		&IgorEngine::handleOptionsMenu_handleKeyDownCtrl
	},
	{
		{ kOptionsPlay, -1 },
		{ 251, 0 },
		0,
		0
	}
};

// Load the texts from the resources of each game version
void IgorEngine::loadOptionsMenuTexts() {
	if (!_optionsMenuTexts.empty())
		return;
	if (_game.version == kIdSpaFloppy) {
		loadFloppyOptionsMenuTexts();
		return;
	}

	const bool demo = _game.flags & kFlagDemo;
	const bool talkie = _game.flags & kFlagTalkie;
	_optionsMenuTexts.resize(kOptionsTextCount);
	const struct {
		int resource;
		int count;
		int firstText;
	} groups[] = {
		{ TXT_OptionsButtons, 5, kOptionsSave },
		{ TXT_OptionsSettings, talkie ? 17 : 12, kOptionsSoundOff },
		{ TXT_OptionsTitle, 1, kOptionsCtrlTitle },
		{ TXT_OptionsQuit, 3, kOptionsQuitTitle },
		{ TXT_OptionsSaveTitle, 1, kOptionsSaveTitle },
		{ TXT_OptionsLoadTitle, talkie ? 2 : 1, talkie ? kOptionsNoSavedGames : kOptionsLoadTitle },
		{ TXT_OptionsUnavailable, demo ? 1 : 0, kOptionsUnavailable }
	};
	for (uint i = 0; i < ARRAYSIZE(groups); ++i) {
		if (groups[i].count == 0)
			continue;
		int size;
		uint8 *data = loadData(groups[i].resource, 0, &size);
		int offset = 0;
		for (int j = 0; j < groups[i].count; ++j) {
			if (offset >= size || data[offset] > size - offset - 1)
				error("Invalid options menu text resource %d", groups[i].resource);
			int length = data[offset++];
			int text = groups[i].firstText + j;
			if (!talkie && groups[i].resource == TXT_OptionsSettings && j >= 3) {
				// Non-talkie versions have one text-only label in place of the CD speech choices.
				text = kOptionsSpeechUnavailable + j - 3;
			}
			_optionsMenuTexts[text] = Common::String((const char *)data + offset, length);
			offset += length;
		}
		if (offset != size)
			error("Unexpected data in options menu text resource %d", groups[i].resource);
		free(data);
	}
	assert(_optionsMenuTexts.size() == kOptionsTextCount);
}

void IgorEngine::drawOptionsMenuText(int text, int x, int y, int color) {
	const char *str = _optionsMenuTexts[text].c_str();
	// center horizontally
	_dialogueFont->drawString(_screenTextLayer, str, x - _dialogueFont->getStringWidth(str) / 2, y, color, -1, 0);
}

void IgorEngine::handleOptionsMenu_paintQuit() {
	drawOptionsMenuText(kOptionsQuitQuestion, kPageCenterX, 64 - kMenuY, kColorTitle);
	drawOptionsMenuText(kOptionsQuitKeys, kPageCenterX, 76 - kMenuY, kColorTitle);
}

bool IgorEngine::handleOptionsMenu_handleKeyDownQuit(int key) {
	const int yesKey = (_game.language == Common::ES_ESP) ? Common::KEYCODE_s : Common::KEYCODE_y;
	if (key == yesKey)
		_currentPart = kInvalidPart;
	return key == yesKey || key == Common::KEYCODE_n || key == Common::KEYCODE_ESCAPE;
}

static void redrawRect(uint8 *dst, int num, bool border) {
	const int x = kMenuX + 6;
	const int y = kPageRowHeight * num;
	for (int i = 0; i < 21; ++i) {
		memset(dst + (y + i) * 320 + x, kColorFill, 222);
	}
	if (border) {
		memset(dst +  y       * 320 + x, kColorFrameLight, 222);
		memset(dst + (y + 20) * 320 + x, kColorFrameDark, 222);
		for (int i = 1; i < 20; ++i) {
			dst[(y + i) * 320 + x      ] = kColorFrameLight;
			dst[(y + i) * 320 + x + 221] = kColorFrameDark;
		}
	}
}

void IgorEngine::handleOptionsMenu_paintCtrl() {
	if (_game.flags & kFlagDemo) {
		const int labels[] = {
			kOptionsSoundOff + CLIP<int>(_gameState.configSoundEnabled, 0, 2),
			kOptionsSpeechUnavailable,
			kOptionsMusicOn,
			kOptionsSpeedVerySlow + CLIP<int>(_gameState.talkSpeed, 1, 5) - 1 //choose option from the enum with base value + delta
		};
		for (int row = 1; row <= 4; ++row) {
			redrawRect(_screenTextLayer, row, row == 4);
			drawOptionsMenuText(labels[row - 1], kPageCenterX, kPageRowHeight * row + 4, row == 4 ? kColorTitle : kColorFrameLight);
		}
		return;
	}
	const int talkMode = getTalkMode();
	const int labels[] = {
		kOptionsSoundOff + CLIP<int>(_gameState.configSoundEnabled, 0, 2),
		(_game.flags & kFlagTalkie) ? kOptionsSpeechOnly + talkMode : kOptionsSpeechUnavailable,
		(ConfMan.getBool("mute") || ConfMan.getBool("music_mute")) ? kOptionsMusicOff : kOptionsMusicOn,
		talkMode == kTalkModeTextOnly
			? kOptionsSpeedVerySlow + CLIP<int>(_gameState.talkSpeed, 1, 5) - 1
			: (talkMode == kTalkModeSpeechOnly ? kOptionsSpeedOff : kOptionsSpeedSpeech)
	};
	for (int row = 1; row <= 4; ++row) {
		redrawRect(_screenTextLayer, row, row == _optionsControlRow);
		drawOptionsMenuText(labels[row - 1], kPageCenterX, kPageRowHeight * row + 4,
			row == _optionsControlRow ? kColorTitle : kColorText);
	}
}

bool IgorEngine::handleOptionsMenu_handleKeyDownCtrl(int key) {
	if (_game.flags & kFlagDemo) {
		if (key == Common::KEYCODE_ESCAPE)
			return true;
		if (key == Common::KEYCODE_RETURN) {
			_gameState.talkSpeed = _gameState.talkSpeed == 5 ? 1 : _gameState.talkSpeed + 1;
			handleOptionsMenu_paintCtrl();
		}
		return false;
	}
	const int firstRow = (_game.flags & kFlagTalkie) ? 1 : 4;
	const int lastRow = getTalkMode() == kTalkModeTextOnly ? 4 : 3;
	switch (key) {
	case Common::KEYCODE_ESCAPE:
		return true;
	case Common::KEYCODE_UP:
		_optionsControlRow = MAX(firstRow, _optionsControlRow - 1);
		break;
	case Common::KEYCODE_DOWN:
		_optionsControlRow = MIN(lastRow, _optionsControlRow + 1);
		break;
	case Common::KEYCODE_RETURN:
		switch (_optionsControlRow) {
		case 1:
			ConfMan.setBool("sfx_mute", !ConfMan.getBool("sfx_mute"));
			break;
		case 2: {
			const int talkMode = (getTalkMode() + 1) % 3;
			ConfMan.setBool("speech_mute", talkMode == kTalkModeTextOnly);
			ConfMan.setBool("subtitles", talkMode != kTalkModeSpeechOnly);
			_gameState.talkSpeed = talkMode == kTalkModeTextOnly ? 3
				: (talkMode == kTalkModeSpeechOnly ? 7 : 6);
			break;
		}
		case 3:
			ConfMan.setBool("music_mute", !ConfMan.getBool("music_mute"));
			break;
		case 4:
			_gameState.talkSpeed = _gameState.talkSpeed == 5 ? 1 : _gameState.talkSpeed + 1;
			break;
		}
		syncSoundSettings();
		ConfMan.flushToDisk();
		break;
	default:
		return false;
	}
	handleOptionsMenu_paintCtrl();
	return false;
}

enum FloppyOptionsText {
	kFloppySave = 0,
	kFloppyLoad,
	kFloppyQuit,
	kFloppyCtrl,
	kFloppyPlay,
	kFloppySpeedTitle,
	kFloppySpeedVerySlow,
	kFloppySpeedSlow,
	kFloppySpeedNormal,
	kFloppySpeedFast,
	kFloppySpeedVeryFast,
	kFloppyMusicTitle,
	kFloppyMusicOff,
	kFloppyMusicOn,
	kFloppyMusicUnavailable,
	kFloppySoundTitle,
	kFloppySoundOff,
	kFloppySoundOn,
	kFloppySoundUnavailable,
	kFloppyCtrlTitle,
	kFloppyQuitTitle,
	kFloppyQuitQuestion,
	kFloppyTextCount
};

static const int kFloppyCaptionX[5] = { 60, 102, 148, 185, 231 };
static const int kFloppySpeedX[5] = { 137, 148, 145, 146, 135 };

// Row of the button image (1-based) and the number of screen rows it is repeated for.
static const struct {
	int row;
	int count;
} kFloppyPanelRows[] = {
	{ 1, 1 }, { 2, 2 }, { 4, 1 }, { 5, 9 }, { 6, 1 }, { 2, 2 },
	{ 7, 1 }, { 8, 85 }, { 9, 1 }, { 2, 2 }, { 3, 1 }
};

void IgorEngine::loadFloppyOptionsMenuTexts() {
	const struct {
		int resource;
		int count;
	} groups[] = {
		{ TXT_OptionsButtons, 5 },
		{ TXT_OptionsSettings, 14 },
		{ TXT_OptionsTitle, 1 },
		{ TXT_OptionsQuit, 2 }
	};
	for (uint i = 0; i < ARRAYSIZE(groups); ++i) {
		int size;
		uint8 *data = loadData(groups[i].resource, 0, &size);
		int offset = 0;
		for (int j = 0; j < groups[i].count; ++j) {
			if (offset >= size || data[offset] > size - offset - 1)
				error("Invalid options menu text resource %d", groups[i].resource);
			const int length = data[offset++];
			_optionsMenuTexts.push_back(Common::String((const char *)data + offset, length));
			offset += length;
		}
		if (offset != size)
			error("Unexpected data in options menu text resource %d", groups[i].resource);
		free(data);
	}
	assert(_optionsMenuTexts.size() == kFloppyTextCount);
}

// Texts are left aligned and drawn with the action sentence font into the menu panel.
void IgorEngine::floppyOptionsDrawText(int text, int x, int y, int color) {
	_sentenceFont->drawString(_screenTextLayer, _optionsMenuTexts[text].c_str(), x, y - kFloppyMenuY, color, -1, -1);
}

void IgorEngine::floppyOptionsFillRow(const uint8 *button, int buttonRow, int y) {
	memcpy(_screenTextLayer + (y - kFloppyMenuY) * 320 + kFloppyMenuX,
	       button + (buttonRow - 1) * kFloppyMenuWidth, kFloppyMenuWidth);
}

void IgorEngine::floppyOptionsDrawPanel(const uint8 *button) {
	int y = 0;
	for (uint i = 0; i < ARRAYSIZE(kFloppyPanelRows); ++i) {
		for (int j = 0; j < kFloppyPanelRows[i].count; ++j) {
			memcpy(_screenTextLayer + y * 320 + kFloppyMenuX, button + (kFloppyPanelRows[i].row - 1) * kFloppyMenuWidth, kFloppyMenuWidth);
			++y;
		}
	}
}

void IgorEngine::floppyOptionsDrawMainPage(const uint8 *button) {
	floppyOptionsDrawPanel(button);
	for (int i = 0; i < 5; ++i) {
		floppyOptionsDrawText(kFloppySave + i, kFloppyCaptionX[i], kFloppyCaptionY, kFloppyColorCaption);
	}
}

void IgorEngine::floppyOptionsClearCaptionBar(const uint8 *button) {
	for (int y = kFloppyCaptionY; y <= kFloppyCaptionLastRow; ++y) {
		floppyOptionsFillRow(button, 2, y);
	}
}

// Sends the panel to the screen, with the frame of the selected button (0 for none) on top.
void IgorEngine::floppyOptionsPresent(int selectedButton) {
	uint8 panel[kFloppyMenuSize];
	memcpy(panel, _screenTextLayer, kFloppyMenuSize);
	if (selectedButton >= 1 && selectedButton <= 5) {
		const int x = kFloppyButtonX + (selectedButton - 1) * kFloppyButtonWidth;
		uint8 *top = panel + (kFloppyButtonY - kFloppyMenuY) * 320 + x;
		memset(top, kFloppyColorButtonLight, kFloppyButtonWidth);
		for (int i = 1; i < kFloppyButtonHeight - 1; ++i) {
			top[i * 320] = kFloppyColorButtonLight;
			top[i * 320 + kFloppyButtonWidth - 1] = kFloppyColorButtonDark;
		}
		memset(top + (kFloppyButtonHeight - 1) * 320, kFloppyColorButtonDark, kFloppyButtonWidth);
	}
	_system->copyRectToScreen(panel, 320, 0, kFloppyMenuY, 320, kFloppyMenuHeight);
	_system->updateScreen();
}

void IgorEngine::floppyOptionsPaintControl(const uint8 *button, int row) {
	for (int i = 0; i < kFloppyControlPanelRows; ++i) {
		floppyOptionsFillRow(button, 8, kFloppyControlPanelFirstRow + i);
	}
	const int speed = CLIP<int>(_gameState.talkSpeed, 1, 5);
	const bool musicOn = !(ConfMan.getBool("mute") || ConfMan.getBool("music_mute"));
	const bool soundOn = !(ConfMan.getBool("mute") || ConfMan.getBool("sfx_mute"));
	const int speedColor = (row == 1) ? kFloppyColorCaption : kFloppyColorText;
	const int musicColor = (row == 2) ? kFloppyColorCaption : kFloppyColorText;
	const int soundColor = (row == 3) ? kFloppyColorCaption : kFloppyColorText;
	floppyOptionsDrawText(kFloppySpeedTitle, 120, 40, speedColor);
	floppyOptionsDrawText(kFloppySpeedVerySlow + speed - 1, kFloppySpeedX[speed - 1], 51, speedColor);
	floppyOptionsDrawText(kFloppyMusicTitle, 147, 68, musicColor);
	if (musicOn)
		floppyOptionsDrawText(kFloppyMusicOn, 141, 79, musicColor);
	else
		floppyOptionsDrawText(kFloppyMusicOff, 133, 79, musicColor);
	floppyOptionsDrawText(kFloppySoundTitle, 122, 96, soundColor);
	if (soundOn)
		floppyOptionsDrawText(kFloppySoundOn, 138, 107, soundColor);
	else
		floppyOptionsDrawText(kFloppySoundOff, 131, 107, soundColor);

	// Frame around the selected row: light top and left edges, dark bottom and right edges.
	const int top = (28 * row + 10 - kFloppyMenuY) * 320 + 59;
	memset(_screenTextLayer + top, kFloppyColorFrameLight, 202);
	for (int i = 1; i <= 23; ++i) {
		_screenTextLayer[top + i * 320] = kFloppyColorFrameLight;
		_screenTextLayer[top + i * 320 + 201] = kFloppyColorFrameDark;
	}
	memset(_screenTextLayer + top + 24 * 320, kFloppyColorFrameDark, 202);
}

void IgorEngine::floppyOptionsControlPage(const uint8 *button) {
	floppyOptionsClearCaptionBar(button);
	floppyOptionsDrawText(kFloppyCtrlTitle, 121, kFloppyCaptionY, kFloppyColorCaption);
	int row = 1;
	bool leave = false;
	floppyOptionsPaintControl(button, row);
	while (!leave && !_eventQuitGame && _currentPart != kInvalidPart) {
		Common::Event ev;
		while (_eventMan->pollEvent(ev)) {
			if (ev.type == Common::EVENT_QUIT) {
				_currentPart = kInvalidPart;
				_eventQuitGame = true;
			} else if (ev.type == Common::EVENT_KEYDOWN) {
				switch (ev.kbd.keycode) {
				case Common::KEYCODE_ESCAPE:
					leave = true;
					break;
				case Common::KEYCODE_UP:
					row = MAX(1, row - 1);
					break;
				case Common::KEYCODE_DOWN:
					row = MIN(3, row + 1);
					break;
				case Common::KEYCODE_RETURN:
				case Common::KEYCODE_KP_ENTER:
					if (row == 1) {
						_gameState.talkSpeed = _gameState.talkSpeed >= 5 ? 1 : _gameState.talkSpeed + 1;
					} else if (row == 2) {
						ConfMan.setBool("music_mute", !ConfMan.getBool("music_mute"));
					} else {
						ConfMan.setBool("sfx_mute", !ConfMan.getBool("sfx_mute"));
					}
					syncSoundSettings();
					ConfMan.flushToDisk();
					break;
				default:
					break;
				}
				floppyOptionsPaintControl(button, row);
			}
		}
		floppyOptionsPresent(0);
		_system->delayMillis(10);
	}
	floppyOptionsDrawMainPage(button);
}

void IgorEngine::floppyOptionsQuitPage(const uint8 *button) {
	floppyOptionsClearCaptionBar(button);
	floppyOptionsDrawText(kFloppyQuitTitle, 132, kFloppyCaptionY, kFloppyColorCaption);
	floppyOptionsDrawText(kFloppyQuitQuestion, 90, 70, kFloppyColorCaption);
	bool leave = false;
	while (!leave && !_eventQuitGame && _currentPart != kInvalidPart) {
		Common::Event ev;
		while (_eventMan->pollEvent(ev)) {
			if (ev.type == Common::EVENT_QUIT) {
				_currentPart = kInvalidPart;
				_eventQuitGame = true;
			} else if (ev.type == Common::EVENT_KEYDOWN) {
				if (ev.kbd.keycode == Common::KEYCODE_s) {
					_currentPart = kInvalidPart;
					leave = true;
				} else if (ev.kbd.keycode == Common::KEYCODE_n || ev.kbd.keycode == Common::KEYCODE_ESCAPE) {
					leave = true;
				}
			}
		}
		floppyOptionsPresent(0);
		_system->delayMillis(10);
	}
	if (_currentPart != kInvalidPart)
		floppyOptionsDrawMainPage(button);
}

// The menus start from the full palette and dim colors 1..248 in steps of three levels.
void IgorEngine::floppyMenuFadeOut() {
	memcpy(_currentPalette, _paletteBuffer, 768);
	for (int m = 3; m <= kFloppyFadeSteps; m += 3) {
		for (int i = 3; i < kFloppyFadeCount; ++i) {
			if (_paletteBuffer[i] >= m) {
				_currentPalette[i] = MAX<int>(_paletteBuffer[i] - m, _currentPalette[i] - 3);
			}
		}
		setPaletteRange(1, kFloppyLastFadedColor);
		_system->updateScreen();
		_system->delayMillis(1000 / 60);
	}
	setPaletteColor(255, 35, 35, 23);
}

void IgorEngine::handleFloppyOptionsMenu() {
	floppyMenuFadeOut();

	memcpy(_screenTextLayer, _screenVGA + kFloppyMenuY * 320, kFloppyMenuSize);
	uint8 *button = loadData(IMG_OptionsButton);
	floppyOptionsDrawMainPage(button);

	int selected = 1;
	bool menuLoop = true;
	while (menuLoop && !_eventQuitGame && _currentPart != kInvalidPart) {
		Common::Event ev;
		while (_eventMan->pollEvent(ev)) {
			if (ev.type == Common::EVENT_QUIT) {
				_currentPart = kInvalidPart;
				_eventQuitGame = true;
			} else if (ev.type == Common::EVENT_KEYDOWN) {
				switch (ev.kbd.keycode) {
				case Common::KEYCODE_ESCAPE:
					menuLoop = false;
					break;
				case Common::KEYCODE_LEFT:
					selected = MAX(1, selected - 1);
					break;
				case Common::KEYCODE_RIGHT:
					selected = MIN(5, selected + 1);
					break;
				case Common::KEYCODE_RETURN:
				case Common::KEYCODE_KP_ENTER:
					if (selected == 1) {
						if (saveGameDialog())
							menuLoop = false;
					} else if (selected == 2) {
						if (loadGameDialog()) {
							free(button);
							_optionsMenuThumbnail.free();
							return;
						}
					} else if (selected == 3) {
						floppyOptionsQuitPage(button);
					} else if (selected == 4) {
						floppyOptionsControlPage(button);
					} else {
						menuLoop = false;
					}
					break;
				default:
					break;
				}
			}
		}
		floppyOptionsPresent(selected);
		_system->delayMillis(10);
	}
	free(button);

	_optionsMenuThumbnail.free();
	_inputVars[kInputClick] = 0;
	_inputVars[kInputRightClick] = 0;
	_inputVars[kInputSkipDialogue] = 0;
	_inputVars[kInputOptions] = 0;
	_inputVars[kInputPause] = 0;
	setPaletteColor(255, 63, 63, 63);
	if (_currentPart != kInvalidPart) {
		fadeIn(kFloppyFadeCount);
	}
	showCursor();
}

void IgorEngine::handleOptionsMenu() {
	// get the gameplay screenshot before the menu draws or dims its palette.
	_optionsMenuThumbnail.free();
	::createThumbnailFromScreen(&_optionsMenuThumbnail);
	loadOptionsMenuTexts();
	// pause engine so no sounds keep playing while the options menu is active.
	PauseToken pauseToken = pauseEngine();

	Audio::PCSpeaker speaker;
	const bool speakerReady = (_game.flags & kFlagFloppy) && speaker.init();

	hideCursor();
	// End the room's spoken line before reusing its text backing store.
	if (_gameState.dialogueTextRunning) {
		if (_currentPart != 40 && _currentPart != 700)
			animateIgorTalking(0);
		memcpy(_screenVGA + _dialogueDirtyRectY, _screenTextLayer + 23040, _dialogueDirtyRectSize);
		stopDialogueSpeech();
		_gameState.dialogueTextRunning = false;
		_currentAction.verbType = 0;
		_currentAction.object2Num = 0;
		_currentAction.object2Type = 0;
	}
	if (_game.version == kIdSpaFloppy) {
		handleFloppyOptionsMenu();
		return;
	}
	// Keep the reserved UI colors: dialogue changes the displayed palette only.
	memcpy(_paletteBuffer, _currentPalette, 208 * 3);
	memcpy(_currentPalette + 208 * 3, _paletteBuffer + 208 * 3, 48 * 3);
	setPaletteRange(208, 255);
	if (!_gameState.dialogueStarted)
		memset(_screenVGA + 144 * 320, 0, 11 * 320);
	_system->copyRectToScreen(_screenVGA, 320, 0, 0, 320, 200);
	for (int m = 3; m <= kFadeSteps; m += 3) {
		for (int i = 1 * 3; i <= kLastFadedColor * 3; ++i) {
			if (_paletteBuffer[i] >= m) {
				_currentPalette[i] = MAX<int>(_paletteBuffer[i] - m, _currentPalette[i] - 3);
			}
		}
		setPaletteRange(1, kLastFadedColor);
		_system->updateScreen();
		_system->delayMillis(1000 / 60);
	}
	setPaletteColor(255, 35, 35, 23);

	memcpy(_screenTextLayer, _screenVGA + kMenuY * 320, kMenuSize);

	uint8 *optionsMenu = loadData(IMG_OptionsMenu);
	for (int i = 0; i < kMenuSize; ++i) {
		if (optionsMenu[i]) {
			_screenTextLayer[i] = optionsMenu[i];
		}
	}
	free(optionsMenu);
	uint8 *optionsButton = loadData(IMG_OptionsButton);
	drawButtonsBar(_screenTextLayer, optionsButton);
	free(optionsButton);

	const int yPosCaption = kCaptionY;
	for (int i = 0; i < 5; ++i) {
		drawOptionsMenuText(pages[i].captions[0], pages[i].xPos[0], yPosCaption, kColorTitle);
	}
	redrawButton(_screenTextLayer, 0, true);

	uint8 buttonsBarBackup[320 * 12];
	int currentPage = 0;
	bool menuLoop = true;
	bool focusOnPage = false;
	while (menuLoop && !_eventQuitGame && _currentPart != kInvalidPart) {
		int previousPage = currentPage;
		Common::Event ev;
		while (_eventMan->pollEvent(ev)) {
			switch (ev.type) {
			case Common::EVENT_QUIT:
				_currentPart = kInvalidPart;
				_eventQuitGame = true;
				break;
			case Common::EVENT_KEYDOWN:
				if (focusOnPage) {
					const bool unavailable = (_game.flags & kFlagDemo) && (currentPage == kPageSave || currentPage == kPageLoad);
					if (unavailable || (this->*pages[currentPage].handleKeyDownProc)(ev.kbd.keycode)) {
						memcpy(_screenTextLayer + 3 * 320, buttonsBarBackup, 320 * 12);
						for (int y = 22; y <= 110; ++y) {
							memset(_screenTextLayer + y * 320 + kMenuX + 4, kColorFill, 226);
						}
						focusOnPage = false;
					}
				} else {
					switch (ev.kbd.keycode) {
					case Common::KEYCODE_ESCAPE:
						menuLoop = false;
						break;
					case Common::KEYCODE_RETURN:
						if (currentPage == kPageSave && !(_game.flags & kFlagDemo)) {
							if (saveGameDialog())
								menuLoop = false;
						} else if (currentPage == kPageLoad && !(_game.flags & kFlagDemo)) {
							if (loadGameDialog()) {
								_optionsMenuThumbnail.free();
								return;
							}
						} else if (currentPage == kPagePlay) {
							menuLoop = false;
						} else {
							memcpy(buttonsBarBackup, _screenTextLayer + 3 * 320, 320 * 12);
							for (int y = 0; y <= 11; ++y) {
								memset(_screenTextLayer + (y + 3) * 320 + kMenuX + 1, kColorFill, 232);
							}
							drawOptionsMenuText(pages[currentPage].captions[1], pages[currentPage].xPos[1], yPosCaption, kColorPageCaption);
							if (currentPage == kPageCtrl)
								_optionsControlRow = (_game.flags & kFlagTalkie) ? 1 : 4;
							if ((_game.flags & kFlagDemo) && (currentPage == kPageSave || currentPage == kPageLoad)) {
								drawOptionsMenuText(kOptionsUnavailable, kPageCenterX, 70 - 16, kColorTitle);
								_system->copyRectToScreen(_screenTextLayer, 320, 0, kMenuY, 320, kMenuHeight);
								_system->updateScreen();
								// The unavailable page requests a PCSpeaker beep before accepting another key.
								if (speakerReady)
									speaker.playQueue(Audio::PCSpeaker::kWaveFormSquare, 1193182.0f / 1320, 333000);
								_system->delayMillis(333);
							} else {
								(this->*pages[currentPage].paintProc)();
							}
							focusOnPage = true;
						}
						break;
					case Common::KEYCODE_LEFT:
						--currentPage;
						if (currentPage < 0) {
							currentPage = 0;
						}
						break;
					case Common::KEYCODE_RIGHT:
						++currentPage;
						if (currentPage > 4) {
							currentPage = 4;
						}
						break;
					default:
						break;
					}
				}
				break;
			default:
				break;
			}
		}
		if (previousPage != currentPage) {
			redrawButton(_screenTextLayer, previousPage, false);
			redrawButton(_screenTextLayer, currentPage, true);
		}
		_system->delayMillis(10);
		_system->copyRectToScreen(_screenTextLayer, 320, 0, kMenuY, 320, kMenuHeight);
		_system->updateScreen();
	}

	_optionsMenuThumbnail.free();
	_inputVars[kInputClick] = 0;
	_inputVars[kInputRightClick] = 0;
	_inputVars[kInputSkipDialogue] = 0;
	_inputVars[kInputOptions] = 0;
	_inputVars[kInputPause] = 0;
	_system->copyRectToScreen(_screenVGA, 320, 0, 0, 320, 200);
	setPaletteColor(255, 63, 63, 63);
	for (int m = kFadeSteps; m >= 3; m -= 3) {
		for (int i = 1 * 3; i <= kLastFadedColor * 3; ++i) {
			if (_paletteBuffer[i] >= m) {
				_currentPalette[i] = MIN<int>(_paletteBuffer[i], _currentPalette[i] + 3);
			}
		}
		setPaletteRange(1, kLastFadedColor);
		_system->updateScreen();
		_system->delayMillis(1000 / 60);
	}
	memcpy(_currentPalette, _paletteBuffer, 768);
	updatePalette(768);
	_system->updateScreen();
	showCursor();
}

} // End of namespace Igor
