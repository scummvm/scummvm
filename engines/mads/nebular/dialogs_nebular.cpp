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
#include "common/util.h"
#include "common/translation.h"

#include "gui/saveload.h"

#include "mads/mads.h"
#include "mads/screen.h"
#include "mads/msurface.h"
#include "mads/staticres.h"
#include "mads/nebular/dialogs_nebular.h"
#include "mads/nebular/game_nebular.h"
#include "mads/nebular/menu_nebular.h"

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
							if (dialog)
								delete dialog;

							if (objectId == -1)
								dialog = new TextDialog(_vm, FONT_INTERFACE, _defaultPosition, _dialogWidth);
							else
								dialog = new PictureDialog(_vm, _defaultPosition, _dialogWidth, objectId);

							dialog->wordWrap(dialogText);
							dialog->incNumLines();
						}
					} else if (commandCheck("ASK", valStr, commandText)) {
						if (!dialog)
							error("DialogsNebular::show - Uninitialized dialog");
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
						if (!dialog)
							error("DialogsNebular::show - Uninitialized dialog");
						dialog->addBarLine();
					} else if (commandCheck("UNDER", valStr, commandText)) {
						underlineFlag = true;
					} else if (commandCheck("DOWN", valStr, commandText)) {
						if (!dialog)
							error("DialogsNebular::show - Uninitialized dialog");
						dialog->downPixelLine();
					} else if (commandCheck("TAB", valStr, commandText)) {
						if (!dialog)
							error("DialogsNebular::show - Uninitialized dialog");
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
			if (objectId == -1)
				dialog = new TextDialog(_vm, FONT_INTERFACE, _defaultPosition, _dialogWidth);
			else
				dialog = new PictureDialog(_vm, _defaultPosition, _dialogWidth, objectId);
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

	if (!dialog)
		error("DialogsNebular::show - Uninitialized dialog");

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
	while (_pendingDialog != DIALOG_NONE && !_vm->shouldQuit()) {
		DialogId dialogId = _pendingDialog;
		_pendingDialog = DIALOG_NONE;

		switch (dialogId) {
		case DIALOG_MAIN_MENU: {
			MainMenu *menu = new MainMenu(_vm);
			menu->show();
			delete menu;
			break;
		}
		case DIALOG_DIFFICULTY: {
			DifficultyDialog *dlg = new DifficultyDialog(_vm);
			dlg->show();
			delete dlg;
			break;
		}
		case DIALOG_GAME_MENU: {
			GameMenuDialog *dlg = new GameMenuDialog(_vm);
			dlg->show();
			delete dlg;
			break;
		}
		case DIALOG_SAVE: {
			showScummVMSaveDialog();
			break;
		}
		case DIALOG_RESTORE: {
			showScummVMRestoreDialog();
			break;
		}
		case DIALOG_OPTIONS: {
			OptionsDialog *dlg = new OptionsDialog(_vm);
			dlg->show();
			delete dlg;
			break;
		}
		case DIALOG_ADVERT: {
			AdvertView *dlg = new AdvertView(_vm);
			dlg->show();
			delete dlg;
			break;
		}
		case DIALOG_TEXTVIEW: {
			TextView *dlg = new RexTextView(_vm);
			dlg->show();
			delete dlg;
			return;
		}
		case DIALOG_ANIMVIEW: {
			AnimationView *dlg = new RexAnimationView(_vm);
			dlg->show();
			delete dlg;
			break;
		}
		default:
			break;
		}
	}
}

