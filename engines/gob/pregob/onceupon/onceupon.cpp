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

#include "gob/gob.h"
#include "gob/global.h"
#include "gob/util.h"
#include "gob/dataio.h"
#include "gob/surface.h"
#include "gob/draw.h"
#include "gob/video.h"
#include "gob/anifile.h"
#include "gob/aniobject.h"

#include "gob/sound/sound.h"

#include "gob/pregob/onceupon/onceupon.h"

static const  int kPaletteSize  = 16;
static const uint kPaletteCount = 20;

static const byte kCopyProtectionPalette[3 * kPaletteSize] = {
	0x00, 0x00, 0x00, 0x19, 0x00, 0x19, 0x00, 0x3F, 0x00, 0x00, 0x2A, 0x2A,
	0x2A, 0x00, 0x00, 0x2A, 0x00, 0x2A, 0x2A, 0x15, 0x00, 0x00, 0x19, 0x12,
	0x00, 0x00, 0x00, 0x15, 0x15, 0x3F, 0x15, 0x3F, 0x15, 0x00, 0x20, 0x3F,
	0x3F, 0x00, 0x00, 0x3F, 0x00, 0x20, 0x3F, 0x3F, 0x00, 0x3F, 0x3F, 0x3F
};

static const byte kGamePalettes[kPaletteCount][3 * kPaletteSize] = {
	{
		0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x18, 0x00, 0x00, 0x3C,
		0x1C, 0x28, 0x00, 0x10, 0x18, 0x00, 0x1C, 0x1C, 0x20, 0x14, 0x14, 0x14,
		0x14, 0x20, 0x04, 0x00, 0x00, 0x24, 0x3C, 0x3C, 0x3C, 0x00, 0x00, 0x00,
		0x3C, 0x2C, 0x00, 0x3C, 0x18, 0x00, 0x3C, 0x04, 0x00, 0x1C, 0x00, 0x00
	},
	{
		0x00, 0x00, 0x00, 0x00, 0x00, 0x24, 0x3C, 0x3C, 0x3C, 0x14, 0x20, 0x04,
		0x3C, 0x2C, 0x00, 0x02, 0x00, 0x18, 0x3C, 0x04, 0x00, 0x1C, 0x00, 0x00,
		0x14, 0x20, 0x04, 0x00, 0x00, 0x24, 0x3C, 0x3C, 0x3C, 0x00, 0x00, 0x00,
		0x3C, 0x2C, 0x00, 0x3C, 0x18, 0x00, 0x3C, 0x04, 0x00, 0x1C, 0x00, 0x00
	},
	{
		0x00, 0x00, 0x00, 0x38, 0x20, 0x3C, 0x2C, 0x10, 0x30, 0x20, 0x08, 0x28,
		0x14, 0x00, 0x1C, 0x20, 0x20, 0x38, 0x18, 0x18, 0x2C, 0x10, 0x10, 0x24,
		0x14, 0x20, 0x04, 0x00, 0x00, 0x24, 0x3C, 0x3C, 0x3C, 0x00, 0x00, 0x00,
		0x3C, 0x2C, 0x00, 0x3C, 0x18, 0x00, 0x3C, 0x04, 0x00, 0x1C, 0x00, 0x00
	},
	{
		0x00, 0x00, 0x00, 0x3C, 0x20, 0x20, 0x24, 0x14, 0x14, 0x1C, 0x10, 0x10,
		0x14, 0x0C, 0x0C, 0x1C, 0x1C, 0x1C, 0x18, 0x18, 0x18, 0x10, 0x10, 0x10,
		0x14, 0x20, 0x04, 0x00, 0x00, 0x24, 0x3C, 0x3C, 0x3C, 0x00, 0x00, 0x00,
		0x3C, 0x2C, 0x00, 0x3C, 0x18, 0x00, 0x3C, 0x04, 0x00, 0x1C, 0x00, 0x00
	},
	{
		0x00, 0x00, 0x00, 0x10, 0x28, 0x1C, 0x10, 0x1C, 0x10, 0x10, 0x14, 0x0C,
		0x1C, 0x1C, 0x3C, 0x24, 0x24, 0x3C, 0x18, 0x18, 0x24, 0x10, 0x10, 0x18,
		0x14, 0x20, 0x04, 0x00, 0x00, 0x24, 0x3C, 0x3C, 0x3C, 0x00, 0x00, 0x00,
		0x3C, 0x2C, 0x00, 0x3C, 0x18, 0x00, 0x3C, 0x04, 0x00, 0x1C, 0x00, 0x00
	},
	{
		0x00, 0x00, 0x00, 0x3F, 0x26, 0x3F, 0x36, 0x1C, 0x36, 0x2C, 0x12, 0x2A,
		0x27, 0x0C, 0x24, 0x22, 0x07, 0x1E, 0x1D, 0x03, 0x18, 0x16, 0x00, 0x10,
		0x14, 0x20, 0x04, 0x00, 0x00, 0x24, 0x3C, 0x3C, 0x3A, 0x00, 0x00, 0x00,
		0x3C, 0x2C, 0x00, 0x3C, 0x18, 0x00, 0x3C, 0x04, 0x00, 0x1C, 0x00, 0x00
	},
	{
		0x00, 0x00, 0x00, 0x3F, 0x39, 0x26, 0x38, 0x34, 0x1C, 0x30, 0x2F, 0x13,
		0x27, 0x29, 0x0C, 0x1D, 0x22, 0x07, 0x14, 0x1B, 0x03, 0x0C, 0x14, 0x00,
		0x14, 0x20, 0x04, 0x00, 0x00, 0x24, 0x3C, 0x3C, 0x3A, 0x00, 0x00, 0x00,
		0x3C, 0x2C, 0x00, 0x3C, 0x18, 0x00, 0x3C, 0x04, 0x00, 0x1C, 0x00, 0x00
	},
	{
		0x00, 0x00, 0x00, 0x24, 0x3C, 0x3C, 0x1C, 0x34, 0x38, 0x14, 0x2C, 0x30,
		0x0C, 0x20, 0x2C, 0x08, 0x18, 0x28, 0x04, 0x10, 0x20, 0x00, 0x08, 0x1C,
		0x14, 0x20, 0x04, 0x00, 0x00, 0x24, 0x3C, 0x3C, 0x38, 0x00, 0x00, 0x00,
		0x3C, 0x2C, 0x00, 0x3C, 0x18, 0x00, 0x3C, 0x04, 0x00, 0x1C, 0x00, 0x00
	},
	{
		0x00, 0x00, 0x00, 0x3C, 0x2C, 0x24, 0x38, 0x24, 0x1C, 0x30, 0x1C, 0x14,
		0x28, 0x18, 0x0C, 0x20, 0x10, 0x04, 0x1C, 0x0C, 0x00, 0x14, 0x08, 0x00,
		0x14, 0x20, 0x04, 0x00, 0x00, 0x24, 0x3C, 0x3C, 0x38, 0x00, 0x00, 0x00,
		0x3C, 0x2C, 0x00, 0x3C, 0x18, 0x00, 0x3C, 0x04, 0x00, 0x1C, 0x00, 0x00
	},
	{
		0x00, 0x00, 0x00, 0x3C, 0x34, 0x24, 0x38, 0x2C, 0x1C, 0x30, 0x24, 0x14,
		0x2C, 0x1C, 0x10, 0x30, 0x30, 0x3C, 0x1C, 0x1C, 0x38, 0x0C, 0x0C, 0x38,
		0x14, 0x20, 0x04, 0x00, 0x00, 0x24, 0x3C, 0x3C, 0x3C, 0x00, 0x00, 0x00,
		0x3C, 0x2C, 0x00, 0x3C, 0x18, 0x00, 0x3C, 0x04, 0x00, 0x1C, 0x00, 0x00
	},
	{
		0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x02, 0x03, 0x14, 0x07, 0x07, 0x1D,
		0x0E, 0x0E, 0x25, 0x17, 0x17, 0x2E, 0x21, 0x22, 0x36, 0x2F, 0x2F, 0x3F,
		0x3F, 0x3F, 0x3F, 0x3F, 0x3B, 0x0D, 0x3A, 0x31, 0x0A, 0x35, 0x28, 0x07,
		0x30, 0x21, 0x04, 0x2B, 0x19, 0x02, 0x26, 0x12, 0x01, 0x16, 0x0B, 0x00
	},
	{
		0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x21, 0x01, 0x00, 0x2A, 0x02, 0x00,
		0x33, 0x03, 0x00, 0x3D, 0x06, 0x00, 0x2A, 0x19, 0x05, 0x15, 0x14, 0x14,
		0x22, 0x1F, 0x1E, 0x2F, 0x2C, 0x28, 0x3F, 0x3C, 0x29, 0x3F, 0x38, 0x0B,
		0x3B, 0x30, 0x0A, 0x37, 0x29, 0x08, 0x33, 0x23, 0x07, 0x2F, 0x1D, 0x06
	},
	{
		0x00, 0x00, 0x00, 0x00, 0x1C, 0x38, 0x34, 0x30, 0x28, 0x2C, 0x24, 0x1C,
		0x24, 0x18, 0x10, 0x1C, 0x10, 0x08, 0x14, 0x04, 0x04, 0x10, 0x00, 0x00,
		0x14, 0x20, 0x04, 0x00, 0x00, 0x24, 0x3C, 0x3C, 0x38, 0x00, 0x00, 0x00,
		0x3C, 0x2C, 0x00, 0x3C, 0x18, 0x00, 0x3C, 0x04, 0x00, 0x1C, 0x00, 0x00
	},
	{
		0x00, 0x00, 0x00, 0x00, 0x1C, 0x38, 0x34, 0x30, 0x28, 0x2C, 0x24, 0x1C,
		0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,
		0x14, 0x20, 0x04, 0x00, 0x00, 0x24, 0x3C, 0x3C, 0x38, 0x00, 0x00, 0x00,
		0x3C, 0x2C, 0x00, 0x3C, 0x18, 0x00, 0x3C, 0x04, 0x00, 0x1C, 0x00, 0x00
	},
	{
		0x00, 0x00, 0x00, 0x1A, 0x30, 0x37, 0x14, 0x28, 0x31, 0x10, 0x20, 0x2C,
		0x0C, 0x19, 0x27, 0x08, 0x12, 0x21, 0x05, 0x0C, 0x1C, 0x03, 0x07, 0x16,
		0x01, 0x03, 0x11, 0x00, 0x00, 0x0C, 0x3C, 0x3C, 0x3C, 0x00, 0x00, 0x00,
		0x3C, 0x2C, 0x00, 0x3C, 0x18, 0x00, 0x3C, 0x04, 0x00, 0x1C, 0x00, 0x00
	},
	{
		0x00, 0x00, 0x00, 0x34, 0x30, 0x34, 0x30, 0x24, 0x30, 0x28, 0x1C, 0x28,
		0x24, 0x14, 0x24, 0x1C, 0x0C, 0x1C, 0x18, 0x08, 0x18, 0x14, 0x04, 0x14,
		0x0C, 0x04, 0x0C, 0x08, 0x00, 0x08, 0x3C, 0x3C, 0x3C, 0x00, 0x00, 0x00,
		0x3C, 0x2C, 0x00, 0x3C, 0x18, 0x00, 0x3C, 0x04, 0x00, 0x1C, 0x00, 0x00
	},
	{
		0x00, 0x00, 0x00, 0x2C, 0x24, 0x0C, 0x34, 0x34, 0x28, 0x2C, 0x2C, 0x1C,
		0x24, 0x24, 0x10, 0x1C, 0x18, 0x08, 0x14, 0x14, 0x08, 0x10, 0x10, 0x04,
		0x0C, 0x0C, 0x04, 0x00, 0x00, 0x24, 0x3C, 0x3C, 0x38, 0x00, 0x00, 0x00,
		0x3C, 0x2C, 0x00, 0x3C, 0x18, 0x00, 0x3C, 0x04, 0x00, 0x1C, 0x00, 0x00
	},
	{
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x14, 0x28, 0x31, 0x10, 0x20, 0x2C,
		0x0C, 0x19, 0x27, 0x08, 0x12, 0x21, 0x05, 0x0C, 0x1C, 0x03, 0x07, 0x16,
		0x01, 0x03, 0x11, 0x00, 0x3C, 0x00, 0x3C, 0x3C, 0x3C, 0x00, 0x00, 0x00,
		0x3C, 0x2C, 0x00, 0x3C, 0x18, 0x00, 0x3C, 0x04, 0x00, 0x1C, 0x00, 0x00
	},
	{
		0x00, 0x00, 0x00, 0x10, 0x28, 0x1C, 0x10, 0x1C, 0x10, 0x10, 0x14, 0x0C,
		0x1C, 0x1C, 0x3C, 0x24, 0x24, 0x3C, 0x18, 0x18, 0x24, 0x10, 0x10, 0x18,
		0x14, 0x20, 0x04, 0x00, 0x00, 0x24, 0x3C, 0x3C, 0x3C, 0x00, 0x00, 0x00,
		0x3C, 0x2C, 0x00, 0x3C, 0x18, 0x00, 0x3C, 0x04, 0x00, 0x1C, 0x00, 0x00
	},
	{
		0x00, 0x00, 0x00, 0x10, 0x28, 0x1C, 0x10, 0x1C, 0x10, 0x10, 0x14, 0x0C,
		0x1C, 0x1C, 0x3C, 0x24, 0x24, 0x3C, 0x18, 0x18, 0x24, 0x10, 0x10, 0x18,
		0x14, 0x20, 0x04, 0x00, 0x00, 0x24, 0x3C, 0x3C, 0x3C, 0x00, 0x00, 0x00,
		0x3C, 0x2C, 0x00, 0x3C, 0x18, 0x00, 0x3C, 0x04, 0x00, 0x1C, 0x00, 0x00
	}
};

