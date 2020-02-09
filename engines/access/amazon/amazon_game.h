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

#ifndef ACCESS_AMAZON_GAME_H
#define ACCESS_AMAZON_GAME_H

#include "access/access.h"
#include "access/amazon/amazon_logic.h"

namespace Access {

namespace Amazon {

class AmazonEngine;

class AmazonEngine : public AccessEngine {
private:
	byte _tileData[1455];
	Common::Array<CellIdent> _chapterCells;

	/**
	 * Setup variables for the game
	 */
	void setupGame();

	/**
	 * Initialize variables found in the config file
	 */
	void configSelect();

	void initVariables();
	void initObjects();
	void calcIQ();
	void helpTitle();
	void drawHelpText(const Common::String &msg);
	void loadEstablish(int estabIndex);
	void doEstablish(int screenId, int estabIndex);

protected:
	/**
	 * Play the game
	 */
	void playGame() override;

	/**
	* Synchronize savegame data
	*/
	void synchronize(Common::Serializer &s) override;
public:
	InactivePlayer _inactive;
	bool _charSegSwitch;
	byte _help1[366];
	byte _help2[366];
	byte _help3[366];
	byte *_helpTbl[3];

	// Fields that are mapped to flags
	int &_guardLocation;
	int &_guardFind;
	int &_helpLevel;
	int &_jasMayaFlag;
	int &_moreHelp;
	int &_flashbackFlag;
	int &_riverFlag;
	int &_aniOutFlag;
	int &_badEnd;
	int &_noHints;
	int &_aniFlag;
	int &_allenFlag;
	int &_noSound;

	// Saved fields
	int _chapter;
	int _rawInactiveX;
	int _rawInactiveY;
	int _inactiveYOff;

	// Other game specific fields
	Ant *_ant;
	Cast *_cast;
	Guard *_guard;
	Jungle *_jungle;
	Opening *_opening;
	Plane *_plane;
	River *_river;
	int _hintLevel;
	int _updateChapter;
	int _oldTitleChapter;
	int _iqValue;
public:
	AmazonEngine(OSystem *syst, const AccessGameDescription *gameDesc);

	~AmazonEngine() override;

	void dead(int deathId) override;

	/**
	* Free the inactive player data
	*/
	void freeInactivePlayer();

	void drawHelp(const Common::String str);

	void establish(int esatabIndex, int sub) override;

	void tileScreen();
	void updateSummary(int chap);
	void establishCenter(int screenId, int esatabIndex);

	/**
	 * Show the start of a chapter
	 */
	void startChapter(int chapter);
};

} // End of namespace Amazon

} // End of namespace Access

#endif /* ACCESS_ACCESS_H */
