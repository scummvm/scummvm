#include "backends/common/keymapper.h"
#include "backends/common/keymap-manager.h"

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

const HardwareKeySet *Keymapper::getHardwareKeySet() {
	return _hardwareKeys;
}

void Keymapper::addGlobalKeyMap(const String& name, Keymap& keymap) {
	_keymapMan->registerGlobalKeymap(name, keymap);
}

void Keymapper::addGameKeyMap(const String& gameid, const String& name, Keymap& keymap) {
	_keymapMan->registerKeymap(name, gameid, keymap);
}

} // end of namespace Common
