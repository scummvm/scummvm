#ifndef COMMON_KEYMAPPER
#define COMMON_KEYMAPPER

#include "backends/common/keymap-manager.h"

namespace Common {

class Keymapper {
public:

	Keymapper();

	

private:

	KeymapManager _manager;

	List<HardwareKey> _hardwareKeys;

};

} // end of namespace Common