void DialogsNebular::showScummVMSaveDialog() {
	Nebular::GameNebular &game = *(Nebular::GameNebular *)_vm->_game;
	Scene &scene = game._scene;
	GUI::SaveLoadChooser *dialog = new GUI::SaveLoadChooser(_("Save game:"), _("Save"), true);

	int slot = dialog->runModalWithCurrentTarget();
	if (slot >= 0) {
		Common::String desc = dialog->getResultString();

		if (desc.empty()) {
			// create our own description for the saved game, the user didn't enter it
			desc = dialog->createDefaultSaveDescription(slot);
		}

		scene._spriteSlots.reset();
		scene.loadScene(scene._currentSceneId, game._aaName, true);
		scene._userInterface.noInventoryAnim();
		game._scene.drawElements(kTransitionFadeIn, false);

		game.saveGame(slot, desc);
	}

	// Flag for scene loading that we're returning from a dialog
	scene._currentSceneId = RETURNING_FROM_DIALOG;

	delete dialog;
}

void DialogsNebular::showScummVMRestoreDialog() {
	Nebular::GameNebular &game = *(Nebular::GameNebular *)_vm->_game;
	GUI::SaveLoadChooser *dialog = new GUI::SaveLoadChooser(_("Restore game:"), _("Restore"), false);
	Scene &scene = game._scene;

	int slot = dialog->runModalWithCurrentTarget();
	if (slot >= 0) {
		game._loadGameSlot = slot;
		game._scene._currentSceneId = RETURNING_FROM_LOADING;
		game._currentSectionNumber = -1;
	} else {
		// Flag for scene loading that we're returning from a dialog
		scene._currentSceneId = RETURNING_FROM_DIALOG;
	}

	delete dialog;
}

/*------------------------------------------------------------------------*/

