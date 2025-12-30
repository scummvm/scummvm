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

/************************************************************************/
/************************************************************************/
/*																		*/
/*						M A I N		P R O G R A M						*/
/*																		*/
/************************************************************************/
/************************************************************************/

/************************************************************************/
/*	Outstanding work :-	LOAD AND SAVE, FIX THE DOOR PROBLEM.
************************************************************************/

#include "ultima/ultima0/akalabeth.h"

namespace Ultima {
namespace Ultima0 {

PLAYER 	   Player;							/* Player data */
WORLDMAP   World;							/* Current world map */
DUNGEONMAP Dungeon;							/* Current dungeon map */

static void _MAINIntro(void);
static void _MAINDisplay(WORLDMAP *, PLAYER *, DUNGEONMAP *);
static void _MAINCommand(int, WORLDMAP *, PLAYER *, DUNGEONMAP *);

/************************************************************************/
/*																		*/
/*								Main Program							*/
/*																		*/
/************************************************************************/

void MAINStart(void) {
	int c;
	RECT r1, r2;
	HWInitialise();							/* Initialise the game */
	HWClear();								/* Clear the graphic display */
	HWStatus(0, 0, 0);						/* Zero the status */
	_MAINIntro();							/* Print startup information */
	MAINSetup();							/* Set up the player */

	do
	{
		_MAINDisplay(&World, &Player,		/* Redraw the main display */
			&Dungeon);
		DRAWText("]"); c = HWGetKey();		/* Read the next command */
		_MAINCommand(c, &World, &Player,		/* Do it */
			&Dungeon);
		if (Player.Attr[AT_HP] <= 0)		/* Died.... */
			DEADCode(&Player);
		if (c == 'Q')						/* Confirm exit */
		{
			DRAWText("Are you sure ?\n");
			if (DEADGetYesNo() == 0) c = '\0';
		}
	} while (c != 'Q' && Player.Attr[AT_HP] > 0);
	HWTerminate();
}

/************************************************************************/
/*																		*/
/*							Set everything up							*/
/*																		*/
/************************************************************************/

void MAINSetup(void) {
	PLAYERInit(&Player);					/* Initialise the player */
	PLAYERCharacter(&Player);				/* Character information */
	WORLDCreate(&Player, &World);			/* Create the over world */
	WORLDDraw(&Player, &World, 0);			/* Draw the player map */
	TOWNEnter(&World, &Player);				/* Visit the shops */
}

/************************************************************************/
/*																		*/
/*							Execute a command							*/
/*																		*/
/************************************************************************/

static void _MAINCommand(int c, WORLDMAP *w, PLAYER *p, DUNGEONMAP *d) {
	if (c == 'A' && p->Level == 0)			/* Only attack in dungeon */
		c = '\0';
	switch (c)
	{
	case 'A':
		DRAWText("Attack.\n");
		ATTACKMonster(p, d);
		break;

	case 'N':
		DRAWText(p->Level == 0 ? "Go North.\n" : "Move.\n");
		MOVEMove(c, w, p, d, 0, -1);
		break;
	case 'S':
		DRAWText(p->Level == 0 ? "Go South.\n" : "Turn Around.\n");
		MOVEMove(c, w, p, d, 0, 1);
		break;
	case 'E':
		DRAWText(p->Level == 0 ? "Go East.\n" : "Turn Right.\n");
		MOVEMove(c, w, p, d, 1, 0);
		break;
	case 'W':
		DRAWText(p->Level == 0 ? "Go West.\n" : "Turn Left.\n");
		MOVEMove(c, w, p, d, -1, 0);
		break;
	case 'I':
		DRAWText("Inventory.\n");
		PLAYERInv(&Player);
		break;
	case 'Q':
		DRAWText("Quit.\n");
		break;
	case 'X':
		MOVEEnterExit(w, p, d);
		break;
	default:
		DRAWText("Huh???\n");
		break;
	}
}

/************************************************************************/
/*																		*/
/*							Introduction text							*/
/*																		*/
/************************************************************************/

static void _MAINIntro(void) {
	DRAWText("Aklabeth by P Robson\n\n");
	DRAWText("From the Apple II\n");
	DRAWText("game by R Garriott\n\n");
	DRAWText("Press any key.\n\n");
	HWGetKey();
}

/************************************************************************/
/*																		*/
/*					Do we support improved features ?					*/
/*																		*/
/************************************************************************/

int MAINSuper(void) {
	return 1;
}

/************************************************************************/
/*																		*/
/*						Redraw the main display							*/
/*																		*/
/************************************************************************/

static void _MAINDisplay(WORLDMAP *w, PLAYER *p, DUNGEONMAP *d) {
	HWStatus(p->Object[OB_FOOD],			/* Refresh the status bar */
		p->Attr[AT_HP], p->Attr[AT_GOLD]);
	HWClear();								/* Clear the screen */
	if (p->Level == 0)						/* Above ground, draw it */
		WORLDDraw(p, w, 0);
	else
		DDRAWDraw(p, d);
}

} // namespace Ultima0
} // namespace Ultima
