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
 * $URL$
 * $Id$
 *
 */

#include "lastexpress/game/fight.h"

#include "lastexpress/data/cursor.h"
#include "lastexpress/data/scene.h"
#include "lastexpress/data/sequence.h"

#include "lastexpress/game/entities.h"
#include "lastexpress/game/inventory.h"
#include "lastexpress/game/logic.h"
#include "lastexpress/game/object.h"
#include "lastexpress/game/scenes.h"
#include "lastexpress/game/sound.h"
#include "lastexpress/game/state.h"

#include "lastexpress/graphics.h"
#include "lastexpress/helpers.h"
#include "lastexpress/lastexpress.h"
#include "lastexpress/resource.h"

#include "common/func.h"

namespace LastExpress {

#define CALL_FUNCTION0(fighter, name) \
	(*fighter->name)(fighter)

#define CALL_FUNCTION1(fighter, name, a) \
	(*fighter->name)(fighter, a)

#define REGISTER_PLAYER_FUNCTIONS(name) \
	if (!_data) \
		error("Fight::load##namePlayer - invalid data!"); \
	_data->player->handleAction = new Common::Functor2Mem<Fighter *, FightAction, void, Fight>(this, &Fight::handleAction##name); \
	_data->player->update = new Common::Functor1Mem<Fighter *, void, Fight>(this, &Fight::update##name); \
	_data->player->canInteract = new Common::Functor2Mem<Fighter const *, FightAction, bool, Fight>(this, &Fight::canInteract##name);

#define REGISTER_OPPONENT_FUNCTIONS(name) \
	if (!_data) \
		error("Fight::load##nameOpponent - invalid data!"); \
	_data->opponent->handleAction = new Common::Functor2Mem<Fighter *, FightAction, void, Fight>(this, &Fight::handleOpponentAction##name); \
	_data->opponent->update = new Common::Functor1Mem<Fighter *, void, Fight>(this, &Fight::updateOpponent##name); \
	_data->opponent->canInteract = new Common::Functor2Mem<Fighter const *, FightAction, bool, Fight>(this, &Fight::canInteract);

#define CHECK_SEQUENCE2(fighter, value) \
	(fighter->frame->getInfo()->field_33 & value)

Fight::Fight(LastExpressEngine *engine) : _engine(engine), _data(NULL), _endType(kFightEndLost), _state(0), _handleTimer(false) {}

Fight::~Fight() {
	clearData();
	_data = NULL;

	// Zero passed pointers
	_engine = NULL;
}

//////////////////////////////////////////////////////////////////////////
// Events
//////////////////////////////////////////////////////////////////////////

void Fight::eventMouse(const Common::Event &ev) {
	if (!_data || _data->index)
		return;

	// TODO move all the egg handling to inventory functions

	getFlags()->mouseLeftClick = false;
	getFlags()->shouldRedraw = false;
	getFlags()->mouseRightClick = false;

	if (ev.mouse.x < 608 || ev.mouse.y < 448 || ev.mouse.x >= 640 || ev.mouse.x >= 480) {

		// Handle right button click
		if (ev.type == Common::EVENT_RBUTTONUP) {
			getSound()->removeFromQueue(kEntityTables0);
			setStopped();

			getGlobalTimer() ? _state = 0 : ++_state;

			getFlags()->mouseRightClick = true;
		}

		if (_handleTimer) {
			// Timer expired => show with full brightness
			if (!getGlobalTimer())
				getInventory()->drawEgg();

			_handleTimer = false;
		}

		// Check hotspots
		Scene *scene = getScenes()->get(getState()->scene);
		SceneHotspot *hotspot = NULL;

		if (!scene->checkHotSpot(ev.mouse, &hotspot)) {
			_engine->getCursor()->setStyle(kCursorNormal);
		} else {
			_engine->getCursor()->setStyle((CursorStyle)hotspot->cursor);

			// Call player function
			if (CALL_FUNCTION1(_data->player, canInteract, (FightAction)hotspot->action)) {
				if (ev.type == Common::EVENT_LBUTTONUP)
					CALL_FUNCTION1(_data->player, handleAction, (FightAction)hotspot->action);
			} else {
				_engine->getCursor()->setStyle(kCursorNormal);
			}
		}
	} else {
		// Handle clicks on menu icon

		if (!_handleTimer) {
			// Timer expired => show with full brightness
			if (!getGlobalTimer())
				getInventory()->drawEgg();

			_handleTimer = true;
		}

		// Stop fight if clicked
		if (ev.type == Common::EVENT_LBUTTONUP) {
			_handleTimer = false;
			getSound()->removeFromQueue(kEntityTables0);
			bailout(kFightEndExit);
		}

		// Reset timer on right click
		if (ev.type == Common::EVENT_RBUTTONUP) {
			if (getGlobalTimer()) {
				if (getSound()->isBuffered("TIMER"))
					getSound()->removeFromQueue("TIMER");

				setGlobalTimer(900);
			}
		}
	}

	getFlags()->shouldRedraw = true;
}

void Fight::eventTick(const Common::Event &ev) {
	handleTick(ev, true);
}

void Fight::handleTick(const Common::Event &ev, bool isProcessing) {
	// TODO move all the egg handling to inventory functions

	// Blink egg
	if (getGlobalTimer()) {
		warning("Fight::handleMouseMove - egg blinking not implemented!");
	}

	if (!_data || _data->index)
		return;

	SceneHotspot *hotspot = NULL;
	if (!getScenes()->get(getState()->scene)->checkHotSpot(ev.mouse, &hotspot) || !CALL_FUNCTION1(_data->player, canInteract, (FightAction)hotspot->action)) {
		_engine->getCursor()->setStyle(kCursorNormal);
	} else {
		_engine->getCursor()->setStyle((CursorStyle)hotspot->cursor);
	}

	CALL_FUNCTION0(_data->player, update);
	CALL_FUNCTION0(_data->opponent, update);

	// Draw sequences
	if (!_data->isRunning)
		return;

	if (isProcessing)
		getScenes()->drawFrames(true);

	if (_data->index) {
		// Set next sequence name index
		_data->index--;
		_data->sequences[_data->index] = loadSequence(_data->names[_data->index]);
	}
}

//////////////////////////////////////////////////////////////////////////
// Setup
//////////////////////////////////////////////////////////////////////////

Fight::FightEndType Fight::setup(FightType type) {
	if (_data)
		error("Fight::setup - calling fight setup again while a fight is already in progress!");

	//////////////////////////////////////////////////////////////////////////
	// Prepare UI & state
	if (_state >= 5 && (type == kFightSalko || type == kFightVesna)) {
		_state = 0;
		return kFightEndWin;
	}

	getInventory()->showHourGlass();
	// TODO events function
	getFlags()->flag_0 = false;
	getFlags()->mouseRightClick = false;
	getEntities()->reset();

	// Compute scene to use
	SceneIndex sceneIndex;
	switch(type) {
	default:
		sceneIndex = kSceneFightDefault;
		break;

	case kFightMilos:
		sceneIndex = (getObjects()->get(kObjectCompartment1).location2 < kObjectLocation3) ? kSceneFightMilos : kSceneFightMilosBedOpened;
		break;

	case kFightAnna:
		sceneIndex = kSceneFightAnna;
		break;

	case kFightIvo:
		sceneIndex = kSceneFightIvo;
		break;

	case kFightSalko:
		sceneIndex = kSceneFightSalko;
		break;

	case kFightVesna:
		sceneIndex = kSceneFightVesna;
		break;
	}

	if (getFlags()->shouldRedraw) {
		getFlags()->shouldRedraw = false;
		askForRedraw();
		//redrawScreen();
	}

	// Load the scene object
	Scene *scene = getScenes()->get(sceneIndex);

	// Update game entities and state
	getEntityData(kEntityPlayer)->entityPosition = scene->entityPosition;
	getEntityData(kEntityPlayer)->location = scene->location;

	getState()->scene = sceneIndex;

	getFlags()->flag_3 = true;

	// Draw the scene
	_engine->getGraphicsManager()->draw(scene, GraphicsManager::kBackgroundC);
	// FIXME move to start of fight?
	askForRedraw();
	redrawScreen();

	//////////////////////////////////////////////////////////////////////////
	// Setup the fight
	_data = new FightData;
	loadData(type);

	// Show opponents & egg button
	Common::Event emptyEvent;
	handleTick(emptyEvent, false);
	getInventory()->drawEgg();

	// Start fight
	_endType = kFightEndLost;
	while (_data->isRunning) {
		if (_engine->handleEvents())
			continue;

		getSound()->updateQueue();
	}

	// Cleanup after fight is over
	clearData();

	return _endType;
}

//////////////////////////////////////////////////////////////////////////
// Status
//////////////////////////////////////////////////////////////////////////

void Fight::setStopped() {
	if (_data)
		_data->isRunning = false;
}

void Fight::bailout(FightEndType type) {
	_state = 0;
	_endType = type;
	setStopped();
}

//////////////////////////////////////////////////////////////////////////
// Cleanup
//////////////////////////////////////////////////////////////////////////

void Fight::clearData() {
	if (!_data)
		return;

	// Clear data
	clearSequences(_data->player);
	clearSequences(_data->opponent);

	SAFE_DELETE(_data->player);
	SAFE_DELETE(_data->opponent);

	SAFE_DELETE(_data);

	_engine->restoreEventHandlers();
}

void Fight::clearSequences(Fighter *combatant) const {
	if (!combatant)
		return;

	// The original game resets the function pointers to default values, just before deleting the struct
	getScenes()->removeAndRedraw(&combatant->frame, false);

	// Free sequences
	for (int i = 0; i < (int)combatant->sequences.size(); i++)
		SAFE_DELETE(combatant->sequences[i]);
}

//////////////////////////////////////////////////////////////////////////
// Drawing
//////////////////////////////////////////////////////////////////////////

void Fight::setSequenceAndDraw(Fighter *combatant, uint32 sequenceIndex, FightSequenceType type) const {
	if (combatant->sequences.size() < sequenceIndex)
		return;

	switch (type) {
	default:
		break;

	case kFightSequenceType0:
		if (combatant->sequenceIndex)
			return;

		combatant->sequence = combatant->sequences[sequenceIndex];
		combatant->sequenceIndex = sequenceIndex;
		draw(combatant);
		break;

	case kFightSequenceType1:
		combatant->sequence = combatant->sequences[sequenceIndex];
		combatant->sequenceIndex = sequenceIndex;
		combatant->sequenceIndex2 = 0;
		draw(combatant);
		break;

	case kFightSequenceType2:
		combatant->sequenceIndex2 = sequenceIndex;
		break;
	}
}

void Fight::draw(Fighter *combatant) const {
	getScenes()->removeAndRedraw(&combatant->frame, false);

	combatant->frameIndex = 0;
	combatant->field_24 = 0;
}

//////////////////////////////////////////////////////////////////////////
// Loading
//////////////////////////////////////////////////////////////////////////

void Fight::loadData(FightType type) {
	if (!_data)
		error("Fight::loadData - invalid data!");

	switch (type) {
	default:
		break;

	case kFightMilos:
		loadMilosPlayer();
		loadMilosOpponent();
		break;

	case kFightAnna:
		loadAnnaPlayer();
		loadAnnaOpponent();
		break;

	case kFightIvo:
		loadIvoPlayer();
		loadIvoOpponent();
		break;

	case kFightSalko:
		loadSalkoPlayer();
		loadSalkoOpponent();
		break;

	case kFightVesna:
		loadVesnaPlayer();
		loadVesnaOpponent();
		break;
	}

	if (!_data->player || !_data->opponent)
		error("Fight::loadData - error loading fight data (type=%d)", type);

	//////////////////////////////////////////////////////////////////////////
	// Start running the fight
	_data->isRunning = true;

	if (_state < 5) {
		setSequenceAndDraw(_data->player, 0, kFightSequenceType0);
		setSequenceAndDraw(_data->opponent, 0, kFightSequenceType0);
		goto end_load;
	}

	switch(type) {
	default:
		break;

	case kFightMilos:
		_data->opponent->countdown = 1;
		setSequenceAndDraw(_data->player, 4, kFightSequenceType0);
		setSequenceAndDraw(_data->opponent, 0, kFightSequenceType0);
		break;

	case kFightIvo:
		_data->opponent->countdown = 1;
		setSequenceAndDraw(_data->player, 3, kFightSequenceType0);
		setSequenceAndDraw(_data->opponent, 6, kFightSequenceType0);
		break;

	case kFightVesna:
		_data->opponent->countdown = 1;
		setSequenceAndDraw(_data->player, 0, kFightSequenceType0);
		setSequenceAndDraw(_data->player, 3, kFightSequenceType2);
		setSequenceAndDraw(_data->opponent, 5, kFightSequenceType0);
		break;
	}

end_load:
	// Setup event handlers
	_engine->backupEventHandlers();
	SET_EVENT_HANDLERS(Fight, this);
}

//////////////////////////////////////////////////////////////////////////
// Shared
//////////////////////////////////////////////////////////////////////////
void Fight::processFighter(Fighter *fighter) {
	if (!_data)
		error("Fight::processFighter - invalid data!");

	if (!fighter->sequence) {
		if (fighter->frame) {
			getScenes()->removeFromQueue(fighter->frame);
			getScenes()->setCoordinates(fighter->frame);
		}
		SAFE_DELETE(fighter->frame);
		return;
	}

	if (fighter->sequence->count() <= fighter->frameIndex) {
		switch(fighter->action) {
		default:
			break;

		case kFightAction101:
			setSequenceAndDraw(fighter, fighter->sequenceIndex2, kFightSequenceType1);
			fighter->sequenceIndex2 = 0;
			break;

		case kFightActionResetFrame:
			fighter->frameIndex = 0;
			break;

		case kFightAction103:
			setSequenceAndDraw(fighter, 0, kFightSequenceType1);
			CALL_FUNCTION1(fighter, handleAction, kFightAction101);
			setSequenceAndDraw(fighter->opponent, 0, kFightSequenceType1);
			CALL_FUNCTION1(fighter->opponent, handleAction, kFightAction101);
			CALL_FUNCTION0(fighter->opponent, update);
			break;

		case kFightActionWin:
			bailout(kFightEndWin);
			break;

		case kFightActionLost:
			bailout(kFightEndLost);
			break;
		}
	}

	if (_data->isRunning) {

		// Get the current sequence frame
		SequenceFrame *frame = new SequenceFrame(fighter->sequence, (uint16)fighter->frameIndex);
		frame->getInfo()->location = 1;

		if (fighter->frame == frame) {
			delete frame;
			return;
		}

		getSound()->playFightSound(frame->getInfo()->soundAction, frame->getInfo()->field_31);

		// Add current frame to queue and advance
		getScenes()->addToQueue(frame);
		fighter->frameIndex++;

		if (fighter->frame) {
			getScenes()->removeFromQueue(fighter->frame);

			if (!frame->getInfo()->field_2E)
				getScenes()->setCoordinates(fighter->frame);
		}

		// Replace by new frame
		delete fighter->frame;
		fighter->frame = frame;
	}
}

void Fight::handleAction(Fighter *fighter, FightAction action) {
	switch (action) {
	default:
		return;

	case kFightAction101:
		break;

	case kFightActionResetFrame:
		fighter->countdown--;
		break;

	case kFightAction103:
		CALL_FUNCTION1(fighter->opponent, handleAction, kFightActionResetFrame);
		break;

	case kFightActionWin:
		_endType = kFightEndWin;
		CALL_FUNCTION1(fighter->opponent, handleAction, kFightActionResetFrame);
		break;

	case kFightActionLost:
		_endType = kFightEndLost;
		CALL_FUNCTION1(fighter->opponent, handleAction, kFightActionResetFrame);
		break;
	}

	// Update action
	fighter->action = action;
}

bool Fight::canInteract(Fighter const *fighter, FightAction /*= (FightAction)0*/ ) {
	return (fighter->action == kFightAction101 && !fighter->sequenceIndex);
}

void Fight::update(Fighter *fighter) {

	processFighter(fighter);

	if (fighter->frame)
		fighter->frame->getInfo()->location = (fighter->action == kFightActionResetFrame ? 2 : 0);
}

void Fight::updateOpponent(Fighter *fighter) {

	// This is an opponent struct!
	Opponent *opponent = (Opponent *)fighter;

	processFighter(opponent);

	if (opponent->field_38 && !opponent->sequenceIndex)
		opponent->field_38--;

	if (fighter->frame)
		fighter->frame->getInfo()->location = 1;
}

//////////////////////////////////////////////////////////////////////////
// Milos
//////////////////////////////////////////////////////////////////////////

void Fight::loadMilosPlayer() {
	REGISTER_PLAYER_FUNCTIONS(Milos)

	_data->player->sequences.push_back(loadSequence("2001cr.seq"));
	_data->player->sequences.push_back(loadSequence("2001cdl.seq"));
	_data->player->sequences.push_back(loadSequence("2001cdr.seq"));
	_data->player->sequences.push_back(loadSequence("2001cdm.seq"));
	_data->player->sequences.push_back(loadSequence("2001csgr.seq"));
	_data->player->sequences.push_back(loadSequence("2001csgl.seq"));
	_data->player->sequences.push_back(loadSequence("2001dbk.seq"));
}

void Fight::loadMilosOpponent() {
	REGISTER_OPPONENT_FUNCTIONS(Milos)

	_data->opponent->sequences.push_back(loadSequence("2001or.seq"));
	_data->opponent->sequences.push_back(loadSequence("2001oal.seq"));
	_data->opponent->sequences.push_back(loadSequence("2001oam.seq"));
	_data->opponent->sequences.push_back(loadSequence("2001okl.seq"));
	_data->opponent->sequences.push_back(loadSequence("2001okm.seq"));
	_data->opponent->sequences.push_back(loadSequence("2001dbk.seq"));
	_data->opponent->sequences.push_back(loadSequence("2001wbk.seq"));

	getSound()->playSound(kEntityTables0, "MUS027", SoundManager::kFlagDefault);

	_data->opponent->field_38 = 35;
}

void Fight::handleActionMilos(Fighter *fighter, FightAction action) {
	switch (action) {
	default:
		handleAction(fighter, action);
		return;

	case kFightAction1:
		if (fighter->sequenceIndex != 1 || CHECK_SEQUENCE2(fighter, 4)) {
			setSequenceAndDraw(fighter, 6, kFightSequenceType1);
			setSequenceAndDraw(fighter->opponent, 3, kFightSequenceType1);

			CALL_FUNCTION1(fighter->opponent, handleAction, kFightAction103);
			CALL_FUNCTION0(fighter, update);
		} else {
			fighter->field_34++;
		}
		break;

	case kFightAction2:
		if ((fighter->sequenceIndex != 2 && fighter->sequenceIndex != 3) || CHECK_SEQUENCE2(fighter, 4)) {
			setSequenceAndDraw(fighter, 6, kFightSequenceType1);
			setSequenceAndDraw(fighter->opponent, 4, kFightSequenceType1);

			CALL_FUNCTION1(fighter->opponent, handleAction, kFightAction103);
			CALL_FUNCTION0(fighter, update);
		} else {
			fighter->field_34++;
		}
		break;

	case kFightAction128:
		if (fighter->sequenceIndex != 1 || CHECK_SEQUENCE2(fighter, 4) || fighter->opponent->sequenceIndex != 1) {
			switch (fighter->opponent->sequenceIndex) {
			default:
				setSequenceAndDraw(fighter, rnd(3) + 1, kFightSequenceType0);
				break;

			case 1:
				setSequenceAndDraw(fighter, 1, kFightSequenceType0);
				break;

			case 2:
				setSequenceAndDraw(fighter, 3, kFightSequenceType0);
				break;
			}
		} else {
			setSequenceAndDraw(fighter, 4, kFightSequenceType1);
			CALL_FUNCTION0(fighter, update);
		}
		break;
	}
}

void Fight::updateMilos(Fighter *fighter) {
	if (fighter->frame && CHECK_SEQUENCE2(fighter, 2)) {

		// Draw sequences
		if (fighter->opponent->countdown <= 0) {
			setSequenceAndDraw(fighter, 5, kFightSequenceType1);
			setSequenceAndDraw(fighter->opponent, 6, kFightSequenceType1);

			getSound()->removeFromQueue(kEntityTables0);
			getSound()->playSound(kEntityTrain, "MUS029", SoundManager::kFlagDefault);

			CALL_FUNCTION1(fighter, handleAction, kFightActionWin);
		}

		if (fighter->sequenceIndex == 4) {
			CALL_FUNCTION1(fighter->opponent, handleAction, kFightAction4);
			_endType = kFightEndLost;
		}
	}

	update(fighter);
}

bool Fight::canInteractMilos(Fighter const *fighter, FightAction action) {
	if (!_data)
		error("Fight::canInteractMilos - invalid data!");

	if (action != kFightAction128
	 || _data->player->sequenceIndex != 1
	 || !fighter->frame
	 || CHECK_SEQUENCE2(fighter, 4)
	 || fighter->opponent->sequenceIndex != 1) {
		 return canInteract(fighter);
	}

	_engine->getCursor()->setStyle(kCursorHand);

	return true;
}

void Fight::handleOpponentActionMilos(Fighter *fighter, FightAction action) {
	if (action == kFightAction4) {
		setSequenceAndDraw(fighter, 5, kFightSequenceType1);
		CALL_FUNCTION1(fighter->opponent, handleAction, kFightAction103);
	} else {
		if (action != kFightAction131)
			handleAction(fighter, action);
	}
}

void Fight::updateOpponentMilos(Fighter *fighter) {
	// This is an opponent struct!
	Opponent *opponent = (Opponent *)fighter;

	if (!opponent->field_38 && CALL_FUNCTION1(opponent, canInteract, kFightAction1) && !opponent->sequenceIndex2) {

		if (opponent->opponent->field_34 >= 2) {
			switch (rnd(5)) {
			default:
				break;

			case 0:
				setSequenceAndDraw(opponent, 1, kFightSequenceType0);
				break;

			case 1:
				setSequenceAndDraw(opponent, 2, kFightSequenceType0);
				break;

			case 2:
				setSequenceAndDraw(opponent, 2, kFightSequenceType0);
				setSequenceAndDraw(opponent, 2, kFightSequenceType1);
				break;

			case 3:
				setSequenceAndDraw(opponent, 1, kFightSequenceType0);
				setSequenceAndDraw(opponent, 2, kFightSequenceType2);
				break;

			case 4:
				setSequenceAndDraw(opponent, 1, kFightSequenceType0);
				setSequenceAndDraw(opponent, 1, kFightSequenceType2);
				break;
			}
		} else {
			setSequenceAndDraw(opponent, 2, kFightSequenceType0);
		}

		// Update field_38
		if (opponent->opponent->field_34 < 5)
			opponent->field_38 = 6 * (5 - opponent->opponent->field_34);
		else
			opponent->field_38 = 0;
	}

	if (opponent->frame && CHECK_SEQUENCE2(opponent, 2)) {
		if (opponent->sequenceIndex == 1 || opponent->sequenceIndex == 2)
			CALL_FUNCTION1(opponent->opponent, handleAction, (FightAction)opponent->sequenceIndex);

		if (opponent->opponent->countdown <= 0) {
			getSound()->removeFromQueue(kEntityTables0);
			CALL_FUNCTION1(opponent, handleAction, kFightActionLost);
		}
	}

	updateOpponent(opponent);
}

//////////////////////////////////////////////////////////////////////////
// Anna
//////////////////////////////////////////////////////////////////////////

void Fight::loadAnnaPlayer() {
	if (!_data)
		error("Fight::loadAnnaPlayer - invalid data!");

	// Special case: we are using some shared functions directly
	_data->player->handleAction = new Common::Functor2Mem<Fighter *, FightAction, void, Fight>(this, &Fight::handleActionAnna);
	_data->player->update = new Common::Functor1Mem<Fighter *, void, Fight>(this, &Fight::update);
	_data->player->canInteract = new Common::Functor2Mem<Fighter const *, FightAction, bool, Fight>(this, &Fight::canInteract);

	_data->player->sequences.push_back(loadSequence("2002cr.seq"));
	_data->player->sequences.push_back(loadSequence("2002cdl.seq"));
	_data->player->sequences.push_back(loadSequence("2002cdr.seq"));
	_data->player->sequences.push_back(loadSequence("2002cdm.seq"));
	_data->player->sequences.push_back(loadSequence("2002lbk.seq"));
}

void Fight::loadAnnaOpponent() {
	if (!_data)
		error("Fight::loadAnnaOpponent - invalid data!");

	// Special case: we are using some shared functions directly
	_data->opponent->handleAction = new Common::Functor2Mem<Fighter *, FightAction, void, Fight>(this, &Fight::handleAction);
	_data->opponent->update = new Common::Functor1Mem<Fighter *, void, Fight>(this, &Fight::updateOpponentAnna);
	_data->opponent->canInteract = new Common::Functor2Mem<Fighter const *, FightAction, bool, Fight>(this, &Fight::canInteract);

	_data->opponent->sequences.push_back(loadSequence("2002or.seq"));
	_data->opponent->sequences.push_back(loadSequence("2002oal.seq"));
	_data->opponent->sequences.push_back(loadSequence("2002oam.seq"));
	_data->opponent->sequences.push_back(loadSequence("2002oar.seq"));
	_data->opponent->sequences.push_back(loadSequence("2002okr.seq"));
	_data->opponent->sequences.push_back(loadSequence("2002okml.seq"));
	_data->opponent->sequences.push_back(loadSequence("2002okm.seq"));

	getSound()->playSound(kEntityTables0, "MUS030", SoundManager::kFlagDefault);

	_data->opponent->field_38 = 30;
}

void Fight::handleActionAnna(Fighter *fighter, FightAction action) {
	switch (action) {
	default:
		handleAction(fighter, action);
		return;

	case kFightAction1:
		if ((fighter->sequenceIndex != 1 && fighter->sequenceIndex != 3) || CHECK_SEQUENCE2(fighter, 4)) {
			setSequenceAndDraw(fighter, 4, kFightSequenceType1);
			setSequenceAndDraw(fighter->opponent, 4, kFightSequenceType1);

			CALL_FUNCTION1(fighter->opponent, handleAction, kFightAction103);
			CALL_FUNCTION0(fighter, update);
		} else {
			fighter->field_34++;
		}
		break;

	case kFightAction2:
		if ((fighter->sequenceIndex != 2 && fighter->sequenceIndex != 3) || CHECK_SEQUENCE2(fighter, 4)) {
			setSequenceAndDraw(fighter, 4, kFightSequenceType1);
			setSequenceAndDraw(fighter->opponent, 5, kFightSequenceType1);

			CALL_FUNCTION1(fighter->opponent, handleAction, kFightAction103);
			CALL_FUNCTION0(fighter, update);
		} else {
			fighter->field_34++;
		}
		break;

	case kFightAction3:
		if ((fighter->sequenceIndex != 2 && fighter->sequenceIndex != 1) || CHECK_SEQUENCE2(fighter, 4)) {
			setSequenceAndDraw(fighter, 4, kFightSequenceType1);
			setSequenceAndDraw(fighter->opponent, 6, kFightSequenceType1);

			CALL_FUNCTION1(fighter->opponent, handleAction, kFightAction103);
			CALL_FUNCTION0(fighter, update);
		} else {
			fighter->field_34++;
		}
		break;

	case kFightAction128:
		switch (fighter->opponent->sequenceIndex) {
		default:
			setSequenceAndDraw(fighter, 3, kFightSequenceType0);
			break;

		case 1:
			setSequenceAndDraw(fighter, 1, kFightSequenceType0);
			break;

		case 2:
			setSequenceAndDraw(fighter, 3, kFightSequenceType0);
			break;

		case 3:
			setSequenceAndDraw(fighter, 2, kFightSequenceType0);
			break;
		}
		break;
	}

	if (fighter->field_34 > 4) {
		getSound()->removeFromQueue(kEntityTables0);
		bailout(kFightEndWin);
	}
}

void Fight::updateOpponentAnna(Fighter *fighter) {
	// This is an opponent struct!
	Opponent *opponent = (Opponent *)fighter;

	if (!opponent->field_38 && CALL_FUNCTION1(opponent, canInteract, kFightAction1) && !opponent->sequenceIndex2) {

		if (opponent->opponent->field_34 >= 2) {
			switch (rnd(6)) {
			default:
				break;

			case 0:
				setSequenceAndDraw(opponent, 1, kFightSequenceType0);
				break;

			case 1:
				setSequenceAndDraw(opponent, 2, kFightSequenceType0);
				break;

			case 2:
				setSequenceAndDraw(opponent, 3, kFightSequenceType0);
				break;

			case 3:
				setSequenceAndDraw(opponent, 3, kFightSequenceType0);
				setSequenceAndDraw(opponent, 2, kFightSequenceType2);
				break;

			case 4:
				setSequenceAndDraw(opponent, 1, kFightSequenceType0);
				setSequenceAndDraw(opponent, 2, kFightSequenceType2);
				break;

			case 5:
				setSequenceAndDraw(opponent, 3, kFightSequenceType0);
				setSequenceAndDraw(opponent, 2, kFightSequenceType2);
				break;
			}
		}

		// Update field_38
		opponent->field_38 = (int32)rnd(15);
	}

	if (opponent->frame && CHECK_SEQUENCE2(opponent, 2)) {
		if (opponent->sequenceIndex == 1 || opponent->sequenceIndex == 2 || opponent->sequenceIndex == 3)
			CALL_FUNCTION1(opponent->opponent, handleAction, (FightAction)opponent->sequenceIndex);

		if (opponent->opponent->countdown <= 0) {
			getSound()->removeFromQueue(kEntityTables0);
			CALL_FUNCTION1(opponent, handleAction, kFightActionLost);
		}
	}

	updateOpponent(opponent);
}

//////////////////////////////////////////////////////////////////////////
// Ivo
//////////////////////////////////////////////////////////////////////////

void Fight::loadIvoPlayer() {
	REGISTER_PLAYER_FUNCTIONS(Ivo)

	_data->player->sequences.push_back(loadSequence("2003cr.seq"));
	_data->player->sequences.push_back(loadSequence("2003car.seq"));
	_data->player->sequences.push_back(loadSequence("2003cal.seq"));
	_data->player->sequences.push_back(loadSequence("2003cdr.seq"));
	_data->player->sequences.push_back(loadSequence("2003cdm.seq"));
	_data->player->sequences.push_back(loadSequence("2003chr.seq"));
	_data->player->sequences.push_back(loadSequence("2003chl.seq"));
	_data->player->sequences.push_back(loadSequence("2003ckr.seq"));
	_data->player->sequences.push_back(loadSequence("2003lbk.seq"));
	_data->player->sequences.push_back(loadSequence("2003fbk.seq"));

	_data->player->countdown = 5;
}

void Fight::loadIvoOpponent() {
	REGISTER_OPPONENT_FUNCTIONS(Ivo)

	_data->opponent->sequences.push_back(loadSequence("2003or.seq"));
	_data->opponent->sequences.push_back(loadSequence("2003oal.seq"));
	_data->opponent->sequences.push_back(loadSequence("2003oar.seq"));
	_data->opponent->sequences.push_back(loadSequence("2003odm.seq"));
	_data->opponent->sequences.push_back(loadSequence("2003okl.seq"));
	_data->opponent->sequences.push_back(loadSequence("2003okj.seq"));
	_data->opponent->sequences.push_back(loadSequence("blank.seq"));
	_data->opponent->sequences.push_back(loadSequence("csdr.seq"));
	_data->opponent->sequences.push_back(loadSequence("2003l.seq"));

	getSound()->playSound(kEntityTables0, "MUS032", SoundManager::kFlagDefault);

	_data->opponent->countdown = 5;
	_data->opponent->field_38 = 15;
}

void Fight::handleActionIvo(Fighter *fighter, FightAction action) {
	switch (action) {
	default:
		handleAction(fighter, action);
		return;

	case kFightAction1:
		if (fighter->sequenceIndex != 1 || CHECK_SEQUENCE2(fighter, 4)) {
			setSequenceAndDraw(fighter, 7, kFightSequenceType1);
			setSequenceAndDraw(fighter->opponent, 4, kFightSequenceType1);

			CALL_FUNCTION1(fighter->opponent, handleAction, kFightAction103);
			CALL_FUNCTION0(fighter, update);
		}
		break;

	case kFightAction2:
		if ((fighter->sequenceIndex != 2 && fighter->sequenceIndex != 3) || CHECK_SEQUENCE2(fighter, 4)) {
			setSequenceAndDraw(fighter, 7, kFightSequenceType1);
			setSequenceAndDraw(fighter->opponent, 5, kFightSequenceType1);

			CALL_FUNCTION1(fighter->opponent, handleAction, kFightAction103);
			CALL_FUNCTION0(fighter, update);
		}
		break;

	case kFightAction128:
		switch (fighter->opponent->sequenceIndex) {
		default:
		case 1:
			setSequenceAndDraw(fighter, 1, kFightSequenceType0);
			break;

		case 2:
			setSequenceAndDraw(fighter, 2, kFightSequenceType0);
			break;
		}
		break;

	case kFightAction129:
		setSequenceAndDraw(fighter, (fighter->opponent->countdown > 1) ? 4 : 3, fighter->sequenceIndex ? kFightSequenceType2 : kFightSequenceType0);
		break;

	case kFightAction130:
		setSequenceAndDraw(fighter, 3, fighter->sequenceIndex ? kFightSequenceType2 : kFightSequenceType0);
		break;
	}
}

void Fight::updateIvo(Fighter *fighter) {

	if ((fighter->sequenceIndex == 3 || fighter->sequenceIndex == 4) && !fighter->frameIndex)
		CALL_FUNCTION1(fighter->opponent, handleAction, kFightAction131);

	if (fighter->frame && CHECK_SEQUENCE2(fighter, 2)) {

		// Draw sequences
		if (fighter->opponent->countdown <= 0) {
			setSequenceAndDraw(fighter, 9, kFightSequenceType1);
			setSequenceAndDraw(fighter->opponent, 8, kFightSequenceType1);
			getSound()->removeFromQueue(kEntityTables0);

			CALL_FUNCTION1(fighter, handleAction, kFightActionWin);
			return;
		}

		if (fighter->sequenceIndex == 3 || fighter->sequenceIndex == 4)
			CALL_FUNCTION1(fighter->opponent, handleAction, (FightAction)fighter->sequenceIndex);
	}

	update(fighter);
}

bool Fight::canInteractIvo(Fighter const *fighter, FightAction action) {
	if (action == kFightAction129 || action == kFightAction130)
		return (fighter->sequenceIndex >= 8);

	return canInteract(fighter);
}

void Fight::handleOpponentActionIvo(Fighter *fighter, FightAction action) {
	// This is an opponent struct!
	Opponent *opponent = (Opponent *)fighter;

	switch (action) {
	default:
		handleAction(fighter, action);
		break;

	case kFightAction3:
		if ((opponent->sequenceIndex != 1 && opponent->sequenceIndex != 3) || CHECK_SEQUENCE2(opponent, 4)) {
			setSequenceAndDraw(opponent, 6, kFightSequenceType1);
			setSequenceAndDraw(opponent->opponent, 6, kFightSequenceType1);
			CALL_FUNCTION1(opponent->opponent, handleAction, kFightAction103);
		}
		break;

	case kFightAction4:
		if ((opponent->sequenceIndex != 2 && opponent->sequenceIndex != 3) || CHECK_SEQUENCE2(opponent, 4)) {
			setSequenceAndDraw(opponent, 6, kFightSequenceType1);
			setSequenceAndDraw(opponent->opponent, 5, kFightSequenceType1);
			CALL_FUNCTION1(opponent->opponent, handleAction, kFightAction103);
		}
		break;

	case kFightAction131:
		if (opponent->sequenceIndex)
			break;

		if (rnd(100) <= (unsigned int)(opponent->countdown > 2 ? 60 : 75)) {
			setSequenceAndDraw(opponent, 3 , kFightSequenceType1);
			if (opponent->opponent->sequenceIndex == 4)
				setSequenceAndDraw(opponent, 2, kFightSequenceType2);
		}
		break;
	}
}

void Fight::updateOpponentIvo(Fighter *fighter) {
	// This is an opponent struct!
	Opponent *opponent = (Opponent *)fighter;

	if (!opponent->field_38 && CALL_FUNCTION1(opponent, canInteract, kFightAction1) && !opponent->sequenceIndex2) {

		if (opponent->opponent->field_34 >= 2) {
			switch (rnd(5)) {
			default:
				break;

			case 0:
				setSequenceAndDraw(opponent, 1, kFightSequenceType0);
				break;

			case 1:
				setSequenceAndDraw(opponent, 2, kFightSequenceType0);
				break;

			case 2:
				setSequenceAndDraw(opponent, 1, kFightSequenceType0);
				setSequenceAndDraw(opponent, 2, kFightSequenceType2);
				break;

			case 3:
				setSequenceAndDraw(opponent, 0, kFightSequenceType2);
				setSequenceAndDraw(opponent, 1, kFightSequenceType2);
				break;

			case 4:
				setSequenceAndDraw(opponent, 0, kFightSequenceType1);
				setSequenceAndDraw(opponent, 1, kFightSequenceType2);
				break;
			}
		}

		// Update field_38
		opponent->field_38 = 3 * opponent->countdown + (int32)rnd(10);
	}

	if (opponent->frame && CHECK_SEQUENCE2(opponent, 2)) {

		if (opponent->opponent->countdown <= 0) {
			setSequenceAndDraw(opponent, 7, kFightSequenceType1);
			setSequenceAndDraw(opponent->opponent, 8, kFightSequenceType1);
			getSound()->removeFromQueue(kEntityTables0);

			CALL_FUNCTION1(opponent->opponent, handleAction, kFightActionWin);

			return;
		}

		if (opponent->sequenceIndex == 1 || opponent->sequenceIndex == 2)
			CALL_FUNCTION1(opponent->opponent, handleAction, (FightAction)opponent->sequenceIndex);
	}

	updateOpponent(opponent);
}

//////////////////////////////////////////////////////////////////////////
// Salko
//////////////////////////////////////////////////////////////////////////

void Fight::loadSalkoPlayer() {
	REGISTER_PLAYER_FUNCTIONS(Salko)

	_data->player->sequences.push_back(loadSequence("2004cr.seq"));
	_data->player->sequences.push_back(loadSequence("2004cdr.seq"));
	_data->player->sequences.push_back(loadSequence("2004chj.seq"));
	_data->player->sequences.push_back(loadSequence("2004bk.seq"));

	_data->player->countdown = 2;
}

void Fight::loadSalkoOpponent() {
	REGISTER_OPPONENT_FUNCTIONS(Salko)

	_data->opponent->sequences.push_back(loadSequence("2004or.seq"));
	_data->opponent->sequences.push_back(loadSequence("2004oam.seq"));
	_data->opponent->sequences.push_back(loadSequence("2004oar.seq"));
	_data->opponent->sequences.push_back(loadSequence("2004okr.seq"));
	_data->opponent->sequences.push_back(loadSequence("2004ohm.seq"));
	_data->opponent->sequences.push_back(loadSequence("blank.seq"));

	getSound()->playSound(kEntityTables0, "MUS035", SoundManager::kFlagDefault);

	_data->opponent->countdown = 3;
	_data->opponent->field_38 = 30;
}

void Fight::handleActionSalko(Fighter *fighter, FightAction action) {
	switch (action) {
	default:
		handleAction(fighter, action);
		return;

	case kFightAction1:
	case kFightAction2:
		if (fighter->sequenceIndex != 1 && CHECK_SEQUENCE2(fighter, 4)) {
			fighter->field_34 = 0;

			setSequenceAndDraw(fighter, 3, kFightSequenceType1);
			setSequenceAndDraw(fighter->opponent, (action == kFightAction1 ? 3 : 4), kFightSequenceType1);

			CALL_FUNCTION1(fighter->opponent, handleAction, kFightAction103);

			if (action == kFightAction2)
				fighter->countdown= 0;

			CALL_FUNCTION0(fighter, update);
		} else {
			fighter->field_34++;
		}
		break;

	case kFightAction5:
		if (fighter->sequenceIndex != 3) {
			CALL_FUNCTION1(fighter->opponent, handleAction, kFightAction103);
			CALL_FUNCTION0(fighter, update);
		}
		break;

	case kFightAction128:
		setSequenceAndDraw(fighter, 1, kFightSequenceType0);
		fighter->field_34 = 0;
		break;

	case kFightAction131:
		setSequenceAndDraw(fighter, 2, (fighter->sequenceIndex ? kFightSequenceType2 : kFightSequenceType0));
		break;
	}
}

void Fight::updateSalko(Fighter *fighter) {
	update(fighter);

	// The original doesn't check for currentSequence2 != NULL (might not happen when everything is working properly, but crashes with our current implementation)
	if (fighter->frame && CHECK_SEQUENCE2(fighter, 2)) {

		if (fighter->opponent->countdown <= 0) {
			getSound()->removeFromQueue(kEntityTables0);
			bailout(kFightEndWin);

			return;
		}

		if (fighter->sequenceIndex == 2)
			CALL_FUNCTION1(fighter->opponent, handleAction, kFightAction2);
	}
}

bool Fight::canInteractSalko(Fighter const *fighter, FightAction action) {
	if (action == kFightAction131) {
		if (fighter->sequenceIndex == 1) {
			if (fighter->opponent->countdown <= 0)
				_engine->getCursor()->setStyle(kCursorHand);

			return true;
		}

		return false;
	}

	return canInteract(fighter);
}

void Fight::handleOpponentActionSalko(Fighter *fighter, FightAction action) {
	if (action == kFightAction2) {
		setSequenceAndDraw(fighter, 5, kFightSequenceType1);
		CALL_FUNCTION1(fighter->opponent, handleAction, kFightAction103);
	} else {
		handleAction(fighter, action);
	}
}

void Fight::updateOpponentSalko(Fighter *fighter) {
	// This is an opponent struct
	Opponent *opponent = (Opponent *)fighter;

	if (!opponent->field_38 && CALL_FUNCTION1(opponent, canInteract, kFightAction1) && !opponent->sequenceIndex2) {

		switch (rnd(5)) {
		default:
			break;

		case 0:
			setSequenceAndDraw(opponent, 1, kFightSequenceType0);
			break;

		case 1:
			setSequenceAndDraw(opponent, 2, kFightSequenceType0);
			break;

		case 2:
			setSequenceAndDraw(opponent, 1, kFightSequenceType0);
			setSequenceAndDraw(opponent, 2, kFightSequenceType2);
			break;

		case 3:
			setSequenceAndDraw(opponent, 2, kFightSequenceType0);
			setSequenceAndDraw(opponent, 1, kFightSequenceType2);
			break;

		case 4:
			setSequenceAndDraw(opponent, 1, kFightSequenceType0);
			setSequenceAndDraw(opponent, 1, kFightSequenceType2);
			break;
		}

		// Update field_38
		opponent->field_38 = 4 * opponent->countdown;
	}

	if (opponent->frame && CHECK_SEQUENCE2(opponent, 2)) {
		if (opponent->opponent->countdown <= 0) {
			getSound()->removeFromQueue(kEntityTables0);
			bailout(kFightEndLost);

			// Stop processing
			return;
		}

		if (opponent->sequenceIndex == 1 || opponent->sequenceIndex == 2)
			CALL_FUNCTION1(opponent->opponent, handleAction, (FightAction)opponent->sequenceIndex);
	}

	updateOpponent(opponent);
}

//////////////////////////////////////////////////////////////////////////
// Vesna
//////////////////////////////////////////////////////////////////////////

void Fight::loadVesnaPlayer() {
	REGISTER_PLAYER_FUNCTIONS(Vesna)

	_data->player->sequences.push_back(loadSequence("2005cr.seq"));
	_data->player->sequences.push_back(loadSequence("2005cdr.seq"));
	_data->player->sequences.push_back(loadSequence("2005cbr.seq"));
	_data->player->sequences.push_back(loadSequence("2005bk.seq"));
	_data->player->sequences.push_back(loadSequence("2005cdm1.seq"));
	_data->player->sequences.push_back(loadSequence("2005chl.seq"));
}

void Fight::loadVesnaOpponent() {
	REGISTER_OPPONENT_FUNCTIONS(Vesna)

	_data->opponent->sequences.push_back(loadSequence("2005or.seq"));
	_data->opponent->sequences.push_back(loadSequence("2005oam.seq"));
	_data->opponent->sequences.push_back(loadSequence("2005oar.seq"));
	_data->opponent->sequences.push_back(loadSequence("2005okml.seq"));
	_data->opponent->sequences.push_back(loadSequence("2005okr.seq"));
	_data->opponent->sequences.push_back(loadSequence("2005odm1.seq"));
	_data->opponent->sequences.push_back(loadSequence("2005csbm.seq"));
	_data->opponent->sequences.push_back(loadSequence("2005oam4.seq"));

	getSound()->playSound(kEntityTables0, "MUS038", SoundManager::kFlagDefault);

	_data->opponent->countdown = 4;
	_data->opponent->field_38 = 30;
}

void Fight::handleActionVesna(Fighter *fighter, FightAction action) {
	switch (action) {
	default:
		handleAction(fighter, action);
		return;

	case kFightAction1:
		if (fighter->sequenceIndex != 1) {
			CALL_FUNCTION1(fighter->opponent, handleAction, kFightAction103);
			CALL_FUNCTION0(fighter, update);
		} else {
			fighter->field_34++;
		}
		break;

	case kFightAction2:
		if (fighter->sequenceIndex != 2) {
			CALL_FUNCTION1(fighter->opponent, handleAction, kFightAction103);
			CALL_FUNCTION0(fighter, update);
		} else {
			fighter->field_34++;
		}
		break;

	case kFightAction5:
		if (fighter->sequenceIndex != 3) {
			CALL_FUNCTION1(fighter->opponent, handleAction, kFightAction103);
			CALL_FUNCTION0(fighter, update);
		}
		break;

	case kFightAction128:
		if (fighter->sequenceIndex == 1 && fighter->opponent->sequenceIndex == 1 && CHECK_SEQUENCE2(fighter, 4)) {
			setSequenceAndDraw(fighter, 5, kFightSequenceType1);
		} else {
			setSequenceAndDraw(fighter, (fighter->opponent->sequenceIndex == 5) ? 3 : 1, kFightSequenceType0);
		}
		break;

	case kFightAction132:
		setSequenceAndDraw(fighter, 2, kFightSequenceType0);
		break;
	}

	if (fighter->field_34 > 10) {
		setSequenceAndDraw(fighter->opponent, 5, kFightSequenceType2);
		fighter->opponent->countdown = 1;
		fighter->field_34 = 0;
	}
}

void Fight::updateVesna(Fighter *fighter) {
	if (fighter->frame && CHECK_SEQUENCE2(fighter, 2)) {

		if (fighter->sequenceIndex == 3)
			CALL_FUNCTION1(fighter->opponent, handleAction, kFightAction3);

		if (fighter->opponent->countdown <= 0) {
			getSound()->removeFromQueue(kEntityTables0);
			bailout(kFightEndWin);
			return;
		}

		if (fighter->sequenceIndex == 5)
			CALL_FUNCTION1(fighter->opponent, handleAction, kFightAction5);
	}

	update(fighter);
}

bool Fight::canInteractVesna(Fighter const *fighter, FightAction action) {
	if (action != kFightAction128)
		return canInteract(fighter);

	if (fighter->sequenceIndex != 1) {

		if (fighter->opponent->sequenceIndex == 5) {
			_engine->getCursor()->setStyle(kCursorDown);
			return true;
		}

		return canInteract(fighter);
	}

	if (fighter->opponent->sequenceIndex == 1 && CHECK_SEQUENCE2(fighter, 4)) {
		_engine->getCursor()->setStyle(kCursorPunchLeft);
		return true;
	}

	return false;
}

void Fight::handleOpponentActionVesna(Fighter *fighter, FightAction action) {
	switch (action) {
	default:
		handleAction(fighter, action);
		break;

	case kFightAction3:
		CALL_FUNCTION1(fighter->opponent, handleAction, kFightAction103);
		break;

	case kFightAction5:
		setSequenceAndDraw(fighter, 7, kFightSequenceType1);
		CALL_FUNCTION1(fighter->opponent, handleAction, kFightAction103);
		if (fighter->countdown <= 1)
			fighter->countdown = 1;
		break;

	case kFightAction131:
		break;
	}
}

void Fight::updateOpponentVesna(Fighter *fighter) {
	// This is an opponent struct
	Opponent *opponent = (Opponent *)fighter;

	if (!opponent->field_38 && CALL_FUNCTION1(opponent, canInteract, kFightAction1) && !opponent->sequenceIndex2) {

		if (opponent->opponent->field_34 == 1) {
			setSequenceAndDraw(opponent, 2, kFightSequenceType0);
		} else {
			switch (rnd(6)) {
			default:
				break;

			case 0:
				setSequenceAndDraw(opponent, 1, kFightSequenceType0);
				break;

			case 1:
				setSequenceAndDraw(opponent, 1, kFightSequenceType0);
				setSequenceAndDraw(opponent, 1, kFightSequenceType2);
				break;

			case 2:
				setSequenceAndDraw(opponent, 2, kFightSequenceType0);
				break;

			case 3:
				setSequenceAndDraw(opponent, 2, kFightSequenceType0);
				setSequenceAndDraw(opponent, 2, kFightSequenceType2);
				break;

			case 4:
				setSequenceAndDraw(opponent, 1, kFightSequenceType0);
				setSequenceAndDraw(opponent, 2, kFightSequenceType2);
				break;

			case 5:
				setSequenceAndDraw(opponent, 2, kFightSequenceType0);
				setSequenceAndDraw(opponent, 1, kFightSequenceType2);
				break;
			}
		}

		// Update field_38
		opponent->field_38 = 4 * opponent->countdown;
	}

	if (opponent->frame && CHECK_SEQUENCE2(opponent, 2)) {
		if (opponent->sequenceIndex == 1 || opponent->sequenceIndex == 2 || opponent->sequenceIndex == 5)
			CALL_FUNCTION1(opponent->opponent, handleAction, (FightAction)opponent->sequenceIndex);

		if (opponent->opponent->countdown <= 0) {

			switch (opponent->sequenceIndex) {
			default:
				break;

			case 1:
				setSequenceAndDraw(opponent, 3, kFightSequenceType1);
				break;

			case 2:
				setSequenceAndDraw(opponent, 4, kFightSequenceType1);
				break;

			case 5:
				setSequenceAndDraw(opponent, 6, kFightSequenceType1);
				break;
			}

			setSequenceAndDraw(opponent->opponent, 4, kFightSequenceType1);

			CALL_FUNCTION1(opponent, handleAction, kFightActionLost);
			CALL_FUNCTION0(opponent->opponent, update);
			CALL_FUNCTION0(opponent, update);

			getSound()->removeFromQueue(kEntityTables0);

			// Stop processing
			return;
		}
	}

	updateOpponent(opponent);
}

} // End of namespace LastExpress
