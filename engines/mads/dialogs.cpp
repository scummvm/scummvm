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
#include "common/config-manager.h"
#include "mads/mads.h"
#include "mads/screen.h"
#include "mads/msurface.h"
#include "mads/nebular/dialogs_nebular.h"
#include "common/config-manager.h"

#ifdef USE_TTS
#include "common/text-to-speech.h"
#endif

namespace MADS {

enum PopupEdge {
	EDGE_UPPER_LEFT = 0, EDGE_UPPER_RIGHT = 1, EDGE_LOWER_LEFT = 2,
	EDGE_LOWER_RIGHT = 3, EDGE_LEFT = 4, EDGE_RIGHT = 5, EDGE_TOP = 6,
	EDGE_BOTTOM = 7, EDGE_UPPER_CENTER = 8
};

Dialog::Dialog(MADSEngine *vm)
	: _vm(vm), _savedSurface(nullptr), _position(Common::Point(-1, -1)),
	  _width(0), _height(0) {
	TEXTDIALOG_CONTENT1 = 0XF8;
	TEXTDIALOG_CONTENT2 = 0XF9;
	TEXTDIALOG_EDGE = 0XFA;
	TEXTDIALOG_BACKGROUND = 0XFB;
	TEXTDIALOG_FC = 0XFC;
	TEXTDIALOG_FD = 0XFD;
	TEXTDIALOG_FE = 0XFE;
	TEXTDIALOG_BLACK = 0;
}

Dialog::~Dialog() {
	delete _savedSurface;
}

void Dialog::save() {
	_savedSurface = new MSurface(_width, _height);
	_savedSurface->blitFrom(*_vm->_screen,
		Common::Rect(_position.x, _position.y, _position.x + _width, _position.y + _height),
		Common::Point());

//	_vm->_screen->copyRectToScreen(getBounds());
}

void Dialog::restore() {
	if (_savedSurface) {
		_vm->_screen->blitFrom(*_savedSurface, _position);
		delete _savedSurface;
		_savedSurface = nullptr;

		Common::copy(&_dialogPalette[0], &_dialogPalette[8 * 3],
			&_vm->_palette->_mainPalette[248 * 3]);
		_vm->_palette->setPalette(&_vm->_palette->_mainPalette[248 * 3], 248, 8);
	}
}

void Dialog::draw() {
	// Calculate the dialog positioning
	calculateBounds();

	// Save the screen portion the dialog will overlap
	save();

	setDialogPalette();

	// Draw the dialog
	// Fill entire content of dialog
	Common::Rect bounds = getBounds();
	_vm->_screen->fillRect(bounds, TEXTDIALOG_BACKGROUND);

	// Draw the outer edge lines
	_vm->_screen->hLine(_position.x + 1, _position.y + _height - 2,
		_position.x + _width - 2, TEXTDIALOG_EDGE);
	_vm->_screen->hLine(_position.x, _position.y + _height - 1,
		_position.x + _width - 1, TEXTDIALOG_EDGE);
	_vm->_screen->vLine(_position.x + _width - 2, _position.y + 2,
		_position.y + _height - 2, TEXTDIALOG_EDGE);
	_vm->_screen->vLine(_position.x + _width - 1, _position.y + 1,
		_position.y + _height - 1, TEXTDIALOG_EDGE);

	// Draw the gravelly dialog content
	drawContent(Common::Rect(_position.x + 2, _position.y + 2,
		_position.x + _width - 2, _position.y + _height - 2), 0,
		TEXTDIALOG_CONTENT1, TEXTDIALOG_CONTENT2);
}

void Dialog::setDialogPalette() {
	// Save the high end of the palette, and set up the entries for dialog display
	Common::copy(&_vm->_palette->_mainPalette[TEXTDIALOG_CONTENT1 * 3],
		&_vm->_palette->_mainPalette[TEXTDIALOG_CONTENT1 * 3 + 8 * 3],
		&_dialogPalette[0]);
	Palette::setGradient(_vm->_palette->_mainPalette, TEXTDIALOG_CONTENT1, 2, 0x90, 0x80);
	Palette::setGradient(_vm->_palette->_mainPalette, TEXTDIALOG_EDGE, 2, 0x9C, 0x70);
	Palette::setGradient(_vm->_palette->_mainPalette, TEXTDIALOG_FC, 2, 0x90, 0x80);
	Palette::setGradient(_vm->_palette->_mainPalette, TEXTDIALOG_FE, 1, 0xDC, 0xDC);

	_vm->_palette->setPalette(_vm->_palette->_mainPalette + (TEXTDIALOG_CONTENT1 * 3),
		TEXTDIALOG_CONTENT1, 8);
}

void Dialog::calculateBounds() {
}

void Dialog::drawContent(const Common::Rect &r, int seed, byte color1, byte color2) {
	uint16 currSeed = seed ? seed : 0xB78E;

	Graphics::Surface dest = _vm->_screen->getSubArea(r);
	for (int yp = 0; yp < r.height(); ++yp) {
		byte *destP = (byte *)dest.getBasePtr(0, yp);

		for (int xp = 0; xp < r.width(); ++xp) {
			uint16 seedAdjust = currSeed;
			currSeed += 0x181D;
			seedAdjust = (seedAdjust >> 9) | ((seedAdjust & 0x1ff) << 7);
			currSeed ^= seedAdjust;
			seedAdjust = (seedAdjust >> 3) | ((seedAdjust & 7) << 13);
			currSeed += seedAdjust;

			*destP++ = (currSeed & 0x10) ? color2 : color1;
		}
	}
}

/*------------------------------------------------------------------------*/

TextDialog::TextDialog(MADSEngine *vm, const Common::String &fontName,
		const Common::Point &pos, int maxChars)
	: Dialog(vm) {
	_font = _vm->_font->getFont(fontName);
	_position = pos;
	_portrait = nullptr;
	_edgeSeries = nullptr;
	_piecesPerCenter = 0;
	_fontSpacing = 0;
	_vm->_font->setColors(TEXTDIALOG_BLACK, TEXTDIALOG_BLACK, TEXTDIALOG_BLACK, TEXTDIALOG_BLACK);
	_piecesPerCenter = 0;

	init(maxChars);
}

TextDialog::TextDialog(MADSEngine *vm, const Common::String &fontName,
		const Common::Point &pos, MSurface *portrait, int maxTextChars): Dialog(vm) {
	_font = _vm->_font->getFont(fontName);
	_position = pos;
	_portrait = portrait;
	_edgeSeries = new SpriteAsset(_vm, "box.ss", PALFLAG_RESERVED);
	_vm->_font->setColors(TEXTDIALOG_BLACK, TEXTDIALOG_BLACK, TEXTDIALOG_BLACK, TEXTDIALOG_BLACK);
	_piecesPerCenter = _edgeSeries->getFrame(EDGE_UPPER_CENTER)->w / _edgeSeries->getFrame(EDGE_BOTTOM)->w;
	_fontSpacing = 0;

	int maxLen = estimatePieces(maxTextChars);
	init(maxLen);
}

void TextDialog::init(int maxTextChars) {
	_innerWidth = (_font->maxWidth() + 1) * maxTextChars;
	_width = _innerWidth + 10;
	if (_portrait != nullptr)
		_width += _portrait->w + 10;
	_lineSize = maxTextChars * 2;
	_lineWidth = 0;
	_currentX = 0;
	_numLines = 0;
	Common::fill(&_lineXp[0], &_lineXp[TEXT_DIALOG_MAX_LINES], 0);
	_askLineNum = -1;
	_askXp = 0;
}

int TextDialog::estimatePieces(int maxLen) {
	int fontLen = (_font->maxWidth() + _fontSpacing) * maxLen;
	int pieces = ((fontLen - 1) / _edgeSeries->getFrame(EDGE_TOP)->w) + 1;
	int estimate = (pieces - _piecesPerCenter) / 2;

	return estimate;
}

TextDialog::~TextDialog() {
#ifdef USE_TTS
	if (ConfMan.getBool("tts_narrator")) {
		Common::TextToSpeechManager* ttsMan = g_system->getTextToSpeechManager();
		if (ttsMan != nullptr)
			ttsMan->stop();
	}
#endif

	delete _edgeSeries;
}

void TextDialog::addLine(const Common::String &line, bool underline) {
	if (_lineWidth > 0 || _currentX > 0)
		incNumLines();

	int stringWidth = _font->getWidth(line, 1);
	if (stringWidth >= _innerWidth || (int)line.size() >= _lineSize) {
		wordWrap(line);
	} else {
		_lineXp[_numLines] = (_innerWidth / 2) - (stringWidth / 2);
		_lines[_numLines] = line;

		if (underline)
			underlineLine();
	}

	incNumLines();
}

void TextDialog::underlineLine() {
	_lineXp[_numLines] |= 0x80;
}

void TextDialog::downPixelLine() {
	_lineXp[_numLines] |= 0x40;
}

void TextDialog::incNumLines() {
	_lineWidth = 0;
	_currentX = 0;
	if (++_numLines == TEXT_DIALOG_MAX_LINES)
		error("Exceeded text dialog line max");
}

void TextDialog::wordWrap(const Common::String &line) {
	Common::String tempLine;

	if (!line.empty()) {
		const char *srcP = line.c_str();

		do {
			tempLine = "";
			bool endWord = false;
			bool newLine = false;
			bool continueFlag = true;

			do {
				if (!*srcP) {
					continueFlag = false;
				} else {
					tempLine += *srcP;

					if (*srcP == 10) {
						continueFlag = false;
						newLine = true;
						++srcP;
						tempLine.deleteLastChar();
					} else if (*srcP == ' ') {
						++srcP;
						endWord = true;
					} else if (!endWord) {
						++srcP;
					} else {
						tempLine.deleteLastChar();
						continueFlag = false;
					}
				}
			} while (continueFlag);

			if (tempLine.hasSuffix(" "))
				tempLine.deleteLastChar();

			Common::String tempLine2;
			if (_currentX > 0)
				tempLine2 += ' ';
			tempLine2 += tempLine;

			int lineWidth = _font->getWidth(tempLine2, 1);
			if (((_currentX + (int)tempLine2.size()) > _lineSize) ||
					((_lineWidth + lineWidth) > _innerWidth)) {
				incNumLines();
				appendLine(tempLine);
			} else {
				appendLine(tempLine2);
			}

			if (newLine)
				incNumLines();
		} while (*srcP);
	}
}

void TextDialog::appendLine(const Common::String &line) {
	_currentX += line.size();
	_lineWidth += _font->getWidth(line, 1) + 1;
	_lines[_numLines] += line;
}

void TextDialog::addInput() {
	_askXp = _currentX + 1;
	_askLineNum = _numLines;
	incNumLines();
}

void TextDialog::addBarLine() {
	if (_lineWidth > 0 || _currentX > 0)
		incNumLines();

	_lineXp[_numLines] = 0xFF;
	incNumLines();
}

void TextDialog::setLineXp(int xp) {
	_lineXp[_numLines] = xp;
}

void TextDialog::draw() {
	if (!_lineWidth)
		--_numLines;

	// Figure out the size and position for the dialog
	calculateBounds();

	// Draw the underlying dialog
	Dialog::draw();

	// Draw the edges
	if (_edgeSeries != nullptr) {
		//_vm->_screen->transBlitFrom(*_edgeSeries->getFrame(EDGE_UPPER_LEFT), _position, 0xFF);
		//_vm->_screen->transBlitFrom(*_edgeSeries->getFrame(EDGE_UPPER_RIGHT), _position);
		//_vm->_screen->transBlitFrom(*_edgeSeries->getFrame(EDGE_LOWER_LEFT), Common::Point(_position.x, _position.y + _height));
		//_vm->_screen->transBlitFrom(*_edgeSeries->getFrame(EDGE_LOWER_RIGHT), _position);
	}

	// Draw the portrait
	if (_portrait != nullptr) {
		Common::Point portraitPos = Common::Point(_position.x + 5, _position.y + 5);
		_vm->_screen->transBlitFrom(*_portrait, portraitPos, 0xFF);
	}

	// Draw the text lines
	int lineYp = _position.y + 5;
#ifdef USE_TTS
	Common::String text;
#endif
	for (int lineNum = 0; lineNum <= _numLines; ++lineNum) {
		if (_lineXp[lineNum] == -1) {
			// Draw a line across the entire dialog
			_vm->_screen->hLine(_position.x + 2,
				lineYp + (_font->getHeight() + 1)  / 2,
				_position.x + _width - 4, TEXTDIALOG_BLACK);
		} else {
			// Draw a text line
			int xp = (_lineXp[lineNum] & 0x7F) + _position.x + 5;
			int yp = lineYp;
			if (_lineXp[lineNum] & 0x40)
				++yp;

			if (_portrait != nullptr)
				xp += _portrait->w + 5;
			_font->writeString(_vm->_screen, _lines[lineNum],
				Common::Point(xp, yp), 1);

			if (_lineXp[lineNum] & 0x80) {
				// Draw an underline under the text - used for the header text
				int lineWidth = _font->getWidth(_lines[lineNum], 1);
				_vm->_screen->hLine(xp, yp + _font->getHeight(), xp + lineWidth,
					TEXTDIALOG_BLACK);
			}
#ifdef USE_TTS
			else {
				text += _lines[lineNum];
			}
#endif
		}

		lineYp += _font->getHeight() + 1;
	}

#ifdef USE_TTS
	if (ConfMan.getBool("tts_narrator")) {
		Common::TextToSpeechManager *ttsMan = g_system->getTextToSpeechManager();
		if (ttsMan != nullptr) {
			ttsMan->stop();
			ttsMan->say(text.c_str());
		}
	}
#endif
}

void TextDialog::calculateBounds() {
	_height = (_font->getHeight() + 1) * (_numLines + 1) + 10;
	if (_position.x == -1)
		_position.x = 160 - (_width / 2);
	if (_position.y == -1)
		_position.y = 100 - (_height / 2);

	if ((_position.x + _width) > _vm->_screen->w)
		_position.x = _vm->_screen->w - (_position.x + _width);
	if ((_position.y + _height) > _vm->_screen->h)
		_position.y = _vm->_screen->h - (_position.y + _height);
}

void TextDialog::drawWithInput() {
	//int innerWidth = _innerWidth;
	//int lineHeight = _font->getHeight() + 1;
	//int xp = _position.x + 5;

	// Draw the content of the dialog
	drawContent(Common::Rect(_position.x + 2, _position.y + 2,
		_position.x + _width - 2, _position.y + _height - 2), 0,
		TEXTDIALOG_CONTENT1, TEXTDIALOG_CONTENT2);

	error("TODO: drawWithInput");
}

void TextDialog::show() {
	// Draw the dialog
	draw();
	_vm->_events->showCursor();

	// Wait for mouse click
	do {
		_vm->_events->waitForNextFrame();
	} while (!_vm->shouldQuit() && !_vm->_events->isKeyPressed() && !_vm->_events->_mouseReleased);

	// Allow the mouse release or keypress to be gobbled up
	if (!_vm->shouldQuit()) {
		_vm->_events->waitForNextFrame();
		_vm->_events->_pendingKeys.clear();
	}

	// Restore the background
	restore();
}

/*------------------------------------------------------------------------*/

MessageDialog::MessageDialog(MADSEngine *vm, int maxChars, ...)
	: TextDialog(vm, FONT_INTERFACE, Common::Point(-1, -1), maxChars) {
	// Add in passed line list
	va_list va;
	va_start(va, maxChars);

	const char *line = va_arg(va, const char *);
	while (line) {
		addLine(line);
		line = va_arg(va, const char *);
	}
	va_end(va);
}

/*------------------------------------------------------------------------*/

Dialogs *Dialogs::init(MADSEngine *vm) {
	if (vm->getGameID() == GType_RexNebular)
		return new Nebular::DialogsNebular(vm);
	//else if (vm->getGameID() == GType_Phantom)
	//	return new Phantom::DialogsPhantom(vm);

	// Throw a warning for now, since the associated Dialogs class isn't implemented yet
	warning("Dialogs: Unknown game");
	// HACK: Reuse the implemented Nebular dialogs for now, to avoid crashing later on
	return new Nebular::DialogsNebular(vm);
}

Dialogs::Dialogs(MADSEngine *vm)
	: _vm(vm) {
	_pendingDialog = DIALOG_NONE;
}

/*------------------------------------------------------------------------*/

FullScreenDialog::FullScreenDialog(MADSEngine *vm) : _vm(vm) {
	switch (_vm->getGameID()) {
	case GType_RexNebular:
		_screenId = 990;
		break;
	case GType_Phantom:
		_screenId = 920;
		break;
	case GType_Dragonsphere:
		_screenId = 922;
		break;
	default:
		error("FullScreenDialog:Unknown game");
	}
	_palFlag = true;
}

FullScreenDialog::~FullScreenDialog() {
	_vm->_screen->resetClipBounds();
	_vm->_game->_scene.restrictScene();
}

void FullScreenDialog::display() {
	Game &game = *_vm->_game;
	Scene &scene = game._scene;

	int nextSceneId = scene._nextSceneId;
	int currentSceneId = scene._currentSceneId;
	int priorSceneId = scene._priorSceneId;

	if (_screenId > 0) {
		SceneInfo *sceneInfo = SceneInfo::init(_vm);
		sceneInfo->load(_screenId, 0, "", 0, scene._depthSurface, scene._backgroundSurface);
		delete sceneInfo;
	}

	scene._priorSceneId = priorSceneId;
	scene._currentSceneId = currentSceneId;
	scene._nextSceneId = nextSceneId;

	_vm->_events->initVars();
	game._kernelMode = KERNEL_ROOM_INIT;

	byte pal[768];
	if (_vm->_screenFade) {
		Common::fill(&pal[0], &pal[PALETTE_SIZE], 0);
		_vm->_palette->setFullPalette(pal);
	} else {
		_vm->_palette->getFullPalette(pal);
		_vm->_palette->fadeOut(pal, nullptr, 0, PALETTE_COUNT, 0, 1, 1, 16);
	}

	// Set Fx state and palette entries
	game._fx = _vm->_screenFade == SCREEN_FADE_SMOOTH ? kTransitionFadeIn : kNullPaletteCopy;
	game._trigger = 0;

	// Clear the screen and draw the upper and lower horizontal lines
	_vm->_screen->clear();
	_vm->_palette->setLowRange();
	_vm->_screen->hLine(0, 20, MADS_SCREEN_WIDTH, 2);
	_vm->_screen->hLine(0, 179, MADS_SCREEN_WIDTH, 2);

	// Restrict the screen to the area between the two lines
	_vm->_screen->setClipBounds(Common::Rect(0, DIALOG_TOP, MADS_SCREEN_WIDTH, DIALOG_TOP + MADS_SCENE_HEIGHT));
	_vm->_game->_scene.restrictScene();

	if (_screenId > 0)
		scene._spriteSlots.fullRefresh();
}

} // End of namespace MADS
