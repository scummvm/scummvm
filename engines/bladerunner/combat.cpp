#include "bladerunner/combat.h"

#include "bladerunner/bladerunner.h"

#include "bladerunner/actor.h"
#include "bladerunner/settings.h"

namespace BladeRunner {

Combat::Combat(BladeRunnerEngine* vm) {
	_vm = vm;

	_active = false;
	_enabled = true;

	_ammoDamage[0] = 10;
	_ammoDamage[1] = 20;
	_ammoDamage[2] = 30;

	for (int i = 0; i < 9; i++) {
		_hitSoundId[i] = -1;
		_missSoundId[i] = -1;
	}
}

Combat::~Combat() {
}

void Combat::activate() {
	if(_enabled) {
		_vm->_playerActor->combatModeOn(-1, -1, -1, -1, 4, 7, 8, -1, -1, -1, _vm->_combat->_ammoDamage[_vm->_settings->getAmmoType()], 0, 0);
		_active = true;
	}
}

void Combat::deactivate() {
	if (_enabled) {
		_vm->_playerActor->combatModeOff();
		_active = false;
	}
}

bool Combat::isActive() {
	return _active;
}

void Combat::enable() {
	_enabled = true;
}

void Combat::disable() {
	_enabled = false;
}

void Combat::setHitSoundId(int row, int column, int soundId) {
	_hitSoundId[row * 3 + column] = soundId;
}

void Combat::setMissSoundId(int row, int column, int soundId) {
	_missSoundId[row * 3 + column] = soundId;
}

} // End of namespace BladeRunner
