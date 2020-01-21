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

#ifndef ULTIMA_ULTIMA1_GAME_H
#define ULTIMA_ULTIMA1_GAME_H

#include "ultima/ultima1/spells/blink.h"
#include "ultima/ultima1/spells/create.h"
#include "ultima/ultima1/spells/destroy.h"
#include "ultima/ultima1/spells/kill.h"
#include "ultima/ultima1/spells/ladder_down.h"
#include "ultima/ultima1/spells/ladder_up.h"
#include "ultima/ultima1/spells/magic_missile.h"
#include "ultima/ultima1/spells/open_unlock.h"
#include "ultima/ultima1/spells/prayer.h"
#include "ultima/ultima1/spells/steal.h"
#include "ultima/shared/early/game.h"
#include "ultima/shared/gfx/visual_container.h"

namespace Ultima {
namespace Ultima1 {

enum VideoMode {
	VIDEOMODE_EGA = 0, VIDEOMODE_VGA = 1
};

enum CharacterClass {
	CLASS_FIGHTER = 0, CLASS_CLERIC = 1, CLASS_WIZARD = 0, CLASS_THIEF = 3
};

class GameResources;

class Ultima1Game : public Shared::Game {
	DECLARE_MESSAGE_MAP;
	enum QuestFlag { UNSTARTED = 0, IN_PROGRESS = -1, COMPLETED = 1 };
private:
	/**
	 * Takes care of final setup as the game starts
	 */
	void setup();
public:
	GameResources *_res;
	Shared::Gfx::VisualItem *_gameView;
	Shared::Gfx::VisualItem *_titleView;
	Shared::Gfx::VisualItem *_charGenView;
	uint _gems[4];
	QuestFlag _questFlags[9];

	Spells::Blink _spellBlink;
	Spells::Create _spellCreate;
	Spells::Destroy _spellDestroy;
	Spells::Kill _spellKill;
	Spells::LadderDown _spellLadderDown;
	Spells::LadderUp _spellLadderUp;
	Spells::MagicMissile _spellMagicMissile;
	Spells::Open _spellOpen;
	Spells::Prayer _spellPrayer;
	Spells::Steal _spellSteal;
	Spells::Unlock _spellUnlock;
public:
	CLASSDEF;
	Ultima1Game();
	virtual ~Ultima1Game();

	/**
	 * Handles loading and saving games
	 */
	virtual void synchronize(Common::Serializer &s) override;

	/**
	 * Returns true if the current video mode is VGA
	 */
	virtual bool isVGA() const { return _videoMode == VIDEOMODE_VGA; }

	/**
	 * Called when the game starts
	 */
	virtual void starting(bool isLoading) override;

	/**
	 * Returns true if the game can currently be saved
	 */
	virtual bool canSaveGameStateCurrently();

	/**
	 * Give some treasure
	 */
	void giveTreasure(int coins, int v2);

	/**
	 * Called when a quest is completed
	 */
	void questCompleted(uint questNum);
};

} // End of namespace Ultima1
} // End of namespace Ultima

#endif
