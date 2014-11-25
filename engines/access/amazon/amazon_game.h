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

#ifndef ACCESS_AMAZON_GAME_H
#define ACCESS_AMAZON_GAME_H

#include "access/access.h"

namespace Access {

namespace Amazon {

class Plane {
public:
	int _pCount;
	Common::Point _position;
	int _planeCount;
	int _propCount;
	int _xCount;
public:
	Plane();
};

class Guard {
public:
	int _guardCel;
	Common::Point _position;
	int _gCode1;
	int _gCode2;
	Common::Rect _bounds;
	int _xMid, _yMid;
public:
	Guard();
};

class InactivePlayer : public ImageEntry {
public:
	SpriteResource *_altSpritesPtr;
};

class AmazonEngine : public AccessEngine {
private:
	Common::Point _tilePos;
	byte _tileData[1455];
	Common::Array<CellIdent> _chapterCells;

	/**
	 * Do the game introduction
	 */
	void doIntroduction();

	/**
	 * Do title sequence
	 */
	void doTitle();

	/**
	 * Do tent scene of introduction
	 */
	void doTent();

	/**
	 * Setup variables for the game
	 */
	void setupGame();

	void initVariables();
	void calcIQ();
	void helpTitle();
	void drawHelpText(const Common::String &msg);
	void doCredit();
	void scrollTitle();
	void loadEstablish(int estabIndex);
	void doEstablish(int screenId, int estabIndex);

protected:
	/**
	 * Play the game
	 */
	virtual void playGame();

	/**
	* Synchronize savegame data
	*/
	virtual void synchronize(Common::Serializer &s);
public:
	InactivePlayer _inactive;
	bool _charSegSwitch;
	bool _skipStart;

	RiverStruct *_topList;
	RiverStruct *_botList;
	int _canoeDir;

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
	int _canoeLane;
	int _canoeYPos;
	int _hitCount;
	int _saveRiver;
	int _hitSafe;
	int _chapter;
	int _riverIndex;
	int _rawInactiveX;
	int _rawInactiveY;
	int _inactiveYOff;
	int _esTabTable[100];

	// Other game specific fields
	Guard _guard;
	Plane _plane;
	int _hintLevel;
	int _updateChapter;
	int _oldTitleChapter;
	int _maxHits;
	int _oldScrollCol;
	bool _deathFlag;
	int _deathCount;
	int _deathType;
	byte *_mapPtr;
	int _canoeVXPos;
	int _canoeMoveCount;
	int _canoeFrame;

	Direction _antDirection;
	Direction _pitDirection;
	int _antCel;
	int _torchCel;
	int _pitCel;
	int _stabCel;
	Common::Point _antPos;
	bool _antDieFl;
	bool _antEatFl;
	bool _stabFl;
	Common::Point _pitPos;
	int _iqValue;
public:
	AmazonEngine(OSystem *syst, const AccessGameDescription *gameDesc);

	virtual ~AmazonEngine();

	/**
	* Free the inactive player data
	*/
	void freeInactivePlayer();

	void drawHelp();

	virtual void establish(int esatabIndex, int sub);

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
