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

#include "mohawk/myst.h"
#include "mohawk/myst_areas.h"
#include "mohawk/myst_card.h"
#include "mohawk/myst_graphics.h"
#include "mohawk/myst_state.h"
#include "mohawk/cursors.h"
#include "mohawk/sound.h"
#include "mohawk/video.h"
#include "mohawk/myst_stacks/menu.h"

#include "common/translation.h"
#include "graphics/cursorman.h"
#include "gui/message.h"

namespace Mohawk {
namespace MystStacks {

Menu::Menu(MohawkEngine_Myst *vm) :
		MystScriptParser(vm, kMenuStack),
		_inGame(false),
		_canSave(false),
		_wasCursorVisible(true),
		_introMoviesRunning(false) {

	for (uint i = 0; i < ARRAYSIZE(_menuItemHovered); i++) {
		_menuItemHovered[i] = false;
	}

	setupOpcodes();
}

Menu::~Menu() {
}

void Menu::setupOpcodes() {
	// "Stack-Specific" Opcodes
	REGISTER_OPCODE(150, Menu, o_menuItemEnter);
	REGISTER_OPCODE(151, Menu, o_menuItemLeave);
	REGISTER_OPCODE(152, Menu, o_menuResume);
	REGISTER_OPCODE(153, Menu, o_menuLoad);
	REGISTER_OPCODE(154, Menu, o_menuSave);
	REGISTER_OPCODE(155, Menu, o_menuNew);
	REGISTER_OPCODE(156, Menu, o_menuOptions);
	REGISTER_OPCODE(157, Menu, o_menuQuit);

	// "Init" Opcodes
	REGISTER_OPCODE(200, Menu, o_playIntroMovies);
	REGISTER_OPCODE(201, Menu, o_menuInit);

	// "Exit" Opcodes
	REGISTER_OPCODE(300, Menu, NOP);
	REGISTER_OPCODE(301, Menu, o_menuExit);
}

void Menu::disablePersistentScripts() {
	_introMoviesRunning = false;
}

void Menu::runPersistentScripts() {
	if (_introMoviesRunning)
		introMovies_run();
}

uint16 Menu::getVar(uint16 var) {
	switch (var) {
	case 1000: // New game
	case 1001: // Load
	case 1004: // Quit
	case 1005: // Options
		return _menuItemHovered[var - 1000] ? 1 : 0;
	case 1002: // Save
		if (_canSave) {
			return _menuItemHovered[var - 1000] ? 1 : 0;
		} else {
			return 2;
		}
	case 1003: // Resume
		if (_inGame) {
			return _menuItemHovered[var - 1000] ? 1 : 0;
		} else {
			return 2;
		}
	default:
		return MystScriptParser::getVar(var);
	}
}

void Menu::o_menuInit(uint16 var, const ArgumentsArray &args) {
	_pauseToken = _vm->pauseEngine();

	if (_inGame) {
		_wasCursorVisible = CursorMan.isVisible();
	}

	if (!_wasCursorVisible) {
		CursorMan.showMouse(true);
	}

	struct MenuButton {
		uint16 highlightedIndex;
		uint16 disabledIndex;
		Graphics::TextAlign align;
	};

	static const MenuButton buttons[] = {
		{ 1, 0, Graphics::kTextAlignRight },
		{ 1, 0, Graphics::kTextAlignRight },
		{ 1, 2, Graphics::kTextAlignRight },
		{ 1, 2, Graphics::kTextAlignRight },
		{ 1, 0, Graphics::kTextAlignRight },
		{ 1, 0, Graphics::kTextAlignLeft  }
	};

	const char **buttonCaptions = getButtonCaptions();

	for (uint i = 0; i < ARRAYSIZE(buttons); i++) {
		MystAreaImageSwitch *image  = _vm->getCard()->getResource<MystAreaImageSwitch>(2 * i + 0);
		MystAreaHover       *hover  = _vm->getCard()->getResource<MystAreaHover>      (2 * i + 1);

		Common::U32String str = Common::convertUtf8ToUtf32(buttonCaptions[i]);
		drawButtonImages(str, image, buttons[i].align, buttons[i].highlightedIndex, buttons[i].disabledIndex);
		hover->setRect(image->getRect());
	}
}

const char **Menu::getButtonCaptions() const {
	static const char *buttonCaptionsEnglish[] = {
		"NEW GAME",
		"LOAD GAME",
		"SAVE GAME",
		"RESUME",
		"QUIT",
		"OPTIONS"
	};

	static const char *buttonCaptionsFrench[] = {
		"NOUVEAU",
		"CHARGER",
		"SAUVER",
		"REPRENDRE",
		"QUITTER",
		"OPTIONS"
	};

	static const char *buttonCaptionsGerman[] = {
		"NEUES SPIEL",
		"SPIEL LADEN",
		"SPIEL SPEICHERN",
		"FORTSETZEN",
		"BEENDEN",
		"OPTIONEN"
	};

	static const char *buttonCaptionsSpanish[] = {
		"JUEGO NUEVO",
		"CARGAR JUEGO",
		"GUARDAR JUEGO",
		"CONTINUAR",
		"SALIR",
		"OPCIONES"
	};

	static const char *buttonCaptionsPolish[] = {
		"NOWA GRA",
		"ZAŁADUJ GRĘ",
		"ZAPISZ GRĘ",
		"POWRÓT",
		"WYJŚCIE",
		"OPCJE"
	};

	switch (_vm->getLanguage()) {
		case Common::FR_FRA:
			return buttonCaptionsFrench;
		case Common::DE_DEU:
			return buttonCaptionsGerman;
		case Common::ES_ESP:
			return buttonCaptionsSpanish;
		case Common::PL_POL:
			return buttonCaptionsPolish;
		case Common::EN_ANY:
		default:
			return buttonCaptionsEnglish;
	}
}

void Menu::drawButtonImages(const Common::U32String &text, MystAreaImageSwitch *area, Graphics::TextAlign align, uint16 highlightedIndex, uint16 disabledIndex) const {
	Common::Rect backgroundRect = area->getRect();
	Common::Rect textBoundingBox = _vm->_gfx->getTextBoundingBox(text, backgroundRect, align);

	// Restrict the rectangle to the portion were the text will be drawn
	if (align == Graphics::kTextAlignLeft) {
		backgroundRect.right = textBoundingBox.right;
	} else if (align == Graphics::kTextAlignRight) {
		backgroundRect.left = textBoundingBox.left;
	} else {
		error("Unexpected align: %d", align);
	}

	// Update the area with the new background rect
	area->setRect(backgroundRect);

	MystAreaImageSwitch::SubImage idle = area->getSubImage(0);
	area->setSubImageRect(0, Common::Rect(backgroundRect.left, idle.rect.top, backgroundRect.right, idle.rect.bottom));

	// Align the text to the top of the destination rectangles
	int16 deltaY;
	if (_vm->getLanguage() == Common::PL_POL) {
		deltaY = -2;
	} else {
		deltaY = backgroundRect.top - textBoundingBox.top;
	}

	if (highlightedIndex) {
		replaceButtonSubImageWithText(text, align, area, highlightedIndex, backgroundRect, deltaY, 215, 216, 219);
	}

	if (disabledIndex) {
		replaceButtonSubImageWithText(text, align, area, disabledIndex, backgroundRect, deltaY, 136, 140, 145);
	}

	uint16 cardBackground = _vm->getCard()->getBackgroundImageId();
	_vm->_gfx->drawText(cardBackground, text, backgroundRect, 181, 184, 189, align, deltaY);
}

void Menu::replaceButtonSubImageWithText(const Common::U32String &text, const Graphics::TextAlign &align, MystAreaImageSwitch *area,
                                         uint16 subimageIndex, const Common::Rect &backgroundRect, int16 deltaY,
                                         uint8 r, uint8 g, uint8 b) const {
	uint16 cardBackground = _vm->getCard()->getBackgroundImageId();

	MystAreaImageSwitch::SubImage highlighted = area->getSubImage(subimageIndex);
	Common::Rect subImageRect(0, 0, backgroundRect.width(), backgroundRect.height());

	// Create an image exactly the size of the rendered text with the backdrop as a background
	_vm->_gfx->replaceImageWithRect(highlighted.wdib, cardBackground, backgroundRect);
	area->setSubImageRect(subimageIndex, subImageRect);

	// Draw the text in the subimage
	_vm->_gfx->drawText(highlighted.wdib, text, subImageRect, r, g, b, align, deltaY);
}

void Menu::o_menuItemEnter(uint16 var, const ArgumentsArray &args) {
	_menuItemHovered[var - 1000] = true;
	_vm->getCard()->redrawArea(var);
}

void Menu::o_menuItemLeave(uint16 var, const ArgumentsArray &args) {
	_menuItemHovered[var - 1000] = false;
	_vm->getCard()->redrawArea(var);
}

void Menu::o_menuResume(uint16 var, const ArgumentsArray &args) {
	if (!_inGame) {
		return;
	}

	_vm->resumeFromMainMenu();
}

void Menu::o_menuLoad(uint16 var, const ArgumentsArray &args) {
	if (!showConfirmationDialog(_("Are you sure you want to load a saved game? All unsaved progress will be lost."),
	                            _("Load game"), _("Cancel"))) {
		return;
	}

	_vm->loadGameDialog();
}

void Menu::o_menuSave(uint16 var, const ArgumentsArray &args) {
	if (!_canSave) {
		return;
	}

	_vm->saveGameDialog();
}

void Menu::o_menuNew(uint16 var, const ArgumentsArray &args) {
	if (!showConfirmationDialog(_("Are you sure you want to start a new game? All unsaved progress will be lost."),
	                            _("New game"), _("Cancel"))) {
		return;
	}

	_vm->_gameState->reset();
	_vm->setTotalPlayTime(0);
	_vm->setMainCursor(kDefaultMystCursor);
	_vm->changeToStack(kIntroStack, 1, 0, 0);
}

void Menu::o_menuOptions(uint16 var, const ArgumentsArray &args) {
	resetButtons();

	_vm->runOptionsDialog();
}

void Menu::o_menuQuit(uint16 var, const ArgumentsArray &args) {
	if (!showConfirmationDialog(_("Are you sure you want to quit? All unsaved progress will be lost."), _("Quit"),
	                            _("Cancel"))) {
		return;
	}

	_vm->changeToStack(kCreditsStack, 10000, 0, 0);
}

void Menu::o_menuExit(uint16 var, const ArgumentsArray &args) {
	if (_inGame) {
		_vm->_gfx->restoreStateForMainMenu();
	}

	CursorMan.showMouse(_wasCursorVisible);

	_pauseToken.clear();
}

void Menu::o_playIntroMovies(uint16 var, const ArgumentsArray &args) {
	_introMoviesRunning = true;
	_introStep = 0;
}

void Menu::introMovies_run() {
	// Play Intro Movies
	// This is all quite messy...

	VideoEntryPtr video;

	switch (_introStep) {
		case 0:
			_introStep = 1;
			video = _vm->playMovieFullscreen("broder", kIntroStack);
			break;
		case 1:
			if (!_vm->_video->isVideoPlaying())
				_introStep = 2;
			break;
		case 2:
			_introStep = 3;
			video = _vm->playMovieFullscreen("cyanlogo", kIntroStack);
			break;
		case 3:
			if (!_vm->_video->isVideoPlaying())
				_introStep = 4;
			break;
		default:
			_vm->changeToCard(1000, kTransitionCopy);
	}
}

bool Menu::showConfirmationDialog(const Common::U32String &message, const Common::U32String &confirmButton, const Common::U32String &cancelButton) {
	if (!_inGame) {
		return true;
	}

	resetButtons();

	GUI::MessageDialog dialog(message, confirmButton, cancelButton);

	return dialog.runModal() !=0;
}

void Menu::resetButtons() {
	for (uint i = 0; i < ARRAYSIZE(_menuItemHovered); i++) {
		_menuItemHovered[i] = false;
		_vm->getCard()->redrawArea(1000 + i);
	}

	_vm->doFrame();
}


} // End of namespace MystStacks
} // End of namespace Mohawk
