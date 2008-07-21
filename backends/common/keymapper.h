#ifndef COMMON_KEYMAPPER
#define COMMON_KEYMAPPER

#include "backends/common/keymap.h"
#include "common/list.h"

namespace Common {

class KeymapManager;

class Keymapper {
public:

	Keymapper(EventManager *eventMan);

	void registerHardwareKeySet(HardwareKeySet *keys);
	const HardwareKeySet *getHardwareKeySet();
	void addGlobalKeyMap(const String& name, Keymap& keymap);
	void addGameKeyMap(const String& gameid, const String& name, Keymap& keymap);

private:

	typedef List<HardwareKey*>::iterator Iterator;

	EventManager *_eventMan;
	KeymapManager *_keymapMan;

	Keymap *_currentMap;

	const HardwareKeySet *_hardwareKeys;

};

} // end of namespace Common

#endif