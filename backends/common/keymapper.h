#ifndef COMMON_KEYMAPPER
#define COMMON_KEYMAPPER

#include "backends/common/keymap-manager.h"

namespace Common {

class Keymapper {
public:

	Keymapper();

	void addHardwareKey(const HardwareKey& key);
	void addGlobalKeyMap(const String& name, Keymap& keymap);

private:

	KeymapManager _manager;

	List<HardwareKey*> _hardwareKeys;

};

} // end of namespace Common

#endif