static const uint kLanguageCount = 5;

static const uint kCopyProtectionHelpStringCount = 3;

static const char *kCopyProtectionHelpStrings[kLanguageCount][kCopyProtectionHelpStringCount] = {
	{ // French
		"Consulte le livret des animaux, rep\212re la",
		"page correspondant \205 la couleur de l\'\202cran",
		"et clique le symbole associ\202 \205 l\'animal affich\202.",
	},
	{ // German
		"Suche im Tieralbum die Seite, die der Farbe auf",
		"dem Bildschirm entspricht und klicke auf das",
		"Tiersymbol.",
	},
	{ // English
		"Consult the book of animals, find the page",
		"corresponding to the colour of screen and click",
		"the symbol associated with the animal displayed.",
	},
	{ // Spanish
		"Consulta el libro de los animales, localiza la ",
		"p\240gina que corresponde al color de la pantalla.",
		"Cliquea el s\241mbolo asociado al animal que aparece.",
	},
	{ // Italian
		"Guarda il libretto degli animali, trova la",
		"pagina che corrisponde al colore dello schermo,",
		"clicca il simbolo associato all\'animale presentato",
	}
};

static const char *kCopyProtectionWrongStrings[kLanguageCount] = {
	"Tu t\'es tromp\202, dommage...", // French
	"Schade, du hast dich geirrt."  , // German
	"You are wrong, what a pity!"   , // English
	"Te equivocas, l\240stima..."   , // Spanish
	"Sei Sbagliato, peccato..."       // Italian
};

