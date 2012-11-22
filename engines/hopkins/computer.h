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

#ifndef HOPKINS_COMPUTER_H
#define HOPKINS_COMPUTER_H

#include "common/scummsys.h"
#include "common/rect.h"
#include "common/str.h"

namespace Hopkins {

class HopkinsEngine;

struct MenuItem {
	bool _actvFl;
	int  _lineSize;
	char _line[90];
};

struct ScoreItem {
	Common::String name;
	Common::String score;
};

enum ComputerEnum { COMPUTER_HOPKINS = 1, COMPUTER_SAMANTHAS = 2, COMPUTER_PUBLIC = 3 };

class ComputerManager {
private:
	HopkinsEngine *_vm;
	MenuItem MenuText[50];
	char Sup_string[200];
	ScoreItem Score[6];
	int TEXT_COL;
	Common::Point _textPosition;
	Common::Point BALLE;
	byte *CASSESPR;
	int16 *CASSETAB;
	bool FMOUSE;
	int NBBRIQUES;

	/**
	 * Breakout score
	 */
	int CASSESCORE;

	/**
	 * Number of Breakout lives remaining
	 */
	int CASSEVIE;

	/**
	 * Breakout game speed
	 */
	int CASSESPEED;

	/**
	 * Breakout ball horizontal position
	 */
	int BALLEHORI;

	/**
	 * Breakout ball vertical position
	 */
	int BALLEVERTI;

	int NB_TABLE;
	int RAQX;
	int CASSE_HISCORE;
	int CASSEP1;
	int CASSEP2;
	int CASSDER;
	int Menu_lignes; // CHECKME: Useless variable

	/**
	 * Load the menu text
	 */
	void Charge_Menu();

	void TXT4(int xp, int yp, int textIdx);

	/**
	 * Restores the scene for the FBI headquarters room
	 */
	void RESTORE_POLICE();

	/**
	 * Sets up textual entry mode. Used by the code for Hopkins computer.
	 */
	void setvideomode();

	/**
	 * Sets up textual entry mode. Used by the code for Hopkins computer.
	 */
	void TEXT_MODE();

	/**
	 * Clear the screen
	 */
	void clearscreen();

	/**
	 * Sets the text mode color
	 */
	void settextcolor(int col);

	/**
	 * Sets the text position.
	 * @param yp		Y position
	 * @param xp		X position
	 * @remarks		Yes, the reverse co-ordinate pair is really like that in the original game.
	 */
	void settextposition(int yp, int xp);

	/**
	 * Outputs a text string
	 */
	void outtext(const Common::String &msg);

	/**
	 * Outputs a text string
	 */
	void outtext2(const Common::String &msg);

	void GAMES();
	void LIT_TEXTE(int a1);
	void CHARGE_SCORE();
	void MODE_VGA256();
	void NEWTAB();
	void AFF_BRIQUES();
	void AFF_VIE();

	/**
	 * Play the Breakout game
	 */
	void PLAY_BRIQUE();

	int HIGHT_SCORE();
	void NAME_SCORE();
	void IMPSCORE(int a1, int a2);
	void SAUVE_SCORE();
	void PRINT_HSCORE(byte *objectData, int a2, int a3, int a4);
	void IMPRIMESCORE();
	int DEP_BALLE();
	void VERIFBRIQUES();
public:
	ComputerManager();
	void setParent(HopkinsEngine *vm);

	/**
	 * Show a computer in the FBI office
	 * @param mode		Which computer to display
	 */
	void COMPUT_HOPKINS(ComputerEnum mode);
};

} // End of namespace Hopkins

#endif /* HOPKINS_COMPUTER_H */
