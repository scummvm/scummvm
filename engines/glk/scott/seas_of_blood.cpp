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

/*
 * Based on ScottFree interpreter version 1.14 developed by Swansea
 * University Computer Society without disassembly of any other game
 * drivers, only of game databases as permitted by EEC law (for purposes
 * of compatibility).
 *
 * Licensed under GPLv2
 *
 * https://github.com/angstsmurf/spatterlight/tree/master/terps/scott
 */

#include "glk/scott/scott.h"
#include "glk/scott/saga_draw.h"
#include "glk/scott/resource.h"
#include "glk/scott/decompress_text.h"
#include "glk/scott/seas_of_blood.h"

namespace Glk {
namespace Scott {

#define VICTORY 0
#define LOSS 1
#define DRAW 2
#define FLEE 3
#define ERROR 99

int getEnemyStats(int *strike, int *stamina, int *boatFlag);
void battleLoop(int enemy, int strike, int stamina, int boatFlag);
void swapStaminaAndCrewStrength(void);
void bloodBattle(void);

void adventureSheet(void) {
	g_scott->glk_stream_set_current(g_scott->glk_window_get_stream(_G(_bottomWindow)));
	g_scott->glk_set_style(style_Preformatted);
	g_scott->output("\nADVENTURE SHEET\n\n");
	g_scott->output("SKILL      :");
	g_scott->outputNumber(9);
	g_scott->output("      STAMINA      :");
	g_scott->outputNumber(_G(_counters)[3]);
	g_scott->output("\nLOG        :");
	g_scott->outputNumber(_G(_counters)[6]);
	if (_G(_counters)[6] < 10)
		g_scott->output("      PROVISIONS   :");
	else
		g_scott->output("     PROVISIONS   :"); // one less space!
	g_scott->outputNumber(_G(_counters)[5]);
	g_scott->output("\nCREW STRIKE:");
	g_scott->outputNumber(9);
	g_scott->output("      CREW STRENGTH:");
	g_scott->outputNumber(_G(_counters)[7]);
	g_scott->output("\n\n * * * * * * * * * * * * * * * * * * * * *\n\n");
	g_scott->listInventory();
	g_scott->output("\n");
	g_scott->glk_set_style(style_Normal);
}

void bloodAction(int p) {
	switch (p) {
	case 0:
		break;
	case 1:
		// Update LOG
		_G(_counters)[6]++;
		break;
	case 2:
		// Battle
		g_scott->look();
		g_scott->output("You are attacked \n");
		g_scott->output("<HIT ENTER> \n");
		g_scott->hitEnter();
		bloodBattle();
		break;
	default:
		error("bloodAction: Unhandled special action %d", p);
		break;
	}
}

void mirrorLeftHalf(void) {
	for (int line = 0; line < 12; line++) {
		for (int col = 32; col > 16; col--) {
			_G(_buffer)[line * 32 + col - 1][8] = _G(_buffer)[line * 32 + (32 - col)][8];
			for (int pixrow = 0; pixrow < 8; pixrow++)
				_G(_buffer)[line * 32 + col - 1][pixrow] = _G(_buffer)[line * 32 + (32 - col)][pixrow];
			flip(_G(_buffer)[line * 32 + col - 1]);
		}
	}
}

void replaceColour(uint8_t before, uint8_t after) {

	// I don't think any of the data has bit 7 set,
	// so masking it is probably unnecessary, but this is what
	// the original code does.
	uint8_t beforeink = before & 7;
	uint8_t afterink = after & 7;
	uint8_t inkmask = 0x07;

	uint8_t beforepaper = beforeink << 3;
	uint8_t afterpaper = afterink << 3;
	uint8_t papermask = 0x38;

	for (int j = 0; j < 384; j++) {
		if ((_G(_buffer)[j][8] & inkmask) == beforeink) {
			_G(_buffer)[j][8] = (_G(_buffer)[j][8] & ~inkmask) | afterink;
		}

		if ((_G(_buffer)[j][8] & papermask) == beforepaper) {
			_G(_buffer)[j][8] = (_G(_buffer)[j][8] & ~papermask) | afterpaper;
		}
	}
}

void drawColour(uint8_t x, uint8_t y, uint8_t colour, uint8_t length) {
	for (int i = 0; i < length; i++) {
		_G(_buffer)[y * 32 + x + i][8] = colour;
	}
}

void makeLight(void) {
	for (int i = 0; i < 384; i++) {
		_G(_buffer)[i][8] = _G(_buffer)[i][8] | 0x40;
	}
}

void flipImage(void) {

	uint8_t mirror[384][9];

	for (int line = 0; line < 12; line++) {
		for (int col = 32; col > 0; col--) {
			for (int pixrow = 0; pixrow < 9; pixrow++)
				mirror[line * 32 + col - 1][pixrow] = _G(_buffer)[line * 32 + (32 - col)][pixrow];
			flip(mirror[line * 32 + col - 1]);
		}
	}

	memcpy(_G(_buffer), mirror, 384 * 9);
}

void drawObjectImage(uint8_t x, uint8_t y) {
	for (int i = 0; i < _G(_gameHeader)->_numItems; i++) {
		if (_G(_items)[i]._flag != MY_LOC)
			continue;
		if (_G(_items)[i]._location != MY_LOC)
			continue;
		drawSagaPictureAtPos(_G(_items)[i]._image, x, y);
		_G(_shouldDrawObjectImages) = 0;
	}
}

void drawBlood(int loc) {
	memset(_G(_buffer), 0, 384 * 9);
	uint8_t *ptr = _G(_bloodImageData);
	for (int i = 0; i < loc; i++) {
		while (*(ptr) != 0xff)
			ptr++;
		ptr++;
	}
	while (ptr < _G(_bloodImageData) + 2010) {
		switch (*ptr) {
		case 0xff:
			if (loc == 13) {
				_G(_buffer)[8 * 32 + 18][8] = _G(_buffer)[8 * 32 + 18][8] & ~0x40;
				_G(_buffer)[8 * 32 + 17][8] = _G(_buffer)[8 * 32 + 17][8] & ~0x40;

				_G(_buffer)[8 * 32 + 9][8] = _G(_buffer)[8 * 32 + 9][8] & ~0x40;
				_G(_buffer)[8 * 32 + 10][8] = _G(_buffer)[8 * 32 + 10][8] & ~0x40;
			}
			return;
		case 0xfe:
			mirrorLeftHalf();
			break;
		case 0xfD:
			replaceColour(*(ptr + 1), *(ptr + 2));
			ptr += 2;
			break;
		case 0xfc: // Draw colour: x, y, attribute, length
			drawColour(*(ptr + 1), *(ptr + 2), *(ptr + 3), *(ptr + 4));
			ptr = ptr + 4;
			break;
		case 0xfb: // Make all screen colours bright
			makeLight();
			break;
		case 0xfa: // Flip entire image horizontally
			flipImage();
			break;
		case 0xf9: // Draw object image (if present) at x, y
			drawObjectImage(*(ptr + 1), *(ptr + 2));
			ptr = ptr + 2;
			break;
		default: // else draw image *ptr at x, y
			drawSagaPictureAtPos(*ptr, *(ptr + 1), *(ptr + 2));
			ptr = ptr + 2;
		}
		ptr++;
	}
}

void seasOfBloodRoomImage(void) {
	_G(_shouldDrawObjectImages) = 1;
	drawBlood(MY_LOC);
	for (int ct = 0; ct <= _G(_gameHeader)->_numItems; ct++)
		if (_G(_items)[ct]._image && _G(_shouldDrawObjectImages)) {
			if ((_G(_items)[ct]._flag & 127) == MY_LOC && _G(_items)[ct]._location == MY_LOC) {
				g_scott->drawImage(_G(_items)[ct]._image);
			}
		}
	drawSagaPictureFromBuffer();
}

//static void SOBPrint(winid_t w, const char *fmt, ...)
//#ifdef __GNUC__
//	__attribute__((__format__(__printf__, 2, 3)))
//#endif
//	;

static void SOBPrint(winid_t w, const char *fmt, ...) {
	va_list ap;
	char msg[2048];

	int size = sizeof msg;

	va_start(ap, fmt);
	vsnprintf(msg, size, fmt, ap);
	va_end(ap);

	g_scott->glk_put_string_stream(g_scott->glk_window_get_stream(w), msg);
}

glui32 optimalDicePixelSize(glui32 *width, glui32 *height) {
	int idealWidth = 8;
	int idealHeight = 8;

	*width = idealWidth;
	*height = idealHeight;
	int multiplier = 1;
	uint graphwidth, graphheight;
	g_scott->glk_window_get_size(_G(_leftDiceWin), &graphwidth, &graphheight);
	multiplier = graphheight / idealHeight;
	if ((glui32)(idealWidth * multiplier) > graphwidth)
		multiplier = graphwidth / idealWidth;

	if (multiplier < 2)
		multiplier = 2;

	multiplier = multiplier / 2;

	*width = idealWidth * multiplier;
	*height = idealHeight * multiplier;

	return multiplier;
}

static void drawBorder(winid_t win) {
	uint width, height;
	g_scott->glk_stream_set_current(g_scott->glk_window_get_stream(win));
	g_scott->glk_window_get_size(win, &width, &height);
	height--;
	width -= 2;
	g_scott->glk_window_move_cursor(win, 0, 0);
	g_scott->glk_put_char_uni(0x250F); // Top left corner
	for (glui32 i = 1; i < width; i++)
		g_scott->glk_put_char_uni(0x2501); // Top
	g_scott->glk_put_char_uni(0x2513);     // Top right corner
	for (glui32 i = 1; i < height; i++) {
		g_scott->glk_window_move_cursor(win, 0, i);
		g_scott->glk_put_char_uni(0x2503);
		g_scott->glk_window_move_cursor(win, width, i);
		g_scott->glk_put_char_uni(0x2503);
	}
	g_scott->glk_window_move_cursor(win, 0, height);
	g_scott->glk_put_char_uni(0x2517);
	for (glui32 i = 1; i < width; i++)
		g_scott->glk_put_char_uni(0x2501);
	g_scott->glk_put_char_uni(0x251B);
}

static void redrawStaticText(winid_t win, int boatFlag) {
	g_scott->glk_stream_set_current(g_scott->glk_window_get_stream(win));
	g_scott->glk_window_move_cursor(win, 2, 4);

	if (boatFlag) {
		g_scott->glk_put_string("STRIKE  :\n");
		g_scott->glk_window_move_cursor(win, 2, 5);
		g_scott->glk_put_string("CRW STR :");
	} else {
		g_scott->glk_put_string("SKILL   :\n");
		g_scott->glk_window_move_cursor(win, 2, 5);
		g_scott->glk_put_string("STAMINA :");
	}

	if (win == _G(_battleRight)) {
		uint width;
		g_scott->glk_window_get_size(_G(_battleRight), &width, 0);
		g_scott->glk_window_move_cursor(_G(_battleRight), width - 6, 1);
		g_scott->glk_put_string("YOU");
	}
}

static void redrawBattleScreen(int boatFlag) {
	uint graphwidth, graphheight;
	glui32 optimal_width, optimal_height;

	g_scott->glk_window_get_size(_G(_leftDiceWin), &graphwidth, &graphheight);

	_G(_dicePixelSize) = optimalDicePixelSize(&optimal_width, &optimal_height);
	_G(_diceXOffset) = (graphwidth - optimal_width) / 2;
	_G(_diceYOffset) = (graphheight - optimal_height - _G(_dicePixelSize)) / 2;

	drawBorder(_G(_topWindow));
	drawBorder(_G(_battleRight));

	redrawStaticText(_G(_topWindow), boatFlag);
	redrawStaticText(_G(_battleRight), boatFlag);
}

static void setupBattleScreen(int boatFlag) {
	winid_t parent = g_scott->glk_window_get_parent(_G(_topWindow));
	g_scott->glk_window_set_arrangement(parent, winmethod_Above | winmethod_Fixed, 7, _G(_topWindow));

	g_scott->glk_window_clear(_G(_topWindow));
	g_scott->glk_window_clear(_G(_bottomWindow));
	_G(_battleRight) = g_scott->glk_window_open(_G(_topWindow), winmethod_Right | winmethod_Proportional,
								  50, wintype_TextGrid, 0);

	_G(_leftDiceWin) = g_scott->glk_window_open(_G(_topWindow), winmethod_Right | winmethod_Proportional,
								  30, wintype_Graphics, 0);
	_G(_rightDiceWin) = g_scott->glk_window_open(_G(_battleRight), winmethod_Left | winmethod_Proportional, 30,
								   wintype_Graphics, 0);

	// Set the graphics window background to match the top window background, best as we can, and clear the window.
	 
	if (g_scott->glk_style_measure(_G(_topWindow), style_Normal, stylehint_BackColor,
						  &_G(_backgroundColour))) {
		g_scott->glk_window_set_background_color(_G(_leftDiceWin), _G(_backgroundColour));
		g_scott->glk_window_set_background_color(_G(_rightDiceWin), _G(_backgroundColour));

		g_scott->glk_window_clear(_G(_leftDiceWin));
		g_scott->glk_window_clear(_G(_rightDiceWin));
	}

	if (_G(_palChosen) == C64B)
		_G(_diceColour) = 0x5f48e9;
	else
		_G(_diceColour) = 0xff0000;

	redrawBattleScreen(boatFlag);
}

void bloodBattle(void) {
	int enemy, strike, stamina, boatFlag;
	enemy = getEnemyStats(&strike, &stamina, &boatFlag); // Determine their stats
	if (enemy == 0) {
		error("Seas of blood battle: No enemy in location?\n");
		return;
	}
	setupBattleScreen(boatFlag);
	battleLoop(enemy, strike, stamina, boatFlag); // Into the battle loops
	if (boatFlag)
		swapStaminaAndCrewStrength(); // Switch back stamina - crew strength
	g_scott->glk_window_close(_G(_leftDiceWin), nullptr);
	g_scott->glk_window_close(_G(_rightDiceWin), nullptr);
	g_scott->glk_window_close(_G(_battleRight), nullptr);
	g_scott->closeGraphicsWindow();
	g_scott->openGraphicsWindow();
	seasOfBloodRoomImage();
}

int getEnemyStats(int *strike, int *stamina, int *boatFlag) {
	int enemy, i = 0;
	while (i < 124) {
		enemy = _G(_enemyTable)[i];
		if (_G(_items)[enemy]._location == MY_LOC) {
			i++;
			*strike = _G(_enemyTable)[i++];
			*stamina = _G(_enemyTable)[i++];
			*boatFlag = _G(_enemyTable)[i];
			if (*boatFlag) {
				swapStaminaAndCrewStrength(); // Switch stamina - crew strength
			}

			return enemy;
		}
		i = i + 4; // Skip to next entry
	}
	return 0;
}

void drawRect(winid_t win, int32_t x, int32_t y, int32_t width, int32_t height,
			   int32_t color) {
	g_scott->glk_window_fill_rect(win, color, x * _G(_dicePixelSize) + _G(_diceXOffset),
						 y * _G(_dicePixelSize) + _G(_diceYOffset),
						 width * _G(_dicePixelSize), height * _G(_dicePixelSize));
}

void drawGraphicalDice(winid_t win, int number) {
	// The eye-less dice backgrounds consist of two rectangles on top of each
	// other
	drawRect(win, 1, 2, 7, 5, _G(_diceColour));
	drawRect(win, 2, 1, 5, 7, _G(_diceColour));

	switch (number + 1) {
	case 1:
		drawRect(win, 4, 4, 1, 1, _G(_backgroundColour));
		break;
	case 2:
		drawRect(win, 2, 6, 1, 1, _G(_backgroundColour));
		drawRect(win, 6, 2, 1, 1, _G(_backgroundColour));
		break;
	case 3:
		drawRect(win, 2, 6, 1, 1, _G(_backgroundColour));
		drawRect(win, 4, 4, 1, 1, _G(_backgroundColour));
		drawRect(win, 6, 2, 1, 1, _G(_backgroundColour));
		break;
	case 4:
		drawRect(win, 2, 6, 1, 1, _G(_backgroundColour));
		drawRect(win, 6, 2, 1, 1, _G(_backgroundColour));
		drawRect(win, 2, 2, 1, 1, _G(_backgroundColour));
		drawRect(win, 6, 6, 1, 1, _G(_backgroundColour));
		break;
	case 5:
		drawRect(win, 2, 6, 1, 1, _G(_backgroundColour));
		drawRect(win, 6, 2, 1, 1, _G(_backgroundColour));
		drawRect(win, 2, 2, 1, 1, _G(_backgroundColour));
		drawRect(win, 6, 6, 1, 1, _G(_backgroundColour));
		drawRect(win, 4, 4, 1, 1, _G(_backgroundColour));
		break;
	case 6:
		drawRect(win, 2, 6, 1, 1, _G(_backgroundColour));
		drawRect(win, 6, 2, 1, 1, _G(_backgroundColour));
		drawRect(win, 2, 2, 1, 1, _G(_backgroundColour));
		drawRect(win, 2, 4, 1, 1, _G(_backgroundColour));
		drawRect(win, 6, 4, 1, 1, _G(_backgroundColour));
		drawRect(win, 6, 6, 1, 1, _G(_backgroundColour));
		break;
	default:
		break;
	}
}

void updateDice(int ourTurn, int leftDice, int rightDice) {
	leftDice--;
	rightDice--;
	g_scott->glk_window_clear(_G(_leftDiceWin));
	g_scott->glk_window_clear(_G(_rightDiceWin));

	_G(_diceXOffset) = _G(_diceXOffset) - _G(_dicePixelSize);
	drawGraphicalDice(_G(_leftDiceWin), leftDice);
	_G(_diceXOffset) = _G(_diceXOffset) + _G(_dicePixelSize);
	drawGraphicalDice(_G(_rightDiceWin), rightDice);

	winid_t win = ourTurn ? _G(_battleRight) : _G(_topWindow);

	g_scott->glk_window_move_cursor(win, 2, 1);

	g_scott->glk_stream_set_current(g_scott->glk_window_get_stream(win));

	g_scott->glk_put_char_uni(0x2680 + leftDice);
	g_scott->glk_put_char('+');
	g_scott->glk_put_char_uni(0x2680 + rightDice);

	g_scott->glk_window_move_cursor(win, 2, 2);
	SOBPrint(win, "%d %d", leftDice + 1, rightDice + 1);
}

void printSum(int ourTurn, int sum, int strike) {
	winid_t win = ourTurn ? _G(_battleRight) : _G(_topWindow);
	g_scott->glk_stream_set_current(g_scott->glk_window_get_stream(win));

	g_scott->glk_window_move_cursor(win, 6, 1);

	SOBPrint(win, "+ %d = %d", strike, sum);

	g_scott->glk_stream_set_current(g_scott->glk_window_get_stream(_G(_battleRight)));
	g_scott->glk_window_move_cursor(_G(_battleRight), 6, 1);
	g_scott->glk_put_string("+ 9 = ");
}

void updateResult(int ourTurn, int strike, int stamina, int boatFlag) {
	winid_t win = ourTurn ? _G(_battleRight) : _G(_topWindow);
	g_scott->glk_stream_set_current(g_scott->glk_window_get_stream(win));

	g_scott->glk_window_move_cursor(win, 2, 4);

	if (boatFlag) {
		SOBPrint(win, "STRIKE  : %d\n", strike);
		g_scott->glk_window_move_cursor(win, 2, 5);
		SOBPrint(win, "CRW STR : %d", stamina);
	} else {
		SOBPrint(win, "SKILL   : %d\n", strike);
		g_scott->glk_window_move_cursor(win, 2, 5);
		SOBPrint(win, "STAMINA : %d", stamina);
	}
}

void clearResult(void) {
	winid_t win = _G(_topWindow);

	uint width;
	for (int j = 0; j < 2; j++) {
		g_scott->glk_window_get_size(win, &width, 0);
		g_scott->glk_stream_set_current(g_scott->glk_window_get_stream(win));

		g_scott->glk_window_move_cursor(win, 11, 1);
		for (int i = 0; i < 10; i++)
			g_scott->glk_put_string(" ");
		drawBorder(win);
		win = _G(_battleRight);
	}
}

void clearStamina(void) {
	winid_t win = _G(_topWindow);

	uint width;
	for (int j = 0; j < 2; j++) {
		g_scott->glk_window_get_size(win, &width, 0);
		g_scott->glk_stream_set_current(g_scott->glk_window_get_stream(win));

		g_scott->glk_window_move_cursor(win, 11, 5);
		for (int i = 0; i < (int)width - 13; i++)
			g_scott->glk_put_string(" ");
		drawBorder(win);
		win = _G(_battleRight);
	}
}

static void rearrangeBattleDisplay(int strike, int stamina, int boatFlag) {
	g_scott->updateSettings();
	g_scott->glk_cancel_char_event(_G(_topWindow));
	g_scott->closeGraphicsWindow();
	g_scott->glk_window_close(_G(_battleRight), nullptr);
	g_scott->glk_window_close(_G(_leftDiceWin), nullptr);
	g_scott->glk_window_close(_G(_rightDiceWin), nullptr);
	seasOfBloodRoomImage();
	setupBattleScreen(boatFlag);
	updateResult(0, strike, stamina, boatFlag);
	updateResult(1, 9, _G(_counters)[3], boatFlag);
	drawBorder(_G(_topWindow));
	drawBorder(_G(_battleRight));
	redrawStaticText(_G(_topWindow), boatFlag);
	redrawStaticText(_G(_battleRight), boatFlag);
	g_scott->glk_request_char_event(_G(_topWindow));
}

int rollDice(int strike, int stamina, int boatFlag) {
	clearResult();
	redrawStaticText(_G(_battleRight), boatFlag);

	g_scott->glk_request_timer_events(60);
	int rolls = 0;
	int ourTurn = 0;
	int leftDice = 1 + g_scott->getRandomNumber(100) % 6;
	int rightDice = 1 + g_scott->getRandomNumber(100) % 6;
	int ourResult;
	int theirResult = 0;
	int theirDiceStopped = 0;

	event_t event;
	int enemyRolls = 20 + g_scott->getRandomNumber(100) % 10;
	g_scott->glk_cancel_char_event(_G(_topWindow));
	g_scott->glk_request_char_event(_G(_topWindow));

	g_scott->glk_stream_set_current(g_scott->glk_window_get_stream(_G(_bottomWindow)));
	g_scott->glk_put_string("Their throw");

	int delay = 60;

	while (!g_vm->shouldQuit()) {
		g_scott->glk_select(&event);
		if (event.type == evtype_Timer) {
			if (theirDiceStopped) {
				g_scott->glk_request_timer_events(60);
				theirDiceStopped = 0;
				printSum(ourTurn, theirResult, strike);
				ourTurn = 1;
				g_scott->glk_window_clear(_G(_bottomWindow));
				g_scott->glk_cancel_char_event(_G(_topWindow));
				g_scott->glk_request_char_event(_G(_topWindow));
				g_scott->glk_stream_set_current(g_scott->glk_window_get_stream(_G(_bottomWindow)));
				g_scott->glk_put_string("Your throw\n");
				g_scott->glk_put_string("<ENTER> to stop dice");
				if (!boatFlag)
					g_scott->glk_put_string("    <X> to run");
			} else if (ourTurn == 0) {
				g_scott->glk_request_timer_events(delay);
			}

			rolls++;

			if (rolls % 2)
				leftDice = 1 + g_scott->getRandomNumber(100) % 6;
			else
				rightDice = 1 + g_scott->getRandomNumber(100) % 6;

			updateDice(ourTurn, leftDice, rightDice);
			if (ourTurn == 0 && rolls == enemyRolls) {
				g_scott->glk_window_clear(_G(_bottomWindow));
				theirResult = leftDice + rightDice + strike;
				SOBPrint(_G(_bottomWindow), "Their result: %d + %d + %d = %d\n", leftDice,
						 rightDice, strike, theirResult);
				g_scott->glk_request_timer_events(1000);
				theirDiceStopped = 1;
			}
		}

		if (event.type == evtype_CharInput) {
			if (ourTurn && (event.val1 == keycode_Return)) {
				updateDice(ourTurn, leftDice, rightDice);
				ourResult = leftDice + rightDice + 9;
				printSum(ourTurn, ourResult, 9);
				if (theirResult > ourResult) {
					return LOSS;
				} else if (ourResult > theirResult) {
					return VICTORY;
				} else {
					return DRAW;
				}
			} else if (MY_LOC != 1 && (event.val1 == 'X' || event.val1 == 'x')) {
				MY_LOC = _G(_savedRoom);
				return FLEE;
			} else {
				g_scott->glk_request_char_event(_G(_topWindow));
			}
		}
		if (event.type == evtype_Arrange) {
			rearrangeBattleDisplay(strike, stamina, boatFlag);
		}
	}
	return ERROR;
}

void battleHitEnter(int strike, int stamina, int boatFlag) {
	g_scott->glk_request_char_event(_G(_bottomWindow));
	event_t ev;
	int result = 0;
	do {
		g_scott->glk_select(&ev);
		if (ev.type == evtype_CharInput) {
			if (ev.val1 == keycode_Return) {
				result = 1;
			} else {
				g_scott->glk_request_char_event(_G(_bottomWindow));
			}
		}

		if (ev.type == evtype_Arrange) {
			rearrangeBattleDisplay(strike, stamina, boatFlag);
		}

	} while (result == 0 && !g_vm->shouldQuit());
}

void battleLoop(int enemy, int strike, int stamina, int boatFlag) {
	updateResult(0, strike, stamina, boatFlag);
	updateResult(1, 9, _G(_counters)[3], boatFlag);
	do {
		int result = rollDice(strike, stamina, boatFlag);
		g_scott->glk_cancel_char_event(_G(_topWindow));
		g_scott->glk_window_clear(_G(_bottomWindow));
		clearStamina();
		g_scott->glk_stream_set_current(g_scott->glk_window_get_stream(_G(_bottomWindow)));
		if (result == LOSS) {
			_G(_counters)[3] -= 2;

			if (_G(_counters)[3] <= 0) {
				SOBPrint(_G(_bottomWindow), "%s\n",
						 boatFlag ? "THE BANSHEE HAS BEEN SUNK!"
								  : "YOU HAVE BEEN KILLED!");
				_G(_counters)[3] = 0;
				_G(_bitFlags) |= (1 << 6);
				_G(_counters)[7] = 0;
			} else {
				SOBPrint(_G(_bottomWindow), "%s", _G(_battleMessages)[1 + g_scott->getRandomNumber(100) % 5 + 16 * boatFlag]);
			}
		} else if (result == VICTORY) {
			stamina -= 2;
			if (stamina <= 0) {
				g_scott->glk_put_string("YOU HAVE WON!\n");
				_G(_bitFlags) &= ~(1 << 6);
				stamina = 0;
			} else {
				SOBPrint(_G(_bottomWindow), "%s", _G(_battleMessages)[6 + g_scott->getRandomNumber(100) % 5 + 16 * boatFlag]);
			}
		} else if (result == FLEE) {
			_G(_bitFlags) |= (1 << 6);
			MY_LOC = _G(_savedRoom);
			return;
		} else {
			SOBPrint(_G(_bottomWindow), "%s", _G(_battleMessages)[11 + g_scott->getRandomNumber(100) % 5 + 16 * boatFlag]);
		}

		g_scott->glk_put_string("\n\n");

		if (_G(_counters)[3] > 0 && stamina > 0) {
			g_scott->glk_put_string("<ENTER> to roll dice");
			if (!boatFlag)
				g_scott->glk_put_string("    <X> to run");
		}

		updateResult(0, strike, stamina, boatFlag);
		updateResult(1, 9, _G(_counters)[3], boatFlag);

		battleHitEnter(strike, stamina, boatFlag);
		g_scott->glk_window_clear(_G(_bottomWindow));
	} while (stamina > 0 && _G(_counters)[3] > 0 && !g_vm->shouldQuit());
}

void swapStaminaAndCrewStrength(void) {
	uint8_t temp = _G(_counters)[7]; // Crew strength
	_G(_counters)[7] = _G(_counters)[3];  // Stamina
	_G(_counters)[3] = temp;
}

int loadExtraSeasOfBloodData(void) {
	_G(_drawToBuffer) = 1;

	int offset;

	offset = 0x47b7 + _G(_fileBaselineOffset);

	uint8_t *ptr = seekToPos(_G(_entireFile), offset);

	int ct;

	for (ct = 0; ct < 124; ct++) {
		_G(_enemyTable)[ct] = *(ptr++);
		if (_G(_enemyTable)[ct] == 0xff)
			break;
	}

	ptr = seekToPos(_G(_entireFile), 0x71DA + _G(_fileBaselineOffset));

	for (int i = 0; i < 32; i++) {
		_G(_battleMessages)[i] = decompressText(ptr, i);
	}

	offset = 0x7af5 - 16357 + _G(_fileBaselineOffset);

	int data_length = 2010;

	_G(_bloodImageData) = new uint8_t[data_length];
	ptr = seekToPos(_G(_entireFile), offset);
	for (int i = 0; i < data_length; i++)
		_G(_bloodImageData)[i] = *(ptr++);

	for (int i = I_DONT_UNDERSTAND; i <= THATS_BEYOND_MY_POWER; i++)
		_G(_sys)[i] = _G(_systemMessages)[4 - I_DONT_UNDERSTAND + i];

	for (int i = YOU_ARE; i <= HIT_ENTER; i++)
		_G(_sys)[i] = _G(_systemMessages)[13 - YOU_ARE + i];

	_G(_sys)[OK] = _G(_systemMessages)[2];
	_G(_sys)[PLAY_AGAIN] = _G(_systemMessages)[3];
	_G(_sys)[YOURE_CARRYING_TOO_MUCH] = _G(_systemMessages)[27];

	_G(_items)[125]._text = "A loose plank";
	_G(_items)[125]._autoGet = "PLAN";

	return 0;
}

int loadExtraSeasOfBlood64Data(void) {
	_G(_drawToBuffer) = 1;

	int offset;

	offset = 0x3fee + _G(_fileBaselineOffset);
	uint8_t *ptr;

	ptr = seekToPos(_G(_entireFile), offset);

	int ct;
	for (ct = 0; ct < 124; ct++) {
		_G(_enemyTable)[ct] = *(ptr++);
		if (_G(_enemyTable)[ct] == 0xff)
			break;
	}

	offset = 0x82f6 + _G(_fileBaselineOffset);
	ptr = seekToPos(_G(_entireFile), offset);

	for (int i = 0; i < 32; i++) {
		_G(_battleMessages)[i] = decompressText(ptr, i);
	}

	offset = 0x5299 + _G(_fileBaselineOffset);

	int data_length = 2010;

	_G(_bloodImageData) = new uint8_t[data_length];

	ptr = seekToPos(_G(_entireFile), offset);
	for (int i = 0; i < data_length; i++) {
		_G(_bloodImageData)[i] = *(ptr++);
	}

	SysMessageType messagekey[] = {
		NORTH,
		SOUTH,
		EAST,
		WEST,
		UP,
		DOWN,
		EXITS,
		YOU_SEE,
		YOU_ARE,
		YOU_CANT_GO_THAT_WAY,
		OK,
		WHAT_NOW,
		HUH,
		YOU_HAVE_IT,
		YOU_HAVENT_GOT_IT,
		DROPPED,
		TAKEN,
		INVENTORY,
		YOU_DONT_SEE_IT,
		THATS_BEYOND_MY_POWER,
		DIRECTION,
		YOURE_CARRYING_TOO_MUCH,
		PLAY_AGAIN,
		RESUME_A_SAVED_GAME,
		YOU_CANT_DO_THAT_YET,
		I_DONT_UNDERSTAND,
		NOTHING};

	for (int i = 0; i < 27; i++) {
		_G(_sys)[messagekey[i]] = _G(_systemMessages)[i];
	}

	return 0;
}

} // End of namespace Scott
} // End of namespace Glk