static const uint kCopyProtectionShapeCount = 5;

static const int16 kCopyProtectionShapeCoords[kCopyProtectionShapeCount][6] = {
	{  0,  51,  26,  75,  60, 154},
	{ 28,  51,  58,  81,  96, 151},
	{ 60,  51,  94,  79, 136, 152},
	{ 96,  51, 136,  71, 180, 155},
	{140,  51, 170,  77, 228, 153}
};

enum ClownAnimation {
	kClownAnimationClownCheer = 0,
	kClownAnimationClownStand = 1,
	kClownAnimationClownCry   = 6
};

namespace Gob {

namespace OnceUpon {

const OnceUpon::MenuButton OnceUpon::kMainMenuDifficultyButton[3] = {
	{false,  29, 18,  77, 57, 0, 0, 0, 0, 0, 0, 0},
	{false, 133, 18, 181, 57, 0, 0, 0, 0, 0, 0, 1},
	{false, 241, 18, 289, 57, 0, 0, 0, 0, 0, 0, 2},
};

const OnceUpon::MenuButton OnceUpon::kSectionButtons[4] = {
	{false,  27, 121,  91, 179,   0, 0,   0,  0,   0,   0,  0},
	{ true,  95, 121, 159, 179,   4, 1,  56, 49, 100, 126,  2},
	{ true, 163, 121, 227, 179,  64, 1, 120, 49, 168, 126,  6},
	{ true, 231, 121, 295, 179, 128, 1, 184, 49, 236, 126, 10}
};

const OnceUpon::MenuButton OnceUpon::kIngameButtons[3] = {
	{true, 108, 83, 139, 116,   0,   0,  31,  34, 108,  83, 0},
	{true, 144, 83, 175, 116,  36,   0,  67,  34, 144,  83, 1},
	{true, 180, 83, 211, 116,  72,   0, 103,  34, 180,  83, 2}
};


OnceUpon::OnceUpon(GobEngine *vm) : PreGob(vm), _jeudak(0), _lettre(0), _plettre(0), _glettre(0),
	_openedArchives(false), _animalsButton(0) {

}

OnceUpon::~OnceUpon() {
	deinit();
}

void OnceUpon::init() {
	deinit();

	bool hasSTK1 = _vm->_dataIO->openArchive("stk1.stk", true);
	bool hasSTK2 = _vm->_dataIO->openArchive("stk2.stk", true);
	bool hasSTK3 = _vm->_dataIO->openArchive("stk3.stk", true);

	if (!hasSTK1 || !hasSTK2 || !hasSTK3)
		error("OnceUpon::OnceUpon(): Failed to open archives");

	_openedArchives = true;

	_jeudak  = _vm->_draw->loadFont("jeudak.let");
	_lettre  = _vm->_draw->loadFont("lettre.let");
	_plettre = _vm->_draw->loadFont("plettre.let");
	_glettre = _vm->_draw->loadFont("glettre.let");

	if (!_jeudak || !_lettre || !_plettre || !_glettre)
		error("OnceUpon::OnceUpon(): Failed to fonts (%d, %d, %d, %d)",
		      _jeudak != 0, _lettre != 0, _plettre != 0, _glettre != 0);

	if (_vm->_global->_language == kLanguageAmerican)
		_vm->_global->_language = kLanguageBritish;

	if ((_vm->_global->_language >= kLanguageCount))
		error("We do not support the language \"%s\".\n"
		      "If you are certain that your game copy includes this language,\n"
		      "please contact the ScummVM team with details about this version.\n"
		      "Thanks", _vm->getLangDesc(_vm->_global->_language));

	initScreen();

	_difficulty = kDifficultyMAX;
	_section    = 0;
}

void OnceUpon::deinit() {
	delete _jeudak;
	delete _lettre;
	delete _plettre;
	delete _glettre;

	_jeudak  = 0;
	_lettre  = 0;
	_plettre = 0;
	_glettre = 0;

	if (_openedArchives) {
		_vm->_dataIO->closeArchive(true);
		_vm->_dataIO->closeArchive(true);
		_vm->_dataIO->closeArchive(true);
	}

	_openedArchives = false;
}

void OnceUpon::setAnimalsButton(const MenuButton *animalsButton) {
	_animalsButton = animalsButton;
}

void OnceUpon::setCopyProtectionPalette() {
	setPalette(kCopyProtectionPalette, kPaletteSize);
}

void OnceUpon::setGamePalette(uint palette) {
	if (palette >= kPaletteCount)
		return;

	setPalette(kGamePalettes[palette], kPaletteSize);
}

void OnceUpon::setGameCursor() {
	Surface cursor(320, 16, 1);

	_vm->_video->drawPackedSprite("icon.cmp", cursor);

	setCursor(cursor, 105, 0, 120, 15, 0, 0);
}

enum CopyProtectionState {
	kCPStateSetup,     // Set up the screen
	kCPStateWaitUser,  // Waiting for the user to pick a shape
	kCPStateWaitClown, // Waiting for the clown animation to finish
	kCPStateFinish     // Finishing
};

bool OnceUpon::doCopyProtection(const uint8 colors[7], const uint8 shapes[7 * 20], const uint8 obfuscate[4]) {
	fadeOut();
	setCopyProtectionPalette();

	Surface sprites[2] = {Surface(320, 200, 1), Surface(320, 200, 1)};

	_vm->_video->drawPackedSprite("grille1.cmp", sprites[0]);
	_vm->_video->drawPackedSprite("grille2.cmp", sprites[1]);

	ANIFile   ani  (_vm, "grille.ani", 320);
	ANIObject clown(ani);

	setCursor(sprites[1], 5, 110, 20, 134, 3, 0);

	CopyProtectionState state = kCPStateSetup;

	uint8 triesLeft   =  2;
	int8  animalShape = -1;
	bool  hasCorrect  = false;

	while (!_vm->shouldQuit() && (state != kCPStateFinish)) {
		clearAnim(clown);

		// Set up the screen
		if (state == kCPStateSetup) {
			animalShape = cpSetup(colors, shapes, obfuscate, sprites);

			setAnimState(clown, kClownAnimationClownStand, false, false);
			state = kCPStateWaitUser;
		}

		drawAnim(clown);

		// If we're waiting for the clown and he finished, evaluate if we're finished
		if (!clown.isVisible() && (state == kCPStateWaitClown))
			state = (hasCorrect || (--triesLeft == 0)) ? kCPStateFinish : kCPStateSetup;

		showCursor();
		fadeIn();

		endFrame(true);

		int16 mouseX, mouseY;
		MouseButtons mouseButtons;

		checkInput(mouseX, mouseY, mouseButtons);

		if (state == kCPStateWaitUser) {
			// Look if we clicked a shaped and got it right

			int8 guessedShape = -1;
			if (mouseButtons == kMouseButtonsLeft)
				guessedShape = cpFindShape(mouseX, mouseY);

			if (guessedShape >= 0) {
				hasCorrect  = guessedShape == animalShape;
				animalShape = -1;

				setAnimState(clown, hasCorrect ? kClownAnimationClownCheer : kClownAnimationClownCry, true, false);
				state = kCPStateWaitClown;
			}
		}
	}

	fadeOut();
	hideCursor();
	clearScreen();

	// Display the "You are wrong" screen
	if (!hasCorrect)
		cpWrong();

	return hasCorrect;
}

int8 OnceUpon::cpSetup(const uint8 colors[7], const uint8 shapes[7 * 20], const uint8 obfuscate[4],
                       const Surface sprites[2]) {

	fadeOut();
	hideCursor();

	// Get a random animal and animal color
	int8 animalColor = _vm->_util->getRandom(7);
	while ((colors[animalColor] == 1) || (colors[animalColor] == 7) || (colors[animalColor] == 11))
		animalColor = _vm->_util->getRandom(7);

	int8 animal = _vm->_util->getRandom(20);

	int8 animalShape = shapes[animalColor * 20 + animal];
	if (animal < 4)
		animal = obfuscate[animal];

	// Get the position of the animal sprite
	int16 animalLeft = (animal % 4) * 80;
	int16 animalTop  = (animal / 4) * 50;

	uint8 sprite = 0;
	if (animalTop >= 200) {
		animalTop -= 200;
		sprite = 1;
	}

	int16 animalRight  = animalLeft + 80 - 1;
	int16 animalBottom = animalTop  + 50 - 1;

	// Fill with the animal color
	_vm->_draw->_backSurface->fill(colors[animalColor]);

	// Print the help line strings
	for (uint i = 0; i < kCopyProtectionHelpStringCount; i++) {
		const char * const helpString = kCopyProtectionHelpStrings[_vm->_global->_language][i];

		const int x = 160 - (strlen(helpString) * _plettre->getCharWidth()) / 2;
		const int y = i * 10 + 5;

		_plettre->drawString(helpString, x, y, 8, 0, true, *_vm->_draw->_backSurface);
	}

	// White rectangle with black border
	_vm->_draw->_backSurface->fillRect( 93, 43, 226, 134, 15);
	_vm->_draw->_backSurface->drawRect( 92, 42, 227, 135,  0);

	// Draw the animal in the animal color
	_vm->_draw->_backSurface->fillRect(120, 63, 199, 112, colors[animalColor]);
	_vm->_draw->_backSurface->blit(sprites[sprite], animalLeft, animalTop, animalRight, animalBottom, 120, 63, 0);

	// Draw the shapes
	for (uint i = 0; i < kCopyProtectionShapeCount; i++) {
		const int16 * const coords = kCopyProtectionShapeCoords[i];

		_vm->_draw->_backSurface->blit(sprites[1], coords[0], coords[1], coords[2], coords[3], coords[4], coords[5], 0);
	}

	_vm->_draw->forceBlit();

	return animalShape;
}

int8 OnceUpon::cpFindShape(int16 x, int16 y) const {
	// Look through all shapes and check if the coordinates are inside one of them
	for (uint i = 0; i < kCopyProtectionShapeCount; i++) {
		const int16 * const coords = kCopyProtectionShapeCoords[i];

		const int16 left   = coords[4];
		const int16 top    = coords[5];
		const int16 right  = coords[4] + (coords[2] - coords[0] + 1) - 1;
		const int16 bottom = coords[5] + (coords[3] - coords[1] + 1) - 1;

		if ((x >= left) && (x <= right) && (y >= top) && (y <= bottom))
			return i;
	}

	return -1;
}

void OnceUpon::cpWrong() {
	// Display the "You are wrong" string, centered

	const char * const wrongString = kCopyProtectionWrongStrings[_vm->_global->_language];
	const int          wrongX      = 160 - (strlen(wrongString) * _plettre->getCharWidth()) / 2;

	_vm->_draw->_backSurface->clear();
	_plettre->drawString(wrongString, wrongX, 100, 15, 0, true, *_vm->_draw->_backSurface);

	_vm->_draw->forceBlit();

	fadeIn();

	waitInput();

	fadeOut();
	clearScreen();
}

void OnceUpon::setAnimState(ANIObject &ani, uint16 state, bool once, bool pause) const {
	ani.setAnimation(state);
	ani.setMode(once ? ANIObject::kModeOnce : ANIObject::kModeContinuous);
	ani.setPause(pause);
	ani.setVisible(true);
	ani.setPosition();
}

void OnceUpon::showWait(uint palette) {
	// Show the loading floppy

	fadeOut();
	clearScreen();
	setGamePalette(palette);

	Surface wait(320, 43, 1);

	_vm->_video->drawPackedSprite("wait.cmp", wait);
	_vm->_draw->_backSurface->blit(wait, 0, 0, 72, 33, 122, 84);

	_vm->_draw->forceBlit();

	fadeIn();
}

void OnceUpon::showIntro() {
	// Show all intro parts

	// "Loading"
	showWait(10);
	if (_vm->shouldQuit())
		return;

	// Quote about fairy tales
	showQuote();
	if (_vm->shouldQuit())
		return;

	// Once Upon A Time title
	showTitle();
	if (_vm->shouldQuit())
		return;

	// Game title screen
	showChapter(0);
	if (_vm->shouldQuit())
		return;

	// "Loading"
	showWait(17);
}

void OnceUpon::showQuote() {
	// Show the quote about fairytales

	fadeOut();
	clearScreen();
	setGamePalette(11);

	static const Font *fonts[3] = { _plettre, _glettre, _plettre };

	TXTFile *quote = loadTXT(getLocFile("gene.tx"), TXTFile::kFormatStringPositionColorFont);
	quote->draw(*_vm->_draw->_backSurface, fonts, ARRAYSIZE(fonts));
	delete quote;

	_vm->_draw->forceBlit();

	fadeIn();

	waitInput();

	fadeOut();
}

void OnceUpon::showTitle() {
	// Show the Once Upon A Time title animation
	// NOTE: This is currently only a mock-up. The real animation is in "ville.seq".

	fadeOut();
	setGamePalette(10);

	warning("OnceUpon::showTitle(): Actually play the SEQ");

	clearScreen();

	_vm->_video->drawPackedSprite("ville.cmp", *_vm->_draw->_backSurface);
	_vm->_draw->forceBlit();

	ANIFile   ani  (_vm, "pres.ani", 320);
	ANIObject title(ani);

	setAnimState(title, 8, false, false);

	playTitleMusic();

	while (!_vm->shouldQuit()) {
		redrawAnim(title);

		fadeIn();

		endFrame(true);

		if (hasInput())
			break;
	}

	fadeOut();
	stopTitleMusic();
}

void OnceUpon::playTitleMusic() {
	if      (_vm->getPlatform() == Common::kPlatformPC)
		playTitleMusicDOS();
	else if (_vm->getPlatform() == Common::kPlatformAmiga)
		playTitleMusicAmiga();
	else if (_vm->getPlatform() == Common::kPlatformAtariST)
		playTitleMusicAtariST();
}

void OnceUpon::playTitleMusicDOS() {
	// Play an AdLib track

	_vm->_sound->adlibLoadTBR("babayaga.tbr");
	_vm->_sound->adlibLoadMDY("babayaga.mdy");
	_vm->_sound->adlibSetRepeating(-1);
	_vm->_sound->adlibPlay();
}

void OnceUpon::playTitleMusicAmiga() {
	// Play a Protracker track

	_vm->_sound->protrackerPlay("mod.babayaga");
}

void OnceUpon::playTitleMusicAtariST() {
	// Play a Soundblaster composition

	static const int16        titleMusic[21] = { 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 1, 0, 2, 0, 1, 0, 0, 0, 0, 0, -1};
	static const char * const titleFiles[ 3] = {"baba1.snd", "baba2.snd", "baba3.snd"};

	for (uint i = 0; i < ARRAYSIZE(titleFiles); i++)
		_vm->_sound->sampleLoad(_vm->_sound->sampleGetBySlot(i), SOUND_SND, titleFiles[i]);

	_vm->_sound->blasterPlayComposition(titleMusic, 0);
	_vm->_sound->blasterRepeatComposition(-1);
}

void OnceUpon::stopTitleMusic() {
	_vm->_sound->adlibSetRepeating(0);
	_vm->_sound->blasterRepeatComposition(0);

	_vm->_sound->adlibStop();
	_vm->_sound->blasterStopComposition();
	_vm->_sound->protrackerStop();

	for (int i = 0; i < Sound::kSoundsCount; i++)
		_vm->_sound->sampleFree(_vm->_sound->sampleGetBySlot(i));
}

void OnceUpon::showChapter(int chapter) {
	// Display the intro text to a chapter

	fadeOut();
	clearScreen();
	setGamePalette(11);

	// Parchment background
	_vm->_video->drawPackedSprite("parch.cmp", *_vm->_draw->_backSurface);

	static const Font *fonts[3] = { _plettre, _glettre, _plettre };

	const Common::String chapterFile = getLocFile(Common::String::format("gene%d.tx", chapter));

	TXTFile *gameTitle = loadTXT(chapterFile, TXTFile::kFormatStringPositionColorFont);
	gameTitle->draw(*_vm->_draw->_backSurface, fonts, ARRAYSIZE(fonts));
	delete gameTitle;

	_vm->_draw->forceBlit();

	fadeIn();

	waitInput();

	fadeOut();
}

OnceUpon::MenuAction OnceUpon::doMenu(MenuType type) {
	bool cursorVisible = isCursorVisible();

	// Set the cursor
	addCursor();
	setGameCursor();

	// Backup the screen
	Surface screenBackup(320, 200, 1);
	screenBackup.blit(*_vm->_draw->_backSurface);

	// Handle the specific menu
	MenuAction action;
	if      (type == kMenuTypeMainStart)
		action = doMenuMainStart();
	else if (type == kMenuTypeMainIngame)
		action = doMenuMainIngame();
	else if (type == kMenuTypeIngame)
		action = doMenuIngame();
	else
		error("OnceUpon::doMenu(): No such menu %d", type);

	if (_vm->shouldQuit())
		return action;

	// The ingame menu cleans itself up in a special way
	if (type == kMenuTypeIngame)
		clearMenuIngame(screenBackup);

	// The main menus fade out
	if ((type == kMenuTypeMainStart) || (type == kMenuTypeMainIngame))
		fadeOut();

	// Restore the screen
	_vm->_draw->_backSurface->blit(screenBackup);
	_vm->_draw->forceBlit();

	// Restore the cursor
	if (!cursorVisible)
		hideCursor();
	removeCursor();

	return action;
}

OnceUpon::MenuAction OnceUpon::doMenuMainStart() {
	fadeOut();
	drawMenuMainStart();
	showCursor();
	fadeIn();

	MenuAction action = kMenuActionNone;
	while (!_vm->shouldQuit() && (action == kMenuActionNone)) {
		endFrame(true);

		// Check user input

		int16 mouseX, mouseY;
		MouseButtons mouseButtons;

		int16 key = checkInput(mouseX, mouseY, mouseButtons);
		if (key == kKeyEscape)
			// ESC -> Quit
			return kMenuActionQuit;

		if (mouseButtons != kMouseButtonsLeft)
			continue;

		// If we clicked on a difficulty button, show the selected difficulty and start the game
		Difficulty difficulty = checkDifficultyButton(mouseX, mouseY);
		if (difficulty < kDifficultyMAX) {
			_difficulty = difficulty;
			action      = kMenuActionPlay;

			drawMenuMainStart();
			_vm->_util->longDelay(1000);
		}

		if (checkAnimalsButton(mouseX, mouseY))
			action = kMenuActionAnimals;

	}

	return action;
}

OnceUpon::MenuAction OnceUpon::doMenuMainIngame() {
	fadeOut();
	drawMenuMainIngame();
	showCursor();
	fadeIn();

	MenuAction action = kMenuActionNone;
	while (!_vm->shouldQuit() && (action == kMenuActionNone)) {
		endFrame(true);

		// Check user input

		int16 mouseX, mouseY;
		MouseButtons mouseButtons;

		int16 key = checkInput(mouseX, mouseY, mouseButtons);
		if (key == kKeyEscape)
			// ESC -> Quit
			return kMenuActionQuit;

		if (mouseButtons != kMouseButtonsLeft)
			continue;

		// If we clicked on a difficulty button, change the current difficulty level
		Difficulty difficulty = checkDifficultyButton(mouseX, mouseY);
		if ((difficulty < kDifficultyMAX) && (_difficulty != difficulty)) {
			_difficulty = difficulty;

			drawMenuMainIngame();
		}

		// If we clicked on a section button, restart the game from this section
		int8 section = checkSectionButton(mouseX, mouseY);
		if (section >= 0) {
			_section = section;
			action   = kMenuActionRestart;
		}

	}

	return action;
}

OnceUpon::MenuAction OnceUpon::doMenuIngame() {
	drawMenuIngame();
	showCursor();

	MenuAction action = kMenuActionNone;
	while (!_vm->shouldQuit() && (action == kMenuActionNone)) {
		endFrame(true);

		// Check user input

		int16 mouseX, mouseY;
		MouseButtons mouseButtons;

		int16 key = checkInput(mouseX, mouseY, mouseButtons);
		if ((key == kKeyEscape) || (mouseButtons == kMouseButtonsRight))
			// ESC or right mouse button -> Dismiss the menu
			action = kMenuActionPlay;

		if (mouseButtons != kMouseButtonsLeft)
			continue;

		// Check if we've pressed one of the buttons
		int8 button = checkIngameButton(mouseX, mouseY);
		if      (button == 0)
			action = kMenuActionQuit;
		else if (button == 1)
			action = kMenuActionMainMenu;
		else if (button == 2)
			action = kMenuActionPlay;

	}

	return action;
}

void OnceUpon::drawMenuMainStart() {
	// Draw the background
	_vm->_video->drawPackedSprite("menu2.cmp", *_vm->_draw->_backSurface);

	// Draw the "Listen to animal names" button
	if (_animalsButton) {
		Surface elements(320, 38, 1);
		_vm->_video->drawPackedSprite("elemenu.cmp", elements);
		_vm->_draw->_backSurface->fillRect(_animalsButton->left , _animalsButton->top,
		                                   _animalsButton->right, _animalsButton->bottom, 0);
		_vm->_draw->_backSurface->blit(elements,
		                               _animalsButton->srcLeft , _animalsButton->srcTop,
		                               _animalsButton->srcRight, _animalsButton->srcBottom,
		                               _animalsButton->dstX    , _animalsButton->dstY);
	}

	// Highlight the current difficulty
	drawMenuDifficulty();

	_vm->_draw->forceBlit();
}

void OnceUpon::drawMenuMainIngame() {
	// Draw the background
	_vm->_video->drawPackedSprite("menu.cmp", *_vm->_draw->_backSurface);

	// Highlight the current difficulty
	drawMenuDifficulty();

	// Draw the section buttons
	Surface elements(320, 200, 1);
	_vm->_video->drawPackedSprite("elemenu.cmp", elements);

	for (uint i = 0; i < ARRAYSIZE(kSectionButtons); i++) {
		const MenuButton &button = kSectionButtons[i];

		if (!button.needDraw)
			continue;

		if (_section >= (uint)button.id)
			_vm->_draw->_backSurface->blit(elements, button.srcLeft, button.srcTop, button.srcRight, button.srcBottom,
			                               button.dstX, button.dstY);
	}

	_vm->_draw->forceBlit();
}

void OnceUpon::drawMenuIngame() {
	Surface menu(320, 34, 1);
	_vm->_video->drawPackedSprite("icon.cmp", menu);

	// Draw the menu in a special way
	for (uint i = 0; i < ARRAYSIZE(kIngameButtons); i++) {
		const MenuButton &button = kIngameButtons[i];

		drawLineByLine(menu, button.srcLeft, button.srcTop, button.srcRight, button.srcBottom,
		               button.dstX, button.dstY);
	}

	_vm->_draw->forceBlit();
	_vm->_video->retrace();
}

void OnceUpon::drawMenuDifficulty() {
	if (_difficulty == kDifficultyMAX)
		return;

	TXTFile *difficulties = loadTXT(getLocFile("diffic.tx"), TXTFile::kFormatStringPositionColor);

	// Draw the difficulty name
	difficulties->draw((uint) _difficulty, *_vm->_draw->_backSurface, &_plettre, 1);

	// Draw a border around the current difficulty
	_vm->_draw->_backSurface->drawRect(kMainMenuDifficultyButton[_difficulty].left,
	                                   kMainMenuDifficultyButton[_difficulty].top,
	                                   kMainMenuDifficultyButton[_difficulty].right,
	                                   kMainMenuDifficultyButton[_difficulty].bottom,
	                                   difficulties->getLines()[_difficulty].color);

	delete difficulties;
}

void OnceUpon::clearMenuIngame(const Surface &background) {
	// Find the area encompassing the whole ingame menu

	int16 left   = 0x7FFF;
	int16 top    = 0x7FFF;
	int16 right  = 0x0000;
	int16 bottom = 0x0000;

	for (uint i = 0; i < ARRAYSIZE(kIngameButtons); i++) {
		const MenuButton &button = kIngameButtons[i];

		if (!button.needDraw)
			continue;

		left   = MIN<int16>(left  , button.dstX);
		top    = MIN<int16>(top   , button.dstY);
		right  = MAX<int16>(right , button.dstX + (button.srcRight  - button.srcLeft + 1) - 1);
		bottom = MAX<int16>(bottom, button.dstY + (button.srcBottom - button.srcTop  + 1) - 1);
	}

	if ((left > right) || (top > bottom))
		return;

	// Clear it line by line
	drawLineByLine(background, left, top, right, bottom, left, top);
}

OnceUpon::Difficulty OnceUpon::checkDifficultyButton(int16 x, int16 y) const {
	for (uint i = 0; i < ARRAYSIZE(kMainMenuDifficultyButton); i++) {
		const MenuButton &button = kMainMenuDifficultyButton[i];

		if ((x >= button.left) && (x <= button.right) && (y >= button.top) && (y <= button.bottom))
			return (Difficulty) button.id;
	}

	return kDifficultyMAX;
}

bool OnceUpon::checkAnimalsButton(int16 x, int16 y) const {
	if (!_animalsButton)
		return false;

	return (x >= _animalsButton->left) && (x <= _animalsButton->right) &&
	       (y >= _animalsButton->top)  && (y <= _animalsButton->bottom);
}

int8 OnceUpon::checkSectionButton(int16 x, int16 y) const {
	for (uint i = 0; i < ARRAYSIZE(kSectionButtons); i++) {
		const MenuButton &button = kSectionButtons[i];

		if ((uint)button.id > _section)
			continue;

		if ((x >= button.left) && (x <= button.right) && (y >= button.top) && (y <= button.bottom))
			return (int8)button.id;
	}

	return -1;
}

int8 OnceUpon::checkIngameButton(int16 x, int16 y) const {
	for (uint i = 0; i < ARRAYSIZE(kIngameButtons); i++) {
		const MenuButton &button = kIngameButtons[i];

		if ((x >= button.left) && (x <= button.right) && (y >= button.top) && (y <= button.bottom))
			return (int8)button.id;
	}

	return -1;
}

void OnceUpon::drawLineByLine(const Surface &src, int16 left, int16 top, int16 right, int16 bottom,
                              int16 x, int16 y) const {

	// A special way of drawing something:
	// Draw every other line "downwards", wait a bit after each line
	// Then, draw the remaining lines "upwards" and again wait a bit after each line.

	if (_vm->shouldQuit())
		return;

	const int16 width  = right  - left + 1;
	const int16 height = bottom - top  + 1;

	if ((width <= 0) || (height <= 0))
		return;

	for (int16 i = 0; i < height; i += 2) {
		if (_vm->shouldQuit())
			return;

		_vm->_draw->_backSurface->blit(src, left, top + i, right, top + i, x, y + i);

		_vm->_draw->dirtiedRect(_vm->_draw->_backSurface, x, y + i, x + width - 1, y + 1);
		_vm->_draw->blitInvalidated();

		_vm->_util->longDelay(1);
	}

	for (int16 i = (height & 1) ? height : (height - 1); i >= 0; i -= 2) {
		if (_vm->shouldQuit())
			return;

		_vm->_draw->_backSurface->blit(src, left, top + i, right, top + i, x, y + i);

		_vm->_draw->dirtiedRect(_vm->_draw->_backSurface, x, y + i, x + width - 1, y + 1);
		_vm->_draw->blitInvalidated();

		_vm->_util->longDelay(1);
	}
}

} // End of namespace OnceUpon

} // End of namespace Gob