CopyProtectionDialog::CopyProtectionDialog(MADSEngine *vm, bool priorAnswerWrong) :
TextDialog(vm, FONT_INTERFACE, Common::Point(-1, -1), 32) {
	getHogAnusEntry(_hogEntry);

	if (priorAnswerWrong) {
		addLine("ANSWER INCORRECT!", true);
		wordWrap("\n");
		addLine("(But we'll give you another chance!)");
	} else {
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

	wordWrap("and type it on the line below (we've even given you");
	wordWrap("first letter as a hint).  As soon as you do that, we can get");
	wordWrap("right into this really COOL adventure game!\n");
	wordWrap("\n");
	wordWrap("                    ");
	addInput();
	wordWrap("\n");
}

void CopyProtectionDialog::show() {
	draw();

	Common::KeyState curKey;
	const Common::Rect inputArea(110, 165, 210, 175);
	MSurface *origInput = new MSurface(inputArea.width(), inputArea.height());
	_vm->_screen->frameRect(inputArea, TEXTDIALOG_BLACK);
	origInput->blitFrom(*_vm->_screen, inputArea, Common::Point(0, 0));
		_font->setColors(TEXTDIALOG_FE, TEXTDIALOG_FE, TEXTDIALOG_FE, TEXTDIALOG_FE);
	_vm->_screen->update();

	bool firstTime = true;

	while (!_vm->shouldQuit()) {
		if (!firstTime) {
			while (!_vm->shouldQuit() && !_vm->_events->isKeyPressed()) {
				_vm->_events->delay(1);
			}

			if (_vm->shouldQuit())
				break;

			curKey = _vm->_events->getKey();

			if (curKey.keycode == Common::KEYCODE_RETURN || curKey.keycode == Common::KEYCODE_KP_ENTER)
				break;
			else if (curKey.keycode == Common::KEYCODE_BACKSPACE)
				_textInput.deleteLastChar();
			else if (_textInput.size() < 14)
				_textInput += curKey.ascii;

			_vm->_events->_pendingKeys.clear();
		} else {
			firstTime = false;
			_textInput = _hogEntry._word[0];
		}

		_vm->_screen->blitFrom(*origInput, Common::Point(inputArea.left, inputArea.top));
		_font->writeString(_vm->_screen, _textInput,
			Common::Point(inputArea.left + 2, inputArea.top + 1), 1);
		_vm->_screen->update();
	}

	origInput->free();
	delete origInput;
}

bool CopyProtectionDialog::isCorrectAnswer() {
	return _hogEntry._word == _textInput;
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
	_savedSurface->blitFrom(*_vm->_screen);

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
	_vm->_screen->translate(map);

	// Load the inventory picture
	Common::String setName = Common::String::format("*OB%.3d.SS", _objectId);
	SpriteAsset *asset = new SpriteAsset(_vm, setName, 0x8000);
	palette.setFullPalette(palette._mainPalette);

	// Get the inventory frame, and adjust the dialog position to allow for it
	MSprite *frame = asset->getFrame(0);
	_position.y = frame->h + 12;
	if ((_position.y + _height) > _vm->_screen->h)
		_position.y -= (_position.y + _height) - _vm->_screen->h;

	// Draw the inventory picture
	_vm->_screen->transBlitFrom(*frame, Common::Point(160 - frame->w / 2, 6),
		frame->getTransparencyIndex());

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
		_vm->_screen->blitFrom(*_savedSurface);
		_savedSurface->free();
		delete _savedSurface;
		_savedSurface = nullptr;

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

GameDialog::DialogLine::DialogLine() {
	_active = true;
	_state = DLGSTATE_UNSELECTED;
	_textDisplayIndex = -1;
	_font = nullptr;
	_widthAdjust = 0;
	_msg = "";
}

GameDialog::DialogLine::DialogLine(const Common::String &s) {
	_active = true;
	_state = DLGSTATE_UNSELECTED;
	_textDisplayIndex = -1;
	_font = nullptr;
	_widthAdjust = -1;
	_msg = s;
}

/*------------------------------------------------------------------------*/

GameDialog::GameDialog(MADSEngine *vm) : FullScreenDialog(vm) {
	Game &game = *_vm->_game;
	Scene &scene = game._scene;

	_tempLine = 0;
	_movedFlag = false;
	_redrawFlag = false;
	_selectedLine = -1;
	_dirFlag = false;
	_textLineCount = 0;
	_lineIndex = -1;
	_screenId = 920;

	chooseBackground();
	game.loadQuoteSet(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
		17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33,
		34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 0);
	game._kernelMode = KERNEL_ROOM_PRELOAD;
	_vm->_events->waitCursor();
	scene.clearVocab();
	scene._dynamicHotspots.clear();
	// Clear scene sprites and objects
	scene._spriteSlots.reset();
	_vm->_game->_screenObjects.clear();
	_vm->_dialogs->_defaultPosition = Common::Point(-1, -1);
	_menuSpritesIndex = 0;
}

void GameDialog::display() {
	Palette &palette = *_vm->_palette;
	palette.initPalette();
	palette.resetGamePalette(18, 10);

	FullScreenDialog::display();

	palette.setEntry(10, 0, 63, 0);
	palette.setEntry(11, 0, 45, 0);
	palette.setEntry(12, 63, 63, 0);
	palette.setEntry(13, 45, 45, 0);
	palette.setEntry(14, 63, 63, 63);
	palette.setEntry(15, 45, 45, 45);

	Scene &scene = _vm->_game->_scene;
	SpriteAsset *menuSprites = new SpriteAsset(_vm, "*MENU", 0);
	_menuSpritesIndex = scene._sprites.add(menuSprites);

	_lineIndex = -1;
	setClickableLines();

	_vm->_events->setCursor(CURSOR_ARROW);
}

GameDialog::~GameDialog() {
	_vm->_game->_scene._currentSceneId = RETURNING_FROM_DIALOG;
}

void GameDialog::clearLines() {
	Scene &scene = _vm->_game->_scene;
	_movedFlag = false;
	_lines.clear();
	scene._spriteSlots.fullRefresh(true);
}

void GameDialog::setClickableLines() {
	ScreenObjects &screenObjects = _vm->_game->_screenObjects;

	for (uint idx = 0; idx < _lines.size(); ++idx) {
		if (_lines[idx]._active) {
			const Common::Point &pt = _lines[idx]._pos;
			int strWidth = _lines[idx]._font->getWidth(_lines[idx]._msg);
			int maxHeight = _lines[idx]._font->getHeight();

			screenObjects.add(Common::Rect(pt.x, pt.y, pt.x + strWidth, pt.y + maxHeight - 1),
				SCREENMODE_VGA, CAT_COMMAND, idx);
		}
	}

	if (_vm->_dialogs->_pendingDialog == DIALOG_SAVE ||
			_vm->_dialogs->_pendingDialog == DIALOG_RESTORE) {
		screenObjects.add(Common::Rect(293, 26, 312, 75), SCREENMODE_VGA, CAT_INV_LIST, 50);
		screenObjects.add(Common::Rect(293, 78, 312, 127), SCREENMODE_VGA, CAT_INV_LIST, 51);
	}
}

void GameDialog::addQuote(int id1, int id2, DialogTextAlign align,
		const Common::Point &pt, Font *font) {
	Common::String msg = _vm->_game->getQuote(id1).c_str();	// c_str() because we need a copy

	if (id2 > 0)
		msg += _vm->_game->getQuote(id2);

	addLine(msg, align, pt, font);
}

void GameDialog::addLine(const Common::String &msg, DialogTextAlign align,
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
	case ALIGN_NONE:
		// No adjustment
		break;

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

			Common::String newMsg = msg.c_str();
			newMsg.deleteChar(ch - msgP);
			line->_msg = newMsg;
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

void GameDialog::initVars() {
	_tempLine = -1;
	_selectedLine = -1;
	_lineIndex = 0;
	_textLineCount = 0;
}

void GameDialog::chooseBackground() {
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

void GameDialog::setFrame(int frameNumber, int depth) {
	Scene &scene = _vm->_game->_scene;
	SpriteAsset *menuSprites = scene._sprites[_menuSpritesIndex];
	MSprite *frame = menuSprites->getFrame(frameNumber - 1);

	SpriteSlot &spriteSlot = scene._spriteSlots[scene._spriteSlots.add()];
	spriteSlot._flags = IMG_UPDATE;
	spriteSlot._seqIndex = 1;
	spriteSlot._spritesIndex = _menuSpritesIndex;
	spriteSlot._frameNumber = frameNumber;
	spriteSlot._position = frame->_offset;
	spriteSlot._depth = depth;
	spriteSlot._scale = 100;
}

void GameDialog::show() {
	display();

	Scene &scene = _vm->_game->_scene;

	while (_selectedLine == -1 && !_vm->shouldQuit()) {
		handleEvents();
		if (_redrawFlag) {
			if (!_tempLine)
				_tempLine = -1;

			refreshText();
			scene.drawElements(_vm->_game->_fx, _vm->_game->_fx);
			_redrawFlag = false;
		}

		_vm->_events->waitForNextFrame();
		_vm->_game->_fx = kTransitionNone;
	}
}

void GameDialog::handleEvents() {
	ScreenObjects &screenObjects = _vm->_game->_screenObjects;
	EventsManager &events = *_vm->_events;
	Nebular::DialogsNebular &dialogs = *(Nebular::DialogsNebular *)_vm->_dialogs;
	int tempLine = _tempLine;

	// Mark all the lines as initially unselected
	for (uint i = 0; i < _lines.size(); ++i)
		_lines[i]._state = DLGSTATE_UNSELECTED;

	// Process pending events
	events.pollEvents();

	if (events.isKeyPressed()) {
		switch (events.getKey().keycode) {
		case Common::KEYCODE_ESCAPE:
			_selectedLine = 0;
			break;
		default:
			break;
		}
	}

	// Scan for objects in the dialog
	Common::Point mousePos = events.currentPos() - Common::Point(0, DIALOG_TOP);
	int objIndex = screenObjects.scan(mousePos, SCREENMODE_VGA);

	if (_movedFlag) {
		int yp = mousePos.y;
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
	if (objIndex > 0 && (events._mouseStatus || events._mouseReleased)) {
		line = screenObjects[objIndex]._descId;
		if (dialogs._pendingDialog == DIALOG_SAVE || dialogs._pendingDialog == DIALOG_RESTORE) {
			if (line > 7 && line <= 14) {
				_lines[line]._state = DLGSTATE_UNSELECTED;
				line -= 7;
			}

			bool movedFlag = line > 0 && line < 8;
			if (events._mouseMoved)
				_movedFlag = movedFlag;
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
		if (!_movedFlag || line <= 18)
			_selectedLine = line;
		_redrawFlag = true;
	}

	_tempLine = line;
	if (tempLine != line || _selectedLine >= 0)
		_redrawFlag = true;
}

void GameDialog::refreshText() {
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

DifficultyDialog::DifficultyDialog(MADSEngine *vm) : GameDialog(vm) {
	setLines();
	_vm->_palette->resetGamePalette(18, 10);
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

void DifficultyDialog::display() {
	GameDialog::display();
	setFrame(8, 2);
}

void DifficultyDialog::show() {
	GameDialog::show();
	Nebular::GameNebular &game = *(Nebular::GameNebular *)_vm->_game;

	switch (_selectedLine) {
	case 1:
		game._difficulty = Nebular::DIFFICULTY_EASY;
		break;
	case 2:
		game._difficulty = Nebular::DIFFICULTY_MEDIUM;
		break;
	case 3:
		game._difficulty = Nebular::DIFFICULTY_HARD;
		break;
	default:
		_vm->quitGame();
	}
}

/*------------------------------------------------------------------------*/

GameMenuDialog::GameMenuDialog(MADSEngine *vm) : GameDialog(vm) {
	setLines();
}

void GameMenuDialog::setLines() {
	Font *font = _vm->_font->getFont(FONT_CONVERSATION);

	int yp = 64 - ((font->getHeight() + 1) * 4 + 6) / 2;

	addQuote(10, 0, ALIGN_CENTER, Common::Point(0, yp), font);
	yp += 6;

	for (int id = 11; id <= 15; ++id) {
		yp += font->getHeight();
		addQuote(id, 0, ALIGN_CENTER, Common::Point(0, yp));
	}
}

void GameMenuDialog::display() {
	GameDialog::display();
	setFrame(1, 2);
}

void GameMenuDialog::show() {
	GameDialog::show();

	switch (_selectedLine) {
	case 1:
		_vm->_dialogs->_pendingDialog = DIALOG_SAVE;
		_vm->_dialogs->showDialog();
		break;
	case 2:
		_vm->_dialogs->_pendingDialog = DIALOG_RESTORE;
		_vm->_dialogs->showDialog();
		break;
	case 3:
		_vm->_dialogs->_pendingDialog = DIALOG_OPTIONS;
		_vm->_dialogs->showDialog();
		break;
	case 5:
		_vm->quitGame();
		break;
	case 4:
	default:
		// Resume game
		break;
	}
}

/*------------------------------------------------------------------------*/

OptionsDialog::OptionsDialog(MADSEngine *vm) : GameDialog(vm) {
	setLines();
}

int OptionsDialog::getOptionQuote(int option) {
	Nebular::GameNebular &game = *(Nebular::GameNebular *)_vm->_game;

	switch (option) {
	case 17:	// Music
		return _vm->_musicFlag ? 24 : 25;	// 24: ON, 25: OFF
	case 18:	// Sound
		return _vm->_soundFlag ? 26 : 27;	// 26: ON, 27: OFF
	case 19:	// Interface
		return !_vm->_easyMouse ? 28 : 29;	// 28: Standard, 29: Easy
	case 20:	// Inventory
		return _vm->_invObjectsAnimated ? 30 : 31;	// 30: Spinning, 31: Still
	case 21:	// Text window
		return !_vm->_textWindowStill ? 32 : 33;	// 32: Animated, 33: Still
	case 22:	// Screen fade
		return 34 + _vm->_screenFade;	// 34: Smooth, 35: Medium, 36: Fast
	case 23:	// Storyline
		return (game._storyMode == STORYMODE_NAUGHTY) ? 37 : 38;	// 37: Naughty, 38: Nice
	default:
		error("getOptionQuote: Unknown option");
	}
}

void OptionsDialog::setLines() {
	Font *font = _vm->_font->getFont(FONT_CONVERSATION);

	int yp = 40 - ((font->getHeight() + 1) * 4 + 6) / 2;

	addQuote(16, 0, ALIGN_CENTER, Common::Point(0, yp), font);
	yp += 6;

	for (int id = 17; id <= 23; ++id) {
		yp += font->getHeight();
		addQuote(id, getOptionQuote(id), ALIGN_AT_CENTER, Common::Point(0, yp));
	}

	yp += 28;
	addQuote(1, 0, ALIGN_NONE, Common::Point(90, yp));
	addQuote(2, 0, ALIGN_NONE, Common::Point(190, yp));
}

void OptionsDialog::display() {
	GameDialog::display();
	setFrame(2, 2);
}

void OptionsDialog::show() {
	Nebular::GameNebular &game = *(Nebular::GameNebular *)_vm->_game;

	// Previous options, restored when cancel is selected
	bool prevMusicFlag = _vm->_musicFlag;
	bool prevEasyMouse = _vm->_easyMouse;
	bool prevInvObjectsAnimated = _vm->_invObjectsAnimated;
	bool prevTextWindowStill = _vm->_textWindowStill;
	ScreenFade prevScreenFade = _vm->_screenFade;
	StoryMode prevStoryMode = game._storyMode;

	do {
		_selectedLine = -1;
		GameDialog::show();

		switch (_selectedLine) {
		case 1:	// Music
			_vm->_musicFlag = _vm->_soundFlag = !_vm->_musicFlag;
			break;
		case 2:	// Sound
			_vm->_musicFlag = _vm->_soundFlag = !_vm->_musicFlag;
			break;
		case 3:	// Interface
			_vm->_easyMouse = !_vm->_easyMouse;
			break;
		case 4:	// Inventory
			_vm->_invObjectsAnimated = !_vm->_invObjectsAnimated;
			break;
		case 5:	// Text window
			_vm->_textWindowStill = !_vm->_textWindowStill;
			break;
		case 6:	// Screen fade
			if (_vm->_screenFade == SCREEN_FADE_FAST)
				_vm->_screenFade = SCREEN_FADE_MEDIUM;
			else if (_vm->_screenFade == SCREEN_FADE_MEDIUM)
				_vm->_screenFade = SCREEN_FADE_SMOOTH;
			else
				_vm->_screenFade = SCREEN_FADE_FAST;
			break;
		case 7:	// Storyline
			game._storyMode = (game._storyMode == STORYMODE_NAUGHTY) ? STORYMODE_NICE : STORYMODE_NAUGHTY;
			break;
		default:
			break;
		}

		// Reload menu
		_lineIndex = -1;
		clearLines();
		_vm->_game->_screenObjects.clear();
		_vm->_game->_scene._spriteSlots.reset();
		setLines();
	} while (!_vm->shouldQuit() && _selectedLine != 0 && _selectedLine <= 7);

	if (_selectedLine == 8) {
		// OK button, save settings
		_vm->saveOptions();
	} else if (_selectedLine == 9) {
		// Cancel button, revert all options from the saved ones
		_vm->_musicFlag = _vm->_soundFlag = prevMusicFlag;
		_vm->_easyMouse = prevEasyMouse;
		_vm->_invObjectsAnimated = prevInvObjectsAnimated;
		_vm->_textWindowStill = prevTextWindowStill;
		_vm->_screenFade = prevScreenFade;
		game._storyMode = prevStoryMode;
	}
}

} // End of namespace Nebular

} // End of namespace MADS
