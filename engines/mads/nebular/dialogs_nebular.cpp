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

#include "common/scummsys.h"
#include "common/config-manager.h"
#include "common/util.h"
#include "mads/mads.h"
#include "mads/screen.h"
#include "mads/msurface.h"
#include "mads/staticres.h"
#include "mads/nebular/dialogs_nebular.h"
#include "mads/nebular/game_nebular.h"

namespace MADS {

namespace Nebular {

bool DialogsNebular::show(int messageId, int objectId) {
	MADSAction &action = _vm->_game->_scene._action;
	Common::StringArray msg = _vm->_game->getMessage(messageId);
	Common::String title;
	Common::String commandText;
	Common::String valStr;
	Common::String dialogText;
	bool result = true;
	bool centerFlag = false;
	bool underlineFlag = false;
	bool commandFlag = false;
	bool crFlag = false;
	TextDialog *dialog = nullptr;
	_dialogWidth = 17;
	_capitalizationMode = kUppercase;

	// Loop through the lines of the returned text
	for (uint idx = 0; idx < msg.size(); ++idx) {
		Common::String srcLine = msg[idx];
		const char *srcP = srcLine.c_str();

		// Loop through the text of the line
		while (srcP < srcLine.c_str() + srcLine.size()) {
			if (*srcP == '[') {
				// Starting a command
				commandText = "";
				commandFlag = true;
			} else if (*srcP == ']') {
				// Ending a command
				if (commandFlag) {
					if (commandCheck("CENTER", valStr, commandText)) {
						centerFlag = true;
					} else if (commandCheck("TITLE", valStr, commandText)) {
						centerFlag = true;
						underlineFlag = true;
						crFlag = true;
						int v = atoi(valStr.c_str());
						if (v != 0)
							_dialogWidth = v;
					} else if (commandCheck("CR", valStr, commandText)) {
						if (centerFlag) {
							crFlag = true;
						} else {
							if (objectId == -1) {
								dialog = new TextDialog(_vm, FONT_INTERFACE, _defaultPosition, _dialogWidth);
							} else {
								dialog = new PictureDialog(_vm, _defaultPosition, _dialogWidth, objectId);
							}
							dialog->wordWrap(dialogText);
							dialog->incNumLines();
						}
					} else if (commandCheck("ASK", valStr, commandText)) {
						dialog->addInput();
					} else if (commandCheck("VERB", valStr, commandText)) {
						dialogText += getVocab(action._activeAction._verbId);
					} else if (commandCheck("INDEX", valStr, commandText)) {
						int idxLocal = atoi(valStr.c_str());
						if (_indexList[idxLocal])
							dialogText += getVocab(_indexList[idxLocal]);
					} else if (commandCheck("NUMBER", valStr, commandText)) {
						int idxLocal = atoi(valStr.c_str());
						dialogText += Common::String::format("%.4d", _indexList[idxLocal]);
					} else if (commandCheck("NOUN1", valStr, commandText)) {
						if (!textNoun(dialogText, 1, valStr))
							dialogText += getVocab(action._activeAction._objectNameId);
					} else if (commandCheck("NOUN2", valStr, commandText)) {
						if (!textNoun(dialogText, 2, valStr))
							dialogText += getVocab(action._activeAction._indirectObjectId);
					} else if (commandCheck("PREP", valStr, commandText)) {
						dialogText += kArticleList[action._savedFields._articleNumber];
					} else if (commandCheck("SENTENCE", valStr, commandText)) {
						dialogText += action._sentence;
					} else if (commandCheck("WIDTH", valStr, commandText)) {
						_dialogWidth = atoi(valStr.c_str());
					} else if (commandCheck("BAR", valStr, commandText)) {
						dialog->addBarLine();
					} else if (commandCheck("UNDER", valStr, commandText)) {
						underlineFlag = true;
					} else if (commandCheck("DOWN", valStr, commandText)) {
						dialog->downPixelLine();
					} else if (commandCheck("TAB", valStr, commandText)) {
						int xp = atoi(valStr.c_str());
						dialog->setLineXp(xp);
					}
				}

				commandFlag = false;
			} else if (commandFlag) {
				// Add the next character to the command
				commandText += *srcP;
			} else {
				// Add to the text to be displayed in the dialog
				dialogText += *srcP;
			}

			++srcP;
		}

		if (!dialog) {
			if (objectId == -1) {
				dialog = new TextDialog(_vm, FONT_INTERFACE, _defaultPosition, _dialogWidth);
			} else {
				dialog = new PictureDialog(_vm, _defaultPosition, _dialogWidth, objectId);
			}
		}

		if (centerFlag) {
			dialog->addLine(dialogText, underlineFlag);
			if (crFlag)
				dialog->incNumLines();
		} else {
			dialog->wordWrap(dialogText);
		}

		// Reset line processing flags in preparation for next line
		dialogText = "";
		commandFlag = false;
		underlineFlag = false;
		centerFlag = false;
		crFlag = false;
	}

	if (!centerFlag)
		dialog->incNumLines();

	// Show the dialog
	_vm->_events->setCursor(CURSOR_ARROW);
	dialog->show();

	delete dialog;
	return result;
}

void DialogsNebular::showItem(int objectId, int messageId, int speech) {
	// MADS engine doesn't currently support speech
	assert(!speech);

	show(messageId, objectId);
}

Common::String DialogsNebular::getVocab(int vocabId) {
	assert(vocabId > 0);

	Common::String vocab = _vm->_game->_scene.getVocab(vocabId);

	switch (_capitalizationMode) {
	case kUppercase:
		vocab.toUppercase();
		break;
	case kLowercase:
		vocab.toLowercase();
		break;
	case kUpperAndLower:
		vocab.toLowercase();
		vocab.setChar(toupper(vocab[0]), 0);
	default:
		break;
	}

	return vocab;
}

bool DialogsNebular::textNoun(Common::String &dest, int nounId, const Common::String &source) {
	// Ensure the destination has parameter specifications
	if (!source.hasPrefix(":"))
		return false;

	// Extract the first (singular) result value
	Common::String param1 = Common::String(source.c_str() + 1);
	Common::String param2;
	const char *sepChar = strchr(source.c_str() + 1, ':');
	if (sepChar) {
		param1 = Common::String(source.c_str() + 1, sepChar);

		// Get the second, plural form
		param2 = Common::String(sepChar + 1);
	}

	// Get the vocab to use
	MADSAction &action = _vm->_game->_scene._action;
	Common::String vocab = _vm->_dialogs->getVocab(action._activeAction._verbId);
	Common::String *str;

	if (vocab.hasSuffix("s") || vocab.hasSuffix("S")) {
		str = &param2;
	} else {
		str = &param1;

		if (param1 == "a ") {
			switch (toupper(vocab[0])) {
			case 'A':
			case 'E':
			case 'I':
			case 'O':
			case 'U':
				param1 = "an ";
				break;
			default:
				break;
			}
		}
	}

	dest += *str;
	return true;
}

bool DialogsNebular::commandCheck(const char *idStr, Common::String &valStr,
		const Common::String &command) {
	uint idLen = strlen(idStr);

	valStr = (command.size() <= idLen) ? "" : Common::String(command.c_str() + idLen);

	// Check whether the command starts with the given Id
	int result = scumm_strnicmp(idStr, command.c_str(), idLen) == 0;
	if (!result)
		return false;

	// It does, so set the command case mode
	if (Common::isUpper(command[0]) && Common::isUpper(command[1])) {
		_capitalizationMode = kUppercase;
	} else if (Common::isUpper(command[0])) {
		_capitalizationMode = kUpperAndLower;
	} else {
		_capitalizationMode = kLowercase;
	}

	return true;
}

void DialogsNebular::showDialog() {
	switch (_pendingDialog) {
	case DIALOG_GAME_MENU:
		//GameMenuDialog::show();
		break;
	case DIALOG_DIFFICULTY: {
/*
		DifficultyDialog *dlg = new DifficultyDialog(_vm);
		dlg->show();
		delete dlg;
*/
		break;
	}
	default:
		break;
	}
}

/*------------------------------------------------------------------------*/

CopyProtectionDialog::CopyProtectionDialog(MADSEngine *vm, bool priorAnswerWrong) :
TextDialog(vm, FONT_INTERFACE, Common::Point(-1, -1), 32) {
	getHogAnusEntry(_hogEntry);

	if (priorAnswerWrong) {
		addLine("ANSWER INCORRECT!", true);
		wordWrap("\n");
		addLine("(But we'll give you another chance!)");
	}
	else {
		addLine("REX NEBULAR version 8.43", true);
		wordWrap("\n");
		addLine("(Copy Protection, for your convenience)");
	}
	wordWrap("\n");

	wordWrap("Now comes the part that everybody hates.  But if we don't");
	wordWrap("do this, nasty rodent-like people will pirate this game");
	wordWrap("and a whole generation of talented designers, programmers,");
	wordWrap("artists, and playtesters will go hungry, and will wander");
	wordWrap("aimlessly through the land at night searching for peace.");
	wordWrap("So let's grit our teeth and get it over with.  Just get");

	Common::String line = "out your copy of ";
	line += _hogEntry._bookId == 103 ? "the GAME MANUAL" : "REX'S LOGBOOK";
	line += ".  See!  That was easy.  ";
	wordWrap(line);

	line = Common::String::format("Next, just turn to page %d. On line %d, find word number %d, ",
		_hogEntry._pageNum, _hogEntry._lineNum, _hogEntry._wordNum);
	wordWrap(line);

	wordWrap("and type it on the line below (we',27h,'ve even given you");
	wordWrap("first letter as a hint).  As soon as you do that, we can get");
	wordWrap("right into this really COOL adventure game!\n");
	wordWrap("\n");
	wordWrap("                    ");
	addInput();
	wordWrap("\n");
}

void CopyProtectionDialog::show() {
	draw();
	_vm->_events->showCursor();

	// TODO: Replace with text input
	while (!_vm->shouldQuit() && !_vm->_events->isKeyPressed() &&
		!_vm->_events->_mouseClicked) {
		_vm->_events->delay(1);
	}

	_vm->_events->_pendingKeys.clear();
}

bool CopyProtectionDialog::getHogAnusEntry(HOGANUS &entry) {
	File f;
	f.open("*HOGANUS.DAT");

	// Read in the total number of entries, and randomly pick an entry to use
	int numEntries = f.readUint16LE();
	int entryIndex = _vm->getRandomNumber(1, numEntries);

	// Read in the encrypted entry
	f.seek(28 * entryIndex + 2);
	byte entryData[28];
	f.read(entryData, 28);

	// Decrypt it
	for (int i = 0; i < 28; ++i)
		entryData[i] = ~entryData[i];

	// Fill out the fields
	entry._bookId = entryData[0];
	entry._pageNum = READ_LE_UINT16(&entryData[2]);
	entry._lineNum = READ_LE_UINT16(&entryData[4]);
	entry._wordNum = READ_LE_UINT16(&entryData[6]);
	entry._word = Common::String((char *)&entryData[8]);

	f.close();
	return true;
}

/*------------------------------------------------------------------------*/

PictureDialog::PictureDialog(MADSEngine *vm, const Common::Point &pos,
		int maxChars, int objectId) :
		TextDialog(vm, FONT_INTERFACE, pos, maxChars), _objectId(objectId) {
	// Turn off cycling if active
	Scene &scene = _vm->_game->_scene;
	_cyclingActive = scene._cyclingActive;
	scene._cyclingActive = false;
}

PictureDialog::~PictureDialog() {
	// Restore cycling flag
	Scene &scene = _vm->_game->_scene;
	scene._cyclingActive = _cyclingActive;
}

void PictureDialog::save() {
	Palette &palette = *_vm->_palette;
	byte map[PALETTE_COUNT];

	// Save the entire screen
	_savedSurface = new MSurface(MADS_SCREEN_WIDTH, MADS_SCREEN_HEIGHT);
	_vm->_screen.copyTo(_savedSurface);

	// Save palette information
	Common::copy(&palette._mainPalette[0], &palette._mainPalette[PALETTE_SIZE], &_palette[0]);
	Common::copy(&palette._palFlags[0], &palette._palFlags[PALETTE_COUNT], &_palFlags[0]);
	_rgbList.copy(palette._rgbList);

	// Set up palette allocation
	Common::fill(&palette._colorFlags[0], &palette._colorFlags[3], true);

	uint32 *palFlagP = &palette._palFlags[0];
	for (int idx = 0; idx < PALETTE_COUNT; ++idx, ++palFlagP) {
		if (idx < PALETTE_RESERVED_LOW_COUNT ||
			idx >= (PALETTE_COUNT - PALETTE_RESERVED_HIGH_COUNT - 10)) {
			*palFlagP = 1;
			map[idx] = idx;
		} else {
			*palFlagP = 0;
		}
	}

	// Reset the flag list
	palette._rgbList.reset();

	// Fade the screen to grey
	int numColors = PALETTE_COUNT - PALETTE_RESERVED_LOW_COUNT - PALETTE_RESERVED_HIGH_COUNT;
	palette.fadeOut(palette._mainPalette, &map[PALETTE_RESERVED_LOW_COUNT],
		PALETTE_RESERVED_LOW_COUNT, numColors, 248, 8, 1, 16);

	// Remap the greyed out screen to use the small greyscale range
	// at the top end of the palette
	_vm->_screen.translate(map);

	// Load the inventory picture
	Common::String setName = Common::String::format("*OB%.3d.SS", _objectId);
	SpriteAsset *asset = new SpriteAsset(_vm, setName, 0x8000);
	palette.setFullPalette(palette._mainPalette);

	// Get the inventory frame, and adjust the dialog position to allow for it
	MSprite *frame = asset->getFrame(0);
	_position.y = frame->h + 12;
	if ((_position.y + _height) > _vm->_screen.getHeight())
		_position.y -= (_position.y + _height) - _vm->_screen.getHeight();

	// Draw the inventory picture
	frame->copyTo(&_vm->_screen, Common::Point(160 - frame->w / 2, 6),
		frame->getTransparencyIndex());
	_vm->_screen.copyRectToScreen(_vm->_screen.getBounds());

	// Adjust the dialog colors to use
	TEXTDIALOG_CONTENT1 -= 10;
	TEXTDIALOG_CONTENT2 -= 10;
	TEXTDIALOG_EDGE -= 10;
	TEXTDIALOG_BACKGROUND -= 10;
	TEXTDIALOG_FC -= 10;
	TEXTDIALOG_FD -= 10;
	TEXTDIALOG_FE -= 10;
}

void PictureDialog::restore() {
	if (_savedSurface) {
		_savedSurface->copyTo(&_vm->_screen);
		delete _savedSurface;
		_savedSurface = nullptr;

		_vm->_screen.copyRectToScreen(_vm->_screen.getBounds());

		// Restore palette information
		Palette &palette = *_vm->_palette;
		Common::copy(&_palette[0], &_palette[PALETTE_SIZE], &palette._mainPalette[0]);
		_vm->_palette->setFullPalette(palette._mainPalette);
		Common::copy(&_palFlags[0], &_palFlags[PALETTE_COUNT], &palette._palFlags[0]);
		palette._rgbList.copy(_rgbList);

		_vm->_dialogs->_defaultPosition.y = -1;
	}
}

/*------------------------------------------------------------------------*/

ScreenDialog::DialogLine::DialogLine() {
	_active = true;
	_state = DLGSTATE_UNSELECTED;
	_textDisplayIndex = -1;
	_font = nullptr;
	_widthAdjust = 0;
}

ScreenDialog::DialogLine::DialogLine(const Common::String &s) {
	_state = DLGSTATE_UNSELECTED;
	_textDisplayIndex = -1;
	_font = nullptr;
	_widthAdjust = -1;
	_msg = s;
}

/*------------------------------------------------------------------------*/

ScreenDialog::ScreenDialog(MADSEngine *vm) : _vm(vm) {
	Game &game = *_vm->_game;
	Scene &scene = game._scene;

	_v1 = 0;
	_v2 = 0;
	_v3 = false;
	_selectedLine = 0;
	_dirFlag = false;
	_textLineCount = 0;
	_screenId = 920;

	chooseBackground();
	game.loadQuoteSet(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
		17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33,
		34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 0);
	game._kernelMode = KERNEL_ROOM_PRELOAD;
	_vm->_events->waitCursor();
	scene.clearVocab();
	scene._dynamicHotspots.clear();
	_vm->_dialogs->_defaultPosition = Common::Point(-1, -1);

	bool palFlag = false;
	int nextSceneId = scene._nextSceneId;
	int currentSceneId = scene._currentSceneId;
	int priorSceneId = scene._priorSceneId;

	if (_vm->_dialogs->_pendingDialog == DIALOG_DIFFICULTY) {
		palFlag = true;
	} else {
		_vm->_palette->initPalette();
	}
	scene.loadScene(_screenId, game._aaName, palFlag);

	scene._priorSceneId = priorSceneId;
	scene._currentSceneId = currentSceneId;
	scene._nextSceneId = nextSceneId;
	scene._posAdjust.y = 22;
	_vm->_sound->pauseNewCommands();
	_vm->_events->initVars();
	game._kernelMode = KERNEL_ROOM_INIT;

	SpriteAsset *menuSprites = new SpriteAsset(_vm, "*MENU", 0);
	_menuSpritesIndex = scene._sprites.add(menuSprites);

	byte pal[768];
	if (_vm->_screenFade) {
		Common::fill(&pal[0], &pal[PALETTE_SIZE], 0);
		_vm->_palette->setFullPalette(pal);
	} else {
		_vm->_palette->getFullPalette(pal);
		_vm->_palette->fadeOut(pal, nullptr, 0, PALETTE_COUNT, 0, 1, 1, 16);
	}

	_vm->_screen.empty();
	_vm->_screen.hLine(0, 0, MADS_SCREEN_WIDTH, 2);

	game._fx = _vm->_screenFade == SCREEN_FADE_SMOOTH ? kTransitionFadeIn : kCenterVertTransition;
	game._trigger = 0;
	_vm->_events->setCursor(CURSOR_ARROW);

	_vm->_palette->setEntry(10, 0, 63, 0);
	_vm->_palette->setEntry(11, 0, 45, 0);
	_vm->_palette->setEntry(12, 63, 63, 0);
	_vm->_palette->setEntry(13, 45, 45, 0);
	_vm->_palette->setEntry(14, 63, 63, 63);
	_vm->_palette->setEntry(15, 45, 45, 45);

	_lineIndex = -1;
}

void ScreenDialog::clearLines() {
	Scene &scene = _vm->_game->_scene;
	_v2 = 0;
	_lines.clear();
	scene._spriteSlots.fullRefresh(true);
}

void ScreenDialog::setClickableLines() {
	ScreenObjects &screenObjects = _vm->_game->_screenObjects;

	for (uint idx = 0; idx < _lines.size(); ++idx) {
		if (_lines[idx]._active) {
			const Common::Point &pt = _lines[idx]._pos;
			int strWidth = _lines[idx]._font->getWidth(_lines[idx]._msg);
			int maxHeight = _lines[idx]._font->getHeight();

			screenObjects.add(Common::Rect(pt.x, pt.y, pt.x + strWidth, pt.y + maxHeight - 1),
				LAYER_GUI, CAT_COMMAND, idx);
		}
	}

	if (_vm->_dialogs->_pendingDialog == DIALOG_SAVE ||
			_vm->_dialogs->_pendingDialog == DIALOG_RESTORE) {
		screenObjects.add(Common::Rect(293, 26, 312, 75), LAYER_GUI, CAT_INV_LIST, 50);
		screenObjects.add(Common::Rect(293, 78, 312, 127), LAYER_GUI, CAT_INV_LIST, 51);
	}
}

void ScreenDialog::addQuote(int id1, int id2, DialogTextAlign align,
		const Common::Point &pt, Font *font) {
	Common::String msg = _vm->_game->getQuote(id1);

	if (id2 > 0) {
		msg += " ";
		msg += _vm->_game->getQuote(id2);
	}

	addLine(msg, align, pt, font);
}

void ScreenDialog::addLine(const Common::String &msg, DialogTextAlign align,
		const Common::Point &pt, Font *font) {
	Scene &scene = _vm->_game->_scene;
	DialogLine *line;

	if (font == nullptr)
		font = _vm->_font->getFont(FONT_CONVERSATION);

	if (_lineIndex < (int)_lines.size()) {
		if (_lines.size() >= 20) {
			++_lineIndex;
			return;
		}

		_lines.push_back(msg);
		line = &_lines[_lines.size() - 1];
	} else {
		line = &_lines[_lineIndex];
		if (msg.compareToIgnoreCase(msg)) {
			++_lineIndex;
			return;
		}

		if (line->_textDisplayIndex >= 0) {
			TextDisplay &textDisplay = scene._textDisplay[line->_textDisplayIndex];
			if (textDisplay._active) {
				textDisplay._expire = -1;
				if (_textLineCount < 20) {
					textDisplay._msg = msg;
					++_textLineCount;
				}
			}
		}
	}

	line->_font = font;
	line->_state = DLGSTATE_UNSELECTED;
	line->_pos = pt;
	line->_widthAdjust = -1;
	line->_textDisplayIndex = -1;

	int xOffset;
	switch (align) {
	case ALIGN_CENTER:
		xOffset = (MADS_SCREEN_WIDTH / 2) - font->getWidth(msg, -1) / 2;
		line->_pos.x += xOffset;
		break;

	case ALIGN_AT_CENTER: {
		const char *msgP = msg.c_str();
		const char *ch = strchr(msgP, '@');
		if (ch) {
			xOffset = (MADS_SCREEN_WIDTH / 2) - font->getWidth(
				Common::String(msgP, ch), line->_widthAdjust);
			line->_pos.x += xOffset;
		}
		break;
	}

	case ALIGN_RIGHT:
		xOffset = font->getWidth(msg, -1);
		line->_pos.x -= xOffset;
		break;

	default:
		break;
	}

	++_lineIndex;
}

void ScreenDialog::initVars() {
	_v1 = -1;
	_selectedLine = -1;
	_lineIndex = 0;
	_textLineCount = 0;
}

void ScreenDialog::chooseBackground() {
	switch (_vm->_game->_currentSectionNumber) {
	case 1:
	case 2:
		_screenId = 921;
		break;
	case 3:
	case 4:
		_screenId = 922;
		break;
	case 5:
	case 6:
	case 7:
		_screenId = 923;
		break;
	case 8:
		_screenId = 924;
		break;
	default:
		_screenId = 920;
		break;
	}
}

void ScreenDialog::setFrame(int frameNumber, int depth) {
	Scene &scene = _vm->_game->_scene;
	SpriteSlot &spriteSlot = scene._spriteSlots[scene._spriteSlots.add()];
	spriteSlot._flags = IMG_UPDATE;
	spriteSlot._seqIndex = 1;
	spriteSlot._spritesIndex = _menuSpritesIndex;
	spriteSlot._frameNumber = frameNumber;
}

void ScreenDialog::show() {
	Scene &scene = _vm->_game->_scene;

	while (_selectedLine < 1 && !_vm->shouldQuit()) {
		handleEvents();
		if (_v3) {
			if (!_v1)
				_v1 = -1;

			refreshText();
			scene.drawElements(_vm->_game->_fx, _vm->_game->_fx);
			_v3 = false;
		}

		_vm->_events->waitForNextFrame();
		_vm->_game->_fx = kTransitionNone;
	}
}

void ScreenDialog::handleEvents() {
	ScreenObjects &screenObjects = _vm->_game->_screenObjects;
	EventsManager &events = *_vm->_events;
	Nebular::DialogsNebular &dialogs = *(Nebular::DialogsNebular *)_vm->_dialogs;
	int v1 = _v1;

	// Mark all the lines as initially unselected
	for (uint i = 0; i < _lines.size(); ++i)
		_lines[i]._state = DLGSTATE_UNSELECTED;

	// Process pending events
	_vm->_events->pollEvents();

	// Scan for objects in the dialog
	int objIndex = screenObjects.scan(events.currentPos() - _vm->_screen._offset, LAYER_GUI);

	if (_v2) {
		int yp = events.currentPos().y - _vm->_screen._offset.y;
		if (yp < screenObjects[1]._bounds.top) {
			if (!events._mouseReleased)
				_lines[1]._state = DLGSTATE_SELECTED;
			objIndex = 19;
		}

		if (yp < screenObjects[7]._bounds.bottom) {
			if (!events._mouseReleased)
				_lines[1]._state = DLGSTATE_SELECTED;
			objIndex = 20;
		}
	}

	int line = -1;
	if (objIndex > 0 || events._mouseButtons) {
		line = screenObjects[objIndex]._descId;
		if (dialogs._pendingDialog == DIALOG_SAVE || dialogs._pendingDialog == DIALOG_RESTORE) {
			if (line > 7 && line <= 14) {
				_lines[line]._state = DLGSTATE_UNSELECTED;
				line -= 7;
			}

			int v2 = (line > 0 && line < 8) ? 1 : 0;
			if (events._mouseMoved)
				_v2 = v2;
		}

		if (screenObjects[objIndex]._category == CAT_COMMAND) {
			_lines[line]._state = DLGSTATE_SELECTED;
		}
	}
	if (!line)
		line = -1;

	if (dialogs._pendingDialog == DIALOG_ERROR && line == 1)
		line = -1;

	if (events._mouseReleased) {
		if (!_v2 || line <= 18)
			_selectedLine = line;
		_v3 = true;
	}

	_v1 = line;
	if (v1 == line || _selectedLine >= 0)
		_v3 = true;
}

void ScreenDialog::refreshText() {
	Scene &scene = _vm->_game->_scene;

	for (uint i = 0; i < _lines.size(); ++i) {
		if (_lines[i]._active) {
			int fontColor;
			switch (_lines[i]._state) {
			case DLGSTATE_UNSELECTED:
				fontColor = 0xB0A;
				break;
			case DLGSTATE_SELECTED:
				fontColor = 0xD0C;
				break;
			default:
				fontColor = 0xF0E;
				break;
			}

			bool skipFlag = false;
			if (_lines[i]._textDisplayIndex >= 0) {
				TextDisplay &textDisplay = scene._textDisplay[_lines[i]._textDisplayIndex];
				int currCol = textDisplay._color1;
				if (currCol != fontColor) {
					scene._textDisplay.expire(_lines[i]._textDisplayIndex);
					_lines[i]._textDisplayIndex = -1;
				} else {
					skipFlag = true;
				}
			}

			if (!skipFlag) {
				_lines[i]._textDisplayIndex = scene._textDisplay.add(_lines[i]._pos.x, _lines[i]._pos.y, 
					fontColor, _lines[i]._widthAdjust, _lines[i]._msg, _lines[i]._font);
			}
		}
	}
}

/*------------------------------------------------------------------------*/

DifficultyDialog::DifficultyDialog(MADSEngine *vm) : ScreenDialog(vm) {
	setFrame(8, 2);
	setLines();
	setClickableLines();
}

void DifficultyDialog::setLines() {
	Font *font = _vm->_font->getFont(FONT_CONVERSATION);
	int yp = 78 - ((font->getHeight() + 1) * 4 + 6) / 2;

	addQuote(41, 0, ALIGN_CENTER, Common::Point(0, yp), font);
	yp += 6;

	for (int id = 42; id <= 44; ++id) {
		yp += font->getHeight();
		addQuote(id, 0, ALIGN_CENTER, Common::Point(0, yp));
	}
}

void DifficultyDialog::show() {
	ScreenDialog::show();
	Nebular::GameNebular &game = *(Nebular::GameNebular *)_vm->_game;

	switch (_selectedLine) {
	case 1:
		game._difficulty = Nebular::DIFFICULTY_HARD;
		break;
	case 2:
		game._difficulty = Nebular::DIFFICULTY_MEDIUM;
		break;
	case 3:
		game._difficulty = Nebular::DIFFICULTY_EASY;
		break;
	default:
		_vm->quitGame();
	}
}

/*------------------------------------------------------------------------*/

GameMenuDialog::GameMenuDialog(MADSEngine *vm) : ScreenDialog(vm) {
	setFrame(1, 2);
}

void GameMenuDialog::addLines() {
	initVars();
	Font *font = _vm->_font->getFont(FONT_CONVERSATION);
	int top = 78 - (font->getHeight() + 2) * 12;
	addQuote(10, 0, ALIGN_CENTER, Common::Point(0, top),  font);
	// TODO
}

} // End of namespace Nebular

} // End of namespace MADS
