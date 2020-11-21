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

#ifndef XEEN_LOCATIONS_H
#define XEEN_LOCATIONS_H

#include "common/scummsys.h"
#include "common/str-array.h"
#include "xeen/dialogs/dialogs.h"
#include "xeen/dialogs/dialogs_message.h"
#include "xeen/party.h"

namespace Xeen {

enum LocationAction {
	BANK = 0, BLACKSMITH = 1, GUILD = 2, TAVERN = 3, TEMPLE = 4,
	TRAINING = 5, ARENA = 6, NO_ACTION = 7, REAPER = 8, GOLEM = 9,
	DWARF_MINE = 10, SPHINX = 11, PYRAMID = 12, DWARF_TOWN = 13
};

class XeenEngine;

namespace Locations {

class BaseLocation : public ButtonContainer {
protected:
	LocationAction _locationActionId;
	Common::Array<SpriteResource> _townSprites;
	SpriteResource _icons1, _icons2;
	int _townMaxId;
	const int &_ccNum;
	int _animFrame;
	Common::String _vocName, _songName;
	Common::Point _animPos;
	int _drawFrameIndex;
	uint _farewellTime;
	int _drawCtr1, _drawCtr2;
	bool _exitToUi;
protected:
	/**
	 * Draw the window
	 */
	void drawWindow();

	/**
	 * Generates the display text for the location, for a given character
	 */
	virtual Common::String createLocationText(Character &ch) { return ""; }

	/**
	 * Draw the visual background
	 */
	virtual void drawBackground();

	/**
	 * Handles options for the particular location
	 */
	virtual Character *doOptions(Character *c) { return c; }

	/**
	 * Handle any farewell
	 */
	virtual void farewell() {}
public:
	BaseLocation(LocationAction action);
	~BaseLocation() override;

	/**
	 * Show the town location
	 */
	virtual int show();

	/**
	 * Draws the animated parts
	 */
	void drawAnim(bool flag);

	/**
	 * Waits for a brief pause, checking for any key or mouse events
	 */
	int wait();
};

class BankLocation : public BaseLocation {
private:
	/**
	 * Handles deposits or withdrawls fro the bank
	 */
	void depositWithdrawl(PartyBank whereId);
protected:
	/**
	 * Generates the display text for the location, for a given character
	 */
	Common::String createLocationText(Character &ch) override;

	/**
	 * Draw the visual background
	 */
	void drawBackground() override;

	/**
	 * Handles options for the particular location
	 */
	Character *doOptions(Character *c) override;
public:
	BankLocation();
	~BankLocation() override {}
};

class BlacksmithLocation : public BaseLocation {
protected:
	/**
	* Generates the display text for the location, for a given character
	*/
	Common::String createLocationText(Character &ch) override;

	/**
	 * Handle any farewell
	 */
	void farewell() override;

	/**
	 * Handles options for the particular location
	 */
	Character *doOptions(Character *c) override;
public:
	BlacksmithLocation();
	~BlacksmithLocation() override {}
};

class GuildLocation : public BaseLocation {
protected:
	/**
	 * Generates the display text for the location, for a given character
	 */
	Common::String createLocationText(Character &ch) override;

	/**
	 * Handles options for the particular location
	 */
	Character *doOptions(Character *c) override;
public:
	GuildLocation();
	~GuildLocation() override {}
};

class TavernLocation : public BaseLocation {
private:
	int _v21;
	uint _v22;
	int _v23;
	int _v24;
protected:
	/**
	* Generates the display text for the location, for a given character
	*/
	Common::String createLocationText(Character &ch) override;

	/**
	 * Handle any farewell
	 */
	void farewell() override;

	/**
	 * Handles options for the particular location
	 */
	Character *doOptions(Character *c) override;
public:
	TavernLocation();
	~TavernLocation() override {}
};

class TempleLocation : public BaseLocation {
private:
	int _currentCharLevel;
	int _donation;
	int _healCost;
	int _uncurseCost;
	int _dayOfWeek;
	int _v10, _v11, _v12;
	int _v13, _v14;
	bool _blessed;
	int _v5, _v6;
protected:
	/**
	* Generates the display text for the location, for a given character
	*/
	Common::String createLocationText(Character &ch) override;

	/**
	 * Handles options for the particular location
	 */
	Character *doOptions(Character *c) override;
public:
	TempleLocation();
	~TempleLocation() override {}
};

class TrainingLocation : public BaseLocation {
private:
	int _charIndex;
	bool _charsTrained[MAX_ACTIVE_PARTY];
	uint _experienceToNextLevel;
protected:
	/**
	 * Computes the maximum training level allowed at this location
	 */
	int maxLevel() const;

	/**
	 * Generates the display text for the location, for a given character
	 */
	Common::String createLocationText(Character &ch) override;

	/**
	 * Handles options for the particular location
	 */
	Character *doOptions(Character *c) override;
public:
	TrainingLocation();
	~TrainingLocation() override {}
};

class ArenaLocation : public BaseLocation {
public:
	ArenaLocation();
	~ArenaLocation() override {}

	/**
	 * Show the town location
	 */
	int show() override;
};

class CutsceneLocation : public BaseLocation {
protected:
	Subtitles _subtitles;
	int _mazeId;
	Direction _mazeDir;
	Common::Point _mazePos;
	bool _keyFound;
protected:
	/**
	 * Sets the new location
	 */
	void setNewLocation();
public:
	CutsceneLocation(LocationAction action);
};

class ReaperCutscene : public CutsceneLocation {
private:
	/**
	 * Get the new location
	 */
	void getNewLocation();
public:
	ReaperCutscene();
	~ReaperCutscene() override {}

	/**
	 * Show the town location
	 */
	int show() override;
};

class GolemCutscene : public CutsceneLocation {
private:
	/**
	 * Get the new location
	 */
	void getNewLocation(); 
public:
	GolemCutscene();
	~GolemCutscene() override {}

	/**
	 * Show the town location
	 */
	int show() override;
};

class DwarfCutscene : public CutsceneLocation {
private:
	/**
	 * Get the new location
	 */
	void getNewLocation();
public:
	DwarfCutscene();
	~DwarfCutscene() override {}

	/**
	 * Show the town location
	 */
	int show() override;
};

class SphinxCutscene : public CutsceneLocation {
private:
	/**
	 * Get the new location
	 */
	void getNewLocation();
public:
	SphinxCutscene();
	~SphinxCutscene() override {}

	/**
	 * Show the town location
	 */
	int show() override;
};

class PyramidLocation : public BaseLocation {
public:
	PyramidLocation();
	~PyramidLocation() override {}

	/**
	 * Show the town location
	 */
	int show() override;
};

} // End of namespace Locations

class LocationMessage : public Locations::BaseLocation {
private:
	SpriteResource _iconSprites;

	LocationMessage() : Locations::BaseLocation(NO_ACTION) { setWaitBounds(); }

	bool execute(int portrait, const Common::String &name,
		const Common::String &text, int confirm);

	void loadButtons();
public:
	static bool show(int portrait, const Common::String &name,
		const Common::String &text, int confirm);
};

class LocationManager {
private:
	Locations::BaseLocation *_location;
public:
	LocationManager();

	/**
	 * Show a given location, and return any result
	 */
	int doAction(int actionId);

	/**
	 * Returns true if a town location (bank, blacksmith, etc.) is currently active
	 */
	bool isActive() const;

	/**
	 * Draws a currently active town location's animation
	 */
	void drawAnim(bool flag);

	/**
	 * Calls the waiting for any currently active town location
	 */
	int wait();
};

} // End of namespace Xeen

#endif /* XEEN_LOCATIONS_H */
