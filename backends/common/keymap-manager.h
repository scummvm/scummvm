#ifndef COMMON_KEYMAP_MANAGER
#define COMMON_KEYMAP_MANAGER

#include "backends/common/hardware-key.h"
#include "backends/common/keymap.h"
#include "common/config-manager.h"
#include "common/hash-str.h"
#include "common/hashmap.h"

namespace Common {

class KeymapManager {
public:

	class Domain {
	public:
		Domain() : _defaultKeymap(0) {}
		~Domain() { deleteAllKeyMaps(); }

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

	void registerHardwareKeySet(HardwareKeySet *keys);

	void registerDefaultGlobalKeymap(Keymap *map);
	void registerGlobalKeymap(const String& name, Keymap *map);

	void registerDefaultGameKeymap(Keymap *map);
	void registerGameKeymap(const String& name, Keymap *map);

	void unregisterAllGameKeymaps();

	Keymap *KeymapManager::getKeymap(const String& name);

private:

	void initKeymap(ConfigManager::Domain *domain, const String& name, Keymap *keymap);
	bool loadKeymap(ConfigManager::Domain *domain, const String& name, Keymap *keymap);
	void saveKeymap(ConfigManager::Domain *domain, const String& name, const Keymap *keymap);
	void automaticMap(Keymap *map);
	bool isMapComplete(const Keymap *map);

	Domain _globalDomain;
	Domain _gameDomain;

	HardwareKeySet *_hardwareKeys;
};

} // end of namespace Common

#endif