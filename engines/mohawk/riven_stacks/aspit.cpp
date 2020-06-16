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

#include "mohawk/riven_stacks/aspit.h"

#include "mohawk/riven.h"
#include "mohawk/riven_card.h"
#include "mohawk/riven_graphics.h"
#include "mohawk/riven_inventory.h"
#include "mohawk/riven_sound.h"
#include "mohawk/riven_video.h"

#include "common/translation.h"

#include "graphics/fonts/ttf.h"
#include "graphics/font.h"
#include "graphics/fontman.h"

#include "gui/message.h"

namespace Mohawk {
namespace RivenStacks {

ASpit::ASpit(MohawkEngine_Riven *vm) :
		RivenStack(vm, kStackAspit) {

	REGISTER_COMMAND(ASpit, xastartupbtnhide);
	REGISTER_COMMAND(ASpit, xasetupcomplete);
	REGISTER_COMMAND(ASpit, xaatrusopenbook);
	REGISTER_COMMAND(ASpit, xaatrusbookback);
	REGISTER_COMMAND(ASpit, xaatrusbookprevpage);
	REGISTER_COMMAND(ASpit, xaatrusbooknextpage);
	REGISTER_COMMAND(ASpit, xacathopenbook);
	REGISTER_COMMAND(ASpit, xacathbookback);
	REGISTER_COMMAND(ASpit, xacathbookprevpage);
	REGISTER_COMMAND(ASpit, xacathbooknextpage);
	REGISTER_COMMAND(ASpit, xtrapbookback);
	REGISTER_COMMAND(ASpit, xatrapbookclose);
	REGISTER_COMMAND(ASpit, xatrapbookopen);
	REGISTER_COMMAND(ASpit, xarestoregame);
	REGISTER_COMMAND(ASpit, xadisablemenureturn);
	REGISTER_COMMAND(ASpit, xaenablemenureturn);
	REGISTER_COMMAND(ASpit, xalaunchbrowser);
	REGISTER_COMMAND(ASpit, xadisablemenuintro);
	REGISTER_COMMAND(ASpit, xaenablemenuintro);
	REGISTER_COMMAND(ASpit, xademoquit);
	REGISTER_COMMAND(ASpit, xaexittomain);

	REGISTER_COMMAND(ASpit, xaSaveGame);
	REGISTER_COMMAND(ASpit, xaResumeGame);
	REGISTER_COMMAND(ASpit, xaOptions);
	REGISTER_COMMAND(ASpit, xaNewGame);
}

struct MenuItemText {
	int language;
	const char *items[7];
} static const menuItems[] = {
	{ Common::EN_ANY, { "SETUP",      "START NEW GAME",  "START SAVED GAME",     "SAVE GAME",       "RESUME",     "OPTIONS",  "QUIT" } },
	{ Common::DE_DEU, { "SETUP",      "SPIELEN",         "SPIELSTAND LADEN",     "SPIEL SPEICHERN", "FORTSETZEN", "OPTIONEN", "BEENDEN" } },
	{ Common::ES_ESP, { "IMAGEN",     "IR A RIVEN",      "CARGAR JUEGO",         "GUARDAR JUEGO",   "CONTINUAR",  "OPCIONES", "SALIR" } },
	{ Common::FR_FRA, { "CONFIG",     "NOUVELLE PARTIE", "CHARGER",              "ENREGISTRER",     "REPRENDRE",  "OPTIONS",  "QUITTER" } },
	{ Common::IT_ITA, { "CONF.",      "GIOCA",           "CARICA GIOCO",         "SALVA IL GIOCO",  "SEGUITARE",  "OPZIONI",  "ESCI" } },
	{ Common::RU_RUS, { "УСТАНОВКИ",  "СТАРТ",           "ПРОДОЛЖИТЬ ИГРУ",      "СОХРАНИТЬ ИГРУ",  "ПРОДОЛЖИТЬ", "ОПЦИИ",    "ВЫЙТИ" } },
	{ Common::JA_JPN, { "セットアップ", "RIVENを演奏する",   "保存したゲームを開始する", "ゲームを保存する",  "持続する",     "オプション","やめる" } },
	{ Common::PL_POL, { "USTAWIENIA", "GRAJ W RIVEN",    "ZAŁADUJ GRĘ",          "ZAPISZ GRĘ",      "POWRÓT",     "OPCJE",    "WYJŚCIE" } },
	{ -1, { 0 } }
};

void ASpit::xastartupbtnhide(const ArgumentArray &args) {
	// The original game hides the start/setup buttons depending on an ini entry.
	// It's safe to ignore this command.

	if (!_vm->isGameVariant(GF_25TH))
		return;

	int lang = -1;
	for (int i = 0; menuItems[i].language != -1; i++) {
		if (menuItems[i].language == _vm->getLanguage()) {
			lang = i;
			break;
		}
	}

	if (lang == -1) {
		warning("Unsupported menu language, falling back to English");
		lang = 0;
	}

	struct MenuItem {
		uint16 blstId;
		bool requiresStartedGame;
	};

	MenuItem items[] = {
		{ 22, false }, // Setup
		{ 16, false }, // New game
		{ 23, false }, // Load game
		{ 24, true  }, // Save game
		{ 25, true  }, // Resume
		{ 26, false }, // Options
		{ 27, false }  // Quit
	};

	for (uint i = 0; i < ARRAYSIZE(items); i++) {
		RivenHotspot *hotspot = _vm->getCard()->getHotspotByBlstId(items[i].blstId);

		if (!hotspot) {
			warning("Missing hotspot %d", items[i].blstId);
			continue;
		}

		bool enabled = !items[i].requiresStartedGame || _vm->isGameStarted();
		hotspot->enable(enabled);

		Common::U32String str = Common::convertUtf8ToUtf32(menuItems[lang].items[i]);
		Common::Rect hotspotRect = hotspot->getRect();
		uint8 greyLevel = enabled ? 164 : 96;

		_vm->_gfx->drawText(str, hotspotRect, greyLevel);
	}
}

void ASpit::xasetupcomplete(const ArgumentArray &args) {
	// The original game sets an ini entry to disable the setup button and use the
	// start button only. It's safe to ignore this part of the command.
	uint16 menuCardId = getCardStackId(0xE2E);
	RivenScriptPtr goToMenuScript = _vm->_scriptMan->createScriptFromData(1, kRivenCommandChangeCard, 1, menuCardId);
	_vm->_scriptMan->runScript(goToMenuScript, false);
}

void ASpit::xaatrusopenbook(const ArgumentArray &args) {
	// Get the variable
	uint32 &page = _vm->_vars["aatrusbook"];

	// Set hotspots depending on the page
	RivenHotspot *openBook = _vm->getCard()->getHotspotByName("openBook");
	RivenHotspot *nextPage = _vm->getCard()->getHotspotByName("nextpage");
	RivenHotspot *prevPage = _vm->getCard()->getHotspotByName("prevpage");
	if (page == 1) {
		prevPage->enable(false);
		nextPage->enable(false);
		openBook->enable(true);
	} else {
		prevPage->enable(true);
		nextPage->enable(true);
		openBook->enable(false);
	}

	// Draw the image of the page
	_vm->getCard()->drawPicture(page);
}

void ASpit::xaatrusbookback(const ArgumentArray &args) {
	_vm->_inventory->backFromItemScript();
}

void ASpit::xaatrusbookprevpage(const ArgumentArray &args) {
	// Get the page variable
	uint32 &page = _vm->_vars["aatrusbook"];

	// Keep turning pages while the mouse is pressed
	while (keepTurningPages()) {
		// Check for the first page
		if (page == 1)
			return;

		// Update the page number
		page--;

		pageTurn(kRivenTransitionWipeRight);
		_vm->getCard()->drawPicture(page);
		_vm->doFrame();

		waitForPageTurnSound();
	}
}

void ASpit::xaatrusbooknextpage(const ArgumentArray &args) {
	// Get the page variable
	uint32 &page = _vm->_vars["aatrusbook"];

	// Keep turning pages while the mouse is pressed
	while (keepTurningPages()) {
		// Check for the last page
		if ((_vm->isGameVariant(GF_DEMO) && page == 6) || page == 10)
			return;

		// Update the page number
		page++;

		pageTurn(kRivenTransitionWipeLeft);
		_vm->getCard()->drawPicture(page);
		_vm->doFrame();

		// Wait until the previous page turn sound completes
		waitForPageTurnSound();
	}
}

void ASpit::xacathopenbook(const ArgumentArray &args) {
	// Get the variable
	uint32 page = _vm->_vars["acathbook"];

	// Set hotspots depending on the page
	RivenHotspot *openBook = _vm->getCard()->getHotspotByName("openBook");
	RivenHotspot *nextPage = _vm->getCard()->getHotspotByName("nextpage");
	RivenHotspot *prevPage = _vm->getCard()->getHotspotByName("prevpage");
	if (page == 1) {
		prevPage->enable(false);
		nextPage->enable(false);
		openBook->enable(true);
	} else {
		prevPage->enable(true);
		nextPage->enable(true);
		openBook->enable(false);
	}

	cathBookDrawPage(page);
}

void ASpit::cathBookDrawPage(uint32 page) {// Draw the image of the page
	_vm->getCard()->drawPicture(page);

	// Draw the white page edges
	if (page > 1 && page < 5)
		_vm->getCard()->drawPicture(50);
	else if (page > 5)
		_vm->getCard()->drawPicture(51);

	if (page == 28) {
		cathBookDrawTelescopeCombination();
	}
}

void ASpit::cathBookDrawTelescopeCombination() {// Draw the telescope combination
	// The images for the numbers are tBMP's 13 through 17.
	// The start point is at (156, 247)
	uint32 teleCombo = _vm->_vars["tcorrectorder"];
	static const uint16 kNumberWidth = 32;
	static const uint16 kNumberHeight = 25;
	static const uint16 kDstX = 156;
	static const uint16 kDstY = 247;

	for (byte i = 0; i < 5; i++) {
			uint16 offset = (getComboDigit(teleCombo, i) - 1) * kNumberWidth;
			Common::Rect srcRect = Common::Rect(offset, 0, offset + kNumberWidth, kNumberHeight);
			Common::Rect dstRect = Common::Rect(i * kNumberWidth + kDstX, kDstY, (i + 1) * kNumberWidth + kDstX, kDstY + kNumberHeight);
			_vm->_gfx->drawImageRect(i + 13, srcRect, dstRect);
		}
}

void ASpit::xacathbookback(const ArgumentArray &args) {
	_vm->_inventory->backFromItemScript();
}

void ASpit::xacathbookprevpage(const ArgumentArray &args) {
	// Get the variable
	uint32 &page = _vm->_vars["acathbook"];

	// Keep turning pages while the mouse is pressed
	while (keepTurningPages()) {
		// Check for the first page
		if (page == 1)
			return;

		// Update the page number
		page--;

		pageTurn(kRivenTransitionWipeDown);
		cathBookDrawPage(page);
		_vm->doFrame();

		waitForPageTurnSound();
	}
}

void ASpit::xacathbooknextpage(const ArgumentArray &args) {
	// Get the variable
	uint32 &page = _vm->_vars["acathbook"];

	// Keep turning pages while the mouse is pressed
	while (keepTurningPages()) {
		// Check for the last page
		if (page == 49)
			return;

		// Update the page number
		page++;

		pageTurn(kRivenTransitionWipeUp);
		cathBookDrawPage(page);
		_vm->doFrame();

		waitForPageTurnSound();
	}
}

void ASpit::xtrapbookback(const ArgumentArray &args) {
	// Return to where we were before entering the book
	_vm->_vars["atrap"] = 0;
	_vm->_inventory->backFromItemScript();
}

void ASpit::xatrapbookclose(const ArgumentArray &args) {
	// Close the trap book
	_vm->_vars["atrap"] = 0;

	pageTurn(kRivenTransitionWipeRight);

	// Stop the flyby movie to prevent a glitch where the book does not actually
	// close because the movie continues to draw over the closed book picture.
	// This glitch also happened in the original engine with transitions disabled.
	RivenVideo *flyby = _vm->_video->getSlot(1);
	flyby->close();

	_vm->getCard()->enter(false);
}

void ASpit::xatrapbookopen(const ArgumentArray &args) {
	// Open the trap book
	_vm->_vars["atrap"] = 1;

	pageTurn(kRivenTransitionWipeLeft);

	_vm->getCard()->enter(false);
}

void ASpit::xarestoregame(const ArgumentArray &args) {
	if (!showConfirmationDialog(_("Are you sure you want to load a saved game? All unsaved progress will be lost."),
	                            _("Load game"), _("Cancel"))) {
		return;
	}

	// Launch the load game dialog
	_vm->loadGameDialog();
}

void ASpit::xaSaveGame(const ArgumentArray &args) {
	_vm->saveGameDialog();
}

void ASpit::xaResumeGame(const ArgumentArray &args) {
	_vm->resumeFromMainMenu();
}

void ASpit::xaOptions(const ArgumentArray &args) {
	_vm->runOptionsDialog();
}

void ASpit::xaNewGame(const ArgumentArray &args) {
	if (!showConfirmationDialog(_("Are you sure you want to start a new game? All unsaved progress will be lost."),
	                            _("New game"), _("Cancel"))) {
		return;
	}

	_vm->startNewGame();

	RivenScriptPtr script = _vm->_scriptMan->createScriptFromData(2,
	                  kRivenCommandTransition,  1, kRivenTransitionBlend,
	                  kRivenCommandChangeCard,  1, 2);

	script->addCommand(RivenCommandPtr(new RivenStackChangeCommand(_vm, 0, 0x6E9A, false, false)));

	script += _vm->_scriptMan->createScriptFromData(1,
	                  kRivenCommandStopSound,   1, 2);

	_vm->_scriptMan->runScript(script, false);
}

bool ASpit::showConfirmationDialog(const Common::U32String &message, const Common::U32String &confirmButton, const Common::U32String &cancelButton) {
	if (!_vm->isGameStarted()) {
		return true;
	}

	GUI::MessageDialog dialog(message, confirmButton, cancelButton);

	return dialog.runModal() != 0;
}

void ASpit::xadisablemenureturn(const ArgumentArray &args) {
	// This function would normally enable the Windows menu item for
	// returning to the main menu. Ctrl+r will do this instead.
	// The original also had this shortcut.
}

void ASpit::xaenablemenureturn(const ArgumentArray &args) {
	// This function would normally enable the Windows menu item for
	// returning to the main menu. Ctrl+r will do this instead.
	// The original also had this shortcut.
}

void ASpit::xalaunchbrowser(const ArgumentArray &args) {
	// Well, we can't launch a browser for obvious reasons ;)
	// The original text is as follows (for reference):

	// If you have an auto-dial configured connection to the Internet,
	// please select YES below.
	//
	// America Online and CompuServe users may experience difficulty. If
	// you find that you are unable to connect, please quit the Riven
	// Demo, launch your browser and type in the following URL:
	//
	//     www.redorb.com/buyriven
	//
	// Would you like to attempt to make the connection?
	//
	// [YES] [NO]

	GUI::MessageDialog dialog(_("At this point, the Riven Demo would\n"
			                          "ask if you would like to open a web browser\n"
			                          "to bring you to the Red Orb store to buy\n"
			                          "the game. ScummVM cannot do that and\n"
			                          "the site no longer exists."));
	dialog.runModal();
}

void ASpit::xadisablemenuintro(const ArgumentArray &args) {
	// This function would normally enable the Windows menu item for
	// playing the intro. Ctrl+p will play the intro movies instead.
	// The original also had this shortcut.

	_vm->_inventory->forceHidden(true);
}

void ASpit::xaenablemenuintro(const ArgumentArray &args) {
	// This function would normally enable the Windows menu item for
	// playing the intro. Ctrl+p will play the intro movies instead.
	// The original also had this shortcut.

	// Show the "exit" button here
	_vm->_inventory->forceHidden(false);
}

void ASpit::xademoquit(const ArgumentArray &args) {
	if (!showConfirmationDialog(_("Are you sure you want to quit? All unsaved progress will be lost."), _("Quit"),
	                            _("Cancel"))) {
		return;
	}

	// Exactly as it says on the tin. In the demo, this function quits.
	_vm->setGameEnded();
}

void ASpit::xaexittomain(const ArgumentArray &args) {
	// One could potentially implement this function, but there would be no
	// point. This function is only used in the demo's aspit card 9 update
	// screen script. However, card 9 is not accessible from the game without
	// jumping to the card and there's nothing going on in the card so it
	// never gets called. There's also no card 9 in the full game, so the
	// functionality of this card was likely removed before release. The
	// demo executable references some other external commands relating to
	// setting and getting the volume, as well as drawing the volume. I'd
	// venture to guess that this would have been some sort of options card
	// replaced with the Windows/Mac API in the final product.
	//
	// Yeah, this function is just dummied and holds a big comment ;)
}

} // End of namespace RivenStacks
} // End of namespace Mohawk
