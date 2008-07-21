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

	const HardwareKeySet *getHardwareKeySet() const;

	void addGlobalKeyMap(const String& name, Keymap *keymap);

	void addGameKeyMap(const String& name, Keymap *keymap);

	void initGame();
	void deInitGame();

	bool switchKeymap(const String& name);

private:

	typedef List<HardwareKey*>::iterator Iterator;

	EventManager *_eventMan;
	KeymapManager *_keymapMan;

	String _gameId;

	Keymap *_currentMap;

	const HardwareKeySet *_hardwareKeys;

};

} // end of namespace Common

#endif