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
#include "igor/igor.h"

namespace Igor {

// Texts of the end credits: every page is a picture with the texts of the first or of the second pass drawn over
// it. Positions are the centre of every line.

struct CreditsLine {
	int16 x;
	int16 y;
	uint8 color;
	bool underline;
	uint16 strId;
};

struct CreditsPage {
	uint16 imgId;
	uint16 palId;
	const CreditsLine *lines[2];
	int count[2];
};

static const CreditsLine kCredits1Pass0[] = {
	{ 232, 90, 0xC2, true, STR_CreditsHistoriaYDiseno },
	{ 232, 118, 0xC3, false, STR_CreditsRamonHernaez },
	{ 232, 133, 0xC3, false, STR_CreditsFelipeGomez },
	{ 232, 148, 0xC3, false, STR_CreditsRafaelLatiegui },
	{ 232, 163, 0xC3, false, STR_CreditsMiguelAngelRamos },
};

static const CreditsLine kCredits1Pass1[] = {
	{ 210, 5, 0xC2, true, STR_CreditsProduccionYDireccionDeVoces },
	{ 210, 33, 0xC3, false, STR_CreditsLunaFaraco },
};

static const CreditsLine kCredits2Pass0[] = {
	{ 74, 105, 0xC2, true, STR_CreditsProgramacion },
	{ 74, 133, 0xC3, false, STR_CreditsRamonHernaez },
	{ 74, 148, 0xC3, false, STR_CreditsFelipeGomez },
	{ 74, 163, 0xC3, false, STR_CreditsMiguelAngelRamos },
};

static const CreditsLine kCredits2Pass1[] = {
	{ 74, 20, 0xC2, true, STR_CreditsVoces },
	{ 74, 48, 0xC3, false, STR_CreditsAngieDeBirch },
	{ 74, 63, 0xC3, false, STR_CreditsAntonioFdezMunoz },
	{ 74, 78, 0xC3, false, STR_CreditsArantxaFranco },
	{ 74, 93, 0xC3, false, STR_CreditsAlfredoGabrieli },
	{ 74, 108, 0xC3, false, STR_CreditsFernandoLuna },
	{ 74, 123, 0xC3, false, STR_CreditsAnaOlivares },
	{ 74, 138, 0xC3, false, STR_CreditsAdolfoPastor },
};

static const CreditsLine kCredits3Pass0[] = {
	{ 244, 30, 0xC2, true, STR_CreditsFondos },
	{ 244, 58, 0xC3, false, STR_CreditsCarlosVeredas },
};

static const CreditsLine kCredits3Pass1[] = {
	{ 244, 30, 0xC2, true, STR_CreditsVoces },
	{ 244, 58, 0xC3, false, STR_CreditsMiguelAngelPerez },
	{ 244, 73, 0xC3, false, STR_CreditsAntonioRamos },
	{ 244, 88, 0xC3, false, STR_CreditsPatriciaReija },
	{ 244, 103, 0xC3, false, STR_CreditsAparicioRivero },
	{ 244, 118, 0xC3, false, STR_CreditsDanielSanchez },
	{ 244, 133, 0xC3, false, STR_CreditsEnriqueSantaren },
	{ 244, 148, 0xC3, false, STR_CreditsCarlosViaga },
};

static const CreditsLine kCredits4Pass0[] = {
	{ 74, 23, 0xC2, true, STR_CreditsAnimaciones },
	{ 74, 51, 0xC3, false, STR_CreditsRafaelLatiegui },
};

static const CreditsLine kCredits4Pass1[] = {
	{ 74, 23, 0xC2, true, STR_CreditsProductorAsociado },
	{ 74, 51, 0xC3, false, STR_CreditsPabloDeLaNuez },
};

static const CreditsLine kCredits5Pass0[] = {
	{ 210, 20, 0xC2, true, STR_CreditsMusicaOriginalYArreglos },
	{ 210, 48, 0xC3, false, STR_CreditsEstebanMoreno },
};

static const CreditsLine kCredits5Pass1[] = {
	{ 240, 20, 0xC2, true, STR_CreditsDistribuidoPor },
	{ 240, 48, 0xC3, false, STR_CreditsDinamicMultimedia },
};

static const CreditsLine kCredits6Pass0[] = {
	{ 90, 5, 0xC2, true, STR_CreditsProduccionMusicaDigital },
	{ 90, 33, 0xC3, false, STR_CreditsAgpDigital },
};

static const CreditsLine kCredits6Pass1[] = {
	{ 74, 72, 0xC2, true, STR_CreditsAgradecimientos },
	{ 74, 100, 0xC3, false, STR_CreditsThePromisedLand },
	{ 74, 115, 0xC3, false, STR_CreditsRaquelAlvarez },
	{ 74, 130, 0xC3, false, STR_CreditsPilarRomero },
	{ 74, 145, 0xC3, false, STR_CreditsDouglasPrats },
	{ 74, 160, 0xC3, false, STR_CreditsEnriqueRAtienzar },
};

static const CreditsLine kCredits7Pass0[] = {
	{ 240, 142, 0xC2, true, STR_CreditsEfectosDeSonido },
	{ 240, 170, 0xC3, false, STR_CreditsLunaFaraco },
};

static const CreditsLine kCredits7Pass1[] = {
	{ 238, 140, 0xC2, true, STR_CreditsCreadoYProducidoPor },
	{ 238, 168, 0xC3, false, STR_CreditsPenduloStudios },
};

static const CreditsPage kCreditsPages[7] = {
	{ IMG_Credits1, PAL_Credits1, { kCredits1Pass0, kCredits1Pass1 }, { ARRAYSIZE(kCredits1Pass0), ARRAYSIZE(kCredits1Pass1) } },
	{ IMG_Credits2, PAL_Credits2, { kCredits2Pass0, kCredits2Pass1 }, { ARRAYSIZE(kCredits2Pass0), ARRAYSIZE(kCredits2Pass1) } },
	{ IMG_Credits3, PAL_Credits3, { kCredits3Pass0, kCredits3Pass1 }, { ARRAYSIZE(kCredits3Pass0), ARRAYSIZE(kCredits3Pass1) } },
	{ IMG_Credits4, PAL_Credits4, { kCredits4Pass0, kCredits4Pass1 }, { ARRAYSIZE(kCredits4Pass0), ARRAYSIZE(kCredits4Pass1) } },
	{ IMG_Credits5, PAL_Credits5, { kCredits5Pass0, kCredits5Pass1 }, { ARRAYSIZE(kCredits5Pass0), ARRAYSIZE(kCredits5Pass1) } },
	{ IMG_Credits6, PAL_Credits6, { kCredits6Pass0, kCredits6Pass1 }, { ARRAYSIZE(kCredits6Pass0), ARRAYSIZE(kCredits6Pass1) } },
	{ IMG_Credits7, PAL_Credits7, { kCredits7Pass0, kCredits7Pass1 }, { ARRAYSIZE(kCredits7Pass0), ARRAYSIZE(kCredits7Pass1) } },
};

// Number of timer units a page stays on the screen
static const int kCreditsPageTicks = 4000;
static const int kCreditsMusic = 9;
// Color of the names, set to white in the palette of every page
static const int kCreditsNameColor = 0xC3;

/**
 * Centers the string on x. The underlined lines (the headings) have a bar of the text color, three rows high and
 * black at both ends, 18 rows below the top of the text.
 */
void IgorEngine::PART_CREDITS_DRAW_TEXT(int x, int y, uint8 color, bool underline, const char *str) {
	const int width = _font.getStringWidth(str);
	_font.drawString(_screenVGA, str, x - (width >> 1), y, color, 0, 0);
	if (underline) {
		const int barX = x - (width >> 1) - 10;
		const int barWidth = width + 21;
		memset(_screenVGA + (y + 18) * 320 + barX, 0, barWidth);
		memset(_screenVGA + (y + 19) * 320 + barX, color, barWidth);
		memset(_screenVGA + (y + 20) * 320 + barX, 0, barWidth);
		_screenVGA[(y + 19) * 320 + barX] = 0;
		_screenVGA[(y + 19) * 320 + barX + barWidth - 1] = 0;
	}
}

/**
 * States 910 .. 970: seven pages with the credits, shown twice (the second pass has other texts over the same
 * pictures), then the logos again.
 */
void IgorEngine::PART_CREDITS() {
	const CreditsPage &page = kCreditsPages[(_currentPart - 910) / 10];

	playMusic(kCreditsMusic);

	memset(_screenVGA, 0, 64000);
	memset(_currentPalette, 0, 768);
	setPaletteRange(0, 255);

	loadData(page.imgId, _screenVGA);
	loadData(page.palId, _paletteBuffer);
	SET_PAL_240_48_1();
	memset(_paletteBuffer + kCreditsNameColor * 3, 0x3F, 3);

	const int pass = (_creditsPass == 0) ? 0 : 1;
	for (int i = 0; i < page.count[pass]; ++i) {
		const CreditsLine &line = page.lines[pass][i];
		PART_CREDITS_DRAW_TEXT(line.x, line.y, line.color, line.underline, getString(line.strId));
	}

	if (_gameStateLoaded) {
		fadeIn(768);
		_gameStateLoaded = false;
	} else {
		PART_79_FADE_COLORS(0, 255, true);
	}

	for (int ticks = 0; ticks < kCreditsPageTicks; ticks += kTimerTicksCount) {
		if (_inputVars[kInputOptions]) {
			_inputVars[kInputOptions] = 0;
			handleOptionsMenu();
		}
		// the original does not look at escape on these pages
		if (_inputVars[kInputEscape] && !_eventQuitGame) {
			_inputVars[kInputEscape] = 0;
		}
		waitForTimer();
		if (_currentPart == kInvalidPart || _gameStateLoaded) {
			break;
		}
	}

	if (_gameStateLoaded || _currentPart == kInvalidPart) {
		// a game was loaded or the game is quit: the state is not changed
		fadeOut(768);
		SET_PAL_208_96_1();
		return;
	}

	PART_79_FADE_COLORS(0, 255, false);
	if (_currentPart == 970) {
		_currentPart = (_creditsPass == 1) ? 900 : 910;
		++_creditsPass;
	} else {
		_currentPart += 10;
	}
}

} // End of namespace Igor
