#ifndef COMMON_KEYMAP_MANAGER
#define COMMON_KEYMAP_MANAGER

#include "backends/common/keymap.h"
#include "common/hash-str.h"
#include "common/hashmap.h"

namespace Common {

class KeymapManager {
public:

	class Domain {
	public:
		Domain() : _defaultKeymap(0) {}

		void addDefaultKeymap(Keymap *map);
		void addKeymap(const String& name, Keymap *map);

		void deleteAllKeyMaps();

		Keymap *getDefaultKeymap();
		Keymap *getKeymap(const String& name);

	private:
		typedef HashMap<String, Keymap*, 
			IgnoreCase_Hash, IgnoreCase_EqualTo> KeymapMap;

		Keymap *_defaultKeymap;
		KeymapMap _keymaps;
	};

	void registerDefaultGlobalKeymap(Keymap *map);
	void registerGlobalKeymap(const String& name, Keymap *map);

	void registerDefaultGameKeymap(Keymap *map);
	void registerGameKeymap(const String& name, Keymap *map);

	void unregisterAllGameKeymaps();

	Keymap *KeymapManager::getKeymap(const String& name);

private:

	Domain _globalDomain;
	Domain _gameDomain;
};

} // end of namespace Common

#endif