#include "backends/common/keymapper.h"
#include "backends/common/keymap-manager.h"
#include "common/config-manager.h"
namespace Common {

Keymapper::Keymapper(EventManager *evtMgr) {
	_eventMan = evtMgr;
	_keymapMan = new KeymapManager();
	_currentMap = 0;
	_hardwareKeys = 0;
}

void Keymapper::registerHardwareKeySet(HardwareKeySet *keys) {
	if (_hardwareKeys)
		error("Hardware key set already registered!\n");
	_hardwareKeys = keys;
}

const HardwareKeySet *Keymapper::getHardwareKeySet() const {
	return _hardwareKeys;
}

void Keymapper::addGlobalKeyMap(const String& name, Keymap *keymap) {
	_keymapMan->registerGlobalKeymap(name, keymap);
}

void Keymapper::addGameKeyMap(const String& name, Keymap *keymap) {
	if (_gameId.size() == 0) {
		initGame();
		if (_gameId.size() == 0)
			return;
	}
	_keymapMan->registerGameKeymap(name, keymap);
}

void Keymapper::initGame() {
	if (ConfMan.getActiveDomain() == 0)
		error("Call to Keymapper::initGame when no game loaded\n");

	if (_gameId.size() > 0)
		deInitGame();
	_gameId = ConfMan.getActiveDomainName();
}

void Keymapper::deInitGame() {
	_keymapMan->unregisterAllGameKeymaps();
	_gameId.clear();
}


bool Keymapper::switchKeymap(const String& name) {
	Keymap *new_map = _keymapMan->getKeymap(name);
	if (!new_map) {
		warning("Keymap '%s' could not be found\n", name.c_str());
		return false;
	}
	_currentMap = new_map;
	return true;
}

} // end of namespace Common
