#include "loadsave.h"
#include "dungeonman.h"
#include "champion.h"



namespace DM {

LoadsaveMan::LoadsaveMan(DMEngine *vm) : _vm(vm) {}


LoadgameResponse LoadsaveMan::loadgame() {
	bool newGame = _vm->_dungeonMan->_messages.newGame;
	ChampionMan &cm = *_vm->_championMan;

	if (newGame) {
		_vm->_restartGameAllowed = false;
		cm._partyChampionCount = 0;
		cm._leaderHand = Thing::thingNone;
		_vm->_gameId = _vm->_rnd->getRandomNumber(65536) * _vm->_rnd->getRandomNumber(65536);
	} else {
		assert(false);
		// MISSING CODE: load game
	}


	if (newGame) {
		warning("MISSING CODE: Timline init, Group init");
	} else {
		assert(false);
		// MISSING CODE: load game
	}
	cm._partyDead = false;
	return kLoadgameSuccess;
}

}