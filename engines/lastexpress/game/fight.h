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

#ifndef LASTEXPRESS_FIGHT_H
#define LASTEXPRESS_FIGHT_H

/*
	Fight structure
	---------------
	uint32 {4}      - player struct
	uint32 {4}      - opponent struct
	uint32 {4}      - hasLost flag

	byte {1}        - isRunning

	Fight participant structure
	---------------------------
	uint32 {4}      - function pointer
	uint32 {4}      - pointer to fight structure
	uint32 {4}      - pointer to opponent (fight participant structure)
	uint32 {4}      - array of sequences
	uint32 {4}      - number of sequences
	uint32 {4}      - ??
	uint32 {4}      - ??
	uint32 {4}      - ??
	uint32 {4}      - ??
	uint32 {4}      - ??
	uint32 {4}      - ??
	uint32 {4}      - ??
	uint32 {4}      - ??
	uint16 {2}      - ??
	uint16 {2}      - ??    - only for opponent structure
	uint32 {4}      - ??    - only for opponent structure

*/

#include "lastexpress/shared.h"

#include "lastexpress/eventhandler.h"

#include "common/array.h"

namespace LastExpress {

class LastExpressEngine;
class Sequence;
class SequenceFrame;

//////////////////////////////////////////////////////////////////////////
// TODO : objectify!
class Fight : public EventHandler {
public:
	enum FightEndType {
		kFightEndWin  = 0,
		kFightEndLost = 1,
		kFightEndExit = 2
	};

	Fight(LastExpressEngine *engine);
	~Fight();

	FightEndType setup(FightType type);

	void eventMouse(const Common::Event &ev);
	void eventTick(const Common::Event &ev);

	void setStopped();
	void resetState() { _state = 0; }

private:
	enum FightSequenceType {
		kFightSequenceType0 = 0,
		kFightSequenceType1 = 1,
		kFightSequenceType2 = 2
	};

	enum FightAction {
		kFightAction1 = 1,
		kFightAction2 = 2,
		kFightAction3 = 3,
		kFightAction4 = 4,
		kFightAction5 = 5,
		kFightAction101 = 101,
		kFightActionResetFrame = 102,
		kFightAction103 = 103,
		kFightActionWin = 104,
		kFightActionLost = 105,
		kFightAction128 = 128,
		kFightAction129 = 129,
		kFightAction130 = 130,
		kFightAction131 = 131,
		kFightAction132 = 132
	};

	struct Fighter {
		Common::Functor2<Fighter *, FightAction, void> *handleAction;
		Common::Functor1<Fighter *, void> *update;
		Common::Functor2<Fighter const *, FightAction, bool> *canInteract;
		Fighter *opponent;
		Common::Array<Sequence *> sequences;
		uint32 sequenceIndex;
		Sequence *sequence;
		SequenceFrame *frame;
		uint32 frameIndex;
		uint32 field_24;
		FightAction action;
		uint32 sequenceIndex2;
		int32 countdown;  // countdown before loosing ?
		uint32 field_34;

		Fighter() {
			handleAction = NULL;
			update = NULL;
			canInteract = NULL;

			opponent = NULL;

			sequenceIndex = 0;
			sequence = NULL;
			frame = NULL;
			frameIndex = 0;

			field_24 = 0;

			action = kFightAction101;
			sequenceIndex2 = 0;

			countdown = 1;

			field_34 = 0;
		}
	};

	// Opponent struct
	struct Opponent : Fighter {
		int32 field_38;

		Opponent() : Fighter() {
			field_38 = 0;
		}
	};

	struct FightData {
		Fighter *player;
		Opponent *opponent;
		int32 index;

		Sequence *sequences[20];
		Common::String names[20];

		bool isRunning;

		FightData() {
			player = new Fighter();
			opponent = new Opponent();

			// Set opponents
			player->opponent = opponent;
			opponent->opponent = player;

			index = 0;

			isRunning = false;
		}
	};

	LastExpressEngine *_engine;
	FightData *_data;
	FightEndType _endType;
	int _state;

	bool _handleTimer;

	// Events
	void handleTick(const Common::Event &ev, bool unknown);

	// State
	void bailout(FightEndType type);


	// Drawing
	void setSequenceAndDraw(Fighter *fighter, uint32 sequenceIndex, FightSequenceType type) const;
	void draw(Fighter *fighter) const;

	// Cleanup
	void clearData();
	void clearSequences(Fighter *fighter) const;

	//////////////////////////////////////////////////////////////////////////
	// Loading
	void loadData(FightType type);

	// Shared
	void processFighter(Fighter *fighter);

	// Default functions
	void handleAction(Fighter *fighter, FightAction action);
	void update(Fighter *fighter);
	bool canInteract(Fighter const *fighter, FightAction = (FightAction)0);
	void updateOpponent(Fighter *fighter);

	// Milos
	void loadMilosPlayer();
	void loadMilosOpponent();
	void handleActionMilos(Fighter *fighter, FightAction action);
	void updateMilos(Fighter *fighter);
	bool canInteractMilos(Fighter const *fighter, FightAction action);
	void handleOpponentActionMilos(Fighter *fighter, FightAction action);
	void updateOpponentMilos(Fighter *fighter);

	// Anna
	void loadAnnaPlayer();
	void loadAnnaOpponent();
	void handleActionAnna(Fighter *fighter, FightAction action);
	void updateOpponentAnna(Fighter *fighter);

	// Ivo
	void loadIvoPlayer();
	void loadIvoOpponent();
	void handleActionIvo(Fighter *fighter, FightAction action);
	void updateIvo(Fighter *fighter);
	bool canInteractIvo(Fighter const *fighter, FightAction action);
	void handleOpponentActionIvo(Fighter *fighter, FightAction action);
	void updateOpponentIvo(Fighter *fighter);

	// Salko
	void loadSalkoPlayer();
	void loadSalkoOpponent();
	void handleActionSalko(Fighter *fighter, FightAction action);
	void updateSalko(Fighter *fighter);
	bool canInteractSalko(Fighter const *fighter, FightAction action);
	void handleOpponentActionSalko(Fighter *fighter, FightAction action);
	void updateOpponentSalko(Fighter *fighter);

	// Vesna
	void loadVesnaPlayer();
	void loadVesnaOpponent();
	void handleActionVesna(Fighter *fighter, FightAction action);
	void updateVesna(Fighter *fighter);
	bool canInteractVesna(Fighter const *fighter, FightAction action);
	void handleOpponentActionVesna(Fighter *fighter, FightAction action);
	void updateOpponentVesna(Fighter *fighter);
};

} // End of namespace LastExpress

#endif // LASTEXPRESS_FIGHT_H
