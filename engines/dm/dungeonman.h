#ifndef DUNGEONMAN_H
#define DUNGEONMAN_H

#include "dm.h"

namespace DM {

class DungeonMan {
	DMEngine *_vm;
	uint32 _dungeonDataSize;
	byte *_dungeonData;
	DungeonMan(const DungeonMan &other); // no implementation on purpose
	void operator=(const DungeonMan &rhs); // no implementation on purpose
public:
	DungeonMan(DMEngine *dmEngine);
	~DungeonMan();
	void loadDungeonFile();
};

}

#endif
